# Lane A155: the Glass House look: the mode card by the menu, the human skates and checks until a foul puts a skater in a box (110 s at most); pokes for STICK LOCKED.
param([string]$Editor = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe', [string]$Project = 'C:\Users\kazda\kiro\thirdjam\Game\ThirdJam.uproject', [string]$Name = 'a155-look2')
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"; $n = 0
function Still([string]$tag) { $script:n++; $f = HW-Shot ('{0:00}-{1}' -f $script:n, $tag); $px = HW-Pixels $f; HW-Note ("STILL $tag lit=$($px.lit) utc=" + [DateTime]::UtcNow.ToString('HH:mm:ss.fff')) }
$ok = HW-Start $Editor "`"$Project`" -game -Multiprocess" $out -WaitSec 240
if (-not $ok) { exit 2 }
[void](HW-Wait "Game class is 'JamMenuGameMode'" 200); Start-Sleep -Seconds 5
HW-Click 960 702; Start-Sleep -Seconds 2; $m = HW-Mark; HW-Click 519 593   # Glass House: third row, first card
[void](HW-Wait 'JAMMATCH level=' 30 $m); HW-Note ("match: " + ((HW-Lines 'JAMMATCH level=' $m) -join ' || ')); Start-Sleep -Seconds 3
Still 'start'
HW-Move 1400 300; HW-KeyDown 'W'
$t0 = Get-Date; $k = 0; $boxed = $false
while (((Get-Date) - $t0).TotalSeconds -lt 110) {
    Start-Sleep -Milliseconds 700; $k++
    HW-Move (700 + 150 * ($k % 6)) (260 + 70 * ($k % 4))
    if ($k % 3 -eq 0) { HW-Key 'Space' 60; Start-Sleep -Milliseconds 120; Still 'poke' }
    if ($k % 4 -eq 0) { [WH]::Button($script:hwh, 1, 960, 400, $true); Start-Sleep -Milliseconds 120; [WH]::Button($script:hwh, 1, 960, 400, $false) }
    if ((HW-Lines 'JAMRINK react box team' $m).Count) { $boxed = $true; break }
}
HW-KeyUp 'W'
if ($boxed) { foreach ($i in 1..5) { Start-Sleep -Milliseconds 600; Still 'box' } } else { Still 'no-foul-end' }
HW-Note ("hud: " + ((HW-Lines 'contact_line=' $m | Select-Object -First 10) -join ' || '))
HW-Note ("box: " + ((HW-Lines 'JAMRINK react box|JAMEVENT (foul|penalty)' $m | Select-Object -First 4) -join ' || '))
[void](HW-Close)
