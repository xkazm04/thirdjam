# S4 Workbench renderer (multi-take aware; keeps every mesh, so the separate stick is drawn).
#   blender -b --factory-startup --python render_s4.py -- <file.fbx|.glb|.blend> <out_dir> <mode> [action|-] [n] [size] [tint]
# mode: turntable  n views around the figure, posed at the action's first frame (or rest when action is "-")
#       motion     n frames sampled over the action, fixed 3/4 front-left camera (the figure does not travel)
#       side       like motion, camera from the figure's left side
#       back       like motion, camera behind and above (a close version of the game's chase view)
#       top        high view from behind (stick aim read)
#       game       n frames, camera behind and 30 deg above (the game's chase angle), orthographic, figure about 40 px tall
# tint: "blue" (default, the reference jersey) or "orange" (the other team, Jersey material recoloured).
# Look: Workbench studio light, material colour, black object outline: a cheap stand-in for the P2 cel look.
import bpy, sys, os, math, mathutils

a = sys.argv[sys.argv.index("--") + 1:]
path, out, mode = a[0], a[1], a[2]
act_sub = a[3] if len(a) > 3 and a[3] != "-" else None
n = int(a[4]) if len(a) > 4 else 8
size = int(a[5]) if len(a) > 5 else 192
tint = a[6] if len(a) > 6 else "blue"
os.makedirs(out, exist_ok=True)

ext = os.path.splitext(path)[1].lower()
if ext == ".blend":
    bpy.ops.wm.open_mainfile(filepath=path)
else:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    if ext == ".fbx":
        bpy.ops.import_scene.fbx(filepath=path, automatic_bone_orientation=False)
    else:
        bpy.ops.import_scene.gltf(filepath=path)

sc = bpy.context.scene
sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading
sh.light = "STUDIO"
sh.color_type = "MATERIAL"
sh.show_specular_highlight = False
sh.show_object_outline = True
sh.object_outline_color = (0.02, 0.02, 0.04)
sh.show_shadows = False
sh.show_cavity = False
sc.display.render_aa = "8" if mode != "game" else "OFF"
sc.render.resolution_x = size
sc.render.resolution_y = int(size * 1.25)
if not sc.world:
    sc.world = bpy.data.worlds.new("w")
sc.world.color = (0.035, 0.045, 0.075)   # night arena backdrop
sh.background_type = "WORLD"
sc.render.image_settings.file_format = "PNG"
sc.render.image_settings.color_mode = "RGB"
sc.view_settings.view_transform = "Standard"

# material colours: Workbench draws diffuse_color; FBX/GLB import fills the node base colour
ORANGE = (0.98, 0.42, 0.06)
for m in bpy.data.materials:
    base = None
    if m.use_nodes and m.node_tree:
        b = m.node_tree.nodes.get("Principled BSDF")
        if b:
            base = tuple(b.inputs["Base Color"].default_value)
    if base:
        m.diffuse_color = base
    if tint == "orange" and m.name.split(".")[0] == "Jersey":
        m.diffuse_color = (*[((c + 0.055) / 1.055) ** 2.4 for c in ORANGE], 1.0)

arm = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
meshes = [o for o in bpy.data.objects if o.type == "MESH"]

# ice
bpy.ops.mesh.primitive_plane_add(size=40, location=(0, 0, 0))
ice = bpy.context.active_object
im = bpy.data.materials.new("Ice")
im.diffuse_color = (0.62, 0.70, 0.80, 1)
ice.data.materials.append(im)

action = None
if arm and bpy.data.actions and act_sub:
    acts = list(bpy.data.actions)
    exact = [x for x in acts if x.name.split("|")[-1] == act_sub]
    acts = exact or [x for x in acts if act_sub.lower() in x.name.lower()]
    if not acts:
        raise SystemExit("no action " + act_sub)
    action = acts[0]
    arm.animation_data_create()
    arm.animation_data.action = action
    print("ACTION", action.name, tuple(action.frame_range))
elif arm and arm.animation_data:
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0)

cam_data = bpy.data.cameras.new("cam")
cam_data.lens = 50
cam = bpy.data.objects.new("cam", cam_data)
sc.collection.objects.link(cam)
sc.camera = cam

def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg)       # yaw 0 = in front of the figure (it faces +X)
    cam.location = target + mathutils.Vector((math.cos(y) * dist, math.sin(y) * dist, height))
    dvec = target - cam.location
    cam.rotation_euler = dvec.to_track_quat("-Z", "Y").to_euler()

f0, f1 = ([int(round(x)) for x in action.frame_range] if action else (0, 0))
H = 1.75
tgt = mathutils.Vector((0.05, 0, 0.82))
if mode == "game":
    cam_data.type = "ORTHO"
    sc.render.resolution_x = size
    sc.render.resolution_y = size
    cam_data.ortho_scale = size / (40.0 / 1.70)    # 40 px per 1.70 m
for i in range(n):
    if mode == "turntable":
        sc.frame_set(f0)
        aim(tgt, 360.0 * i / n, H * 1.75, H * 0.2)
    else:
        f = f0 + round((f1 - f0) * i / max(n - 1, 1))
        sc.frame_set(f)
        if mode == "motion":
            aim(tgt, 40, H * 1.75, H * 0.25)
        elif mode == "side":
            aim(tgt, 90, H * 1.75, H * 0.1)
        elif mode == "top":
            aim(tgt, 200, H * 1.1, H * 1.9)
        elif mode == "back":
            aim(tgt, 200, H * 1.9, H * 0.9)
        elif mode == "game":
            aim(tgt, 180, 6.0, 3.5)
    sc.render.filepath = os.path.join(out, "f%02d.png" % i)
    bpy.ops.render.render(write_still=True)
print("DONE", out, n)
