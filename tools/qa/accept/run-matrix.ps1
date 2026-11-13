# A108: the acceptance matrix, sequential (one cursor). run-matrix.ps1 -Package <dir> [-Only a,b] ; names: reg1..3, regcart, mode-<slug>, tier-<t>
param([string]$Package, [string[]]$Only = @(), [int]$Port = 4708)
$Only = @($Only | % { $_ -split "," });
$here = $PSScriptRoot; $cuts = Join-Path $here 'matrix-cuts'
$jobs = @()
1..3 | % { $jobs += @{ n = "m-reg$_"; cut = '' } }
$jobs += @{ n = 'm-regcart'; cut = (Join-Path (Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent) 'Game\Content\Cartridges\hockey.regular.cartridge.json') }
Get-ChildItem $cuts -Filter 'mode-*.json' | % { $jobs += @{ n = 'm-' + $_.BaseName; cut = $_.FullName } }
Get-ChildItem $cuts -Filter 'tier-*.json' | % { $jobs += @{ n = 'm-' + $_.BaseName; cut = $_.FullName } }
foreach ($j in $jobs) {
  if ($Only.Count -and -not ($Only | ? { $j.n -eq "m-$_" -or $j.n -eq $_ })) { continue }
  Write-Host "=== $($j.n) $(Get-Date -Format HH:mm:ss)"
  $a = @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $here 'accept.ps1'),'-Package',$Package,'-Name',$j.n,'-Port',$Port)
  if ($j.cut) { $a += @('-Cut', $j.cut) }
  & powershell @a | Select-Object -Last 1
  Write-Host "exit $LASTEXITCODE"
}
