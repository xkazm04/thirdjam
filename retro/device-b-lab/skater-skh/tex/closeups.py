# blender -b --factory-startup --python closeups.py -- <fbx> <outdir> [<texture png or ->] [mode: all|legs]
import bpy, sys, os, math, mathutils
a = sys.argv[sys.argv.index("--") + 1:]
fbx, out = a[0], a[1]; tex = a[2] if len(a) > 2 and a[2] != "-" else None; mode = a[3] if len(a) > 3 else "all"
os.makedirs(out, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=fbx)
ob = next(o for o in bpy.data.objects if o.type == "MESH")
if tex:
    im = bpy.data.images.load(tex, check_existing=False)
    for s in ob.material_slots:
        for n in s.material.node_tree.nodes:
            if n.type == "TEX_IMAGE" and "base" in (n.image.name.lower() if n.image else ""): n.image = im
pts = [ob.matrix_world @ mathutils.Vector(c) for c in ob.bound_box]
z0 = min(p.z for p in pts); z1 = max(p.z for p in pts); H = z1 - z0
sc = bpy.context.scene; sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading; sh.light = "FLAT"; sh.color_type = "TEXTURE"; sh.show_object_outline = False; sh.show_specular_highlight = False
sc.view_settings.view_transform = "Standard"
w = bpy.data.worlds.new("w"); w.color = (0.55, 0.6, 0.65); sc.world = w
cam = bpy.data.cameras.new("c"); co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
def shoot(name, zf, dist, az, res=(320, 400), lens=50):
    cam.lens = lens; sc.render.resolution_x, sc.render.resolution_y = res
    t = mathutils.Vector((0, 0, z0 + zf * H)); r = math.radians(az)
    co.location = t + mathutils.Vector((math.sin(r) * dist, -math.cos(r) * dist, 0))
    co.rotation_euler = (t - co.location).to_track_quat("-Z", "Y").to_euler()
    sc.render.filepath = os.path.join(out, name + ".png"); bpy.ops.render.render(write_still=True)
for az in range(0, 360, 45):
    if mode == "all":
        shoot("full%03d" % az, 0.5, H * 2.2, az)
    shoot("legs%03d" % az, 0.12, H * 0.75, az, res=(320, 320))
if mode == "all":
    for az in (0, 90, 180, 270):
        shoot("head%03d" % az, 0.86, H * 0.55, az, res=(320, 320))
        shoot("chest%03d" % az, 0.62, H * 0.75, az, res=(320, 320))
