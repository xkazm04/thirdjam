# Lane A54: launch one game exe, run a plan (keys, console lines, named 1080p stills, stills triggered by log lines), stop only the process started here.
#   powershell -NoProfile -ExecutionPolicy Bypass -File shoot.ps1 -Exe <ThirdJam.exe> -PlanFile plan.json -OutDir <dir> [-GameArgs '...'] [-Log <abs game.log>]
# plan.json: { "seconds": 90, "steps": [ {"at":12,"keys":"{ENTER}"}, {"at":14,"console":"jam.menuscreen cuts"}, {"at":20,"shot":"faceoff"},
#   {"at":20,"down":"S"}, {"at":26,"up":"S"} ], "triggers": [ {"on":"JAMEVENT goal ","after":0.8,"shot":"goal","max":1} ] }
# "at" is seconds since launch. Keys: SendKeys text (menu); down/up: held keys through SendInput (W A S D, SHIFT, CTRL, SPACE, E, Q, ESC, ENTER, LEFT...).
# Stills: PrintWindow(PW_CLIENTONLY|PW_RENDERFULLCONTENT) of the game's own client area, <OutDir>\<shot>.png, 1920x1080 expected. Log: capture.log in OutDir.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$PlanFile,
    [Parameter(Mandatory = $true)][string]$OutDir,
    [string]$GameArgs = '',
    [string]$Log = '',
    [int]$ResX = 1920, [int]$ResY = 1080, [int]$WinX = 40, [int]$WinY = 40
)
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct RECT { public int Left, Top, Right, Bottom; }
[StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx, dy; public uint mouseData, dwFlags, time; public IntPtr dwExtraInfo; }
[StructLayout(LayoutKind.Sequential)] public struct KEYBDINPUT { public ushort wVk, wScan; public uint dwFlags, time; public IntPtr dwExtraInfo; }
[StructLayout(LayoutKind.Explicit)] public struct INPUTUNION { [FieldOffset(0)] public MOUSEINPUT mi; [FieldOffset(0)] public KEYBDINPUT ki; }
[StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public INPUTUNION u; }
public static class W32 {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
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
  [DllImport("user32.dll")] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint mapType);
  public static void Key(ushort vk, bool up) {
    INPUT[] a = new INPUT[1]; a[0].type = 1; a[0].u.ki.wVk = vk; a[0].u.ki.wScan = (ushort)MapVirtualKey(vk, 0); a[0].u.ki.dwFlags = up ? 2u : 0u;
    SendInput(1, a, Marshal.SizeOf(typeof(INPUT)));
  }
}
"@
function PostKey([IntPtr]$hw, [uint16]$k, [bool]$up) {
    $sc = [W32]::MapVirtualKey($k, 0)
    if ($up) { [W32]::PostMessage($hw, 0x101, [IntPtr]$k, [IntPtr](0xC0000001 -bor ($sc -shl 16))) | Out-Null } else { [W32]::PostMessage($hw, 0x100, [IntPtr]$k, [IntPtr](1 -bor ($sc -shl 16))) | Out-Null }
}
$vk = @{ W = 0x57; A = 0x41; S = 0x53; D = 0x44; E = 0x45; Q = 0x51; SHIFT = 0x10; CTRL = 0x11; SPACE = 0x20; ESC = 0x1B; ENTER = 0x0D; LEFT = 0x25; UP = 0x26; RIGHT = 0x27; DOWN = 0x28; ALT = 0x12 }
$script:focusFails = 0
function Focus([IntPtr]$hw) {
    # A background process may not steal the foreground: the Alt tap is the documented way to be allowed to; verify. After two failed tries
    # (a dialog of another program holds the foreground) stop trying: every try costs a second of the schedule, and PostMessage does the job.
    if ($script:focusFails -ge 2) { return $false }
    for ($i = 0; $i -lt 3; $i++) {
        [W32]::Key(0x12, $false); Start-Sleep -Milliseconds 30; [W32]::Key(0x12, $true); Start-Sleep -Milliseconds 30
        [W32]::SetForegroundWindow($hw) | Out-Null; Start-Sleep -Milliseconds 100
        if ([W32]::GetForegroundWindow() -eq $hw) { $script:focusFails = 0; return $true }
    }
    $script:focusFails++
    return $false
}
New-Item -ItemType Directory -Force $OutDir | Out-Null
$capLog = Join-Path $OutDir 'capture.log'
function Note([string]$s) { "$(Get-Date -Format 'HH:mm:ss.fff') $s" | Add-Content $capLog; Write-Host $s }
$plan = Get-Content $PlanFile -Raw | ConvertFrom-Json
$a = "$GameArgs -windowed -ResX=$ResX -ResY=$ResY -WinX=$WinX -WinY=$WinY"
if ($Log) { if (Test-Path $Log) { Remove-Item $Log -Force }; $a = "$a -log -abslog=`"$Log`"" }
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
Note "start pid $($p.Id) $Exe $a"
$sw = [Diagnostics.Stopwatch]::StartNew()
$h = [IntPtr]::Zero
while ($sw.Elapsed.TotalSeconds -lt 60 -and $h -eq [IntPtr]::Zero) { Start-Sleep -Milliseconds 250; $p.Refresh(); if ($p.HasExited) { break }; $h = [W32]::FindGameWindow([uint32]$p.Id) }
if ($h -eq [IntPtr]::Zero) { Note 'FAIL no UnrealWindow'; if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false }; exit 1 }
Note "game window $h after $([int]$sw.Elapsed.TotalSeconds) s"
$steps = @(); foreach ($s in $plan.steps) { $steps += [pscustomobject]@{ s = $s; done = $false } }
$trig = @(); foreach ($t in @($plan.triggers)) { if ($t) { $trig += [pscustomobject]@{ t = $t; seen = 0; fired = 0 } } }
function Shoot([string]$name) {
    $r = New-Object RECT; [W32]::GetClientRect($h, [ref]$r) | Out-Null
    $w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
    if ($w -lt 100 -or $hh -lt 100) { Note "shot $name SKIPPED client ${w}x${hh}"; return }
    $bmp = New-Object System.Drawing.Bitmap $w, $hh
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $g.GetHdc(); $ok = [W32]::PrintWindow($h, $hdc, 3); $g.ReleaseHdc($hdc)
    $file = Join-Path $OutDir "$name.png"
    $bmp.Save($file, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose()
    Note ("shot {0} {1}x{2} printwindow={3} at {4:0.0}s" -f $name, $w, $hh, $ok, $sw.Elapsed.TotalSeconds)
}
$end = [double]$plan.seconds
$logPos = 0L
$pending = @()
while ($sw.Elapsed.TotalSeconds -lt $end) {
    Start-Sleep -Milliseconds 50
    $p.Refresh()
    if ($p.HasExited) { Note "game exited at $([int]$sw.Elapsed.TotalSeconds) s code $($p.ExitCode)"; break }
    $t = $sw.Elapsed.TotalSeconds
    foreach ($st in $steps) {
        if ($st.done -or $t -lt [double]$st.s.at) { continue }
        $st.done = $true; $s = $st.s
        if ($s.shot) { Shoot ([string]$s.shot) }
        $fg = $true
        if ($s.keys -or $s.console -or $s.down -or $s.up) {
            $fg = Focus $h
            if (-not $fg) { Note ("step at {0:0.0}s: game not in the foreground (another window holds it); keys go to the game window by PostMessage" -f $t) }
        }
        if ($s.down) { if ($fg) { [W32]::Key([uint16]$vk[[string]$s.down], $false) } else { PostKey $h ([uint16]$vk[[string]$s.down]) $false }; Note ("down {0} at {1:0.0}s fg={2}" -f $s.down, $t, $fg) }
        if ($s.up) { if ($fg) { [W32]::Key([uint16]$vk[[string]$s.up], $true) } else { PostKey $h ([uint16]$vk[[string]$s.up]) $true }; Note ("up {0} at {1:0.0}s fg={2}" -f $s.up, $t, $fg) }
        if ($s.keys) {
            if ($fg) { [System.Windows.Forms.SendKeys]::SendWait([string]$s.keys) }
            else { foreach ($m in [regex]::Matches([string]$s.keys, '\{(\w+)\}')) { $k = [uint16]$vk[$m.Groups[1].Value.ToUpper()]; PostKey $h $k $false; Start-Sleep -Milliseconds 60; PostKey $h $k $true; Start-Sleep -Milliseconds 60 } }
            Note ("keys at {0:0.0}s: {1} fg={2}" -f $t, $s.keys, $fg)
        }
        if ($s.console) {
            [System.Windows.Forms.SendKeys]::SendWait('`'); Start-Sleep -Milliseconds 400
            [System.Windows.Forms.SendKeys]::SendWait(([string]$s.console) + '{ENTER}'); Start-Sleep -Milliseconds 300
            [System.Windows.Forms.SendKeys]::SendWait('`'); Note ("console at {0:0.0}s: {1}" -f $t, $s.console)
        }
    }
    # Log triggers: new lines of the game log matching a pattern schedule a still 'after' seconds later (at most 'max' per trigger).
    if ($Log -and $trig.Count -gt 0 -and (Test-Path $Log)) {
        try {
            $fs = [System.IO.File]::Open($Log, 'Open', 'Read', 'ReadWrite')
            if ($fs.Length -gt $logPos) {
                $fs.Seek($logPos, 'Begin') | Out-Null
                $buf = New-Object byte[] ($fs.Length - $logPos); $null = $fs.Read($buf, 0, $buf.Length); $logPos = $fs.Length
                $txt = [Text.Encoding]::UTF8.GetString($buf)
                foreach ($tr in $trig) {
                    $n = ([regex]::Matches($txt, [regex]::Escape([string]$tr.t.on))).Count
                    for ($i = 0; $i -lt $n; $i++) {
                        if (($tr.seen + $i) -ge [int]$tr.t.skip -and $tr.fired -lt [int]$tr.t.max) {
                            $tr.fired++
                            $nm = [string]$tr.t.shot; if ($tr.fired -gt 1) { $nm = "$nm-$($tr.fired)" }
                            $pending += [pscustomobject]@{ at = $t + [double]$tr.t.after; name = $nm }
                            Note ("trigger '{0}' seen at {1:0.0}s" -f $tr.t.on, $t)
                        }
                    }
                    $tr.seen += $n
                }
            }
            $fs.Dispose()
        } catch { }
    }
    foreach ($pe in @($pending)) { if ($t -ge $pe.at) { Shoot $pe.name; $pending = @($pending | Where-Object { $_ -ne $pe }) } }
}
foreach ($k in 'W', 'A', 'S', 'D', 'SHIFT', 'CTRL') { [W32]::Key([uint16]$vk[$k], $true) }
$p.Refresh()
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false; Note "stopped game pid $($p.Id) after $([int]$sw.Elapsed.TotalSeconds) s" } else { Note 'game gone' }
