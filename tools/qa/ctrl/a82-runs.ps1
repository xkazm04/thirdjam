# Lane A82: the five proof runs on the A82 Development package, one after the other (drills 7 sweep, 8 spin, 9 saucer, 10 shots, 11 one-timers), each through
# a82-prove.ps1 and analysed by a82-analyze.mjs. Stops only the game processes it started (a82-prove.ps1 does).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a82-runs.ps1 -Exe Game\Packaged\A82-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [int[]]$Drills = @(7, 8, 9, 10, 11)
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$names = @{ 7 = 'a82-sweep'; 8 = 'a82-spin'; 9 = 'a82-saucer'; 10 = 'a82-shots'; 11 = 'a82-onetimer' }
$secs = @{ 7 = 70; 8 = 90; 9 = 150; 10 = 150; 11 = 120 }
foreach ($d in $Drills) {
    $n = $names[$d]
    Write-Host "=== drill $d ($n) ==="
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'a82-prove.ps1') -Exe $Exe -Drill $d -Name $n -Seconds $secs[$d]
    $log = Join-Path $root "tools\qa\out\ctrl\$n\game.log"
    if (Test-Path $log) { & node (Join-Path $here 'a82-analyze.mjs') $log | Tee-Object -FilePath (Join-Path $root "tools\qa\out\ctrl\$n\analyze.txt") } else { Write-Host "FAIL no log for $n" }
}
Write-Host 'runs done'
