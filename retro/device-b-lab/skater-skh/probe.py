import bpy, sys, math, numpy as np
from mathutils import Matrix, Vector
f = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=f, automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
ms = [o for o in bpy.data.objects if o.type == "MESH"]
print("PROBE meshes", [(o.name, len(o.data.vertices), [m.name for m in o.data.materials]) for o in ms], "arm", [round(x,3) for x in arm.matrix_world.to_euler()], [round(x,3) for x in arm.matrix_world.to_scale()])
print("PROBE images", [(i.name, list(i.size), i.filepath) for i in bpy.data.images])
m = ms[0]; mw = m.matrix_world
V = np.array([tuple(mw @ v.co) for v in m.data.vertices])
aw = arm.matrix_world
B = {b.name: aw @ b.head_local for b in arm.data.bones}
print("PROBE bounds", V.min(0).round(3).tolist(), V.max(0).round(3).tolist())
print("PROBE bones", {k: [round(x,3) for x in v] for k, v in B.items() if k in ("pelvis","thigh_l","calf_l","foot_l","ball_l","head","hand_l","lowerarm_l","upperarm_l","neck_01")})
gn = {g.index: g.name for g in m.vertex_groups}
dom = {}
for v in m.data.vertices:
    if v.groups:
        g = max(v.groups, key=lambda g: g.weight); dom.setdefault(gn[g.group], []).append(v.index)
up = 2 # z
for n in ("head", "hand_l", "hand_r", "foot_l", "calf_l", "ball_l"):
    idx = dom.get(n, [])
    if not idx: print("PROBE", n, "none"); continue
    P = V[idx]; c = (P.min(0) + P.max(0)) / 2
    print("PROBE", n, len(idx), "bbox", (P.max(0)-P.min(0)).round(3).tolist(), "centre", c.round(3).tolist(), "maxdist", round(float(np.linalg.norm(P - c, axis=1).max()),3), "zrange", round(float(P[:,2].min()),3), round(float(P[:,2].max()),3))
fz = B["foot_l"].z
low = [i for i in range(len(V)) if V[i][2] < fz + 0.0]
cl = [i for i in low if i in set(dom.get("calf_l", []) + dom.get("calf_r", []))]
print("PROBE below foot head z", round(fz,3), len(low), "calf-dominant", len(cl))
