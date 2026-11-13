# A54: every 90 minutes until -Until, read the ref of main (the one allowed git call) and, when a package Hokej_Hokej_H2* (or newer than the last graded) exists,
# run the full grid on it (shoot, eyes, table, report with the diff against the previous run). Usage: powershell -File loop.ps1 -Until 06:30
param([string]$Until = '06:30', [string]$Last = 'h19')
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
while ((Get-Date) -lt [datetime]::Today.AddDays(1).Date.Add([timespan]$Until) -and (Get-Date).Hour -ne ([timespan]$Until).Hours + 1) {
    $head = (& git -C C:\Users\kazda\kiro\thirdjam log -1 --format=%h main).Trim()
    $pk = Get-ChildItem C:\Users\kazda\kiro\thirdjam\Game\Packaged -Directory -Filter 'Hokej_Hokej_H2*' -ErrorAction SilentlyContinue | Sort-Object Name | Select-Object -Last 1
    "$(Get-Date -Format HH:mm) main=$head package=$($pk.Name)" | Add-Content "$PSScriptRoot\loop.log"
    if ($pk -and -not (Test-Path "$root\tools\qa\out\eyesgrid\$($pk.Name.Split('_')[-1].ToLower())\manifest.json")) {
        $run = $pk.Name.Split('_')[-1].ToLower()
        node "$root\tools\qa\eyesgrid\grid.mjs" shoot --run $run --pkg "$($pk.FullName)\Windows" --only hockey
        node "$root\tools\qa\eyesgrid\grid.mjs" eyes --run $run
        node "$root\tools\qa\eyesgrid\table.mjs" --run $run
        node "$root\tools\qa\eyesgrid\report.mjs" --runs "$Last,$run"
        $Last = $run
    }
    Start-Sleep -Seconds 5400
}
