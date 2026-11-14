#!/bin/sh
# Lane A153 (lane A134's pt.sh, logs under Game/logs/a153). usage: pt.sh <tag> <cartridge> <level> <player> <seeds> [trace] [extra args...]
# One headless JamTournament process, log under Game/logs/a153/<tag>.log
W="$(cd "$(dirname "$0")/../../../.." && pwd)"
UE="/c/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
TAG=$1; CART=$2; LVL=$3; PL=$4; SEEDS=$5; TR=${6:-0}; shift; shift; shift; shift; shift; shift
LOG="$(cygpath -w $W/Game/logs/a153/$TAG.log)"
TRARG=""
[ "$TR" != "0" ] && TRARG="-trace=$TR"
"$UE" "$(cygpath -w $W/Game/ThirdJam.uproject)" -run=JamTournament -sim=jam -cartridge=$CART -difficulty=$LVL -player=$PL -seeds=$SEEDS -maxframes=36000 $TRARG -nullrhi -Multiprocess -unattended -nopause -nosplash -log -ABSLOG="$LOG" "$@" > /dev/null 2>&1
echo "exit $? $TAG $(grep -c 'thirdjam.playtest/1' $W/Game/logs/a153/$TAG.log)"
