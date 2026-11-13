<#
.SYNOPSIS
  Builds a project target with Unreal Build Tool (Build.bat). One command, explicit timeout, log in <project>\logs.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\build.ps1 -Project C:\jam\Game\Game.uproject
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\build.ps1 -Project C:\jam\Game -Target Game -Config Shipping

.NOTES
  -Target Editor (default) builds <Name>Editor Win64 Development: what the editor and MCP need.
  The editor target has no Shipping configuration, so -Config Shipping implies -Target Game (a pre-package compile check).
  Exit: 0 built, 1 build failed or timed out, 2 could not run.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [ValidateSet('Development', 'Shipping', 'DebugGame')][string]$Config = 'Development',
    [ValidateSet('Editor', 'Game')][string]$Target = 'Editor',
    [int]$TimeoutSec = 3600,
    [double]$MinFreeGB = -1
)
. "$PSScriptRoot\common.ps1"

$uproject = Resolve-UProject $Project
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject at '$Project'"; Exit-WithResult 'build' 2 }
$engine = Get-EngineRoot $uproject
Set-EngineDotnet $engine
if (-not $engine) { Write-Step 'FAIL' 'engine' 'Unreal Engine install not found (set UE_ROOT)'; Exit-WithResult 'build' 2 }
$buildBat = Join-Path $engine 'Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path $buildBat)) { Write-Step 'FAIL' 'engine' "missing $buildBat"; Exit-WithResult 'build' 2 }

if ($Config -eq 'Shipping' -and $Target -eq 'Editor') { Write-Host 'INFO build: the editor target has no Shipping configuration; building the Game target instead'; $Target = 'Game' }

$floor = Get-MinFreeGB $MinFreeGB
$free = Get-FreeGB $uproject
if ($free -lt $floor) { Write-Step 'FAIL' 'disk' "free $free GB is below the floor of $floor GB"; Exit-WithResult 'build' 2 }

$projDir = Split-Path $uproject -Parent
$name = [System.IO.Path]::GetFileNameWithoutExtension($uproject)
$targetName = $name; if ($Target -eq 'Editor') { $targetName = $name + 'Editor' }
$logDir = Get-LogDir $projDir
$log = Join-Path $logDir ("build-{0}-{1}-{2}.log" -f $targetName, $Config, (Get-Stamp))

# Parallel builds across worktrees (Director, 13 Nov 22:10). Build.bat takes a lock file under %TMP% and UBT a machine-wide mutex, so every worktree's build
# queued behind every other (a one-minute rebuild waited up to 47 minutes with a dozen lanes). Each project now builds with its own TMP (its own Build.bat
# lock), -NoMutex (no machine-wide UBT mutex: each worktree has its own Intermediate; the installed engine is read-only) and -NoUBA (the accelerator's
# store wants exclusive access), in one of a few slots so the machine is not flooded. TJ_SERIAL_BUILD=1 gives back the old serial build.
$mutexArgs = '-WaitMutex'
$slot = $null
$oldTmp = $env:TMP; $oldTemp = $env:TEMP
if (-not $env:TJ_SERIAL_BUILD) {
    $mutexArgs = '-NoMutex -NoUBA'
    $md5 = [System.Security.Cryptography.MD5]::Create()
    $key = ([System.BitConverter]::ToString($md5.ComputeHash([System.Text.Encoding]::UTF8.GetBytes($projDir.ToLowerInvariant())))).Replace('-', '').Substring(0, 12)
    $buildTmp = Join-Path (Join-Path $env:LOCALAPPDATA 'Temp') ('tj-build-' + $key)
    New-Item -ItemType Directory -Force -Path $buildTmp | Out-Null
    $env:TMP = $buildTmp; $env:TEMP = $buildTmp
    $maxSlots = 3; if ($env:TJ_BUILD_SLOTS) { $maxSlots = [int]$env:TJ_BUILD_SLOTS }
    $waited = 0
    while (-not $slot) {
        for ($i = 0; $i -lt $maxSlots -and -not $slot; $i++) {
            $m = New-Object System.Threading.Mutex($false, "tj-build-slot-$i")
            try { if ($m.WaitOne(0)) { $slot = $m } else { $m.Dispose() } } catch [System.Threading.AbandonedMutexException] { $slot = $m }
        }
        if (-not $slot) { if ($waited -eq 0) { Write-Host "INFO build: all $maxSlots build slots are taken; waiting" }; Start-Sleep -Seconds 5; $waited += 5 }
    }
    if ($waited -gt 0) { Write-Host "INFO build: got a slot after ${waited}s" }
}
$argLine = ('{0} Win64 {1} -Project="{2}" {3} -NoHotReloadFromIDE -FromMsBuild' -f $targetName, $Config, $uproject, $mutexArgs)
Write-Host "INFO build: $buildBat $argLine"
Write-Host "INFO build: log $log"
$r = Invoke-Watched -FilePath $buildBat -Arguments $argLine -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB $floor -DiskPath $uproject -Label 'build'
$env:TMP = $oldTmp; $env:TEMP = $oldTemp
if ($slot) { try { $slot.ReleaseMutex() } catch {}; $slot.Dispose() }

$extra = @{ target = $targetName; config = $Config; seconds = $r.seconds; log = $log; free_gb = (Get-FreeGB $uproject) }
if ($r.timedOut) { Write-Step 'FAIL' 'build' ("timeout after {0}s (limit {1}s); see log" -f $r.seconds, $TimeoutSec); Exit-WithResult 'build' 1 $extra }
if ($r.diskStop) { Write-Step 'FAIL' 'build' 'stopped: disk below floor'; Exit-WithResult 'build' 1 $extra }
if ($r.exit -ne 0) {
    foreach ($l in (Get-ErrorLines $log)) { Write-Host "  | $l" }
    Write-Step 'FAIL' 'build' ("{0} {1} exit {2} after {3}s" -f $targetName, $Config, $r.exit, $r.seconds)
    Exit-WithResult 'build' 1 $extra
}
Write-Step 'PASS' 'build' ("{0} {1} in {2}s" -f $targetName, $Config, $r.seconds)
Exit-WithResult 'build' 0 $extra
