# Lane A87 (R-0043 point 1): the acceptance run. Plays the first minutes of a match in a packaged Development game through the real input path and says pass or
# fail for each verb the player has (docs/experiments/ACCEPT.md). The game runs jam.ctrl.Drill 20 (Game/Source/JamGame/Private/HokejAcceptDrill.cpp): the drill
# stages each step and logs JAMACCEPT cursor / act lines; this driver is the hand, the way A77 / A82 drove their drills without the foreground:
#   the cursor: SetCursorPos + a posted WM_MOUSEMOVE (Slate reads it; the blade aims at the cursor on the ice);
#   keys (W, S, Shift, E, Q / F, Esc): posted WM_KEYDOWN / WM_CHAR / WM_KEYUP; buttons: posted WM_[LRM]BUTTONDOWN / UP; the wheel: a posted WM_MOUSEWHEEL.
# The pass key is read live: the driver taps Q, then F, and keeps the one that raised a `JAMCTRL ... btn=pass held=1` line (A84 moves the pass to Q).
# PLAY is one Return posted to the window; the end is `quit` over the remote console. Stops only the process it started. Then accept.mjs judges.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\accept.ps1 -Package Game\Packaged\A87-dev
#   -Cut <cartridge> plays another cut (accept-cut-reach165.json: the control with R-0042's carrier reach 1.65 as shot.reach_m); -From / -To run a part.
# Exit 3 = machine: the run lost to the machine (a first act more than 1 s behind its log line, a press behind its cue, a tap held far over its ask, a foreign game on the cursor), not to the game. Repeat it.
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
    [int]$TapMs = 10,
    [int]$ResX = 1280, [int]$ResY = 720,
    # Lane A183: the hand presses the bindings of keys.json beside this script (the game's defaults, R-0064 point 1: shoot and cross-check on LMB, pass and
    # check on RMB). -OldKeys gives back the table of before (pass on the probed Q / F, cross-check on MMB) for a package older than A183.
    [switch]$OldKeys
)
$ErrorActionPreference = 'Continue'
$here = $PSScriptRoot
. (Join-Path $here 'winhand.ps1')   # A155: the still and the pixel count (the frame-is-not-black row)
# A131: the driver is the hand: it must not be the slow link on a busy machine. 1 ms timer resolution (Sleep otherwise rounds to 15.6 ms) and above-normal priority.
Add-Type -Namespace A131 -Name Win -MemberDefinition '[DllImport("winmm.dll")] public static extern uint timeBeginPeriod(uint ms);'
[void][A131.Win]::timeBeginPeriod(1)
try { (Get-Process -Id $PID).PriorityClass = 'AboveNormal' } catch {}
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
# A131: inside the act loop a Note is queued (its time stamp is taken now) and written after the act: a press waited 160 ms behind the Add-Content / Write-Host of its own LAG line.
$script:defer = $false
$script:nq = New-Object System.Collections.ArrayList
function FlushNotes { if ($script:nq.Count) { Add-Content $ev @($script:nq); foreach ($l in $script:nq) { Write-Host $l }; $script:nq.Clear() } }
function Note([string]$s) { $l = "{0} t={1:0.000} {2}" -f (Get-Date -Format 'HH:mm:ss.fff'), $sw.Elapsed.TotalSeconds, $s; if ($script:defer) { [void]$script:nq.Add($l) } else { Add-Content $ev $l; Write-Host $l } }
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
# Lane A183: the key table. Jam.Controls.AcceptKeysMatchDefaults holds keys.json against the game's FJamBindings::Defaults().
$hand = @{ shoot = 'LMB'; crosscheck = 'LMB'; pass = 'RMB'; check = 'RMB'; onetimer = 'LMB' }
if (-not $OldKeys) { try { $kj = Get-Content (Join-Path $here 'keys.json') -Raw | ConvertFrom-Json; foreach ($n in @('shoot', 'crosscheck', 'pass', 'check', 'onetimer')) { if ($kj.$n) { $hand[$n] = [string]$kj.$n } } } catch {} }
else { $hand.crosscheck = 'MMB'; $hand.pass = 'KEY' }
$btnOf = @{ LMB = 0; RMB = 1; MMB = 2 }
$vk = @{ W = 0x57; S = 0x53; SHIFT = 0x10; E = 0x45; Q = 0x51; F = 0x46; ESC = 0x1B }
$ch = @{ W = 119; S = 115; SHIFT = 0; E = 101; Q = 113; F = 102; ESC = 27 }

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
$script:pend = New-Object System.Text.StringBuilder     # A131: every byte read, by anyone (WaitLog included), reaches the act loop once
$script:readTod = 0.0
$script:minOff = [double]::MaxValue                     # smallest (driver read time - log line time): the clock offset (the game logs UTC) with the least delay in it
$script:firstAct = @{}                                  # step -> $true once the first act of the step was timed
function ReadNew {
    $f = LogFile; if (-not $f) { return '' }
    if (-not $script:pos.ContainsKey($f)) { $script:pos[$f] = 0L; if ($f -ne $log) { Note "NOTE the game did not open -abslog; tailing $f" } }
    try {
        $fs = [IO.File]::Open($f, 'Open', 'Read', 'ReadWrite')
        if ($fs.Length -le $script:pos[$f]) { $fs.Close(); return '' }   # the offset never moves back (a Saved\Logs file's length was seen to step back: lines re-read)
        $null = $fs.Seek($script:pos[$f], 'Begin'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $script:pos[$f] = $fs.Position; $sr.Close()
        $null = $script:all.Append($t); $null = $script:pend.Append($t); $script:readTod = [DateTime]::Now.TimeOfDay.TotalMilliseconds; return $t
    } catch { return '' }
}
function WaitLog([string]$pat, [int]$sec, [int]$fromLen = 0) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) {
        if ($p.HasExited) { return $false }
        $null = ReadNew
        if ($script:all.ToString($fromLen, $script:all.Length - $fromLen).Contains($pat)) { return $true }
        Start-Sleep -Milliseconds 10
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
    $script:defer = $false; FlushNotes
    Note $why
    $o = Others; if ($o.Count) { Note "WARN other ThirdJam processes at the end (shared OS cursor): $($o -join ' ')" }
    # A155 (PIXEL ROW): one still of the live window before it is quit, and what share of it is not black. A packaged match drawing the HUD over a black world (H27, 14 Nov: the menu's
    # double build tore the look down) passed every verb; accept.mjs fails the run on this line when the share is under 0.20 (a lit rink reads 0.99, the black world with its HUD 0.01).
    try { if ($h -ne [IntPtr]::Zero -and -not $p.HasExited) { $script:hwh = $h; $script:hwout = $out; $png = HW-Shot 'pixel'; $px = HW-Pixels $png; Note ('PIXEL lit={0} mean={1} file={2}' -f $px.lit, $px.mean, $png) } else { Note 'PIXEL none (no window to photograph)' } } catch { Note "PIXEL none ($($_.Exception.Message))" }
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
function Home { [A87W]::Cursor($h, $script:hx, $script:hy) }
function Hold([string]$keys, [int]$ms) {
    foreach ($k in $keys.Split('+')) {
        if ($k -eq 'SHIFT') { $okS = [A87W]::ShiftState($h, $true); Note "shift key state held via the game's input thread: $okS" }
        [A87W]::KeyDown($h, $vk[$k], $ch[$k]); [void]$pending.Add(@{ vk = $vk[$k]; at = $sw.Elapsed.TotalMilliseconds + $ms })
    }
}
function ProbePass {
    # A179 (item 3): the btn=pass line can come late (0.9 s on Bruisers, H29: the old 1 s wait took F for the pass key). Each key gets 2 s, and the pair is re-probed
    # until the line arrives or 5 s have passed; events.log says which round and how late.
    $tp = $sw.Elapsed.TotalSeconds; $round = 0
    while ($true) {
        $round++
        foreach ($k in @('Q', 'F')) {
            $mark = $script:all.Length; $tk = $sw.Elapsed.TotalSeconds
            [A87W]::Key($h, $vk[$k], $ch[$k], 60)
            if (WaitLog 'btn=pass held=1' 2 $mark) { $script:passKey = $k; Note ("pass key resolved: $k (JAMCTRL btn=pass held=1 after $k, {0:0.00} s late, probe round $round)" -f ($sw.Elapsed.TotalSeconds - $tk)); return }
            Note "pass probe: $k raised no JAMCTRL btn=pass line within 2 s (round $round)"
            Start-Sleep -Milliseconds 300
        }
        if ($sw.Elapsed.TotalSeconds - $tp -ge 5 -or $p.HasExited) { break }
        Note "pass probe: no line after round $round ($([int]($sw.Elapsed.TotalSeconds - $tp)) s of 5); re-probing"
    }
    $script:passKey = 'F'; Note "pass key unresolved: neither Q nor F raised JAMCTRL btn=pass in $round round(s); using F"
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
    $null = ReadNew
    if ($script:pend.Length -gt 0) {
        $buf = $script:pend.ToString()
        $nl = $buf.LastIndexOf("`n")   # A131: not $cut: the -Cut parameter is a [string] and $cut + 1 was a concatenation, so Substring threw, the buffer never shrank and was rescanned whole every pass
        if ($nl -ge 0) {
            $lines = $buf.Substring(0, $nl).Split("`n"); $null = $script:pend.Remove(0, $nl + 1)
            $script:defer = $true
            foreach ($line in $lines) {
                if ($line -match '^\[\d{4}\.\d\d\.\d\d-(\d\d)\.(\d\d)\.(\d\d):(\d\d\d)\]') {
                    $logMs = ([int]$Matches[1] * 3600 + [int]$Matches[2] * 60 + [int]$Matches[3]) * 1000.0 + [int]$Matches[4]
                    $off = ((($script:readTod - $logMs) % 86400000.0) + 86400000.0) % 86400000.0
                    if ($off -lt $script:minOff) { $script:minOff = $off }
                    $lagNow = [Math]::Max(0.0, ((([DateTime]::Now.TimeOfDay.TotalMilliseconds - $logMs) % 86400000.0) + 86400000.0) % 86400000.0 - $script:minOff)
                }
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
                $stepN = Num $rest 'step' 0; $kN = Num $rest 'k' 0; $lagMs = [int]$lagNow
                Note "LAG step=$stepN k=$kN act=$what lag_ms=$lagMs"
                if (-not $script:firstAct.ContainsKey($stepN)) { $script:firstAct[$stepN] = 1; if ($lagMs -gt 1000) { Finish 3 "MACHINE: step $stepN first act lagged its log line by $lagMs ms (more than 1000): the driver read the log late, this run is not a verdict" } }
                switch ($what) {
                    'keys' { $keys = 'W'; if ($rest -match 'keys=([\w+]+)') { $keys = $Matches[1] }; $ms = Num $rest 'ms' 2000; Hold $keys $ms; Note "act keys $keys held $ms ms [$rest]" }
                    'lmb' { $fr = Num $rest 'frames' 0; $tap = ($fr -le 4)
                        # the full band is >= 26 sim frames and the charge stops at charge_max: hold half again as long, a sim that dropped to 48 Hz still reaches it (glass-house: 25)
                        $want = if ($tap) { $TapMs } elseif ($fr -ge 26) { [int]($fr * 1000.0 / 60.0 * 1.5 + 10) } else { [int][Math]::Max(40, $fr * 1000.0 / 60.0 + 10) }
                        $sp = [Diagnostics.Stopwatch]::StartNew()
                        [A87W]::Button($h, 0, $script:hx, $script:hy, $true)
                        if ($tap) { while ($sp.Elapsed.TotalMilliseconds -lt $want) { } } else { Start-Sleep -Milliseconds $want }   # a tap is spun, not slept: a Sleep on a busy machine ran 120 ms where 40 was asked (charge 7 against a band of 4)
                        [A87W]::Button($h, 0, $script:hx, $script:hy, $false); $held = [int]$sp.Elapsed.TotalMilliseconds
                        Note "act lmb held $held ms (wanted $want) [$rest]"; Note "HOLD step=$stepN k=$kN want_ms=$want held_ms=$held" }
                    'probe_pass' { if ($btnOf.ContainsKey($hand.pass)) { $script:passKey = $hand.pass; Note "pass key resolved: $($hand.pass) (the A183 key table: the pass is a mouse button with the puck, no probe)" } else { ProbePass } }
                    'pass' { $fr = Num $rest 'frames' 0; $want = [int][Math]::Max(40, $fr * 1000.0 / 60.0 + 10); $sp = [Diagnostics.Stopwatch]::StartNew()
                        if ($btnOf.ContainsKey($hand.pass)) { $b = $btnOf[$hand.pass]; [A87W]::Button($h, $b, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds $want; [A87W]::Button($h, $b, $script:hx, $script:hy, $false); $pk = $hand.pass }
                        else { if (-not $script:passKey) { ProbePass }; [A87W]::Key($h, $vk[$script:passKey], $ch[$script:passKey], $want); $pk = $script:passKey }
                        $held = [int]$sp.Elapsed.TotalMilliseconds; Note "act pass key $pk held $held ms (wanted $want) [$rest]"; Note "HOLD step=$stepN k=$kN want_ms=$want held_ms=$held" }
                    'rmb' { $b = $btnOf[$hand.check]; [A87W]::Button($h, $b, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds 40; [A87W]::Button($h, $b, $script:hx, $script:hy, $false); Note "act rmb (the check: $($hand.check)) [$rest]" }
                    'mmb' { $b = $btnOf[$hand.crosscheck]; [A87W]::Button($h, $b, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds 40; [A87W]::Button($h, $b, $script:hx, $script:hy, $false); Note "act mmb (the cross-check: $($hand.crosscheck)) [$rest]" }
                    'lmb_hold' { $ms = Num $rest 'ms' 2000; $b = $btnOf[$hand.crosscheck]; $sp = [Diagnostics.Stopwatch]::StartNew(); [A87W]::Button($h, $b, $script:hx, $script:hy, $true); Start-Sleep -Milliseconds $ms; [A87W]::Button($h, $b, $script:hx, $script:hy, $false); Note "act lmb_hold (the held cross-check: $($hand.crosscheck)) held $([int]$sp.Elapsed.TotalMilliseconds) ms (wanted $ms) [$rest]" }   # lane A191
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
                FlushNotes
            }
            $script:defer = $false; FlushNotes
        }
    }
    if ($done) { break }
    if ($sw.Elapsed.TotalSeconds - $lastHome -gt 1.0 -and -not $ot) { Home; $lastHome = $sw.Elapsed.TotalSeconds }   # a lost move message would park the blade
    Start-Sleep -Milliseconds 4
}
foreach ($k in $pending) { [A87W]::KeyUp($h, $k.vk); if ($k.vk -eq 0x10) { [void][A87W]::ShiftState($h, $false) } }
if ($done) { Finish 0 'run finished (summary seen)' } else { Finish 1 'run time box reached without a summary' }
