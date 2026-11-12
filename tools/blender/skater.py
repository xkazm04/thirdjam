"""tools/blender/skater.py: one parametric skater for Third Jam (team Hokej), lane A22, Blender 4.2.

Run headless (never with an interactive editor):
  blender -b -P tools/blender/skater.py -- --variant toy --out tools/blender/out [--render] [--turntable] [--no-export]

One script, three figures (`--variant toy | clay | broadcast`):
  toy        the code-built toy figure of ToySkaterVisual.cpp as a skeletal mesh: capsule torso, ball head, helmet band, stub legs on a skate plate
  clay       the toy figure with fingerprint dimples (displacement) and a felt stitch seam (P3 clay miniature)
  broadcast  limbs lengthened toward a 1.8 m humanoid with a helmet and no face (P4 stylized broadcast)

Space and units: the scene unit scale is 0.01 (1 Blender unit = 1 cm) so the FBX carries centimetres with the armature at scale 1.0.
The figure faces Blender +X with the stick on Blender -Y (its right side). Unreal's FBX import flips Y, so in Unreal the figure faces +X and the
stick is on +Y, the same frame as the toy figure in ToySkaterVisual.cpp (stick pivot at (0, +0.3, 0.9) m, puck_socket at (1.15, 0.35, 0) m).

Armature (deform bones): root, pelvis, torso, head, arm_l, arm_r, stick_root. Socket bones (leaf, no deform): stick_socket (the grip, child of stick_root),
puck_socket (in front of the blade on the ice, child of stick_root), head_socket (above the head, child of head). Unreal reads bone names as sockets.
The arms are STRETCH_TO constraints on the stick, so the mittens stay on the shaft in every pose and the FBX bake writes the stretch as bone scale.

Actions (60 fps, timings from ToySkaterVisual.cpp, names after contracts/skater-visual.md v3): idle, skate, carry, aim_left, aim_right, poke, lift,
shoot_windup, shoot_release, check, knocked_down, get_up, faceoff_ready, draw, goalie_left, goalie_centre, goalie_right.

Outputs (in --out): SK_Skater<Variant>.fbx, SK_Skater<Variant>.json (triangle counts, bones, actions, timings) and, with --render, render/cell_NN.png
(idle, carry, poke at 1024 px) and render/<variant>_tt_NNN.png (60 turntable frames at 512 px) for ffmpeg (see run_all.sh).
"""
import bpy
import bmesh
import json
import math
import os
import sys
import time
from mathutils import Matrix, Vector

T0 = time.time()

# ---------------------------------------------------------------- parameters (metres and degrees; edit here)

VARIANT = "toy"

BASE = dict(
    # body (toy: 1.6 m to the top of the head, a big head, shoulders wider than the hips)
    body_w=0.64, hip_w=0.46, body_bottom=0.14, body_top=1.02, cap_h=0.30,   # tapered torso from hip_w at the bottom to body_w at the shoulders, a dome of cap_h on top
    head_r=0.245, head_z=1.345, helmet_band=True, band_h=0.13, visor=True, helmet=False, cage=False, neck=False,
    legs=False, leg_r=0.09, leg_bottom=0.06, leg_top=0.22, leg_y=0.13, knees=False, knee_z=0.0, thigh_r=0.0, shin_r=0.0,
    plate=True, plate_x=0.46, plate_y=0.40, plate_h=0.025, runners=True, runner_h=0.01,
    skate_len=0.32, skate_w=0.09, skate_h=0.09, skate_y=0.13,
    shoulder_y=0.30, shoulder_z=0.98, arm_r=0.065, elbow_offset=(0.30, 0.0, -0.20),   # elbows forward: both arms cross in front of the chest to the stick (two-bone IK arms, baked)
    # stick: the shaft top (grip end) is at the top hand; the shaft runs forward and down to the ice at the lie angle
    stick_shaft=1.30, stick_blade=0.35, stick_lie_deg=42.0, blade_out_deg=30.0, stick_r_top=0.032, stick_r_heel=0.024,
    stick_top=(0.33, -0.38, 0.88),                                  # the grip in front of the right shoulder, outside the torso                                   # Blender space: -Y is the skater's right
    blade_h=0.08, blade_h_toe=0.055, blade_t=0.03, blade_t_toe=0.018, heel_r=0.045, tape_len=0.15, knob=True,   # the blade tapers toward the toe
    hand_upper=0.07, hand_lower=0.43, hand_r=0.085, gloves=True,
    lean_deg=15.0,                                                  # the skate_left / skate_right lean; the game caps the roll it adds on top
    puck_front=0.12,                                                # puck_socket this far in front of the blade face, on the ice
    # look
    clay=False, seam=False, dimple_strength=0.035, dimple_scale=0.16,    # thumb dents: a 10 cm miniature's fingerprints scaled to 1.6 m
    height=1.6,                                                     # for the camera framing only
    number_z=0.66, number_size=0.30,
)

VARIANTS = {
    "toy": dict(),
    "clay": dict(clay=True, seam=True),
    "broadcast": dict(
        body_w=0.50, hip_w=0.36, body_bottom=0.86, body_top=1.44, cap_h=0.22,
        head_r=0.13, head_z=1.64, helmet_band=False, visor=False, helmet=True, cage=True, neck=True,
        legs=True, leg_r=0.0, leg_bottom=0.08, leg_top=0.86, leg_y=0.14, knees=True, knee_z=0.48, thigh_r=0.095, shin_r=0.075,
        plate=False, runners=True, skate_y=0.14, skate_len=0.32, skate_w=0.09, skate_h=0.08,
        shoulder_y=0.29, shoulder_z=1.40, arm_r=0.055, elbow_offset=(0.22, 0.0, -0.16),
        stick_shaft=1.55, stick_lie_deg=48.0, stick_top=(0.30, -0.34, 1.16), hand_upper=0.07, hand_lower=0.52, hand_r=0.08, gloves=True,
        height=1.85, number_z=1.12, number_size=0.28,
    ),
}

COLOURS = dict(
    base=(0.05, 0.25, 0.85, 1.0),     # team colour (TeamColour in Unreal)
    trim=(0.03, 0.03, 0.035, 1.0),    # helmet, gloves, skates, blade
    number=(0.95, 0.95, 0.95, 1.0),   # number patch on the back
    stick=(0.62, 0.46, 0.26, 1.0),    # shaft and blade wood; the tape band and the knob are trim
    head=(0.48, 0.58, 0.92, 1.0),     # a lighter team colour (the toy code lerps TeamColour 45 % toward white)
    steel=(0.74, 0.76, 0.80, 1.0),    # skate blade strips and the cage line (never the number slot: the game puts a digits material there)
)
SLOT_ORDER = ["base", "number", "head", "trim", "stick", "steel"]   # material slot names and order in the exported mesh

FPS = 60
CELL_PX = 1024
TURNTABLE_PX = 512
TURNTABLE_FRAMES = 60
SHEET_POSES = [("idle", 0), ("carry", 15), ("poke", 6)]


# ---------------------------------------------------------------- arguments

def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    opts = {"variant": "toy", "out": "tools/blender/out", "render": False, "turntable": False, "export": True}
    i = 0
    while i < len(argv):
        a = argv[i]
        if a == "--variant":
            opts["variant"] = argv[i + 1]; i += 1
        elif a == "--out":
            opts["out"] = argv[i + 1]; i += 1
        elif a == "--render":
            opts["render"] = True
        elif a == "--turntable":
            opts["turntable"] = True
        elif a == "--no-export":
            opts["export"] = False
        i += 1
    return opts


# ---------------------------------------------------------------- helpers (cm from here on)

def cm(x):
    return x * 100.0


def V(x, y, z):
    return Vector((cm(x), cm(y), cm(z)))


def clamp01(t):
    return max(0.0, min(1.0, t))


def smooth01(t):
    t = clamp01(t)
    return t * t * (3.0 - 2.0 * t)


def lerp(a, b, t):
    return a + (b - a) * t


def rad(d):
    return math.radians(d)


# Unreal -> Blender rotation conventions (Blender +X forward, -Y right, Z up; Unreal +X forward, +Y right, Z up, left-handed):
# Unreal yaw psi   -> Blender rotation about Z by -psi
# Unreal pitch th  -> Blender rotation about Y by -th
# Unreal roll phi  -> Blender rotation about X by +phi
def R_yaw(ue_yaw_deg):
    return Matrix.Rotation(rad(-ue_yaw_deg), 4, "Z")


def R_pitch(ue_pitch_deg):
    return Matrix.Rotation(rad(-ue_pitch_deg), 4, "Y")


def R_roll(ue_roll_deg):
    return Matrix.Rotation(rad(ue_roll_deg), 4, "X")


def T(v):
    return Matrix.Translation(v)


def about(point, rot):
    return T(point) @ rot @ T(-point)


class Build:
    def __init__(self, p):
        self.p = p
        self.parts_body = []
        self.parts_stick = []
        self.materials = {}
        self.stats = {}

    # ---- scene
    def reset_scene(self):
        bpy.ops.wm.read_factory_settings(use_empty=True)
        sc = bpy.context.scene
        sc.unit_settings.system = "METRIC"
        sc.unit_settings.scale_length = 0.01
        sc.unit_settings.length_unit = "CENTIMETERS"
        sc.render.fps = FPS
        sc.render.fps_base = 1.0
        sc.frame_start = 0
        self.scene = sc

    def material(self, key):
        if key in self.materials:
            return self.materials[key]
        m = bpy.data.materials.new(key)   # the slot name the game sees: base, number, head, trim, stick, steel
        m.use_nodes = True
        bsdf = m.node_tree.nodes.get("Principled BSDF")
        bsdf.inputs["Base Color"].default_value = COLOURS[key]
        bsdf.inputs["Roughness"].default_value = 0.45 if key in ("base", "head") else 0.6
        self.materials[key] = m
        return m

    # ---- parts
    def _finish(self, obj, name, mat, bone, group, smooth=True):
        obj.name = name
        obj.data.name = name
        obj.data.materials.append(self.material(mat))
        vg = obj.vertex_groups.new(name=bone)
        vg.add([v.index for v in obj.data.vertices], 1.0, "REPLACE")
        if smooth:
            for poly in obj.data.polygons:
                poly.use_smooth = True
        group.append(obj)
        return obj

    def cylinder(self, name, mat, bone, group, r, z0, z1, x=0.0, y=0.0, verts=24):
        bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=cm(r), depth=cm(z1 - z0), location=V(x, y, (z0 + z1) * 0.5))
        return self._finish(bpy.context.active_object, name, mat, bone, group)

    def cone(self, name, mat, bone, group, r_bottom, r_top, z0, z1, x=0.0, y=0.0, verts=24):
        bpy.ops.mesh.primitive_cone_add(vertices=verts, radius1=cm(r_bottom), radius2=cm(r_top), depth=cm(z1 - z0), location=V(x, y, (z0 + z1) * 0.5))
        return self._finish(bpy.context.active_object, name, mat, bone, group)

    def cone_between(self, name, mat, bone, group, a, b, r_a, r_b, verts=12):
        a, b = Vector(a), Vector(b)
        d = b - a
        rot = d.to_track_quat("Z", "Y").to_euler()
        bpy.ops.mesh.primitive_cone_add(vertices=verts, radius1=cm(r_a), radius2=cm(r_b), depth=d.length * 100.0, location=(a + b) * 50.0, rotation=rot)
        return self._finish(bpy.context.active_object, name, mat, bone, group)

    def cylinder_between(self, name, mat, bone, group, a, b, r, verts=16):
        a, b = Vector(a), Vector(b)
        d = b - a
        rot = d.to_track_quat("Z", "Y").to_euler()
        bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=cm(r), depth=d.length * 100.0, location=(a + b) * 50.0, rotation=rot)
        return self._finish(bpy.context.active_object, name, mat, bone, group)

    def sphere(self, name, mat, bone, group, r, x, y, z, scale=(1, 1, 1), seg=24, rings=12):
        bpy.ops.mesh.primitive_uv_sphere_add(segments=seg, ring_count=rings, radius=cm(r), location=V(x, y, z))
        o = bpy.context.active_object
        o.scale = scale
        bpy.ops.object.transform_apply(scale=True)
        return self._finish(o, name, mat, bone, group)

    def box(self, name, mat, bone, group, size, loc, rot_z_deg=0.0, rot=None):
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=V(*loc))
        o = bpy.context.active_object
        o.scale = (cm(size[0]), cm(size[1]), cm(size[2]))
        if rot is not None:
            o.rotation_euler = rot
        else:
            o.rotation_euler = (0, 0, rad(rot_z_deg))
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        return self._finish(o, name, mat, bone, group, smooth=False)

    def number_patch(self, group):
        # a flat square patch standing off the back (-X side), UV 0..1 with u running left to right as seen from behind, its own slot "number"
        p = self.p
        x = -(self._body_r_at(p["number_z"]) + 0.02)
        half = p["number_size"] * 0.5
        zc = p["number_z"]
        bm = bmesh.new()
        uv_layer = bm.loops.layers.uv.new("UVMap")
        # seen from behind (looking along +X) the viewer's right is -Y in Blender, and the FBX Y flip keeps "right" on the right in Unreal
        pts = [(x, half, zc - half), (x, -half, zc - half), (x, -half, zc + half), (x, half, zc + half)]
        uvs = [(0.0, 0.0), (1.0, 0.0), (1.0, 1.0), (0.0, 1.0)]
        verts = [bm.verts.new(tuple(V(*q))) for q in pts]
        f = bm.faces.new(verts)
        for loop, uv in zip(f.loops, uvs):
            loop[uv_layer].uv = uv
        me = bpy.data.meshes.new("number")
        bm.to_mesh(me)
        bm.free()
        o = bpy.data.objects.new("number", me)
        bpy.context.collection.objects.link(o)
        bpy.context.view_layer.objects.active = o
        return self._finish(o, "number", "number", "torso", group, smooth=False)

    def tapered_blade(self, name, mat, bone, group, heel, bdir, out, length, t0, t1, h0, h1):
        """a blade box that tapers in thickness and height from the heel (t0, h0) to the toe (t1, h1), lying on the ice"""
        bm = bmesh.new()
        q = []
        for end, t, h in ((0.0, t0, h0), (length, t1, h1)):
            c = heel + bdir * end
            for sgn in (-1, 1):
                for z in (0.0, h):
                    pt = c + out * (sgn * t * 0.5) + Vector((0, 0, z))
                    q.append(bm.verts.new(tuple(V(*pt))))
        # q index: end(0/1)*4 + sgn(0/1)*2 + z(0/1)
        faces = [(0, 1, 5, 4), (2, 6, 7, 3), (0, 4, 6, 2), (1, 3, 7, 5), (0, 2, 3, 1), (4, 5, 7, 6)]
        for fc in faces:
            bm.faces.new([q[i] for i in fc])
        bmesh.ops.recalc_face_normals(bm, faces=bm.faces)
        me = bpy.data.meshes.new(name)
        bm.to_mesh(me)
        bm.free()
        o = bpy.data.objects.new(name, me)
        bpy.context.collection.objects.link(o)
        bpy.context.view_layer.objects.active = o
        return self._finish(o, name, mat, bone, group, smooth=False)

    # ---- the figure
    def stick_frame(self):
        p = self.p
        top = Vector(p["stick_top"])
        lie = rad(p["stick_lie_deg"])
        # the shaft runs forward (+X) with a little outward drift (-Y) and down at the lie angle
        horiz = Vector((0.995, -0.1, 0.0)).normalized()
        d = Vector((horiz.x * math.cos(lie), horiz.y * math.cos(lie), -math.sin(lie)))
        heel = top + d * p["stick_shaft"]
        # blade: lies on the ice, turned blade_out_deg outward (toward -Y, the skater's right) from the shaft's horizontal direction
        bdir = Matrix.Rotation(rad(-p["blade_out_deg"]), 3, "Z") @ Vector((horiz.x, horiz.y, 0.0))
        bdir.normalize()
        out = Vector((bdir.y, -bdir.x, 0.0))  # the blade's outward normal (away from the body, toward -Y)
        if out.y > 0:
            out = -out
        return top, d, heel, bdir, out

    def build_mesh(self):
        p = self.p
        body, stick = self.parts_body, self.parts_stick
        r_top = p["body_w"] * 0.5
        r_hip = p["hip_w"] * 0.5
        # torso: tapered (shoulders wider than the hips), a dome on top, a rounded bottom
        self.cone("body", "base", "torso", body, r_hip, r_top, p["body_bottom"], p["body_top"])
        self.sphere("body_cap", "base", "torso", body, r_top, 0, 0, p["body_top"], scale=(1, 1, p["cap_h"] / p["body_w"]))
        self.sphere("body_bottom", "base", "pelvis" if p["legs"] else "torso", body, r_hip, 0, 0, p["body_bottom"], scale=(1, 1, 0.45))
        self.number_patch(body)
        # head, helmet band with a visor, or a helmet with a cage line
        if p["neck"]:
            self.cylinder("neck", "head", "head", body, p["head_r"] * 0.55, p["body_top"] + p["cap_h"] * 0.3, p["head_z"] - p["head_r"] * 0.5, verts=12)
        self.sphere("head", "head", "head", body, p["head_r"], 0, 0, p["head_z"])
        if p["helmet_band"]:
            self.cylinder("helmet_band", "trim", "head", body, p["head_r"] * 1.07, p["head_z"] + 0.01, p["head_z"] + 0.01 + p["band_h"])
            self.sphere("helmet_top", "trim", "head", body, p["head_r"] * 1.03, 0, 0, p["head_z"] + 0.03, scale=(1, 1, 0.9))
        if p["visor"]:
            self.box("visor", "trim", "head", body, (0.06, p["head_r"] * 1.5, 0.07), (p["head_r"] * 0.9, 0, p["head_z"] - 0.02))
        if p["helmet"]:
            self.sphere("helmet", "trim", "head", body, p["head_r"] * 1.2, 0, 0, p["head_z"] + 0.02, scale=(1.0, 1.0, 0.95))
        if p["cage"]:
            # a cage line: a thin bright bar across the face and a vertical bar, in front of the helmet
            fx = p["head_r"] * 1.2
            self.box("cage_h", "steel", "head", body, (0.02, p["head_r"] * 1.6, 0.025), (fx, 0, p["head_z"] - 0.03))
            self.box("cage_v", "steel", "head", body, (0.02, 0.025, p["head_r"] * 0.9), (fx, 0, p["head_z"] - 0.1))
        # legs, knees and skates
        if p["legs"]:
            for sgn, nm in ((1, "l"), (-1, "r")):
                y = sgn * p["leg_y"]
                self.cylinder("thigh_" + nm, "trim", "pelvis", body, p["thigh_r"], p["knee_z"], p["leg_top"], y=y, verts=14)
                self.sphere("knee_" + nm, "trim", "pelvis", body, p["thigh_r"] * 1.05, 0, y, p["knee_z"], seg=14, rings=8)
                self.cylinder("shin_" + nm, "trim", "pelvis", body, p["shin_r"], p["leg_bottom"], p["knee_z"], y=y, verts=14)
        else:
            for sgn, nm in ((1, "leg_l"), (-1, "leg_r")):
                self.cylinder(nm, "trim", "pelvis", body, p["leg_r"], p["leg_bottom"], p["leg_top"], y=sgn * p["leg_y"], verts=12)
        if p["plate"]:
            self.box("skate_plate", "trim", "pelvis", body, (p["plate_x"], p["plate_y"], p["plate_h"]), (0, 0, p["plate_h"] * 0.5 + 0.02))
        z0 = (p["plate_h"] + 0.02) if p["plate"] else 0.0
        rh = p["runner_h"] if p["runners"] else 0.0
        for sgn, nm in ((1, "skate_l"), (-1, "skate_r")):
            # the black skate block sits on a light steel strip (the blade line), which sits on the plate or the ice
            self.box(nm, "trim", "pelvis", body, (p["skate_len"], p["skate_w"], p["skate_h"]), (0, sgn * p["skate_y"], z0 + rh + p["skate_h"] * 0.5))
            if p["runners"]:
                self.box(nm + "_runner", "steel", "pelvis", body, (p["skate_len"] * 0.92, 0.014, rh), (0, sgn * p["skate_y"], z0 + rh * 0.5))
        # stick: tapered shaft, knob, heel sphere, wooden blade with a dark tape band
        top, d, heel, bdir, out = self.stick_frame()
        self.cone_between("stick_shaft", "stick", "stick_root", stick, top, heel, p["stick_r_top"], p["stick_r_heel"], verts=10)
        if p["knob"]:
            self.cylinder_between("knob", "trim", "stick_root", stick, top - d * 0.02, top + d * 0.04, p["stick_r_top"] * 1.25, verts=10)
        self.sphere("heel", "stick", "stick_root", stick, p["heel_r"], heel.x, heel.y, p["blade_h"] * 0.5, seg=12, rings=6)
        bc = heel + bdir * (p["stick_blade"] * 0.5)
        ang = math.degrees(math.atan2(bdir.y, bdir.x))
        self.tapered_blade("blade", "stick", "stick_root", stick, heel, bdir, out, p["stick_blade"], p["blade_t"], p["blade_t_toe"], p["blade_h"], p["blade_h_toe"])
        tc = heel + bdir * (p["stick_blade"] - p["tape_len"] * 0.5 - 0.06)
        tt = lerp(p["blade_t"], p["blade_t_toe"], (p["stick_blade"] - p["tape_len"] * 0.5 - 0.06) / p["stick_blade"])
        th = lerp(p["blade_h"], p["blade_h_toe"], (p["stick_blade"] - p["tape_len"] * 0.5 - 0.06) / p["stick_blade"])
        self.box("tape", "trim", "stick_root", stick, (p["tape_len"], tt + 0.01, th + 0.008), (tc.x, tc.y, th * 0.5), rot_z_deg=ang)
        # two mitten hands: the upper at the top of the shaft, the lower a third down, with a cuff each on the broadcast figure
        self.hands = {}
        for nm, t in (("hand_upper", p["hand_upper"]), ("hand_lower", p["hand_lower"])):
            hp = top + d * t
            self.hands[nm] = hp
            self.sphere(nm, "trim", "stick_root", stick, p["hand_r"], hp.x, hp.y, hp.z, scale=(1.25, 1.0, 0.85), seg=16, rings=8)
            if p["gloves"]:
                cuff_to = hp + Vector((-0.06, 0.02 * (1 if nm == "hand_upper" else -1), 0.09))
                self.cylinder_between(nm + "_cuff", "trim", "stick_root", stick, hp, cuff_to, p["hand_r"] * 0.85, verts=12)
        # arms: shoulder to hand, left arm to the upper hand (a right-handed shooter), right arm to the lower hand
        self.shoulders = {"arm_l": Vector((0.0, p["shoulder_y"], p["shoulder_z"])), "arm_r": Vector((0.0, -p["shoulder_y"], p["shoulder_z"]))}
        self.arm_hand = {"arm_l": "hand_upper", "arm_r": "hand_lower"}
        self.elbows = {}
        for bone, hand in self.arm_hand.items():
            sh, hp = self.shoulders[bone], self.hands[hand]
            self.sphere(bone + "_shoulder", "base", "torso", body, p["arm_r"] * 1.15, sh.x, sh.y, sh.z, seg=12, rings=6)
            mid = (sh + hp) * 0.5
            ox, oy, oz = p["elbow_offset"]
            el = mid + Vector((ox, oy if bone == "arm_l" else -oy, oz))
            self.elbows[bone] = el
            # two bones per arm: <arm>_upper (shoulder to elbow) and <arm>_fore (elbow to hand), posed by IK and baked to plain keys
            self.cylinder_between(bone + "_upper", "base", bone + "_upper", body, sh, el, p["arm_r"], verts=12)
            self.sphere(bone + "_elbow", "base", bone + "_fore", body, p["arm_r"] * 1.1, el.x, el.y, el.z, seg=12, rings=6)
            self.cylinder_between(bone + "_fore", "base", bone + "_fore", body, el, hp, p["arm_r"] * 0.9, verts=12)
        self.puck_socket_pos = bc + out * (p["blade_t"] * 0.5 + p["puck_front"])
        self.puck_socket_pos.z = 0.0
        self.stick_socket_pos = top.copy()
        self.heel = heel

    def _body_r_at(self, z):
        p = self.p
        t = clamp01((z - p["body_bottom"]) / (p["body_top"] - p["body_bottom"]))
        return lerp(p["hip_w"] * 0.5, p["body_w"] * 0.5, t)

    def join(self, parts, name):
        bpy.ops.object.select_all(action="DESELECT")
        for o in parts:
            o.select_set(True)
        bpy.context.view_layer.objects.active = parts[0]
        bpy.ops.object.join()
        o = bpy.context.active_object
        o.name = name
        o.data.name = name
        return o

    def clay_pass(self, obj):
        p = self.p
        bpy.ops.object.select_all(action="DESELECT")
        obj.select_set(True)
        bpy.context.view_layer.objects.active = obj
        sub = obj.modifiers.new("subsurf", "SUBSURF")
        sub.subdivision_type = "SIMPLE"   # split only: keeps the toy's shapes, adds the vertices the dimples need
        sub.levels = 2
        sub.render_levels = 2
        bpy.ops.object.modifier_apply(modifier=sub.name)
        tex = bpy.data.textures.new("dimples", "CLOUDS")
        tex.noise_scale = cm(p["dimple_scale"])
        tex.noise_depth = 1
        tex.intensity = 1.0
        disp = obj.modifiers.new("dimples", "DISPLACE")
        disp.texture = tex
        disp.strength = cm(p["dimple_strength"])
        disp.mid_level = 0.5
        disp.direction = "NORMAL"
        disp.texture_coords = "LOCAL"
        bpy.ops.object.modifier_apply(modifier=disp.name)

    def seam_parts(self):
        # a felt stitch seam: a thin ring around the body waist and one over the head, with short stitch bars
        p = self.p
        parts = []
        zc = p["body_bottom"] + (p["body_top"] - p["body_bottom"]) * 0.5
        r = self._body_r_at(zc) + 0.006
        bpy.ops.mesh.primitive_torus_add(major_radius=cm(r), minor_radius=cm(0.018), major_segments=32, minor_segments=8, location=V(0, 0, zc))
        self._finish(bpy.context.active_object, "seam_waist", "trim", "torso", parts)
        for k in range(16):
            a = 2 * math.pi * k / 16 + 0.1
            self.box("stitch_%d" % k, "number", "torso", parts, (0.03, 0.02, 0.07), ((r + 0.004) * math.cos(a), (r + 0.004) * math.sin(a), zc), rot_z_deg=math.degrees(a))
        # the head seam runs over the top, front to back
        hr = p["head_r"] + 0.004
        bpy.ops.mesh.primitive_torus_add(major_radius=cm(hr), minor_radius=cm(0.014), major_segments=32, minor_segments=8, location=V(0, 0, p["head_z"]), rotation=(rad(90), 0, 0))
        self._finish(bpy.context.active_object, "seam_head", "trim", "head", parts)
        return parts

    def assemble(self):
        p = self.p
        if p["clay"]:
            # dents only on the torso (the first three body parts: cone, dome, bottom); limbs and head keep their low count
            torso = self.join(self.parts_body[:3], "figure_torso")
            self.clay_pass(torso)
            body = self.join([torso] + self.parts_body[3:], "figure_body")
        else:
            body = self.join(self.parts_body, "figure_body")
        parts = [body] + self.parts_stick
        if p["seam"]:
            parts += self.seam_parts()
        mesh = self.join(parts, "SK_Skater" + VARIANT.capitalize())
        try:
            bpy.ops.object.shade_smooth_by_angle(angle=rad(42))
        except Exception as e:  # older operator name
            print("shade_smooth_by_angle failed:", e)
        self.mesh = mesh
        mesh.data.calc_loop_triangles()
        self.stats["triangles"] = len(mesh.data.loop_triangles)
        self.stats["vertices"] = len(mesh.data.vertices)
        self.stats["material_slots"] = [m.name for m in mesh.data.materials]
        return mesh

    # ---- armature
    def build_armature(self):
        p = self.p
        bpy.ops.object.armature_add(enter_editmode=False, location=(0, 0, 0))
        arm = bpy.context.active_object
        # the armature object's name prefixes every FBX take ("<object>|<action>"); Unreal turns "AS_SkaterToy|idle" into the contract name AS_SkaterToy_idle
        arm.name = "AS_Skater" + VARIANT.capitalize()
        arm.data.name = "SKEL_Skater" + VARIANT.capitalize()
        arm.show_in_front = True
        bpy.ops.object.mode_set(mode="EDIT")
        eb = arm.data.edit_bones
        for b in list(eb):
            eb.remove(b)
        hip_z = p["body_bottom"] + 0.14 if not p["legs"] else p["leg_top"] - 0.02
        top, d, heel, bdir, out = self.stick_frame()
        spec = [
            # name, head, tail, parent, deform
            ("root", (0, 0, 0), (0, 0, 0.05), None, True),
            ("pelvis", (0, 0, 0.02), (0, 0, hip_z), "root", True),
            ("torso", (0, 0, hip_z), (0, 0, p["body_top"]), "pelvis", True),
            ("head", (0, 0, p["body_top"]), (0, 0, p["head_z"] + p["head_r"]), "torso", True),
            ("arm_l_upper", tuple(self.shoulders["arm_l"]), tuple(self.elbows["arm_l"]), "torso", True),
            ("arm_l_fore", tuple(self.elbows["arm_l"]), tuple(self.hands["hand_upper"]), "arm_l_upper", True),
            ("arm_r_upper", tuple(self.shoulders["arm_r"]), tuple(self.elbows["arm_r"]), "torso", True),
            ("arm_r_fore", tuple(self.elbows["arm_r"]), tuple(self.hands["hand_lower"]), "arm_r_upper", True),
            ("stick_root", tuple(top), tuple(heel), "torso", True),
            ("ik_hand_l", tuple(self.hands["hand_upper"]), tuple(self.hands["hand_upper"] + Vector((0, 0, 0.05))), "stick_root", False),
            ("ik_hand_r", tuple(self.hands["hand_lower"]), tuple(self.hands["hand_lower"] + Vector((0, 0, 0.05))), "stick_root", False),
            ("stick_socket", tuple(top), tuple(top + Vector((0, 0, 0.05))), "stick_root", False),
            ("puck_socket", tuple(self.puck_socket_pos), tuple(self.puck_socket_pos + Vector((0, 0, 0.05))), "stick_root", False),
            ("head_socket", (0, 0, p["head_z"] + p["head_r"] + 0.25), (0, 0, p["head_z"] + p["head_r"] + 0.30), "head", False),
        ]
        for name, h, t, parent, deform in spec:
            b = eb.new(name)
            b.head = V(*h)
            b.tail = V(*t)
            b.use_deform = deform
            if parent:
                b.parent = eb[parent]
                b.use_connect = False
        bpy.ops.object.mode_set(mode="OBJECT")
        self.arm = arm
        # rest matrices in armature space
        self.rest = {b.name: b.matrix_local.copy() for b in arm.data.bones}
        for pb in arm.pose.bones:
            pb.rotation_mode = "QUATERNION"
        # arms reach their hand on the stick by a two-bone IK to a helper bone that rides stick_root; bake_arms() turns the result into plain keys
        for bone in ("arm_l", "arm_r"):
            pb = arm.pose.bones[bone + "_fore"]
            c = pb.constraints.new("IK")
            c.target = arm
            c.subtarget = "ik_hand_" + bone[-1]
            c.chain_count = 2
            c.use_tail = True
            c.use_stretch = False
        # skin
        mesh = self.mesh
        mesh.parent = arm
        mod = mesh.modifiers.new("Armature", "ARMATURE")
        mod.object = arm
        mod.use_vertex_groups = True
        self.stats["bones"] = [b.name for b in arm.data.bones]

    # ---- poses -> actions
    def apply_channels(self, ch, frame):
        """ch: dict of Unreal-convention channels; writes a key on every driven bone at `frame`."""
        p = self.p
        arm = self.arm
        g = lambda k, d=0.0: ch.get(k, d)
        hip = self.rest["torso"].translation
        head_pt = self.rest["head"].translation
        grip = self.rest["stick_root"].translation
        M = {}
        M["root"] = Matrix.Identity(4)
        # lean node: tip (fall) and roll about the ground point, body shift forward
        M["pelvis"] = T(V(g("shift"), 0, g("crouch"))) @ about(Vector((0, 0, 0)), R_pitch(g("tip")) @ R_roll(g("tip_roll")) @ R_roll(g("roll")))
        # body pivot: bob, yaw sway, pitch about the hip
        M["torso"] = T(V(0, 0, g("bob"))) @ about(hip, R_yaw(g("sway")) @ R_pitch(g("body_pitch")))
        M["head"] = about(head_pt, R_pitch(g("head_pitch")))
        # the stick is a child of the torso: its pitch compensates the body pitch so a pose's stick_pitch is relative to the ice (0 = blade lying flat)
        # the slide is keyed before the torso's transform is applied, so it is pre-rotated by the inverse torso rotation to stay horizontal on the ice;
        # the torso's pitch also carries the grip forward and down, which is cancelled so the blade stays where it is (the hands slide on the body instead)
        gm = (M["torso"] @ grip.to_4d()).to_3d()
        delta = gm - grip
        slide_world = V(g("stick_fwd"), g("stick_side"), g("stick_up")) - Vector((0.0, 0.0, delta.z))   # only the drop is cancelled: a crouch may push the stick forward
        slide = (R_pitch(-g("body_pitch")) @ R_yaw(-g("sway")) @ slide_world.to_4d()).to_3d()
        M["stick_root"] = T(slide) @ about(grip, R_yaw(g("stick_yaw")) @ R_pitch(g("stick_pitch") - g("body_pitch")) @ R_roll(g("stick_roll")))
        for name, m in M.items():
            pb = arm.pose.bones[name]
            ML = self.rest[name]
            pb.matrix_basis = ML.inverted() @ m @ ML
            pb.keyframe_insert("location", frame=frame)
            pb.keyframe_insert("rotation_quaternion", frame=frame)
            pb.keyframe_insert("scale", frame=frame)

    def make_action(self, name, nframes, fn, step=1, loop=False):
        arm = self.arm
        if arm.animation_data is None:
            arm.animation_data_create()
        act = bpy.data.actions.new(name)
        arm.animation_data.action = act
        frames = list(range(0, nframes + 1, step))
        if frames[-1] != nframes:
            frames.append(nframes)
        for f in frames:
            self.apply_channels(fn(f), f)
        act.use_frame_range = True
        act.frame_start = 0
        act.frame_end = nframes
        act.use_cyclic = loop
        act.use_fake_user = True
        for fc in act.fcurves:
            for kp in fc.keyframe_points:
                kp.interpolation = "BEZIER"
                kp.handle_left_type = "AUTO_CLAMPED"
                kp.handle_right_type = "AUTO_CLAMPED"
        return act

    def build_actions(self):
        REST_PITCH = -10.0

        # ToySkaterVisual.cpp: ShotStickYaw keys 0/18/25/34/40/51 -> 0/-100/10/75/75/0, smoothstep between keys
        def shot_yaw(f):
            kf = [0, 18, 25, 34, 40, 51]
            kv = [0.0, -65.0, 6.0, 50.0, 50.0, 0.0]   # the code's -100/10/75 scaled to what two-bone arms reach around the body
            if f <= 0 or f >= 51:
                return 0.0
            for i in range(5):
                if f <= kf[i + 1]:
                    t = (f - kf[i]) / float(kf[i + 1] - kf[i])
                    return lerp(kv[i], kv[i + 1], smooth01(t))
            return 0.0

        def check01(f):
            if f < 0 or f >= 20:
                return 0.0
            return smooth01(f / 8.0) if f <= 8 else 1.0 - smooth01((f - 8) / 12.0)

        def knock_tip(f):
            if f < 14:
                t = f / 14.0
                return 88.0 * t * t
            if f < 17:
                return lerp(88.0, 80.0, (f - 14) / 3.0)
            if f < 20:
                return lerp(80.0, 88.0, (f - 17) / 3.0)
            return 88.0

        def idle(f):
            w = 2 * math.pi * f / 60.0
            return dict(body_pitch=REST_PITCH + 0.6 * math.sin(w), bob=0.004 * math.sin(w), stick_yaw=1.5 * math.sin(w), head_pitch=2.0 * math.sin(w))

        def skate_fn(lean):
            # lean: Unreal roll of the whole figure, negative = left side down = a left turn; at most lean_deg, the game caps what it adds on top
            def skate(f):
                ph = f / 36.0
                return dict(body_pitch=-16.0, sway=4.0 * math.sin(2 * math.pi * ph) + 0.3 * lean, bob=0.04 * abs(math.sin(math.pi * ph)),
                            roll=lean - 4.0 * math.sin(2 * math.pi * ph), stick_yaw=-3.0 * math.sin(2 * math.pi * ph) - 0.4 * lean,
                            stick_fwd=0.02 * math.sin(2 * math.pi * ph), head_pitch=0.0)
            return skate
        skate = skate_fn(0.0)
        LEAN = self.p["lean_deg"]

        def carry(f):
            w = 2 * math.pi * f / 60.0
            return dict(body_pitch=-13.0, stick_yaw=-10.0 + 8.0 * math.sin(w), stick_fwd=0.10, stick_side=0.02 * math.sin(w), head_pitch=-6.0)

        def aim_left(f):
            return dict(body_pitch=-12.0, sway=-15.0, stick_yaw=-70.0, stick_fwd=0.20, stick_side=0.10, head_pitch=-4.0)

        def aim_right(f):
            return dict(body_pitch=-12.0, sway=15.0, stick_yaw=70.0, stick_fwd=0.20, stick_side=-0.10, head_pitch=-4.0)

        def poke(f):
            k = smooth01(f / 6.0) if f <= 6 else 1.0 - smooth01((f - 6) / 12.0)
            return dict(body_pitch=-12.0 - 10.0 * k, stick_fwd=0.30 * k, stick_pitch=6.0 * k, stick_yaw=-8.0 * k, shift=0.08 * k)

        def lift(f):
            k = math.sin(math.pi * f / 20.0)
            return dict(body_pitch=-12.0, stick_pitch=22.0 * k, stick_fwd=0.05 * k, stick_yaw=-5.0 * k)

        def shoot_windup(f):
            k = smooth01(f / 18.0)
            return dict(body_pitch=-14.0 - 4.0 * k, stick_yaw=-40.0 * k, stick_pitch=-15.0 * k, sway=-6.0 * k)

        def shoot_release(f):
            y = shot_yaw(f)
            return dict(body_pitch=-14.0 - 6.0 * math.sin(math.pi * clamp01(f / 51.0)), stick_yaw=y, sway=0.3 * y, stick_pitch=-8.0 * math.sin(math.pi * clamp01(f / 34.0)))

        def check(f):
            c = check01(f)
            return dict(body_pitch=REST_PITCH - 30.0 * c, shift=0.15 * c, stick_yaw=10.0 * c, head_pitch=-5.0 * c)

        def knocked_down(f):
            t = knock_tip(min(f, 20))
            k = clamp01(f / 14.0)
            return dict(tip=t, tip_roll=-10.0 * k, body_pitch=REST_PITCH + 10.0 * k, stick_yaw=35.0 * k, stick_pitch=10.0 * k, head_pitch=8.0 * k)

        def get_up(f):
            t = clamp01(f / 20.0)
            ease = 1.0 - (1.0 - t) * (1.0 - t)
            return dict(tip=88.0 * (1.0 - ease), tip_roll=-10.0 * (1.0 - ease), body_pitch=REST_PITCH + 10.0 * (1.0 - ease), stick_yaw=35.0 * (1.0 - ease), stick_pitch=10.0 * (1.0 - ease))

        def faceoff_ready(f):
            return dict(body_pitch=-35.0, bob=-0.08, stick_fwd=0.25, stick_pitch=8.0, stick_yaw=20.0, head_pitch=14.0)

        def draw(f):
            k = smooth01(f / 6.0) if f <= 6 else 1.0 - smooth01((f - 6) / 9.0)
            return dict(body_pitch=-35.0 - 5.0 * k, bob=-0.08 - 0.02 * k, stick_fwd=0.25 - 0.25 * k, stick_pitch=8.0, stick_yaw=20.0 - 50.0 * k, head_pitch=14.0)

        def goalie(side):
            def fn(f):
                w = 2 * math.pi * f / 30.0
                ch = dict(body_pitch=-25.0, bob=-0.10 + 0.004 * math.sin(w), stick_yaw=35.0, stick_pitch=6.0, stick_fwd=0.15, head_pitch=10.0)
                if side < 0:
                    ch.update(roll=-12.0, stick_yaw=-25.0, stick_side=0.12, sway=-10.0)
                elif side > 0:
                    ch.update(roll=12.0, stick_yaw=70.0, stick_side=-0.12, sway=10.0)
                return ch
            return fn

        plan = [
            ("idle", 60, idle, 6, True),
            ("skate", 36, skate, 3, True),
            ("skate_left", 36, skate_fn(-LEAN), 3, True),
            ("skate_right", 36, skate_fn(LEAN), 3, True),
            ("carry", 60, carry, 6, True),
            ("aim_left", 30, aim_left, 30, False),
            ("aim_right", 30, aim_right, 30, False),
            ("poke", 18, poke, 2, False),
            ("lift", 20, lift, 2, False),
            ("shoot_windup", 24, shoot_windup, 3, False),
            ("shoot_release", 51, shoot_release, 1, False),
            ("check", 20, check, 2, False),
            ("knocked_down", 40, knocked_down, 2, False),
            ("get_up", 20, get_up, 2, False),
            ("faceoff_ready", 30, faceoff_ready, 30, False),
            ("draw", 15, draw, 1, False),
            ("goalie_left", 30, goalie(-1), 10, True),
            ("goalie_centre", 30, goalie(0), 10, True),
            ("goalie_right", 30, goalie(1), 10, True),
        ]
        self.actions = {}
        for name, n, fn, step, loop in plan:
            self.actions[name] = self.make_action(name, n, fn, step=step, loop=loop)
        self.bake_arms()
        self.arm.animation_data.action = self.actions["idle"]
        self.stats["actions"] = {k: int(v.frame_end) for k, v in self.actions.items()}

    def bake_arms(self):
        """write the IK result of the four arm bones into every action as plain location/rotation keys per frame, then drop the IK constraints
        so the FBX carries nothing a game engine has to re-solve (the first pass's STRETCH_TO scale did not survive the import, lane A26).
        Done by hand (evaluate, convert the constrained pose to the bone's local basis, key it) because nla.bake wrote rest values headless."""
        arm = self.arm
        arm_bones = ("arm_l_upper", "arm_l_fore", "arm_r_upper", "arm_r_fore")
        baked = {}
        worst = 0.0
        per_action = {}
        for name, act in self.actions.items():
            arm.animation_data.action = act
            frames = {}
            per_action[name] = 0.0
            for f in range(0, int(act.frame_end) + 1):
                self.scene.frame_set(f)
                bpy.context.view_layer.update()
                frames[f] = {bn: arm.convert_space(pose_bone=arm.pose.bones[bn], matrix=arm.pose.bones[bn].matrix, from_space="POSE", to_space="LOCAL").copy() for bn in arm_bones}
                for fore, hand in (("arm_l_fore", "ik_hand_l"), ("arm_r_fore", "ik_hand_r")):
                    gap = (arm.pose.bones[fore].tail - arm.pose.bones[hand].head).length
                    worst = max(worst, gap)
                    per_action[name] = max(per_action[name], gap)
            baked[name] = frames
        self.stats["ik_worst_gap_cm"] = round(worst, 2)
        self.stats["ik_gap_cm"] = {k: round(v, 1) for k, v in per_action.items()}
        for bn in ("arm_l_fore", "arm_r_fore"):
            pb = arm.pose.bones[bn]
            for c in list(pb.constraints):
                pb.constraints.remove(c)
        for name, act in self.actions.items():
            arm.animation_data.action = act
            for f, per_bone in baked[name].items():
                for bn, m in per_bone.items():
                    pb = arm.pose.bones[bn]
                    pb.matrix_basis = m
                    pb.keyframe_insert("location", frame=f)
                    pb.keyframe_insert("rotation_quaternion", frame=f)
                    pb.keyframe_insert("scale", frame=f)
            for fc in act.fcurves:
                for kp in fc.keyframe_points:
                    kp.interpolation = "LINEAR"
        # proof: with the constraints gone, the keyed arms must still end at the mittens
        post = 0.0
        for name, act in self.actions.items():
            arm.animation_data.action = act
            for f in range(0, int(act.frame_end) + 1, 3):
                self.scene.frame_set(f)
                bpy.context.view_layer.update()
                for fore, hand in (("arm_l_fore", "ik_hand_l"), ("arm_r_fore", "ik_hand_r")):
                    post = max(post, (arm.pose.bones[fore].tail - arm.pose.bones[hand].head).length)
        self.stats["baked_worst_gap_cm"] = round(post, 2)
        self.stats["arms_baked"] = len(self.actions)

    # ---- export
    def export_fbx(self, path):
        bpy.ops.object.select_all(action="DESELECT")
        self.arm.select_set(True)
        self.mesh.select_set(True)
        bpy.context.view_layer.objects.active = self.arm
        bpy.context.scene.frame_set(0)
        bpy.ops.export_scene.fbx(
            filepath=path,
            use_selection=True,
            object_types={"ARMATURE", "MESH"},
            apply_unit_scale=True,
            apply_scale_options="FBX_SCALE_NONE",
            global_scale=1.0,
            axis_forward="-Z",
            axis_up="Y",
            use_mesh_modifiers=True,
            mesh_smooth_type="EDGE",
            use_tspace=False,
            add_leaf_bones=False,
            primary_bone_axis="Y",
            secondary_bone_axis="X",
            use_armature_deform_only=False,
            armature_nodetype="NULL",
            bake_anim=True,
            bake_anim_use_all_bones=True,
            bake_anim_use_nla_strips=False,
            bake_anim_use_all_actions=True,
            bake_anim_force_startend_keying=True,
            bake_anim_step=1.0,
            bake_anim_simplify_factor=0.0,
            path_mode="AUTO",
            embed_textures=False,
        )
        self.stats["fbx"] = path
        self.stats["fbx_mb"] = round(os.path.getsize(path) / 1e6, 2)

    # ---- renders
    def setup_render(self):
        p = self.p
        sc = self.scene
        sc.render.engine = "BLENDER_EEVEE_NEXT"
        sc.eevee.taa_render_samples = 24
        sc.render.image_settings.file_format = "PNG"
        sc.render.image_settings.color_mode = "RGB"
        sc.render.film_transparent = False
        sc.view_settings.view_transform = "Standard"
        sc.view_settings.look = "None"
        # ice
        bpy.ops.mesh.primitive_plane_add(size=cm(40), location=(0, 0, 0))
        ice = bpy.context.active_object
        ice.name = "ice"
        mi = bpy.data.materials.new("ice")
        mi.use_nodes = True
        b = mi.node_tree.nodes["Principled BSDF"]
        b.inputs["Base Color"].default_value = (0.86, 0.90, 0.95, 1)
        b.inputs["Roughness"].default_value = 0.35
        ice.data.materials.append(mi)
        # world
        w = bpy.data.worlds.new("world")
        w.use_nodes = True
        bg = w.node_tree.nodes["Background"]
        bg.inputs[0].default_value = (0.16, 0.17, 0.20, 1)
        bg.inputs[1].default_value = 1.0
        sc.world = w
        # lights (sun strength is unit-independent)
        bpy.ops.object.light_add(type="SUN", location=V(2, -3, 5))
        key = bpy.context.active_object
        key.data.energy = 3.2
        key.data.angle = rad(8)
        key.rotation_euler = (rad(48), 0, rad(-35))
        bpy.ops.object.light_add(type="SUN", location=V(-3, 3, 4))
        fill = bpy.context.active_object
        fill.data.energy = 0.9
        fill.data.use_shadow = False
        fill.rotation_euler = (rad(60), 0, rad(140))
        # rim light from behind the figure toward the camera: lifts the clay dents and the seam off the silhouette
        bpy.ops.object.light_add(type="SUN", location=V(-3, 2, 3))
        rim = bpy.context.active_object
        rim.data.energy = 2.6
        rim.data.color = (1.0, 0.93, 0.82)
        rim.data.use_shadow = False
        rim.rotation_euler = (rad(62), 0, rad(-150))
        # a puck for the carry cell, placed at puck_socket when that cell renders (a render prop, never exported)
        bpy.ops.mesh.primitive_cylinder_add(vertices=24, radius=cm(0.12), depth=cm(0.08), location=V(0, 0, 0.04))
        puck = bpy.context.active_object
        puck.name = "render_puck"
        mp = bpy.data.materials.new("puck")
        mp.use_nodes = True
        mp.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.02, 0.02, 0.02, 1)
        puck.data.materials.append(mp)
        puck.hide_render = True
        self.puck = puck
        # camera on a pivot
        bpy.ops.object.empty_add(location=(0, 0, 0))
        pivot = bpy.context.active_object
        pivot.name = "cam_pivot"
        h = p["height"]
        bpy.ops.object.camera_add(location=V(2.05 * h, 1.4 * h, 1.25 * h))
        cam = bpy.context.active_object
        cam.data.lens = 46
        cam.data.clip_end = cm(200)
        cam.parent = pivot
        bpy.ops.object.empty_add(location=V(0.5, -0.2, h * 0.4))
        target = bpy.context.active_object
        target.name = "cam_target"
        c = cam.constraints.new("TRACK_TO")
        c.target = target
        c.track_axis = "TRACK_NEGATIVE_Z"
        c.up_axis = "UP_Y"
        sc.camera = cam
        # label text parented to the camera
        bpy.ops.object.text_add()
        txt = bpy.context.active_object
        txt.name = "label"
        txt.data.size = 7.0
        txt.data.body = ""
        txt.parent = cam
        txt.location = (-62.0, -52.0, -170.0)
        txt.rotation_euler = (0, 0, 0)
        mt = bpy.data.materials.new("label")
        mt.use_nodes = True
        nb = mt.node_tree.nodes["Principled BSDF"]
        nb.inputs["Emission Color"].default_value = (1, 1, 1, 1)
        nb.inputs["Emission Strength"].default_value = 3.0
        nb.inputs["Base Color"].default_value = (1, 1, 1, 1)
        txt.data.materials.append(mt)
        self.cam, self.pivot, self.label = cam, pivot, txt

    def render_to(self, path, px):
        sc = self.scene
        sc.render.resolution_x = px
        sc.render.resolution_y = px
        sc.render.resolution_percentage = 100
        sc.render.filepath = path
        bpy.ops.render.render(write_still=True)

    def render_cells(self, outdir, variant_index):
        t = time.time()
        for i, (action, frame) in enumerate(SHEET_POSES):
            self.arm.animation_data.action = self.actions[action]
            self.scene.frame_set(frame)
            bpy.context.view_layer.update()
            if action == "carry":
                pos = (self.arm.matrix_world @ self.arm.pose.bones["puck_socket"].matrix).translation
                self.puck.location = (pos.x, pos.y, cm(0.04))
                self.puck.hide_render = False
            else:
                self.puck.hide_render = True
            self.render_to(os.path.join(outdir, "cell_%02d.png" % (variant_index * 3 + i + 1)), CELL_PX)
        self.stats["render_cells_s"] = round(time.time() - t, 1)

    def render_turntable(self, outdir):
        t = time.time()
        self.arm.animation_data.action = self.actions["idle"]
        self.puck.hide_render = True
        for f in range(TURNTABLE_FRAMES):
            self.scene.frame_set(f)
            self.pivot.rotation_euler = (0, 0, 2 * math.pi * f / TURNTABLE_FRAMES)
            self.render_to(os.path.join(outdir, "%s_tt_%03d.png" % (VARIANT, f)), TURNTABLE_PX)
        self.stats["render_turntable_s"] = round(time.time() - t, 1)


def main():
    global VARIANT
    opts = parse_args()
    VARIANT = opts["variant"]
    if VARIANT not in VARIANTS:
        raise SystemExit("unknown variant %s" % VARIANT)
    p = dict(BASE)
    p.update(VARIANTS[VARIANT])
    out = os.path.abspath(opts["out"])
    os.makedirs(out, exist_ok=True)
    b = Build(p)
    b.reset_scene()
    t = time.time()
    b.build_mesh()
    b.assemble()
    b.build_armature()
    b.build_actions()
    b.stats["build_s"] = round(time.time() - t, 1)
    b.stats["variant"] = VARIANT
    if opts["export"]:
        t = time.time()
        b.export_fbx(os.path.join(out, "SK_Skater%s.fbx" % VARIANT.capitalize()))
        b.stats["export_s"] = round(time.time() - t, 1)
    if opts["render"] or opts["turntable"]:
        rdir = os.path.join(out, "render")
        os.makedirs(rdir, exist_ok=True)
        b.setup_render()
        if opts["render"]:
            b.render_cells(rdir, list(VARIANTS).index(VARIANT))
        if opts["turntable"]:
            b.render_turntable(rdir)
    b.stats["total_s"] = round(time.time() - T0, 1)
    with open(os.path.join(out, "SK_Skater%s.json" % VARIANT.capitalize()), "w") as fh:
        json.dump(b.stats, fh, indent=2)
    print("A22 skater.py PASS", json.dumps(b.stats))


main()
