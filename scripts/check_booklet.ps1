param([switch]$Render)
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot 'check_process.ps1')
. (Join-Path $PSScriptRoot 'booklet_markdown.ps1')
$fixture=Join-Path $root ('.zoi-checks/booklet-test-'+[Guid]::NewGuid().ToString('N'))
$enc=New-Object Text.UTF8Encoding($false)
function Put([string]$Rel,[string]$Text) {
    $p=Join-Path $fixture $Rel
    [void][IO.Directory]::CreateDirectory((Split-Path -Parent $p))
    [IO.File]::WriteAllText($p,$Text,$enc)
}
function Assert([bool]$Ok,[string]$Message) { if (-not $Ok) { throw $Message } }
Put 'scripts/placeholder' ''
foreach ($name in @('make_booklet.ps1','booklet_markdown.ps1','booklet_tree.ps1')) { Copy-Item -LiteralPath (Join-Path $PSScriptRoot $name) -Destination (Join-Path $fixture ('scripts/'+$name)) }
$compiler=(Get-Command typst -ErrorAction SilentlyContinue | Select-Object -First 1).Source
if (-not $compiler -and (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'typst.exe'))) { $compiler=Join-Path $PSScriptRoot 'typst.exe' }
if ($Render -and -not $compiler) { throw 'Render tests require Typst' }
$catalog="a`talgorithms/domain/family/a.cpp`nb`talgorithms/domain/family/b.cpp`nc`talgorithms/domain/other/a.cpp`n"
Put 'zoi/_catalog.txt' $catalog
Put 'algorithms/domain/family/a.cpp' "#include `"../other/a.cpp`"`nint SOURCE_A;"
Put 'algorithms/domain/family/b.cpp' 'int SOURCE_B; // ``` #not-executable'
Put 'algorithms/domain/other/a.cpp' 'int SOURCE_C;'
Put 'algorithms/README.md' '# ROOT_ROADMAP_MUST_NOT_PRINT'
Put 'algorithms/domain/README.md' '# ANCESTOR_MUST_NOT_PRINT'
Put 'algorithms/domain/family/nested/README.md' '# DESCENDANT_MUST_NOT_PRINT'
Put 'algorithms/domain/empty/README.md' '# EMPTY_MUST_NOT_PRINT'
Put 'docs/roadmaps/domain.md' '# ROADMAP_MUST_NOT_PRINT'
Put 'docs/booklet/captions.tsv' 'OLD_CAPTION_MUST_NOT_PRINT'
$testFolder=-join ([char]0x5BF9,[char]0x62CD)
Put ('algorithms/domain/'+$testFolder+'/check.cpp') 'int TEST_SOURCE_MUST_NOT_PRINT;'
$pluginFolder=-join ([char]0x63D2,[char]0x4EF6)
Put ('algorithms/domain/'+$pluginFolder+'/sample.cpp') "int PLUGIN_KEEP; /* Usage: int main() {} */"
Put ('algorithms/domain/'+$pluginFolder+'/solution.cpp') 'int main() { return 0; } // SOLUTION_MUST_NOT_PRINT'

$manual=@'
# SHARED_MANUAL

Paragraph **BOLD_KEEP** and `CODE_KEEP`, [LINK_KEEP](<a.cpp>) #not-code $ [ ] \.

Inline $x_i^2+1$ MATH_INLINE_KEEP; code `$literal$`.

$$\sum_{i=1}^{n}i=\frac{n(n+1)}{2}$$

$$
R=\left\lceil\frac{D}{2}\right\rceil
$$

MATH_DISPLAY_KEEP

## LIST_HEADING

1. NUMBERED_ONE_KEEP
2. NUMBERED_TWO_KEEP
- BULLET_KEEP

| Column | Meaning |
|---|---|
| `a|b` | TABLE_KEEP |
| escaped \| pipe | SECOND_ROW_KEEP |
| $|x|$ | MATH_TABLE_KEEP |

```cpp
int FENCED_KEEP;
// ``` embedded ticks must not end the source block
```

LAST_MANUAL_KEEP
'@
Put 'algorithms/domain/family/README.md' $manual
. (Join-Path $PSScriptRoot 'booklet_markdown.ps1')
# Non-ASCII digits are outside this TeX subset and must terminate with an error.
$rejected=$false
try { $null=Convert-BookletMath ([string][char]0xFF11) } catch { $rejected=$true }
Assert $rejected 'Unsupported Unicode digit was accepted'
$hierarchy='algorithms'
foreach ($depth in 1..7) { $hierarchy+='/DEPTHLEVEL'+$depth }
[void][IO.Directory]::CreateDirectory((Join-Path $fixture $hierarchy))
$script:calls=0
function Build([string]$Filter='',[int]$Expected=0,[int]$Solo=0,[string]$Output='out/book.pdf') {
    $script:calls++
    $arguments=@('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $fixture 'scripts/make_booklet.ps1'),'-OutFile',$Output)
    if (-not $Render) { $arguments+='-SourceOnly' }
    elseif ($compiler) { $arguments+=@('-TypstPath',$compiler) }
    if ($Filter) { $arguments+=@('-Filter',$Filter) }
    if ($Solo) { $arguments+=@('-SoloMin',[string]$Solo) }
    $r=@(Invoke-CheckProcess (Get-Process -Id $PID).Path $arguments $fixture 120 (Join-Path $fixture ('build-'+$script:calls)))[-1]
    Assert (-not $r.TimedOut -and $r.ExitCode -eq $Expected) ('Unexpected build result '+$script:calls+'; inspect '+$fixture)
    if ($Expected -eq 0) { return [IO.File]::ReadAllText((Join-Path $fixture ([IO.Path]::ChangeExtension($Output,'.typ'))),$enc) }
}
$s=Build
Assert (([regex]::Matches($s,'// manual: ')).Count -eq 1) 'Shared manual repeated or missing'
Assert ($s.IndexOf('SOURCE_B') -lt $s.IndexOf('// manual: ') -and $s.IndexOf('// manual: ') -lt $s.IndexOf('SOURCE_C')) 'Manual detached from its source family'
Assert (-not $s.Contains('MUST_NOT_PRINT')) 'Ancestor, descendant or roadmap leaked into booklet'
Assert ($s.Contains('PLUGIN_KEEP')) 'Commented Usage main hid a real plugin'
Assert ($s.Contains('#include \"c.h\"')) 'Filtered include map confused equal basenames'
foreach ($marker in @('BOLD_KEEP','CODE_KEEP','LINK_KEEP','NUMBERED_ONE_KEEP','NUMBERED_TWO_KEEP','BULLET_KEEP','TABLE_KEEP','SECOND_ROW_KEEP','FENCED_KEEP','LAST_MANUAL_KEEP')) {
    Assert ($s.Contains($marker)) ('Markdown content lost: '+$marker)
}
Assert ($s.Contains('#table(') -and $s.Contains('#strong[')) 'Markdown formatting not rendered'
Assert ($s.Contains('frac(') -and $s.Contains('lr(ceil.l') -and $s.Contains('sum _(i = 1) ^(n)') -and $s.Contains('"$literal$"')) 'Math structure or code isolation broken'
Assert ($s.Contains('// directory: algorithms/domain/empty') -and $s.Contains('// directory: algorithms/domain/family/nested')) 'Empty/nested directory was omitted'
Assert (-not $s.Contains('outlined: false, "a"') -and $s.Contains('// entry: algorithms/domain/family/a.cpp') -and $s.Contains('// entry: algorithms/domain/family/b.cpp')) 'Sibling sources were hidden from contents'
Assert ($s.IndexOf('#pagebreak(weak: true)', $s.IndexOf('SOURCE_A')) -lt $s.IndexOf('SOURCE_B')) 'Sibling entries lost independent page starts'
Write-Host '[PASS] adjacent discovery / shared once after siblings / roadmap exclusion / Markdown content and structure'
$s=Build 'b'
Assert ($s.Contains('SOURCE_B') -and -not $s.Contains('SOURCE_A') -and $s.Contains('SHARED_MANUAL')) 'Filtered sibling lost its manual'
$s=Build 'empty'
Assert ($s.Contains('// directory: algorithms/domain/empty') -and -not $s.Contains('SOURCE_A') -and -not $s.Contains('SHARED_MANUAL')) 'Directory-only preview requires source or leaks a sibling'
[void][IO.Directory]::CreateDirectory((Join-Path $fixture 'algorithms/new-domain/new-family/new-leaf'))
$s=Build
Assert ($s.Contains('// directory: algorithms/new-domain/new-family/new-leaf')) 'New directory required registration'
[IO.Directory]::Move((Join-Path $fixture 'algorithms/new-domain/new-family/new-leaf'),(Join-Path $fixture 'algorithms/new-domain/new-family/renamed-leaf'))
$s=Build 'new-domain'
Assert ($s.Contains('/renamed-leaf') -and -not $s.Contains('/new-leaf') -and -not $s.Contains('SOURCE_A')) 'Renamed directory was stale or preview leaked sources'
Put 'algorithms/domain/solo/solo.cpp' ((1..100 | ForEach-Object { '// SOLO_'+$_ }) -join "`n")
Put 'zoi/_catalog.txt' ($catalog+"solo`talgorithms/domain/solo/solo.cpp`n")
$s=Build 'solo' 0 90
Assert (([regex]::Matches($s,'#heading\([^\r\n]*"solo"')).Count -eq 1 -and $s.Contains('<e-solo>')) 'Equal directory/source titles repeated or lost anchor'
Put 'zoi/_catalog.txt' $catalog
$s=Build 'other'

Assert (-not $s.Contains('SHARED_MANUAL') -and $s.Contains('SOURCE_C')) 'Unrelated manual leaked into filtered build'
Build 'no-such-entry' 1
Build 'b' 1 0 'docs/booklet/output/zoi-booklet-print.pdf'
Write-Host '[PASS] scoped selection / no match failure / canonical preview protection'
Put 'algorithms/domain/other/README.md' '# NEW_MANUAL_AUTO'
$s=Build
Assert ($s.Contains('NEW_MANUAL_AUTO') -and ([regex]::Matches($s,'// manual: ')).Count -eq 2) 'New manual required manual registration'
Put 'algorithms/domain/other/README.md' "``````cpp`nunterminated"
Build '' 1
foreach ($badMath in @('$$\unknown{x}$$','$$\frac{n}{2$$','$$\left(x\right]$$','$$#read("private")$$','$$')) {
    Put 'algorithms/domain/other/README.md' $badMath
    Build '' 1
}
[IO.File]::Delete((Join-Path $fixture 'algorithms/domain/other/README.md'))
Put 'algorithms/domain/family/a.cpp' ((1..1200 | ForEach-Object { '// GROWTH_'+$_ }) -join "`n")
$s=Build '' 0 90
Assert ($s.Contains('GROWTH_1200') -and $s.IndexOf('GROWTH_1200') -lt $s.IndexOf('SOURCE_B') -and $s.Contains('#pagebreak(to: "odd"')) 'Growth truncated source or broke parity option'
Assert (-not $s.Contains('NEW_MANUAL_AUTO')) 'Deleted manual remained in output'
Write-Host '[PASS] new manual auto-discovery / malformed Markdown rejects / 1200-line growth / odd-page policy'
Write-Host ('Booklet self-test: '+$script:calls+' builds passed; render='+$Render+'; logs: '+$fixture)
Complete-CheckWorkspace $fixture 'tooling'
