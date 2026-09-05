param([string]$BuildRoot='')
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (-not $BuildRoot) { $BuildRoot=Join-Path $root '.zoi-checks' }
$fixture=Join-Path ([IO.Path]::GetFullPath($BuildRoot)) ('setup-test-'+[Guid]::NewGuid().ToString('N'))
$lib=Join-Path $fixture 'library'; $profiles=Join-Path $fixture 'profiles'
foreach ($p in @((Join-Path $lib 'scripts'),(Join-Path $lib 'zoi'),$profiles)) { [void][IO.Directory]::CreateDirectory($p) }
foreach ($name in @('install-zoi.ps1','uninstall-zoi.ps1','zoi_setup.ps1','zoi_package.ps1','uninstall-zoi.cmd')) {
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $lib 'scripts')
}
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
. (Join-Path $PSScriptRoot 'check_process.ps1')
$enc=New-Object Text.UTF8Encoding($false); $script:calls=0; $script:groups=0
function Assert([bool]$ok,[string]$message) { if (-not $ok) { throw $message } }
function Put([string]$path,[string]$text) { [void][IO.Directory]::CreateDirectory((Split-Path -Parent $path)); [IO.File]::WriteAllText($path,$text,$enc) }
function File([string]$name) { return Join-Path (Join-Path $profiles $name) 'settings.json' }
function Tasks([string]$name) { return Join-Path (Split-Path -Parent (File $name)) 'tasks.json' }
function Run([string]$action,[string]$name,[int]$expected=0,[switch]$Purge,[string]$Library=$lib) {
    $script:calls++
    $argsForRun=@('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $Library ('scripts/'+$action+'-zoi.ps1')),'-SettingsFile',(File $name))
    if ($action -eq 'install') { $argsForRun+='-NoStubs' }
    if ($Purge) { $argsForRun+='-RemoveLibrary' }
    $r=Invoke-CheckProcess (Get-Process -Id $PID).Path $argsForRun $fixture 30 (Join-Path $fixture ('call-'+$script:calls))
    Assert (-not $r.TimedOut -and $r.ExitCode -eq $expected) "Unexpected setup result: $action $name; call $script:calls"
}
function Clean([string]$name) {
    $dir=Split-Path -Parent (File $name)
    if ([IO.Directory]::Exists($dir)) {
        Assert (@(Get-ChildItem -LiteralPath $dir -Recurse -Force | Where-Object Name -Match 'zoi|\.bak$').Count -eq 0) "Setup residue: $name"
    }
}
function Pass([string]$message) { $script:groups++; Write-Host "[PASS] $message" }
Run install fresh
$st=Setup-Read ((File fresh)+'.zoi-state')
Run install fresh
Assert ((Setup-Read ((File fresh)+'.zoi-state')) -ceq $st) 'Reinstall overwrote original snapshots'
$data=JC-Value (Setup-Read (Tasks fresh)); Assert ($data.tasks.Count -eq 5 -and $data.tasks[0].type -eq 'process') 'Missing user tasks'
Run uninstall fresh; Clean fresh
Assert (-not [IO.Directory]::Exists((Split-Path -Parent (File fresh)))) 'Fresh profile directories remained'
Run uninstall fresh
Pass 'fresh profile / repeat install / repeat uninstall / no state or directories left'

$settings=@'
// keep this comment
{
    "[cpp]": { "C_Cpp.default.includePath": ["nested"], },
    "C_Cpp.default.includePath": ["C:/other", /* ] } */ "C:/more",],
    "cph.language.cpp.Args": "-O3 -IC:/first -IC:/second -DMSG=\"hello\"",
    "editor.fontSize": 17,
} // trailing comment
'@
$tasks="// user tasks`r`n{`r`n`"version`":`"2.0.0`",`r`n`"tasks`":[{`"label`":`"mine`",`"type`":`"shell`",`"command`":`"echo ok`",`"group`":{`"kind`":`"build`",`"isDefault`":true}}],`r`n}`r`n"
Put (File existing) ([string][char]0xfeff+$settings); Put (Tasks existing) $tasks
Run install existing
$after=JC-Value (Setup-Read (File existing))
Assert ($after.'[cpp]'.'C_Cpp.default.includePath'[0] -eq 'nested') 'Nested property was edited'
Assert ($after.'cph.language.cpp.Args'.Contains('-IC:/first -IC:/second')) 'Other include flags were replaced'
$afterTasks=JC-Value (Setup-Read (Tasks existing))
Assert (-not $afterTasks.tasks[1].group.isDefault) 'Existing default build was displaced'
Run uninstall existing; Clean existing
Assert ((Setup-Read (File existing)) -ceq ([string][char]0xfeff+$settings)) 'Settings did not restore byte-for-byte'
Assert ((Setup-Read (Tasks existing)) -ceq $tasks) 'Tasks did not restore byte-for-byte'
Pass 'JSONC comments / nested keys / BOM / CRLF / multiple -I / exact restoration'

Put (File edited) '{"C_Cpp.default.includePath":"C:/keep","cph.language.cpp.Args":"-O2"}'
Run install edited
$s=Setup-Read (File edited)
$raw=JC-Get $s 'C_Cpp.default.includePath'; $s=JC-Set $s 'C_Cpp.default.includePath' (JC-Append $raw '"C:/new-user-path"')
$s=JC-Set $s 'editor.fontSize' '23'
$v=JC-Value (JC-Get $s 'cph.language.cpp.Args'); $s=JC-Set $s 'cph.language.cpp.Args' (Setup-Json ($v+' -DNEW=1'))
Put (File edited) $s
$t=Setup-Read (Tasks edited); $t=JC-Set $t 'tasks' (JC-Append (JC-Get $t 'tasks') '{"label":"user-added","type":"shell","command":"echo keep"}')
Put (Tasks edited) $t
Run uninstall edited; Clean edited
$s=JC-Value (Setup-Read (File edited)); $t=JC-Value (Setup-Read (Tasks edited))
Assert ($s.'C_Cpp.default.includePath'.Count -eq 2 -and $s.'C_Cpp.default.includePath'[1] -eq 'C:/new-user-path') 'User include addition lost'
Assert ($s.'editor.fontSize' -eq 23 -and $s.'cph.language.cpp.Args' -eq '-O2 -DNEW=1') 'User settings edits lost'
Assert ($t.tasks.Count -eq 1 -and $t.tasks[0].label -eq 'user-added') 'User task addition lost'
Pass 'uninstall merges later settings, include flags and unrelated tasks'

Put (File invalid) '{"a":1,"a":2}'
Run install invalid 1; Clean invalid
Assert ((Setup-Read (File invalid)) -ceq '{"a":1,"a":2}') 'Duplicate-key document changed'
Put (File collision) '{}'; Put (Tasks collision) '{"tasks":[{"label":"zoi-expand","command":"my tool"}]}'
Run install collision 1; Clean collision
Assert ((Setup-Read (File collision)) -ceq '{}') 'Task collision changed settings'
Put (File legacy) '{}'; Put ((File legacy)+'.zoi-state') '{"zoi_path":"old"}'
Run uninstall legacy 1
Assert ([IO.File]::Exists(((File legacy)+'.zoi-state'))) 'Unknown state was deleted'
Pass 'invalid JSONC / task label collision / legacy state are preserved'

foreach ($fault in @('file1','file2')) {
    $name='failure-'+$fault; $old=$env:ZOI_SETUP_TEST_FAULT
    try { $env:ZOI_SETUP_TEST_FAULT=$fault; Run install $name 1 }
    finally { $env:ZOI_SETUP_TEST_FAULT=$old }
    Run install $name
    try { $env:ZOI_SETUP_TEST_FAULT=$fault; Run uninstall $name 1 }
    finally { $env:ZOI_SETUP_TEST_FAULT=$old }
    Run uninstall $name; Clean $name
}
Pass 'interrupted settings/task writes recover in install and uninstall'

# Package deletion must not erase a user's changed or newly added file.
$modes=@('clean','changed','extra')
if ([IO.Path]::DirectorySeparatorChar -eq '\') { $modes+='cmd' }
foreach ($mode in $modes) {
    $pkg=Join-Path $fixture ('package-'+$mode)
    [void][IO.Directory]::CreateDirectory($pkg)
    Copy-Item -LiteralPath (Join-Path $lib 'scripts') -Destination (Join-Path $pkg 'scripts') -Recurse
    $entries=@(Get-ChildItem -LiteralPath $pkg -Recurse -File | ForEach-Object { @{path=$_.FullName.Substring($pkg.Length+1).Replace('\','/'); hash=(Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash} })
    Put (Join-Path $pkg '.zoi-package.json') (Setup-Json @{format=1; product='HNIST-ZOI-team-package'; files=$entries})
    if ($mode -eq 'changed') { [IO.File]::AppendAllText((Join-Path $pkg 'scripts/install-zoi.ps1'),"`n# keep edits",$enc) }
    if ($mode -eq 'extra') { Put (Join-Path $pkg 'my-solution.cpp') '// keep my code' }
    if ($mode -eq 'cmd') {
        $psi=New-Object Diagnostics.ProcessStartInfo
        $psi.FileName=$env:ComSpec
        $psi.Arguments='/d /s /c ""'+(Join-Path $pkg 'scripts/uninstall-zoi.cmd')+'" -SettingsFile "'+(File $mode)+'""'
        $psi.WorkingDirectory=Join-Path $pkg 'scripts'
        $psi.UseShellExecute=$false; $psi.CreateNoWindow=$true
        $psi.RedirectStandardInput=$true; $psi.RedirectStandardOutput=$true; $psi.RedirectStandardError=$true
        $psi.EnvironmentVariables['TEMP']=$fixture; $psi.EnvironmentVariables['TMP']=$fixture
        $process=New-Object Diagnostics.Process; $process.StartInfo=$psi
        [void]$process.Start(); $process.StandardInput.Close()
        $out=$process.StandardOutput.ReadToEndAsync(); $err=$process.StandardError.ReadToEndAsync()
        if (-not $process.WaitForExit(30000)) { $process.Kill(); throw 'Uninstall CMD timed out' }
        $stdout=$out.GetAwaiter().GetResult(); $stderr=$err.GetAwaiter().GetResult()
        [IO.File]::WriteAllText((Join-Path $fixture 'uninstall-cmd.log'),$stdout+$stderr,$enc)
        Assert ($process.ExitCode -eq 0 -and -not [IO.Directory]::Exists($pkg)) 'Self-deleting CMD failed'
        $process.Dispose(); $script:calls++
    }
    elseif ($mode -eq 'clean') { Run uninstall $mode -Purge -Library $pkg; Assert (-not [IO.Directory]::Exists($pkg)) 'Clean package remained' }
    else { Run uninstall $mode 1 -Purge -Library $pkg; Assert ([IO.Directory]::Exists($pkg)) 'Modified package removed' }
}
Pass 'owned package deletion / modified or added files retained'
Write-Host "Setup self-test: $script:groups groups passed ($script:calls commands); logs: $fixture"
