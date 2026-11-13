<#
.SYNOPSIS
  Lane A55: one UBT run of the editor target with extra UBT arguments (static analyzer, timing), the same way tools/run/build.ps1 builds
  (Build.bat, -WaitMutex so a concurrent lane's build is waited out, watched, log in Game\logs). PASS/FAIL and a JSON line last.
.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\analysis\ubt.ps1 -Label analyze-msvc -Extra '-StaticAnalyzer=VisualCpp -StaticAnalyzerProjectOnly'
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\analysis\ubt.ps1 -Label timing -Extra '-Timing'
.NOTES
  Any -StaticAnalyzer forces bDisableLinking and bIgnoreBuildOutputs (UEBuildWindows.cs): no binaries come out, and the next plain build recompiles.
#>
param([Parameter(Mandatory = $true)][string]$Label, [string]$Extra = '', [int]$TimeoutSec = 3600)
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repo 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $repo 'Game')
$engine = Get-EngineRoot $uproject
Set-EngineDotnet $engine
$buildBat = Join-Path $engine 'Engine\Build\BatchFiles\Build.bat'
$logDir = Get-LogDir (Join-Path $repo 'Game')
$log = Join-Path $logDir ("a55-{0}-{1}.log" -f $Label, (Get-Stamp))
$argLine = ('ThirdJamEditor Win64 Development -Project="{0}" -WaitMutex -NoHotReloadFromIDE {1}' -f $uproject, $Extra)
Write-Host "INFO ubt: $buildBat $argLine"
Write-Host "INFO ubt: log $log"
$r = Invoke-Watched -FilePath $buildBat -Arguments $argLine -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB (Get-MinFreeGB -1) -DiskPath $uproject -Label $Label
$x = @{ label = $Label; seconds = $r.seconds; log = $log; exit = $r.exit }
if ($r.timedOut -or $r.exit -ne 0) { foreach ($l in (Get-ErrorLines $log)) { Write-Host "  | $l" }; Write-Step 'FAIL' $Label ("exit {0} after {1}s" -f $r.exit, $r.seconds); Exit-WithResult 'ubt' 1 $x }
Write-Step 'PASS' $Label ("{0}s" -f $r.seconds)
Exit-WithResult 'ubt' 0 $x
