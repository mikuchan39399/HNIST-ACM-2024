param([string]$Workspace='',[string]$SettingsFile='')
$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot 'zoi_setup.ps1')
$root=Split-Path -Parent $PSScriptRoot
$paths=Setup-Paths $SettingsFile ''
Write-Host ('[LIBRARY] '+$root)
Write-Host ('[PROFILE] '+$paths.settings)
Write-Host ('[TASKS] '+$paths.tasks)
Write-Host ('[HEADER] bit.h exists: '+[IO.File]::Exists((Join-Path $root 'zoi/bit.h')))
$compiler=Get-Command g++ -ErrorAction SilentlyContinue
if ($compiler) { Write-Host ('[COMPILER] '+$compiler.Source) } else { Write-Host '[WARN] g++ not on PATH; configure your compilerPath separately.' }
$inputs=@($paths.settings,$paths.tasks)
if ($Workspace) {
    $inputs+=@(Join-Path $Workspace '.vscode/settings.json'; Join-Path $Workspace '.vscode/c_cpp_properties.json')
}
foreach ($p in $inputs) {
    Write-Host ('[CHECK] '+$p)
    if (-not [IO.File]::Exists($p)) { Write-Host '  absent'; continue }
    try {
        $text=Setup-Read $p; $null=JC-Parse $text
        foreach ($key in @('C_Cpp.autocomplete','C_Cpp.errorSquiggles','C_Cpp.intelliSenseEngine','C_Cpp.default.includePath')) {
            $v=JC-Get $text $key; if ($null -ne $v) { Write-Host ('  '+$key+' = '+$v) }
        }
        $v=JC-Get $text 'tasks'; if ($v) { $ts=JC-Value $v; Write-Host ('  zoi tasks: '+(@($ts | Where-Object label -Like 'zoi-*').Count)) }
        $v=JC-Get $text 'configurations'
        if ($v) { foreach ($c in (JC-Value $v)) { Write-Host ('  configuration '+$c.name+': '+(Setup-Json $c)) } }
    } catch { Write-Host ('[FAIL] invalid JSONC: '+$_.Exception.Message) }
}
Write-Host '[CHECK] In the active VS Code profile enable ms-vscode.cpptools. The installer does not install extensions.'
Write-Host '[CHECK] Providers/compile_commands and .code-workspace settings can override folder/user defaults.'
Write-Host '[RUN] Ctrl+Alt+T or Ctrl+P then task zoi-. Select a saved .cpp before expand; use double-quoted includes.'
