# Lane A10: run the staged Development game with the scripted human on a cartridge path, capture its window every N seconds.
#   powershell -File tools\qa\gamerun.ps1 -Cut <cartridge json> -Name <run> [-Seconds 100] [-Every 2] [-Exe <ThirdJam.exe>] [-ExtraArgs '...']
param([string]$Cut, [string]$Name = 'game', [int]$Seconds = 100, [double]$Every = 2,
      [string]$Exe = 'C:\Users\kazda\kiro\thirdjam\Game\Saved\StagedBuilds\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe',
      [string]$ExtraArgs = '-dpcvars=jam.ScriptHuman=1 -ExecCmds="jam.ScriptHuman 1"')
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct RECT { public int Left, Top, Right, Bottom; }
public static class W32 {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref System.Drawing.Point p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
}
"@
$out = Join-Path 'C:\Users\kazda\kiro\thirdjam\tools\qa\out' $Name
New-Item -ItemType Directory -Force (Join-Path $out 'shots') | Out-Null
$log = Join-Path $out 'game.log'
$cutArg = $Cut.Replace('\', '/')
$args = "-windowed -ResX=1280 -ResY=720 -log -abslog=`"$log`" -cut=$cutArg $ExtraArgs"
$p = Start-Process -FilePath $Exe -ArgumentList $args -PassThru
@{ pid = $p.Id; start = $p.StartTime.ToString('o'); args = $args; lane = 'A10' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
$sw = [Diagnostics.Stopwatch]::StartNew()
$n = 0
while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
  Start-Sleep -Milliseconds ([int]($Every * 1000))
  $p.Refresh()
  if ($p.HasExited) { "game exited at $([int]$sw.Elapsed.TotalSeconds) s code $($p.ExitCode)" | Add-Content (Join-Path $out 'capture.log'); break }
  $h = $p.MainWindowHandle
  if ($h -eq [IntPtr]::Zero) { continue }
  $r = New-Object RECT; [W32]::GetClientRect($h, [ref]$r) | Out-Null
  $pt = New-Object System.Drawing.Point 0, 0; [W32]::ClientToScreen($h, [ref]$pt) | Out-Null
  $w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
  if ($w -lt 100 -or $hh -lt 100) { continue }
  if ($n -eq 0) { [W32]::SetForegroundWindow($h) | Out-Null }
  $bmp = New-Object System.Drawing.Bitmap $w, $hh
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $g.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size)
  $file = Join-Path $out ('shots\t{0:000}s.png' -f [int]$sw.Elapsed.TotalSeconds)
  $bmp.Save($file, [System.Drawing.Imaging.ImageFormat]::Png)
  $g.Dispose(); $bmp.Dispose(); $n++
}
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false; "stopped game pid $($p.Id) after $Seconds s, $n captures" | Add-Content (Join-Path $out 'capture.log') }
Get-Content (Join-Path $out 'capture.log') -ErrorAction SilentlyContinue
