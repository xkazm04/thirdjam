# SKF Workbench renderer: one Blender session renders many jobs from one file (FBX re-import, or a .blend).
#   blender -b --factory-startup --python render_skf.py -- <file.fbx|.blend> <out_dir> <style> <job> [<job> ...]
# job = name:mode:action:n:size[:tint[:frame]]   action "-" = rest pose; frame fixes one frame (n views or 1)
# mode: turntable (n views around, at the action's first frame or `frame`), motion (3/4 front-left), side (from the figure's left),
#       back (from directly behind, a little above), game (orthographic chase view from behind and 30 deg above, about 40 px per 1.70 m)
# Look per style: p2_cel studio light + ink outline (a stand-in for the cel shader); p3_clay matcap + cavity (ridges and valleys) so the
# clay irregularity and the thumb dents read; p4_broadcast studio light with specular + soft cavity + outline.
import bpy, sys, os, math, mathutils
a = sys.argv[sys.argv.index("--") + 1:]
path, out, style, jobs = os.path.abspath(a[0]), os.path.abspath(a[1]), a[2], a[3:]
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from skf_style import TEAM_SLOTS
os.makedirs(out, exist_ok=True)
ext = os.path.splitext(path)[1].lower()
if ext == ".blend":
    bpy.ops.wm.open_mainfile(filepath=path)
else:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=path, automatic_bone_orientation=False)
sc = bpy.context.scene
sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading
sh.color_type = "MATERIAL"
sh.show_shadows = False
if style == "p3_clay":
    sh.light = "MATCAP"
    sh.studio_light = "basic_1.exr"
    sh.show_cavity = True
    sh.cavity_type = "BOTH"
    sh.cavity_ridge_factor = 1.6
    sh.cavity_valley_factor = 1.8
    sh.curvature_ridge_factor = 1.4
    sh.curvature_valley_factor = 1.6
    sc.display.matcap_ssao_distance = 0.08
    sc.display.matcap_ssao_attenuation = 1.0
    sc.display.matcap_ssao_samples = 16
    sh.show_object_outline = False
    sh.show_specular_highlight = False
elif style == "p4_broadcast":
    sh.light = "STUDIO"
    sh.show_specular_highlight = True
    sh.show_cavity = True
    sh.cavity_type = "WORLD"
    sc.display.matcap_ssao_distance = 0.15
    sh.show_object_outline = True
    sh.object_outline_color = (0.05, 0.05, 0.07)
else:
    sh.light = "STUDIO"
    sh.show_specular_highlight = False
    sh.show_object_outline = True
    sh.object_outline_color = (0.02, 0.02, 0.04)
    sh.show_cavity = False
if not sc.world:
    sc.world = bpy.data.worlds.new("w")
sc.world.color = (0.035, 0.045, 0.075)
sh.background_type = "WORLD"
sc.render.image_settings.file_format = "PNG"
sc.render.image_settings.color_mode = "RGB"
sc.view_settings.view_transform = "Standard"

def lin(c):
    return tuple(((x + 0.055) / 1.055) ** 2.4 for x in c)
ORANGE = (0.98, 0.42, 0.06)
BASE = {}
for m in bpy.data.materials:
    base = None
    if m.use_nodes and m.node_tree:
        b = m.node_tree.nodes.get("Principled BSDF")
        if b:
            base = tuple(b.inputs["Base Color"].default_value)
            m.roughness = b.inputs["Roughness"].default_value
    if base:
        m.diffuse_color = base
    BASE[m.name] = tuple(m.diffuse_color)

def tint(kind):
    for m in bpy.data.materials:
        slot = m.name.split(".")[0]
        if kind == "orange" and slot in TEAM_SLOTS[style]:
            k = 1.0 if slot == "Jersey" else (0.75 if slot == "Helmet" else 0.55)
            o = lin(ORANGE)
            m.diffuse_color = (o[0] * k, o[1] * k, o[2] * k, 1.0)
        elif m.name in BASE:
            m.diffuse_color = BASE[m.name]

arm = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
bpy.ops.mesh.primitive_plane_add(size=40, location=(0, 0, 0))
ice = bpy.context.active_object
im = bpy.data.materials.new("Ice")
im.diffuse_color = (0.62, 0.70, 0.80, 1)
ice.data.materials.append(im)
BASE["Ice"] = tuple(im.diffuse_color)

cam_data = bpy.data.cameras.new("cam")
cam = bpy.data.objects.new("cam", cam_data)
sc.collection.objects.link(cam)
sc.camera = cam

def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg)
    cam.location = target + mathutils.Vector((math.cos(y) * dist, math.sin(y) * dist, height))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()

def set_action(name):
    if arm is None:
        return None
    arm.animation_data_create()
    if name == "-":
        arm.animation_data.action = None
        for pb in arm.pose.bones:
            pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0); pb.scale = (1, 1, 1)
        return None
    acts = [x for x in bpy.data.actions if x.name.split("|")[-1] == name] or [x for x in bpy.data.actions if name.lower() in x.name.lower()]
    if not acts:
        raise SystemExit("no action " + name)
    arm.animation_data.action = acts[0]
    return acts[0]

H = 1.80
P_H = 1.0
tgt = mathutils.Vector((0.05, 0, 0.85))
for job in jobs:
    parts = job.split(":")
    name, mode, act_name, n, size = parts[0], parts[1], parts[2], int(parts[3]), int(parts[4])
    tk = parts[5] if len(parts) > 5 else "blue"
    fixed = int(parts[6]) if len(parts) > 6 else None
    tint(tk)
    act = set_action(act_name)
    f0, f1 = ([int(round(x)) for x in act.frame_range] if act else (0, 0))
    od = os.path.join(out, name)
    os.makedirs(od, exist_ok=True)
    sc.render.resolution_x = size
    sc.render.resolution_y = int(size * 1.25)
    sc.display.render_aa = "8"
    cam_data.type = "PERSP"
    cam_data.lens = 50
    if mode == "game":
        cam_data.type = "ORTHO"
        sc.render.resolution_y = size
        sc.display.render_aa = "OFF"
        cam_data.ortho_scale = size / (40.0 / 1.70)
    for i in range(n):
        if mode == "close":            # close turntable on the chest and head (surface read)
            sc.frame_set(fixed if fixed is not None else f0)
            aim(mathutils.Vector((0.0, 0, 1.08 * P_H)), 360.0 * i / n + 20, 1.35, 0.25)
        elif mode == "turntable":
            sc.frame_set(fixed if fixed is not None else f0)
            aim(tgt, 360.0 * i / n, H * 1.75, H * 0.2)
        else:
            f = fixed if fixed is not None else f0 + round((f1 - f0) * i / max(n - 1, 1))
            sc.frame_set(f)
            if mode == "motion":
                aim(tgt, 40, H * 1.75, H * 0.25)
            elif mode == "side":
                aim(tgt, 90, H * 1.75, H * 0.1)
            elif mode == "back":
                aim(tgt, 180, H * 1.9, H * 0.6)
            elif mode == "front":
                aim(tgt, 0, H * 1.75, H * 0.15)
            elif mode == "game":
                aim(tgt, 180, 6.0, 3.5)
        sc.render.filepath = os.path.join(od, "f%02d.png" % i)
        bpy.ops.render.render(write_still=True)
    print("JOB", name, "frames", f0, f1)
print("DONE", out)
