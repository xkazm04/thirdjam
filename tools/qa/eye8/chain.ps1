# Lane A230: films one after another (ONE editor film at a time: they share Game\Saved\Screenshots\WindowsEditor). Each line of -List: run|choose|resX|resY[|extra cvars[|frames[|every]]]
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye8\chain.ps1 -List "t-arsenal-1080|modes/hockey.mode.arsenal|1920|1080;..."
param([string]$List, [string]$Level = 'rookie')
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
$dir = Join-Path $root 'Game\logs\a230'
New-Item -ItemType Directory -Force $dir | Out-Null
foreach ($item in ($List -split ';')) {
    if (-not $item.Trim()) { continue }
    $f = $item.Trim() -split '\|'
    $run = $f[0]; $choose = $f[1]; $rx = $f[2]; $ry = $f[3]
    $extra = ''; if ($f.Count -gt 4) { $extra = $f[4] }
    $frames = 3600; if ($f.Count -gt 5 -and $f[5]) { $frames = [int]$f[5] }
    $every = 30; if ($f.Count -gt 6 -and $f[6]) { $every = [int]$f[6] }
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', (Join-Path $PSScriptRoot 'eye8-run.ps1'), '-Editor', '-Run', $run, '-Level', $Level, '-Choose', $choose, '-ScriptHuman', '2', '-Every', $every, '-Frames', $frames, '-ResX', $rx, '-ResY', $ry)
    if ($extra) { $a += @('-ExtraCVars', $extra) }
    & powershell @a *> (Join-Path $dir "$run.out")
    Write-Host ("{0} {1}" -f (Get-Date -Format 'HH:mm:ss'), (Get-Content (Join-Path $dir "$run.out") | Select-Object -Last 1))
}
