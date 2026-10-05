param([string]$Filter = '', [string]$OutFile = '', [int]$SoloMin = 0, [switch]$SourceOnly, [string]$TypstPath = '', [string]$Chapter = '', [string]$OutDir = 'docs/booklet/output/chapters', [switch]$GenerateOnly)
# make_booklet.ps1 - printable contest booklet generator (typst, A4 portrait, one column)
# Directories define chapters; catalog order only ranks existing source families.
# A README beside a selected source follows the last selected sibling, once.
# Roadmaps live in docs/roadmaps and never enter this print pipeline.
# Usage: powershell -ExecutionPolicy Bypass -File scripts\make_booklet.ps1              -> one PDF per top-level chapter
#        powershell -File scripts\make_booklet.ps1 -Filter seg -OutFile .zoi-checks/seg.pdf
# SoloMin: with a value > 0, real entries with >= SoloMin lines additionally start
#          on an odd page (= a physical sheet's front side) for duplex printing;
#          the default 0 starts each entry on a fresh page without parity padding.
# Pipeline: directory tree + catalog identities -> C++20 projection -> include rewrite
#           -> SHA256[:8] over LF-normalized text -> booklet.typ -> typst compile.
# Requires: typst on PATH, or scripts\typst.exe next to this script (auto-detected).
# NOTE: keep this file ASCII-only (PS 5.1 reads no-BOM as ANSI). CJK text
#       (family names, entry titles) flows in from file paths at runtime.
$ErrorActionPreference = 'Stop'
if (-not $OutFile -and -not $SourceOnly -and -not $GenerateOnly -and -not $Filter -and -not $Chapter) {
    & (Join-Path $PSScriptRoot 'build_booklet.ps1') -OutDir $OutDir -TypstPath $TypstPath -SoloMin $SoloMin
    return
}
if (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'sync_layout.ps1')) { & (Join-Path $PSScriptRoot 'sync_layout.ps1') }
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
. (Join-Path $PSScriptRoot 'booklet_cpp.ps1')

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
$tree=Get-BookletTree $root $allEntries $plugins $Filter $Chapter
if (-not $OutFile) {
    if ($Filter -or $Chapter) { throw 'A filtered/chapter preview requires -OutFile' }
    $destination = if ([IO.Path]::IsPathRooted($OutDir)) { [IO.Path]::GetFullPath($OutDir) } else { [IO.Path]::GetFullPath((Join-Path $root $OutDir)) }
    $formal = [IO.Path]::GetFullPath((Join-Path $root 'docs/booklet/output/chapters'))
    if (($SourceOnly -or $GenerateOnly) -and ($destination -eq $formal -or $destination.StartsWith($formal + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase))) { throw 'Unverified generation requires a separate -OutDir' }
    # Finish every chapter in isolation before replacing the published set.
    $scratch = [IO.Path]::GetFullPath((Join-Path $root '.zoi-checks/codex-work'))
    $stage = Join-Path $scratch ('booklet-build-' + [Guid]::NewGuid().ToString('N'))
    [void][IO.Directory]::CreateDirectory($stage)
    $manifest = @()
    foreach ($node in $tree.Roots) {
        $file = $node.Name + '.pdf'
        $argsForChapter = @{ Chapter=$node.Name; OutFile=(Join-Path $stage $file); SoloMin=$SoloMin; SourceOnly=$SourceOnly; TypstPath=$typst }
        & $PSCommandPath @argsForChapter
        $manifest += [pscustomobject]@{ chapter=$node.Name; pdf=$file; source=([IO.Path]::ChangeExtension($file,'.typ')); code=([IO.Path]::ChangeExtension($file,'.code.md')) }
    }
    [void][IO.Directory]::CreateDirectory($destination)
    foreach ($entry in $manifest) {
        foreach ($file in @($entry.source, $entry.code) + @($(if (-not $SourceOnly) { $entry.pdf }))) {
            if ($file) { Copy-Item -LiteralPath (Join-Path $stage $file) -Destination (Join-Path $destination $file) -Force }
        }
    }
    $index = [pscustomobject]@{ schema=1; paper='A4'; orientation='portrait'; columns=1; cppStandard='c++20'; profile='contest'; chapters=@($manifest) }
    [IO.File]::WriteAllText((Join-Path $destination 'chapters.json'), ($index | ConvertTo-Json -Depth 5), $enc)
    if (-not $stage.StartsWith($scratch + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid staging cleanup path' }
    Remove-Item -LiteralPath $stage -Recurse -Force
    Write-Host ('[OK] chapter set: ' + $manifest.Count + ' separate volumes -> ' + $destination)
    return
}
$entries=@($allEntries | Where-Object { $tree.Files.ContainsKey($_.Rel) })
$plugins=@($plugins | Where-Object { $tree.Files.ContainsKey($_.Rel) })

# ---- per-entry transform: rewrite includes, LF-normalize, hash ----
function Convert-Entry($e) {
    $path = Join-Path $root ($e.Rel.Replace('\', '/'))
    $trace = New-Object Collections.ArrayList
    $rew = if ($e.Prose) { [IO.File]::ReadAllText($path, $enc) } else { Read-BookletCode $path $stubByFile $zoiDir @() $trace }
    $norm = ($rew -replace "`r`n", "`n").TrimEnd()
    # trim decorative '='/'-' banner lines to the column width before hashing:
    # Keep decorative rules within the portrait text width at 9 pt.
    $cap = if ($e.Prose) { 85 } else { 95 }
    $trimDeco = [System.Text.RegularExpressions.MatchEvaluator]{ param($m) $m.Groups[1].Value.Substring(0, $cap) }
    $norm = [regex]::Replace($norm, ('(?m)^(\s*[=\-]{' + ($cap + 1) + ',})\s*$'), $trimDeco)
    $sha = [System.Security.Cryptography.SHA256]::Create()
    $h = ($sha.ComputeHash($enc.GetBytes($norm)) | ForEach-Object { $_.ToString('x2') }) -join ''
    $sha.Dispose()
    [pscustomobject]@{ Meta = $e; Text = $norm; Hash = $h.Substring(0, 8); FullHash = $h; Lines = ($norm -split "`n").Count; Trace = $trace }
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
[void]$s.AppendLine('#set page(paper: "a4", flipped: false, margin: (x: 1.5cm, y: 1.5cm, top: 1.8cm), numbering: (..a) => text(size: 8pt, fill: luma(120), { let n = a.pos().at(0); let t = if a.pos().len() > 1 { a.pos().at(1) } else { none }; let label = if t == none { str(n) } else { str(n) + " / " + str(t) }; if n == 39 { context { if counter(page).get().first() == 39 { text(size: 8pt, fill: rgb("#68aaa3"), "MIKU \u{2661}") } else { label } } } else { label } }))')
[void]$s.AppendLine('#set text(font: (' + (Typ-String $bodyFont) + ', ' + (Typ-String $cjkFont) + '), size: 10pt, lang: "zh", region: "cn", cjk-latin-spacing: auto)')
[void]$s.AppendLine('#set par(leading: 0.4em, spacing: 0.85em, justify: false)')
[void]$s.AppendLine('#set heading(numbering: none)')
# Depth controls all heading and contents styles; no deeper level falls back
# to Typst's larger default. Clamp only the font floor, never tree depth.
[void]$s.AppendLine('#let chapter-style(depth, contents: false) = {')
[void]$s.AppendLine('  let i = calc.min(depth, 5) - 1')
[void]$s.AppendLine('  if contents { (size: (12pt, 10pt, 9pt, 8.5pt, 8.2pt).at(i), weight: ("bold", "bold", "regular", "regular", "regular").at(i), ink: (rgb("#1f4e79"), luma(35), luma(55), luma(75), luma(85)).at(i)) }')
[void]$s.AppendLine('  else { (size: (18pt, 14pt, 12pt, 11pt, 10.5pt).at(i), weight: ("bold", "bold", "bold", "semibold", "regular").at(i), ink: (rgb("#1f4e79"), rgb("#243e52"), luma(45), luma(65), luma(80)).at(i)) }')
[void]$s.AppendLine('}')
[void]$s.AppendLine('#show heading: it => {')
[void]$s.AppendLine('  let style = chapter-style(it.level)')
[void]$s.AppendLine('  block(sticky: true, above: if it.level == 1 { 7pt } else if it.level == 2 { 10pt } else { 5pt }, below: 3pt, width: 100%)[#text(size: style.size, weight: style.weight, fill: style.ink, it.body)#if it.level == 1 { v(5pt); line(length: 100%, stroke: 1pt + style.ink) }]')
[void]$s.AppendLine('}')
$printTheme = [IO.File]::ReadAllText((Join-Path $root 'docs/booklet/assets/print.tmTheme'), $enc)
[void]$s.AppendLine('#set raw(theme: bytes(' + (Typ-String $printTheme) + '))')
[void]$s.AppendLine('#show raw: set text(font: booklet-mono, size: 9pt, fill: rgb("#1f2937"))')
[void]$s.AppendLine('#show raw.where(block: true): it => block(width: 100%, fill: none, stroke: (left: 1.1pt + luma(150), top: 0.35pt + luma(215), right: 0.35pt + luma(215), bottom: 0.35pt + luma(215)), inset: (x: 5pt, y: 3.5pt), radius: (top-right: 2pt, bottom-right: 2pt), it)')
[void]$s.AppendLine('#set outline.entry(fill: repeat(gap: 0.45em)[#text(fill: luma(165))[.]])')
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
[void]$s.AppendLine('  grid(columns: (auto, 1fr, auto), box(fill: luma(239), inset: (x: 4pt, y: 1pt), text(size: 7.5pt, weight: "bold", fill: luma(60))[#dom]), align(center, text(size: 7.5pt, fill: luma(110), flowing)), link(<booklet-contents>, text(size: 7.5pt, fill: luma(75))[' + (Zh '76ee 5f55') + ']) )')
[void]$s.AppendLine('  v(0.3em)')
[void]$s.AppendLine('  line(length: 100%, stroke: 0.3pt + luma(205))')
[void]$s.AppendLine('}')
[void]$s.AppendLine('')
# Embed the official vector artwork so each .typ remains portable and offline.
$coverTitle = if ($Chapter) { $Chapter } elseif ($Filter) { $Filter } else { (Zh '7b97 6cd5 7ade 8d5b 624b 518c') }
$coverSize = if ($coverTitle.Length -gt 12) { '26pt' } else { '42pt' }
$coverLogo = [IO.File]::ReadAllText((Join-Path $root 'docs/booklet/assets/icpc-foundation.svg'), $enc)
[void]$s.AppendLine('#let entrymeta(b) = block(width: 100%, above: 2pt, below: 3pt, fill: luma(245), inset: (x: 4pt, y: 2pt), text(size: 8pt, fill: luma(75), b))')
$documentTitle = if ($Chapter -or $Filter) { $coverTitle + ' / ' + (Zh '7b97 6cd5 7ade 8d5b 624b 518c') } else { $coverTitle }
[void]$s.AppendLine('#set document(title: ' + (Typ-String $documentTitle) + ')')
[void]$s.AppendLine('// cover: icpc-minimal')
[void]$s.AppendLine('#page(numbering: none, header: none, footer: none, margin: 25mm)[')
[void]$s.AppendLine('#set align(center)')
[void]$s.AppendLine('#v(24mm)')
[void]$s.AppendLine('#image(bytes(' + (Typ-String $coverLogo) + '), format: "svg", width: 62mm, alt: "ICPC Foundation")')
[void]$s.AppendLine('#v(23mm)')
[void]$s.AppendLine('#text(size: ' + $coverSize + ', weight: "bold", tracking: 1pt, fill: rgb("#172c46"))[' + (Esc $coverTitle) + ']')
if ($Chapter -or $Filter) {
    [void]$s.AppendLine('#v(6mm)')
    [void]$s.AppendLine('#text(size: 12pt, tracking: 1.5pt, fill: luma(105))[' + (Zh '7b97 6cd5 7ade 8d5b 624b 518c') + ']')
}
[void]$s.AppendLine('#v(14mm)')
[void]$s.AppendLine('#grid(columns: (14mm, 14mm, 14mm), gutter: 0pt, ..("#4c82c3", "#ffbc10", "#b8281c").map(c => rect(width: 14mm, height: 1.2pt, fill: rgb(c), stroke: none)))')
[void]$s.AppendLine(']')
[void]$s.AppendLine('#set page(header: text(size: 10pt, weight: "bold", fill: luma(75))[' + (Zh '76ee 5f55') + '])')
[void]$s.AppendLine('#metadata("booklet-contents") <booklet-contents>')
[void]$s.AppendLine('#outline(title: none, indent: 10pt)')
[void]$s.AppendLine('#pagebreak()')
[void]$s.AppendLine('#set page(columns: 1, margin: (x: 1.5cm, y: 1.5cm, top: 1.8cm), header: pagehead)')
[void]$s.AppendLine('')
$blockByPath=@{}
foreach ($b in @($blocks)+@($plugBlocks)) { $blockByPath[$b.Meta.Rel]=$b }
$pluginLabels=@{}
for ($pi=0; $pi -lt $plugBlocks.Count; $pi++) { $pluginLabels[$plugBlocks[$pi].Meta.Rel]='e-plug'+$pi }
$script:printedDirs=@{}
$script:pendingHeadings=New-Object Text.StringBuilder
$script:newDomain=$false
$script:lastBlockKind=''
[void]$s.AppendLine('// directory-scope: '+$(if ($Filter) { 'filtered' } elseif ($Chapter) { 'chapter' } else { 'all' }))
if ($Chapter) { [void]$s.AppendLine('// chapter-scope: ' + $Chapter) }
foreach ($node in $tree.Roots) { Write-BookletNode $node $s $blockByPath $pluginLabels $SoloMin }
if ($script:pendingHeadings.Length) { throw 'Unflushed directory headings' }
$outputPath = if ([IO.Path]::IsPathRooted($OutFile)) { [IO.Path]::GetFullPath($OutFile) } else { [IO.Path]::GetFullPath((Join-Path $root $OutFile)) }
if ([IO.Path]::GetExtension($outputPath) -ne '.pdf') { throw 'OutFile must use the .pdf extension' }
$canonical = [IO.Path]::GetFullPath((Join-Path $root 'docs/booklet/output/zoi-booklet-print.pdf'))
$canonicalDir = [IO.Path]::GetFullPath((Join-Path $root 'docs/booklet/output/chapters')) + [IO.Path]::DirectorySeparatorChar
if ($outputPath -eq $canonical -or $outputPath.StartsWith($canonicalDir, [StringComparison]::OrdinalIgnoreCase)) { throw 'A preview requires -OutFile in the workspace scratch directory' }
[void][IO.Directory]::CreateDirectory((Split-Path -Parent $outputPath))
$typPath = [IO.Path]::ChangeExtension($outputPath, '.typ')
if ($typPath -eq $outputPath) { throw 'OutFile must not use the .typ extension' }
[IO.File]::WriteAllText($typPath, $s.ToString(), $enc)
# Human-readable, exact print input. The PDF itself stays free of build details.
$audit = New-Object Text.StringBuilder
[void]$audit.AppendLine('# C++20 contest code')
[void]$audit.AppendLine('')
[void]$audit.AppendLine('Profile: ZOI_BOOKLET defined; __cplusplus = 202002L. Generated; edit the source instead.')
foreach ($b in @($blocks) + @($plugBlocks)) {
    if ($b.Meta.Prose) { continue }
    [void]$audit.AppendLine("`n<!-- entry: " + $b.Meta.Rel + ' -->')
    [void]$audit.AppendLine('## ' + $b.Meta.Rel)
    [void]$audit.AppendLine("`nSHA256: " + $b.FullHash + '; lines: ' + $b.Lines)
    foreach ($choice in $b.Trace) {
        $origin = $choice.source.Replace($root + [IO.Path]::DirectorySeparatorChar, '').Replace('\','/')
        [void]$audit.AppendLine('- ' + $origin + ': ' + $choice.condition + ' -> ' + $choice.selected)
    }
    $fenceLength = 3
    foreach ($ticks in [regex]::Matches($b.Text, '`+')) { $fenceLength = [Math]::Max($fenceLength, $ticks.Length + 1) }
    $fence = '`' * $fenceLength
    [void]$audit.AppendLine("`n" + $fence + 'cpp')
    [void]$audit.AppendLine($b.Text)
    [void]$audit.AppendLine($fence)
}
[IO.File]::WriteAllText([IO.Path]::ChangeExtension($outputPath, '.code.md'), $audit.ToString(), $enc)

if ($SourceOnly) { Write-Host ('[OK] source: ' + $entries.Count + ' entries, ' + $printedManuals.Count + ' manuals -> ' + $typPath); return }

# ---- compile ----
Push-Location $root
try { & $typst compile $typPath $outputPath; if ($LASTEXITCODE -ne 0) { throw 'typst compile failed' } }
finally { Pop-Location }

Write-Host ('[OK] booklet: ' + $realCount + ' catalog entries + ' + $printedManuals.Count + ' manuals, ' + $plugBlocks.Count + ' plugins -> ' + $outputPath)

# ---- anchor eval: one query feeds both audits ----
# NOTE: `typst query` output carries no location on current toolchains; the old
#       JSON audit silently matched nothing (vacuous OK). eval() is the truth.
function Invoke-BookletEval([string]$Expression) {
    # Bypass PowerShell's legacy native argument binder: PS 5.1 strips quotes
    # inside Typst expressions. Use ArgumentList on .NET, CRT quoting on .NET FX.
    $psi=New-Object Diagnostics.ProcessStartInfo
    $psi.FileName=$typst
    $psi.WorkingDirectory=$root
    $psi.UseShellExecute=$false
    $psi.CreateNoWindow=$true
    $psi.RedirectStandardOutput=$true
    $psi.RedirectStandardError=$true
    $psi.StandardOutputEncoding=$enc
    $psi.StandardErrorEncoding=$enc
    $arguments=@('eval',$Expression,'--in',$typPath)
    if ($psi.PSObject.Properties.Name -contains 'ArgumentList') {
        foreach ($arg in $arguments) { $psi.ArgumentList.Add($arg) }
    } else {
        $quoted=foreach ($arg in $arguments) {
            '"'+([regex]::Replace([regex]::Replace($arg,'(\\*)"','$1$1\"'),'(\\+)$','$1$1'))+'"'
        }
        $psi.Arguments=$quoted -join ' '
    }
    $process=New-Object Diagnostics.Process
    $process.StartInfo=$psi
    try {
        [void]$process.Start()
        $stdout=$process.StandardOutput.ReadToEndAsync()
        $stderr=$process.StandardError.ReadToEndAsync()
        $process.WaitForExit()
        $result=$stdout.GetAwaiter().GetResult()
        $diagnostic=$stderr.GetAwaiter().GetResult()
        if ($process.ExitCode -ne 0) { throw ('Typst audit query failed: '+$diagnostic) }
        if ($diagnostic) { Write-Host $diagnostic }
        return $result
    }
    finally { $process.Dispose() }
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
if ([int]($mathActual -join '') -ne $mathExpected) { throw 'Math equation coverage mismatch' }
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
