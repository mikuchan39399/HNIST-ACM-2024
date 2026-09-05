param([string]$Compiler = 'g++', [string]$BuildRoot = '')
# Isolated behavioral tests; never reads or restores a user's problem files.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (-not $BuildRoot) { $BuildRoot=Join-Path $root '.zoi-checks' }
$fixture=Join-Path ([IO.Path]::GetFullPath($BuildRoot)) ('expand-test-'+[Guid]::NewGuid().ToString('N'))
$enc=New-Object Text.UTF8Encoding($false)
foreach ($d in @('scripts','zoi','headers','other','problems')) { [void][IO.Directory]::CreateDirectory((Join-Path $fixture $d)) }
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'zoi.ps1') -Destination (Join-Path $fixture 'scripts/zoi.ps1')
. (Join-Path $PSScriptRoot 'check_process.ps1')
$hostExe=(Get-Process -Id $PID).Path
$script:calls=0; $script:passed=0
function Put([string]$p,[string]$s) { [IO.File]::WriteAllText((Join-Path $fixture $p),$s,$enc) }
function Read([string]$p) { return $enc.GetString([IO.File]::ReadAllBytes((Join-Path $fixture $p))) }
function Assert([bool]$ok,[string]$message) { if (-not $ok) { throw $message } }
function Equal([string]$a,[string]$b) { Assert ($a -ceq $b) "Text mismatch: expected <$b>, actual <$a>" }
function Run([string]$action,[string]$p,[int]$code=0,[string]$pattern='', [switch]$Force, [switch]$Real) {
    $script:calls++
    $tool=Join-Path $fixture 'scripts/zoi.ps1'; if ($Real) { $tool=Join-Path $PSScriptRoot 'zoi.ps1' }
    $argsForRun=@('-NoProfile','-ExecutionPolicy','Bypass','-File',$tool,$action,(Join-Path $fixture $p),'-NoClipboard')
    if ($Force) { $argsForRun+='-Force' }
    $log=Join-Path $fixture ('call-'+$script:calls)
    $r=Invoke-CheckProcess $hostExe $argsForRun $fixture 30 $log
    Assert (-not $r.TimedOut -and $r.ExitCode -eq $code) "Command failed: $action $p (expected $code). See $log"
    if ($pattern) {
        $output=[IO.File]::ReadAllText($log+'.stdout.log')
        Assert ($output -match $pattern) "Expected '$pattern': $output"
    }
}
function Clean([string]$name) {
    foreach ($ext in @('.zoi.cpp','.zoi.sha','.zoi.state.json','.zoi.pending.json','.cpp.zoi.lock','.cpp.zoi-write.tmp','.zoi.state.json.zoi-write.tmp')) {
        Assert (-not [IO.File]::Exists((Join-Path $fixture ('problems/'+$name+$ext)))) "Leftover $name$ext"
    }
}
function Pass([string]$s) { $script:passed++; Write-Host "[PASS] $s" }
function Compile-Run([string]$p) {
    $script:calls++; $base=Join-Path $fixture ('build-'+$script:calls); $exe=$base+'.exe'
    $r=Invoke-CheckProcess $Compiler @('-std=c++20','-Wall','-Wextra','-Werror','-I',(Join-Path $fixture 'zoi'),'-I',(Join-Path $root 'zoi'),(Join-Path $fixture $p),'-o',$exe) $fixture 60 $base
    Assert (-not $r.TimedOut -and $r.ExitCode -eq 0) "Compile failed: $base"
    $r=Invoke-CheckProcess $exe @() $fixture 10 ($base+'-run')
    Assert (-not $r.TimedOut -and $r.ExitCode -eq 0) "Runtime failed: $base"
}
Put 'headers/d.h' "#ifndef D_H`n#define D_H`ninline int diamond() { return 7; }`n#endif`n"
Put 'headers/b.h' "#pragma once`n#include `"d.h`"`ninline int left() { return diamond(); }`n"
Put 'headers/c.h' "#ifndef C_H`n#define C_H`n#include `"./d.h`"`ninline int right() { return diamond(); }`n#endif`n"
Put 'headers/e.h' "#pragma once`ninline int extra() { return 3; }`n"
foreach ($n in @('b','c','e')) { Put ('zoi/'+$n+'.h') ("// zoi stub -> headers/$n.h`n#include `"../headers/$n.h`"`n") }
$original="// keep this comment`n#include `"b.h`" // first`n`n#include `"c.h`"`n#include `"b.h`"`nint main() { return left()+right() == 14 ? 0 : 1; }`n"
Put 'problems/A.cpp' $original
Compile-Run 'problems/A.cpp'
Run expand 'problems/A.cpp'
Assert (([regex]::Matches((Read 'problems/A.cpp'),'inline int diamond')).Count -eq 1) 'Diamond was duplicated'
Compile-Run 'problems/A.cpp'
$expanded=Read 'problems/A.cpp'; $state=Read 'problems/A.zoi.state.json'
$stamp=(Get-Item -LiteralPath (Join-Path $fixture 'problems/A.cpp')).LastWriteTimeUtc
Run expand 'problems/A.cpp'
Equal (Read 'problems/A.cpp') $expanded
Equal (Read 'problems/A.zoi.state.json') $state
Assert ((Get-Item -LiteralPath (Join-Path $fixture 'problems/A.cpp')).LastWriteTimeUtc -eq $stamp) 'Idempotent expand rewrote source'
Run status 'problems/A.cpp' 0 'recoverable'
Run restore 'problems/A.cpp'; Equal (Read 'problems/A.cpp') $original; Clean A
Pass 'diamond / forwarding aliases / duplicate includes / exact restore / idempotence'

Run expand 'problems/A.cpp'
Put 'problems/A.cpp' ((Read 'problems/A.cpp').Replace('== 14','== 14 /* edited solve */')+"#include `"e.h`"`n")
Run expand 'problems/A.cpp'
Assert ((Read 'problems/A.cpp').Contains('inline int extra')) 'New include was not expanded'
Compile-Run 'problems/A.cpp'
Run restore 'problems/A.cpp'
Equal (Read 'problems/A.cpp') ($original.Replace('== 14','== 14 /* edited solve */')+"#include `"e.h`"`n")
Clean A; Pass 'add include after expansion; preserve solve edits'

Put 'problems/A.cpp' $original; Run expand 'problems/A.cpp'
$current=Read 'problems/A.cpp'
$blockPattern=New-Object regex('(?s)// zoi:begin [^\r\n]+\n.*?// zoi:end [^\r\n]+\n')
$current=$blockPattern.Replace($current,'',1)
Put 'problems/A.cpp' $current; Run expand 'problems/A.cpp'
Assert (([regex]::Matches((Read 'problems/A.cpp'),'inline int diamond')).Count -eq 1) 'Shared dependency disappeared after deletion'
Run restore 'problems/A.cpp'; Clean A
Pass 'delete a complete block then rebuild shared dependencies'

Put 'problems/A.cpp' $original; Run expand 'problems/A.cpp'
Put 'headers/d.h' ( (Read 'headers/d.h').Replace('return 7','return 8') )
Run expand 'problems/A.cpp' 0 '\[UPDATE\]'
Assert ((Read 'problems/A.cpp').Contains('return 8')) 'Dependency update missing'
Run restore 'problems/A.cpp'; Equal (Read 'problems/A.cpp') $original
Put 'headers/d.h' ((Read 'headers/d.h').Replace('return 8','return 7'))
Pass 'library changes refreshed without changing compact source'

Run expand 'problems/A.cpp'
Put 'problems/A.cpp' ((Read 'problems/A.cpp').Replace('return 7','return 9'))
$edited=Read 'problems/A.cpp'
Run expand 'problems/A.cpp' 1 'Edited generated block'
Run restore 'problems/A.cpp' 1 'Edited generated block'; Equal (Read 'problems/A.cpp') $edited
Run forget 'problems/A.cpp'
Assert ((Read 'problems/A.cpp').Contains('return 9') -and -not (Read 'problems/A.cpp').Contains('// zoi:')) 'Forget lost edits or kept markers'
Clean A; Pass 'edited template conflict; forget preserves customization'

foreach ($kind in @('missing','duplicate','malformed')) {
    Put 'problems/A.cpp' $original; Run expand 'problems/A.cpp'
    $s=Read 'problems/A.cpp'
    if ($kind -eq 'missing') { $s=[regex]::Replace($s,'(?m)^// zoi:end[^\n]*\n','') }
    if ($kind -eq 'duplicate') { $s=$s+$s }
    if ($kind -eq 'malformed') { $s=$s.Replace('// zoi:begin','// zoi:begin broken') }
    Put 'problems/A.cpp' $s; Run restore 'problems/A.cpp' 1; Equal (Read 'problems/A.cpp') $s
    Run restore 'problems/A.cpp' -Force; Equal (Read 'problems/A.cpp') $original; Clean A
}
Pass 'marker damage rejected; explicit Force restores snapshot'

$unicode='problems/'+[char]0x9898+' space.cpp'
$text=[string][char]0xfeff+"#include `"b.h`"`r`n// root`r`n#include `"c.h`""
Put $unicode $text; Run expand $unicode
Put $unicode ((Read $unicode).Replace("`r`n","`n"))
Run restore $unicode; Equal (Read $unicode) $text.Replace('// root'+"`r`n",'// root'+"`n")
Pass 'Unicode and spaces; BOM; CRLF/LF normalization; no final newline'

Put 'headers/repeat.h' "value += 1;`n"
Put 'other/d.h' "#pragma once`ninline int other() { return 2; }`n"
Put 'problems/path.cpp' "#include `"b.h`"`n#include `"../headers/../headers/d.h`"`n#include `"../other/d.h`"`nint main() { int value=0;`n#include `"../headers/repeat.h`"`n#include `"../headers/repeat.h`"`nreturn value == 2 && other() == 2 ? 0 : 1; }`n"
Compile-Run 'problems/path.cpp'; Run expand 'problems/path.cpp'; Compile-Run 'problems/path.cpp'
Assert (([regex]::Matches((Read 'problems/path.cpp'),'inline int diamond')).Count -eq 1) 'Canonical alias duplicated'
Run restore 'problems/path.cpp'
Pass 'canonical paths / same basename distinct files / unguarded repeat'

Put 'headers/x.h' "#ifndef X_H`n#define X_H`n#include `"y.h`"`n#endif`n"
Put 'headers/y.h' "#ifndef Y_H`n#define Y_H`n#include `"x.h`"`n#endif`n"
Put 'problems/cycle.cpp' "#include `"../headers/x.h`"`nint main() {}"
Run expand 'problems/cycle.cpp'; Compile-Run 'problems/cycle.cpp'; Run restore 'problems/cycle.cpp'
Put 'headers/x.h' "#include `"y.h`"`n"; Put 'headers/y.h' "#include `"x.h`"`n"
Run expand 'problems/cycle.cpp' 1 'cycle'; Clean cycle
Pass 'guarded cycle terminates; unguarded cycle rejects before write'

Put 'zoi/x.h' "// zoi stub -> headers/x.h`n#include `"../headers/x.h`"`n"
Put 'headers/x.h' "#pragma once`n#include `"../zoi/x.h`"`ninline int cycle_ok() { return 0; }`n"
Put 'problems/alias.cpp' "#include `"x.h`"`nint main() { return cycle_ok(); }`n"
Run expand 'problems/alias.cpp'; Compile-Run 'problems/alias.cpp'; Run restore 'problems/alias.cpp'; Clean alias
Put 'zoi/x.h' "// zoi stub -> zoi/x.h`n#include `"x.h`"`n"
Run expand 'problems/alias.cpp' 1 'Forwarding cycle'; Clean alias
Put 'zoi/nonpure.h' "// zoi stub -> headers/b.h`n#include `"../headers/b.h`"`ninline int wrapper() { return 0; }`n"
Put 'problems/wrapper.cpp' "#include `"nonpure.h`"`nint main() { return wrapper(); }"
Run expand 'problems/wrapper.cpp'; Compile-Run 'problems/wrapper.cpp'; Run restore 'problems/wrapper.cpp'
Pass 'guarded cycle through a stub; pure-stub cycle; non-pure wrapper preserved'

foreach ($s in @("#if 0`n#include `"b.h`"`n#endif`n#include `"b.h`"`n", "#define HEADER `"b.h`"`n#include HEADER`n", "#include `"missing.h`"`n",
    "#include `"b.h`"`n#undef D_H`n#include `"b.h`"", "/* comment`n*/ #include `"b.h`"`n", "// continued \`n#include `"b.h`"`n")) {
    Put 'problems/bad.cpp' $s; Run expand 'problems/bad.cpp' 1; Equal (Read 'problems/bad.cpp') $s; Clean bad
}
$s="/*`n#include `"missing.h`"`n*/`nconst char* s=R`"tag(`n#include `"missing.h`"`n)tag`";`n#include `"b.h`"`n"
Put 'problems/comments.cpp' $s; Run expand 'problems/comments.cpp'; Run restore 'problems/comments.cpp'; Equal (Read 'problems/comments.cpp') $s
Pass 'conditional/macro/unresolved includes rejected; comments and raw strings ignored'

Put 'headers/marked.h' "// zoi:begin copied-from-a-problem`nint stale;`n"
Put 'problems/marked.cpp' "#include `"../headers/marked.h`"`n"
$s=Read 'problems/marked.cpp'; Run expand 'problems/marked.cpp' 1 'Reserved generated marker'
Equal (Read 'problems/marked.cpp') $s; Clean marked
Pass 'copied generated markers in dependencies rejected before creating state'

foreach ($fault in @('journal','source')) {
    Put 'problems/A.cpp' $original
    $oldFault=$env:ZOI_TEST_FAULT
    try { $env:ZOI_TEST_FAULT=$fault; Run expand 'problems/A.cpp' 1 'Injected failure' }
    finally { $env:ZOI_TEST_FAULT=$oldFault }
    $saved=Read 'problems/A.cpp'; Run status 'problems/A.cpp' 1 'Pending transaction'; Equal (Read 'problems/A.cpp') $saved
    Run expand 'problems/A.cpp'; Run restore 'problems/A.cpp'; Equal (Read 'problems/A.cpp') $original; Clean A
    Run expand 'problems/A.cpp'
    try { $env:ZOI_TEST_FAULT=$fault; Run restore 'problems/A.cpp' 1 'Injected failure' }
    finally { $env:ZOI_TEST_FAULT=$oldFault }
    Run restore 'problems/A.cpp'; Equal (Read 'problems/A.cpp') $original; Clean A
}
Pass 'interrupted expand and restore at both transaction boundaries recover'

Put 'problems/A.cpp' $original
$oldFault=$env:ZOI_TEST_FAULT
try { $env:ZOI_TEST_FAULT='source'; Run expand 'problems/A.cpp' 1 'Injected failure' }
finally { $env:ZOI_TEST_FAULT=$oldFault }
$after=Read 'problems/A.cpp'; Put 'problems/A.cpp' ($after+'// unsaved transaction edit')
Run restore 'problems/A.cpp' 1 'Source changed during interrupted write'
Equal (Read 'problems/A.cpp') ($after+'// unsaved transaction edit')
Put 'problems/A.cpp' $after; Run restore 'problems/A.cpp'; Equal (Read 'problems/A.cpp') $original; Clean A
Run expand 'problems/A.cpp'; $savedState=Read 'problems/A.zoi.state.json'; $after=Read 'problems/A.cpp'
Put 'problems/A.zoi.state.json' '{"version":99}'
Run forget 'problems/A.cpp' 1 'Invalid state'; Equal (Read 'problems/A.cpp') $after
Put 'problems/A.zoi.state.json' $savedState
# An externally held per-file lock must stop writes without altering either file.
$lock=New-Object IO.FileStream((Join-Path $fixture 'problems/A.cpp.zoi.lock'), [IO.FileMode]::OpenOrCreate, [IO.FileAccess]::ReadWrite, [IO.FileShare]::None)
try { Run expand 'problems/A.cpp' 1; Equal (Read 'problems/A.cpp') $after }
finally { $lock.Dispose(); [IO.File]::Delete((Join-Path $fixture 'problems/A.cpp.zoi.lock')) }
Run restore 'problems/A.cpp'; Clean A
Pass 'interrupted write plus subsequent edits / corrupt state / concurrent invocation preserve source'

function Legacy([string]$name, [bool]$sha=$true) {
    Put ('problems/'+$name+'.cpp') "// legacy expanded`nint main() {}`n"
    Put ('problems/'+$name+'.zoi.cpp') $original
    if ($sha) { Put ('problems/'+$name+'.zoi.sha') (Get-FileHash -LiteralPath (Join-Path $fixture ('problems/'+$name+'.cpp')) -Algorithm SHA1).Hash }
}
Legacy 'old'; Run expand 'problems/old.cpp'; Run restore 'problems/old.cpp'; Equal (Read 'problems/old.cpp') $original; Clean old
Legacy 'no-sha' $false; Run restore 'problems/no-sha.cpp' 1 'missing SHA'
Legacy 'edited'; Put 'problems/edited.cpp' '// keep edits'; Run restore 'problems/edited.cpp' 1 'Legacy conflict'
Legacy 'missing'; [IO.File]::Delete((Join-Path $fixture 'problems/missing.cpp'))
Put 'problems/orphan.zoi.sha' 'orphan'
Put 'problems/A.cpp' $original; Run expand 'problems/A.cpp'; Run restore 'problems' 1
Equal (Read 'problems/A.cpp') $original; Clean A
Equal (Read 'problems/edited.cpp') '// keep edits'
Assert ([IO.File]::Exists((Join-Path $fixture 'problems/missing.zoi.cpp'))) 'Orphan backup deleted'
Pass 'legacy migration / missing SHA / edited source / missing source / independent batch restore'

Run forget 'problems/edited.cpp'; Equal (Read 'problems/edited.cpp') '// keep edits'; Clean edited
Run forget 'problems/no-sha.cpp' 1 'valid legacy SHA'
Assert ([IO.File]::Exists((Join-Path $fixture 'problems/no-sha.zoi.cpp'))) 'Unknown legacy backup deleted'
Legacy 'forget-fault'; Put 'problems/forget-fault.cpp' '// completed solution'
$oldFault=$env:ZOI_TEST_FAULT
try { $env:ZOI_TEST_FAULT='source'; Run forget 'problems/forget-fault.cpp' 1 'Injected failure' }
finally { $env:ZOI_TEST_FAULT=$oldFault }
Run forget 'problems/forget-fault.cpp'; Equal (Read 'problems/forget-fault.cpp') '// completed solution'; Clean forget-fault
Pass 'forget edited legacy backup without changing source; interrupted cleanup recovers'

# Use the real library root, not the fake headers, for this final compile check.
if ([IO.File]::Exists((Join-Path $root 'zoi/seg.h'))) {
    Put 'problems/real.cpp' "#include `"seg.h`"`n#include `"bit.h`"`n#include `"hld.h`"`nint main() { return sizeof(LL) == 8 ? 0 : 1; }`n"
    Compile-Run 'problems/real.cpp'; Run expand 'problems/real.cpp' -Real; Compile-Run 'problems/real.cpp'
    Run restore 'problems/real.cpp' -Real; Clean real
    Pass 'real seg + bit + hld shared utils compile and run in both forms'
} else { throw 'Real library fixture missing: zoi/seg.h' }
Write-Host "zoi self-test: $script:passed groups passed ($script:calls commands); logs: $fixture"
