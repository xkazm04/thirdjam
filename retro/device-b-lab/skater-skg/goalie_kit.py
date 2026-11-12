# SKG goalie kit: rigid meshes that attach to the skater's bones, built from any armature with the s4 bone names.
# Imported by build_goalie.py (not run on its own). Blender axes: figure faces +X, left = +Y, up = +Z, metres, rest A-pose.
# Every size comes from the armature's rest bone positions (and, when a skinned body is given, from the bounds of the body parts
# weighted to the head, calf, hand and spine_03 bones), so the same script fits p2_cel, p3_clay and p4_broadcast figures.
# Each piece is authored in its attach bone's rest frame (origin = bone head) except the stick, which is authored in the grip frame
# of SM_S4_Stick (origin = grip, shaft along -Z, blade toe along +X once the shaft is tilted by its lie about Y).
# Provenance: original (made by this script).
import bpy, bmesh, math
from mathutils import Vector, Matrix, noise

# ---------------------------------------------------------------- style table (one parameter set per look)
KIT_STYLES = {
    "p2_cel": dict(      # flat planes, sharp silhouette
        bevel=0.0, bseg=1, smooth=False, disp=0.0, puff=1.0,
        pad_w=0.27, pad_t=0.17, pad_above=0.20, pad_below=-0.03, rolls=0, stack=2,
        mask_seg=10, mask_rings=7, chin=0.0, throat=True, glove_curve=False, glove_seg=8,
        chest_w=1.0, floater=1.0, stick_paddle=0.085, stick_blade_h=0.12, mask_mat="Helmet"),
    "p3_clay": dict(     # rounded, slightly irregular (seeded displacement), thick edges
        bevel=0.032, bseg=2, smooth=True, disp=0.007, puff=1.08, seed=7,
        pad_w=0.28, pad_t=0.19, pad_above=0.20, pad_below=-0.035, rolls=0, stack=2,
        mask_seg=14, mask_rings=9, chin=0.02, throat=True, glove_curve=False, glove_seg=12,
        chest_w=1.04, floater=1.1, stick_paddle=0.095, stick_blade_h=0.13, mask_mat="Pad"),
    "p4_broadcast": dict(  # real pad shapes: vertical rolls, a curved catching glove, a mask with a chin
        bevel=0.012, bseg=2, smooth=True, disp=0.0, puff=1.0,
        pad_w=0.27, pad_t=0.15, pad_above=0.21, pad_below=-0.03, rolls=3, stack=3,
        mask_seg=16, mask_rings=10, chin=0.06, throat=True, glove_curve=True, glove_seg=14,
        chest_w=1.0, floater=1.0, stick_paddle=0.08, stick_blade_h=0.115, mask_mat="Pad"),
}
# sRGB; the skater's nine slot names plus "Pad" (the white-ish pad faces). Skater slots are reused from the body when it has them.
PAD_COLOUR = {"p2_cel": (0.95, 0.95, 0.92), "p3_clay": (0.94, 0.90, 0.81), "p4_broadcast": (0.97, 0.97, 0.96)}
SKATER_COLOURS = {
    "p2_cel": {"Jersey": (0.10, 0.26, 0.78), "Trim": (0.95, 0.95, 0.93), "Helmet": (0.93, 0.93, 0.90), "Gloves": (0.10, 0.11, 0.15),
               "Pads": (0.12, 0.13, 0.18), "Blade": (0.78, 0.82, 0.86), "Stick": (0.72, 0.52, 0.26)},
    "p3_clay": {"Jersey": (0.16, 0.30, 0.74), "Trim": (0.95, 0.91, 0.82), "Helmet": (0.20, 0.32, 0.76), "Gloves": (0.17, 0.20, 0.36),
                "Pads": (0.22, 0.20, 0.26), "Blade": (0.93, 0.90, 0.83), "Stick": (0.30, 0.22, 0.18)},
    "p4_broadcast": {"Jersey": (0.08, 0.22, 0.72), "Trim": (0.96, 0.96, 0.96), "Helmet": (0.07, 0.17, 0.58), "Gloves": (0.07, 0.15, 0.48),
                     "Pads": (0.06, 0.12, 0.42), "Blade": (0.70, 0.76, 0.82), "Stick": (0.13, 0.13, 0.14)},
}
# goalie stick (grip to heel, lie = shaft angle from vertical at which the blade lies flat); shared with the clip solver
STICK_G = dict(L=0.72, lie=34.0, above=0.30, paddle_len=0.40, blade_len=0.40)


def srgb2lin(c):
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def get_materials(style):
    mats = {}
    cols = dict(SKATER_COLOURS[style]); cols["Pad"] = PAD_COLOUR[style]
    for name, c in cols.items():
        m = bpy.data.materials.get(name)
        if m is None:
            m = bpy.data.materials.new(name)
            lin = tuple(srgb2lin(x) for x in c)
            m.diffuse_color = (*lin, 1.0)
            m.use_nodes = True
            b = m.node_tree.nodes.get("Principled BSDF")
            b.inputs["Base Color"].default_value = (*lin, 1.0)
            b.inputs["Roughness"].default_value = 0.8
        mats[name] = m
    return mats


# ---------------------------------------------------------------- mesh parts (built in armature rest space, metres)
class Piece:
    def __init__(self, name, bone, st):
        self.name, self.bone, self.st = name, bone, st
        self.parts = []          # (verts, faces, material, smooth)

    def add_bm(self, bm, mat, M=None, disp=True):
        st = self.st
        if M is not None:
            bm.transform(M)
        if disp and st["disp"] > 0:
            bm.normal_update()
            off = Vector((st.get("seed", 0) * 13.1, st.get("seed", 0) * 7.3, 3.7))
            for v in bm.verts:
                n = noise.noise(v.co * 9.0 + off) + 0.5 * noise.noise(v.co * 21.0 + off)
                v.co += v.normal * st["disp"] * n
        verts = [v.co.copy() for v in bm.verts]
        bm.verts.index_update()
        faces = [[v.index for v in f.verts] for f in bm.faces]
        bm.free()
        self.parts.append((verts, faces, mat))


def _bevel(bm, st, width=None):
    w = st["bevel"] if width is None else width
    if w > 0:
        bmesh.ops.bevel(bm, geom=list(bm.edges), offset=w, segments=st["bseg"], profile=0.5, affect="EDGES", clamp_overlap=True)


def frame_from(x, y):
    """3x3 rotation whose columns are x, y (orthonormalised) and x cross y."""
    x = Vector(x).normalized()
    y = (Vector(y) - x * Vector(y).dot(x)).normalized()
    return Matrix((x, y, x.cross(y))).transposed()


def M4(R, c):
    return Matrix.Translation(Vector(c)) @ R.to_4x4()


def box(p, dims, c, R=None, mat="Pad", bevel=None, taper=None):
    bm = bmesh.new()
    bmesh.ops.create_cube(bm, size=1.0)
    for v in bm.verts:
        if taper and v.co.z > 0:
            v.co.x *= taper[0]; v.co.y *= taper[1]
        v.co.x *= dims[0]; v.co.y *= dims[1]; v.co.z *= dims[2]
    _bevel(bm, p.st, bevel)
    p.add_bm(bm, mat, M4(R or Matrix.Identity(3), c))


def ellipsoid(p, radii, c, R=None, mat="Pad", seg=12, rings=8, cut=None, disp=True):
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=seg, v_segments=rings, radius=1.0)
    if cut:
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if cut(v.co)], context="VERTS")
    for v in bm.verts:
        v.co.x *= radii[0]; v.co.y *= radii[1]; v.co.z *= radii[2]
    p.add_bm(bm, mat, M4(R or Matrix.Identity(3), c), disp=disp)


def cyl(p, a, b, r, mat="Pad", seg=8, r2=None, caps=True):
    a, b = Vector(a), Vector(b)
    L = (b - a).length
    bm = bmesh.new()
    bmesh.ops.create_cone(bm, cap_ends=caps, cap_tris=False, segments=seg, radius1=r, radius2=r if r2 is None else r2, depth=L)
    bmesh.ops.translate(bm, verts=bm.verts, vec=(0, 0, L / 2))
    R = Vector((0, 0, 1)).rotation_difference((b - a).normalized()).to_matrix()
    p.add_bm(bm, mat, M4(R, a))


def shell(p, radii, c, R, mat, seg, rings, cut, thick):
    """Ellipsoid shell with thickness (solidified by hand: outer + inner surface + rim)."""
    bm = bmesh.new()
    bmesh.ops.create_uvsphere(bm, u_segments=seg, v_segments=rings, radius=1.0)
    if cut:
        bmesh.ops.delete(bm, geom=[v for v in bm.verts if cut(v.co)], context="VERTS")
    for v in bm.verts:
        v.co.x *= radii[0]; v.co.y *= radii[1]; v.co.z *= radii[2]
    if thick > 0:
        res = bmesh.ops.solidify(bm, geom=list(bm.faces), thickness=thick)
    p.add_bm(bm, mat, M4(R, c))


# ---------------------------------------------------------------- measurements of the figure
def bone_info(arm):
    B = arm.data.bones
    H = {b.name: b.head_local.copy() for b in B}
    T = {b.name: b.tail_local.copy() for b in B}
    R = {b.name: b.matrix_local.to_3x3() for b in B}
    return H, T, R


def group_bounds(body, bone, H, R, axes="arm"):
    """Bounds of the body vertices weighted to `bone`, in armature axes relative to the bone head (or None)."""
    if body is None or bone not in body.vertex_groups:
        return None
    gi = body.vertex_groups[bone].index
    pts = [body.matrix_world @ v.co for v in body.data.vertices if any(g.group == gi and g.weight > 0.5 for g in v.groups)]
    if not pts:
        return None
    mn = Vector([min(p[i] for p in pts) for i in range(3)])
    mx = Vector([max(p[i] for p in pts) for i in range(3)])
    return mn, mx, pts


def hand_frame(H, T, side):
    """Canonical rest frame of a hand: x = finger direction, y = back of the hand (up and out in the A-pose), z = x cross y."""
    d = (T["hand_" + side] - H["hand_" + side]).normalized()
    s = 1 if side == "l" else -1
    up = Vector((0, 0, 1))
    back = (up - d * up.dot(d)).normalized()       # perpendicular to the arm, towards the top of the arm in the A-pose
    return frame_from(d, back)


# ---------------------------------------------------------------- pieces
def build(arm, body, style):
    st = dict(KIT_STYLES[style])
    H, T, R = bone_info(arm)
    pieces = []
    pf = st["puff"]

    # -- leg pads (calf_l, calf_r): tall flat-fronted slab from the boot top to above the knee, knee stack on the front
    for side, s in (("l", 1), ("r", -1)):
        p = Piece("Pad_" + side.upper(), "calf_" + side, st)
        knee, ank = H["calf_" + side], T["calf_" + side]
        gb = group_bounds(body, "calf_" + side, H, R)
        front = (gb[1].x - knee.x) if gb else 0.12                      # front of the knee pad / sock
        z0 = ank.z - st["pad_below"] * (knee.z - ank.z) / 0.36
        z1 = knee.z + st["pad_above"] * (knee.z - ank.z) / 0.36
        w, t = st["pad_w"] * pf, st["pad_t"] * pf
        xb = knee.x + front - t + 0.035                                 # back face, inside the leg
        xf = knee.x + front + 0.035                                     # front face
        yc = knee.y + s * 0.012
        cz = (z0 + z1) / 2
        box(p, (xf - xb, w, z1 - z0), (0.5 * (xf + xb), yc, cz), mat="Pad")
        # outer side band in the team colour (reads as the pad's side rolls at distance)
        box(p, (xf - xb + 0.01, 0.022, z1 - z0 - 0.04), (0.5 * (xf + xb) + 0.004, yc + s * (w / 2 + 0.006), cz), mat="Jersey",
            bevel=min(st["bevel"], 0.008))
        # knee stack: blocks on the front at knee height
        kz = knee.z + 0.01
        for i in range(st["stack"]):
            hh = 0.055
            box(p, (0.035 + 0.012 * (st["stack"] - i), w * (0.74 - 0.06 * i), hh),
                (xf + 0.012 + 0.006 * (st["stack"] - i), yc, kz - 0.03 + i * hh * 0.95), mat="Pad" if i < st["stack"] - 1 else "Jersey")
        # inner knee wing (the "knee landing" on the inside edge)
        box(p, (0.12, 0.05, 0.16), (xb + 0.05, yc - s * (w / 2 - 0.025), kz - 0.04), mat="Pad")
        # boot strap / toe line, dark
        box(p, (0.012, w * 0.86, 0.04), (xf + 0.004, yc, z0 + 0.07), mat="Jersey", bevel=0.0)
        if st["rolls"]:
            # vertical rolls: outer roll big, inner roll, two channel ridges on the face
            zz0, zz1 = z0 + 0.02, kz - 0.05
            for frac, r, mat in ((0.47, 0.034, "Pad"), (-0.47, 0.026, "Pad"), (0.16, 0.012, "Pad"), (-0.16, 0.012, "Pad")):
                cyl(p, (xf - 0.004, yc + s * frac * w, zz0), (xf - 0.004, yc + s * frac * w, zz1), r, mat=mat, seg=8)
            for frac, r in ((0.47, 0.034), (-0.47, 0.026)):  # thigh rider part of the rolls above the stack
                cyl(p, (xf - 0.004, yc + s * frac * w, kz + st["stack"] * 0.052), (xf - 0.004, yc + s * frac * w, z1 - 0.01), r * 0.9, seg=8)
        pieces.append(p)

    # -- blocker (hand_r): rectangular board on the back of the stick hand, cuff around the wrist
    F = hand_frame(H, T, "r")
    wr = H["hand_r"]
    gb = group_bounds(body, "hand_r", H, R)
    gsz = ((gb[1] - gb[0]).length / 0.28) if gb else 1.0
    p = Piece("Blocker", "hand_r", st)
    box(p, (0.32 * gsz * pf, 0.035 * pf, 0.20 * gsz * pf), wr + F @ Vector((0.075, 0.105, 0.0)), F, mat="Pad")
    box(p, (0.32 * gsz * pf + 0.012, 0.012, 0.03), wr + F @ Vector((0.075, 0.125, 0.0)), F, mat="Jersey", bevel=0.0)  # centre stripe
    cyl(p, wr + F @ Vector((-0.07, 0, 0)), wr + F @ Vector((0.03, 0, 0)), 0.105 * pf, mat="Jersey", seg=10)
    box(p, (0.16, 0.07, 0.15), wr + F @ Vector((0.09, -0.02, 0.0)), F, mat="Gloves")                  # glove fingers
    pieces.append(p)

    # -- catching glove (hand_l): big mitt, the pocket on the palm side (-y of the hand frame), fingers along +x
    F = hand_frame(H, T, "l")
    wr = H["hand_l"]
    gb = group_bounds(body, "hand_l", H, R)
    gsz = ((gb[1] - gb[0]).length / 0.28) if gb else 1.0
    p = Piece("Glove", "hand_l", st)
    gw, gl = 0.30 * gsz * pf, 0.34 * gsz * pf
    gc = wr + F @ Vector((0.12, -0.03, 0.0))
    if st["glove_curve"]:
        # curved mitt: a cupped ellipsoid shell opening towards the palm side, a thumb roll and the T-trap web
        Rg = F @ Matrix.Rotation(math.radians(-90), 3, "X")   # cup opening along -y of the hand frame
        shell(p, (gl / 2, gw / 2, 0.11), gc + F @ Vector((0, 0.04, 0)), F, "Pad", st["glove_seg"], 10,
              cut=lambda c: c.y < -0.25, thick=0.025)
        cyl(p, gc + F @ Vector((-0.10, -0.02, gw / 2 - 0.02)), gc + F @ Vector((0.13, -0.05, gw / 2 - 0.05)), 0.03, mat="Pad", seg=8)
        box(p, (0.14, 0.012, 0.08), gc + F @ Vector((0.13, -0.06, 0.05)), F, mat="Gloves", bevel=0.0)    # web
        box(p, (gl * 0.5, 0.012, gw * 0.42), gc + F @ Vector((0.02, -0.01, -0.02)), F, mat="Gloves", bevel=0.0)   # pocket (dark)
    else:
        # flat mitt: an octagonal plate (p2) or a rounded puffy blob (p3) with a dark pocket face
        bm = bmesh.new()
        bmesh.ops.create_circle(bm, cap_ends=True, segments=st["glove_seg"], radius=1.0)
        res = bmesh.ops.extrude_face_region(bm, geom=list(bm.faces))
        bmesh.ops.translate(bm, verts=[e for e in res["geom"] if isinstance(e, bmesh.types.BMVert)], vec=(0, 0, 1))
        for v in bm.verts:
            v.co.x *= gl / 2; v.co.y *= gw / 2; v.co.z = (v.co.z - 0.5) * 0.10
        _bevel(bm, st)
        # plate lies in the x-z plane of the hand frame (normal along y)
        Rp = F @ Matrix(((1, 0, 0), (0, 0, 1), (0, -1, 0)))
        p.add_bm(bm, "Pad", M4(Rp, gc + F @ Vector((0, 0.02, 0))))
        box(p, (gl * 0.7, 0.012, gw * 0.62), gc + F @ Vector((0.0, -0.035, 0)), F, mat="Gloves", bevel=0.0)  # pocket face
        box(p, (gl * 0.5, 0.05, 0.06), gc + F @ Vector((0.0, -0.03, gw / 2 - 0.02)), F, mat="Pad")         # thumb ridge
    cyl(p, wr + F @ Vector((-0.07, 0, 0)), wr + F @ Vector((0.02, 0, 0)), 0.105 * pf, mat="Jersey", seg=10)
    pieces.append(p)

    # -- chest protector under the jersey (spine_03): a wider, squarer torso shell
    p = Piece("Chest", "spine_03", st)
    sp3, nk = H["spine_03"], H["neck_01"]
    gb = group_bounds(body, "spine_03", H, R)
    if gb:
        x0, x1, yw, ztop = gb[0].x, gb[1].x, max(abs(gb[0].y), abs(gb[1].y)), gb[1].z
    else:
        x0, x1, yw, ztop = -0.17, 0.17, 0.38, nk.z + 0.03
    cw = st["chest_w"]
    zb = H["spine_01"].z + 0.03
    box(p, ((x1 - x0) + 0.07, 2 * (yw - 0.07) * cw, ztop - zb - 0.02), (0.5 * (x0 + x1) + 0.012, 0, 0.5 * (zb + ztop) - 0.01), mat="Jersey",
        bevel=max(st["bevel"], 0.0), taper=(1.0, 1.10))
    box(p, ((x1 - x0) * 0.55, 2 * (yw - 0.11) * cw, 0.11), (x1 - 0.03, 0, zb - 0.02), mat="Jersey")    # belly flap hanging in front
    box(p, ((x1 - x0) + 0.08, 2 * (yw - 0.065) * cw * 1.02, 0.035), (0.5 * (x0 + x1) + 0.012, 0, zb + 0.06), mat="Trim", bevel=min(st["bevel"], 0.01))
    box(p, (0.10, 0.20, 0.05), (x1 - 0.01, 0, ztop - 0.03), mat="Trim")                                  # collar
    pieces.append(p)

    # -- arms (upperarm_l, upperarm_r): shoulder floater cap + a fat sleeve with a stripe
    for side, s in (("l", 1), ("r", -1)):
        p = Piece("Arm_" + side.upper(), "upperarm_" + side, st)
        sh, el = H["upperarm_" + side], T["upperarm_" + side]
        d = (el - sh).normalized()
        Fa = frame_from(d, Vector((0, 0, 1)))
        fl = st["floater"] * pf
        ellipsoid(p, (0.13 * fl, 0.105 * fl, 0.15 * fl), sh + Fa @ Vector((0.03, 0.06, 0)), Fa, mat="Jersey", seg=st["mask_seg"], rings=6)
        cyl(p, sh + d * 0.05, el + d * 0.02, 0.118 * pf, mat="Jersey", seg=10, r2=0.108 * pf)
        cyl(p, sh + d * 0.18, sh + d * 0.235, 0.121 * pf, mat="Trim", seg=10)
        ellipsoid(p, (0.10 * pf, 0.10 * pf, 0.10 * pf), el, Fa, mat="Jersey", seg=10, rings=6)       # elbow pad
        pieces.append(p)

    # -- mask (head): shell over the helmet, a face window with a cage, a chin (p4) and a throat guard
    p = Piece("Mask", "head", st)
    gb = group_bounds(body, "head", H, R)
    if gb:
        mn, mx = gb[0], gb[1]
    else:
        hd, ht = H["head"], T["head"]
        L = (ht - hd).length
        mn, mx = hd + Vector((-0.9 * L, -0.9 * L, -0.25 * L)), hd + Vector((L, 0.9 * L, 1.15 * L))
    c = (mn + mx) / 2
    ext = (mx - mn) / 2
    rx, ry, rz = ext.x * 1.04 + 0.015, ext.y * 1.02 + 0.02, ext.z * 0.98 + 0.02
    c = c + Vector((-0.012, 0, 0.012))
    # window: front, between the brow and the chin; the shell is cut there
    win = lambda v: v.x > 0.45 and -0.62 < v.z < 0.28 and abs(v.y) < 0.62
    shell(p, (rx, ry, rz), c, Matrix.Identity(3), st["mask_mat"], st["mask_seg"], st["mask_rings"], cut=win, thick=0.012)
    # team-colour stripe over the crown
    ellipsoid(p, (rx * 0.92, 0.035, rz * 1.02 + 0.004), c + Vector((-0.02, 0, 0.006)), mat="Jersey", seg=st["mask_seg"], rings=st["mask_rings"],
              cut=lambda v: v.x > 0.55 or v.z < 0.05)
    # cage: vertical and horizontal bars across the window
    xw = c.x + rx * 0.86
    zlo, zhi = c.z - rz * 0.62, c.z + rz * 0.28
    for y in (-0.6, -0.3, 0.0, 0.3, 0.6):
        box(p, (0.012, 0.012, zhi - zlo), (xw - abs(y) * rx * 0.25, y * ry * 0.62, (zlo + zhi) / 2), mat="Blade", bevel=0.0)
    for f in (0.15, 0.45, 0.75):
        z = zlo + f * (zhi - zlo)
        box(p, (0.012, 2 * ry * 0.66, 0.012), (xw - 0.01, 0, z), mat="Blade", bevel=0.0)
    if st["chin"] > 0:
        box(p, (0.09, ry * 0.9, st["chin"] + 0.03), (c.x + rx * 0.62, 0, zlo - st["chin"] * 0.4), mat=st["mask_mat"])
    if st["throat"]:
        box(p, (0.02, 0.13, 0.10), (c.x + rx * 0.55, 0, zlo - st["chin"] - 0.06), mat="Pad", bevel=min(st["bevel"], 0.01))
    pieces.append(p)

    # -- goalie stick (stick_socket), authored in the grip frame (pivot = grip, shaft along -Z, blade toe +X at tilt = lie)
    p = Piece("Stick", "stick_socket", st)
    L, lie = STICK_G["L"] * H["head"].z / 1.37, math.radians(STICK_G["lie"])   # scales with the figure, as in the clip solver
    p.grip_frame = True
    box(p, (0.040, 0.030, L - STICK_G["paddle_len"] + STICK_G["above"]), (0, 0, (STICK_G["above"] - (L - STICK_G["paddle_len"])) / 2),
        mat="Stick", bevel=min(st["bevel"], 0.006))
    box(p, (0.05, 0.036, 0.05), (0, 0, STICK_G["above"]), mat="Trim", bevel=min(st["bevel"], 0.008))      # knob
    pl = STICK_G["paddle_len"]
    box(p, (st["stick_paddle"], 0.026, pl - 0.03), (0.004, 0, -(L - pl) - (pl - 0.03) / 2), mat="Pad", bevel=min(st["bevel"], 0.008))
    Rb = Matrix.Rotation(lie, 3, "Y")
    heel = Vector((0, 0, -L))
    bd, bu = Rb @ Vector((1, 0, 0)), Rb @ Vector((0, 0, 1))
    bl, bh = STICK_G["blade_len"], st["stick_blade_h"]
    box(p, (bl, 0.024, bh), heel + bd * (bl / 2 - 0.03) + bu * (bh / 2), Rb, mat="Pad", bevel=min(st["bevel"], 0.01))
    box(p, (bl * 0.6, 0.026, 0.03), heel + bd * (bl * 0.55) + bu * (bh * 0.55), Rb, mat="Gloves", bevel=0.0)   # tape band
    pieces.append(p)
    return pieces


# ---------------------------------------------------------------- objects, attach data
def realise(pieces, arm, mats, style, rs0):
    """Make one object per piece with vertices in its local frame; parent it to its bone (preview). Returns {name: obj}, attach dict."""
    H, T, R = bone_info(arm)
    objs, attach = {}, {}
    st = KIT_STYLES[style]
    for p in pieces:
        name = "SM_Goalie_" + p.name
        order = []
        verts, faces, fmat = [], [], []
        for vs, fs, m in p.parts:
            if m not in order:
                order.append(m)
            base = len(verts)
            verts += vs
            for f in fs:
                faces.append([base + i for i in f]); fmat.append(order.index(m))
        if getattr(p, "grip_frame", False):
            Rrel = R[p.bone].transposed() @ rs0           # bone frame -> grip frame
            local = verts                                 # already in the grip frame
        else:
            Rrel = Matrix.Identity(3)
            Ri = R[p.bone].transposed()
            local = [Ri @ (v - H[p.bone]) for v in verts]
        me = bpy.data.meshes.new(name)
        me.from_pydata([tuple(v) for v in local], [], faces)
        me.validate()
        for m in order:
            me.materials.append(mats[m])
        me.polygons.foreach_set("material_index", fmat)
        for poly in me.polygons:
            poly.use_smooth = st["smooth"]
        if st["smooth"]:
            try:
                me.set_sharp_from_angle(angle=math.radians(45))
            except Exception:
                pass
        me.update()
        ob = bpy.data.objects.new(name, me)
        bpy.context.scene.collection.objects.link(ob)
        ob.parent = arm
        ob.parent_type = "BONE"
        ob.parent_bone = p.bone
        blen = (T[p.bone] - H[p.bone]).length
        ob.matrix_parent_inverse = Matrix.Identity(4)
        ob.matrix_basis = Matrix.Translation((0, -blen, 0)) @ Rrel.to_4x4()   # bone parenting is at the tail in Blender
        me.calc_loop_triangles()
        objs[p.name] = ob
        q = Rrel.to_quaternion()
        Mrest = R[p.bone] @ Rrel
        hcm = H[p.bone] * 100
        attach[name] = {
            "bone": p.bone, "triangles": len(me.loop_triangles), "materials": order,
            "relative_location_cm": [0.0, 0.0, 0.0],
            "relative_rotation_quat_wxyz_blender_bone_space": [round(x, 6) for x in q],
            "relative_rotation_quat_wxyz_ue": [round(q.w, 6), round(-q.x, 6), round(q.y, 6), round(-q.z, 6)],
            "rest_origin_cm_blender_armature": [round(x, 3) for x in hcm],
            "rest_origin_cm_ue_component": [round(hcm.x, 3), round(-hcm.y, 3), round(hcm.z, 3)],
            "rest_axes_blender_armature": {"x": [round(v, 5) for v in Mrest.col[0]], "y": [round(v, 5) for v in Mrest.col[1]],
                                           "z": [round(v, 5) for v in Mrest.col[2]]},
            "local_frame": "grip frame of SM_S4_Stick: pivot = grip, shaft along -Z, blade toe +X at tilt = lie (%.0f deg)" % STICK_G["lie"]
            if getattr(p, "grip_frame", False) else "the bone's own rest frame (origin = bone head)",
        }
    return objs, attach
