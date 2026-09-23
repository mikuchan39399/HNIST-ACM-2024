param([string]$SnippetsDir='', [switch]$Uninstall)
# Install a standalone VS Code snippet; no settings or keybindings changes.
$ErrorActionPreference='Stop'
$source=Join-Path $PSScriptRoot 'snippets/seg-add-sum.json'
$text=[IO.File]::ReadAllText($source)
$null=ConvertFrom-Json $text
if (-not $SnippetsDir) {
    if (-not $env:APPDATA) { throw 'Pass -SnippetsDir for this VS Code profile.' }
    $SnippetsDir=Join-Path $env:APPDATA 'Code/User/snippets'
}
$directory=[IO.Path]::GetFullPath($SnippetsDir)
$target=Join-Path $directory 'zoi-seg-add-sum.code-snippets'
if (Test-Path -LiteralPath $target) {
    if (-not [IO.File]::Exists($target)) { throw "Target is not a file: $target" }
    if ([IO.File]::ReadAllText($target) -cne $text) {
        throw "Existing snippet differs and was preserved. Move or rename it before installing/uninstalling: $target"
    }
    if ($Uninstall) {
        Remove-Item -LiteralPath $target
        Write-Host "[OK] removed: $target"
    } else { Write-Host "[OK] already installed: $target" }
    return
}
if ($Uninstall) { Write-Host '[OK] snippet is not installed'; return }
[void][IO.Directory]::CreateDirectory($directory)
$bytes=(New-Object Text.UTF8Encoding($false)).GetBytes($text)
$stream=[IO.File]::Open($target,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::None)
try { $stream.Write($bytes,0,$bytes.Length) } finally { $stream.Dispose() }
Write-Host "[OK] installed: $target"
Write-Host '[USE] In a C++ editor, type zoisegaddsum and select the snippet, or run Insert Snippet.'
