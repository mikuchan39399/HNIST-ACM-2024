param([string]$SettingsFile='', [string]$TasksFile='', [string]$KeybindingsFile='', [switch]$RemoveLibrary)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$paths=Setup-Paths $SettingsFile $TasksFile $KeybindingsFile
$sp=$paths.state; $lock=$null; $dirs=@()
try {
    if ([IO.File]::Exists($sp)) {
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
        $state=Setup-State $sp $root $paths; $dirs=@($state.dirs)
        if ($state.phase -ne 'installed') { Setup-Recover $state $sp }
        if ([IO.File]::Exists($sp)) {
            $state=Setup-State $sp $root $paths
            for ($i=0;$i -lt $state.docs.Count;$i++) {
                $d=$state.docs[$i]; $now=Setup-Snapshot $d.path; $target=$now
                if (Setup-Same $now $d.after) { $target=$d.before }
                elseif ($now.exists) {
                    $before='{}'; if ($d.before.exists) { $before=$d.before.text }
                    $next=$now.text
                    if ($i -eq 0) {
                        if ($state.incAdded) { $next=Setup-RestoreProperty $next $before $d.after.text 'C_Cpp.default.includePath' $state.zoi }
                        if ($state.cphAdded) { $next=Setup-RestoreProperty $next $before $d.after.text 'cph.language.cpp.Args' $state.flag }
                        foreach ($key in @($state.scalarKeys)) {
                            $current=JC-Get $next $key; $after=JC-Get $d.after.text $key
                            if ($null -ne $current -and $current -ceq $after) { $next=JC-Set $next $key (JC-Get $before $key) }
                        }
                    } elseif ($i -eq 1) {
                        $next=Setup-RemoveTasks $next $before $d.after.text $state.labels
                        if ($null -eq (JC-Get $before 'version') -and (JC-Get $next 'version') -eq '"2.0.0"') { $next=JC-Set $next 'version' $null }
                    } else {
                        $node=JC-Parse $next
                        if ($node.kind -ne '[') { throw 'Edited keybindings must be an array; preserved' }
                        for ($j=$node.children.Count-1;$j -ge 0;$j--) {
                            $raw=JC-Raw $next $node.children[$j].value
                            foreach ($owned in @($state.ownedKeys)) {
                                if ((JC-Normal $raw) -ceq (JC-Normal $owned)) { $next=JC-Cut $next $node $j; $node=JC-Parse $next; break }
                            }
                        }
                    }
                    $target=@{exists=$true;text=$next}
                    if (-not $d.before.exists -and (JC-Parse $next).children.Count -eq 0) { $target=@{exists=$false;text=''} }
                }
                $d.from=$now; $d.to=$target
            }
            $state.phase='removing'; Setup-Write $sp (Setup-Json $state)
            Setup-Recover $state $sp
        }
        Write-Host '[OK] managed settings/tasks/shortcuts restored; setup state removed; no .bak created'
    } else {
        if ([IO.File]::Exists((Join-Path $PSScriptRoot '.zoi-install-state.json'))) { throw 'Legacy setup state preserved; this installer cannot infer ownership of the old manual tasks.' }
        Write-Host '[OK] no v2/v3 installation state; configuration left untouched'
    }
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally {
    if ($null -ne $lock) { $lock.Dispose() }
    foreach ($dir in @($dirs | Sort-Object Length -Descending)) {
        if ([IO.Directory]::Exists($dir) -and @(Get-ChildItem -LiteralPath $dir -Force).Count -eq 0) { [IO.Directory]::Delete($dir) }
    }
}
if ($RemoveLibrary) {
    . (Join-Path $PSScriptRoot 'zoi_package.ps1')
    Set-Location -LiteralPath ([IO.Path]::GetTempPath())
    Remove-ZoiPackage $root
}
