#!/usr/bin/env bash
# run_live.sh CUT NAME MAXFRAMES TIMEOUT [extra dpcvars]: one headless game process of this worktree on port 4772 (lane A172), the camera's pose logged.
# The process logs to Game/Saved/Logs/ThirdJam.log on this tree (-abslog was not honoured on 14 Nov 06:00); the log is copied to Game/logs/NAME.log.
# Stops only the process it started (found by this worktree's path and port 4772 in its command line).
set -u
W=$(pwd -W); CUT=$1; NAME=$2; MAXF=${3:-6000}; TIMEOUT=${4:-240}; EXTRA=${5:+,$5}
UE="C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
SAVED="$W/Game/Saved/Logs/ThirdJam.log"
MSYS_NO_PATHCONV=1 "$UE" "$W/Game/ThirdJam.uproject" /Game/Maps/Rink -game -nullrhi -Multiprocess -unattended -nosplash -NoSound -log \
    -cut="$CUT" -dpcvars=t.MaxFPS=60,jam.ScriptHuman=1,jam.camera.Log=1$EXTRA -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=4772 > /dev/null 2>&1 &
T0=$SECONDS; sleep 20
while [ $((SECONDS - T0)) -lt "$TIMEOUT" ]; do
    sleep 5
    F=$(grep -a -o 'JAMSIM frame=[0-9]*' "$SAVED" 2>/dev/null | tail -1 | cut -d= -f2); F=${F:-0}
    OVER=$(grep -a -c 'phase=2' "$SAVED" 2>/dev/null)
    if [ "$F" -ge "$MAXF" ] || [ "${OVER:-0}" -ge 2 ]; then break; fi
done
powershell -NoProfile -Command "Get-CimInstance Win32_Process -Filter \"Name='UnrealEditor-Cmd.exe'\" | Where-Object { \$_.CommandLine -like '*worktrees/f3c*' -and \$_.CommandLine -like '*RemoteConsolePort=4772*' } | ForEach-Object { Stop-Process -Id \$_.ProcessId -Force }"
sleep 2
cp "$SAVED" "$W/Game/logs/$NAME.log"
echo "run_live $NAME: last JAMSIM frame $F after $((SECONDS - T0)) s; $(grep -a -c JAMCAM "$W/Game/logs/$NAME.log") camera lines; fouls $(grep -a -c 'JAMEVENT foul ' "$W/Game/logs/$NAME.log"); goals $(grep -a -c 'JAMEVENT goal ' "$W/Game/logs/$NAME.log")"
