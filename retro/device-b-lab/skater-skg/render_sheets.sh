#!/usr/bin/env bash
# Renders the SKG sheets for one style from preview/goalie_<style>.blend.
#   bash render_sheets.sh <style>
set -e
S="$(cd "$(dirname "$0")" && pwd -W 2>/dev/null || pwd)"
B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
st="$1"
P="$S/preview/goalie_$st.blend"
R="$S/r/$st"
rm -rf "$R"; mkdir -p "$R" "$S/sheets"
run() { "$B" -b --factory-startup --python "$S/render_g.py" -- "$@" 2>&1 | grep -E "COVER|Error|rror:" || true; }
CLIPS="Goalie_Stance:0,Goalie_ShuffleL:6,Goalie_ShuffleR:6,Goalie_Butterfly_Drop:6,Goalie_Butterfly_Hold:0,Goalie_Butterfly_Recover:12,Goalie_GloveSave:10,Goalie_BlockerSave:10,Goalie_PadSaveL:11,Goalie_PadSaveR:11,Goalie_PokeCheck:9,Goalie_Freeze:24,Goalie_Beaten:60,Goalie_Celebrate_Save:8"
run "$P" "$R/turn" turn Goalie_Stance 0 8 200
run "$P" "$R/front" keys front "$CLIPS" 150
run "$P" "$R/game" keys game "$CLIPS" 150
run "$P" "$R/goal" goal "Goalie_Stance:0,Goalie_Butterfly_Hold:0,Goalie_GloveSave:10,Goalie_PadSaveL:11" 320
run "$P" "$R/mask" mask "Goalie_Stance:0,Goalie_Butterfly_Hold:0,Goalie_GloveSave:10,Goalie_PadSaveL:11" | tee "$R/coverage.txt"
SKATER="C:/Users/kazda/jam/skater-lab/s4/s4_skater_anim.blend"
SK_USED="$(cat "$S/export/$st/skeleton_used.txt" 2>/dev/null || true)"
case "$SK_USED" in *.fbx) SKATER="$SK_USED";; esac
run "$P" "$R/dist" dist "Goalie_Stance:0" "$SKATER"
mf() { local c=$1 n=$2; local o=""; for i in 0 1 2 3 4 5 6 7; do o="$o,$c:$(( (n * i + 3) / 7 ))"; done; echo "${o#,}"; }
MOT="$(mf Goalie_Butterfly_Drop 14),$(mf Goalie_GloveSave 42),$(mf Goalie_PadSaveL 46),$(mf Goalie_PokeCheck 40),$(mf Goalie_ShuffleL 24)"
run "$P" "$R/motion" keys front "$MOT" 120
cd "$R"
ff() { ffmpeg -y -loglevel error "$@"; }
PAL=",split[a][b];[a]palettegen=max_colors=128[p];[b][p]paletteuse=dither=none"
ff -i turn/f%02d.png -vf "tile=8x1$PAL" "$S/sheets/${st}_turntable.png"
ff -i front/k%02d.png -vf tile=7x2 front.png
ff -i game/k%02d.png -vf tile=7x2 game.png
ff -i front.png -i game.png -filter_complex "vstack=inputs=2$PAL" "$S/sheets/${st}_keys.png"
ff -i goal/g%02d.png -vf tile=4x1 goal.png
ff -i mask/m%02d.png -vf "tile=4x1:padding=4:color=0x404040,scale=1280:-2" mask.png
ff -i goal.png -i mask.png -filter_complex "vstack=inputs=2$PAL" "$S/sheets/${st}_goal.png"
ff -i dist/d%02d.png -vf "tile=4x1,scale=iw*3:ih*3:flags=neighbor" "$S/sheets/${st}_40px.png"
ff -i motion/k%02d.png -vf "tile=8x5$PAL" "$S/sheets/${st}_motion.png"
ls -la "$S/sheets" | grep "$st"
