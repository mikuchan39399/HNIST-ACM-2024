param([string]$Compiler='g++')
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'check_verification.ps1')
. (Join-Path $PSScriptRoot 'check_process.ps1')
$root=Split-Path -Parent $PSScriptRoot
$fixture=Join-Path $root ('.zoi-checks/verification-test-'+[Guid]::NewGuid().ToString('N'))
foreach($d in @('scripts','rules','zoi','algorithms','records/verification/runs')) { [void][IO.Directory]::CreateDirectory((Join-Path $fixture $d)) }
foreach($f in @('run_checks.ps1','check_process.ps1','check_inventory.ps1','check_verification.ps1','make_verification.ps1')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $f) -Destination (Join-Path $fixture ('scripts/'+$f)) }
$enc=New-Object Text.UTF8Encoding($false)
function Put([string]$p,[string]$s) { [IO.File]::WriteAllText((Join-Path $fixture $p),$s,$enc) }
function Assert([bool]$ok,[string]$why) { if(-not $ok){throw $why} }
function U([string]$hex) { -join @($hex.Split(' ') | ForEach-Object {[char][Convert]::ToInt32($_,16)}) }
Put 'algorithms/dep.h' "#pragma once`ninline int value() { return 1; }`n"
Put 'algorithms/engine.cpp' "// zoi: tiny`n#include `"dep.h`"`n"
Put 'algorithms/one_check.cpp' "#include `"engine.cpp`"`nint main(){return value()-1;}`n"
Put 'algorithms/CF_fixture.cpp' 'int solution;'
Put 'zoi/_catalog.txt' "tiny`talgorithms/engine.cpp`n!algorithms/CF_fixture.cpp`n"
Put 'zoi/tiny.h' '#include "../algorithms/engine.cpp"'
Put 'rules/verification.json' '{"schema":1,"coverage":[{"template":"algorithms/engine.cpp","suite":"algorithms/one_check.cpp","api":["value"],"oracle":"constant","cases":["single"],"summary":"value","limitations":"tiny fixture"}]}'
$a=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
Put 'algorithms/dep.h' "#pragma once`r`ninline int value() { return 1; }`r`n"
Assert ((Get-VSnapshot $fixture 'algorithms/one_check.cpp').hash -eq $a.hash) 'CRLF should be normalized'
Put 'algorithms/dep.h' "#pragma once`ninline int value() { return 2; }`n"
Assert ((Get-VSnapshot $fixture 'algorithms/one_check.cpp').hash -ne $a.hash) 'Transitive edit not detected'
Put 'algorithms/dep.h' "#pragma once`ninline int value() { return 1; }`n"
$r=[pscustomobject]@{stable=$true;fingerprint=$a.hash;status='PASS'}
Assert ((Get-VState $null $a.hash) -eq 'none') 'No evidence was not unknown'
Assert ((Get-VState $r $a.hash) -eq 'pass') 'Pass classification'
$r.stable=$false
Assert ((Get-VState $r $a.hash) -eq 'stale') 'Mid-run mutation must be stale'
$r.stable=$true; $r.status='RUN FAIL'
Assert ((Get-VState $r $a.hash) -eq 'fail') 'Failure classification'
$hostExe=(Get-Process -Id $PID).Path
$script:index=0
function Run([string]$script,[string[]]$extra,[int]$expected=0) {
 $script:index++
 $p=Invoke-CheckProcess $hostExe (@('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $fixture ('scripts/'+$script)))+$extra) $fixture 90 (Join-Path $fixture ('step-'+$script:index))
 Assert (-not $p.TimedOut -and $p.ExitCode -eq $expected) ('Command failed: '+$script+'; logs: '+$fixture)
}
# Do not let CI mode redirect fixture evidence away from the test's private store.
$oldCI=$env:GITHUB_ACTIONS
try {
 $env:GITHUB_ACTIONS='false'
 Run 'run_checks.ps1' @('-Compiler',$Compiler) 0
 $e=@(Get-ChildItem (Join-Path $fixture 'records/verification/runs') -Filter '*.json')
 Assert ($e.Count -eq 1) 'Evidence not persisted'
 foreach($table in @('status.md','details.md')) {
  $tableText=Read-VText (Join-Path $fixture ('docs/verification/'+$table))
  Assert (-not $tableText.Contains('CF_fixture.cpp')) 'Exempt solution leaked into template table'
 }
 $saved=(Read-VText $e[0].FullName) | ConvertFrom-Json
 Assert ($saved.results[0].files.path -contains 'algorithms/dep.h') 'Missing transitive dependency in evidence'
 Put 'algorithms/one_check.cpp' "#include `"engine.cpp`"`nint main(){return value();}`n"
 Run 'run_checks.ps1' @('-Compiler',$Compiler) 1
 $text=Read-VText (Join-Path $fixture 'docs/verification/details.md')
 Assert ($text.Contains((U '5f53 524d 6e90 7801 5931 8d25'))) 'Latest fail hidden by old pass'
 Run 'run_checks.ps1' @('-Compiler',$Compiler,'-Mode','Syntax') 0
 $text=Read-VText (Join-Path $fixture 'docs/verification/details.md')
 Assert ($text.Contains((U '5f53 524d 6e90 7801 5931 8d25'))) 'Syntax pass replaced regression failure'
 Put 'algorithms/dep.h' "#pragma once`ninline int value() { return 3; }`n"
 Run 'make_verification.ps1' @() 0
 $text=Read-VText (Join-Path $fixture 'docs/verification/details.md')
 Assert ($text.Contains((U '5f85 91cd 9a8c'))) 'Stale evidence not shown'
 # CI mode must only write evidence and tables below its report directory.
 $count=@(Get-ChildItem (Join-Path $fixture 'records/verification/runs') -Filter '*.json').Count
 $env:GITHUB_ACTIONS='true'
 Run 'run_checks.ps1' @('-Compiler',$Compiler,'-Mode','Syntax','-ReportDir',(Join-Path $fixture 'ci-report')) 0
 Assert ((Test-Path (Join-Path $fixture 'ci-report/verification/status.md'))) 'CI report missing'
 Assert (@(Get-ChildItem (Join-Path $fixture 'records/verification/runs') -Filter '*.json').Count -eq $count) 'CI wrote local evidence store'
} finally { $env:GITHUB_ACTIONS=$oldCI }
Write-Host ('[PASS] verification fingerprints, failure precedence, stale state, syntax isolation, CI output; logs: '+$fixture)
exit 0
