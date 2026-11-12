#!/usr/bin/env bash
# tools/p4/window2.sh (lane A28): the second editor window: sockets on the three figures, the stick imported and attached, the Meshy library
# falls imported on each skeleton, one fall captured per figure (with the ragdoll window), sheets, save, exit. Editor on 8015 must be up.
set -u; export MSYS_NO_PATHCONV=1; cd "$(dirname "$0")/../.."
U="--url http://127.0.0.1:8015/mcp"; M="node tools/mcp/mcp.mjs"; PY="editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python"; J=tools/p4/jobs
echo "A28 window2 start $(date +%H:%M:%S)"
# stick static mesh
cat > $J/_stick.mcp.json <<'EOS'
{"script":{"content":"import unreal\nat=unreal.AssetToolsHelpers.get_asset_tools(); lib=unreal.EditorAssetLibrary\nt=unreal.AssetImportTask(); t.set_editor_property('filename', r'C:/Users/kazda/kiro/thirdjam/tools/p4/out/SM_P4_Stick.fbx'); t.set_editor_property('destination_path','/Game/P4/Props'); t.set_editor_property('destination_name','SM_P4_Stick'); t.set_editor_property('replace_existing',True); t.set_editor_property('automated',True); t.set_editor_property('save',True)\nui=unreal.FbxImportUI(); ui.set_editor_property('import_mesh',True); ui.set_editor_property('import_as_skeletal',False); ui.set_editor_property('import_animations',False); ui.set_editor_property('import_materials',False); ui.set_editor_property('import_textures',False); ui.set_editor_property('automated_import_should_detect_type',False); ui.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_STATIC_MESH)\nsm=ui.get_editor_property('static_mesh_import_data'); sm.set_editor_property('convert_scene',True); sm.set_editor_property('convert_scene_unit',True); sm.set_editor_property('combine_meshes',True)\nt.set_editor_property('options',ui); t.set_editor_property('factory',unreal.FbxFactory())\nat.import_asset_tasks([t])\nm=lib.load_asset('/Game/P4/Props/SM_P4_Stick')\nprint('A28 stick imported', m.get_path_name() if m else None, m.get_bounds().box_extent if m else None)\n","language":"Python"}}
EOS
$M call $PY @$J/_stick.mcp.json $U --timeout 120 | tail -1 | cut -c1-200
for V in Meshy Rodin Tripo; do
  L=$(echo $V | tr A-Z a-z)
  bash tools/p4/sockets.sh $V hand_r root 2>&1 | grep -v '^{"tool"' | tr -d '\n' | cut -c1-300; echo
  cat > $J/_${L}_falls.mcp.json <<EOS
{"script":{"content":"A28_CLIPS={'vendor':'$V','dest':'/Game/P4/$V/Falls','skeleton':'/Game/P4/$V/SKEL_P4_$V','clips':[{'fbx':r'C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/NORM/falls.fbx','name':'AS_P4_${V}_fall'}]}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_clips.py').read())\n","language":"Python"}}
EOS
  if [ -f tools/p4/out/$L/NORM/falls.fbx ]; then $M call $PY @$J/_${L}_falls.mcp.json $U --timeout 300 | tail -1 | cut -c1-200; fi
  cat > $J/_${L}_lab2.mcp.json <<EOS
{"script":{"content":"A28_LAB={'vendor':'$V','mesh':'/Game/P4/$V/SK_P4_$V','stick':'/Game/P4/Props/SM_P4_Stick','stick_rot':[0,0,0]}\nexec(open(r'C:/Users/kazda/kiro/thirdjam/tools/p4/ue_lab.py').read())\n","language":"Python"}}
EOS
  $M call $PY @$J/_${L}_lab2.mcp.json $U --timeout 300 | tail -1 | cut -c1-200
  # which fall sequence exists? the legacy importer names takes <name>_<take>; find the first under /Falls
  FALL=$(grep -a "A28 $V clips" Game/logs/a28-editor.log | tail -1 | sed 's/^.*clips //' | python -I -c "import json,sys; l=json.load(sys.stdin); print(sorted(l)[0][0] if l else '')" 2>/dev/null)
  for clip in "/Game/P4/$V/Anims/MF_Unarmed_Walk_Fwd_P4_$V $L-walk-stick" "/Game/P4/$V/Falls/$FALL $L-fall"; do set -- $clip
    [ -n "$FALL" ] || [ "$2" != "$L-fall" ] || continue
    OUT="C:/Users/kazda/kiro/thirdjam/tools/p4/out/$L/cap"; mkdir -p "$OUT"
    python -I - "$V" "$1" "$OUT" "$2" "$J/_${L}_cap2.mcp.json" <<'EOP'
import json, sys
V, CLIP, OUT, TAG, dst = sys.argv[1:6]
s = open("tools/p4/ue_capture.py", encoding="utf8").read()
s = s.replace("__VENDOR__", V).replace("__CLIP__", CLIP).replace("__OUT__", OUT).replace("__TAG__", TAG).replace("__RAGDOLL__", "1" if TAG.endswith("fall") else "0")
json.dump({"script": {"content": s, "language": "Python"}}, open(dst, "w"))
EOP
    $M call EditorToolset.EditorAppToolset.StartPIE '{"options":{"bSimulate":true}}' $U --timeout 120 | tail -1 | cut -c1-80
    sleep 4
    $M call $PY @$J/_${L}_cap2.mcp.json $U --timeout 300 | tail -1 | cut -c1-300
    $M call EditorToolset.EditorAppToolset.StopPIE '{}' $U --timeout 60 | tail -1 | cut -c1-80
    TS=$(date +%Y%m%d-%H%M%S)
    python -I tools/p4/sheet.py "$OUT" "$2" chase "devlog/shots/$TS-A-a28-$2-chase.png" "devlog/shots/$TS-A-a28-$2-chase.mp4" "A28 $V $2 chase 4.2 m: f00-f15 = 2 s of the clip, rag/back = ragdoll 20 frames and the sequence again"
    python -I tools/p4/sheet.py "$OUT" "$2" far "devlog/shots/$TS-A-a28-$2-far.png" - "A28 $V $2 far 24 m"
  done
done
grep -a "A28 .* clips \|A28 stick\|stick_attached\|A28 .* cap clip_len" Game/logs/a28-editor.log | tail -20 | sed 's/^.*LogPython: //' | cut -c1-300
$M call $PY @$J/_saveall.mcp.json $U --timeout 120 | tail -1 | cut -c1-100
echo "A28 window2 end $(date +%H:%M:%S)"
