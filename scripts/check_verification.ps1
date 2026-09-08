# ASCII-only, PS5.1/PS7. Evidence is recorded by the runner.
function Get-VHash([string]$Text) {
    $sha=[Security.Cryptography.SHA256]::Create()
    try { return ([BitConverter]::ToString($sha.ComputeHash([Text.Encoding]::UTF8.GetBytes($Text)))).Replace('-','').ToLowerInvariant() } finally { $sha.Dispose() }
}
function Read-VText([string]$Path) { [IO.File]::ReadAllText($Path,[Text.Encoding]::UTF8).Replace("`r`n","`n") }
function Get-VSnapshot([string]$Root,[string]$Source) {
    $Root=[IO.Path]::GetFullPath($Root).TrimEnd('\','/')
    $pending=New-Object 'Collections.Generic.Queue[string]'
    $pending.Enqueue($Source)
    foreach ($f in @('scripts/run_checks.ps1','scripts/check_process.ps1','scripts/check_inventory.ps1','scripts/check_verification.ps1')) {
        if ([IO.File]::Exists((Join-Path $Root $f))) { $pending.Enqueue($f) }
    }
    $seen=@{}; $files=@(); $allAdded=$false
    while ($pending.Count) {
        $name=$pending.Dequeue()
        $full=if([IO.Path]::IsPathRooted($name)){[IO.Path]::GetFullPath($name)}else{[IO.Path]::GetFullPath((Join-Path $Root $name))}
        if (-not $full.StartsWith($Root+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Dependency outside repository' }
        $rel=$full.Substring($Root.Length+1).Replace('\','/')
        if ($seen.ContainsKey($rel)) { continue }; $seen[$rel]=$true
        if (-not [IO.File]::Exists($full)) { $files+= [pscustomobject]@{path=$rel;hash='missing'}; continue }
        $body=Read-VText $full
        $files += [pscustomobject]@{path=$rel;hash=(Get-VHash $body)}
        if ($rel -notmatch '\.(cpp|h|hpp)$') { continue }
        foreach ($m in [regex]::Matches($body,'(?m)^\s*#\s*include\s+([^\r\n]+)')) {
            $inc=$m.Groups[1].Value.Trim()
            if ($inc.StartsWith('<')) { continue }
            if ($inc -match '^"([^"]+)"') {
                $pending.Enqueue([IO.Path]::GetFullPath((Join-Path (Split-Path -Parent $full) $Matches[1])))
            } elseif (-not $allAdded) {
                $allAdded=$true
                foreach ($f in Get-ChildItem -LiteralPath (Join-Path $Root 'algorithms') -Recurse -File | Where-Object { $_.Extension -in @('.cpp','.h','.hpp') }) { $pending.Enqueue($f.FullName) }
            }
        }
    }
    $byPath=@{}
    foreach($file in $files){$byPath[$file.path]=$file}
    [string[]]$keys=@($byPath.Keys)
    [Array]::Sort($keys,[StringComparer]::Ordinal)
    $files=@($keys | ForEach-Object {$byPath[$_]})
    $parts=@($files | ForEach-Object { $_.path+'='+$_.hash })
    return [pscustomobject]@{version=2; hash=(Get-VHash ("v2`n"+($parts -join "`n"))); files=$files; scopes=@(Get-VScopes $Root $Source)}
}
function Read-VSpec([string]$Root,$Inventory) {
    $spec=(Read-VText (Join-Path $Root 'rules/verification.json')) | ConvertFrom-Json
    if ($spec.schema -ne 1) { throw 'Unknown verification schema' }
    $keys=@{}
    foreach ($item in $spec.coverage) {
        $key=$item.template+'|'+$item.suite
        if ($keys.ContainsKey($key)) { throw ('Duplicate coverage: '+$key) }; $keys[$key]=$true
        if ($Inventory.checks -cnotcontains $item.suite -or $Inventory.entries.path -cnotcontains $item.template) { throw ('Unknown coverage target: '+$key) }
        if (-not $item.summary -or -not $item.oracle -or @($item.api).Count -eq 0 -or @($item.cases).Count -eq 0 -or -not $item.limitations) { throw ('Incomplete coverage: '+$key) }
    }
    return $spec
}
# Length prefixes avoid delimiter collisions; ordinal sorting is culture independent.
# Summary is presentation only. All behavioral fields remain evidence-bound.
function Get-VScopeHash($Item) {
    $parts=New-Object 'Collections.Generic.List[string]'
    foreach($field in @('template','suite','api','oracle','cases','limitations')) {
        $parts.Add($field)
        [string[]]$values=@($Item.$field | ForEach-Object { ([string]$_).Replace("`r`n","`n") })
        if($field -in @('api','cases')) { [Array]::Sort($values,[StringComparer]::Ordinal) }
        $parts.Add([string]$values.Count)
        foreach($value in $values) { $parts.Add(([string]$value.Length)+':'+$value) }
    }
    Get-VHash ($parts -join "`n")
}
function Get-VScopes([string]$Root,[string]$Suite) {
    $path=Join-Path $Root 'rules/verification.json'
    if(-not [IO.File]::Exists($path)) { return }
    $spec=(Read-VText $path) | ConvertFrom-Json
    if($spec.schema -ne 1) { throw 'Unknown verification schema' }
    $byTemplate=@{}
    foreach($item in $spec.coverage) {
        if($item.suite -cne $Suite) { continue }
        if($byTemplate.ContainsKey($item.template)) { throw ('Duplicate scope: '+$item.template) }
        $byTemplate[$item.template]=[pscustomobject]@{template=$item.template; hash=(Get-VScopeHash $item)}
    }
    [string[]]$keys=@($byTemplate.Keys)
    [Array]::Sort($keys,[StringComparer]::Ordinal)
    foreach($key in $keys) { $byTemplate[$key] }
}
function Get-VScopeEvidence($Before,$After) {
    foreach($scope in $Before.scopes) {
        $end=@($After.scopes | Where-Object { $_.template -ceq $scope.template })
        [pscustomobject]@{template=$scope.template;hash=$scope.hash;stable=($end.Count -eq 1 -and $end[0].hash -ceq $scope.hash)}
    }
}
function Get-VAssessment($Record,$Snapshot,[string]$Template='') {
    $state='pass'; $reasons=@(); $changed=@()
    if(-not $Record) { return [pscustomobject]@{state='none'; reasons=@('no-record'); changed=@()} }
    # Old records cannot certify individual scopes. Never reconstruct old passes.
    if($Record.fingerprintVersion -ne 2) {
        return [pscustomobject]@{state='legacy'; reasons=@('legacy'); changed=@()}
    }
    if(-not $Record.stable) { $reasons+='during-run' }
    if($Record.fingerprint -cne $Snapshot.hash) {
        $old=@{}; $now=@{}
        foreach($f in $Record.files) { $old[$f.path]=$f.hash }
        foreach($f in $Snapshot.files) { $now[$f.path]=$f.hash }
        [string[]]$names=@(@($old.Keys)+@($now.Keys) | Sort-Object -Unique)
        [Array]::Sort($names,[StringComparer]::Ordinal)
        foreach($name in $names) {
            if($old[$name] -ceq $now[$name]) { continue }
            $changed+=$name
            if($name -like 'scripts/*') { $reasons+='tooling' }
            elseif($name -ceq $Record.path) { $reasons+='test' }
            else { $reasons+='source' }
        }
        if(-not $changed.Count) { $reasons+='fingerprint' }
    }
    if($Template) {
        $before=@($Record.scopes | Where-Object { $_.template -ceq $Template })
        $now=@($Snapshot.scopes | Where-Object { $_.template -ceq $Template })
        if($before.Count -ne 1 -or $now.Count -ne 1) { $reasons+='new-scope' }
        elseif(-not $before[0].stable) { $reasons+='scope-during-run' }
        elseif($before[0].hash -cne $now[0].hash) { $reasons+='scope' }
    }
    if($reasons.Count) { $state='stale' }
    elseif($Record.status -ne 'PASS') { $state='fail'; $reasons+='failed' }
    [pscustomobject]@{state=$state;reasons=@($reasons | Select-Object -Unique);changed=$changed}
}
function Get-VState($Record,$Snapshot,[string]$Template='') {
    (Get-VAssessment $Record $Snapshot $Template).state
}
