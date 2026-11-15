# Lane A231 (U6): lane A220's MODES stills driver with this lane's port and folders: stills of MODES (the list, the preview with the mode's illustration) from this lane's own game process, off screen,
# no window grab, no system cursor, -dpcvars r.RDG.AsyncCompute=0. Port 4731. Driven through the remote console by the menu's QA command
# (jam.menuscreen modes | key down | key up | key w | key s | wheel <n> | preview) and the still command (jam.screen.Still <name>). Both sizes in one process.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\a231\a231-modes.ps1 -Tag after
#   -Before: the screen as it was (the cards): the opening at both sizes only.
# The game's pid goes to Game\logs\a231\<Tag>-game.pid; the stills to tools\qa\screen\out\a231-<Tag>\ ; the JAMMENU lines to menu-lines.txt there.
# Exit 0: every still asked for exists. 1: one is missing. 2: could not run.
param([string]$Tag = 'after', [int]$Port = 4731, [switch]$Before, [int]$StartTimeoutSec = 900)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL a231-modes: no project or no $exe"; exit 2 }
$out = Join-Path $root "tools\qa\screen\out\a231-$Tag"
New-Item -ItemType Directory -Force $out | Out-Null
Get-ChildItem $out -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
Get-ChildItem $shotDir -Filter "a231-$Tag-*.png" -ErrorAction SilentlyContinue | Remove-Item -Force
$a = "`"$uproject`" /Game/Maps/Menu -game -RenderOffScreen -Multiprocess -ResX=1920 -ResY=1080 -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
     "-dpcvars=r.RDG.AsyncCompute=0,jam.screen.FadeSeconds=0 -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$start = $p.StartTime
New-Item -ItemType Directory -Force (Join-Path $root 'Game\logs\a231') | Out-Null
Set-Content (Join-Path $root "Game\logs\a231\$Tag-game.pid") "$($p.Id) $($start.ToString('o'))"
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
    $n = "a231-$Tag-$name"
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
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMMENU' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'menu-lines.txt') }
    if ($code -eq 0 -and ($missing -gt 0 -or $want.Count -eq 0)) { $code = 1; $why = "$missing stills missing of $($want.Count)" }
    Write-Host ("{0} a231-modes {1} ({2}; {3} stills in {4})" -f $(if ($code -eq 0) { 'PASS' } else { 'FAIL' }), $Tag, $why, ($want.Count - $missing), $out)
    exit $code
}
Note "started pid $($p.Id) port $Port tag $Tag"
$t0 = $sw.Elapsed.TotalSeconds; $up = $false
while ($sw.Elapsed.TotalSeconds - $t0 -lt $StartTimeoutSec -and -not $p.HasExited -and -not $up) { $up = Console 'jam.menuscreen main' 3 -Quiet; if (-not $up) { Start-Sleep -Seconds 2 } }
if (-not $up) { Finish 2 'the menu never answered its console' }
Note 'the console answers; letting the menu draw'
Start-Sleep -Seconds 8
if ($Before) {
    foreach ($res in @(@{ n = '1080'; s = '1920x1080w' }, @{ n = '720'; s = '1280x720w' })) {
        [void](Console "r.SetRes $($res.s)"); Start-Sleep -Seconds 2
        [void](Console 'jam.menuscreen modes'); Still "$($res.n)-open" 1500
    }
    Finish 0 'the cards as they were'
}
$rows = 14
foreach ($res in @(@{ n = '1080'; s = '1920x1080w' }, @{ n = '720'; s = '1280x720w' })) {
    [void](Console "r.SetRes $($res.s)"); Start-Sleep -Seconds 2
    [void](Console 'jam.menuscreen main'); Start-Sleep -Milliseconds 400
    [void](Console 'jam.menuscreen modes'); Start-Sleep -Milliseconds 600
    [void](Console 'jam.menuscreen preview')
    Still "$($res.n)-open" 1200
    # The pad walks the list top to bottom (D-pad down): a still of every row at 1080; at 720 the middle and the last.
    for ($i = 1; $i -lt $rows; $i++) {
        [void](Console 'jam.menuscreen key down'); Start-Sleep -Milliseconds 250
        [void](Console 'jam.menuscreen preview')
        if ($res.n -eq '1080') { Still ('1080-pad-{0:d2}' -f ($i + 1)) 500 }
        elseif ($i -eq 6 -or $i -eq $rows - 1) { Still ('720-pad-{0:d2}' -f ($i + 1)) 600 }
    }
    # One more down: BACK (the preview holds the last mode).
    [void](Console 'jam.menuscreen key down'); [void](Console 'jam.menuscreen preview'); Still "$($res.n)-back" 600
    # Up to the last row, the wheel three rows up, then W once and S twice.
    [void](Console 'jam.menuscreen key up'); [void](Console 'jam.menuscreen wheel 3'); [void](Console 'jam.menuscreen preview'); Still "$($res.n)-wheel-up3" 600
    [void](Console 'jam.menuscreen key w'); [void](Console 'jam.menuscreen key s'); [void](Console 'jam.menuscreen key s'); [void](Console 'jam.menuscreen preview')
}
[void](Console 'r.SetRes 1920x1080w'); Start-Sleep -Seconds 1
[void](Console 'jam.menuscreen where'); Start-Sleep -Milliseconds 500
Finish 0 'modes stills taken'
