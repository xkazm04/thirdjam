#!/bin/sh
# Lane A198 (track S3, evolution 5, item 4): the bindings file as a player's disk can leave it. For each variant the worktree's own settings file
# (Game/Saved/Config/WindowsEditor/GameUserSettings.ini, the editor-binary game's profile) is written, then one headless process loads it through
# FJamSettings::Load (Bar.Hard.BindingsRead: the game's own read, GConfig's parser) and, for the read-only variant, saves through it (Bar.Hard.BindingsWrite).
# PASS per variant: the process exits 0 (no crash), the table read back is valid. The verbs it leaves without a key are printed (FAULT 1 in status/A198.md).
# Every variant is the file as the game writes it (the engine's first line, [/Script/Engine.GameUserSettings] with its Version, [Jam.Settings]:
# tools/fuzz/out/ini-base.txt, taken from a file this tree's game wrote) with the [Jam.Controls] section damaged, plus 'no-version' (the section alone).
# Found on the way (passes 1 and 2): the editor binary's read loses [Jam.Controls] when the file has no GameUserSettings Version and is writable
# (controls=0), keeps it when the file is read-only (controls=15), and reads nothing of the user file under -MultiprocessSaveConfig.
# The file is put back as it was at the end (absent if it was absent). Usage: sh tools/fuzz/bindings-file.sh [out dir]
cd "$(dirname "$0")/../.."
OUT=${1:-tools/fuzz/out/a198-bindings3}
BASE="$(cat tools/fuzz/out/ini-base.txt)"; mkdir -p "$OUT"
INI=Game/Saved/Config/WindowsEditor/GameUserSettings.ini
UE="/c/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe"; UP="$(cygpath -w "$PWD/Game/ThirdJam.uproject")"
mkdir -p "$(dirname $INI)"
[ -f "$INI" ] && cp "$INI" "$OUT/ini.before" || rm -f "$OUT/ini.before"
HDR=';METADATA=(Diff=true, UseCommands=true)'
FULL='[Jam.Controls]
Shoot=LeftMouseButton|Gamepad_RightTriggerAxis
CrossCheck=LeftMouseButton,MiddleMouseButton,ThumbMouseButton|Gamepad_RightTriggerAxis,Gamepad_LeftShoulder
Pass=Z,Q|Gamepad_RightShoulder,Gamepad_FaceButton_Left
Check=RightMouseButton|Gamepad_RightShoulder
Poke=SpaceBar|Gamepad_FaceButton_Bottom
Deke=MouseScrollUp,MouseScrollDown|Gamepad_FaceButton_Top
Spin=G|Gamepad_FaceButton_Right
Ability=F|Gamepad_RightThumbstick
Sprint=LeftShift|Gamepad_LeftTriggerAxis
Brake=LeftControl|Gamepad_LeftThumbstick
SkateForward=W,Up|
SkateBack=S,Down|
SkateLeft=A,Left|
SkateRight=D,Right|
Restart=R|Gamepad_Special_Left'
put() { { printf '%s\n\n' "$BASE"; cat; } | sed 's/$/\r/' > "$INI"; }   # the game's own file, then the damaged section, CRLF
: > "$OUT/table.md"
echo "| variant | exit | verdict | read back |" >> "$OUT/table.md"; echo "|---|---|---|---|" >> "$OUT/table.md"
run() {   # $1 name, $2 test, $3 expect, $4 extra flag
  JAM_HARD_EXPECT=$3 "$UE" "$UP" -ExecCmds="Automation RunTests $2; Quit" -nullrhi -Multiprocess $4 -unattended -nopause -nosplash -log -ABSLOG="$(cygpath -w "$PWD/$OUT/$1.log")" > /dev/null 2>&1
  code=$?
  line=$(grep -h "JAMHARD bindings" "$OUT/$1.log" | tail -1 | sed 's/.*JAMHARD bindings //')
  ok=$(grep -c "Result={Success}" "$OUT/$1.log"); crash=$(grep -c "Fatal error\|Unhandled Exception\|Assertion failed" "$OUT/$1.log")
  v=PASS; [ "$code" != 0 ] && v=FAIL; [ "$ok" = 0 ] && v=FAIL; [ "$crash" != 0 ] && v=CRASH
  echo "| $1 | $code | $v | $line |" >> "$OUT/table.md"; echo "$v $1 (exit $code) $line"
}
printf '%s\n' "$FULL" | put;                                                     run control-full Bar.Hard.BindingsRead rebound
printf '%s\n' "$FULL" | sed 's/$/\r/' > "$INI";                                  run no-version Bar.Hard.BindingsRead any
rm -f "$INI";                                                                    run missing Bar.Hard.BindingsRead any
: > "$INI";                                                                      run empty Bar.Hard.BindingsRead any
printf '%s\n' "$FULL" | put; n=$(wc -c < "$INI"); head -c $((n - 400)) "$INI" > "$INI.cut"; mv "$INI.cut" "$INI";   run cut-in-half Bar.Hard.BindingsRead any
printf '[Jam.Controls]\nShoot=NoSuchKey|Gamepad_Nope\nJump=SpaceBar|\nPass=Z|\n' | put;   run unknown-keys Bar.Hard.BindingsRead any
printf '[Jam.Controls]\nPoke=W|\n' | put;                                        run two-actions-one-key Bar.Hard.BindingsRead any
printf '[Jam.Controls]\nShoot=Mouse_Left|RightTrigger\n' | put;                  run key-gone Bar.Hard.BindingsRead any
printf '[Jam.Controls]\nShoot=|\nCrossCheck=|\nPass=|\nCheck=|\nPoke=|\nDeke=|\nSpin=|\nAbility=|\nSprint=|\nBrake=|\nSkateForward=|\nSkateBack=|\nSkateLeft=|\nSkateRight=|\nRestart=|\n' | put;   run all-bars Bar.Hard.BindingsRead any
{ printf '%s\n\n[Jam.Controls]\n' "$BASE" | sed 's/$/\r/'; head -c 4096 /dev/urandom; } > "$INI";   run binary-garbage Bar.Hard.BindingsRead any
printf '%s\n[Jam.Controls]\nPass=X|\n[Jam.Controls\nShoot\n=\n' "$FULL" | put;   run duplicate-section Bar.Hard.BindingsRead any
printf '%s\n' "$FULL" | put; attrib +R "$(cygpath -w $INI)";                     run readonly-read Bar.Hard.BindingsRead rebound
run readonly-write Bar.Hard.BindingsWrite any -MultiprocessSaveConfig
attrib -R "$(cygpath -w $INI)" > /dev/null 2>&1
if [ -f "$OUT/ini.before" ]; then cp "$OUT/ini.before" "$INI"; else rm -f "$INI"; fi
echo done
