# Re-import SK_Skater_S5.fbx and measure every take from the evaluated file.
#   blender -b --factory-startup --python verify.py -- <fbx> <out.json>
import bpy, sys, os, json, math
import numpy as np
from mathutils import Vector

fbx, out = sys.argv[sys.argv.index("--") + 1:][:2]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=fbx, automatic_bone_orientation=False)
sc = bpy.context.scene
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
body = max(meshes, key=lambda o: len(o.data.vertices))
stick = next((o for o in meshes if o is not body), None)
dg = bpy.context.evaluated_depsgraph_get()

def co_of(o):
    ev = o.evaluated_get(dg); me = ev.to_mesh()
    c = np.zeros(len(me.vertices) * 3); me.vertices.foreach_get("co", c); ev.to_mesh_clear()
    c = c.reshape(-1, 3)
    mw = np.array(o.matrix_world)
    return c @ mw[:3, :3].T + mw[:3, 3]

def tris(o):
    o.data.calc_loop_triangles(); return len(o.data.loop_triangles)

res = {"file": os.path.basename(fbx), "bytes": os.path.getsize(fbx), "fps": sc.render.fps,
       "armature": arm.name, "armature_scale": [round(x, 5) for x in arm.scale],
       "armature_rot_deg": [round(math.degrees(x), 3) for x in arm.rotation_euler],
       "bone_count": len(arm.data.bones), "top_bones": [b.name for b in arm.data.bones if b.parent is None],
       "bones": [(b.name, b.parent.name if b.parent else None) for b in arm.data.bones],
       "meshes": {o.name: {"tris": tris(o), "verts": len(o.data.vertices), "uv": [u.name for u in o.data.uv_layers],
                           "materials": [s.material.name for s in o.material_slots if s.material]} for o in meshes},
       "images": [{"name": i.name, "size": list(i.size), "packed": bool(i.packed_file)} for i in bpy.data.images],
       "takes": {}}
hb = [b.name for b in arm.data.bones]
rootb = arm.data.bones["root"]
res["root_rest_matrix_local"] = [[round(x, 6) for x in row] for row in rootb.matrix_local]
res["root_rest_is_identity"] = max(abs(rootb.matrix_local[i][j] - (i == j)) for i in range(4) for j in range(4)) < 1e-5
res["armature_object_scale"] = [round(x, 5) for x in arm.matrix_world.to_scale()]
if arm.animation_data:
    arm.animation_data.action = None
for pb in arm.pose.bones:
    pb.matrix_basis.identity()
sc.frame_set(1)
rest = co_of(body)
res["rest_height_cm"] = round(float(rest[:, 2].max() - rest[:, 2].min()) * 100, 2)
res["rest_lowest_cm"] = round(float(rest[:, 2].min()) * 100, 3)
gn = {g.index: g.name for g in body.vertex_groups}
skate = np.array([v.index for v in body.data.vertices if any(gn[g.group] in ("foot_l", "foot_r") and g.weight > 0.5 for g in v.groups)])
res["skate_verts"] = len(skate)
res["material_slots"] = {o.name: [(s.material.name, [n.image.name for n in s.material.node_tree.nodes if n.type == "TEX_IMAGE" and n.image])
                                  if s.material and s.material.use_nodes else (s.material.name if s.material else None)
                                  for s in o.material_slots] for o in meshes}
for act in bpy.data.actions:
    arm.animation_data_create(); arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    P, low, slow, root_mv, sklow = [], [], [], 0.0, []
    pel = []
    for f in range(f0, f1 + 1):
        sc.frame_set(f)
        mw = arm.matrix_world
        P.append({n: (mw @ arm.pose.bones[n].head).copy() for n in hb})
        pel.append((mw @ arm.pose.bones["pelvis"].head).copy())
        root_mv = max(root_mv, (mw @ arm.pose.bones["root"].head).length)
        cb = co_of(body)
        low.append(cb[:, 2].min()); sklow.append(cb[skate, 2].min())
        if stick:
            slow.append(co_of(stick)[:, 2].min())
    seam = max((P[0][n] - P[-1][n]).length for n in hb)
    p0 = pel[0]
    res["takes"][act.name] = {
        "frames": f1 - f0 + 1,
        "pelvis_horizontal_max_cm": round(max(math.hypot(p.x - p0.x, p.y - p0.y) for p in pel) * 100, 4),
        "pelvis_xy_cm": [round(p0.x * 100, 2), round(p0.y * 100, 2)],
        "root_max_cm": round(root_mv * 100, 4),
        "seam_max_joint_cm": round(seam * 100, 4),
        "lowest_body_cm": [round(float(min(low)) * 100, 2), round(float(max(low)) * 100, 2)],
        "lowest_stick_cm": round(float(min(slow)) * 100, 2) if slow else None,
        "lowest_skate_cm": [round(float(min(sklow)) * 100, 2), round(float(max(sklow)) * 100, 2)],
        "root_keyed": any('"root"' in fc.data_path for fc in act.fcurves),
        "object_keyed": any(not fc.data_path.startswith("pose.") for fc in act.fcurves),
    }
with open(out, "w") as fh:
    json.dump(res, fh, indent=1)
print("VERIFY", json.dumps({k: v for k, v in res.items() if k not in ("bones",)}))
