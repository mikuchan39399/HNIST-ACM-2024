param([string]$SettingsFile='', [string]$TasksFile='', [switch]$RemoveLibrary)
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$paths=Setup-Paths $SettingsFile $TasksFile
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
                    } else {
                        $next=Setup-RemoveTasks $next $before $d.after.text $state.labels
                        if ($null -eq (JC-Get $before 'version') -and (JC-Get $next 'version') -eq '"2.0.0"') { $next=JC-Set $next 'version' $null }
                    }
                    $target=@{exists=$true;text=$next}
                    if (-not $d.before.exists -and (JC-Parse $next).children.Count -eq 0) { $target=@{exists=$false;text=''} }
                }
                $d.from=$now; $d.to=$target
            }
            $state.phase='removing'; Setup-Write $sp (Setup-Json $state)
            Setup-Recover $state $sp
        }
        Write-Host '[OK] settings/tasks restored; setup state removed; no .bak created'
    } else {
        if ([IO.File]::Exists((Join-Path $PSScriptRoot '.zoi-install-state.json'))) { throw 'Legacy setup state preserved; v2 cannot infer ownership of the old manual tasks.' }
        Write-Host '[OK] no v2 installation state; configuration left untouched'
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
