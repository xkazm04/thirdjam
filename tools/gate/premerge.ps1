<#
.SYNOPSIS
  The Director's one command at every merge window: ownership, assets ledger, node tests, editor build,
  Unreal automation tests (group Jam (the tests are named Jam.<Area>.<Name>; the brief said Hokej)), dev log build. PASS/FAIL lines and a JSON line last.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\gate\premerge.ps1 -Device A
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\gate\premerge.ps1 -Device B -Base main -SkipBuild -SkipAutomation

.NOTES
  - Steps: ownership (check-ownership.mjs --device X --base B), ledger (check-assets-ledger.mjs), node (node --test over
    schema, tools and devlog), build (tools\run\build.ps1, editor target), automation (UnrealEditor-Cmd, "Automation RunTests Jam",
    -nullrhi, result lines parsed), devlog (the page still builds).
  - build and automation are heavy: -SkipBuild / -SkipAutomation skip them (printed as SKIP, never as PASS, and listed in the JSON).
    Automation is also skipped, with a line saying so, when the source has no automation tests yet.
  - Automation runs in its own editor process; it uses no MCP port and never touches an editor it did not start.
  - -Drift (lane A126, off by default) adds step 7: tools/soak/drift-gate.mjs, the soak drift gate (docs/experiments/SOAK-DRIFT.md), run only when the changed paths touch
    cartridges, the sim or the brain, under -DriftTimeoutSec (480); needs the build of this tree; a skip is printed as SKIP and listed in the JSON.
  - Writes only logs under Game\logs. Runs no git command itself; check-ownership.mjs runs read-only git diff and status.
  Exit: 0 every step that ran passed, 1 a step failed or timed out, 2 could not run (no node, no project).
#>
param(
    [Parameter(Mandatory = $true)][ValidateSet('A', 'B')][string]$Device,
    [string]$Base = 'main',
    [switch]$SkipBuild,
    [switch]$SkipAutomation,
    [switch]$SkipDevlog,
    [switch]$Drift,
    [int]$DriftTimeoutSec = 480,
    [string]$Filter = 'Jam',
    [int]$BuildTimeoutSec = 3600,
    [int]$AutomationTimeoutSec = 1200
)
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
. (Join-Path $repo 'tools\run\common.ps1')

if (-not (Get-Command node -ErrorAction SilentlyContinue)) { Write-Step 'FAIL' 'node' 'node not found on PATH'; Exit-WithResult 'premerge' 2 }
$uproject = Resolve-UProject (Join-Path $repo 'Game')
if (-not $uproject) { Write-Step 'FAIL' 'project' "no .uproject under $repo\Game"; Exit-WithResult 'premerge' 2 }
$logDir = Get-LogDir (Join-Path $repo 'Game')
$stamp = Get-Stamp
$extra = [ordered]@{ device = $Device; base = $Base }
$skipped = @()

function Invoke-Node([string[]]$argv, [string]$LogName) {
    $log = Join-Path $logDir ("premerge-{0}-{1}.log" -f $LogName, $stamp)
    $out = @(& node @argv 2>&1 | ForEach-Object { "$_" })
    $code = $LASTEXITCODE
    Set-Content -Path $log -Value $out -Encoding utf8
    return @{ code = $code; out = $out; log = $log }
}

Push-Location $repo
try {
    # 1. ownership
    $r = Invoke-Node @('tools/gate/check-ownership.mjs', '--device', $Device, '--base', $Base) 'ownership'
    if ($r.code -eq 0) { Write-Step 'PASS' 'ownership' "device $Device against $Base" }
    elseif ($r.code -eq 1) { foreach ($l in ($r.out | Select-Object -First 15)) { Write-Host "  | $l" }; Write-Step 'FAIL' 'ownership' "paths outside the ownership of device $Device (log $($r.log))" }
    else { foreach ($l in ($r.out | Select-Object -First 8)) { Write-Host "  | $l" }; Write-Step 'FAIL' 'ownership' "could not run (exit $($r.code))" }

    # 2. assets ledger
    $r = Invoke-Node @('tools/gate/check-assets-ledger.mjs') 'ledger'
    if ($r.code -eq 0) { Write-Step 'PASS' 'ledger' 'assets ledger complete and inside budget' }
    else { foreach ($l in ($r.out | Select-Object -First 15)) { Write-Host "  | $l" }; Write-Step 'FAIL' 'ledger' "exit $($r.code) (log $($r.log))" }

    # 3. node tests (schema, tools, devlog). Node 24 expands the quoted globs itself.
    $r = Invoke-Node @('--test', '--test-reporter=tap', 'schema/**/*.test.mjs', 'tools/**/*.test.mjs', 'devlog/**/*.test.mjs') 'node-tests'
    $pass = ($r.out | Select-String -Pattern '^# pass (\d+)' | Select-Object -Last 1)
    $failn = ($r.out | Select-String -Pattern '^# fail (\d+)' | Select-Object -Last 1)
    $pn = if ($pass) { $pass.Matches[0].Groups[1].Value } else { '?' }
    $fn = if ($failn) { $failn.Matches[0].Groups[1].Value } else { '?' }
    $extra['node_pass'] = $pn; $extra['node_fail'] = $fn
    if ($r.code -eq 0) { Write-Step 'PASS' 'node-tests' "$pn passed, $fn failed" }
    else {
        foreach ($l in ($r.out | Where-Object { $_ -match '^\s*not ok' } | Select-Object -First 10)) { Write-Host "  | $l" }
        Write-Step 'FAIL' 'node-tests' "$pn passed, $fn failed (log $($r.log))"
    }

    # 3b. test guards (lane A159, R-0057 point 3): test code closes inside #if WITH_DEV_AUTOMATION_TESTS; no Unreal, seconds.
    # Only the Shipping game target fails on a test below its #endif; the Development editor and every test stay green.
    $r = Invoke-Node @('tools/gate/check-test-guards.mjs') 'test-guards'
    if ($r.code -eq 0) { Write-Step 'PASS' 'test-guards' ($r.out | Where-Object { $_ -match '^PASS' } | Select-Object -Last 1) }
    else { foreach ($l in ($r.out | Where-Object { $_ -match '^FAIL' } | Select-Object -First 15)) { Write-Host "  | $l" }; Write-Step 'FAIL' 'test-guards' "exit $($r.code): test code outside its WITH_DEV_AUTOMATION_TESTS guard (log $($r.log))" }

    # 4. editor build
    if ($SkipBuild) { Write-Host 'SKIP build (-SkipBuild)'; $skipped += 'build' }
    else {
        $out = @(& powershell -NoProfile -ExecutionPolicy Bypass -File (Join-Path $repo 'tools\run\build.ps1') -Project $uproject -TimeoutSec $BuildTimeoutSec 2>&1 | ForEach-Object { "$_" })
        $code = $LASTEXITCODE
        foreach ($l in $out) { if ($l -match '^(INFO|WARN|  \|)') { Write-Host $l } }
        if ($code -eq 0) { Write-Step 'PASS' 'build' 'editor target built' }
        else { Write-Step 'FAIL' 'build' "build.ps1 exit $code" }
    }

    # 5. automation tests
    $hasTests = $false
    foreach ($s in @((Join-Path $repo 'Game\Source'), (Join-Path $repo 'Game\Plugins'))) {
        if (-not (Test-Path $s)) { continue }
        $hit = Get-ChildItem -Path $s -Recurse -Include *.cpp -File -ErrorAction SilentlyContinue | Select-String -Pattern 'IMPLEMENT_(SIMPLE|COMPLEX|CUSTOM_SIMPLE)_AUTOMATION_TEST|DEFINE_SPEC' -List | Select-Object -First 1
        if ($hit) { $hasTests = $true; break }
    }
    if ($SkipAutomation) { Write-Host 'SKIP automation (-SkipAutomation)'; $skipped += 'automation' }
    elseif (-not $hasTests) { Write-Host 'SKIP automation (no automation tests in the source yet)'; $skipped += 'automation' }
    else {
        $engine = Get-EngineRoot $uproject
        $cmd = $null
        if ($engine) { $cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe' }
        if (-not $cmd -or -not (Test-Path $cmd)) { Write-Step 'FAIL' 'automation' 'UnrealEditor-Cmd.exe not found (set UE_ROOT)' }
        else {
            $log = Join-Path $logDir ("premerge-automation-{0}.log" -f $stamp)
            $abslog = Join-Path $logDir ("premerge-automation-ue-{0}.log" -f $stamp)
            $ue = ('"{0}" -ExecCmds="Automation RunTests {1}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="{2}"' -f $uproject, $Filter, $abslog)
            $w = Invoke-Watched -FilePath $cmd -Arguments $ue -LogFile $log -TimeoutSec $AutomationTimeoutSec -MinFreeGB (Get-MinFreeGB 40) -DiskPath $uproject -Label 'automation'
            $src = $log
            if (Test-Path $abslog) { $src = $abslog }
            $ok = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Success\}' -ErrorAction SilentlyContinue).Count
            $bad = @(Select-String -Path $src -Pattern 'Test Completed\. Result=\{Fail[^}]*\}' -ErrorAction SilentlyContinue)
            $extra['automation_pass'] = $ok; $extra['automation_fail'] = $bad.Count
            # B-to-A-0027: a material without the skeletal-mesh usage flag is silent in the editor and total in a package (grey figures)
            $flag = @(Select-String -Path $src -Pattern 'missing usage flag' -ErrorAction SilentlyContinue)
            $extra['usage_flag_lines'] = $flag.Count
            if ($w.timedOut) { Write-Step 'FAIL' 'automation' "timeout after $($w.seconds)s (log $src)" }
            elseif ($bad.Count -gt 0) { foreach ($b in ($bad | Select-Object -First 10)) { Write-Host "  | $($b.Line.Trim())" }; Write-Step 'FAIL' 'automation' "$ok passed, $($bad.Count) failed (log $src)" }
            elseif ($flag.Count -gt 0) { foreach ($f in ($flag | Select-Object -First 3)) { Write-Host "  | $($f.Line.Trim())" }; Write-Step 'FAIL' 'automation' "$ok passed, but $($flag.Count) 'missing usage flag' line(s): a material on a skeletal mesh lacks the flag (log $src)" }
            elseif ($ok -eq 0) { foreach ($l in (Get-ErrorLines $src 6)) { Write-Host "  | $l" }; Write-Step 'FAIL' 'automation' "no test result lines for filter '$Filter' (exit $($w.exit), log $src)" }
            else { Write-Step 'PASS' 'automation' "$ok tests passed (filter '$Filter', $($w.seconds)s)" }
        }
    }

    # 6. dev log still builds
    if ($SkipDevlog) { Write-Host 'SKIP devlog (-SkipDevlog)'; $skipped += 'devlog' }
    else {
        $tmp = Join-Path $logDir ("premerge-devlog-{0}.html" -f $stamp)
        $r = Invoke-Node @('devlog/build.mjs', '--out', $tmp) 'devlog'
        $last = $r.out | Select-Object -Last 1
        if ($r.code -eq 0) { Write-Step 'PASS' 'devlog' "$last" } else { Write-Step 'FAIL' 'devlog' "exit $($r.code): $last" }
        Remove-Item $tmp -Force -ErrorAction SilentlyContinue
    }
    # 7. drift gate (lane A126): off unless -Drift. Runs only when the changed paths touch cartridges, the sim or the brain; --only modes / takes as fits; capped at -DriftTimeoutSec (8 min).
    if (-not $Drift) { Write-Host 'SKIP drift (off: pass -Drift)'; $skipped += 'drift' }
    else {
        $r = Invoke-Node @('tools/soak/drift-gate.mjs', '--uproject', $uproject, '--base', $Base, '--timeout-s', "$DriftTimeoutSec") 'drift'
        $last = ($r.out | Select-Object -Last 1)
        $dj = $null; try { $dj = $last | ConvertFrom-Json } catch { }
        foreach ($l in ($r.out | Where-Object { $_ -match '^(MOVED|NEW|GONE|FAIL|INFO)' } | Select-Object -First 15)) { Write-Host "  | $l" }
        if ($dj -and $dj.verdict -eq 'skipped') { Write-Host 'SKIP drift (no changed path touches cartridges, the sim or the brain)'; $skipped += 'drift' }
        elseif ($r.code -eq 0) { Write-Step 'PASS' 'drift' "scope $($dj.scope), $($dj.seconds)s, nothing moved" }
        else { Write-Step 'FAIL' 'drift' "exit $($r.code) (log $($r.log))" }
    }
}
finally { Pop-Location }

$extra['skipped'] = $skipped
Exit-WithResult 'premerge' -1 $extra
