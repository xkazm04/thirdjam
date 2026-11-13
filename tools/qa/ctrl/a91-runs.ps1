# Lane A91 (R-0044): the five proof runs on the A91 Development package, one after the other (drills 7 sweep, 8 spin, 9 saucer, 10 shots, 11 one-timers), each through
# a91-prove.ps1 and analysed by a91-analyze.mjs (the bars as R-0044 restates them). Stops only the game processes it started (a91-prove.ps1 does).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a91-runs.ps1 -Exe Game\Packaged\A91-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe [-Drills 10,11] [-Tag run2]
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string]$Drills = '7,8,9,10,11',   # a comma list (a string, so -File passes it whole)
    [string]$Tag = ''
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$names = @{ 7 = 'a91-sweep'; 8 = 'a91-spin'; 9 = 'a91-saucer'; 10 = 'a91-shots'; 11 = 'a91-onetimer' }
$secs = @{ 7 = 90; 8 = 100; 9 = 300; 10 = 300; 11 = 150 }
# Drill 9 plays the rookie timer cut (lane A91): under the pro cut the carrier is at contact with a defender nearly always and a clear launch is rare; the saucer's
# flight over a blade does not depend on who holds the blade. Drills 7 and 8 keep the pro cut (the shield is proven against pros), 10 and 11 the rookie cut.
$cuts = @{ 9 = (Join-Path $here 'a85-cut-rookie.json') }
foreach ($d in ($Drills -split ',' | ForEach-Object { [int]$_.Trim() })) {
    $n = $names[$d]; if ($Tag) { $n = "$n-$Tag" }
    Write-Host "=== drill $d ($n) ==="
    if ($cuts.ContainsKey($d)) { & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'a91-prove.ps1') -Exe $Exe -Drill $d -Name $n -Seconds $secs[$d] -Cut $cuts[$d] | Select-Object -Last 4 }
    else { & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $here 'a91-prove.ps1') -Exe $Exe -Drill $d -Name $n -Seconds $secs[$d] | Select-Object -Last 4 }
    $log = Join-Path $root "tools\qa\out\ctrl\$n\game.log"
    if (Test-Path $log) { & node (Join-Path $here 'a91-analyze.mjs') $log | Tee-Object -FilePath (Join-Path $root "tools\qa\out\ctrl\$n\analyze.txt") } else { Write-Host "FAIL no log for $n" }
}
Write-Host 'runs done'
