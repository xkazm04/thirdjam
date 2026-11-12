# Largest one-frame displacement of the hands (hand_l, hand_r heads) and of the stick blade centre, per take, in cm.
# Main files repeat frame 1 as the last frame on loops, so the last step IS the loop wrap.
#   blender -b --factory-startup --python handjump.py -- <fbx> <out.json>
import bpy, sys, json
import numpy as np
fbx, out = sys.argv[sys.argv.index("--") + 1:][:2]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=fbx, automatic_bone_orientation=False)
sc = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
stick = next(o for o in bpy.data.objects if o.type == "MESH" and [g.name for g in o.vertex_groups] == ["stick_socket"])
dg = bpy.context.evaluated_depsgraph_get()
def co(o):
    ev = o.evaluated_get(dg); me = ev.to_mesh()
    c = np.zeros(len(me.vertices) * 3); me.vertices.foreach_get("co", c); ev.to_mesh_clear()
    mw = np.array(o.matrix_world); return c.reshape(-1, 3) @ mw[:3, :3].T + mw[:3, 3]
if arm.animation_data:
    arm.animation_data.action = None
for pb in arm.pose.bones:
    pb.matrix_basis.identity()
sc.frame_set(1)
SM = arm.matrix_world @ arm.pose.bones["stick_socket"].matrix
R = np.array(SM.to_3x3().normalized()); t = np.array(SM.translation)
blade = np.where(((co(stick) - t) @ R)[:, 1] > 1.2)[0]
res = {"file": fbx, "takes": {}}
for act in bpy.data.actions:
    cn = act.name.split("|")[-1]
    arm.animation_data_create(); arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    H, Bc = [], []
    for f in range(f0, f1 + 1):
        sc.frame_set(f); mw = arm.matrix_world
        H.append([np.array(mw @ arm.pose.bones[n].head) for n in ("hand_l", "hand_r")])
        Bc.append(co(stick)[blade].mean(0))
    H = np.array(H); Bc = np.array(Bc)
    dh = np.linalg.norm(H[1:] - H[:-1], axis=2).max(1) * 100
    db = np.linalg.norm(Bc[1:] - Bc[:-1], axis=1) * 100
    res["takes"][cn] = {"hand_max_cm": round(float(dh.max()), 1), "hand_max_step": int(dh.argmax()) + f0,
                        "blade_max_cm": round(float(db.max()), 1), "blade_max_step": int(db.argmax()) + f0,
                        "steps_over_15cm": int(((dh > 15) | (db > 15)).sum()), "steps": len(dh),
                        "wrap_hand_cm": round(float(dh[-1]), 1), "wrap_blade_cm": round(float(db[-1]), 1)}
json.dump(res, open(out, "w"), indent=1)
print("JUMP done", fbx)
