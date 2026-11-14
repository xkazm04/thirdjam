# Lane A155 (S2 evolution 3, row 2): the menu row. Plays the REAL first screen through the window (no drill), and judges what the match that is PLAYED is,
# by the rule the first run of this row got wrong (H27: the first JAMMATCH line said rookie, the second one, 0.15 s later, was the match and said pro, and drew a black world):
#   - exactly ONE `JAMMATCH` and ONE `JAMCART loaded` per match start (a second build is a fault),
#   - the LAST JAMMATCH line (the match that plays) has the expected level and source,
#   - a `JAMLOOK tier=` line is there, and the still taken 9 s after the start is not black (lit share >= 0.20; a rendered rink reads 0.99).
# Scenarios (-Scenarios, default all):
#   rookie-return   clean profile, one Return on PLAY NOW                       -> level=rookie source=settings
#   rookie-click    clean profile, a real click on PLAY NOW                     -> level=rookie source=settings
#   pro-click       clean profile, click PRO (JAMMENU level picked=pro, ini DifficultyChosen=True), quit, start again: PRO lit (still), the settings still say it,
#                   then PLAY NOW by Return                                     -> level=pro source=settings
#   pro-direct      click PRO then click PLAY NOW in the same launch            -> level=pro source=settings
#   pro-return      click PRO then ONE Return                                   -> level=pro source=settings (Return must still start PLAY NOW)
#   mode            CHOOSE YOUR CUT, the first mode card (Pond)                 -> source=cartridge (its own level)
#   season          PLAY THE SEASON                                             -> source=cartridge (the take's own level)
#   launch          -cut=hockey.regular on the command line, one Return         -> source=launch
#   walk            (package only) `jam.menuscreen walk` in the window: its verdict line (blind when the window is not the active one)
# Package mode (default): the package's own Saved folder is renamed aside for the clean-profile scenarios and put back in a finally block.
# Editor mode (-Editor <UnrealEditor.exe> -Project <x.uproject>): the uncooked game as `-game -windowed`; no profile is moved (the project's Saved is shared with other sessions: the
# scenarios then do not assume a clean profile and the first two are told so in the row), the window is closed by WM_CLOSE.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\menu-row.ps1 [-Package Game\Packaged\Accept_H27] [-Name a155-menu] [-Scenarios rookie-return,mode]
param([string]$Package = 'C:\Users\kazda\kiro\thirdjam\Game\Packaged\Accept_H27', [string]$Name = 'a155-menu', [int]$Port = 4708,
      [string[]]$Scenarios = @(), [string]$Editor = '', [string]$Project = '')
$Scenarios = @($Scenarios | ForEach-Object { $_ -split ',' })
. (Join-Path $PSScriptRoot 'winhand.ps1')
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = Join-Path $root "tools\qa\out\menu\$Name"
$isEditor = [bool]$Editor
if ($isEditor) { $exe = $Editor; $pre = "`"$Project`" -game -Multiprocess"; $saved = Join-Path (Split-Path $Project -Parent) 'Saved'; $aside = '' }
else { $stage = Join-Path (Resolve-Path $Package).Path 'Windows\ThirdJam'; $exe = Join-Path $stage 'Binaries\Win64\ThirdJam.exe'; $pre = ''; $saved = Join-Path $stage 'Saved'; $aside = Join-Path $stage 'Saved.a155aside' }
$res = [ordered]@{}; $fails = New-Object System.Collections.ArrayList
function Row([string]$k, [string]$v) { $res[$k] = $v; HW-Note "ROW $k = $v" }
function Verdict([string]$k, [bool]$ok, [string]$why) { Row $k ($(if ($ok) { 'PASS' } else { 'FAIL' }) + " $why"); if (-not $ok) { [void]$fails.Add($k) } }
function Want([string]$n) { return ($Scenarios.Count -eq 0 -or $Scenarios -contains $n) }
function Clean { if ($isEditor) { return }; if (Test-Path $saved) { Remove-Item $saved -Recurse -Force -ErrorAction SilentlyContinue } }
function Settings { $f = Get-ChildItem $saved -Recurse -Filter '*.ini' -ErrorAction SilentlyContinue | Where-Object { $_.Name -ne 'CrashReportClient.ini' }; $t = ''; foreach ($x in $f) { $c = Get-Content $x.FullName -Raw; if ($c -match 'Jam\.Settings') { $t += "[$($x.Name)] " + (([regex]::Match($c, '(?s)\[[^\]]*Jam\.Settings[^\]]*\].*?(?=\r?\n\[|\z)')).Value -replace '\s+', ' ') } }; return $t }
function Close-Game { if ($isEditor) { [void](HW-Close) } else { [void](HW-Close -ByConsole) }; Start-Sleep -Seconds 2 }
# one Return, repeated (as accept.ps1 does) when no `JAMMENU start match` follows within 6 s: under load the first screen has not always taken its focus yet
function PressReturn([int]$m) { for ($i = 0; $i -lt 4; $i++) { HW-Key 'Return'; if (HW-Wait 'JAMMENU start match' 6 $m) { return } ; HW-Note "Return $($i + 1) started nothing, again" } }
# one launch: opens the menu, waits for it, runs the body, closes
function Launch([string]$tag, [string]$extra, [scriptblock]$body) {
    $o = Join-Path $out $tag
    $a = ("$pre $extra").Trim()
    if (-not (HW-Start $exe $a $o -Port $Port -Console:(-not $isEditor) -WaitSec 180)) { Row "$tag.launch" 'FAIL no window'; [void]$fails.Add("$tag.launch"); return }
    $null = HW-Wait "Game class is 'JamMenuGameMode'" 150
    Start-Sleep -Seconds 4
    & $body $o
    Close-Game
}
# the judgement of one match start: $m is the log offset taken just before the press
function Judge([string]$k, [int]$m, [string]$level, [string]$source, [string]$tag, [bool]$needTakenBack = $false) {
    Start-Sleep -Seconds 9
    $matches_ = @(HW-Lines 'JAMMATCH level=' $m); $carts = @(HW-Lines 'JAMCART loaded' $m); $looks = @(HW-Lines 'JAMLOOK tier=' $m)
    $f = HW-Shot "$tag-match"; $px = HW-Pixels $f
    $last = if ($matches_.Count) { ($matches_[-1] -replace '^.*?JAMMATCH', 'JAMMATCH') } else { '(none)' }
    $lv = ''; $src = ''; if ($last -match 'level=(\S+) source=(\S+)') { $lv = $Matches[1]; $src = $Matches[2] }
    $why = @()
    if ($matches_.Count -ne 1) { $why += "JAMMATCH x$($matches_.Count) (want 1)" }
    if ($carts.Count -ne 1) { $why += "JAMCART loaded x$($carts.Count) (want 1)" }
    if ($level -and $lv -ne $level) { $why += "level=$lv (want $level)" }
    if ($src -ne $source) { $why += "source=$src (want $source)" }
    if ($looks.Count -lt 1) { $why += 'no JAMLOOK tier line' }
    $tb = @(HW-Lines 'JAMDESIGNER scheduled apply taken back' $m).Count
    if ($needTakenBack -and $tb -lt 1) { $why += 'no `JAMDESIGNER scheduled apply taken back` line' }
    if ($px.lit -lt 0.2) { $why += "BLACK FRAME lit=$($px.lit) mean=$($px.mean)" }
    Verdict $k ($why.Count -eq 0) ("last=[$last] builds=$($matches_.Count)/$($carts.Count) looks=$($looks.Count) taken_back=$tb pixels lit=$($px.lit) mean=$($px.mean)" + $(if ($why.Count) { ' :: ' + ($why -join '; ') } else { '' }))
}
if ($aside -and (Test-Path $aside)) { Write-Host "FAIL $aside already exists: a previous run did not put the profile back; not touching it"; exit 2 }
New-Item -ItemType Directory -Force $out | Out-Null
$moved = $false
try {
    if ($aside -and (Test-Path $saved)) { Rename-Item $saved $aside; $moved = $true; HW-Note 'Saved renamed aside' }
    if ($isEditor) { Row 'note' 'editor mode: the profile is the project Saved as it is; level expectations for the first two scenarios assume DifficultyChosen is not set there' }
    if (Want 'rookie-return') {
        Clean
        Launch 'rookie-return' '' { param($o) HW-Shot 'first-screen' | Out-Null; $m = HW-Mark; $t0 = [DateTime]::UtcNow; if ($isEditor) { HW-Click 788 551; Start-Sleep -Milliseconds 500 }; $m = HW-Mark; PressReturn $m; Judge 'rookie-return' $m 'rookie' 'settings' 'rookie-return' $true }
    }
    if (Want 'rookie-click') {
        Clean
        Launch 'rookie-click' '' { param($o) $m = HW-Mark; if ($isEditor) { HW-Click 788 551; Start-Sleep -Milliseconds 500 }; $m = HW-Mark; HW-Click 960 472; Judge 'rookie-click' $m 'rookie' 'settings' 'rookie-click' $true }
    }
    if (Want 'pro-click') {
        Clean
        Launch 'pro-click-1' '' { param($o) $m = HW-Mark; HW-Click 959 551; $l = HW-Wait 'JAMMENU level picked=\w+' 6 $m; Start-Sleep -Milliseconds 600; HW-Shot 'pro-lit' | Out-Null; Row 'pro-click.picked' "$l"; Row 'pro-click.settings' (Settings) }
        Row 'pro-click.settings_after_quit' (Settings)
        Verdict 'pro-click.saved' ((Settings) -match 'Difficulty=pro' -and (Settings) -match 'DifficultyChosen=True') 'settings file after quit'
        Launch 'pro-click-2' '' { param($o) HW-Shot 'pro-after-restart' | Out-Null; $m = HW-Mark; PressReturn $m; Judge 'pro-click' $m 'pro' 'settings' 'pro-click' $true }
    }
    if (Want 'pro-direct') {
        # PRO lit in this same launch (no restart: no dependence on the saved file), then PLAY NOW by a click
        Launch 'pro-direct' '' { param($o) HW-Click 959 551; Start-Sleep -Milliseconds 700; HW-Shot 'pro-lit' | Out-Null; $m = HW-Mark; HW-Click 960 472; Judge 'pro-direct' $m 'pro' 'settings' 'pro-direct' $true }
    }
    if (Want 'pro-return') {
        Clean
        # PRO clicked, then ONE Return: the segments take no focus, so Return must still start PLAY NOW
        Launch 'pro-return' '' { param($o) HW-Click 959 551; Start-Sleep -Milliseconds 700; $m = HW-Mark; PressReturn $m; Judge 'pro-return' $m 'pro' 'settings' 'pro-return' $true }
    }
    if (Want 'mode') {
        Launch 'mode' '' { param($o) HW-Click 960 702; Start-Sleep -Seconds 2; HW-Shot 'cuts-screen' | Out-Null; $m = HW-Mark; HW-Click 519 221; Judge 'mode' $m '' 'cartridge' 'mode' }
    }
    if (Want 'season') {
        Launch 'season' '' { param($o) $m = HW-Mark; HW-Click 960 635; Judge 'season' $m '' 'cartridge' 'season' }
    }
    if (Want 'launch') {
        Launch 'launch' '-cut=hockey.regular' { param($o) $m = HW-Mark; PressReturn $m; Judge 'launch' $m '' 'launch' 'launch' $true }
    }
    if ((Want 'walk') -and -not $isEditor) {
        Launch 'walk' '' { param($o) $m = HW-Mark; [void](HW-Console 'jam.menuscreen walk' 10); $w = HW-Wait 'JAMMENU walk done[^\r\n]*' 120 $m; Row 'walk' "$w" }
    }
}
finally {
    Start-Sleep -Seconds 2
    Clean
    if ($moved -and (Test-Path $aside)) { Rename-Item $aside $saved; HW-Note 'Saved put back' }
    ($res.GetEnumerator() | ForEach-Object { "$($_.Key) = $($_.Value)" }) | Set-Content (Join-Path $out 'rows.txt')
}
$verdictLine = @{ tool = 'thirdjam.menurow/1'; verdict = $(if ($fails.Count) { 'fail' } else { 'pass' }); failed = @($fails); out = $out } | ConvertTo-Json -Compress
Write-Host $verdictLine
if ($fails.Count) { exit 1 } else { exit 0 }
