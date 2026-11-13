# Lane A51 (R-0034): the stick-handling proofs in a packaged Development game through the REAL input path (A49's user32 SendInput driver). The game runs the
# 1v1 drill (jam.ctrl.Drill 3 = sweep, 4 = spin: the skating is scripted at the nearest defender, the cursor and the keys stay real), and this script moves the
# OS cursor and presses E as the operator's hand would. Pair with a51-analyze.mjs on the log.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a51-prove.ps1 -Exe <Development ThirdJam.exe> -Mode sweep -Seconds 110
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\ctrl\a51-prove.ps1 -Exe <Development ThirdJam.exe> -Mode spin -Seconds 110
# sweep: every 0.6 s the cursor crosses the skater from one side to the other in 3 steps (about 3 frames: well above sweep.min_deg_per_frame 6) and holds;
#        the game logs JAMCTRL sweep per crossing the sim saw and JAMDRILL stick_result (kept / poked) 30 frames after each.
# spin:  E every 2.5 s (the cooldown is 1.9 s) while the cursor holds the puck forward-left; JAMCTRL spin, JAMDRILL stick_result with knock_frames when a
#        check landed mid-spin (135 = 1.5 x 90).
# Output: tools\qa\out\ctrl\a51-<Mode>\game.log (+ events.log). This script only launches the game it starts and closes only that process.
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [ValidateSet('sweep', 'spin')][string]$Mode = 'sweep',
    [int]$Seconds = 110,
    [int]$MenuWait = 14,
    [int]$MatchWait = 8,
    [int]$ResX = 1280, [int]$ResY = 720, [int]$WinX = 40, [int]$WinY = 40
)
$here = $PSScriptRoot
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$name = "a51-$Mode"
$drill = if ($Mode -eq 'spin') { 4 } else { 3 }
$outDir = Join-Path $root "tools\qa\out\ctrl\$name"
New-Item -ItemType Directory -Force $outDir | Out-Null
$gameLog = Join-Path $outDir 'game.log'
$events = Join-Path $outDir 'events.log'
Remove-Item $gameLog, $events -ErrorAction SilentlyContinue
# The driver's window / input helpers (A49): load the C# bridge from a49-prove.ps1's Add-Type block by sourcing a trimmed copy.
$a49 = Get-Content (Join-Path $here 'a49-prove.ps1') -Raw
$typeBlock = [regex]::Match($a49, 'Add-Type @"(.*?)"@', 'Singleline').Groups[1].Value
Add-Type $typeBlock
function Log([string]$m) { $line = ('{0:HH:mm:ss.fff} {1}' -f (Get-Date), $m); Add-Content $events $line; Write-Host $line }
Add-Type -Namespace W32X -Name Win -MemberDefinition '[DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd); [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);'
function Focus([IntPtr]$hw) {
    # A background process may not steal the foreground: the Alt tap is the documented way to be allowed to; restore + top + set, verify, retry five times.
    foreach ($try in 1..5) {
        [W32X.Win]::ShowWindow($hw, 9) | Out-Null
        [W32]::Key(0x12, $false); Start-Sleep -Milliseconds 30; [W32]::Key(0x12, $true); Start-Sleep -Milliseconds 30
        [W32X.Win]::BringWindowToTop($hw) | Out-Null
        [W32]::SetForegroundWindow($hw) | Out-Null; Start-Sleep -Milliseconds 300
        if ([W32]::GetForegroundWindow() -eq $hw) { return $true }
        Start-Sleep -Milliseconds 700
    }
    return $false
}
function ClientCentre([IntPtr]$hw) {
    $r = New-Object RECT; [W32]::GetClientRect($hw, [ref]$r) | Out-Null
    $o = New-Object PT; $o.X = 0; $o.Y = 0; [W32]::ClientToScreen($hw, [ref]$o) | Out-Null
    return @(($o.X + ($r.Right - $r.Left) / 2), ($o.Y + ($r.Bottom - $r.Top) / 2))
}
function Tap([int]$vk) { [W32]::Key($vk, $false); Start-Sleep -Milliseconds 40; [W32]::Key($vk, $true) }

# No -log: the console window it opens takes the foreground when the map loads (both first runs lost it a second into the match); the Development exe writes Saved\Logs\ThirdJam.log anyway.
$args = "-windowed -ResX=$ResX -ResY=$ResY -WinX=$WinX -WinY=$WinY -abslog=`"$gameLog`" -dpcvars=jam.ctrl.Log=1,jam.ctrl.Drill=$drill"
Log "launch $Exe $args"
$p = Start-Process -FilePath $Exe -ArgumentList $args -PassThru
Start-Sleep -Seconds $MenuWait
$hw = [W32]::FindGameWindow([uint32]$p.Id)
if ($hw -eq [IntPtr]::Zero) { Log 'FAIL no UnrealWindow for the process'; Stop-Process -Id $p.Id -Force; exit 2 }
if (-not (Focus $hw)) {
    # Never send keys blind (they would land in another window): close the game this script started and fail.
    Log 'FAIL could not take the foreground after 5 tries; nothing sent'
    [W32]::PostMessage($hw, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
    if (-not $p.WaitForExit(15000)) { Stop-Process -Id $p.Id -Force }
    exit 2
}
Tap 0x0D   # Enter: the film-slate menu starts the match (A48)
Log 'menu Enter'
Start-Sleep -Seconds $MatchWait
$c = ClientCentre $hw
$cx = [int]$c[0]; $cy = [int]$c[1]
$end = (Get-Date).AddSeconds($Seconds)
$side = -1
if ($Mode -eq 'sweep') {
    # Hold the puck forward-left (the cursor 150 px up-left of the skater), then every 0.6 s cross to the other side in 3 cursor steps.
    [W32]::MoveAbs($cx - 110, $cy - 100); Log 'cursor forward-left'
    while ((Get-Date) -lt $end) {
        Start-Sleep -Milliseconds 600
        if ([W32]::GetForegroundWindow() -ne $hw) { if (Focus $hw) { Log 'the game lost the foreground; taken back' } else { Log 'FAIL the game lost the foreground and could not take it back; stopping the sends'; break } }
        $side = -$side
        foreach ($k in 1..3) { [W32]::MoveAbs($cx + [int](110 * $side * $k / 3), $cy - 100); Start-Sleep -Milliseconds 16 }
        Log "sweep to side $side"
    }
}
else {
    [W32]::MoveAbs($cx - 110, $cy - 100); Log 'cursor forward-left'
    while ((Get-Date) -lt $end) {
        Start-Sleep -Milliseconds 2500
        if ([W32]::GetForegroundWindow() -ne $hw) { if (Focus $hw) { Log 'the game lost the foreground; taken back' } else { Log 'FAIL the game lost the foreground and could not take it back; stopping the sends'; break } }
        Tap 0x45   # E: the spin
        Log 'E spin'
    }
}
Log 'closing the game window (WM_CLOSE)'
[W32]::PostMessage($hw, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero) | Out-Null
if (-not $p.WaitForExit(15000)) { Log 'the game did not close on WM_CLOSE; stopping the process this script started'; Stop-Process -Id $p.Id -Force }
Start-Sleep -Seconds 2
# The Development exe ignores -abslog (A49's note) and writes <stage>\ThirdJam\Saved\Logs\ThirdJam.log: copy that when -abslog left nothing.
if (-not (Test-Path $gameLog)) {
    $saved = Join-Path (Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent) 'Saved\Logs\ThirdJam.log'
    if (Test-Path $saved) { Copy-Item $saved $gameLog -Force; Log "copied $saved" }
}
Log "done; log $gameLog"
