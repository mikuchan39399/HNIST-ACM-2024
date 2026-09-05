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
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
$inventory = Get-CheckInventory $root
. (Join-Path $PSScriptRoot 'check_verification.ps1')
$trackVerification = Test-Path -LiteralPath (Join-Path $root 'rules/verification.json')
if ($trackVerification) { $null=Read-VSpec $root $inventory }
$startedUtc=[DateTime]::UtcNow.ToString('o')
$gaps = @($inventory.entries | Where-Object { $_.kind -eq 'engine' -and $_.suites.Count -eq 0 })
foreach ($gap in $gaps) { Write-Host ('[TEST GAP] no direct test include: ' + $gap.path) -ForegroundColor Yellow }
if ($gaps.Count) { Write-Host ('[NOTE] ' + $gaps.Count + ' engines have no direct test include (warn only; not a behavior coverage metric)') -ForegroundColor Yellow }
$checks = @($inventory.checks | ForEach-Object { Get-Item -LiteralPath (Join-Path $root $_) })
if ($Filter) { $checks = @($checks | Where-Object { $_.BaseName.IndexOf($Filter, [StringComparison]::OrdinalIgnoreCase) -ge 0 }) }
$sources = @()
if ($Mode -ne 'Syntax') {
    if ($checks.Count -eq 0) { throw 'No regression suite selected; check -Filter.' }
    $sources += $checks
}
if ($Mode -ne 'Regression') {
    $syntax = @($inventory.engines | ForEach-Object { Get-Item -LiteralPath (Join-Path $root $_) })
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
    $before = Get-VSnapshot $root $relative
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
    $after = Get-VSnapshot $root $relative
    $results += [pscustomobject]@{fingerprint=$before.hash; files=$before.files; stable=($before.hash -ceq $after.hash);  path=$relative; phase=$(if ($isCheck) {'regression'} else {'syntax'}); status=$status; exitCode=$exitCode; seconds=[Math]::Round($elapsed, 3) }
    Write-Host ('[' + $status + '] ' + $relative)
}
$failed = @($results | Where-Object { $_.status -ne 'PASS' }).Count
$summary = [ordered]@{ compiler=$compilerPath; flags=$flags; mode=$Mode; filter=$Filter; sanitize=[bool]$Sanitize; total=$results.Count; failed=$failed; results=$results }
[IO.File]::WriteAllText((Join-Path $ReportDir 'summary.json'), ($summary | ConvertTo-Json -Depth 8), $utf8)
if ($trackVerification) {
    $id=[Guid]::NewGuid().ToString('N')
    $logs=$ReportDir
    if ($logs.StartsWith($root,[StringComparison]::OrdinalIgnoreCase)) { $logs=$logs.Substring($root.Length+1).Replace('\','/') }
    $evidence=[ordered]@{schema=1; id=$id; startedUtc=$startedUtc; finishedUtc=[DateTime]::UtcNow.ToString('o'); os=$(if([IO.Path]::DirectorySeparatorChar -eq '\'){'Windows'}else{'Linux'}); compilerVersion=(Read-VText (Join-Path $ReportDir 'compiler.stdout.log')).Split("`n")[0]; flags=$flags; sanitize=[bool]$Sanitize; logs=$logs; results=$results}
    $store=Join-Path $root 'records/verification/runs'
    $output=Join-Path $root 'docs/verification'
    if ($env:GITHUB_ACTIONS -eq 'true') { $store=Join-Path $ReportDir 'verification-runs'; $output=Join-Path $ReportDir 'verification' }
    [void][IO.Directory]::CreateDirectory($store)
    [IO.File]::WriteAllText((Join-Path $store ($id+'.json')),($evidence | ConvertTo-Json -Depth 10),$utf8)
    & (Join-Path $PSScriptRoot 'make_verification.ps1') -RunDir $store -OutputDir $output
}
Write-Host ('total: {0}, failed: {1}; logs: {2}' -f $results.Count, $failed, $ReportDir)
if ($failed) { exit 1 }
exit 0
