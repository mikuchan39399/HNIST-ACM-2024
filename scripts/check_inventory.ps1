# Shared scaffold validation and static direct-include inventory.
# ASCII-only for PS5.1. This is an asset relation, never behavior coverage.
function Sort-InventoryPaths($paths) {
    $list = New-Object 'Collections.Generic.List[string]'
    foreach ($p in $paths) { $list.Add([string]$p) }
    $list.Sort([StringComparer]::Ordinal)
    return $list.ToArray()
}
function Get-CheckInventory([string]$root) {
    $root = [IO.Path]::GetFullPath($root).TrimEnd([IO.Path]::DirectorySeparatorChar)
# scaffold sync preflight:
#   1. every catalog target exists          -> [STUB BROKEN]
#   2. every catalog entry has its .h stub  -> [STUB STALE]
#   3. no orphan stub outside catalog       -> [STUB ORPHAN]
#   4. every exemption glob matches >=1 file-> [EXEMPT DEAD]
#   5. every engine cpp is covered (catalog entry or '!' exemption)
#      -> [UNCOVERED]: a new template without a stub cannot slip through
# catalog line formats: "name<TAB>relpath" = stub entry, "!glob" = exemption
$zoiDir = Join-Path $root 'zoi'
$cat = Join-Path $zoiDir '_catalog.txt'
if (-not (Test-Path -LiteralPath $cat)) { throw 'Catalog missing; preflight cannot be skipped.' }
if (Test-Path -LiteralPath $cat) {
    $enc2 = New-Object System.Text.UTF8Encoding($false)
    $entries = @()
    $names = @{}
    $targets = @{}
    $exempt = @()
    $bad = 0
    foreach ($l in [IO.File]::ReadAllLines($cat, $enc2)) {
        $t = $l.Trim()
        if ($t -eq '' -or $t.StartsWith('#')) { continue }
        if ($t.StartsWith('!')) { $exempt += $t.Substring(1).Trim(); continue }
        if ($t.StartsWith('^')) {
            # prose entry (^name<TAB>file): target must exist; no stub, no stamp, no coverage duty
            $pp = $t.Substring(1) -split "`t"
            if ($pp.Count -ne 2 -or -not (Test-Path -LiteralPath (Join-Path $root ($pp[1].Trim())))) {
                Write-Host ('[PROSE BROKEN] prose target missing: ' + $t) -ForegroundColor Red
                $bad++
            }
            if ($pp.Count -eq 2) { $entries += [pscustomobject]@{name=$pp[0].Trim(); path=$pp[1].Trim().Replace('\','/'); kind='note'; suites=@()} }
            continue
        }
        $parts = $t -split "`t"
        if ($parts.Count -ne 2) {
            Write-Host ('[CATALOG BAD] expected name<TAB>path: ' + $t) -ForegroundColor Red
            $bad++
            continue
        }
        $name = $parts[0].Trim()
        $rel = $parts[1].Trim()
        if ($name -eq '' -or $rel -eq '' -or $rel.Contains("`t")) {
            Write-Host ('[CATALOG BAD] empty field: ' + $t) -ForegroundColor Red
            $bad++
            continue
        }
        if ($names.ContainsKey($name) -or $targets.ContainsKey($rel.Replace('\', '/'))) {
            Write-Host ('[CATALOG DUPLICATE] ' + $t)
            $bad++
        }
        $names[$name] = $true
        if (-not (Test-Path -LiteralPath (Join-Path $root $rel))) {
            Write-Host ('[STUB BROKEN] catalog target missing: ' + $rel) -ForegroundColor Red
            $bad++
        }
        if (-not (Test-Path -LiteralPath (Join-Path $zoiDir ($name + '.h')))) {
            Write-Host ('[STUB STALE] stub file missing, rerun make_stubs: ' + $name) -ForegroundColor Red
            $bad++
        }
        $stubPath = Join-Path $zoiDir ($name + '.h')
        if (Test-Path -LiteralPath $stubPath) {
            $stubLines = [IO.File]::ReadAllLines($stubPath, $enc2)
            $expected = '#include "../' + $rel + '"'
            $actual = @($stubLines | Where-Object { $_ -match '^\s*#\s*include' })
            if ($actual.Count -ne 1 -or $actual[0].Trim() -cne $expected) {
                Write-Host ('[STUB STALE] wrong include target: ' + $name)
                $bad++
            }
        }
        $entries += [pscustomobject]@{name=$name; path=$rel.Replace('\','/'); kind='engine'; suites=@()}
        $relN = $rel.Replace('\', '/')
        $targets[$relN] = $true
        # 7th check data: engine first line must be '// zoi: <name>' (stamp)
        $engFirst = ''
        $engPath = Join-Path $root ($rel.Replace('\', '/'))
        if (Test-Path -LiteralPath $engPath) { $engFirst = ([IO.File]::ReadAllLines($engPath, $enc2) | Select-Object -First 1) }
        if ($engFirst -ne ('// zoi: ' + $name)) {
            Write-Host ('[STUB MISMATCH] engine stamp wrong or missing (rerun make_stubs): ' + $rel + ' -> ' + $name) -ForegroundColor Red
            $bad++
        }
    }
    foreach ($h in Get-ChildItem -LiteralPath $zoiDir -Filter '*.h') {
        if (-not $names.ContainsKey($h.BaseName)) {
            Write-Host ('[STUB ORPHAN] stub not in catalog: ' + $h.BaseName) -ForegroundColor Red
            $bad++
        }
    }
    $engines = @()
    foreach ($cpp in Get-ChildItem -LiteralPath (Join-Path $root 'algorithms') -Recurse -Filter '*.cpp') {
        if ($cpp.Name.EndsWith('_check.cpp')) { continue }
        $r = $cpp.FullName.Substring($root.Length + 1).Replace('\', '/')
        $engines += $r
    }
    foreach ($e in $exempt) {
        $hit = $false
        foreach ($r in $engines) { if ($r -like $e) { $hit = $true; break } }
        if (-not $hit) {
            Write-Host ('[EXEMPT DEAD] exemption matches nothing: ' + $e) -ForegroundColor Red
            $bad++
        }
    }
    foreach ($r in $engines) {
        if ($targets.ContainsKey($r)) { continue }
        $hit = $false
        foreach ($e in $exempt) { if ($r -like $e) { $hit = $true; break } }
        if (-not $hit) {
            Write-Host ('[UNCOVERED] no stub and no exemption: ' + $r) -ForegroundColor Red
            $bad++
        }
    }
    if ($targets.Count -eq 0) { throw 'Catalog has no engines.' }
    if ($bad -gt 0) { throw ('scaffold sync failed: ' + $bad + ' problem(s)') }
    Write-Host ('[OK] scaffold sync: ' + $targets.Count.ToString() + ' stubs checked, ' + $exempt.Count.ToString() + ' exemptions, catalog coverage')
}

    $comparison = [StringComparer]::Ordinal
    if ([IO.Path]::DirectorySeparatorChar -eq '\') { $comparison = [StringComparer]::OrdinalIgnoreCase }
    $byPath = New-Object 'Collections.Generic.Dictionary[string,object]' ($comparison)
    foreach ($e in $entries) {
        $key=[IO.Path]::GetFullPath((Join-Path $root $e.path))
        if ($byPath.ContainsKey($key)) { throw "Duplicate canonical target: $($e.path)" }
        $byPath[$key]=$e
        if ($e.kind -eq 'engine') { $byPath[[IO.Path]::GetFullPath((Join-Path $zoiDir ($e.name+'.h')))]=$e }
    }
    foreach ($r in (Sort-InventoryPaths $engines)) {
        $key=[IO.Path]::GetFullPath((Join-Path $root $r))
        if (-not $byPath.ContainsKey($key)) {
            $e=[pscustomobject]@{name=''; path=$r; kind='exempt'; suites=@()}
            $entries += $e; $byPath[$key]=$e
        }
    }
    $suitePaths=@(Sort-InventoryPaths @(Get-ChildItem -LiteralPath (Join-Path $root 'algorithms') -Recurse -Filter '*_check.cpp' | ForEach-Object { $_.FullName.Substring($root.Length+1).Replace('\','/') }))
    foreach ($suite in $suitePaths) {
        $file=Join-Path $root $suite
        $text=[IO.File]::ReadAllText($file,$enc2)
        $text=$text -replace '\\\r?\n',''
        # Ignore commented-out directives and raw-string examples. Conditional
        # includes are still static references, not claims that a branch ran.
        $pattern='(?s)R"(?<delim>[^ ()\\\t\r\n]{0,16})\(.*?\)\k<delim>"|"(?:\\.|[^"\\\r\n])*"|''(?:\\.|[^''\\\r\n])*''|/\*.*?\*/|//[^\r\n]*'
        $text=[regex]::Replace($text,$pattern,[Text.RegularExpressions.MatchEvaluator]{param($m)
            if ($m.Value.StartsWith('/') -or $m.Value.StartsWith('R"')) { return [regex]::Replace($m.Value,'[^\r\n]',' ') }
            return $m.Value
        })
        foreach ($m in [regex]::Matches($text,'(?m)^\s*#\s*include\s*"([^"]+)"')) {
            $inc=$m.Groups[1].Value.Replace('\','/')
            $candidates=@((Join-Path (Split-Path -Parent $file) $inc),(Join-Path $zoiDir $inc),(Join-Path $root $inc))
            if ([IO.Path]::IsPathRooted($inc)) { $candidates=@($inc) }
            foreach ($candidate in $candidates) {
                $key=[IO.Path]::GetFullPath($candidate)
                if ([IO.File]::Exists($key)) {
                    if ($byPath.ContainsKey($key)) {
                        $e=$byPath[$key]
                        if ($e.suites -cnotcontains $suite) { $e.suites += $suite }
                    }
                    break
                }
            }
        }
    }
    return [pscustomobject]@{entries=@($entries); checks=$suitePaths; engines=@(Sort-InventoryPaths $engines); exemptions=@($exempt)}
}
