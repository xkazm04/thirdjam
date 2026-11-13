# Lane A82 (R-0040): A77's driver (tools/qa/ctrl/a77-prove.ps1) run again on the A82 package: the same posted-message input path (the cursor by SetCursorPos + WM_MOUSEMOVE,
# E / F / the left button by PostMessage, PLAY by one Return, `quit` over the remote console; nothing needs the foreground). Changes: jam.ctrl.HandPuck 1 (drills 10 / 11
# hand the puck by SetPuck), the tap's left button held 40 ms (two frames, so one sim frame surely reads it; charge 2/30 is still a tap), lane A82 in run.json.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl82-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 7 -Name a82-sweep
# Output: tools\qa\out\ctrl\<Name>\game.log, events.log, run.json, stills77-<kind>-<start>-f<frame>.png (the game's still prefix is A77's). Pair with a82-analyze.mjs.
# Lane A77 (R-0036): the mechanics proofs in a packaged Development game through the real input path, driven without the foreground (a foreign dialog may hold it, as
# lanes A54 / A58 / A73 found). The game runs jam.ctrl.Drill 7 (sweep past a defender), 8 (spin with the puck), 9 (saucer over a defender's blade), 10 (shots from
# 10 m), 11 (one-timers off a scripted teammate's pass); the skating is scripted, and the drill logs JAMCUE lines when a person would act. This driver tails the log
# (-ForceLogFlush) and acts on each cue the way the operator's hand would, into the game's Slate window only:
#   the cursor: SetCursorPos (the OS cursor, no foreground needed) + a posted WM_MOUSEMOVE so Slate reads it (the blade aims at the cursor on the ice);
#   E, F: posted WM_KEYDOWN / WM_CHAR / WM_KEYUP (F is held for the saucer: 13 frames); the left button: posted WM_LBUTTONDOWN / WM_LBUTTONUP (held for the charge).
# The menu's PLAY is one Return posted to the window; the end is `quit` over UE 5.9's TCP remote console. Stops only the process it started.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a77-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 7 -Name a77-sweep
# Output: tools\qa\out\ctrl\<Name>\game.log, events.log, run.json, stills\a77-<kind>-<start>-f<frame>.png. Pair with a77-analyze.mjs on game.log.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [int]$Drill = 7,
    [string]$Name = '',
    [int]$Seconds = 180,
    [string]$Cut = '',
    [int]$Port = 4659,
    [int]$ResX = 1280, [int]$ResY = 720,
    [int]$Stills = 2,
    [int]$HandPuck = 1
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$Exe = (Resolve-Path $Exe).Path
if (-not $Name) { $Name = "a82-drill$Drill" }
if (-not $Cut) { $Cut = Join-Path $here $(if ($Drill -ge 10) { 'a77-cut-rookie.json' } else { 'a77-cut.json' }) }
$Cut = (Resolve-Path $Cut).Path -replace '\\', '/'
$out = Join-Path $root "tools\qa\out\ctrl\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$ev = Join-Path $out 'events.log'
$log = Join-Path $out 'game.log'
Remove-Item $log, $ev -ErrorAction SilentlyContinue
$sw = [Diagnostics.Stopwatch]::StartNew()
function Note([string]$s) { $l = "{0} t={1:0.000} {2}" -f (Get-Date -Format 'HH:mm:ss.fff'), $sw.Elapsed.TotalSeconds, $s; Add-Content $ev $l; Write-Host $l }
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct A77R { public int Left, Top, Right, Bottom; }
public struct A77P { public int X, Y; }
public static class A77W {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint type);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out A77R r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref A77P p);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  public static IntPtr FindGameWindow(uint pid) {
    IntPtr found = IntPtr.Zero; int best = 0;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid || !IsWindowVisible(h)) return true;
      var sb = new System.Text.StringBuilder(64); GetClassName(h, sb, 64);
      if (sb.ToString() != "UnrealWindow") return true;
      A77R r; GetClientRect(h, out r); int area = (r.Right - r.Left) * (r.Bottom - r.Top);
      if (area > best) { best = area; found = h; }
      return true; }, IntPtr.Zero);
    return found;
  }
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void KeyDown(IntPtr h, int vk, int ch) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0100, (IntPtr)vk, L(scan, false)); if (ch != 0) PostMessage(h, 0x0102, (IntPtr)ch, L(scan, false)); }
  public static void KeyUp(IntPtr h, int vk) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0101, (IntPtr)vk, L(scan, true)); }
  public static void Key(IntPtr h, int vk, int ch, int holdMs) { KeyDown(h, vk, ch); System.Threading.Thread.Sleep(holdMs); KeyUp(h, vk); }
  // The cursor in client pixels: the OS cursor moves (Slate reads GetCursorPos on the move message), and the window is told it moved.
  public static void Cursor(IntPtr h, int cx, int cy) {
    A77P p; p.X = cx; p.Y = cy; ClientToScreen(h, ref p); SetCursorPos(p.X, p.Y);
    PostMessage(h, 0x0200, IntPtr.Zero, (IntPtr)(long)((uint)(cy & 0xFFFF) << 16 | (uint)(cx & 0xFFFF))); }
  public static void LButton(IntPtr h, int cx, int cy, bool down) {
    IntPtr lp = (IntPtr)(long)((uint)(cy & 0xFFFF) << 16 | (uint)(cx & 0xFFFF));
    PostMessage(h, down ? 0x0201u : 0x0202u, (IntPtr)(down ? 1 : 0), lp); }
}
"@
$a = "-cut=`"$Cut`" -dpcvars=jam.ctrl.Drill=$Drill,jam.ctrl.Stills=$Stills,jam.ctrl.Log=1,jam.ctrl.HandPuck=$HandPuck -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40 -log -abslog=`"$log`" -ForceLogFlush -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; drill = $Drill; lane = 'A82' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
Note "start pid $($p.Id) $a"

$stage = Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent
$savedLog = Join-Path $stage 'Saved\Logs\ThirdJam.log'
function LogFile { if (Test-Path $log) { return $log }; if ((Test-Path $savedLog) -and (Get-Item $savedLog).LastWriteTime -ge $start) { return $savedLog }; return '' }
function LogText { $f = LogFile; if (-not $f) { return '' }; try { $fs = [IO.File]::Open($f, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); return $t } catch { return '' } }
function WaitLog([string]$pat, [int]$sec) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if ((LogText).Contains($pat)) { return $true }; Start-Sleep -Milliseconds 250 }
    return $false
}
$script:rcId = 1
function Console([string]$cmd, [int]$waitSec = 10) {
    for ($try = 1; $try -le 3; $try++) {
        try {
            $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $waitSec * 1000; $c.Connect('127.0.0.1', $Port)
            $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
            $w.WriteLine('{"id":1,"op":"hello","version":1}')
            $null = $r.ReadLine()
            $script:rcId++; $id = $script:rcId
            $w.WriteLine((@{ id = $id; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
            $ans = $null; $t0 = $sw.Elapsed.TotalSeconds
            while ($sw.Elapsed.TotalSeconds - $t0 -lt $waitSec) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith("{`"id`":$id,")) { $ans = $l; break } }
            $c.Close()
            if ($ans -and $ans.Contains('"ok":true')) { Note "console '$cmd' OK (try $try)"; return $true }
            Note "console '$cmd' answer (try $try): $ans"
        } catch { Note "console '$cmd' try ${try}: $($_.Exception.Message)" }
        Start-Sleep -Seconds 1
    }
    return $false
}

# 1. The menu, then one Return posted to the Slate window (no foreground needed).
if (WaitLog '/Game/Maps/Menu' 120) { Note 'menu map logged' } else { Note 'WARN menu map never logged' }
Start-Sleep -Seconds 5
$h = [IntPtr]::Zero
$entered = $false
for ($i = 0; $i -lt 6 -and -not $entered; $i++) {
    $h = [A77W]::FindGameWindow([uint32]$p.Id)
    if ($h -eq [IntPtr]::Zero) { Note 'no UnrealWindow yet'; Start-Sleep -Seconds 2; continue }
    [A77W]::Key($h, 0x0D, 13, 25); Note "Return posted to window $h"
    $entered = WaitLog 'JAMMENU start match' 6
}
if ($entered) { Note 'JAMMENU start match seen' } else { Note 'menu Return did not start a match; console fallback: open /Game/Maps/Rink'; Console 'open /Game/Maps/Rink' | Out-Null }
if (WaitLog 'JAMCART loaded' 120) { Note 'JAMCART loaded' } else { Note 'FAIL: no JAMCART loaded' }
Start-Sleep -Seconds 3
$h = [A77W]::FindGameWindow([uint32]$p.Id)
$r = New-Object A77R; [A77W]::GetClientRect($h, [ref]$r) | Out-Null
$cx = [int](($r.Right - $r.Left) / 2); $cy = [int](($r.Bottom - $r.Top) / 2)
# The cursor's home: on the skater (the blade ahead) for the shots, the saucer and the one-timer; forward-left of it for the sweep and the spin (A51's 110 x 100 px).
$side = -1
function Home { if ($Drill -eq 7 -or $Drill -eq 8) { [A77W]::Cursor($h, $cx + 110 * $script:side, $cy - 100) } else { [A77W]::Cursor($h, $cx, $cy) } }
Home; Note "cursor home (client centre $cx,$cy; drill $Drill)"

# 2. The cue loop: tail the log, act on each JAMCUE line of this drill once.
$cues = @{ sweep = 0; spin = 0; saucer = 0; shot = 0; onetimer = 0; pass_to_mate = 0 }
$seen = 0
$t0 = $sw.Elapsed.TotalSeconds
$summary = switch ($Drill) { 7 { 'JAMDRILL stick_summary' } 8 { 'JAMDRILL stick_summary' } 9 { 'JAMDRILL saucer_summary' } 10 { 'JAMDRILL shot_summary shots=30' } 11 { 'JAMDRILL onetimer_summary shots=10' } default { 'never' } }
$done = $false
$lastHome = $sw.Elapsed.TotalSeconds
while ($sw.Elapsed.TotalSeconds - $t0 -lt $Seconds) {
    if ($p.HasExited) { Note 'game exited during the drill'; break }
    $text = LogText
    if ($text.Length -gt $seen) {
        $new = $text.Substring($seen); $seen = $text.Length
        foreach ($line in ($new -split "`n")) {
            if ($line -notmatch 'JAMCUE (\w+) frame=(\d+)(.*)$') { continue }
            $kind = $Matches[1]; $rest = $Matches[3]
            switch ($kind) {
                'sweep' { if ($Drill -ne 7) { continue }
                    # cross the cursor to the other side in three steps (about 3 frames: well over sweep.min_deg_per_frame 6) and hold there
                    $script:side = -$script:side
                    foreach ($k in 1..3) { [A77W]::Cursor($h, $cx + [int](110 * $script:side * (-1 + 2 * $k / 3)), $cy - 100); Start-Sleep -Milliseconds 16 }
                    $cues.sweep++; Note "cue sweep -> cursor crossed to side $script:side ($rest)" }
                'spin' { if ($Drill -ne 8) { continue }; [A77W]::Key($h, 0x45, 101, 30); $cues.spin++; Note "cue spin -> E ($rest)" }
                'saucer' { if ($Drill -ne 9) { continue }; [A77W]::KeyDown($h, 0x51, 113); Start-Sleep -Milliseconds 220; [A77W]::KeyUp($h, 0x51); $cues.saucer++; Note "cue saucer -> Q held 220 ms (lane A84: the pass moved to Q) ($rest)" }
                'shot' { if ($Drill -ne 10) { continue }
                    $hold = 40; if ($rest -match 'kind=half') { $hold = 135 } elseif ($rest -match 'kind=full') { $hold = 650 }
                    [A77W]::LButton($h, $cx, $cy, $true); Start-Sleep -Milliseconds $hold; [A77W]::LButton($h, $cx, $cy, $false)
                    $cues.shot++; Note "cue shot -> left button held $hold ms ($rest)" }
                'onetimer' { if ($Drill -ne 11) { continue }
                    $eta = 30; if ($rest -match 'eta_frames=(\d+)') { $eta = [int]$Matches[1] }
                    $ms = [int][Math]::Max(0, $eta * 1000.0 / 60.0 - 45)
                    Start-Sleep -Milliseconds $ms
                    [A77W]::LButton($h, $cx, $cy, $true); Start-Sleep -Milliseconds 40; [A77W]::LButton($h, $cx, $cy, $false)
                    $cues.onetimer++; Note "cue onetimer -> waited $ms ms, left button tapped ($rest)" }
                'pass_to_mate' { if ($Drill -ne 11) { continue }; [A77W]::Key($h, 0x51, 113, 30); $cues.pass_to_mate++; Note "cue pass_to_mate -> Q tapped (lane A84) ($rest)" }
            }
        }
        if ($text.Contains($summary)) { $done = $true; break }
    }
    if ($sw.Elapsed.TotalSeconds - $lastHome -gt 2.0) { Home; $lastHome = $sw.Elapsed.TotalSeconds }   # the cursor drifts nowhere, but a lost move message would park the blade
    Start-Sleep -Milliseconds 40
}
$acted = @(); foreach ($k in $cues.Keys) { if ($cues[$k] -gt 0) { $acted += "$k=$($cues[$k])" } }
$how = 'time box reached'; if ($done) { $how = 'summary seen' }
Note ("drill {0} after {1:0} s; cues acted on: {2}" -f $how, ($sw.Elapsed.TotalSeconds - $t0), ($acted -join ' '))
Start-Sleep -Seconds 4
Console 'quit' 3 | Out-Null
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
if (-not $p.HasExited) {
    . (Join-Path $root 'tools\run\common.ps1')
    [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our game process tree (quit did not exit it)'
} else { Note "game exited, code $($p.ExitCode)" }
Start-Sleep -Seconds 2
if (-not (Test-Path $log)) { if (Test-Path $savedLog) { Copy-Item $savedLog $log -Force; Note "copied $savedLog" } }
$shots = Join-Path $stage 'Saved\Screenshots'
$dst = Join-Path $out 'stills'
New-Item -ItemType Directory -Force $dst | Out-Null
$n = 0
if (Test-Path $shots) { Get-ChildItem $shots -Recurse -Filter 'a77-*.png' | Where-Object { $_.LastWriteTime -ge $start } | ForEach-Object { Move-Item $_.FullName (Join-Path $dst $_.Name) -Force; $n++ } }
Note "stills moved: $n -> $dst"
Note "done; log $log"
