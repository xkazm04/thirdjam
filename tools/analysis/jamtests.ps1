<#
.SYNOPSIS
  Lane A55: the gate's automation step alone (tools/gate/premerge.ps1 step 5, the same UnrealEditor-Cmd invocation), for a fix loop that must not run
  ownership, ledger, node tests or the dev log each time. Prints PASS/FAIL, the Jam.Core.Determinism line, and a JSON line last.
.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\analysis\jamtests.ps1
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\analysis\jamtests.ps1 -Filter Jam.Core
#>
param([string]$Filter = 'Jam', [int]$TimeoutSec = 1200)
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repo 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $repo 'Game')
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject under $repo\Game"; Exit-WithResult 'jamtests' 2 }
$engine = Get-EngineRoot $uproject
$cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path $cmd)) { Write-Step 'FAIL' 'automation' 'UnrealEditor-Cmd.exe not found (set UE_ROOT)'; Exit-WithResult 'jamtests' 2 }
$logDir = Get-LogDir (Join-Path $repo 'Game')
$stamp = Get-Stamp
$log = Join-Path $logDir ("a55-automation-{0}.log" -f $stamp)
$abslog = Join-Path $logDir ("a55-automation-ue-{0}.log" -f $stamp)
$ue = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}"' -f $uproject, $Filter, $abslog)
$w = Invoke-Watched -FilePath $cmd -Arguments $ue -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB (Get-MinFreeGB 40) -DiskPath $uproject -Label 'automation'
$src = $log; if (Test-Path $abslog) { $src = $abslog }
$ok = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Success\}' -ErrorAction SilentlyContinue).Count
$bad = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Fail[^}]*\}' -ErrorAction SilentlyContinue)
$det = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{[^}]*\} Name=\{Determinism\}' -ErrorAction SilentlyContinue)
foreach ($d in $det) { Write-Host "  | $($d.Line.Trim())" }
$extra = @{ pass = $ok; fail = $bad.Count; seconds = $w.seconds; log = $src; filter = $Filter }
if ($w.timedOut) { Write-Step 'FAIL' 'automation' "timeout after $($w.seconds)s (log $src)"; Exit-WithResult 'jamtests' 1 $extra }
if ($bad.Count -gt 0) { foreach ($b in ($bad | Select-Object -First 10)) { Write-Host "  | $($b.Line.Trim())" }; Write-Step 'FAIL' 'automation' "$ok passed, $($bad.Count) failed (log $src)"; Exit-WithResult 'jamtests' 1 $extra }
if ($ok -eq 0) { Write-Step 'FAIL' 'automation' "no test result lines (exit $($w.exit), log $src)"; Exit-WithResult 'jamtests' 1 $extra }
Write-Step 'PASS' 'automation' "$ok tests passed (filter '$Filter', $($w.seconds)s)"
Exit-WithResult 'jamtests' 0 $extra
