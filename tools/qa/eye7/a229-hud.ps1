# Lane A229 (U5, the HUD round): before / after stills of the match HUD from this worktree's EDITOR binary (UnrealEditor.exe -game), off screen
# (-RenderOffScreen), no window grab, no cursor, -dpcvars r.RDG.AsyncCompute=0, fixed step 60. Port 4729. Built from lane A222's a222-panel.ps1.
# One process per rink: the faceoff at 1080, play stills at 1080 and 720, the faceoff dial previewed at 720, the pause card at both, a goal by
# jam.ctrl.EndMatch 1 (the GOAL banner, then the end card) at both sizes.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye7\a229-hud.ps1 -Tag before -Rink clay
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye7\a229-hud.ps1 -Tag after -Rink glow
# pid: Game\logs\a229\<Tag>-<Rink>-game.pid; stills: tools\qa\eye7\out\<Tag>-<Rink>\. Exit 0 all stills, 1 one missing, 2 could not run.
param([string]$Tag = 'after', [string]$Rink = 'clay', [int]$Port = 4729, [int]$StartTimeoutSec = 900, [int]$OneLook = -1, [int]$Every = 0)
# -Every N (wave 2): jam.screen.StillEvery N from the first frame until the 1080 play stills are done (u1-*-f<frame>.png into out\<run>\every): the box plate's foul.
# -OneLook 0 / 1 sets jam.hud.OneLook on the command line (0 = the HUD of before on the same binary: the neutral value); -1 leaves the default.
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL a229-hud: runs only from a worktree (root $root)"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL a229-hud: no project or no $exe"; exit 2 }
$run = "$Tag-$Rink"
$out = Join-Path $root "tools\qa\eye7\out\$run"
New-Item -ItemType Directory -Force $out | Out-Null
Get-ChildItem $out -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter "a229-$run-*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=1920 -ResY=1080 -ForceRes -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0,jam.ScriptHuman=2$(if ($OneLook -ge 0) { ",jam.hud.OneLook=$OneLook" })$(if ($Every -gt 0) { ",jam.screen.StillEvery=$Every" }) -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
$start = $p.StartTime
New-Item -ItemType Directory -Force (Join-Path $root 'Game\logs\a229') | Out-Null
Set-Content (Join-Path $root "Game\logs\a229\$run-game.pid") "$($p.Id) $($start.ToString('o'))"
function Note([string]$s) { Write-Host ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s) }
function LogText { if (-not (Test-Path $log)) { return '' }; try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t } catch { return '' } }
function WaitLog([string]$pat, [int]$sec) { $t0 = $sw.Elapsed.TotalSeconds; while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if ((LogText) -match $pat) { return $true }; Start-Sleep -Milliseconds 200 }; return $false }
function Console([string]$cmd, [int]$waitSec = 6, [switch]$Quiet) {
    try {
        $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $waitSec * 1000; $c.Connect('127.0.0.1', $Port)
        $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
        $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
        $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
        $ans = $null
        for ($i = 0; $i -lt 50; $i++) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith('{"id":2,')) { $ans = $l; break } }
        $c.Close()
        return ($ans -and $ans.Contains('"ok":true'))
    } catch { if (-not $Quiet) { Note "console '$cmd': $($_.Exception.Message)" }; return $false }
}
$want = New-Object System.Collections.ArrayList
function Still([string]$name, [int]$settleMs = 600) {
    Start-Sleep -Milliseconds $settleMs
    $n = "a229-$run-$name"
    [void](Console "jam.screen.Still $n")
    [void]$want.Add($n)
    $f = Join-Path $shotDir "$n.png"
    for ($i = 0; $i -lt 40 -and -not (Test-Path $f); $i++) { Start-Sleep -Milliseconds 150 }
    Note "still $n $(if (Test-Path $f) { 'ok' } else { 'MISSING' })"
}
function BarkStill([string]$name) {
    # A bubble is up for about two seconds after a JAMBARK line: wait (15 s at most) for the next one, then take the still.
    $n0 = ([regex]::Matches((LogText), 'JAMBARK who=')).Count
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt 15 -and ([regex]::Matches((LogText), 'JAMBARK who=')).Count -le $n0) { Start-Sleep -Milliseconds 150 }
    Still $name 250
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'quit' 3 -Quiet | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process tree (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    $ev = Join-Path $out 'every'; New-Item -ItemType Directory -Force $ev | Out-Null
    Get-ChildItem $shotDir -Filter 'u1-*.png' -ErrorAction SilentlyContinue | ForEach-Object { Move-Item $_.FullName (Join-Path $ev $_.Name) -Force }
    $missing = 0
    foreach ($n in $want) { $f = Join-Path $shotDir "$n.png"; if (Test-Path $f) { Move-Item $f (Join-Path $out "$n.png") -Force } else { $missing++ } }
    if ($code -eq 0 -and ($missing -gt 0 -or $want.Count -eq 0)) { $code = 1; $why = "$missing stills missing of $($want.Count)" }
    Write-Host ("{0} a229-hud {1} ({2}; {3} stills in {4})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $run, $why, ($want.Count - $missing), $out)
    exit $code
}
Note "started pid $($p.Id) port $Port run $run (editor binary of this worktree)"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.screen.FadeSeconds 0' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
if (-not (WaitLog 'JAMPRELOAD warm scene down|JAMPRELOAD done' 120)) { Note 'no JAMPRELOAD line in 120 s: going on' }
Start-Sleep -Seconds 3
[void](Console 'jam.menuscreen level rookie')
$started = $false
for ($try = 0; $try -lt 6 -and -not $started -and -not $p.HasExited; $try++) {
    if ($Rink -eq 'glow') { [void](Console 'jam.menuscreen choose modes/hockey.mode.glass-house') } else { [void](Console 'jam.menuscreen play') }
    $started = WaitLog 'JAMMATCH level=' 20
}
if (-not $started) { Finish 2 'no match started' }
Still '1080-faceoff-a' 0
Still '1080-faceoff-b' 500
Start-Sleep -Seconds 4
for ($i = 0; $i -lt 4; $i++) { Still ('1080-play-{0}' -f $i) 2200 }
BarkStill '1080-bark'
if ($Every -gt 0) { [void](Console 'jam.screen.StillEvery 0') }
[void](Console 'jam.ui.effects pickup-speed:#speed#168'); Still '1080-effect' 300
[void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
for ($i = 0; $i -lt 3; $i++) { Still ('720-play-{0}' -f $i) 2200 }
BarkStill '720-bark'
[void](Console 'jam.ui.effects pickup-heavy:#heavy_puck#600'); Still '720-effect' 300
[void](Console 'jam.ui.effects'); Start-Sleep -Milliseconds 300
[void](Console 'jam.ui.faceoff 240'); Still '720-faceoff-dial' 500
Start-Sleep -Seconds 4
[void](Console 'jam.pausemenu open'); Still '720-pause' 900
[void](Console 'jam.pausemenu close'); Start-Sleep -Milliseconds 600
[void](Console 'r.SetRes 1920x1080w'); Start-Sleep -Seconds 2
[void](Console 'jam.pausemenu open'); Still '1080-pause' 900
[void](Console 'jam.pausemenu close'); Start-Sleep -Milliseconds 600
[void](Console 'jam.ctrl.EndMatch 0')
if (-not (WaitLog 'JAMCTRL endmatch done' 90)) { Note 'no endmatch done line in 90 s' }
Still '1080-goal' 200
[void](Console 'r.SetRes 1280x720w'); Still '720-goal' 700
Still '720-end' 2500
[void](Console 'r.SetRes 1920x1080w'); Still '1080-end' 1500
Finish 0 'HUD stills taken'
