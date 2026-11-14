# Lane A196 (track F3, evolution 9): the hits' frame strips, off screen, from the game camera (a188-stills.ps1 with the hits' capture).
# One editor-binary game process per call (-RenderOffScreen: no window, no cursor) on a fixed 60 Hz step (-UseFixedTimeStep -FPS=60: one sim step a rendered
# frame, so a still a sim frame). Lane A49's drills run one after the other through the remote console: 1 (the body check: 10 presses, lands and misses),
# 2 (the pressed cross-check), 23 (lane A196: the held cross-check skated into the carrier). jam.hit.Capture N takes a still every sim frame from each of the
# human's presses to 30 frames after its contact or miss (a196-<kind>-<press frame>-f<frame>.png), N strips a kind; the JAMHIT lines name each result.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a196-strips.ps1 -Mode after  [-Port 4796]
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a196-strips.ps1 -Mode before   (every jam.hit.* at 0: the screen of before)
# The game process's id is written to out\<run>\game.json as it starts (stop only by that id). Exit 0: strips taken. 1: none. 2: could not run.
param(
    [string]$Cut = 'tools/qa/screen/a196/cut-clay-long.json',
    [string]$Tier = 'clay',
    [ValidateSet('after', 'before')][string]$Mode = 'after',
    [string]$Drills = '23,2,1',
    [int]$DrillSeconds = 240,
    [int]$BraceSeconds = 40,
    [int]$Port = 4796,
    [int]$Strips = 10,
    [int]$ResX = 960, [int]$ResY = 540,
    [string]$ExtraCVars = '',
    [string]$RunName = '',
    [int]$StartTimeoutSec = 900
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL a196-strips: no project or no $exe"; exit 2 }
$name = "a196-$Tier-$Mode"
if ($RunName) { $name = $RunName }
$out = Join-Path $root "tools\qa\screen\out\$name"
New-Item -ItemType Directory -Force (Join-Path $out 'shots') | Out-Null
Get-ChildItem (Join-Path $out 'shots') -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$cut = (Join-Path $root $Cut).Replace('\', '/')
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter 'a196-*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$cvars = "jam.screen.HoldStill=1,jam.screen.FadeSeconds=0,jam.hit.Capture=$Strips"
if ($Mode -eq 'before') { $cvars += ',jam.hit.Tell=0,jam.hit.Lean=0,jam.hit.Burst=0,jam.hit.Down=0,jam.hit.PuckFlash=0' }
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$a = "`"$uproject`" /Game/Maps/Rink -game -RenderOffScreen -Multiprocess -ResX=$ResX -ResY=$ResY -ForceRes -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -log -abslog=`"$log`" -ForceLogFlush " +
     "-cut=$cut -dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A196' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
$events = Join-Path $out 'events.log'
Remove-Item $events -ErrorAction SilentlyContinue
function Note([string]$s) { $line = ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s); Write-Host $line; Add-Content $events $line }
function FindLog {
    if (Test-Path $script:log) { return }
    $dir = Join-Path $root 'Game\Saved\Logs'
    Get-ChildItem $dir -Filter '*.log' -ErrorAction SilentlyContinue | Where-Object { $_.Name -notmatch 'backup' -and $_.LastWriteTime -ge $start.AddSeconds(-2) } | Sort-Object LastWriteTime -Descending | ForEach-Object {
        if (Test-Path $script:log) { return }
        try {
            $fs = [IO.File]::Open($_.FullName, 'Open', 'Read', 'ReadWrite'); $buf = New-Object byte[] 262144; $n = $fs.Read($buf, 0, $buf.Length); $fs.Close()
            if ([Text.Encoding]::UTF8.GetString($buf, 0, $n).Contains("-RemoteConsolePort=$Port")) { $script:log = $_.FullName }
        } catch { }
    }
}
function LogHas([string]$pat) {
    FindLog
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
    Get-ChildItem $shotDir -Filter 'a196-*.png' -ErrorAction SilentlyContinue | ForEach-Object { Move-Item $_.FullName (Join-Path $out "shots\$($_.Name)") -Force; $n++ }
    Note "$n stills moved to $out\shots"
    if (Test-Path $log) {
        if ($log -ne (Join-Path $out 'game.log')) { Copy-Item $log (Join-Path $out 'game.log') -Force; $log = Join-Path $out 'game.log' }
        Select-String -Path $log -Pattern 'JAMHIT|JAMDRILL|JAMVIS crosscheck|JAMVIS crosshold|JAMHITSTOP|contact_line|JAMSIM frame=|JAMCUT rejected' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'hit-lines.txt')
        Write-Host "  $((Get-Content (Join-Path $out 'hit-lines.txt')).Count) lines in hit-lines.txt"
    }
    if ($code -eq 0 -and $n -lt 2) { $code = 1; $why = 'no strip was taken' }
    Write-Host ("{0} a196-strips {1} ({2})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $name, $why)
    exit $code
}

Note "started pid $($p.Id) mode $Mode port $Port"
if (-not (WaitLog 'JAMSCREEN light hold' $StartTimeoutSec)) { Finish 2 'the match never started (no JAMSCREEN light hold line)' }
Note "the match is built"
# The drills in turn, each switched on the log: drill 1 and 2 switch themselves off after 10 presses ("JAMDRILL summary mode=N"); 23 runs $BraceSeconds s.
# Seconds are wall clock (the fixed step makes the game run slower than it): each has a ceiling of $DrillSeconds.
foreach ($d in $Drills.Split(',')) {
    $ok = $false
    for ($k = 0; $k -lt 10 -and -not $ok -and -not $p.HasExited; $k++) { $ok = Console "jam.ctrl.Drill $d"; if (-not $ok) { Start-Sleep -Seconds 2 } }
    Note "drill $d on ok=$ok"
    if ($d -eq '23') { $t0 = $sw.Elapsed.TotalSeconds; while ($sw.Elapsed.TotalSeconds - $t0 -lt $BraceSeconds -and -not $p.HasExited) { Start-Sleep -Milliseconds 500 }; Console 'jam.ctrl.Drill 0' | Out-Null }
    elseif (-not (WaitLog "JAMDRILL summary mode=$d" $DrillSeconds)) { Note "drill ${d} - no summary in $DrillSeconds s"; Console 'jam.ctrl.Drill 0' | Out-Null }
    else { Note "drill $d done" }
}
Finish 0 'played'
