<#
  Lane A151 (lane A132's copy of a92-tests.ps1 with -Multiprocess): run an automation filter headless on this worktree's editor build, the gate's invocation (tools/gate/premerge.ps1 step 5), and print the counts.
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a92-tests.ps1 -Filter Jam -Name final
  Logs: Game\logs\a92-tests-<Name>-ue.log. Exit 0 all green, 1 a failure or no result lines.
#>
param([string]$Filter = 'Jam', [string]$Name = 'run', [int]$TimeoutSec = 3000)
. "$PSScriptRoot\..\..\run\common.ps1"
$repo = (Resolve-Path "$PSScriptRoot\..\..\..").Path
$uproject = Join-Path $repo 'Game\ThirdJam.uproject'
$engine = Get-EngineRoot $uproject
$cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$logDir = Join-Path $repo 'Game\logs'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$abslog = Join-Path $logDir ("a151-tests-{0}-ue.log" -f $Name)
$out = Join-Path $logDir ("a151-tests-{0}.log" -f $Name)
Remove-Item $abslog -Force -ErrorAction SilentlyContinue
$ue = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}"' -f $uproject, $Filter, $abslog)
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $cmd -ArgumentList $ue -PassThru -WindowStyle Hidden -RedirectStandardOutput $out
if (-not $p.WaitForExit($TimeoutSec * 1000)) { try { $p.Kill() } catch {}; Write-Host "FAIL automation: timeout after $TimeoutSec s"; exit 1 }
$src = $out; if (Test-Path $abslog) { $src = $abslog }
$ok = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Success\}').Count
$bad = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Fail[^}]*\}')
$flag = @(Select-String -Path $src -Pattern 'missing usage flag').Count
foreach ($b in $bad) { Write-Host "  | $($b.Line.Trim())" }
Write-Host ("automation filter '{0}': {1} PASS, {2} FAIL, usage-flag lines {3}, {4:n0} s, log {5}" -f $Filter, $ok, $bad.Count, $flag, $sw.Elapsed.TotalSeconds, $src)
if ($bad.Count -gt 0 -or $ok -eq 0) { exit 1 } else { exit 0 }
