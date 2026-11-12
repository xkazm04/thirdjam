# Lane A49: drive a packaged Development game through the REAL input path (user32 SendInput: the OS cursor, mouse buttons, wheel and keys reach the
# window as they would from the operator's hand), then stop only the process this script started. Pair with a49-analyze.mjs on the log.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a49-prove.ps1 -Exe <Development ThirdJam.exe> -Name bindings -Seconds 75 `
#     -GameArgs '-dpcvars=jam.ctrl.Log=1,jam.camera.Log=1'
#   ... -Name drill -Seconds 120 -GameArgs '-dpcvars=jam.ctrl.Drill=1' -NoSequence       (the scripted tackle drill; the mouse / keys stay idle)
# Output: tools\qa\out\ctrl\<Name>\game.log (the game's -abslog), events.log (what was sent, wall-clock stamped), game.json.
# Sequence (seconds after the match key): the mouse circles the window centre while W is held (the aim sweeps every direction around the skater, the camera
# follows a moving skater), then every binding once: LMB hold 1 s (shoot), RMB (check), MMB (cross-check), mouse 4, wheel up and down, Q, Space, E, 1 2 3 4,
# Shift held 4 s with W (sprint), Ctrl 1 s (brake), A S D taps, Esc (pause) and Esc (resume); the circle continues to the end for the 20 s camera strip.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string]$Name = 'run',
    [int]$Seconds = 75,
    [string]$GameArgs = '',
    [int]$MenuWait = 14,
    [int]$MatchWait = 8,
    [switch]$NoSequence,
    [int]$ResX = 1280, [int]$ResY = 720, [int]$WinX = 40, [int]$WinY = 40
)
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct RECT { public int Left, Top, Right, Bottom; }
public struct PT { public int X, Y; }
[StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx, dy; public uint mouseData, dwFlags, time; public IntPtr dwExtraInfo; }
[StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort wVk, wScan; public uint dwFlags, time; public IntPtr dwExtraInfo; }
[StructLayout(LayoutKind.Explicit)] public struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
[StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public INPUTUNION u; }
public static class W32 {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref PT p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  // The game's Slate window (class UnrealWindow), not the -log console window the process also owns.
  public static IntPtr FindGameWindow(uint pid) {
    IntPtr found = IntPtr.Zero; int best = 0;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid || !IsWindowVisible(h)) return true;
      var sb = new System.Text.StringBuilder(64); GetClassName(h, sb, 64);
      if (sb.ToString() != "UnrealWindow") return true;
      RECT r; GetClientRect(h, out r); int area = (r.Right - r.Left) * (r.Bottom - r.Top);
      if (area > best) { best = area; found = h; }
      return true; }, IntPtr.Zero);
    return found;
  }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint mapType);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
  public static void Mouse(uint flags, int dx, int dy, uint data) {
    INPUT[] a = new INPUT[1]; a[0].type = 0; a[0].u.mi.dx = dx; a[0].u.mi.dy = dy; a[0].u.mi.mouseData = data; a[0].u.mi.dwFlags = flags;
    SendInput(1, a, Marshal.SizeOf(typeof(INPUT)));
  }
  public static void Key(ushort vk, bool up) {
    INPUT[] a = new INPUT[1]; a[0].type = 1; a[0].u.ki.wVk = vk; a[0].u.ki.wScan = (ushort)MapVirtualKey(vk, 0); a[0].u.ki.dwFlags = up ? 2u : 0u;
    SendInput(1, a, Marshal.SizeOf(typeof(INPUT)));
  }
  public static void MoveAbs(int sx, int sy) {
    int w = GetSystemMetrics(0), h = GetSystemMetrics(1);
    Mouse(0x8001, (int)Math.Round(sx * 65535.0 / (w - 1)), (int)Math.Round(sy * 65535.0 / (h - 1)), 0);
  }
}
"@
function Focus([IntPtr]$hw) {
    # A background process may not steal the foreground: the Alt tap is the documented way to be allowed to; then verify.
    [W32]::Key(0x12, $false); Start-Sleep -Milliseconds 30; [W32]::Key(0x12, $true); Start-Sleep -Milliseconds 30
    [W32]::SetForegroundWindow($hw) | Out-Null; Start-Sleep -Milliseconds 150
    return ([W32]::GetForegroundWindow() -eq $hw)
}
function ClientCentre([IntPtr]$hw) {
    $r = New-Object RECT; [W32]::GetClientRect($hw, [ref]$r) | Out-Null
    $o = New-Object PT; $o.X = 0; $o.Y = 0; [W32]::ClientToScreen($hw, [ref]$o) | Out-Null
    return @(($o.X + ($r.Right - $r.Left) / 2), ($o.Y + ($r.Bottom - $r.Top) / 2), ($r.Right - $r.Left), ($r.Bottom - $r.Top), $o.X, $o.Y)
}
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $root "tools\qa\out\ctrl\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$evLog = Join-Path $out 'events.log'
$gameLog = Join-Path $out 'game.log'
if (Test-Path $gameLog) { Remove-Item $gameLog -Force }
function Note([string]$s) { "$(Get-Date -Format 'HH:mm:ss.fff') $s" | Add-Content $evLog; Write-Host $s }
$a = "$GameArgs -windowed -ResX=$ResX -ResY=$ResY -WinX=$WinX -WinY=$WinY -log -abslog=`"$gameLog`""
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; lane = 'A49' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
Note "start pid $($p.Id) $Exe $a"

# Window and client rectangle (screen coordinates).
$h = [IntPtr]::Zero
$sw = [Diagnostics.Stopwatch]::StartNew()
while ($sw.Elapsed.TotalSeconds -lt 60 -and $h -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 250; $p.Refresh(); if ($p.HasExited) { break }; $h = [W32]::FindGameWindow([uint32]$p.Id) }
if ($h -eq [IntPtr]::Zero) { Note 'FAIL no UnrealWindow'; exit 1 }
Start-Sleep -Seconds 2
$h2 = [W32]::FindGameWindow([uint32]$p.Id); if ($h2 -ne [IntPtr]::Zero) { $h = $h2 }
Note "game window (class UnrealWindow) $h; MainWindowHandle (the -log console) $($p.MainWindowHandle)"
$cc = ClientCentre $h
Note "window $h client $($cc[2])x$($cc[3]) at $($cc[4]),$($cc[5]) centre $($cc[0]),$($cc[1]) (before the menu)"

# Menu: wait, then ENTER on the first button (PLAY THE SEASON, A48's path), then the match load.
Start-Sleep -Seconds $MenuWait
$fg = Focus $h
Note "foreground before ENTER: $fg"
[W32]::Key(0x0D, $false); Start-Sleep -Milliseconds 60; [W32]::Key(0x0D, $true)
Note 'key ENTER (start the match)'
Start-Sleep -Seconds $MatchWait
$fg = Focus $h
$cc = ClientCentre $h
$cx = $cc[0]; $cy = $cc[1]
Note "match: foreground $fg; client $($cc[2])x$($cc[3]) at $($cc[4]),$($cc[5]) centre $cx,$cy"
[W32]::MoveAbs([int]$cx, [int]($cy - 120))
$t0 = $sw.Elapsed.TotalSeconds
Note "match sequence starts at $([math]::Round($t0,1)) s"

$circleRadius = 180.0
$circlePeriod = 3.0
function Circle([double]$t) { $ang = 2 * [Math]::PI * ($t / $circlePeriod); [W32]::MoveAbs([int]($cx + $circleRadius * [Math]::Cos($ang)), [int]($cy - 120 + $circleRadius * [Math]::Sin($ang))) }
function Tap([int]$vk, [string]$label) { [W32]::Key($vk, $false); Start-Sleep -Milliseconds 70; [W32]::Key($vk, $true); Note "key $label" }
function Click([uint32]$down, [uint32]$up, [uint32]$data, [string]$label, [int]$holdMs = 80) { [W32]::Mouse($down, 0, 0, $data); Start-Sleep -Milliseconds $holdMs; [W32]::Mouse($up, 0, 0, $data); Note "mouse $label" }

# The schedule: a list of (at seconds, action). Between actions the mouse circles and W stays held.
$sched = @()
if (-not $NoSequence) {
    $sched += ,@(1.0, { [W32]::Key(0x57, $false); Note 'key W down (skate)' })
    $sched += ,@(12.0, { Click 0x0002 0x0004 0 'LMB hold 1 s (shoot: charge, release)' 1000 })
    $sched += ,@(14.0, { Click 0x0008 0x0010 0 'RMB (check)' })
    $sched += ,@(15.5, { Click 0x0020 0x0040 0 'MMB (cross-check)' })
    $sched += ,@(17.0, { Click 0x0080 0x0100 1 'mouse 4 (cross-check)' })
    $sched += ,@(18.5, { [W32]::Mouse(0x0800, 0, 0, 120); Note 'mouse wheel up (deke)' })
    $sched += ,@(20.0, { [W32]::Mouse(0x0800, 0, 0, [uint32]4294967176); Note 'mouse wheel down (deke)' })
    $sched += ,@(21.5, { Tap 0x51 'Q (deke)' })
    $sched += ,@(23.0, { Tap 0x20 'SPACE (poke)' })
    $sched += ,@(24.5, { Tap 0x45 'E (pass)' })
    $sched += ,@(26.0, { Tap 0x31 '1 (signal)' })
    $sched += ,@(27.0, { Tap 0x32 '2 (signal)' })
    $sched += ,@(28.0, { Tap 0x33 '3 (signal)' })
    $sched += ,@(29.0, { Tap 0x34 '4 (signal)' })
    $sched += ,@(30.5, { [W32]::Key(0xA0, $false); Note 'key LSHIFT down (sprint)' })
    $sched += ,@(34.5, { [W32]::Key(0xA0, $true); Note 'key LSHIFT up' })
    $sched += ,@(36.0, { [W32]::Key(0xA2, $false); Note 'key LCTRL down (brake)' })
    $sched += ,@(37.0, { [W32]::Key(0xA2, $true); Note 'key LCTRL up' })
    $sched += ,@(38.0, { Tap 0x41 'A' })
    $sched += ,@(39.0, { Tap 0x53 'S' })
    $sched += ,@(40.0, { Tap 0x44 'D' })
    $sched += ,@(42.0, { Tap 0x1B 'ESC (pause)' })
    $sched += ,@(44.5, { Tap 0x1B 'ESC (resume)' })
    $sched += ,@(46.0, { $f = Focus $h; Note "foreground again: $f" })
}
$i = 0
$wDown = -not $NoSequence
while ($sw.Elapsed.TotalSeconds - $t0 -lt $Seconds) {
    Start-Sleep -Milliseconds 33
    $p.Refresh(); if ($p.HasExited) { Note "game exited code $($p.ExitCode)"; break }
    $t = $sw.Elapsed.TotalSeconds - $t0
    if (-not $NoSequence) { Circle $t }
    while ($i -lt $sched.Count -and $t -ge $sched[$i][0]) { & $sched[$i][1]; $i++ }
}
if ($wDown) { [W32]::Key(0x57, $true); Note 'key W up' }
if (-not $p.HasExited) {
    # A clean close first (WM_CLOSE: the engine flushes its log; a forced kill loses the buffered tail), then stop only the tree this script started.
    $kids = @(Get-CimInstance Win32_Process -Filter "ParentProcessId=$($p.Id)" -ErrorAction SilentlyContinue)
    [void][W32]::PostMessage($h, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)   # WM_CLOSE to the game window
    if (-not $p.WaitForExit(20000)) { Stop-Process -Id $p.Id -Force -ErrorAction SilentlyContinue; Note 'forced stop' }
    foreach ($k in $kids) { Stop-Process -Id $k.ProcessId -Force -ErrorAction SilentlyContinue }
    Note "stopped pid $($p.Id)"
}
Start-Sleep -Seconds 2
# The Development exe ignores -abslog here and writes <stage>\ThirdJam\Saved\Logs\ThirdJam.log: copy that when -abslog left nothing.
if (-not (Test-Path $gameLog)) {
    $saved = Join-Path (Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent) 'Saved\Logs\ThirdJam.log'
    if (Test-Path $saved) { Copy-Item $saved $gameLog -Force; Note "copied $saved" }
}
Note "done; log $gameLog"
