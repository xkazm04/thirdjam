# Probe: print armature/mesh facts of one FBX (rest matrices in world space, object transforms, actions).
import bpy, sys, mathutils, math
p = sys.argv[sys.argv.index("--") + 1]
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=p, automatic_bone_orientation=False)
for o in bpy.data.objects:
    print("OBJ", o.name, o.type, "parent", o.parent.name if o.parent else None, "loc", tuple(round(v, 4) for v in o.location),
          "rot", tuple(round(math.degrees(v), 2) for v in o.rotation_euler), "scale", tuple(round(v, 4) for v in o.scale))
    if o.type == "MESH":
        me = o.data
        me.calc_loop_triangles()
        ws = [o.matrix_world @ v.co for v in me.vertices]
        lo = [min(w[i] for w in ws) for i in range(3)]; hi = [max(w[i] for w in ws) for i in range(3)]
        print("   tris", len(me.loop_triangles), "verts", len(me.vertices), "lo", [round(x, 3) for x in lo], "hi", [round(x, 3) for x in hi],
              "uv", [u.name for u in me.uv_layers], "mats", [s.material.name if s.material else None for s in o.material_slots],
              "vgroups", len(o.vertex_groups), "mods", [(m.type, getattr(m, 'object', None) and m.object.name) for m in o.modifiers])
for a in [o for o in bpy.data.objects if o.type == "ARMATURE"]:
    mw = a.matrix_world
    print("ARM", a.name, "bones", len(a.data.bones))
    for b in a.data.bones:
        h = mw @ b.head_local; t = mw @ b.tail_local
        print("  %-16s par=%-14s head=(%.3f %.3f %.3f) tail=(%.3f %.3f %.3f) len=%.3f" % (b.name, b.parent.name if b.parent else "-", *h, *t, (t - h).length))
    for act in bpy.data.actions:
        print("ACT", act.name, tuple(act.frame_range), len(act.fcurves))
print("FPS", bpy.context.scene.render.fps)
for im in bpy.data.images:
    print("IMG", im.name, tuple(im.size), im.filepath)
