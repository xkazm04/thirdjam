# Lane A155: one look at a tree in a window (the uncooked game from the main checkout): the clay regular cut by PLAY NOW, then by console: poke, knockdown, goal, Glass House.
param([string]$Editor = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe', [string]$Project = 'C:\Users\kazda\kiro\thirdjam\Game\ThirdJam.uproject', [string]$Name = 'a155-look', [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"
$n = 0
function Still([string]$tag) { $script:n++; $f = HW-Shot ('{0:00}-{1}' -f $script:n, $tag); $px = HW-Pixels $f; HW-Note ("STILL $tag lit=$($px.lit) mean=$($px.mean) utc=" + [DateTime]::UtcNow.ToString('HH:mm:ss.fff')) }
function Cmd([string]$c) { return (HW-Console $c 10) }
$ok = HW-Start $Editor "`"$Project`" -game -Multiprocess" $out -Port $Port -Console -WaitSec 240
if (-not $ok) { HW-Note 'FAIL no window'; exit 2 }
[void](HW-Wait "Game class is 'JamMenuGameMode'" 200); Start-Sleep -Seconds 5
$m = HW-Mark; for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { break } }
[void](HW-Wait 'JAMMATCH level=' 30 $m); HW-Note ("match lines: " + ((HW-Lines 'JAMMATCH level=' $m) -join ' || ')); Start-Sleep -Seconds 2
Still 'faceoff-a'; Start-Sleep -Milliseconds 700; Still 'faceoff-b'; Start-Sleep -Seconds 4
# the poke that misses (STICK LOCKED)
$m2 = HW-Mark; HW-Key 'Space' 80; Start-Sleep -Milliseconds 250; Still 'poke-a'; Start-Sleep -Milliseconds 300; Still 'poke-b'
HW-Note ("poke lines: " + ((HW-Lines 'JAMHUD|JAMVIS press' $m2) -join ' || '))
# a carrier ring: stills through a stretch of play
foreach ($i in 1..8) { Start-Sleep -Milliseconds 900; Still 'play' }
# knockdown of the human: the acceptance drill's step 8 (the opponent checks the carrying human)
$consoleOk = Cmd 'jam.accept.From 8'; HW-Note "console works: $consoleOk"
if ($consoleOk) {
    [void](Cmd 'jam.accept.To 8'); $m3 = HW-Mark
    [void](Cmd 'open Rink?cut=C:/Users/kazda/kiro/thirdjam/.claude/worktrees/a108/tools/qa/accept/accept-cut.json'); [void](HW-Wait 'JAMMATCH level=' 30 $m3); Start-Sleep -Seconds 2
    $m3 = HW-Mark; [void](Cmd 'jam.ctrl.Drill 20')
    $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt 25 -and -not (HW-Lines 'JAMEVENT knockdown who=0 ' $m3).Count) { Start-Sleep -Milliseconds 40 }
    foreach ($i in 1..8) { Still 'knock'; Start-Sleep -Milliseconds 250 }
    Start-Sleep -Milliseconds 1500; Still 'after-getup'; Start-Sleep -Milliseconds 600; Still 'after-getup2'
    HW-Note ("knock lines: " + ((HW-Lines 'JAMEVENT knockdown|JAMACCEPT step=8|JAMACCEPT stage' $m3 | Select-Object -First 8) -join ' || '))
    # a goal: lamp, ice
    [void](Cmd 'jam.ctrl.Drill 0'); $m4 = HW-Mark
    [void](Cmd 'open Rink?cut=hockey.regular'); [void](HW-Wait 'JAMMATCH level=' 30 $m4); Start-Sleep -Seconds 3
    [void](Cmd 'jam.ctrl.EndMatch 0'); $t0 = Get-Date
    while (((Get-Date) - $t0).TotalSeconds -lt 30 -and -not (HW-Lines 'JAMRINK react goal' $m4).Count) { Start-Sleep -Milliseconds 30 }
    foreach ($i in 1..6) { Still 'goal'; Start-Sleep -Milliseconds 350 }
    HW-Note ("goal lines: " + ((HW-Lines 'JAMRINK react goal' $m4) -join ' || '))
    # Glass House
    $m5 = HW-Mark
    [void](Cmd 'open Rink?cut=../../../ThirdJam/Content/Cartridges/modes/hockey.mode.glass-house.cartridge.json'); [void](HW-Wait 'JAMMATCH level=' 30 $m5); Start-Sleep -Seconds 4
    Still 'glass-house-start'
    $t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt 110 -and -not (HW-Lines 'JAMRINK react box team' $m5).Count) { Start-Sleep -Milliseconds 100 }
    if ((HW-Lines 'JAMRINK react box team' $m5).Count) { foreach ($i in 1..4) { Start-Sleep -Milliseconds 700; Still 'glass-house-box' } } else { HW-Note 'no foul in 110 s' }
    HW-Note ("glass lines: " + ((HW-Lines 'JAMRINK arena box|JAMRINK react box|JAMEVENT (foul|penalty)' $m5 | Select-Object -First 6) -join ' || '))
}
HW-Note ("hitch lines: " + ((HW-Lines 'itch' 0 | Select-Object -First 8) -join ' || '))
[void](HW-Close)
