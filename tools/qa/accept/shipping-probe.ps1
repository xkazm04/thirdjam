# Lane A87: what a Shipping package lets the acceptance run judge. Starts the Shipping exe (its own process only), asks the TCP remote console on the
# port, looks for a log file, stops what it started. Output: tools\qa\out\accept\shipping-<Name>\probe.txt.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\shipping-probe.ps1 -Exe Game\Packaged\Hokej_Hokej_H24\Windows\ThirdJam\Binaries\Win64\ThirdJam-Win64-Shipping.exe -Name h24
param([Parameter(Mandatory = $true)][string]$Exe, [string]$Name = 'h24', [int]$Port = 4662)
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $root "tools\qa\out\accept\shipping-$Name"; New-Item -ItemType Directory -Force $out | Out-Null
$rep = Join-Path $out 'probe.txt'; Remove-Item $rep -ErrorAction SilentlyContinue
function Note([string]$s) { Add-Content $rep $s; Write-Host $s }
$Exe = (Resolve-Path $Exe).Path
$log = Join-Path $out 'game.log'
$a = "-dpcvars=jam.ctrl.Drill=20 -windowed -ResX=1280 -ResY=720 -WinX=60 -WinY=60 -log -abslog=`"$log`" -ForceLogFlush -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru; $null = $p.Handle; $start = $p.StartTime
Note "start pid $($p.Id) $Exe $a"
Start-Sleep -Seconds 25
$conn = 'refused'
try { $c = New-Object System.Net.Sockets.TcpClient; $c.Connect('127.0.0.1', $Port); $conn = 'connected'; $c.Close() } catch { $conn = "refused ($($_.Exception.InnerException.Message))" }
Note "remote console on ${Port}: $conn"
Note ("abslog file: " + $(if (Test-Path $log) { "present, $((Get-Item $log).Length) bytes" } else { 'absent' }))
$saved = @((Join-Path $env:LOCALAPPDATA 'ThirdJam\Saved\Logs'), (Join-Path (Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent) 'Saved\Logs'))
foreach ($d in $saved) { if (Test-Path $d) { Get-ChildItem $d | Where-Object { $_.LastWriteTime -ge $start } | ForEach-Object { Note "log written since start: $($_.FullName) $($_.Length) bytes" } } else { Note "no dir $d" } }
Note "process alive after 25 s: $(-not $p.HasExited)"
. (Join-Path $root 'tools\run\common.ps1'); [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our process tree'
