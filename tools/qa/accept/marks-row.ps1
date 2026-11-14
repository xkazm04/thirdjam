# Lane A155 (H28 row 2): the marks in a long real match. PLAY NOW by Return, the human skates and aims for -Seconds; a still every ~0.8 s; the log's JAMHUD mark lines name when each mark was up.
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H28', [string]$Name = 'a155-marks', [int]$Seconds = 150, [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"; $exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
[void](HW-Start $exe '' $out -Port $Port -Console -WaitSec 120)
[void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
$m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
[void](HW-Wait 'JAMMATCH level=' 30 $m); Start-Sleep -Seconds 3
$t0 = Get-Date; $k = 0; $n = 0
while (((Get-Date) - $t0).TotalSeconds -lt $Seconds) {
    $k++
    # skate toward the puck side by turns: W with the aim swinging, a tap of S now and then (the brake), RMB and Space now and then
    if ($k % 10 -eq 1) { HW-KeyDown 'W' }
    if ($k % 10 -eq 6) { HW-KeyUp 'W'; HW-KeyDown 'S'; Start-Sleep -Milliseconds 150; HW-KeyUp 'S' }
    HW-Move (500 + 150 * ($k % 9)) (240 + 80 * ($k % 6))
    if ($k % 4 -eq 0) { HW-Key 'Space' 40 }
    if ($k % 11 -eq 0) { [WH]::Button($script:hwh, 1, 960, 400, $true); Start-Sleep -Milliseconds 100; [WH]::Button($script:hwh, 1, 960, 400, $false) }
    $n++; $f = HW-Shot ('{0:000}' -f $n); $px = HW-Pixels $f
    Start-Sleep -Milliseconds 300
}
HW-KeyUp 'W'
HW-Note ("marks: " + ((HW-Lines 'mark=' $m | ForEach-Object { ($_ -replace '^.*JAMHUD ', '') } | Group-Object { ($_ -split ' ')[1] + ' ' + ($_ -split ' ')[2] } | ForEach-Object { "$($_.Name) x$($_.Count)" }) -join ' | '))
[void](HW-Close -ByConsole)
