<#
.SYNOPSIS
  Lane A56: one headless soak process. Runs Bar.Net.SoakClean / SoakLossy / SoakMismatch (JamNet/Private/Tests/JamNetSoakTests.cpp) in its own
  UnrealEditor-Cmd (-nullrhi -unattended), never an interactive editor. Every match writes a thirdjam.soaklock/1 JSON line to -Out.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools/soak/lockstep/run.ps1 -Test SoakClean -SeedFirst 1 -Seeds 50 -Out Game/Saved/SoakLockstep/clean-1.jsonl
  powershell -NoProfile -ExecutionPolicy Bypass -File tools/soak/lockstep/run.ps1 -Test SoakLossy -Profiles loss30,jitter0-6 -Seeds 50 -Out Game/Saved/SoakLockstep/lossy-a.jsonl

.NOTES
  Split seeds over several processes yourself (each is one editor process, about 1.5 GB); tools/soak/lockstep/summarize.mjs reads the .jsonl files.
  Exit: 0 the test passed, 1 it failed or timed out, 2 could not run.
#>
param(
    [ValidateSet('SoakClean', 'SoakLossy', 'SoakMismatch')][string]$Test = 'SoakClean',
    [int]$SeedFirst = 1,
    [int]$Seeds = 200,
    [int]$Frames = 3600,
    [string]$Profiles = '',
    [string]$Cut = 'hockey.regular',
    [Parameter(Mandatory = $true)][string]$Out,
    [int]$TimeoutSec = 7200
)
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $repo 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $repo 'Game')
if (-not $uproject) { Write-Step 'FAIL' 'project' 'no .uproject'; Exit-WithResult 'soak-lockstep' 2 }
$engine = Get-EngineRoot $uproject
$cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path $cmd)) { Write-Step 'FAIL' 'engine' 'UnrealEditor-Cmd.exe not found'; Exit-WithResult 'soak-lockstep' 2 }
if (-not [System.IO.Path]::IsPathRooted($Out)) { $Out = Join-Path $repo $Out }
New-Item -ItemType Directory -Force (Split-Path $Out -Parent) | Out-Null
$logDir = Get-LogDir (Join-Path $repo 'Game')
$stem = [System.IO.Path]::GetFileNameWithoutExtension($Out)
$log = Join-Path $logDir ("soak-{0}-{1}.log" -f $stem, (Get-Stamp))
$abslog = Join-Path $logDir ("soak-{0}-ue-{1}.log" -f $stem, (Get-Stamp))
$soak = "-SoakSeedFirst=$SeedFirst -SoakSeeds=$Seeds -SoakFrames=$Frames -SoakOut=`"$Out`""
if ($Profiles) { $soak += " -SoakProfiles=$Profiles" }
if ($Cut -ne 'hockey.regular') { $soak += " -SoakCut=$Cut" }
$ue = ('"{0}" -ExecCmds="Automation RunTests Bar.Net.{1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}" {3}' -f $uproject, $Test, $abslog, $soak)
Write-Host "INFO soak: $Test seeds $SeedFirst..$($SeedFirst + $Seeds - 1) frames $Frames profiles '$Profiles' -> $Out"
$w = Invoke-Watched -FilePath $cmd -Arguments $ue -LogFile $log -TimeoutSec $TimeoutSec -MinFreeGB (Get-MinFreeGB 40) -DiskPath $uproject -Label 'soak'
$src = $log; if (Test-Path $abslog) { $src = $abslog }
$ok = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Success\}' -ErrorAction SilentlyContinue).Count
$bad = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Fail[^}]*\}' -ErrorAction SilentlyContinue).Count
$extra = @{ test = $Test; out = $Out; log = $src; seconds = $w.seconds }
if ($w.timedOut) { Write-Step 'FAIL' 'soak' "timeout after $($w.seconds)s (log $src)"; Exit-WithResult 'soak-lockstep' 1 $extra }
if ($bad -gt 0) { Write-Step 'FAIL' 'soak' "Bar.Net.$Test failed (log $src)"; Exit-WithResult 'soak-lockstep' 1 $extra }
if ($ok -eq 0) { Write-Step 'FAIL' 'soak' "no result line (exit $($w.exit), log $src)"; Exit-WithResult 'soak-lockstep' 2 $extra }
Write-Step 'PASS' 'soak' "Bar.Net.$Test in $($w.seconds)s"
Exit-WithResult 'soak-lockstep' 0 $extra
