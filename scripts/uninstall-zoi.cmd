@echo off
setlocal
cd /d "%TEMP%"
(
    "%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0uninstall-zoi.ps1" -RemoveLibrary %*
    if errorlevel 1 (
        pause
        exit 1
    ) else (
        pause
        exit 0
    )
)
