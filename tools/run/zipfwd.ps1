# New-ZipForwardSlash (lane A96): a zip whose entry names use '/' as the zip format requires.
# Windows PowerShell 5.1 (.NET Framework) ZipFile.CreateFromDirectory writes '\' in entry names, which Explorer and 7-Zip accept
# but many other unzippers (python, unzip, macOS Archive Utility) turn into one flat folder of oddly named files. A judge may use any.
# Dot-source it: . "$PSScriptRoot\zipfwd.ps1"
function New-ZipForwardSlash {
    param([Parameter(Mandatory = $true)][string]$SourceDir, [Parameter(Mandatory = $true)][string]$ZipPath)
    Add-Type -AssemblyName System.IO.Compression
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $src = (Resolve-Path $SourceDir).Path.TrimEnd('\')
    $base = Split-Path $src -Leaf                      # the top folder inside the zip ("Windows"), as CreateFromDirectory(includeBase)
    $fs = [System.IO.File]::Open($ZipPath, [System.IO.FileMode]::Create)
    try {
        $za = New-Object System.IO.Compression.ZipArchive($fs, [System.IO.Compression.ZipArchiveMode]::Create)
        try {
            foreach ($f in (Get-ChildItem -Path $src -Recurse -File -Force)) {
                $rel = $base + '/' + $f.FullName.Substring($src.Length + 1).Replace('\', '/')
                [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile($za, $f.FullName, $rel, [System.IO.Compression.CompressionLevel]::Optimal)
            }
            foreach ($d in (Get-ChildItem -Path $src -Recurse -Directory -Force)) {
                if (-not (Get-ChildItem -Path $d.FullName -Force | Select-Object -First 1)) {   # empty folders keep their entry
                    [void]$za.CreateEntry($base + '/' + $d.FullName.Substring($src.Length + 1).Replace('\', '/') + '/')
                }
            }
        } finally { $za.Dispose() }
    } finally { $fs.Dispose() }
}
