# Shared JSONC edits and reversible setup transactions. ASCII for PS5.1.
$script:SetupEncoding=New-Object Text.UTF8Encoding($false,$true)
function Setup-Read([string]$p) { return $script:SetupEncoding.GetString([IO.File]::ReadAllBytes($p)) }
function Setup-Json($value) { return ConvertTo-Json -InputObject $value -Depth 60 -Compress }
function Setup-CphPath([string]$path) {
    if ($path -notmatch '\s') { return $path.Replace('\','/') }
    if ([IO.Path]::DirectorySeparatorChar -eq '\') {
        if (-not ('ZOI.NativePath' -as [type])) {
            Add-Type -Namespace ZOI -Name NativePath -MemberDefinition '[System.Runtime.InteropServices.DllImport("kernel32.dll", CharSet=System.Runtime.InteropServices.CharSet.Unicode, SetLastError=true)] public static extern uint GetShortPathName(string path, System.Text.StringBuilder result, uint size);'
        }
        $buffer=New-Object Text.StringBuilder 32768
        $n=[ZOI.NativePath]::GetShortPathName($path,$buffer,32768)
        if ($n -gt 0 -and $n -lt 32768 -and $buffer.ToString() -notmatch '\s') { return $buffer.ToString().Replace('\','/') }
    }
    throw 'CPH splits Args on spaces. This volume provides no usable short path; put the library in a path without spaces (for example C:\zoi), then install again. Configuration has not been changed.'
}
function JC-Tokens([string]$text) {
    $pattern='\G(?:[\s\uFEFF]+|//[^\r\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\\r\n])*"|[{}\[\]:,]|true|false|null|-?(?:0|[1-9]\d*)(?:\.\d+)?(?:[eE][+-]?\d+)?)'
    $list=New-Object 'Collections.Generic.List[object]'; $pos=0
    while ($pos -lt $text.Length) {
        # Match from the current position, not from the document beginning.
        $m=(New-Object regex($pattern)).Match($text,$pos)
        if (-not $m.Success -or $m.Index -ne $pos) { throw "Invalid JSONC near offset $pos" }
        if ($m.Value -notmatch '^(?:[\s\uFEFF]|//|/\*)') { $list.Add(@{s=$pos; e=$pos+$m.Length; v=$m.Value}) }
        $pos += $m.Length
    }
    return ,$list
}
function JC-Node($c) {
    if ($c.i -ge $c.tokens.Count) { throw 'Unexpected end of JSONC' }
    $t=$c.tokens[$c.i]; $c.i++
    $node=@{s=$t.s; e=$t.e; kind='scalar'; children=@(); comma=$null}
    if ($t.v -eq '{' -or $t.v -eq '[') {
        $node.kind=$t.v; $close=']'; if ($t.v -eq '{') { $close='}' }
        $keys=New-Object 'Collections.Generic.HashSet[string]' ([StringComparer]::Ordinal)
        while ($true) {
            if ($c.i -ge $c.tokens.Count) { throw 'Unclosed JSONC container' }
            if ($c.tokens[$c.i].v -eq $close) { $node.e=$c.tokens[$c.i].e; $c.i++; break }
            $key=$null; $start=$c.tokens[$c.i].s
            if ($node.kind -eq '{') {
                $k=$c.tokens[$c.i]; $c.i++
                if (-not $k.v.StartsWith('"')) { throw 'JSONC property name must be a string' }
                $key=$k.v | ConvertFrom-Json
                if (-not $keys.Add($key)) { throw "Duplicate JSONC property: $key" }
                if ($c.i -ge $c.tokens.Count -or $c.tokens[$c.i].v -ne ':') { throw 'Missing JSONC colon' }
                $c.i++
            }
            $child=JC-Node $c
            $item=@{s=$start; e=$child.e; value=$child; key=$key; comma=$null}
            if ($c.i -lt $c.tokens.Count -and $c.tokens[$c.i].v -eq ',') { $item.comma=$c.tokens[$c.i]; $c.i++ }
            elseif ($c.i -ge $c.tokens.Count -or $c.tokens[$c.i].v -ne $close) { throw 'Missing JSONC comma' }
            $node.children += $item
        }
    } elseif ($t.v -match '^[}\]:,]$') { throw 'Unexpected JSONC punctuation' }
    else { $null=$t.v | ConvertFrom-Json }
    return $node
}
function JC-Parse([string]$text) {
    $c=@{tokens=(JC-Tokens $text); i=0}; $node=JC-Node $c
    if ($c.i -ne $c.tokens.Count) { throw 'Trailing JSONC tokens' }
    return $node
}
function JC-Normal([string]$raw) {
    $null=JC-Parse $raw; $t=JC-Tokens $raw; $s=New-Object Text.StringBuilder
    for ($i=0;$i -lt $t.Count;$i++) {
        if ($t[$i].v -eq ',' -and $i+1 -lt $t.Count -and $t[$i+1].v -in @('}',']')) { continue }
        [void]$s.Append($t[$i].v)
    }
    return $s.ToString()
}
function JC-Value([string]$raw) { return ,((JC-Normal $raw) | ConvertFrom-Json) }
function JC-Member([string]$text,[string]$key) {
    $node=JC-Parse $text
    if ($node.kind -ne '{') { throw 'Expected a JSONC object' }
    foreach ($item in $node.children) { if ($item.key -ceq $key) { return $item } }
    return $null
}
function JC-Raw([string]$text,$node) { return $text.Substring($node.s,$node.e-$node.s) }
function JC-Get([string]$text,[string]$key) {
    $m=JC-Member $text $key
    if ($null -eq $m) { return $null }
    return JC-Raw $text $m.value
}
function JC-Cut([string]$text,$parent,[int]$index) {
    $item=$parent.children[$index]; $start=$item.s; $end=$item.e
    if ($null -ne $item.comma) { $end=$item.comma.e }
    elseif ($index -gt 0) { $start=$parent.children[$index-1].comma.s }
    return $text.Remove($start,$end-$start)
}
function JC-Set([string]$text,[string]$key,$raw) {
    $root=JC-Parse $text
    if ($root.kind -ne '{') { throw 'Expected a JSONC object' }
    for ($i=0;$i -lt $root.children.Count;$i++) {
        $m=$root.children[$i]
        if ($m.key -ceq $key) {
            if ($null -eq $raw) { return JC-Cut $text $root $i }
            return $text.Remove($m.value.s,$m.value.e-$m.value.s).Insert($m.value.s,$raw)
        }
    }
    if ($null -eq $raw) { return $text }
    $nl="`n"; if ($text.Contains("`r`n")) { $nl="`r`n" }
    $at=$root.e-1
    if ($root.children.Count -gt 0) {
        $last=$root.children[-1]
        if ($null -eq $last.comma) { $text=$text.Insert($last.e,','); $at++ }
    }
    return $text.Insert($at,$nl+'    '+(Setup-Json $key)+': '+$raw+$nl)
}
function JC-Append([string]$raw,[string]$item) {
    $node=JC-Parse $raw
    if ($node.kind -ne '[') { throw 'Expected a JSONC array' }
    $at=$node.e-1
    if ($node.children.Count) {
        $last=$node.children[-1]
        if ($null -eq $last.comma) { $raw=$raw.Insert($last.e,','); $at++ }
    }
    return $raw.Insert($at,"`n"+$item+"`n")
}
function Setup-Write([string]$path,[string]$text) {
    $tmp=$path+'.zoi-tmp'
    $fs=New-Object IO.FileStream($tmp,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write)
    try { $bytes=$script:SetupEncoding.GetBytes($text); $fs.Write($bytes,0,$bytes.Length); $fs.Flush($true) }
    finally { $fs.Dispose() }
    if ([IO.File]::Exists($path)) { [IO.File]::Replace($tmp,$path,[NullString]::Value) }
    else { [IO.File]::Move($tmp,$path) }
}
function Setup-Snapshot([string]$p) {
    if ([IO.File]::Exists($p)) { return @{exists=$true; text=(Setup-Read $p)} }
    return @{exists=$false; text=''}
}
function Setup-Same($a,$b) { return $a.exists -eq $b.exists -and $a.text -ceq $b.text }
function Setup-Paths([string]$settings,[string]$tasks) {
    if (-not $settings) { $settings=Join-Path $env:APPDATA 'Code/User/settings.json' }
    $settings=[IO.Path]::GetFullPath($settings)
    if (-not $tasks) { $tasks=Join-Path (Split-Path -Parent $settings) 'tasks.json' }
    $tasks=[IO.Path]::GetFullPath($tasks)
    if ($settings -eq $tasks) { throw 'Settings and tasks must be different files' }
    return @{settings=$settings; tasks=$tasks; state=($settings+'.zoi-state')}
}
function Setup-State([string]$path,[string]$root,$paths) {
    $state=(Setup-Read $path) | ConvertFrom-Json
    if ($state.version -ne 2 -or $state.root -ne $root -or $state.settings -ne $paths.settings -or $state.tasks -ne $paths.tasks -or
        $state.phase -notin @('installing','installed','removing') -or $state.docs.Count -ne 2) {
        throw "Unknown/other installation state preserved: $path"
    }
    if ($state.docs[0].path -ne $paths.settings -or $state.docs[1].path -ne $paths.tasks) { throw 'Invalid state document paths' }
    foreach ($doc in $state.docs) {
        foreach ($key in @('before','after','from','to')) {
            $value=$doc.$key
            if ($value.exists -isnot [bool] -or $value.text -isnot [string]) { throw 'Invalid state snapshot; preserved' }
            if ($value.exists) { $null=JC-Parse $value.text }
        }
    }
    foreach ($dir in $state.dirs) {
        $prefix=[IO.Path]::GetFullPath($dir).TrimEnd('\','/')+[IO.Path]::DirectorySeparatorChar
        if (-not $paths.settings.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase) -and -not $paths.tasks.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid state directory; preserved' }
    }
    return $state
}
function Setup-Recover($state,[string]$sp) {
    if ($state.phase -eq 'installed') { return }
    foreach ($d in $state.docs) {
        $current=Setup-Snapshot $d.path
        if (-not (Setup-Same $current $d.from) -and -not (Setup-Same $current $d.to)) { throw "Config changed during interrupted setup; preserved: $($d.path)" }
    }
    $i=0
    foreach ($d in $state.docs) {
        $current=Setup-Snapshot $d.path
        $tmp=$d.path+'.zoi-tmp'
        if ([IO.File]::Exists($tmp)) { [IO.File]::Delete($tmp) }
        if (-not (Setup-Same $current $d.to)) {
            if ($d.to.exists) { Setup-Write $d.path $d.to.text }
            elseif ([IO.File]::Exists($d.path)) { [IO.File]::Delete($d.path) }
        }
        $i++
        if ($env:ZOI_SETUP_TEST_FAULT -eq "file$i") { throw "Injected setup failure: file$i" }
    }
    if ($state.phase -eq 'installing') {
        $state.phase='installed'; Setup-Write $sp (Setup-Json $state)
    } else {
        [IO.File]::Delete($sp)
        foreach ($dir in @($state.dirs | Sort-Object Length -Descending)) {
            if ([IO.Directory]::Exists($dir) -and @(Get-ChildItem -LiteralPath $dir -Force).Count -eq 0) { [IO.Directory]::Delete($dir) }
        }
    }
}
function Setup-RestoreProperty([string]$current,[string]$before,[string]$after,[string]$key,[string]$owned) {
    $b=JC-Get $before $key; $a=JC-Get $after $key; $c=JC-Get $current $key
    if ($b -ceq $a -or $null -eq $c) { return $current }
    if ((JC-Normal $c) -ceq (JC-Normal $a)) { return JC-Set $current $key $b }
    if ($key -eq 'C_Cpp.default.includePath') {
        $node=JC-Parse $c
        if ($node.kind -ne '[') { throw "Edited $key has unsupported shape; preserved" }
        for ($i=$node.children.Count-1;$i -ge 0;$i--) {
            if ((JC-Value (JC-Raw $c $node.children[$i].value)) -ceq $owned) { $c=JC-Cut $c $node $i; $node=JC-Parse $c }
        }
        if ($null -ne $b) {
            $cv=JC-Value $c; $bv=JC-Value $b
            if ((Setup-Json @($cv)) -ceq (Setup-Json @($bv))) { $c=$b }
        }
        if ($node.children.Count -eq 0 -and $null -eq $b) { $c=$null }
    } else {
        $v=JC-Value $c
        if ($v -isnot [string]) { throw "Edited $key is not a string; preserved" }
        $v=[regex]::Replace($v,'(?:^|\s+)'+[regex]::Escape($owned)+'(?=\s|$)','')
        $c=Setup-Json $v
    }
    return JC-Set $current $key $c
}
function Setup-RemoveTasks([string]$current,[string]$before,[string]$after,$labels) {
    $b=JC-Get $before 'tasks'; $a=JC-Get $after 'tasks'; $c=JC-Get $current 'tasks'
    if ($null -eq $c) { return $current }
    if ((JC-Normal $c) -ceq (JC-Normal $a)) { return JC-Set $current 'tasks' $b }
    $node=JC-Parse $c
    if ($node.kind -ne '[') { throw 'Edited tasks is not an array; preserved' }
    for ($i=$node.children.Count-1;$i -ge 0;$i--) {
        $raw=JC-Raw $c $node.children[$i].value
        $task=JC-Value $raw
        if ($labels -ccontains $task.label) {
            # The explicit uninstall removes these installer-created labels,
            # including edits to their presentation; other labels stay intact.
            $c=JC-Cut $c $node $i; $node=JC-Parse $c
        }
    }
    if ($node.children.Count -eq 0 -and $null -eq $b) { $c=$null }
    return JC-Set $current 'tasks' $c
}
