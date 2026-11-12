# Lane A49: the two scripted drills in the packaged Development build, one game at a time, then the analysis of each log.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a49-drills.ps1 -Exe <Development ThirdJam.exe>
# jam.ctrl.Drill 1 = tackle (what the right mouse button sends: one frame of bCheck), 2 = cross-check (middle mouse: one frame of bLift); the human's Move is the
# WASD unit vector toward the carrier; 10 attempts each, the sim's own events decide, JAMDRILL lines and a summary.
param([Parameter(Mandatory = $true)][string]$Exe, [int]$Seconds = 110)
$here = $PSScriptRoot
foreach ($d in 1, 2) {
    $name = "drill$d"
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'a49-prove.ps1') -Exe $Exe -Name $name -Seconds $Seconds -GameArgs "-dpcvars=jam.ctrl.Drill=$d" -NoSequence
    $log = Join-Path (Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent) "tools\qa\out\ctrl\$name\game.log"
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMDRILL' | ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' } }
    else { Write-Host "FAIL $name no log" }
}
