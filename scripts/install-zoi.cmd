@echo off
"%SystemRoot%\System32\WindowsPowerShell\v1.0\powershell.exe" -NoProfile -ExecutionPolicy Bypass -File "%~dp0install-zoi.ps1" %*
set "zoi_exit=%errorlevel%"
pause
exit /b %zoi_exit%
