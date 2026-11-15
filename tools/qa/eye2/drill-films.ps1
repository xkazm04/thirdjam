# Lane A219: a look whose match the person-like hand plays without a cross-check (the night look: Fog of War) filmed with the game's own drills instead:
# jam.ctrl.Drill is set on the command line, so the drill presses from the first frame and the match is the same on every start and on every exe of one
# sim; jam.hit.Capture takes the every-frame strips of the human's presses (2 = the tapped cross-check, 23 = the held one skated into a man).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye2\drill-films.ps1 -Exe <...\ThirdJam.exe> -Tag b [-Look night] [-Port 4719]
# One game process at a time. Output: tools\qa\eye2\out\<tag>-<look>-<size>-d<drill>\hits.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$Tag,
    [string]$Look = 'night',
    [string]$Drills = '2,23',
    [string]$Sizes = '720,1080',
    [int]$HitCapture = 2,
    [int]$WaitSec = 45,
    [int]$Port = 4719
)
$run = Join-Path $PSScriptRoot 'eye2-run.ps1'
$logs = Join-Path (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path 'Game\logs\a219'
$looks = @{ clay = ''; night = 'modes/hockey.mode.fog-of-war'; glow = 'modes/hockey.mode.glass-house' }
$res = @{ '720' = @(1280, 720); '1080' = @(1920, 1080) }
foreach ($size in ($Sizes -split ',')) {
    foreach ($d in ($Drills -split ',')) {
        $name = "$Tag-$Look-$size-d$d"
        $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $run, '-Exe', $Exe, '-Run', $name, '-ScriptHuman', '0', '-HitCapture', "$HitCapture", '-WaitSec', "$WaitSec", '-Port', "$Port",
            '-ResX', "$($res[$size][0])", '-ResY', "$($res[$size][1])", '-ExtraCVars', "jam.ctrl.Drill=$d")
        if ($looks[$Look]) { $a += @('-Choose', $looks[$Look]) }
        & powershell @a *> (Join-Path $logs "$name.out")
        Write-Host ("{0}  {1}" -f (Get-Date -Format 'HH:mm:ss'), (Get-Content (Join-Path $logs "$name.out") -Tail 1))
    }
}
