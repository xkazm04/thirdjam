# Lane A207: stills of the pass's power line (HUDPassMeter), off screen from the game camera. One editor-binary game process, -RenderOffScreen (no window,
# no cursor), the scripted human (jam.ScriptHuman 1), the line drawn while he carries as if held 20 frames (jam.hud.PassMeterDemo 20), a still 10 frames into
# each showing (jam.hud.PassMeterStills 10, at most 12). Its pid goes to out\meterstill.json; it is stopped by that pid after $Seconds.
param([int]$Seconds = 150, [int]$Demo = 20, [int]$Port = 4807)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
$uproject = Join-Path $root 'Game\ThirdJam.uproject'
$exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe'
$out = Join-Path $PSScriptRoot 'out'
$log = Join-Path $out 'meterstill.log'
$cut = (Join-Path $root 'tools\qa\screen\a196\cut-clay-long.json').Replace('\', '/')
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter 'u1-passmeter-*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$cvars = "jam.ScriptHuman=1,jam.hud.PassMeterDemo=$Demo,jam.hud.PassMeterStills=10,jam.screen.HoldStill=1,jam.screen.FadeSeconds=0,r.RDG.AsyncCompute=0"
$a = "`"$uproject`" /Game/Maps/Rink -game -RenderOffScreen -Multiprocess -ResX=1280 -ResY=720 -ForceRes -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -log -abslog=`"$log`" -cut=$cut -dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
@{ pid = $p.Id; start = (Get-Date).ToString('o'); lane = 'A207' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'meterstill.json')
Write-Host "started pid $($p.Id)"
$t = 0
while ($t -lt $Seconds -and -not $p.HasExited) { Start-Sleep -Seconds 5; $t += 5; if ((Get-ChildItem $shotDir -Filter 'u1-passmeter-*.png' -ErrorAction SilentlyContinue).Count -ge 6) { break } }
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force; Write-Host "stopped pid $($p.Id) (mine)" }
$shots = Get-ChildItem $shotDir -Filter 'u1-passmeter-*.png' -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force (Join-Path $out 'stills') | Out-Null
$shots | ForEach-Object { Copy-Item $_.FullName (Join-Path $out 'stills') -Force }
Write-Host "stills: $($shots.Count)"
