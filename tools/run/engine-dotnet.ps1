# engine-dotnet.ps1: dot-sourced by common.ps1. From device B's handoff B-to-A-0001 (ruling R-0002).
# Windows on ARM64: the launcher engine ships no win-arm64 .NET; GetDotnetPath.bat then falls through to the system runtime and UnrealBuildTool (.NET 10)
# cannot start (exit -2147450730). Forcing the engine's bundled x64 .NET (runs emulated) fixes build.ps1 and package.ps1. No-op on AMD64 or when already set.
function Set-EngineDotnet {
    param([string]$Engine)
    if ($env:PROCESSOR_ARCHITECTURE -ne 'ARM64' -or $env:UE_DOTNET_VERSION) { return }
    $root = Join-Path $Engine 'Engine\Binaries\ThirdParty\DotNet'
    $ver = Get-ChildItem $root -Directory -ErrorAction SilentlyContinue | Sort-Object Name -Descending | Select-Object -First 1
    if (-not $ver) { return }
    $d = Join-Path $ver.FullName 'win-x64'
    if ((Test-Path (Join-Path $ver.FullName 'win-arm64')) -or -not (Test-Path $d)) { return }
    $env:UE_DOTNET_VERSION = $ver.Name; $env:UE_DOTNET_ARCH = 'win-x64'; $env:UE_DOTNET_DIR = $d; $env:DOTNET_ROOT = $d
    $env:DOTNET_MULTILEVEL_LOOKUP = '0'; $env:DOTNET_ROLL_FORWARD = 'LatestMajor'; $env:PATH = "$d;$env:PATH"
    Write-Output "INFO engine-dotnet: ARM64 host, using bundled x64 .NET $($ver.Name)"
}
