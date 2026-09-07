param([string]$Filter = '', [string]$OutFile = 'docs/booklet/output/zoi-booklet-print.pdf', [int]$SoloMin = 0, [switch]$SourceOnly, [string]$TypstPath = '')
# make_booklet.ps1 - printable contest booklet generator (typst, A4 landscape, 3 columns)
# Directories define chapters; catalog order only ranks existing source families.
# A README beside a selected source follows the last selected sibling, once.
# Roadmaps live in docs/roadmaps and never enter this print pipeline.
# Usage: powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1              -> full
#        powershell -File scripts\make_booklet.ps1 -Filter seg -OutFile .zoi-checks/seg.pdf
# SoloMin: with a value > 0, real entries with >= SoloMin lines additionally start
#          on an odd page (= a physical sheet's front side) for duplex printing;
#          the default 0 starts each entry on a fresh page without parity padding.
# Pipeline: directory tree + catalog identities -> include rewrite (stub names)
#           -> SHA256[:8] over LF-normalized text -> booklet.typ -> typst compile.
# Requires: typst on PATH, or scripts\typst.exe next to this script (auto-detected).
# NOTE: keep this file ASCII-only (PS 5.1 reads no-BOM as ANSI). CJK text
#       (family names, entry titles) flows in from file paths at runtime.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$enc = New-Object System.Text.UTF8Encoding($false)
$plugName = -join ([char]0x63D2, [char]0x4EF6)   # plugin folder marker, CJK kept out of source bytes

$typst = if ($TypstPath) { [IO.Path]::GetFullPath($TypstPath) } else { (Get-Command typst -ErrorAction SilentlyContinue | Select-Object -First 1).Source }
if (-not $typst) {
    $local = Join-Path $PSScriptRoot 'typst.exe'
    if (Test-Path -LiteralPath $local) { $typst = $local }
    elseif (-not $SourceOnly) { throw 'typst not found: install to PATH or drop typst.exe into scripts\' }
}

. (Join-Path $PSScriptRoot 'booklet_markdown.ps1')
. (Join-Path $PSScriptRoot 'booklet_tree.ps1')

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
$allEntries = @($entries)
# basename -> stub map for local include rewrite
$stubByFile = @{}
foreach ($e in $allEntries) { if (-not $e.Prose) { $stubByFile[[IO.Path]::GetFullPath((Join-Path $root $e.Rel))] = $e.Name } }

# ---- plugin appendix (algebra layer copy sources; full solutions skipped) ----
$plugins = @()
foreach ($d in Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Directory) {
    if ($d.Name -notlike "*$plugName*") { continue }
    foreach ($f in Get-ChildItem $d.FullName -Recurse -Filter *.cpp) {
        # Ignore comments and quoted examples before looking for a real main.
        $source=[IO.File]::ReadAllText($f.FullName,$enc)
        $code=[regex]::Replace($source, '(?s)/\*.*?\*/|//[^\r\n]*|"(?:\\.|[^"\\])*"|''(?:\\.|[^''\\])*''', ' ')
        if ($code -match '\bint\s+main\s*\(') { continue }
        $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
        $plugins += [pscustomobject]@{ Name = $f.BaseName; Rel = $rel; Family = 'Appendix'; Cn = $f.BaseName }
    }
}

$plugins = @($plugins | Sort-Object Rel -Unique -Culture zh-CN)
$tree=Get-BookletTree $root $allEntries $plugins $Filter
$entries=@($allEntries | Where-Object { $tree.Files.ContainsKey($_.Rel) })
$plugins=@($plugins | Where-Object { $tree.Files.ContainsKey($_.Rel) })

# ---- per-entry transform: rewrite includes, LF-normalize, hash ----
$script:warn = @()
$script:stubMap = $stubByFile
$script:stubKeys = @($stubByFile.Keys)
function Convert-Entry($e) {
    $text = [IO.File]::ReadAllText((Join-Path $root ($e.Rel.Replace('\', '/'))), $enc)
    $ev = [System.Text.RegularExpressions.MatchEvaluator]{ param($m)
        $base = [IO.Path]::GetFullPath((Join-Path (Split-Path -Parent (Join-Path $root $e.Rel)) $m.Groups[2].Value))
        if ($script:stubKeys -contains $base) { $m.Groups[1].Value + $script:stubMap[$base] + '.h"' }
        else { $script:warn += ($e.Name + ': unresolved include ' + $base); $m.Value }
    }
    $rew = [regex]::Replace($text, '(?m)^(\s*#\s*include\s+")([^"]+\.cpp)"', $ev)
    $norm = ($rew -replace "`r`n", "`n").TrimEnd()
    # trim decorative '='/'-' banner lines to the column width before hashing:
    # a code column fits ~70 chars of 6pt Consolas inside the raw box inset,
    # prose ~60 of 7.5pt Noto; unbreakable runs past that spill into the next
    # column. Applied pre-hash so the printed fingerprint matches the paper.
    $cap = if ($e.Prose) { 60 } else { 70 }
    $trimDeco = [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $m.Groups[1].Value.Substring(0, $cap) }
    $norm = [regex]::Replace($norm, ('(?m)^(\s*[=\-]{' + ($cap + 1) + ',})\s*$'), $trimDeco)
    $sha = [System.Security.Cryptography.SHA256]::Create()
    $h = ($sha.ComputeHash($enc.GetBytes($norm)) | ForEach-Object { $_.ToString('x2') }) -join ''
    [pscustomobject]@{ Meta = $e; Text = $norm; Hash = $h.Substring(0, 8); Lines = ($norm -split "`n").Count }
}
$blocks = @()
foreach ($e in $entries) { $blocks += Convert-Entry $e }
$realCount = $blocks.Count
$plugBlocks = @()
foreach ($p in $plugins) { $plugBlocks += Convert-Entry $p }

# One manual per source directory. Never search ancestors or descendants.
$manualOwner = @{}
$manuals = @{}
foreach ($b in @($blocks) + @($plugBlocks)) {
    $rd = ($b.Meta.Rel -replace '/[^/]+$','') + '/README.md'
    if (Test-Path -LiteralPath (Join-Path $root $rd) -PathType Leaf) {
        $manualOwner[$rd] = $b.Meta.Rel
        $manuals[$rd] = [IO.File]::ReadAllText((Join-Path $root $rd), $enc)
    }
}
$printedManuals = @{}
function Append-Manual($Builder, $Entry) {
    $rd = ($Entry.Meta.Rel -replace '/[^/]+$','') + '/README.md'
    if ($manualOwner.ContainsKey($rd) -and $manualOwner[$rd] -eq $Entry.Meta.Rel) {
        [void]$Builder.AppendLine('// manual: ' + $rd)
        [void]$Builder.AppendLine('#metadata(' + (Typ-String $rd) + ') <manual-' + $printedManuals.Count + '>')
        [void]$Builder.AppendLine((Convert-BookletMarkdown $manuals[$rd]))
        $printedManuals[$rd] = $true
    }
}

# ---- assemble typst source ----
function Zh([string]$hex) { -join @($hex.Split(' ') | ForEach-Object { [char][Convert]::ToInt32($_,16) }) }
function Esc([string]$t) { return ($t -replace '([_\*\[\]#`$\\@<>])', '\$1') }   # typst markup escape
$fontList = if ($typst -and -not $SourceOnly) { @(& $typst fonts) } else { @('Source Sans Pro','Consolas','Noto Sans SC') }
function Font-Choice($Choices) {
    foreach ($f in $Choices) { if ($fontList -contains $f) { return $f } }
    throw ('Missing booklet font; install one of: ' + ($Choices -join ', '))
}
$cjkFont = Font-Choice @('Noto Sans SC','Noto Sans CJK SC','Microsoft YaHei')
$bodyFont = Font-Choice @('Source Sans Pro','DejaVu Sans')
$monoFont = Font-Choice @('Consolas','DejaVu Sans Mono')
$s = New-Object System.Text.StringBuilder
[void]$s.AppendLine('#let booklet-mono = (' + (Typ-String $monoFont) + ', ' + (Typ-String $cjkFont) + ')')
[void]$s.AppendLine('#set page(paper: "a4", flipped: true, margin: (x: 0.8cm, y: 1.0cm, top: 1.5cm), numbering: (..a) => text(size: 6pt, fill: luma(120), { let n = a.pos().at(0); let t = if a.pos().len() > 1 { a.pos().at(1) } else { none }; let label = if t == none { str(n) } else { str(n) + " / " + str(t) }; if n == 39 { context { if counter(page).get().first() == 39 { text(size: 6pt, fill: rgb("#68aaa3"), "MIKU \u{2661}") } else { label } } } else { label } }))')
[void]$s.AppendLine('#set text(font: (' + (Typ-String $bodyFont) + ', ' + (Typ-String $cjkFont) + '), size: 7.5pt, lang: "zh", region: "cn", cjk-latin-spacing: auto)')
[void]$s.AppendLine('#set par(leading: 0.52em, spacing: 0.85em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
# Depth controls all heading and contents styles; no deeper level falls back
# to Typst's larger default. Clamp only the font floor, never tree depth.
[void]$s.AppendLine('#let chapter-style(depth, contents: false) = {')
[void]$s.AppendLine('  let i = calc.min(depth, 5) - 1')
[void]$s.AppendLine('  if contents { (size: (8.5pt, 7.2pt, 6.8pt, 6.4pt, 6.2pt).at(i), weight: ("bold", "bold", "regular", "regular", "regular").at(i), ink: (rgb("#1f4e79"), luma(35), luma(55), luma(75), luma(85)).at(i)) }')
[void]$s.AppendLine('  else { (size: (13pt, 9.5pt, 8.2pt, 7.6pt, 7.2pt).at(i), weight: ("bold", "bold", "bold", "semibold", "regular").at(i), ink: (rgb("#1f4e79"), rgb("#243e52"), luma(45), luma(65), luma(80)).at(i)) }')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#show heading: it => {')
[void]$s.AppendLine('  let style = chapter-style(it.level)')
[void]$s.AppendLine('  block(sticky: true, above: if it.level == 1 { 7pt } else if it.level == 2 { 10pt } else { 5pt }, below: 3pt, width: 100%)[#text(size: style.size, weight: style.weight, fill: style.ink, it.body)#if it.level == 1 { v(5pt); line(length: 100%, stroke: 1pt + style.ink) }]')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#show raw: set text(font: booklet-mono, size: 6pt)')
[void]$s.AppendLine('#show raw.where(block: true): it => block(width: 100%, fill: none, stroke: (left: 1.1pt + luma(150), top: 0.35pt + luma(215), right: 0.35pt + luma(215), bottom: 0.35pt + luma(215)), inset: (x: 5pt, y: 3.5pt), radius: (top-right: 2pt, bottom-right: 2pt), it)')
[void]$s.AppendLine('#show outline.entry: it => {')
[void]$s.AppendLine('  let style = chapter-style(it.level, contents: true)')
[void]$s.AppendLine('  set text(size: style.size, weight: style.weight, fill: style.ink)')
[void]$s.AppendLine('  set par(leading: 0.45em)')
[void]$s.AppendLine('  block(above: if it.level == 1 { 6pt } else if it.level == 2 { 2pt } else { 0pt }, below: 1.8pt, inset: (top: 0.7pt, bottom: 0.7pt), it)')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#let pagehead = context {')
[void]$s.AppendLine('  let pg = here().page()')
[void]$s.AppendLine('  let doms = query(heading.where(level: 1)).filter(h => h.location().page() <= pg)')
[void]$s.AppendLine('  let dom = if doms.filter(h => h.location().page() == pg).len() > 0 { doms.filter(h => h.location().page() == pg).first().body } else if doms.len() > 0 { doms.last().body } else { [--] }')
[void]$s.AppendLine('  let ents = query(heading).filter(h => h.has(str(label)) and h.location().page() <= pg)')
[void]$s.AppendLine('  let local = ents.filter(h => h.location().page() == pg)')
[void]$s.AppendLine('  let sources = local.filter(h => str(h.label).starts-with("e-"))')
[void]$s.AppendLine('  let flowing = if sources.len() > 0 { sources.first().body } else if local.len() > 0 { local.first().body } else if ents.len() > 0 { ents.last().body } else { [--] }')
[void]$s.AppendLine('  grid(columns: (auto, 1fr, auto), box(fill: luma(239), inset: (x: 4pt, y: 1pt), text(size: 6pt, weight: "bold", fill: luma(60))[#dom]), align(center, text(size: 6pt, fill: luma(110), flowing)), text(size: 6pt, fill: luma(110))[zoi booklet])')
[void]$s.AppendLine('  v(0.3em)')
[void]$s.AppendLine('  line(length: 100%, stroke: 0.3pt + luma(205))')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#let colrule = [#place(line(start: (33.34%, 0%), end: (33.34%, 100%), stroke: 0.4pt + luma(210))) #place(line(start: (66.67%, 0%), end: (66.67%, 100%), stroke: 0.4pt + luma(210)))]')
[void]$s.AppendLine('')
$scopeText = if ($Filter) { (Zh '7b5b 9009') + ': ' + $Filter } else { (Zh '5168 5e93') }
$editionText = (Get-Date -Format 'yyyy-MM-dd') + ' / ' + $scopeText
[void]$s.AppendLine('#let quick(n, title, body) = block(width: 100%, height: 100pt, inset: 12pt, stroke: 0.5pt + luma(200), radius: 3pt)[#text(size: 18pt, fill: luma(175))[#n] #v(6pt) #text(size: 10pt, weight: "bold")[#title] #v(5pt) #text(size: 7.5pt, fill: luma(80))[#body]]')
[void]$s.AppendLine('#let entrymeta(b) = block(width: 100%, above: 2pt, below: 3pt, fill: luma(245), inset: (x: 4pt, y: 2pt), text(size: 5.4pt, fill: luma(75), b))')
[void]$s.AppendLine('#v(0.65fr)')
[void]$s.AppendLine('#align(center)[#block(width: 82%)[#text(size: 9pt, weight: "bold", fill: rgb("#1f4e79"))[HNIST / ICPC FIELD NOTES]')
[void]$s.AppendLine('#v(12pt)')
[void]$s.AppendLine('#text(size: 32pt, weight: "bold")[' + (Zh '7b97 6cd5 7ade 8d5b 624b 518c') + ']')
[void]$s.AppendLine('#v(7pt)')
[void]$s.AppendLine('#text(size: 9pt, fill: luma(90))[' + (Zh '5148 5b9a 4f4d 2c 20 518d 6838 5bf9 63a5 53e3 2c 20 6700 540e 7ec4 88c5') + ']')
[void]$s.AppendLine('#v(16pt) #line(length: 100%, stroke: 1.3pt + rgb("#1f4e79")) #v(12pt)')
[void]$s.AppendLine('#text(size: 8pt, fill: luma(85))[' + (Esc $editionText) + ' | ' + $realCount + ' ' + (Zh '4e2a 767b 8bb0 6761 76ee') + ' | ' + $plugBlocks.Count + ' ' + (Zh '4e2a 4ee3 6570 63d2 4ef6') + ']')
[void]$s.AppendLine('#v(20pt)')
[void]$s.AppendLine('#grid(columns: (1fr, 1fr, 1fr), gutter: 12pt, quick([01], [' + (Zh '5b9a 4f4d 6a21 677f') + '], [' + (Zh '76ee 5f55 67e5 9875 7801 2c 20 9875 7709 770b 65b9 5411 4e0e 6761 76ee 3b 20 4fe1 606f 6761 7ed9 51fa 8df3 677f 77ed 540d') + ']), quick([02], [' + (Zh '6838 5bf9 5951 7ea6') + '], [' + (Zh '5148 770b 6784 9020 5bb9 91cf 3001 7f16 53f7 548c 8fd4 56de 503c 3b 20 591a 6d4b 7559 610f 20 69 6e 69 74 20 2f 20 63 6c 65 61 72 20 7684 8c03 7528 987a 5e8f') + ']), quick([03], [' + (Zh '7ec4 88c5 4f7f 7528') + '], [' + (Zh '6309 20 55 73 61 67 65 20 63a5 5165 9898 89e3 3b 20 8de8 6a21 677f 4f20 56fe 65f6 6838 5bf9 70b9 6570 3001 65b9 5411 548c 6743 503c') + ']))')
[void]$s.AppendLine('#v(16pt)')
[void]$s.AppendLine('#text(size: 7pt, fill: luma(100))[' + (Zh '6253 5370 524d 91cd 65b0 751f 6210 6574 672c 2c 20 76ee 5f55 4e0e 9875 7801 4f1a 968f 4ee3 7801 957f 5ea6 53d8 5316 2e 20 5f85 8865 6761 76ee 4e0d 662f 53ef 7528 6a21 677f 2e 20 4fe1 606f 6761 6307 7eb9 5bf9 5e94 624b 518c 4e2d 7684 4ee3 7801 2c 20 4e0d 4ee3 8868 5bf9 62cd 8bc4 7ea7') + ']')
[void]$s.AppendLine(']]')
[void]$s.AppendLine('#v(1fr)')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#columns(3, gutter: 0.9cm)[#outline(title: none, indent: 6pt)]')
[void]$s.AppendLine((Zh '23 76 28 31 32 70 74 29 a 23 62 6c 6f 63 6b 28 77 69 64 74 68 3a 20 31 30 30 25 2c 20 62 72 65 61 6b 61 62 6c 65 3a 20 66 61 6c 73 65 2c 20 69 6e 73 65 74 3a 20 31 32 70 74 2c 20 73 74 72 6f 6b 65 3a 20 30 2e 35 70 74 20 2b 20 6c 75 6d 61 28 32 30 30 29 2c 20 72 61 64 69 75 73 3a 20 33 70 74 29 5b a 23 73 65 74 20 74 65 78 74 28 73 69 7a 65 3a 20 39 70 74 29 a 23 74 65 78 74 28 73 69 7a 65 3a 20 31 32 70 74 2c 20 77 65 69 67 68 74 3a 20 22 62 6f 6c 64 22 29 5b 6253 5370 8bbe 7f6e 5d a 23 76 28 36 70 74 29 a 23 67 72 69 64 28 63 6f 6c 75 6d 6e 73 3a 20 28 31 66 72 2c 20 31 66 72 29 2c 20 67 75 74 74 65 72 3a 20 38 70 74 2c a 5b 7eb8 5f20 3a 20 41 34 20 2f 20 6a2a 5411 5d 2c 20 5b 7f29 653e 3a 20 5b9e 9645 5927 5c0f 20 2f 20 31 30 30 25 5d 2c a 5b 6bcf 5f20 7eb8 3a 20 31 20 9875 20 50 44 46 2c 20 9875 9762 672c 8eab 5df2 6709 4e09 680f 5d 2c 20 5b 53cc 9762 3a 20 77ed 8fb9 7ffb 8f6c 5d 2c a 5b 989c 8272 3a 20 9ed1 767d 5373 53ef 2c 20 4fdd 7559 7070 5ea6 5d 2c 20 5b 6b63 6587 3a 20 36 20 70 74 2c 20 5efa 8bae 5148 8bd5 5370 4e00 9875 4ee3 7801 5d 29 a 23 76 28 36 70 74 29 a 23 74 65 78 74 28 73 69 7a 65 3a 20 38 70 74 2c 20 66 69 6c 6c 3a 20 6c 75 6d 61 28 38 30 29 29 5b 5148 786e 8ba4 4ee3 7801 548c 7070 8272 6ce8 91ca 6e05 6670 3001 9875 8fb9 672a 88c1 5207 2c 20 518d 6253 5370 6574 672c 3b 20 82e5 6253 5370 673a 63d0 793a 8d85 51fa 53ef 6253 5370 533a 57df 2c 20 5148 68c0 67e5 9884 89c8 2c 20 5fc5 8981 65f6 9002 914d 53ef 6253 5370 533a 57df 5e76 518d 6b21 8bd5 5370 5d a 5d a'))
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#set page(columns: 3, margin: (x: 0.7cm, y: 0.95cm, top: 1.4cm), header: pagehead, foreground: colrule)')
[void]$s.AppendLine('')
$blockByPath=@{}
foreach ($b in @($blocks)+@($plugBlocks)) { $blockByPath[$b.Meta.Rel]=$b }
$pluginLabels=@{}
for ($pi=0; $pi -lt $plugBlocks.Count; $pi++) { $pluginLabels[$plugBlocks[$pi].Meta.Rel]='e-plug'+$pi }
$script:printedDirs=@{}
$script:pendingHeadings=New-Object Text.StringBuilder
$script:newDomain=$false
$script:lastBlockKind=''
[void]$s.AppendLine('// directory-scope: '+$(if ($Filter) { 'filtered' } else { 'all' }))
foreach ($node in $tree.Roots) { Write-BookletNode $node $s $blockByPath $pluginLabels $SoloMin }
if ($script:pendingHeadings.Length) { throw 'Unflushed directory headings' }
$outputPath = if ([IO.Path]::IsPathRooted($OutFile)) { [IO.Path]::GetFullPath($OutFile) } else { [IO.Path]::GetFullPath((Join-Path $root $OutFile)) }
if ([IO.Path]::GetExtension($outputPath) -ne '.pdf') { throw 'OutFile must use the .pdf extension' }
$canonical = [IO.Path]::GetFullPath((Join-Path $root 'docs/booklet/output/zoi-booklet-print.pdf'))
if (($Filter -or $SourceOnly) -and $outputPath -eq $canonical) { throw 'A preview requires -OutFile in the workspace scratch directory' }
[void][IO.Directory]::CreateDirectory((Split-Path -Parent $outputPath))
$typPath = [IO.Path]::ChangeExtension($outputPath, '.typ')
if ($typPath -eq $outputPath) { throw 'OutFile must not use the .typ extension' }
[IO.File]::WriteAllText($typPath, $s.ToString(), $enc)

if ($SourceOnly) { Write-Host ('[OK] source: ' + $entries.Count + ' entries, ' + $printedManuals.Count + ' manuals -> ' + $typPath); return }

# ---- compile ----
Push-Location $root
try { & $typst compile $typPath $outputPath; if ($LASTEXITCODE -ne 0) { throw 'typst compile failed' } }
finally { Pop-Location }

Write-Host ('[OK] booklet: ' + $realCount + ' catalog entries + ' + $printedManuals.Count + ' manuals, ' + $plugBlocks.Count + ' plugins -> ' + $outputPath)
foreach ($w in $script:warn) { Write-Host ('[WARN] ' + $w) -ForegroundColor Yellow }

# ---- anchor eval: one query feeds both audits ----
# NOTE: `typst query` output carries no location on current toolchains; the old
#       JSON audit silently matched nothing (vacuous OK). eval() is the truth.
function Invoke-BookletEval([string]$Expression) {
    # Typst writes UTF-8 even when PowerShell is launched with redirected pipes.
    $previousEncoding=[Console]::OutputEncoding
    try {
        [Console]::OutputEncoding=$enc
        & $typst eval $Expression --in $typPath
        if ($LASTEXITCODE -ne 0) { throw 'Typst audit query failed' }
    }
    finally { [Console]::OutputEncoding=$previousEncoding }
}
$expr = 'query(heading).filter(h=>h.outlined and h.has(str(label))).map(h=>(h.label,h.location().page()))'
$evalOut = Invoke-BookletEval $expr

# Every selected entry and every discovered manual must survive typesetting.
$expected = @($blocks | ForEach-Object { 'e-' + $_.Meta.Name })
for ($pi=0; $pi -lt $plugBlocks.Count; $pi++) { $expected += 'e-plug' + $pi }
foreach ($anchor in $expected) {
    if (([regex]::Matches(($evalOut -join ' '), ('"<' + [regex]::Escape($anchor) + '>"'))).Count -ne 1) { throw ('Missing or repeated entry anchor: ' + $anchor) }
}
$startPages=@{}
foreach ($m in [regex]::Matches(($evalOut -join ' '), '"<(e-[a-zA-Z0-9]+)>",(\d+)')) { $startPages[$m.Groups[1].Value]=[int]$m.Groups[2].Value }
$endEval=Invoke-BookletEval 'query(metadata).filter(m=>type(m.value)==str and m.value.starts-with("entry-end:")).map(m=>(m.value,m.location().page()))'
if ($LASTEXITCODE -ne 0) { throw 'Entry reserve audit query failed' }
$ends=[regex]::Matches(($endEval -join ' '), '"entry-end:(e-[a-zA-Z0-9]+)",(\d+)')
if ($ends.Count -ne $expected.Count) { throw 'Entry reserve audit missed an end marker' }
$previousEnd=0
foreach ($m in $ends) {
    $anchor=$m.Groups[1].Value; $end=[int]$m.Groups[2].Value
    if (-not $startPages.ContainsKey($anchor) -or $startPages[$anchor] -le $previousEnd -or $end -lt $startPages[$anchor]) { throw ('Entry does not own its pages: '+$anchor) }
    $previousEnd=$end
}
Write-Host ('[OK] reserve: '+$ends.Count+' entries own separate page ranges including manuals')
if ($printedManuals.Count -ne $manuals.Count) { throw 'Manual discovery/emission mismatch' }
$manualQuery = 'query(metadata).map(m=>m.value)'
$manualEval = Invoke-BookletEval $manualQuery
if ($LASTEXITCODE -ne 0) { throw 'Manual audit query failed' }
foreach ($rd in $manuals.Keys) {
    if (([regex]::Matches(($manualEval -join ' '), [regex]::Escape((Typ-String $rd)))).Count -ne 1) { throw ('Missing or repeated manual: ' + $rd) }
}
Write-Host ('[OK] coverage: ' + $expected.Count + ' entries, ' + $manuals.Count + ' adjacent manuals; roadmap prose excluded')
foreach ($dir in @($tree.Nodes.Values | Where-Object { $_.Selected })) {
    if (-not $script:printedDirs.ContainsKey($dir.Rel)) { throw ('Directory was not emitted: '+$dir.Rel) }
    if (([regex]::Matches(($manualEval -join ' '),[regex]::Escape((Typ-String ('directory:'+$dir.Rel))))).Count -ne 1) { throw ('Directory coverage mismatch: '+$dir.Rel) }
}
Write-Host ('[OK] directories: '+$script:printedDirs.Count+' real algorithm directories')
$mathExpected=[regex]::Matches($s.ToString(),[regex]::Escape('#metadata("booklet-math")')).Count
$mathActual=Invoke-BookletEval 'query(math.equation).len()'
if ($LASTEXITCODE -ne 0 -or [int]($mathActual -join '') -ne $mathExpected) { throw 'Math equation coverage mismatch' }
Write-Host ('[OK] math: '+$mathExpected+' equations typeset')

# ---- parity audit: only meaningful when SoloMin>0 (duplex sheet fronts) ----
if ($SoloMin -le 0) {
    if (($evalOut -join ' ') -notmatch '(e-|dir-)') { Write-Host '[FAIL] anchor eval matched nothing (eval broken?)'; exit 1 }
    Write-Host '[OK] parity: skipped (SoloMin=0, fresh entry pages, no parity filler)'
}
else {
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
if ($bigHits -ne $bigset.Count) { throw 'Parity audit missed expected big-entry anchors' }
if ($expected.Count -gt 0 -and $hits -eq 0) { Write-Host '[FAIL] parity audit matched no entry anchors (eval broken?)'; exit 1 }
if ($viol -eq 0) { Write-Host ('[OK] parity: ' + $bigHits + '/' + $bigset.Count + ' big entries start on odd pages (sheet fronts)') }
else { Write-Host ('[FAIL] parity violations: ' + $viol); exit 1 }
}
