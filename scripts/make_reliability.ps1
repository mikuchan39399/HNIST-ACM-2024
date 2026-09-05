param([switch]$Check)
# ASCII-only; stable UTF-8 Markdown, no timestamps or execution claims.
$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
. (Join-Path $PSScriptRoot 'check_inventory.ps1')
$inventory=Get-CheckInventory $root
$lines=New-Object 'Collections.Generic.List[string]'
function U([string]$hex) { -join @($hex.Split(' ') | ForEach-Object { [char][Convert]::ToInt32($_,16) }) }
function Link([string]$label,[string]$path) {
    $label=$label.Replace('\','\\').Replace('|','\|').Replace('[','\[').Replace(']','\]')
    $path=$path.Replace('%','%25').Replace('#','%23').Replace(' ','%20').Replace('<','%3C').Replace('>','%3E')
    return '['+$label+'](<../'+$path+'>)'
}
$lines.Add((U '23 20 6a21 677f 5bf9 62cd 8d44 4ea7 8868 ff08 81ea 52a8 751f 6210 ff09'))
$lines.Add('')
$lines.Add((U '3c 21 2d 2d 20 7531 20 73 63 72 69 70 74 73 2f 6d 61 6b 65 5f 72 65 6c 69 61 62 69 6c 69 74 79 2e 70 73 31 20 81ea 52a8 751f 6210 ff0c 8bf7 52ff 624b 5de5 4fee 6539 3002 20 2d 2d 3e'))
$lines.Add('')
$lines.Add((U '41 2f 42 2f 43 20 53ea 8868 793a 9759 6001 6d4b 8bd5 8d44 4ea7 5173 7cfb ff0c 4e0d 4ee3 8868 53ef 9760 7a0b 5ea6 6216 6b63 786e 6027 3002 76f4 63a5 20 69 6e 63 6c 75 64 65 ff08 5305 62ec 6761 4ef6 5206 652f 4e2d 7684 5f15 7528 ff09 4e0d 80fd 8bc1 660e 63a5 53e3 88ab 8c03 7528 3001 66b4 529b 7b54 6848 72ec 7acb 6216 6d4b 8bd5 5df2 901a 8fc7 ff1b 4e0d 7edf 8ba1 5b8f 5f0f 5f15 7528 548c 4f20 9012 4f9d 8d56 3002'))
$lines.Add('')
$lines.Add((U '6570 636e 6765 81ea 20 7a 6f 69 2f 5f 63 61 74 61 6c 6f 67 2e 74 78 74 20 548c 20 61 6c 67 6f 72 69 74 68 6d 73 2f 2a 2a 2f 2a 5f 63 68 65 63 6b 2e 63 70 70 3002 7eaf 8df3 677f 89e3 6790 5230 6bcd 7248 ff0c 5ffd 7565 6ce8 91ca 4e2d 7684 793a 4f8b 3002 8fd0 884c 7ed3 679c 3001 73af 5883 548c 5386 53f2 5ba1 67e5 8bb0 5f55 89c1 8fd0 884c 62a5 544a 4e0e 5b 534f 4f5c 5386 53f2 5d 28 73 77 65 65 70 2d 68 69 73 74 6f 72 79 2e 6d 64 29 3002'))
$lines.Add('')
$lines.Add((U '66f4 65b0 ff1a 60 2e 2f 73 63 72 69 70 74 73 2f 6d 61 6b 65 5f 72 65 6c 69 61 62 69 6c 69 74 79 2e 70 73 31 60 ff1b 53ea 8bfb 68c0 67e5 ff1a 60 2e 2f 73 63 72 69 70 74 73 2f 6d 61 6b 65 5f 72 65 6c 69 61 62 69 6c 69 74 79 2e 70 73 31 20 2d 43 68 65 63 6b 60 3002 666e 901a 56de 5f52 4e0d 4f1a 6539 5199 672c 8868 3002'))
$groups=@(
    @{title=(U '41 20 7c7b ff1a 6709 76f4 63a5 5bf9 62cd 5f15 7528 7684 73b0 5f79 6a21 677f'); entries=@($inventory.entries | Where-Object { $_.kind -eq 'engine' -and $_.suites.Count -gt 0 })},
    @{title=(U '42 20 7c7b ff1a 672a 53d1 73b0 76f4 63a5 5bf9 62cd 5f15 7528 7684 73b0 5f79 6a21 677f'); entries=@($inventory.entries | Where-Object { $_.kind -eq 'engine' -and $_.suites.Count -eq 0 })},
    @{title=(U '43 20 7c7b ff1a 7b14 8bb0 6761 76ee ff08 4e0d 53c2 4e0e 7f16 8bd1 ff09'); entries=@($inventory.entries | Where-Object { $_.kind -eq 'note' })},
    @{title=(U '8c41 514d 4ee3 7801 ff1a 65e0 76ee 5f55 8df3 677f 7684 20 43 2b 2b 20 6587 4ef6 ff08 4e0d 7b49 540c 4e8e 7b14 8bb0 ff09'); entries=@($inventory.entries | Where-Object { $_.kind -eq 'exempt' })}
)
foreach ($group in $groups) {
    $lines.Add(''); $lines.Add('## '+$group.title+' ['+$group.entries.Count+']'); $lines.Add('')
    $lines.Add((U '7c 20 6a21 677f 6216 6e90 6587 4ef6 20 7c 20 76f4 63a5 5f15 7528 5b83 7684 5bf9 62cd 6587 4ef6 20 7c')); $lines.Add('|---|---|')
    foreach ($e in $group.entries) {
        $label=$e.path; if ($e.name) { $label=$e.name+' - '+$e.path }
        $refs=@($e.suites | ForEach-Object { Link $_ $_ })
        $value=(U '672a 53d1 73b0 76f4 63a5 5f15 7528'); if ($e.kind -eq 'note') { $value=(U '4e0d 9002 7528 ff08 7b14 8bb0 ff09') }
        elseif ($refs.Count) { $value=$refs -join '<br>' }
        $lines.Add('| '+(Link $label $e.path)+' | '+$value+' |')
    }
}
$lines.Add(''); $lines.Add((U '23 23 20 5df2 53d1 73b0 7684 5bf9 62cd 5957 4ef6 20 5b')+$inventory.checks.Count+']'); $lines.Add('')
foreach ($suite in $inventory.checks) { $lines.Add('- '+(Link $suite $suite)) }
$text=($lines -join "`n")+"`n"
$enc=New-Object Text.UTF8Encoding($false)
$output=Join-Path $root 'rules/reliability.md'
$same=[IO.File]::Exists($output) -and [IO.File]::ReadAllText($output,$enc).Replace("`r`n","`n") -ceq $text
if ($Check) {
    if (-not $same) { Write-Host '[FAIL] reliability.md is stale; run scripts/make_reliability.ps1'; exit 1 }
    Write-Host '[OK] reliability.md is current'; exit 0
}
if (-not $same) { [IO.File]::WriteAllText($output,$text,$enc) }
Write-Host ('[OK] reliability.md: '+$inventory.entries.Count+' assets, '+$inventory.checks.Count+' suites')
