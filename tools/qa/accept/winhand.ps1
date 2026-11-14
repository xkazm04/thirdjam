# Lane A155: the window hand, for driver rows that play the real menu / a real window (dot-source it: . winhand.ps1).
# Posted keys and buttons plus SetCursorPos (as accept.ps1), PrintWindow stills of the client area (not affected by a window in front), the remote console, the log tail.
# Everything stops only the process it started. Stills of the hosting lobby show the OS user name and LAN addresses: keep them under tools/qa/out/.
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices; using System.Text;
public struct WHR { public int Left, Top, Right, Bottom; }
public struct WHP { public int X, Y; }
public static class WH {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint c, uint t);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out WHR r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref WHP p);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint f);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP f, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  public static IntPtr Find(uint[] pids) { IntPtr f = IntPtr.Zero; int best = 0;
    EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (Array.IndexOf(pids, p) < 0 || !IsWindowVisible(h)) return true;
      var sb = new StringBuilder(64); GetClassName(h, sb, 64); if (sb.ToString() != "UnrealWindow") return true;
      WHR r; GetClientRect(h, out r); int a = (r.Right - r.Left) * (r.Bottom - r.Top); if (a > best) { best = a; f = h; } return true; }, IntPtr.Zero); return f; }
  public static string Fg() { IntPtr h = GetForegroundWindow(); var sb = new StringBuilder(128); GetWindowText(h, sb, 128); uint p; GetWindowThreadProcessId(h, out p); return p + ":" + sb.ToString(); }
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void KeyDown(IntPtr h, int vk, int ch) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0100, (IntPtr)vk, L(scan, false)); if (ch != 0) PostMessage(h, 0x0102, (IntPtr)ch, L(scan, false)); }
  public static void KeyUp(IntPtr h, int vk) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, 0x0101, (IntPtr)vk, L(scan, true)); }
  static IntPtr XY(int cx, int cy) { return (IntPtr)(long)((uint)(cy & 0xFFFF) << 16 | (uint)(cx & 0xFFFF)); }
  public static void Cursor(IntPtr h, int cx, int cy) { WHP p; p.X = cx; p.Y = cy; ClientToScreen(h, ref p); SetCursorPos(p.X, p.Y); PostMessage(h, 0x0200, IntPtr.Zero, XY(cx, cy)); }
  public static void Button(IntPtr h, int which, int cx, int cy, bool down) {
    uint[] dn = { 0x0201u, 0x0204u, 0x0207u }; uint[] up = { 0x0202u, 0x0205u, 0x0208u }; int[] mk = { 1, 2, 16 };
    PostMessage(h, down ? dn[which] : up[which], (IntPtr)(down ? mk[which] : 0), XY(cx, cy)); }
  public static bool Shot(IntPtr h, string file) {
    WHR r; GetClientRect(h, out r); int w = r.Right - r.Left, ht = r.Bottom - r.Top; if (w < 100) return false;
    using (var bmp = new System.Drawing.Bitmap(w, ht)) using (var g = System.Drawing.Graphics.FromImage(bmp)) {
      IntPtr dc = g.GetHdc(); bool ok = PrintWindow(h, dc, 3); g.ReleaseHdc(dc); bmp.Save(file, System.Drawing.Imaging.ImageFormat.Png); return ok; } }
}
"@ -ReferencedAssemblies System.Drawing
$script:HWVK = @{ W = 0x57; A = 0x41; S = 0x53; D = 0x44; Q = 0x51; E = 0x45; F = 0x46; R = 0x52; Return = 0x0D; Esc = 0x1B; Space = 0x20; Left = 0x25; Up = 0x26; Right = 0x27; Down = 0x28 }
$script:HWCH = @{ W = 119; A = 97; S = 115; D = 100; Q = 113; E = 101; F = 102; R = 114; Return = 13; Esc = 27; Space = 32; Left = 0; Up = 0; Right = 0; Down = 0 }
$script:hwsw = [Diagnostics.Stopwatch]::StartNew()
function HW-Note([string]$s) { $l = "{0} t={1:0.0} {2}" -f (Get-Date -Format 'HH:mm:ss.fff'), $script:hwsw.Elapsed.TotalSeconds, $s; if ($script:hwev) { Add-Content $script:hwev $l }; Write-Host $l }
function HW-Others { @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.ProcessName -like 'ThirdJam*' -and $script:hwmine -notcontains $_.Id } | ForEach-Object { "$($_.Id):$($_.Path)" }) }
# HW-Start: starts the exe with the args, finds the game process (a child of the Shipping stub) and its window, opens the log tail. Returns $true when a window exists.
function HW-Start([string]$Exe, [string]$GameArgs, [string]$Out, [int]$Port = 4708, [int]$ResX = 1920, [int]$ResY = 1080, [switch]$Console, [int]$WaitSec = 90, [switch]$NoLog) {
    New-Item -ItemType Directory -Force $Out | Out-Null
    $script:hwout = $Out; $script:hwev = Join-Path $Out 'events.log'; $script:hwlog = Join-Path $Out 'game.log'; $script:hwport = $Port; $script:hwrcid = 1
    Remove-Item $script:hwev, $script:hwlog -ErrorAction SilentlyContinue
    $a = "$GameArgs -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40"
    if (-not $NoLog) { $a += " -log -abslog=`"$($script:hwlog)`" -ForceLogFlush" }
    if ($Console) { $a += " -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port" }
    $o = HW-Others; if ($o.Count) { HW-Note "WARN other ThirdJam processes at start: $($o -join ' ')" }
    $script:hwp = Start-Process -FilePath $Exe -ArgumentList $a -PassThru; $null = $script:hwp.Handle
    $script:hwmine = @($script:hwp.Id); $script:hwstart = $script:hwp.StartTime; $script:hwexe = $Exe; $script:hwt0 = $script:hwsw.Elapsed.TotalSeconds; $script:hwnote = 0
    HW-Note "start pid $($script:hwp.Id) $Exe $a"
    $script:hwpos = 0L; $script:hwtext = New-Object System.Text.StringBuilder; $script:hwh = [IntPtr]::Zero
    $t0 = $script:hwsw.Elapsed.TotalSeconds
    while ($script:hwsw.Elapsed.TotalSeconds - $t0 -lt $WaitSec) {
        Start-Sleep -Milliseconds 200
        foreach ($c in (Get-CimInstance Win32_Process -Filter "Name='ThirdJam-Win64-Shipping.exe'" -ErrorAction SilentlyContinue | Where-Object { $script:hwmine -contains $_.ParentProcessId })) { if ($script:hwmine -notcontains $c.ProcessId) { $script:hwmine += $c.ProcessId; HW-Note "game child pid $($c.ProcessId)" } }
        if ($script:hwp.HasExited -and $script:hwmine.Count -eq 1) { HW-Note "exited early code $($script:hwp.ExitCode)"; return $false }
        $h = [WH]::Find([uint32[]]$script:hwmine); if ($h -ne [IntPtr]::Zero) { $script:hwh = $h; HW-Note "window found after $([int]($script:hwsw.Elapsed.TotalSeconds - $t0)) s"; return $true }
    }
    return $false
}
function HW-Alive { foreach ($i in $script:hwmine) { if (Get-Process -Id $i -ErrorAction SilentlyContinue) { return $true } }; return $false }
function HW-ReadLog {
    $f = $script:hwlog
    # the packaged game sometimes does not open -abslog (ACCEPT.md, known quirk): from 12 s on, tail the stage's SavedLogsThirdJam.log instead
    if (-not (Test-Path $f)) {
        if ($script:hwsw.Elapsed.TotalSeconds - $script:hwt0 -gt 12) { $sv = Join-Path (Split-Path (Split-Path (Split-Path $script:hwexe -Parent) -Parent) -Parent) 'Saved\Logs\ThirdJam.log'; if ((Test-Path $sv) -and (Get-Item $sv).LastWriteTime -ge $script:hwstart) { $f = $sv; if ($script:hwnote -ne 1) { $script:hwnote = 1; HW-Note "NOTE no -abslog file; tailing $sv" } } }
        if (-not (Test-Path $f)) { return }
    }
    try { $fs = [IO.File]::Open($f, 'Open', 'Read', 'ReadWrite'); if ($fs.Length -gt $script:hwpos) { $null = $fs.Seek($script:hwpos, 'Begin'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $script:hwpos = $fs.Position; $null = $script:hwtext.Append($t); $sr.Close() } else { $fs.Close() } } catch {}
}
# HW-Wait: the matched text when the regex shows in the log (from character offset $From of everything read so far) within $Sec seconds, else $null; HW-Mark gives the offset now.
function HW-Mark { HW-ReadLog; return $script:hwtext.Length }
function HW-Wait([string]$Pat, [int]$Sec, [int]$From = 0) {
    $t0 = $script:hwsw.Elapsed.TotalSeconds
    do { HW-ReadLog; $s = $script:hwtext.ToString($From, $script:hwtext.Length - $From); $m = [regex]::Match($s, $Pat); if ($m.Success) { return $m.Value }; Start-Sleep -Milliseconds 25 } while ($script:hwsw.Elapsed.TotalSeconds - $t0 -lt $Sec -and (HW-Alive))
    return $null
}
function HW-Lines([string]$Pat, [int]$From = 0) { HW-ReadLog; return @($script:hwtext.ToString($From, $script:hwtext.Length - $From) -split "`r?`n" | Where-Object { $_ -match $Pat }) }
function HW-Key([string]$Name, [int]$HoldMs = 60) { $v = $script:HWVK[$Name]; [WH]::KeyDown($script:hwh, $v, $script:HWCH[$Name]); Start-Sleep -Milliseconds $HoldMs; [WH]::KeyUp($script:hwh, $v) }
function HW-KeyDown([string]$Name) { [WH]::KeyDown($script:hwh, $script:HWVK[$Name], $script:HWCH[$Name]) }
function HW-KeyUp([string]$Name) { [WH]::KeyUp($script:hwh, $script:HWVK[$Name]) }
function HW-Move([int]$x, [int]$y) { [WH]::Cursor($script:hwh, $x, $y) }
# a click: the cursor arrives (two moves, so Slate has its hover), then the button goes down and up
function HW-Click([int]$x, [int]$y, [int]$Which = 0, [int]$HoldMs = 70) {
    [WH]::Cursor($script:hwh, $x, $y); Start-Sleep -Milliseconds 120; [WH]::Cursor($script:hwh, $x, $y); Start-Sleep -Milliseconds 120
    [WH]::Button($script:hwh, $Which, $x, $y, $true); Start-Sleep -Milliseconds $HoldMs; [WH]::Button($script:hwh, $Which, $x, $y, $false)
}
function HW-Shot([string]$Name) { $f = Join-Path $script:hwout "$Name.png"; $ok = [WH]::Shot($script:hwh, $f); HW-Note "still $Name ok=$ok"; return $f }
function HW-Console([string]$cmd, [int]$waitSec = 10) {
    for ($try = 1; $try -le 3; $try++) {
        try {
            $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $waitSec * 1000; $c.Connect('127.0.0.1', $script:hwport)
            $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
            $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
            $script:hwrcid++; $id = $script:hwrcid
            $w.WriteLine((@{ id = $id; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
            $ans = $null; $t0 = $script:hwsw.Elapsed.TotalSeconds
            while ($script:hwsw.Elapsed.TotalSeconds - $t0 -lt $waitSec) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith("{`"id`":$id,")) { $ans = $l; break } }
            $c.Close()
            if ($ans -and $ans.Contains('"ok":true')) { HW-Note "console '$cmd' OK"; return $true }
            HW-Note "console '$cmd' answer (try $try): $ans"
        } catch { HW-Note "console '$cmd' try ${try}: $($_.Exception.Message)" }
        Start-Sleep -Seconds 1
    }
    return $false
}
# HW-Close: the window's close (WM_CLOSE posted) or, with -ByConsole, `quit` over the remote console; if the game stays, `quit`, last our own process tree. Returns a text.
function HW-Close([switch]$ByConsole, [int]$WaitSec = 40) {
    $o = HW-Others; if ($o.Count) { HW-Note "WARN other ThirdJam processes at the end: $($o -join ' ')" }
    if ($ByConsole) { [void](HW-Console 'quit' 5) } else { [WH]::PostMessage($script:hwh, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null; HW-Note 'WM_CLOSE posted' }
    $t0 = $script:hwsw.Elapsed.TotalSeconds
    while ((HW-Alive) -and $script:hwsw.Elapsed.TotalSeconds - $t0 -lt $WaitSec) { Start-Sleep -Milliseconds 300 }
    $res = ''
    if (HW-Alive) {
        if (-not $ByConsole) { [void](HW-Console 'quit' 5); $t1 = $script:hwsw.Elapsed.TotalSeconds; while ((HW-Alive) -and $script:hwsw.Elapsed.TotalSeconds - $t1 -lt 15) { Start-Sleep -Milliseconds 300 } }
        if (HW-Alive) { . 'C:\Users\kazda\kiro\thirdjam\tools\run\common.ps1'; foreach ($i in $script:hwmine) { $q = Get-Process -Id $i -ErrorAction SilentlyContinue; if ($q) { [void](Stop-OwnedTree -ProcessId $i -StartTime $q.StartTime) } }; $res = 'STOPPED-BY-US (did not exit)' }
        else { $res = 'exited after quit' }
    } else { $res = "exited by close, code $($script:hwp.ExitCode)" }
    HW-Note "close: $res"; HW-ReadLog; return $res
}
# HW-Foreground: asks Windows to make the game's window the active one (the attach-to-the-foreground-thread way). Never used while a "Windows Security" dialog holds the foreground.
Add-Type -Namespace WHF -Name N -MemberDefinition '[DllImport("user32.dll")] public static extern bool SetForegroundWindow(System.IntPtr h); [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f); [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId(); [DllImport("user32.dll")] public static extern System.IntPtr GetForegroundWindow(); [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(System.IntPtr h, out uint p); [DllImport("user32.dll")] public static extern bool ShowWindow(System.IntPtr h, int c);'
function HW-Foreground {
    $fg = [WHF.N]::GetForegroundWindow(); $p = [uint32]0; $ft = [WHF.N]::GetWindowThreadProcessId($fg, [ref]$p); $me = [WHF.N]::GetCurrentThreadId()
    $title = [WH]::Fg(); if ($title -match 'Windows Security') { HW-Note "foreground is a Windows Security dialog: not touched"; return $false }
    [void][WHF.N]::AttachThreadInput($me, $ft, $true); [void][WHF.N]::SetForegroundWindow($script:hwh); [void][WHF.N]::AttachThreadInput($me, $ft, $false)
    Start-Sleep -Milliseconds 400; $now = [WH]::Fg(); HW-Note "foreground: $title -> $now"; return ($now -match 'ThirdJam|Hokej|^\d+:\s*$' -or $now -like "$($script:hwmine -join '|')*")
}
# A155: "the frame is not black". Share of sampled pixels (every 3rd in x and y) whose luminance is above 12 of 255, and the mean luminance, of a still.
Add-Type -ReferencedAssemblies System.Drawing -TypeDefinition @"
using System; using System.Drawing; using System.Drawing.Imaging;
public static class WHPix {
  public static double[] Stats(string file) {
    using (var b = new Bitmap(file)) {
      var r = new Rectangle(0, 0, b.Width, b.Height); var d = b.LockBits(r, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
      try {
        byte[] row = new byte[d.Stride]; long n = 0, lit = 0; double sum = 0;
        for (int y = 0; y < b.Height; y += 3) {
          System.Runtime.InteropServices.Marshal.Copy(IntPtr.Add(d.Scan0, y * d.Stride), row, 0, d.Stride);
          for (int x = 0; x < b.Width; x += 3) { int i = x * 4; double l = 0.114 * row[i] + 0.587 * row[i + 1] + 0.299 * row[i + 2]; sum += l; n++; if (l > 12) lit++; }
        }
        return new double[] { (double)lit / n, sum / n };
      } finally { b.UnlockBits(d); }
    }
  }
}
"@
function HW-Pixels([string]$file) { $s = [WHPix]::Stats($file); return [pscustomobject]@{ lit = [math]::Round($s[0], 4); mean = [math]::Round($s[1], 1) } }
