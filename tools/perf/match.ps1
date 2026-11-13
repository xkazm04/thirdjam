<#
.SYNOPSIS
  Lane A58: one profiled bot match in the packaged Development build. Menu -> Enter -> match on a perf cut (tools/perf/cuts),
  the human scripted (jam.ScriptHuman 1, so every skater plays itself), CSV profile over the match, memreport -full at match start and end,
  optional Insights trace, optional jam.livecut leak check. Stops only the process it started.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\perf\match.ps1 -Exe Game\Packaged\A58-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Tier l2_night -Trace

.NOTES
  Output: tools\perf\out\<Name>\ game.log, events.log (wall clock of every step), run.json, csv\*.csv, memreport\*.memreport, trace.utrace.
  Console commands go through UE 5.9's TCP remote console (-RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=<port>, Development only; JSON lines,
  Launch/Private/TcpRemoteConsole.cpp): no keyboard, no foreground needed. The menu's PLAY is one Return posted to the game window (post.ps1).
  Every command's {"op":"exec","ok":...} answer is written to events.log.
#>
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$Tier,
    [string]$Name = '',
    [int]$MatchSeconds = 180,
    [switch]$Trace,
    [string]$TraceChannels = 'cpu,gpu,frame,bookmark,loadtime,memtag',
    [int]$LiveCuts = 0,
    [string]$LiveCutTier = 'p3_clay',
    [switch]$NoMemreport,
    [int]$ResX = 1920, [int]$ResY = 1080,
    [int]$Port = 4658
)
$wt = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$Exe = (Resolve-Path $Exe).Path
if (-not $Name) { $Name = "$Tier-" + (Get-Date -Format 'HHmmss') }
$out = Join-Path $wt "tools\perf\out\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$ev = Join-Path $out 'events.log'
$sw = [Diagnostics.Stopwatch]::StartNew()
function Note([string]$s) { $l = "{0} t={1:0.000} {2}" -f (Get-Date -Format 'HH:mm:ss.fff'), $sw.Elapsed.TotalSeconds, $s; Add-Content $ev $l; Write-Host $l }
$cut = (Join-Path $wt "tools\perf\cuts\perf.$Tier.cartridge.json") -replace '\\', '/'
$liveCut = (Join-Path $wt "tools\perf\cuts\perf.$LiveCutTier.cartridge.json") -replace '\\', '/'
$back = $cut
$log = Join-Path $out 'game.log'
function OtherWindows { @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 -and $_.Id -ne $script:gamePid -and ($_.ProcessName -match '^(ThirdJam|UnrealEditor|UnrealGame|UnrealInsights)') } | ForEach-Object { "$($_.ProcessName):$($_.Id)" }) }
$script:gamePid = -1
$others = OtherWindows
Note "other game/editor windows at start: $($others -join ' ')"
$a = "-cut=`"$cut`" -dpcvars=jam.ScriptHuman=1,r.GPUCsvStatsEnabled=1 -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40 -log -abslog=`"$log`" -statnamedevents -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
if ($Trace) { $a += " -trace=$TraceChannels -tracefile=`"$(Join-Path $out 'trace.utrace')`"" }
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
$script:gamePid = $p.Id
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; tier = $Tier; lane = 'A58'; others = $others } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
Note "start pid $($p.Id) $a"

function LogHas([string]$pat) {
    if (-not (Test-Path $log)) { return $false }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $txt = $sr.ReadToEnd(); $sr.Close() } catch { return $false }
    return $txt.Contains($pat)
}
function WaitLog([string]$pat, [int]$sec) {
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $sec) { if ($p.HasExited) { return $false }; if (LogHas $pat) { return $true }; Start-Sleep -Milliseconds 200 }
    return $false
}
function Hwnd { $p.Refresh(); $p.MainWindowHandle }
$script:rcId = 1
function Console([string]$cmd, [int]$waitSec = 15) {
    for ($try = 1; $try -le 3; $try++) {
        try {
            $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $waitSec * 1000; $c.Connect('127.0.0.1', $Port)
            $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
            $w.WriteLine('{"id":1,"op":"hello","version":1}')
            $null = $r.ReadLine()
            $script:rcId++; $id = $script:rcId
            $w.WriteLine((@{ id = $id; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress))
            $ans = $null; $t0 = $sw.Elapsed.TotalSeconds
            while ($sw.Elapsed.TotalSeconds - $t0 -lt $waitSec) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith("{`"id`":$id,")) { $ans = $l; break } }
            $c.Close()
            if ($ans -and $ans.Contains('"ok":true')) { Note "console '$cmd' OK (try $try) $ans"; return $true }
            Note "console '$cmd' answer (try $try): $ans"
        } catch { Note "console '$cmd' try ${try}: $($_.Exception.Message)" }
        Start-Sleep -Seconds 1
    }
    return $false
}

# 1. Menu up -> Enter.
if (-not (WaitLog '/Game/Maps/Menu' 120)) { Note 'menu map never logged' }
Note 'menu map logged'
Start-Sleep -Seconds 5
$entered = $false
for ($i = 0; $i -lt 5 -and -not $entered; $i++) {
    & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'post.ps1') -ProcessId $p.Id -Key Enter; Note 'enter posted'
    $entered = WaitLog 'JAMMENU start match' 6
}
if ($entered) { Note 'JAMMENU start match seen' } else { Note 'menu Enter did not start a match; console fallback: open /Game/Maps/Rink'; Console 'open /Game/Maps/Rink' | Out-Null }
if (WaitLog 'JAMCART loaded' 120) { Note 'JAMCART loaded seen' } else { Note 'FAIL: no JAMCART loaded' }
if (WaitLog 'JAMSIM frame=600 ' 90) { Note 'sim frame 600 seen (10 s into the match)' } else { Note 'no JAMSIM frame=600' }
if (-not $NoMemreport) { Console 'memreport -full' 40 | Out-Null; Start-Sleep -Seconds 3 }
Console 'csvprofile start' | Out-Null
if ($Trace) { Console 'trace.bookmark A58_match_start' | Out-Null }
Note "match profiling window: $MatchSeconds s"
$t0 = $sw.Elapsed.TotalSeconds
while ($sw.Elapsed.TotalSeconds - $t0 -lt $MatchSeconds) { if ($p.HasExited) { Note 'game exited during the match'; break }; Start-Sleep -Seconds 1 }
Console 'csvprofile stop' | Out-Null
Start-Sleep -Seconds 3
if (-not $NoMemreport) { Console 'memreport -full' 40 | Out-Null; Start-Sleep -Seconds 3 }
for ($i = 1; $i -le $LiveCuts; $i++) {
    $target = if ($i % 2 -eq 1) { $liveCut } else { $back }
    Console "jam.livecut $target" 10 | Out-Null
    Start-Sleep -Seconds 8
    Console 'memreport -full' 40 | Out-Null
    Start-Sleep -Seconds 3
}
if ($Trace) { Console 'trace.stop' | Out-Null; Start-Sleep -Seconds 3 }
Console 'quit' 3 | Out-Null
for ($i = 0; $i -lt 60 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
if (-not $p.HasExited) {
    . (Join-Path $wt 'tools\run\common.ps1')
    [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start); Note 'stopped our game process tree (quit did not exit it)'
} else { Note "game exited, code $($p.ExitCode)" }
# Collect the CSV and memreports written during this run (Saved of the packaged project).
$saved = Join-Path (Split-Path (Split-Path (Split-Path $Exe -Parent) -Parent) -Parent) 'Saved'
foreach ($kind in @(@('Profiling\CSV', '*.csv', 'csv'), @('Profiling\MemReports', '*.memreport', 'memreport'))) {
    $src = Join-Path $saved $kind[0]
    if (Test-Path $src) {
        $dst = Join-Path $out $kind[2]; New-Item -ItemType Directory -Force $dst | Out-Null
        Get-ChildItem $src -Recurse -Filter $kind[1] | Where-Object { $_.LastWriteTime -ge $start } | Sort-Object LastWriteTime | ForEach-Object {
            $n = $_.Name; if ($kind[2] -eq 'memreport') { $n = $_.Directory.Name + '_' + $_.Name }
            Copy-Item $_.FullName (Join-Path $dst $n); Note "collected $n" }
    } else { Note "no $src" }
}
Note "other game/editor windows at end: $((OtherWindows) -join ' ')"
Note 'done'
