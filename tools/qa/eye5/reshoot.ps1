# Lane A226 (track D2): the page's pictures shot again from another package, one command.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye5\reshoot.ps1 -Package C:\...\Game\Packaged\Accept_H44 -Tag h44
# -Package: a DEVELOPMENT package folder (the one that holds Windows\ThirdJam\Binaries\Win64\ThirdJam.exe, or its Windows folder). It is read only:
#           it is copied (robocopy /E /XJ, no link, without its Saved folder) to tools\qa\eye5\pkg-<tag>\Windows and only the copy is run, off screen.
#           A Shipping package cannot be filmed (it has no console) and is refused.
# -Tag:     the names of the pictures: submission\shots\<tag>-NN-<what>.png, <tag>-contact.jpg, <TAG>-PICKS.md (default h43r).
# -Full:    film the runs whole as on 15 Nov (tools\qa\eye5\runs.json: about 3 000 stills, 9 GB, 20 minutes) instead of only the picked frames.
# What it does: for every run that picks.json names, the same seeded match (the person-like hand, jam.ScriptHuman 2) or the same front-end walk is played
# and a still is taken at the picked sim frames; deliver.py then copies the picks. The same sim gives the same frames; if the package's sim changed
# (any rule, brain or cartridge change), the frames show other moments: look at the contact sheet before using it, and pick again from a -Full film.
# One film at a time; the game's pid is in Game\logs\a226\<run>.pid and it is stopped only by that id.
param([Parameter(Mandatory = $true)][string]$Package, [string]$Tag = 'h43r', [switch]$Full, [string]$Dest = '')
$ErrorActionPreference = 'Stop'
$here = $PSScriptRoot
$root = (Resolve-Path (Join-Path $here '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL reshoot: runs only from a worktree (root $root)"; exit 2 }
if (-not $Dest) { $Dest = Join-Path $root 'submission\shots' }
$src = (Resolve-Path $Package).Path
if (Test-Path (Join-Path $src 'Windows\ThirdJam\Binaries\Win64')) { $src = Join-Path $src 'Windows' }
$bin = Join-Path $src 'ThirdJam\Binaries\Win64'
if (-not (Test-Path (Join-Path $bin 'ThirdJam.exe'))) { Write-Host "FAIL reshoot: no Development exe at $bin\ThirdJam.exe (a Shipping package has ThirdJam-Win64-Shipping.exe and cannot be filmed)"; exit 2 }
if (Get-ChildItem $bin -Filter '*Shipping*' -ErrorAction SilentlyContinue) { Write-Host 'FAIL reshoot: a Shipping binary is in this package; never started'; exit 2 }
$sw = [Diagnostics.Stopwatch]::StartNew()
$pkg = Join-Path $here "pkg-$Tag\Windows"
if ($src.TrimEnd('\') -ne $pkg.TrimEnd('\')) {
    New-Item -ItemType Directory -Force (Split-Path $pkg) | Out-Null
    robocopy $src $pkg /E /XJ /XD (Join-Path $src 'ThirdJam\Saved') /NFL /NDL /NP /NJH /NJS /R:1 /W:1 | Out-Null
    if ($LASTEXITCODE -ge 8) { Write-Host "FAIL reshoot: the copy failed (robocopy $LASTEXITCODE)"; exit 2 }
}
$exe = Join-Path $pkg 'ThirdJam\Binaries\Win64\ThirdJam.exe'
$runs = Get-Content (Join-Path $here 'runs.json') -Raw | ConvertFrom-Json
$picks = Get-Content (Join-Path $here 'picks.json') -Raw | ConvertFrom-Json
$fail = 0
$ErrorActionPreference = 'Continue'
foreach ($name in @($picks | ForEach-Object { $_.run } | Select-Object -Unique)) {
    $r = $runs.$name
    if (-not $r) { Write-Host "FAIL reshoot: picks.json names run $name, runs.json has none"; $fail++; continue }
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $here 'eye5-run.ps1'), '-Exe', $exe, '-Run', "$Tag-$name", '-Level', $r.level, '-ResX', '1920', '-ResY', '1080', '-SettleSec', '1', '-MaxWallSec', '900')
    if ($r.menu) { $a += @('-Menu', $r.menu) }
    else {
        if ($r.choose) { $a += @('-Choose', $r.choose) }
        $a += @('-ScriptHuman', '2')
        if ($Full) { $a += @('-Every', "$($r.every)", '-Frames', "$($r.frames)", '-Dense', $r.dense) }
        else {
            # each picked frame f: every 6th frame from f - 80, every frame from f - 20 to f; 120 elsewhere (the runner slows to 30 on the way in).
            # A pick in the first 400 frames is reached before the driver can steer: the film then starts at a step that divides it (at most 6).
            $fs = @($picks | Where-Object { $_.run -eq $name -and $_.frame } | ForEach-Object { [int]$_.frame } | Sort-Object -Unique)
            $early = @($fs | Where-Object { $_ -lt 400 }); $se = 0
            if ($early.Count) { foreach ($k in 6, 5, 4, 3, 2, 1) { if (-not @($early | Where-Object { $_ % $k }).Count) { $se = $k; break } } }
            $d = @(); $last = -1
            foreach ($f in $fs) {
                if ($f -lt 400) { $d += "$($last + 1):$($f):$se"; $last = $f; continue }
                $a0 = [Math]::Max($last + 1, $f - 80); if ($f - 21 -ge $a0) { $d += "$($a0):$($f - 21):6"; $a0 = $f - 20 }; $d += "$($a0):$($f):1"; $last = $f
            }
            if ($se) { $a += @('-StartEvery', "$se") }
            $a += @('-Every', '120', '-Frames', "$($fs[-1] + 1)", '-Dense', ($d -join ','))
        }
    }
    Write-Host ("{0,6:n0}s  run {1}" -f $sw.Elapsed.TotalSeconds, $name)
    & powershell @a *> (Join-Path $here "out\$Tag-$name.console.txt")
    Get-Content (Join-Path $here "out\$Tag-$name.console.txt") | Select-String '^(PASS|FAIL) ' | ForEach-Object { Write-Host "         $($_.Line)"; if ($_.Line -like 'FAIL*') { $fail++ } }
}
& python (Join-Path $here 'deliver.py') (Join-Path $here 'out') "$Tag-" $Tag $Dest
if ($LASTEXITCODE -ne 0) { $fail++ }
Write-Host ("{0} reshoot {1} from {2} in {3:n0} s; pictures in {4}" -f $(if ($fail) { 'FAIL' } else { 'PASS' }), $Tag, $src, $sw.Elapsed.TotalSeconds, $Dest)
exit $(if ($fail) { 1 } else { 0 })
