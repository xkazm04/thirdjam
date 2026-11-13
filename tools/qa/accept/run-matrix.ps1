# A108/A131: the acceptance matrix, sequential (one cursor). run-matrix.ps1 -Package <dir> [-Only a,b] [-Repeat N] [-Retries 2]
# names: reg1..3, regcart, mode-<slug>, tier-<t>. A run that ends as a machine verdict (exit 3: the hand was late, a foreign game shared the cursor) is repeated up to -Retries
# times (folder <name>-m1, -m2 keep the lost runs); a pass or a fail is final. -Repeat N plays each selected job N times (<name>-r1..rN) for the stability proof.
param([string]$Package, [string[]]$Only = @(), [int]$Port = 4708, [int]$Repeat = 1, [int]$Retries = 2)
$Only = @($Only | % { $_ -split "," });
$here = $PSScriptRoot; $cuts = Join-Path $here 'matrix-cuts'
$root = Split-Path (Split-Path (Split-Path $here -Parent) -Parent) -Parent
$jobs = @()
1..3 | % { $jobs += @{ n = "m-reg$_"; cut = '' } }
$jobs += @{ n = 'm-regcart'; cut = (Join-Path $root 'Game\Content\Cartridges\hockey.regular.cartridge.json'); sec = 200 }   # first_to 3: the drill's goals end the match after step 4, nothing more comes
Get-ChildItem $cuts -Filter 'mode-*.json' | % { $jobs += @{ n = 'm-' + $_.BaseName; cut = $_.FullName } }
Get-ChildItem $cuts -Filter 'tier-*.json' | % { $jobs += @{ n = 'm-' + $_.BaseName; cut = $_.FullName } }
foreach ($j in $jobs) {
  if ($Only.Count -and -not ($Only | ? { $j.n -eq "m-$_" -or $j.n -eq $_ })) { continue }
  for ($r = 1; $r -le $Repeat; $r++) {
    $base = if ($Repeat -gt 1) { "$($j.n)-r$r" } else { $j.n }
    for ($try = 0; $try -le $Retries; $try++) {
      $name = if ($try -eq 0) { $base } else { "$base-t$try" }
      Write-Host "=== $name $(Get-Date -Format HH:mm:ss)"
      $a = @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $here 'accept.ps1'),'-Package',$Package,'-Name',$name,'-Port',$Port)
      if ($j.cut) { $a += @('-Cut', $j.cut) }
      if ($j.sec) { $a += @('-Seconds', $j.sec) }
      & powershell @a | Select-Object -Last 1
      $code = $LASTEXITCODE
      Write-Host "exit $code"
      if ($code -ne 3) { break }
      Write-Host "machine verdict: repeating $base"
    }
  }
}
