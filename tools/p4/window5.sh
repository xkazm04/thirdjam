#!/usr/bin/env bash
# tools/p4/window5.sh (lane A28, pass 3): one editor window on 8015 (the lab map open at start):
#   1. A/B lab: Rodin2, Tripo3 and B's TB2_P4 at the origin (hidden), cameras reused; 2. Simulate: each revealed alone, its clip at game frame 8, chase (4.2 m)
#   and far (24 m) stills; 3. hand-made capsules on PA_P4_Rodin2 (ue_pa_plan.py -> PhysicsAssetToolset calls), 4. Simulate: Rodin2 ragdoll measured.
#   tools/p4/window5.sh <TB2 mesh asset path> <TB2 clip asset path>
set -u; export MSYS_NO_PATHCONV=1; cd "$(dirname "$0")/../.."
TB2_MESH="$1"; TB2_CLIP="$2"
U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; PY="editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python"; J=tools/p4/jobs
OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/ab/cap"; mkdir -p "$OUT"; rm -f "$OUT"/*.png
echo "A28 window5 start $(date +%H:%M:%S)"
T3=$(python -I tools/p4/alias_of.py tripo3)
python -I - "$TB2_MESH" "$TB2_CLIP" "$T3" "$OUT" <<'EOP'
import json, sys
tb2, clip, t3, out = sys.argv[1:5]
t3 = json.loads(t3.replace("'", '"'))
figs = [{"label": "Rodin2", "mesh": "/Game/P4/Rodin2/SK_P4_Rodin2", "clip": "/Game/P4/Rodin2/Anims/MF_Unarmed_Walk_Fwd_P4_Rodin2", "alias": {}},
        {"label": "Tripo3", "mesh": "/Game/P4/Tripo3/SK_P4_Tripo3", "clip": "/Game/P4/Tripo3/AS_P4_Tripo3_vendor_walk", "alias": t3},
        {"label": "TB2P4", "mesh": tb2, "clip": clip, "alias": {}}]
lab = {"figures": [{k: f[k] for k in ("label", "mesh", "alias")} for f in figs]}
json.dump({"script": {"content": "A28_AB=%s\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_ablab.py').read())\n" % json.dumps(lab), "language": "Python"}}, open("tools/p4/jobs/_ab_lab.mcp.json", "w"))
s = open("tools/p4/ue_abcap.py", encoding="utf8").read()
PY = lambda o: json.dumps(o).replace("false", "False").replace("true", "True").replace("null", "None")
cap = s.replace("__FIGURES__", PY([{**f, "ragdoll": False} for f in figs])).replace("__OUT__", out).replace("__FRAME__", "8").replace("__TAG__", "ab")
json.dump({"script": {"content": cap, "language": "Python"}}, open("tools/p4/jobs/_ab_cap.mcp.json", "w"))
rag = s.replace("__FIGURES__", PY([{**figs[0], "ragdoll": True}])).replace("__OUT__", out).replace("__FRAME__", "8").replace("__TAG__", "rag")
json.dump({"script": {"content": rag, "language": "Python"}}, open("tools/p4/jobs/_ab_rag.mcp.json", "w"))
json.dump({"script": {"content": "A28_PA={'label':'Rodin2','mesh':'/Game/P4/Rodin2/SK_P4_Rodin2','alias':{}}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_pa_plan.py').read())\n", "language": "Python"}}, open("tools/p4/jobs/_ab_paplan.mcp.json", "w"))
print("ab jobs written")
EOP
$M call $PY @$J/_ab_lab.mcp.json $U --timeout 300 | tail -1 | cut -c1-400
$M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1 | cut -c1-60
sleep 4
$M call $PY @$J/_ab_cap.mcp.json $U --timeout 300 | tail -1 | cut -c1-400
$M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1 | cut -c1-60
# capsules on Rodin2
$M call $PY @$J/_ab_paplan.mcp.json $U --timeout 120 | tail -1 | cut -c1-200
PLAN=$(grep -a "A28 pa plan" Game/logs/a28-editor-w5.log | tail -1 | sed 's/^.*A28 pa plan //')
$M tools PhysicsToolsets.PhysicsAssetToolset.AddBody PhysicsToolsets.PhysicsAssetToolset.SetCapsule PhysicsToolsets.PhysicsAssetToolset.SetSphere PhysicsToolsets.PhysicsAssetToolset.AddConstraint PhysicsToolsets.PhysicsAssetToolset.RemoveBody $U --timeout 60 > tools/p4/out/ab/pa_schemas.txt 2>&1
PA="/Game/P4/Rodin2/PA_P4_Rodin2.PA_P4_Rodin2"
for b in $($M call PhysicsToolsets.PhysicsAssetToolset.GetBodyNames "{\"physicsAsset\":{\"refPath\":\"$PA\"}}" $U --timeout 60 | python -I -c "
import sys,json,re
t=sys.stdin.read(); m=re.search(r'\"returnValue\\\\\":\[(.*?)\]', t); print(' '.join(x.strip('\\\\\"') for x in m.group(1).split(',')) if m else '')"); do
  $M call PhysicsToolsets.PhysicsAssetToolset.RemoveBody "{\"physicsAsset\":{\"refPath\":\"$PA\"},\"boneName\":\"$b\"}" $U --timeout 60 | tail -1 | cut -c1-60
done
python -I - "$PLAN" "$PA" tools/p4/out/ab/pa_schemas.txt tools/p4/out/ab/pa_calls.txt <<'EOP'
import json, sys, re
plan, pa, schemas, dst = sys.argv[1:5]
plan = json.loads(plan)
sch = open(schemas, encoding="utf8", errors="replace").read()
def params(tool):
    i = sch.find('"' + tool + '"')
    seg = sch[i:i + 4000]
    return re.findall(r'"([A-Za-z]+)": \{\s*"type"', seg) + re.findall(r'"([A-Za-z]+)": \{\s*"title"', seg)
c_par = params("PhysicsToolsets.PhysicsAssetToolset.AddConstraint")
child, parent = "bone1Name", "bone2Name"   # AddConstraint: bone1Name = child, bone2Name = parent (schema read 19:03)
lines = []
ref = {"physicsAsset": {"refPath": pa}}
for cl in plan["calls"]:
    if cl["op"] == "AddBody": lines.append(("AddBody", {**ref, "boneName": cl["bone"]}))
    elif cl["op"] == "SetSphere": lines.append(("SetSphere", {**ref, "boneName": cl["bone"], "shapeName": cl["name"], "center": dict(zip("xyz", cl["center"])), "radius": cl["radius"]}))
    elif cl["op"] == "SetCapsule": lines.append(("SetCapsule", {**ref, "boneName": cl["bone"], "shapeName": cl["name"], "center": dict(zip("xyz", cl["center"])), "rotation": dict(zip(("pitch", "yaw", "roll"), cl["rotation"])), "radius": cl["radius"], "length": cl["length"]}))
    elif cl["op"] == "AddConstraint": lines.append(("AddConstraint", {**ref, child: cl["child"], parent: cl["parent"]}))
open(dst, "w").write("\n".join("%s %s" % (t, json.dumps(a)) for t, a in lines))
print("pa calls", len(lines), "constraint params", child, parent)
EOP
n=0; while IFS= read -r line; do t=${line%% *}; a=${line#* }; r=$($M call PhysicsToolsets.PhysicsAssetToolset.$t "$a" $U --timeout 60 | tail -1); echo "$r" | grep -q '"verdict":"pass"' || echo "PA call failed: $t $(echo "$r" | cut -c1-160)"; n=$((n+1)); done < tools/p4/out/ab/pa_calls.txt; echo "A28 pa calls run $n"
$M call PhysicsToolsets.PhysicsAssetToolset.GetBodyNames "{\"physicsAsset\":{\"refPath\":\"$PA\"}}" $U --timeout 60 | grep -v '^{"tool"' | tr -d '\n ' | cut -c1-300; echo
$M call EditorToolset.AssetToolset.SaveAsset '{"assetPath":"/Game/P4/Rodin2/PA_P4_Rodin2"}' $U --timeout 60 | tail -1 | cut -c1-60
$M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1 | cut -c1-60
sleep 4
$M call $PY @$J/_ab_rag.mcp.json $U --timeout 300 | tail -1 | cut -c1-400
$M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1 | cut -c1-60
$M call $PY @$J/_saveall.mcp.json $U --timeout 120 | tail -1 | cut -c1-60
grep -a "A28 ab lab\|A28 ab bones\|A28 ab cap\|A28 pa plan" Game/logs/a28-editor-w5.log | sed 's/^.*LogPython: //' | cut -c1-300 | tail -14
ls "$OUT"
echo "A28 window5 end $(date +%H:%M:%S)"
