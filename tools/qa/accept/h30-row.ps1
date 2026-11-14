# Lane A155 (H30 rows): the menu's three level lines (a still each, first screen), then a PRO match by PLAY NOW: the hint strip at the first faceoff, then drill 21 (stills for the stop LINE armed and lit, and the mark's JAMHUD lines);
# then free play with the human skating for the off-line cases. -Res 1280x720 runs only the menu still and the faceoff (the strip at that size).
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H30', [string]$Name = 'a155-h30', [int]$ResX = 1920, [int]$ResY = 1080, [int]$Seconds = 60, [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"; $exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
$n = 0; function Still([string]$tag) { $script:n++; [void](HW-Shot ('{0:000}-{1}' -f $script:n, $tag)) }
[void](HW-Start $exe '' $out -Port $Port -Console -ResX $ResX -ResY $ResY -WaitSec 120)
[void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
[void](HW-Console 'jam.menuscreen where' 5)
$sx = [double]$ResX / 1920.0
function Cx([int]$x) { return [int]($x * $sx) }
function Cy([int]$y) { return [int]($y * $sx) }
HW-Click (Cx 789) (Cy 551); Start-Sleep -Milliseconds 700; HW-Move 100 100; Start-Sleep -Milliseconds 300; Still 'menu-rookie'
HW-Click (Cx 960) (Cy 551); Start-Sleep -Milliseconds 700; HW-Move 100 100; Start-Sleep -Milliseconds 300; Still 'menu-pro'
HW-Click (Cx 1131) (Cy 551); Start-Sleep -Milliseconds 700; HW-Move 100 100; Start-Sleep -Milliseconds 300; Still 'menu-allstar'
HW-Click (Cx 960) (Cy 551); Start-Sleep -Milliseconds 700   # back to PRO
$m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
[void](HW-Wait 'JAMMATCH level=' 30 $m); HW-Note ("match: " + ((HW-Lines 'JAMMATCH level=' $m) -join ' || '))
Start-Sleep -Milliseconds 800; Still 'faceoff-strip-a'; Start-Sleep -Milliseconds 500; Still 'faceoff-strip-b'
if ($ResX -lt 1900) { [void](HW-Close -ByConsole); return }
Start-Sleep -Seconds 4
[void](HW-Console 'jam.ctrl.Drill 21' 5)
$t0 = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt $Seconds) { Still 'carry'; Start-Sleep -Milliseconds 100; if ((HW-Lines 'JAMDRILL carry summary' $m).Count) { break } }
[void](HW-Console 'jam.ctrl.Drill 0' 5)
$ml = HW-Lines 'mark=stop' $m; HW-Note ("stop lines: " + ($ml.Count) + " first: " + (($ml | Select-Object -First 4 | ForEach-Object { $_ -replace '^.*JAMHUD ', '' }) -join ' || '))
# free play: skate the open ice
$m2 = HW-Mark; [void](HW-Console 'open Rink?cut=hockey.regular?level=pro' 10); [void](HW-Wait 'JAMMATCH level=' 30 $m2); Start-Sleep -Seconds 4
HW-KeyDown 'W'; $t1 = Get-Date; $k = 0
while (((Get-Date) - $t1).TotalSeconds -lt 60) { $k++; HW-Move (500 + 150 * ($k % 9)) (240 + 80 * ($k % 6)); if ($k % 10 -eq 0) { HW-KeyUp 'W'; Start-Sleep -Milliseconds 150; HW-KeyDown 'W' }; Still 'play'; Start-Sleep -Milliseconds 250 }
HW-KeyUp 'W'
HW-Note ("play marks: " + ((HW-Lines 'mark=' $m2 | ForEach-Object { ($_ -replace '^.*JAMHUD frame=\d+ ', '') } | Select-Object -First 30) -join ' || '))
[void](HW-Close -ByConsole)
