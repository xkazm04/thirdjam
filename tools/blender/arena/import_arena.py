"""tools/blender/arena/import_arena.py: headless import of the arena pieces into /Game/Arena (Third Jam, track F3, lanes A103 and A119).

Run (Git Bash, from the worktree root; no editor window, no MCP port):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -run=pythonscript \
      -script="$PWD/tools/blender/arena/import_arena.py" -unattended -nopause -nosplash -NullRHI -Multiprocess -abslog="$PWD/Game/logs/a119-import.log"

What it does, for every piece arena.py built (the bench, the box, the scorer's table, the goal variants; FBX in tools/blender/arena/out).
A piece whose asset already equals Blender's build (triangles and size) is checked and KEPT, not imported again (ARENA_FORCE=1 imports everything):
  1. legacy FbxFactory static-mesh import: no materials, no textures, no collision, no lightmap UVs, normals from the file;
  2. binds device B's instances to the slots BY REFERENCE (nothing is created, nothing under /JamAssets is written);
  3. checks: triangle count under 3,000 and equal to Blender's, the slots Blender wrote, UV channel 0 read back with Blender's extents, bounds, the Seat
     socket on the box, and on a goal the inner opening measured on the imported vertices (within 2 cm of the opening it was built for);
  4. saves what it imported and writes tools/blender/arena/out/import.json. Prints "A119 IMPORT PASS|FAIL".
"""
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "out")
DEST = "/Game/Arena"
FORCE = os.environ.get("ARENA_FORCE", "") == "1"
# slot name (the FBX material name) -> device B's instance. RinkActor re-binds `team` per side (MI_Toon_Blue / MI_Toon_Orange) at Build.
SLOT_LOOK = {
    "board": "/JamAssets/Look/MI_Toon_Board",
    "kick": "/JamAssets/Look/MI_Toon_KickPlate",
    "bench": "/JamAssets/Look/MI_Toon_Bench",
    "team": "/JamAssets/Look/MI_Toon_Blue",
    "dark": "/JamAssets/Look/MI_Toon_Dark",
    "frame": "/JamAssets/Look/MI_Toon_NetFrame",   # lane A119: the goal's frame, the table's lamp and horn
    "net": "/JamAssets/Look/MI_Toon_NetMesh",      # lane A119: the goal's cords
    "lamp": "/JamAssets/Look/MI_Toon_NetFrame",    # lane A127: the table's lamp and the goal lights' (RinkActor puts a dynamic instance of it on the slot and drives its Emissive)
}
TRI_BUDGET = 3000


def log(msg):
    unreal.log("A119 IMPORT " + msg)


def import_piece(name):
    task = unreal.AssetImportTask()
    task.filename = os.path.join(SRC, name + ".fbx").replace("\\", "/")
    task.destination_path = DEST
    task.destination_name = name
    task.automated = True
    task.replace_existing = True
    task.save = False
    task.factory = unreal.FbxFactory()
    ui = unreal.FbxImportUI()
    ui.import_mesh = True
    ui.import_as_skeletal = False
    ui.import_animations = False
    ui.import_materials = False
    ui.import_textures = False
    ui.automated_import_should_detect_type = False
    ui.mesh_type_to_import = unreal.FBXImportType.FBXIT_STATIC_MESH
    ui.original_import_type = unreal.FBXImportType.FBXIT_STATIC_MESH
    sm = ui.static_mesh_import_data
    sm.combine_meshes = True
    sm.generate_lightmap_u_vs = False
    sm.auto_generate_collision = False
    sm.remove_degenerates = True
    sm.build_nanite = False
    sm.normal_import_method = unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS
    sm.import_uniform_scale = 1.0
    sm.convert_scene = True
    sm.convert_scene_unit = True
    sm.force_front_x_axis = False
    task.options = ui
    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    return unreal.load_asset(DEST + "/" + name)


def uv_bounds(mesh):
    """UV channel 0 read back from the imported mesh description: [umin, vmin, umax, vmax] and the count of vertex instances read.
    (StaticMeshEditorSubsystem has no instance under -run=pythonscript, so the count of channels is not asked; the UVs themselves are.)"""
    desc = mesh.get_static_mesh_description(0)
    n = desc.get_vertex_instance_count()
    lo = [1e9, 1e9]
    hi = [-1e9, -1e9]
    vid = unreal.VertexInstanceID()
    for i in range(n):
        vid.set_editor_property("id_value", i)
        uv = desc.get_vertex_instance_uv(vid, 0)
        lo = [min(lo[0], uv.x), min(lo[1], uv.y)]
        hi = [max(hi[0], uv.x), max(hi[1], uv.y)]
    return [round(lo[0], 3), round(lo[1], 3), round(hi[0], 3), round(hi[1], 3)], n


def goal_opening(mesh, goal):
    """The inner opening measured on the imported vertices (Unreal centimetres: x along the goal line, y into the net, z up): the least |x| on the posts
    below the crossbar and the least z on the crossbar between the posts. The same rule as arena.py's own measure."""
    desc = mesh.get_static_mesh_description(0)
    hw, h, post = goal["width_m"] * 50.0, goal["height_m"] * 100.0, 16.0
    inner, under = 1e9, 1e9
    vid = unreal.VertexID()
    for i in range(desc.get_vertex_count()):
        vid.set_editor_property("id_value", i)
        p = desc.get_vertex_position(vid)
        if -post < p.y < post:
            if 3.0 < p.z < h - 3.0 and abs(p.x) < hw + post:
                inner = min(inner, abs(p.x))
            if abs(p.x) < hw - 5.0 and p.z > h - 10.0:
                under = min(under, p.z)
    return round(inner / 100.0, 4), round(under / 100.0, 4)


def unchanged(name, want):
    """The asset already in /Game/Arena when it equals Blender's build (triangles and size within 2 cm), else None."""
    path = DEST + "/" + name
    if FORCE or not unreal.EditorAssetLibrary.does_asset_exist(path):
        return None
    mesh = unreal.load_asset(path)
    if not mesh or not isinstance(mesh, unreal.StaticMesh) or mesh.get_num_triangles(0) != want["triangles"]:
        return None
    # lane A127: the same triangles on other slots is another piece (the table's lamp moved from `frame` to a slot of its own)
    if sorted(str(m.material_slot_name) for m in mesh.static_materials) != sorted(want.get("slots", ["board", "kick", "bench", "team", "dark"])):
        return None
    box = mesh.get_bounding_box()
    size = [(box.max.x - box.min.x) / 100.0, (box.max.y - box.min.y) / 100.0, (box.max.z - box.min.z) / 100.0]
    return mesh if all(abs(a - b) < 0.02 for a, b in zip(size, want["dimensions_m"])) else None


def main():
    with open(os.path.join(SRC, "arena.json")) as fh:
        blender = json.load(fh)["pieces"]
    report = {"pieces": {}, "slot_look": SLOT_LOOK}
    ok = True
    for name in blender.keys():
        kept = unchanged(name, blender[name])
        mesh = kept or import_piece(name)
        if not mesh or not isinstance(mesh, unreal.StaticMesh):
            log("FAIL %s: no static mesh after the import" % name)
            ok = False
            continue
        mats = list(mesh.static_materials)
        slots = []
        for i, sm_mat in enumerate(mats):
            slot = str(sm_mat.material_slot_name)
            slots.append(slot)
            look = SLOT_LOOK.get(slot)
            mi = unreal.load_asset(look) if look else None
            if mi:
                if not kept:
                    mesh.set_material(i, mi)
            else:
                log("WARN %s slot %d '%s': no look instance bound (%s)" % (name, i, slot, look))
                ok = False
        tris = mesh.get_num_triangles(0)
        box = mesh.get_bounding_box()
        size = [round((box.max.x - box.min.x) / 100.0, 3), round((box.max.y - box.min.y) / 100.0, 3), round((box.max.z - box.min.z) / 100.0, 3)]
        socket = mesh.find_socket("Seat")
        uvb, uvn = uv_bounds(mesh)
        bu0, bu1 = blender[name]["uv_min"], blender[name]["uv_max"]
        row = {
            "path": mesh.get_path_name(),
            "imported": not kept,
            "triangles": tris,
            "triangles_blender": blender[name]["triangles"],
            "lods": mesh.get_num_lods(),
            "sections": mesh.get_num_sections(0),
            "uv0_bounds": uvb,
            "uv0_bounds_blender": [bu0[0], bu0[1], bu1[0], bu1[1]],
            "vertex_instances": uvn,
            "slots": slots,
            "materials": [m.get_path_name() if m else None for m in [mesh.get_material(i) for i in range(len(mats))]],
            "bounds_min_cm": [round(box.min.x, 1), round(box.min.y, 1), round(box.min.z, 1)],
            "bounds_max_cm": [round(box.max.x, 1), round(box.max.y, 1), round(box.max.z, 1)],
            "size_m": size,
            "size_m_blender": blender[name]["dimensions_m"],
            "seat_socket_cm": [round(socket.relative_location.x, 1), round(socket.relative_location.y, 1), round(socket.relative_location.z, 1)] if socket else None,
            "nanite": bool(mesh.get_editor_property("nanite_settings").enabled),
        }
        checks = {
            "under_budget": tris < blender[name].get("budget", TRI_BUDGET),
            "triangles_match_blender": tris == blender[name]["triangles"],
            "slots_match_blender": sorted(slots) == sorted(blender[name].get("slots", ["board", "kick", "bench", "team", "dark"])),
            "look_instances_bound": all(m is not None and m.startswith("/JamAssets/Look/MI_Toon_") for m in row["materials"]),
            # Unreal flips V on import (v -> 1 - v): the extents must match Blender's, the U range exactly
            "has_uvs": uvn > 0 and abs(uvb[0] - bu0[0]) < 0.01 and abs(uvb[2] - bu1[0]) < 0.01 and abs((uvb[3] - uvb[1]) - (bu1[1] - bu0[1])) < 0.01 and (uvb[2] - uvb[0]) > 0.1,
            "extends_to_plus_y": box.max.y > (30.0 if name == "SM_Arena_GoalLight" else 100.0) and box.min.y > -30.0,   # the goal light is a post, half a metre deep
            "stands_on_the_ice": abs(box.min.z) < (8.0 if "net_of" in blender[name] else 5.0),   # a net hangs from the bars: its lowest cord ends in the bottom bar
            "size_matches_blender": all(abs(a - b) < 0.02 for a, b in zip(size, blender[name]["dimensions_m"])),
        }
        if name == "SM_Arena_PenaltyBox":
            checks["seat_socket"] = socket is not None
        if "goal" in blender[name]:
            g = blender[name]["goal"]
            half, height = goal_opening(mesh, g)
            row["goal"] = {"built_for_m": [g["width_m"], g["height_m"]], "opening_half_width_m": half, "opening_height_m": height,
                           "opening_error_cm": round(max(abs(half - g["width_m"] * 0.5), abs(height - g["height_m"])) * 100.0, 2)}
            checks["opening_within_2cm"] = row["goal"]["opening_error_cm"] <= 2.0
            row["goal"]["net_mesh"] = g.get("net_mesh")
            row["goal"]["triangles_with_net"] = tris + blender[g["net_mesh"]]["triangles"]
            checks["goal_with_net_under_budget"] = row["goal"]["triangles_with_net"] < TRI_BUDGET
        row["checks"] = checks
        if not all(checks.values()):
            ok = False
        report["pieces"][name] = row
        if not kept:
            unreal.EditorAssetLibrary.save_asset(DEST + "/" + name, only_if_is_dirty=False)
        log("%s %s" % (name, json.dumps(row)))
    report["verdict"] = "pass" if ok else "fail"
    with open(os.path.join(SRC, "import.json"), "w") as fh:
        json.dump(report, fh, indent=2)
    log("PASS" if ok else "FAIL")


main()
