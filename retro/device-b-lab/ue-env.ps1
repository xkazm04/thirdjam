# ue-env.ps1: device B (Windows on ARM64) only. Dot-source it before any Unreal batch file runs:
#   . C:\Users\kazda\jam\ue-env.ps1            (engine 5.9)
#   . C:\Users\kazda\jam\ue-env.ps1 -Assoc 5.8
# The launcher engine ships only an x64 .NET; Engine\Build\BatchFiles\GetDotnetPath.bat asks for win-arm64 on this host and
# UnrealBuildTool then cannot start. This sets the bundled x64 .NET for THIS PROCESS and its children only (nothing machine- or
# user-wide: a global DOTNET_ROOT would break native ARM64 .NET programs). Lives outside the repo because tools/** belongs to
# device A (handoff B-to-A-0001 asks for the same logic in tools/run/common.ps1).
param([string]$Assoc = '5.9')

$tjEngine = $null
$tjDat = 'C:\ProgramData\Epic\UnrealEngineLauncher\LauncherInstalled.dat'
if (Test-Path $tjDat) {
    try {
        foreach ($i in (Get-Content $tjDat -Raw | ConvertFrom-Json).InstallationList) {
            if ($i.AppName -eq ('UE_' + $Assoc) -and (Test-Path $i.InstallLocation)) { $tjEngine = $i.InstallLocation }
        }
    } catch {}
}
if (-not $tjEngine) { $f = 'C:\Program Files\Epic Games\UE_' + $Assoc; if (Test-Path $f) { $tjEngine = $f } }

if ($env:PROCESSOR_ARCHITECTURE -ne 'ARM64' -and -not ${env:ProgramFiles(Arm)}) {
    Write-Host 'INFO ue-env: not an ARM64 host; nothing to do'
} elseif (-not $tjEngine) {
    Write-Host "WARN ue-env: engine UE_$Assoc not found; nothing set"
} else {
    $tjRoot = Join-Path $tjEngine 'Engine\Binaries\ThirdParty\DotNet'
    $tjVer = Get-ChildItem $tjRoot -Directory -ErrorAction SilentlyContinue | Sort-Object { [version]$_.Name } -Descending | Select-Object -First 1
    if (-not $tjVer) {
        Write-Host "WARN ue-env: no bundled .NET under $tjRoot; nothing set"
    } elseif (Test-Path (Join-Path $tjVer.FullName 'win-arm64')) {
        Write-Host 'INFO ue-env: the engine ships win-arm64 .NET; nothing to do'
    } elseif (-not (Test-Path (Join-Path $tjVer.FullName 'win-x64\dotnet.exe'))) {
        Write-Host "WARN ue-env: no win-x64 .NET under $($tjVer.FullName); nothing set"
    } else {
        $tjDir = Join-Path $tjVer.FullName 'win-x64'
        $env:UE_DOTNET_VERSION = $tjVer.Name
        $env:UE_DOTNET_ARCH = 'win-x64'
        $env:UE_DOTNET_DIR = $tjDir
        $env:DOTNET_ROOT = $tjDir
        $env:DOTNET_MULTILEVEL_LOOKUP = '0'
        $env:DOTNET_ROLL_FORWARD = 'LatestMajor'
        if (($env:PATH -split ';')[0] -ne $tjDir) { $env:PATH = "$tjDir;$env:PATH" }
        Write-Host "INFO ue-env: UE_$Assoc bundled .NET $($tjVer.Name) win-x64 set for this process ($tjDir)"
    }
}
