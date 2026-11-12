# Lane A38: the two-process lockstep proof, unattended and headless. Starts a host and a guest game process on this machine
# (UnrealEditor-Cmd -game -nullrhi), the host on UDP 7700 with "jam.host <cut> Ada auto", the guest with "jam.join 127.0.0.1:7700 Bo",
# both with the scripted human and jam.net.QuitAtFrame so they quit on their own; then compares the JAMNET hash lines of the two logs.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Cut hockey.boost -Name boost
#   powershell -File tools\net\twoproc.ps1 -Cut hockey.chaos -Name chaos-80ms -Cvars 'jam.net.SimDelayMs=40,jam.net.SimLoss=0.02'
# Prints PASS/FAIL lines and one JSON line last ({"tool":"thirdjam.net/1", ...}); exit 0 pass, 1 fail, 2 could not run.
# Only the two processes this script started are ever stopped. Logs: <OutDir>\host.log, <OutDir>\guest.log.
param(
    [string]$Cut = 'hockey.boost',
    [string]$Name = 'boost',
    [int]$Frames = 9000,
    [string]$Cvars = '',
    [int]$TimeoutSec = 480,
    [int]$GuestDelaySec = 8,
    [string]$Project = '',
    [string]$Exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor-Cmd.exe',
    [string]$OutDir = ''
)
$ErrorActionPreference = 'Continue'
if (-not $Project) { $Project = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) 'Game\ThirdJam.uproject' }
if (-not $OutDir) { $OutDir = Join-Path $PSScriptRoot "out\$Name" }
if (-not (Test-Path $Exe)) { Write-Host "FAIL engine missing $Exe"; Write-Host '{"tool":"thirdjam.net/1","verdict":"could_not_run","exit":2}'; exit 2 }
if (-not (Test-Path $Project)) { Write-Host "FAIL project missing $Project"; Write-Host '{"tool":"thirdjam.net/1","verdict":"could_not_run","exit":2}'; exit 2 }
New-Item -ItemType Directory -Force $OutDir | Out-Null
$hostLog = Join-Path $OutDir 'host.log'
$guestLog = Join-Path $OutDir 'guest.log'
Remove-Item $hostLog, $guestLog -ErrorAction SilentlyContinue
$dp = "t.MaxFPS=60,jam.ScriptHuman=1,jam.net.QuitAtFrame=$Frames"
if ($Cvars) { $dp = "$dp,$Cvars" }
$common = "`"$Project`" -game -nullrhi -unattended -nosplash -NoSound -log"
$hostArgs = "$common -abslog=`"$hostLog`" -ExecCmds=`"jam.host $Cut Ada auto`" -dpcvars=$dp"
$guestArgs = "$common -abslog=`"$guestLog`" -ExecCmds=`"jam.join 127.0.0.1:7700 Bo`" -dpcvars=$dp"
Write-Host "INFO twoproc: cut $Cut, $Frames frames, cvars $dp"
Write-Host "INFO twoproc: logs $hostLog | $guestLog"
$t0 = Get-Date
$h = Start-Process -FilePath $Exe -ArgumentList $hostArgs -PassThru
Start-Sleep -Seconds $GuestDelaySec
$g = Start-Process -FilePath $Exe -ArgumentList $guestArgs -PassThru
Write-Host ("INFO twoproc: host pid {0}, guest pid {1}, started {2}" -f $h.Id, $g.Id, $t0.ToString('HH:mm:ss'))
$sw = [Diagnostics.Stopwatch]::StartNew()
while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    $h.Refresh(); $g.Refresh()
    if ($h.HasExited -and $g.HasExited) { break }
    Start-Sleep -Seconds 2
}
$h.Refresh(); $g.Refresh()
$timedOut = -not ($h.HasExited -and $g.HasExited)
foreach ($p in @($h, $g)) { if (-not $p.HasExited) { Write-Host ("INFO twoproc: stopping my pid {0} after {1}s" -f $p.Id, [int]$sw.Elapsed.TotalSeconds); Stop-Process -Id $p.Id -Force -Confirm:$false } }
Start-Sleep -Seconds 1
$wall = [int]((Get-Date) - $t0).TotalSeconds

function Read-Frames([string]$log) {
    $m = @{}
    if (-not (Test-Path $log)) { return $m }
    foreach ($line in (Select-String -Path $log -Pattern 'JAMNET (frame|proof done) frame=(\d+) hash=([0-9a-f]+)')) {
        $f = [int]$line.Matches[0].Groups[2].Value
        $m[$f] = $line.Matches[0].Groups[3].Value
    }
    return $m
}
function Last-Match([string]$log, [string]$pattern) {
    if (-not (Test-Path $log)) { return '' }
    $r = Select-String -Path $log -Pattern $pattern | Select-Object -Last 1
    if ($r) { return $r.Line.Substring($r.Line.IndexOf('JAMNET')) } else { return '' }
}
$hf = Read-Frames $hostLog
$gf = Read-Frames $guestLog
$agree = 0; $differ = @()
foreach ($f in ($hf.Keys | Sort-Object)) { if ($gf.ContainsKey($f)) { if ($hf[$f] -eq $gf[$f]) { $agree++ } else { $differ += "f$f host=$($hf[$f]) guest=$($gf[$f])" } } }
$hostDone = Last-Match $hostLog 'JAMNET proof done'
$guestDone = Last-Match $guestLog 'JAMNET proof done'
$hostFirst = Last-Match $hostLog 'JAMNET first frame'
$guestFirst = Last-Match $guestLog 'JAMNET first frame'
$desync = (Last-Match $hostLog 'JAMNET desync') + (Last-Match $guestLog 'JAMNET desync')
$lost = (Last-Match $hostLog 'JAMNET lost') + (Last-Match $guestLog 'JAMNET lost')
$hostMax = 0; if ($hf.Count) { $hostMax = ($hf.Keys | Measure-Object -Maximum).Maximum }
$guestMax = 0; if ($gf.Count) { $guestMax = ($gf.Keys | Measure-Object -Maximum).Maximum }

$fail = $false
if ($timedOut) { Write-Host "FAIL timeout after ${TimeoutSec}s (host reached f$hostMax, guest f$guestMax)"; $fail = $true }
if ($desync) { Write-Host "FAIL desync: $desync"; $fail = $true }
if ($lost) { Write-Host "FAIL connection lost: $lost"; $fail = $true }
if ($differ.Count) { Write-Host ("FAIL hashes differ at {0} checkpoint lines: {1}" -f $differ.Count, ($differ -join '; ')); $fail = $true }
if ($hostMax -lt $Frames -or $guestMax -lt $Frames) { Write-Host "FAIL frames: host reached f$hostMax, guest f$guestMax of $Frames"; $fail = $true }
if ($agree -lt [math]::Floor($Frames / 600)) { Write-Host "FAIL only $agree hash lines compared (expected at least $([math]::Floor($Frames / 600)))"; $fail = $true }
if (-not $fail) { Write-Host ("PASS hashes {0} of {0} logged checkpoints (every 600 frames) identical on host and guest, both reached f{1}, {2}s wall" -f $agree, $Frames, $wall) }
Write-Host "INFO host:  $hostDone"
Write-Host "INFO guest: $guestDone"
Write-Host "INFO host first frame:  $hostFirst"
Write-Host "INFO guest first frame: $guestFirst"
$code = 0; if ($fail) { $code = 1 }
$obj = [ordered]@{ tool = 'thirdjam.net/1'; verdict = @('pass', 'fail')[$code]; exit = $code; cut = $Cut; frames = $Frames; agree = $agree; differ = $differ.Count; wall_s = $wall; cvars = $dp; host_log = $hostLog; guest_log = $guestLog; host_done = $hostDone; guest_done = $guestDone; host_first = $hostFirst; guest_first = $guestFirst }
Write-Host ($obj | ConvertTo-Json -Compress)
exit $code
