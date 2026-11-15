# Lane A219: the film set of one packaged exe: the person-like hand (jam.ScriptHuman 2: LMB tapped and held through the button resolver) on the three
# looks at two sizes, the game's own every-frame strips of each of his presses (jam.hit.Capture). The match is seeded by its cartridge, so two exes of
# one sim (a presentation-only change) show the SAME frames: the before / after sheets are cut from these (pair.py).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye2\films.ps1 -Exe <...\ThirdJam.exe> -Tag h39 [-Only "clay-1080,night-720"] [-ExtraCVars a=b,c=d]
# One game process at a time (each run's pid in Game\logs\a219\<run>.pid). Output: tools\qa\eye2\out\<tag>-<look>-<size>\hits.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$Tag,
    [string]$Only = '',
    [string]$ExtraCVars = '',
    [int]$HitCapture = 3,
    [int]$WaitSec = 120,
    [int]$Port = 4719
)
$run = Join-Path $PSScriptRoot 'eye2-run.ps1'
$logs = Join-Path (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path 'Game\logs\a219'
$looks = [ordered]@{ clay = ''; night = 'modes/hockey.mode.fog-of-war'; glow = 'modes/hockey.mode.glass-house' }
$sizes = [ordered]@{ '720' = @(1280, 720); '1080' = @(1920, 1080) }
$want = @($Only -split ',' | Where-Object { $_ })
foreach ($look in $looks.Keys) {
    foreach ($size in $sizes.Keys) {
        $id = "$look-$size"
        if ($want.Count -gt 0 -and $want -notcontains $id) { continue }
        $name = "$Tag-$id"
        $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $run, '-Exe', $Exe, '-Run', $name, '-ScriptHuman', '2', '-HitCapture', "$HitCapture", '-WaitSec', "$WaitSec", '-Port', "$Port",
            '-ResX', "$($sizes[$size][0])", '-ResY', "$($sizes[$size][1])")
        if ($looks[$look]) { $a += @('-Choose', $looks[$look]) }
        if ($ExtraCVars) { $a += @('-ExtraCVars', $ExtraCVars) }
        & powershell @a *> (Join-Path $logs "$name.out")
        Write-Host ("{0}  {1}" -f (Get-Date -Format 'HH:mm:ss'), (Get-Content (Join-Path $logs "$name.out") -Tail 1))
    }
}
