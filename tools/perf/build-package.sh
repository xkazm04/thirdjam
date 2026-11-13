#!/usr/bin/env bash
# Lane A58: Editor target (needed by -nocompileeditor) then the Development package into Game/Packaged/A58-dev.
set -u
WT="$(cd "$(dirname "$0")/../.." && pwd -W)"
cd "$WT"
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/build.ps1 -Project "$WT/Game" -Target Editor -Config Development
echo "EDITOR_BUILD_EXIT=$?"
powershell -NoProfile -ExecutionPolicy Bypass -File tools/run/package.ps1 -Project "$WT/Game" -Out "$WT/Game/Packaged/A58-dev" -Config Development -ExtraArgs "-ubtargs=-WaitMutex"
echo "PACKAGE_EXIT=$?"
