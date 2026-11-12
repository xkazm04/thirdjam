# Blender headless frame renderer for evaluation strips (Workbench, flat lighting, texture colour, outline:
# a cheap stand-in for the P2 cel look, not the game's shader).
#   blender -b --factory-startup --python tools/meshy/render.py -- <file.fbx|glb> <out_dir> <mode> [action_substring] [frames] [size]
# mode: turntable (spins the camera around the rest/first-frame pose) | motion (samples the action, camera follows the pelvis)
# Writes <out_dir>/f00.png ... ; compose a strip with tools/meshy/strip.py.
import bpy, sys, os, math, mathutils

a = sys.argv[sys.argv.index("--") + 1:]
path, out, mode = a[0], a[1], a[2]
act_sub = a[3] if len(a) > 3 and a[3] != "-" else None
n = int(a[4]) if len(a) > 4 else 8
size = int(a[5]) if len(a) > 5 else 192
os.makedirs(out, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
ext = os.path.splitext(path)[1].lower()
if ext == ".fbx":
    bpy.ops.import_scene.fbx(filepath=path)
else:
    bpy.ops.import_scene.gltf(filepath=path)

sc = bpy.context.scene
sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading
sh.light = "FLAT"
sh.color_type = "TEXTURE"
sh.show_object_outline = True
sh.object_outline_color = (0.05, 0.05, 0.08)
sh.show_shadows = False
sh.show_cavity = True
sh.cavity_type = "WORLD"
sc.render.resolution_x = size
sc.render.resolution_y = int(size * 1.25)
sc.render.film_transparent = False
if not sc.world:
    sc.world = bpy.data.worlds.new("w")
sc.world.color = (0.80, 0.86, 0.92)
sh.background_type = "WORLD"
sc.render.image_settings.file_format = "PNG"
sc.render.image_settings.color_mode = "RGB"

arm = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
# hide helper meshes (e.g. bone-shape spheres) by keeping only the largest mesh visible
if len(meshes) > 1:
    big = max(meshes, key=lambda m: len(m.data.polygons))
    for m in meshes:
        if m is not big:
            m.hide_render = True
    meshes = [big]

action = None
if arm and bpy.data.actions:
    acts = list(bpy.data.actions)
    if act_sub:
        acts = [x for x in acts if act_sub.lower() in x.name.lower()] or acts
    action = acts[0]
    arm.animation_data_create()
    arm.animation_data.action = action
    print("ACTION", action.name, tuple(action.frame_range))

def bbox():
    dg = bpy.context.evaluated_depsgraph_get()
    pts = []
    for m in meshes:
        e = m.evaluated_get(dg)
        pts += [e.matrix_world @ mathutils.Vector(c) for c in e.bound_box]
    lo = mathutils.Vector([min(p[i] for p in pts) for i in range(3)])
    hi = mathutils.Vector([max(p[i] for p in pts) for i in range(3)])
    return lo, hi

cam_data = bpy.data.cameras.new("cam")
cam_data.lens = 50
cam = bpy.data.objects.new("cam", cam_data)
sc.collection.objects.link(cam)
sc.camera = cam

def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg)
    # yaw 0 = looking at the figure's front (figure faces -Y after import)
    cam.location = target + mathutils.Vector((math.sin(y) * dist, -math.cos(y) * dist, height))
    d = target - cam.location
    cam.rotation_euler = d.to_track_quat("-Z", "Y").to_euler()

if action:
    f0, f1 = [int(round(x)) for x in action.frame_range]
else:
    f0, f1 = 1, 1
sc.frame_set(f0)
lo, hi = bbox()
h = max(hi.z - lo.z, 0.5)
dist = h * 2.0
for i in range(n):
    if mode == "turntable":
        sc.frame_set(f0)
        lo, hi = bbox()
        c = (lo + hi) / 2
        aim(c, 360.0 * i / n, dist, h * 0.15)
    else:
        f = f0 + round((f1 - f0) * i / max(n - 1, 1))
        sc.frame_set(f)
        lo, hi = bbox()
        c = (lo + hi) / 2
        c.z = lo.z + h * 0.5
        aim(c, 55, dist, h * 0.2)
    sc.render.filepath = os.path.join(out, "f%02d.png" % i)
    bpy.ops.render.render(write_still=True)
print("DONE", out, n)
