import bpy, sys, mathutils, numpy as np
from mathutils.interpolate import poly_3d_calc
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True); bpy.ops.import_scene.fbx(filepath=a[0])
ob = next(o for o in bpy.data.objects if o.type == "MESH")
dg = bpy.context.evaluated_depsgraph_get(); ev = ob.evaluated_get(dg); me = ev.to_mesh()
mw = ob.matrix_world; inv = mw.inverted()
pts = [mw @ mathutils.Vector(c) for c in ob.bound_box]; z0 = min(p.z for p in pts); H = max(p.z for p in pts) - z0
uvl = me.uv_layers.active.data; hits = []
for dz in np.linspace(0.03, 0.08, 11):
    for x in np.linspace(-0.07, 0.07, 15):
        o = inv @ mathutils.Vector((x, -1, z0 + 0.93 * H + dz)); d = (inv.to_3x3() @ mathutils.Vector((0, 1, 0))).normalized()
        ok, loc, n, fi = ev.ray_cast(o, d)
        if not ok: continue
        p = me.polygons[fi]; vs = [me.vertices[v].co for v in p.vertices]; w = poly_3d_calc(vs, loc)
        uv = sum((mathutils.Vector(uvl[l].uv) * wi for l, wi in zip(p.loop_indices, w)), mathutils.Vector((0, 0)))
        hits.append((int(uv.x * 2048), int((1 - uv.y) * 2048)))
xs = [h[0] for h in hits]; ys = [h[1] for h in hits]
import collections
print("RAY", len(hits), sorted(collections.Counter((x // 64 * 64, y // 64 * 64) for x, y in hits).items(), key=lambda t: -t[1])[:10])
