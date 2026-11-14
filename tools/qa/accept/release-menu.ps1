# Lane A179 (S3 evolution 5, item 1): the menu row the release chain runs itself, on the DEVELOPMENT package, after the acceptance run.
# What it does (the rule the window lane learnt at H27; docs/experiments/ACCEPT-WINDOW.md section 1):
#   scenario `return`: clean profile, the package's first screen, ONE Return -> PLAY NOW
#   scenario `click` : clean profile, a click on PLAY NOW at the place `jam.menuscreen where` prints for it (`JAMMENU place ... label="PLAY NOW..." x= y=`, client pixels)
# and for each: exactly ONE `JAMMATCH` and ONE `JAMCART loaded` from the press on, the LAST JAMMATCH says level=rookie source=settings (a clean profile's first match), a
# `JAMLOOK tier=` line, and the still taken 9 s after the press is lit (lit share >= 0.90; a rendered rink reads 0.99, the black world with its HUD 0.01).
# The package's own Saved folder is moved aside for the run and put back in a finally block; a leftover `Saved.a179aside` stops the run (exit 2) rather than guess.
#
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\release-menu.ps1 -Package Game\Packaged\Accept_H30 [-Name h30-menu] [-Port 4779] [-Scenarios return,click]
# Exit: 0 pass; 1 fail (the game is wrong: two builds, black frame, wrong level, no match); 3 machine (another game or window disturbed it: no window, no menu, a press that
# started nothing after four tries, the game gone; the caller repeats once, as for accept.ps1); 2 could not run. The last stdout line is JSON `thirdjam.releasemenu/1`.
# It takes the cursor and posts keys to its own window: run it where nothing else drives a window. Stops only the process it started.
param([string]$Package = '', [string]$Name = 'release-menu', [int]$Port = 4779, [string[]]$Scenarios = @(), [double]$LitMin = 0.90)
$Scenarios = @($Scenarios | ForEach-Object { $_ -split ',' } | Where-Object { $_ })
. (Join-Path $PSScriptRoot 'winhand.ps1')
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
if (-not $Package) { Write-Host 'FAIL give -Package <packaged dir>'; exit 2 }
$stage = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam'
$exe = Join-Path $stage 'Binaries\Win64\ThirdJam.exe'
if (-not (Test-Path $exe)) { Write-Host "FAIL no $exe"; exit 2 }
$saved = Join-Path $stage 'Saved'; $aside = Join-Path $stage 'Saved.a179aside'
if (Test-Path $aside) { Write-Host "FAIL $aside already exists: a previous run did not put the profile back; not touching it"; exit 2 }
$out = Join-Path $root "tools\qa\out\release\$Name"
New-Item -ItemType Directory -Force $out | Out-Null
$rows = [ordered]@{}; $fails = New-Object System.Collections.ArrayList; $machine = New-Object System.Collections.ArrayList
function Want([string]$n) { return ($Scenarios.Count -eq 0 -or $Scenarios -contains $n) }
function Row([string]$k, [string]$v) { $script:rows[$k] = $v; HW-Note "ROW $k = $v" }
function Fail([string]$k, [string]$why) { [void]$script:fails.Add("${k}: $why"); Row $k "FAIL $why" }
function Mach([string]$k, [string]$why) { [void]$script:machine.Add("${k}: $why"); Row $k "MACHINE $why" }
function CleanProfile { if (Test-Path $saved) { Remove-Item $saved -Recurse -Force -ErrorAction SilentlyContinue } }
# one press that must start a match; repeated (as accept.ps1 does) when no `JAMMENU start match` follows within 6 s: under load the first screen has not always taken its focus
function PressUntilStart([scriptblock]$press, [int]$m) { for ($i = 0; $i -lt 4; $i++) { & $press; if (HW-Wait 'JAMMENU start match' 6 $m) { return $true }; HW-Note "press $($i + 1) started nothing, again" }; return $false }
function Judge([string]$k, [int]$m) {
    Start-Sleep -Seconds 9
    $mt = @(HW-Lines 'JAMMATCH level=' $m); $ca = @(HW-Lines 'JAMCART loaded' $m); $lk = @(HW-Lines 'JAMLOOK tier=' $m)
    $f = HW-Shot "$k-match"; $px = HW-Pixels $f
    $last = if ($mt.Count) { ($mt[-1] -replace '^.*?JAMMATCH', 'JAMMATCH') } else { '(none)' }
    $lv = ''; $src = ''; if ($last -match 'level=(\S+) source=(\S+)') { $lv = $Matches[1]; $src = $Matches[2] }
    $why = @()
    if ($mt.Count -ne 1) { $why += "JAMMATCH x$($mt.Count) (want exactly 1)" }
    if ($ca.Count -ne 1) { $why += "JAMCART loaded x$($ca.Count) (want exactly 1)" }
    if ($lv -ne 'rookie') { $why += "level=$lv (want rookie: a clean profile's first match)" }
    if ($src -ne 'settings') { $why += "source=$src (want settings)" }
    if ($lk.Count -lt 1) { $why += 'no JAMLOOK tier line' }
    if ($px.lit -lt $LitMin) { $why += "NOT LIT lit=$($px.lit) mean=$($px.mean) (want >= $LitMin)" }
    $facts = "last=[$last] builds=$($mt.Count)/$($ca.Count) looks=$($lk.Count) lit=$($px.lit) mean=$($px.mean)"
    if ($why.Count) { Fail $k ($facts + ' :: ' + ($why -join '; ')) } else { Row $k "PASS $facts" }
}
function Launch([string]$tag, [scriptblock]$body) {
    $o = Join-Path $out $tag
    CleanProfile
    if (-not (HW-Start $exe '' $o -Port $Port -Console -WaitSec 180)) { Mach $tag 'no game window'; return }
    if (-not (HW-Wait "Game class is 'JamMenuGameMode'" 150)) { Mach $tag 'the menu never came up'; [void](HW-Close -ByConsole); return }
    Start-Sleep -Seconds 4
    $os = HW-Others; if ($os.Count) { Mach "$tag.foreign" "other ThirdJam processes share the cursor: $($os -join ' ')" }
    & $body
    if (-not (HW-Alive)) { Mach $tag 'the game was gone before the close' }
    [void](HW-Close -ByConsole); Start-Sleep -Seconds 2
}
$moved = $false
try {
    if (Test-Path $saved) { Rename-Item $saved $aside; $moved = $true; HW-Note 'Saved renamed aside' }
    if (Want 'return') {
        Launch 'return' {
            $m = HW-Mark
            if (-not (PressUntilStart { HW-Key 'Return' } $m)) { Mach 'return' 'four Returns started no match (the window was not taking input)'; return }
            Judge 'return' $m
        }
    }
    if (Want 'click') {
        Launch 'click' {
            $m0 = HW-Mark
            [void](HW-Console 'jam.menuscreen where' 8)
            $pl = HW-Wait 'JAMMENU place [^\r\n]*PLAY NOW[^\r\n]*' 8 $m0
            if (-not $pl -or $pl -notmatch ' x=(\d+) y=(\d+) ') { Fail 'click' "jam.menuscreen where printed no PLAY NOW place ($pl)"; return }
            $cx = [int]$Matches[1]; $cy = [int]$Matches[2]
            if ($pl -match 'painted=0') { Fail 'click' "the PLAY NOW place is not painted: $pl"; return }
            HW-Note "PLAY NOW at client ($cx,$cy) from jam.menuscreen where"
            $m = HW-Mark
            if (-not (PressUntilStart { HW-Click $cx $cy } $m)) { Mach 'click' "four clicks at ($cx,$cy) started no match"; return }
            Judge 'click' $m
        }
    }
}
finally {
    Start-Sleep -Seconds 2
    CleanProfile
    if ($moved -and (Test-Path $aside)) { Rename-Item $aside $saved; HW-Note 'Saved put back' }
    ($rows.GetEnumerator() | ForEach-Object { "$($_.Key) = $($_.Value)" }) | Set-Content (Join-Path $out 'rows.txt')
}
# a real fault outranks a machine one: a fail is a fail even if the cursor was also shared
$verdict = if ($fails.Count) { 'fail' } elseif ($machine.Count) { 'machine' } else { 'pass' }
$code = @{ pass = 0; fail = 1; machine = 3 }[$verdict]
Write-Host (@{ tool = 'thirdjam.releasemenu/1'; verdict = $verdict; package = $Package; failed = @($fails); machine = @($machine); out = $out } | ConvertTo-Json -Compress)
exit $code
