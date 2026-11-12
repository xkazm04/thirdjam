"""tools/blender/props.py: the props the arena needs without film props (Third Jam, team Hokej, lane A22), Blender 4.2.

Run headless:
  blender -b -P tools/blender/props.py -- --out tools/blender/out [--render]

Builds, each as one low-poly mesh with ONE material slot (M_PropsAtlas) on one shared 1K RGBA texture T_Props_Atlas.png:
  SM_Net            tubular frame (posts, crossbar, back and bottom rails) plus netting cards (alpha from the atlas's tiling net pattern)
  SM_Board_Segment  2 m boards segment: kick plate, board, cap rail, glass panel (alpha 0.3 band), two stanchions
  SM_Bench          players' bench: seat plank, back board, two legs
  SM_Puck           disc, chamfered (bevel), game size r 0.12 m x h 0.08 m
  SM_GoalLight      goal lamp housing: cylinder body, red lens dome, wall bracket
  SM_Pedestal_Round pickup pedestal, round, with the socket SOCKET_PickupTop (-> UE socket PickupTop)
  SM_Pedestal_Hex   pickup pedestal, hexagonal, same socket

Atlas layout (1024 x 1024): v 0.5..1 = a 32 x 32 cell net pattern (opaque thread on alpha 0); v 0..0.5 = 16 flat colour bands of 32 px
(see BANDS). Faces of a flat colour map to the centre of their band; netting cards map u across the pattern (u wraps seamlessly), v inside the
pattern region (one cell = 5 cm of netting). Units: scene unit scale 0.01, so the FBX is in centimetres; parameters below are metres.
"""
import bpy
import json
import math
import os
import sys
import time

import numpy as np
from mathutils import Vector

T0 = time.time()

# ---------------------------------------------------------------- parameters (metres)

NET = dict(width=4.0, height=1.5, depth=1.5, tube_r=0.05, back_height=1.1, cell=0.05)
BOARDS = dict(length=2.0, height=1.2, kick=0.25, glass=1.0, thickness=0.08, cap=0.06, stanchion=0.06)
BENCH = dict(length=2.4, depth=0.4, seat_h=0.45, back_h=0.9, plank=0.06)
PUCK = dict(r=0.12, h=0.08, chamfer=0.012)
LAMP = dict(r=0.12, h=0.22, lens_r=0.11, bracket=0.25)
PEDESTAL = dict(r=0.45, h=0.12, top_z=0.45)

BANDS = [
    # name, rgba (bands run upward from v = 0, 32 px each)
    ("frame_red", (0.75, 0.08, 0.06, 1.0)),
    ("board_white", (0.92, 0.92, 0.90, 1.0)),
    ("kick_yellow", (0.95, 0.75, 0.10, 1.0)),
    ("glass", (0.70, 0.85, 0.95, 0.30)),
    ("dark", (0.10, 0.10, 0.11, 1.0)),
    ("black", (0.02, 0.02, 0.02, 1.0)),
    ("wood", (0.60, 0.42, 0.22, 1.0)),
    ("metal", (0.55, 0.57, 0.60, 1.0)),
    ("lens_red", (0.95, 0.12, 0.10, 1.0)),
    ("team_blue", (0.05, 0.25, 0.85, 1.0)),
    ("team_orange", (0.95, 0.45, 0.08, 1.0)),
    ("ice_white", (0.86, 0.90, 0.95, 1.0)),
]
TEX_PX = 1024
PATTERN_CELLS = 32


def parse_args():
    argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []
    opts = {"out": "tools/blender/out", "render": False}
    i = 0
    while i < len(argv):
        if argv[i] == "--out":
            opts["out"] = argv[i + 1]; i += 1
        elif argv[i] == "--render":
            opts["render"] = True
        i += 1
    return opts


def cm(x):
    return x * 100.0


def V(x, y, z):
    return Vector((cm(x), cm(y), cm(z)))


def rad(d):
    return math.radians(d)


# ---------------------------------------------------------------- atlas

def band_uv(name):
    k = [b[0] for b in BANDS].index(name)
    v0 = k * 32.0 / TEX_PX
    v1 = (k + 1) * 32.0 / TEX_PX
    return (0.5, (v0 + v1) * 0.5)


def write_atlas(path):
    px = np.zeros((TEX_PX, TEX_PX, 4), dtype=np.float32)
    # bands (row 0 is the bottom in Blender's pixel order)
    for k, (name, rgba) in enumerate(BANDS):
        px[k * 32:(k + 1) * 32, :, :] = rgba
    # net pattern in the top half: 32 x 32 cells of 16 px, 2 px threads, alpha 0 elsewhere
    half = TEX_PX // 2
    cell = half // PATTERN_CELLS
    thread = (0.80, 0.80, 0.78, 1.0)
    region = np.zeros((half, TEX_PX, 4), dtype=np.float32)
    for i in range(0, TEX_PX, cell):
        region[:, i:i + 2, :] = thread
    for j in range(0, half, cell):
        region[j:j + 2, :, :] = thread
    px[half:, :, :] = region
    img = bpy.data.images.new("T_Props_Atlas", TEX_PX, TEX_PX, alpha=True)
    img.pixels.foreach_set(px.ravel())
    img.filepath_raw = path
    img.file_format = "PNG"
    img.save()
    return img


def make_material(img):
    m = bpy.data.materials.new("M_PropsAtlas")
    m.use_nodes = True
    nt = m.node_tree
    bsdf = nt.nodes["Principled BSDF"]
    tex = nt.nodes.new("ShaderNodeTexImage")
    tex.image = img
    tex.interpolation = "Closest"
    nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
    nt.links.new(tex.outputs["Alpha"], bsdf.inputs["Alpha"])
    bsdf.inputs["Roughness"].default_value = 0.55
    m.blend_method = "HASHED"
    try:
        m.surface_render_method = "DITHERED"
    except Exception:
        pass
    m.use_backface_culling = False
    return m


# ---------------------------------------------------------------- builders

class Props:
    def __init__(self, mat):
        self.mat = mat
        self.stats = {}

    def _finish(self, obj, name, band, parts, smooth=False):
        obj.name = name
        obj.data.name = name
        obj.data.materials.append(self.mat)
        uv = obj.data.uv_layers.new(name="UVMap") if not obj.data.uv_layers else obj.data.uv_layers[0]
        u, v = band_uv(band)
        for loop in obj.data.loops:
            uv.data[loop.index].uv = (u, v)
        if smooth:
            for poly in obj.data.polygons:
                poly.use_smooth = True
        parts.append(obj)
        return obj

    def box(self, name, band, parts, size, loc, rot_z_deg=0.0):
        bpy.ops.mesh.primitive_cube_add(size=1.0, location=V(*loc))
        o = bpy.context.active_object
        o.scale = (cm(size[0]), cm(size[1]), cm(size[2]))
        o.rotation_euler = (0, 0, rad(rot_z_deg))
        bpy.ops.object.transform_apply(scale=True, rotation=True)
        return self._finish(o, name, band, parts)

    def tube(self, name, band, parts, a, b, r, verts=8):
        a, b = Vector(a), Vector(b)
        d = b - a
        rot = d.to_track_quat("Z", "Y").to_euler()
        bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=cm(r), depth=d.length * 100.0, location=(a + b) * 50.0, rotation=rot)
        return self._finish(bpy.context.active_object, name, band, parts, smooth=True)

    def cylinder(self, name, band, parts, r, z0, z1, x=0.0, y=0.0, verts=24, smooth=True):
        bpy.ops.mesh.primitive_cylinder_add(vertices=verts, radius=cm(r), depth=cm(z1 - z0), location=V(x, y, (z0 + z1) * 0.5))
        return self._finish(bpy.context.active_object, name, band, parts, smooth=smooth)

    def dome(self, name, band, parts, r, x, y, z, scale_z=0.5, seg=16, rings=8):
        bpy.ops.mesh.primitive_uv_sphere_add(segments=seg, ring_count=rings, radius=cm(r), location=V(x, y, z))
        o = bpy.context.active_object
        o.scale = (1, 1, scale_z)
        bpy.ops.object.transform_apply(scale=True)
        return self._finish(o, name, band, parts, smooth=True)

    def net_card(self, name, parts, corners, w_m, h_m):
        """a quad with netting UVs: corners = 4 points (metres) in order; u tiles across the pattern, v inside it"""
        me = bpy.data.meshes.new(name)
        verts = [tuple(V(*c)) for c in corners]
        me.from_pydata(verts, [], [(0, 1, 2, 3)])
        me.update()
        o = bpy.data.objects.new(name, me)
        bpy.context.collection.objects.link(o)
        bpy.context.view_layer.objects.active = o
        o.data.materials.append(self.mat)
        uv = o.data.uv_layers.new(name="UVMap")
        span = PATTERN_CELLS * NET["cell"]  # metres covered by one pattern repeat
        us = [0.0, w_m / span, w_m / span, 0.0]
        vs = [0.5, 0.5, 0.5 + 0.5 * min(0.98, h_m / span), 0.5 + 0.5 * min(0.98, h_m / span)]
        for loop in o.data.loops:
            uv.data[loop.index].uv = (us[loop.vertex_index], vs[loop.vertex_index])
        parts.append(o)
        return o

    def join(self, parts, name):
        bpy.ops.object.select_all(action="DESELECT")
        for o in parts:
            o.select_set(True)
        bpy.context.view_layer.objects.active = parts[0]
        bpy.ops.object.join()
        o = bpy.context.active_object
        o.name = name
        o.data.name = name
        o.data.calc_loop_triangles()
        self.stats[name] = {"triangles": len(o.data.loop_triangles)}
        return o

    # ---- the props (each faces +X = the open mouth of the net toward +X; the net sits behind the goal line at x = 0)
    def net(self):
        n = NET
        parts = []
        w, h, d, r = n["width"], n["height"], n["depth"], n["tube_r"]
        hw = w * 0.5
        bh = n["back_height"]
        pts = {
            "post_l": ((0, hw, 0), (0, hw, h)), "post_r": ((0, -hw, 0), (0, -hw, h)),
            "crossbar": ((0, hw, h), (0, -hw, h)),
            "top_l": ((0, hw, h), (-d * 0.55, hw, bh)), "top_r": ((0, -hw, h), (-d * 0.55, -hw, bh)),
            "back_top": ((-d * 0.55, hw, bh), (-d * 0.55, -hw, bh)),
            "back_l": ((-d * 0.55, hw, bh), (-d, hw, 0.0)), "back_r": ((-d * 0.55, -hw, bh), (-d, -hw, 0.0)),
            "base_l": ((0, hw, r), (-d, hw, r)), "base_r": ((0, -hw, r), (-d, -hw, r)),
            "base_back": ((-d, hw, r), (-d, -hw, r)),
        }
        for k, (a, b) in pts.items():
            self.tube("net_" + k, "frame_red", parts, a, b, r)
        # netting cards: back (two slopes), sides, top
        self.net_card("mesh_back_upper", parts, [(-d * 0.55, hw, bh), (-d * 0.55, -hw, bh), (0, -hw, h), (0, hw, h)], w, math.hypot(d * 0.55, h - bh))
        self.net_card("mesh_back_lower", parts, [(-d, hw, 0), (-d, -hw, 0), (-d * 0.55, -hw, bh), (-d * 0.55, hw, bh)], w, math.hypot(d * 0.45, bh))
        self.net_card("mesh_side_l", parts, [(0, hw, 0), (-d, hw, 0), (-d * 0.55, hw, bh), (0, hw, h)], d, h)
        self.net_card("mesh_side_r", parts, [(0, -hw, 0), (0, -hw, h), (-d * 0.55, -hw, bh), (-d, -hw, 0)], d, h)
        o = self.join(parts, "SM_Net")
        return o

    def boards(self):
        b = BOARDS
        parts = []
        L, H, K, G, t = b["length"], b["height"], b["kick"], b["glass"], b["thickness"]
        # the segment runs along Y, the ice side faces +X
        self.box("kick", "kick_yellow", parts, (t, L, K), (0, 0, K * 0.5))
        self.box("board", "board_white", parts, (t, L, H - K), (0, 0, K + (H - K) * 0.5))
        self.box("cap", "dark", parts, (t + 0.04, L, b["cap"]), (0, 0, H + b["cap"] * 0.5))
        self.box("glass", "glass", parts, (0.012, L - b["stanchion"], G), (0, 0, H + b["cap"] + G * 0.5))
        for s in (1, -1):
            self.box("stanchion_%d" % s, "metal", parts, (b["stanchion"], b["stanchion"], G + b["cap"]), (0, s * (L * 0.5 - b["stanchion"] * 0.5), H + (G + b["cap"]) * 0.5))
        return self.join(parts, "SM_Board_Segment")

    def bench(self):
        b = BENCH
        parts = []
        self.box("seat", "wood", parts, (b["depth"], b["length"], b["plank"]), (0, 0, b["seat_h"]))
        self.box("back", "wood", parts, (b["plank"], b["length"], 0.3), (-b["depth"] * 0.5, 0, b["back_h"] - 0.15))
        for s in (1, -1):
            self.box("leg_%d" % s, "metal", parts, (b["depth"] - 0.05, 0.06, b["seat_h"]), (0, s * (b["length"] * 0.5 - 0.15), b["seat_h"] * 0.5))
            self.box("post_%d" % s, "metal", parts, (0.06, 0.06, b["back_h"]), (-b["depth"] * 0.5 + 0.03, s * (b["length"] * 0.5 - 0.15), b["back_h"] * 0.5))
        return self.join(parts, "SM_Bench")

    def puck(self):
        p = PUCK
        parts = []
        o = self.cylinder("puck", "black", parts, p["r"], 0.0, p["h"], verts=32)
        bev = o.modifiers.new("chamfer", "BEVEL")
        bev.width = cm(p["chamfer"])
        bev.segments = 2
        bev.limit_method = "ANGLE"
        bpy.ops.object.modifier_apply(modifier=bev.name)
        for poly in o.data.polygons:
            poly.use_smooth = True
        return self.join(parts, "SM_Puck")

    def goal_light(self):
        l = LAMP
        parts = []
        self.box("bracket", "metal", parts, (l["bracket"], 0.08, 0.04), (-l["bracket"] * 0.5, 0, 0.0))
        self.cylinder("body", "dark", parts, l["r"], 0.0, l["h"], verts=16)
        self.dome("lens", "lens_red", parts, l["lens_r"], 0, 0, l["h"], scale_z=0.8)
        self.cylinder("rim", "metal", parts, l["r"] * 1.05, l["h"] - 0.03, l["h"], verts=16)
        return self.join(parts, "SM_GoalLight")

    def pedestal(self, kind):
        p = PEDESTAL
        parts = []
        if kind == "round":
            o = self.cylinder("base", "metal", parts, p["r"], 0.0, p["h"], verts=24)
            self.cylinder("inlay", "team_blue", parts, p["r"] * 0.8, p["h"], p["h"] + 0.015, verts=24)
            self.cylinder("step", "dark", parts, p["r"] * 1.1, 0.0, p["h"] * 0.35, verts=24)
            name = "SM_Pedestal_Round"
        else:
            o = self.cylinder("base", "dark", parts, p["r"], 0.0, p["h"], verts=6, smooth=False)
            self.cylinder("inlay", "team_orange", parts, p["r"] * 0.8, p["h"], p["h"] + 0.015, verts=6, smooth=False)
            self.cylinder("step", "metal", parts, p["r"] * 1.1, 0.0, p["h"] * 0.35, verts=6, smooth=False)
            name = "SM_Pedestal_Hex"
        mesh = self.join(parts, name)
        # UE static mesh socket from an empty named SOCKET_<name>, parented to the mesh
        bpy.ops.object.empty_add(location=V(0, 0, p["top_z"]))
        e = bpy.context.active_object
        e.name = "SOCKET_PickupTop"
        e.empty_display_size = 5
        e.parent = mesh
        self.sockets = getattr(self, "sockets", {})
        self.sockets[name] = e
        return mesh


def export_fbx(objs, path):
    bpy.ops.object.select_all(action="DESELECT")
    for o in objs:
        o.select_set(True)
    bpy.context.view_layer.objects.active = objs[0]
    bpy.ops.export_scene.fbx(
        filepath=path, use_selection=True, object_types={"MESH", "EMPTY"},
        apply_unit_scale=True, apply_scale_options="FBX_SCALE_NONE", global_scale=1.0,
        axis_forward="-Z", axis_up="Y", use_mesh_modifiers=True, mesh_smooth_type="EDGE", use_tspace=False,
        bake_anim=False, path_mode="AUTO", embed_textures=False,
    )
    return round(os.path.getsize(path) / 1e6, 2)


def render_overview(objs, path):
    sc = bpy.context.scene
    sc.render.engine = "BLENDER_EEVEE_NEXT"
    sc.eevee.taa_render_samples = 24
    sc.render.resolution_x = 1536
    sc.render.resolution_y = 768
    sc.render.image_settings.file_format = "PNG"
    sc.render.image_settings.color_mode = "RGB"
    sc.view_settings.view_transform = "Standard"
    # spread the props along Y on an ice plane
    x = 0.0
    y = -5.2
    for o in objs:
        o.location = V(0, y, 0)
        dims = o.dimensions
        y += dims.y / 100.0 * 0.5 + 0.8
        if o.name.startswith("SM_Puck"):
            o.location = V(1.2, o.location.y / 100.0, 0)
    bpy.ops.mesh.primitive_plane_add(size=cm(60), location=(0, 0, -0.1))
    ice = bpy.context.active_object
    mi = bpy.data.materials.new("ice")
    mi.use_nodes = True
    mi.node_tree.nodes["Principled BSDF"].inputs["Base Color"].default_value = (0.86, 0.90, 0.95, 1)
    ice.data.materials.append(mi)
    w = bpy.data.worlds.new("world")
    w.use_nodes = True
    w.node_tree.nodes["Background"].inputs[0].default_value = (0.16, 0.17, 0.20, 1)
    sc.world = w
    bpy.ops.object.light_add(type="SUN")
    key = bpy.context.active_object
    key.data.energy = 3.0
    key.data.angle = rad(8)
    key.rotation_euler = (rad(50), 0, rad(-40))
    bpy.ops.object.light_add(type="SUN")
    fill = bpy.context.active_object
    fill.data.energy = 0.8
    fill.data.use_shadow = False
    fill.rotation_euler = (rad(60), 0, rad(140))
    bpy.ops.object.camera_add(location=V(9.5, -2.5, 4.5))
    cam = bpy.context.active_object
    cam.data.lens = 26
    cam.data.clip_end = cm(300)
    bpy.ops.object.empty_add(location=V(0, -1.2, 0.6))
    tgt = bpy.context.active_object
    c = cam.constraints.new("TRACK_TO")
    c.target = tgt
    c.track_axis = "TRACK_NEGATIVE_Z"
    c.up_axis = "UP_Y"
    sc.camera = cam
    sc.render.filepath = path
    bpy.ops.render.render(write_still=True)


def main():
    opts = parse_args()
    out = os.path.abspath(opts["out"])
    os.makedirs(out, exist_ok=True)
    bpy.ops.wm.read_factory_settings(use_empty=True)
    sc = bpy.context.scene
    sc.unit_settings.system = "METRIC"
    sc.unit_settings.scale_length = 0.01
    sc.unit_settings.length_unit = "CENTIMETERS"
    img = write_atlas(os.path.join(out, "T_Props_Atlas.png"))
    mat = make_material(img)
    P = Props(mat)
    t = time.time()
    objs = [P.net(), P.boards(), P.bench(), P.puck(), P.goal_light(), P.pedestal("round"), P.pedestal("hex")]
    P.stats["build_s"] = round(time.time() - t, 1)
    for o in objs:
        extra = [P.sockets[o.name]] if o.name in getattr(P, "sockets", {}) else []
        mb = export_fbx([o] + extra, os.path.join(out, o.name + ".fbx"))
        P.stats[o.name]["fbx_mb"] = mb
        P.stats[o.name]["dimensions_m"] = [round(d / 100.0, 3) for d in o.dimensions]
    P.stats["atlas"] = "T_Props_Atlas.png 1024x1024 RGBA"
    if opts["render"]:
        t = time.time()
        render_overview(objs, os.path.join(out, "render", "props_overview.png"))
        P.stats["render_s"] = round(time.time() - t, 1)
    P.stats["total_s"] = round(time.time() - T0, 1)
    with open(os.path.join(out, "props.json"), "w") as fh:
        json.dump(P.stats, fh, indent=2)
    print("A22 props.py PASS", json.dumps(P.stats))


main()
