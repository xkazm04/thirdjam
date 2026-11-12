$out = 'C:\Users\kazda\kiro\thirdjam\tools\qa\out'
$bin = 'C:\Users\kazda\kiro\thirdjam\Game\Binaries\Win64'
$deadline = (Get-Date).AddMinutes(25)
$quiet = 0
while ((Get-Date) -lt $deadline) {
  $net = Test-Path (Join-Path $bin 'UnrealEditor-JamNet.dll')
  $game = Get-Item (Join-Path $bin 'UnrealEditor-JamGame.dll') -ErrorAction SilentlyContinue
  $fresh = $game -and $game.LastWriteTime -gt [datetime]'2026-11-12 11:07'
  $ubt = Get-CimInstance Win32_Process -Filter "Name = 'dotnet.exe'" | Where-Object { $_.CommandLine -like '*UnrealBuildTool*' }
  $cl = Get-Process cl, link, UbaAgent -ErrorAction SilentlyContinue
  if ($net -and $fresh -and -not $ubt -and -not $cl) { $quiet++ } else { $quiet = 0 }
  "$(Get-Date -Format HH:mm:ss) net=$net fresh=$fresh ubt=$([bool]$ubt) cl=$([bool]$cl) quiet=$quiet" | Add-Content (Join-Path $out 'watch.log')
  if ($quiet -ge 3) { break }
  Start-Sleep -Seconds 10
}
if ($quiet -ge 3) {
  $log = Join-Path $out 'editor-8010-4.log'
  $exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe'
  $a = @('"C:\Users\kazda\kiro\thirdjam\Game\ThirdJam.uproject"','-nosplash','-ModelContextProtocolStartServer','-ModelContextProtocolPort=8010',"-abslog=`"$log`"")
  $p = Start-Process -FilePath $exe -ArgumentList $a -PassThru
  $state = [ordered]@{ pid=$p.Id; start=$p.StartTime.ToString('o'); port=8010; log=$log; lane='A10' }
  $state | ConvertTo-Json -Compress | Set-Content -Encoding utf8 (Join-Path $out 'editor-8010.json')
  "$(Get-Date -Format HH:mm:ss) started editor pid $($p.Id)" | Add-Content (Join-Path $out 'watch.log')
} else {
  "$(Get-Date -Format HH:mm:ss) gave up: binaries never settled" | Add-Content (Join-Path $out 'watch.log')
}
