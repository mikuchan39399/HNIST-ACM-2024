param([string]$BuildRoot='')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (-not $BuildRoot) { $BuildRoot=Join-Path $root '.zoi-checks' }
$fixture=Join-Path ([IO.Path]::GetFullPath($BuildRoot)) ('deploy-test-'+[Guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($fixture)
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
. (Join-Path $PSScriptRoot 'check_process.ps1')
$hostExe=(Get-Process -Id $PID).Path; $script:calls=0
function Assert([bool]$ok,[string]$msg) { if (-not $ok) { throw $msg } }
function Put([string]$p,[string]$text) { [void][IO.Directory]::CreateDirectory((Split-Path -Parent $p)); Setup-Write $p $text }
function Run([string]$tool,[string[]]$Arguments,[int]$Expected=0) {
    $script:calls++
    $r=Invoke-CheckProcess $hostExe (@('-NoProfile','-ExecutionPolicy','Bypass','-File',$tool)+$Arguments) $fixture 120 (Join-Path $fixture ('call-'+$script:calls))
    Assert (-not $r.TimedOut -and $r.ExitCode -eq $Expected) ('Command failed: '+$tool+' call '+$script:calls)
}
$settings=Join-Path $fixture 'profile/settings.json'; $tasks=Join-Path $fixture 'profile/tasks.json'; $keys=Join-Path $fixture 'profile/keybindings.json'
$initial='{"C_Cpp.autocomplete":"disabled","editor.fontSize":19}'
$bindings='[{"key":"ctrl+alt+t","command":"keep.user.command"}]'
Put $settings $initial; Put $keys $bindings
Run (Join-Path $PSScriptRoot 'install-zoi.ps1') @('-SettingsFile',$settings)
$s=JC-Value (Setup-Read $settings); $t=JC-Value (Setup-Read $tasks); $k=JC-Value (Setup-Read $keys)
Assert ($s.'C_Cpp.autocomplete' -eq 'default' -and $s.'C_Cpp.errorSquiggles' -eq 'enabled') 'IntelliSense not enabled'
Assert ($t.tasks.Count -eq 9 -and @($k).Count -eq 2 -and $k[0].command -eq 'keep.user.command') 'Tasks/shortcut collision handling failed'
$packTask=@($t.tasks | Where-Object label -eq 'zoi-package')[0]
Assert ($packTask.args[-1] -match '/scripts/make_team_package.ps1$') 'Package task depends on an active source file'
# A missing owned task is repaired without losing the original pre-install snapshot.
$raw=Setup-Read $tasks; $arr=JC-Get $raw 'tasks'; $node=JC-Parse $arr
$arr=JC-Cut $arr $node 8; Put $tasks (JC-Set $raw 'tasks' $arr)
Run (Join-Path $PSScriptRoot 'install-zoi.ps1') @('-SettingsFile',$settings)
Assert ((JC-Value (Setup-Read $tasks)).tasks.Count -eq 9) 'Missing owned task not repaired'
# Emulate the supported v2 schema (two documents), then upgrade in place.
$st=Setup-Read ($settings+'.zoi-state') | ConvertFrom-Json
$st.version=2; $st.docs=@($st.docs[0],$st.docs[1]); $st.labels=@($st.labels | Select-Object -First 5)
$st.docs[1].after.text=JC-Set $st.docs[1].after.text 'tasks' (Setup-Json @((JC-Value (Setup-Read $tasks)).tasks | Select-Object -First 5))
Put $tasks $st.docs[1].after.text
Put $keys $bindings
Put ($settings+'.zoi-state') (Setup-Json $st)
Run (Join-Path $PSScriptRoot 'install-zoi.ps1') @('-SettingsFile',$settings)
Assert ((Setup-Read ($settings+'.zoi-state') | ConvertFrom-Json).version -eq 3) 'v2 state not upgraded'
Run (Join-Path $PSScriptRoot 'uninstall-zoi.ps1') @('-SettingsFile',$settings)
Assert ((Setup-Read $settings) -ceq $initial) 'Upgrade lost original settings snapshot'
Assert ((Setup-Read $keys) -ceq $bindings) 'Upgrade lost existing shortcut'
Write-Host '[PASS] defaults / shortcut collision / task repair / v2 upgrade / original restoration'

$manual=Setup-Json @{version='2.0.0';tasks=@(@{label='zoi-expand';type='shell';command='powershell';args=@('-File',(Join-Path $PSScriptRoot 'zoi.ps1'),'expand','${file}')})}
Put $tasks $manual
Run (Join-Path $PSScriptRoot 'install-zoi.ps1') @('-SettingsFile',$settings,'-AdoptExistingTasks')
$edited=Setup-Read $tasks
Put $tasks (JC-Set $edited 'tasks' (JC-Append (JC-Get $edited 'tasks') '{"label":"later-user-task","type":"shell","command":"echo keep"}'))
Run (Join-Path $PSScriptRoot 'uninstall-zoi.ps1') @('-SettingsFile',$settings)
$after=JC-Value (Setup-Read $tasks)
Assert ($after.tasks.Count -eq 2 -and @($after.tasks | Where-Object { $_.label -eq 'zoi-expand' -and $_.type -eq 'shell' }).Count -eq 1) 'Adopted manual task or later user task lost'
Put $tasks '{"tasks":[{"label":"zoi-expand","command":"other-tool","args":[]}]}'
Run (Join-Path $PSScriptRoot 'install-zoi.ps1') @('-SettingsFile',$settings,'-AdoptExistingTasks') 1
Write-Host '[PASS] explicit local-task adoption / later unrelated edits / foreign command refused'

$work=Join-Path $fixture ('problem-'+[char]0x8349+' # space')
$cpp=Join-Path $work '.vscode/c_cpp_properties.json'; $ws=Join-Path $work '.vscode/settings.json'
$cppBefore='// keep comment
{"configurations":[{"name":"A","includePath":["C:/keep"],"defines":["KEEP=1"]},{"name":"B","includePath":["${default}"]}],"version":4}'
Put $cpp $cppBefore; Put $ws '{"C_Cpp.autocomplete":"disabled","editor.tabSize":3}'
Run (Join-Path $PSScriptRoot 'configure-zoi.ps1') @('-Workspace',$work)
$config=JC-Value (Setup-Read $cpp)
foreach ($c in $config.configurations) { Assert ($c.includePath -contains ($root.Replace('\','/')+'/zoi')) 'Explicit workspace configuration was not patched' }
Assert ($config.configurations[0].defines[0] -eq 'KEEP=1') 'Workspace defines lost'
Run (Join-Path $PSScriptRoot 'configure-zoi.ps1') @('-Workspace',$work)
Run (Join-Path $PSScriptRoot 'doctor-zoi.ps1') @('-Workspace',$work,'-SettingsFile',$settings)
Run (Join-Path $PSScriptRoot 'configure-zoi.ps1') @('-Workspace',$work,'-Undo')
Assert ((Setup-Read $cpp) -ceq $cppBefore) 'Workspace exact restoration failed'
Run (Join-Path $PSScriptRoot 'configure-zoi.ps1') @('-Workspace',$work)
Put $cpp ((Setup-Read $cpp)+"`n// later edit")
Run (Join-Path $PSScriptRoot 'configure-zoi.ps1') @('-Workspace',$work,'-Undo') 1
Assert ((Setup-Read $cpp).Contains('later edit')) 'Workspace later edit lost'
Write-Host '[PASS] explicit multi-config include paths / spaces and Unicode / repeat / undo / later edits preserved'

# Round-trip a real release, then inject the exact legacy-state failure into the isolated package.
$zip=Join-Path $fixture 'first.zip'
Run (Join-Path $PSScriptRoot 'make_team_package.ps1') @('-OutputPath',$zip)
Add-Type -AssemblyName System.IO.Compression.FileSystem
$unpack=Join-Path $fixture 'unpacked'
[IO.Compression.ZipFile]::ExtractToDirectory($zip,$unpack)
$lib=Join-Path $unpack 'HNIST-ZOI'
Put (Join-Path $lib 'scripts/.zoi-install-state.json') '{"zoi_path":"F:/old-owner/zoi"}'
Put (Join-Path $lib 'scripts/private.zoi.state.json') '{"secret":"must-not-ship"}'
Run (Join-Path $lib 'scripts/make_team_package.ps1') @()
$release=@(Get-ChildItem -LiteralPath (Join-Path $lib 'docs/releases') -Filter '*.zip')
Assert ($release.Count -eq 1) 'Default release path failed'
$archive=[IO.Compression.ZipFile]::OpenRead($release[0].FullName)
try {
    $names=@($archive.Entries | ForEach-Object { $_.FullName.Replace('\','/') })
    Assert (@($names | Where-Object { $_ -match 'zoi-install-state|private.zoi|/(releases|backups)/(?!README\.md$)' }).Count -eq 0) 'Personal state leaked'
    foreach ($guide in @('docs/releases/README.md','docs/backups/README.md')) {
        Assert ($names -contains ('HNIST-ZOI/'+$guide)) ('Delivery guide missing: '+$guide)
    }
} finally { $archive.Dispose() }
$delivered=Join-Path $fixture 'delivered'; [IO.Compression.ZipFile]::ExtractToDirectory($release[0].FullName,$delivered)
$install=Join-Path $delivered 'HNIST-ZOI/scripts/install-zoi.ps1'
Run $install @('-SettingsFile',(Join-Path $fixture 'recipient/settings.json'))
Run (Join-Path $delivered 'HNIST-ZOI/scripts/uninstall-zoi.ps1') @('-SettingsFile',(Join-Path $fixture 'recipient/settings.json'))
Write-Host '[PASS] actual ZIP delivery / default releases directory / legacy exclusion / clean recipient install'

$cache=Join-Path $fixture 'cache'; [void][IO.Directory]::CreateDirectory($cache)
foreach ($i in 1..5) {
    $d=Join-Path $cache ('run'+$i); [void][IO.Directory]::CreateDirectory($d)
    Put (Join-Path $d '.zoi-run.json') (Setup-Json @{format=1;path=$d;kind='setup';status='PASS';completedUtc=('2026-01-0'+$i+'T00:00:00Z')})
}
foreach ($name in @('codex-work','unknown','failed','wrong-path','corrupt')) { Put (Join-Path $cache ($name+'/keep.cpp')) '// keep' }
$failed=Join-Path $cache 'failed'
Put (Join-Path $failed '.zoi-run.json') (Setup-Json @{format=1;path=$failed;kind='setup';status='FAIL';completedUtc='2026-01-01T00:00:00Z'})
Put (Join-Path $cache 'wrong-path/.zoi-run.json') (Setup-Json @{format=1;path=$fixture;kind='setup';status='PASS';completedUtc='2026-01-01T00:00:00Z'})
Put (Join-Path $cache 'corrupt/.zoi-run.json') '{broken'
Run (Join-Path $PSScriptRoot 'clean_checks.ps1') @('-BuildRoot',$cache)
Assert ([IO.Directory]::Exists((Join-Path $cache 'run1'))) 'Preview deleted files'
$cacheLock=New-Object IO.FileStream((Join-Path $cache '.zoi-clean.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
try {
    Run (Join-Path $PSScriptRoot 'clean_checks.ps1') @('-BuildRoot',$cache,'-Apply')
    Assert ([IO.Directory]::Exists((Join-Path $cache 'run1'))) 'Concurrent cleanup ignored the cache lock'
} finally { $cacheLock.Dispose() }
Run (Join-Path $PSScriptRoot 'clean_checks.ps1') @('-BuildRoot',$cache,'-Apply')
Assert (-not [IO.Directory]::Exists((Join-Path $cache 'run2')) -and [IO.Directory]::Exists((Join-Path $cache 'run3'))) 'Retention count wrong'
foreach ($name in @('codex-work','unknown','failed','wrong-path','corrupt')) { Assert ([IO.File]::Exists((Join-Path $cache ($name+'/keep.cpp')))) 'Protected cache removed' }
Write-Host '[PASS] cleanup preview / concurrency lock / retain three / failed, unknown, malformed, wrong-path and manual work preserved'
Write-Host ('Deployment self-test passed: '+$script:calls+' commands; logs: '+$fixture)
Complete-CheckWorkspace $fixture 'tooling'
