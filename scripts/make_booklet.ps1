param([string]$Filter = '', [string]$OutFile = 'docs/booklet/output/zoi-booklet-print.pdf', [int]$SoloMin = 0)
# make_booklet.ps1 - printable contest booklet generator (typst, A4 landscape, 3 columns)
# Layout: every catalog entry (code or ^ prose) opens a fresh page; skeleton entries
#         auto-scanned from library folders flow compactly on shared pages.
#         Page header center shows the entry flowing on that page. Catalog lines
#         starting with '^' are prose entries (theorems/notes, no code, no stub).
#         TOC is two-level (domains + entries); sub-domains render as separators.
#         Folder coverage audit: every knowledge-point folder must surface in the
#         booklet (own entry, catalog file, or group README note) or the build
#         exits 1 -- no silent missing folders.
# Usage: powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1              -> full
#        powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1 -Filter seg  -> scoped (audit skipped)
# SoloMin: with a value > 0, real entries with >= SoloMin lines additionally start
#          on an odd page (= a physical sheet's front side) for duplex printing;
#          the default 0 keeps one continuous sequence with no fully blank pages.
# Pipeline: catalog (order = section order) -> include rewrite (stub names)
#           -> SHA256[:8] over LF-normalized text -> booklet.typ -> typst compile.
# Requires: typst on PATH, or scripts\typst.exe next to this script (auto-detected).
# NOTE: keep this file ASCII-only (PS 5.1 reads no-BOM as ANSI). CJK text
#       (family names, entry titles) flows in from file paths at runtime.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$enc = New-Object System.Text.UTF8Encoding($false)
$plugName = -join ([char]0x63D2, [char]0x4EF6)   # plugin folder marker, CJK kept out of source bytes

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
$allEntries = @($entries)
if ($Filter -ne '') {
    $entries = @($entries | Where-Object { $_.Domain -like "*$Filter*" -or $_.Sub -like "*$Filter*" -or $_.Name -like "*$Filter*" -or $_.Cn -like "*$Filter*" })
}
if ($entries.Count -eq 0) { throw 'no catalog entries matched' }

# basename -> stub map for local include rewrite
$stubByFile = @{}
foreach ($e in $allEntries) { if (-not $e.Prose) { $stubByFile[[IO.Path]::GetFullPath((Join-Path $root $e.Rel))] = $e.Name } }

# ---- library walk: every knowledge-point folder must surface in the booklet ----
# knowledge folder = depth >= 2 dir whose path avoids the aux layers (duipai /
# liti / plugin markers, spelled in codepoints to keep this file ASCII-only).
# Represented folders already carry catalog files; missing LEAVES become auto
# entries (README as prose body, else a placeholder title); unrepresented
# groups lend their README to the sub separator as an intro note.
$algDir = Join-Path $root 'algorithms'
$auxDp = -join ([char]0x5BF9, [char]0x62CD)   # dui-pai harness layer
$auxLt = -join ([char]0x4F8B, [char]0x9898)   # li-ti solution layer
$kDirs = @{}
foreach ($d in Get-ChildItem $algDir -Recurse -Directory) {
    $rel = $d.FullName.Substring($algDir.Length + 1).Replace('\', '/')
    if ($rel -notlike '*/*') { continue }
    $aux = $false
    foreach ($sg in ($rel -split '/')) { if ($sg -eq $auxDp -or $sg -eq $auxLt -or $sg.Contains($plugName)) { $aux = $true; break } }
    if ($aux) { continue }
    $kDirs[$rel] = $true
}
$kRep = @{}
foreach ($rel in $kDirs.Keys) {
    $pre = 'algorithms/' + $rel + '/'
    foreach ($e2 in $entries) { if ($e2.Rel.StartsWith($pre)) { $kRep[$rel] = $true; break } }
}
$kKids = @{}
foreach ($rel in $kDirs.Keys) {
    $n = 0
    foreach ($r2 in $kDirs.Keys) { if ($r2 -ne $rel -and $r2.StartsWith($rel + '/')) { $n++ } }
    $kKids[$rel] = $n
}
$autoRaw = @()
$subIntro = @{}
$subIntroRaw = @()
foreach ($rel in $kDirs.Keys) {
    if ($kRep.ContainsKey($rel)) { continue }
    $segs = $rel -split '/'
    $rdLocal = Join-Path $algDir ($rel.Replace('/', '\') + '\README.md')
    $rdThere = Test-Path -LiteralPath $rdLocal
    if ($kKids[$rel] -gt 0) {
        if ($rdThere) { $subIntroRaw += [pscustomobject]@{ Key = $segs[0] + '/' + $segs[1]; Rel = $rel; Text = [IO.File]::ReadAllText($rdLocal, $enc) } }
        continue
    }
    $cn = $segs[$segs.Count - 1]
    if ($segs.Count -gt 3) {
        $mid = $segs[2..($segs.Count - 2)] -join ' '
        if ($mid -ne $cn) { $cn = $mid + ' ' + $cn }
    }
    $autoRaw += [pscustomobject]@{ Domain = $segs[0]; Sub = $segs[1]; Cn = $cn; Rel = ('algorithms/' + $rel + '/README.md'); HasReadme = $rdThere }
}
# group READMEs sharing one sub (e.g. a group nested under another group) are
# concatenated in path order -- never overwrite each other, never depend on
# dictionary enumeration order
foreach ($grp in ($subIntroRaw | Sort-Object Key, Rel | Group-Object Key)) {
    $subIntro[$grp.Name] = (($grp.Group | ForEach-Object { $_.Text.Trim() }) -join "`n`n")
}
if ($Filter -ne '') {
    $autoRaw = @($autoRaw | Where-Object { $_.Domain -like "*$Filter*" -or $_.Sub -like "*$Filter*" -or $_.Cn -like "*$Filter*" })
}
$autoRaw = @($autoRaw | Sort-Object Domain, Sub, Cn)

# ---- plugin appendix (algebra layer copy sources; full solutions skipped) ----
$plugins = @()
foreach ($d in Get-ChildItem (Join-Path $root 'algorithms') -Recurse -Directory) {
    if ($d.Name -notlike "*$plugName*") { continue }
    foreach ($f in Get-ChildItem $d.FullName -Recurse -Filter *.cpp) {
        if (Select-String -Path $f.FullName -Pattern 'int\s+main' -Quiet) { continue }
        $rel = $f.FullName.Substring($root.Length + 1).Replace('\', '/')
        $plugins += [pscustomobject]@{ Name = $f.BaseName; Rel = $rel; Family = 'Appendix'; Cn = $f.BaseName }
    }
}

$plugins = @($plugins | Sort-Object Rel -Culture zh-CN)

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

# ---- auto entries: library skeleton -> booklet. The stable merge keeps the
# (Domain,Sub) contiguity the emit loop relies on: first-appearance ranks for
# known subs/domains, unknowns ranked after them, catalog blocks keep order,
# autos attach after their own group.
$autoBlocks = @()
$gId = 0
foreach ($a in $autoRaw) {
    $gId++
    $nm = 'g' + $gId.ToString('000')
    if ($a.HasReadme) {
        $ab = Convert-Entry ([pscustomobject]@{ Name = $nm; Rel = $a.Rel; Domain = $a.Domain; Sub = $a.Sub; Cn = $a.Cn; Prose = $true })
        $autoBlocks += [pscustomobject]@{ Meta = $ab.Meta; Text = $ab.Text; Hash = $ab.Hash; Lines = $ab.Lines; Auto = $true }
    }
    else {
        $autoBlocks += [pscustomobject]@{ Meta = [pscustomobject]@{ Name = $nm; Rel = $a.Rel; Domain = $a.Domain; Sub = $a.Sub; Cn = $a.Cn; Prose = $true }; Text = ''; Hash = ''; Lines = 0; Auto = $true }
    }
}
$domRank = @{}
$subRank = @{}
foreach ($b in $blocks) {
    if (!$domRank.ContainsKey($b.Meta.Domain)) { $domRank[$b.Meta.Domain] = $domRank.Count }
    $sk2 = $b.Meta.Domain + '/' + $b.Meta.Sub
    if (!$subRank.ContainsKey($sk2)) { $subRank[$sk2] = $subRank.Count }
}
foreach ($ab in $autoBlocks) {
    if (!$domRank.ContainsKey($ab.Meta.Domain)) { $domRank[$ab.Meta.Domain] = 1000 + $domRank.Count }
    $sk2 = $ab.Meta.Domain + '/' + $ab.Meta.Sub
    if (!$subRank.ContainsKey($sk2)) { $subRank[$sk2] = 1000 + $subRank.Count }
}
$ordered = @()
$i2 = 0
foreach ($b in $blocks) { $ordered += [pscustomobject]@{ D = $domRank[$b.Meta.Domain]; S = $subRank[$b.Meta.Domain + '/' + $b.Meta.Sub]; T = 0; I = $i2; B = $b }; $i2++ }
$j2 = 5000
foreach ($ab in $autoBlocks) { $ordered += [pscustomobject]@{ D = $domRank[$ab.Meta.Domain]; S = $subRank[$ab.Meta.Domain + '/' + $ab.Meta.Sub]; T = $j2; I = $i2; B = $ab }; $i2++; $j2++ }
# multi-key sort: domain rank, then sub rank, then catalog-before-auto tie.
# (a packed numeric key would overflow between rank fields once auto-only
# subs get 1000+ sub ranks)
$blocks = @($ordered | Sort-Object D, S, T, I | ForEach-Object { $_.B })

# ---- assemble typst source ----
function Zh([string]$hex) { -join @($hex.Split(' ') | ForEach-Object { [char][Convert]::ToInt32($_,16) }) }
function Esc([string]$t) { return ($t -replace '([_\*\[\]#`$\\@<>])', '\$1') }   # typst markup escape
$s = New-Object System.Text.StringBuilder
[void]$s.AppendLine('#set page(paper: "a4", flipped: true, margin: (x: 0.8cm, y: 1.0cm, top: 1.5cm), numbering: (..a) => text(size: 6pt, fill: luma(120), { let n = a.pos().at(0); let t = if a.pos().len() > 1 { a.pos().at(1) } else { none }; let label = if t == none { str(n) } else { str(n) + " / " + str(t) }; if n == 39 { context { if counter(page).get().first() == 39 { text(size: 6pt, fill: rgb("#68aaa3"), "MIKU \u{2661}") } else { label } } } else { label } }))')
[void]$s.AppendLine('#set text(font: ("Source Sans Pro", "Noto Sans SC"), size: 7.5pt, lang: "zh", region: "cn", cjk-latin-spacing: auto)')
[void]$s.AppendLine('#set par(leading: 0.52em, spacing: 0.85em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
[void]$s.AppendLine('#show heading.where(level: 1): it => block(above: 0.5em, below: 0.65em, width: 100%)[#text(size: 13pt, weight: "bold", fill: rgb("#1f4e79"), it.body) #v(0.5em) #line(length: 100%, stroke: 1pt + rgb("#1f4e79"))]')
[void]$s.AppendLine('#show raw: set text(font: ("Consolas", "Noto Sans SC"), size: 6pt)')
[void]$s.AppendLine('#show raw.where(block: true): it => block(width: 100%, fill: none, stroke: (left: 1.1pt + luma(150), top: 0.35pt + luma(215), right: 0.35pt + luma(215), bottom: 0.35pt + luma(215)), inset: (x: 5pt, y: 3.5pt), radius: (top-right: 2pt, bottom-right: 2pt), it)')
[void]$s.AppendLine('#show heading.where(level: 2): it => block(above: 1.3em, below: 0.3em, text(weight: "bold", size: 7.8pt, fill: luma(25), it.body))')
[void]$s.AppendLine('#let subsep(b) = block(above: 1.0em, below: 0.45em, width: 100%)[#text(size: 9.5pt, weight: "bold", fill: rgb("#2e6da4"), b) #v(0.4em) #line(length: 100%, stroke: 0.7pt + luma(165))]')
[void]$s.AppendLine('#let subintro(b) = block(above: 0em, below: 0.8em, width: 100%, fill: luma(246), inset: (x: 4.5pt, y: 3.5pt), radius: 2pt)[#text(size: 6.2pt, fill: luma(105), b)]')
[void]$s.AppendLine('#show outline: set text(size: 7pt)')
[void]$s.AppendLine('#show outline.entry: set par(leading: 0.5em)')
[void]$s.AppendLine('#show outline.entry.where(level: 1): it => block(above: 0.85em, below: 0.15em, text(weight: "bold", size: 8.5pt, fill: rgb("#1f4e79"), it))')
[void]$s.AppendLine('#show outline.entry.where(level: 2): it => { set text(size: 6.4pt, fill: luma(75)); it }')
[void]$s.AppendLine('#let pagehead = context {')
[void]$s.AppendLine('  let pg = here().page()')
[void]$s.AppendLine('  let doms = query(heading.where(level: 1)).filter(h => h.location().page() <= pg)')
[void]$s.AppendLine('  let dom = if doms.filter(h => h.location().page() == pg).len() > 0 { doms.filter(h => h.location().page() == pg).first().body } else if doms.len() > 0 { doms.last().body } else { [--] }')
[void]$s.AppendLine('  let ents = query(heading).filter(h => h.has(str(label)) and h.location().page() <= pg)')
[void]$s.AppendLine('  let flowing = if ents.filter(h => h.location().page() == pg).len() > 0 { ents.filter(h => h.location().page() == pg).first().body } else if ents.len() > 0 { ents.last().body } else { [--] }')
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
[void]$s.AppendLine('#columns(3, gutter: 0.9cm)[#outline(title: none, depth: 2, indent: 0.6em)]')
[void]$s.AppendLine((Zh '23 76 28 31 32 70 74 29 a 23 62 6c 6f 63 6b 28 77 69 64 74 68 3a 20 31 30 30 25 2c 20 62 72 65 61 6b 61 62 6c 65 3a 20 66 61 6c 73 65 2c 20 69 6e 73 65 74 3a 20 31 32 70 74 2c 20 73 74 72 6f 6b 65 3a 20 30 2e 35 70 74 20 2b 20 6c 75 6d 61 28 32 30 30 29 2c 20 72 61 64 69 75 73 3a 20 33 70 74 29 5b a 23 73 65 74 20 74 65 78 74 28 73 69 7a 65 3a 20 39 70 74 29 a 23 74 65 78 74 28 73 69 7a 65 3a 20 31 32 70 74 2c 20 77 65 69 67 68 74 3a 20 22 62 6f 6c 64 22 29 5b 6253 5370 8bbe 7f6e 5d a 23 76 28 36 70 74 29 a 23 67 72 69 64 28 63 6f 6c 75 6d 6e 73 3a 20 28 31 66 72 2c 20 31 66 72 29 2c 20 67 75 74 74 65 72 3a 20 38 70 74 2c a 5b 7eb8 5f20 3a 20 41 34 20 2f 20 6a2a 5411 5d 2c 20 5b 7f29 653e 3a 20 5b9e 9645 5927 5c0f 20 2f 20 31 30 30 25 5d 2c a 5b 6bcf 5f20 7eb8 3a 20 31 20 9875 20 50 44 46 2c 20 9875 9762 672c 8eab 5df2 6709 4e09 680f 5d 2c 20 5b 53cc 9762 3a 20 77ed 8fb9 7ffb 8f6c 5d 2c a 5b 989c 8272 3a 20 9ed1 767d 5373 53ef 2c 20 4fdd 7559 7070 5ea6 5d 2c 20 5b 6b63 6587 3a 20 36 20 70 74 2c 20 5efa 8bae 5148 8bd5 5370 4e00 9875 4ee3 7801 5d 29 a 23 76 28 36 70 74 29 a 23 74 65 78 74 28 73 69 7a 65 3a 20 38 70 74 2c 20 66 69 6c 6c 3a 20 6c 75 6d 61 28 38 30 29 29 5b 5148 786e 8ba4 4ee3 7801 548c 7070 8272 6ce8 91ca 6e05 6670 3001 9875 8fb9 672a 88c1 5207 2c 20 518d 6253 5370 6574 672c 3b 20 82e5 6253 5370 673a 63d0 793a 8d85 51fa 53ef 6253 5370 533a 57df 2c 20 5148 68c0 67e5 9884 89c8 2c 20 5fc5 8981 65f6 9002 914d 53ef 6253 5370 533a 57df 5e76 518d 6b21 8bd5 5370 5d a 5d a'))
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
$pbPage = '#pagebreak(weak: true)'             # every real entry opens a fresh page
$pbOdd = '#pagebreak(to: "odd", weak: true)'   # SoloMin>0: big entries jump to a sheet front
$pbDom = if ($SoloMin -gt 0) { $pbOdd } else { $pbPage }  # domain openers always take a fresh page
$tobu = -join ([char]0x5F85, [char]0x8865)     # 'to be filled' tag; codepoints keep this file ASCII-only
$prevAuto = $false
foreach ($b in $blocks) {
    $newDom = $b.Meta.Domain -ne $lastDom
    $newSub = $newDom -or ($b.Meta.Domain + '/' + $b.Meta.Sub -ne ($lastDom + '/' + $lastSub))
    if ($newDom) {
        [void]$s.AppendLine($pbDom)            # domain heading always opens a fresh page
        [void]$s.AppendLine('= ' + (Esc $b.Meta.Domain))
        $lastDom = $b.Meta.Domain
    }
    elseif (-not $b.Auto) { [void]$s.AppendLine($(if ($SoloMin -gt 0 -and $b.Lines -ge $SoloMin) { $pbOdd } else { $pbPage })) }   # real entries: one page each
    elseif (-not $prevAuto) { [void]$s.AppendLine($pbPage) } # skeleton runs start on a fresh page too
    if ($newSub) {
        [void]$s.AppendLine('#subsep[' + (Esc $b.Meta.Sub) + ']')
        $ikey = $b.Meta.Domain + '/' + $b.Meta.Sub
        if ($subIntro.ContainsKey($ikey)) { [void]$s.AppendLine('#subintro[' + (Esc ($subIntro[$ikey].Trim())) + ']') }
        $lastSub = $b.Meta.Sub
    }
    $tag = ''
    if ($b.Meta.Prose -and ($b.Lines -eq 0 -or $b.Text -match ('^' + (Zh '5360 4f4d') + '[:\uFF1A]') -or $b.Text.Contains((Zh '9aa8 67b6 7a7a 76ee 5f55')))) {
        $tag = ' #text(fill: luma(165), size: 0.72em)[' + $tobu + ']'   # catalog prose + empty shells carry the marker
    }
    [void]$s.AppendLine('== ' + (Esc $b.Meta.Cn) + $tag + ' <e-' + $b.Meta.Name + '>')
    if ($b.Lines -gt 0) {
        [void]$s.AppendLine('#entrymeta[' + $(if (-not $b.Meta.Prose -and $b.Meta.PSObject.Properties['Domain']) { (Esc ($b.Meta.Name + '.h')) + ' | ' } else { '' }) + $b.Lines + (Zh '20 884c 20 7c 20 53 48 41 32 35 36 20')  + $b.Hash + ']')
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
    }
    [void]$s.AppendLine('')
    $prevAuto = [bool]$b.Auto
}
if ($plugBlocks.Count -gt 0) {
    $firstPlug = $true
    $pi = 0
    foreach ($b in $plugBlocks) {
        $brk = if ($SoloMin -gt 0 -and $b.Lines -ge $SoloMin) { $pbOdd } else { $pbPage }
        if ($firstPlug) { [void]$s.AppendLine($brk); [void]$s.AppendLine('= ' + (Zh '9644 5f55 ff1a 4ee3 6570 63d2 4ef6')); $firstPlug = $false }
        else { [void]$s.AppendLine($brk) }
        [void]$s.AppendLine('=== ' + (Esc $b.Meta.Name) + ' <e-plug' + $pi + '>')
        [void]$s.AppendLine('#entrymeta[' + $(if (-not $b.Meta.Prose -and $b.Meta.PSObject.Properties['Domain']) { (Esc ($b.Meta.Name + '.h')) + ' | ' } else { '' }) + $b.Lines + (Zh '20 884c 20 7c 20 53 48 41 32 35 36 20')  + $b.Hash + ']')
        [void]$s.AppendLine('```cpp')
        [void]$s.AppendLine($b.Text)
        [void]$s.AppendLine('```')
        [void]$s.AppendLine('')
        $pi++
    }
}
$outputPath = if ([IO.Path]::IsPathRooted($OutFile)) { [IO.Path]::GetFullPath($OutFile) } else { [IO.Path]::GetFullPath((Join-Path $root $OutFile)) }
[void][IO.Directory]::CreateDirectory((Split-Path -Parent $outputPath))
$typPath = [IO.Path]::ChangeExtension($outputPath, '.typ')
if ($typPath -eq $outputPath) { throw 'OutFile must not use the .typ extension' }
[IO.File]::WriteAllText($typPath, $s.ToString(), $enc)

# ---- compile ----
Push-Location $root
try { & $typst compile $typPath $outputPath; if ($LASTEXITCODE -ne 0) { throw 'typst compile failed' } }
finally { Pop-Location }

Write-Host ('[OK] booklet: ' + $realCount + ' catalog entries + ' + $autoBlocks.Count + ' skeleton entries, ' + $plugBlocks.Count + ' plugins -> ' + $outputPath)
foreach ($w in $script:warn) { Write-Host ('[WARN] ' + $w) -ForegroundColor Yellow }

# ---- anchor eval: one query feeds both audits ----
# NOTE: `typst query` output carries no location on current toolchains; the old
#       JSON audit silently matched nothing (vacuous OK). eval() is the truth.
$prevEAP = $ErrorActionPreference
$ErrorActionPreference = 'Continue'
$expr = 'query(heading).filter(h=>h.has(str(label))).map(h=>(h.label,h.location().page()))'
$evalOut = & $typst eval $expr --in $typPath 2>$null
$ErrorActionPreference = $prevEAP

# ---- folder coverage audit: the booklet must anchor every knowledge folder ----
# independent recheck of the walk against what the generated source really
# emits: each scanned skeleton folder must show up as its labeled heading,
# otherwise the build fails here with the missing list printed.
if ($Filter -ne '') {
    Write-Host '[SKIP] folder coverage audit (filtered build)'
}
else {
    $gmap = @{}
    foreach ($ab in $autoBlocks) { $gmap[$ab.Meta.Name] = $ab }
    $found = @{}
    foreach ($m in [regex]::Matches(($evalOut -join ' '), '"<e-(g[0-9]+)>",(\d+)')) { $found[$m.Groups[1].Value] = $true }
    $missing = @($gmap.Keys | Where-Object { !$found.ContainsKey($_) })
    if ($missing.Count -gt 0) {
        foreach ($nm in $missing) { Write-Host ('[COVERAGE MISSING] ' + $gmap[$nm].Meta.Cn + '  ' + $gmap[$nm].Meta.Rel) -ForegroundColor Red }
        Write-Host ('[FAIL] folder coverage: ' + $missing.Count + ' of ' + $autoBlocks.Count + ' library folders missing from booklet'); exit 1
    }
    Write-Host ('[OK] folder coverage: ' + $kDirs.Count + '/' + $kDirs.Count + ' knowledge folders anchored (' + $autoBlocks.Count + ' skeleton entries, ' + $realCount + ' catalog entries, ' + $subIntroRaw.Count + ' group notes)')
}

# ---- parity audit: only meaningful when SoloMin>0 (duplex sheet fronts) ----
if ($SoloMin -le 0) {
    if (($evalOut -join ' ') -notmatch 'e-') { Write-Host '[FAIL] anchor eval matched nothing (eval broken?)'; exit 1 }
    Write-Host '[OK] parity: skipped (SoloMin=0, continuous layout, no blank filler pages)'
}
else {
$viol = 0
$hits = 0
$bigHits = 0
$bigset = @()
foreach ($b in $blocks) { if (-not $b.Auto -and $b.Lines -ge $SoloMin) { $bigset += $b.Meta.Name } }
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
if ($hits -eq 0) { Write-Host '[FAIL] parity audit matched no entry anchors (eval broken?)'; exit 1 }
if ($viol -eq 0) { Write-Host ('[OK] parity: ' + $bigHits + '/' + $bigset.Count + ' big entries start on odd pages (sheet fronts)') }
else { Write-Host ('[FAIL] parity violations: ' + $viol); exit 1 }
}
