# Shared helpers for tools/run/*.ps1 (Windows PowerShell 5.1).
# Dot-source it: . "$PSScriptRoot\common.ps1"
#
# Conventions every script follows:
#   - prints "PASS <step> <detail>" / "FAIL <step> <detail>" / "INFO ..." lines, and one JSON line last
#     ({"tool":"thirdjam.run/1", ...}).
#   - exit 0 pass, 1 fail, 2 could not run (missing engine, missing project, disk below the floor, bad arguments).
#   - every wait has an explicit wall-clock timeout; a timeout is reported as "timeout", never as a pass.
#   - only processes this script started are ever stopped (tracked by PID and start time).
#   - logs go to <project dir>\logs\ (or next to the exe for run-packaged.ps1).

Set-StrictMode -Version 2.0

$script:TJ_Steps = New-Object System.Collections.ArrayList
$script:TJ_Failed = $false

function Write-Step {
    param([string]$Status, [string]$Name, [string]$Detail = '')
    $line = ('{0} {1} {2}' -f $Status, $Name, $Detail).TrimEnd()
    Write-Host $line
    if ($Status -eq 'PASS' -or $Status -eq 'FAIL') {
        [void]$script:TJ_Steps.Add([ordered]@{ step = $Name; status = $Status.ToLower(); detail = $Detail })
        if ($Status -eq 'FAIL') { $script:TJ_Failed = $true }
    }
}

function ConvertTo-JsonLine {
    param($Object)
    return ($Object | ConvertTo-Json -Depth 8 -Compress)
}

# Ends the script: prints the JSON line and exits with 0/1/2.
function Exit-WithResult {
    param([string]$Script, [int]$Code = -1, [hashtable]$Extra = @{})
    if ($Code -lt 0) { if ($script:TJ_Failed) { $Code = 1 } else { $Code = 0 } }
    $verdict = @('pass', 'fail', 'could_not_run')[$Code]
    $obj = [ordered]@{ tool = 'thirdjam.run/1'; script = $Script; verdict = $verdict; exit = $Code; steps = @($script:TJ_Steps) }
    foreach ($k in $Extra.Keys) { $obj[$k] = $Extra[$k] }
    Write-Host (ConvertTo-JsonLine $obj)
    exit $Code
}

# Engine root: $env:UE_ROOT, else the launcher install named by the .uproject EngineAssociation, else UE_5.9 default.
function Get-EngineRoot {
    param([string]$Project = '')
    if ($env:UE_ROOT -and (Test-Path $env:UE_ROOT)) { return (Resolve-Path $env:UE_ROOT).Path }
    $assoc = '5.9'
    if ($Project -and (Test-Path $Project)) {
        try { $u = Get-Content $Project -Raw | ConvertFrom-Json; if ($u.EngineAssociation) { $assoc = $u.EngineAssociation } } catch {}
    }
    $dat = 'C:\ProgramData\Epic\UnrealEngineLauncher\LauncherInstalled.dat'
    if (Test-Path $dat) {
        try {
            $list = (Get-Content $dat -Raw | ConvertFrom-Json).InstallationList
            foreach ($i in $list) { if ($i.AppName -eq ('UE_' + $assoc) -and (Test-Path $i.InstallLocation)) { return $i.InstallLocation } }
        } catch {}
    }
    $fallback = 'C:\Program Files\Epic Games\UE_' + $assoc
    if (Test-Path $fallback) { return $fallback }
    return $null
}

function Resolve-UProject {
    param([string]$Project)
    if (-not $Project) { return $null }
    if (-not (Test-Path $Project)) { return $null }
    $item = Get-Item $Project
    if ($item.PSIsContainer) {
        $u = Get-ChildItem -Path $item.FullName -Filter *.uproject | Select-Object -First 1
        if (-not $u) { return $null }
        return $u.FullName
    }
    if ($item.Extension -ne '.uproject') { return $null }
    return $item.FullName
}

function Get-LogDir {
    param([string]$BaseDir)
    $d = Join-Path $BaseDir 'logs'
    if (-not (Test-Path $d)) { New-Item -ItemType Directory -Force -Path $d | Out-Null }
    return $d
}

function Get-Stamp { return (Get-Date -Format 'yyyyMMdd-HHmmss') }

function Get-FreeGB {
    param([string]$Path = 'C:\')
    $root = [System.IO.Path]::GetPathRoot((Resolve-Path $Path).Path)
    $d = New-Object System.IO.DriveInfo($root)
    return [math]::Round($d.AvailableFreeSpace / 1GB, 1)
}

# Floor for free disk space in GB: -MinFreeGB on the command line, else $env:TJ_MIN_FREE_GB, else 20.
function Get-MinFreeGB {
    param([double]$Requested = -1)
    if ($Requested -ge 0) { return $Requested }
    if ($env:TJ_MIN_FREE_GB) { return [double]$env:TJ_MIN_FREE_GB }
    return 20
}

function Get-DirSizeMB {
    param([string]$Path)
    if (-not (Test-Path $Path)) { return 0 }
    $sum = (Get-ChildItem -Path $Path -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object -Property Length -Sum).Sum
    if (-not $sum) { return 0 }
    return [math]::Round($sum / 1MB, 1)
}

# Kills a process tree rooted at a PID this script started. Refuses if the PID now belongs to a
# different process (start time mismatch), so a recycled PID is never touched.
function Stop-OwnedTree {
    param([int]$ProcessId, [datetime]$StartTime)
    $p = Get-Process -Id $ProcessId -ErrorAction SilentlyContinue
    if (-not $p) { return $true }
    try { if ([math]::Abs(($p.StartTime - $StartTime).TotalSeconds) -gt 2) { Write-Step 'INFO' 'stop' "pid $ProcessId was reused by another process; not touched"; return $false } } catch {}
    & taskkill.exe /PID $ProcessId /T /F 2>&1 | Out-Null
    for ($i = 0; $i -lt 30; $i++) { if (-not (Get-Process -Id $ProcessId -ErrorAction SilentlyContinue)) { return $true }; Start-Sleep -Milliseconds 500 }
    return $false
}

# Runs an executable to completion with a timeout and a disk floor. Output goes to $LogFile.
# Returns @{ exit; timedOut; diskStop; seconds; pid }.
function Invoke-Watched {
    param(
        [string]$FilePath,
        [string]$Arguments,
        [string]$LogFile,
        [int]$TimeoutSec,
        [double]$MinFreeGB,
        [string]$DiskPath = 'C:\',
        [string]$Label = 'job'
    )
    $errFile = $LogFile + '.stderr'
    $sw = [System.Diagnostics.Stopwatch]::StartNew()
    $p = Start-Process -FilePath $FilePath -ArgumentList $Arguments -RedirectStandardOutput $LogFile -RedirectStandardError $errFile -NoNewWindow -PassThru
    $null = $p.Handle  # PS 5.1: keeps ExitCode readable after exit
    $start = $p.StartTime
    $timedOut = $false; $diskStop = $false; $lastBeat = 0
    while (-not $p.HasExited) {
        Start-Sleep -Seconds 2
        $elapsed = $sw.Elapsed.TotalSeconds
        if ($elapsed -gt $TimeoutSec) { $timedOut = $true; break }
        $free = Get-FreeGB $DiskPath
        if ($free -lt $MinFreeGB) { $diskStop = $true; Write-Step 'INFO' $Label "free space $free GB is below the floor of $MinFreeGB GB; stopping"; break }
        if ($elapsed - $lastBeat -ge 60) { $lastBeat = $elapsed; Write-Host ('INFO {0} running {1:N0}s, free {2} GB' -f $Label, $elapsed, $free) }
    }
    if ($timedOut -or $diskStop) { [void](Stop-OwnedTree -ProcessId $p.Id -StartTime $start) }
    $p.WaitForExit(10000) | Out-Null
    $sw.Stop()
    if (Test-Path $errFile) {
        $e = Get-Content $errFile -Raw -ErrorAction SilentlyContinue
        if ($e) { Add-Content -Path $LogFile -Value $e }
        Remove-Item $errFile -Force -ErrorAction SilentlyContinue
    }
    $code = $null
    try { $code = $p.ExitCode } catch {}
    if ($null -eq $code) { $code = -1 }
    return @{ exit = $code; timedOut = $timedOut; diskStop = $diskStop; seconds = [math]::Round($sw.Elapsed.TotalSeconds, 1); pid = $p.Id }
}

# Last N lines of a log that look like errors, for a FAIL detail.
function Get-ErrorLines {
    param([string]$LogFile, [int]$Max = 8)
    if (-not (Test-Path $LogFile)) { return @() }
    $hits = Select-String -Path $LogFile -Pattern '(error|Error:|ERROR|fatal|Fatal|exception|Exception|missing|could not|Unable to)' -ErrorAction SilentlyContinue | Select-Object -Last $Max
    return @($hits | ForEach-Object { $_.Line.Trim() })
}
