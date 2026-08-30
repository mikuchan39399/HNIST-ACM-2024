param([string]$Filter = '')
# run_checks.ps1 - one-shot regression runner (rule.cpp section 9)
# Usage: powershell -ExecutionPolicy Bypass -File run_checks.ps1              -> full regression
#        powershell -ExecutionPolicy Bypass -File run_checks.ps1 -Filter oset -> scoped (name contains 'oset')
# NOTE: keep this file ASCII-only; PS 5.1 reads no-BOM files as ANSI and CJK
#       comments can corrupt line structure.
$root = Split-Path -Parent $PSScriptRoot   # scripts/ -> library root
# scaffold sync preflight:
#   1. every catalog target exists          -> [STUB BROKEN]
#   2. every catalog entry has its .h stub  -> [STUB STALE]
#   3. no orphan stub outside catalog       -> [STUB ORPHAN]
#   4. every exemption glob matches >=1 file-> [EXEMPT DEAD]
#   5. every engine cpp is covered (catalog entry or '!' exemption)
#      -> [UNCOVERED]: a new template without a stub cannot slip through
# catalog line formats: "name<TAB>relpath" = stub entry, "!glob" = exemption
$cat = Join-Path $root 'zoi\_catalog.txt'
if (Test-Path -LiteralPath $cat) {
    $enc2 = New-Object System.Text.UTF8Encoding($false)
    $names = @{}
    $targets = @{}
    $exempt = @()
    $bad = 0
    foreach ($l in [IO.File]::ReadAllLines($cat, $enc2)) {
        $t = $l.Trim()
        if ($t -eq '' -or $t.StartsWith('#')) { continue }
        if ($t.StartsWith('!')) { $exempt += $t.Substring(1).Trim(); continue }
        $parts = $t -split "`t"
        if ($parts.Count -ne 2) {
            Write-Host ('[CATALOG BAD] expected name<TAB>path: ' + $t) -ForegroundColor Red
            $bad++
            continue
        }
        $name = $parts[0].Trim()
        $rel = $parts[1].Trim()
        if ($name -eq '' -or $rel -eq '' -or $rel.Contains("`t")) {
            Write-Host ('[CATALOG BAD] empty field: ' + $t) -ForegroundColor Red
            $bad++
            continue
        }
        $names[$name] = $true
        if (-not (Test-Path -LiteralPath (Join-Path $root $rel))) {
            Write-Host ('[STUB BROKEN] catalog target missing: ' + $rel) -ForegroundColor Red
            $bad++
        }
        if (-not (Test-Path -LiteralPath (Join-Path $root ('zoi\' + $name + '.h')))) {
            Write-Host ('[STUB STALE] stub file missing, rerun make_stubs: ' + $name) -ForegroundColor Red
            $bad++
        }
        $relN = $rel.Replace('\', '/')
        $targets[$relN] = $true
    }
    foreach ($h in Get-ChildItem -LiteralPath (Join-Path $root 'zoi') -Filter '*.h') {
        if (-not $names.ContainsKey($h.BaseName)) {
            Write-Host ('[STUB ORPHAN] stub not in catalog: ' + $h.BaseName) -ForegroundColor Red
            $bad++
        }
    }
    $engines = @()
    foreach ($cpp in Get-ChildItem -LiteralPath (Join-Path $root 'algorithms') -Recurse -Filter '*.cpp') {
        if ($cpp.Name.EndsWith('_check.cpp')) { continue }
        $r = $cpp.FullName.Substring($root.Length + 1).Replace('\', '/')
        $engines += $r
    }
    foreach ($e in $exempt) {
        $hit = $false
        foreach ($r in $engines) { if ($r -like $e) { $hit = $true; break } }
        if (-not $hit) {
            Write-Host ('[EXEMPT DEAD] exemption matches nothing: ' + $e) -ForegroundColor Red
            $bad++
        }
    }
    foreach ($r in $engines) {
        if ($targets.ContainsKey($r)) { continue }
        $hit = $false
        foreach ($e in $exempt) { if ($r -like $e) { $hit = $true; break } }
        if (-not $hit) {
            Write-Host ('[UNCOVERED] no stub and no exemption: ' + $r) -ForegroundColor Red
            $bad++
        }
    }
    if ($bad -gt 0) { Write-Host ('scaffold sync failed: ' + $bad + ' problem(s)'); exit 1 }
    Write-Host ('[OK] scaffold sync: ' + $targets.Count.ToString() + ' stubs, ' + $exempt.Count.ToString() + ' exemptions, full coverage')
}
$pattern = '*_check.cpp'
if ($Filter -ne '') { $pattern = '*' + $Filter + '*_check.cpp' }
$checks = Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Filter $pattern | Sort-Object FullName
$fail = 0
foreach ($c in $checks) {
    Push-Location $c.DirectoryName
    $exe = Join-Path $env:TEMP ($c.BaseName + '_run.exe')
    g++ -std=c++20 -Wall -Wextra -O2 $c.Name -o $exe
    if ($LASTEXITCODE -ne 0) {
        Write-Host ("[COMPILE FAIL] " + $c.FullName.Substring($root.Length + 1)) -ForegroundColor Red
        $fail++
    } else {
        & $exe
        if ($LASTEXITCODE -eq 0) {
            Write-Host ("[PASS] " + $c.FullName.Substring($root.Length + 1)) -ForegroundColor Green
        } else {
            Write-Host ("[RUN FAIL] " + $c.FullName.Substring($root.Length + 1)) -ForegroundColor Red
            $fail++
        }
    }
    Pop-Location
}
Write-Host ""
Write-Host ("total: $($checks.Count), failed: $fail")
exit $fail
