<#
.SYNOPSIS
  Starts the Unreal editor on a project (GUI, -nosplash, MCP server on), optionally as a watchdog; or stops the editor it started.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project C:\jam\Game\Game.uproject
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project C:\jam\Game -Watch -MaxRestarts 5
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project C:\jam\Game -Stop

.NOTES
  - Without -Watch: starts the editor, records PID and start time in <project>\logs\editor.json, returns at once
    (or after the MCP server answers, with -WaitMcp).
  - With -Watch: stays in the foreground; when the editor exits without a stop request it waits -PauseSeconds and
    restarts it, at most -MaxRestarts times; every start and exit is appended to <project>\logs\editor-watch.log.
    The watchdog ends after -MaxHours (explicit timeout) or when -Stop is requested.
  - -Stop: writes <project>\logs\editor.stop (so a watchdog does not restart), asks the editor to close, and after
    -StopGraceSec force-stops only the process tree whose PID and start time are in editor.json. Nothing else is touched.
  - The MCP server is started by -ModelContextProtocolStartServer (and by bAutoStartServer=True in
    Config\DefaultEditorPerProjectUserSettings.ini if the project has it).
  Exit: 0 started/stopped/watch ended cleanly, 1 failed (crash loop, MCP not ready, could not stop), 2 could not run.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [switch]$Watch,
    [switch]$Stop,
    [switch]$WaitMcp,
    [int]$McpTimeoutSec = 600,
    [int]$McpPort = 8000,
    [int]$MaxRestarts = 5,
    [int]$PauseSeconds = 15,
    [double]$MaxHours = 100,
    [int]$StopGraceSec = 30,
    [string]$Ddc = '',
    [string]$ExtraArgs = '',
    [double]$MinFreeGB = -1
)
. "$PSScriptRoot\common.ps1"

$uproject = Resolve-UProject $Project
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject at '$Project'"; Exit-WithResult 'editor' 2 }
$projDir = Split-Path $uproject -Parent
$logDir = Get-LogDir $projDir
$stateFile = Join-Path $logDir 'editor.json'
$stopFile = Join-Path $logDir 'editor.stop'
$watchLog = Join-Path $logDir 'editor-watch.log'

function Write-WatchLog([string]$m) { $l = ('{0} {1}' -f (Get-Date -Format 's'), $m); Add-Content -Path $watchLog -Value $l; Write-Host "INFO $m" }

function Read-State {
    if (-not (Test-Path $stateFile)) { return $null }
    try { return (Get-Content $stateFile -Raw | ConvertFrom-Json) } catch { return $null }
}

function Stop-Editor {
    New-Item -ItemType File -Force -Path $stopFile | Out-Null
    $s = Read-State
    if (-not $s) { Write-Step 'PASS' 'stop' 'no editor recorded as started by these scripts'; return $true }
    $start = [datetime]::Parse($s.start)
    $p = Get-Process -Id $s.pid -ErrorAction SilentlyContinue
    if (-not $p) { Write-Step 'PASS' 'stop' "pid $($s.pid) already gone"; return $true }
    if ([math]::Abs(($p.StartTime - $start).TotalSeconds) -gt 2) { Write-Step 'PASS' 'stop' "pid $($s.pid) now belongs to another process; not touched"; return $true }
    [void]$p.CloseMainWindow()
    if ($p.WaitForExit($StopGraceSec * 1000)) { Write-Step 'PASS' 'stop' "editor pid $($s.pid) closed"; return $true }
    if (Stop-OwnedTree -ProcessId $s.pid -StartTime $start) { Write-Step 'PASS' 'stop' "editor pid $($s.pid) force-stopped after ${StopGraceSec}s"; return $true }
    Write-Step 'FAIL' 'stop' "editor pid $($s.pid) did not stop"
    return $false
}

if ($Stop) { if (Stop-Editor) { Exit-WithResult 'editor' 0 } else { Exit-WithResult 'editor' 1 } }

$engine = Get-EngineRoot $uproject
if (-not $engine) { Write-Step 'FAIL' 'engine' 'Unreal Engine install not found (set UE_ROOT)'; Exit-WithResult 'editor' 2 }
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path $exe)) { Write-Step 'FAIL' 'engine' "missing $exe"; Exit-WithResult 'editor' 2 }
$floor = Get-MinFreeGB $MinFreeGB
$free = Get-FreeGB $uproject
if ($free -lt $floor) { Write-Step 'FAIL' 'disk' "free $free GB is below the floor of $floor GB"; Exit-WithResult 'editor' 2 }

$s = Read-State
if ($s) {
    $p = Get-Process -Id $s.pid -ErrorAction SilentlyContinue
    if ($p -and [math]::Abs(($p.StartTime - [datetime]::Parse($s.start)).TotalSeconds) -le 2) {
        Write-Step 'FAIL' 'start' "an editor started by these scripts is already running (pid $($s.pid)); use -Stop first"
        Exit-WithResult 'editor' 2
    }
}
$listener = Get-NetTCPConnection -LocalPort $McpPort -State Listen -ErrorAction SilentlyContinue | Select-Object -First 1
if ($listener) {
    Write-Step 'FAIL' 'port' "port $McpPort is already in use by pid $($listener.OwningProcess); another editor or service owns it"
    Exit-WithResult 'editor' 2
}
if (Test-Path $stopFile) { Remove-Item $stopFile -Force }

function Start-Editor([int]$n) {
    $log = Join-Path $logDir ("editor-{0}-{1}.log" -f (Get-Stamp), $n)
    $a = ('"{0}" -nosplash -ModelContextProtocolStartServer -ModelContextProtocolPort={1} -abslog="{2}"' -f $uproject, $McpPort, $log)
    if ($Ddc) { $a += " -DDC=$Ddc" }
    if ($ExtraArgs) { $a += ' ' + $ExtraArgs }
    $p = Start-Process -FilePath $exe -ArgumentList $a -PassThru
    $null = $p.Handle
    $state = [ordered]@{ pid = $p.Id; start = $p.StartTime.ToString('o'); log = $log; args = $a; port = $McpPort }
    ($state | ConvertTo-Json -Compress) | Set-Content -Path $stateFile -Encoding ascii
    Write-WatchLog ("start #{0} pid {1} log {2}" -f $n, $p.Id, $log)
    return $p
}

function Wait-Mcp {
    $node = (Get-Command node -ErrorAction SilentlyContinue)
    if (-not $node) { Write-Step 'FAIL' 'mcp' 'node not found on PATH'; return $false }
    $mcp = Join-Path (Split-Path $PSScriptRoot -Parent) 'mcp\mcp.mjs'
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $last = ''
    # Short polls so an editor that died (or sits on a modal dialog) is noticed between them.
    while ($sw.Elapsed.TotalSeconds -lt $McpTimeoutSec) {
        if ($proc.HasExited) { Write-Step 'FAIL' 'mcp' ("editor exited (code {0}) before the server answered" -f $proc.ExitCode); return $false }
        $out = & node $mcp wait --quiet --url "http://127.0.0.1:$McpPort/mcp" --timeout 15
        $code = $LASTEXITCODE
        $last = @($out)[-1]
        if ($code -eq 0) { Write-Step 'PASS' 'mcp' ("server answered after {0:N0}s" -f $sw.Elapsed.TotalSeconds); return $true }
        if ($code -eq 1) { break }
        $proc.Refresh()
        if ($proc.MainWindowTitle -eq 'Message') { Write-Host 'INFO mcp: the editor shows a modal "Message" dialog; it blocks start-up (see the editor log)' }
    }
    Write-Step 'FAIL' 'mcp' ("not ready after {0:N0}s: {1}" -f $sw.Elapsed.TotalSeconds, $last)
    return $false
}

$sw = [System.Diagnostics.Stopwatch]::StartNew()
$proc = Start-Editor 0
$extra = @{ pid = $proc.Id; port = $McpPort; state = $stateFile }

if (-not $Watch) {
    Write-Step 'PASS' 'start' "editor pid $($proc.Id)"
    if ($WaitMcp) {
        $ok = Wait-Mcp
        $extra['mcp_ready_s'] = [math]::Round($sw.Elapsed.TotalSeconds, 1)
        if (-not $ok -and $proc.HasExited) { Write-Step 'FAIL' 'editor' "editor exited with code $($proc.ExitCode) while waiting for MCP" }
    }
    Exit-WithResult 'editor' -1 $extra
}

# Watchdog loop.
$restarts = 0
$deadline = (Get-Date).AddHours($MaxHours)
while ($true) {
    while (-not $proc.HasExited) {
        Start-Sleep -Seconds 5
        if ((Get-Date) -gt $deadline) { Write-WatchLog "watch limit of $MaxHours h reached; stopping the editor"; [void](Stop-Editor); break }
    }
    $proc.WaitForExit(10000) | Out-Null
    $code = $null; try { $code = $proc.ExitCode } catch {}
    if (Test-Path $stopFile) { Write-WatchLog "editor exited (code $code) after a stop request; watchdog ends"; break }
    if ((Get-Date) -gt $deadline) { break }
    Write-WatchLog "editor exited unexpectedly (code $code)"
    if ($restarts -ge $MaxRestarts) { Write-Step 'FAIL' 'watch' "editor exited $($restarts + 1) times; restart limit $MaxRestarts reached"; Exit-WithResult 'editor' 1 @{ restarts = $restarts } }
    if ((Get-FreeGB $uproject) -lt $floor) { Write-Step 'FAIL' 'watch' 'disk below floor; not restarting'; Exit-WithResult 'editor' 1 @{ restarts = $restarts } }
    Start-Sleep -Seconds $PauseSeconds
    if (Test-Path $stopFile) { Write-WatchLog 'stop requested during the pause; watchdog ends'; break }
    $restarts++
    $proc = Start-Editor $restarts
}
Write-Step 'PASS' 'watch' "watchdog ended; $restarts restart(s)"
Exit-WithResult 'editor' 0 @{ restarts = $restarts }
