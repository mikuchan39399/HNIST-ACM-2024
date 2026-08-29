param([string]$Filter = '')
# run_checks.ps1 - one-shot regression runner (rule.cpp section 9)
# Usage: powershell -ExecutionPolicy Bypass -File run_checks.ps1              -> full regression
#        powershell -ExecutionPolicy Bypass -File run_checks.ps1 -Filter oset -> scoped (name contains 'oset')
# NOTE: keep this file ASCII-only; PS 5.1 reads no-BOM files as ANSI and CJK
#       comments can corrupt line structure.
$root = $PSScriptRoot
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
