# Lane A12: run the rink map in a windowed -game process from the editor build (no editor UI, no MCP port), with the scripted human so goals and
# checks happen, force a few barks through the console at given seconds, capture the window, then stop only that process and list the JAMBARK log lines.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\barks\pieshot.ps1 [-Map /Game/Maps/Rink] [-Seconds 45] [-ShotAt 20,30,40] [-Cut hockey.regular]
param([string]$Map = '/Game/Maps/Rink', [int]$Seconds = 45, [int[]]$ShotAt = @(20, 30, 40), [string]$Cut = 'hockey.regular', [string]$Name = 'barks')
Add-Type -AssemblyName System.Drawing
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @"
using System; using System.Runtime.InteropServices;
public struct RECT { public int Left, Top, Right, Bottom; }
public static class W32 {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref System.Drawing.Point p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
"@
$repo = 'C:\Users\kazda\kiro\thirdjam'
$exe = 'C:\Program Files\Epic Games\UE_5.9\Engine\Binaries\Win64\UnrealEditor.exe'
$out = Join-Path $repo "tools\barks\out\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$log = Join-Path $out 'game.log'
$args = "`"$repo\Game\ThirdJam.uproject`" $Map -game -windowed -ResX=1280 -ResY=720 -log -abslog=`"$log`" -cut=$Cut -ExecCmds=`"jam.ScriptHuman 1`""
$p = Start-Process -FilePath $exe -ArgumentList $args -PassThru
"started pid $($p.Id)" | Set-Content (Join-Path $out 'capture.log')
$sw = [Diagnostics.Stopwatch]::StartNew()
$done = @{}
while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
  Start-Sleep -Milliseconds 500
  $p.Refresh()
  if ($p.HasExited) { "game exited at $([int]$sw.Elapsed.TotalSeconds) s code $($p.ExitCode)" | Add-Content (Join-Path $out 'capture.log'); break }
  $t = [int]$sw.Elapsed.TotalSeconds
  if (($ShotAt -contains $t) -and -not $done[$t]) {
    $done[$t] = $true
    $h = $p.MainWindowHandle
    if ($h -eq [IntPtr]::Zero) { "no window at $t s" | Add-Content (Join-Path $out 'capture.log'); continue }
    $r = New-Object RECT; [W32]::GetClientRect($h, [ref]$r) | Out-Null
    $pt = New-Object System.Drawing.Point 0, 0; [W32]::ClientToScreen($h, [ref]$pt) | Out-Null
    $w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
    if ($w -lt 100 -or $hh -lt 100) { "window too small at $t s" | Add-Content (Join-Path $out 'capture.log'); continue }
    [W32]::SetForegroundWindow($h) | Out-Null
    Start-Sleep -Milliseconds 200
    $bmp = New-Object System.Drawing.Bitmap $w, $hh
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $g.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size)
    $file = Join-Path $out ('t{0:000}s.png' -f $t)
    $bmp.Save($file, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    "captured $file" | Add-Content (Join-Path $out 'capture.log')
  }
}
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false; "stopped game pid $($p.Id) after $Seconds s" | Add-Content (Join-Path $out 'capture.log') }
Start-Sleep -Seconds 2
Get-Content (Join-Path $out 'capture.log')
if (Test-Path $log) {
  $barks = Select-String -Path $log -Pattern 'JAMBARK' | ForEach-Object { $_.Line }
  "JAMBARK lines: $($barks.Count)"
  $barks | Select-Object -First 30 | ForEach-Object { $_.Substring([Math]::Min(30, $_.Length)) }
  $events = Select-String -Path $log -Pattern 'JAMEVENT (goal|faceoff|check|knockdown|win|lose)' | Measure-Object
  "JAMEVENT goal/faceoff/check/knockdown/win/lose lines: $($events.Count)"
}
