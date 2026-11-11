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

$argLine = ('{0} Win64 {1} -Project="{2}" -WaitMutex -NoHotReloadFromIDE -FromMsBuild' -f $targetName, $Config, $uproject)
Write-Host "INFO build: $buildBat $argLine"
Write-Host "INFO build: log $log"
$r = Invoke-Watched -FilePath $buildBat -Arguments $argLine -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB $floor -DiskPath $uproject -Label 'build'

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
