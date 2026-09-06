param([string]$BuildRoot='',[ValidateRange(0,1000)][int]$KeepLast=3,[switch]$Apply)
# Only completed, marked test workspaces are candidates. Never infer ownership from a name alone.
$ErrorActionPreference='Stop'
if (-not $BuildRoot) { $BuildRoot=Join-Path (Split-Path -Parent $PSScriptRoot) '.zoi-checks' }
$base=[IO.Path]::GetFullPath($BuildRoot).TrimEnd('\','/')
if (-not [IO.Directory]::Exists($base)) { Write-Host '[OK] no test cache'; exit 0 }
if ((Get-Item -LiteralPath $base -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Reparse cache root refused' }
$cleanupLock=$null
try {
    $cleanupLock=New-Object IO.FileStream((Join-Path $base '.zoi-clean.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
} catch [IO.IOException] {
    Write-Host '[NOTE] Another cache cleanup is active; skipped this invocation.'
    exit 0
}
try {
$candidates=@(); $skipped=0
foreach ($dir in Get-ChildItem -LiteralPath $base -Directory -Force) {
    if ($dir.Name -eq 'codex-work' -or ($dir.Attributes -band [IO.FileAttributes]::ReparsePoint)) { $skipped++; continue }
    $marker=Join-Path $dir.FullName '.zoi-run.json'
    if (-not [IO.File]::Exists($marker)) { $skipped++; continue }
    try { $m=[IO.File]::ReadAllText($marker) | ConvertFrom-Json } catch { $skipped++; continue }
    if ($m.format -ne 1 -or $m.status -ne 'PASS' -or $m.path -ne $dir.FullName -or $m.kind -notin @('regression','setup','expand','runner','tooling')) { $skipped++; continue }
    try { $date=[DateTime]::Parse($m.completedUtc).ToUniversalTime() } catch { $skipped++; continue }
    $candidates += [pscustomobject]@{path=$dir.FullName; kind=$m.kind; date=$date}
}
$removed=0; $bytes=0L
foreach ($group in @($candidates | Group-Object kind)) {
    foreach ($item in @($group.Group | Sort-Object date -Descending | Select-Object -Skip $KeepLast)) {
        $target=[IO.Path]::GetFullPath($item.path)
        if ((Split-Path -Parent $target) -ne $base) { throw 'Cleanup target escaped cache root' }
        $pending=New-Object 'Collections.Generic.Stack[string]'; $pending.Push($target)
        $size=0L; $safe=$true
        while ($pending.Count) {
            foreach ($entry in Get-ChildItem -LiteralPath $pending.Pop() -Force) {
                if ($entry.Attributes -band [IO.FileAttributes]::ReparsePoint) { $safe=$false; continue }
                if ($entry.PSIsContainer) { $pending.Push($entry.FullName) } else { $size+=$entry.Length }
            }
        }
        if (-not $safe) { Write-Host ('[KEEP] reparse point inside '+$target); continue }
        Write-Host ('['+$(if ($Apply) {'REMOVE'} else {'PREVIEW'})+'] '+$target+' ('+[math]::Round($size/1MB,2)+' MiB)')
        if ($Apply) { Remove-Item -LiteralPath $target -Recurse -Force }
        $removed++; $bytes+=$size
    }
}
Write-Host ('[OK] candidates='+$removed+'; MiB='+[math]::Round($bytes/1MB,2)+'; unmarked/failed/protected='+$skipped+'; keep per kind='+$KeepLast)
if (-not $Apply) { Write-Host '[NOTE] Preview only. Use -Apply to remove listed completed workspaces.' }
} finally { if ($cleanupLock) { $cleanupLock.Dispose() } }
