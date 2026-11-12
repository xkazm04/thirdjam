# Lane A10: copy a capture into devlog/shots as a JPEG under 300 KB, named by the devlog convention (CET time now).
#   powershell -File tools\qa\devshot.ps1 -File <png> -Label feel-<what> [-Caption "..."] [-Width 1280] [-Quality 82]
param([string]$File, [string]$Label, [string]$Caption = '', [int]$Width = 1280, [int]$Quality = 82)
Add-Type -AssemblyName System.Drawing
$src = [System.Drawing.Image]::FromFile($File)
$h = [int]($src.Height * $Width / $src.Width)
$bmp = New-Object System.Drawing.Bitmap $Width, $h
$g = [System.Drawing.Graphics]::FromImage($bmp); $g.InterpolationMode = 'HighQualityBicubic'; $g.DrawImage($src, 0, 0, $Width, $h)
$codec = [System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object { $_.MimeType -eq 'image/jpeg' }
$ep = New-Object System.Drawing.Imaging.EncoderParameters 1
$ep.Param[0] = New-Object System.Drawing.Imaging.EncoderParameter ([System.Drawing.Imaging.Encoder]::Quality), $Quality
$stamp = (Get-Date).ToString('yyyyMMdd-HHmmss')
$name = "$stamp-A-$Label.jpg"
$dst = Join-Path 'C:\Users\kazda\kiro\thirdjam\devlog\shots' $name
$bmp.Save($dst, $codec, $ep); $g.Dispose(); $bmp.Dispose(); $src.Dispose()
if ($Caption) { Set-Content -Encoding utf8 ($dst -replace '\.jpg$', '.txt') $Caption }
$size = (Get-Item $dst).Length
"$(if ($size -lt 300KB) {'PASS'} else {'FAIL'}) $name $([math]::Round($size/1KB)) KB"
