<#
.SYNOPSIS
  Lane A106 (track Q3): one profiled bot match in a packaged Development build with no posted input and no cursor. Like start.ps1 (lane A65), but the
  menu -> match step is the remote console's `open /Game/Maps/Rink` (the cut is on the command line, jam.ScriptHuman=1 so all skaters are bots), not a
  Return posted to the window. The CSV profiler runs from the menu through the menu -> match frame and -PlaySeconds of play (startframes.mjs reads
  the load frame; analyze-csv.mjs / five.mjs the play). Optional Insights trace over the same window. Stops only the process it started.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\perfot.ps1 -Exe Game\Packaged\A106-dev\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe -Tier l2_night_5v5 -PlaySeconds 120

.NOTES
  Output: tools\perf\out\<Name>\ game.log, events.log, run.json, csv\*.csv, trace.utrace. Port 4706 (lane A106's own).
#>
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [Parameter(Mandatory = $true)][string]$Tier,
    [string]$Name = '',
    [int]$WaitMenu = 8,
    [string]$Dp = '',
    [switch]$Memreport,
    [string]$Shot = '',
    [int]$PlaySeconds = 120,
    [switch]$Trace,
    [string]$TraceChannels = 'cpu,gpu,frame,bookmark',
    [int]$ResX = 1920, [int]$ResY = 1080,
    [int]$Port = 4706,
    # Lane A150: 'open' (default, lane A106: the remote console's open /Game/Maps/Rink) or 'play' (PLAY NOW's own path: jam.menuscreen play).
    [ValidateSet('open', 'play')][string]$StartPath = 'open',
    # Lane A150: no -cut on the command line (the operator's PLAY NOW: hockey.regular from the menu's own guess).
    [switch]$NoCut
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
$log = Join-Path $out 'game.log'
function OtherWindows { @(Get-Process -ErrorAction SilentlyContinue | Where-Object { $_.MainWindowHandle -ne 0 -and $_.Id -ne $script:gamePid -and ($_.ProcessName -match '^(ThirdJam|UnrealEditor|UnrealGame|UnrealInsights)') } | ForEach-Object { "$($_.ProcessName):$($_.Id)" }) }
$script:gamePid = -1
$others = OtherWindows
Note "other game/editor windows at start: $($others -join ' ')"
$cutArg = if ($NoCut) { '' } else { "-cut=`"$cut`" " }
$a = "$cutArg-dpcvars=jam.ScriptHuman=1,r.GPUCsvStatsEnabled=1$(if ($Dp) { ',' + $Dp }) -windowed -ResX=$ResX -ResY=$ResY -WinX=40 -WinY=40 -log -abslog=`"$log`" -statnamedevents -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
if ($Trace) { $a += " -trace=$TraceChannels -tracefile=`"$(Join-Path $out 'trace.utrace')`"" }
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
$script:gamePid = $p.Id
@{ pid = $p.Id; start = $start.ToString('o'); exe = $Exe; args = $a; tier = $Tier; lane = 'A106'; dp = $Dp; startpath = $StartPath; nocut = [bool]$NoCut; others = $others } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
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

# 1. Menu up, a fixed wait (the preload of lane A65 runs in it), CSV on, Enter.
if (-not (WaitLog '/Game/Maps/Menu' 120)) { Note 'menu map never logged' }
Note 'menu map logged'
Start-Sleep -Seconds $WaitMenu
Console 'csvprofile start' | Out-Null
if ($Trace) { Console 'trace.bookmark A106_open' | Out-Null }
$startCmd = if ($StartPath -eq 'play') { 'jam.menuscreen play' } else { 'open /Game/Maps/Rink' }
if (Console $startCmd 20) { Note "$startCmd sent (no posted input)" } else { Note "FAIL: $startCmd did not answer" }
if (WaitLog 'JAMCART loaded' 120) { Note 'JAMCART loaded seen' } else { Note 'FAIL: no JAMCART loaded' }
if (WaitLog 'JAMSIM frame=600 ' 90) { Note 'sim frame 600 seen (10 s into the match)' } else { Note 'no JAMSIM frame=600' }
if ($PlaySeconds -le 0) { Console 'csvprofile stop' | Out-Null; Start-Sleep -Seconds 3 }
if ($Shot) { & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot 'shot.ps1') -ProcessId $p.Id -Out $Shot | ForEach-Object { Note $_ } }
if ($Memreport) { Console 'memreport -full' 40 | Out-Null; Start-Sleep -Seconds 3 }
if ($PlaySeconds -gt 0) {
    Note "play window: $PlaySeconds s"
    $t0 = $sw.Elapsed.TotalSeconds
    while ($sw.Elapsed.TotalSeconds - $t0 -lt $PlaySeconds) { if ($p.HasExited) { Note 'game exited during the match'; break }; Start-Sleep -Seconds 1 }
    Console 'csvprofile stop' | Out-Null
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
