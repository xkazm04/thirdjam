# Lane A217: copy of lane A211's tools/qa/eye/eye-hash.ps1 with a -Map (rookie: /Game/Maps/Rink?level=rookie with -Cut ''), port 4817 and output under Game\logs\a217\game\<run>.
# One editor-binary GAME process of this tree, headless (-nullrhi), jam.ScriptHuman=2: the game's own JAMSIM hashes, to compare with -run=JamHandCells -hashes.
# Exit 0: the frame was reached (or the match ended). 1: not. 2: could not run. The process id is in game.json and in Game\logs\a217\pids.txt.
param(
    [int]$ScriptHuman = 1,
    [int]$Frames = 1800,
    [string]$Cut = 'hockey.regular',
    [string]$Map = '/Game/Maps/Rink',
    [string]$Run = '',
    [int]$Port = 4817,
    [string]$ExtraCVars = '',
    [int]$TimeoutSec = 600
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL eye-hash: this tool runs only from a worktree (root $root)"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL eye-hash: no project or no $exe"; exit 2 }
if (-not $Run) { $Run = "hash$ScriptHuman-" + (Get-Date -Format 'HHmmss') }
$out = Join-Path $root "Game\logs\a217\game\$Run"
New-Item -ItemType Directory -Force $out | Out-Null
$log = Join-Path $out 'game.log'
Remove-Item $log -ErrorAction SilentlyContinue
$cvars = "jam.ScriptHuman=$ScriptHuman,jam.screen.LightHold=0"
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$a = "`"$uproject`" $Map -game -nullrhi -Multiprocess -unattended -nosplash -nosound -UseFixedTimeStep -FPS=60 -log -abslog=`"$log`" -ForceLogFlush " +
     $(if ($Cut) { "-cut=$Cut " } else { "" }) + "-dpcvars=$cvars -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=$Port"
$sw = [Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
Add-Content (Join-Path $root "Game\logs\a217\pids.txt") ("game " + $p.Id + " " + $Run + " " + (Get-Date -Format HH:mm))
$start = $p.StartTime
@{ pid = $p.Id; start = $start.ToString('o'); args = $a; lane = 'A217' } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'game.json')
function LogText {
    if (-not (Test-Path $log)) { return '' }
    try { $fs = [IO.File]::Open($log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t } catch { return '' }
}
function Console([string]$cmd) {
    try {
        $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = 3000; $c.Connect('127.0.0.1', $Port)
        $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
        $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
        $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $cmd } | ConvertTo-Json -Compress)); $null = $r.ReadLine(); $c.Close()
    } catch { }
}
$ok = $false
while ($sw.Elapsed.TotalSeconds -lt $TimeoutSec -and -not $p.HasExited) {
    $t = LogText; if ($t -match "JAMSIM frame=$Frames hash=" -or $t -match "JAMSIM frame=\d+ hash=\S+ score=\S+ phase=2") { $ok = $true; break }
    Start-Sleep -Milliseconds 500
}
if (-not $p.HasExited) { Console 'quit' }
for ($i = 0; $i -lt 40 -and -not $p.HasExited; $i++) { Start-Sleep -Milliseconds 500 }
if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start) }
$lines = @([regex]::Matches((LogText), 'JAM(SIM frame=\d+ hash=\S+ score=\S+ phase=\d|MATCH level=\S+ source=\S+ cut=\S+|SCRIPT human=2[^\r\n]*)') | ForEach-Object { $_.Value })
$lines | Set-Content (Join-Path $out 'hashes.txt')
$lines | ForEach-Object { Write-Host "  | $_" }
Write-Host ("{0} eye-hash {1} (jam.ScriptHuman {2}, {3} hash lines, {4:n0} s)" -f $(if ($ok) { 'PASS' } else { 'FAIL' }), $Run, $ScriptHuman, $lines.Count, $sw.Elapsed.TotalSeconds)
if ($ok) { exit 0 } else { exit 1 }
