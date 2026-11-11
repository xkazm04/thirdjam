<#
.SYNOPSIS
  The one-command proof for the start of every working session: build the editor target, start the editor,
  wait for the MCP server, tools/list, spawn an actor and read it back, stop the editor this script started.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\smoke.ps1 -Project C:\jam\Game
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\smoke.ps1 -Project C:\jam\Game -SkipBuild -KeepEditor

.NOTES
  - If an editor started by these scripts is already up and answering MCP, it is reused and left running.
  - The spawned actor is a StaticMeshActor labelled TJ_SmokeProbe at (0,0,500); it is deleted again and the level
    is not saved.
  - Prints PASS/FAIL lines and a final JSON line. Exit 0 all passed, 1 a step failed, 2 could not run.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [switch]$SkipBuild,
    [switch]$KeepEditor,
    [int]$McpTimeoutSec = 900,
    [int]$McpPort = 8000,
    [string]$Ddc = ''
)
. "$PSScriptRoot\common.ps1"

$uproject = Resolve-UProject $Project
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject at '$Project'"; Exit-WithResult 'smoke' 2 }
if (-not (Get-Command node -ErrorAction SilentlyContinue)) { Write-Step 'FAIL' 'node' 'node not found on PATH'; Exit-WithResult 'smoke' 2 }
$mcp = Join-Path (Split-Path $PSScriptRoot -Parent) 'mcp\mcp.mjs'
$url = "http://127.0.0.1:$McpPort/mcp"
$t0 = [System.Diagnostics.Stopwatch]::StartNew()
$timings = [ordered]@{}

function Invoke-Child([string]$script, [string[]]$argv) {
    $out = & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $PSScriptRoot $script) @argv
    $code = $LASTEXITCODE
    $json = $null
    foreach ($l in $out) { if ($l -match '^\{"tool":"thirdjam.run/1"') { $json = $l | ConvertFrom-Json } elseif ($l -match '^(INFO|WARN|  \|)') { Write-Host $l } }
    return @{ code = $code; json = $json; out = $out }
}
function Invoke-Mcp([string[]]$argv, [int]$timeout = 120) {
    $out = & node $mcp @argv --url $url --timeout $timeout
    $code = $LASTEXITCODE
    $status = $null
    $lines = @($out)
    if ($lines.Count -gt 0) { try { $status = $lines[-1] | ConvertFrom-Json } catch {} }
    $body = ($lines | Select-Object -First ([math]::Max(0, $lines.Count - 1))) -join "`n"
    return @{ code = $code; status = $status; body = $body }
}

# 1. Build.
if (-not $SkipBuild) {
    $b = Invoke-Child 'build.ps1' @('-Project', $uproject)
    if ($b.code -eq 0) { Write-Step 'PASS' 'build' ("{0}s" -f $b.json.seconds); $timings['build_s'] = $b.json.seconds }
    else { Write-Step 'FAIL' 'build' ("exit {0}; log {1}" -f $b.code, $b.json.log); Exit-WithResult 'smoke' 1 @{ timings = $timings } }
}

# 2. Editor: reuse ours if it answers, else start one.
$started = $false
$probe = Invoke-Mcp @('wait', '--quiet') 5
if ($probe.code -eq 0) {
    Write-Step 'PASS' 'editor' 'an MCP server already answers; reusing it (not stopped at the end)'
} else {
    $edArgs = @('-Project', $uproject, '-WaitMcp', '-McpTimeoutSec', "$McpTimeoutSec", '-McpPort', "$McpPort")
    if ($Ddc) { $edArgs += @('-Ddc', $Ddc) }
    $e = Invoke-Child 'editor.ps1' $edArgs
    $started = $true
    if ($e.code -ne 0) {
        Write-Step 'FAIL' 'editor' ("editor or MCP did not come up (exit {0})" -f $e.code)
        [void](Invoke-Child 'editor.ps1' @('-Project', $uproject, '-Stop'))
        Exit-WithResult 'smoke' 1 @{ timings = $timings }
    }
    $timings['editor_to_mcp_s'] = $e.json.mcp_ready_s
    Write-Step 'PASS' 'editor' ("pid {0}, MCP ready after {1}s" -f $e.json.pid, $e.json.mcp_ready_s)
}

# 3. tools/list and toolsets.
$l = Invoke-Mcp @('list')
if ($l.code -eq 0) { Write-Step 'PASS' 'tools/list' (("{0} top-level tools" -f (@($l.body -split "`n").Count))) } else { Write-Step 'FAIL' 'tools/list' $l.status.error }
$ts = Invoke-Mcp @('toolsets')
if ($ts.code -eq 0) { Write-Step 'PASS' 'list_toolsets' 'answered' } else { Write-Step 'FAIL' 'list_toolsets' $ts.status.error }

# 4. Spawn an actor and read it back.
# Arguments go through files: PowerShell 5.1 strips double quotes inside native-command arguments.
$argDir = Join-Path (Get-LogDir (Split-Path $uproject -Parent)) 'mcp-args'
if (-not (Test-Path $argDir)) { New-Item -ItemType Directory -Force -Path $argDir | Out-Null }
function Write-Args([string]$name, $obj) { $f = Join-Path $argDir "$name.json"; [System.IO.File]::WriteAllText($f, ($obj | ConvertTo-Json -Depth 10 -Compress)); return "@$f" }

$SPAWN_TOOL = 'EditorToolset.LevelToolset.AddToSceneFromClass'
$READ_TOOL = 'EditorToolset.LevelToolset.FindActors'
$DELETE_TOOL = 'EditorToolset.LevelToolset.RemoveFromScene'
$spawnArgs = Write-Args 'spawn' @{ actorType = @{ refPath = '/Script/Engine.StaticMeshActor' }; name = 'TJ_SmokeProbe'; xform = @{ translation = @{ x = 0; y = 0; z = 500 }; rotation = @{ pitch = 0; yaw = 0; roll = 0 }; scale3D = @{ x = 1; y = 1; z = 1 } } }
$s = Invoke-Mcp @('call', $SPAWN_TOOL, $spawnArgs)
$actorPath = $null
if ($s.code -eq 0) { try { $actorPath = ($s.body | ConvertFrom-Json).structuredContent.returnValue.refPath } catch {} }
if ($s.code -eq 0 -and $actorPath) { Write-Step 'PASS' 'spawn' $actorPath } else { Write-Step 'FAIL' 'spawn' ("{0}: {1}" -f $SPAWN_TOOL, $s.status.error) }
$r = Invoke-Mcp @('call', $READ_TOOL, (Write-Args 'read' @{ filter = @{ name = 'TJ_SmokeProbe' } }))
if ($r.code -eq 0 -and $r.body -match '"label":\s*"TJ_SmokeProbe"' -and $r.body -match '"z":\s*500') { Write-Step 'PASS' 'read-back' 'TJ_SmokeProbe found at z=500' }
elseif ($r.code -eq 0) { Write-Step 'FAIL' 'read-back' 'answer did not contain TJ_SmokeProbe at z=500' }
else { Write-Step 'FAIL' 'read-back' ("{0}: {1}" -f $READ_TOOL, $r.status.error) }
if ($actorPath) {
    $d = Invoke-Mcp @('call', $DELETE_TOOL, (Write-Args 'delete' @{ actor = @{ refPath = $actorPath } }))
    if ($d.code -ne 0) { Write-Host ("INFO cleanup: {0} failed: {1}" -f $DELETE_TOOL, $d.status.error) }
}

# 5. Stop the editor if this script started it.
if ($started -and -not $KeepEditor) {
    $x = Invoke-Child 'editor.ps1' @('-Project', $uproject, '-Stop', '-StopGraceSec', '20')
    if ($x.code -eq 0) { Write-Step 'PASS' 'stop' 'editor stopped' } else { Write-Step 'FAIL' 'stop' 'could not stop the editor this script started' }
}
$timings['total_s'] = [math]::Round($t0.Elapsed.TotalSeconds, 1)
Exit-WithResult 'smoke' -1 @{ timings = $timings }
