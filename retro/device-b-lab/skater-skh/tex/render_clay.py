# Clay-look renderer (EEVEE, matte). Cameras as skater-lab/s5/render_s5.py.
#   blender -b --factory-startup --python render_clay.py -- --in F.fbx --out ABS_DIR --mode front34|side|back|front|close|close_side
#       [--action NAME] [--n 8] [--frames a,b,c] [--normal N.png [--normal-dx]] [--basecolor B.png] [--size 192] [--zoom 0.85] [--tz 0.72]
#       [--engine EEVEE|CYCLES] [--strength 1.0]
# --normal-dx: the map is DirectX (green down, as written for UE); the green channel is flipped for Blender.
# Writes ABS_DIR/f00.png ... (absolute paths only: Blender resolves relative render paths against C:\).
import bpy, sys, os, math, mathutils, argparse
ap = argparse.ArgumentParser()
ap.add_argument("--in", dest="inp"); ap.add_argument("--out"); ap.add_argument("--mode", default="front34")
ap.add_argument("--action"); ap.add_argument("--n", type=int, default=8); ap.add_argument("--frames")
ap.add_argument("--normal"); ap.add_argument("--normal-dx", action="store_true"); ap.add_argument("--basecolor")
ap.add_argument("--size", type=int, default=192); ap.add_argument("--zoom", type=float, default=0.85); ap.add_argument("--tz", type=float, default=0.72)
ap.add_argument("--yaw", type=float, default=0.0); ap.add_argument("--engine", default="EEVEE"); ap.add_argument("--strength", type=float, default=1.0)
A = ap.parse_args(sys.argv[sys.argv.index("--") + 1:])
if not os.path.isabs(A.out): raise SystemExit("--out must be absolute")
os.makedirs(A.out, exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=A.inp, automatic_bone_orientation=False)
arm = next((o for o in bpy.data.objects if o.type == "ARMATURE"), None)
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
body = max(meshes, key=lambda o: len(o.data.vertices))
action = None
if A.action:
    acts = [x for x in bpy.data.actions if x.name.split("|")[-1] == A.action] or [x for x in bpy.data.actions if A.action.lower() in x.name.lower()]
    if not acts: raise SystemExit("no action " + A.action)
    action = acts[0]
if arm:
    arm.animation_data_create(); arm.animation_data.action = action
    if action is None:
        for pb in arm.pose.bones:
            pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.rotation_euler = (0, 0, 0); pb.scale = (1, 1, 1)
# ---- matte clay materials
def clay_body(mat):
    nt = mat.node_tree; img = None
    for n in nt.nodes:
        if n.type == "TEX_IMAGE" and n.image and img is None and "normal" not in n.image.name.lower(): img = n.image
    if A.basecolor: img = bpy.data.images.load(A.basecolor, check_existing=True)
    nt.nodes.clear()
    out = nt.nodes.new("ShaderNodeOutputMaterial"); b = nt.nodes.new("ShaderNodeBsdfPrincipled")
    b.inputs["Roughness"].default_value = 0.9; b.inputs["Specular IOR Level"].default_value = 0.25
    nt.links.new(b.outputs[0], out.inputs[0])
    if img:
        t = nt.nodes.new("ShaderNodeTexImage"); t.image = img; nt.links.new(t.outputs["Color"], b.inputs["Base Color"])
    if A.normal:
        ni = bpy.data.images.load(A.normal, check_existing=True); ni.colorspace_settings.name = "Non-Color"
        t = nt.nodes.new("ShaderNodeTexImage"); t.image = ni; src = t.outputs["Color"]
        if A.normal_dx:
            sp = nt.nodes.new("ShaderNodeSeparateColor"); cb = nt.nodes.new("ShaderNodeCombineColor"); inv = nt.nodes.new("ShaderNodeMath")
            inv.operation = "SUBTRACT"; inv.inputs[0].default_value = 1.0
            nt.links.new(src, sp.inputs[0]); nt.links.new(sp.outputs[0], cb.inputs[0]); nt.links.new(sp.outputs[1], inv.inputs[1])
            nt.links.new(inv.outputs[0], cb.inputs[1]); nt.links.new(sp.outputs[2], cb.inputs[2]); src = cb.outputs[0]
        nm = nt.nodes.new("ShaderNodeNormalMap"); nm.inputs["Strength"].default_value = A.strength
        nt.links.new(src, nm.inputs["Color"]); nt.links.new(nm.outputs[0], b.inputs["Normal"])
for o in meshes:
    for s in o.material_slots:
        m = s.material
        if not m: continue
        m.use_nodes = True
        if o is body: clay_body(m)
        else:
            b = next((n for n in m.node_tree.nodes if n.type == "BSDF_PRINCIPLED"), None)
            if b: b.inputs["Roughness"].default_value = 0.9
sc = bpy.context.scene
eng = {"EEVEE": "BLENDER_EEVEE_NEXT", "CYCLES": "CYCLES"}[A.engine.upper()]
try:
    sc.render.engine = eng
except TypeError:
    sc.render.engine = "BLENDER_EEVEE"
if eng == "CYCLES":
    sc.cycles.samples = 24; sc.cycles.use_denoising = False
else:
    sc.eevee.taa_render_samples = 32
sc.render.resolution_x = A.size; sc.render.resolution_y = int(A.size * 1.25)
sc.render.image_settings.file_format = "PNG"; sc.render.image_settings.color_mode = "RGB"
sc.view_settings.view_transform = "Standard"
w = bpy.data.worlds.new("w"); w.use_nodes = True; w.node_tree.nodes["Background"].inputs[0].default_value = (0.05, 0.06, 0.09, 1)
w.node_tree.nodes["Background"].inputs[1].default_value = 1.0; sc.world = w
bpy.ops.mesh.primitive_plane_add(size=40, location=(0, 0, 0)); ice = bpy.context.active_object
im = bpy.data.materials.new("Ice"); im.use_nodes = True; ib = im.node_tree.nodes["Principled BSDF"]
ib.inputs["Base Color"].default_value = (0.62, 0.70, 0.80, 1); ib.inputs["Roughness"].default_value = 0.6; ice.data.materials.append(im)
def light(name, typ, loc, energy, color, size):
    ld = bpy.data.lights.new(name, typ); ld.energy = energy; ld.color = color
    if typ == "AREA": ld.size = size
    lo = bpy.data.objects.new(name, ld); sc.collection.objects.link(lo); lo.location = loc
    lo.rotation_euler = (mathutils.Vector((0, 0, 0.9)) - mathutils.Vector(loc)).to_track_quat("-Z", "Y").to_euler()
light("key", "AREA", (2.5, 2.0, 3.5), 900, (1.0, 0.96, 0.9), 2.5)
light("fill", "AREA", (2.0, -3.0, 1.5), 250, (0.85, 0.9, 1.0), 3.0)
light("bounce", "AREA", (1.5, 0.0, 0.05), 180, (1.0, 0.75, 0.5), 3.0)      # warm bounce from the floor side
light("rim", "AREA", (-3.0, 0.5, 2.5), 300, (0.9, 0.9, 1.0), 2.0)
cd = bpy.data.cameras.new("cam"); cd.lens = 50; cam = bpy.data.objects.new("cam", cd); sc.collection.objects.link(cam); sc.camera = cam
def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg + A.yaw)
    cam.location = target + mathutils.Vector((math.cos(y) * dist, math.sin(y) * dist, height))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()
f0, f1 = ([int(round(x)) for x in action.frame_range] if action else (1, 1))
H = 1.75; tgt = mathutils.Vector((0.05, 0, A.tz)); d = H * 1.75 / A.zoom
frames = [int(x) for x in A.frames.split(",")] if A.frames else ([f0 + round((f1 - f0) * i / max(A.n - 1, 1)) for i in range(A.n)] if action else [1])
# --yaw: extra camera yaw; a raw Meshy FBX faces -Y, so use --yaw -90 for it (exports face +X: 0)
for i, f in enumerate(frames):
    sc.frame_set(f); m = A.mode
    if m == "front34": aim(tgt, 40, d, H * 0.25)
    elif m == "front": aim(tgt, 0, d, H * 0.1)
    elif m == "side": aim(tgt, 90, d, H * 0.05)
    elif m == "back": aim(tgt, 180, d * 1.05, H * 0.35)
    elif m == "close": aim(mathutils.Vector((0.0, 0, 1.25)), 30, 1.3, 0.1)
    elif m == "close_side": aim(mathutils.Vector((0.0, 0, 1.25)), 100, 1.3, 0.1)
    sc.render.filepath = os.path.join(A.out, "f%02d.png" % i)
    bpy.ops.render.render(write_still=True)
print("DONE", sc.render.engine, A.out, len(frames))
