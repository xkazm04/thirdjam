# Lane A155 (S2 evolution 3, row 3 and the stills of row 5): the rows other lanes queued for the cursor's owner, played in a real window of a Development package.
# The match is started by the console (`open Rink?cut=<cut>`, one build, a rendered rink: see the A155 finding on the menu's second build), then the drill is typed:
#   law      jam.ctrl.Drill 22 with jam.ctrl.LawHuman 0 (the human throws the nudge) and 1 (the human throws the hit): the JAMDRILL law1 lines and a still every 0.5 s
#   carry    jam.ctrl.Drill 21 at rookie, pro, all_star (20 rounds each): JAMDRILL carry summary (past=), the coach line on screen
#   goal     jam.ctrl.EndMatch 0 (a win) and 1 (a loss): one `JAMRINK react goal` line per goal, the end card, the lamp and the net
#   refused  drill 10 + HandPuck (the human carries), RMB: `JAMVIS press` / `JAMHUD contact_line=NOT_WITH_THE_PUCK refused=1` and its still
#   board    the human skates into the boards: `JAMRINK react board` and the flash
#   pause    Esc in a match: the pause card
#   shotgun  the Shotgun mode, F pressed: ability fired
# Stills (and each one's lit share) go to tools\qa\out\drills\<Name>\; the log lines to rows.txt.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\drills-row.ps1 [-Package <dev package>] [-Name a155-drills] [-Scenarios law,carry]
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H27', [string]$Name = 'a155-drills', [int]$Port = 4708, [string[]]$Scenarios = @())
$Scenarios = @($Scenarios | ForEach-Object { $_ -split ',' })
. (Join-Path $PSScriptRoot 'winhand.ps1')
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $root "tools\qa\out\drills\$Name"
$stage = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam'
$exe = Join-Path $stage 'Binaries\Win64\ThirdJam.exe'
$res = New-Object System.Collections.ArrayList
function Row([string]$s) { [void]$res.Add($s); HW-Note "ROW $s" }
function Want([string]$n) { return ($Scenarios.Count -eq 0 -or $Scenarios -contains $n) }
function Cmd([string]$c) { [void](HW-Console $c 10) }
$script:stillN = 0
function Still([string]$tag) { $script:stillN++; $f = HW-Shot ('{0:000}-{1}' -f $script:stillN, $tag); $px = HW-Pixels $f; Row ("still {0} lit={1} mean={2} at {3}" -f (Split-Path $f -Leaf), $px.lit, $px.mean, [DateTime]::UtcNow.ToString('HH:mm:ss.fff')); return $f }
function OpenMatch([string]$cutPath, [string]$level = '') {
    Cmd 'jam.ctrl.Drill 0'; Cmd 'jam.ctrl.HandPuck 0'   # a drill left on by the match before does not reach this one
    $m = HW-Mark
    $opt = "open Rink?cut=$cutPath"; if ($level) { $opt += "?level=$level" }
    Cmd $opt
    $l = HW-Wait 'JAMMATCH level=' 25 $m
    Start-Sleep -Seconds 4
    $mm = @(HW-Lines 'JAMMATCH level=' $m)
    Row ("open $cutPath $level -> builds=$($mm.Count) last=[" + ($(if ($mm.Count) { $mm[-1] -replace '^.*?JAMMATCH', 'JAMMATCH' } else { 'none' })) + "]")
    return $m
}
function Snap([string]$pat, [int]$m) { foreach ($l in (HW-Lines $pat $m)) { Row ($l -replace '^\[(\d+\.\d+\.\d+-)', '[') } }
New-Item -ItemType Directory -Force $out | Out-Null
$ok = HW-Start $exe '' $out -Port $Port -Console -WaitSec 120
if (-not $ok) { Row 'FAIL no window'; exit 2 }
try {
    [void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
    $regular = 'hockey.regular'
    if (Want 'menus') {
        # the menu screens, before any match: the first screen with the level switch, the twelve mode cards, the controls screen
        [void](Still 'first-screen')
        HW-Click 960 702; Start-Sleep -Seconds 2; [void](Still 'cuts-screen'); HW-Key 'Esc'; Start-Sleep -Seconds 1
        HW-Click 696 900; Start-Sleep -Seconds 2; [void](Still 'controls-screen'); HW-Key 'Esc'; Start-Sleep -Seconds 1
    }
    if (Want 'law') {
        foreach ($lh in 0, 1) {
            $m = OpenMatch $regular
            Cmd "jam.ctrl.LawHuman $lh"; Cmd 'jam.ctrl.Drill 22'
            $t0 = Get-Date
            while (((Get-Date) - $t0).TotalSeconds -lt 40 -and -not (HW-Lines 'JAMDRILL law1 summary' $m).Count) { [void](Still "law$lh"); Start-Sleep -Milliseconds 350 }
            Snap 'JAMDRILL law1|JAMHITSTOP|JAMKICK|JAMVIS (hit|board)' $m
        }
    }
    if (Want 'carry') {
        foreach ($lv in 'rookie', 'pro', 'all_star') {
            $m = OpenMatch $regular $lv
            Cmd 'jam.ctrl.Drill 21'
            $t0 = Get-Date; $n = 0
            while (((Get-Date) - $t0).TotalSeconds -lt 150 -and -not (HW-Lines 'JAMDRILL carry summary' $m).Count) { if ($n % 6 -eq 0) { [void](Still "carry-$lv") }; $n++; Start-Sleep -Milliseconds 500 }
            Snap 'JAMDRILL carry (summary|line n=1 )' $m
            Snap 'JAMDRILL carry result' $m | Select-Object -First 0
            Cmd 'jam.ctrl.Drill 0'
        }
    }
    if (Want 'goal') {
        foreach ($team in 0, 1) {
            $m = OpenMatch $regular
            Cmd "jam.ctrl.EndMatch $team"
            $t0 = Get-Date
            while (((Get-Date) - $t0).TotalSeconds -lt 45 -and -not (HW-Lines 'JAMCTRL endmatch done' $m).Count) { [void](Still "goal-team$team"); Start-Sleep -Milliseconds 700 }
            Start-Sleep -Seconds 2; [void](Still "endcard-team$team")
            Snap 'JAMRINK react goal|JAMCTRL endmatch|JAMEVENT (win|lose|goal)' $m
        }
    }
    if (Want 'refused') {
        $m = OpenMatch 'C:/Users/kazda/kiro/thirdjam/.claude/worktrees/a108/tools/qa/accept/accept-cut.json'
        Cmd 'jam.ctrl.HandPuck 1'; Cmd 'jam.ctrl.Drill 10'
        Start-Sleep -Seconds 6
        for ($i = 0; $i -lt 12 -and -not (HW-Lines 'refused=1' $m).Count; $i++) {
            HW-Move 960 400; [WH]::Button($script:hwh, 1, 960, 400, $true); Start-Sleep -Milliseconds 120; [WH]::Button($script:hwh, 1, 960, 400, $false); Start-Sleep -Milliseconds 350; [void](Still 'refused-try'); Start-Sleep -Milliseconds 500
        }
        Snap 'JAMVIS press|JAMHUD|JAMCTRL.*btn=check' $m
    }
    if (Want 'board') {
        # the human holds W toward the boards; the moment a `JAMRINK react board` line shows, three stills with no work between them (the flash is short)
        $m = OpenMatch $regular
        Start-Sleep -Seconds 2; HW-Move 1400 300; HW-KeyDown 'W'
        $t0 = Get-Date
        while (((Get-Date) - $t0).TotalSeconds -lt 20 -and -not (HW-Lines 'JAMVIS board frame=\d+ skater=0 ' $m).Count) { Start-Sleep -Milliseconds 20 }
        foreach ($i in 1..3) { $null = HW-Shot "board-hit-$i" }
        HW-KeyUp 'W'; Snap 'JAMRINK react board|JAMVIS board' $m
    }
    if (Want 'stopgo') {
        $m = OpenMatch $regular 'pro'
        Cmd 'jam.ctrl.Drill 21'
        $t0 = Get-Date
        while (((Get-Date) - $t0).TotalSeconds -lt 22) { [void](Still 'stopgo'); Start-Sleep -Milliseconds 200 }
        Snap 'JAMDRILL carry (line|stage|result) n=[12] ' $m
        Cmd 'jam.ctrl.Drill 0'
    }
    if (Want 'pause') {
        $m = OpenMatch $regular
        Start-Sleep -Seconds 3; HW-Key 'Esc'; Start-Sleep -Seconds 1; [void](Still 'pause'); HW-Key 'Esc'
    }
    if (Want 'shotgun') {
        $m = OpenMatch '../../../ThirdJam/Content/Cartridges/modes/hockey.mode.shotgun.cartridge.json'
        Start-Sleep -Seconds 6; HW-Key 'F'; Start-Sleep -Milliseconds 300; [void](Still 'shotgun-F'); Start-Sleep -Milliseconds 400; [void](Still 'shotgun-F2')
        Snap 'JAMABILITY|JAMEVENT ability' $m
    }
    if (Want 'faceoff') {
        $m = OpenMatch $regular
        [void](Still 'faceoff'); Start-Sleep -Seconds 1; [void](Still 'faceoff2')
    }
}
finally {
    [void](HW-Close -ByConsole)
    $res | Set-Content (Join-Path $out 'rows.txt')
}
