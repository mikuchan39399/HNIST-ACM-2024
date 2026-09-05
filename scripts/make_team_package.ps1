param([Parameter(Mandatory=$true)][string]$OutputPath)
# Build a portable zip from source assets only, never personal setup state.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$output=[IO.Path]::GetFullPath($OutputPath)
if ([IO.File]::Exists($output)) { throw 'Output already exists; choose a new file' }
$parent=Split-Path -Parent $output
if (-not [IO.Directory]::Exists($parent)) { throw 'Output directory must exist' }
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
$null=Get-CheckInventory $root
$stage=Join-Path $parent ('zoi-package-'+[Guid]::NewGuid().ToString('N'))
$package=Join-Path $stage 'HNIST-ZOI'
[void][IO.Directory]::CreateDirectory($package)
$files=@()
foreach ($dir in @('algorithms','zoi','scripts','rules','docs','records/tooling','.clinerules')) {
    $files += @(Get-ChildItem -LiteralPath (Join-Path $root $dir) -Recurse -File | Where-Object {
        $_.FullName -notmatch '[\\/]docs[\\/](backups|releases|booklet[\\/]output)[\\/]' -and $_.Extension -in @('.ps1','.cmd','.cpp','.h','.txt','.md','.py','.typ') -and $_.Name -notmatch '\.zoi\.'
    })
}
foreach ($name in @('rule.md','README.md','AGENTS.md','LICENSE')) { if ([IO.File]::Exists((Join-Path $root $name))) { $files += Get-Item -LiteralPath (Join-Path $root $name) } }
try {
    $items=@()
    foreach ($f in $files) {
        if ($f.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Reparse points cannot enter the package' }
        $rel=$f.FullName.Substring($root.Length+1).Replace('\','/')
        $dst=Join-Path $package $rel
        [void][IO.Directory]::CreateDirectory((Split-Path -Parent $dst))
        Copy-Item -LiteralPath $f.FullName -Destination $dst
        $items += @{path=$rel; hash=(Get-FileHash -LiteralPath $dst -Algorithm SHA256).Hash}
    }
    $data=@{format=1; product='HNIST-ZOI-team-package'; files=$items}
    [IO.File]::WriteAllText((Join-Path $package '.zoi-package.json'),($data | ConvertTo-Json -Depth 5),(New-Object Text.UTF8Encoding($false)))
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::CreateFromDirectory($stage,$output)
    Write-Host "[OK] team package: $output ($($items.Count) source files)"
} finally {
    # This fresh GUID staging folder belongs to this invocation and is confined
    # to the caller's explicitly chosen output directory.
    $resolved=[IO.Path]::GetFullPath($stage)
    if (-not $resolved.StartsWith($parent+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected staging path; retained' }
    Remove-Item -LiteralPath $resolved -Recurse -Force
}
