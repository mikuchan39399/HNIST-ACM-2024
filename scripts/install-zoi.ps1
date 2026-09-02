# install-zoi.ps1 - wire THIS clone's zoi stub board into local VS Code (user scope).
# Usage:  powershell -ExecutionPolicy Bypass -File install-zoi.ps1
#   1. backup  %APPDATA%\Code\User\settings.json -> settings.json.zoi.bak (once)
#   2. append  <repo>/zoi into C_Cpp.default.includePath   (IntelliSense side)
#   3. set     cph.language.cpp.Args = "-std=c++23 -O2 -Wall -I<repo>/zoi"
#              (existing key: only the -I flag is swapped in, other flags untouched)
#   4. regenerate zoi stubs (idempotent) + save install state for uninstall
# Problem folders need NO config: includePath and -I are global search paths,
# any .cpp anywhere resolves the stubs. Only the repo location matters.
# ASCII-only on purpose: PS 5.1 reads no-BOM files as ANSI (see zoi.ps1 NOTE).
param(
    [string]$SettingsFile = '',   # test hook: operate on a copy, not real settings
    [switch]$NoStubs              # test hook: skip make_stubs
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$zoiPath = ($Root -replace '\\', '/') + '/zoi'
$setFile = if ($SettingsFile -ne '') { $SettingsFile } else { Join-Path $env:APPDATA 'Code\User\settings.json' }
$stateFile = if ($SettingsFile -ne '') { "$SettingsFile.zoi-state" } else { Join-Path $PSScriptRoot '.zoi-install-state.json' }
$enc = New-Object System.Text.UTF8Encoding($false)

if (-not (Test-Path -LiteralPath $setFile)) {
    [IO.File]::WriteAllText($setFile, "{`n}`n", $enc)
    Write-Host '[OK] created empty settings.json'
}
$bak = [IO.Path]::ChangeExtension($setFile, '.json.zoi.bak')
if (-not (Test-Path -LiteralPath $bak)) {
    Copy-Item -LiteralPath $setFile -Destination $bak
    Write-Host "[OK] backup: $bak"
}
else { Write-Host "[SKIP] backup exists: $bak" }

$text = [IO.File]::ReadAllText($setFile)

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
function Insert-BeforeLastBrace([string]$t, [string]$snippet) {
    $i = $t.Length - 1
    while ($i -ge 0 -and ([string]$t[$i]) -match '\s') { $i-- }
    if ($i -lt 0 -or $t[$i] -ne '}') { throw 'cannot locate the closing brace of settings.json (trailing comments?) - edit manually' }
    $j = $i - 1
    while ($j -ge 0 -and ([string]$t[$j]) -match '\s') { $j-- }
    $comma = ''
    if ($j -ge 0 -and $t[$j] -ne '{' -and $t[$j] -ne ',') { $comma = ',' }
    return $t.Substring(0, $i) + $comma + "`n    " + $snippet + $t.Substring($i)
}
# ---- 1) C_Cpp.default.includePath ----
$incKey = 'C_Cpp.default.includePath'
$incTok = '"' + $zoiPath + '"'
$ki = Find-KeyInCode $text $incKey
if ($ki -lt 0) {
    $text = Insert-BeforeLastBrace $text ('"' + $incKey + '": [' + "`n        " + $incTok + "`n    ]")
    Write-Host "[OK] created $incKey with zoi entry"
}
else {
    $ci = $text.IndexOf(':', ($ki + $incKey.Length + 2)); $v = Skip-Ws $text ($ci + 1)
    if ($v -lt 0) { throw "$incKey : cannot find value" }
    if ($text[$v] -eq '"') {
        $ve = End-Of-String $text $v
        $oldVal = $text.Substring($v, $ve - $v + 1)
        if ($oldVal -eq $incTok) { Write-Host "[SKIP] $incKey already covers zoi" }
        else {
            $arr = '[' + "`n        " + $oldVal + ',' + "`n        " + $incTok + "`n    ]"
            $text = $text.Substring(0, $v) + $arr + $text.Substring($ve + 1)
            Write-Host "[OK] $incKey was a string, normalized to array + zoi"
        }
    }
    elseif ($text[$v] -eq '[') {
        $depth = 0; $i = $v
        while ($i -lt $text.Length) {
            $c = $text[$i]
            if ($c -eq '"') { $i = End-Of-String $text $i; if ($i -lt 0) { throw 'unterminated string in includePath' }; $i++; continue }
            if ($c -eq '[') { $depth++ }
            if ($c -eq ']') { $depth--; if ($depth -eq 0) { break } }
            $i++
        }
        $body = $text.Substring($v, $i - $v + 1)
        if ($body.Contains($incTok)) { Write-Host "[SKIP] includePath entry already present" }
        else {
            $nxt = Skip-Ws $text ($v + 1)
            if ($nxt -ge 0 -and $text[$nxt] -eq ']') {
                $text = $text.Substring(0, $nxt) + $incTok + $text.Substring($nxt)
            }
            else {
                $text = $text.Substring(0, $v + 1) + $incTok + ',' + $text.Substring($v + 1)
            }
            Write-Host "[OK] includePath += $incTok"
        }
    }
    else { throw "$incKey has an unsupported value form (expected string or array)" }
}

# ---- 2) cph.language.cpp.Args ----
$cphKey = 'cph.language.cpp.Args'
$cphNew = '-std=c++23 -O2 -Wall -I' + $zoiPath
$cphPrev = $null
$ki = Find-KeyInCode $text $cphKey
if ($ki -lt 0) {
    $text = Insert-BeforeLastBrace $text ('"' + $cphKey + '": "' + $cphNew + '"')
    Write-Host "[OK] created $cphKey"
}
else {
    $ci = $text.IndexOf(':', ($ki + $cphKey.Length + 2)); $v = Skip-Ws $text ($ci + 1)
    if ($v -lt 0 -or $text[$v] -ne '"') { throw "$cphKey has an unsupported value form" }
    $ve = End-Of-String $text $v
    $old = $text.Substring($v + 1, $ve - $v - 1)
    $cphPrev = $old
    if ($old -match '-I\S+') {
        $oldI = ($old | Select-String '-I\S+').Matches[0].Value
        $new = $old -replace '-I\S+', ('-I' + $zoiPath)
        if ($oldI -ne ('-I' + $zoiPath)) { Write-Host "[WARN] cph -I switched: $oldI -> -I$zoiPath" }
        else { Write-Host '[SKIP] cph -I already points at this clone' }
    }
    else { $new = $old + ' -I' + $zoiPath; Write-Host '[OK] cph Args += -I (no -I flag was present)' }
    $text = $text.Substring(0, $v + 1) + $new + $text.Substring($ve)
}

[IO.File]::WriteAllText($setFile, $text, $enc)

# ---- 3) stubs + state ----
if (-not $NoStubs) { & (Join-Path $PSScriptRoot 'make_stubs.ps1') | Write-Host }
if (-not (Test-Path -LiteralPath $stateFile)) {
    $state = @{
        zoi_path    = $zoiPath
        cph_existed = ($null -ne $cphPrev)
        cph_prev    = $cphPrev
        stamp       = (Get-Date -Format 'yyyy-MM-dd HH:mm:ss')
    }
    [IO.File]::WriteAllText($stateFile, ($state | ConvertTo-Json), $enc)
    Write-Host "[OK] state saved: $stateFile"
}
else { Write-Host '[SKIP] install state kept (re-install must not clobber pre-install value)' }
Write-Host ''
Write-Host 'verify: open any .cpp anywhere, type  #include "cartesian.h  -> completion should fire.'
Write-Host 'cph: save the file before testing. Reload Window if IntelliSense lags.'
