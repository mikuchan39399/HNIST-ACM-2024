# make_stubs.ps1 - (re)generate zoi/ stub headers from zoi/_catalog.txt
# Usage: powershell -ExecutionPolicy Bypass -File make_stubs.ps1
# Catalog line format: <shortname><TAB><target path relative to library root>
# NOTE: keep this file ASCII-only; PS 5.1 reads no-BOM files as ANSI and CJK
#       comments can corrupt line structure. Catalog is read with explicit UTF-8.
$Root = Split-Path -Parent $PSScriptRoot   # scripts/ -> library root
$enc = New-Object System.Text.UTF8Encoding($false)
$cat = Join-Path $Root 'zoi\_catalog.txt'
if (-not (Test-Path -LiteralPath $cat)) { Write-Host '[FAIL] catalog not found'; exit 1 }
$lines = [IO.File]::ReadAllLines($cat, $enc)
$n = 0
$names = @()
foreach ($l in $lines) {
    $t = $l.Trim()
    if ($t -eq '' -or $t.StartsWith('#') -or $t.StartsWith('!')) { continue }
    $parts = $t -split "`t", 2
    if ($parts.Count -ne 2) { Write-Host ('[SKIP] bad line: ' + $t); continue }
    $name = $parts[0].Trim()
    $target = $parts[1].Trim()
    $names += $name
    $stub = Join-Path (Join-Path $Root 'zoi') ($name + '.h')
    $body = '// zoi stub -> ' + $target + "`n" + '#include "../' + $target + '"' + "`n"
    [IO.File]::WriteAllText($stub, $body, $enc)
    $n++
}
# self-clean: drop .h stubs no longer named in the catalog (renames leave
# old files behind otherwise; note NTFS overwrites case-only renames in
# place, so pure-case changes like fhqseq->fhqSeq need a manual delete first)
$removed = 0
foreach ($h in Get-ChildItem -LiteralPath (Join-Path $Root 'zoi') -Filter '*.h') {
    if ($names -notcontains $h.BaseName) {
        Remove-Item -LiteralPath $h.FullName
        $removed++
    }
}
Write-Host ('[OK] generated ' + $n + ' stubs, removed ' + $removed + ' orphan(s) under zoi\')
exit 0