# Restricted Markdown -> Typst content. No packages or executable markup.
# Plain text is always a quoted string, never interpreted as Typst source.
function Typ-String([string]$Text) {
    '"' + $Text.Replace('\','\\').Replace('"','\"').Replace("`r",'').Replace("`n",'\n').Replace("`t",'\t') + '"'
}
function Read-MathGroup($State) {
    while ($State.Pos -lt $State.Text.Length -and [char]::IsWhiteSpace($State.Text[$State.Pos])) { $State.Pos++ }
    if ($State.Pos -ge $State.Text.Length -or $State.Text[$State.Pos] -ne '{') { throw 'Math command requires a braced argument' }
    $State.Pos++
    Read-MathSequence $State $true
}
function Read-MathSequence($State,[bool]$Grouped=$false) {
    $parts=New-Object 'Collections.Generic.List[string]'
    $pairs=New-Object 'Collections.Generic.Stack[string]'
    $symbols=@{max='max';min='min';sum='sum';prod='product';log='log';ln='ln';
        le='<=';leq='<=';ge='>=';geq='>=';ne='!=';neq='!=';in='in';
        cdot='dot.op';times='times';infty='oo';to='->';mapsto='|->';equiv='equiv';
        lceil='ceil.l';rceil='ceil.r';lfloor='floor.l';rfloor='floor.r';
        mid='bar.v';alpha='alpha';beta='beta';Delta='Delta';theta='theta';
        quad='quad';qquad='quad';bmod='mod'}
    while ($State.Pos -lt $State.Text.Length) {
        $c=$State.Text[$State.Pos]; $State.Pos++
        if ([char]::IsWhiteSpace($c)) { continue }
        if ($c -eq '}') { if ($Grouped -and $pairs.Count -eq 0) { return ($parts -join ' ') }; throw 'Unmatched math brace/delimiter' }
        if ($c -eq '{') { $parts.Add('('+(Read-MathSequence $State $true)+')'); continue }
        if ($c -eq '\') {
            $tail=$State.Text.Substring($State.Pos)
            if ($tail -match '^([A-Za-z]+)') {
                $command=$Matches[1]; $State.Pos+=$command.Length
                if ($symbols.ContainsKey($command)) { $parts.Add($symbols[$command]) }
                elseif ($command -in @('left','right')) {
                    $rest=$State.Text.Substring($State.Pos)
                    if ($rest -notmatch '^\s*(\\(?:lceil|rceil|lfloor|rfloor|\{|\})|[()[\]|])') { throw 'Unsupported paired math delimiter' }
                    $delimiter=$Matches[1]; $State.Pos+=$Matches[0].Length
                    $mapping=@{'('=')';'['=']';'|'='|';'\lceil'='\rceil';'\lfloor'='\rfloor';'\{'='\}'}
                    if ($delimiter.StartsWith('\')) {
                        $name=$delimiter.Substring(1)
                        $value=if ($name -eq '{') { 'brace.l' } elseif ($name -eq '}') { 'brace.r' } else { $symbols[$name] }
                    } else { $value=$delimiter }
                    if ($command -eq 'left') {
                        if (-not $mapping.ContainsKey($delimiter)) { throw 'Invalid left math delimiter' }
                        $pairs.Push($mapping[$delimiter]); $parts.Add('lr('+$value)
                    } else {
                        if ($pairs.Count -eq 0 -or $pairs.Pop() -ne $delimiter) { throw 'Unmatched right math delimiter' }
                        $parts.Add($value+')')
                    }
                }
                elseif ($command -eq 'frac') {
                    $num=Read-MathGroup $State; $den=Read-MathGroup $State
                    $parts.Add('frac('+$num+','+$den+')')
                } elseif ($command -eq 'sqrt') { $parts.Add('sqrt('+(Read-MathGroup $State)+')') }
                elseif ($command -in @('operatorname','mathrm','mathcal')) {
                    $start=$State.Pos; $null=Read-MathGroup $State
                    $arg=$State.Text.Substring($start,$State.Pos-$start).Trim().Trim('{','}')
                    if ($arg -notmatch '^[A-Za-z]+$') { throw 'Math text commands accept letters only' }
                    if ($command -eq 'mathcal') { $parts.Add('cal('+(Typ-String $arg)+')') }
                    elseif ($command -eq 'operatorname') { $parts.Add('op('+(Typ-String $arg)+')') }
                    else { $parts.Add((Typ-String $arg)) }
                } else { throw ('Unsupported math command: \'+$command) }
            } elseif ($tail.Length -gt 0 -and $tail[0] -in @('{','}',',',';','!',' ')) {
                $State.Pos++
                if ($tail[0] -eq '{') { $parts.Add('brace.l') }
                elseif ($tail[0] -eq '}') { $parts.Add('brace.r') }
                elseif ($tail[0] -ne '!') { $parts.Add('thin') }
            } else { throw 'Unsupported math escape' }
        } elseif ($c -match '^[0-9]$') {
            $number=[regex]::Match($State.Text.Substring($State.Pos-1),'^[0-9]+(?:\.[0-9]+)?').Value
            $parts.Add($number); $State.Pos += $number.Length-1
        } elseif ($c -eq [char]39) { $parts.Add([string]$c) }
        elseif ($c -match '[A-Za-z0-9+\-=<>(),.\[\]/:;!|]') { $parts.Add([string]$c) }
        elseif ($c -in @('_','^')) {
            # TeX scripts consume one atom or one braced group, not the rest.
            while ($State.Pos -lt $State.Text.Length -and [char]::IsWhiteSpace($State.Text[$State.Pos])) { $State.Pos++ }
            if ($State.Pos -ge $State.Text.Length) { throw 'Missing math script' }
            if ($State.Text[$State.Pos] -eq '{') { $arg=Read-MathGroup $State }
            else {
                $arg=[string]$State.Text[$State.Pos]; $State.Pos++
                if ($arg -notmatch '^[A-Za-z0-9]$') { throw 'Use braces around a compound math script' }
            }
            $parts.Add(([string]$c)+'('+$arg+')')
        } else { throw ('Unsupported character in math: '+$c) }
    }
    if ($Grouped -or $pairs.Count) { throw 'Unclosed math brace/delimiter' }
    $parts -join ' '
}
function Convert-BookletMath([string]$Text) {
    if (-not $Text.Trim()) { throw 'Empty math expression' }
    Read-MathSequence ([pscustomobject]@{Text=$Text;Pos=0})
}
function Typ-Inline([string]$Text) {
    $out = New-Object Text.StringBuilder
    # Links keep their readable caption on paper; URLs remain in the Markdown.
    $pattern = '(`+)(.+?)\1|\*\*(.+?)\*\*|\[([^\]]+)\]\((?:<[^>]+>|[^()]*(?:\([^()]*\)[^()]*)*)\)|(?<!\\)\$([^$\n]+)\$'
    $pos = 0
    foreach ($m in [regex]::Matches($Text, $pattern)) {
        if ($m.Index -gt $pos) { [void]$out.Append('#text(' + (Typ-String $Text.Substring($pos,$m.Index-$pos)) + ')') }
        if ($m.Groups[2].Success) {
            # Inline code can wrap inside narrow columns, unlike a raw box.
            [void]$out.Append('#text(font: booklet-mono, size: 0.9em, ' + (Typ-String $m.Groups[2].Value) + ')')
        } elseif ($m.Groups[3].Success) { [void]$out.Append('#strong[' + (Typ-Inline $m.Groups[3].Value) + ']') }
        elseif ($m.Groups[5].Success) { [void]$out.Append('#metadata("booklet-math")$'+(Convert-BookletMath $m.Groups[5].Value)+'$') }
        else { [void]$out.Append((Typ-Inline $m.Groups[4].Value)) }
        $pos = $m.Index + $m.Length
    }
    if ($pos -lt $Text.Length) { [void]$out.Append('#text(' + (Typ-String $Text.Substring($pos)) + ')') }
    $out.ToString()
}
function Markdown-Cells([string]$Line) {
    # Split outside inline code, preserving both escaped and code pipes.
    $cells = New-Object 'Collections.Generic.List[string]'
    $cell = New-Object Text.StringBuilder
    $ticks = 0
    $math = $false
    $lineBody = $Line.Trim().Trim('|')
    for ($i=0; $i -lt $lineBody.Length; $i++) {
        $c = $lineBody[$i]
        if ($c -eq '$' -and $ticks -eq 0 -and ($i -eq 0 -or $lineBody[$i-1] -ne '\')) { $math=-not $math }
        if ($c -eq '\' -and $i+1 -lt $lineBody.Length -and $lineBody[$i+1] -eq '|') { [void]$cell.Append('|'); $i++; continue }
        if ($c -eq [char]96) {
            $n=1; while ($i+$n -lt $lineBody.Length -and $lineBody[$i+$n] -eq [char]96) { $n++ }
            if ($ticks -eq 0) { $ticks=$n } elseif ($ticks -eq $n) { $ticks=0 }
            [void]$cell.Append(('`' * $n)); $i += $n-1; continue
        }
        if ($c -eq '|' -and $ticks -eq 0 -and -not $math) { $cells.Add($cell.ToString().Trim()); [void]$cell.Clear() }
        else { [void]$cell.Append($c) }
    }
    $cells.Add($cell.ToString().Trim())
    $cells.ToArray()
}
function Convert-BookletMarkdown([string]$Text) {
    $lines = $Text.Replace("`r`n","`n") -split "`n"
    $out = New-Object Text.StringBuilder
    [void]$out.AppendLine('#block(above: 5pt, below: 2pt, breakable: true)[')
    [void]$out.AppendLine('#let manual-ink = rgb("#294f4b")')
    [void]$out.AppendLine('#set text(size: 7.1pt)')
    [void]$out.AppendLine('#set par(leading: 0.52em, spacing: 4pt, justify: false)')
    [void]$out.AppendLine('#show math.equation: set text(font: "New Computer Modern Math")')
    $section=0
    $formulaLabel=''
    $leadParagraph=$false
    for ($i=0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        # A standalone bold paragraph immediately before display math is its caption.
        if ($line -match '^\s*\*\*(.+?)\*\*\s*$') {
            $caption=$Matches[1]; $next=$i+1
            while ($next -lt $lines.Count -and -not $lines[$next].Trim()) { $next++ }
            if ($next -lt $lines.Count -and $lines[$next].Trim().StartsWith('$$')) {
                $formulaLabel=$caption; $i=$next; $line=$lines[$i]
            }
        }
        if ($line -match '^\s*(`{3,}|~{3,})(\w*)\s*$') {
            $leadParagraph=$false
            $fence=$Matches[1]; $lang=$Matches[2]; $body=New-Object 'Collections.Generic.List[string]'; $closed=$false
            for ($i++; $i -lt $lines.Count; $i++) {
                if ($lines[$i] -match ('^\s*' + [regex]::Escape($fence[0]) + '{' + $fence.Length + ',}\s*$')) { $closed=$true; break }
                $body.Add($lines[$i])
            }
            if (-not $closed) { throw 'README has an unclosed code fence' }
            [void]$out.AppendLine('#raw(' + (Typ-String ($body -join "`n")) + ', block: true, lang: ' + (Typ-String $lang) + ')')
        } elseif ($line.Trim().StartsWith('$$')) {
            $leadParagraph=$false
            $formula=$line.Trim().Substring(2)
            if ($formula.EndsWith('$$') -and $formula.Length -ge 2) { $formula=$formula.Substring(0,$formula.Length-2) }
            else {
                $closed=$false
                for ($i++; $i -lt $lines.Count; $i++) {
                    if ($lines[$i].Trim() -eq '$$') { $closed=$true; break }
                    $formula+=' '+$lines[$i]
                }
                if (-not $closed) { throw 'Unclosed display math' }
            }
            [void]$out.AppendLine('#block(width: 100%, above: 4pt, below: 5pt, inset: (x: 6pt, y: 5pt), fill: luma(247), radius: 2pt)[')
            if ($formulaLabel) {
                [void]$out.AppendLine('#text(size: 6.2pt, fill: luma(90))['+(Typ-Inline $formulaLabel)+'] #v(2pt)')
                $formulaLabel=''
            }
            [void]$out.AppendLine('#metadata("booklet-math")#align(center, text(size: 8pt)[$ ' + (Convert-BookletMath $formula) + ' $])]')
        } elseif ($line -match '^\s*(#{1,6})\s+(.+)$') {
            $level=$Matches[1].Length; $title=$Matches[2]
            $leadParagraph=$level -eq 1
            if ($level -eq 1) {
                $section=0
                [void]$out.AppendLine('#block(sticky: true, above: 5pt, below: 5pt, width: 100%)[#text(size: 12pt, weight: "bold", fill: manual-ink)['+(Typ-Inline $title)+']')
                # One quoted line immediately below H1 is the optional subtitle.
                $next=$i+1
                while ($next -lt $lines.Count -and -not $lines[$next].Trim()) { $next++ }
                if ($next -lt $lines.Count -and $lines[$next] -match '^\s*>\s?(.+)$') {
                    [void]$out.AppendLine('#v(3pt) #text(size: 6.6pt, fill: luma(90))['+(Typ-Inline $Matches[1])+']')
                    $i=$next
                }
                [void]$out.AppendLine('#v(5pt) #line(length: 100%, stroke: 1pt + manual-ink)]')
            } elseif ($level -eq 2) {
                $section++
                [void]$out.AppendLine('#block(sticky: true, above: 8pt, below: 3.5pt, width: 100%)[#grid(columns: (18pt, 1fr), align: horizon, text(size: 7pt, fill: luma(135), '+(Typ-String $section.ToString('00'))+'), text(size: 8.5pt, weight: "bold", fill: manual-ink)['+(Typ-Inline $title)+']) #v(2.5pt) #line(length: 100%, stroke: 0.35pt + luma(210))]')
            } else {
                [void]$out.AppendLine('#block(sticky: true, above: 5pt, below: 3pt)[#text(size: 7.5pt, weight: "bold", fill: manual-ink)['+(Typ-Inline $title)+']]')
            }
        } elseif ($line.Trim().StartsWith('|') -and $i+1 -lt $lines.Count -and $lines[$i+1] -match '^\s*\|?\s*:?-{3,}') {
            $leadParagraph=$false
            $header = @(Markdown-Cells $line); $cols=$header.Count
            [void]$out.AppendLine('#table(columns: ('+(('1fr,' * $cols))+'), inset: (x: 4pt, y: 3pt), stroke: (left: none, right: none, top: none, bottom: 0.3pt + luma(215)), fill: (x,y) => if y == 0 { luma(244) },')
            [void]$out.AppendLine('table.header(' + (($header | ForEach-Object { '[#strong['+(Typ-Inline $_)+']]' }) -join ',') + '),')
            $i += 2
            while ($i -lt $lines.Count -and $lines[$i].Trim().StartsWith('|')) {
                $cells = @(Markdown-Cells $lines[$i])
                if ($cells.Count -ne $cols) { throw ('README table column mismatch at line '+($i+1)) }
                [void]$out.AppendLine((($cells | ForEach-Object { '['+(Typ-Inline $_)+']' }) -join ',') + ',')
                $i++
            }
            $i--
            [void]$out.AppendLine(')')
        } elseif ($line -match '^\s*([-+*]|\d+[.)])\s+(.+)$') {
            $leadParagraph=$false
            $numbered=[regex]::IsMatch($Matches[1],'^\d'); $start=1
            if ($numbered) { $start=[int]($Matches[1] -replace '\D','') }
            $kind=if ($numbered) { 'enum(start: '+$start+', ' } else { 'list(' }
            [void]$out.AppendLine('#'+$kind+'indent: 7pt, body-indent: 3pt, spacing: 2pt,')
            do {
                [void]$out.AppendLine('['+(Typ-Inline $Matches[2])+'],')
                $i++
            } while ($i -lt $lines.Count -and $lines[$i] -match '^\s*([-+*]|\d+[.)])\s+(.+)$' -and ([regex]::IsMatch($Matches[1],'^\d') -eq $numbered))
            $i--
            [void]$out.AppendLine(')')
        } elseif ($line -match '^\s*>\s?(.*)$') {
            $leadParagraph=$false
            [void]$out.AppendLine('#block(width: 100%, above: 5pt, below: 4pt, breakable: true, inset: (left: 6pt, right: 3pt, y: 3pt), stroke: (left: 1pt + manual-ink))[#show strong: it => text(size: 6.8pt, fill: manual-ink, it)')
            do {
                [void]$out.AppendLine((Typ-Inline $Matches[1]))
                $i++
            } while ($i -lt $lines.Count -and $lines[$i] -match '^\s*>\s?(.*)$')
            $i--
            [void]$out.AppendLine(']')
        } elseif ($line -match '^\s*(-{3,}|\*{3,})\s*$') { [void]$out.AppendLine('#line(length: 100%, stroke: 0.3pt + luma(200))') }
        elseif ($leadParagraph -and $line.Trim()) {
            # Keep title + opening prose with the next block (often a definition).
            # Only the short opening paragraph is grouped, never the whole manual.
            [void]$out.AppendLine('#block(sticky: true)[')
            do {
                [void]$out.AppendLine((Typ-Inline $lines[$i]))
                $i++
            } while ($i -lt $lines.Count -and $lines[$i].Trim() -and $lines[$i] -notmatch '^\s*(#|>|\$\$|\||`{3,}|~{3,}|[-+*]\s|\d+[.)]\s)')
            $i--
            $leadParagraph=$false
            [void]$out.AppendLine(']')
        } else { [void]$out.AppendLine((Typ-Inline $line)) }
    }
    [void]$out.AppendLine(']')
    $out.ToString()
}
