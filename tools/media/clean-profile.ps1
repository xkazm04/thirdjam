# Lane A88: clean-profile test of a Shipping zip, unzipped outside the repo. Launches the root exe, finds the game process (child of the stub),
# photographs the client area with PrintWindow, posts Return to start a match from the menu, stops only the processes it started.
#   clean-profile.ps1 -Root <unzipped Windows folder> -Out <dir> [-Seconds 70] [-StartAt 12] [-GameArgs '-cut=...']
param([Parameter(Mandatory=$true)][string]$Root,[Parameter(Mandatory=$true)][string]$Out,[int]$Seconds=70,[int]$StartAt=12,[string]$GameArgs='',[double]$Every=5)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct RECT2 { public int Left, Top, Right, Bottom; }
public static class CP {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT2 r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
}
"@
New-Item -ItemType Directory -Force $Out | Out-Null
$log = Join-Path $Out 'run.log'
function Note([string]$s){ "$(Get-Date -Format 'HH:mm:ss.fff') $s" | Add-Content $log; Write-Host $s }
$stub = Start-Process -FilePath (Join-Path $Root 'ThirdJam.exe') -ArgumentList "$GameArgs -windowed -ResX=1920 -ResY=1080 -WinX=40 -WinY=40" -PassThru
Note "stub pid $($stub.Id)"
$mine = @($stub.Id); $game = $null; $sw=[Diagnostics.Stopwatch]::StartNew(); $next=0; $sent=$false; $alive=$true
while($sw.Elapsed.TotalSeconds -lt $Seconds){
  Start-Sleep -Milliseconds 200
  if(-not $game){
    $c = Get-CimInstance Win32_Process -Filter "Name='ThirdJam-Win64-Shipping.exe'" | Where-Object { $mine -contains $_.ParentProcessId } | Select-Object -First 1
    if($c){ $game = Get-Process -Id $c.ProcessId; $mine += $game.Id; Note "game pid $($game.Id) (child of stub) at $([int]$sw.Elapsed.TotalSeconds) s" }
    elseif($stub.HasExited){ Note "stub exited code $($stub.ExitCode) at $([int]$sw.Elapsed.TotalSeconds) s, no child"; }
    continue
  }
  $game.Refresh()
  if($game.HasExited){ Note "GAME EXITED at $([int]$sw.Elapsed.TotalSeconds) s code $($game.ExitCode)"; $alive=$false; break }
  $h=$game.MainWindowHandle; if($h -eq [IntPtr]::Zero){ continue }
  $t=$sw.Elapsed.TotalSeconds
  if(-not $sent -and $t -ge $StartAt){ $sent=$true; [CP]::PostMessage($h,0x100,[IntPtr]13,[IntPtr]0x001C0001)|Out-Null; Start-Sleep -Milliseconds 60; [CP]::PostMessage($h,0x101,[IntPtr]13,[IntPtr]0xC01C0001)|Out-Null; Note "Return posted at $([int]$t) s" }
  if($t -ge $next){
    $next += $Every
    $r=New-Object RECT2; [CP]::GetClientRect($h,[ref]$r)|Out-Null; $w=$r.Right-$r.Left; $hh=$r.Bottom-$r.Top
    if($w -lt 100){ continue }
    $bmp=New-Object System.Drawing.Bitmap $w,$hh; $g=[System.Drawing.Graphics]::FromImage($bmp); $hdc=$g.GetHdc(); $ok=[CP]::PrintWindow($h,$hdc,3); $g.ReleaseHdc($hdc)
    $f=Join-Path $Out ('t{0:000}s.png' -f [int]$t); $bmp.Save($f,[System.Drawing.Imaging.ImageFormat]::Png); $g.Dispose(); $bmp.Dispose()
    Note ("shot {0} {1}x{2} pw={3} ws={4:0}MB" -f (Split-Path $f -Leaf),$w,$hh,$ok,($game.WorkingSet64/1MB))
  }
}
if($alive -and $game){ Note "ALIVE at end, $([int]$sw.Elapsed.TotalSeconds) s" }
foreach($id in ($mine | Sort-Object -Descending)){ try{ Stop-Process -Id $id -Force -ErrorAction Stop; Note "stopped my pid $id" }catch{} }
