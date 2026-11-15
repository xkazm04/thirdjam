# Lane A219: which ThirdJam executables were started from the desktop shell (Explorer: a double click, a shortcut), and when last.
# Reads the current user's UserAssist counters (names are ROT13; bytes 4..7 the run count, 60..67 the last start as a FILETIME). Read only.
# Only entries whose path names ThirdJam are shown. A start from a terminal or a script is not recorded there.
function Rot13([string]$s) {
    -join ($s.ToCharArray() | ForEach-Object {
        $c = [int]$_
        if ($c -ge 65 -and $c -le 90) { [char](($c - 65 + 13) % 26 + 65) }
        elseif ($c -ge 97 -and $c -le 122) { [char](($c - 97 + 13) % 26 + 97) }
        else { [char]$c }
    })
}
$base = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\UserAssist'
$rows = @()
foreach ($g in (Get-ChildItem $base -ErrorAction SilentlyContinue)) {
    $it = Get-Item (Join-Path $g.PSPath 'Count') -ErrorAction SilentlyContinue
    if (-not $it) { continue }
    foreach ($n in $it.GetValueNames()) {
        $d = Rot13 $n
        if ($d -notmatch 'ThirdJam') { continue }
        $b = $it.GetValue($n)
        $ft = 0
        if ($b.Length -ge 68) { $ft = [BitConverter]::ToInt64($b, 60) }
        $t = '-'
        if ($ft -gt 0) { $t = [DateTime]::FromFileTime($ft).ToString('MM-dd HH:mm:ss') }
        $rows += ('{0}  runs={1}  {2}' -f $t, [BitConverter]::ToInt32($b, 4), $d)
    }
}
"entries naming ThirdJam: $($rows.Count)"
"--- with a last start:"
$rows | Where-Object { $_ -notmatch '^-' } | Sort-Object
"--- any entry for H35 to H39, or outside Game\Packaged:"
$rows | Where-Object { $_ -match 'H3[5-9]' -or $_ -notmatch 'Game\\Packaged' }
