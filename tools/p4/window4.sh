#!/usr/bin/env bash
# tools/p4/window4.sh (lane A28, pass 2): captures only: lab + walk (ragdoll) + fall1 (ragdoll) per figure, full error text, sheets. Editor on 8015, lab map open.
set -u; export MSYS_NO_PATHCONV=1; cd "$(dirname "$0")/../.."
U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; PY="editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python"; J=tools/p4/jobs
echo "A28 window4 start $(date +%H:%M:%S)"
for V in "$@"; do
  L=$(echo $V | tr A-Z a-z); ALIAS=$(python -I tools/p4/alias_of.py $L); OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/cap"; mkdir -p "$OUT"
  cat > $J/_${L}_lab4.mcp.json <<EOS
{"script":{"content":"A28_LAB={'vendor':'$V','mesh':'/Game/P4/$V/SK_P4_$V','stick':'/Game/P4/Props/SM_P4_Stick','stick_rot':[0,0,0],'alias':$ALIAS}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_lab.py').read())\n","language":"Python"}}
EOS
  $M call $PY @$J/_${L}_lab4.mcp.json $U --timeout 300 | tail -1 | cut -c1-700
  FALL=""; [ -f Game/Content/P4/$V/Falls/AS_P4_${V}_fall1.uasset ] && FALL=AS_P4_${V}_fall1
  CLIPS="/Game/P4/$V/Anims/MF_Unarmed_Walk_Fwd_P4_$V $L-walk2"; [ -n "${ONLY_FALL:-}" ] && CLIPS=""
  for clip in $CLIPS "/Game/P4/$V/Falls/$FALL $L-fall2"; do set -- $clip
    [ "$2" = "$L-fall2" ] && [ -z "$FALL" ] && continue
    python -I - "$V" "$1" "$OUT" "$2" "$ALIAS" "$J/_${L}_cap4.mcp.json" <<'EOP'
import json, sys
V, CLIP, OUT, TAG, ALIAS, dst = sys.argv[1:7]
s = open("tools/p4/ue_capture.py", encoding="utf8").read()
s = s.replace("__VENDOR__", V).replace("__CLIP__", CLIP).replace("__OUT__", OUT).replace("__TAG__", TAG).replace("__RAGDOLL__", "1").replace("__ALIAS__", ALIAS or "{}")
json.dump({"script": {"content": s, "language": "Python"}}, open(dst, "w")); print("cap args", TAG)
EOP
    $M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1 | cut -c1-60
    sleep 4
    $M call $PY @$J/_${L}_cap4.mcp.json $U --timeout 300 | tail -1 | cut -c1-500
    $M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1 | cut -c1-60
    TS=$(date +%Y%m%d-%H%M%S)
    python -I tools/p4/sheet.py "$OUT" "$2" chase "devlog/shots/$TS-A-a28-$2-chase.png" "devlog/shots/$TS-A-a28-$2-chase.mp4" "A28 $V $2 pass 2 chase 4.2 m: stick at the computed grip, upper arms +20 deg (Z) in the retarget pose, mass-scaled shove; f00-f15 = 2 s, rag/back = ragdoll 20 frames and the sequence again" 2>&1 | grep "A28"
    python -I tools/p4/sheet.py "$OUT" "$2" far "devlog/shots/$TS-A-a28-$2-far.png" - "A28 $V $2 pass 2 far 24 m" 2>&1 | grep "A28 sheet"
  done
done
$M call $PY @$J/_saveall.mcp.json $U --timeout 120 | tail -1 | cut -c1-80
echo "A28 window4 end $(date +%H:%M:%S)"
