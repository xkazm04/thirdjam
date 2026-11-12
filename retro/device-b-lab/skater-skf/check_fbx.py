# Re-imports an exported SKF FBX and measures every take (s4's check_fbx.py plus: lowest skate point, lowest stick point, triangles).
#   blender -b --factory-startup --python check_fbx.py -- <file.fbx> [out.json]
# pelvis horizontal excursion (max over the clip), root movement, loop seam (max joint distance first vs last frame), lowest point of
# the skates (vertices weighted to foot_l / foot_r), of the stick mesh, and of everything, sampled every 2nd frame (and the last).
import bpy, sys, json, math

a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=a[0], automatic_bone_orientation=False)
sc = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
stick = next(o for o in meshes if o.name.endswith("_Stick"))
body = next(o for o in meshes if o is not stick)
def tris(ob):
    ob.data.calc_loop_triangles()
    return len(ob.data.loop_triangles)
gi = {g.index for g in body.vertex_groups if g.name in ("foot_l", "foot_r")}
skate_idx = [v.index for v in body.data.vertices if any(g.group in gi and g.weight > 0.5 for g in v.groups)]
def rnd(M): return [[round(x, 4) for x in row] for row in M]
out = {"file": a[0], "armature_object": arm.name, "armature_matrix_world": rnd(arm.matrix_world),
       "root_rest_matrix_local": rnd(arm.data.bones["root"].matrix_local), "unit_scale_length": sc.unit_settings.scale_length,
       "bone_count": len(arm.data.bones), "bones": [b.name for b in arm.data.bones], "root_bones": [b.name for b in arm.data.bones if not b.parent],
       "triangles": {"body": tris(body), "stick": tris(stick)}, "materials": sorted({m.name for o in meshes for m in o.data.materials}),
       "fps": sc.render.fps, "takes": {}}
arm.animation_data_create()
# second pass (twist continuity): looping takes; their wrap step (last frame -> first) is measured too
LOOPS = {"Idle_Faceoff", "Stride", "Glide", "Lean_Left", "Lean_Right", "Crossover_Left", "Crossover_Right", "Faceoff_Ready", "Goalie_Hold",
         "Shot_Charge", "Aim_Front", "Aim_Left", "Aim_Right", "Aim_FarReach"}
ROT_BONES = [b.name for b in arm.data.bones if b.name not in ("root", "head_socket", "puck_socket")]
from mathutils import Vector
def rot_step(Ma, Mb, bone):
    """frame-to-frame rotation of one bone (armature space): total angle and the twist about the bone's own (local Y) axis, degrees"""
    R = (Ma.transposed() @ Mb)            # in the bone's previous local frame
    q = R.to_quaternion()
    if q.w < 0:
        q.negate()
    ang = math.degrees(2 * math.acos(max(-1.0, min(1.0, q.w))))
    tw = math.degrees(2 * math.atan2(q.y, q.w))
    tw = (tw + 180) % 360 - 180
    return ang, tw
out["rot_over_45"] = {}
out["rot_over_60_count"] = {}
out["foot_pitch"] = {}
for act in sorted(bpy.data.actions, key=lambda x: x.name):
    arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    pel, joints = [], []
    low_all, low_skate, low_stick, top = 9.0, 9.0, 9.0, -9.0
    mats = []
    fp_max_dorsi, fp_max_plantar = -999.0, -999.0
    for f in range(f0, f1 + 1):
        sc.frame_set(f)
        mw = arm.matrix_world
        mats.append({b: arm.pose.bones[b].matrix.to_3x3().normalized() for b in ROT_BONES})
        for sd in ("l", "r"):    # foot pitch relative to the shin: + dorsiflexion (toes towards the shin), - plantar flexion
            pf, pc = arm.pose.bones["foot_" + sd], arm.pose.bones["calf_" + sd]
            fwd = (pf.matrix.to_3x3() @ arm.data.bones["foot_" + sd].matrix_local.to_3x3().transposed() @ Vector((1, 0, 0))).normalized()
            up = (pc.head - pc.tail).normalized()
            pitch = 90.0 - math.degrees(fwd.angle(up))
            fp_max_dorsi = max(fp_max_dorsi, pitch)
            fp_max_plantar = max(fp_max_plantar, -pitch)
        pel.append(mw @ arm.pose.bones["pelvis"].head)
        joints.append([mw @ pb.head for pb in arm.pose.bones])
        if (f - f0) % 2 == 0 or f == f1:
            dg = bpy.context.evaluated_depsgraph_get()
            eb = body.evaluated_get(dg); es = stick.evaluated_get(dg)
            bz = [(eb.matrix_world @ v.co).z for v in eb.data.vertices]
            sz = [(es.matrix_world @ v.co).z for v in es.data.vertices]
            low_skate = min(low_skate, min(bz[i] for i in skate_idx))
            low_stick = min(low_stick, min(sz))
            low_all = min(low_all, min(bz), min(sz))
            top = max(top, max(bz))
    p0 = pel[0]
    tname = act.name.split("|")[-1]
    steps = [(i, i + 1) for i in range(len(mats) - 1)]
    if tname in LOOPS:
        steps.append((len(mats) - 1, 0))       # the wrap (zero motion in the main file, one frame of motion in loopcut)
    over45, n60, worst = [], 0, (0.0, 0.0, "")
    for a_, b_ in steps:
        for bn in ROT_BONES:
            ang, tw = rot_step(mats[a_][bn], mats[b_][bn], bn)
            if ang > worst[0]:
                worst = (ang, tw, "%s f%d->%d" % (bn, a_ + f0, b_ + f0))
            if ang > 45:
                over45.append([bn, a_ + f0, b_ + f0, round(ang, 1), round(tw, 1)])
            if ang > 60:
                n60 += 1
    out["rot_over_45"][tname] = over45
    out["rot_over_60_count"][tname] = n60
    out["foot_pitch"][tname] = {"max_dorsiflexion_deg": round(fp_max_dorsi, 1), "max_plantar_flexion_deg": round(fp_max_plantar, 1)}
    out["takes"][tname] = {
        "max_step_deg": round(worst[0], 1), "max_step_twist_deg": round(worst[1], 1), "max_step_where": worst[2],
        "max_dorsiflexion_deg": round(fp_max_dorsi, 1), "max_plantar_flexion_deg": round(fp_max_plantar, 1),
        "frames": f1 - f0 + 1,
        "pelvis_horizontal_max_cm": round(max(math.hypot(p.x - p0.x, p.y - p0.y) for p in pel) * 100, 3),
        "root_moves_cm": round(max((j[0] - joints[0][0]).length for j in joints) * 100, 3),
        "seam_first_last_max_joint_cm": round(max((x - y).length for x, y in zip(joints[0], joints[-1])) * 100, 3),
        "lowest_skate_cm": round(low_skate * 100, 2), "lowest_stick_cm": round(low_stick * 100, 2),
        "lowest_point_cm": round(low_all * 100, 2), "top_cm": round(top * 100, 1)}
arm.animation_data.action = None
for pb in arm.pose.bones:
    pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0); pb.scale = (1, 1, 1)
sc.frame_set(0)
dg = bpy.context.evaluated_depsgraph_get()
pts = [o.evaluated_get(dg).matrix_world @ v.co for o in meshes for v in o.evaluated_get(dg).data.vertices]
out["rest_bounds_cm"] = {ax: [round(min(p[i] for p in pts) * 100, 1), round(max(p[i] for p in pts) * 100, 1)] for i, ax in enumerate("xyz")}
out["rest_pose_bone_heads_cm"] = {b: [round(x * 100, 1) for x in (arm.matrix_world @ arm.data.bones[b].head_local)]
                                  for b in ("root", "pelvis", "head", "hand_r", "stick_socket", "puck_socket", "head_socket")}
print("SKFCHECK " + json.dumps({k: v for k, v in out.items() if k != "takes"}))
if len(a) > 1:
    with open(a[1], "w") as fh:
        json.dump(out, fh, indent=1)
