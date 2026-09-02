# uninstall-zoi.ps1 - remove THIS clone's zoi wiring from local VS Code settings.
# Usage:  powershell -ExecutionPolicy Bypass -File uninstall-zoi.ps1
#   1. remove  <repo>/zoi from C_Cpp.default.includePath (other clones stay)
#   2. restore cph.language.cpp.Args from install state (created key -> removed)
#   3. delete the state file; the .bak backup is kept on purpose
# ASCII-only on purpose: PS 5.1 reads no-BOM files as ANSI.
param([string]$SettingsFile = '')   # test hook, mirrors install-zoi.ps1
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$zoiPath = ($Root -replace '\\', '/') + '/zoi'
$setFile = if ($SettingsFile -ne '') { $SettingsFile } else { Join-Path $env:APPDATA 'Code\User\settings.json' }
$stateFile = if ($SettingsFile -ne '') { "$SettingsFile.zoi-state" } else { Join-Path $PSScriptRoot '.zoi-install-state.json' }
$enc = New-Object System.Text.UTF8Encoding($false)

if (-not (Test-Path -LiteralPath $setFile)) { Write-Host '[OK] nothing to do (no settings.json)'; exit 0 }
$text = [IO.File]::ReadAllText($setFile)
$state = $null
if (Test-Path -LiteralPath $stateFile) { $state = Get-Content -LiteralPath $stateFile -Raw | ConvertFrom-Json }

function Skip-Ws([string]$t, [int]$pos) {
    while ($pos -lt $t.Length) {
        $c = $t[$pos]
        if ($c -eq ' ' -or $c -eq "`t" -or $c -eq "`r" -or $c -eq "`n") { $pos++; continue }
        if ($c -eq '/' -and $pos + 1 -lt $t.Length -and $t[$pos + 1] -eq '/') {
            $nl = $t.IndexOf("`n", $pos); if ($nl -lt 0) { return -1 }; $pos = $nl + 1; continue
        }
        if ($c -eq '/' -and $pos + 1 -lt $t.Length -and $t[$pos + 1] -eq '*') {
            $end = $t.IndexOf('*/', $pos + 2); if ($end -lt 0) { return -1 }; $pos = $end + 2; continue
        }
        return $pos
    }
    return -1
}
function End-Of-String([string]$t, [int]$q) {
    $i = $q + 1
    while ($i -lt $t.Length) {
        if ($t[$i] -eq '\') { $i += 2; continue }
        if ($t[$i] -eq '"') { return $i }
        $i++
    }
    return -1
}
function Find-KeyInCode([string]$t, [string]$key) {
    $k = '"' + $key + '"'
    $i = 0; $n = $t.Length
    $inS = $false; $inL = $false; $inB = $false
    while ($i -lt $n) {
        $c = $t[$i]
        if ($inL) { if ($c -eq "`n") { $inL = $false }; $i++; continue }
        if ($inB) { if ($c -eq '*' -and $i + 1 -lt $n -and $t[$i + 1] -eq '/') { $inB = $false; $i += 2; continue }; $i++; continue }
        if ($inS) { if ($c -eq '\') { $i += 2; continue }; if ($c -eq '"') { $inS = $false }; $i++; continue }
        if ($c -eq '/' -and $i + 1 -lt $n -and $t[$i + 1] -eq '/') { $inL = $true; $i += 2; continue }
        if ($c -eq '/' -and $i + 1 -lt $n -and $t[$i + 1] -eq '*') { $inB = $true; $i += 2; continue }
        if ($c -eq '"') {
            $len = [Math]::Min($k.Length, $n - $i)
            if ($t.Substring($i, $len) -eq $k) {
                $after = Skip-Ws $t ($i + $k.Length)
                if ($after -ge 0 -and $t[$after] -eq ':') { return $i }
            }
            $inS = $true
        }
        $i++
    }
    return -1
}
# ---- 1) includePath: drop our token (+ its comma) ----
$incKey = 'C_Cpp.default.includePath'
$incTok = '"' + $zoiPath + '"'
$ki = Find-KeyInCode $text $incKey
if ($ki -lt 0) { Write-Host "[SKIP] no $incKey key" }
else {
    $ci = $text.IndexOf(':', ($ki + $incKey.Length + 2)); $v = Skip-Ws $text ($ci + 1)
    $depth = 0; $i = $v
    while ($i -lt $text.Length) {
        $c = $text[$i]
        if ($c -eq '"') { $i = End-Of-String $text $i; $i++; continue }
        if ($c -eq '[') { $depth++ }
        if ($c -eq ']') { $depth--; if ($depth -eq 0) { break } }
        $i++
    }
    $body = $text.Substring($v, $i - $v + 1)
    $hit = $body.IndexOf($incTok)
    if ($hit -lt 0) { Write-Host "[SKIP] zoi entry not present in $incKey" }
    else {
        $abs = $v + $hit
        $afterTok = $abs + $incTok.Length
        $nxt = Skip-Ws $text $afterTok
        $cutS = $abs
        $cutE = $afterTok
        if ($nxt -ge 0 -and $text[$nxt] -eq ',') { $cutE = $nxt + 1 }
        elseif ($nxt -ge 0 -and $text[$nxt] -eq ']') {
            # our token was the last element: swallow the preceding comma too
            $j = $abs - 1
            while ($j -ge $v -and ([string]$text[$j]) -match '[ \t\r\n]') { $j-- }
            if ($j -ge $v -and $text[$j] -eq ',') { $cutS = $j }
        }
        $text = $text.Substring(0, $cutS) + $text.Substring($cutE)
        Write-Host "[OK] includePath -= $incTok"
    }
}

# ---- 2) cph.language.cpp.Args ----
$cphKey = 'cph.language.cpp.Args'
$ki = Find-KeyInCode $text $cphKey
if ($ki -lt 0) { Write-Host "[SKIP] no $cphKey key" }
elseif ($null -eq $state) {
    Write-Host "[WARN] no install state; $cphKey left untouched - fix -I manually if it pointed here"
}
else {
    $ci = $text.IndexOf(':', ($ki + $cphKey.Length + 2)); $v = Skip-Ws $text ($ci + 1)
    $ve = End-Of-String $text $v
    if ($state.cph_existed) {
        $text = $text.Substring(0, $v + 1) + $state.cph_prev + $text.Substring($ve)
        Write-Host '[OK] cph Args restored to pre-install value'
    }
    else {
        # we created the key: remove the whole key block (+ trailing comma)
        $after = Skip-Ws $text ($ve + 1)
        $end = $ve + 1
        if ($after -ge 0 -and $text[$after] -eq ',') { $end = $after + 1 }
        # object becomes empty? then also swallow the preceding comma we injected
        $p = Skip-Ws $text $end
        $q = $ki - 1
        while ($q -ge 0 -and ([string]$text[$q]) -match '[ \t\r\n]') { $q-- }
        if ($p -ge 0 -and $text[$p] -eq '}' -and $q -ge 0 -and $text[$q] -eq ',') { $q-- }
        $text = $text.Substring(0, $q + 1) + $text.Substring($end)
        Write-Host "[OK] $cphKey removed (install had created it)"
    }
}

[IO.File]::WriteAllText($setFile, $text, $enc)
if ($null -ne $state -and (Test-Path -LiteralPath $stateFile)) { Remove-Item -LiteralPath $stateFile }
Write-Host '[OK] uninstall done. The .bak backup was kept on purpose.'
