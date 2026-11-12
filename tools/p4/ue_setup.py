"""tools/p4/ue_setup.py (lane A28): one vendor figure into Unreal, measured, retargeted from Manny, with a physics asset.

Run inside the editor's Python (MCP execute_python) as:
    A28_CFG = r"C:/.../tools/p4/jobs/<vendor>.ue.json"; exec(open(r"C:/.../tools/p4/ue_setup.py").read())
or headless: UnrealEditor-Cmd ThirdJam.uproject -run=pythonscript -script=... with A28_CFG in the environment.

Config keys: vendor, dest (/Game/P4/<Vendor>), sk_name (SK_P4_<Vendor>), mesh_fbx, clips [{fbx, name}] (vendor clips on the same
skeleton), texture_png (optional base colour), manny_clips [asset paths], chains (manual IK rig chains if the auto characterization fails).
Every fact is printed as one "A28 <vendor> <key> <value>" line; the last line is a JSON summary {"tool":"thirdjam.p4-setup/1", ...}.
"""
import unreal, json, os, math

cfg = json.load(open(os.environ.get("A28_CFG") or A28_CFG, encoding="utf8"))
V = cfg["vendor"]; DEST = cfg["dest"]; SK = cfg["sk_name"]
out = {"tool": "thirdjam.p4-setup/1", "vendor": V}
lib = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
def say(k, v):
    print("A28 %s %s %s" % (V, k, v)); out[k] = v

def fbx_task(fbx, dest, name, anim_only=False, skeleton=None):
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", fbx)
    task.set_editor_property("destination_path", dest)
    task.set_editor_property("destination_name", name)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("automated", True)
    task.set_editor_property("save", True)
    ui = unreal.FbxImportUI()
    ui.set_editor_property("import_mesh", not anim_only)
    ui.set_editor_property("import_as_skeletal", True)
    ui.set_editor_property("import_animations", anim_only)
    ui.set_editor_property("import_materials", False)
    ui.set_editor_property("import_textures", False)
    ui.set_editor_property("create_physics_asset", False)
    ui.set_editor_property("automated_import_should_detect_type", False)
    t = unreal.FBXImportType.FBXIT_ANIMATION if anim_only else unreal.FBXImportType.FBXIT_SKELETAL_MESH
    ui.set_editor_property("mesh_type_to_import", t)
    ui.set_editor_property("original_import_type", t)
    if skeleton:
        ui.set_editor_property("skeleton", skeleton)
    smid = ui.get_editor_property("skeletal_mesh_import_data")
    smid.set_editor_property("import_uniform_scale", float(cfg.get("import_scale", 1.0)))
    smid.set_editor_property("convert_scene", True)
    smid.set_editor_property("convert_scene_unit", True)
    smid.set_editor_property("import_morph_targets", False)
    smid.set_editor_property("import_meshes_in_bone_hierarchy", bool(cfg.get("meshes_in_bone_hierarchy", True)))
    smid.set_editor_property("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)
    asid = ui.get_editor_property("anim_sequence_import_data")
    asid.set_editor_property("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME)
    asid.set_editor_property("import_custom_attribute", False)
    asid.set_editor_property("import_uniform_scale", float(cfg.get("import_scale", 1.0)))
    asid.set_editor_property("convert_scene", True)
    asid.set_editor_property("convert_scene_unit", True)
    task.set_editor_property("options", ui)
    task.set_editor_property("factory", unreal.FbxFactory())
    at.import_asset_tasks([task])
    return [str(p) for p in task.get_editor_property("imported_object_paths")]

# 1. mesh
paths = fbx_task(cfg["mesh_fbx"], DEST, SK)
say("import_mesh_paths", paths)
mesh = lib.load_asset(DEST + "/" + SK)
if not mesh:
    raise RuntimeError("mesh import failed")
skel = mesh.skeleton
old_skel = skel.get_path_name().split(".")[0]
new_skel = DEST + "/SKEL_P4_" + V
if old_skel != new_skel and not lib.does_asset_exist(new_skel):
    say("rename_skeleton", lib.rename_asset(old_skel, new_skel))
    skel = lib.load_asset(new_skel)
say("skeleton", skel.get_path_name())

# 2. bones, bounds, size check against Manny
comp = unreal.SkeletalMeshComponent()
comp.set_skeletal_mesh_asset(mesh)
bones = [str(comp.get_bone_name(i)) for i in range(comp.get_num_bones())]
say("bone_count", len(bones)); say("bones", " ".join(bones))
b = mesh.get_bounds(); say("bounds_origin_cm", [round(b.origin.x, 1), round(b.origin.y, 1), round(b.origin.z, 1)]); say("bounds_extent_cm", [round(b.box_extent.x, 1), round(b.box_extent.y, 1), round(b.box_extent.z, 1)])
def measure(c):
    def loc(n): return c.get_socket_location(n) if c.does_socket_exist(n) else None
    def dist(a, b_): return None if (a is None or b_ is None) else round((a - b_).length(), 1)
    pel = loc("pelvis"); hl, hr = loc("hand_l"), loc("hand_r"); head = loc("head")
    hand_end = None
    for n in ("RightHand_End", "middle_01_r", "middle_metacarpal_r", "index_01_r"):
        if c.does_socket_exist(n): hand_end = loc(n); break
    bb = c.skeletal_mesh_asset.get_bounds() if hasattr(c, "skeletal_mesh_asset") else c.get_skeletal_mesh_asset().get_bounds()
    return {"pelvis_z_cm": round(pel.z, 1) if pel else None, "head_z_cm": round(head.z, 1) if head else None,
            "hand_span_cm": dist(hl, hr), "hand_len_cm": dist(hr, hand_end), "height_cm": round(bb.origin.z + bb.box_extent.z, 1)}
m_v = measure(comp)
manny = lib.load_asset("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple")
cm = unreal.SkeletalMeshComponent(); cm.set_skeletal_mesh_asset(manny)
m_m = measure(cm)
say("size_vendor", m_v); say("size_manny", m_m)
say("size_ratio", {k: (round(m_v[k] / m_m[k], 2) if m_v.get(k) and m_m.get(k) else None) for k in m_v})

# 3. texture + material (base colour only)
if cfg.get("texture_png") and os.path.exists(cfg["texture_png"]):
    tt = unreal.AssetImportTask()
    tt.set_editor_property("filename", cfg["texture_png"]); tt.set_editor_property("destination_path", DEST)
    tt.set_editor_property("destination_name", "T_P4_" + V + "_BC"); tt.set_editor_property("replace_existing", True)
    tt.set_editor_property("automated", True); tt.set_editor_property("save", True)
    at.import_asset_tasks([tt])
    tex = lib.load_asset(DEST + "/T_P4_" + V + "_BC")
    if tex:
        mpath = DEST + "/M_P4_" + V
        mat = lib.load_asset(mpath) if lib.does_asset_exist(mpath) else at.create_asset("M_P4_" + V, DEST, unreal.Material, unreal.MaterialFactoryNew())
        mel = unreal.MaterialEditingLibrary
        if mel.get_num_material_expressions(mat) == 0:
            ts = mel.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -350, 0)
            ts.set_editor_property("texture", tex)
            mel.connect_material_property(ts, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
            mel.recompile_material(mat)
        mats = list(mesh.get_editor_property("materials"))
        for i in range(len(mats)):
            mats[i].material_interface = mat
        mesh.set_editor_property("materials", mats)
        say("material", mpath)
        say("texture_size", [tex.blueprint_get_size_x(), tex.blueprint_get_size_y()])

# 4. vendor clips on the same skeleton
clip_paths = []
for c in cfg.get("clips", []):
    p = fbx_task(c["fbx"], DEST, c["name"], anim_only=True, skeleton=skel)
    clip_paths += p
say("vendor_clip_paths", clip_paths)

# 5. IK rig on the vendor skeleton, IK rig on Manny, retargeter Manny -> vendor, batch retarget
def make_rig(name, folder, skm, chains=None):
    p = folder + "/" + name
    rig = lib.load_asset(p) if lib.does_asset_exist(p) else at.create_asset(name, folder, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(skm)
    ok = c.apply_auto_generated_retarget_definition()
    rep = {"auto": bool(ok)}
    if not ok and chains:
        c.set_retarget_root(chains["root"])
        for ch in chains["chains"]:
            c.add_retarget_chain(ch[0], ch[1], ch[2], "")
        rep["manual_chains"] = [ch[0] for ch in chains["chains"]]
    try:
        rep["chains"] = [str(x.chain_name) for x in c.get_retarget_chains()]
    except Exception as e:
        rep["chains_err"] = str(e)[:120]
    try:
        rep["retarget_root"] = str(c.get_retarget_root())
    except Exception:
        pass
    lib.save_asset(p, only_if_is_dirty=False)
    return rig, rep
tgt_rig, rep_t = make_rig("IK_P4_" + V, DEST, mesh, cfg.get("chains"))
say("ikrig_vendor", rep_t)
src_rig, rep_s = make_rig("IK_P4_Manny", "/Game/P4/Manny", manny)
say("ikrig_manny", rep_s)
rtg_path = DEST + "/RTG_Manny_to_P4_" + V
rtg = lib.load_asset(rtg_path) if lib.does_asset_exist(rtg_path) else at.create_asset("RTG_Manny_to_P4_" + V, DEST, unreal.IKRetargeter, unreal.IKRetargetFactory())
rc = unreal.IKRetargeterController.get_controller(rtg)
S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
rc.set_ik_rig(S, src_rig); rc.set_ik_rig(T, tgt_rig)
rc.set_preview_mesh(S, manny); rc.set_preview_mesh(T, mesh)
try:
    rc.add_default_ops(); rc.assign_ik_rig_to_all_ops(S, src_rig); rc.assign_ik_rig_to_all_ops(T, tgt_rig)
except Exception as e:
    say("retargeter_ops_err", str(e)[:200])
rc.auto_map_chains(unreal.AutoMapChainType.FUZZY, True)
try:
    rc.auto_align_all_bones(S); rc.auto_align_all_bones(T)
except Exception as e:
    say("auto_align_err", str(e)[:200])
try:
    mapping = []
    for ch in rc.get_chain_mappings() if hasattr(rc, "get_chain_mappings") else []:
        mapping.append(str(ch))
    say("chain_mappings", mapping[:40])
except Exception as e:
    say("chain_mappings_err", str(e)[:120])
lib.save_asset(rtg_path, only_if_is_dirty=False)
clips = [lib.find_asset_data(p) for p in cfg.get("manny_clips", [])]
clips = [c for c in clips if c.is_valid()]
res_paths = []
try:
    res = unreal.IKRetargetBatchOperation.duplicate_and_retarget(clips, manny, mesh, rtg, prefix="", suffix="_P4_" + V, target_path=DEST + "/Anims")
    res_paths = [str(a.package_name) for a in res]
except Exception as e:
    say("batch_retarget_err", str(e)[:300])
    try:
        inp = unreal.IKRetargetBatchOperationInputs()
        inp.set_editor_property("assets_to_retarget", clips)
        inp.set_editor_property("source_mesh", manny); inp.set_editor_property("target_mesh", mesh); inp.set_editor_property("ik_retarget_asset", rtg)
        inp.set_editor_property("target_path", DEST + "/Anims"); inp.set_editor_property("suffix", "_P4_" + V)
        res = unreal.IKRetargetBatchOperation.run_batch_retarget_with_results(inp)
        res_paths = [str(r) for r in res]
    except Exception as e2:
        say("batch_retarget_err2", str(e2)[:300])
say("retargeted", res_paths)
for p in res_paths:
    a = lib.load_asset(p)
    if a:
        try:
            a.set_editor_property("enable_root_motion", False)
        except Exception:
            pass
        say("retargeted_len_s", [p.split("/")[-1], round(a.get_play_length(), 3)])
lib.save_directory(DEST, only_if_is_dirty=False)

# 6. physics asset
pa_path = DEST + "/PA_P4_" + V
try:
    sub = unreal.get_editor_subsystem(unreal.SkeletalMeshEditorSubsystem)
    pa = lib.load_asset(pa_path) if lib.does_asset_exist(pa_path) else sub.create_physics_asset(mesh)
    if pa and pa.get_path_name().split(".")[0] != pa_path:
        lib.rename_asset(pa.get_path_name().split(".")[0], pa_path); pa = lib.load_asset(pa_path)
    mesh.set_editor_property("physics_asset", pa)
    try:
        bodies = [str(bs.bone_name) for bs in pa.get_editor_property("skeletal_body_setups")]
    except Exception:
        bodies = []
    say("physics_asset", {"path": pa_path, "bodies": bodies})
except Exception as e:
    say("physics_asset_err", str(e)[:300])
lib.save_asset(DEST + "/" + SK, only_if_is_dirty=False)
lib.save_directory(DEST, only_if_is_dirty=False)
# redirectors
ar = unreal.AssetRegistryHelpers.get_asset_registry()
for a in ar.get_assets_by_path(DEST, recursive=True):
    if str(a.asset_class_path.asset_name) == "ObjectRedirector":
        say("delete_redirector", [str(a.asset_name), lib.delete_asset(str(a.package_name))])
print(json.dumps(out))
if cfg.get("report"):
    open(cfg["report"], "w", encoding="utf8").write(json.dumps(out, indent=1))
