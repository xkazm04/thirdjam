# register-h0.ps1: register (or remove) the Windows scheduled task that starts the jam at 2026-11-12 06:00 local time.
# Usage: powershell -NoProfile -ExecutionPolicy Bypass -File tools/h0/register-h0.ps1 [-Remove] [-At "2026-11-12 06:00"]
param(
  [switch]$Remove,
  [string]$At = '2026-11-12 06:00',
  [string]$Repo = 'C:\Users\kazda\kiro\thirdjam',
  [string]$Prompt = 'tools\h0\H0-PROMPT.md',
  [string]$TaskName = 'ThirdJam-H0'
)
$name = $TaskName
if ($Remove) { schtasks /Delete /TN $name /F; exit $LASTEXITCODE }
$when = [datetime]::Parse($At)
$script = Join-Path $Repo 'tools\h0\start-h0.ps1'
$cmd = "powershell.exe -NoProfile -ExecutionPolicy Bypass -WindowStyle Hidden -File `"$script`" -Repo `"$Repo`" -Prompt `"$Prompt`""
schtasks /Create /F /TN $name /SC ONCE /SD ($when.ToString('MM/dd/yyyy')) /ST ($when.ToString('HH:mm')) /TR $cmd /RL LIMITED
if ($LASTEXITCODE -ne 0) { Write-Output "FAIL schtasks create exit $LASTEXITCODE"; exit 1 }
schtasks /Query /TN $name /V /FO LIST | Select-String -Pattern 'TaskName|Next Run Time|Status|Task To Run'
Write-Output "PASS task $name at $($when.ToString('yyyy-MM-dd HH:mm')) local"
