# Lane A230: one build of this worktree through tools\run\build.ps1 (its slot queue untouched); the child's process id goes to Game\logs\a230\<name>.pid.
param([string]$Name = 'build', [string]$Config = 'Development', [string]$Target = 'Editor')
$root = (Resolve-Path (Join-Path $PSScriptRoot '..\..\..')).Path
if ($root -notmatch '\\.claude\worktrees\') { Write-Host "FAIL build: runs only from a worktree"; exit 2 }
$dir = Join-Path $root 'Game\logs\a230'
New-Item -ItemType Directory -Force $dir | Out-Null
$out = Join-Path $dir "$Name.out"
$a = "-NoProfile -ExecutionPolicy Bypass -File `"$root\tools\run\build.ps1`" -Project `"$root\Game`" -Config $Config -Target $Target -TimeoutSec 3000"
$p = Start-Process -FilePath powershell -ArgumentList $a -PassThru -WindowStyle Hidden -RedirectStandardOutput $out -RedirectStandardError "$out.err"
$null = $p.Handle
"$($p.Id) $($p.StartTime.ToString('o')) $Name powershell build.ps1 $Config $Target" | Set-Content (Join-Path $dir "$Name.pid")
Write-Host "started pid $($p.Id)"
$p.WaitForExit()
Get-Content $out | Select-Object -Last 6
exit $p.ExitCode
