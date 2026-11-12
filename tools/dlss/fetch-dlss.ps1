<#
.SYNOPSIS
  Fetches NVIDIA's DLSS 4.5 Unreal plugin (v8.9.0, UE 5.8 build), installs the Super Resolution / DLAA part into Game/Plugins/
  and applies the Third Jam 5.9 port (format-string fixes). Lane A11, docs/experiments/DLSS.md.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\dlss\fetch-dlss.ps1
  powershell -NoProfile -ExecutionPolicy Bypass -File tools\dlss\fetch-dlss.ps1 -Zip D:\downloads\2026.10.16_UE5.8_DLSS4.5Plugin_v8.9.0.zip

.NOTES
  - Downloading accepts the NVIDIA RTX SDKs License (Documentation/NVIDIA RTX SDK EULA.pdf in the zip). Never commit the plugin
    (.gitignore: Game/Plugins/DLSS*/, Game/Plugins/Streamline*/).
  - Installs only Plugins/DLSS and Plugins/StreamlineNGXCommon: no Frame Generation, Reflex, NIS, DeepDVC, samples.
    Ray Reconstruction (nvngx_dlssd.dll), arm64 files, prebuilt 5.8 editor binaries, Intermediate and .pdb files are left out.
  - Nothing inside the archive is executed; the plugin is built later by UBT with the project (tools\run\build.ps1).
  - Re-running replaces Game/Plugins/DLSS and Game/Plugins/StreamlineNGXCommon.
  Exit: 0 installed, 1 failed (hash, patch), 2 could not run (download, disk).
#>
param(
    [string]$Zip = '',
    [string]$WorkDir = (Join-Path $env:TEMP 'thirdjam-dlss')
)
. "$PSScriptRoot\..\run\common.ps1"
Add-Type -AssemblyName System.IO.Compression.FileSystem

$Url = 'https://developer.nvidia.com/downloads/assets/gameworks/downloads/secure/dlss/ue-dlss-5.8/8.9.0/2026.10.16_ue5.8_dlss4.5plugin_v8.9.0.zip'
$ZipName = '2026.10.16_UE5.8_DLSS4.5Plugin_v8.9.0.zip'
$ZipBytes = 1166608575
$ZipSha256 = 'ADEEB59B9635794E05252438BF01307BF8A629769E8F0517237363F690BB380C'

$repo = (Resolve-Path (Join-Path $PSScriptRoot '..\..')).Path
$plugins = Join-Path $repo 'Game\Plugins'
if (-not (Test-Path $plugins)) { Write-Step 'FAIL' 'repo' "no $plugins"; Exit-WithResult 'fetch-dlss' 2 }

# 1. the zip
if (-not $Zip) {
    if (-not (Test-Path $WorkDir)) { New-Item -ItemType Directory -Force -Path $WorkDir | Out-Null }
    $Zip = Join-Path $WorkDir $ZipName
    if (-not ((Test-Path $Zip) -and ((Get-Item $Zip).Length -eq $ZipBytes))) {
        if ((Get-FreeGB $WorkDir) -lt 5) { Write-Step 'FAIL' 'disk' 'need 5 GB free for the download'; Exit-WithResult 'fetch-dlss' 2 }
        Write-Host "INFO fetch-dlss: downloading $ZipName (1.09 GiB) to $WorkDir"
        & curl.exe -sSL -o $Zip $Url
        if ($LASTEXITCODE -ne 0) { Write-Step 'FAIL' 'download' "curl exit $LASTEXITCODE"; Exit-WithResult 'fetch-dlss' 2 }
    }
}
if (-not (Test-Path $Zip)) { Write-Step 'FAIL' 'zip' "missing $Zip"; Exit-WithResult 'fetch-dlss' 2 }
$hash = (Get-FileHash -Algorithm SHA256 $Zip).Hash
if ($hash -ne $ZipSha256) { Write-Step 'FAIL' 'sha256' "$hash, expected $ZipSha256"; Exit-WithResult 'fetch-dlss' 1 }
Write-Step 'PASS' 'sha256' $hash

# 2. extract the two plugins, filtered
$keepPrefixes = @('Plugins/DLSS/', 'Plugins/StreamlineNGXCommon/')
function Test-Skip([string]$n) {
    if ($n.EndsWith('/')) { return $true }
    if ($n -match '/Intermediate/') { return $true }
    if ($n -match '\.pdb$') { return $true }
    if ($n -match '/WinArm64/' -or $n -match '/Lib/arm64/') { return $true }
    if ($n -match '^Plugins/[^/]+/Binaries/Win64/') { return $true }     # prebuilt UE 5.8 editor modules
    if ($n -match 'nvngx_dlssd\.dll$') { return $true }                   # Ray Reconstruction: not shipped
    return $false
}
foreach ($p in @('DLSS', 'StreamlineNGXCommon')) { $d = Join-Path $plugins $p; if (Test-Path $d) { Remove-Item -Recurse -Force $d } }
$z = [System.IO.Compression.ZipFile]::OpenRead($Zip)
$n = 0
try {
    foreach ($e in $z.Entries) {
        $name = $e.FullName.Replace('\', '/')
        $keep = $false; foreach ($k in $keepPrefixes) { if ($name.StartsWith($k)) { $keep = $true } }
        if (-not $keep -or (Test-Skip $name)) { continue }
        $rel = $name.Substring('Plugins/'.Length)
        $dest = [System.IO.Path]::GetFullPath((Join-Path $plugins $rel))
        if (-not $dest.StartsWith($plugins)) { Write-Step 'FAIL' 'extract' "path escapes the plugin folder: $name"; Exit-WithResult 'fetch-dlss' 1 }
        $dir = Split-Path $dest -Parent
        if (-not (Test-Path $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
        [System.IO.Compression.ZipFileExtensions]::ExtractToFile($e, $dest, $true)
        $n++
    }
} finally { $z.Dispose() }
Write-Step 'PASS' 'extract' "$n files into Game/Plugins/DLSS and Game/Plugins/StreamlineNGXCommon"

# 3. the 5.9 port: engine version and the format-string checker fixes (UE 5.9 rejects enum / int64 arguments for %d and %u)
$src = Join-Path $plugins 'DLSS\Source'
$edits = @(
    @{ f = '..\DLSS.uplugin'; a = '"EngineVersion": "5.8.0"'; b = '"EngineVersion": "5.9.0"' },
    @{ f = '..\..\StreamlineNGXCommon\StreamlineNGXCommon.uplugin'; a = '"EngineVersion": "5.8.0"'; b = '"EngineVersion": "5.9.0"' },
    @{ f = 'DLSSBlueprint\Private\DLSSLibrary.cpp'; a = 'invalid DLSSMode enum value (%d) \"%s\""'; b = 'invalid DLSSMode enum value (%lld) \"%s\"" /* Third Jam 5.9 port: %d -> %lld (int64) */'; all = $true },
    @{ f = 'DLSSBlueprint\Private\DLSSLibrary.cpp'; a = 'from the higher level code"), InDLSSQualityMode);'; b = 'from the higher level code"), int32(InDLSSQualityMode)); // Third Jam 5.9 port: enum cast' },
    @{ f = 'DLSSBlueprint\Private\DLSSLibrary.cpp'; a = '%u "), DLSSModule->QueryDLSSSRSupport());'; b = '%u "), uint32(DLSSModule->QueryDLSSSRSupport())); // Third Jam 5.9 port: enum cast' },
    @{ f = 'DLSS\Private\DLSSUpscaler.cpp'; a = 'TEXT("%u is not a valid Quality mode"), DLSSQualityMode);'; b = 'TEXT("%u is not a valid Quality mode"), uint32(DLSSQualityMode)); // Third Jam 5.9 port: enum cast for the format checker' },
    @{ f = 'DLSS\Private\DLSSUpscaler.cpp'; a = 'GetMaxUpsampleResolutionFraction(), DLSSQualityMode);'; b = 'GetMaxUpsampleResolutionFraction(), int32(DLSSQualityMode)); // Third Jam 5.9 port: enum cast' },
    @{ f = 'DLSS\Private\DLSSUpscaler.cpp'; a = 'TEXT("%u is not a valid Quality mode"), Quality);'; b = 'TEXT("%u is not a valid Quality mode"), uint32(Quality)); // Third Jam 5.9 port: enum cast'; all = $true }
)
foreach ($ed in $edits) {
    $path = [System.IO.Path]::GetFullPath((Join-Path $src $ed.f))
    $text = [System.IO.File]::ReadAllText($path)
    if (-not $text.Contains($ed.a)) { Write-Step 'FAIL' 'patch' ("{0}: text not found: {1}" -f $ed.f, $ed.a); Exit-WithResult 'fetch-dlss' 1 }
    $text = $text.Replace($ed.a, $ed.b)
    [System.IO.File]::WriteAllText($path, $text, (New-Object System.Text.UTF8Encoding($false)))
}
Write-Step 'PASS' 'patch' ("{0} edits (5.9 port)" -f $edits.Count)

$mb = [math]::Round(((Get-ChildItem -Recurse -File (Join-Path $plugins 'DLSS'), (Join-Path $plugins 'StreamlineNGXCommon') | Measure-Object Length -Sum).Sum) / 1MB, 1)
Write-Step 'PASS' 'installed' "$mb MB on disk; next: tools\run\build.ps1 -Project Game\ThirdJam.uproject"
Exit-WithResult 'fetch-dlss' 0 @{ zip = $Zip; sha256 = $hash; installed_mb = $mb }
