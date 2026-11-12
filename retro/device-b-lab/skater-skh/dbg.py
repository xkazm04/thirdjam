import bpy, sys, numpy as np
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=sys.argv[-1], automatic_bone_orientation=False)
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
ms = [o for o in bpy.data.objects if o.type == "MESH"]
for o in ms: print("DBG", o.name, len(o.data.vertices), [g.name for g in o.vertex_groups][:5], o.parent.name if o.parent else None, [round(x,3) for x in o.matrix_world.to_scale()])
body = max(ms, key=lambda o: len(o.data.vertices)); stick = [o for o in ms if o is not body][0]
dg = bpy.context.evaluated_depsgraph_get()
def co(o):
    ev = o.evaluated_get(dg); me = ev.to_mesh(); c = np.array([tuple(v.co) for v in me.vertices]); ev.to_mesh_clear()
    mw = np.array(o.matrix_world); return c @ mw[:3,:3].T + mw[:3,3]
sr = co(stick); print("DBG stick rest bounds", sr.min(0).round(3), sr.max(0).round(3))
c0 = sr.mean(0); u0 = np.linalg.svd(sr - c0)[2][0]; t = (sr - c0) @ u0
print("DBG u0", u0.round(3), "t range", t.min().round(3), t.max().round(3), np.histogram(t, 10)[0])
gn = {g.index: g.name for g in body.vertex_groups}
hl = [v.index for v in body.data.vertices if v.groups and gn[max(v.groups, key=lambda g: g.weight).group] == "hand_l"]
hr = [v.index for v in body.data.vertices if v.groups and gn[max(v.groups, key=lambda g: g.weight).group] == "hand_r"]
b = co(body); print("DBG mitt l", b[hl].mean(0).round(3), "r", b[hr].mean(0).round(3))
pb = arm.pose.bones["stick_socket"]; M = arm.matrix_world @ pb.matrix
h = np.array(M.translation); y = np.array(M.to_3x3().normalized().col[1])
d = sr - h; tt = d @ y; perp = np.linalg.norm(d - np.outer(tt, y), axis=1)
print("DBG socket", h.round(3), "y", y.round(3))
for lo, hi in ((-9, -0.05), (-0.05, 0.3), (0.3, 1.0), (1.0, 1.3), (1.3, 9)):
    m = (tt > lo) & (tt < hi); print("DBG t in", lo, hi, int(m.sum()), "perp", perp[m].round(3)[:8] if m.any() else None)
