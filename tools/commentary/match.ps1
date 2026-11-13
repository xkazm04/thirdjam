<#
.SYNOPSIS
  Lane A90: one headless bot match for the commentator's proof. UnrealEditor-Cmd -game -nullrhi on the Rink map with -cut=<cut>, the human scripted
  (jam.ScriptHuman 1, so every skater plays itself), t.MaxFPS 60; stops the process it started once the log shows a sim frame at or past -Frames (or after
  -TimeoutSec), then counts the JAMVO lines per situation and the JAMEVENT check / knockdown / lift / goal lines.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\commentary\match.ps1 -Cut hockey.regular -Name pro
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\commentary\match.ps1 -Cut hockey.mode.bruisers -Name bruisers

.NOTES
  Output: tools\commentary\out\<Name>\game.log and counts.json. PASS/FAIL lines and a JSON line last. Only the process this script started is ever stopped.
#>
param(
    [string]$Cut = 'hockey.regular',
    [string]$Name = 'pro',
    [int]$Frames = 10800,
    [int]$TimeoutSec = 600,
    [string]$Cvars = '',
    [string]$Exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
)
$ErrorActionPreference = 'Continue'
$wt = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$project = Join-Path $wt 'Game\ThirdJam.uproject'
$out = Join-Path $PSScriptRoot "out\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
if (-not (Test-Path $Exe)) { Write-Host "FAIL engine missing $Exe"; Write-Host '{"tool":"thirdjam.commentary.match/1","verdict":"could_not_run","exit":2}'; exit 2 }
$dp = 't.MaxFPS=60,jam.ScriptHuman=1'
if ($Cvars) { $dp = "$dp,$Cvars" }
$args = "`"$project`" /Game/Maps/Rink -game -nullrhi -unattended -nosplash -NoSound -log -abslog=`"$log`" -cut=$Cut -dpcvars=$dp"
$p = Start-Process -FilePath $Exe -ArgumentList $args -PassThru
Write-Host ("INFO match: cut {0}, until frame {1}, pid {2}, log {3}" -f $Cut, $Frames, $p.Id, $log)
$sw = [Diagnostics.Stopwatch]::StartNew()
$last = 0
while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    Start-Sleep -Seconds 5
    $p.Refresh()
    if ($p.HasExited) { break }
    if (Test-Path $log) {
        $m = Select-String -Path $log -Pattern 'JAMEVENT \S+ .* frame=(\d+)' | Select-Object -Last 1
        if ($m) { $last = [int64]$m.Matches[0].Groups[1].Value }
        if ($last -ge $Frames) { break }
    }
}
$p.Refresh()
if (-not $p.HasExited) { Write-Host ("INFO match: stopping my pid {0} at frame {1} after {2}s" -f $p.Id, $last, [int]$sw.Elapsed.TotalSeconds); Stop-Process -Id $p.Id -Force -Confirm:$false }
Start-Sleep -Seconds 1

$vo = @{}; $ev = @{}
$maxFrame = 0
foreach ($l in (Get-Content $log -ErrorAction SilentlyContinue)) {
    if ($l -match 'JAMVO id=(\S+)\.\d+ event=(\S+) frame=(\d+)') { $k = $Matches[1]; $vo[$k] = 1 + [int]$vo[$k] }
    elseif ($l -match 'JAMEVENT (\S+) who=\S+ .* strength=([0-9.]+) frame=(\d+) kind=(\S*)') {
        $f = [int64]$Matches[3]; if ($f -le $Frames) { if ($f -gt $maxFrame) { $maxFrame = $f }; $k = "$($Matches[1]).$($Matches[4])"; $ev[$k] = 1 + [int]$ev[$k] }
    }
}
$rough = 'check.light', 'check.solid', 'knockdown.big', 'check.cross', 'knockdown.human', 'knockdown.massacre', 'goal.after_hit'
foreach ($k in ($vo.Keys | Sort-Object)) { Write-Host ("  JAMVO {0,-26} {1}" -f $k, $vo[$k]) }
foreach ($k in ($ev.Keys | Sort-Object)) { Write-Host ("  JAMEVENT {0,-26} {1}" -f $k, $ev[$k]) }
$missing = @($rough | Where-Object { -not $vo.ContainsKey($_) })
$res = [ordered]@{ tool = 'thirdjam.commentary.match/1'; cut = $Cut; frames = $maxFrame; jamvo = $vo; events = $ev; rough_missing = $missing }
($res | ConvertTo-Json -Depth 4) | Set-Content -Encoding utf8 (Join-Path $out 'counts.json')
Write-Host ("{0} match {1}: {2} frames, {3} JAMVO lines, rough situations not spoken: {4}" -f ($(if ($missing.Count) { 'INFO' } else { 'PASS' })), $Cut, $maxFrame, (($vo.Values | Measure-Object -Sum).Sum), ($missing -join ', '))
Write-Host (($res | ConvertTo-Json -Depth 4 -Compress))
