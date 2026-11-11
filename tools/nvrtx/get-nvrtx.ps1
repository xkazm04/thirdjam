# get-nvrtx.ps1: clone NVIDIA's RTX branch of Unreal (NvRTX) into a separate folder, run Setup, generate project files,
# and build the editor. The whole script runs at BelowNormal priority (child processes inherit it). Stage by stage, each
# logged; stops at the first failure.
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File tools/nvrtx/get-nvrtx.ps1 -Branch nvrtx-5.8 -Dest C:\Users\kazda\kiro\nvrtx-5.8 [-NoBuild]
# Access: the GitHub account must be linked to an Epic account with the NvRTX invite accepted (git reuses the gh credential).
param(
  [string]$Branch = 'nvrtx-5.8',
  [string]$Dest = 'C:\Users\kazda\kiro\nvrtx-5.8',
  [switch]$NoBuild,
  [int]$MinFreeGB = 60
)
$ErrorActionPreference = 'Continue'
try { [System.Diagnostics.Process]::GetCurrentProcess().PriorityClass = 'BelowNormal' } catch {}
$parent = Split-Path $Dest -Parent
$log = Join-Path $parent ("nvrtx-" + $Branch + ".log")
$out = Join-Path $parent ("nvrtx-" + $Branch + ".stdout.log")
function Say($m) { $line = (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + ' ' + $m; Write-Output $line; Add-Content -Path $log -Value $line -Encoding utf8 }
function FreeGB { [math]::Round((Get-PSDrive -Name ($Dest.Substring(0,1))).Free / 1GB, 1) }
function Run($cmdline, $cwd) {
  Add-Content -Path $out -Value ("==== " + (Get-Date -Format 'HH:mm:ss') + " " + $cmdline) -Encoding utf8
  # Nested quotes from PowerShell to cmd.exe are unreliable: write the stage as a .cmd file and run that.
  $stage = Join-Path $env:TEMP 'nvrtx-stage.cmd'
  @('@echo off', ('cd /d "' + $cwd + '"'), ($cmdline + ' >> "' + $out + '" 2>&1'), 'exit /b %ERRORLEVEL%') | Set-Content -Path $stage -Encoding ascii
  cmd.exe /c "$stage"
  $code = $LASTEXITCODE
  Get-Content $out -Tail 5 | ForEach-Object { Say ("  | " + $_) } | Out-Null  # Out-Null: a function returns ALL output, not only `return`
  return [int]$code
}
Say "start branch=$Branch dest=$Dest free=$(FreeGB) GB"
if ((FreeGB) -lt $MinFreeGB) { Say "FAIL free space below $MinFreeGB GB"; exit 1 }
if (-not (Test-Path (Join-Path $Dest '.git'))) {
  $t = Get-Date
  $code = Run "git clone --depth 1 --branch $Branch --single-branch https://github.com/NvRTX/UnrealEngine `"$Dest`"" $parent
  Say ("clone exit=$code in {0:N0} s, free=$(FreeGB) GB" -f ((Get-Date) - $t).TotalSeconds)
  if ($code -ne 0) { Say 'FAIL clone'; exit 1 }
} else { Say 'clone: folder exists, skipping' }
$t = Get-Date
$code = Run ("call `"" + (Join-Path $Dest 'Setup.bat') + "`" --force") $Dest  # absolute path: cmd may not resolve a bare .bat from the cwd
Say ("Setup.bat exit=$code in {0:N0} s, free=$(FreeGB) GB" -f ((Get-Date) - $t).TotalSeconds)
if ($code -ne 0) { Say 'FAIL Setup.bat'; exit 1 }
$t = Get-Date
$code = Run ("call `"" + (Join-Path $Dest 'GenerateProjectFiles.bat') + "`"") $Dest
Say ("GenerateProjectFiles exit=$code in {0:N0} s" -f ((Get-Date) - $t).TotalSeconds)
if ($code -ne 0) { Say 'FAIL GenerateProjectFiles'; exit 1 }
if ($NoBuild) { Say 'PASS (no build requested)'; exit 0 }
if ((FreeGB) -lt $MinFreeGB) { Say "FAIL free space below $MinFreeGB GB before build"; exit 1 }
$t = Get-Date
$code = Run ("call `"" + (Join-Path $Dest 'Engine\Build\BatchFiles\Build.bat') + "`" UnrealEditor Win64 Development -WaitMutex") $Dest
Say ("Build UnrealEditor exit=$code in {0:N0} min, free=$(FreeGB) GB" -f ((Get-Date) - $t).TotalMinutes)
if ($code -ne 0) { Say 'FAIL build'; exit 1 }
$size = [math]::Round((Get-ChildItem $Dest -Recurse -File -ErrorAction SilentlyContinue | Measure-Object Length -Sum).Sum / 1GB, 1)
Say "PASS folder size $size GB, free=$(FreeGB) GB"
exit 0
