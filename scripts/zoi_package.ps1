# Use .NET so a Windows PowerShell child of pwsh does not depend on inherited PSModulePath.
function Get-ZoiPackageHash([string]$path) {
    $stream=[IO.File]::OpenRead($path)
    $sha=[Security.Cryptography.SHA256]::Create()
    try { return [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-','') }
    finally { $sha.Dispose(); $stream.Dispose() }
}
# Delete only an unchanged, explicitly marked distribution package.
function Remove-ZoiPackage([string]$root) {
    $root=[IO.Path]::GetFullPath($root).TrimEnd([IO.Path]::DirectorySeparatorChar)
    $manifest=Join-Path $root '.zoi-package.json'
    if (-not [IO.File]::Exists($manifest)) { Write-Host '[NOTE] Not a managed package; library source retained'; return }
    $state=[IO.File]::ReadAllText($manifest) | ConvertFrom-Json
    if ($state.format -ne 1 -or $state.product -cne 'HNIST-ZOI-team-package' -or $state.files.Count -lt 1 -or $root -eq [IO.Path]::GetPathRoot($root).TrimEnd('\','/')) { throw 'Invalid package manifest; retained' }
    $prefix=$root+[IO.Path]::DirectorySeparatorChar
    $comparer=[StringComparer]::OrdinalIgnoreCase
    $owned=New-Object 'Collections.Generic.Dictionary[string,object]' ($comparer)
    foreach ($f in $state.files) {
        $path=[IO.Path]::GetFullPath((Join-Path $root $f.path))
        if (-not $path.StartsWith($prefix,[StringComparison]::OrdinalIgnoreCase) -or $path -eq $manifest -or $owned.ContainsKey($path) -or $f.hash -notmatch '^[A-F0-9]{64}$') { throw 'Invalid package path/hash; retained' }
        $owned[$path]=$f.hash
    }
    $all=@(Get-ChildItem -LiteralPath $root -Recurse -Force)
    if (((Get-Item -LiteralPath $root -Force).Attributes -band [IO.FileAttributes]::ReparsePoint) -or @($all | Where-Object { $_.Attributes -band [IO.FileAttributes]::ReparsePoint }).Count) { throw 'Package has reparse points; retained' }
    foreach ($f in @($all | Where-Object { -not $_.PSIsContainer })) {
        if ($f.FullName -eq $manifest) { continue }
        if (-not $owned.ContainsKey($f.FullName) -or (Get-ZoiPackageHash $f.FullName) -cne $owned[$f.FullName]) {
            throw "Added/modified file retained with package: $($f.FullName). VS Code wiring has already been removed."
        }
    }
    # Every absolute target is verified above; no shell-built recursive delete.
    # Keep the manifest and uninstall entry points until ordinary files are gone.
    # Virus scanners/indexers can briefly hold a just-extracted file on Windows.
    $ordered=@($all | Where-Object { -not $_.PSIsContainer } | Sort-Object @{Expression={
        if ($_.FullName -eq $manifest) { 2 }
        elseif ($_.FullName -match '[\\/]scripts[\\/](uninstall-zoi\.(ps1|cmd)|zoi_(setup|package)\.ps1)$') { 1 }
        else { 0 }
    }})
    foreach ($f in $ordered) {
        for ($attempt=0; ; $attempt++) {
            try { Remove-Item -LiteralPath $f.FullName -ErrorAction Stop; break }
            catch {
                if ($attempt -ge 9) { throw "Package file is still locked; close programs using it and rerun uninstall: $($f.FullName)" }
                Start-Sleep -Milliseconds 200
            }
        }
    }
    foreach ($d in @($all | Where-Object PSIsContainer | Sort-Object { $_.FullName.Length } -Descending)) {
        if (@(Get-ChildItem -LiteralPath $d.FullName -Force).Count -eq 0) { [IO.Directory]::Delete($d.FullName) }
    }
    if (@(Get-ChildItem -LiteralPath $root -Force).Count -eq 0) { [IO.Directory]::Delete($root) }
    Write-Host '[OK] unchanged team package removed'
}
