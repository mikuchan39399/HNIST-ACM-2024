param(
    [Parameter(Position = 0)][ValidateSet('expand', 'restore', 'status')][string]$Action = 'expand',
    [Parameter(Position = 1)][string]$Target = '',
    [switch]$Force
)
# zoi.ps1 - in-place include bundler (make-style two-way toggle)
#   expand  <file> : backup file -> file.zoi.cpp, then write the fully inlined
#                    version INTO the file itself (cph tests it, you submit it;
#                    zero dependency on the library path afterwards)
#   restore <dir>  : recursive; copy every *.zoi.cpp back over its base .cpp
#                    and delete the temp copy (batch rollback)
#   status  <dir>  : dry-run; list pending temp copies + mtime conflicts
# Safety rails:
#   - expand refuses if <file>.zoi.cpp already exists (restore first)
#   - expand refuses a *.zoi.cpp temp as its own target
#   - expand is a no-op when the file has no resolvable local include
#   - restore skips a pair when the base was edited after expansion (base
#     mtime newer than temp mtime); -Force overrules
# NOTE: keep this file ASCII-only; PS 5.1 reads no-BOM files as ANSI and CJK
#       comments can corrupt line structure. Sources are read/written UTF-8.
# Save the editor buffer (Ctrl+S) before running expand on an open file.
$Root = Split-Path -Parent $PSScriptRoot   # scripts/ -> library root
$enc = New-Object System.Text.UTF8Encoding($false)
$script:seen = @{}

function Resolve-Inc([string]$base, [string]$inc) {
    if ([IO.Path]::IsPathRooted($inc)) {
        if (Test-Path -LiteralPath $inc) { return (Resolve-Path -LiteralPath $inc).Path }
        return $null
    }
    $c1 = Join-Path $base $inc
    if (Test-Path -LiteralPath $c1) { return (Resolve-Path -LiteralPath $c1).Path }
    $c2 = Join-Path (Join-Path $Root 'zoi') $inc
    if (Test-Path -LiteralPath $c2) { return (Resolve-Path -LiteralPath $c2).Path }
    $c3 = Join-Path $Root $inc
    if (Test-Path -LiteralPath $c3) { return (Resolve-Path -LiteralPath $c3).Path }
    return $null
}

function Expand-File([string]$path, [System.Collections.Generic.List[string]]$sink) {
    if ($script:seen.ContainsKey($path)) { return }
    $script:seen[$path] = $true
    $rel = $path
    if ($path.StartsWith($Root)) { $rel = $path.Substring($Root.Length + 1) }
    $sink.Add('// ===== expanded from: ' + $rel + ' =====')
    $dir = Split-Path -Parent $path
    foreach ($line in [IO.File]::ReadAllLines($path, $enc)) {
        if ($line -match '^\s*#\s*include\s*"([^"]+)"') {
            $target = Resolve-Inc $dir $Matches[1]
            if ($null -ne $target) {
                Expand-File $target $sink
            } else {
                Write-Host ('[WARN] unresolved include kept: ' + $line.Trim())
                $sink.Add($line)
            }
        } else {
            $sink.Add($line)
        }
    }
    $sink.Add('// ===== end: ' + $rel + ' =====')
}

function Has-LocalInc([string]$path) {
    $dir = Split-Path -Parent $path
    foreach ($line in [IO.File]::ReadAllLines($path, $enc)) {
        if ($line -match '^\s*#\s*include\s*"([^"]+)"') {
            if ($null -ne (Resolve-Inc $dir $Matches[1])) { return $true }
        }
    }
    return $false
}

if ($Action -eq 'expand') {
    if ($Target -eq '') { Write-Host '[FAIL] expand needs a file argument'; exit 1 }
    $src = (Resolve-Path -LiteralPath $Target).Path
    if ($src.EndsWith('.zoi.cpp')) { Write-Host '[FAIL] refusing to expand a temp copy'; exit 1 }
    $bak = [IO.Path]::ChangeExtension($src, '.zoi.cpp')
    if (Test-Path -LiteralPath $bak) {
        Write-Host ('[FAIL] backup already exists, restore first: ' + $bak)
        exit 1
    }
    if (-not (Has-LocalInc $src)) {
        Write-Host '[OK] nothing to expand (no local includes), file untouched'
        exit 0
    }
    Copy-Item -LiteralPath $src -Destination $bak
    $sink = New-Object 'System.Collections.Generic.List[string]'
    Expand-File $src $sink
    [IO.File]::WriteAllText($src, ($sink -join "`n") + "`n", $enc)
    # sync temp mtime to the freshly written base: otherwise the backup keeps
    # the pre-expand timestamp and restore's conflict check would fire on
    # EVERY pair (base is rewritten after the copy, so it is always newer);
    # with mtimes equal at expansion time, "base newer" means exactly
    # "base was edited after expand", which is what we want to protect
    (Get-Item -LiteralPath $bak).LastWriteTime = (Get-Item -LiteralPath $src).LastWriteTime
    Write-Host ('[OK] expanded in place: ' + (Get-Item -LiteralPath $src).Name +
        ' (' + $sink.Count.ToString() + ' lines, ' + $script:seen.Count.ToString() +
        ' files); compact copy: ' + (Get-Item -LiteralPath $bak).Name)
    exit 0
}

if ($Action -eq 'restore' -or $Action -eq 'status') {
    if ($Target -eq '') { $Target = (Get-Location).Path }
    $dir = (Resolve-Path -LiteralPath $Target).Path
    $temps = @(Get-ChildItem -LiteralPath $dir -Recurse -Filter '*.zoi.cpp')
    if ($temps.Count -eq 0) { Write-Host '[OK] no pending temp copies under ' + $dir; exit 0 }
    $skipped = 0
    foreach ($t in $temps) {
        # base name: strip trailing ".zoi.cpp" (8 chars) and re-attach ".cpp"
        $base = $t.FullName.Substring(0, $t.FullName.Length - 8) + '.cpp'
        $conflict = $false
        if (Test-Path -LiteralPath $base) {
            if ((Get-Item -LiteralPath $base).LastWriteTime -gt $t.LastWriteTime) { $conflict = $true }
        }
        if ($Action -eq 'status') {
            $tag = ''
            if ($conflict) { $tag = '  [CONFLICT: base edited after expand]'; $skipped++ }
            Write-Host ('  ' + $t.FullName + $tag)
            continue
        }
        if ($conflict -and -not $Force) {
            Write-Host ('[SKIP] base newer than temp, edits would be lost: ' + $base + '  (use -Force)')
            $skipped++
            continue
        }
        Copy-Item -LiteralPath $t.FullName -Destination $base -Force
        Remove-Item -LiteralPath $t.FullName
        Write-Host ('[OK] restored: ' + $base)
    }
    if ($skipped -gt 0) { exit 1 }
    exit 0
}