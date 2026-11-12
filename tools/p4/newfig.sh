#!/usr/bin/env bash
# tools/p4/newfig.sh <Vendor> <model_url> [height_m]: a generated mesh (public URL) rigged on Meshy ue5_mannequin, downloaded, normalized, textured,
# and a tools/p4/jobs/<vendor>.ue.json written for run_vendor.sh. No editor. Ledgers the rig in tools/p4/ledger/meshy.jsonl.
set -u; cd "$(dirname "$0")/../.."
V="$1"; URL="$2"; H="${3:-1.8}"; L=$(echo "$V" | tr 'A-Z' 'a-z'); D=tools/p4/out/$L; B="/c/Program Files/Blender Foundation/Blender 4.2/blender.exe"
mkdir -p $D/RIG $D/NORM
node tools/meshy/meshy.mjs rig --model-url "$URL" --template ${TEMPLATE:-ue5_mannequin} --height $H --wait --interval-s 5 --ledger tools/p4/ledger/meshy.jsonl 2>&1 | grep "^{" | cut -c1-160
RIG=$(tail -1 tools/p4/ledger/meshy.jsonl | python -I -c "import json,sys; d=json.loads(sys.stdin.read()); print(d['task_id'] if d.get('ok') else '')")
[ -n "$RIG" ] || { echo "A28 newfig $V rig failed"; exit 1; }
node tools/meshy/meshy.mjs download $RIG $D/RIG --only fbx,glb 2>&1 | grep "^{" | cut -c1-160
for f in rigged_character_fbx_url basic_animations.walking_fbx_url basic_animations.running_fbx_url; do
  "$B" -b --factory-startup --python tools/p4/normalize_fbx.py -- "C:/Users/kazda/kiro/thirdjam/$D/RIG/$f.fbx" "C:/Users/kazda/kiro/thirdjam/$D/NORM/$f.fbx" root zup -90 one 0 2>/dev/null | grep "A28 norm wrote" | cut -c1-140
done
"$B" -b --factory-startup --python tools/p4/glb_texture.py -- "C:/Users/kazda/kiro/thirdjam/$D/RIG/rigged_character_glb_url.glb" "C:/Users/kazda/kiro/thirdjam/$D/RIG/tex" 2>/dev/null | grep "A28 glbtex" | cut -c1-160
TEX=$(ls $D/RIG/tex/*.png 2>/dev/null | grep -iv "normal\|rough\|metal\|orm" | head -1)
python -I - "$V" "$L" "$TEX" "$RIG" <<'EOF'
import json, sys
V, v, tex, rig = sys.argv[1:5]
base = "C:/Users/kazda/kiro/thirdjam/tools/p4/out/%s" % v
d = json.load(open("tools/p4/jobs/meshy.ue.json"))
d["vendor"] = V; d["dest"] = "/Game/P4/" + V; d["sk_name"] = "SK_P4_" + V
d["mesh_fbx"] = base + "/NORM/rigged_character_fbx_url.fbx"
d["texture_png"] = "C:/Users/kazda/kiro/thirdjam/" + tex.replace("\\", "/") if tex else ""
d["clips"] = [{"fbx": base + "/NORM/basic_animations.walking_fbx_url.fbx", "name": "AS_P4_%s_vendor_walk" % V}, {"fbx": base + "/NORM/basic_animations.running_fbx_url.fbx", "name": "AS_P4_%s_vendor_run" % V}]
d["report"] = base + "/ue_setup.report.json"; d["rig_task"] = rig
json.dump(d, open("tools/p4/jobs/%s.ue.json" % v, "w"), indent=2); print("A28 newfig config", v, "rig", rig, "tex", bool(tex))
EOF
