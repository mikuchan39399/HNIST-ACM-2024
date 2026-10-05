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
                $d=$state.docs[$i]; $now=Setup-Snapshot $d.path
                $target=Setup-UninstallDocument $state $i $now
                $d.from=$now; $d.to=$target
            }
            $state.phase='removing'; Setup-Write $sp (Setup-Json $state)
            Setup-Recover $state $sp
        }
        Write-Host '[OK] unchanged managed entries restored; user edits retained; setup state removed; no .bak created'
    } else {
        if ([IO.File]::Exists((Join-Path $PSScriptRoot '.zoi-install-state.json'))) { throw 'Legacy setup state preserved; this installer cannot infer ownership of the old manual tasks.' }
        Write-Host '[OK] no v2/v3 installation state; configuration left untouched'
    }
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally {
    if ($null -ne $lock) { $lock.Dispose() }
    Setup-CleanDirs $dirs
}
if ($RemoveLibrary) {
    . (Join-Path $PSScriptRoot 'zoi_package.ps1')
    Set-Location -LiteralPath ([IO.Path]::GetTempPath())
    Remove-ZoiPackage $root
}
