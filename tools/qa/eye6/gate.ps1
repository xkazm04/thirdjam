# Lane A227: the full Jam filter on this worktree's editor binary, as tools\gate\premerge.ps1 runs it (no git, no ownership step), its pid recorded.
param([string]$Filter = 'Jam', [string]$Tag = 'gate1')
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$uproject = Join-Path $root 'Game\ThirdJam.uproject'
$cmd = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$abslog = Join-Path $root "Game\logs\a227\$Tag-ue.log"
$a = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}"' -f $uproject, $Filter, $abslog)
$p = Start-Process -FilePath $cmd -ArgumentList $a -PassThru -WindowStyle Hidden
"$($p.Id) $($p.StartTime.ToString('o')) $Tag" | Set-Content (Join-Path $root "Game\logs\a227\$Tag.pid")
$p.WaitForExit()
$ok = @(Select-String -Path $abslog -Pattern 'Test Completed\. Result=\{Success\}').Count
$bad = @(Select-String -Path $abslog -Pattern 'Test Completed\. Result=\{Fail')
$found = (Select-String -Path $abslog -Pattern 'Found (\d+) automation tests' | Select-Object -First 1)
Write-Host "GATE $Tag exit $($p.ExitCode): $ok Success, $($bad.Count) Fail; $found"
foreach ($b in $bad) { Write-Host "  $($b.Line)" }
