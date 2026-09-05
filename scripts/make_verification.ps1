param([string]$RunDir='', [string]$OutputDir='')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
. (Join-Path $PSScriptRoot 'check_verification.ps1')
$inventory=Get-CheckInventory $root
$spec=Read-VSpec $root $inventory
if (-not $RunDir) { $RunDir=Join-Path $root 'records/verification/runs' }
if (-not $OutputDir) { $OutputDir=Join-Path $root 'docs/verification' }
$OutputDir=[IO.Path]::GetFullPath($OutputDir)
[void][IO.Directory]::CreateDirectory($OutputDir)
$rows=@()
if (Test-Path -LiteralPath $RunDir) {
 foreach ($f in Get-ChildItem -LiteralPath $RunDir -Filter '*.json' -File) {
  $run=(Read-VText $f.FullName) | ConvertFrom-Json
  if ($run.schema -ne 1 -or -not $run.id -or -not $run.finishedUtc) { throw ('Invalid evidence: '+$f.Name) }
  foreach ($r in $run.results) {
   if ($r.phase -ne 'regression') { continue }
   if (-not $r.fingerprint -or -not $r.files) { throw ('Missing fingerprint: '+$f.Name) }
   $rows += [pscustomobject]@{suite=$r.path; profile=($run.os+' / '+$(if($run.sanitize){'sanitizer'}else{'normal'})); date=([DateTime]$run.finishedUtc).ToUniversalTime().ToString('o'); run=$run; result=$r}
  }
 }
}
$latest=@{}
foreach ($r in ($rows | Sort-Object date)) { $latest[$r.suite+'|'+$r.profile]=$r }
function U([string]$hex) { -join @($hex.Split(' ') | ForEach-Object { [char][Convert]::ToInt32($_,16) }) }
function Profile([string]$s) { $s.Replace(' / normal',(U '20 666e 901a 56de 5f52')).Replace(' / sanitizer',(U '20 5185 5b58 4e0e 672a 5b9a 4e49 884c 4e3a 68c0 67e5')) }
function ResultLabel([string]$s) { switch($s) { 'PASS' {(U '901a 8fc7')} 'COMPILE FAIL' {(U '7f16 8bd1 5931 8d25')} 'COMPILE TIMEOUT' {(U '7f16 8bd1 8d85 65f6')} 'RUN FAIL' {(U '8fd0 884c 5931 8d25')} 'RUN TIMEOUT' {(U '8fd0 884c 8d85 65f6')} default { $s } } }
function Cell($v) { ([string]$v).Replace('|','\|').Replace("`r",'').Replace("`n",'<br>') }
function Link([string]$path,[string]$label='') {
 if (-not $label) { $label=$path }
 $target=[Uri]::new((Join-Path $root $path)); $from=[Uri]::new($OutputDir+[IO.Path]::DirectorySeparatorChar)
 '['+(Cell $label)+'](<'+$from.MakeRelativeUri($target).ToString()+'>)'
}
$user=New-Object 'Collections.Generic.List[string]'
$detail=New-Object 'Collections.Generic.List[string]'
$user.Add((U '23 20 6a21 677f 73b0 5728 6d4b 5f97 600e 4e48 6837')); $user.Add('')
$user.Add((U '81ea 52a8 751f 6210 2c 20 4e0d 624b 6539 2e 20 4ec5 603b 7ed3 5df2 767b 8bb0 8303 56f4 5185 7684 8bc1 636e 2e 20 4ee3 7801 53d8 5316 540e 8bf7 91cd 65b0 751f 6210 2c 20 9759 6001 6587 4ef6 4e0d 4f1a 81ea 884c 5237 65b0 2e')); $user.Add('')
$user.Add((U '5b 41 49 20 9a8c 8bc1 660e 7ec6 5d 28 64 65 74 61 69 6c 73 2e 6d 64 29')); $user.Add('')
$user.Add((U '7c 20 6a21 677f 20 7c 20 73b0 5728 600e 4e48 6837 20 7c 20 5df2 6d4b 5185 5bb9 20 7c 20 8fd8 8981 6ce8 610f 4ec0 4e48 20 7c')); $user.Add('|---|---|---|---|')
$detail.Add((U '23 20 41 49 20 9a8c 8bc1 660e 7ec6')); $detail.Add('')
$detail.Add((U '81ea 52a8 751f 6210 2e 20 69 6e 63 6c 75 64 65 20 53ea 8868 793a 9759 6001 5173 8054 2c 20 884c 4e3a 8303 56f4 7531 20 76 65 72 69 66 69 63 61 74 69 6f 6e 2e 6a 73 6f 6e 20 767b 8bb0 2c 20 8fd0 884c 8bc1 636e 7531 20 72 75 6e 6e 65 72 20 8bb0 5f55 2e')); $detail.Add('')
$detail.Add((U '7c 20 6a21 677f 20 7c 20 5957 4ef6 20 7c 20 41 50 49 20 7c 20 72ec 7acb 53c2 7167 20 7c 20 6570 636e 4e0e 8fb9 754c 20 7c 20 9650 5236 20 7c')); $detail.Add('|---|---|---|---|---|---|')
$snapshots=@{}
foreach ($e in $inventory.entries) {
 if ($e.kind -ne 'engine') { continue }
 $scopes=@($spec.coverage | Where-Object { $_.template -ceq $e.path })
 $states=@(); $profiles=@()
 foreach ($suite in @($scopes | ForEach-Object {$_.suite} | Sort-Object -Unique)) {
  if (-not $snapshots.ContainsKey($suite)) { $snapshots[$suite]=Get-VSnapshot $root $suite }
  $found=@($latest.Values | Where-Object {$_.suite -ceq $suite})
  if (-not $found.Count) { $states+='none' }
  foreach ($r in $found) { $states+=Get-VState $r.result $snapshots[$suite].hash; $profiles+=(Profile $r.profile) }
 }
 $state=(U '8fd8 6ca1 6709 767b 8bb0 5177 4f53 6d4b 4e86 54ea 4e9b 884c 4e3a')
 if ($scopes.Count) {
  $state=(U '767b 8bb0 7684 6d4b 8bd5 90fd 5df2 901a 8fc7')
  if ($states -contains 'fail') { $state=(U '6709 6d4b 8bd5 6ca1 901a 8fc7 2c 20 5148 67e5 770b 660e 7ec6') }
  elseif ($states -contains 'stale') { $state=(U '4ee3 7801 6216 9a8c 8bc1 6761 4ef6 53d8 4e86 2c 20 9700 8981 91cd 8dd1') }
  elseif ($states -contains 'none') { $state=(U '8303 56f4 5df2 767b 8bb0 2c 20 8fd8 6709 6d4b 8bd5 6ca1 6709 8fd0 884c 8bb0 5f55') }
  if ($profiles.Count) { $state+=' ('+(($profiles | Sort-Object -Unique) -join ', ')+')' }
 }
 $summary=($scopes | ForEach-Object {$_.summary} | Select-Object -Unique) -join ' / '
 $limits=($scopes | ForEach-Object {$_.limitations} | Select-Object -Unique) -join ' / '
 if (-not $scopes.Count) {
  $summary=if($e.suites.Count){(U '6709 76f8 5173 5957 4ef6 2c 20 5c1a 672a 6838 5bf9 5177 4f53 8986 76d6')}else{(U '5c1a 672a 53d1 73b0 76f4 63a5 5173 8054 7684 5957 4ef6')}
  $limits=(U '4e0d 80fd 4ec5 51ed 20 69 6e 63 6c 75 64 65 20 6216 5176 4ed6 6a21 677f 901a 8fc7 5c31 5224 65ad 5b83 5df2 6d4b 597d')
 }
 $user.Add('| '+(Link $e.path $(if($e.name){$e.name}else{[IO.Path]::GetFileNameWithoutExtension($e.path)}))+' | '+(Cell $state)+' | '+(Cell $summary)+' | '+(Cell $limits)+' |')
 if (-not $scopes.Count) { $detail.Add('| '+(Link $e.path)+' | '+(($e.suites | ForEach-Object {Link $_}) -join '<br>')+' | '+(U '672a 767b 8bb0')+' | '+(U '672a 6838 5b9e')+' | '+(U '672a 6838 5b9e')+' | '+(U '4e0d 80fd 4ece 9759 6001 5173 8054 63a8 65ad 884c 4e3a 8986 76d6')+' |') }
 foreach ($s in $scopes) {
  $detail.Add('| '+(Link $e.path)+' | '+(Link $s.suite)+' | '+(Cell ($s.api -join ', '))+' | '+(Cell $s.oracle)+' | '+(Cell ($s.cases -join '; '))+' | '+(Cell $s.limitations)+' |')
 }
}
$detail.Add(''); $detail.Add((U '23 23 20 5404 73af 5883 6700 65b0 8fd0 884c')); $detail.Add('')
$detail.Add((U '7c 20 5957 4ef6 20 7c 20 73af 5883 20 7c 20 5f53 524d 5224 65ad 20 7c 20 5b9e 9645 7ed3 679c 20 7c 20 55 54 43 20 65f6 95f4 20 7c 20 7f16 8bd1 5668 4e0e 53c2 6570 20 7c 20 8bc1 636e 7f16 53f7 4e0e 6307 7eb9 20 7c 20 65e5 5fd7 4f4d 7f6e 20 7c'))
$detail.Add('|---|---|---|---|---|---|---|---|')
foreach ($r in ($latest.Values | Sort-Object suite,profile)) {
 if (-not $snapshots.ContainsKey($r.suite)) { $snapshots[$r.suite]=Get-VSnapshot $root $r.suite }
 $state=Get-VState $r.result $snapshots[$r.suite].hash
 $label=switch($state){pass {(U '5f53 524d 6e90 7801 901a 8fc7')} stale {(U '5f85 91cd 9a8c')} fail {(U '5f53 524d 6e90 7801 5931 8d25')} default {(U '65e0 8bb0 5f55')}}
 $detail.Add('| '+(Link $r.suite)+' | '+(Cell (Profile $r.profile))+' | '+$label+' | '+(Cell (ResultLabel $r.result.status))+' | '+$r.date+' | '+(Cell ($r.run.compilerVersion+' / '+($r.run.flags -join ' ')))+' | '+$r.run.id+' / '+$r.result.fingerprint+' | '+(Cell $r.run.logs)+' |')
}
$detail.Add(''); $detail.Add((U '4a 53 4f 4e 20 8bc1 636e 4fdd 7559 5b8c 6574 4f9d 8d56 6307 7eb9 2e 20 540c 73af 5883 6700 65b0 5931 8d25 4e0d 4f1a 88ab 65e7 901a 8fc7 8986 76d6 2e 20 8bed 6cd5 68c0 67e5 4e0d 7b97 5bf9 62cd 901a 8fc7 2e 20 8fd0 884c 671f 95f4 6e90 7801 53d8 5316 5219 5f85 91cd 9a8c 2e'))
$utf8=New-Object Text.UTF8Encoding($false)
[IO.File]::WriteAllText((Join-Path $OutputDir 'status.md'),(($user -join "`n")+"`n"),$utf8)
[IO.File]::WriteAllText((Join-Path $OutputDir 'details.md'),(($detail -join "`n")+"`n"),$utf8)
Write-Host ('[OK] verification tables -> '+$OutputDir)
