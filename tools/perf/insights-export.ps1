<#
.SYNOPSIS
  Lane A58: headless Unreal Insights export of a .utrace (no UI): timer statistics per thread and the timing events of a time window.
  UnrealInsights.exe -OpenTraceFile=<utrace> -NoUI -AutoQuit -nullrhi -ExecOnAnalysisCompleteCmd="<cmd>" (the form of UAT's
  ExportTimerStatisticsFromUtrace.cs). One command per Insights run (each run re-analyses the trace: ~20 s for a 0.6 GB trace).
  The value must be quoted after the '=' (-ExecOnAnalysisCompleteCmd="..."); a whole-token-quoted argument is ignored without an error.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\perf\insights-export.ps1 -Trace tools\perf\out\l2_night\trace.utrace -Cmd "TimingInsights.ExportTimerStatistics {out}\timers-gt.csv -threads=GameThread -sortBy=TotalInclusiveTime"
  {out} is replaced by <trace dir>\insights.
#>
param(
    [Parameter(Mandatory = $true)][string]$Trace,
    [Parameter(Mandatory = $true)][string]$Cmd,
    [int]$TimeoutSec = 600
)
$ui = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealInsights.exe'
$Trace = (Resolve-Path $Trace).Path
$dir = Join-Path (Split-Path $Trace -Parent) 'insights'
New-Item -ItemType Directory -Force $dir | Out-Null
$c = $Cmd.Replace('{out}', $dir)
$log = Join-Path $dir ('insights-{0}.log' -f (Get-Date -Format 'HHmmssfff'))
$a = "-OpenTraceFile=`"$Trace`" -unattended -autoquit -noui -nullrhi -ExecOnAnalysisCompleteCmd=`"$c`" -abslog=`"$log`""
$p = Start-Process -FilePath $ui -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Stop-Process -Id $p.Id -Force; Write-Host "FAIL insights: timeout after $TimeoutSec s"; exit 1 }
$lines = Select-String -Path $log -Pattern 'TimingInsights\.|Filename|Exported|Error: \[(?!MemAlloc)' | Select-Object -First 12
foreach ($l in $lines) { Write-Host "  | $($l.Line)" }
Write-Host "PASS insights: exit $($p.ExitCode), log $log"
