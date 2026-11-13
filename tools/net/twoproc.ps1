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
    [ValidateSet('play', 'refuse', 'hostleaves', 'hostkilled')][string]$Mode = 'play',
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
if (-not $Project) { $Project = Join-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) 'Game\ThirdJam.uproject' }
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
    foreach ($part in ($hostingLine -split '\|')) { if ($part -match '^\s*(\S+)\s+\(([0-9.]+)\)') { if ($Matches[2] -eq $LanIp) { $code = $Matches[1] } } }
    if (-not $code) { Write-Host "FAIL -Lan: the host listed no code for $LanIp"; if (-not $h.HasExited) { Stop-Process -Id $h.Id -Force -Confirm:$false }; Write-Host '{"tool":"thirdjam.net/1","verdict":"fail","exit":1}'; exit 1 }
    $joinAddr = $code
    Write-Host "INFO twoproc: the guest types $code ($LanIp)"
    $guestArgs = "$common -abslog=`"$guestLog`" -ExecCmds=`"jam.join $joinAddr Bo`" -dpcvars=$guestDp"
}
$g = Start-Process -FilePath $Exe -ArgumentList $guestArgs -PassThru
Write-Host ("INFO twoproc: host pid {0}, guest pid {1}, started {2}" -f $h.Id, $g.Id, $t0.ToString('HH:mm:ss'))
$sw = [Diagnostics.Stopwatch]::StartNew()
$killedAt = $null
while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec) {
    $h.Refresh(); $g.Refresh()
    if ($h.HasExited -and $g.HasExited) { break }
    if ($Mode -eq 'refuse' -and $g.HasExited) { break }   # the host waits for a friend forever: stopped below
    if ($Mode -eq 'hostkilled' -and -not $killedAt -and (Test-Path $guestLog) -and (Select-String -Path $guestLog -Pattern 'JAMNET first frame' -Quiet)) {
        if ($sw.Elapsed.TotalSeconds -ge $GuestDelaySec + [math]::Ceiling($LeaveAt / 60)) { $killedAt = Get-Date; Write-Host ("INFO twoproc: stopping my host pid {0} (no BYE) at {1}" -f $h.Id, $killedAt.ToString('HH:mm:ss.fff')); Stop-Process -Id $h.Id -Force -Confirm:$false }
    }
    Start-Sleep -Milliseconds 500
}
$h.Refresh(); $g.Refresh()
$timedOut = -not ($h.HasExited -and $g.HasExited) -and -not ($Mode -eq 'refuse' -and $g.HasExited)
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
if ($Mode -eq 'play') {
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
$code = 0; if ($fail) { $code = 1 }
$obj = [ordered]@{ tool = 'thirdjam.net/1'; verdict = @('pass', 'fail')[$code]; exit = $code; mode = $Mode; cut = $Cut; frames = $Frames; wall_s = $wall; host_log = $hostLog; guest_log = $guestLog }
foreach ($k in $extra.Keys) { $obj[$k] = $extra[$k] }
Write-Host ($obj | ConvertTo-Json -Compress)
exit $code
