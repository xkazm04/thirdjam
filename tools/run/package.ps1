<#
.SYNOPSIS
  Packages a project for Win64 with RunUAT BuildCookRun (build, cook, stage, pak, archive) and reports sizes
  against the contest limit of 1 GB.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\package.ps1 -Project C:\jam\Game -Out C:\jam\out
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\package.ps1 -Project C:\jam\Game -Out C:\jam\out -Config Shipping -Zip -Name TeamName_GameName

.NOTES
  - Output: <Out>\Windows\... (the archive). Log: <project>\logs\package-<config>-<stamp>.log plus UAT's own logs.
  - Size report after a good package: staged (archived) folder size, the ten largest folders and pak/utoc/ucas
    files, zip size with -Zip. Budget: PASS under -LimitMB (1000), WARN at -WarnMB (900), FAIL above the limit.
    The zip is measured when made; otherwise the folder size is checked (a zip is usually smaller than the folder).
  - -Map limits the cook to listed maps (e.g. /Engine/Maps/Entry); default: the project's MapsToCook / all maps.
  - -Ddc names a derived-data graph from the project's config, passed to the cook as -ddc=<name>.
  - Do not run while the editor is building or cooking (one heavy job at a time); an open editor is fine.
  Exit: 0 packaged and within the limit, 1 failed / timeout / over the limit, 2 could not run.
#>
param(
    [Parameter(Mandatory = $true)][string]$Project,
    [Parameter(Mandatory = $true)][string]$Out,
    [ValidateSet('Development', 'Shipping')][string]$Config = 'Development',
    [switch]$Zip,
    [string]$Name = '',
    [string]$Map = '',
    [string]$Ddc = '',
    [string]$ExtraArgs = '',
    [int]$TimeoutSec = 5400,
    [int]$LimitMB = 1000,
    [int]$WarnMB = 900,
    [double]$MinFreeGB = -1
)
. "$PSScriptRoot\common.ps1"

$uproject = Resolve-UProject $Project
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject at '$Project'"; Exit-WithResult 'package' 2 }
$engine = Get-EngineRoot $uproject
Set-EngineDotnet $engine
if (-not $engine) { Write-Step 'FAIL' 'engine' 'Unreal Engine install not found (set UE_ROOT)'; Exit-WithResult 'package' 2 }
$uat = Join-Path $engine 'Engine\Build\BatchFiles\RunUAT.bat'
if (-not (Test-Path $uat)) { Write-Step 'FAIL' 'engine' "missing $uat"; Exit-WithResult 'package' 2 }
$floor = Get-MinFreeGB $MinFreeGB
$free = Get-FreeGB $uproject
if ($free -lt $floor) { Write-Step 'FAIL' 'disk' "free $free GB is below the floor of $floor GB"; Exit-WithResult 'package' 2 }
if ($Zip -and -not $Name) { Write-Step 'FAIL' 'args' '-Zip needs -Name (contest format: TeamName_GameName)'; Exit-WithResult 'package' 2 }

$projDir = Split-Path $uproject -Parent
$projName = [System.IO.Path]::GetFileNameWithoutExtension($uproject)
if (-not (Test-Path $Out)) { New-Item -ItemType Directory -Force -Path $Out | Out-Null }
$Out = (Resolve-Path $Out).Path
$logDir = Get-LogDir $projDir
$log = Join-Path $logDir ("package-{0}-{1}.log" -f $Config, (Get-Stamp))

$a = @('BuildCookRun', ('-project="{0}"' -f $uproject), '-noP4', '-platform=Win64', ('-clientconfig={0}' -f $Config),
    '-build', '-cook', '-stage', '-pak', '-iostore', '-package', '-archive', ('-archivedirectory="{0}"' -f $Out),
    '-prereqs', '-nocompileeditor', '-unattended', '-utf8output', '-NoSubmit')
if ($Config -eq 'Shipping') { $a += '-nodebuginfo' }
if ($Map) {
    # Git Bash rewrites "/Engine/Maps/X" into "C:/Program Files/Git/Engine/Maps/X" before PowerShell sees it.
    if ($Map -match '^[A-Za-z]:[/\\].*?[/\\]Git[/\\]((Game|Engine)[/\\].*)$') {
        $fixed = '/' + ($Matches[1] -replace '\\', '/')
        Write-Host "WARN package: -Map '$Map' looks rewritten by Git Bash; using '$fixed' (set MSYS_NO_PATHCONV=1 to avoid this)"
        $Map = $fixed
    }
    $a += ('-map={0}' -f $Map)
}
if ($Ddc) { $a += ('-ddc={0}' -f $Ddc) }
if ($ExtraArgs) { $a += $ExtraArgs }
$argLine = $a -join ' '
Write-Host "INFO package: $uat $argLine"
Write-Host "INFO package: log $log"
$r = Invoke-Watched -FilePath $uat -Arguments $argLine -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB $floor -DiskPath $uproject -Label 'package'

# Phase timings from the UAT log.
$phases = [ordered]@{}
foreach ($pat in @(@('build', 'Total execution time'), @('cook', 'Cook command time'), @('stage', 'Copying NonUFSFiles'), @('total', 'BuildCookRun time'))) {
    $m = Select-String -Path $log -Pattern ($pat[1] + '[^0-9]*([0-9.]+)\s*s') -ErrorAction SilentlyContinue | Select-Object -Last 1
    if ($m) { $phases[$pat[0] + '_s'] = [double]$m.Matches[0].Groups[1].Value }
}
$extra = [ordered]@{ config = $Config; seconds = $r.seconds; log = $log; out = $Out }
foreach ($k in $phases.Keys) { $extra[$k] = $phases[$k] }

if ($r.timedOut) { Write-Step 'FAIL' 'package' ("timeout after {0}s (limit {1}s)" -f $r.seconds, $TimeoutSec); Exit-WithResult 'package' 1 $extra }
if ($r.diskStop) { Write-Step 'FAIL' 'package' 'stopped: disk below floor'; Exit-WithResult 'package' 1 $extra }
if ($r.exit -ne 0) {
    foreach ($l in (Get-ErrorLines $log 12)) { Write-Host "  | $l" }
    Write-Step 'FAIL' 'package' ("BuildCookRun {0} exit {1} after {2}s" -f $Config, $r.exit, $r.seconds)
    Exit-WithResult 'package' 1 $extra
}
Write-Step 'PASS' 'package' ("BuildCookRun {0} in {1}s" -f $Config, $r.seconds)

# The archive layout is <Out>\Windows (5.x) or <Out>\WindowsNoEditor (older).
$staged = @('Windows', 'WindowsNoEditor', 'Win64') | ForEach-Object { Join-Path $Out $_ } | Where-Object { Test-Path $_ } | Select-Object -First 1
if (-not $staged) { Write-Step 'FAIL' 'stage' "no Windows folder under $Out"; Exit-WithResult 'package' 1 $extra }
$exe = Get-ChildItem -Path $staged -Filter '*.exe' -File | Select-Object -First 1
$stagedMB = Get-DirSizeMB $staged
$extra['staged'] = $staged
$extra['staged_mb'] = $stagedMB
if ($exe) { $extra['exe'] = $exe.FullName }

# Ten largest: every folder (recursive total) and every file under a Paks folder.
$items = @()
foreach ($d in (Get-ChildItem -Path $staged -Recurse -Directory -Force)) {
    $items += [pscustomobject]@{ path = $d.FullName.Substring($staged.Length + 1) + '\'; mb = (Get-DirSizeMB $d.FullName) }
}
foreach ($f in (Get-ChildItem -Path $staged -Recurse -File -Force | Where-Object { $_.DirectoryName -match '\\Paks$' -or $_.Length -gt 20MB })) {
    $items += [pscustomobject]@{ path = $f.FullName.Substring($staged.Length + 1); mb = [math]::Round($f.Length / 1MB, 1) }
}
$largest = @($items | Sort-Object -Property mb -Descending | Select-Object -First 10)
Write-Host ("INFO size: staged {0} MB in {1}" -f $stagedMB, $staged)
foreach ($i in $largest) { Write-Host ('INFO size: {0,9:N1} MB  {1}' -f $i.mb, $i.path) }
$extra['largest'] = @($largest | ForEach-Object { [ordered]@{ path = $_.path; mb = $_.mb } })

$checkMB = $stagedMB
$checkWhat = 'staged folder'
if ($Zip) {
    $zipPath = Join-Path $Out ($Name + '.zip')
    if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
    $zsw = [System.Diagnostics.Stopwatch]::StartNew()
    try {
        Add-Type -AssemblyName System.IO.Compression.FileSystem
        [System.IO.Compression.ZipFile]::CreateFromDirectory($staged, $zipPath, [System.IO.Compression.CompressionLevel]::Optimal, $true)
        $zipMB = [math]::Round((Get-Item $zipPath).Length / 1MB, 1)
        $extra['zip'] = $zipPath; $extra['zip_mb'] = $zipMB; $extra['zip_s'] = [math]::Round($zsw.Elapsed.TotalSeconds, 1)
        Write-Step 'PASS' 'zip' ("{0} {1} MB in {2:N0}s" -f $zipPath, $zipMB, $zsw.Elapsed.TotalSeconds)
        $checkMB = $zipMB; $checkWhat = 'zip'
    } catch { Write-Step 'FAIL' 'zip' $_.Exception.Message }
}
if ($checkMB -gt $LimitMB) { Write-Step 'FAIL' 'size' ("{0} is {1} MB, over the {2} MB limit" -f $checkWhat, $checkMB, $LimitMB) }
elseif ($checkMB -gt $WarnMB) { Write-Host ("WARN size {0} is {1} MB, above the {2} MB warning line" -f $checkWhat, $checkMB, $WarnMB); Write-Step 'PASS' 'size' ("{0} {1} MB (warn)" -f $checkWhat, $checkMB) }
else { Write-Step 'PASS' 'size' ("{0} {1} MB, limit {2} MB" -f $checkWhat, $checkMB, $LimitMB) }
$extra['free_gb'] = (Get-FreeGB $uproject)
Exit-WithResult 'package' -1 $extra
