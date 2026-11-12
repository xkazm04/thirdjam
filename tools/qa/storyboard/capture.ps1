# Lane A48: launch a game exe (Shipping or Development), capture its client area every N seconds as 1080p PNG stills, send timed key
# sequences (menu navigation, the in-game console of a Development build), then stop only the process this script started.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\storyboard\capture.ps1 -Exe <game exe> -Name <run> -Seconds 240 -Every 2 `
#     -GameArgs '/Game/Maps/Rink?cut=hockey.regular -dpcvars=jam.ScriptHuman=1' [-Log <abslog path>] [-Keys '12|{ENTER}','30|`jam.playcut hockey.boost{ENTER}']
# Output: tools\qa\out\storyboard\<Name>\shots\t###s.png, capture.log (one line per still and key event, wall-clock stamped), game.json (pid, args).
# Keys: '<seconds>|<SendKeys string>'; a leading backtick opens the Development console (Tilde), the rest is typed and {ENTER} runs it. Shipping has no console.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [string]$Name = 'run',
    [int]$Seconds = 120,
    [double]$Every = 2,
    [string]$GameArgs = '',
    [string]$Log = '',
    [string[]]$Keys = @(),
    [int]$ResX = 1920, [int]$ResY = 1080, [int]$WinX = 40, [int]$WinY = 40
)
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @"
using System; using System.Runtime.InteropServices;
public struct RECT { public int Left, Top, Right, Bottom; }
public struct PT { public int X, Y; }
public static class W32 {
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref PT p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
}
"@
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $root "tools\qa\out\storyboard\$Name"
New-Item -ItemType Directory -Force (Join-Path $out 'shots') | Out-Null
$capLog = Join-Path $out 'capture.log'
function Note([string]$s) { "$(Get-Date -Format 'HH:mm:ss.fff') $s" | Add-Content $capLog; Write-Host $s }
$a = "$GameArgs -windowed -ResX=$ResX -ResY=$ResY -WinX=$WinX -WinY=$WinY"
if ($Log) { $a = "$a -log -abslog=`"$Log`"" }
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
@{ pid = $p.Id; start = $p.StartTime.ToString('o'); exe = $Exe; args = $a; lane = 'A48' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
Note "start pid $($p.Id) $Exe $a"
$sched = @()
foreach ($k in $Keys) { $i = $k.IndexOf('|'); if ($i -gt 0) { $sched += [pscustomobject]@{ at = [double]$k.Substring(0, $i); keys = $k.Substring($i + 1); done = $false } } }
$sw = [Diagnostics.Stopwatch]::StartNew()
$n = 0; $next = $Every; $fg = $false
while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
    Start-Sleep -Milliseconds 100
    $p.Refresh()
    if ($p.HasExited) { Note "game exited at $([int]$sw.Elapsed.TotalSeconds) s code $($p.ExitCode)"; break }
    $h = $p.MainWindowHandle
    if ($h -eq [IntPtr]::Zero) { continue }
    if (-not $fg) { [W32]::SetForegroundWindow($h) | Out-Null; $fg = $true }
    $t = $sw.Elapsed.TotalSeconds
    foreach ($s in $sched) {
        if (-not $s.done -and $t -ge $s.at) {
            $s.done = $true
            [W32]::SetForegroundWindow($h) | Out-Null
            Start-Sleep -Milliseconds 120
            # Keys go only to the game: if another window holds the foreground (the operator is at the desk), the sequence is skipped and noted.
            if ([W32]::GetForegroundWindow() -eq $h) { [System.Windows.Forms.SendKeys]::SendWait($s.keys); Note ("keys at {0:0.0}s: {1}" -f $t, $s.keys) }
            else { Note ("keys at {0:0.0}s SKIPPED (game not in the foreground): {1}" -f $t, $s.keys) }
        }
    }
    if ($t -lt $next) { continue }
    $next += $Every
    $r = New-Object RECT; [W32]::GetClientRect($h, [ref]$r) | Out-Null
    $pt = New-Object PT; [W32]::ClientToScreen($h, [ref]$pt) | Out-Null
    $w = $r.Right - $r.Left; $hh = $r.Bottom - $r.Top
    if ($w -lt 100 -or $hh -lt 100) { continue }
    $bmp = New-Object System.Drawing.Bitmap $w, $hh
    $g = [System.Drawing.Graphics]::FromImage($bmp)
    # PrintWindow with PW_CLIENTONLY | PW_RENDERFULLCONTENT reads the game's own swap chain, so a window in front of the game does not end up in the still.
    $hdc = $g.GetHdc(); $ok = [W32]::PrintWindow($h, $hdc, 3); $g.ReleaseHdc($hdc)
    $how = 'printwindow'
    if (-not $ok) { $g.CopyFromScreen($pt.X, $pt.Y, 0, 0, $bmp.Size); $how = 'screen' }
    $file = Join-Path $out ('shots\t{0:000}s.png' -f [int]$t)
    $bmp.Save($file, [System.Drawing.Imaging.ImageFormat]::Png)
    $g.Dispose(); $bmp.Dispose(); $n++
    $fgs = if ([W32]::GetForegroundWindow() -eq $h) { 'fg' } else { 'bg' }
    Note ("shot {0} {1}x{2} {3} {4}" -f (Split-Path $file -Leaf), $w, $hh, $how, $fgs)
}
$p.Refresh()
if (-not $p.HasExited) { Stop-Process -Id $p.Id -Force -Confirm:$false; Note "stopped game pid $($p.Id) after $([int]$sw.Elapsed.TotalSeconds) s, $n captures" }
else { Note "game gone, $n captures" }
