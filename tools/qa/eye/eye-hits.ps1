# Lane A211 (track E1, the eye): a whole match as the operator starts it, off screen, from the game camera.
# One editor-binary game process of THIS tree (-RenderOffScreen: no window, no cursor; fixed 60 Hz step: one sim frame a rendered frame) on the Menu map:
# a still of the landing and of HOME, the level picked on HOME's switch (jam.menuscreen level <level>), PLAY NOW (jam.menuscreen play), then the match
# played by the game's scripted human (jam.ScriptHuman 1). A still every -Every sim frames to frame -DenseUntil, every -EveryLate after (jam.screen.StillEvery),
# and every sim frame inside each of -Windows (frame ranges "a-b,c-d": the match is seeded by its cartridge, so a second run shows the same frames and the
# windows come from the first run's log). The stills move to tools\qa\eye\out\<run>\shots as they are written; the event lines of the log go to events.txt.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye\eye-match.ps1 -Level rookie -Run r1
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye\eye-match.ps1 -Level rookie -Run r1w -Windows "1200-1380,2400-2580" -NoMenuStills
# -Choose <name>: a mode or cut through the menu (jam.menuscreen choose <name>) instead of PLAY NOW. -Drill N: jam.ctrl.Drill N once the match is up.
# The game's process id is written to out\<run>\game.json as it starts (it is stopped only by that id). Every run passes r.RDG.AsyncCompute=0.
# Exit 0: the match was filmed. 1: the game died or no still. 2: could not run.
param(
    [ValidateSet('rookie', 'pro', 'allstar')][string]$Level = 'rookie',
    [string]$Run = '',
    [int]$Port = 4811,
    [int]$Every = 20,
    [int]$EveryLate = 60,
    [int]$DenseUntil = 7200,
    [string]$Windows = '',
    [int]$WindowLead = 60,
    [int]$MaxFrames = 0,          # 0 = to the end card
    [int]$MaxWallSec = 2400,
    [string]$Choose = '',
    [int]$Drill = 0,
    [string]$Drills = '',         # a list ("23,2,1"): each in turn, switched on its JAMDRILL summary line or after -DrillSeconds; then the run ends
    [int]$DrillSeconds = 240,
    [int]$BraceSeconds = 60,
    [int]$HitCapture = 0,         # jam.hit.Capture N: N every-frame strips a kind of the human's presses, into out\<run>\hits
    [int]$HitCaptureBots = 0,     # jam.hit.CaptureBots: 1 = the bots' presses near the human too
    [int]$MaxFps = 0,             # 0 = as fast as it renders
    [switch]$NoMenuStills,
    [int]$ScriptHuman = 1,        # jam.ScriptHuman: 1 the old script, 2 the playtester's hand through the button resolver (lane A211's ScriptHuman2.cpp), 0 the keys (nobody at them)
    [int]$ScriptHumanCam = 0,
    [string]$StillTier = 'none',   # the look tier of the cut filmed: the game names the periodic stills u1-<tier>-f<frame>; a second run in the same tree must film another tier (or -Every 0 with -StillTier none)
    [int]$ResX = 1280, [int]$ResY = 720,
    [string]$ExtraCVars = '',
    [int]$StartTimeoutSec = 900
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL eye-match: this tool runs only from a worktree (root $root)"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL eye-match: no project or no $exe"; exit 2 }
if (-not $Run) { $Run = "$Level-" + (Get-Date -Format 'HHmm') }
$out = Join-Path $root "tools\qa\eye\out\$Run"
$shots = Join-Path $out 'shots'
New-Item -ItemType Directory -Force $shots | Out-Null
Get-ChildItem $shots -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$hits = Join-Path $out 'hits'
New-Item -ItemType Directory -Force $hits | Out-Null
Get-ChildItem $hits -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
$stillPat = '^(u1-' + [regex]::Escape($StillTier) + '-|u1-goal-|u1-mark-|u1-passmeter-|eye-' + [regex]::Escape($Run) + '-)'
if ($StillTier -ne 'p3_clay') { $stillPat = '^(u1-' + [regex]::Escape($StillTier) + '-|eye-' + [regex]::Escape($Run) + '-)' }
New-Item -ItemType Directory -Force $shotDir | Out-Null
Get-ChildItem $shotDir -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object { $_.Name -match $stillPat } | Remove-Item -Force
$cvars = "r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0,jam.ScriptHuman=$ScriptHuman,jam.screen.StillEvery=$Every"
if ($ScriptHumanCam -gt 0) { $cvars += ",jam.ScriptHumanCam=$ScriptHumanCam" }
if ($MaxFps -gt 0) { $cvars += ",t.MaxFPS=$MaxFps" }
if ($HitCapture -gt 0) { $cvars += ",jam.hit.Capture=$HitCapture" }
if ($HitCaptureBots -gt 0) { $cvars += ",jam.hit.CaptureBots=$HitCaptureBots" }
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=$ResX -ResY=$ResY -ForceRes -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port -JamHitShots=`"$($hits.Replace([char]92, [char]47))`""
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A211' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
$notes = Join-Path $out 'driver.log'
Remove-Item $notes -ErrorAction SilentlyContinue
function Note([string]$s) { $line = ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s); Write-Host $line; Add-Content $notes $line }
function LogText {
    if (-not (Test-Path $log)) { return '' }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t } catch { return '' }
}
# The log's tail only (the log grows to megabytes over a match): the last $bytes bytes.
function LogTail([int]$bytes = 200000) {
    if (-not (Test-Path $log)) { return '' }
    try {
        $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite')
        if ($fs.Length -gt $bytes) { [void]$fs.Seek(-$bytes, 'End') }
        $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t
    } catch { return '' }
}
function WaitLog([string]$pat, [int]$sec) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if ((LogText) -match $pat) { return $true }; Start-Sleep -Milliseconds 250 }
    return $false
}
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
function Still([string]$name, [int]$settleMs = 1000) {
    Start-Sleep -Milliseconds $settleMs
    [void](Console "jam.screen.Still eye-$Run-$name")
    $f = Join-Path $shotDir "eye-$Run-$name.png"
    for ($i = 0; $i -lt 40 -and -not (Test-Path $f); $i++) { Start-Sleep -Milliseconds 150 }
    Note "still eye-$Run-$name $(if (Test-Path $f) { 'ok' } else { 'MISSING' })"
}
# Moves the finished stills to this run's folder; returns the highest sim frame seen in a u1-<tier>-f<frame> name. A file is moved once it is half a second old.
$script:maxFrame = 0
function Sweep([switch]$All) {
    $now = Get-Date
    Get-ChildItem $shotDir -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object { $_.Name -match $stillPat } | ForEach-Object {
        if ($_.Name -match '^u1-[a-z0-9_]+-f(\d+)\.png$') { $f = [int]$Matches[1]; if ($f -gt $script:maxFrame) { $script:maxFrame = $f } }
        if ($All -or ($now - $_.LastWriteTime).TotalMilliseconds -gt 500) { try { Move-Item $_.FullName (Join-Path $shots $_.Name) -Force -ErrorAction Stop } catch { } }
    }
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'quit' 3 -Quiet | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process tree (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    Start-Sleep -Milliseconds 700
    Sweep -All
    $n = @(Get-ChildItem $shots -Filter '*.png' -ErrorAction SilentlyContinue).Count + @(Get-ChildItem $hits -Filter '*.png' -ErrorAction SilentlyContinue).Count
    if (Test-Path $log) {
        Select-String -Path $log -Pattern 'JAMMATCH|JAMSIM frame=|JAMPASS|JAMSHOT|JAMRINK react goal|JAMVIS|JAMHIT|JAMDRILL|JAMCUE|JAMHUD|JAMCTRL|JAMMENU|JAMCART|JAMSEASON|JAMRULE|Fatal|GPU crash|DXGI_ERROR|D3D device' |
            ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' } | Set-Content (Join-Path $out 'events.txt')
    }
    if ($code -eq 0 -and $n -lt 2) { $code = 1; $why = 'no still was taken' }
    Write-Host ("{0} eye-match {1} ({2}; {3} stills in {4}; last sim frame {5})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Run, $why, $n, $shots, $script:maxFrame)
    exit $code
}

Note "started pid $($p.Id) level $Level port $Port run $Run"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.screen.FadeSeconds 0' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Note 'the console answers; letting the menu draw'
# The menu takes no command while its preload runs (a warm start answers the console 15 s before the menu is up: two runs lost at 00:54).
if (-not (WaitLog 'JAMPRELOAD warm scene down|JAMPRELOAD done' 120)) { Note 'no JAMPRELOAD done line in 120 s: going on' }
Start-Sleep -Seconds 3
if (-not $NoMenuStills) {
    Still 'menu-0-landing' 0
    [void](Console 'jam.menuscreen start')
    Still 'menu-1-home'
}
[void](Console "jam.menuscreen level $Level")
if (-not $NoMenuStills) { Still "menu-2-home-$Level" 600 }
$started = $false
for ($try = 0; $try -lt 6 -and -not $started -and -not $p.HasExited; $try++) {
    if ($try -gt 0) { Note "no match yet: the start command again (try $($try + 1))"; [void](Console "jam.menuscreen level $Level") }
    if ($Choose) { [void](Console "jam.menuscreen choose $Choose") } else { [void](Console 'jam.menuscreen play') }
    $started = WaitLog 'JAMMATCH level=' 20
}
if (-not $started) { Finish 2 'no match started (no JAMMATCH line)' }
$m = [regex]::Match((LogText), 'JAMMATCH level=\S+ source=\S+ cut=\S+ id=\S+')
Note "match: $($m.Value)"
if ($Drill -gt 0) { [void](Console "jam.ctrl.Drill $Drill"); Note "drill $Drill on" }

# The windows: every sim frame from (a - lead) to b.
$wins = @()
foreach ($w in ($Windows -split ',')) { if ($w -match '^\s*(\d+)-(\d+)\s*$') { $wins += , @([int]$Matches[1], [int]$Matches[2]) } }
$cur = $Every
$over = $false; $overAt = 0.0
$lastProgress = $sw.Elapsed.TotalSeconds; $lastSeen = 0; $lastNote = 0
$drillList = @($Drills -split ',' | Where-Object { $_ }); $di = -1; $drillAt = 0.0
while ($true) {
    if ($p.HasExited) { Finish 1 "the game process died at about sim frame $($script:maxFrame) (exit $($p.ExitCode))" }
    Sweep
    $f = $script:maxFrame
    if ($drillList.Count -gt 0) {
        $d = $null; if ($di -ge 0) { $d = $drillList[$di] }
        $limit = $DrillSeconds; if ($d -eq '23') { $limit = $BraceSeconds }
        $done = ($di -lt 0) -or ($sw.Elapsed.TotalSeconds - $drillAt -gt $limit) -or ($d -ne '23' -and (LogTail) -match "JAMDRILL summary mode=$d")
        if ($done) {
            if ($di -ge 0) { Note "drill $d ended after $([int]($sw.Elapsed.TotalSeconds - $drillAt)) s"; [void](Console 'jam.ctrl.Drill 0' 4 -Quiet) }
            $di++
            if ($di -ge $drillList.Count) { Finish 0 'the drills are played' }
            [void](Console "jam.ctrl.Drill $($drillList[$di])"); $drillAt = $sw.Elapsed.TotalSeconds; Note "drill $($drillList[$di]) on"
        }
        $lastProgress = $sw.Elapsed.TotalSeconds
    }
    if ($f -gt $lastSeen) { $lastSeen = $f; $lastProgress = $sw.Elapsed.TotalSeconds }
    $want = $Every; if ($f -ge $DenseUntil -and $Every -gt 0) { $want = $EveryLate }
    foreach ($w in $wins) { if ($f -ge ($w[0] - $WindowLead) -and $f -lt $w[1]) { $want = 1 } }
    if ($want -ne $cur) { if (Console "jam.screen.StillEvery $want" 4 -Quiet) { Note "sim frame ${f}: a still every $want"; $cur = $want } }
    if ($f - $lastNote -ge 1800) { $lastNote = $f; Note "sim frame $f" }
    if ($MaxFrames -gt 0 -and $f -ge $MaxFrames) { Finish 0 "reached sim frame $f" }
    if (-not $over) {
        $tail = LogTail
        if ($tail -match 'JAMSIM frame=\d+ hash=\S+ score=\d+:\d+ phase=2' -or $tail -match 'JAMSEASON take \d+ .* (WON|LOST)|LogHokej: JAMEND') { $over = $true; $overAt = $sw.Elapsed.TotalSeconds; Note "the match is over near sim frame $f" }
    } elseif ($drillList.Count -gt 0 -and $di -lt $drillList.Count) {
        # a drill run outlives a match: the rematch (the R key's command), and the drill goes on
        [void](Console 'jam.Season.Restart' 4 -Quiet); Note 'the match ended inside a drill: restarted'; Start-Sleep -Seconds 3; $over = $false
    } elseif ($sw.Elapsed.TotalSeconds - $overAt -gt 6) {
        Still 'end-card' 0
        Finish 0 'played to the end card'
    }
    if ($Every -gt 0 -and $sw.Elapsed.TotalSeconds - $lastProgress -gt 40) { Still 'stalled' 0; Finish 0 "no new still for 40 s at sim frame $f (the match ended or stalled)" }
    if ($sw.Elapsed.TotalSeconds -gt $MaxWallSec) { Finish 0 "wall clock limit at sim frame $f" }
    Start-Sleep -Milliseconds 100
}
