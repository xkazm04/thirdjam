# Lane A211 (the eye): the gate of the capture tool (jam.ScriptHuman 2), run in THIS tree, one heavy process after the other:
#   1. the test-guard check (node tools/gate/check-test-guards.mjs)
#   2. the Shipping compile of the Game target (tools\run\build.ps1 -Target Game -Config Shipping)
#   3. the full Jam automation filter in a cleared environment (no JAM_ variable), the invocation of tools\gate\premerge.ps1's step 5
# It runs no git command (premerge.ps1's ownership step does, through node: that step is the Director's). Logs under Game\logs\a211.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye\eye-gate.ps1 [-SkipShipping] [-SkipAutomation]
# Exit 0: every step that ran passed. 1: one failed. 2: could not run.
param(
    [switch]$SkipShipping,
    [switch]$SkipAutomation,
    [string]$Filter = 'Jam',
    [int]$BuildTimeoutSec = 3600,
    [int]$AutomationTimeoutSec = 2400
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL eye-gate: this tool runs only from a worktree (root $root)"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
if (-not $uproject -or -not $engine) { Write-Host 'FAIL eye-gate: no project or no engine'; exit 2 }
$logDir = Join-Path $root 'Game\logs\a211'
New-Item -ItemType Directory -Force $logDir | Out-Null
$stamp = Get-Date -Format 'HHmmss'
$fail = 0
Push-Location $root
try {
    $g = @(& node 'tools/gate/check-test-guards.mjs' 2>&1 | ForEach-Object { "$_" })
    if ($LASTEXITCODE -eq 0) { Write-Host ("PASS test-guards {0}" -f ($g | Select-Object -Last 1)) } else { $fail++; Write-Host ("FAIL test-guards exit $LASTEXITCODE") }

    if ($SkipShipping) { Write-Host 'SKIP shipping (-SkipShipping)' }
    else {
        $blog = Join-Path $logDir "shipping-$stamp.log"
        & powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $root 'tools\run\build.ps1') -Project $uproject -Target Game -Config Shipping -TimeoutSec $BuildTimeoutSec *> $blog
        $code = $LASTEXITCODE
        $last = (Get-Content $blog | Where-Object { $_ -match '^(PASS|FAIL) build' } | Select-Object -Last 1)
        if ($code -eq 0) { Write-Host "PASS shipping $last (log $blog)" } else { $fail++; Write-Host "FAIL shipping exit $code $last (log $blog)"; Get-Content $blog | Where-Object { $_ -match 'error' } | Select-Object -First 12 | ForEach-Object { Write-Host "  | $_" } }
    }

    if ($SkipAutomation) { Write-Host 'SKIP automation (-SkipAutomation)' }
    else {
        # the cleared environment: no JAM_ variable reaches the editor
        $cleared = @()
        Get-ChildItem Env: | Where-Object { $_.Name -like 'JAM_*' } | ForEach-Object { $cleared += $_.Name; Remove-Item ("Env:" + $_.Name) }
        $cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
        $log = Join-Path $logDir "automation-$stamp.log"
        $abslog = Join-Path $logDir "automation-ue-$stamp.log"
        $ue = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}"' -f $uproject, $Filter, $abslog)
        $w = Invoke-Watched -FilePath $cmd -Arguments $ue -LogFile $log -TimeoutSec $AutomationTimeoutSec -MinFreeGB (Get-MinFreeGB 40) -DiskPath $uproject -Label 'automation'
        $src = $log; if (Test-Path $abslog) { $src = $abslog }
        $ok = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Success\}' -ErrorAction SilentlyContinue).Count
        $bad = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Fail[^}]*\}' -ErrorAction SilentlyContinue)
        if ($w.timedOut) { $fail++; Write-Host "FAIL automation timeout after $($w.seconds)s, $ok passed so far (log $src)" }
        elseif ($bad.Count -gt 0) { $fail++; $bad | Select-Object -First 12 | ForEach-Object { Write-Host "  | $($_.Line.Trim())" }; Write-Host "FAIL automation $ok Success, $($bad.Count) Fail (log $src)" }
        elseif ($ok -eq 0) { $fail++; Write-Host "FAIL automation no result lines (exit $($w.exit), log $src)" }
        else { Write-Host "PASS automation $ok Success, 0 Fail, filter '$Filter', $($w.seconds)s, JAM_ variables cleared: $($cleared.Count) (log $src)" }
    }
} finally { Pop-Location }
if ($fail -gt 0) { exit 1 } else { exit 0 }
