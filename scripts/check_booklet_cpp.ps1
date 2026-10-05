$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'booklet_cpp.ps1')
function Assert-Cpp([bool]$Ok, [string]$Message) { if (-not $Ok) { throw $Message } }
$source = @'
#ifndef ENGINE
#define ENGINE
#if __cplusplus >= 202302L
FUTURE_DROP
#elif __cplusplus >= 202002L // select the first true branch
#ifdef LOCAL
DEBUG_KEEP
#if __cplusplus < 201703L
OLD_DEBUG_DROP
#else
NEW_DEBUG_KEEP
#endif
#else
RELEASE_KEEP
#endif
#else
OLD_DROP
#endif
#if __cplusplus < 201103L
NO_ELSE_DROP
#endif
#endif
/* Usage
#if __cplusplus >= 202002L
USAGE_KEEP
#else
USAGE_DROP
#endif
*/
'@
$out = Convert-BookletCpp20 $source 'nested fixture'
Assert-Cpp (-not $out.Contains('DROP') -and -not $out.Contains('__cplusplus')) 'Version branch was not fully reduced'
foreach ($word in @('#ifndef ENGINE','#define ENGINE','#ifdef LOCAL','DEBUG_KEEP','NEW_DEBUG_KEEP','RELEASE_KEEP','USAGE_KEEP','/* Usage','*/')) {
    Assert-Cpp ($out.Contains($word)) ('Lost retained text: ' + $word)
}
Assert-Cpp (([regex]::Matches($out,'#endif')).Count -eq 2) 'Unknown guards must stay balanced'
# A shared C++11 core stays outside the optional C++20 adapter branch.
$shared = @'
#ifndef SHARED_ENGINE
#define SHARED_ENGINE
SHARED_CORE_KEEP
#if __cplusplus >= 202002L
SPAN_ADAPTER_KEEP
#endif
/* Usage
VECTOR_USAGE_KEEP
#if __cplusplus >= 202002L
SPAN_USAGE_KEEP
#endif
*/
#endif
'@
$out = Convert-BookletCpp20 $shared 'shared engine fixture'
foreach ($word in @('SHARED_CORE_KEEP','SPAN_ADAPTER_KEEP','VECTOR_USAGE_KEEP','SPAN_USAGE_KEEP')) {
    Assert-Cpp (([regex]::Matches($out,$word)).Count -eq 1) ('Lost or duplicated shared engine text: ' + $word)
}
Assert-Cpp (-not $out.Contains('__cplusplus') -and ([regex]::Matches($out,'#endif')).Count -eq 1) 'Shared core projection damaged its guard'
# Preserve all text except the documented CRLF/CR to LF normalization.
$literal = @'
// #if __cplusplus < 199711L
/*
#if __cplusplus < 199711L
This is documentation, not a directive.
#endif
*/
const char* raw = R"tag(
#if __cplusplus < 199711L
#endif
/* Usage fake */
)tag";
const char* escaped = "\"#if __cplusplus";
int separated = 1'000'000;
char quote = '\'';
// continued comment \
#if __cplusplus < 199711L
#ifdef LOCAL
KEEP
#else
ALSO_KEEP
#endif
'@
$literal = $literal.Replace("`r`n", "`n").Replace("`r", "`n")
foreach ($newline in @("`n", "`r`n", "`r")) {
    $inputText = $literal.Replace("`n", $newline)
    Assert-Cpp ((Convert-BookletCpp20 $inputText) -ceq $literal) 'Comment/literal/LOCAL text was changed beyond newline normalization'
}
$trace = New-Object Collections.ArrayList
$profile = @'
#ifdef ZOI_BOOKLET
void renamed_and_reformatted(auto view) {
#else
void legacy(const char* ptr, int n) {
#endif
SHARED_CORE_KEEP
}
#ifndef ZOI_BOOKLET
LEGACY_DROP
#elif __cplusplus >= 202002L
MODERN_KEEP
#endif
#if defined(ZOI_BOOKLET)
#ifdef LOCAL
DEBUG_KEEP
#endif
#endif
/* Usage
#if !defined ZOI_BOOKLET
USAGE_DROP
#else
USAGE_KEEP
#endif
*/
'@
$out = Convert-BookletCpp20 $profile 'profile fixture' $trace
Assert-Cpp (-not $out.Contains('DROP') -and -not $out.Contains('ZOI_BOOKLET') -and -not $out.Contains('legacy(')) 'Paper branch not reduced'
foreach ($word in @('renamed_and_reformatted(auto view)','SHARED_CORE_KEEP','MODERN_KEEP','#ifdef LOCAL','DEBUG_KEEP','USAGE_KEEP')) {
    Assert-Cpp ($out.Contains($word)) ('Profile dropped shared code: ' + $word)
}
Assert-Cpp ($trace.Count -eq 4 -and $trace[0].source -eq 'profile fixture Usage') 'Missing profile decision trace'
foreach ($directive in @('#ifdef ZOI_BOOKLET', '#if defined ZOI_BOOKLET', '#if defined(ZOI_BOOKLET)', '#if defined ( ZOI_BOOKLET )')) {
    Assert-Cpp ((Convert-BookletCpp20 ($directive+"`nYES`n#else`nNO`n#endif")).Trim() -ceq 'YES') 'Defined form failed'
}
foreach ($pair in @(@('>=',202002,$true),@('>',202002,$false),@('<=',202002,$true),@('<',202002,$false),@('==',202002,$true),@('!=',202002,$false),@('>=',201703,$true),@('>=',202302,$false))) {
    $text = '#if __cplusplus ' + $pair[0] + ' ' + $pair[1] + "L`nYES`n#else`nNO`n#endif"
    Assert-Cpp ((Convert-BookletCpp20 $text).Trim() -ceq $(if ($pair[2]) { 'YES' } else { 'NO' })) 'Comparison evaluation failed'
}
foreach ($bad in @(
    "#if __cplusplus >= 202002L && defined(LOCAL)`nX`n#endif",
    "#if LOCAL`nX`n#elif __cplusplus >= 202002L`nY`n#endif",
    "#ifdef __cplusplus`nX`n#endif",
    "#if __cplusplus >= 202002L`nX",
    '#endif',
    "#if LOCAL`n#else`n#elif OTHER`n#endif",
    "#if __cplusplus >= \`n202002L`n#endif",
    '#define __cplusplus 201103L',
    '#define ZOI_BOOKLET 1',
    '#undef ZOI_BOOKLET',
    "#if ZOI_BOOKLET`nX`n#endif",
    "#if defined(ZOI_BOOKLET) && defined(LOCAL)`nX`n#endif",
    "#ifdef ZOI_BOOKLET`nX`n#elif LOCAL`nY`n#endif"
)) {
    $rejected = $false
    try { $null = Convert-BookletCpp20 $bad 'bad fixture' } catch { $rejected = $true }
    Assert-Cpp $rejected ('Unsafe condition was accepted: ' + $bad)
}
Write-Host '[PASS] contest projection: source profile/version/elif/nested guards/LOCAL/Usage/literals/trace/rejection'
