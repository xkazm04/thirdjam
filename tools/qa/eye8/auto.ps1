# Lane A230 (from lane A219): the automation filter of this worktree in a cleared environment (no JAM_* variable), headless (-nullrhi), no window.
# The editor-cmd's process id goes to Game\logs\a230\<name>.pid. Prints the counts and the failed lines.
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\eye8\auto.ps1 -Filter Jam -Name auto-1
param([string]$Filter = 'Jam', [string]$Name = 'auto', [int]$TimeoutSec = 2400)
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\\.claude\\worktrees\\') { Write-Host "FAIL auto: runs only from a worktree"; exit 2 }
. (Join-Path $root 'tools\run\common.ps1')
$uproject = Resolve-UProject (Join-Path $root 'Game')
$engine = Get-EngineRoot $uproject
$cmd = Join-Path $engine 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$dir = Join-Path $root 'Game\logs\a230'
New-Item -ItemType Directory -Force $dir | Out-Null
$log = Join-Path $dir "$Name.log"
Remove-Item $log -ErrorAction SilentlyContinue
$cleared = @()
Get-ChildItem Env: | Where-Object { $_.Name -like 'JAM_*' } | ForEach-Object { $cleared += $_.Name; Remove-Item "Env:$($_.Name)" }
$a = "`"$uproject`" -ExecCmds=`"Automation RunTests $Filter; Quit`" -nullrhi -Multiprocess -unattended -nopause -nosplash -ABSLOG=`"$log`""
$p = Start-Process -FilePath $cmd -ArgumentList $a -PassThru -WindowStyle Hidden
$null = $p.Handle
"$($p.Id) $($p.StartTime.ToString('o')) $Name UnrealEditor-Cmd.exe" | Set-Content (Join-Path $dir "$Name.pid")
Write-Host "started pid $($p.Id) filter $Filter (cleared: $($cleared -join ', '))"
if (-not $p.WaitForExit($TimeoutSec * 1000)) { Write-Host "FAIL auto: still running after $TimeoutSec s (pid $($p.Id) left alone)"; exit 1 }
$ok = @(Select-String -Path $log -Pattern 'Test Completed\. Result=\{Success\}' -ErrorAction SilentlyContinue).Count
$bad = @(Select-String -Path $log -Pattern 'Test Completed\. Result=\{Fail[^}]*\}' -ErrorAction SilentlyContinue)
foreach ($b in ($bad | Select-Object -First 20)) { Write-Host "  | $($b.Line.Trim())" }
$flag = @(Select-String -Path $log -Pattern 'missing usage flag' -ErrorAction SilentlyContinue).Count
Write-Host ("{0} auto {1}: {2} Success, {3} Fail, usage-flag lines {4}, exit {5} (log {6})" -f $(if ($bad.Count -eq 0 -and $ok -gt 0) { 'PASS' } else { 'FAIL' }), $Filter, $ok, $bad.Count, $flag, $p.ExitCode, $log)
if ($bad.Count -eq 0 -and $ok -gt 0) { exit 0 } else { exit 1 }
