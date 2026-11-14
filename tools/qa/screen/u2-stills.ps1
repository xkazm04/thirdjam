# Lane A144 (track U2, the menu): stills of every menu screen from this lane's own game process, off screen, with no window grab and no system cursor.
# One editor-binary game process on the Menu map (-RenderOffScreen), driven through its remote console (port 4744) by the menu's own QA command
# (jam.menuscreen main|cuts|direct|settings|controls|credits|say <sentence>), the lobby's (jam.lobby, jam.host, jam.leave) and the still command
# (jam.screen.Still <name>). Both resolutions in one process (r.SetRes). The stills are copied to tools\qa\screen\out\u2-<Tag>\.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag after
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag end -EndCard     (both end cards, 90 s)
# -EndCard: instead of the menu screens, presses PLAY NOW (jam.menuscreen play), ends the match for the other side by real goals
#   (jam.ctrl.EndMatch 1: the loss card), takes the card at both sizes, restarts the match (jam.Season.Restart, the R key) and ends it for the
#   player's side (jam.ctrl.EndMatch 0: the win card). The JAMMENU play now / JAMCTRL endmatch lines land in menu-lines.txt.
# The process runs with -Multiprocess (the machine's rule for every headless start), and under that flag the engine writes no config file: a level
# picked here lives for this process only. A pick that survives a restart cannot be shown with this tool (Jam.Menu.PickSurvivesSaveAndLoad holds the
# game's Save() and Load(); a package writes the file).
# Exit 0: every still asked for exists. 1: one is missing. 2: could not run.
param(
    [string]$Tag = 'after',
    [int]$Port = 4744,
    [switch]$EndCard,
    [string]$LaunchCut = '',  # start the game with -cut=<this>, as the acceptance run does: PLAY NOW must then start this cut at its own level (JAMMATCH ... source=launch)
    [string]$LevelCut = '',  # with -Level: this cut or mode through the menu (jam.menuscreen choose <name>) instead of PLAY NOW
    [string[]]$Before = @(), # console commands run once the menu is up, before the stills (for example "jam.menuscreen level pro")
    [switch]$Switch,       # instead of the stills: the first screen's level switch, stepped by real key presses (jam.menuscreen key right / left), then Enter: the match must start at the level picked
    [switch]$Level,        # instead of the stills: PLAY NOW, then the match card and the pause card, which name the level in force (JAMMATCH level=... in menu-lines.txt)
    [switch]$Walk,         # instead of the stills: the pad walk (jam.menuscreen walk); prints its JAMMENU walk lines; exit 0 when the verdict is pass
    [switch]$Say,          # also direct one sentence on the Direct screen: with an API key on the machine this is a live, paid call
    [int]$ScriptHuman = 1,
    [int]$EndTimeoutSec = 120,
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
if ($LaunchCut) { $a += " -cut=`"$LaunchCut`"" }
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
    if (Test-Path $log) { Select-String -Path $log -Pattern 'JAMMENU|JAMMATCH|JAMSEASON|JAMCTRL endmatch|jam\.host|jam\.join' | ForEach-Object { $_.Line } | Set-Content (Join-Path $out 'menu-lines.txt') }
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

foreach ($cmd in $Before) { if ($cmd) { [void](Console $cmd); Start-Sleep -Milliseconds 300 } }

if ($Switch) {
    # NOTE: this writes the level into this tree's own saved settings (Game\Saved\...\GameUserSettings.ini), as a player's pick does.
    [void](Console 'jam.menuscreen level rookie')
    Still '1080-switch-rookie' 800
    [void](Console 'jam.menuscreen key right')
    Still '1080-switch-pro' 800
    [void](Console 'jam.menuscreen key right')
    Still '1080-switch-allstar' 800
    [void](Console 'jam.menuscreen key right')          # the right end holds
    [void](Console 'jam.menuscreen key left')
    [void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
    Still '720-switch-pro' 800
    [void](Console 'jam.menuscreen key left')
    Still '720-switch-rookie' 800
    [void](Console 'jam.menuscreen key arrowright')     # the keyboard's arrow, as the pad's
    [void](Console 'jam.menuscreen key enter')          # the acceptance run's Return: the focus never left PLAY NOW
    $started = WaitLog 'JAMMATCH level=' 60
    Start-Sleep -Seconds 2
    $lines = @(Select-String -Path $log -Pattern 'JAMMENU key|JAMMENU level picked|JAMMENU start match|JAMMATCH level' | ForEach-Object { $_.Line -replace '^.*(JAMMENU|JAMMATCH)', '$1' })
    $lines | ForEach-Object { Write-Host "  | $_" }
    $lines | Set-Content (Join-Path $out 'switch-lines.txt')
    if (-not $started) { Finish 1 'Enter on the first screen did not start a match' }
    if (@($lines | Where-Object { $_ -match 'MATCH level=pro source=settings' }).Count -eq 0) { Finish 1 'the match did not play the level the switch showed (pro)' }
    Finish 0 'the switch steps by the keys and Enter plays its level'
}

if ($Level) {
    if ($LevelCut) { [void](Console "jam.menuscreen choose $LevelCut") } else { [void](Console 'jam.menuscreen play') }
    if (-not (WaitLog 'JAMMATCH level=' 60)) { Finish 2 'PLAY NOW did not start a match (no JAMMATCH line)' }
    $shown = $false
    for ($i = 0; $i -lt 8 -and -not $shown; $i++) { Start-Sleep -Milliseconds 700; Still "1080-match-card-$i" 0; $shown = $true }
    Still '1080-match-card-late' 900
    [void](Console 'jam.pausemenu open')
    Still '1080-pause' 1200
    [void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
    Still '720-pause' 800
    Finish 0 'the level is on the match card and the pause card'
}

if ($Walk) {
    [void](Console 'jam.menuscreen walk')
    $okw = WaitLog 'JAMMENU walk done' 180
    Still '1080-after-walk' 500
    $lines = @(Select-String -Path $log -Pattern 'JAMMENU walk' | ForEach-Object { $_.Line -replace '^.*JAMMENU walk', 'JAMMENU walk' })
    $lines | ForEach-Object { Write-Host "  | $_" }
    $lines | Set-Content (Join-Path $out 'walk-lines.txt')
    if (-not $okw) { Finish 1 'the walk did not finish' }
    if (@($lines | Where-Object { $_ -match 'walk done .*verdict=blind' }).Count -gt 0) { Finish 2 'could not judge the directions: this off-screen process has no menu in its hit-test grid (run jam.menuscreen walk in a windowed game); the opening focus and B were judged, see the lines' }
    if (@($lines | Where-Object { $_ -match 'walk done .*verdict=pass' }).Count -eq 0) { Finish 1 'the walk found a place the pad cannot reach or leave, or a screen B does not leave' }
    Finish 0 'the pad walked every screen'
}

if ($EndCard) {
    [void](Console 'jam.menuscreen play')
    if (-not (WaitLog 'JAMMENU start match' 30)) { Finish 2 'PLAY NOW did not start a match' }
    foreach ($card in @(@{ n = 'loss'; team = 1 }, @{ n = 'win'; team = 0 })) {
        Start-Sleep -Seconds 12                                   # the rink is up and the first faceoff is over
        $before = if (Test-Path $log) { @(Select-String -Path $log -Pattern 'JAMCTRL endmatch done').Count } else { 0 }
        [void](Console "jam.ctrl.EndMatch $($card.team)")
        $t1 = $sw.Elapsed.TotalSeconds; $done = $false
        while ($sw.Elapsed.TotalSeconds - $t1 -lt $EndTimeoutSec -and -not $p.HasExited -and -not $done) {
            Start-Sleep -Milliseconds 500
            $done = (Test-Path $log) -and (@(Select-String -Path $log -Pattern 'JAMCTRL endmatch done').Count -gt $before)
        }
        if (-not $done) { Still "1080-end-$($card.n)-timeout" 0; Finish 1 "the match did not end for team $($card.team) inside the time box" }
        [void](Console 'r.SetRes 1920x1080w'); Start-Sleep -Seconds 2
        Still "1080-end-$($card.n)" 1500
        [void](Console 'r.SetRes 1280x720w'); Start-Sleep -Seconds 2
        Still "720-end-$($card.n)" 1000
        [void](Console 'r.SetRes 1920x1080w')
        if ($card.n -eq 'loss') { [void](Console 'jam.Season.Restart') }
    }
    Finish 0 'end cards taken'
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
