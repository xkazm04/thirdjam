# Lane A155 (row 6): the Shipping zip, played once through the window: unzipped copy, start, first screen, Return, the first minute by posted keys, quit by the window's close.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\shipping-play.ps1 -Root <unzipped Windows folder> [-Name a155-ship]
param([string]$Root, [string]$Name = 'a155-ship')
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\ship\$Name"
$ok = HW-Start (Join-Path $Root 'ThirdJam.exe') '' $out -NoLog -WaitSec 120
if (-not $ok) { HW-Note 'FAIL no window'; exit 2 }
$fgBad = 0
function Fg { $t = [WH]::Fg(); if ($t -match 'Windows Security|Firewall') { $script:fgBad++; HW-Note "FOREIGN DIALOG holds the foreground: $t (not touched)" } }
Start-Sleep -Seconds 15; Fg
$f = HW-Shot 'menu'; HW-Note ('menu pixels ' + ((HW-Pixels $f) | ConvertTo-Json -Compress))
HW-Key 'Return'; Start-Sleep -Seconds 8
$f = HW-Shot 'match-8s'; $px = HW-Pixels $f; HW-Note ('match 8 s pixels ' + ($px | ConvertTo-Json -Compress))
# the first minute: skate, aim, shoot, check
HW-Move 1300 400; HW-KeyDown 'W'
$t0 = Get-Date; $n = 0
while (((Get-Date) - $t0).TotalSeconds -lt 50) {
    Start-Sleep -Seconds 5; $n++
    HW-Move (900 + 100 * ($n % 5)) (300 + 60 * ($n % 3))
    if ($n % 3 -eq 0) { [WH]::Button($script:hwh, 0, 960, 400, $true); Start-Sleep -Milliseconds 200; [WH]::Button($script:hwh, 0, 960, 400, $false) }
    if ($n % 4 -eq 0) { HW-KeyUp 'W'; Start-Sleep -Milliseconds 300; HW-KeyDown 'W' }
    $f = HW-Shot ("play-$n"); $px = HW-Pixels $f; HW-Note ("play $n pixels $($px | ConvertTo-Json -Compress) alive=$(HW-Alive)"); Fg
    if (-not (HW-Alive)) { HW-Note 'GAME GONE while playing'; break }
}
HW-KeyUp 'W'
$res = HW-Close -WaitSec 40
HW-Note "RESULT close=$res foreign_dialog_checks_bad=$fgBad"
