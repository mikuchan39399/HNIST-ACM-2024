# Bounded native process execution shared by regression and runner self-tests.
# ASCII only; compatible with Windows PowerShell 5.1 and PowerShell 7.
function Complete-CheckWorkspace([string]$Path,[string]$Kind,[bool]$Passed=$true) {
    $pathFull=[IO.Path]::GetFullPath($Path)
    $data=@{format=1;path=$pathFull;kind=$Kind;status=$(if ($Passed) {'PASS'} else {'FAIL'});completedUtc=[DateTime]::UtcNow.ToString('o')}
    [IO.File]::WriteAllText((Join-Path $pathFull '.zoi-run.json'),($data | ConvertTo-Json),(New-Object Text.UTF8Encoding($false)))
    $base=Split-Path -Parent $pathFull
    $default=Join-Path (Split-Path -Parent $PSScriptRoot) '.zoi-checks'
    $clean=Join-Path $PSScriptRoot 'clean_checks.ps1'
    if ($Passed -and $base -eq $default -and [IO.File]::Exists($clean)) {
        # Best-effort maintenance never changes a test's verdict.
        try { & $clean -BuildRoot $base -KeepLast 3 -Apply } catch { Write-Host ('[WARN] cache cleanup: '+$_.Exception.Message) }
    }
}
function Invoke-CheckProcess {
    param([string]$File, [string[]]$Arguments, [string]$WorkingDirectory, [int]$Seconds, [string]$LogPrefix)
    $psi = New-Object System.Diagnostics.ProcessStartInfo
    $psi.FileName = $File
    $psi.WorkingDirectory = $WorkingDirectory
    $psi.UseShellExecute = $false
    $psi.CreateNoWindow = $true
    $psi.RedirectStandardInput = $true
    $psi.RedirectStandardOutput = $true
    $psi.RedirectStandardError = $true
    if ($psi.PSObject.Properties.Name -contains 'ArgumentList') {
        foreach ($arg in $Arguments) { $psi.ArgumentList.Add($arg) }
    } else {
        # Windows CRT quoting, including trailing backslashes and embedded quotes.
        $quoted = foreach ($arg in $Arguments) {
            '"' + ([regex]::Replace([regex]::Replace($arg, '(\\*)"', '$1$1\"'), '(\\+)$', '$1$1')) + '"'
        }
        $psi.Arguments = $quoted -join ' '
    }
    foreach ($key in @('TEMP', 'TMP', 'TMPDIR')) { $psi.EnvironmentVariables[$key] = $WorkingDirectory }
    if ($Sanitize) {
        $psi.EnvironmentVariables['ASAN_OPTIONS'] = 'detect_leaks=1:halt_on_error=1'
        $psi.EnvironmentVariables['UBSAN_OPTIONS'] = 'halt_on_error=1:print_stacktrace=1'
    }
    $process = New-Object System.Diagnostics.Process
    $process.StartInfo = $psi
    $out = [IO.File]::Create($LogPrefix + '.stdout.log')
    $err = [IO.File]::Create($LogPrefix + '.stderr.log')
    $watch = [Diagnostics.Stopwatch]::StartNew()
    $timedOut = $false
    try {
        [void]$process.Start()
        $process.StandardInput.Close()
        $outTask = $process.StandardOutput.BaseStream.CopyToAsync($out)
        $errTask = $process.StandardError.BaseStream.CopyToAsync($err)
        if (-not $process.WaitForExit($Seconds * 1000)) {
            $timedOut = $true
            if ($PSVersionTable.PSVersion.Major -ge 7) { $process.Kill($true) }
            else { & taskkill.exe /PID $process.Id /T /F | Out-Null }
            $process.WaitForExit()
        }
        $outTask.GetAwaiter().GetResult()
        $errTask.GetAwaiter().GetResult()
        $exitCode = $process.ExitCode
    } finally {
        $watch.Stop()
        $out.Dispose()
        $err.Dispose()
        $process.Dispose()
    }
    if ($timedOut -or $exitCode -ne 0) {
        foreach ($suffix in @('.stdout.log', '.stderr.log')) {
            Get-Content -LiteralPath ($LogPrefix + $suffix) -Tail 30 | ForEach-Object { Write-Host $_ }
        }
    }
    return [pscustomobject]@{ ExitCode=$exitCode; TimedOut=$timedOut; Seconds=$watch.Elapsed.TotalSeconds }
}
