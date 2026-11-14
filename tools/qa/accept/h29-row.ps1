# Lane A155 (H29 rows 1 and 2): the faceoff without the post, then the stop mark in a PRO match started by PLAY NOW (click PRO, Return) with drill 21 staging the carries:
# the distance at which the mark first lights in each episode (`JAMHUD mark=stop state=lit ... dist=`): min / median / max, and a still of it lit.
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H29', [string]$Name = 'a155-h29', [int]$Seconds = 75, [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"; $exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
$n = 0
function Still([string]$tag) { $script:n++; $f = HW-Shot ('{0:000}-{1}' -f $script:n, $tag) }
[void](HW-Start $exe '' $out -Port $Port -Console -WaitSec 120)
[void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
HW-Click 959 551; Start-Sleep -Milliseconds 700
$m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
[void](HW-Wait 'JAMMATCH level=' 30 $m); HW-Note ("match: " + ((HW-Lines 'JAMMATCH level=' $m) -join ' || '))
Start-Sleep -Milliseconds 800; Still 'faceoff-a'; Start-Sleep -Milliseconds 500; Still 'faceoff-b'
Start-Sleep -Seconds 4
[void](HW-Console 'jam.ctrl.Drill 21' 5)
$t0 = Get-Date
while (((Get-Date) - $t0).TotalSeconds -lt $Seconds) { Still 'carry'; Start-Sleep -Milliseconds 150; if ((HW-Lines 'JAMDRILL carry summary' $m).Count) { break } }
[void](HW-Console 'jam.ctrl.Drill 0' 5)
$lines = HW-Lines 'mark=stop' $m
$first = @(); $prev = 'off'
foreach ($l in $lines) { if ($l -match 'state=(\w+) def=(-?\d+) dist=([\d.]+)') { $st = $Matches[1]; $d = [double]$Matches[3]; if ($st -eq 'lit' -and $prev -ne 'lit') { $first += $d }; $prev = $st } }
$s = $first | Sort-Object
if ($s.Count) { $med = if ($s.Count % 2) { $s[[int](($s.Count - 1) / 2)] } else { ($s[$s.Count / 2 - 1] + $s[$s.Count / 2]) / 2 }; HW-Note ("MARK first-lit dist episodes=$($s.Count) min=$($s[0]) median=$med max=$($s[-1]) all=" + ($s -join ',')) } else { HW-Note 'MARK no lit episode' }
HW-Note ("JAMMATCH: " + ((HW-Lines 'JAMMATCH level=' $m) -join ' || '))
[void](HW-Close -ByConsole)
