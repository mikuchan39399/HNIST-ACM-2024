param(
    [string]$Filter = '',
    [ValidateSet('Regression', 'Syntax', 'All')][string]$Mode = 'Regression',
    [string]$Compiler = 'g++',
    [switch]$Sanitize,
    [ValidateRange(1, 3600)][int]$TimeoutSec = 120,
    [ValidateRange(1, 3600)][int]$CompileTimeoutSec = 120,
    [string]$BuildRoot = '',
    [string]$ReportDir = ''
)
$ErrorActionPreference = 'Stop'
if ($Filter -and $Mode -ne 'Regression') { throw '-Filter applies only to Regression mode.' }

# run_checks.ps1 - one-shot regression runner (rule.md section 9)
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
$zoiDir = Join-Path $root 'zoi'
$cat = Join-Path $zoiDir '_catalog.txt'
if (-not (Test-Path -LiteralPath $cat)) { throw 'Catalog missing; preflight cannot be skipped.' }
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
        if ($t.StartsWith('^')) {
            # prose entry (^name<TAB>file): target must exist; no stub, no stamp, no coverage duty
            $pp = $t.Substring(1) -split "`t"
            if ($pp.Count -ne 2 -or -not (Test-Path -LiteralPath (Join-Path $root ($pp[1].Trim())))) {
                Write-Host ('[PROSE BROKEN] prose target missing: ' + $t) -ForegroundColor Red
                $bad++
            }
            continue
        }
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
        if ($names.ContainsKey($name) -or $targets.ContainsKey($rel.Replace('\', '/'))) {
            Write-Host ('[CATALOG DUPLICATE] ' + $t)
            $bad++
        }
        $names[$name] = $true
        if (-not (Test-Path -LiteralPath (Join-Path $root $rel))) {
            Write-Host ('[STUB BROKEN] catalog target missing: ' + $rel) -ForegroundColor Red
            $bad++
        }
        if (-not (Test-Path -LiteralPath (Join-Path $zoiDir ($name + '.h')))) {
            Write-Host ('[STUB STALE] stub file missing, rerun make_stubs: ' + $name) -ForegroundColor Red
            $bad++
        }
        $stubPath = Join-Path $zoiDir ($name + '.h')
        if (Test-Path -LiteralPath $stubPath) {
            $stubLines = [IO.File]::ReadAllLines($stubPath, $enc2)
            $expected = '#include "../' + $rel + '"'
            $actual = @($stubLines | Where-Object { $_ -match '^\s*#\s*include' })
            if ($actual.Count -ne 1 -or $actual[0].Trim() -cne $expected) {
                Write-Host ('[STUB STALE] wrong include target: ' + $name)
                $bad++
            }
        }
        $relN = $rel.Replace('\', '/')
        $targets[$relN] = $true
        # 7th check data: engine first line must be '// zoi: <name>' (stamp)
        $engFirst = ''
        $engPath = Join-Path $root ($rel.Replace('\', '/'))
        if (Test-Path -LiteralPath $engPath) { $engFirst = ([IO.File]::ReadAllLines($engPath, $enc2) | Select-Object -First 1) }
        if ($engFirst -ne ('// zoi: ' + $name)) {
            Write-Host ('[STUB MISMATCH] engine stamp wrong or missing (rerun make_stubs): ' + $rel + ' -> ' + $name) -ForegroundColor Red
            $bad++
        }
    }
    foreach ($h in Get-ChildItem -LiteralPath $zoiDir -Filter '*.h') {
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
    # sixth check (WARN level, not a gate): stubbed engines whose basename is
    # not directly included by any *_check.cpp. Resolved direct include paths only --
    # no substring matching (rule: grep hit != code call). Light pieces may
    # legitimately lack a suite, so this is a dashboard, not a failure.
    $covered = @{}
    foreach ($cf in Get-ChildItem -LiteralPath (Join-Path $root 'algorithms') -Recurse -Filter '*_check.cpp') {
        foreach ($cl in [IO.File]::ReadAllLines($cf.FullName, $enc2)) {
            if ($cl -match '^\s*#\s*include\s*"([^"]+)"') {
                $included = [IO.Path]::GetFullPath((Join-Path $cf.DirectoryName $Matches[1]))
                $covered[$included] = $true
            }
        }
    }
    $gaps = 0
    foreach ($tv in $targets.Keys) {
        if (-not $covered.ContainsKey([IO.Path]::GetFullPath((Join-Path $root $tv)))) {
            Write-Host ('[TEST GAP] no direct test include: ' + $tv) -ForegroundColor Yellow
            $gaps++
        }
    }
    if ($gaps -gt 0) {
        Write-Host ('[NOTE] ' + $gaps.ToString() + ' engines have no direct test include (warn only; not a behavior coverage metric)') -ForegroundColor Yellow
    }
    if ($targets.Count -eq 0) { throw 'Catalog has no engines.' }
    if ($bad -gt 0) { Write-Host ('scaffold sync failed: ' + $bad + ' problem(s)'); exit 1 }
    Write-Host ('[OK] scaffold sync: ' + $targets.Count.ToString() + ' stubs checked, ' + $exempt.Count.ToString() + ' exemptions, catalog coverage')
}
$checks = @(Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Filter '*_check.cpp' | Sort-Object FullName)
if ($Filter) { $checks = @($checks | Where-Object { $_.BaseName.IndexOf($Filter, [StringComparison]::OrdinalIgnoreCase) -ge 0 }) }
$sources = @()
if ($Mode -ne 'Syntax') {
    if ($checks.Count -eq 0) { throw 'No regression suite selected; check -Filter.' }
    $sources += $checks
}
if ($Mode -ne 'Regression') {
    $syntax = @(Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Filter '*.cpp' | Where-Object { $_.Name -notlike '*_check.cpp' } | Sort-Object FullName)
    if ($syntax.Count -eq 0) { throw 'No syntax targets found.' }
    $sources += $syntax
}
$compilerPath = (Get-Command $Compiler -CommandType Application -ErrorAction Stop | Select-Object -First 1).Source
if (-not $BuildRoot) { $BuildRoot = Join-Path $root '.zoi-checks' }
$BuildRoot = [IO.Path]::GetFullPath($BuildRoot)
$runDir = Join-Path $BuildRoot ([Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $runDir -Force | Out-Null
if (-not $ReportDir) { $ReportDir = Join-Path $runDir 'logs' }
$ReportDir = [IO.Path]::GetFullPath($ReportDir)
New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null
$utf8 = New-Object System.Text.UTF8Encoding($false)
. (Join-Path $PSScriptRoot 'check_process.ps1')
$version = Invoke-CheckProcess $compilerPath @('--version') $runDir 30 (Join-Path $ReportDir 'compiler')
if ($version.ExitCode -ne 0) { throw 'Compiler version probe failed.' }
$flags = @('-std=c++20', '-Wall', '-Wextra', '-Werror', '-UNDEBUG')
if ($Sanitize) {
    $flags += @('-O1', '-g', '-fsanitize=address,undefined', '-fno-sanitize-recover=all', '-fno-omit-frame-pointer', '-D_GLIBCXX_ASSERTIONS')
} else { $flags += '-O2' }
$results = @()
$index = 0
foreach ($source in $sources) {
    $index++
    $relative = $source.FullName.Substring($root.Length + 1).Replace('\', '/')
    $isCheck = $source.Name.EndsWith('_check.cpp')
    $label = '{0:D3}-{1}' -f $index, $source.BaseName
    $exe = Join-Path $runDir ($label + '.exe')
    $compileArgs = $flags + @($source.FullName)
    if ($isCheck) { $compileArgs += @('-o', $exe) } else { $compileArgs += '-fsyntax-only' }
    Write-Host ('[START] ' + $relative)
    $compiled = Invoke-CheckProcess $compilerPath $compileArgs $runDir $CompileTimeoutSec (Join-Path $ReportDir ($label + '-compile'))
    $status = 'PASS'
    $exitCode = $compiled.ExitCode
    $elapsed = $compiled.Seconds
    if ($compiled.TimedOut) { $status = 'COMPILE TIMEOUT' }
    elseif ($exitCode -ne 0) { $status = 'COMPILE FAIL' }
    elseif ($isCheck) {
        $ran = Invoke-CheckProcess $exe @() $runDir $TimeoutSec (Join-Path $ReportDir ($label + '-run'))
        $exitCode = $ran.ExitCode
        $elapsed += $ran.Seconds
        if ($ran.TimedOut) { $status = 'RUN TIMEOUT' }
        elseif ($exitCode -ne 0) { $status = 'RUN FAIL' }
    }
    if (Test-Path -LiteralPath $exe) { Remove-Item -LiteralPath $exe -Force }
    $results += [pscustomobject]@{ path=$relative; phase=$(if ($isCheck) {'regression'} else {'syntax'}); status=$status; exitCode=$exitCode; seconds=[Math]::Round($elapsed, 3) }
    Write-Host ('[' + $status + '] ' + $relative)
}
$failed = @($results | Where-Object { $_.status -ne 'PASS' }).Count
$summary = [ordered]@{ compiler=$compilerPath; flags=$flags; mode=$Mode; filter=$Filter; sanitize=[bool]$Sanitize; total=$results.Count; failed=$failed; results=$results }
[IO.File]::WriteAllText((Join-Path $ReportDir 'summary.json'), ($summary | ConvertTo-Json -Depth 5), $utf8)
Write-Host ('total: {0}, failed: {1}; logs: {2}' -f $results.Count, $failed, $ReportDir)
if ($failed) { exit 1 }
exit 0
