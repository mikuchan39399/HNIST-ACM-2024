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
$r=[pscustomobject]@{fingerprintVersion=2;path='algorithms/one_check.cpp';stable=$true;fingerprint=$a.hash;files=$a.files;scopes=@(Get-VScopeEvidence $a $a);status='PASS'}
Assert ((Get-VState $null $a) -eq 'none') 'No evidence was not unknown'
Assert ((Get-VState $r $a) -eq 'pass') 'Pass classification'
$r.stable=$false
Assert ((Get-VState $r $a) -eq 'stale') 'Mid-run mutation must be stale'
$r.stable=$true; $r.status='RUN FAIL'
Assert ((Get-VState $r $a) -eq 'fail') 'Failure classification'
# Independent scopes must not contaminate code or sibling-template evidence.
$baseSpec=Read-VText (Join-Path $fixture 'rules/verification.json')
function Spec($obj) { Put 'rules/verification.json' ($obj | ConvertTo-Json -Depth 10) }
function Record($snapshot) {
 [pscustomobject]@{fingerprintVersion=2;path='algorithms/one_check.cpp';stable=$true;fingerprint=$snapshot.hash;files=$snapshot.files;scopes=@(Get-VScopeEvidence $snapshot $snapshot);status='PASS'}
}
$definition=$baseSpec | ConvertFrom-Json
$peer=$baseSpec | ConvertFrom-Json
$peer.coverage[0].template='algorithms/peer.cpp'
$definition.coverage=@($definition.coverage)+@($peer.coverage)
Spec $definition
$both=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
$record=Record $both
Assert ($both.hash -eq $a.hash) 'Adding coverage changed executable fingerprint'
Assert ((Get-VState (Record $a) $both 'algorithms/peer.cpp') -eq 'stale') 'New coverage inherited an old pass'
$definition.coverage[0].api=@('value','new API')
Spec $definition
$changed=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
Assert ($changed.hash -eq $both.hash) 'Scope edit changed code fingerprint'
Assert ((Get-VState $record $changed 'algorithms/engine.cpp') -eq 'stale') 'Related API edit did not invalidate scope'
Assert ((Get-VState $record $changed 'algorithms/peer.cpp') -eq 'pass') 'Sibling scope edit invalidated peer'
$inFlight=Record $both
$inFlight.scopes=@(Get-VScopeEvidence $both $changed)
Assert ((Get-VState $inFlight $changed 'algorithms/engine.cpp') -eq 'stale') 'Mid-run scope mutation was accepted'
Assert ((Get-VState $inFlight $changed 'algorithms/peer.cpp') -eq 'pass') 'Mid-run sibling change leaked'
$definition.coverage[0].api=@('value')
$definition.coverage[0].summary='New display wording'
Spec $definition
$cosmetic=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
Assert ((Get-VState $record $cosmetic 'algorithms/engine.cpp') -eq 'pass') 'Summary wording invalidated evidence'
# Reordered objects/properties and list entries must have the same scope hash.
$definition.coverage[0].cases=@('single','another')
Spec $definition
$order=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
$definition.coverage[0].cases=@('another','single')
$definition.coverage=@($definition.coverage[1],$definition.coverage[0])
Put 'rules/verification.json' ($definition | ConvertTo-Json -Depth 10 -Compress)
Assert ((Get-VState (Record $order) (Get-VSnapshot $fixture 'algorithms/one_check.cpp') 'algorithms/engine.cpp') -eq 'pass') 'JSON/list order or formatting affected fingerprint'
foreach($field in @('oracle','limitations','cases')) {
 $definition=$baseSpec | ConvertFrom-Json
 $definition.coverage[0].$field=if($field -eq 'cases'){@('changed')}else{'changed'}
 Spec $definition
 Assert ((Get-VState (Record $a) (Get-VSnapshot $fixture 'algorithms/one_check.cpp') 'algorithms/engine.cpp') -eq 'stale') ('Behavior field ignored: '+$field)
}
$definition=$baseSpec | ConvertFrom-Json
$foreign=$baseSpec | ConvertFrom-Json
$foreign.coverage[0].suite='algorithms/unrelated_check.cpp'
$definition.coverage=@($definition.coverage)+@($foreign.coverage)
Spec $definition
Assert ((Get-VState (Record $a) (Get-VSnapshot $fixture 'algorithms/one_check.cpp') 'algorithms/engine.cpp') -eq 'pass') 'Unrelated suite registration leaked'
Put 'rules/verification.json' $baseSpec
Put 'algorithms/unrelated.h' 'int unrelated;'
Assert ((Get-VSnapshot $fixture 'algorithms/one_check.cpp').hash -eq $a.hash) 'Unrelated source file invalidated evidence'
$legacy=[pscustomobject]@{fingerprint=$a.hash;files=$a.files;stable=$true;status='PASS'}
Assert ((Get-VState $legacy $a 'algorithms/engine.cpp') -eq 'legacy') 'Legacy evidence promoted without per-scope data'
# Runner edits, test edits, missing dependencies and macro/conditional includes stay conservative.
$runner=Read-VText (Join-Path $fixture 'scripts/run_checks.ps1')
Put 'scripts/run_checks.ps1' ($runner+"`n# edit")
$assessment=Get-VAssessment (Record $a) (Get-VSnapshot $fixture 'algorithms/one_check.cpp')
Assert ($assessment.state -eq 'stale' -and $assessment.reasons -contains 'tooling') 'Runner change not explained'
Put 'scripts/run_checks.ps1' $runner
$source=Read-VText (Join-Path $fixture 'algorithms/one_check.cpp')
Put 'algorithms/one_check.cpp' ($source+"`n// edit")
Assert ((Get-VAssessment (Record $a) (Get-VSnapshot $fixture 'algorithms/one_check.cpp')).reasons -contains 'test') 'Test change not explained'
Put 'algorithms/one_check.cpp' $source
$dependency=Read-VText (Join-Path $fixture 'algorithms/dep.h')
[IO.File]::Delete((Join-Path $fixture 'algorithms/dep.h'))
Assert ((Get-VAssessment (Record $a) (Get-VSnapshot $fixture 'algorithms/one_check.cpp')).reasons -contains 'source') 'Deleted dependency not explained'
Put 'algorithms/dep.h' $dependency
Put 'algorithms/one_check.cpp' ($source+"`n#if 0`n#include `"conditional.h`"`n#endif`n")
$conditional=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
Put 'algorithms/conditional.h' 'int conditional;'
Assert ((Get-VSnapshot $fixture 'algorithms/one_check.cpp').hash -ne $conditional.hash) 'Conditional missing dependency creation ignored'
Put 'algorithms/one_check.cpp' ($source+"`n#include HEADER_MACRO`n")
$macro=Get-VSnapshot $fixture 'algorithms/one_check.cpp'
Put 'algorithms/unrelated.h' 'int changed;'
Assert ((Get-VSnapshot $fixture 'algorithms/one_check.cpp').hash -ne $macro.hash) 'Macro include must conservatively cover all sources'
Put 'algorithms/one_check.cpp' $source
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
  $tableLines=@($tableText.Split("`n") | Where-Object { $_.StartsWith('|') })
  $columns=0
  foreach($line in $tableLines) {
   $count=[regex]::Matches($line,'(?<!\\)\|').Count
   if($line -match '^\|---') { Assert ($columns -eq $count) 'Table heading and separator widths differ' }
   $columns=$count
  }
 }
 $saved=(Read-VText $e[0].FullName) | ConvertFrom-Json
 Assert ($saved.results[0].files.path -contains 'algorithms/dep.h') 'Missing transitive dependency in evidence'
 Assert ($saved.results[0].fingerprintVersion -eq 2) 'Runner did not write versioned fingerprints'
 Assert ($saved.results[0].scopes.Count -eq 1 -and $saved.results[0].scopes[0].stable) 'Runner did not persist stable per-template scope'
 $definition=$baseSpec | ConvertFrom-Json
 $definition.coverage[0].summary='Edited user wording'
 Spec $definition
 Run 'make_verification.ps1' @() 0
 $user=Read-VText (Join-Path $fixture 'docs/verification/status.md')
 Assert ($user.Contains('Edited user wording') -and $user.Contains((U '767b 8bb0 7684 6d4b 8bd5 90fd 5df2 901a 8fc7'))) 'Summary edit changed pass or was not displayed'
 $definition.coverage[0].cases=@('new case')
 Spec $definition
 Run 'make_verification.ps1' @() 0
 $user=Read-VText (Join-Path $fixture 'docs/verification/status.md')
 Assert ($user.Contains((U '8be5 9879 9a8c 8bc1 8303 56f4 53d8 5316'))) 'Related scope change not explained in user table'
 Put 'rules/verification.json' $baseSpec
 Put 'algorithms/peer.cpp' "// zoi: peer`n"
 $catalog=Read-VText (Join-Path $fixture 'zoi/_catalog.txt')
 Put 'zoi/_catalog.txt' ($catalog+"peer`talgorithms/peer.cpp`n")
 Put 'zoi/peer.h' '#include "../algorithms/peer.cpp"'
 $definition=$baseSpec | ConvertFrom-Json
 $peer=$baseSpec | ConvertFrom-Json
 $peer.coverage[0].template='algorithms/peer.cpp'
 $definition.coverage=@($definition.coverage)+@($peer.coverage)
 Spec $definition
 Run 'run_checks.ps1' @('-Compiler',$Compiler) 0
 $definition.coverage[0].cases=@('new case')
 Spec $definition
 Run 'make_verification.ps1' @() 0
 $user=Read-VText (Join-Path $fixture 'docs/verification/status.md')
 $peerLine=@($user.Split("`n") | Where-Object { $_.Contains('[peer]') })
 $tinyLine=@($user.Split("`n") | Where-Object { $_.Contains('[tiny]') })
 Assert ($peerLine.Count -eq 1 -and $peerLine[0].Contains((U '767b 8bb0 7684 6d4b 8bd5 90fd 5df2 901a 8fc7'))) 'Same-suite peer lost pass after another scope changed'
 Assert ($tinyLine.Count -eq 1 -and $tinyLine[0].Contains((U '8be5 9879 9a8c 8bc1 8303 56f4 53d8 5316'))) 'Changed template was not stale in generated table'
 Put 'rules/verification.json' $baseSpec
 Put 'algorithms/one_check.cpp'  "#include `"engine.cpp`"`nint main(){return value();}`n"
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
Write-Host ('[PASS] per-template scopes, cosmetic isolation, legacy handling, change reasons, verification fingerprints, failure precedence, stale state, syntax isolation, CI output; logs: '+$fixture)
exit 0
