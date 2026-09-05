param([string]$BuildRoot='')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (-not $BuildRoot) { $BuildRoot=Join-Path $root '.zoi-checks' }
$fixture=Join-Path ([IO.Path]::GetFullPath($BuildRoot)) ('inventory-test-'+[Guid]::NewGuid().ToString('N'))
foreach ($d in @('scripts','rules','zoi','algorithms/other','algorithms/tests')) { [void][IO.Directory]::CreateDirectory((Join-Path $fixture $d)) }
foreach ($f in @('check_inventory.ps1','make_reliability.ps1')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $f) -Destination (Join-Path $fixture 'scripts') }
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
. (Join-Path $PSScriptRoot 'check_process.ps1')
$enc=New-Object Text.UTF8Encoding($false)
function Put([string]$p,[string]$s) { [IO.File]::WriteAllText((Join-Path $fixture $p),$s,$enc) }
function Assert([bool]$ok,[string]$message) { if (-not $ok) { throw $message } }
function Inventory { return Get-CheckInventory $fixture }
$script:n=0
function Generate([int]$expected=0,[switch]$Check) {
    $script:n++; $argsForRun=@('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $fixture 'scripts/make_reliability.ps1'))
    if ($Check) { $argsForRun+='-Check' }
    $r=Invoke-CheckProcess (Get-Process -Id $PID).Path $argsForRun $fixture 30 (Join-Path $fixture ('generate-'+$script:n))
    Assert (-not $r.TimedOut -and $r.ExitCode -eq $expected) 'Generator returned unexpected status'
}
Put 'algorithms/one.cpp' "// zoi: one`nint one;`n"
Put 'algorithms/other/one.cpp' "// zoi: two`nint two;`n"
Put 'algorithms/note.txt' 'not compiled'
Put 'algorithms/example.cpp' 'int main() {}'
$catalog="one`talgorithms/one.cpp`ntwo`talgorithms/other/one.cpp`n^note`talgorithms/note.txt`n!algorithms/example.cpp`n"
Put 'zoi/_catalog.txt' $catalog
Put 'zoi/one.h' '#include "../algorithms/one.cpp"'
Put 'zoi/two.h' '#include "../algorithms/other/one.cpp"'
Put 'algorithms/tests/a_check.cpp' "#include `"../one.cpp`"`n#include `".././one.cpp`"`n"
$i=Inventory
Assert ($i.entries.Count -eq 4 -and $i.checks.Count -eq 1 -and $i.engines.Count -eq 3) 'Inventory lost entries'
Assert ($i.entries[0].suites.Count -eq 1 -and $i.entries[1].suites.Count -eq 0) 'Canonical identity or duplicate reference broken'
Generate
$p=Join-Path $fixture 'rules/reliability.md'; $snapshot=[IO.File]::ReadAllText($p); $stamp=(Get-Item -LiteralPath $p).LastWriteTimeUtc
Generate; Generate -Check
Assert ((Get-Item -LiteralPath $p).LastWriteTimeUtc -eq $stamp) 'No-op generation rewrote file'
Assert ($snapshot.Contains((-join @([char]0x43,[char]0x20,[char]0x7c7b,[char]0xff1a,[char]0x7b14,[char]0x8bb0,[char]0x6761,[char]0x76ee))) -and $snapshot.Contains((-join @([char]0x8c41,[char]0x514d,[char]0x4ee3,[char]0x7801))) -and $snapshot.Contains('example.cpp')) 'Prose/exempt source missing'
Write-Host '[PASS] complete inventory / distinct same basename / duplicate reference / stable generation'

Put 'algorithms/tests/a_check.cpp' "/*`n#include `"../one.cpp`"`n*/`nconst char* text=R`"(`n#include `"../one.cpp`"`n)`";`n// continued \`n#include `"../one.cpp`"`n#if 0`n#include `"../../zoi/two.h`"`n#endif`n"
$i=Inventory
Assert ($i.entries[0].suites.Count -eq 0 -and $i.entries[1].suites.Count -eq 1) 'Comments, raw strings or stub resolution broken'
Generate 1 -Check
Assert ([IO.File]::ReadAllText($p) -ceq $snapshot) '-Check modified stale file'
Generate; Generate -Check
Write-Host '[PASS] moved references / comment and raw-string masking / stub alias / stale Check is read-only'

Put 'algorithms/tests/a_check.cpp' "#include `"../example.cpp`"`n"
$i=Inventory; Assert ($i.entries[3].suites.Count -eq 1) 'Exempt source reference lost'
Put 'algorithms/tests/b_check.cpp' "#include `"../one.cpp`"`n"
Generate; $bytes=[IO.File]::ReadAllText($p)
Put 'rules/reliability.md' $bytes.Replace("`n","`r`n")
Generate -Check
Assert ([IO.File]::ReadAllText($p).Contains("`r`n")) '-Check rewrote CRLF'
Write-Host '[PASS] exempt references / new suite / cross-platform line endings'

Put 'zoi/two.h' '#include "../algorithms/one.cpp"'
$before=[IO.File]::ReadAllText($p)
Generate 1
Assert ([IO.File]::ReadAllText($p) -ceq $before) 'Invalid scaffold overwrote report'
Put 'zoi/two.h' '#include "../algorithms/other/one.cpp"'
Put 'zoi/_catalog.txt' ($catalog+"^lost`tmissing.txt`n")
Generate 1 -Check
Put 'zoi/_catalog.txt' $catalog
[IO.File]::Delete((Join-Path $fixture 'algorithms/tests/a_check.cpp'))
[IO.File]::Delete((Join-Path $fixture 'algorithms/tests/b_check.cpp'))
$i=Inventory; Assert ($i.checks.Count -eq 0 -and $i.entries[0].suites.Count -eq 0) 'Deleted suites remained in inventory'
Generate
Write-Host '[PASS] invalid scaffold preserves report / missing prose rejects / removed suites disappear'
Write-Host "Inventory self-test: 4 groups passed ($script:n generator commands); logs: $fixture"
