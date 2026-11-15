# Lane A224: lane A223's eye3-run.ps1 copied (out under tools/qa/eye4/out, pids in Game/logs/a224, port 4724).
# Lane A223 (track L1): a look filmed from the game camera, off screen, on a PACKAGED Development exe of this worktree (-Exe) or the editor binary of this
# worktree (-Editor). Lane A219's eye2-run.ps1 with one more way to film: -Every N -Frames F sets jam.screen.StillEvery on the command line, so the stills are
# of the same sim frames on every start (N, 2N, ...), and waits until sim frame F is filmed. The match is the cartridge's seeded one with the person-like hand
# (-ScriptHuman 2). No window, no cursor. The game's process id goes to Game\logs\a223\<run>.pid and out\<run>\game.json; it is stopped only by that id.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye3\eye3-run.ps1 -Editor -Run pond-1080 -Choose modes/hockey.mode.pond -ScriptHuman 2 -Every 30 -Frames 3600 -ResX 1920 -ResY 1080
param(
    [string]$Exe = '',
    [switch]$Editor,
    [string]$Run = 'run',
    [string]$Level = 'rookie',
    [string]$Choose = '',
    [string]$Drills = '2,23',
    [int]$DrillFrames = 600,
    [int]$ScriptHuman = 0,
    [int]$Port = 4724,
    [int]$ResX = 1280, [int]$ResY = 720,
    [string]$ExtraCVars = '',
    [string]$PreCmds = '',          # console commands sent once the match is up, ';' separated
    [int]$SettleSec = 4,
    [switch]$DumpCVars,             # asks the running game for the value of every name in cvars.txt (jam.hit.*, jam.hud.*) into out\<run>\cvars-seen.txt
    [int]$HitCapture = 0,           # jam.hit.Capture N: the game's own every-frame strips of the human's presses into out\<run>\hits (no drills: -WaitSec of match)
    [int]$WaitSec = 0,
    [int]$Every = 0, [int]$Frames = 3600,   # -Every N: a still every N sim frames from the first frame, until sim frame -Frames
    [int]$MaxWallSec = 1500
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL eye3: this tool runs only from a worktree (root $root)"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$out = Join-Path $root "tools\qa\eye4\out\$Run"
$shots = Join-Path $out 'shots'
New-Item -ItemType Directory -Force $shots | Out-Null
Get-ChildItem $shots -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
Remove-Item (Join-Path $out 'drills.txt') -ErrorAction SilentlyContinue
$cvars = "r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0,jam.ScriptHuman=$ScriptHuman"
if ($Every -gt 0) { $cvars += ",jam.screen.StillEvery=$Every" }
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$hits = Join-Path $out 'hits'
New-Item -ItemType Directory -Force $hits | Out-Null
if ($HitCapture -gt 0) { $cvars += ",jam.hit.Capture=$HitCapture"; Get-ChildItem $hits -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force }
$common = "-RenderOffScreen -ResX=$ResX -ResY=$ResY -ForceRes -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -abslog=`"$log`" -ForceLogFlush -dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port -JamHitShots=`"$($hits.Replace([char]92, [char]47))`""
if ($Editor) {
    $uproject = Resolve-UProject (Join-Path $root 'Game')
    $engine = Get-EngineRoot $uproject
    $Exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
    $a = "`"$uproject`" /Game/Maps/Menu -game -Multiprocess $common"
    $shotDirs = @((Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'))
} else {
    $Exe = (Resolve-Path $Exe).Path
    if (-not $Exe.StartsWith($root, [StringComparison]::OrdinalIgnoreCase)) { Write-Host "FAIL eye3: the exe is not under this worktree ($Exe)"; exit 2 }
    if ($Exe -match 'Shipping') { Write-Host 'FAIL eye3: never a Shipping exe'; exit 2 }
    $a = $common
    $saved = Join-Path (Split-Path (Split-Path (Split-Path $Exe))) 'Saved'
    $shotDirs = @((Join-Path $saved 'Screenshots\Windows'), (Join-Path $saved 'Screenshots\WindowsNoEditor'))
}
foreach ($d in $shotDirs) { if (Test-Path $d) { Get-ChildItem $d -Filter 'u1-*.png' -ErrorAction SilentlyContinue | Remove-Item -Force } }
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
$start = $p.StartTime
$pidDir = Join-Path $root 'Game\logs\a224'
New-Item -ItemType Directory -Force $pidDir | Out-Null
"$($p.Id) $($start.ToString('o')) $Run $Exe" | Set-Content (Join-Path $pidDir "$Run.pid")
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; lane = 'A224' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
$notes = Join-Path $out 'driver.log'
Remove-Item $notes -ErrorAction SilentlyContinue
function Note([string]$s) { $line = ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s); Write-Host $line; Add-Content $notes $line }
function LogText {
    if (-not (Test-Path $log)) { return '' }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t } catch { return '' }
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
$script:n = 0; $script:maxFrame = 0
function Sweep([switch]$All) {
    $now = Get-Date
    foreach ($d in $shotDirs) {
        Get-ChildItem $d -Filter '*.png' -ErrorAction SilentlyContinue | ForEach-Object {
            if ($_.Name -match '^u1-[a-z0-9_]+-f(\d+)\.png$') { $f = [int]$Matches[1]; if ($f -gt $script:maxFrame) { $script:maxFrame = $f } }
            if ($All -or ($now - $_.LastWriteTime).TotalMilliseconds -gt 500) { try { Move-Item $_.FullName (Join-Path $shots $_.Name) -Force -ErrorAction Stop; $script:n++ } catch { } }
        }
    }
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'jam.screen.StillEvery 0' 3 -Quiet | Out-Null; Console 'quit' 3 -Quiet | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    Start-Sleep -Milliseconds 700
    Sweep -All
    if (Test-Path $log) {
        Select-String -Path $log -Pattern 'JAMMATCH|JAMSIM frame=|JAMVIS|JAMHIT|JAMDRILL|JAMCTRL|JAMMENU|JAMCART|JAMSCRIPT|JAMBAR|Fatal|GPU crash|DXGI_ERROR|D3D device' |
            ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' } | Set-Content (Join-Path $out 'events.txt')
    }
    Write-Host ("{0} eye3 {1} ({2}; {3} stills in {4}; last sim frame {5})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Run, $why, $script:n, $shots, $script:maxFrame)
    exit $code
}
Note "started pid $($p.Id) run $Run port $Port exe $Exe"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt 600 -and -not $p.HasExited -and -not $up) { $up = Console 'jam.screen.FadeSeconds 0' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Note 'the console answers'
if (-not (WaitLog 'JAMPRELOAD warm scene down|JAMPRELOAD done' 120)) { Note 'no JAMPRELOAD done line in 120 s: going on' }
Start-Sleep -Seconds 3
[void](Console "jam.menuscreen level $Level")
$started = $false
for ($try = 0; $try -lt 6 -and -not $started -and -not $p.HasExited; $try++) {
    if ($try -gt 0) { Note "no match yet: again (try $($try + 1))"; [void](Console "jam.menuscreen level $Level") }
    if ($Choose) { [void](Console "jam.menuscreen choose $Choose") } else { [void](Console 'jam.menuscreen play') }
    $started = WaitLog 'JAMMATCH level=' 20
}
if (-not $started) { Finish 2 'no match started (no JAMMATCH line)' }
$m = [regex]::Match((LogText), 'JAMMATCH level=\S+ source=\S+ cut=\S+ id=\S+')
Note "match: $($m.Value)"
Start-Sleep -Seconds $SettleSec
if ($DumpCVars) {
    $seen = Join-Path $out 'cvars-seen.txt'; Remove-Item $seen -ErrorAction SilentlyContinue
    foreach ($n in (Get-Content (Join-Path $PSScriptRoot 'cvars.txt'))) {
        try {
            $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = 4000; $c.Connect('127.0.0.1', $Port)
            $st = $c.GetStream(); $w = New-Object IO.StreamWriter($st); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($st)
            $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
            $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $n } | ConvertTo-Json -Compress))
            for ($i = 0; $i -lt 50; $i++) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith('{"id":2,')) { Add-Content $seen "$n  $l"; break } }
            $c.Close()
        } catch { Add-Content $seen "$n  ERROR $($_.Exception.Message)" }
    }
    Note "cvars asked: $seen"
}
foreach ($c in ($PreCmds -split ';')) { if ($c.Trim()) { [void](Console $c.Trim()); Note "cmd: $($c.Trim())" } }
if ($WaitSec -gt 0) {
    $t1 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t1 -lt $WaitSec -and -not $p.HasExited) { Start-Sleep -Seconds 2 }
    $script:n = @(Get-ChildItem $hits -Filter '*.png' -ErrorAction SilentlyContinue).Count
    Finish 0 "waited $WaitSec s of match with the hit capture on"
}
if ($Every -gt 0) {
    $t1 = $sw.Elapsed.TotalSeconds
    while ($true) {
        if ($p.HasExited) { Finish 1 "the game died at about sim frame $($script:maxFrame) (exit $($p.ExitCode))" }
        Sweep
        if ($script:maxFrame -ge $Frames) { break }
        if ($sw.Elapsed.TotalSeconds - $t1 -gt $MaxWallSec) { Finish 0 "wall clock limit at sim frame $($script:maxFrame)" }
        Start-Sleep -Milliseconds 500
    }
    Finish 0 "filmed every $Every frames to sim frame $($script:maxFrame)"
}
foreach ($d in @($Drills -split ',' | Where-Object { $_ })) {
    [void](Console "jam.ctrl.Drill $d")
    [void](Console 'jam.screen.StillEvery 1')
    Sweep; $f0 = $script:maxFrame; $t1 = $sw.Elapsed.TotalSeconds; $first = 0
    Note "drill $d on at about sim frame $f0"
    while ($true) {
        if ($p.HasExited) { Finish 1 "the game died at about sim frame $($script:maxFrame) (exit $($p.ExitCode))" }
        Sweep
        if ($first -eq 0 -and $script:maxFrame -gt $f0) { $first = $script:maxFrame }
        if ($first -gt 0 -and $script:maxFrame -ge $first + $DrillFrames) { break }
        if ($sw.Elapsed.TotalSeconds - $t1 -gt 600) { Note "drill ${d}: 600 s without $DrillFrames frames"; break }
        if ($sw.Elapsed.TotalSeconds -gt $MaxWallSec) { Finish 0 "wall clock limit at sim frame $($script:maxFrame)" }
        Start-Sleep -Milliseconds 200
    }
    [void](Console 'jam.screen.StillEvery 0')
    Note "drill $d filmed: frames $first to $($script:maxFrame)"
    "$d $first $($script:maxFrame)" | Add-Content (Join-Path $out 'drills.txt')
    [void](Console 'jam.ctrl.Drill 0')
    Start-Sleep -Seconds 1
}
Finish 0 'the drills are filmed'
