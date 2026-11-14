# Lane A198 (track S3, evolution 5): a copy of tools/net/twoproc.ps1 (lane A38/A120/A129/A140) with two more modes, kept in the hardening family:
#   -Mode guestleaves   the GUEST quits at lockstep frame -LeaveAt (says BYE): PASS = the host's card names it, the host does not freeze.
#   -PauseAt S -PauseFor D -GuestRcPort P   (play mode) the GUEST opens its pause card (jam.pausemenu open, over its remote console on 127.0.0.1:P) S seconds
#                       after its first lockstep frame and closes it D seconds later: the pause of one machine in an online match.
#   -Mode latejoin      a THIRD process (a second guest, port -LatePort) joins the host -LateDelaySec after the guest, mid-match: PASS = the match
#                       of host and guest goes on with equal hashes (no desync, no lost connection), and the late guest is told with words or times out
#                       on its own card (it never takes the seat). Its log: <OutDir>\late.log.
# Lane A204 (track N1): copied from lane A198's tree (s3b) with one more switch:
#   -HostRcPort P -HostPauseAt S   the HOST opens its pause card S seconds after the guest's first lockstep frame and closes it -PauseFor later (after the
#                       guest's pause when both are given: the ruling's "a pause of 10 s on the guest and then on the host"). PASS also needs each paused
#                       machine to log the refused world pause and its seat's neutral input (JAMNET pause card open / closed).
#   -GuestCardQuitAt S (with -GuestRcPort)   the GUEST presses QUIT on its pause card (jam.pausemenu quit) S seconds after its first frame: PASS = the
#                       host's card says the guest left (words, not a frozen picture); both processes are then stopped by their recorded ids.
#   -CutAt S -CutMs M (with -GuestRcPort)   the GUEST's network is cut both ways for M ms (jam.net.testcut) S seconds after its first frame. Under 3 s:
#                       PASS = the play checks (no connection lost, hashes equal). With -QuitBoth (a cut of 3 s or more, -HostRcPort too): PASS = both
#                       cards say the connection was lost within 3.5 s of the cut, then QUIT on each pause card takes each machine back to the menu.
#   -GuestQuitOnResult (with -GuestRcPort, -ToEnd)   on the result card the GUEST presses QUIT on its pause card: PASS = the host's card says the guest left.
#   -AskThenHostLeaves (with -HostRcPort, -ToEnd)   on the result card only the GUEST asks for the rematch (jam.net.ProofRematch on the guest); 3 s later the
#                       HOST presses QUIT on its pause card: PASS = the guest's screen said 'rematch asked' and then its card says the host left.
#   -GuestQuitInLoad (with -GuestRcPort and -Cvars jam.net.TestRinkDelayMs=8000)   the guest's game quits while its rink is still to load:
#                       PASS = the host's card says the guest left within 1 s of the guest's quit.
# Everything else as twoproc.ps1; loopback only when -HostPort/-GuestPort are given (jam.net.Loopback=1).
# Lane A38: the two-process lockstep proof, unattended and headless. Starts a host and a guest game process on this machine
# (UnrealEditor-Cmd -game -nullrhi), the host with "jam.host <cut> Ada auto", the guest with "jam.join 127.0.0.1:<port> Bo", both through the real lobby
# flow, both with the scripted human and jam.net.QuitAtFrame so they quit on their own; then compares the JAMNET hash lines of the two logs.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\net\twoproc.ps1 -Cut hockey.boost -Name boost
#   powershell -File tools\net\twoproc.ps1 -Cut hockey.chaos -Name chaos-80ms -Cvars 'jam.net.SimDelayMs=40,jam.net.SimLoss=0.02'
# Lane A120 (track Q1, evolution 3):
#   -HostPort 4720 -GuestPort 4721   fixed UDP ports, sockets bound to 127.0.0.1 only (jam.net.Port, jam.net.Loopback): no firewall prompt, beside other lanes.
#   -Verbs                            the scripted human sprints, holds the saucer, brakes, spins, checks and presses F (jam.net.ScriptVerbs 1).
#   -ToEnd                            play the match to its end (first_to / timer): both processes log "proof done" at the final frame and quit;
#                                     -Frames is then only the cap.
#   -Mode refuse                      the guest's HELLO carries protocol byte -TestVersion (another build): PASS = refused in under 1 s, the reason on its screen.
#   -Mode hostleaves                  the host quits at -LeaveAt (says BYE): PASS = the guest's card names it; -Mode hostkilled: the host process is stopped
#                                     at -LeaveAt seconds (no BYE): PASS = the connection-lost card. Both print the seconds to the card.
# Lane A129 (track Q1, evolution 4):
#   -Player good                      the scripted seat on both machines is the playtester of that class (jam.net.ScriptPlayer): it carries, charges, shoots.
#   -Rematch                          at the end both scripted seats ask for the rematch on the card (jam.net.ProofRematch 1); PASS also needs the card line for
#                                     each machine's own seat, the rematch started on both on the same lockstep frame with the same hash, and the proof line
#                                     600 frames into the second match.
#   -LiveCutAt N                      the guest tries a live cut at lockstep frame N (jam.net.TestLiveCutAt): PASS needs it refused, on its screen, hashes equal.
# Lane A140 (track Q1, evolution 5):
#   -Lan                              not loopback: no jam.net.Loopback, the host binds every address on UDP 7700 (the real game's port; -HostPort fixes another),
#                                     the guest waits for the host's invite codes in its log and joins with the CODE of -LanIp (default: the address of the
#                                     interface holding the default route), as the operator types it. Prints each machine's status lines as the screen showed them.
#   -Exe <ThirdJam.exe>               a packaged game instead of the editor binary (no project, no -game, no -Multiprocess).
#   Every play run also checks each machine's press accounting (the proof line's presses=[...] presses_lost=N): FAIL on any press lost.
# Prints PASS/FAIL lines and one JSON line last ({"tool":"thirdjam.net/1", ...}); exit 0 pass, 1 fail, 2 could not run.
# Only the two processes this script started are ever stopped. Logs: <OutDir>\host.log, <OutDir>\guest.log.
param(
    [string]$Cut = 'hockey.boost',
    [string]$Name = 'boost',
    [int]$Frames = 9000,
    [string]$Cvars = '',
    [int]$TimeoutSec = 480,
    [int]$GuestDelaySec = 8,
    [int]$HostPort = 0,
    [int]$GuestPort = 0,
    [switch]$Verbs,
    [switch]$ToEnd,
    [ValidateSet('play', 'refuse', 'hostleaves', 'hostkilled', 'guestleaves', 'latejoin')][string]$Mode = 'play',
    [int]$LatePort = 0,
    [int]$PauseAt = 0,
    [int]$PauseFor = 10,
    [int]$GuestRcPort = 0,
    [int]$HostRcPort = 0,
    [int]$HostPauseAt = 0,
    [int]$GuestCardQuitAt = 0,
    [int]$CutAt = 0,
    [int]$CutMs = 2000,
    [switch]$QuitBoth,
    [switch]$GuestQuitInLoad,
    [switch]$GuestQuitOnResult,
    [switch]$AskThenHostLeaves,
    [int]$LateDelaySec = 20,
    [int]$TestVersion = 7,
    [int]$LeaveAt = 1200,
    [string]$Player = '',
    [switch]$Rematch,
    [int]$LiveCutAt = 0,
    [string]$Project = '',
    [string]$Exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor-Cmd.exe',
    [string]$OutDir = '',
    [switch]$Lan,
    [string]$LanIp = ''
)
$ErrorActionPreference = 'Continue'
if (-not $Project) { $Project = Join-Path (Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent) 'Game\ThirdJam.uproject' }
if (-not $OutDir) { $OutDir = Join-Path $PSScriptRoot "out\$Name" }
if (-not (Test-Path $Exe)) { Write-Host "FAIL engine missing $Exe"; Write-Host '{"tool":"thirdjam.net/1","verdict":"could_not_run","exit":2}'; exit 2 }
$packaged = ($Exe -notmatch 'UnrealEditor')
if ($packaged) { $Project = '(packaged)' }
if (-not $packaged -and -not (Test-Path $Project)) { Write-Host "FAIL project missing $Project"; Write-Host '{"tool":"thirdjam.net/1","verdict":"could_not_run","exit":2}'; exit 2 }
New-Item -ItemType Directory -Force $OutDir | Out-Null
$hostLog = Join-Path $OutDir 'host.log'
$guestLog = Join-Path $OutDir 'guest.log'
Remove-Item $hostLog, $guestLog -ErrorAction SilentlyContinue
$quit = $Frames
$dp = "t.MaxFPS=60,jam.ScriptHuman=1"
if ($Verbs) { $dp = "$dp,jam.net.ScriptVerbs=1" }
if (($HostPort -gt 0 -or $GuestPort -gt 0) -and -not $Lan) { $dp = "$dp,jam.net.Loopback=1" }
if ($Player) { $dp = "$dp,jam.net.ScriptPlayer=$Player" }
if ($Rematch) { $dp = "$dp,jam.net.ProofRematch=1" }
if ($Cvars) { $dp = "$dp,$Cvars" }
$hostDp = "$dp,jam.net.QuitAtFrame=$quit"
$guestDp = "$dp,jam.net.QuitAtFrame=$quit"
if ($Mode -eq 'hostleaves') { $hostDp = "$dp,jam.net.QuitAtFrame=$LeaveAt"; $guestDp = "$dp,jam.net.QuitAtFrame=$([math]::Max($quit, $LeaveAt * 10))" }
if ($Mode -eq 'guestleaves') { $guestDp = "$dp,jam.net.QuitAtFrame=$LeaveAt"; $hostDp = "$dp,jam.net.QuitAtFrame=$([math]::Max($quit, $LeaveAt * 10))" }
if ($AskThenHostLeaves) { $hostDp = "$dp,jam.net.QuitAtFrame=0"; $guestDp = "$dp,jam.net.QuitAtFrame=0,jam.net.ProofRematch=1,jam.net.ProofRematchAskFrames=60" }
if ($QuitBoth -or $GuestQuitOnResult) { $hostDp = "$dp,jam.net.QuitAtFrame=0"; $guestDp = "$dp,jam.net.QuitAtFrame=0" }   # lane A204: the cards stay up for the QUIT presses; the script stops its two pids after
if ($LiveCutAt -gt 0) { $guestDp = "$guestDp,jam.net.TestLiveCutAt=$LiveCutAt" }
if ($Mode -eq 'refuse') { $guestDp = "$guestDp,jam.net.TestVersion=$TestVersion" }
if ($HostPort -gt 0) { $hostDp = "$hostDp,jam.net.Port=$HostPort" }
if ($GuestPort -gt 0) { $guestDp = "$guestDp,jam.net.Port=$GuestPort" }
$joinPort = if ($HostPort -gt 0) { $HostPort } else { 7700 }
$common = "`"$Project`" -game -nullrhi -Multiprocess -unattended -nosplash -NoSound -log"
if ($packaged) { $common = "-nullrhi -unattended -nosplash -NoSound -log" }
$joinAddr = "127.0.0.1:$joinPort"
if ($Lan -and -not $LanIp) {
    # The default route's interface with a real (DHCP or manual, not link-local 169.254) address: on this machine Ethernet 2, 10.0.0.141.
    foreach ($route in (Get-NetRoute -DestinationPrefix '0.0.0.0/0' -ErrorAction SilentlyContinue | Sort-Object { $_.RouteMetric + $_.InterfaceMetric })) {
        $ip = Get-NetIPAddress -InterfaceIndex $route.ifIndex -AddressFamily IPv4 -ErrorAction SilentlyContinue | Where-Object { $_.IPAddress -notmatch '^(169.254|127).' } | Select-Object -First 1
        if ($ip) { $LanIp = $ip.IPAddress; break }
    }
    if (-not $LanIp) { Write-Host 'FAIL -Lan: no LAN address found (no default route)'; Write-Host '{"tool":"thirdjam.net/1","verdict":"could_not_run","exit":2}'; exit 2 }
}
if ($Lan) { Write-Host "INFO twoproc: LAN mode, host binds every address, the guest joins with the code of $LanIp" }
$hostArgs = "$common -abslog=`"$hostLog`" -ExecCmds=`"jam.host $Cut Ada auto`" -dpcvars=$hostDp"
$guestArgs = "$common -abslog=`"$guestLog`" -ExecCmds=`"jam.join $joinAddr Bo`" -dpcvars=$guestDp"
Write-Host "INFO twoproc: mode $Mode, cut $Cut, cap $Frames frames, to the end $([bool]$ToEnd), host cvars $hostDp"
Write-Host "INFO twoproc: guest cvars $guestDp"
Write-Host "INFO twoproc: logs $hostLog | $guestLog"
$t0 = Get-Date
if ($HostRcPort -gt 0) { $hostArgs += " -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$HostRcPort" }
$h = Start-Process -FilePath $Exe -ArgumentList $hostArgs -PassThru
Start-Sleep -Seconds $GuestDelaySec
$hostingLine = ''
if ($Lan) {
    # Lane A140: the operator reads the guest the code next to the guest's network address; the guest types the code.
    $wait = [Diagnostics.Stopwatch]::StartNew()
    while ($wait.Elapsed.TotalSeconds -lt 120 -and -not $hostingLine) {
        if (Test-Path $hostLog) { $r = Select-String -Path $hostLog -Pattern 'JAMNET hosting .*codes: (.*)$' | Select-Object -First 1; if ($r) { $hostingLine = $r.Matches[0].Groups[1].Value } }
        if (-not $hostingLine) { $h.Refresh(); if ($h.HasExited) { break }; Start-Sleep -Milliseconds 500 }
    }
    Write-Host "INFO twoproc: host's codes: $hostingLine"
    $code = ''
    foreach ($part in ($hostingLine -split '\|')) { if ($part -match '^\s*(\S+)\s+[^(]*\(([0-9.]+)\)') { if ($Matches[2] -eq $LanIp) { $code = $Matches[1] } } }
    if (-not $code) { Write-Host "FAIL -Lan: the host listed no code for $LanIp"; if (-not $h.HasExited) { Stop-Process -Id $h.Id -Force -Confirm:$false }; Write-Host '{"tool":"thirdjam.net/1","verdict":"fail","exit":1}'; exit 1 }
    $joinAddr = $code
    Write-Host "INFO twoproc: the guest types $code ($LanIp)"
    $guestArgs = "$common -abslog=`"$guestLog`" -ExecCmds=`"jam.join $joinAddr Bo`" -dpcvars=$guestDp"
}
if ($GuestRcPort -gt 0) { $guestArgs += " -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$GuestRcPort" }
function Rc([int]$port, [string]$cmd) {
    try {
        $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = 6000; $c.Connect('127.0.0.1', $port)
        $st = $c.GetStream(); $w = New-Object IO.StreamWriter($st); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($st)
        $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
        $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
        $ans = $null
        for ($i = 0; $i -lt 50; $i++) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith('{"id":2,')) { $ans = $l; break } }
        $c.Close(); return ($ans -and $ans.Contains('"ok":true'))
    } catch { return $false }
}
$pauseFirst = $null; $pausedAt = $null; $resumedAt = $null; $hPausedAt = $null; $hResumedAt = $null; $cardQuitAt = $null; $cutAtS = $null; $askLeaveAt = $null; $resultQuitAt = $null; $quitBothAt = $null; $loadQuitAt = $null
$g = Start-Process -FilePath $Exe -ArgumentList $guestArgs -PassThru
$late = $null
$lateLog = Join-Path $OutDir 'late.log'
Remove-Item $lateLog -ErrorAction SilentlyContinue
Write-Host ("INFO twoproc: host pid {0}, guest pid {1}, started {2}" -f $h.Id, $g.Id, $t0.ToString('HH:mm:ss'))
$sw = [Diagnostics.Stopwatch]::StartNew()
$killedAt = $null
while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    $h.Refresh(); $g.Refresh()
    if ($h.HasExited -and $g.HasExited) { break }
    if ($Mode -eq 'refuse' -and $g.HasExited) { break }
    if ($PauseAt -gt 0 -and $GuestRcPort -gt 0) {
        if (-not $pauseFirst -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) { $pauseFirst = $sw.Elapsed.TotalSeconds }
        if ($pauseFirst -and -not $pausedAt -and $sw.Elapsed.TotalSeconds -ge $pauseFirst + $PauseAt) { $ok = Rc $GuestRcPort 'jam.pausemenu open'; $pausedAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: guest pause card opened at {0:N1}s (ok={1})" -f $pausedAt, $ok) }
        if ($pausedAt -and -not $resumedAt -and $sw.Elapsed.TotalSeconds -ge $pausedAt + $PauseFor) { $ok = Rc $GuestRcPort 'jam.pausemenu close'; $resumedAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: guest pause card closed at {0:N1}s (ok={1})" -f $resumedAt, $ok) }
    }
    if ($HostPauseAt -gt 0 -and $HostRcPort -gt 0) {
        if (-not $pauseFirst -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) { $pauseFirst = $sw.Elapsed.TotalSeconds }
        if ($pauseFirst -and -not $hPausedAt -and $sw.Elapsed.TotalSeconds -ge $pauseFirst + $HostPauseAt) { $ok = Rc $HostRcPort 'jam.pausemenu open'; $hPausedAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: host pause card opened at {0:N1}s (ok={1})" -f $hPausedAt, $ok) }
        if ($hPausedAt -and -not $hResumedAt -and $sw.Elapsed.TotalSeconds -ge $hPausedAt + $PauseFor) { $ok = Rc $HostRcPort 'jam.pausemenu close'; $hResumedAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: host pause card closed at {0:N1}s (ok={1})" -f $hResumedAt, $ok) }
    }
    if ($GuestCardQuitAt -gt 0 -and $GuestRcPort -gt 0) {
        if (-not $pauseFirst -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) { $pauseFirst = $sw.Elapsed.TotalSeconds }
        if ($pauseFirst -and -not $cardQuitAt -and $sw.Elapsed.TotalSeconds -ge $pauseFirst + $GuestCardQuitAt) { $null = Rc $GuestRcPort 'jam.pausemenu open'; $ok = Rc $GuestRcPort 'jam.pausemenu quit'; $cardQuitAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: guest pressed QUIT on its pause card at {0:N1}s (ok={1})" -f $cardQuitAt, $ok) }
        if ($cardQuitAt -and (Test-Path $hostLog) -and (Select-String -Path $hostLog -Pattern 'JAMNET card:' -Quiet)) { Start-Sleep -Seconds 3; break }
    }
    if ($CutAt -gt 0 -and $GuestRcPort -gt 0) {
        if (-not $pauseFirst -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) { $pauseFirst = $sw.Elapsed.TotalSeconds }
        if ($pauseFirst -and -not $cutAtS -and $sw.Elapsed.TotalSeconds -ge $pauseFirst + $CutAt) { $ok = Rc $GuestRcPort "jam.net.testcut $CutMs"; $cutAtS = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: guest network cut for {0} ms at {1:N1}s (ok={2})" -f $CutMs, $cutAtS, $ok) }
        if ($QuitBoth -and $cutAtS -and -not $quitBothAt -and (Test-Path $hostLog) -and (Test-Path $guestLog) -and (Select-String -Path $hostLog -Pattern 'JAMNET card:' -Quiet) -and (Select-String -Path $guestLog -Pattern 'JAMNET card:' -Quiet)) {
            Start-Sleep -Seconds 2
            $a1 = Rc $GuestRcPort 'jam.pausemenu open'; $a2 = Rc $GuestRcPort 'jam.pausemenu quit'
            $b1 = $false; $b2 = $false; if ($HostRcPort -gt 0) { $b1 = Rc $HostRcPort 'jam.pausemenu open'; $b2 = Rc $HostRcPort 'jam.pausemenu quit' }
            $quitBothAt = $sw.Elapsed.TotalSeconds; Write-Host ("INFO twoproc5: QUIT pressed on both pause cards at {0:N1}s (guest {1}/{2}, host {3}/{4})" -f $quitBothAt, $a1, $a2, $b1, $b2)
        }
        if ($quitBothAt -and $sw.Elapsed.TotalSeconds -ge $quitBothAt + 12) { break }
    }
    if ($GuestQuitInLoad -and $GuestRcPort -gt 0 -and -not $loadQuitAt -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'opens the rink .* late' -Quiet)) {
        Start-Sleep -Seconds 2
        $ok = Rc $GuestRcPort 'quit'; $loadQuitAt = Get-Date; Write-Host ("INFO twoproc5: the guest's game quits while its rink waits to load ({0}, ok={1})" -f $loadQuitAt.ToString('HH:mm:ss.fff'), $ok)
    }
    if ($loadQuitAt -and (Test-Path $hostLog) -and (Select-String -Path $hostLog -Pattern 'JAMNET card:' -Quiet)) { Start-Sleep -Seconds 2; break }
    if ($GuestQuitOnResult -and $GuestRcPort -gt 0 -and -not $resultQuitAt -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET match over' -Quiet)) {
        Start-Sleep -Seconds 3; $a1 = Rc $GuestRcPort 'jam.pausemenu open'; $a2 = Rc $GuestRcPort 'jam.pausemenu quit'; $resultQuitAt = $sw.Elapsed.TotalSeconds
        Write-Host ("INFO twoproc5: the guest pressed QUIT on its pause card on the result card at {0:N1}s ({1}/{2})" -f $resultQuitAt, $a1, $a2)
    }
    if ($resultQuitAt -and $sw.Elapsed.TotalSeconds -ge $resultQuitAt + 8) { break }
    if ($AskThenHostLeaves -and $HostRcPort -gt 0 -and -not $askLeaveAt -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET rematch asked by seat' -Quiet)) {
        Start-Sleep -Seconds 3; $a1 = Rc $HostRcPort 'jam.pausemenu open'; $a2 = Rc $HostRcPort 'jam.pausemenu quit'; $askLeaveAt = $sw.Elapsed.TotalSeconds
        Write-Host ("INFO twoproc5: the guest asked for the rematch; the host pressed QUIT at {0:N1}s ({1}/{2})" -f $askLeaveAt, $a1, $a2)
    }
    if ($askLeaveAt -and $sw.Elapsed.TotalSeconds -ge $askLeaveAt + 8) { break }
    if ($Mode -eq 'guestleaves' -and $g.HasExited -and (Test-Path $hostLog) -and (Select-String -Path $hostLog -Pattern 'JAMNET card:' -Quiet)) { Start-Sleep -Seconds 3; break }
    if ($Mode -eq 'latejoin' -and -not $late -and $sw.Elapsed.TotalSeconds -ge $LateDelaySec) {
        $lateDp = "$dp,jam.net.QuitAtFrame=$quit"; if ($LatePort -gt 0) { $lateDp = "$lateDp,jam.net.Port=$LatePort" }
        $late = Start-Process -FilePath $Exe -ArgumentList "$common -abslog=`"$lateLog`" -ExecCmds=`"jam.join $joinAddr Cy`" -dpcvars=$lateDp" -PassThru
        Write-Host ("INFO twoproc5: late guest pid {0} joined at {1}s" -f $late.Id, [int]$sw.Elapsed.TotalSeconds)
    }   # the host waits for a friend forever: stopped below
    if ($Mode -eq 'hostkilled' -and -not $killedAt -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) {
        if ($sw.Elapsed.TotalSeconds -ge $GuestDelaySec + [math]::Ceiling($LeaveAt / 60)) { $killedAt = Get-Date; Write-Host ("INFO twoproc: stopping my host pid {0} (no BYE) at {1}" -f $h.Id, $killedAt.ToString('HH:mm:ss.fff')); Stop-Process -Id $h.Id -Force -Confirm:$false }
    }
    Start-Sleep -Milliseconds 500
}
$h.Refresh(); $g.Refresh()
$timedOut = -not ($h.HasExited -and $g.HasExited) -and -not ($Mode -eq 'refuse' -and $g.HasExited)
if ($late) { $late.Refresh(); if (-not $late.HasExited) { Write-Host ("INFO twoproc5: stopping my late guest pid {0}" -f $late.Id); Stop-Process -Id $late.Id -Force -Confirm:$false } }
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
# UE log stamp [2026.11.13-19.24.48:079] -> DateTime (UTC as logged; both logs share the clock)
function Stamp([string]$log, [string]$pattern) {
    if (-not (Test-Path $log)) { return $null }
    $r = Select-String -Path $log -Pattern $pattern | Select-Object -First 1
    if (-not $r) { return $null }
    if ($r.Line -match '^\[(\d{4})\.(\d{2})\.(\d{2})-(\d{2})\.(\d{2})\.(\d{2}):(\d{3})\]') { return New-Object DateTime ([int]$Matches[1]), ([int]$Matches[2]), ([int]$Matches[3]), ([int]$Matches[4]), ([int]$Matches[5]), ([int]$Matches[6]), ([int]$Matches[7]) }
    return $null
}
$fail = $false
$extra = [ordered]@{}
if ($Mode -eq 'refuse') {
    $failed = Last-Match $guestLog 'JAMNET lobby failed'
    $screen = Last-Match $guestLog 'JAMNET screen:'
    $hostRefused = Last-Match $hostLog 'JAMNET refused a protocol'
    $secs = -1.0
    if ($failed -match '\(([0-9.]+) s after the join\)') { $secs = [double]$Matches[1] }
    if (-not $hostRefused) { Write-Host 'FAIL the host never logged a refusal'; $fail = $true }
    if (-not $failed -or $failed -notmatch 'different build') { Write-Host "FAIL the guest did not fail with the build reason: '$failed'"; $fail = $true }
    elseif ($secs -lt 0 -or $secs -ge 1.0) { Write-Host "FAIL refused after $secs s (want < 1 s)"; $fail = $true }
    if (-not $fail) { Write-Host ("PASS a protocol-{0} guest refused in {1:N3} s; its screen: {2}" -f $TestVersion, $secs, ($screen -replace '^JAMNET screen: ', '')) }
    Write-Host "INFO host: $hostRefused"
    $extra.refuse_s = $secs; $extra.screen = $screen; $extra.host_refused = $hostRefused
} elseif ($Mode -eq 'guestleaves') {
    $card = Last-Match $hostLog 'JAMNET card:'
    $quitLine = Last-Match $guestLog 'JAMNET quitting'
    if (-not $card -or $card -notmatch 'left') { Write-Host "FAIL the host showed no 'left' card: '$card'"; $fail = $true }
    if ($timedOut) { Write-Host 'FAIL the host froze (timeout)'; $fail = $true }
    if (-not $fail) { Write-Host "PASS guestleaves at frame ${LeaveAt}: host card '$card'; guest '$quitLine'" }
    $extra.card = $card; $extra.guest_quit = $quitLine
} elseif ($Mode -eq 'hostleaves' -or $Mode -eq 'hostkilled') {
    $card = Last-Match $guestLog 'JAMNET card:'
    $ended = Last-Match $guestLog 'JAMNET match ended'
    $secs = -1.0
    if ($card -match ', ([0-9.]+) s after the last frame') { $secs = [double]$Matches[1] }
    $hostQuitAt = Stamp $hostLog 'JAMNET quitting'
    $cardAt = Stamp $guestLog 'JAMNET card:'
    $fromLeave = -1.0
    if ($Mode -eq 'hostleaves' -and $hostQuitAt -and $cardAt) { $fromLeave = ($cardAt - $hostQuitAt).TotalSeconds }
    if ($Mode -eq 'hostkilled' -and $killedAt -and $cardAt) { $fromLeave = ($cardAt - $killedAt.ToUniversalTime()).TotalSeconds }
    $want = if ($Mode -eq 'hostleaves') { 'left' } else { 'connection lost' }
    if (-not $card -or $card -notmatch $want) { Write-Host "FAIL the guest showed no '$want' card: '$card'"; $fail = $true }
    if ($timedOut) { Write-Host 'FAIL the guest froze (timeout)'; $fail = $true }
    if (-not $fail) { Write-Host ("PASS {0}: the guest's card '{1}' {2:N2} s after its last frame, {3:N2} s after the host went" -f $Mode, ($ended -replace '^JAMNET match ended: ', ''), $secs, $fromLeave) }
    $extra.card = $card; $extra.card_after_last_frame_s = $secs; $extra.card_after_host_went_s = [math]::Round($fromLeave, 3)
} else {
    $hf = Read-Frames $hostLog
    $gf = Read-Frames $guestLog
    $agree = 0; $differ = @()
    foreach ($f in ($hf.Keys | Sort-Object)) { if ($gf.ContainsKey($f)) { if ($hf[$f] -eq $gf[$f]) { $agree++ } else { $differ += "f$f host=$($hf[$f]) guest=$($gf[$f])" } } }
    $hostDone = Last-Match $hostLog 'JAMNET proof done'
    $guestDone = Last-Match $guestLog 'JAMNET proof done'
    $desync = (Last-Match $hostLog 'JAMNET desync') + (Last-Match $guestLog 'JAMNET desync')
    # A BYE after both logged "proof done" is the other process quitting at the end (lane A120's in-match BYE): not a lost match.
    $lost = (Last-Match $hostLog 'JAMNET lost') + (Last-Match $guestLog 'JAMNET lost')
    if ((Last-Match $hostLog 'JAMNET proof done') -and (Last-Match $guestLog 'JAMNET proof done')) { $lost = ((@((Last-Match $hostLog 'JAMNET lost'), (Last-Match $guestLog 'JAMNET lost')) | Where-Object { $_ -and $_ -notmatch 'peer said BYE' }) -join ' ') }
    $over = Last-Match $hostLog 'JAMNET match over'
    $hFinal = ''; $gFinal = ''; $hFrame = -1; $gFrame = -1
    if ($hostDone -match 'frame=(\d+) hash=([0-9a-f]+)') { $hFrame = [int]$Matches[1]; $hFinal = $Matches[2] }
    if ($guestDone -match 'frame=(\d+) hash=([0-9a-f]+)') { $gFrame = [int]$Matches[1]; $gFinal = $Matches[2] }
    $guestSeat = ''; $guestAb = -1
    if ($guestDone -match 'guest=\[([^\]]*)\]') { $guestSeat = $Matches[1]; if ($guestSeat -match ' ability (\d+) \|') { $guestAb = [int]$Matches[1] } }   # lane A140: F applied over both matches (abilities_used restarts with the rematch)
    if ($timedOut) { Write-Host "FAIL timeout after ${TimeoutSec}s"; $fail = $true }
    if ($desync) { Write-Host "FAIL desync: $desync"; $fail = $true }
    if ($lost) { Write-Host "FAIL connection lost: $lost"; $fail = $true }
    if ($differ.Count) { Write-Host ("FAIL hashes differ at {0} checkpoint lines: {1}" -f $differ.Count, ($differ -join '; ')); $fail = $true }
    if (-not $hFinal -or $hFinal -ne $gFinal -or $hFrame -ne $gFrame) { Write-Host "FAIL final lines differ or missing: host f$hFrame $hFinal, guest f$gFrame $gFinal"; $fail = $true }
    if ($ToEnd -and -not $over) { Write-Host "FAIL the match did not reach its end within $Frames frames"; $fail = $true }
    # The guest's F counts only where the cut gives its seat an ability (shotgun, arsenal pickups, ...): hockey.regular holds none.
    if ($Verbs -and $guestAb -le 0 -and $Cut -match 'shotgun') { Write-Host "FAIL guest abilities used $guestAb (want > 0)"; $fail = $true }
    if ($ToEnd -and $over) {
        $hCard = Last-Match $hostLog 'JAMNET card: (WIN|LOSE)'; $gCard = Last-Match $guestLog 'JAMNET card: (WIN|LOSE)'
        if (-not $hCard -or -not $gCard) { Write-Host "FAIL a machine showed no win / lose card: host '$hCard', guest '$gCard'"; $fail = $true }
        elseif (($hCard -match 'WIN') -eq ($gCard -match 'WIN')) { Write-Host "FAIL both cards say the same for opposite teams: host '$hCard', guest '$gCard'"; $fail = $true }
        else { Write-Host "PASS cards: host '$($hCard -replace '^JAMNET card: ', '')' | guest '$($gCard -replace '^JAMNET card: ', '')'" }
        $extra.card_host = $hCard; $extra.card_guest = $gCard
    }
    if ($Rematch) {
        $hR = Last-Match $hostLog 'JAMNET rematch started'; $gR = Last-Match $guestLog 'JAMNET rematch started'
        $hRk = if ($hR -match 'lockstep frame (\d+) sim_frame=\d+ hash=([0-9a-f]+)') { "$($Matches[1])/$($Matches[2])" } else { '' }
        $gRk = if ($gR -match 'lockstep frame (\d+) sim_frame=\d+ hash=([0-9a-f]+)') { "$($Matches[1])/$($Matches[2])" } else { '' }
        if (-not $hRk -or $hRk -ne $gRk) { Write-Host "FAIL rematch not started on both at the same frame and hash: host '$hR', guest '$gR'"; $fail = $true }
        else { Write-Host "PASS rematch started on both at lockstep frame/hash $hRk" }
        $extra.rematch_host = $hR; $extra.rematch_guest = $gR
    }
    if ($LiveCutAt -gt 0) {
        $lc = Last-Match $guestLog 'JAMNET test live cut'; $ls = Last-Match $guestLog 'JAMNET screen: a live cut'
        if ($lc -notmatch 'refused' -or -not $ls) { Write-Host "FAIL the guest's live cut was not refused on screen: '$lc' '$ls'"; $fail = $true }
        else { Write-Host "PASS live cut refused on the guest: $($ls -replace '^JAMNET screen: ', '')" }
        $extra.live_cut = $lc
    }
    if (-not $fail) { Write-Host ("PASS final hash {0} at f{1} equal on host and guest, {2} checkpoint lines equal, {3}, {4}s wall" -f $hFinal, $hFrame, $agree, $(if ($over) { $over } else { 'stopped at the cap' }), $wall) }
    Write-Host "INFO host:  $hostDone"
    Write-Host "INFO guest: $guestDone"
    $extra.final_frame = $hFrame; $extra.final_hash_host = $hFinal; $extra.final_hash_guest = $gFinal; $extra.agree = $agree; $extra.differ = $differ.Count
    $extra.goals = if ($hostDone -match 'score=(\d+):(\d+)') { "$($Matches[1]):$($Matches[2])" } else { '' }
    $extra.guest_seat = $guestSeat; $extra.guest_abilities_used = $guestAb; $extra.match_over = $over
}
if ($CutAt -gt 0 -and $QuitBoth) {
    foreach ($side in @(@('host', $hostLog), @('guest', $guestLog))) {
        $cardLine = Last-Match $side[1] 'JAMNET card:'; $lostLine = Last-Match $side[1] 'JAMNET lost frame'
        $cutStamp = Stamp $guestLog 'JAMNET test cut'; $cardStamp = Stamp $side[1] 'JAMNET card:'
        $after = -1.0; if ($cutStamp -and $cardStamp) { $after = ($cardStamp - $cutStamp).TotalSeconds }
        $menu = @(Select-String -Path $side[1] -Pattern 'JAMNET leaving the rink' -ErrorAction SilentlyContinue).Count
        $menuWorld = @(Select-String -Path $side[1] -Pattern 'LoadMap: /Game/Maps/Menu' -SimpleMatch -ErrorAction SilentlyContinue).Count - 1   # the first is the start
        if (-not $cardLine -or $cardLine -notmatch 'connection lost' -or $after -lt 0 -or $after -gt 3.5) { Write-Host ("FAIL {0}: card '{1}' {2:N2} s after the cut" -f $side[0], $cardLine, $after); $fail = $true }
        else { Write-Host ("PASS {0}: '{1}' {2:N2} s after the cut" -f $side[0], $cardLine, $after) }
        if ($menu -lt 1 -or $menuWorld -lt 1) { Write-Host "FAIL $($side[0]): QUIT did not reach the menu (leaving=$menu, menu lines=$menuWorld)"; $fail = $true } else { Write-Host "PASS $($side[0]): QUIT on the pause card left the rink and loaded the menu" }
        $extra["cut_card_$($side[0])"] = $cardLine; $extra["cut_card_after_s_$($side[0])"] = [math]::Round($after, 2)
    }
}
if ($GuestQuitOnResult) {
    $over = Last-Match $hostLog 'JAMNET card: (WIN|LOSE)'; $left = Last-Match $hostLog 'JAMNET card: .* left'; $gl = Last-Match $guestLog 'JAMNET leaving the rink'
    if (-not $over -or -not $left -or -not $gl) { Write-Host "FAIL QUIT on the result card: host result '$over', host left card '$left', guest '$gl'"; $fail = $true } else { Write-Host "PASS QUIT on the result card: host '$over' then '$left'; guest '$gl'" }
}
if ($AskThenHostLeaves) {
    $ask = Last-Match $guestLog 'JAMNET screen: rematch asked'; $left = Last-Match $guestLog 'JAMNET card: .* left'; $still = Last-Match $guestLog 'JAMNET end card still'; $hl = Last-Match $hostLog 'JAMNET leaving the rink'
    if (-not $ask -or -not $left -or -not $hl) { Write-Host "FAIL ask then leave: guest ask '$ask', guest card '$left', host '$hl'"; $fail = $true } else { Write-Host "PASS ask then leave: guest '$ask' then '$left'; host '$hl'; $still" }
}
if ($GuestQuitInLoad) {
    $cardLine = Last-Match $hostLog 'JAMNET card:'; $cardStamp = Stamp $hostLog 'JAMNET card:'; $byeLine = Last-Match $guestLog 'BYE sent'; $byeStamp = Stamp $guestLog 'BYE sent'
    $after = -1.0; if ($cardStamp -and $byeStamp) { $after = ($cardStamp - $byeStamp).TotalSeconds }
    $fromQuit = -1.0; if ($cardStamp -and $loadQuitAt) { $fromQuit = ($cardStamp - $loadQuitAt.ToUniversalTime()).TotalSeconds }
    if (-not $cardLine -or $cardLine -notmatch 'left' -or $after -lt 0 -or $after -gt 1.0) { Write-Host ("FAIL guest quit in its load: host card '{0}' {1:N2} s after the guest's BYE ({2:N2} s after the quit); guest '{3}'" -f $cardLine, $after, $fromQuit, $byeLine); $fail = $true }
    else { Write-Host ("PASS guest quit in its load: host '{0}' {1:N2} s after the guest's BYE, {2:N2} s after the quit command (the guest's own shutdown in between); guest '{3}'" -f $cardLine, $after, $fromQuit, $byeLine) }
    $extra.load_quit_from_quit_s = [math]::Round($fromQuit, 2)
    $extra.load_quit_card = $cardLine; $extra.load_quit_after_s = [math]::Round($after, 2)
}
if ($GuestCardQuitAt -gt 0) {
    $card = Last-Match $hostLog 'JAMNET card:'; $ended = Last-Match $hostLog 'JAMNET match ended'; $gLeft = Last-Match $guestLog 'JAMNET leaving the rink'
    if (-not $card -or $card -notmatch 'left') { Write-Host "FAIL QUIT on the guest's card: the host showed '$card'"; $fail = $true } else { Write-Host "PASS QUIT on the guest's card: host '$card' / '$ended'; guest '$gLeft'" }
    $extra.card_quit_host = $card
}
if ($Mode -eq 'play') {
    # Lane A204: each paused machine refused the world pause and sent neutral input under its card.
    foreach ($pz in @(@('guest', $guestLog, ($PauseAt -gt 0 -and $GuestRcPort -gt 0)), @('host', $hostLog, ($HostPauseAt -gt 0 -and $HostRcPort -gt 0)))) {
        if (-not $pz[2]) { continue }
        $ref = Last-Match $pz[1] 'JAMNET pause refused'; $open = Last-Match $pz[1] 'JAMNET pause card open'; $closed = Last-Match $pz[1] 'JAMNET pause card closed'
        if (-not $ref -or -not $open -or -not $closed) { Write-Host "FAIL $($pz[0]) pause: refused='$ref' open='$open' closed='$closed'"; $fail = $true }
        else { Write-Host "PASS $($pz[0]) pause card: $open | $closed" }
        $extra["pause_$($pz[0])"] = "$open | $closed"
    }
    # Lane A140: each machine's own presses (read from its controls / applied to its seat / in flight at the end / folded in a stall).
    foreach ($side in @(@('host', $hostLog), @('guest', $guestLog))) {
        $done = Last-Match $side[1] 'JAMNET proof done'
        if ($done -match '(presses=\[[^\]]*\]) \(read/applied/in_flight/folded\)(?: held_repeats=\d+)? presses_lost=(-?\d+)') {
            $pr = $Matches[1]; $lostN = [int]$Matches[2]; $folded = 0
            foreach ($m in [regex]::Matches($pr, '(\d+)/(\d+)/(\d+)/(\d+)')) { $folded += [int]$m.Groups[4].Value }
            if ($lostN -ne 0) { Write-Host "FAIL $($side[0]): $lostN presses lost ($folded folded by a full stall queue): $pr"; $fail = $true }
            else { Write-Host "PASS $($side[0]) presses (read/applied/in flight/folded): $pr lost=$lostN folded=$folded" }
            $extra["presses_$($side[0])"] = $pr; $extra["presses_lost_$($side[0])"] = $lostN; $extra["presses_folded_$($side[0])"] = $folded
        } else { Write-Host "INFO $($side[0]): no press accounting on the proof line (a build before lane A140)" }
    }
}
foreach ($side in @(@('host', $hostLog), @('guest', $guestLog))) {
    if (-not (Test-Path $side[1])) { continue }
    $st = @(Select-String -Path $side[1] -Pattern 'JAMNET lobby state=\d+ .*status="([^"]*)"' | ForEach-Object { $_.Matches[0].Groups[1].Value })
    if ($st.Count) { Write-Host ("INFO {0} status lines: {1}" -f $side[0], ($st -join ' -> ')); $extra["status_$($side[0])"] = ($st -join ' -> ') }
}
if ($Lan) { $extra.lan_ip = $LanIp; $extra.join_code = $joinAddr; $extra.host_codes = $hostingLine }
if ($Mode -eq 'latejoin') {
    $lf = Last-Match $lateLog 'JAMNET lobby failed'; $ls = Last-Match $lateLog 'JAMNET screen:'; $lstate = Last-Match $lateLog 'JAMNET lobby state='
    $hostSawLate = @(Select-String -Path $hostLog -Pattern 'JAMNET guest "Cy"' -ErrorAction SilentlyContinue).Count
    Write-Host "INFO late guest: failed='$lf' screen='$ls' last state='$lstate' host_saw_hello=$hostSawLate"
    if (Select-String -Path $lateLog -Pattern 'JAMNET first frame' -Quiet -ErrorAction SilentlyContinue) { Write-Host 'FAIL the late guest started a match'; $fail = $true }
    $extra.late_failed = $lf; $extra.late_screen = $ls; $extra.late_state = $lstate; $extra.host_saw_late_hello = $hostSawLate
}
$code = 0; if ($fail) { $code = 1 }
$obj = [ordered]@{ tool = 'thirdjam.net/1'; verdict = @('pass', 'fail')[$code]; exit = $code; mode = $Mode; cut = $Cut; frames = $Frames; wall_s = $wall; host_log = $hostLog; guest_log = $guestLog }
foreach ($k in $extra.Keys) { $obj[$k] = $extra[$k] }
Write-Host ($obj | ConvertTo-Json -Compress)
exit $code
