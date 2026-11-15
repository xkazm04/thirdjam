# Lane A222 (U3, B-to-A-0044): stills of the controls panel from this lane's own game process, off screen (-RenderOffScreen), no window grab, no system
# cursor, -dpcvars r.RDG.AsyncCompute=0. Port 4722. Copied from lane A220's driver (tools/qa/screen/a220/a220-modes.ps1). SETTINGS with the new row at
# both sizes, then PLAY NOW with the scripted human (jam.ScriptHuman 2: a person's hand through the button resolver) and the panel's own proof stills
# (jam.hud.PanelStills 1: each frame of a possession swap's fade, the first light of each row), then by console: the pad's names (jam.ui.padlast 1),
# a rebind (jam.ui.rebind Shoot G), OFF (jam.ui.panel 0), the pause card, and the GOAL banner and end card (jam.ctrl.EndMatch 1).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a222\a222-panel.ps1 -Tag after
# The game's pid goes to Game\logs\a222\<Tag>-game.pid; the stills to tools\qa\screen\out\a222-<Tag>\ ; the JAMHUD panel lines to panel-lines.txt there.
# Exit 0: every still asked for exists. 1: one is missing. 2: could not run.
param([string]$Tag = 'after', [int]$Port = 4722, [int]$PlaySec = 40, [int]$StartTimeoutSec = 900)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL a222-modes: no project or no $exe"; exit 2 }
$out = Join-Path $root "tools\qa\screen\out\a222-$Tag"
New-Item -ItemType Directory -Force $out | Out-Null
Get-ChildItem $out -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter "a222-*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=1920 -ResY=1080 -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0,jam.ScriptHuman=2,jam.hud.PanelStills=1 -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
New-Item -ItemType Directory -Force (Join-Path $root 'Game\logs\a222') | Out-Null
Set-Content (Join-Path $root "Game\logs\a222\$Tag-game.pid") "$($p.Id) $($start.ToString('o'))"
function Note([string]$s) { Write-Host ('{0,7:n1}s  {1}' -f $sw.Elapsed.TotalSeconds, $s) }
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
function Still([string]$name, [int]$settleMs = 900) {
    Start-Sleep -Milliseconds $settleMs
    $n = "a222-$Tag-$name"
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
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMHUD.*panel|JAMMENU|JAMSETTINGS' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'panel-lines.txt')
    Get-ChildItem $shotDir -Filter 'a222-panel-*.png' -ErrorAction SilentlyContinue | ForEach-Object { Copy-Item $_.FullName (Join-Path $out $_.Name) -Force } }
    if ($code -eq 0 -and ($missing -gt 0 -or $want.Count -eq 0)) { $code = 1; $why = "$missing stills missing of $($want.Count)" }
    Write-Host ("{0} a222-panel {1} ({2}; {3} stills in {4})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Tag, $why, ($want.Count - $missing), $out)
    exit $code
}
Note "started pid $($p.Id) port $Port tag $Tag"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.menuscreen main' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Note 'the console answers; letting the menu draw'
Start-Sleep -Seconds 8
foreach ($res in @(@{ n = '1080'; s = '1920x1080w' }, @{ n = '720'; s = '1280x720w' })) {
    [void](Console "r.SetRes $($res.s)"); Start-Sleep -Seconds 2
    [void](Console 'jam.menuscreen settings'); Still "$($res.n)-settings" 1500
    [void](Console 'jam.menuscreen where')
}
[void](Console 'r.SetRes 1920x1080w'); Start-Sleep -Seconds 2
[void](Console 'jam.menuscreen main'); Start-Sleep -Milliseconds 600
[void](Console 'jam.menuscreen play')
Note 'PLAY NOW pressed; the scripted human plays'
Start-Sleep -Seconds 12
Still '1080-play-a' 300
$half = [int]($PlaySec / 2)
for ($i = 0; $i -lt $half; $i += 3) { Still ('1080-play-t{0:d2}' -f $i) 2500 }
[void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
for ($i = 0; $i -lt $half; $i += 3) { Still ('720-play-t{0:d2}' -f $i) 2500 }
foreach ($res in @(@{ n = '720'; s = '1280x720w' }, @{ n = '1080'; s = '1920x1080w' })) {
    [void](Console "r.SetRes $($res.s)"); Start-Sleep -Seconds 2
    [void](Console 'jam.ui.hold LeftMouseButton 1'); Still "$($res.n)-held-lmb" 400
    [void](Console 'jam.ui.hold LeftMouseButton 0')
    [void](Console 'jam.ui.hold LeftShift 1'); [void](Console 'jam.ui.hold W 1'); Still "$($res.n)-held-sprint" 400
    [void](Console 'jam.ui.hold LeftShift 0'); [void](Console 'jam.ui.hold W 0')
    [void](Console 'jam.ui.padlast 1'); Still "$($res.n)-pad" 120
    [void](Console 'jam.ui.padlast 0')
    [void](Console 'jam.ui.rebind Shoot G'); [void](Console 'jam.ui.rebind Spin RightShift'); Still "$($res.n)-rebind" 600
    [void](Console 'jam.ui.rebind reset')
    [void](Console 'jam.ui.panel 0'); Still "$($res.n)-off" 600
    [void](Console 'jam.ui.panel 1'); Still "$($res.n)-on-again" 600
    [void](Console 'jam.pausemenu open'); Still "$($res.n)-pause" 900
    [void](Console 'jam.pausemenu close'); Start-Sleep -Milliseconds 600
}
[void](Console 'jam.ctrl.EndMatch 1')
for ($i = 0; $i -lt 6; $i++) { Still ('1080-end-{0:d2}' -f $i) 700 }
Finish 0 'panel stills taken'
