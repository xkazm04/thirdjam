$out = 'C:\Users\kazda\kiro\thirdjam\tools\qa\out\soak-1'
New-Item -ItemType Directory -Force $out | Out-Null
$wlog = Join-Path $out 'window.log'
$deadline = [datetime]'2026-11-12 15:40'
$window = [datetime]'2026-11-12 14:30'
$quiet = 0
while ((Get-Date) -lt $deadline) {
  $eds = @(Get-Process UnrealEditor, UnrealEditor-Cmd, UnrealPak, ShaderCompileWorker -ErrorAction SilentlyContinue)
  $ubt = @(Get-CimInstance Win32_Process -Filter "Name = 'dotnet.exe'" | Where-Object { $_.CommandLine -like '*UnrealBuildTool*' -or $_.CommandLine -like '*AutomationTool*' })
  $cl = @(Get-Process cl, link -ErrorAction SilentlyContinue)
  $inWindow = (Get-Date) -ge $window
  if ($inWindow -and $eds.Count -eq 0 -and $ubt.Count -eq 0 -and $cl.Count -eq 0) { $quiet++ } else { $quiet = 0 }
  "$(Get-Date -Format HH:mm:ss) window=$inWindow editors=$($eds.Count) [$(($eds | ForEach-Object { $_.Id }) -join ',')] ubt=$($ubt.Count) cl=$($cl.Count) quiet=$quiet" | Add-Content $wlog
  if ($quiet -ge 3) { break }
  Start-Sleep -Seconds 10
}
if ($quiet -ge 3) {
  $log = Join-Path $out 'editor-soak-1.log'
  $exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe'
  $a = @('"C:\Users\kazda\kiro\thirdjam\Game\ThirdJam.uproject"','-nosplash','-ModelContextProtocolStartServer','-ModelContextProtocolPort=8010',"-abslog=`"$log`"")
  $p = Start-Process -FilePath $exe -ArgumentList $a -PassThru
  @{ pid=$p.Id; start=$p.StartTime.ToString('o'); port=8010; log=$log; lane='A10' } | ConvertTo-Json -Compress | Set-Content -Encoding utf8 'C:\Users\kazda\kiro\thirdjam\tools\qa\out\editor-8010.json'
  "$(Get-Date -Format HH:mm:ss) started editor pid $($p.Id) log $log" | Add-Content $wlog
  Write-Output "STARTED $($p.Id)"
} else {
  "$(Get-Date -Format HH:mm:ss) gave up at the deadline" | Add-Content $wlog
  Write-Output 'GAVEUP'
}
