# Lane A179 (S3 evolution 5, item 1): the shipping-play row the release chain runs itself on the FRESH ZIP (the file that is handed over, not a staged folder).
# The zip is unpacked into tools\qa\out\release\<Name>\unz (removed again unless -Keep), then:
#   1. ThirdJam.exe starts and a window appears (a 1920x1080 client, windowed),
#   2. the first screen draws (a still of the menu is not black: lit >= 0.20),
#   3. Return on the landing, Return on HOME (PLAY NOW) -> a match renders: the still 9 s after the Return is lit (lit share >= 0.90; the H27 black world read 0.01),
#   4. the game stays alive for -PlaySeconds (default 30) of skating with W and the mouse, a still every 5 s, each lit >= 0.90,
#   5. the window's close (WM_CLOSE) ends the process cleanly (exit code 0, no crash report folder newer than the start).
# A Shipping package has no remote console and no JAM logs: everything is read from the window (stills, the process), as the window lane did.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-ship.ps1 -Zip Game\Packaged\Hokej_Hokej_H30\Hokej_Hokej_H30.zip [-Name h30-ship] [-PlaySeconds 30] [-Keep]
# Exit: 0 pass; 1 fail (black or unlit frame, the game died, the close left it running or with a crash code); 3 machine (a foreign window or dialog held the foreground, another
# ThirdJam shared the cursor, no window within the wait: repeat once); 2 could not run. The last stdout line is JSON `thirdjam.releaseship/1`.
param([string]$Zip = '', [string]$Name = 'release-ship', [int]$PlaySeconds = 30, [double]$LitMin = 0.90, [switch]$Keep)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
if (-not $Zip -or -not (Test-Path $Zip)) { Write-Host "FAIL give -Zip <the release zip> (got '$Zip')"; exit 2 }
$out = Join-Path $root "tools\qa\out\release\$Name"
$unz = Join-Path $out 'unz'
if (Test-Path $unz) { Remove-Item $unz -Recurse -Force }
New-Item -ItemType Directory -Force $unz | Out-Null
$fails = New-Object System.Collections.ArrayList; $machine = New-Object System.Collections.ArrayList; $rows = [ordered]@{}
function Row([string]$k, [string]$v) { $script:rows[$k] = $v; HW-Note "ROW $k = $v" }
function Fail([string]$why) { [void]$script:fails.Add($why); HW-Note "FAIL $why" }
function Mach([string]$why) { [void]$script:machine.Add($why); HW-Note "MACHINE $why" }
$t0 = Get-Date
Expand-Archive -Path $Zip -DestinationPath $unz -Force
$exe = Get-ChildItem $unz -Recurse -Filter 'ThirdJam.exe' | Where-Object { $_.DirectoryName -notmatch 'Binaries' } | Select-Object -First 1
if (-not $exe) { Write-Host "FAIL no ThirdJam.exe in the zip"; Write-Host (@{ tool = 'thirdjam.releaseship/1'; verdict = 'fail'; failed = @('no ThirdJam.exe in the zip'); machine = @(); out = $out } | ConvertTo-Json -Compress); exit 1 }
Row 'unzip' ("{0:0} s, {1}" -f ((Get-Date) - $t0).TotalSeconds, $exe.FullName)
$started = Get-Date
$ok = HW-Start $exe.FullName '' $out -NoLog -WaitSec 120
if (-not $ok) { Mach 'no window within 120 s' }
$fgBad = 0
function Fg { $t = [WH]::Fg(); if ($t -match "Windows Security|Firewall|Windows Defender") { $script:fgBad++; if ($script:fgBad -eq 1) { HW-Note "WARN a system dialog holds the foreground: $t (a first start from a new path asks the firewall; posted input to the game window is not affected; not touched)" } } }
$px = $null
try {
    if ($ok) {
        Start-Sleep -Seconds 15; Fg
        $px = HW-Pixels (HW-Shot 'menu'); Row 'menu' ($px | ConvertTo-Json -Compress)
        if ($px.lit -lt 0.2) { Fail "the first screen is dark: lit=$($px.lit)" }
        # Director, 14 Nov 17:50: since lane A187's front end the first screen is the landing: its Return goes to HOME (PLAY NOW focused), the second plays.
        # H33's first ship row pressed once and photographed HOME six times (lit 0.69: the barn backdrop). Two Returns, three seconds apart; an old
        # package (before 13e36667) needs -OldMenu on release-menu.ps1 and is not run through this row any more.
        HW-Key 'Return'; Start-Sleep -Seconds 3; Fg
        $px = HW-Pixels (HW-Shot 'home'); Row 'home' ($px | ConvertTo-Json -Compress)
        HW-Key 'Return'; Start-Sleep -Seconds 9; Fg
        $px = HW-Pixels (HW-Shot 'match-9s'); Row 'match_9s' ($px | ConvertTo-Json -Compress)
        if (-not (HW-Alive)) { Fail 'the game is gone after PLAY NOW' }
        elseif ($px.lit -lt $LitMin) { Fail "the match is not lit: lit=$($px.lit) mean=$($px.mean) (want >= $LitMin)" }
        HW-Move 1300 400; HW-KeyDown 'W'
        $t1 = Get-Date; $n = 0; $minLit = 1.0
        while ((HW-Alive) -and ((Get-Date) - $t1).TotalSeconds -lt $PlaySeconds) {
            Start-Sleep -Seconds 5; $n++
            HW-Move (900 + 100 * ($n % 5)) (300 + 60 * ($n % 3))
            $p2 = HW-Pixels (HW-Shot "play-$n"); if ($p2.lit -lt $minLit) { $minLit = $p2.lit }
            Fg
        }
        HW-KeyUp 'W'
        Row 'play' "$n stills, lowest lit $minLit, alive $(HW-Alive)"
        if (-not (HW-Alive)) { Fail "the game died while playing (after $n stills)" }
        elseif ($minLit -lt $LitMin) { Fail "a frame while playing was not lit: lowest lit $minLit (want >= $LitMin)" }
    }
}
finally {
    if ($ok) {
        $res = HW-Close -WaitSec 40
        Row 'close' $res
        if ($res -like 'STOPPED-BY-US*') { Fail 'the window close left the game running (stopped by the driver)' }
        elseif ($res -match 'code (-?\d+)' -and [int]$Matches[1] -ne 0) { Fail "the game exited with code $($Matches[1]) after the window close" }
        $crash = @(Get-ChildItem (Join-Path $exe.DirectoryName 'ThirdJam\Saved\Crashes') -Directory -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $started })
        if ($crash.Count) { Fail "a crash report folder was written: $($crash[0].FullName)" }
    }
    ($rows.GetEnumerator() | ForEach-Object { "$($_.Key) = $($_.Value)" }) | Set-Content (Join-Path $out 'rows.txt')
    if (-not $Keep) { Start-Sleep -Seconds 2; Remove-Item $unz -Recurse -Force -ErrorAction SilentlyContinue }
}
$verdict = if ($fails.Count) { 'fail' } elseif ($machine.Count) { 'machine' } else { 'pass' }
$code = @{ pass = 0; fail = 1; machine = 3 }[$verdict]
Write-Host (@{ tool = 'thirdjam.releaseship/1'; verdict = $verdict; zip = $Zip; foreground_dialog_checks = $fgBad; failed = @($fails); machine = @($machine); out = $out } | ConvertTo-Json -Compress)
exit $code
