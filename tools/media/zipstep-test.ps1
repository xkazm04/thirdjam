<#
 Unit run of package.ps1's README + zip + submission-zip steps (lane A96), no build, no UAT.
 Replicates the folder list of an existing staged folder (empty dirs, small top-level files real, large files as 1 KB stand-ins)
 under $env:TEMP, then runs the same statements package.ps1 runs after stage-clean. Prints PASS/FAIL.
 usage: powershell -NoProfile -File tools\media\zipstep-test.ps1 -Staged <...\Hokej_Hokej_H25\Windows>
#>
param([Parameter(Mandatory=$true)][string]$Staged)
$ErrorActionPreference = 'Stop'
$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$work = Join-Path $env:TEMP 'a96-zipstep'
if (Test-Path $work) { Remove-Item $work -Recurse -Force }
$out = Join-Path $work 'out'; $replica = Join-Path $out 'Windows'
New-Item -ItemType Directory -Force -Path $replica | Out-Null
$Staged = (Resolve-Path $Staged).Path
foreach ($d in Get-ChildItem $Staged -Recurse -Directory -Force) { New-Item -ItemType Directory -Force -Path (Join-Path $replica $d.FullName.Substring($Staged.Length + 1)) | Out-Null }
$n = 0
foreach ($f in Get-ChildItem $Staged -Recurse -File -Force) {
  $dst = Join-Path $replica $f.FullName.Substring($Staged.Length + 1)
  if ($f.DirectoryName -eq $Staged -and $f.Length -lt 1MB) { Copy-Item $f.FullName $dst } else { [IO.File]::WriteAllBytes($dst, (New-Object byte[] 1024)) }
  $n++
}
Write-Host "INFO replica: $n files, $((Get-ChildItem $replica -Recurse -Directory).Count) dirs from $Staged"
# ---- the statements of package.ps1 (copied, same variables) ----
$staged = $replica; $PSScriptRoot_ = Join-Path $repo 'tools\run'; $Out = $out
$Submission = $true; $SubmissionName = 'Hokej_Hokej'; $Name = 'Hokej_Hokej_H26'
$extra = [ordered]@{}
function Write-Step($s, $k, $m) { Write-Host "$s $k $m" }
$readmeSrc = Join-Path $PSScriptRoot_ '..\..\submission\package\README.txt'
if (Test-Path $readmeSrc) {
    Copy-Item -Path $readmeSrc -Destination (Join-Path $staged 'README.txt') -Force
    Write-Step 'PASS' 'readme' 'README.txt staged at the package root'
} else { Write-Step 'FAIL' 'readme' "$readmeSrc not found" }
$zipPath = Join-Path $Out ($Name + '.zip')
. "$PSScriptRoot_\zipfwd.ps1"
New-ZipForwardSlash -SourceDir $staged -ZipPath $zipPath
Add-Type -AssemblyName System.IO.Compression.FileSystem
$zipMB = [math]::Round((Get-Item $zipPath).Length / 1MB, 3)
Write-Step 'PASS' 'zip' "$zipPath $zipMB MB"
if ($Submission -and $Name -ne $SubmissionName) {
    $subZip = Join-Path $Out ($SubmissionName + '.zip')
    Copy-Item -Path $zipPath -Destination $subZip -Force
    Write-Step 'PASS' 'submission-zip' "$subZip"
}
# ---- assertions ----
$z = [System.IO.Compression.ZipFile]::OpenRead((Join-Path $Out 'Hokej_Hokej.zip'))
$names = @($z.Entries | ForEach-Object { $_.FullName }); $z.Dispose()
$ok = $true
$bs = @($names | Where-Object { $_.Contains('\') })
if ($bs.Count -gt 0) { Write-Step 'FAIL' 'slash' "$($bs.Count) entries with a backslash"; $ok = $false } else { Write-Step 'PASS' 'slash' 'no backslash in any entry name' }
foreach ($want in 'Windows/README.txt', 'Windows/ThirdJam.exe', 'Windows/NOTICES.txt', 'Windows/ThirdJam/Content/Paks/ThirdJam-Windows.pak') { if ($names -notcontains $want) { Write-Step 'FAIL' 'entry' "missing $want"; $ok = $false } else { Write-Step 'PASS' 'entry' $want } }
$same = (Get-FileHash (Join-Path $Out 'Hokej_Hokej.zip')).Hash -eq (Get-FileHash (Join-Path $Out 'Hokej_Hokej_H26.zip')).Hash
Write-Step ($(if ($same) { 'PASS' } else { 'FAIL' })) 'twins' 'Hokej_Hokej.zip is byte-identical to Hokej_Hokej_H26.zip'
Write-Host ("INFO entries in zip: {0}, top folders: {1}" -f $names.Count, (($names | ForEach-Object { $_.Split('/')[0] } | Sort-Object -Unique) -join ','))
Remove-Item $work -Recurse -Force
if ($ok -and $same) { exit 0 } else { exit 1 }
