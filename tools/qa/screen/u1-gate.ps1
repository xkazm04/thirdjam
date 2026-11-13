# Track U1 (lanes A123, A135): the automation step of tools\gate\premerge.ps1 on this worktree (the full Jam filter by default, headless, -Multiprocess),
# with the result lines counted and the lines the track's proofs read printed (hashes, JAMSCREEN lines of the tests).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-gate.ps1 -Tag run1 [-Filter Jam]
# The log: Game\logs\u1-automation-<Tag>.log (copied from Game\Saved\Logs where the engine does not honour -ABSLOG). Exit 0: every test Success.
param([string]$Tag = 'run1', [string]$Filter = 'Jam', [int]$TimeoutSec = 1500)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
New-Item -ItemType Directory -Force (Join-Path $root 'Game\logs') | Out-Null
$log = Join-Path $root "Game\logs\u1-automation-$Tag.log"
Remove-Item $log -ErrorAction SilentlyContinue
$marker = "u1gate=$Tag-$PID"
$a = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}" -{3}' -f $uproject, $Filter, $log, $marker)
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $cmd -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host 'FAIL gate: timeout (the process is left to the caller)'; exit 1 }
if (-not (Test-Path $log)) {
    Get-ChildItem (Join-Path $root 'Game\Saved\Logs') -Filter '*.log' -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $start.AddSeconds(-2) } | ForEach-Object {
        if (-not (Test-Path $log) -and (Select-String -Path $_.FullName -Pattern $marker -SimpleMatch -Quiet)) { Copy-Item $_.FullName $log -Force }
    }
}
if (-not (Test-Path $log)) { Write-Host 'FAIL gate: no log found'; exit 1 }
$t = Get-Content $log
$ok = @($t | Select-String -Pattern 'Test Completed. Result=\{Success\}').Count
$all = @($t | Select-String -Pattern 'Test Completed. Result=').Count
$bad = $t | Select-String -Pattern 'Test Completed. Result=\{(?!Success)'
Write-Host ("gate {0}: {1} of {2} Success in {3:n0} s, exit {4}; log {5}" -f $Tag, $ok, $all, $sw.Elapsed.TotalSeconds, $p.ExitCode, $log)
$bad | ForEach-Object { Write-Host "  BAD $($_.Line)" }
$t | Select-String -Pattern 'LogAutomationController: Error' | Select-Object -First 30 | ForEach-Object { Write-Host "  ERR $($_.Line)" }
$t | Select-String -Pattern 'state_hash|JAMSCREEN keep|JAMSCREEN bark lane|JAMSCREEN prompt keys|hockey\.regular|hockey\.raw|seam' | Select-Object -First 40 | ForEach-Object { Write-Host "  | $($_.Line)" }
if ($all -gt 0 -and $ok -eq $all) { exit 0 } else { exit 1 }
