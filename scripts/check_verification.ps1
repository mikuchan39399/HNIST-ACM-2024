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
    foreach ($f in @('scripts/run_checks.ps1','scripts/check_process.ps1','scripts/check_inventory.ps1','scripts/check_verification.ps1','rules/verification.json')) {
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
    return [pscustomobject]@{hash=(Get-VHash ($parts -join "`n")); files=$files}
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
function Get-VState($Record,[string]$CurrentHash) {
    if (-not $Record) { return 'none' }
    if (-not $Record.stable -or $Record.fingerprint -cne $CurrentHash) { return 'stale' }
    if ($Record.status -ne 'PASS') { return 'fail' }
    return 'pass'
}
