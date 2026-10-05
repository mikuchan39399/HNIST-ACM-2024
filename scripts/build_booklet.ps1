param(
    [string]$Python = '',
    [string]$Compiler = 'g++',
    [string]$TypstPath = '',
    [string]$OutDir = 'docs/booklet/output/chapters',
    [int]$SoloMin = 0
)
# Full release pipeline: generate in isolation, validate, then publish.
# make_booklet.ps1 remains the low-level generator for scoped previews.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if (-not $Python) {
    foreach ($candidate in @('.zoi-checks/booklet-tools/venv/Scripts/python.exe', '.zoi-checks/booklet-tools/venv/bin/python')) {
        $path = Join-Path $root $candidate
        if ([IO.File]::Exists($path)) { $Python = $path; break }
    }
    if (-not $Python) { $Python = 'python' }
}
foreach ($command in @($Python, $Compiler)) {
    if (-not (Get-Command $command -ErrorAction SilentlyContinue)) { throw ('Missing build prerequisite: ' + $command) }
}
& $Python -c 'import pypdf'
if ($LASTEXITCODE -ne 0) { throw 'PDF verification requires pypdf in the selected Python environment' }
$destination = if ([IO.Path]::IsPathRooted($OutDir)) { [IO.Path]::GetFullPath($OutDir) } else { [IO.Path]::GetFullPath((Join-Path $root $OutDir)) }
$scratch = [IO.Path]::GetFullPath((Join-Path $root '.zoi-checks/codex-work'))
$stage = Join-Path $scratch ('booklet-release-' + [Guid]::NewGuid().ToString('N'))
[void][IO.Directory]::CreateDirectory($stage)
try {
    & (Join-Path $PSScriptRoot 'make_booklet.ps1') -GenerateOnly -OutDir $stage -TypstPath $TypstPath -SoloMin $SoloMin
    & $Python (Join-Path $PSScriptRoot 'check_booklet_code.py') $stage --compiler $Compiler
    if ($LASTEXITCODE -ne 0) { throw 'Printed C++20 regression failed' }
    & $Python (Join-Path $PSScriptRoot 'check_booklet_pdf.py') $stage
    if ($LASTEXITCODE -ne 0) { throw 'PDF verification failed' }

    $enc = New-Object Text.UTF8Encoding($false)
    $index = [IO.File]::ReadAllText((Join-Path $stage 'chapters.json'), $enc) | ConvertFrom-Json
    $index | Add-Member -NotePropertyName validation -NotePropertyValue ([pscustomobject]@{
        checkedAtUtc=[DateTime]::UtcNow.ToString('o')
        code='check_booklet_code.py'; pdf='check_booklet_pdf.py'
    })
    $newNames = @{}
    foreach ($chapter in $index.chapters) {
        foreach ($name in @($chapter.pdf, $chapter.source, $chapter.code)) {
            if (-not $name -or [IO.Path]::GetFileName($name) -cne $name) { throw 'Invalid generated chapter filename' }
            $newNames[$name] = $true
        }
    }
    $oldNames = @{}
    $oldIndex = Join-Path $destination 'chapters.json'
    if ([IO.File]::Exists($oldIndex)) {
        $previous = [IO.File]::ReadAllText($oldIndex, $enc) | ConvertFrom-Json
        if ($previous.schema -ne 1 -or $previous.cppStandard -ne 'c++20' -or $null -eq $previous.chapters) { throw 'Unrecognized existing chapter manifest; preserve and inspect it before publishing' }
        foreach ($chapter in $previous.chapters) {
            foreach ($name in @($chapter.pdf, $chapter.source, $chapter.code)) {
                if (-not $name) { continue }
                if ([IO.Path]::GetFileName($name) -cne $name -or $name -notmatch '\.(pdf|typ|code\.md)$') { throw 'Unsafe path in existing chapter manifest' }
                $oldNames[$name] = $true
            }
        }
    }
    [void][IO.Directory]::CreateDirectory($destination)
    foreach ($chapter in $index.chapters) {
        foreach ($name in @($chapter.pdf, $chapter.source, $chapter.code)) {
            Copy-Item -LiteralPath (Join-Path $stage $name) -Destination (Join-Path $destination $name) -Force
        }
    }
    foreach ($name in $oldNames.Keys) {
        if ($newNames.ContainsKey($name)) { continue }
        $obsolete = [IO.Path]::GetFullPath((Join-Path $destination $name))
        if (-not $obsolete.StartsWith($destination.TrimEnd([IO.Path]::DirectorySeparatorChar) + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid obsolete artifact path' }
        if ([IO.File]::Exists($obsolete)) { Remove-Item -LiteralPath $obsolete -Force }
    }
    # The manifest is the authoritative set; write it only after all files copy.
    [IO.File]::WriteAllText((Join-Path $destination 'chapters.json'), ($index | ConvertTo-Json -Depth 6), $enc)
    if (-not $stage.StartsWith($scratch + [IO.Path]::DirectorySeparatorChar, [StringComparison]::OrdinalIgnoreCase)) { throw 'Invalid release cleanup path' }
    Remove-Item -LiteralPath $stage -Recurse -Force
    Write-Host ('[OK] verified booklet: ' + $index.chapters.Count + ' chapters -> ' + $destination)
} catch {
    Write-Host ('[FAIL] publication incomplete; inspect ' + $stage)
    throw
}
