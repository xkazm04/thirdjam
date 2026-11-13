param([int]$X = 1000, [int]$Y = 330, [int]$Seconds = 60)
Add-Type -Name C -Namespace W -MemberDefinition '[DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);'
$end = (Get-Date).AddSeconds($Seconds)
while ((Get-Date) -lt $end) { [W.C]::SetCursorPos($X, $Y) | Out-Null; Start-Sleep -Milliseconds 500 }
