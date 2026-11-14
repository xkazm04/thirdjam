# Lane A155 (H28 row 3): STICK LOCKED by hand. The acceptance drill's step 7 stages the human at 4 m/s toward the carrying opponent (inside 3 m the lock is by rule); this hand presses Space (the poke)
# every 250 ms from the stage line for 7 s and reads the HUD's contact line. Up to -Launches launches (each stages step 7 twice: check and cross-check).
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H28', [string]$Name = 'a155-lock', [int]$Launches = 3, [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
$cut = (Join-Path $PSScriptRoot 'accept-cut.json') -replace '\', '/'
$presses = 0; $locked = 0
for ($L = 1; $L -le $Launches -and $locked -eq 0; $L++) {
    $out = Join-Path $repo "tools\qa\out\$Name\l$L"
    [void](HW-Start $exe "-cut=`"$cut`" -dpcvars=jam.ctrl.Drill=20,jam.accept.From=7,jam.accept.To=7" $out -Port $Port -Console -WaitSec 120)
    [void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
    $m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
    $from = 0
    foreach ($k in 1, 2) {
        $s = HW-Wait 'JAMACCEPT stage step=7' 60 $from; if (-not $s) { break }
        $from = HW-Mark
        $t0 = Get-Date
        while (((Get-Date) - $t0).TotalSeconds -lt 7) { HW-Key 'Space' 40; $presses++; Start-Sleep -Milliseconds 210 }
        $hud = HW-Lines 'contact_line=' $m; if ($hud | ? { $_ -match 'STICK_LOCKED|STICK LOCKED' }) { $locked++ }
    }
    HW-Note ("launch $L presses so far $presses; contact lines: " + ((HW-Lines 'contact_line=|JAMVIS press|JAMVIS stickhit' $m | Select-Object -First 14) -join ' || '))
    [void](HW-Close -ByConsole); Start-Sleep -Seconds 3
}
HW-Note "RESULT presses=$presses stick_locked_lines=$locked"
