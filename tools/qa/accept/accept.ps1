# Lane A87 (R-0043 point 1): the acceptance run. Plays the first minutes of a match in a packaged Development game through the real input path and says pass or
# fail for each verb the player has (docs/experiments/ACCEPT.md). The game runs jam.ctrl.Drill 20 (Game/Source/JamGame/Private/HokejAcceptDrill.cpp): the drill
# stages each step and logs JAMACCEPT cursor / act lines; this driver is the hand, the way A77 / A82 drove their drills without the foreground:
#   the cursor: SetCursorPos + a posted WM_MOUSEMOVE (Slate reads it; the blade aims at the cursor on the ice);
#   keys (W, Shift, E, Q / F, Esc): posted WM_KEYDOWN / WM_CHAR / WM_KEYUP; buttons: posted WM_[LRM]BUTTONDOWN / UP; the wheel: a posted WM_MOUSEWHEEL.
# The pass key is read live: the driver taps Q, then F, and keeps the one that raised a `JAMCTRL ... btn=pass held=1` line (A84 moves the pass to Q).
# PLAY is one Return posted to the window; the end is `quit` over the remote console. Stops only the process it started. Then accept.mjs judges.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\accept.ps1 -Package Game\Packaged\A87-dev
#   -Cut <cartridge> plays another cut (accept-cut-reach165.json: the control with R-0042's carrier reach 1.65 as shot.reach_m); -From / -To run a part.
# Output: tools\qa\out\accept\<Name>\game.log, events.log, run.json, verdict.txt; the last line is accept.mjs's thirdjam.accept/1 JSON. Exit 0 pass, 1 fail, 2 could not run.
param(
    [string]$Package = '',
    [string]$Exe = '',
    [string]$Name = 'accept',
    [string]$Cut = '',
    [int]$From = 1,
    [int]$To = 10,
    [int]$Seconds = 420,
    [int]$Port = 4661,
    [int]$ResX = 1280, [int]$ResY = 720
)
$ErrorActionPreference = 'Continue'
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
if (-not $Exe) {
    if (-not $Package) { Write-Host 'FAIL give -Package <packaged dir> or -Exe <ThirdJam.exe>'; exit 2 }
    $cand = Get-ChildItem -Path $Package -Recurse -Filter 'ThirdJam*.exe' -ErrorAction SilentlyContinue | Where-Object { $_.FullName -match 'Binaries\\Win64' } | Select-Object -First 1
    if (-not $cand) { Write-Host "FAIL no ThirdJam*.exe under $Package\...\Binaries\Win64"; exit 2 }
    $Exe = $cand.FullName
}
$Exe = (Resolve-Path $Exe).Path
if (-not $Cut) { $Cut = Join-Path $here 'accept-cut.json' }
$Cut = (Resolve-Path $Cut).Path -replace '\\', '/'
$out = Join-Path $root "tools\qa\out\accept\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$ev = Join-Path $out 'events.log'
$log = Join-Path $out 'game.log'
Remove-Item $log, $ev -ErrorAction SilentlyContinue
$sw = [Diagnostics.Stopwatch]::StartNew()
function Note([string]$s) { $l = "{0} t={1:0.000} {2}" -f (Get-Date -Format 'HH:mm:ss.fff'), $sw.Elapsed.TotalSeconds, $s; Add-Content $ev $l; Write-Host $l }
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct A87R { public int Left, Top, Right, Bottom; }
public struct A87P { public int X, Y; }
public static class A87W {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint type);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out A87R r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref A87P p);
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
      A87R r; GetClientRect(h, out r); int area = (r.Right - r.Left) * (r.Bottom - r.Top);
      if (area > best) { best = area; found = h; }
      return true; }, IntPtr.Zero);
    return found;
  }
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void KeyDown(IntPtr h, int vk, int ch) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0100, (IntPtr)vk, L(scan, false)); if (ch != 0) PostMessage(h, 0x0102, (IntPtr)ch, L(scan, false)); }
  public static void KeyUp(IntPtr h, int vk) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0101, (IntPtr)vk, L(scan, true)); }
  public static void Key(IntPtr h, int vk, int ch, int holdMs) { KeyDown(h, vk, ch); System.Threading.Thread.Sleep(holdMs); KeyUp(h, vk); }
  static IntPtr XY(int cx, int cy) { return (IntPtr)(long)((uint)(cy & 0xFFFF) << 16 | (uint)(cx & 0xFFFF)); }
  public static void Cursor(IntPtr h, int cx, int cy) { A87P p; p.X = cx; p.Y = cy; ClientToScreen(h, ref p); SetCursorPos(p.X, p.Y); PostMessage(h, 0x0200, IntPtr.Zero, XY(cx, cy)); }
  // which: 0 left, 1 right, 2 middle
  public static void Button(IntPtr h, int which, int cx, int cy, bool down) {
    uint[] dn = { 0x0201u, 0x0204u, 0x0207u }; uint[] up = { 0x0202u, 0x0205u, 0x0208u }; int[] mk = { 1, 2, 16 };
    PostMessage(h, down ? dn[which] : up[which], (IntPtr)(down ? mk[which] : 0), XY(cx, cy)); }
  [DllImport("user32.dll")] static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("kernel32.dll")] static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] static extern bool GetKeyboardState(byte[] k);
  [DllImport("user32.dll")] static extern bool SetKeyboardState(byte[] k);
  // UE polls GetKeyState(VK_LSHIFT) every frame and sends itself a Shift up when the key state says up (FWindowsApplication's shift-up check), so a posted
  // Shift lasts one frame. While a posted Shift is held, this driver attaches to the game's input thread and marks Shift down in the shared key state
  // (SetKeyboardState), the state a real key press leaves; on the release it clears it and detaches. Nothing else in the input path changes.
  static uint attachedTo = 0;
  public static bool ShiftState(IntPtr h, bool down) {
    uint pid; uint tid = GetWindowThreadProcessId(h, out pid); uint me = GetCurrentThreadId();
    if (down && attachedTo == 0) { if (!AttachThreadInput(me, tid, true)) return false; attachedTo = tid; }
    byte[] k = new byte[256]; GetKeyboardState(k);
    byte v = (byte)(down ? 0x80 : 0); k[0x10] = v; k[0xA0] = v;
    bool ok = SetKeyboardState(k);
    if (!down && attachedTo != 0) { AttachThreadInput(me, attachedTo, false); attachedTo = 0; }
    return ok;
  }
  public static void Wheel(IntPtr h, int cx, int cy, int delta) {
    A87P p; p.X = cx; p.Y = cy; ClientToScreen(h, ref p);
    PostMessage(h, 0x020A, (IntPtr)(long)((uint)(delta & 0xFFFF) << 16), (IntPtr)(long)((uint)(p.Y & 0xFFFF) << 16 | (uint)(p.X & 0xFFFF))); }
}
"@
$vk = @{ W = 0x57; SHIFT = 0x10; E = 0x45; Q = 0x51; F = 0x46; ESC = 0x1B }
$ch = @{ W = 119; SHIFT = 0; E = 101; Q = 113; F = 102; ESC = 27 }

$a = "-cut=`"$Cut`" -dpcvars=jam.ctrl.Drill=20,jam.ctrl.Log=0,jam.accept.From=$From,jam.accept.To=$To -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40 -log -abslog=`"$log`" -ForceLogFlush -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
# The OS cursor is one per machine: another lane's game driven by SetCursorPos moves this run's cursor too (run control-reach165 lost 3 of 4 aim targets so).
function Others { @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -like 'ThirdJam*' -and $_.Id -ne $p.Id } | ForEach-Object { "$($_.Id):$($_.Path)" }) }
$p = $null
$o = Others; if ($o.Count) { Note "WARN other ThirdJam processes at start (shared OS cursor): $($o -join ' ')" }
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; lane = 'A87'; from = $From; to = $To } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
Note "start pid $($p.Id) $a"

$stage = Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent
$savedLog = Join-Path $stage 'Saved\Logs\ThirdJam.log'
$script:logPick = ''
function LogFile {
    if ($script:logPick) { return $script:logPick }
    if (Test-Path $log) { $script:logPick = $log; return $log }
    if ($sw.Elapsed.TotalSeconds -gt 20 -and (Test-Path $savedLog) -and (Get-Item $savedLog).LastWriteTime -ge $start) { $script:logPick = $savedLog; return $savedLog }
    return ''
}
# The log, read incrementally (the tail since the last read).
# One offset per file (the -abslog file, or the stage's SavedLogsThirdJam.log when the game did not open the -abslog file: seen once, run control-reach165);
# a file that shrinks is never re-read from its start (a run once acted twice on every line that way).
$script:pos = @{}
$script:all = New-Object System.Text.StringBuilder
function ReadNew {
    $f = LogFile; if (-not $f) { return '' }
    if (-not $script:pos.ContainsKey($f)) { $script:pos[$f] = 0L; if ($f -ne $log) { Note "NOTE the game did not open -abslog; tailing $f" } }
    try {
        $fs = [IO.File]::Open($f, 'Open', 'Read', 'ReadWrite')
        if ($fs.Length -le $script:pos[$f]) { $fs.Close(); return '' }   # the offset never moves back (a Saved\Logs file's length was seen to step back: lines re-read)
        $null = $fs.Seek($script:pos[$f], 'Begin'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $script:pos[$f] = $fs.Position; $sr.Close()
        $null = $script:all.Append($t); return $t
    } catch { return '' }
}
function WaitLog([string]$pat, [int]$sec, [int]$fromLen = 0) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) {
        if ($p.HasExited) { return $false }
        $null = ReadNew
        if ($script:all.ToString($fromLen, $script:all.Length - $fromLen).Contains($pat)) { return $true }
        Start-Sleep -Milliseconds 50
    }
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
function Finish([int]$code, [string]$why) {
    Note $why
    $o = Others; if ($o.Count) { Note "WARN other ThirdJam processes at the end (shared OS cursor): $($o -join ' ')" }
    Start-Sleep -Seconds 2
    if (-not $p.HasExited) { Console 'quit' 3 | Out-Null }
    for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { . (Join-Path $root 'tools\run\common.ps1'); [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our game process tree (quit did not exit it)' }
    else { Note "game exited, code $($p.ExitCode)" }
    Start-Sleep -Seconds 1
    if (-not (Test-Path $log) -and (Test-Path $savedLog)) { Copy-Item $savedLog $log -Force; Note "copied $savedLog" }
    if (-not (Test-Path $log)) { Write-Host 'FAIL no game log'; Write-Host '{"tool":"thirdjam.accept/1","verdict":"error","exit":2,"reason":"no game log"}'; exit 2 }
    $vt = & node (Join-Path $here 'accept.mjs') $log $ev
    $code = $LASTEXITCODE
    $vt | Out-File -FilePath (Join-Path $out 'verdict.txt') -Encoding utf8
    $vt | ForEach-Object { Write-Host $_ }
    exit $code
}

# 1. The menu, then one Return posted to the Slate window (no foreground needed).
if (WaitLog '/Game/Maps/Menu' 120) { Note 'menu map logged' } else { Note 'WARN menu map never logged' }
Start-Sleep -Seconds 5
$h = [IntPtr]::Zero
$entered = $false
for ($i = 0; $i -lt 6 -and -not $entered; $i++) {
    $h = [A87W]::FindGameWindow([uint32]$p.Id)
    if ($h -eq [IntPtr]::Zero) { Note 'no UnrealWindow yet'; Start-Sleep -Seconds 2; continue }
    $mark = $script:all.Length
    [A87W]::Key($h, 0x0D, 13, 25); Note "Return posted to window $h"
    $entered = WaitLog 'JAMMENU start match' 6 $mark
}
if (-not $entered) { Note 'menu Return did not start a match; console fallback: open /Game/Maps/Rink'; Console 'open /Game/Maps/Rink' | Out-Null }
if (-not (WaitLog 'JAMACCEPT begin' 120)) { Finish 2 'FAIL: no JAMACCEPT begin (the drill never started)' }
Note 'JAMACCEPT begin seen'
Start-Sleep -Milliseconds 500
$h = [A87W]::FindGameWindow([uint32]$p.Id)
$r = New-Object A87R; [A87W]::GetClientRect($h, [ref]$r) | Out-Null
$cw = $r.Right - $r.Left; $chh = $r.Bottom - $r.Top
$script:hx = [int]($cw / 2); $script:hy = [int]($chh / 2)
Note "client ${cw}x${chh}"
[A87W]::Cursor($h, $script:hx, $script:hy)

# 2. The act loop: tail the log and do what each JAMACCEPT line asks.
$pending = New-Object System.Collections.ArrayList   # key-ups due: @{ vk; at }
$acted = @{}                                         # act lines done (each once, keyed by the line)
$seenLines = @{}                                     # every JAMACCEPT line handled once
$script:passKey = ''
$ot = $null                                          # the one-timer's held button: @{ pressAt; releaseBy; down }
$t0 = $sw.Elapsed.TotalSeconds
$lastHome = $t0
$done = $false
$buf = ''
function Home { [A87W]::Cursor($h, $script:hx, $script:hy) }
function Hold([string]$keys, [int]$ms) {
    foreach ($k in $keys.Split('+')) {
        if ($k -eq 'SHIFT') { $okS = [A87W]::ShiftState($h, $true); Note "shift key state held via the game's input thread: $okS" }
        [A87W]::KeyDown($h, $vk[$k], $ch[$k]); [void]$pending.Add(@{ vk = $vk[$k]; at = $sw.Elapsed.TotalMilliseconds + $ms })
    }
}
function ProbePass {
    foreach ($k in @('Q', 'F')) {
        $mark = $script:all.Length
        [A87W]::Key($h, $vk[$k], $ch[$k], 60)
        if (WaitLog 'btn=pass held=1' 1 $mark) { $script:passKey = $k; Note "pass key resolved: $k (JAMCTRL btn=pass held=1 after $k)"; return }
        Note "pass probe: $k raised no JAMCTRL btn=pass line"
        Start-Sleep -Milliseconds 300
    }
    $script:passKey = 'F'; Note 'pass key unresolved: neither Q nor F raised JAMCTRL btn=pass; using F'
}
function Sweep([int]$lx, [int]$ly, [int]$rx, [int]$ry, [int]$n) {
    for ($c = 0; $c -lt $n; $c++) {
        foreach ($leg in @(@($lx, $ly, $rx, $ry), @($rx, $ry, $lx, $ly))) {
            for ($s = 0; $s -le 10; $s++) { $x = [int]($leg[0] + ($leg[2] - $leg[0]) * $s / 10); $y = [int]($leg[1] + ($leg[3] - $leg[1]) * $s / 10); [A87W]::Cursor($h, $x, $y); Start-Sleep -Milliseconds 16 }
            Start-Sleep -Milliseconds 40
        }
    }
}
function Num([string]$line, [string]$key, [int]$def = 0) { if ($line -match "\b$key=(-?\d+)") { return [int]$Matches[1] }; return $def }
while ($sw.Elapsed.TotalSeconds - $t0 -lt $Seconds) {
    if ($p.HasExited) { Note 'game exited during the run'; break }
    $nowMs = $sw.Elapsed.TotalMilliseconds
    for ($i = $pending.Count - 1; $i -ge 0; $i--) { if ($nowMs -ge $pending[$i].at) { [A87W]::KeyUp($h, $pending[$i].vk); if ($pending[$i].vk -eq 0x10) { [void][A87W]::ShiftState($h, $false) }; $pending.RemoveAt($i) } }
    if ($ot) {
        if (-not $ot.down -and $nowMs -ge $ot.pressAt) { [A87W]::Button($h, 0, $script:hx, $script:hy, $true); $ot.down = $true; $ot.downAt = $nowMs; Note 'one-timer: left button down' }
        if ($ot.down -and $nowMs -ge $ot.releaseBy) { [A87W]::Button($h, 0, $script:hx, $script:hy, $false); Note 'one-timer: left button up (time box, no reception line)'; $ot = $null }
    }
    $new = ReadNew
    if ($new) {
        $buf += $new
        $cut = $buf.LastIndexOf("`n")
        if ($cut -ge 0) {
            $lines = $buf.Substring(0, $cut).Split("`n"); $buf = $buf.Substring($cut + 1)
            foreach ($line in $lines) {
                if ($line -notmatch 'JAMACCEPT (\S+)(.*)$') { continue }
                $lk = $line.Trim(); if ($seenLines.ContainsKey($lk)) { continue }; $seenLines[$lk] = 1
                $kind = $Matches[1]; $rest = $Matches[2].Trim()
                if ($kind -like 'step=*' -or $kind -eq 'begin' -or $kind -eq 'stage' -or $kind -eq 'wait' -or $kind -eq 'resumed') { Note "game: JAMACCEPT $kind $rest"; continue }
                if ($kind -eq 'summary') { Note "game: JAMACCEPT summary $rest"; $done = $true; break }
                if ($kind -eq 'cursor') {
                    $sx = Num $rest 'sx' -1; $sy = Num $rest 'sy' -1
                    if ($sx -ge 0 -and $sy -ge 0 -and $sx -lt $cw -and $sy -lt $chh) { $script:hx = $sx; $script:hy = $sy; Home; Start-Sleep -Milliseconds 30; Home; Note "cursor -> ($sx,$sy) [$rest]" }
                    else { Note "cursor target off the client area, not moved [$rest]" }
                    continue
                }
                if ($kind -eq 'received') { if ($ot -and $ot.down) { [A87W]::Button($h, 0, $script:hx, $script:hy, $false); Note "one-timer: left button up on the reception line [$rest]"; $ot = $null }; continue }
                if ($kind -ne 'act') { continue }
                if ($acted.ContainsKey($rest)) { continue }; $acted[$rest] = 1
                if ($rest -notmatch 'act=(\w+)') { continue }
                $what = $Matches[1]
                switch ($what) {
                    'keys' { $keys = 'W'; if ($rest -match 'keys=([\w+]+)') { $keys = $Matches[1] }; $ms = Num $rest 'ms' 2000; Hold $keys $ms; Note "act keys $keys held $ms ms [$rest]" }
                    'lmb' { $fr = Num $rest 'frames' 0; $ms = [int][Math]::Max(40, $fr * 1000.0 / 60.0 + 10); [A87W]::Button($h, 0, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds $ms; [A87W]::Button($h, 0, $script:hx, $script:hy, $false); Note "act lmb held $ms ms [$rest]" }
                    'probe_pass' { ProbePass }
                    'pass' { if (-not $script:passKey) { ProbePass }; $fr = Num $rest 'frames' 0; $ms = [int][Math]::Max(40, $fr * 1000.0 / 60.0 + 10); [A87W]::Key($h, $vk[$script:passKey], $ch[$script:passKey], $ms); Note "act pass key $($script:passKey) held $ms ms [$rest]" }
                    'rmb' { [A87W]::Button($h, 1, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds 40; [A87W]::Button($h, 1, $script:hx, $script:hy, $false); Note "act rmb [$rest]" }
                    'mmb' { [A87W]::Button($h, 2, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds 40; [A87W]::Button($h, 2, $script:hx, $script:hy, $false); Note "act mmb [$rest]" }
                    'wheel' { [A87W]::Wheel($h, $script:hx, $script:hy, 120); Note "act wheel +120 [$rest]" }
                    'key' { $k = 'E'; if ($rest -match 'key=(\w+)') { $k = $Matches[1] }; [A87W]::Key($h, $vk[$k], $ch[$k], 40); Note "act key $k [$rest]" }
                    'sweep' { $n = Num $rest 'n' 1; Sweep (Num $rest 'lx') (Num $rest 'ly') (Num $rest 'rx') (Num $rest 'ry') $n; $script:hx = Num $rest 'lx'; $script:hy = Num $rest 'ly'; Note "act sweep x$n [$rest]" }
                    'lmb_onetimer' { $eta = Num $rest 'eta_frames' 30; $now = $sw.Elapsed.TotalMilliseconds; $ot = @{ pressAt = $now + [Math]::Max(0, ($eta - 10) * 1000.0 / 60.0); releaseBy = $now + 1800; down = $false }; Note "act one-timer: press in $([int](($eta - 10) * 1000.0 / 60.0)) ms, release on the reception line [$rest]" }
                    'pause_resume' {
                        $mark = $script:all.Length
                        [A87W]::Key($h, $vk.ESC, $ch.ESC, 40); Note 'act Esc (pause)'
                        if (WaitLog 'JAMCALL paused' 3 $mark) { Note 'JAMCALL paused seen' } else { Note 'no JAMCALL paused line within 3 s' }
                        Start-Sleep -Milliseconds 1200
                        $mark = $script:all.Length
                        [A87W]::Key($h, $vk.ESC, $ch.ESC, 40); Note 'act Esc (resume)'
                        if (WaitLog 'JAMACCEPT resumed' 4 $mark) { Note 'the drill saw the resume' } else { Note 'no JAMACCEPT resumed line within 4 s (still paused?)' }
                        Home
                    }
                    'none' { Note "act none [$rest]" }
                    default { Note "unknown act $what [$rest]" }
                }
            }
        }
    }
    if ($done) { break }
    if ($sw.Elapsed.TotalSeconds - $lastHome -gt 1.0 -and -not $ot) { Home; $lastHome = $sw.Elapsed.TotalSeconds }   # a lost move message would park the blade
    Start-Sleep -Milliseconds 15
}
foreach ($k in $pending) { [A87W]::KeyUp($h, $k.vk); if ($k.vk -eq 0x10) { [void][A87W]::ShiftState($h, $false) } }
if ($done) { Finish 0 'run finished (summary seen)' } else { Finish 1 'run time box reached without a summary' }
