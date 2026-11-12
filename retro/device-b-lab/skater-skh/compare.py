# Regression: re-import two exports of the same figure and compare every take frame by frame.
#   blender -b --factory-startup --python compare.py -- <a.fbx> <b.fbx> <out.json>
# Per take: max difference of any bone head (world), of any evaluated body vertex and of any stick vertex, in cm.
import bpy, sys, json
import numpy as np

a_fbx, b_fbx, out = sys.argv[sys.argv.index("--") + 1:][:3]

def sample(fbx):
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=fbx, automatic_bone_orientation=False)
    sc = bpy.context.scene
    arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
    ms = [o for o in bpy.data.objects if o.type == "MESH"]
    body = max(ms, key=lambda o: len(o.data.vertices)); stick = next((o for o in ms if o is not body), None)
    dg = bpy.context.evaluated_depsgraph_get()
    def co(o):
        ev = o.evaluated_get(dg); me = ev.to_mesh()
        c = np.zeros(len(me.vertices) * 3); me.vertices.foreach_get("co", c); ev.to_mesh_clear()
        mw = np.array(o.matrix_world); return c.reshape(-1, 3) @ mw[:3, :3].T + mw[:3, 3]
    res = {}
    names = sorted(b.name for b in arm.data.bones)
    for act in bpy.data.actions:
        arm.animation_data_create(); arm.animation_data.action = act
        f0, f1 = [int(round(x)) for x in act.frame_range]
        B, V, S = [], [], []
        for f in range(f0, f1 + 1):
            sc.frame_set(f)
            mw = arm.matrix_world
            B.append({n: np.array(mw @ arm.pose.bones[n].head) for n in names})
            V.append(co(body)); S.append(co(stick) if stick else None)
        res[act.name.split("|")[-1]] = (B, V, S)
    return res

A = sample(a_fbx); Bs = sample(b_fbx)
rep = {"a": a_fbx, "b": b_fbx, "takes": {}}
worst = {"bone": 0.0, "body": 0.0, "stick": 0.0}
for k in A:
    if k not in Bs:
        rep["takes"][k] = "missing in b"; continue
    (Ba, Va, Sa), (Bb, Vb, Sb) = A[k], Bs[k]
    if len(Ba) != len(Bb):
        rep["takes"][k] = {"frames": [len(Ba), len(Bb)]}; continue
    common = [n for n in Ba[0] if n in Bb[0]]
    db = max(float(np.linalg.norm(Ba[i][n] - Bb[i][n])) for i in range(len(Ba)) for n in common) * 100
    dv = max(float(np.abs(Va[i] - Vb[i]).max()) for i in range(len(Va))) * 100 if Va[0].shape == Vb[0].shape else None
    ds = max(float(np.abs(Sa[i] - Sb[i]).max()) for i in range(len(Sa))) * 100 if Sa[0] is not None and Sb[0] is not None else None
    rep["takes"][k] = {"frames": len(Ba), "bone_max_cm": round(db, 5), "body_vert_max_cm": None if dv is None else round(dv, 5),
                       "stick_vert_max_cm": None if ds is None else round(ds, 5)}
    worst["bone"] = max(worst["bone"], db); worst["body"] = max(worst["body"], dv or 0); worst["stick"] = max(worst["stick"], ds or 0)
rep["takes_a"] = len(A); rep["takes_b"] = len(Bs); rep["worst_cm"] = {k: round(v, 5) for k, v in worst.items()}
json.dump(rep, open(out, "w"), indent=1)
print("COMPARE", json.dumps({k: v for k, v in rep.items() if k != "takes"}))
for k, v in rep["takes"].items():
    print("CTAKE", k, json.dumps(v))
