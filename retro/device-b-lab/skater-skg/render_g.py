# SKG Workbench renderer for the goalie preview .blend files (figure + kit + goalie actions).
#   blender -b --factory-startup --python render_g.py -- <preview.blend> <out_dir> <mode> [args...]
# modes:
#   turn  <clip> <frame> [n=8] [size=200]          n views around the goalie
#   keys  <view front|game> <clip:frame,...> [size]  one image per clip key frame; front = the shooter's view, game = high and in front
#                                                    (the game camera behind the attacking player, looking down the rink at the goalie)
#   goal  <clip:frame,...> [size]                    front orthographic view in a 183 x 122 cm goal frame
#   mask  <clip:frame,...>                           white silhouette inside the goal opening (2 px per cm); prints COVER lines
#   dist  <clip:frame> <s4 anim blend> [tint list]   40 px tall figures from the game camera at 30 m: skater blue, goalie blue,
#                                                    goalie orange, skater orange
# Look: studio light, material colour, black outline (stand-in for the P2 cel look), night backdrop, ice plane.
import bpy, sys, os, math, mathutils
from mathutils import Vector

a = sys.argv[sys.argv.index("--") + 1:]
path, out, mode = os.path.abspath(a[0]), os.path.abspath(a[1]), a[2]
rest = a[3:]
os.makedirs(out, exist_ok=True)
bpy.ops.wm.open_mainfile(filepath=path)
sc = bpy.context.scene
style = os.path.basename(path).replace("goalie_", "").replace(".blend", "")
TEAM = {"p2_cel": ("Jersey",), "p3_clay": ("Jersey", "Helmet"), "p4_broadcast": ("Jersey", "Helmet", "Gloves", "Pads")}.get(style, ("Jersey",))
ORANGE = (0.98, 0.42, 0.06)
lin = lambda c: tuple(((x + 0.055) / 1.055) ** 2.4 for x in c)

sc.render.engine = "BLENDER_WORKBENCH"
sh = sc.display.shading
sh.light = "STUDIO"; sh.color_type = "MATERIAL"; sh.show_specular_highlight = False
sh.show_object_outline = True; sh.object_outline_color = (0.02, 0.02, 0.04)
sh.show_shadows = False; sh.show_cavity = False
sc.display.render_aa = "8"
if not sc.world:
    sc.world = bpy.data.worlds.new("w")
sc.world.color = (0.035, 0.045, 0.075)
sh.background_type = "WORLD"
sc.render.image_settings.file_format = "PNG"; sc.render.image_settings.color_mode = "RGB"
sc.view_settings.view_transform = "Standard"
for m in bpy.data.materials:
    if m.use_nodes and m.node_tree:
        b = m.node_tree.nodes.get("Principled BSDF")
        if b:
            m.diffuse_color = tuple(b.inputs["Base Color"].default_value)

arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
goalie_objs = [o for o in bpy.data.objects if o.type == "MESH"]
bpy.ops.mesh.primitive_plane_add(size=60, location=(0, 0, 0))
ice = bpy.context.active_object
im = bpy.data.materials.new("Ice"); im.diffuse_color = (0.62, 0.70, 0.80, 1); ice.data.materials.append(im)

cam_data = bpy.data.cameras.new("cam"); cam_data.lens = 50
cam = bpy.data.objects.new("cam", cam_data); sc.collection.objects.link(cam); sc.camera = cam


def aim(target, yaw_deg, dist, height):
    y = math.radians(yaw_deg)
    cam.location = target + Vector((math.cos(y) * dist, math.sin(y) * dist, height))
    cam.rotation_euler = (target - cam.location).to_track_quat("-Z", "Y").to_euler()


def pose(clip, frame, rig=arm):
    act = bpy.data.actions.get(clip)
    rig.animation_data_create()
    rig.animation_data.action = act
    sc.frame_set(int(frame))


def render(name):
    sc.render.filepath = os.path.join(out, name)
    bpy.ops.render.render(write_still=True)


def tint(names, colour, mats=None):
    for m in (mats or bpy.data.materials):
        if m.name.split(".")[0] in names:
            m.diffuse_color = (*lin(colour), 1.0)


def goal_frame():
    red = bpy.data.materials.new("GoalRed"); red.diffuse_color = (*lin((0.85, 0.08, 0.06)), 1)
    net = bpy.data.materials.new("Net"); net.diffuse_color = (*lin((0.80, 0.82, 0.86)), 1)
    objs = []
    def bx(dims, loc, mat):
        bpy.ops.mesh.primitive_cube_add(size=1, location=loc)
        o = bpy.context.active_object; o.scale = dims; o.data.materials.append(mat); objs.append(o)
    r = 0.025
    for s in (1, -1):
        bx((2 * r, 2 * r, 1.22 + r), (0, s * (0.915 + r), (1.22 + r) / 2), red)
    bx((2 * r, 1.83 + 4 * r, 2 * r), (0, 0, 1.22 + r), red)
    bx((0.02, 1.83, 1.22), (-1.0, 0, 0.61), net)              # back netting, 1 m behind the line
    return objs


GX = 0.40   # goalie pelvis 40 cm out from the goal line

if mode == "turn":
    clip, frame = rest[0], rest[1]
    n = int(rest[2]) if len(rest) > 2 else 8
    size = int(rest[3]) if len(rest) > 3 else 200
    sc.render.resolution_x, sc.render.resolution_y = size, int(size * 1.25)
    pose(clip, frame)
    for i in range(n):
        aim(Vector((0.1, 0, 0.62)), 360.0 * i / n, 3.0, 0.45)
        render("f%02d.png" % i)

elif mode == "keys":
    view = rest[0]
    items = [x.split(":") for x in rest[1].split(",")]
    size = int(rest[2]) if len(rest) > 2 else 160
    sc.render.resolution_x, sc.render.resolution_y = size, int(size * 1.15)
    if view == "game":
        cam_data.type = "ORTHO"; cam_data.ortho_scale = 2.4
    for i, (clip, frame) in enumerate(items):
        pose(clip, frame)
        if view == "front":
            aim(Vector((0.1, 0, 0.62)), 0, 3.4, 0.25)
        else:
            aim(Vector((0.1, 0, 0.62)), 0, 20.0, 7.3)       # about 20 deg down: the chase camera seen from the attacking end
        render("k%02d.png" % i)

elif mode == "goal":
    items = [x.split(":") for x in rest[0].split(",")]
    size = int(rest[1]) if len(rest) > 1 else 300
    goal_frame()
    arm.location.x = GX
    sc.render.resolution_x, sc.render.resolution_y = size, int(size * 0.80)
    cam_data.type = "ORTHO"; cam_data.ortho_scale = 2.4
    for i, (clip, frame) in enumerate(items):
        pose(clip, frame)
        aim(Vector((0, 0, 0.75)), 0, 8.0, 0.0)
        render("g%02d.png" % i)

elif mode == "mask":
    items = [x.split(":") for x in rest[0].split(",")]
    sh.color_type = "SINGLE"; sh.single_color = (1, 1, 1); sh.light = "FLAT"; sh.show_object_outline = False
    sc.display.render_aa = "OFF"
    sc.world.color = (0, 0, 0)
    ice.hide_render = True
    W, Hh = 366, 244
    sc.render.resolution_x, sc.render.resolution_y = W, Hh
    cam_data.type = "ORTHO"; cam_data.ortho_scale = 1.83
    arm.location.x = GX
    for i, (clip, frame) in enumerate(items):
        pose(clip, frame)
        cam.location = Vector((8.0, 0, 0.61)); cam.rotation_euler = (math.radians(90), 0, math.radians(90))
        fn = "m%02d.png" % i
        render(fn)
        img = bpy.data.images.load(os.path.join(out, fn))
        px = list(img.pixels)
        on = [[px[(r * W + c) * 4] > 0.5 for c in range(W)] for r in range(Hh)]     # row 0 = bottom (the ice)
        cov = sum(sum(r) for r in on) / float(W * Hh)
        # five-hole: largest run of uncovered columns in the band 0..15 cm above the ice within 40 cm of the centre
        band = range(0, 30)
        cols = [all(on[r][c] for r in range(0, 3)) for c in range(W)]
        best, run = 0, 0
        for c in range(W // 2 - 80, W // 2 + 80):
            open_ = not any(on[r][c] for r in band)
            run = run + 1 if open_ else 0
            best = max(best, run)
        low = sum(1 for c in range(W) for r in band if on[r][c]) / float(W * len(band))
        print("COVER %s:%s coverage %.4f low_band_0_15cm %.4f widest_open_slot_at_ice_cm %.1f" % (clip, frame, cov, low, best / 2.0))

elif mode == "dist":
    clip, frame = rest[0].split(":")
    s4 = rest[1]
    size = 64
    g_mats = {s for o in goalie_objs for s in o.data.materials}
    if s4.lower().endswith(".fbx"):          # lane SKF's figure for this style
        before_o, before_m, before_a = set(bpy.data.objects), set(bpy.data.materials), set(bpy.data.actions)
        bpy.ops.import_scene.fbx(filepath=s4, automatic_bone_orientation=False)
        sk_objs = [o for o in bpy.data.objects if o not in before_o]
        sk_act = next(x for x in bpy.data.actions if x not in before_a and x.name.split("|")[-1] == "Idle_Faceoff")
        for m in bpy.data.materials:
            if m not in before_m and m.use_nodes and m.node_tree and m.node_tree.nodes.get("Principled BSDF"):
                m.diffuse_color = tuple(m.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value)
    else:
        with bpy.data.libraries.load(s4) as (src, dst):
            dst.objects = [n for n in src.objects]
            dst.actions = ["Idle_Faceoff"]
        sk_objs = [o for o in dst.objects if o is not None]
        for o in sk_objs:
            sc.collection.objects.link(o)
        sk_act = dst.actions[0]
    sk_arm = next(o for o in sk_objs if o.type == "ARMATURE")
    sk_mats = {s for o in sk_objs if o.type == "MESH" for s in o.data.materials}
    sk_arm.animation_data_create(); sk_arm.animation_data.action = sk_act
    sc.render.resolution_x, sc.render.resolution_y = size, size
    sc.display.render_aa = "OFF"
    cam_data.type = "ORTHO"; cam_data.ortho_scale = size / (40.0 / 1.70)
    pose(clip, frame)
    # the chase camera behind an attacking player: 6 m behind and 3.5 m above the player, the goalie 30 m further on
    cam_pos = lambda tgt: tgt + Vector((36.0, 0, 3.5))
    for j, (who, colour) in enumerate((("skater", None), ("goalie", None), ("goalie", ORANGE), ("skater", ORANGE))):
        for m in bpy.data.materials:
            if m.use_nodes and m.node_tree and m.node_tree.nodes.get("Principled BSDF"):
                m.diffuse_color = tuple(m.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value)
        if colour:
            tint(TEAM if who == "goalie" else ("Jersey",), colour, g_mats if who == "goalie" else sk_mats)
        for o in sk_objs:
            o.hide_render = who != "skater"
        for o in goalie_objs:
            o.hide_render = who != "goalie"
        sk_arm.rotation_euler = (0, 0, math.radians(180))     # the skater skates away from the camera, the goalie faces it
        tgt = Vector((0, 0, 0.85))
        cam.location = cam_pos(tgt)
        cam.rotation_euler = (tgt - cam.location).to_track_quat("-Z", "Y").to_euler()
        sc.frame_set(int(frame))
        render("d%02d.png" % j)
print("DONE", out)
