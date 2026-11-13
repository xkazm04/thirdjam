# Lane A80 (from A73's driver): the tackling proof in a packaged Development game, driven without the foreground (a foreign dialog may hold it): the menu's PLAY is one Return
# posted to the game's Slate window (WM_KEYDOWN / WM_CHAR / WM_KEYUP, as lanes A54 / A58 did), everything else is the game's own drill (jam.ctrl.Drill 5 = the
# human checks the carrier from the side at 3-6 m/s, 6 = the human carries at the pro bots and is checked), the stills are the game's (jam.ctrl.Stills 1: a
# screenshot every 4 sim frames after a knockdown that involves the human, a board hit, a stick hit), and the end is `quit` over UE 5.9's TCP remote console
# (-RemoteConsoleIP / -RemoteConsolePort, Development only). Stops only the process it started.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a80-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 5 -Name a80-side
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a80-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 6 -Name a80-taken -HitStop 3
# Output: tools\qa\out\ctrl\<Name>\game.log, events.log, run.json, stills\a80-<kind>-<start>-f<frame>.png. Pair with a80-analyze.mjs on game.log.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [int]$Drill = 5,
    [string]$Name = '',
    [int]$Seconds = 150,
    [int]$HitStop = 3,
    [string]$Cut = '',
    [int]$ScriptHuman = 0,
    [int]$UntilFrame = 0,
    [string]$Stage = '',
    [int]$Port = 4658,
    [int]$ResX = 1280, [int]$ResY = 720
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$Exe = (Resolve-Path $Exe).Path
if (-not $Name) { $Name = "a80-drill$Drill" }
if (-not $Cut) { $Cut = Join-Path $here 'a73-cut.json' }
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
public struct A80R { public int Left, Top, Right, Bottom; }
public static class A80W {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint msg, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint code, uint type);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out A80R r);
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc f, IntPtr l);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  // The game's Slate window (class UnrealWindow), the largest one of the process: not the -log console window.
  public static IntPtr FindGameWindow(uint pid) {
    IntPtr found = IntPtr.Zero; int best = 0;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid || !IsWindowVisible(h)) return true;
      var sb = new System.Text.StringBuilder(64); GetClassName(h, sb, 64);
      if (sb.ToString() != "UnrealWindow") return true;
      A80R r; GetClientRect(h, out r); int area = (r.Right - r.Left) * (r.Bottom - r.Top);
      if (area > best) { best = area; found = h; }
      return true; }, IntPtr.Zero);
    return found;
  }
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void Key(IntPtr h, int vk, int ch) {
    uint scan = MapVirtualKey((uint)vk, 0);
    PostMessage(h, 0x0100, (IntPtr)vk, L(scan, false));
    if (ch != 0) PostMessage(h, 0x0102, (IntPtr)ch, L(scan, false));
    System.Threading.Thread.Sleep(25);
    PostMessage(h, 0x0101, (IntPtr)vk, L(scan, true)); System.Threading.Thread.Sleep(15); }
}
"@
$a = "-cut=`"$Cut`" -dpcvars=jam.ctrl.Drill=$Drill,jam.ctrl.Stills=1,jam.hitstop=$HitStop,jam.camera.Log=1,jam.vis.lowlog=1,jam.ScriptHuman=$ScriptHuman -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40 -log -abslog=`"$log`" -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; drill = $Drill; hitstop = $HitStop; lane = 'A73' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
Note "start pid $($p.Id) $a"

# The Development exe ignores -abslog (A49's note) and writes <stage>\ThirdJam\Saved\Logs\ThirdJam.log: read whichever exists.
$stage = Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent
$savedLog = Join-Path $stage 'Saved\Logs\ThirdJam.log'
function LogText {
    $f = if (Test-Path $log) { $log } elseif ((Test-Path $savedLog) -and (Get-Item $savedLog).LastWriteTime -ge $start) { $savedLog } else { return '' }
    try { $fs = [IO.File]::Open($f, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); return $t } catch { return '' }
}
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
            if ($ans -and $ans.Contains('"ok":true')) { Note "console '$cmd' OK (try $try) $ans"; return $true }
            Note "console '$cmd' answer (try $try): $ans"
        } catch { Note "console '$cmd' try ${try}: $($_.Exception.Message)" }
        Start-Sleep -Seconds 1
    }
    return $false
}

# 1. The menu, then one Return posted to the Slate window (no foreground needed).
if (WaitLog '/Game/Maps/Menu' 120) { Note 'menu map logged' } else { Note 'WARN menu map never logged' }
Start-Sleep -Seconds 5
$entered = $false
for ($i = 0; $i -lt 6 -and -not $entered; $i++) {
    $h = [A80W]::FindGameWindow([uint32]$p.Id)
    if ($h -eq [IntPtr]::Zero) { Note 'no UnrealWindow yet'; Start-Sleep -Seconds 2; continue }
    [A80W]::Key($h, 0x0D, 13); Note "Return posted to window $h"
    $entered = WaitLog 'JAMMENU start match' 6
}
if ($entered) { Note 'JAMMENU start match seen' } else { Note 'menu Return did not start a match; console fallback: open /Game/Maps/Rink'; Console 'open /Game/Maps/Rink' | Out-Null }
if (WaitLog 'JAMCART loaded' 120) { Note 'JAMCART loaded' } else { Note 'FAIL: no JAMCART loaded' }

# 2. The drill runs itself; wait for its summary or the time box.
$summary = if ($UntilFrame) { "JAMSIM frame=$UntilFrame " } elseif ($Drill -eq 6) { 'JAMDRILL kd_summary' } else { 'JAMDRILL summary' }
$t0 = $sw.Elapsed.TotalSeconds
$done = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $Seconds) {
    if ($p.HasExited) { Note 'game exited during the drill'; break }
    if ((LogText).Contains($summary)) { $done = $true; break }
    Start-Sleep -Seconds 1
}
Note ("drill {0} after {1:0} s" -f ($(if ($done) { 'summary seen' } else { 'time box reached' }), ($sw.Elapsed.TotalSeconds - $t0)))
# A few seconds more so the last still sequence completes (24 stills x 4 frames = 1.6 s) and the last get-up lands.
Start-Sleep -Seconds 4
Console 'quit' 3 | Out-Null
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
if (-not $p.HasExited) {
    . (Join-Path $root 'tools\run\common.ps1')
    [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our game process tree (quit did not exit it)'
} else { Note "game exited, code $($p.ExitCode)" }
Start-Sleep -Seconds 2
if (-not (Test-Path $log)) {
    if (Test-Path $savedLog) { Copy-Item $savedLog $log -Force; Note "copied $savedLog" }
}
# 3. The stills this run wrote.
$shots = Join-Path $stage 'Saved\Screenshots'
$dst = Join-Path $out 'stills'
New-Item -ItemType Directory -Force $dst | Out-Null
$n = 0
if (Test-Path $shots) {
    Get-ChildItem $shots -Recurse -Filter 'a73-*.png' | Where-Object { $_.LastWriteTime -ge $start } | ForEach-Object { Move-Item $_.FullName (Join-Path $dst $_.Name) -Force; $n++ }
}
Note "stills moved: $n -> $dst"
Note "done; log $log"
