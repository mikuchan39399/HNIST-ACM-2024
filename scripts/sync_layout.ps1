param([switch]$Check, [switch]$NoRefresh)
# Stable source stamps own engine identity; paths are derived, not guessed.
# Pure PowerShell 5.1/7; no Git or Python dependency in normal commands.
$ErrorActionPreference='Stop'
$layoutRoot=[IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$layoutEnc=New-Object Text.UTF8Encoding($false)
function Layout-Full([string]$base,[string]$name) {
    $p=[IO.Path]::GetFullPath((Join-Path $base $name))
    if (-not $p.StartsWith($layoutRoot+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw ('Layout path outside repository: '+$name) }
    return $p
}
function Layout-Rel([string]$base,[string]$path) {
    $u=New-Object Uri ($base.TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar)
    return [Uri]::UnescapeDataString($u.MakeRelativeUri((New-Object Uri $path)).ToString())
}
function Layout-Files([string]$dir) {
    foreach ($f in Get-ChildItem -LiteralPath $dir -Force) {
        if ($f.Name -in @('.git','.zoi-checks','.ci-results','.vscode','__pycache__')) { continue }
        if ($f.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw ('Layout refuses reparse point: '+$f.FullName) }
        if ($f.PSIsContainer) {
            if ($f.FullName -in @((Join-Path $layoutRoot 'docs/releases'),(Join-Path $layoutRoot 'docs/backups'),(Join-Path $layoutRoot 'docs/booklet/output'))) { continue }
            Layout-Files $f.FullName
        } else { $f }
    }
}
$files=@(Layout-Files $layoutRoot)
$cpp=@($files | Where-Object { $_.FullName.StartsWith((Join-Path $layoutRoot 'algorithms')+[IO.Path]::DirectorySeparatorChar) -and $_.Extension -in @('.cpp','.h','.hpp') })
$stamps=@{}; $catalogNames=@{}; $moves=@{}; $inverse=@{}; $plan=@{}; $original=@{}
function Layout-Plan([string]$path,[string]$body) {
    $old=if([IO.File]::Exists($path)){[IO.File]::ReadAllText($path,$layoutEnc)}else{$null}
    if ($null -eq $old -or $old -cne $body) { $plan[$path]=$body; $original[$path]=$old }
}
function Layout-Move([string]$old,[string]$new) {
    if ($old -ceq $new) { return }
    if ($moves.ContainsKey($old) -and $moves[$old] -cne $new) { throw ('Conflicting relocation: '+$old) }
    if ($inverse.ContainsKey($new) -and $inverse[$new] -cne $old) { throw ('Ambiguous relocation: '+$new) }
    $moves[$old]=$new; $inverse[$new]=$old
}
foreach ($f in $cpp) {
    $reader=[IO.File]::OpenText($f.FullName)
    try { $first=$reader.ReadLine() } finally { $reader.Dispose() }
    if ($first -match '^// zoi: ([A-Za-z][A-Za-z0-9_]*)$') {
        $id=$Matches[1]
        if ($stamps.ContainsKey($id)) { throw ('Duplicate zoi identity: '+$id+'; change the copied template stamp before syncing') }
        $stamps[$id]=$f.FullName
    }
}
$cat=Join-Path $layoutRoot 'zoi/_catalog.txt'
$catalog=[IO.File]::ReadAllText($cat,$layoutEnc)
$newCatalog=[regex]::Replace($catalog,'(?m)^(?<id>\^?[A-Za-z][A-Za-z0-9_]*)\t(?<path>[^\r\n]+)',[Text.RegularExpressions.MatchEvaluator]{param($m)
    $id=$m.Groups['id'].Value; $old=Layout-Full $layoutRoot $m.Groups['path'].Value
    if ($catalogNames.ContainsKey($id)) { throw ('Duplicate catalog identity: '+$id) }
    $catalogNames[$id]=$true
    if ($id.StartsWith('^')) {
        if ([IO.File]::Exists($old)) { return $m.Value }
        $found=@($files | Where-Object { $_.FullName.StartsWith((Join-Path $layoutRoot 'algorithms')+[IO.Path]::DirectorySeparatorChar) -and $_.Name -ceq [IO.Path]::GetFileName($old) })
        if ($found.Count -ne 1) { throw ('Missing/ambiguous note; update catalog explicitly: '+$id) }
        Layout-Move $old $found[0].FullName
        return $id+"`t"+(Layout-Rel $layoutRoot $found[0].FullName)
    }
    if (-not $stamps.ContainsKey($id)) { throw ('Missing source stamp: '+$id+'; deletion/merge must be explicitly registered') }
    $new=$stamps[$id]
    if ($old -cne $new -and [IO.File]::Exists($old)) { throw ('Old catalog path still exists: '+$old+'; resolve replacement explicitly') }
    Layout-Move $old $new
    return $id+"`t"+(Layout-Rel $layoutRoot $new)
})
foreach ($id in @($stamps.Keys | Sort-Object)) {
    $target=Layout-Rel $layoutRoot $stamps[$id]
    if (-not $catalogNames.ContainsKey($id)) {
        $newCatalog=$newCatalog.TrimEnd("`r","`n")+"`n"+$id+"`t"+$target+"`n"
        Write-Host ('[LAYOUT NEW] '+$id+'; learning/design/behavior coverage still need human decisions')
    }
    Layout-Plan (Join-Path $layoutRoot ('zoi/'+$id+'.h')) ('// zoi stub -> '+$target+"`n"+'#include "../'+$target+'"'+"`n")
}
# A registered suite has a stable, unique file name; renamed suites are explicit.
$specPath=Join-Path $layoutRoot 'rules/verification.json'
if ([IO.File]::Exists($specPath)) {
    $specText=[IO.File]::ReadAllText($specPath,$layoutEnc)
    $spec=$specText | ConvertFrom-Json
    foreach ($row in $spec.coverage) {
        $old=Layout-Full $layoutRoot $row.suite
        if (-not [IO.File]::Exists($old) -and -not $moves.ContainsKey($old)) {
            $found=@($cpp | Where-Object { $_.Name -ceq [IO.Path]::GetFileName($old) })
            if ($found.Count -ne 1) { throw ('Missing/ambiguous suite; update verification.json explicitly: '+$row.suite) }
            Layout-Move $old $found[0].FullName
        }
    }
    $specText=[regex]::Replace($specText,'"(?:template|suite)"\s*:\s*"([^"\\]+)"',[Text.RegularExpressions.MatchEvaluator]{param($m)
        $old=Layout-Full $layoutRoot $m.Groups[1].Value
        if ($moves.ContainsKey($old)) { return $m.Value.Replace($m.Groups[1].Value,(Layout-Rel $layoutRoot $moves[$old])) }
        return $m.Value
    })
    Layout-Plan $specPath $specText
}
if ($moves.Count) {
    foreach ($f in $cpp) {
        $path=$f.FullName; $base=$f.DirectoryName
        $oldBase=if($inverse.ContainsKey($path)){Split-Path -Parent $inverse[$path]}else{$base}
        $body=[IO.File]::ReadAllText($path,$layoutEnc)
        $masked=[regex]::Replace($body,'(?s)R"(?<d>[^ ()\\\t\r\n]{0,16})\(.*?\)\k<d>"|"(?:\\.|[^"\\\r\n])*"|''(?:\\.|[^''\\\r\n])*''|/\*.*?\*/|//[^\r\n]*',[Text.RegularExpressions.MatchEvaluator]{param($m)
            if ($m.Value.StartsWith('/') -or $m.Value.StartsWith('R"')) { return [regex]::Replace($m.Value,'[^\r\n]',' ') }; return $m.Value
        })
        $matches=@([regex]::Matches($masked,'(?m)^\s*#\s*include\s*"([^"]+)"'))
        for ($i=$matches.Count-1;$i -ge 0;$i--) {
            $g=$matches[$i].Groups[1]; $inc=$g.Value
            if ([IO.Path]::IsPathRooted($inc)) { continue }
            $old=Layout-Full $oldBase $inc; $now=Layout-Full $base $inc
            $target=if($moves.ContainsKey($old)){$moves[$old]}elseif([IO.File]::Exists($old)){$old}else{$null}
            if ($target -and $target -cne $now) {
                if ([IO.File]::Exists($now) -and $now -cne $old) { throw ('Ambiguous include after relocation: '+$path+' -> '+$inc) }
                $body=$body.Substring(0,$g.Index)+(Layout-Rel $base $target)+$body.Substring($g.Index+$g.Length)
            }
        }
        Layout-Plan $path $body
    }
    foreach ($f in @($files | Where-Object { $_.Extension -eq '.md' })) {
        $rel=Layout-Rel $layoutRoot $f.FullName
        if ($rel -eq 'records/tooling/rule_history.md' -or ($rel -match '^records/tooling/docs-20260906(-pass2)?/' -and $f.Name -ne 'README.md')) { continue }
        $body=[IO.File]::ReadAllText($f.FullName,$layoutEnc)
        # Mask examples/comments without changing offsets. Labels and historical text stay intact.
        $masked=[regex]::Replace($body,'(?ms)^ {0,3}(?<f>`{3,}|~{3,})[^\r\n]*\r?\n.*?^ {0,3}\k<f>[^\r\n]*|<!--.*?-->|(?<tick>`+)[^\r\n]*?\k<tick>',[Text.RegularExpressions.MatchEvaluator]{param($m) [regex]::Replace($m.Value,'[^\r\n]',' ')})
        $links=@([regex]::Matches($masked,'\]\(\s*(?:<(?<angle>[^>\r\n]+)>|(?<plain>[^\s<>\)]+))|(?m)^ {0,3}\[[^\]\r\n]+\]:\s*(?:<(?<refAngle>[^>\r\n]+)>|(?<refPlain>[^\s<>]+))'))
        for ($i=$links.Count-1;$i -ge 0;$i--) {
            $g=@('angle','plain','refAngle','refPlain' | ForEach-Object { $links[$i].Groups[$_] } | Where-Object { $_.Success })[0]
            $url=$g.Value
            if ($url -match '^(#|/|[A-Za-z][A-Za-z0-9+.-]*:)') { continue }
            $parts=$url -split '#',2
            $old=Layout-Full $f.DirectoryName ([Uri]::UnescapeDataString($parts[0]))
            if ($moves.ContainsKey($old)) {
                $new=(Layout-Rel $f.DirectoryName $moves[$old]).Replace('%','%25').Replace(' ','%20').Replace('#','%23')
                if ($parts.Count -gt 1) { $new+='#'+$parts[1] }
                $body=$body.Substring(0,$g.Index)+$new+$body.Substring($g.Index+$g.Length)
            }
        }
        Layout-Plan $f.FullName $body
    }
}
Layout-Plan $cat $newCatalog
if ($Check) {
    if ($plan.Count) { throw ('Layout out of date: '+$plan.Count+' files; run scripts/sync_layout.ps1') }
    Write-Host '[OK] layout current'; return
}
# Validate the whole plan before writing, keep catalog last so an interrupted
# write retains the old-to-new mapping. Refuse concurrent edits; rollback I/O failures.
foreach ($path in $plan.Keys) {
    $current=if([IO.File]::Exists($path)){[IO.File]::ReadAllText($path,$layoutEnc)}else{$null}
    if ($current -cne $original[$path]) { throw ('Concurrent edit, retry layout sync: '+$path) }
}
$written=New-Object 'Collections.Generic.List[string]'
try {
    foreach ($path in @($plan.Keys | Where-Object { $_ -ne $cat })+@($cat | Where-Object { $plan.ContainsKey($_) })) {
        $written.Add($path)
        [IO.File]::WriteAllText($path,$plan[$path],$layoutEnc)
    }
} catch {
    foreach ($path in $written) {
        if ($null -eq $original[$path]) { [IO.File]::Delete($path) } else { [IO.File]::WriteAllText($path,$original[$path],$layoutEnc) }
    }
    throw
}
if ($plan.Count) { Write-Host ('[OK] layout: '+$moves.Count+' relocations, '+$plan.Count+' updated files') }
if ($plan.Count -and -not $NoRefresh) {
    foreach ($script in @('make_features.ps1','make_reliability.ps1','make_verification.ps1')) {
        $entry=Join-Path $PSScriptRoot $script
        if ([IO.File]::Exists($entry)) { & $entry }
    }
}
