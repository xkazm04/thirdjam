"""tools/blender/arena/arena.py: the arena dressing in the clay voice (Third Jam, team Hokej, track F3, lanes A103 and A119), Blender 4.2.

Run headless from the repository root:
  blender -b -P tools/blender/arena/arena.py -- --length 120 --width 60 --out tools/blender/arena/out [--render | --render2] [--img docs/experiments/img/arena]

Evolution 2 (lane A119) adds, further down (GOAL_VARIANTS, goal(), scorer_table(), renders2()):
  SM_Arena_Goal_W<w>H<h>     a goal frame built for an opening w x h (game.rink.goal_width_m x goal_height_m, centimetres in the name): the posts' inner
                             faces at +-w/2 and the crossbar's underside at h are the sim's goal volume; slot frame. One per entry of GOAL_VARIANTS.
  SM_Arena_GoalNet_W<w>H<h>  its net: real cords from bar to bar, slot net; a mesh of its own so Unreal can leave the cords out of the outline stencil.
  SM_Arena_ScorerTable       the scorer's table between the two penalty boxes; slots board, kick, bench, dark, frame.
A goal's frame: origin on the goal line in the middle of the mouth, +x along the line, "out" = into the net. --render2 writes evolution 2's stills.

Builds two static meshes, each under 3,000 triangles, each with UVs (world scale, 1 UV unit = 2 m) and five material SLOTS and no material of its own
(the slots are names; Unreal binds device B's instances to them by reference, see tools/blender/arena/import_arena.py and RinkActor.cpp):
  SM_Arena_TeamBench   a players' bench: floor, back wall with a kick band, two end walls, a thumb-pressed cap rail, a seat plank on legs,
                       a backrest, two doors standing open at the board line. Length = 0.1 x the rink's length (A25's rule in RinkActor.cpp).
  SM_Arena_PenaltyBox  a penalty box: floor, back wall, two side walls, cap rail, a short seat, one door, a clock on the back wall,
                       and the socket SOCKET_Seat (Unreal socket "Seat"): where a boxed skater stands.

Slots: board (walls), kick (kick band, seat, backrest), bench (floor), team (cap rail, doors), dark (legs, hinges, handles, the clock's face).

Frame of each piece: origin on the ice (z = 0) at the middle of its rink-side edge; +x along the boards; the piece extends AWAY from the rink.
"out" (away from the rink) is Blender -Y, because Unreal's FBX import flips Y: in Unreal the piece extends along +Y. RinkActor turns the pieces on the
-y long side by 180 degrees.

Everything is deterministic: the unevenness is Perlin noise of the position (mathutils.noise) with fixed offsets; two runs give the same mesh.
Units as tools/blender/props.py: scene unit scale 0.01 (1 unit = 1 cm), parameters below in metres.
"""
import bpy
import bmesh
import json
import math
import os
import sys
import time

from mathutils import Matrix, Vector, noise

T0 = time.time()
TRI_BUDGET = 3000
UV_M = 2.0            # metres per UV unit
SLOTS = ["board", "kick", "bench", "team", "dark"]
PREVIEW = {           # render-only colours (nothing of this reaches Unreal: materials are not imported)
    "board": (0.88, 0.86, 0.80, 1.0),
    "kick": (0.90, 0.60, 0.08, 1.0),
    "bench": (0.50, 0.33, 0.20, 1.0),
    "team": (0.07, 0.22, 0.80, 1.0),
    "dark": (0.06, 0.06, 0.07, 1.0),
    "frame": (0.70, 0.04, 0.03, 1.0),
    "net": (0.93, 0.93, 0.90, 1.0),
}

# ---------------------------------------------------------------- parameters (metres)

P = dict(
    bench_len_factor=0.1,   # team bench length = factor x rink length (RinkActor.cpp, lane A25)
    bench_x_factor=0.0833,  # team bench centres at +- factor x rink length
    box_len=4.4,            # penalty box length along the boards (fixed: the sim boxes a skater at x = +-3 on every rink)
    box_x=3.0,              # UJamSim::BoxPos: x = -3 (team 0), +3 (team 1)
    depth=1.6,              # how far a piece reaches away from the boards
    gap=0.05,               # between the board's outer face and the piece (RinkActor.cpp: Y0 = W/2 + T + 5 cm)
    board_t=0.2,            # ARinkActor::BoardThicknessM
    board_h=1.2,            # ARinkActor::BoardHeightM
    floor_h=0.10,
    wall_t=0.18,
    wall_h=1.30,            # the end walls: 10 cm over the boards (1.2 m)
    back_h=1.75,            # the back wall: its team-colour rail shows over the boards from the ice
    rail_w=0.28, rail_h=0.15,
    seat_h=0.46, seat_t=0.12, seat_d=0.42,
    door_w=1.05, door_h=1.08, door_t=0.09, door_open_deg=72.0,
    wobble=0.012,           # the clay's unevenness, metres
    press_r=0.27, press_depth=0.05, press_every=1.15,
    seat_socket=(0.35, 0.62, 0.10),   # where a boxed skater stands in the penalty box (piece space: x, out, z)
)


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    o = {"length": 120.0, "width": 60.0, "out": "tools/blender/arena/out", "render": False, "render2": False, "img": ""}
    i = 0
    while i < len(argv):
        a = argv[i]
        if a in ("--length", "--width"):
            o[a[2:]] = float(argv[i + 1]); i += 1
        elif a in ("--out", "--img"):
            o[a[2:]] = argv[i + 1]; i += 1
        elif a == "--render":
            o["render"] = True
        elif a == "--render2":     # evolution 2's stills (the goal, the scorer's table); use it without --render
            o["render2"] = True
        i += 1
    return o


def cm(v):
    return v * 100.0


# ---------------------------------------------------------------- the one primitive: a clay bar

def section(w, h, r, segs):
    """Rounded rectangle w x h, corner radius r, segs arc steps per corner; counter-clockwise from the +a side. Returns [(a, b)], [perimeter at each point], total."""
    r = min(r, w * 0.5 - 1e-4, h * 0.5 - 1e-4)
    pts = []
    centres = [(w * 0.5 - r, h * 0.5 - r), (-(w * 0.5 - r), h * 0.5 - r), (-(w * 0.5 - r), -(h * 0.5 - r)), (w * 0.5 - r, -(h * 0.5 - r))]
    for c, (ca, cb) in enumerate(centres):
        for s in range(segs + 1):
            ang = c * math.pi * 0.5 + (math.pi * 0.5) * s / segs
            pts.append((ca + math.cos(ang) * r, cb + math.sin(ang) * r))
    per = [0.0]
    for i in range(1, len(pts)):
        per.append(per[-1] + math.dist(pts[i - 1], pts[i]))
    total = per[-1] + math.dist(pts[-1], pts[0])
    return pts, per, total


class Piece:
    """One static mesh: bars lofted into a single bmesh, a material index per face, UVs per loop."""

    def __init__(self, name, seed, slots=None):
        self.name = name
        self.seed = float(seed)
        self.slots = list(slots or SLOTS)
        self.bm = bmesh.new()
        self.uv = self.bm.loops.layers.uv.new("UVMap")
        self.parts = {}
        self.rails = []      # (x0, x1, out, z_top) of each cap rail along x, and (out0, out1, x, z_top) across: for the thumb presses
        self.sockets = {}

    def _wob(self, p, amp, k):
        """Smooth, deterministic unevenness: Perlin noise of the position."""
        f = 1.3
        q = Vector((p.x * f + self.seed, p.y * f + 7.1 * k, p.z * f + 3.7))
        return Vector((noise.noise(q), noise.noise(q + Vector((11.3, 0, 0))), noise.noise(q + Vector((0, 23.9, 0))))) * amp

    def bar(self, part, slot, axis, length, w, h, centre, rings=1, r=0.035, segs=1, wobble=None, xform=None, pillow=0.012):
        """A bar `length` long along `axis` ('x', 'o' = out, 'z'), cross-section w x h with rounded corners, rounded ends, `rings` spans along it.
        Cross-section axes: axis x -> (out, z); axis o -> (x, z); axis z -> (x, out). centre = (x, out, z) metres. xform = Matrix applied in piece space."""
        wobble = P["wobble"] if wobble is None else wobble
        pts, per, total = section(w, h, r, segs)
        n = len(pts)
        e = min(r, length * 0.25)
        ts = [-length * 0.5, -length * 0.5 + e] + [(-length * 0.5 + e) + (length - 2 * e) * i / rings for i in range(1, rings)] + [length * 0.5 - e, length * 0.5]
        inset = [True, False] + [False] * (rings - 1) + [False, True]
        cx, co, cz = centre

        def place(t, a, b):
            if axis == "x":
                v = Vector((cx + t, co + a, cz + b))
            elif axis == "o":
                v = Vector((cx + a, co + t, cz + b))
            else:
                v = Vector((cx + a, co + b, cz + t))
            return v

        ring_verts = []
        k = len(self.parts) + 1
        for t, ins in zip(ts, inset):
            row = []
            for (a, b) in pts:
                if ins:   # the end ring is the section pulled in by the corner radius: a rounded end
                    a = a * max(0.05, (w - 2 * e) / w)
                    b = b * max(0.05, (h - 2 * e) / h)
                v = place(t, a, b)
                v = v + self._wob(v, wobble, k)
                if xform is not None:
                    v = xform @ v
                row.append(self.bm.verts.new(self._blender(v)))
            ring_verts.append(row)
        mi = self.slots.index(slot)
        faces = 0
        for i in range(len(ts) - 1):
            for j in range(n):
                j2 = (j + 1) % n
                f = self.bm.faces.new((ring_verts[i][j], ring_verts[i + 1][j], ring_verts[i + 1][j2], ring_verts[i][j2]))
                f.material_index = mi
                f.smooth = True
                u0, u1 = (ts[i] - ts[0]) / UV_M, (ts[i + 1] - ts[0]) / UV_M
                v0 = per[j] / UV_M
                v1 = (per[j2] if j2 else total) / UV_M
                for loop, uvv in zip(f.loops, ((u0, v0), (u1, v0), (u1, v1), (u0, v1))):
                    loop[self.uv].uv = uvv
                faces += 2
        for end, t, sign in ((0, ts[0], -1.0), (len(ts) - 1, ts[-1], 1.0)):
            c = place(t + sign * pillow, 0.0, 0.0)
            if xform is not None:
                c = xform @ c
            cv = self.bm.verts.new(self._blender(c))
            for j in range(n):
                j2 = (j + 1) % n
                tri = (cv, ring_verts[end][j], ring_verts[end][j2]) if sign > 0 else (cv, ring_verts[end][j2], ring_verts[end][j])
                f = self.bm.faces.new(tri)
                f.material_index = mi
                f.smooth = True
                # planar UVs on the cap, beside the strip (u beyond the bar's length)
                uu = (ts[-1] - ts[0]) / UV_M + 0.05
                sc = max(0.05, (w - 2 * e) / w)
                cap = {0: (uu + w * 0.5 / UV_M, h * 0.5 / UV_M)}
                for loop, key in zip(f.loops, (None, j if sign > 0 else j2, j2 if sign > 0 else j)):
                    if key is None:
                        loop[self.uv].uv = cap[0]
                    else:
                        loop[self.uv].uv = (uu + (pts[key][0] * sc + w * 0.5) / UV_M, (pts[key][1] * sc + h * 0.5) / UV_M)
                faces += 1
        self.parts[part] = self.parts.get(part, 0) + faces
        return faces

    def cord(self, part, slot, p0, p1, thick, mid=1, sag=(0.0, 0.0, 0.0)):
        """A net cord (lane A119): a square tube from p0 to p1 (piece space, metres) with `mid` inner rings, no caps (its ends are inside the bars).
        The inner rings hang by `sag` (a parabola) and wander a little (Perlin noise of the position): clay string, not wire."""
        a, b = Vector(p0), Vector(p1)
        d = (b - a)
        length = d.length
        d = d / length
        up = Vector((0, 0, 1)) if abs(d.z) < 0.9 else Vector((1, 0, 0))
        s1 = d.cross(up).normalized()
        s2 = d.cross(s1).normalized()
        k = len(self.parts) + 1
        mi = self.slots.index(slot)
        rings = []
        for i in range(mid + 2):
            t = i / (mid + 1)
            c = a.lerp(b, t)
            if 0 < i < mid + 1:
                c = c + Vector(sag) * (4.0 * t * (1.0 - t)) + self._wob(c * 2.3, 0.008, k)
            rings.append([self.bm.verts.new(self._blender(c + (s1 * ca + s2 * cb) * (thick * 0.5))) for ca, cb in ((1, 0), (0, 1), (-1, 0), (0, -1))])
        faces = 0
        for i in range(mid + 1):
            for j in range(4):
                j2 = (j + 1) % 4
                f = self.bm.faces.new((rings[i][j], rings[i + 1][j], rings[i + 1][j2], rings[i][j2]))
                f.material_index = mi
                f.smooth = True
                u0, u1 = length * i / (mid + 1) / UV_M, length * (i + 1) / (mid + 1) / UV_M
                v0, v1 = j * thick / UV_M, (j + 1) * thick / UV_M
                for loop, uvv in zip(f.loops, ((u0, v0), (u1, v0), (u1, v1), (u0, v1))):
                    loop[self.uv].uv = uvv
                faces += 2
        self.parts[part] = self.parts.get(part, 0) + faces
        return faces

    @staticmethod
    def _blender(v):
        """Piece space (x, out, z) metres -> Blender (x, -out, z) centimetres."""
        return (cm(v.x), -cm(v.y), cm(v.z))

    def press(self, centre, radius, depth, direction=(0.0, 0.0, -1.0)):
        """A thumb: push the clay in at `centre` (piece space, metres), raise a small lip around it."""
        c = Vector(self._blender(Vector(centre)))
        d = Vector((direction[0], -direction[1], direction[2])).normalized()
        R = cm(radius)
        for v in self.bm.verts:
            dist = (v.co - c).length
            if dist < R:
                f = (1.0 - (dist / R) ** 2) ** 2
                v.co += d * cm(depth) * f
            elif dist < R * 1.7:
                g = (dist - R) / (R * 0.7)
                v.co -= d * cm(depth) * 0.22 * math.sin(g * math.pi)

    def thumbs(self):
        """Thumb presses along every cap rail: one about every press_every metres, uneven in place, depth and lean."""
        count = 0
        for (kind, a0, a1, fixed, z) in self.rails:
            span = a1 - a0
            nmax = max(1, int(round(span / P["press_every"])))
            for i in range(nmax):
                q = Vector((i * 1.71 + self.seed, 5.5 if kind == "x" else 9.5, fixed))
                jitter = noise.noise(q) * 0.35
                t = a0 + span * (i + 0.5 + jitter) / nmax
                depth = P["press_depth"] * (0.75 + 0.5 * abs(noise.noise(q + Vector((3.3, 0, 0)))))
                lean = noise.noise(q + Vector((0, 8.8, 0))) * 0.6
                side = noise.noise(q + Vector((0, 0, 4.4))) * P["rail_w"] * 0.35
                if kind == "x":
                    self.press((t, fixed + side, z), P["press_r"], depth, (0.0, lean, -1.0))
                else:
                    self.press((fixed + side, t, z), P["press_r"], depth, (lean, 0.0, -1.0))
                count += 1
        return count

    def finish(self):
        bmesh.ops.recalc_face_normals(self.bm, faces=self.bm.faces)
        me = bpy.data.meshes.new(self.name)
        self.bm.to_mesh(me)
        tris = sum(len(f.verts) - 2 for f in self.bm.faces)
        by_slot = {s: 0 for s in self.slots}
        for f in self.bm.faces:
            by_slot[self.slots[f.material_index]] += len(f.verts) - 2
        uvs = [l[self.uv].uv.copy() for f in self.bm.faces for l in f.loops]
        self.bm.free()
        for s in self.slots:
            m = bpy.data.materials.get(s) or bpy.data.materials.new(s)
            m.use_nodes = True
            b = m.node_tree.nodes["Principled BSDF"]
            b.inputs["Base Color"].default_value = PREVIEW[s]
            b.inputs["Roughness"].default_value = 0.82
            me.materials.append(m)
        ob = bpy.data.objects.new(self.name, me)
        bpy.context.scene.collection.objects.link(ob)
        mod = ob.modifiers.new("clay_normals", "WEIGHTED_NORMAL")   # flats stay flat, the chamfers carry the roundness
        mod.keep_sharp = False
        mod.weight = 60
        empties = []
        for sname, pos in self.sockets.items():
            e = bpy.data.objects.new("SOCKET_" + sname, None)
            e.empty_display_size = 10.0
            e.location = Vector(self._blender(Vector(pos)))
            bpy.context.scene.collection.objects.link(e)
            e.parent = ob
            empties.append(e)
        bpy.context.view_layer.update()
        stats = {
            "triangles": tris,
            "triangles_by_slot": by_slot,
            "triangles_by_part": dict(self.parts),
            "vertices": len(me.vertices),
            "uv_loops": len(uvs),
            "uv_min": [round(min(u.x for u in uvs), 3), round(min(u.y for u in uvs), 3)],
            "uv_max": [round(max(u.x for u in uvs), 3), round(max(u.y for u in uvs), 3)],
            "dimensions_m": [round(d / 100.0, 3) for d in ob.dimensions],
            "sockets": {k: [round(c, 3) for c in v] for k, v in self.sockets.items()},
            "under_budget": tris < TRI_BUDGET,
        }
        return ob, empties, stats


# ---------------------------------------------------------------- the pieces

def enclosure(pc, length, door_xs, seat_len, seat_x, legs, rings):
    """What a bench and a box share: floor, back wall with kick band, end walls, cap rails, seat on legs, backrest, doors."""
    D, WT, WH, FH, BH = P["depth"], P["wall_t"], P["wall_h"], P["floor_h"], P["back_h"]
    RW, RH = P["rail_w"], P["rail_h"]
    # floor
    pc.bar("floor", "bench", "x", length, D, FH, (0, D * 0.5, FH * 0.5), rings=rings["floor"], r=0.03)
    # back wall and its kick band (the boards' ochre foot, repeated outside)
    back_o = D - WT * 0.5
    pc.bar("back_wall", "board", "x", length, WT, BH - FH + 0.02, (0, back_o, FH + (BH - FH) * 0.5 - 0.01), rings=rings["wall"], r=0.04)
    pc.bar("kick_band", "kick", "x", length - 2 * WT - 0.04, 0.07, 0.25, (0, D - WT - 0.025, FH + 0.125), rings=rings["kick"], r=0.025)
    # end walls: from the board line to the back wall
    end_len = D - WT + 0.04
    for sx in (-1, 1):
        pc.bar("end_wall", "board", "o", end_len, WT, WH - FH + 0.02, (sx * (length * 0.5 - WT * 0.5), end_len * 0.5, FH + (WH - FH) * 0.5 - 0.01), rings=2, r=0.04)
    # cap rails, thumb-pressed afterwards: dense rings so a thumb has clay to move
    zr = WH + RH * 0.5 - 0.02
    zb = BH + RH * 0.5 - 0.02
    pc.bar("cap_rail", "team", "x", length + 0.06, RW, RH, (0, back_o, zb), rings=rings["rail"], r=0.05, wobble=P["wobble"] * 1.3)
    pc.rails.append(("x", -length * 0.5 + 0.2, length * 0.5 - 0.2, back_o, zb + RH * 0.5))
    side_len = D - WT + 0.04
    for sx in (-1, 1):
        x = sx * (length * 0.5 - WT * 0.5)
        pc.bar("cap_rail", "team", "o", side_len, RW, RH, (x, side_len * 0.5 - 0.02, zr), rings=rings["side_rail"], r=0.05, wobble=P["wobble"] * 1.3)
        pc.rails.append(("o", 0.15, side_len - 0.15, x, zr + RH * 0.5))
    # seat plank on chunky legs, and a backrest plank on the wall
    seat_o = D - WT - 0.07 - P["seat_d"] * 0.5
    pc.bar("seat", "kick", "x", seat_len, P["seat_d"], P["seat_t"], (seat_x, seat_o, P["seat_h"] + P["seat_t"] * 0.5), rings=rings["seat"], r=0.04)
    for i in range(legs):
        lx = seat_x - (seat_len - 0.6) * 0.5 + (seat_len - 0.6) * (i / max(1, legs - 1)) if legs > 1 else seat_x
        pc.bar("legs", "dark", "z", P["seat_h"] - FH + 0.02, 0.16, 0.30, (lx, seat_o, FH + (P["seat_h"] - FH) * 0.5), rings=1, r=0.03)
    pc.bar("backrest", "kick", "x", seat_len, 0.08, 0.20, (seat_x, D - WT - 0.03, 0.86), rings=rings["backrest"], r=0.03)
    # doors: a leaf hinged at the board line, standing open into the piece; two hinge lumps and a handle
    for dx, swing in door_xs:
        # The leaf lies along local +x from its hinge at (dx, 0.06) in piece space (x, out, z) and is turned about z by `a`:
        # swing +1 (hinge at the left end) from +x towards +out; swing -1 (hinge at the right end) from -x towards +out.
        a = math.radians(P["door_open_deg"]) if swing > 0 else math.pi - math.radians(P["door_open_deg"])
        m = Matrix.Translation(Vector((dx, 0.06, 0.0))) @ Matrix(((math.cos(a), -math.sin(a), 0, 0), (math.sin(a), math.cos(a), 0, 0), (0, 0, 1, 0), (0, 0, 0, 1)))
        zc = FH + 0.04 + P["door_h"] * 0.5
        pc.bar("door", "team", "x", P["door_w"], P["door_t"], P["door_h"], (P["door_w"] * 0.5 + 0.03, 0, zc), rings=2, r=0.035, xform=m)
        pc.bar("door_fittings", "dark", "z", P["door_h"] * 0.8, 0.10, 0.10, (0.0, 0.0, zc), rings=1, r=0.03, xform=m, wobble=0.004)                    # the hinge post
        pc.bar("door_fittings", "dark", "x", 0.24, 0.07, 0.07, (P["door_w"] - 0.20, -0.075 * swing, zc + 0.08), rings=1, r=0.025, xform=m, wobble=0.004)  # the handle


def team_bench(rink_len):
    length = P["bench_len_factor"] * rink_len
    pc = Piece("SM_Arena_TeamBench", seed=17.0)
    half = length * 0.5
    enclosure(pc, length, door_xs=[(-half + 0.45, 1.0), (half - 0.45, -1.0)], seat_len=length - 2 * P["wall_t"] - 0.5, seat_x=0.0, legs=4,
              rings=dict(floor=6, wall=8, kick=6, rail=50, side_rail=6, seat=12, backrest=6))   # fixed counts: the triangle count is the same on every rink
    pc.thumbs_done = pc.thumbs()
    return pc, length


def penalty_box():
    length = P["box_len"]
    pc = Piece("SM_Arena_PenaltyBox", seed=41.0)
    half = length * 0.5
    D, WT = P["depth"], P["wall_t"]
    enclosure(pc, length, door_xs=[(-half + 0.45, 1.0)], seat_len=2.0, seat_x=half - WT - 1.15, legs=2,
              rings=dict(floor=3, wall=4, kick=3, rail=20, side_rail=6, seat=3, backrest=2))
    # the clock on the back wall's rail: a chunky disc (rim in the board colour, a dark face, two hands as lumps), on a short post
    back_o = D - WT * 0.5
    zr = P["back_h"] + P["rail_h"] - 0.03
    pc.bar("clock", "dark", "z", 0.20, 0.10, 0.10, (0.0, back_o, zr + 0.09), rings=1, r=0.03, wobble=0.004)
    pc.bar("clock", "board", "o", 0.14, 0.62, 0.62, (0.0, back_o, zr + 0.48), rings=1, r=0.27, segs=4, wobble=0.006, pillow=0.0)
    pc.bar("clock", "dark", "o", 0.03, 0.46, 0.46, (0.0, back_o - 0.075, zr + 0.48), rings=1, r=0.20, segs=4, wobble=0.003, pillow=0.006)
    pc.bar("clock", "team", "z", 0.19, 0.05, 0.035, (0.0, back_o - 0.10, zr + 0.48 + 0.085), rings=1, r=0.012, wobble=0.002)
    pc.bar("clock", "team", "x", 0.14, 0.035, 0.05, (0.06, back_o - 0.10, zr + 0.48), rings=1, r=0.012, wobble=0.002)
    # where a boxed skater stands: on the floor, clear of the seat and of the door's swing, facing the rink (-out)
    pc.sockets["Seat"] = tuple(P["seat_socket"])
    pc.thumbs_done = pc.thumbs()
    return pc, length


# ---------------------------------------------------------------- evolution 2 (lane A119): the goal, the scorer's table

G = dict(
    depth=1.5,              # UJamSim::NetDepth
    post=0.16,              # the front posts and the crossbar: chunky
    low=0.11,               # bottom bars, back posts, back bar, braces
    back_h_factor=0.6, back_h_max=0.9,   # the back frame's height: RinkActor's code frame keeps 0.9 under a 1.5 m bar
    cord=0.03, cell=0.25,   # the net: real cords, about 25 cm cells (12 % of a cell covered each way: nothing is hidden)
    wobble=0.005,           # half the benches' unevenness: the inner opening is the sim's goal volume within 2 cm
    press_r=0.13, press_depth=0.045, press_every=0.45,
    tri_target=2900,
)
GOAL_VARIANTS = [(4.0, 0.61), (6.0, 0.61), (4.0, 1.22)]   # game.rink.goal_width_m x goal_height_m: the default, hockey.mode.pond, hockey.mode.sniper
GOAL_SLOTS = ["frame"]
NET_SLOTS = ["net"]       # the net is its own mesh: RinkActor gives the frame the outline stencil and the cords none (inked cords read as a black comb at 11 m)
TABLE_SLOTS = ["board", "kick", "bench", "dark", "frame"]
TABLE = dict(width=1.36, sign_z=2.20, sign_h=0.56, sign_w=1.30)


def goal_name(w, h):
    return "SM_Arena_Goal_W%03dH%03d" % (round(w * 100), round(h * 100))


def net_name(w, h):
    return "SM_Arena_GoalNet_W%03dH%03d" % (round(w * 100), round(h * 100))


def rot_x(a):
    """Piece space (x, out, z): turn about x so that +out leans towards +z by a."""
    return Matrix(((1, 0, 0, 0), (0, math.cos(a), -math.sin(a), 0), (0, math.sin(a), math.cos(a), 0), (0, 0, 0, 1)))


def goal_build(w, h, cell):
    """One goal for an opening w x h (metres): origin on the goal line at the middle of the mouth, +x along the goal line, out = into the net.
    The posts' inner faces stand at x = +-w/2 and the crossbar's underside at z = h: the sim's goal volume (UJamSim::InGoal)."""
    pc = Piece(goal_name(w, h), seed=73.0 + w * 3.1 + h * 5.3, slots=GOAL_SLOTS)
    PT, LT, D, WB = G["post"], G["low"], G["depth"], G["wobble"]
    hw = w * 0.5
    xp = hw + PT * 0.5                       # post centres
    top = h + PT * 0.5                       # crossbar centre
    bh = min(G["back_h_max"], h * G["back_h_factor"])
    for sx in (-1, 1):
        pc.bar("post", "frame", "z", h + PT, PT, PT, (sx * xp, 0.0, (h + PT) * 0.5), rings=5 if h < 1.0 else 9, r=0.055, segs=2, wobble=WB)
    pc.bar("crossbar", "frame", "x", w + 2 * PT, PT, PT, (0.0, 0.0, top), rings=max(8, int(round(w / 0.2))), r=0.055, segs=2, wobble=WB)
    for sx in (-1, 1):
        pc.bar("bottom_bar", "frame", "o", D + LT, LT, LT, (sx * xp, D * 0.5, LT * 0.5), rings=2, r=0.035, wobble=WB)
        pc.bar("back_post", "frame", "z", bh, LT, LT, (sx * xp, D, bh * 0.5), rings=1, r=0.035, wobble=WB)
        # the brace from the crossbar's end down to the back post's top
        dz = bh - top
        ln = math.hypot(D, dz)
        m = Matrix.Translation(Vector((sx * xp, D * 0.5, (top + bh) * 0.5))) @ rot_x(math.atan2(dz, D))
        pc.bar("brace", "frame", "o", ln + 0.04, LT, LT, (0.0, 0.0, 0.0), rings=2, r=0.035, wobble=WB, xform=m)
    pc.bar("bottom_bar", "frame", "x", w + 2 * PT, LT, LT, (0.0, D, LT * 0.5), rings=4, r=0.035, wobble=WB)
    pc.bar("back_bar", "frame", "x", w + 2 * PT, LT, LT, (0.0, D, bh), rings=4, r=0.035, wobble=WB)
    # thumbs: along the crossbar's top, and on the front of each post (the presses move clay up, down and along the goal's depth: never across the mouth)
    presses = 0
    n = max(2, int(round(w / G["press_every"])))
    for i in range(n):
        q = Vector((i * 1.93 + pc.seed, 2.5, h))
        t = -hw + w * (i + 0.5 + noise.noise(q) * 0.3) / n
        depth = G["press_depth"] * (0.8 + 0.4 * abs(noise.noise(q + Vector((3.3, 0, 0)))))
        pc.press((t, noise.noise(q + Vector((0, 0, 4.4))) * 0.03, h + PT), G["press_r"], depth, (0.0, noise.noise(q + Vector((0, 8.8, 0))) * 0.5, -1.0))
        presses += 1
    for sx in (-1, 1):
        for j in range(1 if h < 1.0 else 2):
            z = h * (j + 0.55) / (1 if h < 1.0 else 2)
            pc.press((sx * xp, -PT * 0.5, z), G["press_r"], G["press_depth"], (0.0, 1.0, 0.15 * sx))
            presses += 1
    pc.thumbs_done = presses
    # the measured opening, before the net (frame verts only): Blender coordinates are (x, -out, z) centimetres
    front = [v.co for v in pc.bm.verts if -PT * 100 < v.co.y < PT * 100]
    inner = [abs(c.x) for c in front if 3.0 < c.z < (h - 0.03) * 100 and abs(c.x) < (hw + PT) * 100]
    under = [c.z for c in front if abs(c.x) < (hw - 0.05) * 100 and c.z > (h - 0.1) * 100]
    pc.opening = {"half_width_m": round(min(inner) / 100.0, 4), "height_m": round(min(under) / 100.0, 4)}
    # ---- the net: cords from bar to bar, in a mesh of their own with the frame's origin
    frame = pc
    pc = Piece(net_name(w, h), seed=frame.seed + 1.0, slots=NET_SLOTS)
    pc.thumbs_done = 0
    CT = G["cord"]
    nx = max(4, int(round(w / cell)))
    xs = [-hw + w * i / nx for i in range(1, nx)]
    f0 = Vector((0.0, PT * 0.2, top))            # where a top cord leaves the crossbar
    b0 = Vector((0.0, D, bh))                    # where it meets the back bar
    slope = (b0 - f0).length
    nt = max(3, int(round(slope / cell)))
    for x in xs:                                 # top, lengthwise; then down the back
        pc.cord("net_top", "net", (x, f0.y, f0.z), (x, b0.y, b0.z), CT, mid=2, sag=(0, 0, -0.025))
        pc.cord("net_back", "net", (x, D, bh), (x, D, LT * 0.5), CT, mid=1 if bh > 0.5 else 0, sag=(0, 0.02, 0))
    for j in range(1, nt):                       # top, across
        p = f0.lerp(b0, j / nt)
        pc.cord("net_top", "net", (-xp, p.y, p.z), (xp, p.y, p.z), CT, mid=max(2, nx // 3), sag=(0, 0, -0.02))
    nb = max(2, int(round(bh / cell)))
    for j in range(1, nb):                       # back, across
        pc.cord("net_back", "net", (-xp, D, bh * j / nb), (xp, D, bh * j / nb), CT, mid=max(2, nx // 3), sag=(0, 0.02, 0))
    no = max(3, int(round(D / cell)))
    for sx in (-1, 1):                           # the two sides
        for k in range(1, no):
            o = D * k / no
            pc.cord("net_side", "net", (sx * xp, o, top + (bh - top) * k / no), (sx * xp, o, LT * 0.5), CT, mid=0)
        z = cell
        while z < top - 0.12:
            o_end = D if z <= bh else D * (top - z) / (top - bh)
            pc.cord("net_side", "net", (sx * xp, PT * 0.2, z), (sx * xp, o_end, z), CT, mid=1, sag=(0.015 * sx, 0, 0))
            z += cell
    return frame, pc


def goal(w, h):
    """The goal (frame and net together) under the triangle target: the net's cell grows a little on a wide goal. Returns the two pieces."""
    cell = G["cell"]
    while True:
        frame, net = goal_build(w, h, cell)
        tris = sum(len(f.verts) - 2 for f in frame.bm.faces) + sum(len(f.verts) - 2 for f in net.bm.faces)
        if tris < G["tri_target"]:
            frame.cell = round(cell, 3)
            net.net_of = frame.name
            return [(frame, w), (net, w)]
        frame.bm.free()
        net.bm.free()
        cell *= 1.12


def scorer_table():
    """The scorer's table between the two penalty boxes (their inner ends stand 1.48 m apart): floor, back wall with an ochre rail, a desk with a horn,
    papers and a mug, a stool, and over the boards a sign with a clock face and a red goal lamp. Same frame as the boxes: origin on the ice in the middle
    of the rink-side edge, the piece reaches away from the rink."""
    pc = Piece("SM_Arena_ScorerTable", seed=59.0, slots=TABLE_SLOTS)
    W, D, WT, WH, FH = TABLE["width"], P["depth"], P["wall_t"], P["wall_h"], P["floor_h"]
    back_o = D - WT * 0.5
    pc.bar("floor", "bench", "x", W, D, FH, (0, D * 0.5, FH * 0.5), rings=2, r=0.03)
    pc.bar("back_wall", "board", "x", W, WT, WH - FH + 0.02, (0, back_o, FH + (WH - FH) * 0.5 - 0.01), rings=2, r=0.04)
    zr = WH + P["rail_h"] * 0.5 - 0.02
    pc.bar("cap_rail", "kick", "x", W + 0.02, P["rail_w"], P["rail_h"], (0, back_o, zr), rings=8, r=0.05, wobble=P["wobble"] * 1.3)
    pc.rails.append(("x", -W * 0.5 + 0.25, W * 0.5 - 0.25, back_o, zr + P["rail_h"] * 0.5))
    # the desk at the board line: a thick ochre top on a board-coloured front and two dark legs
    pc.bar("desk", "kick", "x", W - 0.10, 0.62, 0.10, (0, 0.37, 0.80), rings=4, r=0.04)
    pc.bar("desk", "board", "x", W - 0.16, 0.09, 0.66, (0, 0.11, FH + 0.33), rings=2, r=0.035)
    for sx in (-1, 1):
        pc.bar("desk_legs", "dark", "z", 0.66, 0.11, 0.11, (sx * (W * 0.5 - 0.16), 0.60, FH + 0.33), rings=1, r=0.03)
    # on the desk: the horn (a red body, a dark mouth towards the rink), a stack of papers, a mug
    pc.bar("horn", "frame", "o", 0.30, 0.17, 0.17, (-0.36, 0.40, 0.945), rings=1, r=0.075, segs=2, wobble=0.004)
    pc.bar("horn", "dark", "o", 0.06, 0.25, 0.25, (-0.36, 0.22, 0.945), rings=1, r=0.11, segs=2, wobble=0.004, pillow=0.0)
    pc.bar("papers", "board", "x", 0.34, 0.25, 0.05, (0.20, 0.36, 0.875), rings=1, r=0.012, wobble=0.004)
    pc.bar("mug", "dark", "z", 0.13, 0.10, 0.10, (0.50, 0.52, 0.915), rings=1, r=0.045, segs=2, wobble=0.003)
    # the stool
    pc.bar("stool", "dark", "z", 0.40, 0.13, 0.13, (0.05, 1.02, FH + 0.20), rings=1, r=0.035)
    pc.bar("stool", "kick", "z", 0.10, 0.40, 0.40, (0.05, 1.02, FH + 0.45), rings=1, r=0.17, segs=3)
    # over the boards: two posts, the sign (board-coloured rim, dark face, two ochre digits, a red colon), the goal lamp
    sz, sh, sw = TABLE["sign_z"], TABLE["sign_h"], TABLE["sign_w"]
    for sx in (-1, 1):
        pc.bar("sign_posts", "dark", "z", sz - sh * 0.5 - WH + 0.10, 0.11, 0.11, (sx * 0.46, back_o, (sz - sh * 0.5 + WH) * 0.5), rings=1, r=0.03)
    pc.bar("sign", "board", "x", sw, 0.16, sh, (0, back_o, sz), rings=6, r=0.07, segs=2, wobble=0.008)
    pc.rails.append(("x", -sw * 0.5 + 0.3, sw * 0.5 - 0.3, back_o, sz + sh * 0.5))
    pc.bar("sign_face", "dark", "x", sw - 0.22, 0.03, sh - 0.20, (0, back_o - 0.085, sz), rings=1, r=0.012, wobble=0.003, pillow=0.004)
    for sx in (-1, 1):
        pc.bar("sign_digits", "kick", "z", 0.24, 0.17, 0.04, (sx * 0.30, back_o - 0.105, sz), rings=1, r=0.015, wobble=0.003)
        pc.bar("sign_digits", "frame", "x", 0.06, 0.04, 0.06, (0.0, back_o - 0.105, sz + sx * 0.07), rings=1, r=0.015, wobble=0.002)
    pc.bar("lamp", "dark", "z", 0.07, 0.30, 0.30, (0, back_o, sz + sh * 0.5 + 0.02), rings=1, r=0.13, segs=2, wobble=0.004)
    pc.bar("lamp", "frame", "z", 0.22, 0.23, 0.23, (0, back_o, sz + sh * 0.5 + 0.15), rings=1, r=0.105, segs=3, wobble=0.004, pillow=0.03)
    pc.thumbs_done = pc.thumbs()
    return pc, W


def renders2(goals, table, box, opts, img, raw):
    """Evolution 2's stills. Landed (img): the goal from the game camera at the far end, the own end seen through the net's top, the far end seen through
    the net from behind, the scorer's table between the boxes. Raw (not landed): a close-up, the two other goal sizes, the table from the ice."""
    for o in bpy.context.scene.objects:
        if o.type == "MESH":
            o.hide_render = True
    cam, tgt = setup_render()
    T = P["board_t"]
    ORANGE, BLUE, BLACK = (0.75, 0.30, 0.10, 1), (0.07, 0.22, 0.80, 1), (0.02, 0.02, 0.02, 1)
    LENS = 18.0 / math.tan(math.radians(35.0))     # the game camera: 70 degrees across (CameraFollow.h FovDeg)
    preview_box("ground", (200, 200, 0.02), (0, 0, -0.05), (0.30, 0.22, 0.16, 1))
    ice = preview_box("ice", (60, 60, 0.02), (0, -26.0, -0.011), (0.80, 0.86, 0.92, 1), 0.35)
    # ---- the goal frame: the goal at the origin, the end boards 4 m behind the goal line (FJamRinkConfig::GoalLineX), the rink on the -out side
    stage = [preview_box("endboards", (60, T, P["board_h"]), (0, 4.0 + T * 0.5, P["board_h"] * 0.5), PREVIEW["board"]),
             preview_box("endkick", (60, 0.02, 0.25), (0, 4.0 - 0.01, 0.125), PREVIEW["kick"]),
             preview_box("goalline", (60, 0.08, 0.004), (0, 0, 0.003), (0.75, 0.05, 0.05, 1))]

    def puck(x, o, z=0.0):
        bpy.ops.mesh.primitive_cylinder_add(vertices=20, radius=cm(0.12), depth=cm(0.06), location=(cm(x), -cm(o), cm(z + 0.03)))
        p = bpy.context.active_object
        m = bpy.data.materials.new("puck")
        m.use_nodes = True
        m.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = BLACK
        p.data.materials.append(m)
        return p

    props = figure(0.5, -0.7, 0.0, ORANGE) + figure(-2.6, -5.0, 0.0, BLUE)                 # a goalie in the crease, an attacker
    props += [puck(-1.2, -1.6), puck(-0.9, 0.8), puck(1.5, 1.25), puck(0.2, 0.35)]        # one in front, three inside the net (mid, back corner, behind the goalie)
    def show(pair, on):
        for o in pair:
            o.hide_render = not on

    main = goals[0]
    for g in goals:
        show(g, False)
    show(main, True)
    j = lambda n: os.path.join(img, n)
    r = lambda n: os.path.join(raw, n)
    shoot(cam, tgt, (-1.9, -11.0, 3.5), (-0.6, -1.5, 0.6), LENS, j("arena-goal-game-camera.jpg"))            # 6 m behind a skater 5 m out, 3.5 m up
    shoot(cam, tgt, (0.7, 3.1, 3.5), (0.2, -1.6, 0.3), LENS, j("arena-goal-own-end-through-net.jpg"))        # defending: the camera behind the own net
    shoot(cam, tgt, (1.3, 3.7, 1.75), (0.0, -0.9, 0.25), 30, j("arena-goal-from-behind.jpg"))                # from the end boards through the net
    shoot(cam, tgt, (-3.3, -3.0, 1.5), (-0.4, 0.5, 0.3), 35, r("goal-closeup.jpg"))
    for g in goals[1:]:
        show(main, False)
        show(g, True)
        shoot(cam, tgt, (-1.9, -11.0, 3.5), (-0.6, -1.5, 0.6), LENS, r(g[0].name + "-game-camera.jpg"))
        shoot(cam, tgt, (-4.0, -3.4, 1.8), (-0.4, 0.5, 0.4), 32, r(g[0].name + "-closeup.jpg"))
        show(g, False)
    for o in stage + props:
        o.hide_render = True
    # ---- the table's frame: the table at the origin, the two boxes at x = -3 and +3, the boards and the rink on the -out side
    G0 = P["gap"]
    ice.location = (0, cm(G0 + T + 30.0), cm(-0.011))
    preview_box("boards", (30, T, P["board_h"]), (0, -G0 - T * 0.5, P["board_h"] * 0.5), PREVIEW["board"])
    preview_box("kickplate", (30, 0.02, 0.25), (0, -G0 - T - 0.01, 0.125), PREVIEW["kick"])
    table.hide_render = False
    for sgn in (-1, 1):
        o = box.copy(); bpy.context.scene.collection.objects.link(o)
        o.location = (cm(sgn * P["box_x"]), 0, 0)
        o.hide_render = False
    figure(-0.25, 1.0, P["floor_h"], (0.55, 0.55, 0.50, 1))          # the scorer
    figure(2.4, -1.5, 0.0, BLUE)
    shoot(cam, tgt, (2.6, -5.2, 4.6), (0.0, 0.7, 1.0), 35, j("arena-scorer-table.jpg"))
    shoot(cam, tgt, (-6.0, -12.0, 3.5), (0.0, 0.6, 1.2), LENS, r("table-from-ice-game-camera.jpg"))
    shoot(cam, tgt, (0.3, 4.2, 3.2), (0.0, 0.5, 0.9), 35, r("table-behind.jpg"))


# ---------------------------------------------------------------- export, stats, renders

def export_fbx(objs, path):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.export_scene.fbx(
        filepath=path, use_selection=True, object_types={"MESH", "EMPTY"},
        apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0,
        axis_forward="-Z", axis_up="Y", use_mesh_modifiers=True, mesh_smooth_type="FACE", use_tspace=False,
        bake_anim=False, path_mode="AUTO", embed_textures=False,
    )
    return round(os.path.getsize(path) / 1e6, 3)


def rad(d):
    return d * math.pi / 180.0


def preview_box(name, size, loc, colour, rough=0.8):
    bpy.ops.mesh.primitive_cube_add(size=1.0, location=(cm(loc[0]), -cm(loc[1]), cm(loc[2])))
    o = bpy.context.active_object
    o.name = name
    o.scale = (cm(size[0]), cm(size[1]), cm(size[2]))
    m = bpy.data.materials.new(name)
    m.use_nodes = True
    b = m.node_tree.nodes["Principled BSDF"]
    b.inputs["Base Color"].default_value = colour
    b.inputs["Roughness"].default_value = rough
    o.data.materials.append(m)
    return o


def figure(x, out, z, colour):
    """A 1.6 m stand-in for the toy figure (scale only): body, head."""
    bpy.ops.mesh.primitive_cylinder_add(vertices=16, radius=cm(0.3), depth=cm(1.05), location=(cm(x), -cm(out), cm(z + 0.525)))
    b = bpy.context.active_object
    bpy.ops.mesh.primitive_uv_sphere_add(segments=16, ring_count=8, radius=cm(0.27), location=(cm(x), -cm(out), cm(z + 1.33)))
    h = bpy.context.active_object
    m = bpy.data.materials.new("fig")
    m.use_nodes = True
    m.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = colour
    for o in (b, h):
        o.data.materials.append(m)
        bpy.ops.object.select_all(action="DESELECT")
        o.select_set(True)
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.shade_smooth()
    return [b, h]


def setup_render(w=1600, h=900):
    sc = bpy.context.scene
    sc.render.engine = "BLENDER_EEVEE_NEXT"
    sc.eevee.taa_render_samples = 32
    sc.render.resolution_x = w
    sc.render.resolution_y = h
    sc.render.image_settings.file_format = "JPEG"
    sc.render.image_settings.quality = 88
    sc.render.image_settings.color_mode = "RGB"
    sc.view_settings.view_transform = "Standard"
    wd = bpy.data.worlds.new("world")
    wd.use_nodes = True
    wd.node_tree.nodes["Background"].inputs[0].default_value = (0.20, 0.17, 0.15, 1)
    wd.node_tree.nodes["Background"].inputs[1].default_value = 0.9
    sc.world = wd
    bpy.ops.object.light_add(type="SUN")
    key = bpy.context.active_object
    key.data.energy = 3.2
    key.data.angle = rad(12)
    key.rotation_euler = (rad(48), 0, rad(-150))
    bpy.ops.object.light_add(type="SUN")
    fill = bpy.context.active_object
    fill.data.energy = 0.9
    fill.data.use_shadow = False
    fill.rotation_euler = (rad(62), 0, rad(40))
    bpy.ops.object.camera_add()
    cam = bpy.context.active_object
    cam.data.clip_end = cm(600)
    bpy.ops.object.empty_add()
    tgt = bpy.context.active_object
    c = cam.constraints.new("TRACK_TO")
    c.target = tgt
    c.track_axis = "TRACK_NEGATIVE_Z"
    c.up_axis = "UP_Y"
    sc.camera = cam
    return cam, tgt


def shoot(cam, tgt, eye, look, lens, path, ortho=None):
    cam.location = (cm(eye[0]), -cm(eye[1]), cm(eye[2]))
    tgt.location = (cm(look[0]), -cm(look[1]), cm(look[2]))
    if ortho:
        cam.data.type = "ORTHO"
        cam.data.ortho_scale = cm(ortho)
    else:
        cam.data.type = "PERSP"
        cam.data.lens = lens
    bpy.context.scene.render.filepath = path
    bpy.ops.render.render(write_still=True)


def renders(bench, box, bench_len, opts, img):
    """Six stills: each piece beside a stretch of boards with 1.6 m stand-ins (from the rink and from behind), the rail close up, the plan, the boxes from the ice."""
    L, W = opts["length"], opts["width"]
    cam, tgt = setup_render()
    T, G = P["board_t"], P["gap"]
    ORANGE, BLUE = (0.75, 0.30, 0.10, 1), (0.07, 0.22, 0.80, 1)
    ground = preview_box("ground", (L + 60, W + 60, 0.02), (0, 0, -0.05), (0.30, 0.22, 0.16, 1))
    # ---- piece frame: the piece at its own origin, the boards and the rink on its -out side
    ice = preview_box("ice", (L, W, 0.02), (0, -(G + T + W * 0.5), -0.011), (0.80, 0.86, 0.92, 1), 0.35)
    boards = preview_box("boards", (bench_len + 8, T, P["board_h"]), (0, -G - T * 0.5, P["board_h"] * 0.5), PREVIEW["board"])
    kick = preview_box("kickplate", (bench_len + 8, 0.02, 0.25), (0, -G - T - 0.01, 0.125), PREVIEW["kick"])
    box.hide_render = True
    figs = figure(-1.5, 0.75, P["floor_h"], ORANGE) + figure(1.4, 0.75, P["floor_h"], ORANGE) + figure(2.2, -1.6, 0.0, BLUE)
    shoot(cam, tgt, (bench_len * 0.5 + 1.0, -6.5, 6.5), (0.8, 0.7, 0.6), 28, os.path.join(img, "arena-bench.jpg"))
    shoot(cam, tgt, (-bench_len * 0.5 - 2.2, 5.6, 3.6), (-bench_len * 0.5 + 3.4, 0.4, 0.8), 30, os.path.join(img, "arena-bench-behind.jpg"))
    shoot(cam, tgt, (-bench_len * 0.5 - 0.4, -1.9, 2.9), (-bench_len * 0.5 + 2.4, 1.3, 1.5), 35, os.path.join(img, "arena-rail-closeup.jpg"))
    bench.hide_render = True
    box.hide_render = False
    for f in figs:
        f.hide_render = True
    sx, so, sz = P["seat_socket"]
    figs2 = figure(sx, so, sz, ORANGE) + figure(-3.4, -1.4, 0.0, BLUE)
    shoot(cam, tgt, (4.4, -5.6, 5.2), (0.1, 0.6, 0.8), 32, os.path.join(img, "arena-box.jpg"))
    shoot(cam, tgt, (-4.6, 5.2, 3.4), (0.0, 0.5, 0.9), 32, os.path.join(img, "arena-box-behind.jpg"))
    for f in figs2:
        f.hide_render = True
    # ---- the rink: the four pieces where RinkActor puts them (Unreal +y = Blender -y)
    boards.hide_render = True
    kick.hide_render = True
    ice.location = (0, 0, cm(-0.011))
    for sy in (-1, 1):
        preview_box("b", (L, T, P["board_h"]), (0, sy * (W * 0.5 + T * 0.5), P["board_h"] * 0.5), PREVIEW["board"])
    for sgn in (-1, 1):
        preview_box("b", (T, W, P["board_h"]), (sgn * (L * 0.5 + T * 0.5), 0, P["board_h"] * 0.5), PREVIEW["board"])
    preview_box("red", (0.3, W, 0.004), (0, 0, 0.003), (0.75, 0.05, 0.05, 1))
    for sgn in (-1, 1):
        preview_box("blue", (0.3, W, 0.004), (sgn * L / 6.0, 0, 0.003), (0.05, 0.15, 0.75, 1))
    y0 = W * 0.5 + T + G
    for sgn in (-1, 1):
        o = bench.copy(); bpy.context.scene.collection.objects.link(o)
        o.location = (cm(sgn * P["bench_x_factor"] * L), -cm(y0), 0)       # piece space out = Unreal +y: the +y long side
        o.hide_render = False
        o = box.copy(); bpy.context.scene.collection.objects.link(o)
        o.location = (cm(sgn * P["box_x"]), cm(y0), 0)                     # the -y long side, turned 180 degrees
        o.rotation_euler = (0, 0, math.pi)
        o.hide_render = False
    box.hide_render = True
    # a boxed skater where UJamSim::BoxPos pins it today (on the ice, 0.7 m inside the -y boards), and one on the Seat socket
    figure(-P["box_x"], -(W * 0.5 - 0.7), 0.0, BLUE)
    figure(P["box_x"] - sx, -(y0 + so), sz, ORANGE)
    bpy.context.scene.render.resolution_y = 1600
    shoot(cam, tgt, (0.0, 0.01, 90.0), (0, 0, 0), 30, os.path.join(img, "arena-plan.jpg"), ortho=W + 10)
    bpy.context.scene.render.resolution_y = 900
    # the penalty side seen from the ice at the game camera's height (3.5 m)
    shoot(cam, tgt, (-10.0, -(W * 0.5 - 10.0), 3.5), (0.0, -(W * 0.5 + 0.6), 0.9), 28, os.path.join(img, "arena-boxes-from-ice.jpg"))
    shoot(cam, tgt, (P["bench_x_factor"] * L - 14.0, (W * 0.5 - 11.0), 3.5), (P["bench_x_factor"] * L - 2.0, (W * 0.5 + 0.6), 0.9), 28, os.path.join(img, "arena-bench-from-ice.jpg"))


def main():
    opts = parse_args()
    out = os.path.abspath(opts["out"])
    os.makedirs(out, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = "METRIC"
    sc.unit_settings.scale_length = 0.01
    sc.unit_settings.length_unit = "CENTIMETERS"
    L, W = opts["length"], opts["width"]
    stats = {"rink": {"length_m": L, "width_m": W}, "params": P, "pieces": {}}
    built = []
    made = [team_bench(L), penalty_box(), scorer_table()]
    for (w, h) in GOAL_VARIANTS:
        made += goal(w, h)
    for pc, length in made:
        opening, cell, net_of = getattr(pc, "opening", None), getattr(pc, "cell", None), getattr(pc, "net_of", None)
        slots = list(pc.slots)
        ob, empties, st = pc.finish()
        st["length_m"] = round(length, 3)
        st["thumb_presses"] = pc.thumbs_done
        st["slots"] = slots
        if opening:      # a goal: the opening it was built for, the opening measured on its frame, and the net's cell
            w, h = [v for v in GOAL_VARIANTS if goal_name(*v) == ob.name][0]
            st["goal"] = {"width_m": w, "height_m": h, "depth_m": G["depth"], "measured_half_width_m": opening["half_width_m"], "measured_height_m": opening["height_m"],
                          "opening_error_cm": round(max(abs(opening["half_width_m"] - w * 0.5), abs(opening["height_m"] - h)) * 100.0, 2), "net_cell_m": cell, "cord_m": G["cord"]}
            st["opening_within_2cm"] = st["goal"]["opening_error_cm"] <= 2.0
        if net_of:       # a goal's net: the frame it hangs in; the pair's triangles are counted on the frame's row once both are built
            st["net_of"] = net_of
        st["fbx_mb"] = export_fbx([ob] + empties, os.path.join(out, ob.name + ".fbx"))
        stats["pieces"][ob.name] = st
        built.append(ob)
    for name, st in stats["pieces"].items():
        if "net_of" in st:
            fr = stats["pieces"][st["net_of"]]
            fr["goal"]["net_mesh"] = name
            fr["goal"]["triangles_with_net"] = fr["triangles"] + st["triangles"]
            fr["under_budget"] = fr["goal"]["triangles_with_net"] < TRI_BUDGET
    y0 = W * 0.5 + P["board_t"] + P["gap"]
    stats["placement_m"] = {
        "note": "Unreal metres, rink centre at the origin, ice top at z 0; RinkActor computes the same from game.rink",
        "SM_Arena_TeamBench": [{"x": -P["bench_x_factor"] * L, "y": y0, "yaw": 0, "team": 0}, {"x": P["bench_x_factor"] * L, "y": y0, "yaw": 0, "team": 1}],
        "SM_Arena_PenaltyBox": [{"x": -P["box_x"], "y": -y0, "yaw": 180, "team": 0}, {"x": P["box_x"], "y": -y0, "yaw": 180, "team": 1}],
        "bench_length_m": P["bench_len_factor"] * L,
        "SM_Arena_ScorerTable": [{"x": 0.0, "y": -y0, "yaw": 180}],
        "goals": {"note": "x = +-GoalLineX (L/2 - 4), y = 0; yaw -90 on the +x end, +90 on the -x end; the variant nearest the cartridge's goal, scaled W/Wv along the line and H/Hv up",
                  "goal_line_x": L * 0.5 - 4.0, "variants": [goal_name(w, h) for (w, h) in GOAL_VARIANTS], "nets": [net_name(w, h) for (w, h) in GOAL_VARIANTS]},
    }
    ok = all(p["under_budget"] and p["uv_loops"] > 0 and p.get("opening_within_2cm", True) for p in stats["pieces"].values())
    if opts["render"]:
        img = os.path.abspath(opts["img"] or os.path.join(out, "render"))
        os.makedirs(img, exist_ok=True)
        t = time.time()
        renders(built[0], built[1], stats["pieces"]["SM_Arena_TeamBench"]["length_m"], opts, img)
        stats["render_s"] = round(time.time() - t, 1)
    if opts["render2"]:
        img = os.path.abspath(opts["img"] or os.path.join(out, "render"))
        raw = os.path.join(out, "render2")
        os.makedirs(img, exist_ok=True)
        os.makedirs(raw, exist_ok=True)
        t = time.time()
        by_name = {o.name: o for o in built}
        renders2([(by_name[goal_name(w, h)], by_name[net_name(w, h)]) for (w, h) in GOAL_VARIANTS], built[2], built[1], opts, img, raw)
        stats["render2_s"] = round(time.time() - t, 1)
    stats["total_s"] = round(time.time() - T0, 1)
    with open(os.path.join(out, "arena.json"), "w") as fh:
        json.dump(stats, fh, indent=2)
    print(("A103 arena.py PASS " if ok else "A103 arena.py FAIL ") + json.dumps({k: {"triangles": v["triangles"], "dims": v["dimensions_m"], **({"goal": v["goal"]} if "goal" in v else {})} for k, v in stats["pieces"].items()}))
    if not ok:
        sys.exit(1)


main()
