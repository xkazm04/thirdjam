#!/usr/bin/env bash
# tools/blender/arena/live_bus.sh (Third Jam, track F3, lane A136): the rink's bus binding in a live match.
# Starts ONE headless game process (UnrealEditor-Cmd -game -nullrhi -Multiprocess: no window, no mouse lock, nothing drawn) on the Rink map with a cut and the
# scripted human (every skater plays itself), waits until the log shows enough goals and sim frames, stops the process IT started (never another one), then
# checks the log: one `JAMRINK react goal` line per `JAMEVENT goal`, one `JAMRINK react board` line per hard skater `JAMEVENT board` (Strength >= 0.45).
# Run from the worktree root (Git Bash):
#   tools/blender/arena/live_bus.sh [cut file or id] [port] [min goals] [max frames] [timeout s]
# Writes Game/logs/a136-live-bus.log and prints "A136 LIVE BUS PASS|FAIL" with the counts; exit 0 pass, 1 fail, 2 could not run.
set -u
W=$(pwd -W)
CUT=${1:-$W/tools/perf/cuts/perf.p3_clay_5v5.cartridge.json}
PORT=${2:-4736}
MIN_GOALS=${3:-2}
MAX_FRAMES=${4:-14400}
TIMEOUT=${5:-420}
UE="${TJ_UE_CMD:-C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe}"
LOG="$W/Game/logs/a136-live-bus.log"
[ -x "$UE" ] || { echo "A136 LIVE BUS could not run: no engine at $UE"; exit 2; }
mkdir -p "$W/Game/logs"
rm -f "$LOG"
# MSYS_NO_PATHCONV: Git Bash would turn /Game/Maps/Rink into a Windows path under its own install
MSYS_NO_PATHCONV=1 "$UE" "$W/Game/ThirdJam.uproject" /Game/Maps/Rink -game -nullrhi -Multiprocess -unattended -nosplash -NoSound -log -abslog="$LOG" \
    -cut="$CUT" -dpcvars=t.MaxFPS=60,jam.ScriptHuman=1 -RemoteConsoleIP=127.0.0.1 -RemoteConsolePort="$PORT" > /dev/null 2>&1 &
PID=$!
echo "INFO live bus: cut $CUT, port $PORT, my pid $PID, log $LOG"
T0=$SECONDS
FRAME=0
GOALS=0
while [ $((SECONDS - T0)) -lt "$TIMEOUT" ]; do
    sleep 5
    kill -0 "$PID" 2>/dev/null || break
    [ -f "$LOG" ] || continue
    FRAME=$(grep -a -o 'JAMEVENT [a-z_]* .* frame=[0-9]*' "$LOG" | tail -1 | grep -o 'frame=[0-9]*' | cut -d= -f2)
    FRAME=${FRAME:-0}
    GOALS=$(grep -a -c 'JAMEVENT goal ' "$LOG")
    if [ "$FRAME" -ge "$MAX_FRAMES" ]; then break; fi
    if [ "$GOALS" -ge "$MIN_GOALS" ] && [ "$FRAME" -ge 3600 ]; then sleep 4; break; fi
done
if kill -0 "$PID" 2>/dev/null; then
    echo "INFO live bus: stopping my pid $PID at sim frame $FRAME after $((SECONDS - T0)) s"
    kill "$PID" 2>/dev/null
    sleep 2
fi
python "$W/tools/blender/arena/live_bus_check.py" "$LOG"
