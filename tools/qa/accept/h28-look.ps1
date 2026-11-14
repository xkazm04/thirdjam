# Lane A155 (H28 rows 1 and 2): the faceoff (the post), and the two marks in a real match. PLAY NOW by Return, then stills through play; then drill 21 (the human carries: the chalk ring) and the acceptance drill's step 8 (a downed human).
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H28', [string]$Name = 'a155-h28look', [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"; $exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
$n = 0
function Still([string]$tag) { $script:n++; $f = HW-Shot ('{0:000}-{1}' -f $script:n, $tag); $px = HW-Pixels $f; HW-Note ("STILL $tag lit=$($px.lit) utc=" + [DateTime]::UtcNow.ToString('HH:mm:ss.fff')) }
[void](HW-Start $exe '' $out -Port $Port -Console -WaitSec 120)
[void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
[void](HW-Console 'jam.menuscreen where' 5)
$m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
[void](HW-Wait 'JAMMATCH level=' 30 $m); HW-Note ("match: " + ((HW-Lines 'JAMMATCH level=|JAMCART loaded' $m) -join ' || '))
Start-Sleep -Milliseconds 800; Still 'faceoff-a'; Start-Sleep -Milliseconds 500; Still 'faceoff-b'
# play: the human skates and aims; stills every ~0.7 s
HW-Move 1300 380; Start-Sleep -Seconds 3
HW-KeyDown 'W'
foreach ($k in 1..45) {
    HW-Move (700 + 120 * ($k % 8)) (260 + 70 * ($k % 5))
    if ($k % 9 -eq 0) { HW-KeyUp 'W'; Start-Sleep -Milliseconds 200; HW-KeyDown 'W' }
    if ($k % 7 -eq 0) { HW-Key 'Space' 40 }
    Still 'play'; Start-Sleep -Milliseconds 350
}
HW-KeyUp 'W'
HW-Note ("hud/marks lines: " + ((HW-Lines 'JAMHUD|JAMMARK|marks' $m | Select-Object -First 25) -join ' || '))
# the carry drill: the chalk ring ahead of the carrying human
[void](HW-Console 'jam.ctrl.Drill 0' 5); $m2 = HW-Mark
[void](HW-Console 'open Rink?cut=hockey.regular?level=pro' 10); [void](HW-Wait 'JAMMATCH level=' 30 $m2); Start-Sleep -Seconds 2
[void](HW-Console 'jam.ctrl.Drill 21' 5)
foreach ($k in 1..30) { Still 'carry'; Start-Sleep -Milliseconds 400 }
[void](HW-Console 'jam.ctrl.Drill 0' 5)
# a downed human (acceptance step 8)
[void](HW-Console 'jam.accept.From 8' 5); [void](HW-Console 'jam.accept.To 8' 5); $m3 = HW-Mark
[void](HW-Console 'open Rink?cut=C:/Users/kazda/kiro/thirdjam/.claude/worktrees/a108/tools/qa/accept/accept-cut.json' 10); [void](HW-Wait 'JAMMATCH level=' 30 $m3); Start-Sleep -Seconds 2
$m3 = HW-Mark; [void](HW-Console 'jam.ctrl.Drill 20' 5)
$t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt 25 -and -not (HW-Lines 'JAMEVENT knockdown who=0 ' $m3).Count) { Start-Sleep -Milliseconds 40 }
foreach ($k in 1..10) { Still 'knock'; Start-Sleep -Milliseconds 200 }
HW-Note ("knock: " + ((HW-Lines 'JAMEVENT knockdown|JAMACCEPT step=8' $m3 | Select-Object -First 3) -join ' || '))
[void](HW-Close -ByConsole)
