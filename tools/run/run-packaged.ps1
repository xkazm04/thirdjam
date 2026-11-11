<#
.SYNOPSIS
  Starts a packaged build with -log, confirms it stays up for -Seconds, then stops only that process.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\run-packaged.ps1 -Exe C:\jam\out\Windows\Game.exe -Seconds 10

.NOTES
  - The launcher exe (<Name>.exe) starts <Name>\Binaries\Win64\<Name>[-Win64-Shipping].exe as a child; both are
    started by this script and stopped as one tree by PID.
  - The game's own log is copied to <exe dir>\logs\ (Saved\Logs of the packaged build; Shipping writes no log).
  - Extra arguments go through -ExtraArgs (e.g. "-windowed -ResX=1280 -ResY=720").
  Exit: 0 stayed up for the whole time, 1 exited early or crashed, 2 could not run.
#>
param(
    [Parameter(Mandatory = $true)][string]$Exe,
    [int]$Seconds = 10,
    [string]$ExtraArgs = '-windowed -ResX=1280 -ResY=720'
)
. "$PSScriptRoot\common.ps1"

if (-not (Test-Path $Exe)) { Write-Step 'FAIL' 'exe' "not found: $Exe"; Exit-WithResult 'run-packaged' 2 }
$Exe = (Resolve-Path $Exe).Path
$dir = Split-Path $Exe -Parent
$logDir = Get-LogDir $dir
$stamp = Get-Stamp
$a = "-log $ExtraArgs".Trim()
$sw = [System.Diagnostics.Stopwatch]::StartNew()
$p = Start-Process -FilePath $Exe -ArgumentList $a -PassThru
$null = $p.Handle
$start = $p.StartTime
Write-Host "INFO run: pid $($p.Id) $Exe $a"

# The launcher may hand over to the real game exe; find children we started.
function Get-Children([int]$ppid) { @(Get-CimInstance Win32_Process -Filter "ParentProcessId=$ppid" -ErrorAction SilentlyContinue) }
$early = $false
while ($sw.Elapsed.TotalSeconds -lt $Seconds) {
    Start-Sleep -Milliseconds 500
    if ($p.HasExited) {
        $kids = Get-Children $p.Id
        if ($kids.Count -eq 0) { $early = $true; break }
    }
}
$kids = Get-Children $p.Id
$aliveNames = @()
if (-not $p.HasExited) { $aliveNames += $p.ProcessName }
foreach ($k in $kids) { $aliveNames += $k.Name }
$upFor = [math]::Round($sw.Elapsed.TotalSeconds, 1)
$code = $null; if ($p.HasExited) { try { $code = $p.ExitCode } catch {} }

# Stop the tree we started.
if (-not $p.HasExited) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start) }
foreach ($k in $kids) {
    $kp = Get-Process -Id $k.ProcessId -ErrorAction SilentlyContinue
    if ($kp) { [void](Stop-OwnedTree -ProcessId $k.ProcessId -StartTime $kp.StartTime) }
}

# Copy the game log, if any.
$gameLog = Get-ChildItem -Path $dir -Recurse -Filter '*.log' -File -ErrorAction SilentlyContinue | Where-Object { $_.FullName -match '\\Saved\\Logs\\' -and $_.LastWriteTime -ge $start.AddSeconds(-5) } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
$copied = ''
if ($gameLog) { $copied = Join-Path $logDir ("game-{0}.log" -f $stamp); Copy-Item $gameLog.FullName $copied }
$extra = @{ exe = $Exe; seconds = $upFor; processes = $aliveNames; log = $copied }
if ($copied) {
    $crash = Select-String -Path $copied -Pattern '(Fatal error|Unhandled Exception|Assertion failed)' | Select-Object -First 3
    if ($crash) { foreach ($c in $crash) { Write-Host "  | $($c.Line.Trim())" }; Write-Step 'FAIL' 'log' 'fatal lines in the game log' }
}
if ($early) { Write-Step 'FAIL' 'run' ("exited after {0}s with code {1}" -f $upFor, $code); Exit-WithResult 'run-packaged' 1 $extra }
Write-Step 'PASS' 'run' ("stayed up {0}s ({1}); stopped" -f $upFor, ($aliveNames -join ', '))
Exit-WithResult 'run-packaged' -1 $extra
