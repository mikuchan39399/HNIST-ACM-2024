param([string]$Filter = '', [string]$OutFile = 'zoi-booklet.pdf', [int]$SoloMin = 90)
# make_booklet.ps1 - printable contest booklet generator (typst, A4 landscape, 3 columns)
# Layout: one code block per page (pagebreak before every entry); entries >= SoloMin
#         additionally start on an odd page (duplex sheet front) + parity audit.
#         Page header center shows the entry flowing on that page. Catalog lines
#         starting with '^' are prose entries (theorems/notes, no code, no stub).
# Usage: powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1              -> full
#        powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1 -Filter seg  -> scoped
# SoloMin: entries with >= SoloMin lines start on an odd page (= a physical sheet's
#          front side), so duplex-printed big algorithms open on a fresh sheet.
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
    $prose = $t.StartsWith('^')            # ^name<TAB>file = prose entry (no code, no stub)
    if ($prose) { $t = $t.Substring(1) }
    $parts = $t -split "`t"
    if ($parts.Count -ne 2) { continue }
    $segs = $parts[1].Trim() -split '/'
    $entries += [pscustomobject]@{
        Name = $parts[0].Trim(); Rel = $parts[1].Trim()
        Domain = $segs[1]
        Sub = $segs[2]
        Cn = [IO.Path]::GetFileNameWithoutExtension($parts[1].Trim())
        Prose = $prose }
}
if ($Filter -ne '') {
    $entries = @($entries | Where-Object { $_.Domain -like "*$Filter*" -or $_.Sub -like "*$Filter*" -or $_.Name -like "*$Filter*" -or $_.Cn -like "*$Filter*" })
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
[void]$s.AppendLine('#set page(paper: "a4", flipped: true, margin: (x: 0.8cm, y: 1.0cm, top: 1.5cm), numbering: (..a) => text(size: 6pt, fill: luma(120), { let n = a.pos().at(0); let t = if a.pos().len() > 1 { a.pos().at(1) } else { none }; if t == none { str(n) } else { str(n) + " / " + str(t) } }))')
[void]$s.AppendLine('#set text(font: ("Source Sans Pro", "Noto Sans SC"), size: 7.5pt, lang: "zh", region: "cn", cjk-latin-spacing: auto)')
[void]$s.AppendLine('#set par(leading: 0.52em, spacing: 0.85em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
[void]$s.AppendLine('#show heading.where(level: 1): it => block(above: 2.4em, below: 1.1em, width: 100%)[#text(size: 13pt, weight: "bold", fill: rgb("#1f4e79"), it.body) #v(0.5em) #line(length: 100%, stroke: 1pt + rgb("#1f4e79"))]')
[void]$s.AppendLine('#show raw: set text(font: ("Consolas", "Noto Sans SC"), size: 6pt)')
[void]$s.AppendLine('#show raw.where(block: true): it => block(width: 100%, fill: luma(249), stroke: (left: 1.1pt + luma(150), top: 0.35pt + luma(215), right: 0.35pt + luma(215), bottom: 0.35pt + luma(215)), inset: (x: 5pt, y: 3.5pt), radius: (top-right: 2pt, bottom-right: 2pt), it)')
[void]$s.AppendLine('#show heading.where(level: 2): it => block(above: 2.1em, below: 0.9em, width: 100%)[#text(size: 10pt, weight: "bold", fill: rgb("#2e6da4"), it.body) #v(0.45em) #line(length: 100%, stroke: 0.7pt + luma(160))]')
[void]$s.AppendLine('#show heading.where(level: 3): it => block(above: 1.15em, below: 0.25em, text(weight: "bold", size: 7.4pt, it.body))')
[void]$s.AppendLine('#show outline: set text(size: 7pt)')
[void]$s.AppendLine('#show outline.entry: set par(leading: 0.5em)')
[void]$s.AppendLine('#show outline.entry.where(level: 1): it => block(above: 0.85em, below: 0.15em, text(weight: "bold", size: 8.5pt, fill: rgb("#1f4e79"), it))')
[void]$s.AppendLine('#show outline.entry.where(level: 2): it => { set text(size: 6.8pt); it }')
[void]$s.AppendLine('#show outline.entry.where(level: 3): it => { set text(size: 6.3pt, fill: luma(75)); it }')
[void]$s.AppendLine('#let pagehead = context {')
[void]$s.AppendLine('  let pg = here().page()')
[void]$s.AppendLine('  let doms = query(heading.where(level: 1).before(here()))')
[void]$s.AppendLine('  let dom = if doms.len() > 0 { doms.last().body } else { [--] }')
[void]$s.AppendLine('  let fams = query(heading.where(level: 2).before(here()))')
[void]$s.AppendLine('  let fam = if fams.len() > 0 { fams.last().body } else { [--] }')
[void]$s.AppendLine('  let ents = query(heading.where(level: 3).before(here())).filter(h => h.has(str(label)))')
[void]$s.AppendLine('  let flowing = if ents.len() > 0 { ents.last().body } else { "" }')
[void]$s.AppendLine('  grid(columns: (auto, 1fr, auto), text(size: 6pt, fill: luma(110))[#dom / #fam], align(center, text(size: 6pt, fill: luma(110), flowing)), text(size: 6pt, fill: luma(110))[zoi booklet])')
[void]$s.AppendLine('  v(0.3em)')
[void]$s.AppendLine('  line(length: 100%, stroke: 0.3pt + luma(205))')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#let colrule = [#place(line(start: (33.34%, 0%), end: (33.34%, 100%), stroke: 0.4pt + luma(210))) #place(line(start: (66.67%, 0%), end: (66.67%, 100%), stroke: 0.4pt + luma(210)))]')
[void]$s.AppendLine('')
[void]$s.AppendLine('#v(1fr)')
[void]$s.AppendLine('#align(center, text(size: 24pt, weight: "bold")[zoi Contest Booklet])')
[void]$s.AppendLine('#v(0.5em)')
[void]$s.AppendLine('#align(center, text(size: 7pt, fill: luma(100))[' + $blocks.Count + ' engines + ' + $plugBlocks.Count + ' algebra plugins - hash = SHA256 first 8 hex over LF-normalized text, includes rewritten to stub names. On-site check: count lines, then hash after LF save.])')
[void]$s.AppendLine('#v(1em)')
[void]$s.AppendLine('#align(center, line(length: 55%, stroke: 1.1pt + rgb("#1f4e79")))')
[void]$s.AppendLine('#v(1fr)')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#columns(3, gutter: 0.9cm)[#outline(title: none, depth: 3, indent: 0.8em)]')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#set page(columns: 3, margin: (x: 0.7cm, y: 0.95cm, top: 1.4cm), header: pagehead, foreground: colrule)')
[void]$s.AppendLine('')
# guard: (Domain, Sub) groups must be contiguous in catalog order, or the emit
# loop prints the same sub heading twice and the TOC lists it two times
$prevKey = ''
$seenSub = @{}
foreach ($b in $blocks) {
    $k = $b.Meta.Domain + '/' + $b.Meta.Sub
    if ($seenSub.ContainsKey($k) -and $prevKey -ne $k) { throw ('catalog: non-contiguous sub-domain: ' + $k) }
    $seenSub[$k] = $true
    $prevKey = $k
}
$lastDom = ''
$lastSub = ''
$pbOdd = '#pagebreak(to: "odd", weak: true)'   # big entries: jump to next odd page (physical sheet front)
$pbPage = '#pagebreak(weak: true)'             # one code block per page: every entry opens a fresh page
foreach ($b in $blocks) {
    $newDom = $b.Meta.Domain -ne $lastDom
    $newSub = $b.Meta.Sub -ne $lastSub
    $big = $b.Lines -ge $SoloMin
    $brk = if ($big) { $pbOdd } else { $pbPage }
    if ($newDom) {
        if ($lastDom -ne '') { [void]$s.AppendLine('') }
        [void]$s.AppendLine($brk)              # domain heading rides the fresh page of its first entry
        [void]$s.AppendLine('= ' + (Esc $b.Meta.Domain))
        $lastDom = $b.Meta.Domain
    }
    else { [void]$s.AppendLine($brk) }
    if ($newSub) {
        [void]$s.AppendLine('== ' + (Esc $b.Meta.Sub))
        $lastSub = $b.Meta.Sub
    }
    if ($b.Meta.Prose) {
        [void]$s.AppendLine('=== [' + (Esc $b.Meta.Cn) + ' #text(fill: luma(165), size: 0.72em)[待补]] <e-' + $b.Meta.Name + '>')
    }
    else {
        [void]$s.AppendLine('=== ' + (Esc $b.Meta.Cn) + ' <e-' + $b.Meta.Name + '>')
    }
    [void]$s.AppendLine('#align(right, text(size: 5.4pt, fill: luma(105))[' + $b.Lines + ' ln -- sha256 ' + $b.Hash + '])')
    if ($b.Meta.Prose) {
        [void]$s.AppendLine('#set par(justify: true)')
        [void]$s.AppendLine((Esc $b.Text))
        [void]$s.AppendLine('#set par(justify: false)')
    }
    else {
        [void]$s.AppendLine('```cpp')
        [void]$s.AppendLine($b.Text)
        [void]$s.AppendLine('```')
    }
    [void]$s.AppendLine('')
}
if ($plugBlocks.Count -gt 0) {
    $firstPlug = $true
    $pi = 0
    foreach ($b in $plugBlocks) {
        $brk = if ($b.Lines -ge $SoloMin) { $pbOdd } else { $pbPage }
        if ($firstPlug) { [void]$s.AppendLine($brk); [void]$s.AppendLine('== Appendix / algebra plugins'); $firstPlug = $false }
        else { [void]$s.AppendLine($brk) }
        [void]$s.AppendLine('=== ' + (Esc $b.Meta.Name) + ' <e-plug' + $pi + '>')
        [void]$s.AppendLine('#align(right, text(size: 5.4pt, fill: luma(105))[' + $b.Lines + ' ln -- sha256 ' + $b.Hash + '])')
        [void]$s.AppendLine('```cpp')
        [void]$s.AppendLine($b.Text)
        [void]$s.AppendLine('```')
        [void]$s.AppendLine('')
        $pi++
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

# ---- parity audit: big entries must start on odd pages (duplex sheet fronts) ----
# NOTE: `typst query` output carries no location on current toolchains; the old
#       JSON audit silently matched nothing (vacuous OK). eval() is the truth.
$prevEAP = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$expr = 'query(heading).filter(h=>h.has(str(label))).map(h=>(h.label,h.location().page()))'
$evalOut = & $typst eval $expr --in $typPath 2>$null
$ErrorActionPreference = $prevEAP
$viol = 0
$hits = 0
$bigHits = 0
$bigset = @()
foreach ($b in $blocks) { if ($b.Lines -ge $SoloMin) { $bigset += $b.Meta.Name } }
for ($pi = 0; $pi -lt $plugBlocks.Count; $pi++) { if ($plugBlocks[$pi].Lines -ge $SoloMin) { $bigset += 'plug' + $pi } }
foreach ($m in [regex]::Matches(($evalOut -join ' '), '"<(e-[a-zA-Z0-9]+)>",(\d+)')) {
    $hits++
    $nm = $m.Groups[1].Value.Substring(2)
    $pg = [int]$m.Groups[2].Value
    if ($bigset -contains $nm) {
        $bigHits++
        if ($pg % 2 -eq 0) {
            Write-Host ('[PARITY VIOLATION] ' + $nm + ' starts on even page ' + $pg) -ForegroundColor Red
            $viol++
        }
    }
}
if ($hits -eq 0) { Write-Host '[FAIL] parity audit matched no entry anchors (eval broken?)'; exit 1 }
if ($viol -eq 0) { Write-Host ('[OK] parity: ' + $bigHits + '/' + $bigset.Count + ' big entries start on odd pages (sheet fronts)') }
else { Write-Host ('[FAIL] parity violations: ' + $viol); exit 1 }
