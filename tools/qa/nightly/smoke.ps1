# Lane A57: smoke launch of a packaged Shipping build: start the inner exe windowed 1280x720, watch 30 s (frame counted by wall clock only here: a smoke, not a proof),
# sample the working set every 2 s, take one still of the client area at 25 s (menu drawn: not black), stop only the process started here.
#   powershell -NoProfile -ExecutionPolicy Bypass -File smoke.ps1 -Pkg <...\Windows> -OutDir <dir> [-Seconds 30]
# Prints one JSON line: {"tool":"a57.smoke","alive":bool,"seconds":30,"peak_ws_mb":..,"still":"<path>","still_mean":..,"exit":null|code}
param([Parameter(Mandatory = $true)][string]$Pkg, [Parameter(Mandatory = $true)][string]$OutDir, [int]$Seconds = 30)
$sh = Join-Path $Pkg 'ThirdJam\Binaries\Win64\ThirdJam-Win64-Shipping.exe'
if (-not (Test-Path $sh)) { $sh = Join-Path $Pkg 'ThirdJam\Binaries\Win64\ThirdJam.exe' }
New-Item -ItemType Directory -Force $OutDir | Out-Null
$plan = Join-Path $OutDir 'plan.json'
@{ seconds = $Seconds; steps = @(@{ at = 25; shot = 'menu' }) } | ConvertTo-Json -Depth 4 | Set-Content $plan -Encoding ascii
$shoot = Join-Path $PSScriptRoot '..\eyesgrid\shoot.ps1'
$t0 = Get-Date
$job = Start-Process powershell -ArgumentList @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', $shoot, '-Exe', $sh, '-PlanFile', $plan, '-OutDir', $OutDir, '-ResX', 1280, '-ResY', 720) -PassThru -WindowStyle Hidden
$peak = 0.0; $alive = $true; $seen = $false
while (-not $job.HasExited) {
    Start-Sleep -Seconds 2
    $g = @(Get-Process -Name 'ThirdJam-Win64-Shipping', 'ThirdJam' -ErrorAction SilentlyContinue | Where-Object { $_.StartTime -ge $t0.AddSeconds(-1) -and $_.Path -eq $sh })
    if ($g.Count -gt 0) { $seen = $true; foreach ($p in $g) { $peak = [math]::Max($peak, $p.WorkingSet64 / 1MB) } }
}
$cap = Get-Content (Join-Path $OutDir 'capture.log') -Raw -ErrorAction SilentlyContinue
if ($cap -match 'game exited at' -or $cap -notmatch 'stopped game pid') { $alive = $false }
if ($cap -match 'FAIL no UnrealWindow') { $alive = $false }
$still = Join-Path $OutDir 'menu.png'; $mean = $null
if (Test-Path $still) { Add-Type -AssemblyName System.Drawing; $b = New-Object System.Drawing.Bitmap $still; $s = 0.0; $n = 0; for ($x = 40; $x -lt $b.Width; $x += 97) { for ($y = 40; $y -lt $b.Height; $y += 89) { $c = $b.GetPixel($x, $y); $s += ($c.R + $c.G + $c.B) / 3.0; $n++ } }; $b.Dispose(); $mean = [math]::Round($s / $n, 1) }
[ordered]@{ tool = 'a57.smoke'; alive = $alive; seconds = $Seconds; peak_ws_mb = [math]::Round($peak); still = $still; still_mean = $mean } | ConvertTo-Json -Compress
