# C++20 print projection. This deliberately is not a general C preprocessor.
# ZOI_BOOKLET is defined for paper only. Unknown macros stay for the compiler.
function Convert-BookletCpp20([string]$Text, [string]$Source = '<source>', [Collections.IList]$Trace = $null) {
    $Text = $Text.Replace("`r`n", "`n").Replace("`r", "`n")
    # Mask comments/literals so examples and raw strings cannot become directives.
    $tokens = '(?s)R"(?<raw>[^ ()\\\t\r\n]{0,16})\(.*?\)\k<raw>"|/\*.*?\*/|//[^\n]*(?:\n(?<=\\\n)[^\n]*)*|"(?:\\.|[^"\\])*"|(?<![\w''])''(?:\\.|[^''\\\n])+'''
    $usage = [Text.RegularExpressions.MatchEvaluator]{ param($m)
        if ($m.Value -match '(?s)^(/\*\s*Usage\b)(.*)(\*/)$') {
            return $Matches[1] + (Convert-BookletCpp20 $Matches[2] ($Source + ' Usage') $Trace) + $Matches[3]
        }
        return $m.Value
    }
    $Text = [regex]::Replace($Text, $tokens, $usage)
    $mask = [Text.RegularExpressions.MatchEvaluator]{ param($m)
        return [regex]::Replace($m.Value, '[^\n]', ' ')
    }
    $masked = [regex]::Replace($Text, $tokens, $mask) -split "`n"
    $lines = $Text -split "`n"
    $rootBody = New-Object Text.StringBuilder
    $stack = New-Object Collections.ArrayList
    for ($i = 0; $i -lt $lines.Count; $i++) {
        $line = $lines[$i]
        $code = $masked[$i]
        $target = $rootBody
        if ($stack.Count) { $target = $stack[$stack.Count - 1].Bodies[-1] }
        if ($code -notmatch '^\s*#\s*(if|ifdef|ifndef|elif|else|endif)\b(.*)$') {
            if ($code -match '^\s*#\s*(define|undef)\s+(__cplusplus|ZOI_BOOKLET)\b') {
                throw ($Source + ':' + ($i + 1) + ': cannot redefine a booklet profile macro')
            }
            [void]$target.Append($line)
            if ($i -lt $lines.Count - 1) { [void]$target.Append("`n") }
            continue
        }
        $kind = $Matches[1]
        $expression = $Matches[2].Trim()
        $where = $Source + ':' + ($i + 1)
        if ($line.TrimEnd().EndsWith('\')) { throw ($where + ': multiline conditional is unsupported; use one directive per line') }
        $value = $null
        if ($kind -in @('if','elif','ifdef','ifndef')) {
            if ($expression -match '\bZOI_BOOKLET\b') {
                if ($kind -eq 'ifdef' -and $expression -ceq 'ZOI_BOOKLET') { $value = $true }
                elseif ($kind -eq 'ifndef' -and $expression -ceq 'ZOI_BOOKLET') { $value = $false }
                elseif ($kind -in @('if','elif') -and $expression -cmatch '^(!\s*)?defined\s*(?:\(\s*ZOI_BOOKLET\s*\)|\s+ZOI_BOOKLET)$') {
                    $value = -not [bool]$Matches[1]
                } else {
                    throw ($where + ': unsupported ZOI_BOOKLET condition; use ifdef/ifndef or defined, and nest other conditions')
                }
            } elseif ($expression -match '\b__cplusplus\b') {
                if ($kind -notin @('if','elif') -or $expression -notmatch '^__cplusplus\s*(>=|<=|==|!=|>|<)\s*([0-9]{6})[lL]?$') {
                    throw ($where + ': unsupported __cplusplus condition; use a direct comparison and nest other macros separately')
                }
                $op = $Matches[1]; $version = [int]$Matches[2]
                $value = switch ($op) {
                    '>=' { 202002 -ge $version }; '<=' { 202002 -le $version }
                    '==' { 202002 -eq $version }; '!=' { 202002 -ne $version }
                    '>' { 202002 -gt $version }; '<' { 202002 -lt $version }
                }
            }
        }
        if ($kind -in @('if','ifdef','ifndef')) {
            $group = [pscustomobject]@{
                Headers = (New-Object Collections.ArrayList)
                Values = (New-Object Collections.ArrayList)
                Bodies = (New-Object Collections.ArrayList)
                HasElse = $false; Where = $where
            }
            [void]$group.Headers.Add($line)
            [void]$group.Values.Add($value)
            [void]$group.Bodies.Add((New-Object Text.StringBuilder))
            [void]$stack.Add($group)
            continue
        }
        if (-not $stack.Count) { throw ($where + ': unmatched #' + $kind) }
        $group = $stack[$stack.Count - 1]
        if ($kind -ne 'endif') {
            if ($group.HasElse) { throw ($where + ': branch after #else') }
            if ($kind -eq 'else') { $group.HasElse = $true }
            [void]$group.Headers.Add($line)
            [void]$group.Values.Add($value)
            [void]$group.Bodies.Add((New-Object Text.StringBuilder))
            continue
        }
        $stack.RemoveAt($stack.Count - 1)
        $target = $rootBody
        if ($stack.Count) { $target = $stack[$stack.Count - 1].Bodies[-1] }
        $conditionCount = $group.Values.Count - [int]$group.HasElse
        $known = 0
        for ($j = 0; $j -lt $conditionCount; $j++) { if ($null -ne $group.Values[$j]) { $known++ } }
        if ($known -and $known -ne $conditionCount) { throw ($group.Where + ': mixed profile/unknown #elif chain; nest the conditions separately') }
        if ($known) {
            $chosen = '<empty>'
            for ($j = 0; $j -lt $group.Bodies.Count; $j++) {
                if (($j -eq $conditionCount) -or $group.Values[$j]) {
                    $chosen = $group.Headers[$j].Trim()
                    [void]$target.Append($group.Bodies[$j].ToString()); break
                }
            }
            if ($null -ne $Trace) {
                [void]$Trace.Add([pscustomobject]@{ source=$Source; condition=$group.Headers[0].Trim(); selected=$chosen })
            }
        } else {
            for ($j = 0; $j -lt $group.Bodies.Count; $j++) {
                [void]$target.Append($group.Headers[$j] + "`n" + $group.Bodies[$j].ToString())
            }
            [void]$target.Append($line)
            if ($i -lt $lines.Count - 1) { [void]$target.Append("`n") }
        }
    }
    if ($stack.Count) { throw ($stack[-1].Where + ': unclosed conditional') }
    return $rootBody.ToString()
}

# Inline private headers only after selecting the live C++20 branch.
# Public catalog dependencies keep their short names; no filename conventions.
function Read-BookletCode([string]$Path, [hashtable]$StubMap, [string]$ZoiDir, [string[]]$Stack = @(), [Collections.IList]$Trace = $null) {
    $Path = [IO.Path]::GetFullPath($Path)
    if ($Stack -contains $Path) { throw ('Cyclic private booklet include: ' + $Path) }
    $text = [IO.File]::ReadAllText($Path, [Text.Encoding]::UTF8)
    $text = Convert-BookletCpp20 $text $Path $Trace
    $ev = [Text.RegularExpressions.MatchEvaluator]{ param($m)
        $base = [IO.Path]::GetFullPath((Join-Path (Split-Path -Parent $Path) $m.Groups[2].Value))
        if ($StubMap.ContainsKey($base)) { return $m.Groups[1].Value + $StubMap[$base] + '.h"' }
        if ([IO.Path]::GetExtension($base) -eq '.h' -and [IO.File]::Exists($base)) {
            return (Read-BookletCode $base $StubMap $ZoiDir ($Stack + @($Path)) $Trace).TrimEnd()
        }
        if ([IO.File]::Exists((Join-Path $ZoiDir $m.Groups[2].Value))) { return $m.Value }
        throw ($Path + ': unresolved booklet include ' + $base)
    }
    return [regex]::Replace($text, '(?m)^([ \t]*#\s*include\s+")([^"\r\n]+\.(?:cpp|h))"[ \t]*\r?$', $ev)
}
