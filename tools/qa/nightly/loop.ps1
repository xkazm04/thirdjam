# Lane A57: the nightly (here: day) loop. Every -EveryMin minutes read main's head (git log -1, read-only); when it moved since the last package: move this worktree
# there (git checkout --detach, the one allowed move), build the Editor target (the cook needs editor binaries of the same source), package Shipping with -Zip,
# smoke-launch 30 s, append a row to docs\experiments\NIGHTLY.md. Holds when free memory is under 10 GB. Stops at -Until (local clock, CET = Windows time zone of the machine).
#   powershell -NoProfile -ExecutionPolicy Bypass -File loop.ps1 -Until 16:00 -Last 59675fe
param([string]$Until = '16:00', [string]$Last = '', [int]$EveryMin = 60, [double]$MinFreeGB = 10)
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$main = 'C:\Users\kazda\kiro\thirdjam'
$pk = "$main\Game\Packaged"
$nightly = "$root\docs\experiments\NIGHTLY.md"
$lg = "$PSScriptRoot\out\loop.log"
New-Item -ItemType Directory -Force "$PSScriptRoot\out" | Out-Null
function Log($s) { "$(Get-Date -Format 'HH:mm:ss') $s" | Add-Content $lg }
function FreeGB { [math]::Round((Get-CimInstance Win32_OperatingSystem).FreePhysicalMemory / 1MB, 1) }
$deadline = [datetime]::Today.Add([timespan]$Until)
Log "loop start until $Until last=$Last"
while ($true) {
    $now = Get-Date
    if ($now -ge $deadline) { Log 'deadline reached'; break }
    $head = (& git -C $main log -1 --format=%h main).Trim()
    $free = FreeGB
    if ($head -eq $Last) { Log "main=$head unchanged" }
    elseif ($free -lt $MinFreeGB) {
        Log "main=$head moved, HELD: free memory $free GB under $MinFreeGB"
        Add-Content $nightly ("| {0} | {1} | held | | | free memory {2} GB under {3} GB, no package this hour |" -f $head, $now.ToString('HH:mm'), $free, $MinFreeGB)
    }
    else {
        $stamp = $now.ToString('HHmm'); $name = "Hokej_Hokej_N$stamp"
        Log "main=$head moved: package $name"
        & git -C $root checkout --detach $head *> $null
        # the Editor target first (a changed source needs fresh editor modules for the cook), same -WaitMutex retry as pack.ps1
        $bok = $false
        for ($i = 1; $i -le 12 -and -not $bok; $i++) {
            & powershell -NoProfile -ExecutionPolicy Bypass -File "$root\tools\run\build.ps1" -Project "$root\Game" *> "$PSScriptRoot\out\build-$name.txt"
            if ($LASTEXITCODE -eq 0) { $bok = $true } else { $t = Get-Content "$PSScriptRoot\out\build-$name.txt" -Raw; if ($t -notmatch 'Error_SDKNotFound|ConflictingInstance|already running') { break }; Start-Sleep -Seconds 90 }
        }
        $t0 = Get-Date
        $pok = $false
        if ($bok) { & powershell -NoProfile -ExecutionPolicy Bypass -File "$PSScriptRoot\pack.ps1" -Config Shipping -Name $name -Zip; $pok = ($LASTEXITCODE -eq 0) }
        $row = ''
        if ($pok) {
            $txt = Get-Content "$PSScriptRoot\out\pack-$name.txt" -Raw
            $json = ($txt -split "`n" | Where-Object { $_ -match '^\{"tool":"thirdjam.run/1"' } | Select-Object -Last 1)
            $j = $null; try { $j = $json | ConvertFrom-Json } catch {}
            $pkgLog = if ($j) { $j.log } else { $null }
            $warn = 0; $usage = 0; $loadErr = 0
            # UAT echoes every cook line twice ("LogInit: Display: <line>"): count distinct warnings, keep their text next to the smoke output
            $wl = @()
            $files = @(); if ($pkgLog -and (Test-Path $pkgLog)) { $files += $pkgLog }
            $files += @(Get-ChildItem "$root\Game\Saved\Logs" -Filter 'Cook*.log' -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $t0.AddMinutes(-30) } | ForEach-Object { $_.FullName })
            foreach ($f in $files) {
                $wl += @(Select-String -Path $f -Pattern 'Warning: ' -SimpleMatch | ForEach-Object { ($_.Line -replace '^\S+: Display: ', '').Trim() })
                $usage += @(Select-String -Path $f -Pattern 'missing usage flag' -SimpleMatch).Count
                $loadErr += @(Select-String -Path $f -Pattern 'LoadErrors' -SimpleMatch).Count
            }
            $wl = @($wl | Sort-Object -Unique)
            $warn = $wl.Count
            New-Item -ItemType Directory -Force "$PSScriptRoot\out\smoke-$name" | Out-Null
            $wl | Set-Content "$PSScriptRoot\out\smoke-$name\warnings.txt" -Encoding utf8
            $sm = & powershell -NoProfile -ExecutionPolicy Bypass -File "$PSScriptRoot\smoke.ps1" -Pkg "$pk\$name\Windows" -OutDir "$PSScriptRoot\out\smoke-$name" | Select-Object -Last 1
            $s = $null; try { $s = $sm | ConvertFrom-Json } catch {}
            $smokeTxt = if ($s -and $s.alive -and $s.still_mean -gt 5) { "PASS alive 30 s, peak {0} MB, menu still mean {1}" -f $s.peak_ws_mb, $s.still_mean } else { "FAIL " + $sm }
            $size = if ($j) { "zip {0} MB / staged {1} MB" -f $j.zip_mb, $j.staged_mb } else { 'n/a' }
            $row = "| {0} | {1} | {2} | {3} (distinct warnings {4}, usage flag {5}, LoadErrors {6}) | {7} | {8} s cook+package+zip |" -f $head, $now.ToString('HH:mm'), $size, $name, $warn, $usage, $loadErr, $smokeTxt, [int]((Get-Date) - $t0).TotalSeconds
        } else { $row = "| {0} | {1} | FAILED | {2} | build ok=$bok | see tools/qa/nightly/out/pack-$name.txt |" -f $head, $now.ToString('HH:mm'), $name }
        Add-Content $nightly $row
        Log "row: $row"
        $Last = $head
    }
    # sleep to the next wall-clock slot, in short steps so the deadline is honoured
    $next = $now.AddMinutes($EveryMin)
    while ((Get-Date) -lt $next -and (Get-Date) -lt $deadline) { Start-Sleep -Seconds 20 }
}
Log 'loop end'
