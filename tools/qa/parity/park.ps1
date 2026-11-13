# Lane A57: hold the mouse cursor on the human's skater while a parity run plays. AHokejGameMode::ReadHumanInput aims the blade at the cursor (CursorAimRink);
# a cursor within 0.7 m of the skater on the ice gives a zero stick, whatever the camera does, and the camera keeps the human at the same client pixel.
# Waits for the UnrealWindow of the exe given (the one started by shoot.ps1), then every 400 ms puts the cursor on client pixel (ClientX, ClientY) until that process ends
# or -MaxSec passes. Started detached by run.mjs. Only SetCursorPos: no click, no key.
param([Parameter(Mandatory = $true)][string]$Exe, [int]$ClientX = 544, [int]$ClientY = 570, [int]$MaxSec = 130)
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct PT { public int X, Y; }
public static class A57Cur {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref PT p);
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int n);
  public static IntPtr Find(uint pid) {
    IntPtr found = IntPtr.Zero;
    EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p != pid || !IsWindowVisible(h)) return true;
      var sb = new System.Text.StringBuilder(64); GetClassName(h, sb, 64); if (sb.ToString() == "UnrealWindow") found = h; return true; }, IntPtr.Zero);
    return found;
  }
}
"@
$t0 = Get-Date
$p = $null
while (((Get-Date) - $t0).TotalSeconds -lt 60 -and -not $p) {
    $p = Get-Process | Where-Object { $_.Path -eq $Exe -and $_.StartTime -ge $t0.AddSeconds(-30) } | Select-Object -First 1
    if (-not $p) { Start-Sleep -Milliseconds 300 }
}
if (-not $p) { exit 1 }
while (((Get-Date) - $t0).TotalSeconds -lt $MaxSec) {
    $p.Refresh(); if ($p.HasExited) { break }
    $h = [A57Cur]::Find([uint32]$p.Id)
    if ($h -ne [IntPtr]::Zero) { $pt = New-Object PT; $pt.X = $ClientX; $pt.Y = $ClientY; [A57Cur]::ClientToScreen($h, [ref]$pt) | Out-Null; [A57Cur]::SetCursorPos($pt.X, $pt.Y) | Out-Null }
    Start-Sleep -Milliseconds 400
}
