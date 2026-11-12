# Turntable of a GLB/FBX (Workbench, texture colour, outline). blender -b --factory-startup --python glb_turn.py -- <in> <outdir> [n] [size]
import bpy, sys, os, math, mathutils
a = sys.argv[sys.argv.index("--") + 1:]
inp, out = a[0], a[1]; n = int(a[2]) if len(a) > 2 else 8; size = int(a[3]) if len(a) > 3 else 224
os.makedirs(out, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
if inp.lower().endswith(".glb"): bpy.ops.import_scene.gltf(filepath=inp)
else: bpy.ops.import_scene.fbx(filepath=inp)
ms = [o for o in bpy.data.objects if o.type == "MESH"]
dg = bpy.context.evaluated_depsgraph_get()
pts = []
for o in ms:
    for c in o.bound_box: pts.append(o.matrix_world @ mathutils.Vector(c))
lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)]); hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
print("BOUNDS", [round(x, 3) for x in lo], [round(x, 3) for x in hi])
tris = 0
for o in ms:
    o.data.calc_loop_triangles(); tris += len(o.data.loop_triangles)
print("TRIS", tris, "IMAGES", [(i.name, list(i.size)) for i in bpy.data.images])
sc = bpy.context.scene; sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading; sh.light = "STUDIO"; sh.color_type = "TEXTURE"; sh.show_object_outline = True
sh.object_outline_color = (0.02, 0.02, 0.04); sh.show_specular_highlight = False
sc.render.resolution_x = size; sc.render.resolution_y = int(size * 1.25)
sc.world = bpy.data.worlds.new("w"); sc.world.color = (0.035, 0.045, 0.075); sh.background_type = "WORLD"
sc.view_settings.view_transform = "Standard"
cd = bpy.data.cameras.new("c"); cd.type = "ORTHO"; cam = bpy.data.objects.new("c", cd); sc.collection.objects.link(cam); sc.camera = cam
ctr = (lo + hi) / 2; H = max(hi - lo) * 1.1; cd.ortho_scale = H
for i in range(n):
    y = 2 * math.pi * i / n
    # Rodin/glTF: up is Z after import, figure faces -Y
    cam.location = ctr + mathutils.Vector((math.sin(y) * 5, -math.cos(y) * 5, 0.4))
    cam.rotation_euler = (ctr - cam.location).to_track_quat("-Z", "Y").to_euler()
    sc.render.filepath = os.path.join(out, "f%02d.png" % i); bpy.ops.render.render(write_still=True)
