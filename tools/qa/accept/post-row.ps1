# Lane A155: what is the black post on the puck at the faceoff? Pause at the opening faceoff, switch niagara / static meshes off one at a time, a still each; then stills of a moving puck.
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H27', [string]$Name = 'a155-post', [int]$Port = 4708)
. (Join-Path $PSScriptRoot 'winhand.ps1')
$repo = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $repo "tools\qa\out\$Name"
$exe = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam\Binaries\Win64\ThirdJam.exe'
$n = 0
function Still([string]$tag) { $script:n++; $f = HW-Shot ('{0:00}-{1}' -f $script:n, $tag); return $f }
[void](HW-Start $exe '' $out -Port $Port -Console -WaitSec 120)
[void](HW-Wait "Game class is 'JamMenuGameMode'" 90); Start-Sleep -Seconds 4
foreach ($flag in 'none', 'niagara', 'staticmeshes', 'skeletalmeshes', 'translucency', 'decals') {
    if ($flag -ne 'none') { [void](HW-Console "showflag.$flag 0" 5) }
    $m = HW-Mark; [void](HW-Console 'open Rink?cut=hockey.regular' 10); [void](HW-Wait 'JAMMATCH level=' 30 $m); Start-Sleep -Milliseconds 500
    [void](Still "faceoff-$flag-off")
    if ($flag -ne 'none') { [void](HW-Console "showflag.$flag 1" 5) }
}
# a moving puck: stills through play with everything on
$m = HW-Mark; [void](HW-Console 'open Rink?cut=hockey.regular' 10); [void](HW-Wait 'JAMMATCH level=' 30 $m); Start-Sleep -Seconds 2
$t0 = Get-Date; while (((Get-Date) - $t0).TotalSeconds -lt 10) { [void](Still 'play'); Start-Sleep -Milliseconds 80 }
[void](HW-Close -ByConsole)
