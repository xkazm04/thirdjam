# Per-bone vertex statistics of a skinned mesh (world space): count, centroid, bounds of verts whose dominant group is the bone.
import bpy, sys
p = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=p, automatic_bone_orientation=False)
me = max([o for o in bpy.data.objects if o.type == "MESH"], key=lambda o: len(o.data.vertices))
names = {g.index: g.name for g in me.vertex_groups}
acc = {}
for v in me.data.vertices:
    if not v.groups:
        acc.setdefault("<none>", []).append(me.matrix_world @ v.co); continue
    g = max(v.groups, key=lambda g: g.weight)
    acc.setdefault(names[g.group], []).append(me.matrix_world @ v.co)
for n, ps in sorted(acc.items(), key=lambda kv: -max(p.z for p in kv[1])):
    c = sum(ps, ps[0] * 0) / len(ps)
    lo = [min(p[i] for p in ps) for i in range(3)]; hi = [max(p[i] for p in ps) for i in range(3)]
    print("W %-14s n=%5d c=(%.3f %.3f %.3f) x[%.3f %.3f] y[%.3f %.3f] z[%.3f %.3f]" % (n, len(ps), *c, lo[0], hi[0], lo[1], hi[1], lo[2], hi[2]))
