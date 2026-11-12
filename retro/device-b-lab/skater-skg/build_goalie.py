# SKG: goalie kit + goalie clips on any armature with the s4 bone names.
#   blender -b --factory-startup --python build_goalie.py -- <skg_dir> <style> <skeleton .blend|.fbx> [--export-skel]
# Writes <skg>/export/<style>/SM_Goalie_<Piece>.fbx, <skg>/export/<style>/attach.json (+ export/attach.json for p2_cel),
#        <skg>/preview/goalie_<style>.blend (figure + kit + goalie actions, for looking only), <skg>/report_<style>.json,
#        and with --export-skel <skg>/export/SK_Goalie_clips_loopcut.fbx (+ SK_Goalie_clips.fbx with the loop frame repeated).
# Solving follows s4/build_clips.py: pose tables, skates and hands by two-bone IK, pelvis x/y locked (only its height is keyed),
# root never keyed, loops by cyclic Catmull-Rom with the first frame copied into the last, one-shots eased per key.
# Goalie additions: absolute hand orientation (direction + back of the hand), explicit knee pole and calf roll pole (pads face
# the shooter in the butterfly), skate orientation that can follow the calf, and the goalie stick's length and lie.
import bpy, math, os, sys, json
import numpy as np
from mathutils import Vector, Matrix

A = sys.argv[sys.argv.index("--") + 1:]
SKG = os.path.abspath(A[0]); STYLE = A[1]; SKEL = os.path.abspath(A[2]); EXPORT_SKEL = "--export-skel" in A
sys.path.insert(0, SKG)
import goalie_kit as GK

# ---------------------------------------------------------------- load the figure
if SKEL.lower().endswith(".blend"):
    bpy.ops.wm.open_mainfile(filepath=SKEL)
else:
    bpy.ops.wm.read_factory_settings(use_empty=True)
    bpy.ops.import_scene.fbx(filepath=SKEL, automatic_bone_orientation=False)
    for ob in list(bpy.data.objects):
        if ob.type == "ARMATURE" and ob.parent is None:
            ob.animation_data_clear()
    # bring a centimetre file back to metres with an identity armature object
    for ob in bpy.data.objects:
        ob.select_set(ob.type in ("ARMATURE", "MESH"))
    bpy.context.view_layer.objects.active = next(o for o in bpy.data.objects if o.type == "ARMATURE")
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    for act in list(bpy.data.actions):
        bpy.data.actions.remove(act)
scn = bpy.context.scene
scn.render.fps = 60
scn.unit_settings.scale_length = 1.0
arm = next(o for o in bpy.data.objects if o.type == "ARMATURE")
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
sticks = [o for o in meshes if "stick" in o.name.lower()]
body = max((o for o in meshes if o not in sticks), key=lambda o: len(o.data.vertices))
for o in sticks:                      # the goalie carries its own stick
    bpy.data.objects.remove(o)
body.name = "SK_Goalie_Body"
arm.animation_data_create()
arm.animation_data.action = None
for pb in arm.pose.bones:
    pb.rotation_mode = "QUATERNION"; pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.scale = (1, 1, 1)

BONES = arm.data.bones
REST_H = {b.name: b.head_local.copy() for b in BONES}
REST_T = {b.name: b.tail_local.copy() for b in BONES}
REST_R = {b.name: b.matrix_local.to_3x3() for b in BONES}
PARENT = {b.name: (b.parent.name if b.parent else None) for b in BONES}
ORDER = sorted(BONES.keys(), key=lambda n: len(BONES[n].parent_recursive))
_sv = (REST_T["stick_socket"] - REST_H["stick_socket"]).normalized()
RS0 = Matrix.Rotation(-math.atan2(_sv.x, -_sv.z), 3, "Y")
I3 = Matrix.Identity(3)
# proportions relative to the s4 figure the pose tables were written for
KX = (REST_H["thigh_l"].z - REST_H["foot_l"].z) / 0.72
KR = REST_H["head"].z / 1.37                       # heights (hands, grip)
KA = ((REST_H["lowerarm_l"] - REST_H["upperarm_l"]).length + (REST_H["hand_l"] - REST_H["lowerarm_l"]).length) / 0.55   # reach
SCL = lambda v: Vector((v[0] * KA, v[1] * KA, v[2] * KR))
UNSCL = lambda v: (v[0] / KA, v[1] / KA, v[2] / KR)
STICK_L, LIE = GK.STICK_G["L"] * KR, GK.STICK_G["lie"]     # the goalie stick scales with the figure (the kit does the same)
ANK0 = REST_H["foot_l"].z


def E(p, r, y):
    return Matrix.Rotation(math.radians(y), 3, "Z") @ Matrix.Rotation(math.radians(p), 3, "Y") @ Matrix.Rotation(math.radians(r), 3, "X")


def stick_abs(yaw, tilt, twist):
    return Matrix.Rotation(math.radians(yaw), 3, "Z") @ Matrix.Rotation(math.radians(-tilt), 3, "Y") @ Matrix.Rotation(math.radians(twist), 3, "Z")


def frame_of(d, pole):
    d = d.normalized()
    p = (pole - d * pole.dot(d))
    if p.length < 1e-6:
        p = Vector((0, 0, 1)) - d * d.z
    p.normalize()
    return Matrix((d, p, d.cross(p))).transposed()


def two_bone(S, T, a, b, pole):
    v = T - S
    dist = min(max(v.length, abs(a - b) + 1e-4), a + b - 1e-4)
    dv = v.normalized()
    x = (a * a - b * b + dist * dist) / (2 * dist)
    h = math.sqrt(max(a * a - x * x, 0.0))
    pp = pole - dv * pole.dot(dv)
    pp.normalize()
    return S + dv * x + pp * h, S + dv * dist


def _end(lower):
    return {"calf_l": "foot_l", "calf_r": "foot_r", "lowerarm_l": "hand_l", "lowerarm_r": "hand_r"}[lower]


LIMB = {"leg_l": ("thigh_l", "calf_l", Vector((1, 0, 0))), "leg_r": ("thigh_r", "calf_r", Vector((1, 0, 0))),
        "arm_l": ("upperarm_l", "lowerarm_l", Vector((-1, 0, 0))), "arm_r": ("upperarm_r", "lowerarm_r", Vector((-1, 0, 0)))}
LEN = {k: ((REST_H[v[1]] - REST_H[v[0]]).length, (REST_H[_end(v[1])] - REST_H[v[1]]).length) for k, v in LIMB.items()}
RFRAME = {k: (frame_of(REST_H[v[1]] - REST_H[v[0]], v[2]), frame_of(REST_H[_end(v[1])] - REST_H[v[1]], v[2])) for k, v in LIMB.items()}
GRIP_OFF = REST_H["stick_socket"] - REST_H["hand_r"]
GRIP_OFF_L = Vector((GRIP_OFF.x, -GRIP_OFF.y, GRIP_OFF.z))
HF_REST = {s: GK.hand_frame(REST_H, REST_T, s) for s in ("l", "r")}
_RB = Matrix.Rotation(math.radians(LIE), 3, "Y")
BLADE_PTS = [Vector((0, 0, -STICK_L)) + _RB @ Vector((x, 0, 0)) for x in (-0.03, GK.STICK_G["blade_len"] - 0.03)]


def hand_W(side, d, b):
    return GK.frame_from(Vector(d), Vector(b)) @ HF_REST[side].transposed()


class Solver:
    def __init__(self, pose):
        self.p = pose; self.W = {}; self.H = {}; self.Q = {}; self.err = 0.0

    def place(self, name, Q):
        par = PARENT[name]
        Wp = self.W[par] if par else I3
        Hp = self.H[par] if par else Vector((0, 0, 0))
        Rp = REST_H[par] if par else Vector((0, 0, 0))
        self.Q[name] = Q
        self.W[name] = Wp @ Q
        self.H[name] = Hp + Wp @ (REST_H[name] - Rp)
        if name == "pelvis":
            self.H[name] = self.H[name] + Vector((0, 0, self.p["pz"] * KX))

    def head_of(self, name):
        par = PARENT[name]
        return self.H[par] + self.W[par] @ (REST_H[name] - REST_H[par])

    def limb(self, key, target, pole, roll=None):
        up, lo, _ = LIMB[key]
        a, b = LEN[key]
        S = self.head_of(up)
        K, Tt = two_bone(S, target, a, b, pole)
        self.err = max(self.err, (Tt - target).length)
        Mu_r, Ml_r = RFRAME[key]
        Wu = frame_of(K - S, roll if roll is not None else pole) @ Mu_r.transposed()
        Wl = frame_of(Tt - K, roll if roll is not None else pole) @ Ml_r.transposed()
        self.place(up, self.W[PARENT[up]].transposed() @ Wu)
        self.place(lo, Wu.transposed() @ Wl)

    def solve(self):
        """Solve, then the ice clamp: if a pad or a skate is below the ice, lift the pelvis and both ankle targets by the deficit
        (a pure vertical translation of the legs) and solve again."""
        self._solve()
        self.lift = 0.0
        self.spread = 0
        if "KITV" in globals() and not getattr(self, "_nospread", False):
            # pad clearance: while the two pad slabs overlap by more than 0.8 cm, turn both knee poles outward a little
            for _ in range(12):
                if sat_overlap(obb(self, "L"), obb(self, "R")) <= 0.008:
                    break
                p = dict(self.p)
                kl, kr = Vector(self._pole["l"]), Vector(self._pole["r"])
                p["kpl"] = tuple(kl.normalized() + Vector((0, 0.06, 0)))
                p["kpr"] = tuple(kr.normalized() + Vector((0, -0.06, 0)))
                self.__init__(p)
                self._solve()
                self.spread += 1
        if "KITV" in globals():
            low = min(self.low_legs(), 9.0)
            if self.p["wfl"] > 0.9 and self.p["wfr"] > 0.9 and low > 0.0:
                low = -low      # butterfly: both pads are meant to lie on the ice, so lower the legs onto it as well
                p = dict(self.p)
                dz = -low / KX
                p["pz"] = p["pz"] - dz
                p["fl"] = tuple(p["fl"][:2]) + (p["fl"][2] - dz,) + tuple(p["fl"][3:])
                p["fr"] = tuple(p["fr"][:2]) + (p["fr"][2] - dz,) + tuple(p["fr"][3:])
                self.__init__(p)
                self._solve()
                self.lift = low
                low = min(self.low_legs(), 9.0)
            if low < 0.0:
                p = dict(self.p)
                dz = -low / KX
                p["pz"] = p["pz"] + dz
                p["fl"] = tuple(p["fl"][:2]) + (p["fl"][2] + dz,) + tuple(p["fl"][3:])
                p["fr"] = tuple(p["fr"][:2]) + (p["fr"][2] + dz,) + tuple(p["fr"][3:])
                self.__init__(p)
                self._solve()
                self.lift = -low
        return self

    def low_legs(self):
        zs = []
        for name in ("Pad_L", "Pad_R"):
            bone, v = KITV[name]
            zs.append((np.array(self.H[bone]) + v @ world_rot(self, bone).T)[:, 2].min())
        for bone in ("foot_l", "foot_r"):
            if bone in BODYV:
                zs.append((np.array(self.H[bone]) + BODYV[bone] @ world_rot(self, bone).T)[:, 2].min())
        return float(min(zs))

    def _solve(self):
        p = self.p
        self.place("root", I3)
        self.place("pelvis", E(*p["pel"]))
        for n, k in (("spine_01", "sp1"), ("spine_02", "sp2"), ("spine_03", "sp3"), ("neck_01", "neck"), ("head", "head")):
            self.place(n, E(*p[k]))
        self.place("head_socket", I3)
        self.place("clavicle_l", E(*p["clav_l"]))
        self.place("clavicle_r", E(*p["clav_r"]))
        for side in ("l", "r"):
            f = p["f" + side]
            ank = Vector((f[0] * KX, f[1] * KX, ANK0 + (f[2] - 0.13) * KX))
            Wabs = E(f[4], f[5], f[3])
            s = 1 if side == "l" else -1
            kp = Vector(p["kp" + side])
            pole = kp if kp.length > 1e-6 else Matrix.Rotation(math.radians(f[3]), 3, "Z") @ Vector((1, s * p["k" + side], 0))
            self._pole = getattr(self, "_pole", {}); self._pole[side] = tuple(pole)
            rp = Vector(p["rp" + side])
            self.limb("leg_" + side, ank, pole, rp if rp.length > 1e-6 else None)
            w = p["wf" + side]
            Wc = self.W["calf_" + side]
            Wf = Wabs if w <= 0 else (Wabs.to_quaternion().slerp(Wc.to_quaternion(), min(w, 1.0))).to_matrix()
            self.place("foot_" + side, Wc.transposed() @ Wf)
        # stick and blocker arm
        st = p["stick"]
        Ws = stick_abs(st["yaw"], st["tilt"], st["twist"])
        tilt = st["tilt"]
        for _ in range(80):      # stick ice clamp (as s4): tilt the shaft until no blade end is below the ice
            Gt = SCL(st["g"]) if "g" in st else SCL(st["h"]) - STICK_L * (Ws @ Vector((0, 0, -1)))
            if min((Gt + Ws @ q).z for q in BLADE_PTS) >= -1e-4:
                break
            tilt += -0.5 if tilt > LIE else 0.5
            Ws = stick_abs(st["yaw"], tilt, st["twist"])
        G = SCL(st["g"]) if "g" in st else SCL(st["h"]) - STICK_L * (Ws @ Vector((0, 0, -1)))
        Whr = hand_W("r", p["hrd"], p["hrb"])
        wrist = G - Whr @ GRIP_OFF
        self.limb("arm_r", wrist, Vector(p["er"]))
        self.place("hand_r", self.W["lowerarm_r"].transposed() @ Whr)
        self.place("stick_socket", self.W["hand_r"].transposed() @ (Ws @ RS0.transposed()))
        # catching glove arm: hl is the glove's palm point (wrist + the grip offset along the fingers)
        Whl = hand_W("l", p["hld"], p["hlb"])
        tgt = SCL(p["hl"])
        self.limb("arm_l", tgt - Whl @ GRIP_OFF_L, Vector(p["el"]))
        self.place("hand_l", self.W["lowerarm_l"].transposed() @ Whl)
        self.place("puck_socket", I3)
        return self


# ---------------------------------------------------------------- interpolation (as s4)
VKEYS = [("pz", 1), ("pel", 3), ("sp1", 3), ("sp2", 3), ("sp3", 3), ("neck", 3), ("head", 3), ("clav_l", 3), ("clav_r", 3),
         ("fl", 6), ("fr", 6), ("kl", 1), ("kr", 1), ("kpl", 3), ("kpr", 3), ("rpl", 3), ("rpr", 3), ("wfl", 1), ("wfr", 1),
         ("el", 3), ("er", 3), ("hl", 3), ("hld", 3), ("hlb", 3), ("hrd", 3), ("hrb", 3)]


def flatten(pose):
    v = []
    for k, n in VKEYS:
        v += [pose[k]] if n == 1 else list(pose[k])
    st = pose["stick"]
    mode = "h" if "h" in st else "g"
    v += list(st[mode]) + [st["yaw"], st["tilt"], st["twist"]]
    return v, mode


def unflatten(v, mode):
    pose, i = {}, 0
    for k, n in VKEYS:
        pose[k] = v[i] if n == 1 else tuple(v[i:i + n]); i += n
    pose["stick"] = {mode: tuple(v[i:i + 3]), "yaw": v[i + 3], "tilt": v[i + 4], "twist": v[i + 5]}
    return pose


def absolutise(pose):
    s = Solver(pose).solve()
    out = dict(pose)
    st = {k: pose["stick"][k] for k in ("yaw", "tilt", "twist")}
    st["g"] = UNSCL(s.H["stick_socket"])
    out["stick"] = st
    return out


def ease(t, kind):
    return {"lin": t, "in": t * t, "out": 1 - (1 - t) * (1 - t)}.get(kind, t * t * (3 - 2 * t))


def catmull(p0, p1, p2, p3, t):
    t2, t3 = t * t, t * t * t
    return [0.5 * ((2 * b) + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t2 + (-a + 3 * b - 3 * c + d) * t3) for a, b, c, d in zip(p0, p1, p2, p3)]


def sample(clip, f):
    keys = clip["keys"]
    frames = [k[0] for k in keys]
    if clip["loop"]:
        modes = {flatten(k[1])[1] for k in keys}
        ks = keys if len(modes) == 1 else [(k[0], absolutise(k[1]), k[2]) for k in keys]
        vecs = [flatten(k[1]) for k in ks]
        period = clip["frames"]
        f = f % period
        n = len(ks)
        for i in range(n):
            fa = frames[i]; fb = frames[i + 1] if i + 1 < n else period
            if fa <= f < fb or i == n - 1:
                t = (f - fa) / (fb - fa)
                return unflatten(catmull(vecs[(i - 1) % n][0], vecs[i][0], vecs[(i + 1) % n][0], vecs[(i + 2) % n][0], t), vecs[i][1])
    if f <= frames[0]:
        return keys[0][1]
    if f >= frames[-1]:
        return keys[-1][1]
    for i in range(len(keys) - 1):
        if frames[i] <= f <= frames[i + 1]:
            t = (f - frames[i]) / (frames[i + 1] - frames[i])
            (a, ma), (b, mb) = flatten(keys[i][1]), flatten(keys[i + 1][1])
            if ma != mb:
                a = flatten(absolutise(keys[i][1]))[0]; b = flatten(absolutise(keys[i + 1][1]))[0]; mb = "g"
            tt = ease(t, keys[i + 1][2])
            return unflatten([x + (y - x) * tt for x, y in zip(a, b)], mb)


def mk(base, **kw):
    d = dict(base)
    for k, v in kw.items():
        if k == "stick":
            s = dict(d["stick"])
            if "h" in v or "g" in v:
                s.pop("h", None); s.pop("g", None)
            s.update(v); d[k] = s
        else:
            d[k] = v
    return d


def hold_keys(pose, period=30):
    w = mk(pose, pel=(pose["pel"][0] + 0.1, pose["pel"][1], pose["pel"][2]), sp2=(pose["sp2"][0] + 0.1, pose["sp2"][1], pose["sp2"][2]),
           pz=pose["pz"] - 0.0005)
    return [(0, pose, "cr"), (period // 2, w, "cr")]


# ---------------------------------------------------------------- the kit
mats = GK.get_materials(STYLE)
pieces = GK.build(arm, body, STYLE)
objs, attach = GK.realise(pieces, arm, mats, STYLE, RS0)
tri_total = sum(v["triangles"] for v in attach.values())
print("SKG KIT", STYLE, {k: v["triangles"] for k, v in attach.items()}, "total", tri_total)

# local vertex arrays for measurement (piece frame -> bone frame via Rrel)
KITV = {}
for name, ob in objs.items():
    bone = ob.parent_bone
    Rrel = (ob.matrix_basis.to_3x3())
    v = np.array([tuple(Rrel @ x.co) for x in ob.data.vertices])
    KITV[name] = (bone, v)
PADBOX = {}
for side in ("L", "R"):
    pc = next(p for p in pieces if p.name == "Pad_" + side)
    vs = pc.parts[0][0]
    Ri = REST_R[pc.bone].transposed()
    loc = np.array([tuple(Ri @ (x - REST_H[pc.bone])) for x in vs])
    PADBOX[side] = (pc.bone, loc.min(0), loc.max(0))
BODYV = {}
for vg in body.vertex_groups:
    if vg.name not in REST_H:
        continue
    idx = vg.index
    pts = [v.co for v in body.data.vertices if any(g.group == idx and g.weight > 0.5 for g in v.groups)]
    if pts:
        Ri = REST_R[vg.name].transposed()
        BODYV[vg.name] = np.array([tuple(Ri @ (p - REST_H[vg.name])) for p in pts])


def world_rot(s, b):
    return np.array(s.W[b] @ REST_R[b])


def obb(s, side):
    bone, mn, mx = PADBOX[side]
    Rw = world_rot(s, bone)
    c_loc = (mn + mx) / 2
    c = np.array(s.H[bone]) + Rw @ c_loc
    return c, Rw, (mx - mn) / 2


def sat_overlap(a, b):
    """Penetration depth of two OBBs (min overlap over the 15 axes); negative = separated (largest separation found)."""
    ca, Ra, ea = a; cb, Rb, eb = b
    axes = [Ra[:, i] for i in range(3)] + [Rb[:, i] for i in range(3)]
    axes += [np.cross(Ra[:, i], Rb[:, j]) for i in range(3) for j in range(3)]
    best = 1e9
    d = cb - ca
    for ax in axes:
        n = np.linalg.norm(ax)
        if n < 1e-6:
            continue
        ax = ax / n
        ra = sum(ea[i] * abs(ax @ Ra[:, i]) for i in range(3))
        rb = sum(eb[i] * abs(ax @ Rb[:, i]) for i in range(3))
        ov = ra + rb - abs(d @ ax)
        best = min(best, ov)
    return best


GRID = np.array([(u, v) for u in np.linspace(-1, 1, 13) for v in np.linspace(-1, 1, 13)])


def obb_surface(o):
    c, R, e = o
    pts = []
    for ax in range(3):
        i, j = [k for k in range(3) if k != ax]
        for sgn in (-1, 1):
            P = np.zeros((len(GRID), 3))
            P[:, ax] = sgn * e[ax]; P[:, i] = GRID[:, 0] * e[i]; P[:, j] = GRID[:, 1] * e[j]
            pts.append(P)
    P = np.vstack(pts)
    return c + P @ R.T


def five_hole(s):
    """Smallest distance between the two pad slabs within 15 cm of the ice (0 if they touch or overlap)."""
    a, b = obb(s, "L"), obb(s, "R")
    pa, pb = obb_surface(a), obb_surface(b)
    pa, pb = pa[pa[:, 2] < 0.15], pb[pb[:, 2] < 0.15]
    if len(pa) == 0 or len(pb) == 0:
        return None
    if sat_overlap(a, b) > 0:
        return 0.0
    d = np.sqrt(((pa[:, None, :] - pb[None, :, :]) ** 2).sum(-1)).min()
    return float(d)


def lowest(s):
    out = {}
    for name, (bone, v) in KITV.items():
        z = (np.array(s.H[bone]) + v @ world_rot(s, bone).T)[:, 2].min()
        out[name] = float(z)
    for bone, v in BODYV.items():
        z = (np.array(s.H[bone]) + v @ world_rot(s, bone).T)[:, 2].min()
        out["body:" + bone] = float(z)
    return out


# ---------------------------------------------------------------- goalie pose tables (s4 proportions, metres, degrees)
# hrd/hrb: blocker hand's finger direction and back-of-hand normal (the board faces along hrb); hld/hlb the same for the catching
# glove (the pocket faces along -hlb). hl: the glove's palm point. kpl/kpr: knee pole (direction from the hip towards where the knee
# should go), rpl/rpr: calf roll pole (where the pad's face points), wfl/wfr: 1 = the skate keeps its rest angle to the calf.
Z3 = (0.0, 0.0, 0.0)
STANCE = dict(
    pz=-0.20, pel=(16, 0, 0), sp1=(6, 0, 0), sp2=(5, 0, 0), sp3=(2, 0, 0), neck=(-11, 0, 0), head=(-18, 0, 0), clav_l=Z3, clav_r=Z3,
    fl=(0.07, 0.33, 0.13, 14, 0, 7), fr=(0.07, -0.33, 0.13, -14, 0, -7), kl=-0.30, kr=-0.30,
    kpl=(1.0, -0.10, 0.0), kpr=(1.0, 0.10, 0.0), rpl=(1.0, -0.05, 0.0), rpr=(1.0, 0.05, 0.0), wfl=0.0, wfr=0.0,
    el=(-0.2, 1.0, -0.6), er=(-0.3, -1.0, -0.4),
    hl=(0.36, 0.44, 0.74), hld=(0.20, 0.10, 1.0), hlb=(-1.0, 0.25, 0.15),
    hrd=(0.25, 0.25, -1.0), hrb=(1.0, -0.35, 0.3),
    stick={"h": (0.44, -0.10, 0.0), "yaw": 72, "tilt": LIE, "twist": 0})
STANCE_B = mk(STANCE, pz=-0.212, sp2=(6.5, 0, 0), hl=(0.36, 0.44, 0.73))


def bfly_legs(py=0.10, k_x=0.25, a_x=0.08, a_y=0.49, h=0.17, k_y=0.177):
    """Butterfly legs: knees in front near the centre line, calves along the ice flared out, pads facing forward."""
    out = {}
    for side, s in (("l", 1), ("r", -1)):
        out["f" + side] = (a_x, s * a_y, h, s * 60, 0, 0)
        out["kp" + side] = (k_x, s * py, -0.25)
        cdir = Vector((a_x - k_x, s * (a_y - k_y), 0)).normalized()
        n = Vector((-cdir.y * s, cdir.x * s, 0)) * s       # perpendicular, forward
        if n.x < 0:
            n = -n
        out["rp" + side] = (n.x, n.y, 0.25)
        out["wf" + side] = 1.0
    return out



FIT_LOG = {}


def fit_bfly(name, base):
    """Fit the butterfly knee pole (py) and skate position (a_x) to this figure and kit: pads overlapping by 0 to 0.8 cm at the
    knees (the five-hole sealed) and as little overlap as possible otherwise; falls back to the smallest overlap."""
    best = None
    for py in [i * 0.025 for i in range(0, 13)]:
        for ax in [i * 0.02 for i in range(-2, 11)]:
            s = Solver(mk(base, **bfly_legs(py=py, a_x=ax))).solve()
            ov = sat_overlap(obb(s, "L"), obb(s, "R"))
            gap = five_hole(s) or 0.0
            cost = abs(ov - 0.004) + 2.0 * gap + 0.002 * s.lift
            if best is None or cost < best[0]:
                best = (cost, py, ax, ov, gap)
    FIT_LOG[name] = dict(py=best[1], a_x=round(best[2], 3), pad_overlap_cm=round(best[3] * 100, 2), five_hole_gap_cm=round(best[4] * 100, 2))
    print("SKG FIT", name, FIT_LOG[name])
    return dict(py=best[1], a_x=best[2])


BFLY_BASE = mk(STANCE, pz=-0.47, pel=(8, 0, 0), sp1=(6, 0, 0), sp2=(6, 0, 0), sp3=(4, 0, 0), neck=(-8, 0, 0), head=(-14, 0, 0),
               hl=(0.36, 0.50, 0.64), hld=(0.20, 0.10, 1.0), hlb=(-1.0, 0.25, 0.1),
               stick={"h": (0.50, -0.10, 0.0), "yaw": 74, "tilt": LIE, "twist": 0}, **bfly_legs())
BFLY = mk(BFLY_BASE, **bfly_legs(**fit_bfly("butterfly", BFLY_BASE)))
BFLY_B = mk(BFLY, pz=-0.475, sp2=(7.0, 0, 0), hl=(0.36, 0.50, 0.63))
GLOVE_SAVE = mk(STANCE, pz=-0.26, pel=(14, -8, 10), sp1=(4, -6, 6), sp2=(2, -4, 4), neck=(-10, 4, 0), head=(-14, 6, 12),
                clav_l=(0, -20, 15), hl=(0.34, 0.66, 1.12), hld=(0.15, 0.35, 1.0), hlb=(-1.0, 0.2, 0.2), el=(-0.6, 0.4, -0.8),
                fl=(0.04, 0.38, 0.13, 18, 0, 4))
BLOCKER_SAVE = mk(STANCE, pz=-0.27, pel=(14, 8, -10), sp1=(4, 6, -6), sp2=(2, 4, -4), neck=(-10, -4, 0), head=(-14, -6, -12),
                  clav_r=(0, 20, -15), hrd=(0.1, -0.1, -1.0), hrb=(1.0, -0.5, 0.15), er=(-0.6, -0.5, -0.8),
                  stick={"g": (0.30, -0.70, 0.98), "yaw": 70, "tilt": 22, "twist": 0}, fr=(0.04, -0.38, 0.13, -18, 0, -4))


def pad_save(s):
    """Kick save towards the low corner on side s (+1 = the goalie's left): that leg slides out straight along the ice, pad facing
    the shooter; the other knee drops into a half butterfly; the body leans over the kick."""
    kick = (0.16, s * 0.80, 0.14, s * 25, 0, 0)
    bl = bfly_legs(py=0.05, a_x=-0.12, a_y=0.36)   # the other knee drops near the centre, its calf tucked back under the body
    o = "r" if s == 1 else "l"
    k = "l" if s == 1 else "r"
    d = dict(pz=-0.42, pel=(10, -s * 20, s * 12), sp1=(6, -s * 10, s * 6), sp2=(4, -s * 6, 4 * s), neck=(-8, s * 8, 0), head=(-12, s * 10, s * 20))
    d["f" + k] = kick
    d["kp" + k] = (0.35, s * 0.6, 0.6)
    d["rp" + k] = (1.0, 0.0, 0.25)
    d["wf" + k] = 0.7
    d["k" + k] = 0.0
    for key in ("f", "kp", "rp", "wf"):
        d[key + o] = bl[key + o]
    if s == 1:   # glove side: the glove comes down to cover above the pad, the stick seals the ice between
        d.update(hl=(0.38, 0.62, 0.42), hld=(0.4, 0.4, 1.0), hlb=(-1.0, 0.3, 0.1),
                 stick={"h": (0.52, 0.05, 0.0), "yaw": 78, "tilt": LIE, "twist": 0})
    else:        # blocker side: the paddle goes down along the kicking pad
        d.update(stick={"h": (0.50, -0.14, 0.0), "yaw": 72, "tilt": LIE, "twist": 0}, hrd=(0.3, 0.2, -1.0), hrb=(1.0, -0.4, 0.3),
                 hl=(0.40, 0.42, 0.62))
    return mk(STANCE, **d)


POKE = mk(STANCE, pz=-0.38, pel=(34, 4, -8), sp1=(10, 2, -6), sp2=(8, 0, -4), sp3=(6, 0, 0), neck=(-22, 0, 0), head=(-30, 0, 6),
          clav_r=(25, 0, 20), er=(-0.2, -1.0, 0.2), hrd=(0.6, 0.0, -0.8), hrb=(0.6, -0.6, 0.6),
          stick={"h": (1.22, -0.12, 0.0), "yaw": 6, "tilt": 62, "twist": 0},
          fl=(0.10, 0.34, 0.13, 18, 0, 6), fr=(-0.12, -0.30, 0.13, -14, 0, -8), kl=-0.2, kr=-0.1, hl=(0.20, 0.46, 0.66))
FREEZE = mk(BFLY, pz=-0.50, pel=(38, 0, 0), sp1=(12, 0, 0), sp2=(10, 0, 0), sp3=(6, 0, 0), neck=(-24, 0, 0), head=(-34, 0, 0),
            hl=(0.58, 0.06, 0.14), hld=(1.0, 0.0, -0.05), hlb=(0.05, 0.0, 1.0), el=(0.0, 1.0, 0.5),
            stick={"h": (0.62, -0.30, 0.0), "yaw": 85, "tilt": LIE, "twist": 0}, hrd=(0.5, 0.3, -1.0), hrb=(1.0, -0.2, 0.4))
FREEZE = mk(FREEZE, **bfly_legs(**fit_bfly("freeze", FREEZE)))
BEATEN_A = mk(STANCE, pz=-0.14, pel=(8, 0, 0), sp1=(4, 0, 0), sp2=(2, 0, 0), sp3=(0, 0, 0), neck=(4, 0, -20), head=(-4, 0, -40),
              fl=(0.02, 0.26, 0.13, 10, 0, 0), fr=(0.02, -0.26, 0.13, -10, 0, 0), kpl=(1.0, 0.05, 0.0), kpr=(1.0, -0.05, 0.0),
              hl=(0.10, 0.42, 0.62), hld=(0.0, 0.3, -1.0), hlb=(-1.0, 0.0, 0.0), el=(-0.5, 1.0, 0.0),
              stick={"h": (0.40, -0.15, 0.0), "yaw": 70, "tilt": 30, "twist": 0}, hrd=(0.2, -0.3, -1.0), hrb=(0.6, -1.0, 0.0))
BEATEN_B = mk(BEATEN_A, pz=-0.20, pel=(22, 0, 0), sp1=(14, 0, 0), sp2=(12, 0, 0), sp3=(8, 0, 0), neck=(26, 0, 0), head=(20, 0, 0),
              clav_l=(10, 12, 0), clav_r=(10, -12, 0), hl=(0.12, 0.40, 0.48),
              stick={"h": (0.55, -0.20, 0.0), "yaw": 30, "tilt": 55, "twist": 0})
CELEB = mk(STANCE, pz=-0.22, pel=(10, 0, 0), sp1=(2, 0, 0), sp2=(0, 0, 0), neck=(-14, 0, 0), head=(-22, 0, 0),
           hl=(0.22, 0.34, 1.32), hld=(0.0, 0.0, 1.0), hlb=(-1.0, 0.0, 0.0), el=(0.0, 1.0, -0.6))
CELEB_B = mk(CELEB, pz=-0.25, hl=(0.26, 0.36, 1.12))


def shuffle_key(s, k):
    """Lateral shuffle towards side s, in place: the lead skate pushes out, the trail skate draws in; pads stay square."""
    lead, trail = ("l", "r") if s == 1 else ("r", "l")
    ys = {0: (0.33, 0.33, 0.13, 0.13), 1: (0.50, 0.28, 0.16, 0.13), 2: (0.40, 0.44, 0.13, 0.13), 3: (0.26, 0.30, 0.13, 0.17)}[k]
    yl, yt, zl, zt = ys
    sl = 1 if lead == "l" else -1
    d = {"f" + lead: (0.02, sl * yl, zl, sl * 14, 0, sl * 7), "f" + trail: (0.02, -sl * yt, zt, -sl * (14 + 8 * (k == 2)), 0, -sl * (7 + 10 * (k == 2)))}
    bob = (-0.30, -0.315, -0.305, -0.29)[k]
    return mk(STANCE, pz=bob, pel=(16, -s * 6 * (k in (1, 2)), s * 4 * (k in (1, 2))), sp1=(6, -s * 3 * (k in (1, 2)), 0), **d)


CLIPS = {
    "Goalie_Stance": dict(frames=60, loop=True, keys=[(0, STANCE, "cr"), (30, STANCE_B, "cr")]),
    "Goalie_ShuffleL": dict(frames=24, loop=True, keys=[(6 * k, shuffle_key(1, k), "cr") for k in range(4)]),
    "Goalie_ShuffleR": dict(frames=24, loop=True, keys=[(6 * k, shuffle_key(-1, k), "cr") for k in range(4)]),
    "Goalie_Butterfly_Drop": dict(frames=14, loop=False, keys=[(0, STANCE, "io"), (10, BFLY, "out"), (14, BFLY, "io")]),
    "Goalie_Butterfly_Hold": dict(frames=30, loop=True, keys=[(0, BFLY, "cr"), (15, BFLY_B, "cr")]),
    "Goalie_Butterfly_Recover": dict(frames=26, loop=False, keys=[(0, BFLY, "io"), (12, mk(BFLY, pz=-0.40, **{k: STANCE[k] for k in ("fr", "kpr", "rpr", "wfr", "kr")}), "io"),
                                                             (26, STANCE, "io")]),
    "Goalie_GloveSave": dict(frames=42, loop=False, keys=[(0, STANCE, "io"), (10, GLOVE_SAVE, "out"), (16, GLOVE_SAVE, "io"), (42, STANCE, "io")]),
    "Goalie_BlockerSave": dict(frames=42, loop=False, keys=[(0, STANCE, "io"), (10, BLOCKER_SAVE, "out"), (16, BLOCKER_SAVE, "io"), (42, STANCE, "io")]),
    "Goalie_PadSaveL": dict(frames=46, loop=False, keys=[(0, STANCE, "io"), (11, pad_save(1), "out"), (17, pad_save(1), "io"), (46, STANCE, "io")]),
    "Goalie_PadSaveR": dict(frames=46, loop=False, keys=[(0, STANCE, "io"), (11, pad_save(-1), "out"), (17, pad_save(-1), "io"), (46, STANCE, "io")]),
    "Goalie_PokeCheck": dict(frames=40, loop=False, keys=[(0, STANCE, "io"), (9, POKE, "out"), (13, POKE, "io"), (40, STANCE, "io")]),
    "Goalie_Freeze": dict(frames=24, loop=False, keys=[(0, STANCE, "io"), (12, FREEZE, "out"), (18, mk(FREEZE, pz=-0.505), "io"), (24, FREEZE, "io")]),
    "Goalie_Beaten": dict(frames=60, loop=False, keys=[(0, STANCE, "io"), (14, BEATEN_A, "out"), (36, BEATEN_B, "io"), (60, mk(BEATEN_B, pz=-0.21), "io")]),
    "Goalie_Celebrate_Save": dict(frames=36, loop=False, keys=[(0, STANCE, "io"), (8, CELEB, "out"), (14, CELEB_B, "io"), (20, CELEB, "io"),
                                                               (36, STANCE, "io")]),
    # s4 names kept working: Goalie_Hold = Goalie_Stance's pose as a 30-frame hold; the blocks drop into the pad save and hold it
    "Goalie_Hold": dict(frames=30, loop=True, keys=hold_keys(STANCE)),
    "Goalie_Block_Left": dict(frames=24, loop=False, keys=[(0, STANCE, "io"), (11, pad_save(1), "out"), (24, pad_save(1), "io")]),
    "Goalie_Block_Right": dict(frames=24, loop=False, keys=[(0, STANCE, "io"), (11, pad_save(-1), "out"), (24, pad_save(-1), "io")]),
}
KEYFRAME = {"Goalie_Stance": 0, "Goalie_ShuffleL": 6, "Goalie_ShuffleR": 6, "Goalie_Butterfly_Drop": 10, "Goalie_Butterfly_Hold": 0,
            "Goalie_Butterfly_Recover": 12, "Goalie_GloveSave": 10, "Goalie_BlockerSave": 10, "Goalie_PadSaveL": 11, "Goalie_PadSaveR": 11,
            "Goalie_PokeCheck": 9, "Goalie_Freeze": 24, "Goalie_Beaten": 60, "Goalie_Celebrate_Save": 8, "Goalie_Hold": 0,
            "Goalie_Block_Left": 24, "Goalie_Block_Right": 24}
ONLY = os.environ.get("SKG_CLIPS")
if ONLY:
    CLIPS = {k: v for k, v in CLIPS.items() if k in ONLY.split(",")}

# ---------------------------------------------------------------- bake + measure
def bake(name, clip):
    act = bpy.data.actions.new(name)
    act.use_fake_user = True
    nfr = clip["frames"] + 1
    chans, prevq = {}, {}
    meas = dict(spread=0, lift=0.0, ik_err=0.0, pad_overlap=-9.0, low_pad=9.0, low_skate=9.0, low_all=9.0, low_stick=9.0, five_hole=[], pel=[])
    for f in range(nfr):
        pose = sample(clip, 0 if (clip["loop"] and f == nfr - 1) else f)
        s = Solver(pose).solve()
        meas["ik_err"] = max(meas["ik_err"], s.err); meas["lift"] = max(meas["lift"], s.lift); meas["spread"] = max(meas["spread"], s.spread)
        meas["pad_overlap"] = max(meas["pad_overlap"], sat_overlap(obb(s, "L"), obb(s, "R")))
        lo = lowest(s)
        meas["low_pad"] = min(meas["low_pad"], lo["Pad_L"], lo["Pad_R"])
        meas["low_skate"] = min(meas["low_skate"], lo.get("body:foot_l", 9), lo.get("body:foot_r", 9))
        meas["low_stick"] = min(meas["low_stick"], lo["Stick"])
        meas["low_all"] = min(meas["low_all"], min(lo.values()))
        meas["pel"].append(s.H["pelvis"].copy())
        if f == KEYFRAME.get(name, -1) or (clip["loop"] and f == 0):
            meas["key_low"] = {k: round(v * 100, 2) for k, v in lo.items() if not k.startswith("body:") or "foot" in k}
            meas["key_five_hole"] = five_hole(s)
            meas["key_pad_overlap"] = sat_overlap(obb(s, "L"), obb(s, "R"))
        for bn in ORDER:
            if bn == "root":
                continue
            R = REST_R[bn]
            q = (R.transposed() @ s.Q[bn] @ R).to_quaternion()
            if bn in prevq and q.dot(prevq[bn]) < 0:
                q.negate()
            prevq[bn] = q
            chans.setdefault((bn, "rotation_quaternion"), []).append(list(q))
            loc = Vector((0, 0, s.p["pz"] * KX)) if bn == "pelvis" else Vector((0, 0, 0))
            chans.setdefault((bn, "location"), []).append(list(R.transposed() @ loc))
    if clip["loop"]:
        for k in chans:
            chans[k][-1] = list(chans[k][0])
    for (bn, prop), vals in chans.items():
        for i in range(len(vals[0])):
            fc = act.fcurves.new('pose.bones["%s"].%s' % (bn, prop), index=i, action_group=bn)
            fc.keyframe_points.add(nfr)
            co = []
            for f in range(nfr):
                co += [float(f), vals[f][i]]
            fc.keyframe_points.foreach_set("co", co)
            for kp in fc.keyframe_points:
                kp.interpolation = "LINEAR"
            fc.update()
    act.use_frame_range = True
    act.frame_start, act.frame_end = 0, clip["frames"]
    act.use_cyclic = clip["loop"]
    p0 = meas["pel"][0]
    rep = dict(frames=clip["frames"] + 1, seconds=round(clip["frames"] / 60, 3), loop=clip["loop"], key_frame=KEYFRAME.get(name),
               key_seconds=round(KEYFRAME.get(name, 0) / 60, 3),
               pelvis_horizontal_max_cm=round(max(math.hypot(p.x - p0.x, p.y - p0.y) for p in meas["pel"]) * 100, 4),
               pelvis_z_range_cm=round((max(p.z for p in meas["pel"]) - min(p.z for p in meas["pel"])) * 100, 2),
               ik_miss_max_cm=round(meas["ik_err"] * 100, 2), ice_clamp_lift_max_cm=round(meas["lift"] * 100, 2), knee_spread_steps_max=meas["spread"],
               pad_overlap_max_cm=round(meas["pad_overlap"] * 100, 2), lowest_pad_cm=round(meas["low_pad"] * 100, 2),
               lowest_skate_cm=round(meas["low_skate"] * 100, 2), lowest_stick_cm=round(meas["low_stick"] * 100, 2),
               lowest_any_cm=round(meas["low_all"] * 100, 2),
               key_five_hole_gap_cm=None if meas.get("key_five_hole") is None else round(meas["key_five_hole"] * 100, 2),
               key_pad_overlap_cm=round(meas.get("key_pad_overlap", 0) * 100, 2), key_lowest_cm=meas.get("key_low"))
    return act, rep


acts, report = [], {}
for n, c in CLIPS.items():
    a, r = bake(n, c)
    acts.append(a); report[n] = r
    print("SKG CLIP", n, json.dumps({k: v for k, v in r.items() if k != "key_lowest_cm"}))
report["_kit"] = {"butterfly_fit": FIT_LOG, "style": STYLE, "skeleton": SKEL, "kx": round(KX, 4), "kr": round(KR, 4), "triangles_total": tri_total,
                  "pieces": {k: {"bone": v["bone"], "triangles": v["triangles"]} for k, v in attach.items()}}
with open(os.path.join(SKG, "report_%s.json" % STYLE), "w") as fh:
    json.dump(report, fh, indent=1)

# ---------------------------------------------------------------- outputs
EXP = os.path.join(SKG, "export")
SEXP = os.path.join(EXP, STYLE)
os.makedirs(SEXP, exist_ok=True)
os.makedirs(os.path.join(SKG, "preview"), exist_ok=True)
meta = {"style": STYLE, "skeleton": os.path.basename(SKEL), "units": "piece FBX files are centimetres (UnitScaleFactor 1), axes as SK_Skater_S4",
        "attach": "attach each SM_Goalie_<Piece> to its bone (or to a socket on that bone with zero offset) with the relative rotation below; "
                  "every piece but the stick is authored in its bone's rest frame, so its relative transform is the identity",
        "goalie_stick": {"grip_to_heel_cm": round(STICK_L * 100, 2), "lie_deg": GK.STICK_G["lie"],
                         "shaft_above_grip_cm": GK.STICK_G["above"] * 100, "blade_len_cm": GK.STICK_G["blade_len"] * 100},
        "triangles_total": tri_total, "pieces": attach}
with open(os.path.join(SEXP, "attach.json"), "w") as fh:
    json.dump(meta, fh, indent=1)
if STYLE == "p2_cel":
    with open(os.path.join(EXP, "attach.json"), "w") as fh:
        json.dump(meta, fh, indent=1)

# reference: world bounds of every kit piece at a few frames (checked against the re-imported FBX files by check_kit.py)
REF = {}
for clip, fr in (("Goalie_Stance", 0), ("Goalie_Butterfly_Hold", 0), ("Goalie_GloveSave", 10), ("Goalie_PokeCheck", 9)):
    if clip not in CLIPS:
        continue
    arm.animation_data.action = bpy.data.actions[clip]
    scn.frame_set(fr)
    dg = bpy.context.evaluated_depsgraph_get()
    for name, ob in objs.items():
        ev = ob.evaluated_get(dg)
        pts = [ev.matrix_world @ v.co for v in ev.data.vertices]
        REF["%s:%d:%s" % (clip, fr, name)] = [[round(min(p[i] for p in pts) * 100, 3) for i in range(3)], [round(max(p[i] for p in pts) * 100, 3) for i in range(3)]]
with open(os.path.join(SEXP, "kit_reference_bounds.json"), "w") as fh:
    json.dump(REF, fh, indent=1)

arm.animation_data.action = bpy.data.actions.get("Goalie_Stance") or acts[0]
scn.frame_set(0)
bpy.ops.wm.save_as_mainfile(filepath=os.path.join(SKG, "preview", "goalie_%s.blend" % STYLE))

# kit pieces as static meshes, centimetres (data scaled x100, scene unit 0.01 m, FBX_SCALE_NONE) like the skeletal file
scn.unit_settings.scale_length = 0.01
for name, ob in objs.items():
    me = ob.data.copy()
    me.transform(Matrix.Scale(100.0, 4))
    tmp = bpy.data.objects.new("SM_Goalie_" + name, me) if False else bpy.data.objects.new("tmp_" + name, me)
    scn.collection.objects.link(tmp)
    ob.name = "old_" + name
    tmp.name = "SM_Goalie_" + name
    for o in bpy.context.view_layer.objects:
        o.select_set(o == tmp)
    bpy.context.view_layer.objects.active = tmp
    bpy.ops.export_scene.fbx(filepath=os.path.join(SEXP, "SM_Goalie_%s.fbx" % name), use_selection=True, object_types={"MESH"},
                             apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0, axis_forward="-Z", axis_up="Y",
                             mesh_smooth_type="FACE", use_tspace=False, bake_anim=False)
    bpy.data.objects.remove(tmp)
    ob.name = "SM_Goalie_" + name
scn.unit_settings.scale_length = 1.0
print("SKG KIT EXPORT OK", SEXP)

if EXPORT_SKEL:
    for ob in list(objs.values()):
        bpy.data.objects.remove(ob)
    scn.unit_settings.scale_length = 0.01
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    for eb in arm.data.edit_bones:
        eb.head = eb.head * 100.0; eb.tail = eb.tail * 100.0
    bpy.ops.object.mode_set(mode="OBJECT")
    body.data.transform(Matrix.Scale(100.0, 4))
    for act in bpy.data.actions:
        for fc in act.fcurves:
            if fc.data_path.endswith(".location"):
                for kp in fc.keyframe_points:
                    kp.co.y *= 100.0; kp.handle_left.y *= 100.0; kp.handle_right.y *= 100.0
    arm.animation_data.action = None
    for pb in arm.pose.bones:
        pb.location = (0, 0, 0); pb.rotation_quaternion = (1, 0, 0, 0); pb.scale = (1, 1, 1)
    scn.frame_set(0)
    FBX_ARGS = dict(use_selection=True, object_types={"ARMATURE", "MESH"},
                    apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0,
                    axis_forward="-Z", axis_up="Y", use_space_transform=True, bake_space_transform=False,
                    mesh_smooth_type="FACE", use_tspace=False, use_mesh_modifiers=True,
                    add_leaf_bones=False, primary_bone_axis="Y", secondary_bone_axis="X", armature_nodetype="NULL",
                    use_armature_deform_only=False,
                    bake_anim=True, bake_anim_use_all_bones=False, bake_anim_use_nla_strips=False, bake_anim_use_all_actions=True,
                    bake_anim_force_startend_keying=True, bake_anim_step=1.0, bake_anim_simplify_factor=0.0)
    for o in bpy.context.view_layer.objects:
        o.select_set(o in (arm, body))
    bpy.ops.export_scene.fbx(filepath=os.path.join(SEXP, "SK_Goalie_clips.fbx"), **FBX_ARGS)
    for act in acts:
        if CLIPS[act.name]["loop"]:
            act.frame_end = CLIPS[act.name]["frames"] - 1
    bpy.ops.export_scene.fbx(filepath=os.path.join(SEXP, "SK_Goalie_clips_loopcut.fbx"), **FBX_ARGS)
    print("SKG SKEL EXPORT OK", SEXP)
