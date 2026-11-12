"""tools/p4/ue_clips.py (lane A28): animation-only FBX imports onto an existing P4 skeleton (legacy FbxFactory, every take becomes a sequence).
Run in the editor's Python: A28_CLIPS = {"vendor": "Meshy", "dest": "/Game/P4/Meshy/Falls", "skeleton": "/Game/P4/Meshy/SKEL_P4_Meshy",
                                        "clips": [{"fbx": "...", "name": "AS_P4_Meshy_fall"}]}; exec(open(r".../ue_clips.py").read())
"""
import unreal, json
P = A28_CLIPS; V = P["vendor"]
lib = unreal.EditorAssetLibrary; at = unreal.AssetToolsHelpers.get_asset_tools()
skel = lib.load_asset(P["skeleton"])
out = []
for c in P["clips"]:
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", c["fbx"]); task.set_editor_property("destination_path", P["dest"]); task.set_editor_property("destination_name", c["name"])
    task.set_editor_property("replace_existing", True); task.set_editor_property("automated", True); task.set_editor_property("save", True)
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", False); ui.set_editor_property("import_as_skeletal", True); ui.set_editor_property("import_animations", True)
    ui.set_editor_property("import_materials", False); ui.set_editor_property("import_textures", False); ui.set_editor_property("automated_import_should_detect_type", False)
    ui.set_editor_property("mesh_type_to_import", unreal.FBXImportType.FBXIT_ANIMATION); ui.set_editor_property("original_import_type", unreal.FBXImportType.FBXIT_ANIMATION)
    ui.set_editor_property("skeleton", skel)
    asid = ui.get_editor_property("anim_sequence_import_data")
    asid.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME); asid.set_editor_property("import_custom_attribute", False)
    asid.set_editor_property("convert_scene", True); asid.set_editor_property("convert_scene_unit", True)
    task.set_editor_property("options", ui); task.set_editor_property("factory", unreal.FbxFactory())
    at.import_asset_tasks([task])
    paths = [str(p) for p in task.get_editor_property("imported_object_paths")]
    for p in paths:
        a = lib.load_asset(p)
        if a:
            try: a.set_editor_property("enable_root_motion", False)
            except Exception: pass
            out.append([p.split(".")[0].split("/")[-1], round(a.get_play_length(), 3)])
lib.save_directory(P["dest"], only_if_is_dirty=False)
print("A28 %s clips %s" % (V, json.dumps(out)))
