# S5 Workbench renderer. Keeps every mesh (stick included); texture colour where a mesh has an image, else material colour.
#   blender -b --factory-startup --python render_s5.py -- --in F.fbx --out DIR --mode MODE [--action NAME] [--n 8] [--size 192]
#        [--anim-fbx CLIP.fbx] [--rotz DEG] [--frames a,b,c] [--zoom 1.0] [--tz 0.82]
# MODE: turntable | front34 | side | back | front | game | legs (close side view of the legs)
# --anim-fbx: take the (single) action from an armature-only FBX and play it on the first file's armature (R + S2 clips).
# --rotz: extra yaw of the armature object (R as delivered by Meshy faces -Y; 90 turns it to +X like S4 and S5).
# Every camera is in absolute world space, so different files render at the same camera.
import bpy, sys, os, math, mathutils, argparse

ap = argparse.ArgumentParser()
ap.add_argument("--in", dest="inp"); ap.add_argument("--out"); ap.add_argument("--mode", default="front34")
ap.add_argument("--action"); ap.add_argument("--n", type=int, default=8); ap.add_argument("--size", type=int, default=192)
ap.add_argument("--anim-fbx"); ap.add_argument("--rotz", type=float, default=0.0); ap.add_argument("--frames")
ap.add_argument("--zoom", type=float, default=1.0); ap.add_argument("--tz", type=float, default=0.82)
ap.add_argument("--tint", default="blue")
A = ap.parse_args(sys.argv[sys.argv.index("--") + 1:])
os.makedirs(A.out, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
ext = os.path.splitext(A.inp)[1].lower()
if ext == ".blend":
    bpy.ops.wm.open_mainfile(filepath=A.inp)
else:
    bpy.ops.import_scene.fbx(filepath=A.inp, automatic_bone_orientation=False)
arm = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
meshes = [o for o in bpy.data.objects if o.type == "MESH"]

action = None
if A.anim_fbx:
    before = set(bpy.data.objects)
    before_acts = set(bpy.data.actions)
    bpy.ops.import_scene.fbx(filepath=A.anim_fbx, automatic_bone_orientation=False)
    new_acts = [a for a in bpy.data.actions if a not in before_acts]
    for o in set(bpy.data.objects) - before:
        bpy.data.objects.remove(o, do_unlink=True)
    action = new_acts[0]
elif A.action:
    acts = [x for x in bpy.data.actions if x.name.split("|")[-1] == A.action] or \
           [x for x in bpy.data.actions if A.action.lower() in x.name.lower()]
    if not acts:
        raise SystemExit("no action " + A.action)
    action = acts[0]
if arm:
    arm.animation_data_create()
    arm.animation_data.action = action
    if action is None:
        for pb in arm.pose.bones:
            pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0); pb.scale = (1, 1, 1)
    if A.rotz:   # through a parent empty: Meshy clips may key the armature object's own transform
        piv = bpy.data.objects.new("yaw", None); sc0 = bpy.context.scene; sc0.collection.objects.link(piv)
        arm.parent = piv
        piv.rotation_euler[2] = math.radians(A.rotz)
    if action is not None:
        print("OBJECT CURVES", sorted({fc.data_path for fc in action.fcurves if not fc.data_path.startswith("pose.")}))
print("ACTION", action.name if action else None, tuple(action.frame_range) if action else None)

sc = bpy.context.scene
sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading
sh.light = "STUDIO"; sh.color_type = "TEXTURE"; sh.show_specular_highlight = False
sh.show_object_outline = True; sh.object_outline_color = (0.02, 0.02, 0.04)
sh.show_shadows = False; sh.show_cavity = False
sc.display.render_aa = "8" if A.mode != "game" else "OFF"
sc.render.resolution_x = A.size; sc.render.resolution_y = int(A.size * 1.25)
if not sc.world:
    sc.world = bpy.data.worlds.new("w")
sc.world.color = (0.035, 0.045, 0.075)
sh.background_type = "WORLD"
sc.render.image_settings.file_format = "PNG"; sc.render.image_settings.color_mode = "RGB"
sc.view_settings.view_transform = "Standard"

ORANGE = (0.98, 0.42, 0.06)
for m in bpy.data.materials:
    if m.use_nodes and m.node_tree:
        b = m.node_tree.nodes.get("Principled BSDF")
        if b:
            m.diffuse_color = tuple(b.inputs["Base Color"].default_value)
    if A.tint == "orange" and m.name.split(".")[0] == "Jersey":
        m.diffuse_color = (*[((c + 0.055) / 1.055) ** 2.4 for c in ORANGE], 1.0)

bpy.ops.mesh.primitive_plane_add(size=40, location=(0, 0, 0))
ice = bpy.context.active_object
im = bpy.data.materials.new("Ice"); im.diffuse_color = (0.62, 0.70, 0.80, 1)
ice.data.materials.append(im)

cd = bpy.data.cameras.new("cam"); cd.lens = 50
cam = bpy.data.objects.new("cam", cd); sc.collection.objects.link(cam); sc.camera = cam

def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg)            # yaw 0 = in front of a figure that faces +X
    cam.location = target + mathutils.Vector((math.cos(y) * dist, math.sin(y) * dist, height))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()

f0, f1 = ([int(round(x)) for x in action.frame_range] if action else (1, 1))
H = 1.75
tgt = mathutils.Vector((0.05, 0, A.tz))
d = H * 1.75 / A.zoom
if A.mode == "game":
    cd.type = "ORTHO"; sc.render.resolution_x = A.size; sc.render.resolution_y = A.size
    cd.ortho_scale = A.size / (40.0 / 1.70)
frames = [int(x) for x in A.frames.split(",")] if A.frames else \
         [f0 + round((f1 - f0) * i / max(A.n - 1, 1)) for i in range(A.n)]
for i, f in enumerate(frames):
    sc.frame_set(f)
    m = A.mode
    if m == "turntable":
        sc.frame_set(f0); aim(tgt, 360.0 * i / len(frames), d, H * 0.2)
    elif m == "front34":
        aim(tgt, 40, d, H * 0.25)
    elif m == "front":
        aim(tgt, 0, d, H * 0.1)
    elif m == "side":
        aim(tgt, 90, d, H * 0.05)       # from the figure's left (+Y)
    elif m == "rside":
        aim(tgt, -90, d, H * 0.05)      # from the figure's right
    elif m == "back":
        aim(tgt, 180, d * 1.05, H * 0.35)
    elif m == "legs":
        aim(mathutils.Vector((0.05, 0, 0.3)), 90, 1.4, 0.05)
    elif m == "top":                     # straight down, orthographic, forward (+X) up, the skater's left on the image left
        cd.type = "ORTHO"; cd.ortho_scale = 3.6
        cam.location = (0.45, 0, 6.0); cam.rotation_euler = (0, 0, -math.pi / 2)
    elif m == "game":
        aim(tgt, 180, 6.0, 3.5)
    sc.render.filepath = os.path.join(A.out, "f%02d.png" % i)
    bpy.ops.render.render(write_still=True)
print("DONE", A.out, len(frames))
