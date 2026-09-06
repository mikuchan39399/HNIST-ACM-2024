param([string]$OutputPath='')
# Build a portable zip from source assets only, never personal setup state.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
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
$stage=Join-Path $parent ('zoi-package-'+[Guid]::NewGuid().ToString('N'))
$package=Join-Path $stage 'HNIST-ZOI'
$files=@()
function Package-Files([string]$Directory) {
    $pending=New-Object 'Collections.Generic.Stack[string]'; $pending.Push($Directory)
    while ($pending.Count) {
        foreach ($entry in Get-ChildItem -LiteralPath $pending.Pop() -Force) {
            $rel=$entry.FullName.Substring($root.Length+1).Replace('\','/')
            if ($rel -match '(^|/)(\.git|\.vscode|\.zoi-checks|\.ci-results)(/|$)|^docs/(backups|releases|booklet/output)(/|$)') { continue }
            if ($entry.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw ('Reparse points cannot enter the package: '+$rel) }
            if ($entry.PSIsContainer) { $pending.Push($entry.FullName) }
            elseif ($entry.Extension -in @('.ps1','.cmd','.cpp','.h','.txt','.md','.py','.typ','.json','.yml','.yaml') -and $entry.Name -notmatch '^\.|\.zoi[.-]|\.saved$|\.bak$') { $entry }
        }
    }
}
foreach ($dir in @('algorithms','zoi','scripts','rules','docs','records/tooling','records/verification','.clinerules','.github')) {
    $files += @(Package-Files (Join-Path $root $dir))
}
foreach ($name in @('rule.md','README.md','AGENTS.md','LICENSE','docs/releases/README.md','docs/backups/README.md')) { if ([IO.File]::Exists((Join-Path $root $name))) { $files += Get-Item -LiteralPath (Join-Path $root $name) } }
try {
    [void][IO.Directory]::CreateDirectory($package)
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
    $partial=$output+'.partial'
    if ([IO.File]::Exists($partial)) { throw 'Partial output already exists; preserved' }
    try {
        [IO.Compression.ZipFile]::CreateFromDirectory($stage,$partial)
        [IO.File]::Move($partial,$output)
    } finally { if ([IO.File]::Exists($partial)) { [IO.File]::Delete($partial) } }
    Write-Host "[OK] team package: $output ($($items.Count) source files)"
} finally {
    # This fresh GUID staging folder belongs to this invocation and is confined
    # to the caller's explicitly chosen output directory.
    $resolved=[IO.Path]::GetFullPath($stage)
    if (-not $resolved.StartsWith($parent+[IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase)) { throw 'Unexpected staging path; retained' }
    for ($attempt=0;$attempt -lt 5;$attempt++) {
        try {
            if ([IO.Directory]::Exists($resolved)) { Remove-Item -LiteralPath $resolved -Recurse -Force }
            break
        } catch {
            if ($attempt -eq 4) { Write-Host ('[WARN] Temporary stage is still locked; retained: '+$resolved) }
            else { Start-Sleep -Milliseconds 200 }
        }
    }
}
