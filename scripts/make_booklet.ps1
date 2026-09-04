param([string]$Filter = '', [string]$OutFile = 'zoi-booklet.pdf', [int]$SoloMin = 0)
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
if ($Filter -ne '') {
    $entries = @($entries | Where-Object { $_.Domain -like "*$Filter*" -or $_.Sub -like "*$Filter*" -or $_.Name -like "*$Filter*" -or $_.Cn -like "*$Filter*" })
}
if ($entries.Count -eq 0) { throw 'no catalog entries matched' }

# basename -> stub map for local include rewrite
$stubByFile = @{}
foreach ($e in $entries) { $stubByFile[[IO.Path]::GetFileName($e.Rel)] = $e.Name }

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
function Esc([string]$t) { return ($t -replace '([_\*\[\]#`$\\@<>])', '\$1') }   # typst markup escape
$s = New-Object System.Text.StringBuilder
[void]$s.AppendLine('#set page(paper: "a4", flipped: true, margin: (x: 0.8cm, y: 1.0cm, top: 1.5cm), numbering: (..a) => text(size: 6pt, fill: luma(120), { let n = a.pos().at(0); let t = if a.pos().len() > 1 { a.pos().at(1) } else { none }; if t == none { str(n) } else { str(n) + " / " + str(t) } }))')
[void]$s.AppendLine('#set text(font: ("Source Sans Pro", "Noto Sans SC"), size: 7.5pt, lang: "zh", region: "cn", cjk-latin-spacing: auto)')
[void]$s.AppendLine('#set par(leading: 0.52em, spacing: 0.85em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
[void]$s.AppendLine('#show heading.where(level: 1): it => block(above: 2.4em, below: 1.1em, width: 100%)[#text(size: 13pt, weight: "bold", fill: rgb("#1f4e79"), it.body) #v(0.5em) #line(length: 100%, stroke: 1pt + rgb("#1f4e79"))]')
[void]$s.AppendLine('#show raw: set text(font: ("Consolas", "Noto Sans SC"), size: 6pt)')
[void]$s.AppendLine('#show raw.where(block: true): it => block(width: 100%, fill: luma(249), stroke: (left: 1.1pt + luma(150), top: 0.35pt + luma(215), right: 0.35pt + luma(215), bottom: 0.35pt + luma(215)), inset: (x: 5pt, y: 3.5pt), radius: (top-right: 2pt, bottom-right: 2pt), it)')
[void]$s.AppendLine('#show heading.where(level: 2): it => block(above: 1.3em, below: 0.3em, text(weight: "bold", size: 7.8pt, fill: luma(25), it.body))')
[void]$s.AppendLine('#let subsep(b) = block(above: 1.9em, below: 0.55em, width: 100%)[#text(size: 9.5pt, weight: "bold", fill: rgb("#2e6da4"), b) #v(0.4em) #line(length: 100%, stroke: 0.7pt + luma(165))]')
[void]$s.AppendLine('#let subintro(b) = block(above: 0em, below: 0.8em, width: 100%, fill: luma(246), inset: (x: 4.5pt, y: 3.5pt), radius: 2pt)[#text(size: 6.2pt, fill: luma(105), b)]')
[void]$s.AppendLine('#show outline: set text(size: 7pt)')
[void]$s.AppendLine('#show outline.entry: set par(leading: 0.5em)')
[void]$s.AppendLine('#show outline.entry.where(level: 1): it => block(above: 0.85em, below: 0.15em, text(weight: "bold", size: 8.5pt, fill: rgb("#1f4e79"), it))')
[void]$s.AppendLine('#show outline.entry.where(level: 2): it => { set text(size: 6.4pt, fill: luma(75)); it }')
[void]$s.AppendLine('#let pagehead = context {')
[void]$s.AppendLine('  let pg = here().page()')
[void]$s.AppendLine('  let doms = query(heading.where(level: 1).before(here()))')
[void]$s.AppendLine('  let dom = if doms.len() > 0 { doms.last().body } else { [--] }')
[void]$s.AppendLine('  let ents = query(heading.where(level: 2).before(here())).filter(h => h.has(str(label)))')
[void]$s.AppendLine('  let flowing = if ents.len() > 0 { ents.last().body } else { [--] }')
[void]$s.AppendLine('  grid(columns: (auto, 1fr, auto), text(size: 6pt, fill: luma(110))[#dom], align(center, text(size: 6pt, fill: luma(110), flowing)), text(size: 6pt, fill: luma(110))[zoi booklet])')
[void]$s.AppendLine('  v(0.3em)')
[void]$s.AppendLine('  line(length: 100%, stroke: 0.3pt + luma(205))')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#let colrule = [#place(line(start: (33.34%, 0%), end: (33.34%, 100%), stroke: 0.4pt + luma(210))) #place(line(start: (66.67%, 0%), end: (66.67%, 100%), stroke: 0.4pt + luma(210)))]')
[void]$s.AppendLine('')
[void]$s.AppendLine('#v(1fr)')
[void]$s.AppendLine('#align(center, text(size: 24pt, weight: "bold")[zoi Contest Booklet])')
[void]$s.AppendLine('#v(0.5em)')
[void]$s.AppendLine('#align(center, text(size: 7pt, fill: luma(100))[' + $realCount + ' catalog entries + ' + $autoBlocks.Count + ' skeleton entries (' + $kDirs.Count + ' knowledge folders, coverage audited) + ' + $plugBlocks.Count + ' algebra plugins - hash = SHA256 first 8 hex over LF-normalized text, includes rewritten to stub names. On-site check: count lines, then hash after LF save.])')
[void]$s.AppendLine('#v(1em)')
[void]$s.AppendLine('#align(center, line(length: 55%, stroke: 1.1pt + rgb("#1f4e79")))')
[void]$s.AppendLine('#v(1fr)')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#columns(3, gutter: 0.9cm)[#outline(title: none, depth: 2, indent: 0.6em)]')
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
    elseif (-not $b.Auto) { [void]$s.AppendLine($pbPage) }   # real entries: one page each
    elseif (-not $prevAuto) { [void]$s.AppendLine($pbPage) } # skeleton runs start on a fresh page too
    if ($newSub) {
        [void]$s.AppendLine('#subsep[' + (Esc $b.Meta.Sub) + ']')
        $ikey = $b.Meta.Domain + '/' + $b.Meta.Sub
        if ($subIntro.ContainsKey($ikey)) { [void]$s.AppendLine('#subintro[' + (Esc ($subIntro[$ikey].Trim())) + ']') }
        $lastSub = $b.Meta.Sub
    }
    $tag = ''
    if ($b.Meta.Prose -and (-not $b.Auto -or $b.Lines -eq 0)) {
        $tag = ' #text(fill: luma(165), size: 0.72em)[' + $tobu + ']'   # catalog prose + empty shells carry the marker
    }
    [void]$s.AppendLine('== ' + (Esc $b.Meta.Cn) + $tag + ' <e-' + $b.Meta.Name + '>')
    if ($b.Lines -gt 0) {
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
    }
    [void]$s.AppendLine('')
    $prevAuto = [bool]$b.Auto
}
if ($plugBlocks.Count -gt 0) {
    $firstPlug = $true
    $pi = 0
    foreach ($b in $plugBlocks) {
        $brk = if ($SoloMin -gt 0 -and $b.Lines -ge $SoloMin) { $pbOdd } else { $pbPage }
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

Write-Host ('[OK] booklet: ' + $realCount + ' catalog entries + ' + $autoBlocks.Count + ' skeleton entries, ' + $plugBlocks.Count + ' plugins -> ' + (Join-Path $root $OutFile))
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
if ($hits -eq 0) { Write-Host '[FAIL] parity audit matched no entry anchors (eval broken?)'; exit 1 }
if ($viol -eq 0) { Write-Host ('[OK] parity: ' + $bigHits + '/' + $bigset.Count + ' big entries start on odd pages (sheet fronts)') }
else { Write-Host ('[FAIL] parity violations: ' + $viol); exit 1 }
}
