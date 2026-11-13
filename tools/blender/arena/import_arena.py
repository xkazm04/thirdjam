"""tools/blender/arena/import_arena.py: headless import of the arena pieces into /Game/Arena (Third Jam, track F3, lane A103).

Run (Git Bash, from the worktree root; no editor window, no MCP port):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -run=pythonscript \
      -script="$PWD/tools/blender/arena/import_arena.py" -unattended -nopause -nosplash -NullRHI -abslog="$PWD/Game/logs/a103-import.log"

What it does, for SM_Arena_TeamBench and SM_Arena_PenaltyBox (FBX from tools/blender/arena/arena.py in tools/blender/arena/out):
  1. legacy FbxFactory static-mesh import: no materials, no textures, no collision, no lightmap UVs, normals from the file;
  2. binds device B's instances to the five slots BY REFERENCE (nothing is created, nothing under /JamAssets is written);
  3. checks: triangle count under 3,000 and equal to Blender's, five slots, UV channel 0 read back with Blender's extents, bounds, the Seat socket on the box;
  4. saves the two assets and writes tools/blender/arena/out/import.json. Prints "A103 IMPORT PASS|FAIL".
"""
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
SRC = os.path.join(HERE, "out")
DEST = "/Game/Arena"
PIECES = ["SM_Arena_TeamBench", "SM_Arena_PenaltyBox"]
# slot name (the FBX material name) -> device B's instance. RinkActor re-binds `team` per side (MI_Toon_Blue / MI_Toon_Orange) at Build.
SLOT_LOOK = {
    "board": "/JamAssets/Look/MI_Toon_Board",
    "kick": "/JamAssets/Look/MI_Toon_KickPlate",
    "bench": "/JamAssets/Look/MI_Toon_Bench",
    "team": "/JamAssets/Look/MI_Toon_Blue",
    "dark": "/JamAssets/Look/MI_Toon_Dark",
}
TRI_BUDGET = 3000


def log(msg):
    unreal.log("A103 IMPORT " + msg)


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


def main():
    with open(os.path.join(SRC, "arena.json")) as fh:
        blender = json.load(fh)["pieces"]
    report = {"pieces": {}, "slot_look": SLOT_LOOK}
    ok = True
    for name in PIECES:
        mesh = import_piece(name)
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
            "under_budget": tris < TRI_BUDGET,
            "triangles_match_blender": tris == blender[name]["triangles"],
            "five_slots": sorted(slots) == sorted(SLOT_LOOK.keys()),
            # Unreal flips V on import (v -> 1 - v): the extents must match Blender's, the U range exactly
            "has_uvs": uvn > 0 and abs(uvb[0] - bu0[0]) < 0.01 and abs(uvb[2] - bu1[0]) < 0.01 and abs((uvb[3] - uvb[1]) - (bu1[1] - bu0[1])) < 0.01 and (uvb[2] - uvb[0]) > 0.1,
            "extends_to_plus_y": box.max.y > 100.0 and box.min.y > -30.0,
            "stands_on_the_ice": abs(box.min.z) < 5.0,
            "size_matches_blender": all(abs(a - b) < 0.02 for a, b in zip(size, blender[name]["dimensions_m"])),
        }
        if name == "SM_Arena_PenaltyBox":
            checks["seat_socket"] = socket is not None
        row["checks"] = checks
        if not all(checks.values()):
            ok = False
        report["pieces"][name] = row
        unreal.EditorAssetLibrary.save_asset(DEST + "/" + name, only_if_is_dirty=False)
        log("%s %s" % (name, json.dumps(row)))
    report["verdict"] = "pass" if ok else "fail"
    with open(os.path.join(SRC, "import.json"), "w") as fh:
        json.dump(report, fh, indent=2)
    log("PASS" if ok else "FAIL")


main()
