<#
.SYNOPSIS
  Shipping package on demand or every 6 hours from H6: package + zip, a 10 s run of the packaged build, the last
  three zips kept in Game\Packaged\, sizes appended to status\A.md, a warning at 900 MB.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\gate\package-schedule.ps1 -Once
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\gate\package-schedule.ps1            # waits for H6, then every 6 h
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\gate\package-schedule.ps1 -StartHour 12 -EveryHours 6 -Rounds 3

.NOTES
  - H0 is 2026-11-12 06:00 local time (devlog\lib\constants.mjs). The hour in the zip name is the whole hours since H0
    when the package started: Hokej_Hokej_H18.zip. Hour boundaries are computed from H0, not from when this script
    was started, so a restart does not shift the schedule.
  - A package is a heavy job: if another package.ps1 / RunUAT / UnrealBuildTool is already running (started by anyone),
    the round waits up to -WaitMinutes for it to finish and is skipped (FAIL) if it does not. Nothing is ever killed.
  - Staging folder: Game\Packaged\_stage (deleted after the round, so a package never leaves 300 MB+ behind).
    Game\Packaged\ is gitignored by the Director's .gitignore; if it is not yet, say so in a handoff request.
  - status\A.md gets one appended block per round:  ## H<n> package  with staged, zip, cook time, verdict, warn.
  - Free space floor 40 GB (package.ps1 stops a package that drops below it).
  - -Once runs one round now and exits with that round's code (0 pass, 1 fail, 2 could not run).
    Without -Once the loop exits 0 after -Rounds rounds (default 14: H6 to H90) or 1 if any round failed.
#>
param(
    [switch]$Once,
    [int]$StartHour = 6,
    [int]$EveryHours = 6,
    [int]$Rounds = 14,
    [int]$Keep = 3,
    [int]$WarnMB = 900,
    [int]$LimitMB = 1000,
    [int]$WaitMinutes = 60,
    [string]$Name = 'Hokej_Hokej',
    [string]$Project = '',
    [string]$OutRoot = '',
    [int]$RunSeconds = 10,
    [switch]$NoRun
)
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repo 'tools\run\common.ps1')

if (-not $Project) { $Project = Join-Path $repo 'Game\ThirdJam.uproject' }
if (-not $OutRoot) { $OutRoot = Join-Path $repo 'Game\Packaged' }
$uproject = Resolve-UProject $Project
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject at '$Project'"; Exit-WithResult 'package-schedule' 2 }
$statusFile = Join-Path $repo 'status\A.md'
$h0 = [datetime]::new(2026, 11, 12, 6, 0, 0, [System.DateTimeKind]::Local)
New-Item -ItemType Directory -Force -Path $OutRoot | Out-Null
$OutRoot = (Resolve-Path $OutRoot).Path

function Get-HourNow { return [int][math]::Floor(((Get-Date) - $h0).TotalHours) }

function Test-HeavyJob {
    # Anything that compiles or cooks right now, started by any lane.
    $n = @(Get-CimInstance Win32_Process -ErrorAction SilentlyContinue | Where-Object {
            $_.Name -match '^(UnrealBuildTool|dotnet|cl|link|ShaderCompileWorker|UnrealEditor-Cmd)\.exe$' -and
            $_.CommandLine -match '(UnrealBuildTool|BuildCookRun|-cook|Build\.bat|RunUAT)'
        })
    return $n.Count -gt 0
}

function Add-StatusBlock([string]$Text) {
    $enc = New-Object System.Text.UTF8Encoding($false)
    [System.IO.File]::AppendAllText($statusFile, "`n" + $Text + "`n", $enc)
}

function Invoke-Round([int]$Hour) {
    $stage = Join-Path $OutRoot '_stage'
    if (Test-Path $stage) { Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue }
    $waited = 0
    while (Test-HeavyJob) {
        if ($waited -ge $WaitMinutes) { Write-Step 'FAIL' "round-H$Hour" "another heavy job still running after $WaitMinutes min; skipped"; return 1 }
        Write-Host "INFO another build or cook is running; waiting (waited $waited min)"
        Start-Sleep -Seconds 60; $waited++
    }
    $zipName = '{0}_H{1}' -f $Name, $Hour
    $t0 = Get-Date
    $out = @(& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'tools\run\package.ps1') -Project $uproject -Config Shipping -Out $stage -Zip -Name $zipName -LimitMB $LimitMB -WarnMB $WarnMB 2>&1 | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    foreach ($l in $out) { if ($l -match '^(INFO size|WARN|PASS|FAIL|  \|)') { Write-Host $l } }
    $json = $null
    foreach ($l in $out) { if ($l -match '^\{"tool":"thirdjam.run/1"') { try { $json = $l | ConvertFrom-Json } catch {} } }
    $stagedMB = if ($json -and $json.PSObject.Properties['staged_mb']) { $json.staged_mb } else { '?' }
    $zipMB = if ($json -and $json.PSObject.Properties['zip_mb']) { $json.zip_mb } else { '?' }
    $cookS = if ($json -and $json.PSObject.Properties['cook_s']) { $json.cook_s } else { '?' }
    $totalS = if ($json -and $json.PSObject.Properties['seconds']) { $json.seconds } else { [math]::Round(((Get-Date) - $t0).TotalSeconds) }
    $verdict = if ($code -eq 0) { 'pass' } else { 'fail' }
    $runNote = 'not run'
    if ($code -eq 0 -and $json -and $json.PSObject.Properties['exe']) {
        if ($NoRun) { $runNote = 'skipped (-NoRun)' }
        else {
            $ro = @(& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'tools\run\run-packaged.ps1') -Exe $json.exe -Seconds $RunSeconds 2>&1 | ForEach-Object { "$_" })
            $rc = $LASTEXITCODE
            $runNote = if ($rc -eq 0) { "stayed up $RunSeconds s" } else { "FAILED (exit $rc)" }
            if ($rc -ne 0) { $verdict = 'fail'; $code = 1 }
            foreach ($l in $ro) { if ($l -match '^(PASS|FAIL|  \|)') { Write-Host $l } }
        }
    }
    $warn = ''
    if ($zipMB -ne '?' -and [double]$zipMB -gt $WarnMB) { $warn = "zip above $WarnMB MB"; Write-Host "WARN size zip $zipMB MB is above $WarnMB MB" }

    # Keep the last three zips; the new one moves out of the staging folder.
    $zipSrc = Join-Path $stage ($zipName + '.zip')
    if ($code -eq 0 -and (Test-Path $zipSrc)) { Move-Item -Path $zipSrc -Destination (Join-Path $OutRoot ($zipName + '.zip')) -Force }
    $zips = @(Get-ChildItem -Path $OutRoot -Filter ($Name + '_H*.zip') -File | Sort-Object LastWriteTime -Descending)
    for ($i = $Keep; $i -lt $zips.Count; $i++) { Remove-Item $zips[$i].FullName -Force; Write-Host "INFO removed old zip $($zips[$i].Name)" }
    if (Test-Path $stage) { Remove-Item $stage -Recurse -Force -ErrorAction SilentlyContinue }

    $lines = @("## H$Hour package", "- config: Shipping, $zipName.zip, verdict $verdict", "- staged_mb: $stagedMB", "- zip_mb: $zipMB", "- cook_s: $cookS (whole package $totalS s)", "- run: $runNote")
    if ($json -and $json.PSObject.Properties['largest']) {
        $top = @($json.largest | Select-Object -First 5 | ForEach-Object { '{0} {1} MB' -f ($_.path -replace '\\$', ''), $_.mb })
        $lines += ('- largest: ' + ($top -join '; '))
    }
    if ($warn) { $lines += "- warn: $warn" }
    if ($code -ne 0 -and $json) { $lines += "- blocked: see $($json.log)" }
    Add-StatusBlock ($lines -join "`n")
    if ($code -eq 0) { Write-Step 'PASS' "round-H$Hour" "$zipName.zip $zipMB MB (staged $stagedMB MB)" } else { Write-Step 'FAIL' "round-H$Hour" "package failed, see status\A.md and the log" }
    return $code
}

$worst = 0
if ($Once) {
    $worst = Invoke-Round (Get-HourNow)
    Exit-WithResult 'package-schedule' $(if ($worst -eq 0) { 0 } else { 1 }) @{ mode = 'once' }
}
$round = 0
$hour = $StartHour
while ($round -lt $Rounds) {
    $due = $h0.AddHours($hour)
    while ((Get-Date) -lt $due) { Start-Sleep -Seconds 30 }
    $rc = Invoke-Round $hour
    if ($rc -ne 0) { $worst = 1 }
    $round++; $hour += $EveryHours
}
Exit-WithResult 'package-schedule' $(if ($worst -eq 0) { 0 } else { 1 }) @{ mode = 'loop'; rounds = $round }
