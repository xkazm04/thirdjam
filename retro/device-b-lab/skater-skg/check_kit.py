# Re-imports the goalie skeletal FBX and every SM_Goalie_<Piece>.fbx, attaches each piece to its bone with only the transform in
# attach.json (the way the engine would), and compares the pieces' world bounds with the source scene's (kit_reference_bounds.json).
#   blender -b --factory-startup --python check_kit.py -- <skeletal.fbx> <style_export_dir> [out.json]
import bpy, sys, os, json, glob
from mathutils import Matrix, Quaternion

a = sys.argv[sys.argv.index("--") + 1:]
skel, sdir = a[0], a[1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=skel, automatic_bone_orientation=False)
sc = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
att = json.load(open(os.path.join(sdir, "attach.json")))["pieces"]
ref = json.load(open(os.path.join(sdir, "kit_reference_bounds.json")))
pieces = {}
for f in sorted(glob.glob(os.path.join(sdir, "SM_Goalie_*.fbx"))):
    before = set(bpy.data.objects)
    bpy.ops.import_scene.fbx(filepath=f)
    new = [o for o in bpy.data.objects if o not in before and o.type == "MESH"]
    ob = new[0]
    for o in bpy.data.objects:
        o.select_set(o == ob)
    bpy.context.view_layer.objects.active = ob
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)   # mesh data in metres, Blender axes
    name = os.path.splitext(os.path.basename(f))[0]
    info = att[name]
    bone = arm.data.bones[info["bone"]]
    q = Quaternion(info["relative_rotation_quat_wxyz_blender_bone_space"])
    ob.parent = arm; ob.parent_type = "BONE"; ob.parent_bone = info["bone"]
    ob.matrix_parent_inverse = Matrix.Identity(4)
    s = 1.0 / arm.scale.x                                  # the armature object carries the cm -> m scale; undo it on the child
    ob.matrix_basis = Matrix.Translation((0, -bone.length, 0)) @ q.to_matrix().to_4x4() @ Matrix.Scale(s, 4)
    pieces[name] = ob
out = {"pieces_imported": len(pieces), "takes": len(bpy.data.actions), "max_bounds_error_cm": 0.0, "per_check": {}}
arm.animation_data_create()
for key, (mn0, mx0) in ref.items():
    clip, fr, name = key.split(":")
    acts = [x for x in bpy.data.actions if x.name.split("|")[-1] == clip]
    if not acts or name not in pieces:
        continue
    arm.animation_data.action = acts[0]
    sc.frame_set(int(fr))
    dg = bpy.context.evaluated_depsgraph_get()
    ev = pieces[name].evaluated_get(dg)
    pts = [ev.matrix_world @ v.co for v in ev.data.vertices]
    mn = [min(p[i] for p in pts) * 100 for i in range(3)]
    mx = [max(p[i] for p in pts) * 100 for i in range(3)]
    err = max(max(abs(x - y) for x, y in zip(mn, mn0)), max(abs(x - y) for x, y in zip(mx, mx0)))
    out["per_check"][key] = round(err, 3)
    out["max_bounds_error_cm"] = max(out["max_bounds_error_cm"], round(err, 3))
print("SKGKIT " + json.dumps({k: v for k, v in out.items() if k != "per_check"}))
if len(a) > 2:
    json.dump(out, open(a[2], "w"), indent=1)
