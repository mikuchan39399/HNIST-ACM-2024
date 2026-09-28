param([string]$OutputPath='')
# Build a portable zip from source assets only, never personal setup state.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
if (Test-Path -LiteralPath (Join-Path $PSScriptRoot 'sync_layout.ps1')) { & (Join-Path $PSScriptRoot 'sync_layout.ps1') }
if (-not $OutputPath) {
    $releaseDir=Join-Path $root 'docs/releases'
    [void][IO.Directory]::CreateDirectory($releaseDir)
    $OutputPath=Join-Path $releaseDir ('HNIST-ZOI-'+(Get-Date -Format 'yyyyMMdd-HHmmss-fff')+'.zip')
}
$output=[IO.Path]::GetFullPath($OutputPath)
if ([IO.File]::Exists($output)) { throw 'Output already exists; choose a new file' }
$parent=Split-Path -Parent $output
if (-not [IO.Directory]::Exists($parent)) { throw 'Output directory must exist' }
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
$null=Get-CheckInventory $root
$files=@()
function Package-Files([string]$Directory) {
    $pending=New-Object 'Collections.Generic.Stack[string]'; $pending.Push($Directory)
    while ($pending.Count) {
        foreach ($entry in Get-ChildItem -LiteralPath $pending.Pop() -Force) {
            $rel=$entry.FullName.Substring($root.Length+1).Replace('\','/')
            if ($rel -match '(^|/)(\.git|\.vscode|\.zoi-checks|\.ci-results|node_modules)(/|$)|^docs/(backups|releases|booklet/output)(/|$)') { continue }
            if ($entry.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw ('Reparse points cannot enter the package: '+$rel) }
            if ($entry.PSIsContainer) { $pending.Push($entry.FullName) }
            elseif (($entry.Extension -in @('.ps1','.cmd','.cjs','.cpp','.h','.txt','.md','.py','.typ','.json','.yml','.yaml') -or ($entry.Extension -eq '.js' -and $rel -like 'scripts/vjudge-extension/*') -or $rel -like 'scripts/statement-extension/*') -and $entry.Name -notmatch '^\.|\.zoi[.-]|\.saved$|\.bak$') { $entry }
        }
    }
}
foreach ($dir in @('algorithms','zoi','scripts','rules','docs','records/tooling','records/verification','.clinerules','.github')) {
    $files += @(Package-Files (Join-Path $root $dir))
}
foreach ($name in @('rule.md','README.md','AGENTS.md','LICENSE','docs/releases/README.md','docs/backups/README.md')) { if ([IO.File]::Exists((Join-Path $root $name))) { $files += Get-Item -LiteralPath (Join-Path $root $name) } }
Add-Type -AssemblyName System.IO.Compression
$partial=$output+'.partial'
if ([IO.File]::Exists($partial)) { throw 'Partial output already exists; preserved' }
$outputStream=$null; $archive=$null; $ownsPartial=$false
try {
    $outputStream=[IO.File]::Open($partial,[IO.FileMode]::CreateNew,[IO.FileAccess]::ReadWrite,[IO.FileShare]::None)
    $ownsPartial=$true
    $archive=New-Object IO.Compression.ZipArchive($outputStream,[IO.Compression.ZipArchiveMode]::Create,$true)
    $items=@()
    foreach ($f in $files) {
        if ($f.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Reparse points cannot enter the package' }
        $rel=$f.FullName.Substring($root.Length+1).Replace('\','/')
        # Read-lock the source while hashing and copying; the manifest describes the exact bytes delivered.
        $inputStream=[IO.File]::Open($f.FullName,[IO.FileMode]::Open,[IO.FileAccess]::Read,[IO.FileShare]::Read)
        $sha=[Security.Cryptography.SHA256]::Create(); $entryStream=$null
        try {
            $hash=[BitConverter]::ToString($sha.ComputeHash($inputStream)).Replace('-','')
            $inputStream.Position=0
            $entry=$archive.CreateEntry('HNIST-ZOI/'+$rel,[IO.Compression.CompressionLevel]::Optimal)
            $entryStream=$entry.Open(); $inputStream.CopyTo($entryStream)
            $items += @{path=$rel; hash=$hash}
        } finally { if ($entryStream) { $entryStream.Dispose() }; $sha.Dispose(); $inputStream.Dispose() }
    }
    $data=@{format=1; product='HNIST-ZOI-team-package'; files=$items}
    $bytes=(New-Object Text.UTF8Encoding($false)).GetBytes(($data | ConvertTo-Json -Depth 5))
    $entryStream=$archive.CreateEntry('HNIST-ZOI/.zoi-package.json').Open()
    try {
        $entryStream.Write($bytes,0,$bytes.Length)
    } finally { $entryStream.Dispose() }
    $archive.Dispose(); $archive=$null; $outputStream.Dispose(); $outputStream=$null
    [IO.File]::Move($partial,$output)
    Write-Host "[OK] team package: $output ($($items.Count) source files)"
} finally {
    if ($archive) { $archive.Dispose() }; if ($outputStream) { $outputStream.Dispose() }
    if ($ownsPartial -and [IO.File]::Exists($partial)) { [IO.File]::Delete($partial) }
}
