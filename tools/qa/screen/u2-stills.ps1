# Lane A144 (track U2, the menu): stills of every menu screen from this lane's own game process, off screen, with no window grab and no system cursor.
# One editor-binary game process on the Menu map (-RenderOffScreen), driven through its remote console (port 4744) by the menu's own QA command
# (jam.menuscreen main|cuts|direct|settings|controls|credits|say <sentence>), the lobby's (jam.lobby, jam.host, jam.leave) and the still command
# (jam.screen.Still <name>). Both resolutions in one process (r.SetRes). The stills are copied to tools\qa\screen\out\u2-<Tag>\.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag after
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag end-win -EndCard -ScriptHuman 1     (a season take played to its card)
# -EndCard: instead of the menu screens, starts the season from the menu (jam.Season.Start), waits for the take's result line (JAMSEASON take ... WON|LOST)
#   and takes the end-of-match card; -ScriptHuman 1 plays the human's seat (it may win), 0 leaves it idle (it loses when the other side scores).
# Exit 0: every still asked for exists. 1: one is missing. 2: could not run.
param(
    [string]$Tag = 'after',
    [int]$Port = 4744,
    [switch]$EndCard,
    [switch]$Say,          # also direct one sentence on the Direct screen: with an API key on the machine this is a live, paid call
    [int]$ScriptHuman = 1,
    [int]$EndTimeoutSec = 900,
    [string[]]$Screens = @('main', 'cuts', 'direct', 'settings', 'controls', 'credits'),
    [int]$StartTimeoutSec = 900
)
$Screens = @($Screens | ForEach-Object { $_ -split ',' } | Where-Object { $_ })   # -File passes a,b,c as one string
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL u2-stills: no project or no $exe"; exit 2 }
$out = Join-Path $root "tools\qa\screen\out\u2-$Tag"
New-Item -ItemType Directory -Force $out | Out-Null
Get-ChildItem $out -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter "u2-$Tag-*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$cvars = "jam.screen.FadeSeconds=0,jam.ScriptHuman=$ScriptHuman"
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=1920 -ResY=1080 -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A144' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
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
function Still([string]$name, [int]$settleMs = 1200) {
    Start-Sleep -Milliseconds $settleMs
    $n = "u2-$Tag-$name"
    [void](Console "jam.screen.Still $n")
    [void]$want.Add($n)
    $f = Join-Path $shotDir "$n.png"
    for ($i = 0; $i -lt 40 -and -not (Test-Path $f); $i++) { Start-Sleep -Milliseconds 150 }
    Note "still $n $(if (Test-Path $f) { 'ok' } else { 'MISSING' })"
}
function Finish([int]$code, [string]$why) {
    Note $why
    if (-not $p.HasExited) { Console 'quit' 3 | Out-Null }
    for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
    if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our own game process tree (quit did not exit it)' } else { Note "game exited, code $($p.ExitCode)" }
    $missing = 0
    foreach ($n in $want) { $f = Join-Path $shotDir "$n.png"; if (Test-Path $f) { Copy-Item $f (Join-Path $out "$n.png") -Force } else { $missing++ } }
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMMENU|JAMSEASON|jam\.host|jam\.join' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'menu-lines.txt') }
    if ($code -eq 0 -and ($missing -gt 0 -or $want.Count -eq 0)) { $code = 1; $why = "$missing stills missing of $($want.Count)" }
    Write-Host ("{0} u2-stills {1} ({2}; {3} stills in {4})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Tag, $why, ($want.Count - $missing), $out)
    exit $code
}

Note "started pid $($p.Id) port $Port tag $Tag"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.menuscreen main' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Note 'the console answers; letting the menu draw'
Start-Sleep -Seconds 8

if ($EndCard) {
    [void](Console 'jam.Season.Reset')
    [void](Console 'jam.Season.Start')
    if (-not (WaitLog 'JAMMENU start match' 30)) { Finish 2 'the season did not start a match' }
    Start-Sleep -Seconds 12
    Still '1080-season-start' 0
    if (-not (WaitLog 'JAMSEASON take \d+ .* (WON|LOST)' $EndTimeoutSec)) { Still '1080-end-timeout' 0; [void](Console 'jam.Season.Reset'); Finish 1 'no season result inside the time box' }
    Still '1080-end-card' 2500
    [void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
    Still '720-end-card' 1500
    [void](Console 'jam.Season.Reset')
    Finish 0 'end card taken'
}

foreach ($res in @(@{ n = '1080'; s = '1920x1080w' }, @{ n = '720'; s = '1280x720w' })) {
    [void](Console "r.SetRes $($res.s)"); Start-Sleep -Seconds 2
    foreach ($scr in $Screens) { if (Console "jam.menuscreen $scr") { Still "$($res.n)-$scr" } else { Note "screen $scr refused" } }
    if ($Say) { [void](Console 'jam.menuscreen say make it faster and meaner'); Still "$($res.n)-direct-result" 3000 }
    [void](Console 'jam.menuscreen main')
}
# The lobby opens over the menu and has no console command to close it: last, at 720 first, then back at 1080.
[void](Console 'jam.lobby')
Still '720-lobby' 1500
[void](Console 'jam.host hockey.regular')
Still '720-lobby-host' 2000
[void](Console 'r.SetRes 1920x1080w'); Start-Sleep -Seconds 2
Still '1080-lobby-host' 1000
[void](Console 'jam.leave')
Still '1080-lobby' 1500
Finish 0 'menu screens taken'
