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
    # fingerprint the expanded content into a sidecar (<name>.zoi.sha): restore
    # compares CONTENT, not mtime -- cph auto-saves before testing and editors
    # touch files without changing them, which falsely tripped the old guard
    $shaFile = $src.Substring(0, $src.Length - 4) + '.zoi.sha'
    [IO.File]::WriteAllText($shaFile, (Get-FileHash -LiteralPath $src -Algorithm SHA1).Hash + "`n", $enc)
    # auto-copy the expanded source to clipboard: paste straight into the OJ
    # (recurring per-problem cost cut; Luogu has no CLI submit, clipboard is
    #  the honest bridge; headless hosts without a clipboard degrade silently)
    try { Set-Clipboard -Value ([IO.File]::ReadAllText($src, $enc)); Write-Host '[OK] expanded source is on the clipboard' }
    catch { Write-Host '[WARN] clipboard unavailable, skip copy' }
    # legacy mtime sync (fallback for pre-sidecar temps): keep base/temp
    # timestamps equal at expansion time so "base newer" == "base edited"
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
        $shaFile = $t.FullName.Substring(0, $t.FullName.Length - 8) + '.zoi.sha'
        $conflict = $false
        if (Test-Path -LiteralPath $shaFile) {
            # content fingerprint guard: identical content = no real edits,
            # no matter how often cph/editors touched the file
            $expected = ([IO.File]::ReadAllText($shaFile, $enc)).Trim()
            $actual = (Get-FileHash -LiteralPath $base -Algorithm SHA1).Hash
            if ($expected -ne $actual) { $conflict = $true }
        }
        elseif (Test-Path -LiteralPath $base) {
            # legacy fallback (pre-sidecar temps): mtime comparison
            if ((Get-Item -LiteralPath $base).LastWriteTime -gt $t.LastWriteTime) { $conflict = $true }
        }
        if ($Action -eq 'status') {
            $tag = ''
            if ($conflict) { $tag = '  [CONFLICT: content differs from expand snapshot]'; $skipped++ }
            Write-Host ('  ' + $t.FullName + $tag)
            continue
        }
        if ($conflict -and -not $Force) {
            Write-Host ('[SKIP] expanded file was really edited, restore would lose it: ' + $base + '  (use -Force to discard)')
            $skipped++
            continue
        }
        Copy-Item -LiteralPath $t.FullName -Destination $base -Force
        Remove-Item -LiteralPath $t.FullName
        if (Test-Path -LiteralPath $shaFile) { Remove-Item -LiteralPath $shaFile }
        Write-Host ('[OK] restored: ' + $base)
    }
    if ($skipped -gt 0) { exit 1 }
    exit 0
}