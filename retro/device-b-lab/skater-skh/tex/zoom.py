# blender ... -- <fbx> <out.png> <zfrac> <az> <distfrac> [tex]
import bpy, sys, math, mathutils
a = sys.argv[sys.argv.index("--") + 1:]
bpy.ops.wm.read_factory_settings(use_empty=True); bpy.ops.import_scene.fbx(filepath=a[0])
ob = next(o for o in bpy.data.objects if o.type == "MESH")
if len(a) > 5:
    im = bpy.data.images.load(a[5], check_existing=False)
    for n in ob.material_slots[0].material.node_tree.nodes:
        if n.type == "TEX_IMAGE" and n.image and "base" in n.image.name.lower(): n.image = im
pts = [ob.matrix_world @ mathutils.Vector(c) for c in ob.bound_box]; z0 = min(p.z for p in pts); H = max(p.z for p in pts) - z0
sc = bpy.context.scene; sc.render.engine = "BLENDER_WORKBENCH"; sh = sc.display.shading; sh.light = "FLAT"; sh.color_type = "TEXTURE"
sc.view_settings.view_transform = "Standard"; w = bpy.data.worlds.new("w"); w.color = (0.55, 0.6, 0.65); sc.world = w
cam = bpy.data.cameras.new("c"); co = bpy.data.objects.new("c", cam); sc.collection.objects.link(co); sc.camera = co
sc.render.resolution_x = sc.render.resolution_y = 480
t = mathutils.Vector((0, 0, z0 + float(a[2]) * H)); r = math.radians(float(a[3])); d = float(a[4]) * H
co.location = t + mathutils.Vector((math.sin(r) * d, -math.cos(r) * d, 0)); co.rotation_euler = (t - co.location).to_track_quat("-Z", "Y").to_euler()
sc.render.filepath = a[1]; bpy.ops.render.render(write_still=True)
