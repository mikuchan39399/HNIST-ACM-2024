param([string]$SettingsFile='', [string]$TasksFile='', [string]$KeybindingsFile='', [switch]$NoStubs,[switch]$AdoptExistingTasks)
# Win11 ships PS5.1; no administrator, Git, Node or plugin install required.
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$paths=Setup-Paths $SettingsFile $TasksFile $KeybindingsFile
$sp=$paths.state; $lock=$null; $oldState=$null
try {
    if ([IO.File]::Exists($sp)) {
        $state=Setup-State $sp $root $paths
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
        if ($state.phase -eq 'removing') { throw 'Uninstall is pending; run uninstall again first' }
        Setup-Recover $state $sp
        $oldState=Setup-State $sp $root $paths
    }
    if ([IO.File]::Exists((Join-Path $PSScriptRoot '.zoi-install-state.json'))) {
        throw 'Legacy install state detected. Preserve it and migrate/uninstall the old setup before using this installer.'
    }
    if (-not $NoStubs) {
        . (Join-Path $PSScriptRoot 'check_inventory.ps1')
        $null=Get-CheckInventory $root
    }
    $setBefore=Setup-Snapshot $paths.settings; $taskBefore=Setup-Snapshot $paths.tasks; $keyBefore=Setup-Snapshot $paths.keys
    $settings='{}'; if ($setBefore.exists) { $settings=$setBefore.text }
    $tasks='{}'; if ($taskBefore.exists) { $tasks=$taskBefore.text }
    $keys='[]'; if ($keyBefore.exists) { $keys=$keyBefore.text }
    $null=JC-Parse $settings; $null=JC-Parse $tasks
    if ((JC-Parse $keys).kind -ne '[') { throw 'keybindings.json must be an array' }
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
    $scalarKeys=@('C_Cpp.autocomplete','C_Cpp.errorSquiggles','C_Cpp.intelliSenseEngine')
    foreach ($key in $scalarKeys) {
        $value=if ($key -eq 'C_Cpp.errorSquiggles') {'enabled'} else {'default'}
        $settings=JC-Set $settings $key (Setup-Json $value)
    }
    if ($null -eq (JC-Get $settings 'C_Cpp.default.cppStandard')) {
        $settings=JC-Set $settings 'C_Cpp.default.cppStandard' '"c++20"'
    }
    $scalarKeys+='C_Cpp.default.cppStandard'
    $rawTasks=JC-Get $tasks 'tasks'; if ($null -eq $rawTasks) { $rawTasks='[]' }
    $node=JC-Parse $rawTasks
    if ($node.kind -ne '[') { throw 'tasks must be an array' }
    $labels=@(); $hasDefault=$false
    # Assignment expands the decoded array, not the pipeline's array wrapper.
    $existing=JC-Value $rawTasks
    if ($AdoptExistingTasks -and -not $oldState) {
        $node=JC-Parse $rawTasks
        for ($i=$node.children.Count-1;$i -ge 0;$i--) {
            $t=JC-Value (JC-Raw $rawTasks $node.children[$i].value)
            if ($t.label -notin @('zoi-expand','zoi-restore','zoi-restore-all','zoi-status','zoi-forget','zoi-configure','zoi-doctor','zoi-package','zoi-clean-checks')) { continue }
            $argsList=@($t.args); $fileIndex=[array]::IndexOf($argsList,'-File')
            $expected=if ($t.label -in @('zoi-expand','zoi-restore','zoi-restore-all','zoi-status','zoi-forget')) {'zoi.ps1'} else {
                @{ 'zoi-configure'='configure-zoi.ps1'; 'zoi-doctor'='doctor-zoi.ps1'; 'zoi-package'='make_team_package.ps1'; 'zoi-clean-checks'='clean_checks.ps1' }[$t.label]
            }
            if ($t.command -notmatch '^(?:powershell|pwsh)(?:\.exe)?$' -or $fileIndex -lt 0 -or $fileIndex+1 -ge $argsList.Count -or
                [IO.Path]::GetFullPath($argsList[$fileIndex+1]) -ne (Join-Path $PSScriptRoot $expected)) { throw ('Cannot adopt a task targeting another command/library: '+$t.label) }
            $rawTasks=JC-Cut $rawTasks $node $i; $node=JC-Parse $rawTasks
        }
        $existing=JC-Value $rawTasks
    }
    foreach ($t in $existing) { if ($t.group.kind -eq 'build' -and $t.group.isDefault) { $hasDefault=$true } }
    $definitions=@(
        @('zoi-expand','expand','${file}','Expand current file; save first'),
        @('zoi-restore','restore','${file}','Restore current file, keeping edits outside generated blocks'),
        @('zoi-restore-all','restore','${workspaceFolder}','Restore managed files in this workspace'),
        @('zoi-status','status','${file}','Inspect current file without writing'),
        @('zoi-forget','forget','${file}','Keep current code and remove expansion management; save first')
    )
    foreach ($def in $definitions) {
        if (@($existing | Where-Object { $_.label -ceq $def[0] }).Count) {
            if ($oldState -and $oldState.labels -ccontains $def[0]) { $labels+=$def[0]; continue }
            throw "Existing task label preserved: $($def[0]). Rename it or choose another profile."
        }
        $t=[ordered]@{label=$def[0]; detail=$def[3]; type='process'; command='powershell.exe';
            args=@('-NoProfile','-ExecutionPolicy','Bypass','-File',($root.Replace('\','/')+'/scripts/zoi.ps1'),$def[1],$def[2]);
            problemMatcher=@(); presentation=@{reveal='always'; clear=$true}; runOptions=@{instanceLimit=1}}
        if ($def[1] -eq 'expand') { $t.group=@{kind='build'; isDefault=(-not $hasDefault)} }
        $rawTasks=JC-Append $rawTasks (Setup-Json $t); $labels += $def[0]
    }
    foreach ($def in @(
        @('zoi-configure','configure-zoi.ps1',@('-Workspace','${workspaceFolder}'),'Configure this problem folder for IntelliSense'),
        @('zoi-doctor','doctor-zoi.ps1',@('-Workspace','${workspaceFolder}','-SettingsFile',$paths.settings),'Check headers, compiler, extensions and configuration'),
        @('zoi-package','make_team_package.ps1',@(),'Build a team ZIP in the library docs/releases folder'),
        @('zoi-clean-checks','clean_checks.ps1',@('-Apply'),'Remove older completed test workspaces; keep latest three per kind')
    )) {
        if (@($existing | Where-Object { $_.label -ceq $def[0] }).Count) {
            if ($oldState -and $oldState.labels -ccontains $def[0]) { $labels+=$def[0]; continue }
            throw "Existing task label preserved: $($def[0])"
        }
        $t=@{label=$def[0]; detail=$def[3]; type='process'; command='powershell.exe';
            args=(@('-NoProfile','-ExecutionPolicy','Bypass','-File',($root.Replace('\','/')+'/scripts/'+$def[1]))+@($def[2]));
            problemMatcher=@(); presentation=@{reveal='always'; clear=$true}; runOptions=@{instanceLimit=1}}
        $rawTasks=JC-Append $rawTasks (Setup-Json $t); $labels+=$def[0]
    }
    $ownedKeys=@(); if ($oldState -and $oldState.version -eq 3) { $ownedKeys=@($oldState.ownedKeys) }
    foreach ($binding in @(
        @{key='ctrl+alt+z'; command='workbench.action.tasks.runTask'; args='zoi-expand'; when='editorLangId == cpp && !editorReadonly'},
        @{key='ctrl+alt+t'; command='workbench.action.quickOpen'; args='task zoi-'}
    )) {
        $found=@((JC-Value $keys) | Where-Object { $_.key -ieq $binding.key })
        if ($found.Count) { Write-Host ('[NOTE] existing shortcut preserved: '+$binding.key); continue }
        $raw=Setup-Json $binding; $keys=JC-Append $keys $raw; $ownedKeys+=$raw
    }
    $tasks=JC-Set $tasks 'tasks' $rawTasks
    if ($null -eq (JC-Get $tasks 'version')) { $tasks=JC-Set $tasks 'version' '"2.0.0"' }
    $docs=@(
        @{path=$paths.settings; before=$setBefore; after=@{exists=$true;text=$settings}; from=$setBefore; to=@{exists=$true;text=$settings}},
        @{path=$paths.tasks; before=$taskBefore; after=@{exists=$true;text=$tasks}; from=$taskBefore; to=@{exists=$true;text=$tasks}},
        @{path=$paths.keys; before=$keyBefore; after=@{exists=$true;text=$keys}; from=$keyBefore; to=@{exists=$true;text=$keys}}
    )
    $dirs=@()
    foreach ($p in @($paths.settings,$paths.tasks,$paths.keys)) {
        $d=Split-Path -Parent $p
        while (-not [IO.Directory]::Exists($d)) { if ($dirs -notcontains $d) { $dirs += $d }; $d=Split-Path -Parent $d; if (-not $d) { throw 'Invalid config directory' } }
    }
    foreach ($d in @($dirs | Sort-Object Length)) { [void][IO.Directory]::CreateDirectory($d) }
    if ($null -eq $lock) {
        $lock=New-Object IO.FileStream(($sp+'.lock'),[IO.FileMode]::OpenOrCreate,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None,1,[IO.FileOptions]::DeleteOnClose)
        if ([IO.File]::Exists($sp)) { throw 'Another installation won the race; rerun install' }
    }
    foreach ($d in $docs) { if (-not (Setup-Same (Setup-Snapshot $d.path) $d.before)) { throw "Config changed during preparation: $($d.path)" } }
    if ($oldState) {
        if ($oldState.version -eq 3 -and @($docs | Where-Object { -not (Setup-Same $_.from $_.to) }).Count -eq 0) {
            Write-Host '[OK] already installed and checked; original snapshots retained'; exit 0
        }
        for ($i=0;$i -lt $oldState.docs.Count;$i++) { $docs[$i].before=$oldState.docs[$i].before }
        $dirs=@($oldState.dirs)+$dirs
        $incAdded=$incAdded -or $oldState.incAdded; $cphAdded=$cphAdded -or $oldState.cphAdded
    }
    $state=[ordered]@{version=3; root=$root; settings=$paths.settings; tasks=$paths.tasks; phase='installing'; docs=$docs;
        dirs=$dirs; incAdded=$incAdded; cphAdded=$cphAdded; zoi=$zoi; flag=$flag; labels=$labels; scalarKeys=$scalarKeys; ownedKeys=$ownedKeys}
    Setup-Write $sp (Setup-Json $state)
    Setup-Recover $state $sp
    Write-Host '[OK] installed: IntelliSense defaults, CPH, nine tasks and available shortcuts'
    Write-Host ('[PATH] settings: '+$paths.settings); Write-Host ('[PATH] tasks: '+$paths.tasks)
    Write-Host ('[PATH] keybindings: '+$paths.keys)
    Write-Host '[NEXT] Install/enable Microsoft C/C++ (ms-vscode.cpptools). Reload VS Code.'
    Write-Host '[NEXT] Open your problem folder and run zoi-configure once; explicit project settings can override user defaults.'
    Write-Host '[KEY] Ctrl+Alt+Z expands the active saved .cpp; Ctrl+Alt+T opens task zoi-. Existing shortcuts are preserved.'
} catch { Write-Host ('[FAIL] '+$_.Exception.Message); exit 1 }
finally { if ($null -ne $lock) { $lock.Dispose() } }
