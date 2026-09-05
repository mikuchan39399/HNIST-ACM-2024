param([string]$SettingsFile='', [string]$TasksFile='', [switch]$NoStubs)
# Win11 ships PS5.1; no administrator, Git, Node or plugin install required.
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$paths=Setup-Paths $SettingsFile $TasksFile
$sp=$paths.state; $lock=$null
try {
    if ([IO.File]::Exists($sp)) {
        $state=Setup-State $sp $root $paths
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
        if ($state.phase -eq 'removing') { throw 'Uninstall is pending; run uninstall again first' }
        Setup-Recover $state $sp
        Write-Host '[OK] already installed (original snapshots retained)'; exit 0
    }
    if ([IO.File]::Exists((Join-Path $PSScriptRoot '.zoi-install-state.json'))) {
        throw 'Legacy install state detected. Preserve it and migrate/uninstall the old setup before installing v2.'
    }
    if (-not $NoStubs) {
        . (Join-Path $PSScriptRoot 'check_inventory.ps1')
        $null=Get-CheckInventory $root
    }
    $setBefore=Setup-Snapshot $paths.settings; $taskBefore=Setup-Snapshot $paths.tasks
    $settings='{}'; if ($setBefore.exists) { $settings=$setBefore.text }
    $tasks='{}'; if ($taskBefore.exists) { $tasks=$taskBefore.text }
    $null=JC-Parse $settings; $null=JC-Parse $tasks
    $zoi=$root.Replace('\','/')+'/zoi'
    $incKey='C_Cpp.default.includePath'; $cphKey='cph.language.cpp.Args'
    $inc=JC-Get $settings $incKey
    $incAdded=$true
    if ($null -eq $inc) { $inc='[]' }
    else {
        $v=JC-Value $inc
        if ($v -is [string]) { $inc=Setup-Json @($v) }
        elseif ($v -isnot [array]) { throw 'includePath must be a string or array' }
        foreach ($x in @($v)) {
            if ($x -isnot [string]) { throw 'includePath entries must be strings' }
            if ($x.Replace('\','/').TrimEnd('/') -eq $zoi) { $incAdded=$false }
        }
    }
    if ($incAdded) { $settings=JC-Set $settings $incKey (JC-Append $inc (Setup-Json $zoi)) }
    $arg=JC-Get $settings $cphKey
    $value='-std=c++20 -O2 -Wall'
    if ($null -ne $arg) { $value=JC-Value $arg; if ($value -isnot [string]) { throw 'CPH Args must be a string' } }
    $flag='-I'+(Setup-CphPath $zoi)
    # CPH uses split(' '), not a shell parser. Never put quotes in this flag.
    $cphAdded=-not [regex]::IsMatch($value,'(?:^|\s)'+[regex]::Escape($flag)+'(?=\s|$)')
    if ($cphAdded) { $settings=JC-Set $settings $cphKey (Setup-Json ($value+' '+$flag)) }
    $rawTasks=JC-Get $tasks 'tasks'; if ($null -eq $rawTasks) { $rawTasks='[]' }
    $node=JC-Parse $rawTasks
    if ($node.kind -ne '[') { throw 'tasks must be an array' }
    $labels=@(); $hasDefault=$false
    # Assignment expands the decoded array, not the pipeline's array wrapper.
    $existing=JC-Value $rawTasks
    foreach ($t in $existing) { if ($t.group.kind -eq 'build' -and $t.group.isDefault) { $hasDefault=$true } }
    $definitions=@(
        @('zoi-expand','expand','${file}','Expand current file; save first'),
        @('zoi-restore','restore','${file}','Restore current file, keeping edits outside generated blocks'),
        @('zoi-restore-all','restore','${workspaceFolder}','Restore managed files in this workspace'),
        @('zoi-status','status','${file}','Inspect current file without writing'),
        @('zoi-forget','forget','${file}','Keep current code and remove expansion management; save first')
    )
    foreach ($def in $definitions) {
        if (@($existing | Where-Object { $_.label -ceq $def[0] }).Count) { throw "Existing task label preserved: $($def[0]). Rename it or choose another profile." }
        $t=[ordered]@{label=$def[0]; detail=$def[3]; type='process'; command='powershell.exe';
            args=@('-NoProfile','-ExecutionPolicy','Bypass','-File',($root.Replace('\','/')+'/scripts/zoi.ps1'),$def[1],$def[2]);
            problemMatcher=@(); presentation=@{reveal='always'; clear=$true}; runOptions=@{instanceLimit=1}}
        if ($def[1] -eq 'expand') { $t.group=@{kind='build'; isDefault=(-not $hasDefault)} }
        $rawTasks=JC-Append $rawTasks (Setup-Json $t); $labels += $def[0]
    }
    $tasks=JC-Set $tasks 'tasks' $rawTasks
    if ($null -eq (JC-Get $tasks 'version')) { $tasks=JC-Set $tasks 'version' '"2.0.0"' }
    $docs=@(
        @{path=$paths.settings; before=$setBefore; after=@{exists=$true;text=$settings}; from=$setBefore; to=@{exists=$true;text=$settings}},
        @{path=$paths.tasks; before=$taskBefore; after=@{exists=$true;text=$tasks}; from=$taskBefore; to=@{exists=$true;text=$tasks}}
    )
    $dirs=@()
    foreach ($p in @($paths.settings,$paths.tasks)) {
        $d=Split-Path -Parent $p
        while (-not [IO.Directory]::Exists($d)) { if ($dirs -notcontains $d) { $dirs += $d }; $d=Split-Path -Parent $d; if (-not $d) { throw 'Invalid config directory' } }
    }
    foreach ($d in @($dirs | Sort-Object Length)) { [void][IO.Directory]::CreateDirectory($d) }
    $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
    if ([IO.File]::Exists($sp)) { throw 'Another installation won the race; rerun install' }
    foreach ($d in $docs) { if (-not (Setup-Same (Setup-Snapshot $d.path) $d.before)) { throw "Config changed during preparation: $($d.path)" } }
    $state=[ordered]@{version=2; root=$root; settings=$paths.settings; tasks=$paths.tasks; phase='installing'; docs=$docs;
        dirs=$dirs; incAdded=$incAdded; cphAdded=$cphAdded; zoi=$zoi; flag=$flag; labels=$labels}
    Setup-Write $sp (Setup-Json $state)
    Setup-Recover $state $sp
    Write-Host '[OK] installed: includePath, CPH include flag, five user tasks'
    Write-Host '[NOTE] Save files before tasks. Reload VS Code if tasks are not visible.'
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally { if ($null -ne $lock) { $lock.Dispose() } }
