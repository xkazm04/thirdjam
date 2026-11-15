#!/bin/sh
# Lane A220: the gate's automation invocation (tools/gate/premerge.ps1) in a cleared environment (env -i: only what Windows and the engine need,
# no API key, no JAM_* switch), headless (-nullrhi -Multiprocess). $1 = log name under Game/logs/a220/, $2 = filter (default Jam).
# The UnrealEditor-Cmd pid goes to Game/logs/a220/$1.pid. Prints the result counts.
cd "$(dirname "$0")/../../../.."
mkdir -p Game/logs/a220
UE='C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe'
LOG="$(pwd -W)/Game/logs/a220/$1.log"
rm -f "Game/logs/a220/$1.log"
env -i SYSTEMROOT="$SYSTEMROOT" WINDIR="$WINDIR" PATH="/usr/bin:/c/WINDOWS/system32:/c/WINDOWS" TEMP="$TEMP" TMP="$TMP" USERPROFILE="$USERPROFILE" \
  LOCALAPPDATA="$LOCALAPPDATA" APPDATA="$APPDATA" PROGRAMDATA="$PROGRAMDATA" COMPUTERNAME="$COMPUTERNAME" USERNAME="$USERNAME" HOMEDRIVE="$HOMEDRIVE" HOMEPATH="$HOMEPATH" \
  NUMBER_OF_PROCESSORS="$NUMBER_OF_PROCESSORS" PROCESSOR_ARCHITECTURE="$PROCESSOR_ARCHITECTURE" \
  "$UE" "$(pwd -W)/Game/ThirdJam.uproject" -ExecCmds="Automation RunTests ${2:-Jam}; Quit" -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$LOG" > /dev/null 2>&1 &
echo $! > "Game/logs/a220/$1.pid"
wait $!
echo "exit $?"
grep -E "Test Completed\. Result=\{(Fail|Success)" "Game/logs/a220/$1.log" | sed 's/.*Result={\([A-Za-z]*\)} Name={\([^}]*\)}.*/\1/' | sort | uniq -c
grep -E "Test Completed\. Result=\{Fail" "Game/logs/a220/$1.log" | head -20
