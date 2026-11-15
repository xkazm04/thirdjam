# Lane A213 (track U3, the night of 14 to 15 Nov): stills of a match as the operator starts it (PLAY NOW, rookie, the regular rink), off screen, from the
# game camera, with the HUD. One editor-binary game process on the Menu map (-RenderOffScreen, no window, no cursor), driven through its remote console
# (port 4815) as lane A144's u2-stills.ps1 drives the menu; the human is scripted (jam.ScriptHuman 1) so the puck changes hands by itself.
# Stills: jam.screen.Still at fixed moments at 1920 x 1080 then 1280 x 720, plus whatever stills the game takes itself through -ExtraCVars
# (jam.hud.PossessStills N: one still N sim frames into each possession state, lane A213). Every run: r.RDG.AsyncCompute=0 (R-0066 point 5).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a213\a213-stills.ps1 -Tag inv
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a213\a213-stills.ps1 -Tag after -ExtraCVars "jam.hud.PossessStills=4"
# The game's process id is written to out\a213-<Tag>\game.json as it starts (stop only by that id). Exit 0 stills taken, 1 some missing, 2 could not run.
param(
    [string]$Tag = 'inv',
    [int]$Port = 4815,
    [string]$Level = 'rookie',
    [int]$ScriptHuman = 1,
    [string]$ExtraCVars = '',
    [string]$At1080 = '1,4,9,14,20,27,35',
    [string]$At720 = '3,8,14,22',
    [string[]]$Before = @(),
    [string]$StartCmd = 'jam.menuscreen play',   # wave 3: e.g. 'jam.menuscreen choose modes/hockey.mode.glass-house' (Glow Ice)
    [string]$LaunchCut = '',   # wave 3: -cut=<this> on the command line (a look tier's proof cut, e.g. tools/qa/screen/u1-cut-night.json)
    [string]$Between = '',   # a console command sent after the 1080 stills, before the 720 ones (e.g. 'jam.ctrl.EndMatch 0': real goals for the player's side)
    [int]$TailSec = 20,
    [int]$StartTimeoutSec = 900
)
$Before = @($Before | ForEach-Object { $_ -split ';' } | Where-Object { $_ })
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL a213-stills: no project or no $exe"; exit 2 }
$out = Join-Path $root "tools\qa\screen\out\a213-$Tag"
New-Item -ItemType Directory -Force $out | Out-Null
Get-ChildItem $out -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter 'a213-*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$cvars = "r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0,jam.ScriptHuman=$ScriptHuman"
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=1920 -ResY=1080 -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
if ($LaunchCut) { $a += " -cut=`"$((Join-Path $root $LaunchCut).Replace([char]92, [char]47))`"" }
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A213' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
function Note([string]$s) { Write-Host ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s) }
function LogHas([string]$pat) {
    if (-not (Test-Path $log)) { return $false }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return ($t -match $pat) } catch { return $false }
}
function WaitLog([string]$pat, [int]$sec) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if (LogHas $pat) { return $true }; Start-Sleep -Milliseconds 250 }
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
$want = New-Object System.Collections.ArrayList
function Still([string]$name) {
    $n = "a213-$Tag-$name"
    [void](Console "jam.screen.Still $n")
    [void]$want.Add($n)
    $f = Join-Path $shotDir "$n.png"
    for ($i = 0; $i -lt 40 -and -not (Test-Path $f); $i++) { Start-Sleep -Milliseconds 150 }
    Note "still $n $(if (Test-Path $f) { 'ok' } else { 'MISSING' })"
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'quit' 3 -Quiet | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process tree (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    $missing = 0
    foreach ($n in $want) { $f = Join-Path $shotDir "$n.png"; if (-not (Test-Path $f)) { $missing++ } }
    # Every still of this run: ours by name, and the game's own proof stills (a213-* / u1-*) written after the start.
    Get-ChildItem $shotDir -Filter '*.png' -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $start -and ($_.Name -like 'a213-*' -or $_.Name -like 'u1-*') } | ForEach-Object { Move-Item $_.FullName (Join-Path $out $_.Name) -Force }
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMHUD|JAMMATCH|JAMMENU start' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'hud-lines.txt') }
    if ($code -eq 0 -and ($missing -gt 0 -or $want.Count -eq 0)) { $code = 1; $why = "$missing stills missing of $($want.Count)" }
    Write-Host ("{0} a213-stills {1} ({2}; {3} stills in {4})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Tag, $why, @(Get-ChildItem $out -Filter '*.png').Count, $out)
    exit $code
}

Note "started pid $($p.Id) port $Port tag $Tag"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.menuscreen main' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Start-Sleep -Seconds 6
[void](Console "jam.menuscreen level $Level"); Start-Sleep -Milliseconds 500
foreach ($cmd in $Before) { [void](Console $cmd); Start-Sleep -Milliseconds 300 }
[void](Console $StartCmd)
if (-not (WaitLog 'JAMMATCH level=' 90)) { Finish 2 'PLAY NOW did not start a match (no JAMMATCH line)' }
$m0 = $sw.Elapsed.TotalSeconds
Note 'the match started'
function At([double]$sec) { $t = $m0 + $sec; while ($sw.Elapsed.TotalSeconds -lt $t -and -not $p.HasExited) { Start-Sleep -Milliseconds 100 } }
foreach ($s in @($At1080 -split ',' | Where-Object { $_ })) { At ([int]$s); Still ('1080-t{0:d3}' -f [int]$s) }
if ($Between) { [void](Console $Between); Note "sent: $Between" }
[void](Console 'r.SetRes 1280x720w')
$m1 = [int]($sw.Elapsed.TotalSeconds - $m0) + 2
foreach ($s in @($At720 -split ',' | Where-Object { $_ })) { $t = $m1 + [int]$s; At $t; Still ('720-t{0:d3}' -f $t) }
[void](Console 'r.SetRes 1920x1080w')
At ($sw.Elapsed.TotalSeconds - $m0 + $TailSec)
Finish 0 'stills taken'
