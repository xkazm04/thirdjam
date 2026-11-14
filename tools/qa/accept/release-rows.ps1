# Lane A179 (S3 evolution 5, item 1): the release chain's two window rows in one call, after the acceptance run.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-rows.ps1 -Package <Development package dir> -Zip <release zip> [-Name h31] [-Only menu|ship]
# Runs release-menu.ps1 on the Development package, then release-ship.ps1 on the zip. A `machine` verdict (exit 3: another window disturbed the row) is repeated ONCE;
# a second machine verdict stays machine. Both rows take the cursor and post input to their own window: run it as the acceptance run is run, with no other window driven.
# Exit: 0 both rows pass; 1 a row failed (the package is wrong: do not hand it over); 3 a row stayed machine after its repeat (look at the log, repeat by hand); 2 could not run.
# Last stdout line: JSON `thirdjam.releaserows/1` with each row's verdict and the repeat count; per-row output under tools\qa\out\release\<Name>-menu, <Name>-ship.
param([string]$Package = '', [string]$Zip = '', [string]$Name = 'release', [ValidateSet('', 'menu', 'ship')][string]$Only = '', [int]$PlaySeconds = 30)
$here = $PSScriptRoot
$res = [ordered]@{}; $worst = 0
function Run-Row([string]$row, [string[]]$argv) {
    $script:res[$row] = @{ verdict = 'none'; repeats = 0; exit = -1 }
    for ($try = 0; $try -le 1; $try++) {
        Write-Host "=== $row try $($try + 1) $(Get-Date -Format HH:mm:ss)"
        $name = if ($try -eq 0) { "$Name-$row" } else { "$Name-$row-t2" }
        $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $here "release-$row.ps1")) + $argv + @('-Name', $name)
        $o = @(& powershell @a 2>&1 | ForEach-Object { "$_" })
        $code = $LASTEXITCODE
        $o | Select-Object -Last 12 | ForEach-Object { Write-Host "  | $_" }
        $script:res[$row] = @{ verdict = @{ 0 = 'pass'; 1 = 'fail'; 2 = 'could_not_run'; 3 = 'machine' }[$code]; repeats = $try; exit = $code; name = $name }
        if ($code -ne 3) { break }
        Write-Host "machine verdict on ${row}: repeating once"
    }
    $c = $script:res[$row].exit
    if ($c -eq 1 -or $c -eq 2) { $script:worst = [Math]::Max($script:worst, $c) } elseif ($c -eq 3 -and $script:worst -eq 0) { $script:worst = 3 }
}
if ($Only -ne 'ship') { if (-not $Package) { Write-Host 'FAIL give -Package'; exit 2 }; Run-Row 'menu' @('-Package', $Package) }
if ($Only -ne 'menu') { if (-not $Zip) { Write-Host 'FAIL give -Zip'; exit 2 }; Run-Row 'ship' @('-Zip', $Zip, '-PlaySeconds', "$PlaySeconds") }
# precedence: a failed row (1) outranks could-not-run (2) outranks machine (3) in the exit code only as 1 / 2 / 3 as written above
$final = if ($res.Values | Where-Object { $_.exit -eq 1 }) { 1 } elseif ($res.Values | Where-Object { $_.exit -eq 2 }) { 2 } elseif ($res.Values | Where-Object { $_.exit -eq 3 }) { 3 } else { 0 }
Write-Host (@{ tool = 'thirdjam.releaserows/1'; verdict = @{ 0 = 'pass'; 1 = 'fail'; 2 = 'could_not_run'; 3 = 'machine' }[$final]; rows = $res } | ConvertTo-Json -Compress -Depth 4)
exit $final
