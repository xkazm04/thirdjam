# Lane A57: package the worktree's commit through tools\run\package.ps1 with -ubtargs=-WaitMutex, retrying while another lane holds the UBT mutex
# (ConflictingInstance / Error_SDKNotFound). Output: <main>\Game\Packaged\<Name>. Exit 0 packaged, 1 failed.
#   powershell -NoProfile -ExecutionPolicy Bypass -File pack.ps1 -Config Shipping -Name Hokej_Hokej_N1010 [-Zip]
param([Parameter(Mandatory = $true)][ValidateSet('Development', 'Shipping')][string]$Config, [Parameter(Mandatory = $true)][string]$Name, [switch]$Zip, [int]$Tries = 12)
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$out = "C:\Users\kazda\kiro\thirdjam\Game\Packaged\$Name"
$outTxt = Join-Path $PSScriptRoot "out\pack-$Name.txt"
New-Item -ItemType Directory -Force (Split-Path $outTxt) | Out-Null
for ($i = 1; $i -le $Tries; $i++) {
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "$root\tools\run\package.ps1", '-Project', "$root\Game", '-Out', $out, '-Config', $Config, '-ExtraArgs', '-ubtargs=-WaitMutex')
    if ($Zip) { $a += @('-Zip', '-Name', 'Hokej_Hokej') }
    & powershell @a *> $outTxt
    $code = $LASTEXITCODE
    "$(Get-Date -Format 'HH:mm:ss') pack $Name $Config try $i exit $code" | Add-Content (Join-Path $PSScriptRoot 'out\pack.log')
    if ($code -eq 0) { exit 0 }
    $t = Get-Content $outTxt -Raw
    if ($t -notmatch 'Error_SDKNotFound|ConflictingInstance|already running') { exit 1 }
    Start-Sleep -Seconds 90
}
exit 1
