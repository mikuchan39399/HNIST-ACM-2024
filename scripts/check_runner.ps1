param([string]$Compiler = 'g++', [string]$BuildRoot = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if (-not $BuildRoot) { $BuildRoot = Join-Path $root '.zoi-checks' }
$fixture = Join-Path ([IO.Path]::GetFullPath($BuildRoot)) ('runner-test-' + [Guid]::NewGuid().ToString('N'))
foreach ($dir in @('scripts', 'zoi', 'algorithms')) { New-Item -ItemType Directory -Path (Join-Path $fixture $dir) -Force | Out-Null }
foreach ($file in @('run_checks.ps1', 'check_process.ps1', 'check_inventory.ps1')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $file) -Destination (Join-Path $fixture 'scripts') }
$utf8 = New-Object System.Text.UTF8Encoding($false)
function Put([string]$Name, [string]$Body) { [IO.File]::WriteAllText((Join-Path $fixture $Name), $Body, $utf8) }
Put 'algorithms/engine.cpp' "// zoi: tiny`n"
$catalog = "tiny`talgorithms/engine.cpp`n"
$stub = '#include "../algorithms/engine.cpp"'
Put 'zoi/_catalog.txt' $catalog
Put 'zoi/tiny.h' $stub
. (Join-Path $PSScriptRoot 'check_process.ps1')
$hostExe = (Get-Process -Id $PID).Path
$cases = @(
    @{name='pass'; body='int main() { assert(true); }'; expected=0},
    @{name='warning'; body='int main() { int unused; }'; expected=1; status='COMPILE FAIL'},
    @{name='compile'; body='int main() { broken syntax }'; expected=1; status='COMPILE FAIL'},
    @{name='assert'; body='int main() { std::signal(SIGABRT, [](int) { std::_Exit(134); }); assert(false); }'; expected=1; status='RUN FAIL'},
    @{name='timeout'; body='int main() { std::this_thread::sleep_for(std::chrono::seconds(10)); }'; expected=1; status='RUN TIMEOUT'},
    @{name='no-match'; body='int main() {}'; expected=1},
    @{name='missing-catalog'; body='int main() {}'; expected=1},
    @{name='stale-stub'; body='int main() {}'; expected=1},
    @{name='duplicate'; body='int main() {}'; expected=1}
)
foreach ($case in $cases) {
    Put 'zoi/_catalog.txt' $catalog
    Put 'zoi/tiny.h' $stub
    Put 'algorithms/case_check.cpp' ("#include <cassert>`n#include <csignal>`n#include <cstdlib>`n#include <thread>`n#include <chrono>`n#include `"engine.cpp`"`n" + $case.body)
    $report = Join-Path $fixture $case.name
    $argsForRun = @('-NoProfile', '-File', (Join-Path $fixture 'scripts/run_checks.ps1'), '-Compiler', $Compiler, '-TimeoutSec', '1', '-ReportDir', $report)
    if ($case.name -eq 'no-match') { $argsForRun += @('-Filter', 'does-not-exist') }
    if ($case.name -eq 'missing-catalog') { Remove-Item -LiteralPath (Join-Path $fixture 'zoi/_catalog.txt') }
    if ($case.name -eq 'stale-stub') { Put 'zoi/tiny.h' '#include "wrong.cpp"' }
    if ($case.name -eq 'duplicate') { Put 'zoi/_catalog.txt' ($catalog + $catalog) }
    $result = Invoke-CheckProcess $hostExe $argsForRun $fixture 60 (Join-Path $fixture $case.name)
    if ($result.TimedOut -or (($result.ExitCode -eq 0) -ne ($case.expected -eq 0))) { throw ('Runner self-test failed: ' + $case.name) }
    if ($case.status) {
        $summary = Get-Content -LiteralPath (Join-Path $report 'summary.json') -Raw | ConvertFrom-Json
        if ($summary.results[0].status -ne $case.status) { throw ('Wrong failure phase: ' + $case.name) }
    }
    Write-Host ('[SELFTEST PASS] ' + $case.name)
}
Write-Host ('Runner self-test: ' + $cases.Count + ' passed; logs: ' + $fixture)
