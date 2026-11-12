# Re-import SK_Skater_S5.fbx and measure every take from the evaluated file.
#   blender -b --factory-startup --python verify.py -- <fbx> <out.json>
import bpy, sys, os, json, math
import numpy as np
from mathutils import Vector

_args = sys.argv[sys.argv.index("--") + 1:]
fbx, out = _args[:2]
REP = json.load(open(_args[2])) if len(_args) > 2 else None    # retarget_report.json: per-frame S4 contact flags, grip weights, radii
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
_ft = {"l": ("foot_l", "ball_l", "LeftToe_end"), "r": ("foot_r", "ball_r", "RightToe_end")}
skate = np.array([v.index for v in body.data.vertices
                  if any(sum(g.weight for g in v.groups if gn[g.group] in _ft[s]) > 0.5 for s in "lr")])
def dom_of(name):
    return np.array([v.index for v in body.data.vertices if v.groups and gn[max(v.groups, key=lambda g: g.weight).group] == name])
mitt_idx = {s: dom_of("hand_" + s) for s in "lr"}
head_idx = dom_of("head")
HEAD_R = REP["head_r"] if REP else 0.28
MITT_R = REP["mitt_r"] if REP else 0.12
# stick shaft: PCA axis of the rest stick; shaft verts = middle of the projection range (the blade is at the far end)
shaft_idx = None
if stick:
    sr = co_of(stick); c0 = sr.mean(0); u0 = np.linalg.svd(sr - c0)[2][0]; t = (sr - c0) @ u0
    lo_t, hi_t = t.min(), t.max(); span = hi_t - lo_t
    shaft_idx = np.where((t > lo_t + 0.15 * span) & (t < hi_t - 0.25 * span))[0]
    if shaft_idx.size < 6:
        shaft_idx = np.arange(len(sr))
def line_fit(P):
    c = P.mean(0); u = np.linalg.svd(P - c)[2][0]; return c, u
def seg_dist(p, c, u, P):
    t = (P - c) @ u; t0, t1 = t.min() - 0.3, t.max() + 0.6        # the shaft runs on past the shaft sample toward the butt and blade
    tp = min(max(float((p - c) @ u), t0), t1)
    return float(np.linalg.norm(p - (c + u * tp)))
res["head_r_m"] = HEAD_R; res["mitt_r_m"] = MITT_R
blade_idx = None
if stick:
    _SM = arm.matrix_world @ arm.pose.bones["stick_socket"].matrix
    _R = np.array(_SM.to_3x3().normalized()); _t = np.array(_SM.translation)
    _loc = (co_of(stick) - _t) @ _R                      # stick verts in socket space (metres)
    blade_idx = np.where(_loc[:, 1] > 1.2)[0]
    res["blade_verts"] = int(blade_idx.size)
res["skate_verts"] = len(skate)
res["material_slots"] = {o.name: [(s.material.name, [n.image.name for n in s.material.node_tree.nodes if n.type == "TEX_IMAGE" and n.image])
                                  if s.material and s.material.use_nodes else (s.material.name if s.material else None)
                                  for s in o.material_slots] for o in meshes}
for act in bpy.data.actions:
    arm.animation_data_create(); arm.animation_data.action = act
    f0, f1 = [int(round(x)) for x in act.frame_range]
    cn = act.name.split("|")[-1]
    pf = REP["clips"].get(cn, {}).get("per_frame") if REP else None
    P, low, slow, root_mv, sklow = [], [], [], 0.0, []
    pel, gripd, mitt_head, stick_head, blade_err = [], [], [], [], []
    reach_m, reach_f = None, None
    for f in range(f0, f1 + 1):
        i = f - f0
        sc.frame_set(f)
        mw = arm.matrix_world
        P.append({n: (mw @ arm.pose.bones[n].head).copy() for n in hb})
        pel.append((mw @ arm.pose.bones["pelvis"].head).copy())
        root_mv = max(root_mv, (mw @ arm.pose.bones["root"].head).length)
        cb = co_of(body)
        low.append(cb[:, 2].min()); sklow.append(cb[skate, 2].min())
        hv = cb[head_idx]; hc = (hv.min(0) + hv.max(0)) / 2
        mc = {s: cb[mitt_idx[s]].mean(0) for s in "lr"}
        mitt_head.append(min(float(np.linalg.norm(mc[s] - hc)) for s in "lr"))
        if stick:
            cs = co_of(stick)
            slow.append(cs[:, 2].min())
            stick_head.append(float(np.linalg.norm(cs - hc, axis=1).min()))
            if REP and REP["clips"].get(cn, {}).get("measure_frame") is not None and f == REP["clips"][cn]["measure_frame"] - REP["clips"][cn]["per_frame"]["frame0"] + f0:
                bc = cs[blade_idx].mean(0); pp = np.array(mw @ arm.pose.bones["pelvis"].head)
                reach_m = float(np.hypot(bc[0] - pp[0], bc[1] - pp[1])); reach_f = f
            # shaft = stick_socket's Y axis (checked: every stick vertex lies within 2.4 cm of it); grip to blade heel ~1.35 m
            SM = mw @ arm.pose.bones["stick_socket"].matrix
            c = np.array(SM.translation); u = np.array(SM.to_3x3().normalized().col[1])
            if pf and i < len(pf["grip_w"]) and pf["grip_w"][i] > 0.5:
                tp = min(max(float((mc["l"] - c) @ u), -0.3), 1.35)
                gripd.append(float(np.linalg.norm(mc["l"] - (c + u * tp))))
            if pf and i < len(pf["s4_blade_z_m"]) and pf["s4_blade_z_m"][i] < 0.03:
                blade_err.append(abs(float(cs[:, 2].min()) - pf["s4_blade_z_m"][i]))
    seam = max((P[0][n] - P[-1][n]).length for n in hb)
    p0 = pel[0]
    on_ice = [sklow[i] for i in range(len(sklow)) if pf and i < len(pf["s4_skate_on_ice"]) and pf["s4_skate_on_ice"][i]]
    res["takes"][cn] = {
        "frames": f1 - f0 + 1,
        "pelvis_horizontal_max_cm": round(max(math.hypot(p.x - p0.x, p.y - p0.y) for p in pel) * 100, 4),
        "pelvis_xy_cm": [round(p0.x * 100, 2), round(p0.y * 100, 2)],
        "root_max_cm": round(root_mv * 100, 4),
        "seam_max_joint_cm": round(seam * 100, 4),
        "lowest_body_cm": [round(float(min(low)) * 100, 2), round(float(max(low)) * 100, 2)],
        "lowest_stick_cm": round(float(min(slow)) * 100, 2) if slow else None,
        "lowest_skate_cm": [round(float(min(sklow)) * 100, 2), round(float(max(sklow)) * 100, 2)],
        "skate_err_on_ice_cm": round(max([abs(x) for x in on_ice] or [0]) * 100, 3), "skate_on_ice_frames": len(on_ice),
        "blade_contact_err_cm": round(max(blade_err or [0]) * 100, 2), "blade_contact_frames": len(blade_err),
        "grip_frames": len(gripd),
        "left_mitt_to_shaft_cm": [round(min(gripd) * 100, 1), round(float(np.mean(gripd)) * 100, 1), round(max(gripd) * 100, 1)] if gripd else None,
        "mitt_to_head_min_cm": round(min(mitt_head) * 100, 1), "mitt_inside_head_frames": sum(1 for d in mitt_head if d < HEAD_R),
        "stick_to_head_min_cm": round(min(stick_head) * 100, 1) if stick_head else None,
        "stick_inside_head_frames": sum(1 for d in stick_head if d < HEAD_R),
        "reach_m": round(reach_m, 4) if reach_m is not None else None, "reach_frame": reach_f,
        "root_keyed": any('"root"' in fc.data_path for fc in act.fcurves),
        "object_keyed": any(not fc.data_path.startswith("pose.") for fc in act.fcurves),
    }
with open(out, "w") as fh:
    json.dump(res, fh, indent=1)
print("VERIFY", json.dumps({k: v for k, v in res.items() if k not in ("bones", "takes")}))
for k, v in res["takes"].items():
    print("TAKE", k, json.dumps(v))
