# Lane A85 (R-0042): the five proof runs on the A85 Development package, one after the other (drills 7 sweep, 8 spin, 9 saucer, 10 shots, 11 one-timers), each through
# a85-prove.ps1 and analysed by a85-analyze.mjs. Stops only the game processes it started (a85-prove.ps1 does).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a85-runs.ps1 -Exe Game\Packaged\A85-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string]$Drills = '7,8,9,10,11'   # a comma list (a string, so -File passes it whole)
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$names = @{ 7 = 'a85-sweep'; 8 = 'a85-spin'; 9 = 'a85-saucer'; 10 = 'a85-shots'; 11 = 'a85-onetimer' }
$secs = @{ 7 = 70; 8 = 90; 9 = 150; 10 = 150; 11 = 120 }
foreach ($d in ($Drills -split ',' | ForEach-Object { [int]$_.Trim() })) {
    $n = $names[$d]
    Write-Host "=== drill $d ($n) ==="
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'a85-prove.ps1') -Exe $Exe -Drill $d -Name $n -Seconds $secs[$d]
    $log = Join-Path $root "tools\qa\out\ctrl\$n\game.log"
    if (Test-Path $log) { & node (Join-Path $here 'a85-analyze.mjs') $log | Tee-Object -FilePath (Join-Path $root "tools\qa\out\ctrl\$n\analyze.txt") } else { Write-Host "FAIL no log for $n" }
}
Write-Host 'runs done'
