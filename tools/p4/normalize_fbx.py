# tools/p4/normalize_fbx.py (lane A28): bake a vendor FBX's armature scale into the bones so Unreal gets bone translations in centimetres
# (a Meshy ue5_mannequin FBX carries a 100x "Armature" node; the IK Retargeter then writes pelvis heights 100x too big).
#   blender -b --factory-startup --python tools/p4/normalize_fbx.py -- <in.fbx> <out.fbx> [root_name] [zup|bake] [prerot_deg] [all|one] [ground_cm|auto]
# Prints "A28 norm <key> <value>" lines. The armature object is renamed root_name (default "root") so Unreal's extra root bone is named root.
import bpy, sys, os
a = sys.argv[sys.argv.index("--") + 1:]
src, dst = a[0], a[1]
root_name = a[2] if len(a) > 2 else "root"
variant = a[3] if len(a) > 3 else "zup"
prerot = float(a[4]) if len(a) > 4 else 0.0
all_actions = len(a) > 5 and a[5] == "all"
ground_cm = a[6] if len(a) > 6 else "auto"   # "auto" = lowest mesh vertex to z 0 (rest file); a number = the rest file's offset, for the clip files of the same figure   # export every action as its own take (the Meshy library file carries one action per clip)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=src, automatic_bone_orientation=False, use_anim=True, ignore_leaf_bones=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
print("A28 norm armature_scale_before", [round(v, 4) for v in arm.scale], "loc", [round(v, 4) for v in arm.location], "rot", [round(v, 4) for v in arm.rotation_euler])
bpy.ops.object.select_all(action="DESELECT")
for o in [arm] + meshes:
    o.select_set(True)
bpy.context.view_layer.objects.active = arm
import mathutils
for o in meshes:
    # the FBX importer parents the mesh with a parent-inverse holding the Y-up rotation; bake it so mesh and bones share one space
    o.matrix_world = o.matrix_world.copy()
    mw = o.matrix_world.copy(); o.matrix_parent_inverse = mathutils.Matrix.Identity(4); o.matrix_world = mw
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
print("A28 norm mesh_local_after", [[round(v, 3) for row in o.matrix_local for v in row][:4] for o in meshes])
# centimetre scene as lane A22's skater.py: 1 Blender unit = 1 cm, so the FBX carries cm with no scale on the armature node
sc = bpy.context.scene
sc.unit_settings.system = "METRIC"; sc.unit_settings.scale_length = 0.01; sc.unit_settings.length_unit = "CENTIMETERS"
arm.scale = (100.0, 100.0, 100.0)
for o in meshes:
    if o.parent is not arm:
        o.scale = (100.0, 100.0, 100.0)
bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
# ground: the Tripo mesh is centred on the origin; put the lowest mesh vertex on z = 0 so the feet stand on the ice
import mathutils as _mu
zmin = min((o.matrix_world @ _mu.Vector(c)).z for o in meshes for c in o.bound_box) if ground_cm == "auto" else -float(ground_cm)
if abs(zmin) > 0.5:
    for o in [arm] + [m for m in meshes if m.parent is not arm]:
        o.location.z -= zmin
    bpy.ops.object.transform_apply(location=True, rotation=False, scale=False)
    print("A28 norm grounded_by_cm", round(-zmin, 2))
if prerot:
    import math
    arm.rotation_euler = (math.radians(prerot), 0.0, 0.0)
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
print("A28 norm armature_scale_after", [round(v, 4) for v in arm.scale], "unit cm prerot", prerot)
arm.name = root_name
arm.data.name = root_name
zs = [ (arm.matrix_world @ b.head_local).z for b in arm.data.bones ]
print("A28 norm bone_z_m min %.3f max %.3f pelvis %.3f" % (min(zs), max(zs), (arm.matrix_world @ arm.data.bones["pelvis"].head_local).z if "pelvis" in arm.data.bones else -1))
print("A28 norm actions", [(ac.name, [int(x) for x in ac.frame_range]) for ac in bpy.data.actions])
bpy.ops.object.select_all(action="SELECT")
bpy.ops.export_scene.fbx(filepath=dst, use_selection=True, apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0,
                         bake_space_transform=(variant == "bake"), add_leaf_bones=False, use_armature_deform_only=False, bake_anim=True,
                         bake_anim_use_all_actions=all_actions, bake_anim_use_nla_strips=False, bake_anim_simplify_factor=0.0,
                         object_types={"ARMATURE", "MESH"}, mesh_smooth_type="FACE", path_mode="COPY", embed_textures=False, axis_forward=("Y" if variant == "zup" else "-Z"), axis_up=("Z" if variant == "zup" else "Y"))
print("A28 norm wrote", dst, os.path.getsize(dst))
