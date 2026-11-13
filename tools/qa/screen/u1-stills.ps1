# Lane A111 (track U1, the screen): stills from this lane's own game process, without a window grab and without the system cursor.
# One editor-binary game process per call, rendering off screen (-RenderOffScreen: no window), on the Rink map with a five-a-side cut on one look tier,
# played by the scripted human. The game takes its own stills (with the HUD) into Game\Saved\Screenshots\WindowsEditor:
#   u1-first-<tier>.png       the first visible frame of the first match of the process (jam.screen.HoldStill 1; the fade is a cut for the proof)
#   u1-<tier>-f<frame>.png    every -EveryFrames sim frames (jam.screen.StillEvery): the sim is fixed-step and the scripted human reads only the sim,
#                             so the -Mode after and -Mode before runs show the same frames of the same match
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-stills.ps1 -Tier clay -Mode after  [-PlaySeconds 50] [-Port 4711]
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-stills.ps1 -Tier clay -Mode before
# -Mode after: this lane's screen (the defaults). -Mode before: every rule at its neutral value (the screen of before the lane).
# The stills are copied to tools\qa\screen\out\<tier>-<mode>\shots; the JAMSCREEN / JAMLOOK log lines are printed and kept in screen-lines.txt.
# Exit 0: the first-frame still and at least one sim-frame still exist. 1: missing. 2: could not run.
param(
    [ValidateSet('clay', 'night')][string]$Tier = 'clay',
    [ValidateSet('after', 'before')][string]$Mode = 'after',
    [int]$PlaySeconds = 50,
    [int]$EveryFrames = 180,
    [int]$Port = 4711,
    [int]$ResX = 1280, [int]$ResY = 720,
    [string]$ExtraCVars = '',
    [int]$StartTimeoutSec = 900
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL u1-stills: no project or no $exe"; exit 2 }
$name = "$Tier-$Mode"
$out = Join-Path $root "tools\qa\screen\out\$name"
New-Item -ItemType Directory -Force (Join-Path $out 'shots') | Out-Null
Get-ChildItem (Join-Path $out 'shots') -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$cut = (Join-Path $root "tools\qa\screen\u1-cut-$Tier.json").Replace('\', '/')
$tierId = if ($Tier -eq 'clay') { 'p3_clay' } else { 'l2_night' }
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter "u1-*$tierId*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$cvars = "jam.ScriptHuman=1,jam.screen.HoldStill=1,jam.screen.FadeSeconds=0,jam.screen.StillEvery=$EveryFrames"
if ($Mode -eq 'before') { $cvars += ',jam.screen.LightHold=0,jam.hud.YouMarker=0,jam.hud.LabelPlates=0,jam.hud.DockCommentary=0,jam.hud.HintEveryMatch=1,jam.camera.CornerLift=0' }
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$a = "`"$uproject`" /Game/Maps/Rink -game -RenderOffScreen -Multiprocess -ResX=$ResX -ResY=$ResY -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
     "-cut=$cut -dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A111' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
$events = Join-Path $out 'events.log'
Remove-Item $events -ErrorAction SilentlyContinue
function Note([string]$s) { $line = ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s); Write-Host $line; Add-Content $events $line }
function LogHas([string]$pat) {
    if (-not (Test-Path $log)) { return $false }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t.Contains($pat) } catch { return $false }
}
function WaitLog([string]$pat, [int]$sec) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if (LogHas $pat) { return $true }; Start-Sleep -Milliseconds 100 }
    return $false
}
function Console([string]$cmd, [int]$waitSec = 6) {
    try {
        $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $waitSec * 1000; $c.Connect('127.0.0.1', $Port)
        $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
        $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
        $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
        $ans = $null
        for ($i = 0; $i -lt 50; $i++) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith('{"id":2,')) { $ans = $l; break } }
        $c.Close()
        return ($ans -and $ans.Contains('"ok":true'))
    } catch { Note "console '$cmd': $($_.Exception.Message)"; return $false }
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'quit' 3 | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process tree (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    $n = 0
    Get-ChildItem $shotDir -Filter "u1-*$tierId*.png" -ErrorAction SilentlyContinue | ForEach-Object { Copy-Item $_.FullName (Join-Path $out "shots\$($_.Name)") -Force; $n++ }
    Note "$n stills copied to $out\shots"
    if (Test-Path $log) {
        Select-String -Path $log -Pattern 'JAMSCREEN light|JAMSCREEN first|JAMLOOK tier=|JAMLOOK rig .* loaded|JAMCUT rejected' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'screen-lines.txt')
        Get-Content (Join-Path $out 'screen-lines.txt') | ForEach-Object { Write-Host "  | $_" }
    }
    $first = Test-Path (Join-Path $out "shots\u1-first-$tierId.png")
    if ($code -eq 0 -and (-not $first -or $n -lt 2)) { $code = 1; $why = 'a still is missing' }
    Write-Host ("{0} u1-stills {1} ({2})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $name, $why)
    exit $code
}

Note "started pid $($p.Id) tier $tierId mode $Mode port $Port"
if (-not (WaitLog 'JAMSCREEN light hold' $StartTimeoutSec)) { Finish 2 'the match never started (no JAMSCREEN light hold line)' }
Note "the match is built; playing $PlaySeconds s, a still every $EveryFrames sim frames"
$t0 = $sw.Elapsed.TotalSeconds
while ($sw.Elapsed.TotalSeconds - $t0 -lt $PlaySeconds -and -not $p.HasExited) { Start-Sleep -Milliseconds 500 }
Finish 0 'played'
