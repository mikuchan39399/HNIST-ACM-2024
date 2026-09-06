param([Parameter(Mandatory=$true)][string]$Workspace,[string]$Compiler='',[switch]$Undo)
# Configure the explicitly selected problem folder, never scan other projects.
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=Split-Path -Parent $PSScriptRoot
$workspacePath=[IO.Path]::GetFullPath($Workspace)
if (-not [IO.Directory]::Exists($workspacePath)) { throw 'Open an existing problem folder in VS Code first' }
$dir=Join-Path $workspacePath '.vscode'
$sp=Join-Path $dir '.zoi-workspace-state.json'
$files=@((Join-Path $dir 'c_cpp_properties.json'),(Join-Path $dir 'settings.json'))
$lock=$null
try {
    $old=$null
    if ([IO.File]::Exists($sp)) {
        $old=Setup-Read $sp | ConvertFrom-Json
        if ($old.format -ne 1 -or $old.root -ne $root -or $old.docs.Count -ne 2) { throw 'Unknown/other workspace state; preserved' }
        for ($i=0;$i -lt 2;$i++) {
            if ($old.docs[$i].path -ne $files[$i]) { throw 'Invalid workspace state paths' }
            if (-not (Setup-Same (Setup-Snapshot $files[$i]) $old.docs[$i].after) -and
                -not (Setup-Same (Setup-Snapshot $files[$i]) $old.docs[$i].before)) {
                throw ('Workspace configuration changed after setup; retained. Keep your edits and reconcile first: '+$files[$i])
            }
        }
    }
    if ($Undo) {
        if (-not $old) { Write-Host '[OK] no workspace setup state'; exit 0 }
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
        foreach ($d in $old.docs) {
            if ($d.before.exists) { Setup-Write $d.path $d.before.text }
            elseif ([IO.File]::Exists($d.path)) { [IO.File]::Delete($d.path) }
        }
        [IO.File]::Delete($sp)
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
        if (@(JC-Value $inc) -notcontains $zoi) { $inc=JC-Append $inc (Setup-Json $zoi) }
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
        $docs+=@{path=$files[$i]; before=$b; after=@{exists=$true;text=$texts[$i]}}
    }
    [void][IO.Directory]::CreateDirectory($dir)
    $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
    for ($i=0;$i -lt 2;$i++) { if (-not (Setup-Same (Setup-Snapshot $files[$i]) $before[$i])) { throw 'Workspace changed during preparation' } }
    Setup-Write $sp (Setup-Json @{format=1;root=$root;docs=$docs})
    foreach ($d in $docs) { Setup-Write $d.path $d.after.text }
    Write-Host ('[OK] configured: '+$workspacePath)
    Write-Host '[NEXT] Enable Microsoft C/C++ in this profile, reload VS Code, use #include "bit.h".'
    if (-not $Compiler) { Write-Host '[WARN] g++ not found on PATH; select the actual compiler in C/C++ configurations.' }
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally { if ($lock) { $lock.Dispose() } }
