# S4 scripted original skater: builds an articulated toy hockey skater from primitives (no downloads, no services).
#   blender -b --factory-startup --python build_skater.py -- [out_dir]
# Writes <out_dir>/s4_skater.blend (mesh + armature, rest pose, no actions) and <out_dir>/SM_S4_Stick.fbx.
# Axes (Blender): figure faces +X, left = +Y, up = +Z, metres, feet on z = 0. Every part is rigid: weight 1.0 to one bone.
# Provenance: original (made by this script).
import bpy, bmesh, math, os, sys
from mathutils import Vector, Matrix

ARGS = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
OUT = os.path.abspath(ARGS[0]) if ARGS else os.path.dirname(os.path.abspath(__file__))
os.makedirs(OUT, exist_ok=True)

bpy.ops.wm.read_factory_settings(use_empty=True)
scn = bpy.context.scene
scn.unit_settings.system = "METRIC"
scn.unit_settings.scale_length = 1.0
scn.render.fps = 60

# ---------------------------------------------------------------- proportions (metres; tune here)
P = dict(
    ankle=0.13, knee=0.49, hip=0.85, hip_y=0.115,
    pelvis=0.88, sp1=0.98, sp2=1.08, sp3=1.17, neck=1.30, head=1.37, head_top=1.62,
    sh_z=1.27, sh_y=0.215, upper=0.29, lower=0.26, arm_drop=50.0,  # A-pose: arms 50 deg below horizontal
    stick_len=1.40, stick_lie=50.0, stick_rest_tilt=62.0,   # grip to heel; blade flat on the ice at tilt = lie; the rest pose clears the ice
    head_scale=1.10,                                                # helmet + face + cage, about the head joint                           # grip to heel; blade lies flat at this tilt
)
d = math.radians(P["arm_drop"])
def arm_pts(side):
    s = 1 if side == "l" else -1
    sh = Vector((0, s * P["sh_y"], P["sh_z"]))
    dirv = Vector((0, s * math.cos(d), -math.sin(d)))
    el = sh + dirv * P["upper"]
    wr = el + dirv * P["lower"]
    hd = wr + dirv * 0.08
    return sh, el, wr, hd, dirv

# ---------------------------------------------------------------- materials (flat colours, no textures)
def srgb2lin(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
COLOURS = {  # sRGB
    "Jersey": (0.10, 0.26, 0.78),   # deep blue reference; the game tints it by TeamColour
    "Trim":   (0.95, 0.95, 0.93),
    "Helmet": (0.93, 0.93, 0.90),
    "Gloves": (0.10, 0.11, 0.15),
    "Skin":   (0.92, 0.70, 0.55),
    "Pads":   (0.12, 0.13, 0.18),
    "Skate":  (0.08, 0.08, 0.09),
    "Blade":  (0.78, 0.82, 0.86),
    "Stick":  (0.72, 0.52, 0.26),
}
MATS = {}
for name, c in COLOURS.items():
    m = bpy.data.materials.new(name)
    lin = tuple(srgb2lin(x) for x in c)
    m.diffuse_color = (*lin, 1.0)
    m.roughness = 0.8
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*lin, 1.0)
    bsdf.inputs["Roughness"].default_value = 0.8
    MATS[name] = m

# ---------------------------------------------------------------- armature (UE mannequin names where they apply)
sh_l, el_l, wr_l, hd_l, dir_l = arm_pts("l")
sh_r, el_r, wr_r, hd_r, dir_r = arm_pts("r")
grip_r = wr_r + dir_r * 0.065            # glove centre = stick grip
Rs0 = Matrix.Rotation(math.radians(-P["stick_rest_tilt"]), 3, "Y")   # rest stick: shaft tilted forward
BONES = [  # name, parent, head, tail
    ("root", None, (0, 0, 0), (0, 0.15, 0)),   # root lies along +Y with roll 0: identity rest matrix, so its FBX node has no rotation
    ("pelvis", "root", (0, 0, P["pelvis"]), (0, 0, P["sp1"])),
    ("spine_01", "pelvis", (0, 0, P["sp1"]), (0, 0, P["sp2"])),
    ("spine_02", "spine_01", (0, 0, P["sp2"]), (0, 0, P["sp3"])),
    ("spine_03", "spine_02", (0, 0, P["sp3"]), (0, 0, P["neck"])),
    ("neck_01", "spine_03", (0, 0, P["neck"]), (0, 0, P["head"])),
    ("head", "neck_01", (0, 0, P["head"]), (0, 0, P["head_top"])),
    ("head_socket", "head", (0, 0, 1.86), (0, 0, 1.94)),
    ("clavicle_l", "spine_03", (0, 0.04, 1.28), (0, 0.19, 1.285)),
    ("upperarm_l", "clavicle_l", tuple(sh_l), tuple(el_l)),
    ("lowerarm_l", "upperarm_l", tuple(el_l), tuple(wr_l)),
    ("hand_l", "lowerarm_l", tuple(wr_l), tuple(hd_l)),
    ("clavicle_r", "spine_03", (0, -0.04, 1.28), (0, -0.19, 1.285)),
    ("upperarm_r", "clavicle_r", tuple(sh_r), tuple(el_r)),
    ("lowerarm_r", "upperarm_r", tuple(el_r), tuple(wr_r)),
    ("hand_r", "lowerarm_r", tuple(wr_r), tuple(hd_r)),
    ("stick_socket", "hand_r", tuple(grip_r), tuple(grip_r + Rs0 @ Vector((0, 0, -0.25)))),
    ("thigh_l", "pelvis", (0, P["hip_y"], P["hip"]), (0, P["hip_y"], P["knee"])),
    ("calf_l", "thigh_l", (0, P["hip_y"], P["knee"]), (0, P["hip_y"], P["ankle"])),
    ("foot_l", "calf_l", (0, P["hip_y"], P["ankle"]), (0.16, P["hip_y"], 0.05)),
    ("thigh_r", "pelvis", (0, -P["hip_y"], P["hip"]), (0, -P["hip_y"], P["knee"])),
    ("calf_r", "thigh_r", (0, -P["hip_y"], P["knee"]), (0, -P["hip_y"], P["ankle"])),
    ("foot_r", "calf_r", (0, -P["hip_y"], P["ankle"]), (0.16, -P["hip_y"], 0.05)),
    ("puck_socket", "root", (0.78, 0.10, 0.0), (0.78, 0.10, 0.08)),
]
arm_data = bpy.data.armatures.new("Armature")
arm = bpy.data.objects.new("Armature", arm_data)   # named "Armature": the UE FBX importer drops a root null with this name
scn.collection.objects.link(arm)
bpy.context.view_layer.objects.active = arm
bpy.ops.object.mode_set(mode="EDIT")
for name, parent, h, t in BONES:
    eb = arm_data.edit_bones.new(name)
    eb.head, eb.tail = Vector(h), Vector(t)
    eb.roll = 0.0
    eb.use_connect = False
    eb.use_deform = name not in ("root", "head_socket", "puck_socket")
    if parent:
        eb.parent = arm_data.edit_bones[parent]
bpy.ops.object.mode_set(mode="OBJECT")

# ---------------------------------------------------------------- geometry helpers
PARTS = []

def finish(name, bm, M, mat, bone, bevel=None, solid=None, subd=0, smooth=40.0):
    me = bpy.data.meshes.new(name)
    bm.to_mesh(me)
    bm.free()
    ob = bpy.data.objects.new(name, me)
    scn.collection.objects.link(ob)
    if bevel:
        mo = ob.modifiers.new("bv", "BEVEL")
        mo.width, mo.segments = bevel
        mo.limit_method = "ANGLE"
    if solid:
        mo = ob.modifiers.new("so", "SOLIDIFY")
        mo.thickness = solid
        mo.offset = -1.0
    if subd:
        mo = ob.modifiers.new("sd", "SUBSURF")
        mo.levels = subd
    dg = bpy.context.evaluated_depsgraph_get()
    me2 = bpy.data.meshes.new_from_object(ob.evaluated_get(dg))
    ob.modifiers.clear()
    ob.data = me2
    bpy.data.meshes.remove(me)
    me2.name = name
    if bone == "head":   # toy head: scale every head part about the neck joint (HEAD_SCALE in P)
        piv = Vector((0, 0, P["head"]))
        M = Matrix.Translation(piv) @ Matrix.Scale(P["head_scale"], 4) @ Matrix.Translation(-piv) @ M
    me2.transform(M)
    me2.materials.append(MATS[mat])
    for p in me2.polygons:
        p.material_index = 0
        p.use_smooth = True
    try:
        me2.set_sharp_from_angle(angle=math.radians(smooth))
    except Exception:
        pass
    vg = ob.vertex_groups.new(name=bone)
    vg.add(list(range(len(me2.vertices))), 1.0, "REPLACE")
    PARTS.append(ob)
    return ob

def T(loc, rot=None, scale=None):
    M = Matrix.Translation(Vector(loc))
    if rot is not None:
        M = M @ (rot.to_4x4() if rot.__class__.__name__ == "Matrix" else rot)
    if scale is not None:
        M = M @ Matrix.Diagonal((*scale, 1.0))
    return M

def box(name, dims, loc, mat, bone, bevel=(0.03, 3), rot=None, taper=None):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        if taper and v.co.z > 0:
            v.co.x *= taper[0]
            v.co.y *= taper[1]
        v.co.x *= dims[0]; v.co.y *= dims[1]; v.co.z *= dims[2]
    return finish(name, bm, T(loc, rot), mat, bone, bevel=bevel)

def sphere(name, r, loc, mat, bone, scale=(1, 1, 1), seg=16, rings=10, cut=None, solid=None, rot=None):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=seg, v_segments=rings, radius=r)
    if cut:
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if cut(v.co)], context="VERTS")
    return finish(name, bm, T(loc, rot, scale), mat, bone, solid=solid, smooth=60)

def tube(name, a, b, r1, r2, mat, bone, seg=12, caps=True):
    a, b = Vector(a), Vector(b)
    L = (b - a).length
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=caps, cap_tris=False, segments=seg, radius1=r1, radius2=r2, depth=L)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(0, 0, L / 2))
    rot = Vector((0, 0, 1)).rotation_difference((b - a).normalized()).to_matrix()
    return finish(name, bm, T(a, rot), mat, bone, smooth=50)

def band(name, a, b, t0, t1, r, mat, bone, seg=12):
    a, b = Vector(a), Vector(b)
    return tube(name, a.lerp(b, t0), a.lerp(b, t1), r, r, mat, bone, seg=seg, caps=True)

def oriented_box(name, dims, centre, xaxis, zhint, mat, bone, bevel=(0.03, 3)):
    x = Vector(xaxis).normalized()
    y = Vector(zhint).cross(x).normalized()
    z = x.cross(y)
    R = Matrix((x, y, z)).transposed()
    return box(name, dims, centre, mat, bone, bevel=bevel, rot=R)

# ---------------------------------------------------------------- body
# pelvis: breezers (hockey pants)
box("breezers", (0.33, 0.46, 0.24), (0, 0, 0.83), "Pads", "pelvis", bevel=(0.08, 3))
# lower jersey + hem stripe
box("belly", (0.31, 0.47, 0.20), (0, 0, 0.985), "Jersey", "spine_01", bevel=(0.07, 3))
box("hem", (0.325, 0.485, 0.05), (0, 0, 0.925), "Trim", "spine_01", bevel=(0.03, 2))
# chest: wide at the shoulders (shoulder pads under the jersey)
box("chest", (0.33, 0.50, 0.34), (0, 0, 1.165), "Jersey", "spine_03", bevel=(0.09, 3), taper=(1.0, 1.12))
for s in (1, -1):
    sphere("shoulder", 0.135, (0.0, s * 0.245, 1.235), "Jersey", "spine_03", scale=(1.0, 0.9, 0.85), seg=12, rings=8)
tube("collar", (0, 0, 1.305), (0, 0, 1.345), 0.085, 0.08, "Trim", "spine_03")
# neck, face, helmet with cage
tube("neck", (0, 0, 1.27), (0, 0, 1.40), 0.07, 0.068, "Skin", "neck_01")
sphere("face", 0.16, (0.035, 0, 1.465), "Skin", "head", scale=(1.0, 0.95, 1.05))
hc = Vector((-0.01, 0, 1.50))
sphere("helmet", 0.205, hc, "Helmet", "head", scale=(1.10, 1.0, 0.95), seg=20, rings=12,
       cut=lambda c: (c.z < -0.025 and c.x > -0.03) or c.z < -0.14, solid=0.016)
for s in (1, -1):   # ear guards
    sphere("ear", 0.065, (0.02, s * 0.198, 1.435), "Helmet", "head", scale=(1.2, 0.5, 1.2), seg=12, rings=8)
for y in (-0.075, 0.0, 0.075):   # cage, vertical bars
    box("cage_v", (0.014, 0.014, 0.19), (0.215 - abs(y) * 0.3, y * 1.2, 1.415), "Blade", "head", bevel=None)
for z in (1.35, 1.41, 1.47):     # cage, horizontal bars
    box("cage_h", (0.014, 0.24, 0.014), (0.21, 0, z), "Blade", "head", bevel=None)
for s in (1, -1):
    box("cage_s", (0.15, 0.014, 0.014), (0.14, s * 0.125, 1.41), "Blade", "head", bevel=None)
box("chin", (0.07, 0.11, 0.035), (0.20, 0, 1.31), "Helmet", "head", bevel=(0.012, 2))
for s in (1, -1):   # face: two dark eye dots (Pads slot, near black), between the cage bars
    sphere("eye", 0.024, (0.188, s * 0.052, 1.488), "Pads", "head", scale=(0.55, 1.0, 1.3), seg=8, rings=6)
box("brow", (0.03, 0.30, 0.02), (0.195, 0, 1.535), "Pads", "head", bevel=(0.008, 1))   # visor line under the helmet brim
# arms
for side, (sh, el, wr, hd, dv) in (("l", arm_pts("l")), ("r", arm_pts("r"))):
    tube("upperarm", sh, el, 0.098, 0.088, "Jersey", "upperarm_" + side)
    sphere("shoulder_ball", 0.095, sh, "Jersey", "clavicle_" + side, seg=10, rings=7)   # toy ball joints, weighted to the parent
    sphere("elbow", 0.09, el, "Jersey", "upperarm_" + side, seg=10, rings=7)
    tube("forearm", el, wr, 0.088, 0.08, "Jersey", "lowerarm_" + side)
    band("sleeve_stripe", el, wr, 0.45, 0.62, 0.0905, "Trim", "lowerarm_" + side)
    tube("cuff", wr - dv * 0.05, wr + dv * 0.02, 0.092, 0.095, "Gloves", "hand_" + side)
    oriented_box("glove", (0.17, 0.15, 0.15), wr + dv * 0.08, dv, (1, 0, 0), "Gloves", "hand_" + side, bevel=(0.045, 3))
# legs
for side, s in (("l", 1), ("r", -1)):
    y = s * P["hip_y"]
    tube("thigh", (0, y, 0.84), (0, y, 0.565), 0.135, 0.112, "Pads", "thigh_" + side)
    sphere("hip_ball", 0.125, (0, y, 0.82), "Pads", "pelvis", seg=10, rings=7)
    sphere("knee_ball", 0.098, (0, y, 0.49), "Pads", "thigh_" + side, seg=10, rings=7)
    sphere("kneepad", 0.088, (0.04, y, 0.49), "Pads", "calf_" + side, scale=(0.85, 1.0, 1.15), seg=12, rings=8)
    tube("sock", (0, y, 0.555), (0, y, 0.17), 0.10, 0.078, "Jersey", "calf_" + side)
    band("sock_stripe", (0, y, 0.555), (0, y, 0.17), 0.42, 0.56, 0.0935, "Trim", "calf_" + side)
    tube("ankle", (0, y, 0.12), (0, y, 0.215), 0.07, 0.066, "Skate", "foot_" + side)
    box("boot", (0.27, 0.115, 0.13), (0.04, y, 0.125), "Skate", "foot_" + side, bevel=(0.04, 3))
    box("holder", (0.26, 0.03, 0.045), (0.04, y, 0.045), "Skate", "foot_" + side, bevel=(0.01, 1))
    box("blade", (0.31, 0.012, 0.026), (0.04, y, 0.013), "Blade", "foot_" + side, bevel=(0.012, 2))

# ---------------------------------------------------------------- stick (separate object, rigid on stick_socket)
STICK_PARTS_START = len(PARTS)
L = P["stick_len"]
Rb = Matrix.Rotation(math.radians(P["stick_lie"]), 3, "Y")   # blade relative to shaft (lie)
def stick_M(local_loc, local_rot=None):
    R = Rs0 @ (local_rot if local_rot is not None else Matrix.Identity(3))
    return Matrix.Translation(grip_r + Rs0 @ Vector(local_loc)) @ R.to_4x4()
def stick_box(name, dims, local_loc, local_rot, mat, bevel):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co.x *= dims[0]; v.co.y *= dims[1]; v.co.z *= dims[2]
    return finish(name, bm, stick_M(local_loc, local_rot), mat, "stick_socket", bevel=bevel)
stick_box("shaft", (0.044, 0.03, L + 0.08), (0, 0, -(L - 0.08) / 2), None, "Stick", (0.008, 1))
heel = Vector((0, 0, -L))
bd = Rb @ Vector((1, 0, 0))
bu = Rb @ Vector((0, 0, 1))
stick_box("stick_blade", (0.34, 0.022, 0.09), tuple(heel + bd * 0.14 + bu * 0.045), Rb, "Stick", (0.012, 2))
stick_box("knob", (0.045, 0.035, 0.05), (0, 0, 0.06), None, "Trim", (0.01, 1))

# ---------------------------------------------------------------- join, skin, normals
def join(objs, name):
    for o in bpy.context.view_layer.objects:
        o.select_set(False)
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.object.join()
    ob = bpy.context.view_layer.objects.active
    ob.name = name
    ob.data.name = name
    # merge identical material slots created by the join
    me = ob.data
    names = [m.name for m in me.materials]
    order = list(dict.fromkeys(names))
    remap = [order.index(n) for n in names]
    idx = [0] * len(me.polygons)
    me.polygons.foreach_get("material_index", idx)
    idx = [remap[i] for i in idx]
    me.materials.clear()
    for n in order:
        me.materials.append(bpy.data.materials[n])
    me.polygons.foreach_set("material_index", idx)
    me.update()
    return ob

body = join(PARTS[:STICK_PARTS_START], "SK_Skater_S4")
# UV0 = planar projection seen from behind, framing the jersey back (u 0..1 across y +0.2..-0.2, v 0..1 over z 0.98..1.28):
# a jersey material can draw NumberTens / NumberOnes there, masked to back-facing normals. No texture is shipped.
uvl = body.data.uv_layers.new(name="UVMap")
for loop in body.data.loops:
    co = body.data.vertices[loop.vertex_index].co
    uvl.data[loop.index].uv = (0.5 - co.y / 0.40, (co.z - 0.98) / 0.30)
stick = join(PARTS[STICK_PARTS_START:], "SK_Skater_S4_Stick")
for ob in (body, stick):
    ob.parent = arm
    mo = ob.modifiers.new("Armature", "ARMATURE")
    mo.object = arm
    for vg in ob.vertex_groups:
        pass

def tris(ob):
    ob.data.calc_loop_triangles()
    return len(ob.data.loop_triangles)
print("S4 TRIS body", tris(body), "stick", tris(stick), "total", tris(body) + tris(stick))
print("S4 BONES", len(arm_data.bones), [b.name for b in arm_data.bones])

# separate static stick for attaching to stick_socket in Unreal (pivot = grip, shaft along -Z, blade toe along +X when tilted 50 deg)
st2 = stick.copy(); st2.data = stick.data.copy(); scn.collection.objects.link(st2)
st2.parent = None; st2.modifiers.clear(); st2.vertex_groups.clear()
st2.data.transform(Matrix.Translation(-grip_r))
st2.data.transform((Rs0.inverted()).to_4x4())
st2.name = "SM_S4_Stick"
for o in bpy.context.view_layer.objects:
    o.select_set(False)
st2.select_set(True)
bpy.ops.export_scene.fbx(filepath=os.path.join(OUT, "SM_S4_Stick.fbx"), use_selection=True, object_types={"MESH"},
                         apply_unit_scale=True, apply_scale_options="FBX_SCALE_ALL", axis_forward="-Z", axis_up="Y",
                         mesh_smooth_type="FACE", bake_anim=False)
bpy.data.objects.remove(st2)

bpy.ops.wm.save_as_mainfile(filepath=os.path.join(OUT, "s4_skater.blend"))
print("S4 BUILD OK", OUT)
