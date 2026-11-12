#!/usr/bin/env bash
# tools/p4/window3.sh (lane A28, pass 2): one editor window on MCP 8015 for the three blockers and the image-to-3D figures.
#   tools/p4/window3.sh <Vendor ...>        e.g. Meshy Rodin Tripo Rodin2 Tripo2
# Per vendor: SM_P4_Stick v2 re-imported once; (new vendors: run_vendor.sh first); grip rotation at stick_socket (ue_grip.py); retarget-pose
# search over upperarm offsets none / Y20 / Z20, each measured by a walk capture (min hand-to-pelvis ground distance over 2 s), the best kept;
# the split falls imported (one FBX per action, X/Y travel stripped); a fall captured with the mass-scaled ragdoll impulse; sheets to devlog/shots.
set -u; export MSYS_NO_PATHCONV=1; cd "$(dirname "$0")/../.."
U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; PY="editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python"; J=tools/p4/jobs
echo "A28 window3 start $(date +%H:%M:%S)"
$M call $PY @$J/_stick.mcp.json $U --timeout 120 | tail -1 | cut -c1-120
capture() { # capture <Vendor> <clip> <tag> <ragdoll 0|1>  -> frames in out/<l>/cap, prints the capture json path
  local V=$1 CLIP=$2 TAG=$3 RAG=$4 L; L=$(echo $1 | tr A-Z a-z); local OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/cap"; mkdir -p "$OUT"
  python -I - "$V" "$CLIP" "$OUT" "$TAG" "$RAG" "$J/_${L}_cap3.mcp.json" "$ALIAS" <<'EOP'
import json, sys
V, CLIP, OUT, TAG, RAG, dst = sys.argv[1:7]
ALIAS = sys.argv[7] if len(sys.argv) > 7 and sys.argv[7] else "{}"
s = open("tools/p4/ue_capture.py", encoding="utf8").read()
s = s.replace("__VENDOR__", V).replace("__CLIP__", CLIP).replace("__OUT__", OUT).replace("__TAG__", TAG).replace("__RAGDOLL__", RAG).replace("__ALIAS__", ALIAS)
json.dump({"script": {"content": s, "language": "Python"}}, open(dst, "w"))
EOP
  $M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1 | cut -c1-60
  sleep 4
  $M call $PY @$J/_${L}_cap3.mcp.json $U --timeout 300 | tail -1 | cut -c1-200
  $M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1 | cut -c1-60
  echo "$OUT/$TAG.capture.json"
}
for V in "$@"; do
  L=$(echo $V | tr A-Z a-z)
  ALIAS=$(python -I tools/p4/alias_of.py $L)
  HAND=$(python -I tools/p4/alias_of.py $L hand_r)
  if [ ! -d Game/Content/P4/$V ] || [ -n "${FRESH:-}" ]; then bash tools/p4/run_vendor.sh $V 2>&1 | grep "A28 run\|A28 sheet\|verdict\":\"fail" | cut -c1-200; fi
  bash tools/p4/sockets.sh $V $HAND root 2>&1 | grep -c SkeletalMeshSocket | sed "s/^/A28 $V sockets /"
  cat > $J/_${L}_grip.mcp.json <<EOS
{"script":{"content":"A28_GRIP={'vendor':'$V','mesh':'/Game/P4/$V/SK_P4_$V','alias':$ALIAS}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_grip.py').read())\n","language":"Python"}}
EOS
  $M call $PY @$J/_${L}_grip.mcp.json $U --timeout 120 | tail -1 | cut -c1-200
  # retarget-pose search (pass-1 figures and new ones alike)
  best=""; bestd=-1
  for variant in "none 0" "Y 20" "Z 20"; do set -- $variant
    cat > $J/_${L}_rpose.mcp.json <<EOS
{"script":{"content":"A28_RP={'vendor':'$V','dest':'/Game/P4/$V','mesh':'/Game/P4/$V/SK_P4_$V','axis':'$1','deg':$2,'clips':['/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd'],'alias':$ALIAS}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_retarget_pose.py').read())\n","language":"Python"}}
EOS
    $M call $PY @$J/_${L}_rpose.mcp.json $U --timeout 300 | tail -1 | cut -c1-160
    cat > $J/_${L}_lab3.mcp.json <<EOS
{"script":{"content":"A28_LAB={'vendor':'$V','mesh':'/Game/P4/$V/SK_P4_$V','stick':'/Game/P4/Props/SM_P4_Stick','stick_rot':[0,0,0],'alias':$ALIAS}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_lab.py').read())\n","language":"Python"}}
EOS
    $M call $PY @$J/_${L}_lab3.mcp.json $U --timeout 300 | tail -1 | cut -c1-100
    jpath=$(capture $V /Game/P4/$V/Anims/MF_Unarmed_Walk_Fwd_P4_$V $L-walk-rp-$1$2 0 | tail -1)
    d=$(python -I -c "
import json,sys
try:
    f=json.load(open(sys.argv[1]))['frames']; print(round(min(min(x['hand_l_from_pelvis_xy'],x['hand_r_from_pelvis_xy']) for x in f),1))
except Exception as e: print(-1)" "$jpath")
    echo "A28 $V rpose variant $1$2 min_hand_from_pelvis_xy_cm $d"
    if python -I -c "import sys; sys.exit(0 if float(sys.argv[1]) > float(sys.argv[2]) else 1)" "$d" "$bestd"; then best="$1 $2"; bestd=$d; fi
  done
  echo "A28 $V rpose best '$best' $bestd cm"
  set -- $best
  cat > $J/_${L}_rpose.mcp.json <<EOS
{"script":{"content":"A28_RP={'vendor':'$V','dest':'/Game/P4/$V','mesh':'/Game/P4/$V/SK_P4_$V','axis':'$1','deg':$2,'clips':['/Game/Characters/Mannequins/Anims/Unarmed/Walk/MF_Unarmed_Walk_Fwd','/Game/Characters/Mannequins/Anims/Unarmed/Jog/MF_Unarmed_Jog_Fwd','/Game/Characters/Mannequins/Anims/Unarmed/MM_Idle'],'alias':$ALIAS}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_retarget_pose.py').read())\n","language":"Python"}}
EOS
  $M call $PY @$J/_${L}_rpose.mcp.json $U --timeout 300 | tail -1 | cut -c1-100
  # the split falls (five files) on this skeleton
  SRC=$(echo $V | tr A-Z a-z | sed 's/2$//')   # Rodin2 / Tripo2 have no falls of their own yet: use none
  if [ -d tools/p4/out/$L/NORM/falls_split ]; then
    python -I - "$V" "$L" "$J/_${L}_falls3.mcp.json" <<'EOP'
import json, sys, glob, os
V, L, dst = sys.argv[1:4]
clips = [{"fbx": os.path.abspath(p).replace("\\", "/"), "name": "AS_P4_%s_%s" % (V, os.path.basename(p)[:-4].lower())} for p in sorted(glob.glob("tools/p4/out/%s/NORM/falls_split/*.fbx" % L))]
cfg = {"vendor": V, "dest": "/Game/P4/%s/Falls" % V, "skeleton": "/Game/P4/%s/SKEL_P4_%s" % (V, V), "clips": clips}
json.dump({"script": {"content": "A28_CLIPS=%s\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_clips.py').read())\n" % json.dumps(cfg), "language": "Python"}}, open(dst, "w"))
print("falls config", len(clips))
EOP
    $M call $PY @$J/_${L}_falls3.mcp.json $U --timeout 300 | tail -1 | cut -c1-200
    FALL=$(grep -a "A28 $V clips" Game/logs/a28-editor.log | tail -1 | sed 's/^.*clips //' | python -I -c "import json,sys; l=json.load(sys.stdin); c=[x[0] for x in l if 'fall1' in x[0].lower()]; print(c[0] if c else (sorted(l)[0][0] if l else ''))" 2>/dev/null)
    [ -n "$FALL" ] && capture $V /Game/P4/$V/Falls/$FALL $L-fall2 1 | tail -1
  fi
  capture $V /Game/P4/$V/Anims/MF_Unarmed_Walk_Fwd_P4_$V $L-walk2 1 | tail -1
  TS=$(date +%Y%m%d-%H%M%S); OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/cap"
  for tag in $L-walk2 $L-fall2; do
    python -I tools/p4/sheet.py "$OUT" "$tag" chase "devlog/shots/$TS-A-a28-$tag-chase.png" "devlog/shots/$TS-A-a28-$tag-chase.mp4" "A28 $V $tag pass 2 chase 4.2 m: stick at the grip, retarget pose $best, mass-scaled shove; f00-f15 = 2 s, rag/back = ragdoll 20 frames and the sequence again" 2>&1 | grep "A28"
    python -I tools/p4/sheet.py "$OUT" "$tag" far "devlog/shots/$TS-A-a28-$tag-far.png" - "A28 $V $tag pass 2 far 24 m" 2>&1 | grep "A28 sheet"
  done
done
grep -a "A28 .* grip socket_relative_rot\|A28 .* grip stick_x_world\|A28 .* grip blade_end\|A28 .* clips \|ragdoll_mass_kg\|A28 .* rpose offset" Game/logs/a28-editor.log | tail -40 | sed 's/^.*LogPython: //' | cut -c1-200
$M call $PY @$J/_saveall.mcp.json $U --timeout 120 | tail -1 | cut -c1-80
echo "A28 window3 end $(date +%H:%M:%S)"
