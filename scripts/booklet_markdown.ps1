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
    [void]$out.AppendLine('#set text(size: 7pt)')
    [void]$out.AppendLine('#set par(leading: 0.45em, spacing: 0.55em)')
    [void]$out.AppendLine('#show math.equation: set text(font: "New Computer Modern Math")')
    [void]$out.AppendLine('#show math.equation.where(block: true): it => block(above: 5pt, below: 5pt, width: 100%, inset: (x: 4pt, y: 4pt), fill: luma(247), radius: 2pt)[#align(center, text(size: 8pt, it))]')
    for ($i=0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        if ($line -match '^\s*(`{3,}|~{3,})(\w*)\s*$') {
            $fence=$Matches[1]; $lang=$Matches[2]; $body=New-Object 'Collections.Generic.List[string]'; $closed=$false
            for ($i++; $i -lt $lines.Count; $i++) {
                if ($lines[$i] -match ('^\s*' + [regex]::Escape($fence[0]) + '{' + $fence.Length + ',}\s*$')) { $closed=$true; break }
                $body.Add($lines[$i])
            }
            if (-not $closed) { throw 'README has an unclosed code fence' }
            [void]$out.AppendLine('#raw(' + (Typ-String ($body -join "`n")) + ', block: true, lang: ' + (Typ-String $lang) + ')')
        } elseif ($line.Trim().StartsWith('$$')) {
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
            [void]$out.AppendLine('#metadata("booklet-math")')
            [void]$out.AppendLine('$ ' + (Convert-BookletMath $formula) + ' $')
        } elseif ($line -match '^\s*#{1,6}\s+(.+)$') {
            [void]$out.AppendLine('#block(sticky: true, above: 5pt, below: 2pt)[#text(weight: "bold", fill: rgb("#2e6da4"))[' + (Typ-Inline $Matches[1]) + ']]')
        } elseif ($line.Trim().StartsWith('|') -and $i+1 -lt $lines.Count -and $lines[$i+1] -match '^\s*\|?\s*:?-{3,}') {
            $header = @(Markdown-Cells $line); $cols=$header.Count
            [void]$out.AppendLine('#table(columns: '+$cols+', inset: 2.5pt, stroke: 0.3pt + luma(205), fill: (x,y) => if y == 0 { luma(242) },')
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
            $itemText=$Matches[2]; $itemMark=$Matches[1]
            $marker=if ($itemMark -match '^\d') { $itemMark } else { [string][char]0x2022 }
            [void]$out.AppendLine('#block(above: 3pt, below: 3pt, inset: (left: 5pt))[' + (Typ-Inline ($marker+' '+$itemText)) + ']')
            [void]$out.AppendLine('')
        } elseif ($line -match '^\s*>\s?(.*)$') {
            [void]$out.AppendLine('#block(inset: (left: 4pt), stroke: (left: 0.6pt + luma(170)))[' + (Typ-Inline $Matches[1]) + ']')
        } elseif ($line -match '^\s*(-{3,}|\*{3,})\s*$') { [void]$out.AppendLine('#line(length: 100%, stroke: 0.3pt + luma(200))') }
        else { [void]$out.AppendLine((Typ-Inline $line)) }
    }
    [void]$out.AppendLine(']')
    $out.ToString()
}
