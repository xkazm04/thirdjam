# Lane A158 (track U1, evolution 5): knockdowns captured off screen, measured by the HUD's own probe (jam.hud.DownProbe).
# One editor-binary game process per seed, all at once (few starts, many seeds), each rendering off screen (-RenderOffScreen: no window, no cursor) on the
# Rink map with the track's five-a-side cut on one look tier and the scripted human. Every knockdown gives a `JAMSCREEN down` line (the frames seen, the
# frames on which a HUD element's painted box met the downed figure's projected box, by element) and, for the human's, two stills (25 and 75 frames in).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after  -Seeds 1,2,3,4,5 [-PlaySeconds 135] [-Tier clay]
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode before -Seeds 1,2,3,4,5      (jam.hud.ClearDown 0, jam.hud.OwnMarkOnNet 1)
# Before and after runs may play at once: the knockdown stills carry a tag in their names (jam.hud.DownStillTag: a<seed> / b<seed>).
# The HUD reads the sim and writes nothing back, so -Mode before and -Mode after play the same matches (the JAMSIM hashes in sim-lines.txt say so).
# Out: tools\qa\screen\out\<name>\seed<N>\{game.log, downs.txt, shots\}, and summary.txt: knockdowns of the human, those with no HUD element over the
# figure on any frame, the frames hit, by element. Exit 0: every process played and at least one knockdown of the human was seen; 1 otherwise; 2 could not run.
# -StillEvery N adds the track's periodic stills (u1-<tier>-f<frame>.png) for ONE seed only (the names carry no seed).
param(
    [string]$Seeds = '1,2,3,4,5',   # a comma list in one string (powershell -File does not pass an array)
    [ValidateSet('after', 'before')][string]$Mode = 'after',
    [ValidateSet('clay', 'night')][string]$Tier = 'clay',
    [ValidateSet(90, 140)][int]$Rink = 90,
    [int]$PlaySeconds = 135,
    [int]$StillAt = 25,
    [int]$StillEvery = 0,
    [int]$ResX = 1280, [int]$ResY = 720,
    [string]$ExtraCVars = '',
    [string]$RunName = '',
    [switch]$CamLog,
    [int]$MarkStills = 0,          # lane A171: jam.hud.MarkStills N (lane A166's stills of the two ice marks, N frames into each showing)
    [int]$CheckCooldown = 0,       # lane A171: game.contact.check_cooldown_frames for this run (0 = the cut's own: off)
    [string]$GameKeys = '',        # lane A171: JSON put first inside the cut's "game" object, e.g. '"contact": { "check_cooldown_frames": 40 },'
    [int]$StartTimeoutSec = 900
)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$exe = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor.exe'
if (-not $uproject -or -not (Test-Path $exe)) { Write-Host "FAIL u5-downs: no project or no $exe"; exit 2 }
if ($CheckCooldown -gt 0) { $GameKeys = '"contact": { "check_cooldown_frames": ' + $CheckCooldown + ' },' }
$SeedList = @($Seeds.Split(',') | Where-Object { $_.Trim() } | ForEach-Object { [int]$_.Trim() })
if ($StillEvery -gt 0 -and $SeedList.Count -ne 1) { Write-Host 'FAIL u5-downs: -StillEvery needs exactly one seed'; exit 2 }
$name = if ($RunName) { $RunName } else { "downs-$Tier-$Mode" }
$out = Join-Path $root "tools\qa\screen\out\$name"
New-Item -ItemType Directory -Force $out | Out-Null
$tierId = if ($Tier -eq 'clay') { 'p3_clay' } else { 'l2_night' }
$cutName = if ($Rink -eq 140) { "u1-cut-$Tier-140.json" } else { "u1-cut-$Tier.json" }
$cutText = Get-Content (Join-Path $root "tools\qa\screen\$cutName") -Raw
$shotDir = Join-Path $root 'Game\Saved\Screenshots\WindowsEditor'
$cvars = "jam.ScriptHuman=1,jam.screen.FadeSeconds=0,jam.hud.DownProbe=600,jam.hud.DownStills=$StillAt,jam.hud.MarkerProbe=600,jam.camera.Probe=600"
if ($StillEvery -gt 0) { $cvars += ",jam.screen.StillEvery=$StillEvery" }
if ($CamLog) { $cvars += ',jam.camera.Log=1' }
if ($Mode -eq 'before') { $cvars += ',jam.hud.ClearDown=0,jam.hud.OwnMarkOnNet=1' }   # evolution 5 at its neutral values
if ($ExtraCVars) { $cvars += ",$ExtraCVars" }
$sw = [Diagnostics.Stopwatch]::StartNew()
$t0 = Get-Date
$runs = @()
foreach ($seed in $SeedList) {
    $tag = "$($Mode.Substring(0, 1))$seed$(if ($Rink -eq 140) { 'w' })"   # in the knockdown stills' names: one screenshot folder, many processes
    $dir = Join-Path $out "seed$seed"
    New-Item -ItemType Directory -Force (Join-Path $dir 'shots') | Out-Null
    Get-ChildItem (Join-Path $dir 'shots') -Filter '*.png' -ErrorAction SilentlyContinue | Remove-Item -Force
    $cut = Join-Path $dir 'cut.json'
    $cutSeed = $cutText -replace '"seed":\s*\d+', "`"seed`": $seed"
    if ($GameKeys) { $k = $cutSeed.IndexOf('"game": {'); if ($k -lt 0) { Write-Host 'FAIL u5-downs: the cut has no "game": { to put -GameKeys in'; exit 2 }; $cutSeed = $cutSeed.Insert($k + 9, "`n    $GameKeys") }
    [IO.File]::WriteAllText($cut, $cutSeed, (New-Object Text.UTF8Encoding($false)))
    $log = Join-Path $dir 'game.log'
    Remove-Item $log -ErrorAction SilentlyContinue
    $marker = "u5run=$name-seed$seed-$PID"
    $a = "`"$uproject`" /Game/Maps/Rink -game -RenderOffScreen -Multiprocess -ResX=$ResX -ResY=$ResY -ForceRes -unattended -nosplash -nosound -log -abslog=`"$log`" -ForceLogFlush " +
         "-cut=$($cut.Replace('\', '/')) -dpcvars=$cvars,jam.hud.DownStillTag=$tag$(if ($MarkStills -gt 0) { ",jam.hud.MarkStills=$MarkStills,jam.hud.MarkStillTag=$tierId-$tag" }) -$marker"
    $p = Start-Process -FilePath $exe -ArgumentList $a -PassThru -WindowStyle Hidden
    $runs += [pscustomobject]@{ seed = $seed; tag = $tag; dir = $dir; log = $log; marker = $marker; p = $p; start = $p.StartTime; built = $false; builtAt = 0.0 }
    Write-Host ("{0,7:n1}s  started seed {1} pid {2} ({3} {4})" -f $sw.Elapsed.TotalSeconds, $seed, $p.Id, $tierId, $Mode)
}
@{ lane = 'A158'; pids = @($runs | ForEach-Object { $_.p.Id }); start = $t0.ToString('o'); cvars = $cvars } | ConvertTo-Json -Compress | Set-Content (Join-Path $out 'run.json')
function FindLog($r) {
    # -abslog honoured: nothing to do. Else the log under Game\Saved\Logs written since the start whose command line carries this run's marker.
    if (Test-Path $r.log) { return }
    Get-ChildItem (Join-Path $root 'Game\Saved\Logs') -Filter '*.log' -ErrorAction SilentlyContinue | Where-Object { $_.Name -notmatch 'backup' -and $_.LastWriteTime -ge $r.start.AddSeconds(-2) } | ForEach-Object {
        if ($r.log -like '*\game.log') {
            try {
                $fs = [IO.File]::Open($_.FullName, 'Open', 'Read', 'ReadWrite'); $buf = New-Object byte[] 262144; $n = $fs.Read($buf, 0, $buf.Length); $fs.Close()
                if ([Text.Encoding]::UTF8.GetString($buf, 0, $n).Contains($r.marker)) { $r.log = $_.FullName }
            } catch { }
        }
    }
}
function LogHas($r, [string]$pat) {
    FindLog $r
    if (-not (Test-Path $r.log)) { return $false }
    try { $fs = [IO.File]::Open($r.log, 'Open', 'Read', 'ReadWrite'); $sr = New-Object IO.StreamReader($fs); $t = $sr.ReadToEnd(); $sr.Close(); $fs.Close(); return $t.Contains($pat) } catch { return $false }
}
# Wait for every match to be built, then play.
while ($sw.Elapsed.TotalSeconds -lt $StartTimeoutSec) {
    foreach ($r in $runs) { if (-not $r.built -and -not $r.p.HasExited -and (LogHas $r 'JAMSCREEN light hold')) { $r.built = $true; $r.builtAt = $sw.Elapsed.TotalSeconds; Write-Host ("{0,7:n1}s  seed {1}: the match is built" -f $sw.Elapsed.TotalSeconds, $r.seed) } }
    if (@($runs | Where-Object { -not $_.built -and -not $_.p.HasExited }).Count -eq 0) { break }
    Start-Sleep -Milliseconds 500
}
$last = ($runs | Where-Object { $_.built } | Measure-Object -Property builtAt -Maximum).Maximum
if ($null -eq $last) { $last = $sw.Elapsed.TotalSeconds }
while ($sw.Elapsed.TotalSeconds - $last -lt $PlaySeconds -and @($runs | Where-Object { -not $_.p.HasExited }).Count -gt 0) { Start-Sleep -Milliseconds 500 }
# Stop our own processes (started here, by pid and start time), collect.
foreach ($r in $runs) { if (-not $r.p.HasExited) { [void](Stop-OwnedTree -ProcessId $r.p.Id -StartTime $r.start) } }
Start-Sleep -Seconds 2
$sum = @()
$tot = @{ downs = 0; clean = 0; frames = 0; hit = 0; odowns = 0; oclean = 0; oframes = 0; ohit = 0 }
$by = @{}; $oby = @{}
foreach ($r in $runs) {
    FindLog $r
    if (-not (Test-Path $r.log)) { $sum += "seed $($r.seed): NO LOG (built=$($r.built))"; continue }
    if ($r.log -ne (Join-Path $r.dir 'game.log')) { Copy-Item $r.log (Join-Path $r.dir 'game.log') -Force }
    $glog = Join-Path $r.dir 'game.log'
    $lines = Select-String -Path $glog -Pattern 'JAMSCREEN down who=' | ForEach-Object { $_.Line -replace '^.*JAMSCREEN down ', '' }
    $lines | Set-Content (Join-Path $r.dir 'downs.txt')
    Select-String -Path $glog -Pattern 'JAMSCREEN downs |JAMSCREEN camera|JAMSCREEN marker|JAMSIM frame=|JAMLOOK tier=|JAMCUT rejected|JAMMATCH level|JAMHUD frame=\d+ (mark|contact_line|cue|mark_still)' | ForEach-Object { $_.Line -replace '^\[[^\]]*\]\[[^\]]*\]', '' } | Set-Content (Join-Path $r.dir 'sim-lines.txt')
    $h = 0; $hc = 0
    foreach ($l in $lines) {
        if ($l -notmatch 'who=(\d+) human=(\d) start=(\d+) end=(\d+) seen=(\d+) hit_frames=(\d+) by=(\S+)') { continue }
        $isH = $Matches[2] -eq '1'; $seen = [int]$Matches[5]; $hit = [int]$Matches[6]; $who = $Matches[7]
        if ($isH) { $h++; $tot.downs++; $tot.frames += $seen; $tot.hit += $hit; if ($hit -eq 0) { $hc++; $tot.clean++ } }
        else { $tot.odowns++; $tot.oframes += $seen; $tot.ohit += $hit; if ($hit -eq 0) { $tot.oclean++ } }
        if ($who -ne 'none') { foreach ($kv in $who.Split(',')) { $k, $v = $kv.Split(':'); if ($isH) { $by[$k] = [int]$by[$k] + [int]$v } else { $oby[$k] = [int]$oby[$k] + [int]$v } } }
    }
    $n = 0
    Get-ChildItem $shotDir -Filter "u1-*$tierId*.png" -ErrorAction SilentlyContinue | Where-Object { $_.LastWriteTime -ge $r.start } | ForEach-Object {
        # a knockdown still carries its process's tag (jam.hud.DownStillTag); a periodic still belongs to the one seed of the run
        $mine = if ($_.Name -like 'u1-down-*') { $_.Name -like "u1-down-$tierId-$($r.tag)-f*" } elseif ($_.Name -like 'u1-mark-*') { $_.Name -like "u1-mark-$tierId-$($r.tag)-*" } else { $SeedList.Count -eq 1 }
        if ($mine) { Copy-Item $_.FullName (Join-Path $r.dir "shots\$($_.Name)") -Force; $n++ }
    }
    $sum += "seed $($r.seed): the human down $h times, $hc with no HUD element over him; $n stills; built=$($r.built) exit=$($r.p.HasExited)"
}
$fmt = { param($m) if ($m.Count -eq 0) { 'none' } else { ($m.GetEnumerator() | Sort-Object Value -Descending | ForEach-Object { "$($_.Key):$($_.Value)" }) -join ',' } }
$sum += ("TOTAL {0} {1}: the human's knockdowns {2}, with no HUD element over the figure on any frame {3}; frames seen down {4}, frames with an element over him {5}, by element: {6}" -f $name, $Mode, $tot.downs, $tot.clean, $tot.frames, $tot.hit, (& $fmt $by))
$sum += ("OTHERS: knockdowns {0}, clean {1}; frames {2}, hit {3}, by element: {4}" -f $tot.odowns, $tot.oclean, $tot.oframes, $tot.ohit, (& $fmt $oby))
$sum | Set-Content (Join-Path $out 'summary.txt')
$sum | ForEach-Object { Write-Host $_ }
$ok = (@($runs | Where-Object { -not $_.built }).Count -eq 0) -and $tot.downs -gt 0
Write-Host ("{0} u5-downs {1} ({2:n0} s)" -f $(if ($ok) { 'PASS' } else { 'FAIL' }), $name, $sw.Elapsed.TotalSeconds)
if ($ok) { exit 0 } else { exit 1 }
