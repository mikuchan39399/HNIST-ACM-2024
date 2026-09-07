# Directory discovery is independent of catalog membership.
# Catalog controls printable source identities, not whether a chapter exists.
function Get-BookletTree([string]$Root, $Entries, $Plugins, [string]$Filter) {
    $nodes=@{}
    $testName=-join ([char]0x5BF9,[char]0x62CD)
    foreach ($d in Get-ChildItem -LiteralPath (Join-Path $Root 'algorithms') -Recurse -Directory) {
        $rel=$d.FullName.Substring($Root.Length+1).Replace('\','/')
        $parts=$rel -split '/'
        if ($parts -contains $testName -or @($parts | Where-Object { $_.StartsWith('.') }).Count) { continue }
        $nodes[$rel]=[pscustomobject]@{Rel=$rel;Name=$d.Name;Depth=$parts.Count-1;Rank=[int]::MaxValue;Children=@();Entries=@();Selected=(!$Filter)}
    }
    $index=0
    foreach ($e in @($Entries)+@($Plugins)) {
        $dir=$e.Rel -replace '/[^/]+$',''
        if (-not $nodes.ContainsKey($dir)) { throw ('Printable source has no algorithm directory: '+$e.Rel) }
        $nodes[$dir].Entries+=,$e
        for ($ancestor=$dir; $nodes.ContainsKey($ancestor); $ancestor=$ancestor -replace '/[^/]+$','') {
            $nodes[$ancestor].Rank=[Math]::Min($nodes[$ancestor].Rank,$index)
        }
        $index++
    }
    $selectedFiles=@{}
    foreach ($node in $nodes.Values) {
        $match=(!$Filter) -or ($node.Rel -like "*$Filter*")
        foreach ($e in $node.Entries) {
            if ($match -or $e.Name -like "*$Filter*" -or $e.Cn -like "*$Filter*") {
                $selectedFiles[$e.Rel]=$true; $node.Selected=$true
            }
        }
        if ($match) { $node.Selected=$true }
    }
    # Keep the full ancestor chain, but do not pull in unrelated siblings.
    foreach ($rel in @($nodes.Keys | Where-Object { $nodes[$_].Selected })) {
        for ($p=$rel -replace '/[^/]+$',''; $nodes.ContainsKey($p); $p=$p -replace '/[^/]+$','') { $nodes[$p].Selected=$true }
    }
    $roots=@()
    foreach ($node in $nodes.Values) {
        if (-not $node.Selected) { continue }
        $node.Entries=@($node.Entries | Where-Object { $selectedFiles.ContainsKey($_.Rel) })
        $parent=$node.Rel -replace '/[^/]+$',''
        if ($nodes.ContainsKey($parent)) { $nodes[$parent].Children+=,$node }
        else { $roots+=,$node }
    }
    if (-not $roots.Count) { throw 'No algorithm directory or source matched' }
    [pscustomobject]@{Roots=@($roots | Sort-Object Rank,Rel -Culture zh-CN);Nodes=$nodes;Files=$selectedFiles}
}

function Flush-BookletHeadings($Builder) {
    [void]$Builder.Append($script:pendingHeadings.ToString())
    [void]$script:pendingHeadings.Clear()
}

function Write-BookletNode($Node, $Builder, $BlockByPath, $PluginLabels, [int]$SoloMin) {
    $sharedTitle=$Node.Entries.Count -eq 1 -and $Node.Entries[0].Cn -eq $Node.Name
    if ($Node.Depth -eq 1) { $script:newDomain=$true }
    $label='dir-'+$script:printedDirs.Count
    if ($sharedTitle) {
        $e=$Node.Entries[0]
        $label=if ($PluginLabels.ContainsKey($e.Rel)) { $PluginLabels[$e.Rel] } else { 'e-'+$e.Name }
    }
    # Delay ancestor headings until their first entry/empty leaf opens its page.
    # Otherwise an entry page break strands the family heading on the prior page.
    [void]$script:pendingHeadings.AppendLine('// directory: '+$Node.Rel)
    [void]$script:pendingHeadings.AppendLine('#metadata('+(Typ-String ('directory:'+ $Node.Rel))+')')
    [void]$script:pendingHeadings.AppendLine('#heading(level: '+$Node.Depth+', '+(Typ-String $Node.Name)+') <'+$label+'>')
    $script:printedDirs[$Node.Rel]=$true
    if (-not $Node.Entries.Count -and -not $Node.Children.Count) {
        # Future entries share a separate run; leave room between named slots.
        if ($script:newDomain -or $script:lastBlockKind -eq 'entry') {
            $break=if ($script:newDomain -and $SoloMin -gt 0) { '#pagebreak(to: "odd", weak: true)' } else { '#pagebreak(weak: true)' }
            [void]$Builder.AppendLine($break)
        }
        Flush-BookletHeadings $Builder
        [void]$Builder.AppendLine('#line(length: 30%, stroke: 0.25pt + luma(210))')
        [void]$Builder.AppendLine('#v(8pt)')
        $script:newDomain=$false; $script:lastBlockKind='empty'
    }
    foreach ($e in $Node.Entries) {
        $b=$BlockByPath[$e.Rel]
        $odd=$SoloMin -gt 0 -and ($script:newDomain -or $b.Lines -ge $SoloMin)
        [void]$Builder.AppendLine($(if ($odd) { '#pagebreak(to: "odd", weak: true)' } else { '#pagebreak(weak: true)' }))
        Flush-BookletHeadings $Builder
        $anchor=if ($PluginLabels.ContainsKey($e.Rel)) { $PluginLabels[$e.Rel] } else { 'e-'+$e.Name }
        if (-not $sharedTitle) { [void]$Builder.AppendLine('#heading(level: '+($Node.Depth+1)+', '+(Typ-String $e.Cn)+') <'+$anchor+'>') }
        [void]$Builder.AppendLine('// entry: '+$e.Rel)
        $stub=if ($e.PSObject.Properties['Domain'] -and -not $e.Prose) { $e.Name+'.h | ' } else { '' }
        [void]$Builder.AppendLine('#entrymeta['+(Esc $stub)+$b.Lines+(Zh '20 884c 20 7c 20 53 48 41 32 35 36 20')+$b.Hash+']')
        if ($e.PSObject.Properties['Prose'] -and $e.Prose) { [void]$Builder.AppendLine((Esc $b.Text)) }
        else { [void]$Builder.AppendLine('#raw('+(Typ-String $b.Text)+', block: true, lang: "cpp")') }
        [void]$Builder.AppendLine('')
        Append-Manual $Builder $b
        [void]$Builder.AppendLine('#metadata('+(Typ-String ('entry-end:'+ $anchor))+')')
        $script:newDomain=$false; $script:lastBlockKind='entry'
    }
    foreach ($child in @($Node.Children | Sort-Object Rank,Rel -Culture zh-CN)) { Write-BookletNode $child $Builder $BlockByPath $PluginLabels $SoloMin }
}
