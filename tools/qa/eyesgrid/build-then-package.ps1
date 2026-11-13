# A54: the worktree has no editor binaries, which BuildCookRun -nocompileeditor needs for the cook (FileNotFound ThirdJamEditor.target).
# Build the Editor target (compile only, no editor is opened), then package Development; both retry while another lane holds the UBT mutex.
$root = Split-Path (Split-Path (Split-Path $PSScriptRoot -Parent) -Parent) -Parent
$log = "$PSScriptRoot\package-retry.log"
function Try-Run($label, $script, $argsList) {
    for ($i = 1; $i -le 20; $i++) {
        "$(Get-Date -Format 'HH:mm:ss') $label try $i" | Add-Content $log
        & powershell -NoProfile -ExecutionPolicy Bypass -File "$root\tools\run\$script" @argsList *> "$PSScriptRoot\$label.out"
        if ($LASTEXITCODE -eq 0) { "$(Get-Date -Format 'HH:mm:ss') $label ok" | Add-Content $log; return $true }
        $t = Get-Content "$PSScriptRoot\$label.out" -Raw
        if ($t -notmatch 'Error_SDKNotFound|ConflictingInstance') { "$(Get-Date -Format 'HH:mm:ss') $label real failure" | Add-Content $log; return $false }
        Start-Sleep -Seconds 90
    }
    return $false
}
if (-not (Test-Path "$root\Game\Binaries\Win64\ThirdJamEditor.target")) {
    if (-not (Try-Run 'build-editor' 'build.ps1' @('-Project', "$root\Game", '-Target', 'Editor', '-Config', 'Development'))) { exit 1 }
}
if (-not (Try-Run 'package-dev' 'package.ps1' @('-Project', "$root\Game", '-Out', "$root\Game\Packaged\A54-dev", '-Config', 'Development'))) { exit 1 }
