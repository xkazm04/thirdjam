# Lane A135 (track U1): one command to a game process's remote console (the stills process of u1-stills.ps1, on this lane's own port).
#   powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-console.ps1 -Port 4735 -Cmd "jam.ui.prompt brake"
# Exit 0: the game answered ok. 1: it did not.
param([int]$Port = 4735, [Parameter(Mandatory = $true)][string]$Cmd, [int]$WaitSec = 6)
try {
    $c = New-Object System.Net.Sockets.TcpClient; $c.ReceiveTimeout = $WaitSec * 1000; $c.Connect('127.0.0.1', $Port)
    $s = $c.GetStream(); $w = New-Object IO.StreamWriter($s); $w.NewLine = "`n"; $w.AutoFlush = $true; $r = New-Object IO.StreamReader($s)
    $w.WriteLine('{"id":1,"op":"hello","version":1}'); $null = $r.ReadLine()
    $w.WriteLine((@{ id = 2; op = 'exec'; cmd = $Cmd } | ConvertTo-Json -Compress))
    $ans = $null
    for ($i = 0; $i -lt 50; $i++) { $l = $r.ReadLine(); if ($null -eq $l) { break }; if ($l.StartsWith('{"id":2,')) { $ans = $l; break } }
    $c.Close()
    if ($ans -and $ans.Contains('"ok":true')) { Write-Host "PASS u1-console $Port '$Cmd'"; exit 0 }
    Write-Host "FAIL u1-console $Port '$Cmd': $ans"; exit 1
} catch { Write-Host "FAIL u1-console $Port '$Cmd': $($_.Exception.Message)"; exit 1 }
