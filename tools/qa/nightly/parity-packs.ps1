# Lane A57: the two parity packages of one commit, Development then Shipping (no zip), one after the other.
param([Parameter(Mandatory = $true)][string]$Tag)
$here = $PSScriptRoot
& powershell -NoProfile -ExecutionPolicy Bypass -File "$here\pack.ps1" -Config Development -Name "Hokej_Hokej_N$Tag-dev"
$d = $LASTEXITCODE
& powershell -NoProfile -ExecutionPolicy Bypass -File "$here\pack.ps1" -Config Shipping -Name "Hokej_Hokej_N$Tag"
$s = $LASTEXITCODE
"parity packs dev=$d ship=$s" | Add-Content "$here\out\pack.log"
