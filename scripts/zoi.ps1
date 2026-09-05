param(
    [Parameter(Position = 0)][ValidateSet('expand', 'restore', 'status', 'forget')][string]$Action = 'expand',
    [Parameter(Position = 1)][string]$Target = '',
    [switch]$Force,
    [switch]$NoClipboard
)
# ASCII only for Windows PowerShell 5.1. Source files are UTF-8.
# Generated blocks are immutable; code outside them remains editable.
# A per-file journal makes interrupted source/state replacement recoverable.
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$enc = New-Object Text.UTF8Encoding($false, $true)
$comparison = [StringComparer]::Ordinal
if ([IO.Path]::DirectorySeparatorChar -eq '\') { $comparison = [StringComparer]::OrdinalIgnoreCase }
function New-Map { return ,(New-Object 'Collections.Generic.Dictionary[string,object]' ($comparison)) }
function Read-Text([string]$p) { return $enc.GetString([IO.File]::ReadAllBytes($p)) }
function Hash([string]$s) {
    $h = [Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($h.ComputeHash($enc.GetBytes($s))).Replace('-', '') }
    finally { $h.Dispose() }
}
function Block-Hash([string]$s) { return Hash ($s.Replace("`r`n", "`n")) }
function Lines([string]$s) { return @([regex]::Matches($s, '[^\n]*(?:\n|$)') | Where-Object Length -gt 0 | ForEach-Object Value) }
function State-Path([string]$p) { return [IO.Path]::ChangeExtension($p, '.zoi.state.json') }
function Journal-Path([string]$p) { return [IO.Path]::ChangeExtension($p, '.zoi.pending.json') }
function Atomic-Write([string]$p, [string]$s) {
    # Never remove an unexplained temp from an earlier/manual operation.
    $tmp = $p + '.zoi-write.tmp'
    $stream = New-Object IO.FileStream($tmp, [IO.FileMode]::CreateNew, [IO.FileAccess]::Write)
    try { $b = $enc.GetBytes($s); $stream.Write($b, 0, $b.Length); $stream.Flush($true) }
    finally { $stream.Dispose() }
    if ([IO.File]::Exists($p)) { [IO.File]::Replace($tmp, $p, [NullString]::Value) }
    else { [IO.File]::Move($tmp, $p) }
}
function Json($o) { return ConvertTo-Json -InputObject $o -Depth 20 -Compress }
function Read-State([string]$p) {
    $s = (Read-Text $p) | ConvertFrom-Json
    Check-State $s $p
    return $s
}
function Check-State($s, [string]$p) {
    if ($s.version -ne 1 -or $s.id -notmatch '^[a-f0-9]{32}$' -or
        $null -eq $s.compact -or $null -eq $s.blocks -or $null -eq $s.dependencies) { throw "Invalid state: $p" }
    $ids = @{}
    foreach ($b in $s.blocks) {
        if ($b.id -notmatch '^b[0-9]+$' -or $ids.ContainsKey($b.id) -or
            $b.hash -notmatch '^[A-F0-9]{64}$' -or $null -eq $b.original) { throw "Invalid block state: $p" }
        $ids[$b.id] = $true
    }
}
function Finish-Journal([string]$src) {
    $jp = Journal-Path $src
    if (-not [IO.File]::Exists($jp)) { return }
    $j = (Read-Text $jp) | ConvertFrom-Json
    if ($j.version -ne 1 -or $j.file -cne [IO.Path]::GetFileName($src) -or
        (Hash $j.before) -ne $j.beforeHash -or (Hash $j.after) -ne $j.afterHash -or
        (Hash (Json $j.state)) -ne $j.stateHash -or
        -not [IO.File]::Exists($src)) { throw "Invalid/orphan journal (preserved): $jp" }
    if ($null -ne $j.state) { Check-State $j.state $jp }
    $actual = Hash (Read-Text $src)
    if ($actual -ne $j.beforeHash -and $actual -ne $j.afterHash) { throw "Source changed during interrupted write: $src" }
    # Only these exact temp names belong to this validated transaction.
    $sp = State-Path $src
    foreach ($p in @($src, $sp)) {
        $tmp = $p + '.zoi-write.tmp'
        if ([IO.File]::Exists($tmp)) { [IO.File]::Delete($tmp) }
    }
    if ($actual -ne $j.afterHash) { Atomic-Write $src $j.after }
    if ($null -eq $j.state) { if ([IO.File]::Exists($sp)) { [IO.File]::Delete($sp) } }
    else { Atomic-Write $sp (Json $j.state) }
    if ($j.legacy) {
        foreach ($ext in @('.zoi.cpp', '.zoi.sha')) {
            $p = [IO.Path]::ChangeExtension($src, $ext)
            if ([IO.File]::Exists($p)) { [IO.File]::Delete($p) }
        }
    }
    [IO.File]::Delete($jp)
}
function Commit([string]$src, [string]$before, [string]$after, $state, [bool]$legacy) {
    if ((Read-Text $src) -cne $before) { throw "Source changed while preparing: $src" }
    $jp = Journal-Path $src
    if ([IO.File]::Exists($jp)) { throw "Pending transaction: $jp" }
    $j = [ordered]@{version=1; file=[IO.Path]::GetFileName($src); before=$before; after=$after;
        beforeHash=(Hash $before); afterHash=(Hash $after); state=$state; stateHash=(Hash (Json $state)); legacy=$legacy}
    Atomic-Write $jp (Json $j)
    # Regression fault injection; normal runs leave this environment unset.
    if ($env:ZOI_TEST_FAULT -eq 'journal') { throw 'Injected failure after journal' }
    if ($before -cne $after) { Atomic-Write $src $after }
    if ($env:ZOI_TEST_FAULT -eq 'source') { throw 'Injected failure after source' }
    Finish-Journal $src
}
function Fold([string]$text, $state, [bool]$forget = $false) {
    $map = @{}; foreach ($b in $state.blocks) { $map[$b.id] = $b }
    $used = @{}; $out = New-Object Text.StringBuilder
    $bom = $text.StartsWith([string][char]0xfeff)
    if ($bom) { [void]$out.Append([char]0xfeff); $text = $text.Substring(1) }
    $active = $null; $body = New-Object Text.StringBuilder
    foreach ($line in (Lines $text)) {
        $plain = $line.TrimEnd([char[]]"`r`n")
        if ($plain -match '^// zoi:(begin|end) ([a-f0-9]{32}):(b[0-9]+)$') {
            $kind = $Matches[1]; $sid = $Matches[2]; $id = $Matches[3]
            if ($sid -cne $state.id -or -not $map.ContainsKey($id)) { throw "Unknown generated marker: $plain" }
            if ($kind -eq 'begin') {
                if ($null -ne $active -or $used.ContainsKey($id)) { throw "Duplicate/nested block: $id" }
                $active = $id; $used[$id] = $true; [void]$body.Clear()
            } else {
                if ($active -cne $id) { throw "Unpaired block: $id" }
                if (-not $forget -and (Block-Hash $body.ToString()) -cne $map[$id].hash) {
                    throw "Edited generated block $id ($($map[$id].original.Trim())). Preserve edits with forget, or undo the block edit."
                }
                if ($forget) { [void]$out.Append($body.ToString()) }
                else { [void]$out.Append($map[$id].original) }
                $active = $null
            }
        } elseif ($plain -match '^\s*//\s*zoi:(begin|end)\b') { throw "Malformed generated marker: $plain" }
        elseif ($null -ne $active) { [void]$body.Append($line) }
        else { [void]$out.Append($line) }
    }
    if ($null -ne $active) { throw "Missing end marker: $active" }
    return $out.ToString()
}
function Resolve-Inc([string]$base, [string]$inc) {
    # C++ headers often use '/' on both platforms.
    $inc = $inc.Replace('\', [IO.Path]::DirectorySeparatorChar).Replace('/', [IO.Path]::DirectorySeparatorChar)
    $candidates = @()
    if ([IO.Path]::IsPathRooted($inc)) { $candidates = @($inc) }
    else { $candidates = @((Join-Path $base $inc), (Join-Path (Join-Path $Root 'zoi') $inc), (Join-Path $Root $inc)) }
    foreach ($p in $candidates) { if ([IO.File]::Exists($p)) { return [IO.Path]::GetFullPath($p) } }
    throw "Unresolved local include '$inc' from $base"
}
function Scan([string]$text, [string]$path) {
    if ($text -match '(?m)^\s*//\s*zoi:(begin|end)\b') { throw "Reserved generated marker in dependency/source: $path" }
    if ($text -match '\\\r?\n') { throw "Line splicing unsupported: $path" }
    # Mask comments and raw string literals, retaining positions/newlines.
    # Ordinary literals are consumed intact so their // and /* aren't comments.
    $pattern = '(?s)R"(?<delim>[^ ()\\\t\r\n]{0,16})\(.*?\)\k<delim>"|"(?:\\.|[^"\\])*"|''(?:\\.|[^''\\])*''|/\*.*?\*/|//[^\r\n]*'
    $clean = [regex]::Replace($text, $pattern, [Text.RegularExpressions.MatchEvaluator]{ param($m)
        if ($m.Value.StartsWith('/*') -or $m.Value.StartsWith('//') -or $m.Value.StartsWith('R"')) {
            return [regex]::Replace($m.Value, '[^\r\n]', ' ')
        }
        return $m.Value
    }).TrimStart([char]0xfeff)
    $ls = @(Lines $clean); $originalLines = @(Lines ($text.TrimStart([char]0xfeff)))
    $guard = ''; $pragma = -1; $first = -1; $last = -1
    for ($i=0; $i -lt $ls.Count; $i++) { if ($ls[$i].Trim()) { if ($first -lt 0) { $first=$i }; $last=$i } }
    $define = -1
    if ($first -ge 0 -and $ls[$first] -match '^\s*#\s*ifndef\s+(\w+)\s*$') {
        $candidate = $Matches[1]
        for ($i=$first+1; $i -lt $ls.Count; $i++) { if ($ls[$i].Trim()) { $define=$i; break } }
        if ($define -ge 0 -and $ls[$define] -match ('^\s*#\s*define\s+'+[regex]::Escape($candidate)+'\s*$') -and
            $ls[$last] -match '^\s*#\s*endif\s*$') { $guard=$candidate }
    }
    $depth=0; $incs=@{}; $guardClosed=$false
    for ($i=0; $i -lt $ls.Count; $i++) {
        $l=$ls[$i]
        if ($l -match '^\s*#.*\\\s*$') { throw "Continued directive unsupported: ${path}:$($i+1)" }
        if ($l -match '^\s*#\s*(if|ifdef|ifndef)\b') { $depth++ }
        if ($l -match '^\s*#\s*(else|elif)\b' -and $guard -ne '' -and $depth -eq 1) { throw "Nonstandard guard: $path" }
        if ($l -match '^\s*#\s*endif\b') {
            $depth--; if ($depth -lt 0) { throw "Unpaired conditional: $path" }
            if ($guard -ne '' -and $depth -eq 0) { if ($i -ne $last) { throw "Nonstandard guard: $path" }; $guardClosed=$true }
        }
        if ($l -match '^\s*#\s*pragma\s+once\s*$') {
            if ($depth -ne 0 -or $i -ne $first) { throw "Conditional/non-leading pragma once: $path" }; $pragma=$i
        }
        if ($l -match '^\s*#\s*include\b(.*)$') {
            $operand=$Matches[1].Trim()
            if ($operand -match '^<[^>]+>$') { continue }
            if ($operand -notmatch '^"([^"]+)"$') { throw "Macro/complex include unsupported: ${path}:$($i+1)" }
            $inc=$Matches[1]; $allowed=0; if ($guard -ne '') { $allowed=1 }
            if ($depth -ne $allowed) { throw "Conditional local include unsupported: ${path}:$($i+1)" }
            # A block comment spanning this directive's line cannot be removed
            # with the include without changing the surrounding comment scope.
            if ($originalLines[$i] -notmatch '^\s*#\s*include\s*"[^"]+"\s*(?://[^\r\n]*)?\s*$') {
                throw "Comment-split local include unsupported: ${path}:$($i+1)"
            }
            $incs[$i]=$inc
        }
        if ($l -match '^\s*#\s*undef\b') { throw "Macro undef unsupported (may reset an include guard): $path" }
    }
    if ($depth -ne 0 -or ($guard -ne '' -and -not $guardClosed)) { throw "Unpaired conditional: $path" }
    return @{includes=$incs; guarded=($guard -ne '' -or $pragma -ge 0); pragma=$pragma}
}
function Render-Dependency([string]$path) {
    # Only make_stubs' pure forwarding form is an alias.
    $aliases=New-Map
    while ($true) {
        if ($aliases.ContainsKey($path)) { throw "Forwarding cycle: $path" }
        $aliases[$path]=$true
        $text = Read-Text $path; $script:deps[$path] = Hash $text
        if ($text -notmatch '\A(?:\uFEFF)?// zoi stub -> [^\r\n]+\r?\n\s*#include "([^"]+)"\s*\z') { break }
        $path=Resolve-Inc (Split-Path -Parent $path) $Matches[1]
    }
    $info = Scan $text $path
    if ($info.guarded -and $script:emitted.ContainsKey($path)) { return '' }
    if ($script:visiting.ContainsKey($path)) { throw "Unguarded include cycle: $($script:chain -join ' -> ') -> $path" }
    if ($info.guarded) { $script:emitted[$path]=$true }
    $script:visiting[$path]=$true; $script:chain.Add($path)
    try {
        $out=New-Object Text.StringBuilder; $ls=@(Lines ($text.TrimStart([char]0xfeff)))
        for ($i=0; $i -lt $ls.Count; $i++) {
            if ($info.includes.ContainsKey($i)) {
                [void]$out.Append((Render-Dependency (Resolve-Inc (Split-Path -Parent $path) $info.includes[$i])))
            } elseif ($i -ne $info.pragma) { [void]$out.Append($ls[$i]) }
        }
        $result=$out.ToString().Replace("`r`n", "`n").Replace("`n", $script:nl)
        if ($result.Length -gt 0 -and -not $result.EndsWith("`n")) { $result += $script:nl }
        return $result
    } finally { [void]$script:visiting.Remove($path); $script:chain.RemoveAt($script:chain.Count-1) }
}
function Expand-Source([string]$src, [string]$compact, [string]$id) {
    if ($compact -match '(?m)^\s*//\s*zoi:(begin|end)\b') { throw 'Unmanaged generated markers; preserve the state file or use a clean compact source.' }
    $script:deps=New-Map; $script:emitted=New-Map; $script:visiting=New-Map
    $script:chain=New-Object 'Collections.Generic.List[string]'
    $script:nl="`n"; if ($compact.Contains("`r`n")) { $script:nl="`r`n" }
    $info=Scan $compact $src; $out=New-Object Text.StringBuilder
    if ($compact.StartsWith([string][char]0xfeff)) { [void]$out.Append([char]0xfeff) }
    $ls=@(Lines ($compact.TrimStart([char]0xfeff))); $blocks=@()
    for ($i=0; $i -lt $ls.Count; $i++) {
        if ($info.includes.ContainsKey($i)) {
            $path=Resolve-Inc (Split-Path -Parent $src) $info.includes[$i]
            $body=Render-Dependency $path; $bid='b'+$blocks.Count
            $blocks += [ordered]@{id=$bid; original=$ls[$i]; hash=(Block-Hash $body); path=$path}
            [void]$out.Append("// zoi:begin ${id}:$bid"+$script:nl+$body+"// zoi:end ${id}:$bid"+$script:nl)
        } else { [void]$out.Append($ls[$i]) }
    }
    $dependencies=@($script:deps.Keys | Sort-Object -CaseSensitive | ForEach-Object { [ordered]@{path=$_; hash=$script:deps[$_]} })
    return @{text=$out.ToString(); state=[ordered]@{version=1; id=$id; compact=$compact; blocks=$blocks; dependencies=$dependencies}}
}
function Process-File([string]$src) {
    if ($src -notmatch '\.cpp$' -or $src -match '\.zoi\.cpp$') { throw "Expected a source .cpp (not a backup): $src" }
    $jp=Journal-Path $src; $sp=State-Path $src
    if ([IO.File]::Exists($jp)) {
        if ($Action -eq 'status') { throw "Pending transaction; expand/restore will recover it: $jp" }
        Finish-Journal $src
    }
    if (-not [IO.File]::Exists($src)) { throw "Missing source; state/backup preserved: $src" }
    $before=Read-Text $src; $compact=$before; $state=$null; $legacy=$false
    $bak=[IO.Path]::ChangeExtension($src, '.zoi.cpp'); $sha=[IO.Path]::ChangeExtension($src, '.zoi.sha')
    if ([IO.File]::Exists($sp)) {
        if ([IO.File]::Exists($bak) -or [IO.File]::Exists($sha)) { throw "Mixed new/legacy state: $src" }
        $state=Read-State $sp
        if ($Force -and $Action -eq 'restore') { $compact=$state.compact }
        else { $compact=Fold $before $state ($Action -eq 'forget') }
    } elseif ([IO.File]::Exists($bak)) {
        $legacy=$true
        if ($Action -eq 'forget') {
            if (-not [IO.File]::Exists($sha) -or (Read-Text $sha).Trim() -notmatch '^[a-fA-F0-9]{40}$') {
                throw 'Forget needs a valid legacy SHA sidecar; unknown backups are preserved.'
            }
            # Explicitly release old management, without parsing old markers or
            # writing the compact backup over the user's completed solution.
            Commit $src $before $before $null $true
            Write-Host "[OK] forgot legacy backup; source unchanged: $src"
            return
        }
        $valid=[IO.File]::Exists($sha)
        if ($valid) { $valid=(Get-FileHash -LiteralPath $src -Algorithm SHA1).Hash -eq (Read-Text $sha).Trim() }
        if (-not $valid -and -not ($Force -and $Action -eq 'restore')) { throw "Legacy conflict/missing SHA; backup preserved: $bak" }
        $compact=Read-Text $bak
    } elseif ([IO.File]::Exists($sha)) { throw "Orphan SHA (preserved): $sha" }
    if ($Action -eq 'status') {
        $tag='unmanaged'; if ($null -ne $state) { $tag='recoverable' }; if ($legacy) { $tag='legacy: ready to migrate/restore' }
        Write-Host "[OK] $tag : $src"; return
    }
    if ($Action -eq 'expand') {
        $id=[Guid]::NewGuid().ToString('N'); if ($null -ne $state) { $id=$state.id }
        $r=Expand-Source $src $compact $id
        if ($null -ne $state) {
            $old=New-Map; foreach ($d in $state.dependencies) { $old[$d.path]=$d.hash }
            foreach ($d in $r.state.dependencies) {
                if ($old.ContainsKey($d.path) -and $old[$d.path] -ne $d.hash) { Write-Host "[UPDATE] dependency: $($d.path)" }
            }
        }
        $next=$r.state; if ($next.blocks.Count -eq 0) { $next=$null }
        $unchanged=$null -ne $state -and $r.text -ceq $before -and $state.compact -ceq $compact -and
            (Json $state.dependencies) -ceq (Json $r.state.dependencies)
        if (-not $unchanged -and ($null -ne $next -or $null -ne $state -or $legacy)) { Commit $src $before $r.text $next $legacy }
        if (-not $NoClipboard) {
            try { Set-Clipboard -Value $r.text; Write-Host '[OK] copied to clipboard' }
            catch { Write-Host '[WARN] clipboard unavailable' }
        }
        Write-Host "[OK] expanded: $src ($($r.state.blocks.Count) blocks)"
    } elseif ($null -ne $state -or $legacy) {
        Commit $src $before $compact $null $legacy
        Write-Host "[OK] ${Action}: $src"
    } else { Write-Host "[OK] unmanaged; unchanged: $src" }
}
try {
    if ($Force -and $Action -ne 'restore') { throw '-Force is only supported by restore (discard edits).' }
    if ($Target -eq '') {
        if ($Action -in @('expand','forget')) { throw "$Action needs a source file" }
        $Target=(Get-Location).Path
    }
    $full=$ExecutionContext.SessionState.Path.GetUnresolvedProviderPathFromPSPath($Target)
    $files=New-Map
    if ([IO.Directory]::Exists($full)) {
        if ($Action -in @('expand','forget')) { throw "$Action needs a source file" }
        foreach ($item in (Get-ChildItem -LiteralPath $full -Recurse -File)) {
            if ($item.Name -match '\.zoi\.(state\.json|pending\.json|cpp|sha)$') {
                $p=$item.FullName -replace '\.zoi\.(state\.json|pending\.json|cpp|sha)$','.cpp'; $files[$p]=$true
            } elseif ($item.Name -like '*.zoi-write.tmp') { Write-Host "[WARN] temporary file retained: $($item.FullName)" }
        }
    } else { $files[$full]=$true }
    $failed=0
    foreach ($src in @($files.Keys | Sort-Object)) {
        $lock=$null
        try {
            if ($Action -ne 'status') {
                # OS closes/deletes this lock even if the process is killed.
                $lock=New-Object IO.FileStream(($src+'.zoi.lock'), [IO.FileMode]::OpenOrCreate,
                    [IO.FileAccess]::ReadWrite, [IO.FileShare]::None, 1, [IO.FileOptions]::DeleteOnClose)
            }
            Process-File $src
        } catch { Write-Host "[FAIL] $src : $($_.Exception.Message)"; $failed++ }
        finally { if ($null -ne $lock) { $lock.Dispose() } }
    }
    if ($files.Count -eq 0) { Write-Host '[OK] no pending state' }
    if ($failed) { exit 1 }; exit 0
} catch { Write-Host "[FAIL] $($_.Exception.Message)"; exit 1 }
