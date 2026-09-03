param([string]$Filter = '', [string]$OutFile = 'zoi-booklet.pdf')
# make_booklet.ps1 - printable contest booklet generator (typst, A4, 2 columns)
# Usage: powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1              -> full
#        powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1 -Filter seg  -> scoped
# Pipeline: catalog (order = section order) -> include rewrite (stub names)
#           -> SHA256[:8] over LF-normalized text -> booklet.typ -> typst compile.
# Requires: typst on PATH, or scripts\typst.exe next to this script (auto-detected).
# NOTE: keep this file ASCII-only (PS 5.1 reads no-BOM as ANSI). CJK text
#       (family names, entry titles) flows in from file paths at runtime.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$enc = New-Object System.Text.UTF8Encoding($false)

$typst = (Get-Command typst -ErrorAction SilentlyContinue).Source
if (-not $typst) {
    $local = Join-Path $PSScriptRoot 'typst.exe'
    if (Test-Path -LiteralPath $local) { $typst = $local }
    else { throw 'typst not found: install to PATH or drop typst.exe into scripts\' }
}

# ---- catalog entries (order = booklet order) ----
$zoiDir = Join-Path $root 'zoi'
$catLines = [IO.File]::ReadAllLines((Join-Path $zoiDir '_catalog.txt'), $enc)
$entries = @()
foreach ($l in $catLines) {
    $t = $l.Trim()
    if ($t -eq '' -or $t.StartsWith('#') -or $t.StartsWith('!')) { continue }
    $parts = $t -split "`t"
    if ($parts.Count -ne 2) { continue }
    $segs = $parts[1].Trim() -split '/'
    $entries += [pscustomobject]@{
        Name = $parts[0].Trim(); Rel = $parts[1].Trim()
        Family = $segs[1] + ' / ' + $segs[2]
        Cn = [IO.Path]::GetFileNameWithoutExtension($parts[1].Trim()) }
}
if ($Filter -ne '') {
    $entries = @($entries | Where-Object { $_.Family -like "*$Filter*" -or $_.Name -like "*$Filter*" -or $_.Cn -like "*$Filter*" })
}
if ($entries.Count -eq 0) { throw 'no catalog entries matched' }

# basename -> stub map for local include rewrite
$stubByFile = @{}
foreach ($e in $entries) { $stubByFile[[IO.Path]::GetFileName($e.Rel)] = $e.Name }

# ---- plugin appendix (algebra layer copy sources; full solutions skipped) ----
$plugName = -join ([char]0x63D2, [char]0x4EF6)   # plugin folder marker, CJK kept out of source bytes
$plugins = @()
foreach ($d in Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Directory) {
    if ($d.Name -notlike "*$plugName*") { continue }
    foreach ($f in Get-ChildItem $d.FullName -Recurse -Filter *.cpp) {
        if (Select-String -Path $f.FullName -Pattern 'int\s+main' -Quiet) { continue }
        $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
        $plugins += [pscustomobject]@{ Name = $f.BaseName; Rel = $rel; Family = 'Appendix'; Cn = $f.BaseName }
    }
}

# ---- per-entry transform: rewrite includes, LF-normalize, hash ----
$script:warn = @()
$script:stubMap = $stubByFile
$script:stubKeys = @($stubByFile.Keys)
function Convert-Entry($e) {
    $text = [IO.File]::ReadAllText((Join-Path $root ($e.Rel.Replace('\', '/'))), $enc)
    $ev = [System.Text.RegularExpressions.MatchEvaluator]{ param($m)
        $base = $m.Groups[2].Value
        if ($script:stubKeys -contains $base) { $m.Groups[1].Value + $script:stubMap[$base] + '.h"' }
        else { $script:warn += ($e.Name + ': unresolved include ' + $base); $m.Value }
    }
    $rew = [regex]::Replace($text, '(?m)^(\s*#\s*include\s+")(?:[^"]*[/\\])?([^"/\\]+\.cpp)"', $ev)
    $norm = ($rew -replace "`r`n", "`n").TrimEnd()
    $sha = [System.Security.Cryptography.SHA256]::Create()
    $h = ($sha.ComputeHash($enc.GetBytes($norm)) | ForEach-Object { $_.ToString('x2') }) -join ''
    [pscustomobject]@{ Meta = $e; Text = $norm; Hash = $h.Substring(0, 8); Lines = ($norm -split "`n").Count }
}
$blocks = @()
foreach ($e in $entries) { $blocks += Convert-Entry $e }
$plugBlocks = @()
foreach ($p in $plugins) { $plugBlocks += Convert-Entry $p }

# ---- assemble typst source ----
function Esc([string]$t) { return ($t -replace '([_\*\[\]#`$\\@<>])', '\$1') }   # typst markup escape
$s = New-Object System.Text.StringBuilder
[void]$s.AppendLine('#set page(paper: "a4", margin: (x: 1.1cm, y: 1.3cm), numbering: "1 / 1")')
[void]$s.AppendLine('#set text(font: ("Consolas", "Microsoft YaHei"), size: 7pt, lang: "zh")')
[void]$s.AppendLine('#set par(leading: 0.55em, spacing: 0.9em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
[void]$s.AppendLine('#show heading.where(level: 1): set text(size: 16pt)')
[void]$s.AppendLine('#show heading.where(level: 2): set text(size: 8.5pt)')
[void]$s.AppendLine('#show heading.where(level: 3): set text(size: 7pt, fill: rgb("#333333"))')
[void]$s.AppendLine('#show raw: set text(size: 5.9pt)')
[void]$s.AppendLine('#show outline: set text(size: 6pt)')
[void]$s.AppendLine('#show outline.entry: set par(leading: 0.45em)')
[void]$s.AppendLine('')
[void]$s.AppendLine('= zoi Contest Booklet')
[void]$s.AppendLine('#text(size: 6pt)[' + $blocks.Count + ' engines + ' + $plugBlocks.Count + ' algebra plugins - generated from zoi catalog. Hash = SHA256 first 8 hex over LF-normalized text, includes rewritten to stub names such as include "graph.h". On-site self-check: count lines first, then hash after LF save.]')
[void]$s.AppendLine('#outline(depth: 3, indent: 0.8em)')
[void]$s.AppendLine('')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#set page(columns: 2, margin: (x: 0.9cm, y: 1.1cm))')
[void]$s.AppendLine('')
$lastFam = ''
foreach ($b in $blocks) {
    if ($b.Meta.Family -ne $lastFam) {
        if ($lastFam -ne '') { [void]$s.AppendLine('') }
        [void]$s.AppendLine('== ' + (Esc $b.Meta.Family))
        $lastFam = $b.Meta.Family
    }
    [void]$s.AppendLine('=== ' + (Esc $b.Meta.Name) + ' -- ' + (Esc $b.Meta.Cn) + ' -- ' + $b.Lines + ' ln -- ' + $b.Hash)
    [void]$s.AppendLine('```cpp')
    [void]$s.AppendLine($b.Text)
    [void]$s.AppendLine('```')
    [void]$s.AppendLine('')
}
if ($plugBlocks.Count -gt 0) {
    [void]$s.AppendLine('== Appendix / algebra plugins (copy into solution, adapt fields)')
    foreach ($b in $plugBlocks) {
        [void]$s.AppendLine('=== ' + (Esc $b.Meta.Name) + ' -- ' + $b.Lines + ' ln -- ' + $b.Hash)
        [void]$s.AppendLine('```cpp')
        [void]$s.AppendLine($b.Text)
        [void]$s.AppendLine('```')
        [void]$s.AppendLine('')
    }
}
$typPath = Join-Path $root 'booklet.typ'
[IO.File]::WriteAllText($typPath, $s.ToString(), $enc)

# ---- compile ----
Push-Location $root
try { & $typst compile booklet.typ $OutFile; if ($LASTEXITCODE -ne 0) { throw 'typst compile failed' } }
finally { Pop-Location }

Write-Host ('[OK] booklet: ' + $blocks.Count + ' engines, ' + $plugBlocks.Count + ' plugins -> ' + (Join-Path $root $OutFile))
foreach ($w in $script:warn) { Write-Host ('[WARN] ' + $w) -ForegroundColor Yellow }
