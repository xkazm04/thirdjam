"""tools/blender/verify_toy.py: headless Unreal check of the imported toy skater (lane A22). Run only when no other UnrealEditor-Cmd or package runs:

  UnrealEditor-Cmd.exe Game/ThirdJam.uproject -run=pythonscript -script="C:/.../tools/blender/verify_toy.py" -unattended -nopause -nosplash -NullRHI -log

Prints one "A22 verify" line per fact: bones, the three socket bones found by name (Unreal treats bone names as sockets), bounds in cm, the
animation sequences with their frame counts at 60 fps, and renames the auto-named skeleton to the contract prefix (SKEL_SkaterToy) when present.
"""
import unreal

FOLDER = "/Game/Figures"
MESH = FOLDER + "/SK_SkaterToy"
ar = unreal.AssetRegistryHelpers.get_asset_registry()
lib = unreal.EditorAssetLibrary

mesh = lib.load_asset(MESH)
if not mesh:
    print("A22 verify FAIL mesh missing at %s" % MESH)
else:
    skel = mesh.skeleton
    names = [str(n) for n in skel.get_editor_property("bone_tree")] if False else []
    try:
        ref = unreal.SkeletonEditorLibrary  # may not exist; fall back to the mesh component below
    except Exception:
        ref = None
    comp = unreal.SkeletalMeshComponent()
    comp.set_skeletal_mesh_asset(mesh)
    bone_names = [str(comp.get_bone_name(i)) for i in range(comp.get_num_bones())]
    print("A22 verify bones %d %s" % (len(bone_names), " ".join(bone_names)))
    for s in ("stick_socket", "puck_socket", "head_socket"):
        has = comp.does_socket_exist(s)
        loc = comp.get_socket_location(s) if has else None
        print("A22 verify socket %s %s %s" % (s, "found" if has else "MISSING", loc))
    b = mesh.get_bounds()
    print("A22 verify bounds origin %s extent %s" % (b.origin, b.box_extent))
    mats = mesh.get_editor_property("materials")
    print("A22 verify material_slots %s" % " ".join(str(m.material_slot_name) for m in mats))
    print("A22 verify lods %d" % mesh.get_num_lods() if hasattr(mesh, "get_num_lods") else "A22 verify lods ?")

anims = [a for a in ar.get_assets_by_path(FOLDER) if str(a.asset_class_path.asset_name) == "AnimSequence"]
for a in sorted(anims, key=lambda x: str(x.asset_name)):
    seq = a.get_asset()
    try:
        frames = seq.get_editor_property("number_of_sampled_frames")
    except Exception:
        frames = -1
    print("A22 verify anim %s frames %s len_s %.3f" % (a.asset_name, frames, seq.get_play_length()))

# the one-group FbxFactory import names sequences "<mesh>_Anim_<armature>_<action>"; the contract wants AS_SkaterToy_<action>
for a in anims:
    name = str(a.asset_name)
    marker = "_Anim_AS_SkaterToy_"
    if marker in name:
        target = FOLDER + "/AS_SkaterToy_" + name.split(marker, 1)[1]
        if not lib.does_asset_exist(target):
            print("A22 verify rename %s -> %s %s" % (name, target, lib.rename_asset(str(a.package_name), target)))
old = FOLDER + "/SK_SkaterToy_Skeleton"
new = FOLDER + "/SKEL_SkaterToy"
if lib.does_asset_exist(old) and not lib.does_asset_exist(new):
    ok = lib.rename_asset(old, new)
    print("A22 verify rename skeleton -> SKEL_SkaterToy %s" % ok)
    lib.save_directory(FOLDER, only_if_is_dirty=False)
    # the rename leaves a redirector at the old path; fix it up so no redirector ships
    try:
        unreal.AssetToolsHelpers.get_asset_tools().fixup_referencers([lib.find_asset_data(old).get_asset()]) if lib.does_asset_exist(old) else None
    except Exception as e:
        print("A22 verify redirector fixup skipped: %s" % e)
    lib.save_directory(FOLDER, only_if_is_dirty=False)
elif lib.does_asset_exist(new):
    print("A22 verify skeleton already SKEL_SkaterToy")
lib.save_directory(FOLDER, only_if_is_dirty=False)
# remove the redirectors the renames left behind (nothing outside this folder references these assets yet)
for a in ar.get_assets_by_path(FOLDER):
    if str(a.asset_class_path.asset_name) == "ObjectRedirector":
        print("A22 verify delete redirector %s %s" % (a.asset_name, lib.delete_asset(str(a.package_name))))
print("A22 verify DONE")
