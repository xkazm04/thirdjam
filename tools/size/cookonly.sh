#!/usr/bin/env bash
# Lane A161: a cook-only of the shipped maps (the project's MapsToCook and DirectoriesToAlwaysCook), headless, with loose cooked files
# (the zen store is switched off on the command line only, so the size by folder can be read from disk), then the size by folder.
# (Restored by lane A175: the A161 landing did not carry it.)
#   bash tools/size/cookonly.sh <label>      -> Game/logs/cook-<label>.log, Game/logs/cooked-<label>.txt (size path), summary on stdout
set -u
label="${1:?label}"
wt="$(cd "$(dirname "$0")/../.." && pwd)"
ue="${UE_ROOT:-/c/Program Files/Epic Games/UE_5.9}/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"
up="$(cd "$wt/Game" && pwd -W)/ThirdJam.uproject"
log="$(cd "$wt/Game" && pwd -W)/logs/cook-$label.log"
mkdir -p "$wt/Game/logs"
t0=$(date +%s)
"$ue" "$up" -run=cook -targetplatform=Windows -unattended -nullrhi -Multiprocess -nopause -nosplash -log -ABSLOG="$log" \
  "-ini:Game:[/Script/UnrealEd.ProjectPackagingSettings]:bUseZenStore=False" > "$wt/Game/logs/cook-$label.out" 2>&1
rc=$?
t1=$(date +%s)
errs=$(grep -cE "\]Log[A-Za-z]+: Error" "$wt/Game/logs/cook-$label.log")
warns=$(grep -E "\]Log[A-Za-z]+: Warning" "$wt/Game/logs/cook-$label.log" | grep -vc "LogConsoleManager")
cooked="$wt/Game/Saved/Cooked/Windows"
(cd "$cooked" && find . -type f \( -name "*.uasset" -o -name "*.uexp" -o -name "*.ubulk" -o -name "*.umap" -o -name "*.uptnl" -o -name "*.ushaderbytecode" \) -printf "%s %p\n") > "$wt/Game/logs/cooked-$label.txt"
echo "cook $label: exit $rc, $((t1 - t0)) s wall, $errs errors, $warns warnings (not ConsoleManager), $(grep -E 'Packages Cooked:' "$wt/Game/logs/cook-$label.log" | sed 's/.*Display: //')"
awk '{p=$2; sub(/^\.\//,"",p); n=split(p,a,"/"); k=a[1]; for(i=2;i<n && i<=6;i++) k=k"/"a[i]; s[k]+=$1; t+=$1} END {for (k in s) if (s[k] > 524288) printf "%8.1f MB %s\n", s[k]/1048576, k; printf "%8.1f MB TOTAL\n", t/1048576}' "$wt/Game/logs/cooked-$label.txt" | sort -rn
exit $rc
