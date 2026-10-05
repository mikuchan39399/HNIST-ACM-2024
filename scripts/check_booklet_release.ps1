param([string]$Python = '')
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot 'check_process.ps1')
if (-not $Python) {
    $Python = Join-Path $root '.zoi-checks/booklet-tools/venv/Scripts/python.exe'
    if (-not [IO.File]::Exists($Python)) { $Python = 'python' }
}
$fixture = Join-Path $root ('.zoi-checks/codex-work/booklet-release-test-' + [Guid]::NewGuid().ToString('N'))
$enc = New-Object Text.UTF8Encoding($false)
function Put([string]$Rel, [string]$Text) {
    $path = Join-Path $fixture $Rel
    [void][IO.Directory]::CreateDirectory((Split-Path -Parent $path))
    [IO.File]::WriteAllText($path, $Text, $enc)
}
function Assert([bool]$Ok, [string]$Message) { if (-not $Ok) { throw $Message } }
Put 'scripts/placeholder' ''
Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'build_booklet.ps1') -Destination (Join-Path $fixture 'scripts/build_booklet.ps1')
# Controlled producer/checkers exercise orchestration failures, not algorithms.
Put 'scripts/make_booklet.ps1' @'
param([switch]$GenerateOnly,[string]$OutDir,[string]$TypstPath,[int]$SoloMin)
$root=Split-Path -Parent $PSScriptRoot
if (-not $GenerateOnly) { throw 'Recursive release dispatch' }
if ((Get-Content -LiteralPath (Join-Path $root 'failure.txt') -Raw).Trim() -eq 'generate') { throw 'Expected generation failure' }
[void][IO.Directory]::CreateDirectory($OutDir)
$enc=New-Object Text.UTF8Encoding($false)
foreach ($name in @('volume.pdf','volume.typ','volume.code.md')) {
    [IO.File]::WriteAllText((Join-Path $OutDir $name),'NEW CONTENT',$enc)
}
$index='{"schema":1,"profile":"contest","cppStandard":"c++20","chapters":[{"chapter":"volume","pdf":"volume.pdf","source":"volume.typ","code":"volume.code.md"}]}'
[IO.File]::WriteAllText((Join-Path $OutDir 'chapters.json'),$index,$enc)
'@
foreach ($kind in @('code','pdf')) {
    Put ('scripts/check_booklet_' + $kind + '.py') (@'
from pathlib import Path
import sys
root = Path(__file__).resolve().parent.parent
kind = "KIND"
with (root / "trace.txt").open("a", encoding="utf-8") as f:
    f.write(kind + "\n")
sys.exit(7 if (root / "failure.txt").read_text().strip() == kind else 0)
'@.Replace('KIND', $kind))
}
foreach ($mode in @('generate','code','pdf','unsafe-index','success')) {
    Put 'failure.txt' $mode
    Put 'trace.txt' ''
    $managed=@('volume.pdf','volume.typ','volume.code.md','retired.pdf','retired.typ','retired.code.md')
    foreach ($name in $managed + @('personal.txt')) { Put ('published/' + $name) ('OLD ' + $name) }
    $oldManifest='{"schema":1,"cppStandard":"c++20","chapters":[{"pdf":"volume.pdf","source":"volume.typ","code":"volume.code.md"},{"pdf":"retired.pdf","source":"retired.typ","code":"retired.code.md"}]}'
    if ($mode -eq 'unsafe-index') { $oldManifest = $oldManifest.Replace('retired.pdf', '../outside.pdf') }
    Put 'published/chapters.json' $oldManifest
    $argsForBuild=@('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $fixture 'scripts/build_booklet.ps1'),'-Python',$Python,'-Compiler',$Python,'-OutDir','published')
    $result=@(Invoke-CheckProcess (Get-Process -Id $PID).Path $argsForBuild $fixture 60 (Join-Path $fixture $mode))[-1]
    Assert (-not $result.TimedOut) ('Pipeline timeout: ' + $mode)
    Assert ([IO.File]::ReadAllText((Join-Path $fixture 'published/personal.txt')) -ceq 'OLD personal.txt') 'Publication touched unrelated file'
    $trace=[IO.File]::ReadAllText((Join-Path $fixture 'trace.txt')).Replace("`r`n", "`n").Trim()
    $expected = if ($mode -eq 'generate') { '' } elseif ($mode -eq 'code') { 'code' } else { "code`npdf" }
    Assert ($trace -ceq $expected) ('Validation order/stop failed: ' + $mode)
    if ($mode -ne 'success') {
        Assert ($result.ExitCode -ne 0) ('Failure was accepted: ' + $mode)
        Assert ([IO.File]::ReadAllText((Join-Path $fixture 'published/chapters.json')) -ceq $oldManifest) 'Failed release changed the manifest'
        foreach ($name in $managed) {
            Assert ([IO.File]::ReadAllText((Join-Path $fixture ('published/' + $name))) -ceq ('OLD ' + $name)) ('Failed release modified output: ' + $mode)
        }
    } else {
        Assert ($result.ExitCode -eq 0) 'Successful release failed'
        $index=Get-Content -LiteralPath (Join-Path $fixture 'published/chapters.json') -Raw | ConvertFrom-Json
        Assert ($index.validation.code -eq 'check_booklet_code.py' -and $index.validation.pdf -eq 'check_booklet_pdf.py') 'Unverified release manifest'
        foreach ($name in @('volume.pdf','volume.typ','volume.code.md')) {
            Assert ([IO.File]::ReadAllText((Join-Path $fixture ('published/' + $name))) -ceq 'NEW CONTENT') 'Missing released artifact'
        }
        foreach ($name in @('retired.pdf','retired.typ','retired.code.md')) {
            Assert (-not [IO.File]::Exists((Join-Path $fixture ('published/' + $name)))) 'Retired chapter left a stale artifact'
        }
    }
}
$resolved=[IO.Path]::GetFullPath($fixture)
$parent=[IO.Path]::GetFullPath((Join-Path $root '.zoi-checks/codex-work'))
if (-not $resolved.StartsWith($parent + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid fixture cleanup path' }
Remove-Item -LiteralPath $resolved -Recurse -Force
Write-Host '[PASS] release: generation/code/PDF failures preserve published files; validation order; successful publication; obsolete managed chapters removed; unrelated files retained'
