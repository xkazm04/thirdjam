# Re-imports the exported FBX and measures every take: pelvis horizontal excursion (max over the clip, not only first-to-last),
# loop seam (max joint distance first vs last frame), and ice contact (lowest mesh point over the clip, sampled every 2nd frame).
#   blender -b --factory-startup --python check_fbx.py -- <file.fbx> [out.json]
import bpy, sys, json, math

a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=a[0], automatic_bone_orientation=False)
sc = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
body = max((o for o in bpy.data.objects if o.type == "MESH"), key=lambda o: len(o.data.vertices))
def rnd(M): return [[round(x, 4) for x in row] for row in M]
out0 = {"armature_matrix_world": rnd(arm.matrix_world), "root_rest_matrix_local": rnd(arm.data.bones["root"].matrix_local),
        "unit_scale_length": sc.unit_settings.scale_length}
out = {"armature_object": arm.name, "armature_scale": [round(x, 4) for x in arm.scale],
       "bone_count": len(arm.data.bones), "root_bones": [b.name for b in arm.data.bones if not b.parent],
       "fps": sc.render.fps, "takes": {}}
arm.animation_data_create()
for act in bpy.data.actions:
    arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    pel, joints, lowest, highest = [], [], 9.0, -9.0
    for f in range(f0, f1 + 1):
        sc.frame_set(f)
        mw = arm.matrix_world
        pel.append(mw @ arm.pose.bones["pelvis"].head)
        joints.append([mw @ pb.head for pb in arm.pose.bones])
        if (f - f0) % 2 == 0 or f == f1:
            dg = bpy.context.evaluated_depsgraph_get()
            zs = [(ev.matrix_world @ v.co).z for ev in (o.evaluated_get(dg) for o in bpy.data.objects if o.type == "MESH")
                  for v in ev.data.vertices]
            lowest = min(lowest, min(zs))
            highest = max(highest, max(zs))
    keyed = sorted({fc.data_path.split('"')[1] for fc in act.fcurves if fc.data_path.startswith("pose.bones")})
    p0 = pel[0]
    out["takes"][act.name] = {
        "frames": f1 - f0 + 1, "keyed_bones": len(keyed), "root_keyed": "root" in keyed,
        "pelvis_roll_mid_deg": None,
        "pelvis_horizontal_max_cm": round(max(math.hypot(p.x - p0.x, p.y - p0.y) for p in pel) * 100, 3),
        "root_moves_cm": round(max((j[0] - joints[0][0]).length for j in joints) * 100, 3),
        "seam_first_last_max_joint_cm": round(max((x - y).length for x, y in zip(joints[0], joints[-1])) * 100, 3),
        "lowest_point_cm": round(lowest * 100, 2), "top_cm": round(highest * 100, 1)}
arm.animation_data.action = None
for pb in arm.pose.bones:
    pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0); pb.scale = (1, 1, 1)
sc.frame_set(0)
dg = bpy.context.evaluated_depsgraph_get()
pts = [o.evaluated_get(dg).matrix_world @ v.co for o in bpy.data.objects if o.type == "MESH" for v in o.evaluated_get(dg).data.vertices]
out["rest_bounds_cm"] = {ax: [round(min(p[i] for p in pts) * 100, 1), round(max(p[i] for p in pts) * 100, 1)] for i, ax in enumerate("xyz")}
out["rest_pose_bone_heads_cm"] = {b: [round(x * 100, 1) for x in arm.data.bones[b].head_local] for b in ("root", "pelvis", "head", "hand_r", "stick_socket", "puck_socket", "head_socket")}
out.update(out0)
print("S4CHECK " + json.dumps(out))
if len(a) > 1:
    with open(a[1], "w") as fh:
        json.dump(out, fh, indent=1)
