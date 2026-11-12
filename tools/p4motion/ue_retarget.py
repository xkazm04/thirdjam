"""tools/p4motion/ue_retarget.py: editor-side import + IK Retargeter for lane A29. Runs inside the Unreal editor's Python (MCP execute_python on port 8016,
or `UnrealEditor-Cmd ... -run=pythonscript -script=...`). Parameters come from a JSON file whose path is in the environment variable P4M_ARGS:

  {"fbx": "<stripped clip .fbx>", "clip": "AS_P4_skate_fwd", "vendor": "meshy", "bonemap": "<tools/p4motion/bonemap-meshy.json>",
   "src_folder": "/Game/P4/Anim/Src/Meshy", "src_mesh_name": "SK_P4_MeshyRef", "out_folder": "/Game/P4/Anim", "target_mesh": "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
   "sample_rate": 60, "reuse_source_skeleton": true}

Steps: 1 import the FBX with the legacy FbxFactory (mesh + animation, no materials/textures, 60 fps sampling) into src_folder, renaming the mesh/skeleton to the
vendor's shared names on the first import; 2 build (once per vendor) the source IK Rig from the bone table, with the totality report; 3 build (once) the target
Manny IK Rig from the auto definition; 4 build (once) the retargeter, map chains by name (EXACT), align bones; 5 duplicate-and-retarget the clip into out_folder
as <clip>; 6 save everything. Prints one JSON line (P4M_RESULT ...) with the asset paths, frame counts and the unmapped bones.
"""
import unreal, json, os

A = json.load(open(os.environ["P4M_ARGS"]))
lib = unreal.EditorAssetLibrary
at = unreal.AssetToolsHelpers.get_asset_tools()
out = {"tool": "thirdjam.p4motion-retarget/1", "clip": A["clip"]}
bm = json.load(open(A["bonemap"]))
src_folder, out_folder = A["src_folder"], A["out_folder"]
SRC_MESH_PATH = src_folder + "/" + A["src_mesh_name"]
SRC_SKEL_PATH = src_folder + "/" + A["src_mesh_name"].replace("SK_", "SKEL_", 1)
rate = int(A.get("sample_rate", 60))


def bone_names(mesh):
    comp = unreal.SkeletalMeshComponent()
    comp.set_skeletal_mesh_asset(mesh)
    return [str(comp.get_bone_name(i)) for i in range(comp.get_num_bones())]


# 1. import
ui = unreal.FbxImportUI()
ui.import_mesh = True
ui.import_as_skeletal = True
ui.import_animations = True
ui.import_materials = False
ui.import_textures = False
ui.create_physics_asset = False
ui.automated_import_should_detect_type = False
ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_SKELETAL_MESH
ui.original_import_type = unreal.FBXImportType.FBXIT_SKELETAL_MESH
have_skel = lib.does_asset_exist(SRC_SKEL_PATH)
if have_skel and A.get("reuse_source_skeleton", True):
    ui.skeleton = lib.load_asset(SRC_SKEL_PATH)
def setp(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception as e:
        out.setdefault("import_props_skipped", []).append(name)
        return False
smd = ui.skeletal_mesh_import_data
for k, v in (("import_morph_targets", False), ("import_meshes_in_bone_hierarchy", True), ("convert_scene", True), ("convert_scene_unit", True),
             ("normal_import_method", unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS)):
    setp(smd, k, v)
asd = ui.anim_sequence_import_data
for k, v in (("animation_length", unreal.FBXAnimationLengthImportType.FBXALIT_EXPORTED_TIME), ("use_default_sample_rate", False), ("custom_sample_rate", rate),
             ("snap_to_closest_frame_boundary", True), ("import_custom_attribute", False)):
    setp(asd, k, v)

task = unreal.AssetImportTask()
task.filename = A["fbx"]
task.destination_path = src_folder + "/import_" + A["clip"]
task.destination_name = A["clip"]
task.replace_existing = True
task.automated = True
task.save = False
task.options = ui
at.import_asset_tasks([task])
imported = [str(p) for p in task.imported_object_paths]
out["imported"] = imported
anim = mesh = skel = None
for p in imported:
    o = lib.load_asset(p.split(".")[0])
    if isinstance(o, unreal.AnimSequence):
        anim = o
    elif isinstance(o, unreal.SkeletalMesh):
        mesh = o
    elif isinstance(o, unreal.Skeleton):
        skel = o
if anim is None:
    print("P4M_RESULT " + json.dumps(dict(out, ok=False, error="no AnimSequence imported")))
    raise SystemExit(0)
# the shared source mesh/skeleton: keep the first import's, drop later duplicates
if not lib.does_asset_exist(SRC_MESH_PATH):
    if mesh is not None:
        lib.rename_asset(mesh.get_path_name().split(".")[0], SRC_MESH_PATH)
    if skel is not None:
        lib.rename_asset(skel.get_path_name().split(".")[0], SRC_SKEL_PATH)
    elif mesh is not None:
        s = lib.load_asset(SRC_MESH_PATH).skeleton
        lib.rename_asset(s.get_path_name().split(".")[0], SRC_SKEL_PATH)
src_mesh = lib.load_asset(SRC_MESH_PATH)
src_skel = lib.load_asset(SRC_SKEL_PATH)
# park the clip under src_folder/<clip>_src and remove any duplicate mesh/skeleton the import made
anim_path = src_folder + "/" + A["clip"] + "_src"
if lib.does_asset_exist(anim_path):
    lib.delete_asset(anim_path)
lib.rename_asset(anim.get_path_name().split(".")[0], anim_path)
anim = lib.load_asset(anim_path)
for p in imported:
    base = p.split(".")[0]
    if lib.does_asset_exist(base) and base not in (SRC_MESH_PATH, SRC_SKEL_PATH, anim_path):
        lib.delete_asset(base)
if lib.does_directory_exist(task.destination_path):
    lib.delete_directory(task.destination_path)
out["src_anim"] = anim_path
out["src_frames"] = anim.get_editor_property("number_of_sampled_frames") if hasattr(anim, "get_editor_property") else None
try:
    out["src_frames"] = unreal.AnimationLibrary.get_num_frames(anim)
    out["src_seconds"] = round(unreal.AnimationLibrary.get_sequence_length(anim), 4)
    out["src_rate"] = str(unreal.AnimationLibrary.get_rate_scale(anim))
except Exception as e:
    out["src_frames_err"] = str(e)[:80]
src_bones = bone_names(src_mesh)
out["src_bone_count"] = len(src_bones)
# the Director's check before any rig is built: the source pelvis must stand about 90 cm up the Z axis, in the bind pose and in the clip's first frame
try:
    root_src = bm["retarget_root"]["source"]
    comp = unreal.SkeletalMeshComponent(); comp.set_skeletal_mesh_asset(src_mesh)
    bind = comp.get_socket_transform(root_src, unreal.RelativeTransformSpace.RTS_COMPONENT).translation
    out["src_pelvis_bind_cm"] = [round(bind.x, 1), round(bind.y, 1), round(bind.z, 1)]
    t0 = unreal.AnimationLibrary.get_bone_pose_for_time(anim, root_src, 0.0, False).translation
    out["src_pelvis_t0_local_cm"] = [round(t0.x, 1), round(t0.y, 1), round(t0.z, 1)]
    parent = str(comp.get_parent_bone(root_src))
    out["src_pelvis_parent"] = parent
    if parent != "None":
        pr = unreal.AnimationLibrary.get_bone_pose_for_time(anim, parent, 0.0, False)
        out["src_parent_t0_rot_euler"] = [round(v, 1) for v in (pr.rotation.euler().x, pr.rotation.euler().y, pr.rotation.euler().z)]
        out["src_parent_t0_scale"] = [round(v, 3) for v in (pr.scale3d.x, pr.scale3d.y, pr.scale3d.z)]
except Exception as e:
    out["src_pelvis_check_err"] = str(e)[:120]

# 2. source IK rig from the bone table
rig_folder = out_folder + "/Rigs"
SRC_RIG = rig_folder + "/IK_P4_" + A["vendor"].capitalize() + "Src"
if not lib.does_asset_exist(SRC_RIG):
    rig = at.create_asset(SRC_RIG.split("/")[-1], rig_folder, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(src_mesh)
    c.set_retarget_root(bm["retarget_root"]["source"])
    missing_src = []
    for ch in bm["chains"]:
        s0, s1 = ch["source"]
        if s0 not in src_bones or s1 not in src_bones:
            missing_src.append(ch["chain"])
            continue
        c.add_retarget_chain(ch["chain"], s0, s1, "")
    out["src_chains_missing_bones"] = missing_src
    lib.save_asset(SRC_RIG, only_if_is_dirty=False)
src_rig = lib.load_asset(SRC_RIG)
out["src_rig"] = SRC_RIG
out["src_chain_count"] = len(unreal.IKRigController.get_controller(src_rig).get_retarget_chains())

# 3. target rig (Manny auto definition)
TGT_MESH = lib.load_asset(A["target_mesh"])
TGT_RIG = rig_folder + "/IK_P4_Manny"
if not lib.does_asset_exist(TGT_RIG):
    rig = at.create_asset("IK_P4_Manny", rig_folder, unreal.IKRigDefinition, unreal.IKRigDefinitionFactory())
    c = unreal.IKRigController.get_controller(rig)
    c.set_skeletal_mesh(TGT_MESH)
    c.apply_auto_generated_retarget_definition()
    c.apply_auto_fbik()
    lib.save_asset(TGT_RIG, only_if_is_dirty=False)
tgt_rig = lib.load_asset(TGT_RIG)
tc = unreal.IKRigController.get_controller(tgt_rig)
tgt_chains = {}
for ch in tc.get_retarget_chains():
    tgt_chains[str(ch.get_editor_property("chain_name"))] = (str(ch.get_editor_property("start_bone").get_editor_property("bone_name")),
                                                             str(ch.get_editor_property("end_bone").get_editor_property("bone_name")))
# totality: which target core bones lie on a chain that the bone table maps
tgt_bones = bone_names(TGT_MESH)
parent_of = {}
comp = unreal.SkeletalMeshComponent(); comp.set_skeletal_mesh_asset(TGT_MESH)
for i, b in enumerate(tgt_bones):
    parent_of[b] = str(comp.get_parent_bone(b))
mapped_names = {ch["chain"] for ch in bm["chains"]}
covered = set()
for name, (b0, b1) in tgt_chains.items():
    if name not in mapped_names:
        continue
    b = b1
    while True:
        covered.add(b)
        if b == b0 or b == "None" or b not in parent_of:
            break
        b = parent_of[b]
covered.add(bm["retarget_root"]["target"])
out["target_unmapped_core"] = [b for b in bm["target_core_bones"] if b not in covered]
out["target_chains_without_source"] = sorted(n for n in tgt_chains if n not in mapped_names)
out["bonemap_chains_not_in_target"] = sorted(n for n in mapped_names if n not in tgt_chains)

# 4. retargeter
RTG = rig_folder + "/RTG_P4_" + A["vendor"].capitalize() + "_Manny"
if not lib.does_asset_exist(RTG):
    rtg = at.create_asset(RTG.split("/")[-1], rig_folder, unreal.IKRetargeter, unreal.IKRetargetFactory())
    rc = unreal.IKRetargeterController.get_controller(rtg)
    S, T = unreal.RetargetSourceOrTarget.SOURCE, unreal.RetargetSourceOrTarget.TARGET
    rc.set_ik_rig(S, src_rig); rc.set_ik_rig(T, tgt_rig)
    rc.set_preview_mesh(S, src_mesh); rc.set_preview_mesh(T, TGT_MESH)
    rc.add_default_ops()
    rc.assign_ik_rig_to_all_ops(S, src_rig); rc.assign_ik_rig_to_all_ops(T, tgt_rig)
    rc.auto_map_chains(unreal.AutoMapChainType.EXACT, True)
    rc.auto_align_all_bones(S); rc.auto_align_all_bones(T)
    lib.save_asset(RTG, only_if_is_dirty=False)
rtg = lib.load_asset(RTG)
# op policy, applied on every run (pass 3): Pelvis Motion ON (the source is normalized by clip.py --normalize: bones in cm, rest skeleton on the ground),
# Root Motion OFF (the sim owns displacement; clip.py strips the pelvis X/Y travel and keeps Z), Run IK Rig OFF (FK only for now).
rc = unreal.IKRetargeterController.get_controller(rtg)
ops = []
for i in range(rc.get_num_retarget_ops()):
    t = str(rc.get_op_type_by_name(rc.get_op_name(i)))
    rc.set_retarget_op_enabled(i, not ("RunIKRig" in t or "RootMotion" in t))
    ops.append([str(rc.get_op_name(i)), rc.get_retarget_op_enabled(i)])
out["retarget_ops"] = ops
lib.save_asset(RTG, only_if_is_dirty=False)
out["retargeter"] = RTG

# 5. retarget the clip
OUT = out_folder + "/" + A["clip"]
if lib.does_asset_exist(OUT):
    lib.delete_asset(OUT)
res = unreal.IKRetargetBatchOperation.duplicate_and_retarget([lib.find_asset_data(anim.get_path_name())], src_mesh, TGT_MESH, rtg, prefix="", target_path=out_folder)
made = [str(a.package_name) for a in res]
out["retargeted"] = made
if made and made[0] != OUT:
    lib.rename_asset(made[0], OUT)
if lib.does_asset_exist(OUT):
    o = lib.load_asset(OUT)
    out["out"] = OUT
    try:
        out["out_frames"] = unreal.AnimationLibrary.get_num_frames(o)
        out["out_seconds"] = round(unreal.AnimationLibrary.get_sequence_length(o), 4)
    except Exception as e:
        out["out_frames_err"] = str(e)[:80]
    out["out_skeleton"] = str(o.get_editor_property("skeleton").get_path_name())
    try:
        L = unreal.AnimationLibrary.get_sequence_length(o)
        zs = [round(unreal.AnimationLibrary.get_bone_pose_for_time(o, "pelvis", min(k * L / 24.0, L), False).translation.z, 1) for k in range(25)]
        out["out_pelvis_z_cm"] = {"t0": zs[0], "min": min(zs), "max": max(zs), "end": zs[-1]}
    except Exception as e:
        out["out_pelvis_err"] = str(e)[:80]
    # moving? sample thigh_l over time
    try:
        ps = [unreal.AnimationLibrary.get_bone_pose_for_time(o, "thigh_l", t, False).rotation.euler() for t in (0.0, 0.5, 1.0)]
        out["thigh_l_euler_0_05_10"] = [[round(v, 1) for v in (p.x, p.y, p.z)] for p in ps]
    except Exception as e:
        out["sample_err"] = str(e)[:80]
    lib.save_asset(OUT, only_if_is_dirty=False)
for p in (SRC_MESH_PATH, SRC_SKEL_PATH, anim_path):
    if lib.does_asset_exist(p):
        lib.save_asset(p, only_if_is_dirty=False)
out["ok"] = bool(made)
print("P4M_RESULT " + json.dumps(out))
