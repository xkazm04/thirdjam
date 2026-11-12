# SKF scripted skater family: one builder, three looks on one skeleton (same bone names, hierarchy, sockets, material slot names).
#   blender -b --factory-startup --python build_skater.py -- <style> [lab_dir]      style: p2_cel | p3_clay | p4_broadcast
# Writes <lab_dir>/work/<style>/skater.blend (mesh + armature, rest pose) and <lab_dir>/export/<style>/SM_Stick_<style>.fbx (cm).
# Axes (Blender): figure faces +X, left = +Y, up = +Z, metres, feet on z = 0. Every part is rigid: weight 1.0 to one bone.
# Provenance: original (made by this script; derived from skater-lab/s4/build_skater.py).
import bpy, bmesh, math, os, sys, random
from mathutils import Vector, Matrix, noise
from mathutils.bvhtree import BVHTree

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
from skf_style import STYLES, COLOURS

ARGS = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
STYLE = ARGS[0] if ARGS else "p2_cel"
LAB = os.path.abspath(ARGS[1]) if len(ARGS) > 1 else HERE
P = STYLES[STYLE]
SUF = os.environ.get("SKF_SUFFIX", "")   # second pass: SKF_SUFFIX=2 writes work2/ and export2/ (export/ is being imported)
WORK = os.path.join(LAB, "work" + SUF, STYLE)
EXP = os.path.join(LAB, "export" + SUF, STYLE)
os.makedirs(WORK, exist_ok=True)
os.makedirs(EXP, exist_ok=True)
CLAY = STYLE == "p3_clay"

bpy.ops.wm.read_factory_settings(use_empty=True)
scn = bpy.context.scene
scn.unit_settings.system = "METRIC"
scn.unit_settings.scale_length = 1.0
scn.render.fps = 60

d = math.radians(P["arm_drop"])
def arm_pts(side):
    s = 1 if side == "l" else -1
    sh = Vector((0, s * P["sh_y"], P["sh_z"]))
    dirv = Vector((0, s * math.cos(d), -math.sin(d)))
    el = sh + dirv * P["upper"]
    wr = el + dirv * P["lower"]
    hd = wr + dirv * 0.08
    return sh, el, wr, hd, dirv

# ---------------------------------------------------------------- materials (flat colours, no textures; same 9 slot names per style)
def srgb2lin(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4
MATS = {}
for name, c in COLOURS[STYLE].items():
    m = bpy.data.materials.new(name)
    lin = tuple(srgb2lin(x) for x in c)
    m.diffuse_color = (*lin, 1.0)
    m.roughness = 0.9 if CLAY else 0.8
    m.use_nodes = True
    bsdf = m.node_tree.nodes.get("Principled BSDF")
    bsdf.inputs["Base Color"].default_value = (*lin, 1.0)
    glossy = STYLE == "p4_broadcast" and name in ("Helmet", "Blade", "Skate")   # broadcast: shiny helmet, visor, boots; cloth matte
    m.roughness = 0.35 if glossy else (0.9 if CLAY else 0.8)
    bsdf.inputs["Roughness"].default_value = m.roughness
    MATS[name] = m

# ---------------------------------------------------------------- armature (identical names and hierarchy for every style)
sh_l, el_l, wr_l, hd_l, dir_l = arm_pts("l")
sh_r, el_r, wr_r, hd_r, dir_r = arm_pts("r")
grip_r = wr_r + dir_r * P["grip_off"]
Rs0 = Matrix.Rotation(math.radians(-P["stick_rest_tilt"]), 3, "Y")
BONES = [
    ("root", None, (0, 0, 0), (0, 0.15, 0)),   # identity rest matrix: its FBX node has no rotation
    ("pelvis", "root", (0, 0, P["pelvis"]), (0, 0, P["sp1"])),
    ("spine_01", "pelvis", (0, 0, P["sp1"]), (0, 0, P["sp2"])),
    ("spine_02", "spine_01", (0, 0, P["sp2"]), (0, 0, P["sp3"])),
    ("spine_03", "spine_02", (0, 0, P["sp3"]), (0, 0, P["neck"])),
    ("neck_01", "spine_03", (0, 0, P["neck"]), (0, 0, P["head"])),
    ("head", "neck_01", (0, 0, P["head"]), (0, 0, P["head_top"])),
    ("head_socket", "head", (0, 0, P["top"] + 0.13), (0, 0, P["top"] + 0.21)),
    ("clavicle_l", "spine_03", (0, 0.04, P["sh_z"] + 0.01), (0, P["sh_y"] - 0.025, P["sh_z"] + 0.015)),
    ("upperarm_l", "clavicle_l", tuple(sh_l), tuple(el_l)),
    ("lowerarm_l", "upperarm_l", tuple(el_l), tuple(wr_l)),
    ("hand_l", "lowerarm_l", tuple(wr_l), tuple(hd_l)),
    ("clavicle_r", "spine_03", (0, -0.04, P["sh_z"] + 0.01), (0, -(P["sh_y"] - 0.025), P["sh_z"] + 0.015)),
    ("upperarm_r", "clavicle_r", tuple(sh_r), tuple(el_r)),
    ("lowerarm_r", "upperarm_r", tuple(el_r), tuple(wr_r)),
    ("hand_r", "lowerarm_r", tuple(wr_r), tuple(hd_r)),
    ("stick_socket", "hand_r", tuple(grip_r), tuple(grip_r + Rs0 @ Vector((0, 0, -0.25)))),
    ("thigh_l", "pelvis", (0, P["hip_y"], P["hip"]), (0, P["hip_y"], P["knee"])),
    ("calf_l", "thigh_l", (0, P["hip_y"], P["knee"]), (0, P["hip_y"], P["ankle"])),
    ("foot_l", "calf_l", (0, P["hip_y"], P["ankle"]), (P["foot_fwd"], P["hip_y"], 0.05)),
    ("thigh_r", "pelvis", (0, -P["hip_y"], P["hip"]), (0, -P["hip_y"], P["knee"])),
    ("calf_r", "thigh_r", (0, -P["hip_y"], P["knee"]), (0, -P["hip_y"], P["ankle"])),
    ("foot_r", "calf_r", (0, -P["hip_y"], P["ankle"]), (P["foot_fwd"], -P["hip_y"], 0.05)),
    ("puck_socket", "root", (0.78, 0.10, 0.0), (0.78, 0.10, 0.08)),
]
arm_data = bpy.data.armatures.new("Armature")
arm = bpy.data.objects.new("Armature", arm_data)
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

# ---------------------------------------------------------------- clay surface (p3 only): seeded low-frequency displacement + thumb dents
RNG = random.Random(1729)
def clayify(me, amp_scale=1.0, dents=True):
    vs = me.vertices
    if len(vs) < 20:
        return
    lo = Vector((min(v.co.x for v in vs), min(v.co.y for v in vs), min(v.co.z for v in vs)))
    hi = Vector((max(v.co.x for v in vs), max(v.co.y for v in vs), max(v.co.z for v in vs)))
    ext = hi - lo
    size = min(ext.x, ext.y, ext.z) if min(ext) > 0.005 else sorted(ext)[1]
    amp = min(0.009, 0.06 * size) * amp_scale
    off = Vector((RNG.uniform(-90, 90), RNG.uniform(-90, 90), RNG.uniform(-90, 90)))
    me.update()
    nrm = [v.normal.copy() for v in vs]
    cos = [v.co.copy() for v in vs]
    disp = [0.0] * len(vs)
    for i, c in enumerate(cos):   # two octaves of lumpiness: hand-rolled, nothing is a perfect primitive
        disp[i] += amp * (noise.noise(c * 7.0 + off) + 0.45 * noise.noise(c * 17.0 + off * 1.3))
    if dents:
        area = sum(p.area for p in me.polygons)
        n = int(area / 0.007) + (1 if area > 0.004 else 0)
        R = max(0.012, min(0.034, 0.32 * size))
        depth = min(0.010, 0.28 * R) * amp_scale
        polys = list(me.polygons)
        weights = [p.area for p in polys]
        for _ in range(n):
            pc = RNG.choices(polys, weights=weights)[0].center.copy()
            rr = R * RNG.uniform(0.7, 1.25)
            for i, c in enumerate(cos):
                dd = (c - pc).length
                if dd < rr:
                    w = 1.0 - (dd / rr) ** 2
                    disp[i] -= depth * w * w     # a thumb press: smooth bowl, deepest in the middle
    for i, v in enumerate(vs):
        v.co = cos[i] + nrm[i] * disp[i]
    me.update()

# ---------------------------------------------------------------- geometry helpers
PARTS = []

def finish(name, bm, M, mat, bone, bevel=None, solid=None, subd=0, smooth=40.0, clay=1.0, dents=True):
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
    if bone == "head" and P["head_scale"] != 1.0:   # toy head: scale every head part about the neck joint
        piv = Vector((0, 0, P["head"]))
        M = Matrix.Translation(piv) @ Matrix.Scale(P["head_scale"], 4) @ Matrix.Translation(-piv) @ M
    me2.transform(M)
    if CLAY and clay > 0:
        clayify(me2, clay, dents)
    me2.materials.append(MATS[mat])
    for p in me2.polygons:
        p.material_index = 0
        p.use_smooth = True
    try:
        me2.set_sharp_from_angle(angle=math.radians(180.0 if CLAY else smooth))
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

def frame3(xaxis, zhint):
    x = Vector(xaxis).normalized()
    y = Vector(zhint).cross(x).normalized()
    z = x.cross(y)
    return Matrix((x, y, z)).transposed()

def box(name, dims, loc, mat, bone, bevel=(0.03, 3), rot=None, taper=None, **kw):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        if taper and v.co.z > 0:
            v.co.x *= taper[0]
            v.co.y *= taper[1]
        v.co.x *= dims[0]; v.co.y *= dims[1]; v.co.z *= dims[2]
    return finish(name, bm, T(loc, rot), mat, bone, bevel=bevel, **kw)

def sphere(name, r, loc, mat, bone, scale=(1, 1, 1), seg=16, rings=10, cut=None, solid=None, rot=None, **kw):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=seg, v_segments=rings, radius=r)
    if cut:
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if cut(v.co)], context="VERTS")
    return finish(name, bm, T(loc, rot, scale), mat, bone, solid=solid, smooth=60, **kw)

def blob(name, half, loc, mat, bone, e=0.6, ez=None, seg=20, rings=12, rot=None, taper=None, cut=None, solid=None, **kw):
    """Superellipsoid: a rounded box (e < 1) or an egg (e = 1); half = half extents; taper scales x,y linearly with height."""
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=seg, v_segments=rings, radius=1.0)
    if cut:
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if cut(v.co)], context="VERTS")
    f = lambda c, ee: math.copysign(abs(c) ** ee, c)
    for v in bm.verts:
        x, y, z = f(v.co.x, e), f(v.co.y, e), f(v.co.z, ez if ez else e)
        if taper:
            t = (z + 1) / 2
            x *= taper[0][0] + (taper[1][0] - taper[0][0]) * t
            y *= taper[0][1] + (taper[1][1] - taper[0][1]) * t
        v.co = Vector((x * half[0], y * half[1], z * half[2]))
    return finish(name, bm, T(loc, rot), mat, bone, solid=solid, smooth=60, **kw)

def tube(name, a, b, r1, r2, mat, bone, seg=12, caps=True, **kw):
    a, b = Vector(a), Vector(b)
    L = (b - a).length
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=caps, cap_tris=False, segments=seg, radius1=r1, radius2=r2, depth=L)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(0, 0, L / 2))
    rot = Vector((0, 0, 1)).rotation_difference((b - a).normalized()).to_matrix()
    return finish(name, bm, T(a, rot), mat, bone, smooth=50, **kw)

def band(name, a, b, t0, t1, r, mat, bone, seg=12, **kw):
    a, b = Vector(a), Vector(b)
    return tube(name, a.lerp(b, t0), a.lerp(b, t1), r, r, mat, bone, seg=seg, caps=True, **kw)

def oriented_box(name, dims, centre, xaxis, zhint, mat, bone, bevel=(0.03, 3), **kw):
    return box(name, dims, centre, mat, bone, bevel=bevel, rot=frame3(xaxis, zhint), **kw)

def capsule(name, a, b, r1, r2, mat, bone, seg=14, nlen=4, ncap=3, sq=(1.0, 1.0), **kw):
    """Rounded cylinder from a to b (hemispherical ends of radius r1 and r2); sq squashes the section (x, y)."""
    a, b = Vector(a), Vector(b)
    L = (b - a).length
    bm = bmesh.new()
    rings = []
    for k in range(ncap, 0, -1):          # bottom cap, from near the pole up to the equator
        th = math.pi / 2 * k / ncap
        rings.append((r1 * math.cos(th), -r1 * math.sin(th)))
    for k in range(nlen + 1):
        t = k / nlen
        rings.append((r1 + (r2 - r1) * t, L * t))
    for k in range(1, ncap + 1):
        th = math.pi / 2 * k / ncap
        rings.append((r2 * math.cos(th), L + r2 * math.sin(th)))
    rings = [rg for rg in rings if rg[0] > 1e-5]
    vr = []
    for r, z in rings:
        vr.append([bm.verts.new((r * math.cos(2 * math.pi * i / seg) * sq[0], r * math.sin(2 * math.pi * i / seg) * sq[1], z)) for i in range(seg)])
    for i in range(len(vr) - 1):
        for j in range(seg):
            bm.faces.new((vr[i][j], vr[i][(j + 1) % seg], vr[i + 1][(j + 1) % seg], vr[i + 1][j]))
    pb = bm.verts.new((0, 0, -r1)); pt = bm.verts.new((0, 0, L + r2))
    for j in range(seg):
        bm.faces.new((pb, vr[0][(j + 1) % seg], vr[0][j]))
        bm.faces.new((pt, vr[-1][j], vr[-1][(j + 1) % seg]))
    rot = Vector((0, 0, 1)).rotation_difference((b - a).normalized()).to_matrix()
    return finish(name, bm, T(a, rot), mat, bone, smooth=60, **kw)

def sweep(name, pts, r, mat, bone, seg=6, closed=False, up=None, flat=1.0, **kw):
    """A round cord along a polyline (world points): seams, rolls, coils, straps."""
    pts = [Vector(p) for p in pts]
    n = len(pts)
    bm = bmesh.new()
    rings = []
    u = None
    for i, p in enumerate(pts):
        if closed:
            t = pts[(i + 1) % n] - pts[i - 1]
        else:
            t = pts[min(i + 1, n - 1)] - pts[max(i - 1, 0)]
        t.normalize()
        if up is not None:
            u = (Vector(up) - t * Vector(up).dot(t)).normalized()
        elif u is None:
            u = t.orthogonal().normalized()
        else:
            u = (u - t * u.dot(t)).normalized()
        v = t.cross(u)
        rings.append([bm.verts.new(p + (u * math.cos(2 * math.pi * j / seg) * flat + v * math.sin(2 * math.pi * j / seg)) * r) for j in range(seg)])
    for i in range(n - 1 + (1 if closed else 0)):
        A, B = rings[i], rings[(i + 1) % n]
        for j in range(seg):
            bm.faces.new((A[j], A[(j + 1) % seg], B[(j + 1) % seg], B[j]))
    if not closed:
        bm.faces.new(list(reversed(rings[0])))
        bm.faces.new(rings[-1])
    return finish(name, bm, Matrix.Identity(4), mat, bone, smooth=70, **kw)

def surface_hit(ob, origin, direction):
    me = ob.data
    bvh = BVHTree.FromPolygons([v.co for v in me.vertices], [p.vertices[:] for p in me.polygons])
    loc, nrm, idx, dist = bvh.ray_cast(Vector(origin), Vector(direction).normalized())
    return (loc, nrm) if loc is not None else (None, None)

def ray_from_axis(ob, z, ang, cx=0.0):
    """Hit point and normal on a part's surface, shooting outwards from the vertical axis at height z, angle ang (0 = +X)."""
    dvec = Vector((math.cos(ang), math.sin(ang), 0))
    return surface_hit(ob, (cx, 0, z), dvec)

# ---------------------------------------------------------------- shared stick frame
L = P["stick_len"]
Rb = Matrix.Rotation(math.radians(P["stick_lie"]), 3, "Y")   # blade relative to shaft (lie)
def stick_M(local_loc, local_rot=None):
    R = Rs0 @ (local_rot if local_rot is not None else Matrix.Identity(3))
    return Matrix.Translation(grip_r + Rs0 @ Vector(local_loc)) @ R.to_4x4()
def stick_box(name, dims, local_loc, local_rot, mat, bevel, **kw):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        v.co.x *= dims[0]; v.co.y *= dims[1]; v.co.z *= dims[2]
    return finish(name, bm, stick_M(local_loc, local_rot), mat, "stick_socket", bevel=bevel, **kw)
heel = Vector((0, 0, -L))
bd = Rb @ Vector((1, 0, 0))
bu = Rb @ Vector((0, 0, 1))
BL = P["stick_blade"]

# ================================================================= P2 painted cel (the s4 figure + face, mittens, helmet)
def build_p2():
    box("breezers", (0.33, 0.46, 0.24), (0, 0, 0.83), "Pads", "pelvis", bevel=(0.08, 3))
    box("belly", (0.31, 0.47, 0.20), (0, 0, 0.985), "Jersey", "spine_01", bevel=(0.07, 3))
    box("hem", (0.325, 0.485, 0.05), (0, 0, 0.925), "Trim", "spine_01", bevel=(0.03, 2))
    box("chest", (0.33, 0.50, 0.34), (0, 0, 1.165), "Jersey", "spine_03", bevel=(0.09, 3), taper=(1.0, 1.12))
    for s in (1, -1):
        sphere("shoulder", 0.135, (0.0, s * 0.245, 1.235), "Jersey", "spine_03", scale=(1.0, 0.9, 0.85), seg=12, rings=8)
    tube("collar", (0, 0, 1.305), (0, 0, 1.345), 0.085, 0.08, "Trim", "spine_03")
    tube("neck", (0, 0, 1.27), (0, 0, 1.40), 0.07, 0.068, "Skin", "neck_01")
    # face (pre head-scale coordinates; scaled 1.10 about the neck joint): round cheeks, a button nose, big eyes, brows, a smile
    sphere("face", 0.152, (0.03, 0, 1.455), "Skin", "head", scale=(1.0, 0.98, 1.04), seg=18, rings=12)
    sphere("nose", 0.03, (0.178, 0, 1.448), "Skin", "head", scale=(0.9, 1.0, 0.85), seg=10, rings=6)
    for s in (1, -1):
        sphere("eye_white", 0.034, (0.152, s * 0.056, 1.487), "Trim", "head", scale=(0.55, 0.85, 1.15), seg=10, rings=7)
        sphere("pupil", 0.019, (0.170, s * 0.052, 1.484), "Pads", "head", scale=(0.5, 0.9, 1.2), seg=8, rings=6)
        oriented_box("brow", (0.016, 0.05, 0.012), (0.160, s * 0.06, 1.532), (0, s * 0.18, 1), (1, 0, 0), "Pads", "head", bevel=(0.005, 1))   # raised outer ends: friendly, not angry
    sweep("smile", [Vector((0.153 + 0.012 * (1 - abs(t)), 0.055 * t, 1.385 + 0.016 * t * t)) for t in (-1, -0.5, 0, 0.5, 1)], 0.0095,
          "Pads", "head", seg=5)
    # helmet: shell opened higher at the front (face reads), a raised centre ridge, ear guards, chin strap
    hc = Vector((-0.012, 0, 1.50))
    sphere("helmet", 0.205, hc, "Helmet", "head", scale=(1.10, 1.0, 0.95), seg=20, rings=12,
           cut=lambda c: (c.z < 0.045 and c.x > 0.02) or c.z < -0.14, solid=0.016)
    sweep("helmet_ridge", [hc + Vector((0.205 * 1.1 * math.cos(a) * 0.98, 0, 0.205 * 0.95 * math.sin(a) * 0.98)) for a in
                           [math.radians(x) for x in (25, 50, 75, 100, 125, 150, 170)]], 0.011, "Helmet", "head", seg=8, flat=3.2, up=(0, 1, 0))   # low wide ridge, not a fin
    for s in (1, -1):
        sphere("ear", 0.066, (0.0, s * 0.198, 1.44), "Helmet", "head", scale=(1.2, 0.5, 1.2), seg=12, rings=8)
    # cage: an open frame below the eyes (two horizontal bars, side bars, one short centre bar)
    for z in (1.345, 1.415):
        box("cage_h", (0.014, 0.26, 0.014), (0.208, 0, z), "Blade", "head", bevel=None)
    for y in (-0.105, 0.105):
        box("cage_v", (0.014, 0.014, 0.20), (0.195, y, 1.42), "Blade", "head", bevel=None)
    box("cage_c", (0.014, 0.014, 0.085), (0.212, 0, 1.38), "Blade", "head", bevel=None)
    for s in (1, -1):
        box("cage_s", (0.13, 0.014, 0.014), (0.13, s * 0.125, 1.495), "Blade", "head", bevel=None)
    box("chin", (0.07, 0.11, 0.035), (0.20, 0, 1.31), "Helmet", "head", bevel=(0.012, 2))
    # arms; mitten gloves with a thumb
    for side, (sh, el, wr, hd, dv) in (("l", arm_pts("l")), ("r", arm_pts("r"))):
        s = 1 if side == "l" else -1
        tube("upperarm", sh, el, 0.098, 0.088, "Jersey", "upperarm_" + side)
        sphere("shoulder_ball", 0.095, sh, "Jersey", "clavicle_" + side, seg=10, rings=7)
        sphere("elbow", 0.09, el, "Jersey", "upperarm_" + side, seg=10, rings=7)
        tube("forearm", el, wr, 0.088, 0.08, "Jersey", "lowerarm_" + side)
        band("sleeve_stripe", el, wr, 0.45, 0.62, 0.0905, "Trim", "lowerarm_" + side)
        tube("cuff", wr - dv * 0.05, wr + dv * 0.035, 0.092, 0.10, "Gloves", "hand_" + side, seg=12)
        c = wr + dv * 0.10
        blob("mitten", (0.085, 0.07, 0.078), c, "Gloves", "hand_" + side, e=0.75, seg=14, rings=9, rot=frame3(dv, (1, 0, 0)))
        th0 = wr + dv * 0.07 + Vector((0.06, 0, 0))
        capsule("thumb", th0, th0 + (dv * 0.05 + Vector((0.035, 0, 0))), 0.029, 0.026, "Gloves", "hand_" + side, seg=8, nlen=1, ncap=2)
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

def stick_p2():
    stick_box("shaft", (0.044, 0.03, L + 0.08), (0, 0, -(L - 0.08) / 2), None, "Stick", (0.008, 1))
    stick_box("stick_blade", (BL, 0.022, 0.09), tuple(heel + bd * 0.14 + bu * 0.045), Rb, "Stick", (0.012, 2))
    stick_box("knob", (0.045, 0.035, 0.05), (0, 0, 0.06), None, "Trim", (0.01, 1))

# ================================================================= P3 clay miniature
def stitched_seam(name, pts_nrm, bone, part_bone_of=None, every=1):
    """A raised clay ridge along points on a surface with short cream cross-stitches over it (geometry)."""
    pts = [p + n * 0.004 for p, n in pts_nrm]
    sweep(name + "_ridge", pts, 0.0075, "Jersey", bone, seg=4, clay=0.3, dents=False)
    for i in range(0, len(pts_nrm) - 1, every):
        p, n = pts_nrm[i]
        q, _ = pts_nrm[i + 1]
        mid = p.lerp(q, 0.5) + n * 0.010
        t = (q - p).normalized()
        across = n.cross(t).normalized()
        R = Matrix((across, t, n)).transposed()   # stitch length along "across"
        box(name + "_stitch", (0.030, 0.009, 0.007), mid, "Trim", bone, bevel=None, rot=R, clay=0.0)

def ring_points(ob, z, n, cx=0.0, a0=0.0, a1=2 * math.pi, closed=True):
    out = []
    for i in range(n):
        a = a0 + (a1 - a0) * i / (n if closed else n - 1)
        h, nr = ray_from_axis(ob, z, a, cx)
        if h is not None:
            out.append((h, nr))
    return out

def build_p3():
    hip, knee, ank = P["hip"], P["knee"], P["ankle"]
    blob("breezers", (0.175, 0.26, 0.125), (0, 0, hip - 0.005), "Pads", "pelvis", e=0.55, seg=20, rings=11)
    belly = blob("belly", (0.168, 0.25, 0.105), (0, 0, P["sp1"] - 0.005), "Jersey", "spine_01", e=0.55, seg=20, rings=10)
    chest = blob("chest", (0.18, 0.255, 0.165), (0, 0, P["sp3"] - 0.005), "Jersey", "spine_03", e=0.55, seg=22, rings=13,
                 taper=((0.96, 0.95), (1.0, 1.12)))
    for s in (1, -1):
        blob("shoulder", (0.12, 0.11, 0.10), (0.0, s * 0.25, P["sh_z"] - 0.01), "Jersey", "spine_03", e=0.8, seg=12, rings=8)
    # hem roll (fat cream clay roll) and collar roll
    hem = ring_points(belly, P["sp1"] - 0.085, 20)
    sweep("hem_roll", [p + n * 0.006 for p, n in hem], 0.02, "Trim", "spine_01", seg=8, closed=True, up=(0, 0, 1), clay=0.5)
    collar = ring_points(chest, P["sp3"] + 0.14, 14)
    sweep("collar_roll", [p.lerp(Vector((0, 0, p.z)), 0.25) + Vector((0, 0, 0.012)) for p, n in collar], 0.024, "Trim", "spine_03",
          seg=8, closed=True, up=(0, 0, 1), clay=0.5)
    # stitched seams: around the collar, above the hem, down both sides (geometry ridge + cross-stitches)
    cs = ring_points(chest, P["sp3"] + 0.105, 18)
    stitched_seam("collar_seam", cs + [cs[0]], "spine_03")
    hs = ring_points(belly, P["sp1"] - 0.045, 21)
    stitched_seam("hem_seam", hs + [hs[0]], "spine_01")
    for s in (1, -1):
        a = math.radians(90 * s)
        side_c = [ray_from_axis(chest, z, a) for z in [P["sp3"] + 0.10 - 0.032 * i for i in range(7)]]
        stitched_seam("side_seam_hi", [h for h in side_c if h[0] is not None], "spine_03")
        side_b = [ray_from_axis(belly, z, a) for z in [P["sp1"] + 0.07 - 0.032 * i for i in range(4)]]
        stitched_seam("side_seam_lo", [h for h in side_b if h[0] is not None], "spine_01")
    # felt number patch on the back: a slightly raised soft panel, no digit, with a dark running stitch round it
    hb, nb = ray_from_axis(chest, P["sp3"] + 0.0, math.pi)
    blob("number_patch", (0.012, 0.13, 0.105), hb + Vector((0.006, 0, 0)), "Trim", "spine_03", e=0.3, ez=0.3, seg=16, rings=10,
         clay=0.4, dents=False)
    for i in range(20):
        a = 2 * math.pi * i / 20
        py, pz = 0.118 * math.cos(a), 0.093 * math.sin(a)
        py = max(-0.112, min(0.112, py * 1.15)); pz = max(-0.088, min(0.088, pz * 1.15))
        q = hb + Vector((-0.008, py, pz))
        box("patch_stitch", (0.006, 0.018 if abs(math.sin(a)) > 0.7 else 0.007, 0.007 if abs(math.sin(a)) > 0.7 else 0.018), q,
            "Pads", "spine_03", bevel=None, clay=0.0)
    # neck and head: a big round clay head, small helmet over it, face visible
    capsule("neck", (0, 0, P["neck"] - 0.06), (0, 0, P["head"] + 0.04), 0.066, 0.062, "Skin", "neck_01", seg=12, nlen=2, ncap=2)
    hz = P["head"] + 0.13
    head = blob("head", (0.150, 0.145, 0.150), (0.02, 0, hz), "Skin", "head", e=0.9, seg=20, rings=13, clay=0.7)
    for s in (1, -1):
        blob("ear_bump", (0.03, 0.02, 0.04), (0.0, s * 0.145, hz - 0.01), "Skin", "head", e=0.9, seg=10, rings=6)
    capsule("nose", (0.15, 0, hz - 0.02), (0.185, 0, hz - 0.025), 0.026, 0.024, "Skin", "head", seg=10, nlen=1, ncap=3, clay=0.5)
    for s in (1, -1):
        h, nr = surface_hit(head, (0.0, s * 0.05, hz + 0.015), (1, s * 0.25, 0))
        blob("eye", (0.012, 0.019, 0.024), h + nr * 0.004, "Pads", "head", e=1.0, seg=10, rings=7, clay=0.0)
        h2, nr2 = surface_hit(head, (0.0, s * 0.085, hz - 0.045), (1, s * 0.4, 0))
        blob("cheek", (0.012, 0.028, 0.022), h2 + nr2 * 0.002, "Skin", "head", e=1.0, seg=10, rings=6, clay=0.5)
        bh, bn = surface_hit(head, (0.0, s * 0.05, hz + 0.06), (1, s * 0.25, 0))
        sweep("brow", [bh + bn * 0.006 + Vector((0, s * 0.022 * t, 0.006 * (1 - t * t))) for t in (-1, 0, 1)], 0.0065, "Pads", "head",
              seg=5, clay=0.0)
    mouth = [surface_hit(head, (0.0, 0.035 * t, hz - 0.075 + 0.012 * t * t), (1, 0.4 * t, 0)) for t in (-1, -0.5, 0, 0.5, 1)]
    sweep("smile", [h + n * 0.003 for h, n in mouth], 0.0065, "Pads", "head", seg=5, clay=0.0)
    hc = Vector((-0.012, 0, hz + 0.04))
    blob("helmet", (0.172, 0.168, 0.15), hc, "Helmet", "head", e=0.85, seg=18, rings=11,
         cut=lambda c: (c.z < 0.28 and c.x > 0.25) or c.z < -0.42, solid=0.022, clay=0.8)
    for s in (1, -1):
        blob("ear_guard", (0.055, 0.03, 0.06), (0.005, s * 0.175, hz - 0.01), "Helmet", "head", e=0.8, seg=14, rings=8)
    for k in (-1, 0, 1):   # clay vent ridges over the shell
        sweep("helmet_ridge", [hc + Vector((0.17 * math.cos(a) * 1.0, k * 0.06, 0.15 * math.sin(a) * 1.04 - abs(k) * 0.012)) for a in
                               [math.radians(x) for x in (45, 70, 95, 120, 145)]], 0.012, "Helmet", "head", seg=6, clay=0.5)
    sweep("helmet_brim", [hc + Vector((0.172 * math.cos(a) * 1.01, 0.168 * math.sin(a) * 1.01, 0.04 - 0.02 * abs(math.sin(a))))
                          for a in [math.radians(x) for x in range(-70, 71, 14)]], 0.016, "Helmet", "head", seg=7, clay=0.5)
    # arms: fat clay sleeves with cream rolls, mitten gloves with a thumb
    for side, (sh, el, wr, hd, dv) in (("l", arm_pts("l")), ("r", arm_pts("r"))):
        s = 1 if side == "l" else -1
        capsule("upperarm", sh, el, 0.085, 0.078, "Jersey", "upperarm_" + side, seg=12, nlen=3, ncap=2)
        sphere("shoulder_ball", 0.088, sh, "Jersey", "clavicle_" + side, seg=10, rings=7)
        sphere("elbow", 0.08, el, "Jersey", "upperarm_" + side, seg=10, rings=7)
        capsule("forearm", el, wr, 0.078, 0.072, "Jersey", "lowerarm_" + side, seg=12, nlen=3, ncap=2)
        ax = (wr - el).normalized()
        u0 = ax.cross(Vector((1, 0, 0))).normalized(); v0 = ax.cross(u0)
        for t in (0.45, 0.62):
            c = el.lerp(wr, t)
            sweep("sleeve_roll", [c + (u0 * math.cos(2 * math.pi * i / 14) + v0 * math.sin(2 * math.pi * i / 10)) * 0.08 for i in range(10)],
                  0.014, "Trim", "lowerarm_" + side, seg=5, closed=True, clay=0.4)
        c = wr + dv * 0.10
        blob("mitten", (0.09, 0.072, 0.08), c, "Gloves", "hand_" + side, e=0.85, seg=14, rings=9, rot=frame3(dv, (1, 0, 0)))
        sweep("cuff_roll", [wr + dv * 0.02 + (u0 * math.cos(2 * math.pi * i / 14) + v0 * math.sin(2 * math.pi * i / 10)) * 0.082
                            for i in range(10)], 0.02, "Gloves", "hand_" + side, seg=5, closed=True, clay=0.5)
        th0 = wr + dv * 0.07 + Vector((0.065, 0, 0))
        capsule("thumb", th0, th0 + (dv * 0.05 + Vector((0.035, 0, 0))), 0.031, 0.028, "Gloves", "hand_" + side, seg=9, nlen=1, ncap=2)
    # legs: fat breezer legs, socks with cream rolls, clay boots, coiled blades
    for side, s in (("l", 1), ("r", -1)):
        y = s * P["hip_y"]
        sphere("hip_ball", 0.12, (0, y, hip - 0.02), "Pads", "pelvis", seg=10, rings=7)
        capsule("thigh", (0, y, hip - 0.03), (0, y, knee + 0.07), 0.13, 0.112, "Pads", "thigh_" + side, seg=11, nlen=3, ncap=2)
        sphere("knee_ball", 0.09, (0, y, knee), "Pads", "thigh_" + side, seg=10, rings=7)
        blob("kneepad", (0.07, 0.085, 0.09), (0.035, y, knee - 0.01), "Pads", "calf_" + side, e=0.9, seg=10, rings=7)
        capsule("sock", (0, y, knee - 0.03), (0, y, ank + 0.06), 0.088, 0.072, "Jersey", "calf_" + side, seg=10, nlen=3, ncap=2)
        for t in (0.35, 0.52):
            zc = knee - 0.03 + (ank + 0.06 - knee + 0.03) * t
            rr = 0.088 + (0.072 - 0.088) * t
            sweep("sock_roll", [Vector((rr * math.cos(2 * math.pi * i / 14), y + rr * math.sin(2 * math.pi * i / 14), zc)) for i in range(10)],
                  0.016, "Trim", "calf_" + side, seg=5, closed=True, up=(0, 0, 1), clay=0.4)
        blob("boot", (0.155, 0.068, 0.07), (0.045, y, ank - 0.045), "Skate", "foot_" + side, e=0.65, seg=14, rings=9)
        capsule("ankle", (0, y, ank - 0.04), (0, y, ank + 0.08), 0.07, 0.068, "Skate", "foot_" + side, seg=12, nlen=2, ncap=2)
        # coiled clay blade: one rolled snake from the heel post, flat along the ice, curling up and over at the toe
        pts = [Vector((-0.10, y, 0.05)), Vector((-0.11, y, 0.025)), Vector((-0.09, y, 0.012))]
        pts += [Vector((x, y, 0.012)) for x in (-0.05, 0.0, 0.05, 0.10, 0.14)]
        cx, cz, rr = 0.16, 0.042, 0.03
        for k in range(1, 12):
            a = -math.pi / 2 + k * (math.radians(400) / 11)
            r_k = rr * (1.0 - 0.45 * k / 11)
            pts.append(Vector((cx + r_k * math.cos(a), y, cz + r_k * math.sin(a))))
        sweep("blade_coil", pts, 0.012, "Blade", "foot_" + side, seg=6, up=(0, 1, 0), clay=0.4, dents=False)
        capsule("blade_post", (0.11, y, 0.02), (0.11, y, ank - 0.08), 0.014, 0.014, "Blade", "foot_" + side, seg=7, nlen=1, ncap=2)

def stick_p3():
    stick_box("shaft", (0.046, 0.034, L + 0.08), (0, 0, -(L - 0.08) / 2), None, "Stick", (0.014, 1), clay=0.6)
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=16, v_segments=10, radius=1.0)
    for v in bm.verts:
        v.co = Vector((math.copysign(abs(v.co.x) ** 0.45, v.co.x) * BL / 2, math.copysign(abs(v.co.y) ** 0.7, v.co.y) * 0.014,
                       math.copysign(abs(v.co.z) ** 0.5, v.co.z) * 0.048))
    finish("stick_blade", bm, stick_M(tuple(heel + bd * 0.15 + bu * 0.045), Rb), "Trim", "stick_socket", clay=0.7)
    sphere("knob", 0.032, grip_r + Rs0 @ Vector((0, 0, 0.07)), "Trim", "stick_socket", seg=8, rings=6)

# ================================================================= P4 stylised broadcast (about 7 heads, real gear)
def build_p4():
    hip, knee, ank = P["hip"], P["knee"], P["ankle"]
    # padded shorts (breezers): hips block + a shell per thigh (deforms with the legs)
    blob("breezers", (0.15, 0.215, 0.10), (0, 0, hip - 0.01), "Pads", "pelvis", e=0.6, seg=16, rings=10)
    # jersey: a wide torso over the shoulder pads (one silhouette), a loose hem hanging over the top of the breezers
    blob("belly", (0.135, 0.195, 0.10), (0, 0, P["sp1"] + 0.0), "Jersey", "spine_01", e=0.6, seg=16, rings=10)
    tube("jersey_hem", (0, 0, P["sp1"] + 0.02), (0, 0, hip - 0.045), 0.205, 0.228, "Jersey", "spine_01", seg=20, caps=False)
    for v in PARTS[-1].data.vertices:        # elliptical section: wider than deep
        v.co.x *= 0.72
    band("hem_stripe", (0, 0, hip - 0.045), (0, 0, P["sp1"] + 0.02), 0.16, 0.34, 0.226, "Trim", "spine_01", seg=20)
    for v in PARTS[-1].data.vertices:
        v.co.x *= 0.725
    blob("chest", (0.155, 0.235, 0.20), (0, 0, P["sp3"] + 0.02), "Jersey", "spine_03", e=0.7, seg=18, rings=12,
         taper=((0.92, 0.80), (1.0, 1.10)))
    for s in (1, -1):   # shoulder pads under the jersey: wide and flat, one line from neck to sleeve (not knobs)
        blob("shoulder_pad", (0.135, 0.125, 0.065), (0.0, s * 0.20, P["sh_z"] + 0.015), "Jersey", "spine_03", e=0.85, seg=14, rings=9)
        tube("yoke_stripe", (0.0, s * 0.16, P["sh_z"] + 0.075), (0.0, s * 0.29, P["sh_z"] + 0.02), 0.018, 0.018, "Trim", "spine_03", seg=8)
    sweep("collar", [Vector((0.075 * math.cos(a) + 0.01, 0.08 * math.sin(a), P["neck"] - 0.035 + 0.025 * max(0, math.cos(a))))
                     for a in [2 * math.pi * i / 16 for i in range(16)]], 0.018, "Trim", "spine_03", seg=6, closed=True, up=(0, 0, 1))
    # neck, head with a face, helmet with visor
    capsule("neck", (0, 0, P["neck"] - 0.06), (0, 0, P["head"] + 0.03), 0.055, 0.052, "Skin", "neck_01", seg=10, nlen=1, ncap=2)
    hz = P["head"] + 0.105
    blob("skull", (0.098, 0.083, 0.11), (0.005, 0, hz), "Skin", "head", e=0.9, seg=16, rings=10)
    blob("jaw", (0.07, 0.068, 0.05), (0.035, 0, hz - 0.08), "Skin", "head", e=0.8, seg=12, rings=8)
    blob("nose", (0.014, 0.012, 0.022), (0.098, 0, hz - 0.018), "Skin", "head", e=0.9, seg=8, rings=6)
    for s in (1, -1):
        blob("eye", (0.008, 0.012, 0.008), (0.094, s * 0.033, hz + 0.005), "Skate", "head", e=1.0, seg=8, rings=5)   # untinted slot (Pads takes team colour in p4)
        oriented_box("brow", (0.012, 0.03, 0.008), (0.096, s * 0.034, hz + 0.024), (0, -s * 0.15, 1), (1, 0, 0), "Skate", "head", bevel=None)
    hc = Vector((-0.012, 0, hz + 0.045))
    blob("helmet", (0.128, 0.107, 0.105), hc, "Helmet", "head", e=0.75, seg=22, rings=14,
         cut=lambda c: (c.z < -0.05 and c.x > 0.35) or c.z < -0.72, solid=0.012)
    for s in (1, -1):
        blob("ear_guard", (0.045, 0.02, 0.05), (0.0, s * 0.108, hz - 0.01), "Helmet", "head", e=0.7, seg=12, rings=7)
        sweep("chin_strap", [Vector((0.0, s * 0.105, hz - 0.04)), Vector((0.04, s * 0.075, hz - 0.11)), Vector((0.07, 0, hz - 0.135))],
              0.008, "Skate", "head", seg=5)
    for k in (-1, 1):   # vent grooves (ridges)
        sweep("helmet_vent", [hc + Vector((0.128 * math.cos(a) * 0.99, k * 0.035, 0.105 * math.sin(a) * 1.0)) for a in
                              [math.radians(x) for x in (40, 65, 90, 115)]], 0.008, "Pads", "head", seg=5)
    # visor: a curved plate in front of the eyes, ear to ear
    bm = bmesh.new()
    rows = []
    for zi, z in enumerate((hz - 0.03, hz + 0.0, hz + 0.035)):
        row = []
        for ai in range(11):
            a = math.radians(-75 + 150 * ai / 10)
            rx, ry = 0.122 - 0.004 * zi, 0.108
            row.append(bm.verts.new((hc.x + rx * math.cos(a) + 0.005, ry * math.sin(a), z)))
        rows.append(row)
    for i in range(2):
        for j in range(10):
            bm.faces.new((rows[i][j], rows[i][j + 1], rows[i + 1][j + 1], rows[i + 1][j]))
    finish("visor", bm, Matrix.Identity(4), "Blade", "head", solid=0.006)
    # arms: loose sleeves over elbow pads, big gauntlet gloves with a thumb
    for side, (sh, el, wr, hd, dv) in (("l", arm_pts("l")), ("r", arm_pts("r"))):
        s = 1 if side == "l" else -1
        sphere("shoulder_ball", 0.08, sh, "Jersey", "clavicle_" + side, seg=12, rings=8)
        blob("sleeve_cap", (0.10, 0.09, 0.09), sh + dv * 0.06, "Jersey", "upperarm_" + side, e=0.85, seg=12, rings=8, rot=frame3(dv, (1, 0, 0)))
        capsule("upperarm", sh, el, 0.082, 0.076, "Jersey", "upperarm_" + side, seg=12, nlen=1, ncap=2)
        blob("elbow_pad", (0.084, 0.08, 0.08), el + Vector((-0.012, 0, 0)), "Jersey", "upperarm_" + side, e=0.85, seg=12, rings=8)
        tube("forearm", el, wr - dv * 0.01, 0.078, 0.086, "Jersey", "lowerarm_" + side, seg=12)
        band("sleeve_stripe_a", el, wr, 0.30, 0.42, 0.0825, "Trim", "lowerarm_" + side, seg=12)
        band("sleeve_stripe_b", el, wr, 0.52, 0.64, 0.0845, "Trim", "lowerarm_" + side, seg=12)
        tube("glove_cuff", wr - dv * 0.035, wr + dv * 0.055, 0.074, 0.084, "Gloves", "hand_" + side, seg=12)
        R = frame3(dv, (1, 0, 0))
        blob("glove_hand", (0.085, 0.07, 0.058), wr + dv * 0.115, "Gloves", "hand_" + side, e=0.7, seg=14, rings=8, rot=R)
        blob("glove_roll", (0.03, 0.06, 0.05), wr + dv * 0.16 + Vector((0.025, 0, 0)), "Gloves", "hand_" + side, e=0.8, seg=10, rings=6, rot=R)
        th0 = wr + dv * 0.08 + Vector((0.05, 0, 0))
        capsule("thumb", th0, th0 + (dv * 0.05 + Vector((0.03, 0, 0))), 0.026, 0.023, "Gloves", "hand_" + side, seg=8, nlen=1, ncap=2)
    # legs: breezer shells, knee caps of the shin guards under the socks, socks with stripes, skates with holders and steel
    for side, s in (("l", 1), ("r", -1)):
        y = s * P["hip_y"]
        sphere("hip_ball", 0.095, (0, y, hip - 0.02), "Pads", "pelvis", seg=12, rings=8)
        bl_top, bl_bot = hip - 0.02, knee + 0.09   # padded shorts: a boxy shell per thigh, flared at the hem
        blob("breezer_leg", (0.13, 0.118, (bl_top - bl_bot) / 2), (0.005, y * 1.1, (bl_top + bl_bot) / 2), "Pads", "thigh_" + side,
             e=0.55, ez=0.35, seg=14, rings=8, taper=((1.06, 1.12), (0.92, 0.9)))
        tube("thigh", (0, y, knee + 0.15), (0, y, knee + 0.02), 0.075, 0.068, "Jersey", "thigh_" + side, seg=10)
        sphere("knee_ball", 0.07, (0, y, knee), "Jersey", "thigh_" + side, seg=10, rings=7)
        blob("shin_cap", (0.065, 0.068, 0.085), (0.03, y, knee - 0.01), "Jersey", "calf_" + side, e=0.8, seg=12, rings=8)
        tube("sock", (0, y, knee - 0.02), (0, y, ank + 0.09), 0.074, 0.058, "Jersey", "calf_" + side, seg=12)
        blob("shin_guard", (0.035, 0.055, 0.14), (0.04, y, (knee + ank) / 2 + 0.02), "Jersey", "calf_" + side, e=0.7, seg=10, rings=8)
        band("sock_stripe_a", (0, y, knee - 0.02), (0, y, ank + 0.09), 0.30, 0.42, 0.071, "Trim", "calf_" + side, seg=12)
        band("sock_stripe_b", (0, y, knee - 0.02), (0, y, ank + 0.09), 0.54, 0.66, 0.068, "Trim", "calf_" + side, seg=12)
        tube("skate_collar", (0, y, ank - 0.03), (-0.005, y, ank + 0.105), 0.058, 0.062, "Skate", "foot_" + side, seg=12)
        blob("boot", (0.14, 0.052, 0.058), (0.04, y, ank - 0.03), "Skate", "foot_" + side, e=0.6, seg=16, rings=8)
        blob("toe_cap", (0.05, 0.05, 0.04), (0.14, y, ank - 0.045), "Skate", "foot_" + side, e=0.8, seg=10, rings=6)
        blob("tendon_guard", (0.022, 0.035, 0.05), (-0.075, y, ank + 0.115), "Skate", "foot_" + side, e=0.7, seg=8, rings=6)
        box("holder", (0.30, 0.03, 0.04), (0.035, y, 0.045), "Trim", "foot_" + side, bevel=(0.012, 2))
        box("blade", (0.31, 0.008, 0.028), (0.035, y, 0.014), "Blade", "foot_" + side, bevel=(0.01, 2))

def stick_p4():
    stick_box("shaft", (0.034, 0.024, L + 0.06), (0, 0, -(L - 0.06) / 2), None, "Stick", (0.006, 1))
    stick_box("stick_blade", (BL, 0.018, 0.075), tuple(heel + bd * 0.14 + bu * 0.04), Rb, "Stick", (0.012, 2))
    stick_box("blade_tape", (BL * 0.62, 0.021, 0.06), tuple(heel + bd * 0.16 + bu * 0.04), Rb, "Trim", (0.008, 1))
    stick_box("knob", (0.040, 0.030, 0.10), (0, 0, 0.03), None, "Trim", (0.008, 1))

{"p2_cel": build_p2, "p3_clay": build_p3, "p4_broadcast": build_p4}[STYLE]()
STICK_PARTS_START = len(PARTS)
{"p2_cel": stick_p2, "p3_clay": stick_p3, "p4_broadcast": stick_p4}[STYLE]()

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

body = join(PARTS[:STICK_PARTS_START], "SK_Skater_" + STYLE)
# UV0 = planar projection seen from behind framing the jersey back (u across y +0.2..-0.2, v over num_z0..num_z1): number area
uvl = body.data.uv_layers.new(name="UVMap")
z0, z1 = P["num_z0"], P["num_z1"]
for loop in body.data.loops:
    co = body.data.vertices[loop.vertex_index].co
    uvl.data[loop.index].uv = (0.5 - co.y / 0.40, (co.z - z0) / (z1 - z0))
stick = join(PARTS[STICK_PARTS_START:], "SK_Skater_" + STYLE + "_Stick")
for ob in (body, stick):
    ob.parent = arm
    mo = ob.modifiers.new("Armature", "ARMATURE")
    mo.object = arm

def tris(ob):
    ob.data.calc_loop_triangles()
    return len(ob.data.loop_triangles)
zs = [v.co.z for v in body.data.vertices]
info = dict(style=STYLE, tris_body=tris(body), tris_stick=tris(stick), verts=len(body.data.vertices) + len(stick.data.vertices),
            height_cm=round(max(zs) * 100, 1), lowest_cm=round(min(zs) * 100, 2), bones=len(arm_data.bones),
            slots=[m.name for m in body.data.materials] + [m.name for m in stick.data.materials if m.name not in [x.name for x in body.data.materials]],
            bone_lengths_cm={b.name: round(b.length * 100, 1) for b in arm_data.bones})
info["tris_total"] = info["tris_body"] + info["tris_stick"]
import json
print("SKF BUILD " + json.dumps(info))
with open(os.path.join(WORK, "build.json"), "w") as fh:
    json.dump(info, fh, indent=1)

# separate static stick for attaching to stick_socket in Unreal (pivot = grip, shaft along -Z, blade toe along +X when tilted 50 deg),
# written in centimetres like the skeletal mesh (unit 0.01, data x100, FBX_SCALE_NONE)
st2 = stick.copy(); st2.data = stick.data.copy(); scn.collection.objects.link(st2)
st2.parent = None; st2.modifiers.clear(); st2.vertex_groups.clear()
st2.data.transform(Matrix.Translation(-grip_r))
st2.data.transform((Rs0.inverted()).to_4x4())
st2.data.transform(Matrix.Scale(100.0, 4))
st2.name = "SM_Stick_" + STYLE
for o in bpy.context.view_layer.objects:
    o.select_set(False)
st2.select_set(True)
scn.unit_settings.scale_length = 0.01
bpy.ops.export_scene.fbx(filepath=os.path.join(EXP, "SM_Stick_%s.fbx" % STYLE), use_selection=True, object_types={"MESH"},
                         apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0, axis_forward="-Z", axis_up="Y",
                         mesh_smooth_type="FACE", bake_anim=False)
scn.unit_settings.scale_length = 1.0
bpy.data.objects.remove(st2)

bpy.ops.wm.save_as_mainfile(filepath=os.path.join(WORK, "skater.blend"))
print("SKF BUILD OK", STYLE, WORK)
