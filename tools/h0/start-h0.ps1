# start-h0.ps1: the scheduled start of the jam on device A. Registered with register-h0.ps1 to fire at 2026-11-12 06:00 local time.
# Runs Claude Code headless in the repo with the H0 prompt, logs everything to logs/h0/, and never asks a question (there is nobody to answer).
param(
  [string]$Repo = 'C:\Users\kazda\kiro\thirdjam',
  [string]$Model = 'claude-fable-5-2',
  [int]$MaxHours = 8,
  [string]$Prompt = 'tools\h0\H0-PROMPT.md'
)
$ErrorActionPreference = 'Continue'
$logDir = Join-Path $Repo 'logs\h0'
New-Item -ItemType Directory -Force -Path $logDir | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$log = Join-Path $logDir ("h0-" + $stamp + ".log")
$transcript = Join-Path $logDir ("h0-" + $stamp + ".jsonl")
function Say($m) { $line = (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') + ' ' + $m; Add-Content -Path $log -Value $line -Encoding utf8 }
Say "start model=$Model repo=$Repo maxHours=$MaxHours"
# Wait for 06:00 local if fired early (the task is set for 06:00; this guards a clock jump).
$h0 = Get-Date -Year 2026 -Month 11 -Day 12 -Hour 6 -Minute 0 -Second 0
while ((Get-Date) -lt $h0) { Start-Sleep -Seconds 30 }
Say "H0 reached"
$prompt = Get-Content (Join-Path $Repo $Prompt) -Raw
Set-Location $Repo
$env:MSYS_NO_PATHCONV = '1'
$env:TJ_MIN_FREE_GB = '40'
$job = Start-Job -ScriptBlock {
  param($Repo, $Model, $prompt, $transcript)
  Set-Location $Repo
  $prompt | & claude -p --model $Model --permission-mode bypassPermissions --output-format stream-json --verbose 2>&1 | Out-File -FilePath $transcript -Encoding utf8
  return $LASTEXITCODE
} -ArgumentList $Repo, $Model, $prompt, $transcript
Say "claude started as job $($job.Id); transcript $transcript"
$done = Wait-Job $job -Timeout ($MaxHours * 3600)
if ($null -eq $done) {
  Say "time box of $MaxHours h reached; stopping the headless session (its commits are already pushed)"
  Stop-Job $job
} else {
  $code = Receive-Job $job
  Say "claude exited with $code"
}
Remove-Job $job -Force
Say "end"
