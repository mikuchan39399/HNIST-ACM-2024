param([Parameter(Mandatory=$true)][string]$Workspace,[string]$Compiler='',[switch]$Undo)
# Configure the explicitly selected problem folder, never scan other projects.
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$workspacePath=[IO.Path]::GetFullPath($Workspace)
if (-not [IO.Directory]::Exists($workspacePath)) { throw 'Open an existing problem folder in VS Code first' }
$dir=Join-Path $workspacePath '.vscode'
$sp=Join-Path $dir '.zoi-workspace-state.json'
$files=@((Join-Path $dir 'c_cpp_properties.json'),(Join-Path $dir 'settings.json'))
$lock=$null; $dirs=@(); $newDirs=@()
try {
    foreach ($p in @($sp,($sp+'.lock'),($sp+'.zoi-tmp'))+$files) { Setup-CheckPath $p }
    if ([IO.Directory]::Exists($dir)) {
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
    }
    $old=$null
    if ([IO.File]::Exists($sp)) {
        $old=Setup-Read $sp | ConvertFrom-Json
        if ($old.format -notin @(1,2) -or $old.root -ne $root -or $old.docs.Count -ne 2) { throw 'Unknown/other workspace state; preserved' }
        if ($old.format -eq 2) {
            if ($old.phase -notin @('installing','installed','removing')) { throw 'Invalid workspace phase; preserved' }
            foreach ($d in $old.dirs) { if ($d -cne $dir) { throw 'Invalid workspace directory ownership; preserved' } }
            $dirs=@($old.dirs)
        }
        for ($i=0;$i -lt 2;$i++) {
            if ($old.docs[$i].path -ne $files[$i]) { throw 'Invalid workspace state paths' }
            $fields=@('before','after'); if ($old.format -eq 2) { $fields+=@('from','to') }
            foreach ($field in $fields) {
                $value=$old.docs[$i].$field
                if ($value.exists -isnot [bool] -or $value.text -isnot [string]) { throw 'Invalid workspace snapshot; preserved' }
                if ($value.exists) { $null=JC-Parse $value.text }
            }
        }
        if ($old.format -eq 2 -and $old.phase -ne 'installed') {
            $phase=$old.phase; Setup-Recover $old $sp
            if ($phase -eq 'removing') {
                if ($Undo) { Write-Host '[OK] interrupted workspace undo completed'; exit 0 }
                $old=$null
            }
        }
        if ($old) { for ($i=0;$i -lt 2;$i++) {
            $now=Setup-Snapshot $files[$i]
            if (-not (Setup-Same $now $old.docs[$i].after) -and -not (Setup-Same $now $old.docs[$i].before)) {
                throw ('Workspace configuration changed after setup; retained. Keep your edits and reconcile first: '+$files[$i])
            }
        } }
    }
    $stateBefore=Setup-Snapshot $sp
    if ($Undo) {
        if (-not $old) { Write-Host '[OK] no workspace setup state'; exit 0 }
        $docs=@()
        foreach ($d in $old.docs) {
            $docs+=@{path=$d.path;before=$d.before;after=$d.after;from=(Setup-Snapshot $d.path);to=$d.before}
        }
        $state=@{format=2;root=$root;phase='removing';dirs=$dirs;docs=$docs}
        Setup-Write $sp (Setup-Json $state)
        Setup-Recover $state $sp
        Write-Host '[OK] workspace configuration restored'; exit 0
    }
    $before=@($files | ForEach-Object { Setup-Snapshot $_ })
    $cpp='{"configurations":[{"name":"ZOI","includePath":["${workspaceFolder}/**"]}],"version":4}'
    if ($before[0].exists) { $cpp=$before[0].text }
    $raw=JC-Get $cpp 'configurations'
    if ($null -eq $raw -or (JC-Parse $raw).kind -ne '[' -or (JC-Parse $raw).children.Count -eq 0) { throw 'c_cpp_properties.json needs a nonempty configurations array' }
    $new='[]'; $zoi=$root.Replace('\','/')+'/zoi'
    if (-not $Compiler) { $c=Get-Command g++ -ErrorAction SilentlyContinue; if ($c) { $Compiler=$c.Source } }
    if ($Compiler -and -not [IO.File]::Exists($Compiler)) { throw 'Compiler must be an existing executable path' }
    foreach ($node in (JC-Parse $raw).children) {
        $cfg=JC-Raw $raw $node.value
        $inc=JC-Get $cfg 'includePath'; if ($null -eq $inc) { $inc='["${default}"]' }
        if ((JC-Parse $inc).kind -ne '[') { throw 'Each includePath must be an array' }
        $includeValues=JC-Value $inc
        if (@($includeValues) -notcontains $zoi) { $inc=JC-Append $inc (Setup-Json $zoi) }
        $cfg=JC-Set $cfg 'includePath' $inc
        if ($null -eq (JC-Get $cfg 'cppStandard')) { $cfg=JC-Set $cfg 'cppStandard' '"c++20"' }
        if ($Compiler -and $null -eq (JC-Get $cfg 'compilerPath')) { $cfg=JC-Set $cfg 'compilerPath' (Setup-Json $Compiler.Replace('\','/')) }
        if ((JC-Get $cfg 'configurationProvider') -or (JC-Get $cfg 'compileCommands')) {
            Write-Host '[WARN] A provider/compileCommands may override includePath; configure its compiler command if needed.'
        }
        $new=JC-Append $new $cfg
    }
    $cpp=JC-Set $cpp 'configurations' $new
    $settings='{}'; if ($before[1].exists) { $settings=$before[1].text }
    foreach ($key in @('C_Cpp.autocomplete','C_Cpp.errorSquiggles','C_Cpp.intelliSenseEngine')) {
        $v=if ($key -eq 'C_Cpp.errorSquiggles') {'enabled'} else {'default'}
        $settings=JC-Set $settings $key (Setup-Json $v)
    }
    $texts=@($cpp,$settings); $docs=@()
    for ($i=0;$i -lt 2;$i++) {
        $b=$before[$i]; if ($old) { $b=$old.docs[$i].before }
        $docs+=@{path=$files[$i]; before=$b; after=@{exists=$true;text=$texts[$i]};from=$before[$i];to=@{exists=$true;text=$texts[$i]}}
    }
    if (-not [IO.Directory]::Exists($dir)) { $newDirs=@($dir); [void][IO.Directory]::CreateDirectory($dir) }
    if ($null -eq $lock) { $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose) }
    if (-not (Setup-Same (Setup-Snapshot $sp) $stateBefore)) { throw 'Another workspace setup won the race; rerun configure' }
    for ($i=0;$i -lt 2;$i++) { if (-not (Setup-Same (Setup-Snapshot $files[$i]) $before[$i])) { throw 'Workspace changed during preparation' } }
    if ($old -and $old.format -eq 2 -and @($docs | Where-Object { -not $_.from.exists -or (JC-Normal $_.from.text) -cne (JC-Normal $_.to.text) }).Count -eq 0) {
        Write-Host '[OK] workspace already configured'; exit 0
    }
    $dirs=@($dirs)+$newDirs
    $state=@{format=2;root=$root;phase='installing';dirs=$dirs;docs=$docs}
    Setup-Write $sp (Setup-Json $state)
    Setup-Recover $state $sp
    Write-Host ('[OK] configured: '+$workspacePath)
    Write-Host '[NEXT] Enable Microsoft C/C++ in this profile, reload VS Code, use #include "bit.h".'
    if (-not $Compiler) { Write-Host '[WARN] g++ not found on PATH; select the actual compiler in C/C++ configurations.' }
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally {
    if ($lock) { $lock.Dispose() }
    if (-not [IO.File]::Exists($sp)) { Setup-CleanDirs (@($dirs)+$newDirs) }
}
