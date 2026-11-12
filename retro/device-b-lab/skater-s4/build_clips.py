# S4 scripted clips + export. Opens s4_skater.blend, authors every clip from the pose tables below, exports FBX (all takes) and GLB.
#   blender -b --factory-startup --python build_clips.py -- [lab_dir]
# Writes <lab_dir>/s4_skater_anim.blend, <lab_dir>/export/SK_Skater_S4.fbx, <lab_dir>/export/SK_Skater_S4.glb, <lab_dir>/clips_report.json
#
# Authoring model (all in armature space: metres, X forward, Y left, Z up, degrees):
#   pz            pelvis height offset (only vertical pelvis motion; pelvis x,y are locked at 0: the game moves the pawn)
#   pel, sp1..3, neck, head, clav_l, clav_r : (p, r, y) rotation deltas, each in its parent's posed frame
#                 p > 0 tips the bone's top forward (+X), r > 0 tips it to the figure's right (-Y), y > 0 turns it left
#   fl, fr        ankle targets for leg IK: (x, y, z, yaw, pitch, roll) of the skate; z 0.13 = blade on the ice
#   stick         {"h": heel xyz} or {"g": grip xyz} or {"gs": grip offset from the right shoulder}, plus "yaw", "tilt", "twist":
#                 tilt 0 = shaft vertical, tilt 50 = blade flat on the ice; yaw turns the shaft's lean direction to the left
#                 the right arm is solved by IK to the grip (the stick is rigid on stick_socket, the right hand's child)
#   hl            left hand: ("grip", fraction down the shaft) | ("rel", dx, dy, dz from the left shoulder) | ("abs", x, y, z)
# Each clip: frames, loop flag and keys [(frame, pose, ease)]; pose dicts inherit from a base pose. Loops use cyclic
# Catmull-Rom between keys and copy frame 0 into the last frame (first == last exactly). One-shots use the ease per key.
import bpy, math, os, sys, json
from mathutils import Vector, Matrix, Euler, Quaternion

ARGS = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
LAB = os.path.abspath(ARGS[0]) if ARGS else os.path.dirname(os.path.abspath(__file__))
bpy.ops.wm.open_mainfile(filepath=os.path.join(LAB, "s4_skater.blend"))
scn = bpy.context.scene
scn.render.fps = 60
arm = bpy.data.objects["Armature"]
BONES = arm.data.bones
REST_H = {b.name: b.head_local.copy() for b in BONES}
REST_T = {b.name: b.tail_local.copy() for b in BONES}
REST_R = {b.name: b.matrix_local.to_3x3() for b in BONES}
PARENT = {b.name: (b.parent.name if b.parent else None) for b in BONES}
ORDER = sorted(BONES.keys(), key=lambda n: len(BONES[n].parent_recursive))
STICK_L = 1.40
_sv = (BONES["stick_socket"].tail_local - BONES["stick_socket"].head_local).normalized()
RS0 = Matrix.Rotation(-math.atan2(_sv.x, -_sv.z), 3, "Y")   # rest stick orientation, read from the stick_socket bone
I3 = Matrix.Identity(3)

def E(p, r, y):
    return (Matrix.Rotation(math.radians(y), 3, "Z") @ Matrix.Rotation(math.radians(p), 3, "Y")
            @ Matrix.Rotation(math.radians(r), 3, "X"))

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
    K = S + dv * x + pp * h
    return K, S + dv * dist

def rest_frame(upper, lower, pole_rest):
    return frame_of(REST_H[lower] - REST_H[upper], pole_rest), frame_of(REST_H[_end(lower)] - REST_H[lower], pole_rest)

def _end(lower):
    return {"calf_l": "foot_l", "calf_r": "foot_r", "lowerarm_l": "hand_l", "lowerarm_r": "hand_r"}[lower]

LIMB = {
    "leg_l": ("thigh_l", "calf_l", Vector((1, 0, 0))),
    "leg_r": ("thigh_r", "calf_r", Vector((1, 0, 0))),
    "arm_l": ("upperarm_l", "lowerarm_l", Vector((-1, 0, 0))),
    "arm_r": ("upperarm_r", "lowerarm_r", Vector((-1, 0, 0))),
}
LEN = {k: ((REST_H[v[1]] - REST_H[v[0]]).length, (REST_H[_end(v[1])] - REST_H[v[1]]).length) for k, v in LIMB.items()}
RFRAME = {k: rest_frame(v[0], v[1], v[2]) for k, v in LIMB.items()}
GRIP_OFF = REST_H["stick_socket"] - REST_H["hand_r"]      # wrist -> grip, in rest hand frame (= armature axes at rest)
GRIP_OFF_L = Vector((GRIP_OFF.x, -GRIP_OFF.y, GRIP_OFF.z))
_RB = Matrix.Rotation(math.radians(50.0), 3, "Y")
BLADE_PTS = [Vector((0, 0, -STICK_L)) + _RB @ Vector((x, 0, z)) for x in (-0.03, 0.31) for z in (0.0,)] + [Vector((0, 0, -STICK_L - 0.02))]

DEFAULT = dict(pz=0.0, pel=(0, 0, 0), sp1=(0, 0, 0), sp2=(0, 0, 0), sp3=(0, 0, 0), neck=(0, 0, 0), head=(0, 0, 0),
               clav_l=(0, 0, 0), clav_r=(0, 0, 0),
               fl=(0.0, 0.17, 0.13, 8, 0, 0), fr=(0.0, -0.17, 0.13, -8, 0, 0),
               kl=0.25, kr=0.25,
               stick={"h": (0.75, 0.0, 0.0), "yaw": 15, "tilt": 52, "twist": 0},
               hl=("grip", 0.42), el=(-0.5, 0.9, -0.4), er=(-0.5, -0.9, -0.4))

class Solver:
    """Evaluates a pose dict into per-bone armature-space rotation deltas (Q) and the pelvis offset."""
    def __init__(self, pose):
        self.p = pose
        self.W = {}
        self.H = {}
        self.Q = {}

    def place(self, name, Q):
        par = PARENT[name]
        Wp = self.W[par] if par else I3
        Hp = self.H[par] if par else Vector((0, 0, 0))
        Rp = REST_H[par] if par else Vector((0, 0, 0))
        self.Q[name] = Q
        self.W[name] = Wp @ Q
        self.H[name] = Hp + Wp @ (REST_H[name] - Rp)
        if name == "pelvis":
            self.H[name] = self.H[name] + Vector((0, 0, self.p["pz"]))

    def head_of(self, name):
        par = PARENT[name]
        return self.H[par] + self.W[par] @ (REST_H[name] - REST_H[par])

    def limb(self, key, target, pole, end_frame=None):
        up, lo, _ = LIMB[key]
        a, b = LEN[key]
        S = self.head_of(up)
        K, T = two_bone(S, target, a, b, pole)
        Mu_r, Ml_r = RFRAME[key]
        Wu = frame_of(K - S, pole) @ Mu_r.transposed()
        Wl = frame_of(T - K, pole) @ Ml_r.transposed()
        Wp = self.W[PARENT[up]]
        self.place(up, Wp.transposed() @ Wu)
        self.place(lo, Wu.transposed() @ Wl)

    def solve(self):
        p = self.p
        self.place("root", I3)
        self.place("pelvis", E(*p["pel"]))
        for n, k in (("spine_01", "sp1"), ("spine_02", "sp2"), ("spine_03", "sp3"), ("neck_01", "neck"), ("head", "head")):
            self.place(n, E(*p[k]))
        self.place("head_socket", I3)
        self.place("clavicle_l", E(*p["clav_l"]))
        self.place("clavicle_r", E(*p["clav_r"]))
        # legs
        for side in ("l", "r"):
            f = p["f" + side]
            ank = Vector(f[:3])
            Wf = E(f[4], f[5], f[3])
            # ice clamp: raise the ankle until no blade end (toe, heel, both edges) is below z = 0
            low = min((Wf @ Vector((dx, dy, -0.13))).z for dx in (0.195, -0.115) for dy in (0.006, -0.006))
            ank.z = max(ank.z, -low)
            s = 1 if side == "l" else -1
            pole = Matrix.Rotation(math.radians(f[3]), 3, "Z") @ Vector((1, s * p["k" + side], 0))
            self.limb("leg_" + side, ank, pole)
            self.place("foot_" + side, self.W["calf_" + side].transposed() @ Wf)
        # stick and right arm
        st = p["stick"]
        Ws = stick_abs(st["yaw"], st["tilt"], st["twist"])
        G = self.grip_of(st, Ws)
        # ice clamp: if the blade would cut the ice, tilt the stick up (towards horizontal) until it clears
        tilt = st["tilt"]
        for _ in range(60):
            if min((G + Ws @ q).z for q in BLADE_PTS) >= 0.0:
                break
            tilt += 1.0 if tilt < 90 else -1.0
            Ws = stick_abs(st["yaw"], tilt, st["twist"])
        er = Vector(p["er"])
        Sr = self.head_of("upperarm_r")
        wrist_t = G - (G - Sr).normalized() * GRIP_OFF.length
        self.limb("arm_r", wrist_t, er)
        self.place("hand_r", I3)
        self.place("stick_socket", self.W["hand_r"].transposed() @ (Ws @ RS0.transposed()))
        # left arm
        el = Vector(p["el"])
        Sl = self.head_of("upperarm_l")
        tgt = self.left_target(p["hl"], Sl)
        wrist_l = tgt - (tgt - Sl).normalized() * GRIP_OFF_L.length
        self.limb("arm_l", wrist_l, el)
        self.place("hand_l", I3)
        self.place("puck_socket", I3)
        return self

    def grip_of(self, st, Ws):
        if "g" in st:
            return Vector(st["g"])
        if "gs" in st:
            return self.head_of("upperarm_r") + Vector(st["gs"])
        return Vector(st["h"]) - STICK_L * (Ws @ Vector((0, 0, -1)))

    def stick_point(self, frac):
        G = self.H["stick_socket"]
        Wst = self.W["stick_socket"] @ RS0
        return G + Wst @ Vector((0, 0, -STICK_L * frac))

    def left_target(self, hl, Sl):
        if hl[0] == "grip":
            G = self.H["stick_socket"] if "stick_socket" in self.H else None
            return self.stick_point(hl[1])
        if hl[0] == "rel":
            return Sl + Vector(hl[1:4])
        return Vector(hl[1:4])

# ---------------------------------------------------------------- interpolation
def ease(t, kind):
    if kind == "lin":
        return t
    if kind == "in":
        return t * t
    if kind == "out":
        return 1 - (1 - t) * (1 - t)
    return t * t * (3 - 2 * t)      # "io"

def flatten(pose):
    """pose dict -> (vector of numbers, structure) so poses interpolate as plain numbers."""
    st = pose["stick"]
    mode = "h" if "h" in st else ("g" if "g" in st else "gs")
    hl = pose["hl"]
    v = [pose["pz"]]
    for k in ("pel", "sp1", "sp2", "sp3", "neck", "head", "clav_l", "clav_r", "fl", "fr", "el", "er"):
        v += list(pose[k])
    v += [pose["kl"], pose["kr"]]
    v += list(st[mode]) + [st["yaw"], st["tilt"], st["twist"]]
    v += list(hl[1:]) + [0.0] * (3 - len(hl[1:]))
    return v, (mode, hl[0])

def unflatten(v, struct):
    mode, hmode = struct
    i = 0
    pose = {"pz": v[0]}; i = 1
    for k, n in (("pel", 3), ("sp1", 3), ("sp2", 3), ("sp3", 3), ("neck", 3), ("head", 3), ("clav_l", 3), ("clav_r", 3),
                 ("fl", 6), ("fr", 6), ("el", 3), ("er", 3)):
        pose[k] = tuple(v[i:i + n]); i += n
    pose["kl"], pose["kr"] = v[i], v[i + 1]; i += 2
    pose["stick"] = {mode: tuple(v[i:i + 3]), "yaw": v[i + 3], "tilt": v[i + 4], "twist": v[i + 5]}; i += 6
    pose["hl"] = (hmode, v[i]) if hmode == "grip" else (hmode, v[i], v[i + 1], v[i + 2])
    return pose

def mk(base, **kw):
    d = dict(DEFAULT)
    d.update(base)
    for k, v in kw.items():
        if k == "stick":
            s = dict(d["stick"]);
            if any(m in v for m in ("h", "g", "gs")):
                for m in ("h", "g", "gs"):
                    s.pop(m, None)
            s.update(v); d[k] = s
        else:
            d[k] = v
    return d

def catmull(p0, p1, p2, p3, t):
    t2, t3 = t * t, t * t * t
    return [0.5 * ((2 * b) + (-a + c) * t + (2 * a - 5 * b + 4 * c - d) * t2 + (-a + 3 * b - 3 * c + d) * t3)
            for a, b, c, d in zip(p0, p1, p2, p3)]

def sample(clip, f):
    keys = clip["keys"]
    vecs = [flatten(k[1]) for k in keys]
    frames = [k[0] for k in keys]
    if clip["loop"]:
        period = clip["frames"]
        f = f % period
        n = len(keys)
        for i in range(n):
            fa = frames[i]
            fb = frames[i + 1] if i + 1 < n else period
            if fa <= f < fb or (i == n - 1):
                t = (f - fa) / (fb - fa)
                p0 = vecs[(i - 1) % n][0]; p1 = vecs[i][0]; p2 = vecs[(i + 1) % n][0]; p3 = vecs[(i + 2) % n][0]
                return unflatten(catmull(p0, p1, p2, p3, t), vecs[i][1])
    if f <= frames[0]:
        return keys[0][1]
    if f >= frames[-1]:
        return keys[-1][1]
    for i in range(len(keys) - 1):
        if frames[i] <= f <= frames[i + 1]:
            t = (f - frames[i]) / (frames[i + 1] - frames[i])
            kind = keys[i + 1][2]
            a, sa = vecs[i]; b, sb = vecs[i + 1]
            if sa != sb:   # different stick/left-hand modes: convert both to absolute grip / absolute hand
                a = flatten(absolutise(keys[i][1]))[0]; b = flatten(absolutise(keys[i + 1][1]))[0]; sb = ("g", "abs")
                if t >= 1.0:
                    return keys[i + 1][1]
            if kind == "cr":
                p0 = vecs[max(i - 1, 0)][0] if sa == vecs[max(i - 1, 0)][1] else a
                p3 = vecs[min(i + 2, len(keys) - 1)][0] if sb == vecs[min(i + 2, len(keys) - 1)][1] else b
                return unflatten(catmull(p0, a, b, p3, t), sb)
            tt = ease(t, kind)
            return unflatten([x + (y - x) * tt for x, y in zip(a, b)], sb)

def absolutise(pose):
    s = Solver(pose).solve()
    out = dict(pose)
    st = dict(pose["stick"])
    for m in ("h", "g", "gs"):
        st.pop(m, None)
    st["g"] = tuple(s.H["stick_socket"])
    out["stick"] = st
    hand = s.H["hand_l"] + s.W["hand_l"] @ GRIP_OFF_L
    out["hl"] = ("abs", hand.x, hand.y, hand.z)
    return out

# ---------------------------------------------------------------- pose tables
GLIDE = dict(pz=-0.12, pel=(18, 0, 0), sp1=(8, 0, 0), sp2=(8, 0, 0), sp3=(5, 0, 0), neck=(-12, 0, 0), head=(-22, 0, 0),
             fl=(0.04, 0.17, 0.13, 6, 0, 0), fr=(-0.02, -0.17, 0.13, -6, 0, 0),
             stick={"h": (0.78, 0.08, 0.0), "yaw": 12, "tilt": 52, "twist": 0}, hl=("grip", 0.42))
GLIDE = mk(GLIDE)

def stride_key(side, phase):
    """side = the pushing leg; phase 'push' (extended) or 'recover' (pushing leg returning under the hip)."""
    s = 1 if side == "l" else -1           # +1 left leg pushes
    if phase == "push":
        push = (-0.20, s * 0.50, 0.13, s * 32, 0, s * 16)       # out to the side and back, toe out, on the inside edge
        sup = (0.16, -s * 0.06, 0.13, -s * 4, 0, s * 6)          # support skate under the body, knee deep
        pz, pel, sp = -0.22, (26, s * 8, -s * 8), (10, -s * 3, s * 5)
        hand_free = ("rel", -0.38, 0.40, -0.14) if s == 1 else ("rel", 0.44, -0.30, -0.18)
        stick = {"gs": (0.26 if s == 1 else 0.06, -0.26 if s == 1 else -0.32, -0.36), "yaw": -22 if s == 1 else -8, "tilt": 62, "twist": 0}
    else:
        push = (0.06, s * 0.22, 0.20, s * 8, 6, 0)               # lifted a little, coming back under the hip
        sup = (0.06, -s * 0.16, 0.13, -s * 10, 0, -s * 8)         # the other skate starts its push
        pz, pel, sp = -0.18, (26, 0, 0), (10, 0, 0)
        hand_free = ("rel", 0.08, 0.18, -0.48)
        stick = {"gs": (0.18, -0.30, -0.38), "yaw": -15, "tilt": 60, "twist": 0}
    fl, fr = (push, sup) if s == 1 else (sup, push)
    return mk(GLIDE, pz=pz, pel=pel, sp1=sp, sp2=(10, 0, 0), sp3=(6, 0, 0), neck=(-16, 0, 0), head=(-26, 0, 0),
              fl=fl, fr=fr, stick=stick, hl=hand_free, kl=0.35, kr=0.35)

FACEOFF = mk(GLIDE, pz=-0.21, pel=(30, 0, 0), sp1=(14, 0, 0), sp2=(14, 0, 0), sp3=(10, 0, 0), neck=(-20, 0, 0), head=(-34, 0, 0),
             fl=(0.10, 0.27, 0.13, 18, 0, -4), fr=(-0.02, -0.27, 0.13, -18, 0, 4), kl=0.5, kr=0.5,
             stick={"h": (0.66, 0.10, 0.0), "yaw": 8, "tilt": 50, "twist": 0}, hl=("grip", 0.46))

STOP_END = mk(GLIDE, pz=-0.27, pel=(0, 30, -80), sp1=(8, -6, 20), sp2=(8, -4, 20), sp3=(6, 0, 18), neck=(-6, 0, 10), head=(-10, 0, 8),
              fl=(0.52, 0.10, 0.13, -80, 0, 34), fr=(0.08, -0.12, 0.13, -80, 0, 26), kl=0.3, kr=0.3,
              stick={"h": (0.80, 0.40, 0.0), "yaw": 25, "tilt": 55, "twist": 0}, hl=("grip", 0.42))
STOP_MID = mk(STOP_END, pz=-0.16, pel=(4, 12, -55), fl=(0.32, 0.13, 0.15, -55, 0, 14), fr=(0.0, -0.14, 0.13, -50, 0, 10))

# slap shot: the figure shoots left-handed (bottom hand left, blade on the left); the body turns, the stick goes up over the
# left shoulder, sweeps through the puck at the front-left and follows through high to the right
SLAP_SET = mk(GLIDE, pz=-0.12, pel=(10, 0, 30), sp1=(6, 0, 10), sp3=(4, 0, 0), head=(-14, 0, -30),
              fl=(0.10, 0.24, 0.13, 30, 0, 0), fr=(-0.10, -0.22, 0.13, -20, 0, 0),
              stick={"h": (0.55, 0.40, 0.0), "yaw": 30, "tilt": 52, "twist": 0}, hl=("grip", 0.55))
SLAP_UP = mk(SLAP_SET, pz=-0.10, pel=(8, -6, 50), sp1=(4, -6, 22), sp2=(4, -6, 18), sp3=(0, -4, 10), head=(-10, 0, -60),
             stick={"g": (-0.10, 0.50, 0.90), "yaw": 165, "tilt": 140, "twist": 0}, hl=("grip", 0.48), el=(-0.2, 1.0, -0.3), er=(-0.6, -0.6, -0.5),
             fl=(0.12, 0.26, 0.13, 32, 0, 0), fr=(-0.12, -0.22, 0.13, -20, 0, 0))
SLAP_HIT = mk(SLAP_SET, pz=-0.16, pel=(14, 6, 20), sp1=(10, 4, -4), sp2=(6, 4, -6), sp3=(4, 0, -6), head=(-16, 0, -20),
              stick={"h": (0.45, 0.42, 0.0), "yaw": 55, "tilt": 55, "twist": -10}, hl=("grip", 0.58))
SLAP_FOLLOW = mk(SLAP_SET, pz=-0.10, pel=(8, 10, -10), sp1=(4, 6, -18), sp2=(2, 4, -14), sp3=(0, 0, -10), head=(-10, 0, 10),
                 stick={"gs": (0.40, -0.14, -0.10), "yaw": -20, "tilt": 125, "twist": -20}, hl=("grip", 0.45),
                 fl=(0.12, 0.22, 0.13, 20, 0, 0), fr=(-0.06, -0.24, 0.16, -30, 10, 0))
WRIST_LOAD = mk(GLIDE, pz=-0.14, pel=(14, 0, 25), sp1=(8, 0, 10), sp2=(6, 0, 6), head=(-18, 0, -30),
                fl=(0.12, 0.24, 0.13, 25, 0, 0), fr=(-0.08, -0.20, 0.13, -15, 0, 0),
                stick={"h": (0.05, 0.52, 0.0), "yaw": 80, "tilt": 52, "twist": 0}, hl=("grip", 0.52))
WRIST_SNAP = mk(WRIST_LOAD, pz=-0.13, pel=(14, 4, 0), sp1=(8, 2, -12), sp2=(6, 2, -8), head=(-16, 0, -5),
                stick={"h": (0.75, 0.20, 0.03), "yaw": 30, "tilt": 55, "twist": -15}, hl=("grip", 0.5))
WRIST_FOLLOW = mk(WRIST_SNAP, pz=-0.11, pel=(10, 4, -15), sp1=(4, 2, -14),
                  stick={"h": (1.00, -0.10, 0.35), "yaw": 0, "tilt": 80, "twist": -25}, hl=("grip", 0.48))
CHECK_LOAD = mk(GLIDE, pz=-0.20, pel=(18, -6, 20), sp1=(10, -6, 10), sp2=(8, 0, 6), head=(-22, 0, -10),
                fl=(0.10, 0.28, 0.13, 20, 0, -8), fr=(-0.10, -0.26, 0.13, -20, 0, 8),
                stick={"gs": (0.25, -0.05, -0.45), "yaw": 0, "tilt": 60, "twist": 0}, hl=("rel", 0.10, 0.18, -0.42))
CHECK_HIT = mk(CHECK_LOAD, pz=-0.06, pel=(10, 12, -40), sp1=(10, 12, -20), sp2=(8, 10, -14), sp3=(4, 8, -4), head=(-14, 0, 30),
               clav_r=(35, 0, 35), fl=(-0.10, 0.30, 0.13, 25, 0, -12), fr=(0.12, -0.20, 0.13, -30, 0, 4),
               stick={"gs": (0.35, 0.05, -0.30), "yaw": 30, "tilt": 70, "twist": 0}, hl=("rel", -0.15, 0.30, -0.35))
FALL_HIT = mk(GLIDE, pz=-0.08, pel=(-25, 0, 0), sp1=(-12, 0, 0), sp2=(-10, 0, 0), neck=(10, 0, 0), head=(16, 0, 0),
              fl=(0.20, 0.20, 0.16, 10, -10, 0), fr=(0.10, -0.20, 0.13, -10, 0, 0),
              stick={"gs": (0.05, -0.40, -0.10), "yaw": -80, "tilt": 70, "twist": 0}, hl=("rel", 0.05, 0.50, 0.05))
FALL_AIR = mk(FALL_HIT, pz=-0.42, pel=(-60, 0, 0), sp1=(-8, 0, 0), sp2=(-6, 0, 0), neck=(18, 0, 0), head=(20, 0, 0),
              fl=(0.62, 0.22, 0.40, 10, -40, 0), fr=(0.55, -0.22, 0.25, -10, -30, 0),
              stick={"gs": (-0.15, -0.50, 0.10), "yaw": -100, "tilt": 85, "twist": 0}, hl=("rel", -0.10, 0.55, 0.15))
FALL_DOWN = mk(FALL_HIT, pz=-0.635, pel=(-86, 0, 0), sp1=(-2, 0, 0), sp2=(0, 0, 0), neck=(14, 0, 0), head=(18, 0, 0),
               fl=(0.66, 0.24, 0.12, 14, -70, 0), fr=(0.60, -0.26, 0.12, -14, -70, 0), kl=0.3, kr=0.3,
               stick={"g": (-0.55, -0.62, 0.135), "yaw": -95, "tilt": 89, "twist": 90}, hl=("abs", -0.55, 0.62, 0.135))
FALL_BOUNCE = mk(FALL_DOWN, pz=-0.60, pel=(-80, 0, 0), neck=(26, 0, 0), fl=(0.62, 0.24, 0.16, 14, -60, 0))
GETUP_SIT = mk(FALL_DOWN, pz=-0.62, pel=(-15, 0, 0), sp1=(15, 0, 0), sp2=(15, 0, 0), sp3=(10, 0, 0), neck=(-5, 0, 0), head=(-10, 0, 0),
               fl=(0.42, 0.22, 0.13, 14, 0, 0), fr=(0.52, -0.24, 0.13, -14, -20, 0), kl=0.4, kr=0.4,
               stick={"g": (-0.15, -0.48, 0.135), "yaw": 160, "tilt": 89, "twist": 90}, hl=("abs", -0.25, 0.40, 0.135))
GETUP_KNEEL = mk(GLIDE, pz=-0.38, pel=(25, 0, 0), sp1=(15, 0, 0), sp2=(10, 0, 0), sp3=(5, 0, 0), head=(-25, 0, 0),
                 fl=(-0.42, 0.16, 0.10, 6, -20, 0), fr=(0.30, -0.20, 0.13, -10, 0, 0),
                 stick={"h": (0.70, -0.30, 0.0), "yaw": -20, "tilt": 45, "twist": 0}, hl=("rel", 0.30, 0.05, -0.50))
CELEB_UP = mk(GLIDE, pz=-0.02, pel=(0, 0, 0), sp1=(-4, 0, 0), sp2=(-4, 0, 0), sp3=(-2, 0, 0), neck=(-6, 0, 0), head=(-14, 0, 0),
              fl=(0.0, 0.18, 0.13, 10, 0, 0), fr=(0.0, -0.18, 0.13, -10, 0, 0),
              stick={"gs": (0.26, -0.20, 0.38), "yaw": 90, "tilt": 100, "twist": 0}, hl=("grip", 0.42),
              el=(-0.3, 1.0, -0.1), er=(-0.3, -1.0, -0.1))
CELEB_PUMP = mk(CELEB_UP, pz=-0.09, pel=(6, 0, 0), stick={"gs": (0.30, -0.22, 0.26), "yaw": 90, "tilt": 96, "twist": 0},
                hl=("grip", 0.44))

# ---- polish pass: stronger check (wind-back, step in, shoulder and elbow lead)
CHECK_WIND = mk(GLIDE, pz=-0.21, pel=(16, -4, 32), sp1=(10, -4, 12), sp2=(8, 0, 8), sp3=(4, 0, 4), head=(-20, 0, -30),
                clav_r=(-15, 0, -25), fl=(0.10, 0.28, 0.13, 20, 0, -8), fr=(-0.14, -0.26, 0.13, -20, 0, 8),
                stick={"gs": (-0.08, -0.16, -0.40), "yaw": -10, "tilt": 62, "twist": 0}, hl=("rel", 0.28, 0.08, -0.32),
                er=(-0.8, -0.4, 0.3))
CHECK_DRIVE = mk(CHECK_WIND, pz=-0.05, pel=(12, 14, -42), sp1=(10, 10, -20), sp2=(8, 8, -14), sp3=(4, 6, -6), head=(-16, 0, 40),
                 clav_r=(35, 0, 35), fl=(-0.18, 0.30, 0.13, 25, 0, -14), fr=(0.26, -0.16, 0.13, -25, 0, 6),
                 stick={"gs": (0.28, 0.12, -0.16), "yaw": 40, "tilt": 75, "twist": 0}, hl=("rel", -0.12, 0.32, -0.28),
                 er=(0.6, -0.5, 0.6))
CHECK_END = mk(CHECK_DRIVE, pz=-0.12, pel=(14, 8, -25), sp1=(10, 6, -12), clav_r=(15, 0, 15),
               stick={"gs": (0.30, -0.08, -0.36), "yaw": 20, "tilt": 66, "twist": 0}, er=(-0.5, -0.9, -0.4))

# ---- polish pass: fall forward, ends face down
FF_HIT = mk(GLIDE, pz=-0.10, pel=(34, 0, 0), sp1=(10, 0, 0), sp2=(8, 0, 0), neck=(-14, 0, 0), head=(-30, 0, 0),
            fl=(-0.06, 0.20, 0.16, 10, 10, 0), fr=(-0.16, -0.20, 0.13, -10, 0, 0),
            stick={"gs": (0.30, -0.22, -0.30), "yaw": -20, "tilt": 70, "twist": 0}, hl=("rel", 0.35, 0.25, -0.20))
FF_AIR = mk(FF_HIT, pz=-0.40, pel=(62, 0, 0), sp1=(6, 0, 0), sp2=(4, 0, 0), neck=(-18, 0, 0), head=(-26, 0, 0),
            fl=(-0.48, 0.22, 0.32, 10, 35, 0), fr=(-0.40, -0.22, 0.22, -10, 25, 0),
            stick={"gs": (0.45, -0.28, -0.05), "yaw": -15, "tilt": 85, "twist": 0}, hl=("rel", 0.45, 0.25, -0.12),
            el=(-0.3, 0.6, 0.7), er=(-0.3, -0.6, 0.7))
FF_DOWN = mk(FF_AIR, pz=-0.635, pel=(86, 0, 0), sp1=(2, 0, 0), sp2=(0, 0, 0), neck=(-16, 0, 0), head=(-22, 0, 25),
             fl=(-0.66, 0.24, 0.12, 14, 70, 0), fr=(-0.60, -0.26, 0.12, -14, 70, 0), kl=0.3, kr=0.3,
             stick={"g": (0.50, -0.55, 0.135), "yaw": -30, "tilt": 89, "twist": 90}, hl=("abs", 0.52, 0.55, 0.135))
FF_BOUNCE = mk(FF_DOWN, pz=-0.60, pel=(80, 0, 0), neck=(-26, 0, 0), fl=(-0.62, 0.24, 0.16, 14, 60, 0))

# ---- polish pass: carving leans (2-frame holds, blended by turn rate); s = +1 left, -1 right
def lean_pose(s):
    off = -s * 0.24                    # skates out to the outside of the turn, body over the inside
    return mk(GLIDE, pz=-0.17, pel=(16, -s * 14, s * 6), sp1=(8, -s * 6, s * 3), sp2=(8, -s * 5, 0), sp3=(5, 0, 0),
              neck=(-12, s * 8, 0), head=(-22, s * 6, s * 8),
              fl=(0.16 if s == 1 else -0.10, 0.17 + off, 0.13, s * 12, 0, -s * 25),
              fr=(-0.10 if s == 1 else 0.16, -0.17 + off, 0.13, s * 12, 0, -s * 25), kl=0.4, kr=0.4,
              stick={"gs": (0.02, -0.20, -0.42), "yaw": -140 if s == 1 else -115, "tilt": 64, "twist": 0},
              hl=("rel", 0.18, 0.36, -0.22) if s == 1 else ("rel", 0.20, 0.10, -0.42))

# ---- polish pass: crossovers (in place, 40-frame loop); the outside skate steps over the inside one, the inside one pushes under
def crossover_key(s, k):
    off = -s * 0.20
    def ft(x, yin, z, yaw=8, pitch=0):
        return (x, s * yin + off, z, s * yaw, pitch, -s * 22)
    feet = [  # (inside skate, outside skate) per key, yin measured towards the inside of the turn
        (ft(0.05, 0.10, 0.13), ft(-0.16, -0.40, 0.13, 20)),
        (ft(0.00, -0.06, 0.13, 0), ft(0.20, 0.06, 0.22, 4, -8)),
        (ft(-0.14, -0.32, 0.13, -10), ft(0.12, 0.20, 0.13, 6)),
        (ft(-0.04, 0.16, 0.21, 10, 6), ft(0.02, -0.14, 0.13, 12)),
    ][k]
    inner, outer = feet
    fl, fr = (inner, outer) if s == 1 else (outer, inner)
    base = lean_pose(s)
    bob = (-0.18, -0.15, -0.19, -0.15)[k]
    sw = (1, 0, -1, 0)[k]
    return mk(base, pz=bob, fl=fl, fr=fr, sp1=(8, -s * 6, s * 3 + 4 * sw), kl=0.3, kr=0.3,
              hl=("rel", 0.18 + 0.12 * sw, s * 0.30 + 0.06, -0.24) if s == 1 else ("rel", 0.22 - 0.1 * sw, 0.10, -0.40))

def hold_keys(pose):
    """A held pose as a 30-frame loop: a 0.1 degree breathing wobble (spine pitch, pelvis pitch) so no curve is constant."""
    w = mk(pose, pel=(pose["pel"][0] + 0.1, pose["pel"][1], pose["pel"][2]), sp2=(pose["sp2"][0] + 0.1, pose["sp2"][1], pose["sp2"][2]),
           pz=pose["pz"] - 0.0005)
    return [(0, pose, "cr"), (15, w, "cr")]

# ---- export fix: get up from the face-down end of Fall_Forward (push up on the hands, knee under, rise into the glide crouch)
GUF_PUSH = mk(FF_DOWN, pz=-0.52, pel=(62, 0, 0), sp1=(4, 0, 0), sp2=(2, 0, 0), neck=(-22, 0, 0), head=(-28, 0, 0),
              fl=(-0.62, 0.22, 0.12, 10, 70, 0), fr=(-0.58, -0.24, 0.12, -10, 70, 0),
              stick={"g": (0.46, -0.36, 0.135), "yaw": -25, "tilt": 89, "twist": 90}, hl=("abs", 0.46, 0.34, 0.135),
              el=(-0.6, 0.6, 0.4), er=(-0.6, -0.6, 0.4))
GUF_KNEE = mk(GUF_PUSH, pz=-0.40, pel=(46, 0, 0), sp1=(6, 0, 0), sp2=(4, 0, 0), neck=(-18, 0, 0), head=(-26, 0, 0),
              fl=(0.12, 0.20, 0.13, 10, 0, 0), fr=(-0.46, -0.20, 0.14, -8, 45, 0), kl=0.35, kr=0.2,
              stick={"g": (0.36, -0.34, 0.16), "yaw": -20, "tilt": 70, "twist": 30}, hl=("rel", 0.22, 0.02, -0.42))
GUF_RISE = mk(GLIDE, pz=-0.24, pel=(30, 0, 0), sp1=(12, 0, 0), sp2=(10, 0, 0), sp3=(6, 0, 0), neck=(-16, 0, 0), head=(-30, 0, 0),
              fl=(0.08, 0.20, 0.13, 10, 0, 0), fr=(-0.10, -0.20, 0.13, -10, 0, 0),
              stick={"h": (0.70, -0.10, 0.0), "yaw": -5, "tilt": 50, "twist": 0}, hl=("rel", 0.25, 0.05, -0.45))

def breathe(base, amt):
    return mk(base, pz=base["pz"] - 0.012 * amt, sp2=(base["sp2"][0] + 2.0 * amt, 0, 0), sp3=(base["sp3"][0] + 1.5 * amt, 0, 0))

GLIDE_B = mk(GLIDE, pz=-0.11, sp2=(7, 0, 0), head=(-15, 0, 0))

CLIPS = {
    "Idle_Faceoff": dict(frames=120, loop=True, keys=[(0, FACEOFF, "cr"), (60, breathe(FACEOFF, 1.0), "cr")]),
    "Stride": dict(frames=60, loop=True, keys=[(0, stride_key("r", "push"), "cr"), (15, stride_key("r", "recover"), "cr"),
                                              (30, stride_key("l", "push"), "cr"), (45, stride_key("l", "recover"), "cr")]),
    "Glide": dict(frames=120, loop=True, keys=[(0, GLIDE, "cr"), (60, GLIDE_B, "cr")]),
    "Stop": dict(frames=36, loop=False, keys=[(0, GLIDE, "io"), (10, STOP_MID, "in"), (22, STOP_END, "out"),
                                              (36, mk(STOP_END, pz=-0.24, pel=(0, 26, -80)), "io")]),
    "Shot_Slap": dict(frames=42, loop=False, keys=[(0, SLAP_SET, "io"),
                                                   (8, mk(SLAP_UP, stick={"g": (-0.05, 0.50, 0.62), "yaw": 150, "tilt": 110, "twist": 0}, hl=("grip", 0.45)), "io"),
                                                   (16, SLAP_UP, "out"),
                                                   (19, mk(SLAP_UP, pz=-0.13, stick={"g": (0.08, 0.52, 0.66), "yaw": 120, "tilt": 115, "twist": 0}, hl=("grip", 0.5)), "in"),
                                                   (23, SLAP_HIT, "out"),
                                                   (31, SLAP_FOLLOW, "out"), (42, mk(SLAP_FOLLOW, pz=-0.11), "io")]),
    "Shot_Wrist": dict(frames=21, loop=False, keys=[(0, WRIST_LOAD, "io"), (9, WRIST_SNAP, "in"), (15, WRIST_FOLLOW, "out"),
                                                    (21, WRIST_FOLLOW, "io")]),
    "Check": dict(frames=30, loop=False, keys=[(0, GLIDE, "io"), (9, CHECK_WIND, "io"), (16, CHECK_DRIVE, "in"),
                                               (21, mk(CHECK_DRIVE, pz=-0.06), "out"), (30, CHECK_END, "io")]),
    "Fall_Forward": dict(frames=36, loop=False, keys=[(0, GLIDE, "io"), (6, FF_HIT, "out"), (18, FF_AIR, "in"), (27, FF_DOWN, "in"),
                                                      (31, FF_BOUNCE, "out"), (36, FF_DOWN, "io")]),
    "Lean_Left": dict(frames=30, loop=True, keys=hold_keys(lean_pose(1))),
    "Lean_Right": dict(frames=30, loop=True, keys=hold_keys(lean_pose(-1))),
    "GetUp_Front": dict(frames=72, loop=False, keys=[(0, FF_DOWN, "io"), (18, GUF_PUSH, "io"), (38, GUF_KNEE, "io"),
                                                     (56, GUF_RISE, "io"), (72, GLIDE, "io")]),
    "Crossover_Left": dict(frames=40, loop=True, keys=[(10 * k, crossover_key(1, k), "cr") for k in range(4)]),
    "Crossover_Right": dict(frames=40, loop=True, keys=[(10 * k, crossover_key(-1, k), "cr") for k in range(4)]),
    "Fall": dict(frames=36, loop=False, keys=[(0, GLIDE, "io"), (6, FALL_HIT, "out"), (18, FALL_AIR, "in"), (27, FALL_DOWN, "in"),
                                              (31, FALL_BOUNCE, "out"), (36, FALL_DOWN, "io")]),
    "GetUp": dict(frames=72, loop=False, keys=[(0, FALL_DOWN, "io"), (22, GETUP_SIT, "io"), (44, GETUP_KNEEL, "io"),
                                               (72, GLIDE, "io")]),
    "Celebrate": dict(frames=72, loop=False, keys=[(0, GLIDE, "io"),
                                                   (7, mk(CELEB_UP, pz=-0.06, stick={"gs": (0.42, -0.22, -0.05), "yaw": 90, "tilt": 95, "twist": 0}), "io"),
                                                   (14, CELEB_UP, "out"), (30, CELEB_PUMP, "io"),
                                                   (46, CELEB_UP, "io"), (60, CELEB_PUMP, "io"), (72, CELEB_UP, "io")]),
}

# ---------------------------------------------------------------- bake into actions
def bake(name, clip):
    act = bpy.data.actions.new(name)
    act.use_fake_user = True
    nfr = clip["frames"] + 1
    chans = {}
    prevq = {}
    for f in range(nfr):
        pose = sample(clip, 0 if (clip["loop"] and f == nfr - 1) else f)
        s = Solver(pose).solve()
        for bn in ORDER:
            if bn == "root":          # the root bone is never keyed: it stays at its reference pose in every take
                continue
            R = REST_R[bn]
            q = (R.transposed() @ s.Q[bn] @ R).to_quaternion()
            if bn in prevq and q.dot(prevq[bn]) < 0:
                q.negate()
            prevq[bn] = q
            chans.setdefault((bn, "rotation_quaternion"), []).append(list(q))
            loc = Vector((0, 0, pose["pz"])) if bn == "pelvis" else Vector((0, 0, 0))
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
    return act

for pb in arm.pose.bones:
    pb.rotation_mode = "QUATERNION"
arm.animation_data_create()
acts = [bake(n, c) for n, c in CLIPS.items()]

# ---------------------------------------------------------------- self-check in Blender (world positions per frame)
report = {}
for act in acts:
    arm.animation_data.action = act
    f0, f1 = int(act.frame_start), int(act.frame_end)
    pel = []
    allpos = []
    feet_min = []
    for f in range(f0, f1 + 1):
        scn.frame_set(f)
        pel.append(arm.matrix_world @ arm.pose.bones["pelvis"].head)
        allpos.append([arm.matrix_world @ pb.head for pb in arm.pose.bones] + [arm.matrix_world @ pb.tail for pb in arm.pose.bones])
    p0 = pel[0]
    drift = max(math.hypot(p.x - p0.x, p.y - p0.y) for p in pel)
    seam = max((a - b).length for a, b in zip(allpos[0], allpos[-1]))
    report[act.name] = dict(frames=f1 - f0 + 1, seconds=round((f1 - f0) / 60, 3), loop=CLIPS[act.name]["loop"],
                            pelvis_horizontal_max_cm=round(drift * 100, 4), pelvis_z_range_cm=round((max(p.z for p in pel) - min(p.z for p in pel)) * 100, 2),
                            seam_max_joint_cm=round(seam * 100, 4))
arm.animation_data.action = bpy.data.actions["Glide"]
scn.frame_set(0)
print("S4 CLIPS", json.dumps(report, indent=1))
with open(os.path.join(LAB, "clips_report.json"), "w") as fh:
    json.dump(report, fh, indent=1)

bpy.ops.wm.save_as_mainfile(filepath=os.path.join(LAB, "s4_skater_anim.blend"))

# ---------------------------------------------------------------- export
# GLB first (metres, glTF convention), then the FBX files in centimetres with an identity armature object:
# the bones and the mesh are scaled x100 in their data, the scene unit is 0.01 m, so FBX_SCALE_NONE writes cm without any node scale,
# Axes stay -Z forward / Y up (Blender's default): the file is Y-up, the armature null carries only the -90 deg X axis turn, which Unreal's
# scene conversion cancels (proved by the first import: no extra rotation came from the null). `root` lies along +Y with roll 0, so its own
# node has no rotation either; the first import's "root roll 90" was the root bone pointing up, its "scale 100" was the metre unit.
EXP = os.path.join(LAB, "export")
os.makedirs(EXP, exist_ok=True)
meshes = [o for o in bpy.data.objects if o.type == "MESH"]
for o in bpy.context.view_layer.objects:
    o.select_set(o.type in ("ARMATURE", "MESH"))
bpy.ops.export_scene.gltf(filepath=os.path.join(EXP, "SK_Skater_S4.glb"), export_format="GLB", use_selection=True,
                          export_animations=True, export_animation_mode="ACTIONS", export_force_sampling=True,
                          export_frame_range=False, export_def_bones=False, export_yup=True)

def to_centimetres():
    assert arm.matrix_world == Matrix.Identity(4)
    scn.unit_settings.scale_length = 0.01
    bpy.context.view_layer.objects.active = arm
    bpy.ops.object.mode_set(mode="EDIT")
    for eb in arm.data.edit_bones:
        eb.head = eb.head * 100.0
        eb.tail = eb.tail * 100.0
    bpy.ops.object.mode_set(mode="OBJECT")
    for ob in meshes:
        assert ob.matrix_world == Matrix.Identity(4)
        ob.data.transform(Matrix.Scale(100.0, 4))
    for act in bpy.data.actions:
        for fc in act.fcurves:
            if fc.data_path.endswith(".location"):
                for kp in fc.keyframe_points:
                    kp.co.y *= 100.0; kp.handle_left.y *= 100.0; kp.handle_right.y *= 100.0
to_centimetres()
arm.animation_data.action = None          # node transforms in the file = rest pose, not a frame of some clip
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
    o.select_set(o.type in ("ARMATURE", "MESH"))
bpy.ops.export_scene.fbx(filepath=os.path.join(EXP, "SK_Skater_S4.fbx"), **FBX_ARGS)
# variant without the trailing repeated frame on looping clips
for act in acts:
    if CLIPS[act.name]["loop"]:
        act.frame_end = CLIPS[act.name]["frames"] - 1
bpy.ops.export_scene.fbx(filepath=os.path.join(EXP, "SK_Skater_S4_loopcut.fbx"), **FBX_ARGS)
for act in acts:
    act.frame_end = CLIPS[act.name]["frames"]
print("S4 EXPORT OK", EXP)
