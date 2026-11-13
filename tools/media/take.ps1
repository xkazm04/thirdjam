# Lane A88: record one take from a packaged game. Launches the game exe, finds its Slate window, records the client area with ffmpeg gdigrab
# (1920x1080, 60 fps, no cursor, NVENC), posts keys and mouse buttons to the window (works while a foreign window holds the foreground), and
# photographs the client area with PrintWindow every 2 s (that still is NOT affected by a window in front: it is the proof of what the game drew).
# Every second the foreground window title is logged: a take with a foreign foreground window over the game is thrown away.
#   take.ps1 -Exe <ThirdJam.exe (root stub or Shipping exe)> -Name <take> -Seconds 60 -GameArgs '-cut=...' -Script '12|tap|Return','20|hold|W|3000','30|click|R|120'
# Script actions: tap|<key>[|ms], hold|<key>|<ms>, down|<key>, up|<key>, click|L/R/M|<ms>.  Keys: W A S D Q E F R Return Esc Space Shift Ctrl Up Down Left Right.
# Output: submission/video/takes/<Name>/{take.mp4, take.log, stills/}. Stops only the processes it started.
param([Parameter(Mandatory=$true)][string]$Exe,[Parameter(Mandatory=$true)][string]$Name,[int]$Seconds=60,[string]$GameArgs='',[string[]]$Script=@(),[int]$Fps=60,[switch]$NoRecord,[int]$RecordFrom=0,[string]$Mode='pw')
Add-Type -AssemblyName System.Drawing
Add-Type @"
using System; using System.Runtime.InteropServices; using System.Text; using System.Diagnostics; using System.Threading; using System.IO;
public struct R3 { public int Left, Top, Right, Bottom; }
public struct P3 { public int X, Y; }
public class PWRec {
  [DllImport("user32.dll")] static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint f);
  [DllImport("user32.dll")] static extern IntPtr GetDC(IntPtr h);
  [DllImport("gdi32.dll")] static extern IntPtr CreateCompatibleDC(IntPtr dc);
  [DllImport("gdi32.dll")] static extern IntPtr SelectObject(IntPtr dc, IntPtr o);
  [DllImport("gdi32.dll")] static extern IntPtr CreateDIBSection(IntPtr dc, ref BMI bmi, uint u, out IntPtr bits, IntPtr sec, uint off);
  [StructLayout(LayoutKind.Sequential)] public struct BMI { public int sz, w, h; public short pl, bc; public int comp, img, xp, yp, cu, ci; }
  Thread th; volatile bool stop; Process ff; public int Grabs, Written; public double SumMs, MaxMs;
  public void Start(IntPtr h, int w, int ht, int fps, string outFile) {
    ProcessStartInfo si = new ProcessStartInfo("ffmpeg", "-hide_banner -loglevel warning -y -f rawvideo -pix_fmt bgra -video_size " + w + "x" + ht + " -framerate " + fps + " -i - -c:v h264_nvenc -preset p5 -rc vbr -cq 17 -b:v 0 -pix_fmt yuv420p -r " + fps + " \"" + outFile + "\"");
    si.UseShellExecute = false; si.RedirectStandardInput = true; si.CreateNoWindow = true; ff = Process.Start(si);
    th = new Thread(() => {
      IntPtr sdc = GetDC(IntPtr.Zero); IntPtr mdc = CreateCompatibleDC(sdc); BMI b = new BMI(); b.sz = 40; b.w = w; b.h = -ht; b.pl = 1; b.bc = 32; IntPtr bits;
      IntPtr bm = CreateDIBSection(sdc, ref b, 0, out bits, IntPtr.Zero, 0); SelectObject(mdc, bm);
      byte[] buf = new byte[w * ht * 4]; Stream o = ff.StandardInput.BaseStream; Stopwatch sw = Stopwatch.StartNew(); long sent = 0;
      while (!stop) {
        double t0 = sw.Elapsed.TotalMilliseconds; PrintWindow(h, mdc, 3); Marshal.Copy(bits, buf, 0, buf.Length);
        double el = sw.Elapsed.TotalMilliseconds; double d = el - t0; SumMs += d; if (d > MaxMs) MaxMs = d; Grabs++;
        long due = (long)(el * fps / 1000.0) + 1 - sent; if (due < 1) due = 1; if (due > fps) due = fps;
        try { for (long i = 0; i < due; i++) { o.Write(buf, 0, buf.Length); } } catch { break; }
        sent += due; Written = (int)sent;
        double next = sent * 1000.0 / fps - sw.Elapsed.TotalMilliseconds; if (next > 1) Thread.Sleep((int)next);
      }
      try { o.Close(); } catch { }
    });
    th.Start();
  }
  public void Stop() { stop = true; if (th != null) th.Join(8000); if (ff != null) { ff.WaitForExit(30000); } }
}
public static class TK {
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern uint MapVirtualKey(uint c, uint t);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out R3 r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref P3 p);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint f);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP f, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder sb, int n);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  public static IntPtr Find(uint pid) { IntPtr f = IntPtr.Zero; int best = 0;
    EnumWindows((h, l) => { uint p; GetWindowThreadProcessId(h, out p); if (p != pid || !IsWindowVisible(h)) return true;
      var sb = new StringBuilder(64); GetClassName(h, sb, 64); if (sb.ToString() != "UnrealWindow") return true;
      R3 r; GetClientRect(h, out r); int a = (r.Right - r.Left) * (r.Bottom - r.Top); if (a > best) { best = a; f = h; } return true; }, IntPtr.Zero); return f; }
  static IntPtr L(uint scan, bool up) { uint v = 1u | (scan << 16); if (up) v |= 0xC0000000u; return (IntPtr)(long)v; }
  public static void Key(IntPtr h, int vk, bool down) { uint scan = MapVirtualKey((uint)vk, 0); PostMessage(h, down ? 0x0100u : 0x0101u, (IntPtr)vk, L(scan, !down)); }
  public static void Btn(IntPtr h, char b, bool down, int x, int y) { uint m = b=='L' ? (down?0x201u:0x202u) : b=='R' ? (down?0x204u:0x205u) : (down?0x207u:0x208u);
    int w = down ? (b=='L'?1:b=='R'?2:16) : 0; PostMessage(h, m, (IntPtr)w, (IntPtr)((y << 16) | (x & 0xFFFF))); }
}
"@
$root = Split-Path (Split-Path $PSScriptRoot -Parent) -Parent
$out = Join-Path $root "submission\video\takes\$Name"; New-Item -ItemType Directory -Force (Join-Path $out 'stills') | Out-Null
$log = Join-Path $out 'take.log'; Remove-Item $log -ErrorAction SilentlyContinue
function Note([string]$s){ "$(Get-Date -Format 'HH:mm:ss.fff') $s" | Add-Content $log; Write-Host $s }
$vk = @{W=0x57;A=0x41;S=0x53;D=0x44;Q=0x51;E=0x45;F=0x46;R=0x52;Return=0x0D;Esc=0x1B;Space=0x20;Shift=0x10;Ctrl=0x11;Up=0x26;Down=0x28;Left=0x25;Right=0x27;Tab=0x09}
$sched=@(); foreach($s in ($Script -join "," -split ",")){ $p=$s.Split('|'); $sched += [pscustomobject]@{at=[double]$p[0];kind=$p[1];arg=$p[2];ms=$(if($p.Count -gt 3){[int]$p[3]}else{80});done=$false;rel=0.0} }
$a = "$GameArgs -windowed -ResX=1920 -ResY=1080 -WinX=1500 -WinY=20"
$stub = Start-Process -FilePath $Exe -ArgumentList $a -PassThru; $mine=@($stub.Id); Note "start $Exe $a (pid $($stub.Id))"
$game=$null; $h=[IntPtr]::Zero; $sw=[Diagnostics.Stopwatch]::StartNew(); $ff=$null; $rec=$null; $t0=$null; $nextShot=0; $lastFg=-1; $foreign=0; $cx=960; $cy=540; $cw=1920; $ch=1080; $pt=$null
try {
while($true){
  Start-Sleep -Milliseconds 15
  if(-not $game){
    foreach($c2 in (Get-CimInstance Win32_Process -Filter "Name='ThirdJam-Win64-Shipping.exe'" | Where-Object { $mine -contains $_.ParentProcessId })){ if($mine -notcontains $c2.ProcessId){ $mine += $c2.ProcessId } }
    $cand = @($mine) | % { [TK]::Find([uint32]$_) } | ? { $_ -ne [IntPtr]::Zero } | Select-Object -First 1
    if($cand){ $h=$cand; $pidw=[uint32]0; [TK]::GetWindowThreadProcessId($h,[ref]$pidw)|Out-Null; $game=Get-Process -Id $pidw; Note "window found pid $pidw at $([int]$sw.Elapsed.TotalSeconds) s"; [TK]::SetForegroundWindow($h)|Out-Null; Start-Sleep -Milliseconds 500
      $r=New-Object R3; [TK]::GetClientRect($h,[ref]$r)|Out-Null; $pt=New-Object P3; [TK]::ClientToScreen($h,[ref]$pt)|Out-Null; $cw=$r.Right-$r.Left; $ch=$r.Bottom-$r.Top; Note "client $cw x $ch at screen $($pt.X),$($pt.Y)"
      $t0=[Diagnostics.Stopwatch]::StartNew() }
    elseif($sw.Elapsed.TotalSeconds -gt 90){ Note "no window in 90 s"; break }
    continue
  }
  $game.Refresh(); if($game.HasExited){ Note "GAME EXITED at $([int]$t0.Elapsed.TotalSeconds) s code $($game.ExitCode)"; break }
  $t=$t0.Elapsed.TotalSeconds
  if(-not $ff -and -not $rec -and -not $NoRecord -and $t -ge $RecordFrom){
    if($Mode -eq 'pw'){ $rec = New-Object PWRec; $rec.Start($h,$cw,$ch,$Fps,(Join-Path $out 'take.mp4')); Note "PrintWindow recorder started at $([int]$t) s (immune to windows in front)" }
    else {
    $ffargs = "-hide_banner -loglevel warning -y -f gdigrab -framerate $Fps -offset_x $($pt.X) -offset_y $($pt.Y) -video_size ${cw}x${ch} -draw_mouse 0 -i desktop -t $($Seconds - [int]$t) -c:v h264_nvenc -preset p5 -rc vbr -cq 17 -b:v 0 -pix_fmt yuv420p -r $Fps `"$(Join-Path $out 'take.mp4')`""
    $ff = Start-Process -FilePath ffmpeg -ArgumentList $ffargs -PassThru -WindowStyle Hidden -RedirectStandardError (Join-Path $out 'ffmpeg.err'); Note "ffmpeg gdigrab pid $($ff.Id) from $([int]$t) s" }
  }
  foreach($s in $sched){
    if($s.done){ continue }
    if($s.kind -eq 'holding'){ if($t -ge $s.rel){ [TK]::Key($h,$vk[$s.arg],$false); $s.done=$true; Note ("t={0:0.0} released {1}" -f $t,$s.arg) }; continue }
    if($t -ge $s.at){
      switch($s.kind){
        'tap'  { [TK]::Key($h,$vk[$s.arg],$true); Start-Sleep -Milliseconds $s.ms; [TK]::Key($h,$vk[$s.arg],$false); $s.done=$true }
        'down' { [TK]::Key($h,$vk[$s.arg],$true); $s.done=$true }
        'up'   { [TK]::Key($h,$vk[$s.arg],$false); $s.done=$true }
        'hold' { [TK]::Key($h,$vk[$s.arg],$true); $s.rel=$t+($s.ms/1000.0); $s.kind='holding' }
        'click'{ [TK]::Btn($h,[char]$s.arg[0],$true,$cx,$cy); Start-Sleep -Milliseconds $s.ms; [TK]::Btn($h,[char]$s.arg[0],$false,$cx,$cy); $s.done=$true }
      }
      Note ("t={0:0.0} {1} {2} {3}" -f $t,$s.kind,$s.arg,$s.ms)
    }
  }
  if([int]$t -ne $lastFg){ $lastFg=[int]$t; $fgh=[TK]::GetForegroundWindow(); if($fgh -ne $h){ $sb=New-Object Text.StringBuilder 128; [TK]::GetWindowText($fgh,$sb,128)|Out-Null; $foreign++; Note ("FG t={0} NOT the game: '{1}'" -f $lastFg,$sb.ToString()) } }
  if($t -ge $nextShot){ $nextShot+=2
    $bmp=New-Object System.Drawing.Bitmap $cw,$ch; $g=[System.Drawing.Graphics]::FromImage($bmp); $hdc=$g.GetHdc(); [TK]::PrintWindow($h,$hdc,3)|Out-Null; $g.ReleaseHdc($hdc)
    $bmp.Save((Join-Path $out ('stills\s{0:000}.png' -f [int]$t)),[System.Drawing.Imaging.ImageFormat]::Png); $g.Dispose(); $bmp.Dispose() }
  if($t -ge $Seconds){ break }
}
} finally {
  foreach($k in 'W','A','S','D','Shift','Ctrl'){ if($h -ne [IntPtr]::Zero){ [TK]::Key($h,$vk[$k],$false) } }
  if($rec){ $rec.Stop(); Note ("pw recorder: grabs $($rec.Grabs) frames written $($rec.Written) mean grab $([math]::Round($rec.SumMs/[math]::Max(1,$rec.Grabs),1)) ms max $([math]::Round($rec.MaxMs,1)) ms") }
  if($ff){ try{ $ff.WaitForExit(20000)|Out-Null }catch{}; if(-not $ff.HasExited){ Stop-Process -Id $ff.Id -Force } }
  Note "foreground was not the game on $foreign one-second checks of $Seconds"
  foreach($id in ($mine | Sort-Object -Descending -Unique)){ try{ Stop-Process -Id $id -Force -ErrorAction Stop; Note "stopped my pid $id" }catch{} }
}
