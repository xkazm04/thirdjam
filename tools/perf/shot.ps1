# Lane A58: one still of a window's client area (PrintWindow, PW_RENDERFULLCONTENT: works behind other windows).
param([int]$ProcessId, [string]$Out)
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct A58R { public int L, T, R, B; }
public static class A58S {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out A58R r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
}
"@
$h = (Get-Process -Id $ProcessId).MainWindowHandle
$r = New-Object A58R; [void][A58S]::GetClientRect($h, [ref]$r)
$bmp = New-Object System.Drawing.Bitmap ($r.R - $r.L), ($r.B - $r.T)
$g = [System.Drawing.Graphics]::FromImage($bmp); $hdc = $g.GetHdc(); $ok = [A58S]::PrintWindow($h, $hdc, 3); $g.ReleaseHdc($hdc)
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png); "saved $Out $($bmp.Width)x$($bmp.Height) ok=$ok"
