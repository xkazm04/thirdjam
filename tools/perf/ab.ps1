# Lane A124 (track Q3): interleaved A/B in one package through bot.ps1 (port 4706), arms by -dpcvars; every arm needs an explicit cvar list.
# Usage: powershell -File tools/perf/ab.ps1 -Prefix <p> [-Old <cvars>] [-New <cvars>] [-N 6] [-PlaySeconds 0] [-Exe <ThirdJam.exe>]
# Start arms: old = jam.StartQuietFrames=0,jam.PreloadRig=0; new = defaults. Six runs a tier per arm.
param([string]$Exe = 'Game\Packaged\A124-after\Windows\ThirdJam\Binaries\Win64\ThirdJam.exe', [int]$N = 6, [string]$Prefix = 'a124-ab', [string]$Old = 'jam.StartQuietFrames=0,jam.PreloadRig=0', [string]$New = 'jam.StartQuietFrames=4,jam.PreloadRig=1', [int]$PlaySeconds = 0, [string]$Arms = 'old,new',
  # Lane A150: 'play' starts each match through PLAY NOW (jam.menuscreen play); -OperatorP3: the p3 runs carry no -cut (the operator's hockey.regular on clay).
  [ValidateSet('open', 'play')][string]$StartPath = 'open', [switch]$OperatorP3)
$wt = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
Set-Location $wt
for ($i = 1; $i -le $N; $i++) {
  foreach ($tier in @('l2_night_5v5', 'p3_clay_5v5')) {
    $order = $Arms.Split(','); if ($i % 2 -eq 0) { [array]::Reverse($order) }
    foreach ($arm in $order) {
      $dp = if ($arm -eq 'old') { $Old } else { $New }
      $name = "$Prefix-$arm-$tier-$i"
      if (Test-Path "tools\perf\out\$name\csv") { continue }
      $extra = @('-StartPath', $StartPath); if ($OperatorP3 -and $tier -like 'p3*') { $extra += '-NoCut' }
      & powershell -NoProfile -ExecutionPolicy Bypass -File tools\perf\bot.ps1 -Exe $Exe -Tier $tier -Name $name -PlaySeconds $PlaySeconds -Dp $dp @extra | Select-Object -Last 2
    }
  }
}
