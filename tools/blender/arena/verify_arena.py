"""tools/blender/arena/verify_arena.py: headless proof that ARinkActor places the arena pieces (Third Jam, track F3, lanes A103 and A119).

Run (Git Bash, from the worktree root; a commandlet: no window, no renderer, no MCP port, no cursor; about 80 s):
  "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe" "$PWD/Game/ThirdJam.uproject" -run=pythonscript \
      -script="$PWD/tools/blender/arena/verify_arena.py" -unattended -nopause -nosplash -NullRHI -Multiprocess -abslog="$PWD/Game/logs/a119-verify.log"
(The full headless editor runs it too, with -ExecutePythonScript=<script> -nullrhi in place of -run and -script, but its start took 2 to 9 minutes under load.)

Spawns a transient ARinkActor in the editor world (nothing is saved), builds it for three rink sizes and every look tier, and checks:
  - the four components carry the two meshes at the places the formula gives (and, for 120 x 60, the places arena.py wrote to arena.json);
  - the five slots carry device B's instances, the `team` slot blue on x < 0 and orange on x > 0;
  - shown on l2_night, p3_clay, p4_broadcast; hidden on l1_day and l3_neon (the bare tiers), where A25's code benches show instead;
  - `jam.rink.arena 0` gives back A25's code benches on every tier;
  - GetPenaltyBoxSeat returns the socket's world place, and how far that is from where the sim pins a boxed skater today.
Lane A119 adds:
  - the scorer's table at x = 0 on the -y side, shown with the boxes; its slots on B's instances (`frame` on MI_Toon_NetFrame);
  - the goals on the goal lines (yaw -90 on the +x end, +90 on the -x end), frame on MI_Toon_NetFrame and cords on MI_Toon_NetMesh; the code frame
    (`Nets`) hidden exactly when they show; on three rink sizes and six goal sizes (the three built, two scaled widths, one scaled both ways) the
    opening in the world (the opening import_arena.py measured on the mesh, times the component's scale) within 2 cm of the cartridge's goal AND of the
    goal trigger box, which Build sizes from the same numbers the sim's goal test reads.
Writes tools/blender/arena/out/verify.json and prints "A119 VERIFY PASS|FAIL".
"""
import json
import os

import unreal

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "out")
BOARD_T, GAP = 0.2, 0.05            # ARinkActor::BoardThicknessM, ArenaGapM
SKATER_R = 0.4                      # UJamSim::SkaterRadius
EXPECT_LOOK = {
    "board": "MI_Toon_Board", "kick": "MI_Toon_KickPlate", "bench": "MI_Toon_Bench", "dark": "MI_Toon_Dark",
}
TIERS = ["l2_night", "p3_clay", "p4_broadcast", "l1_day", "l3_neon"]
BARE = {"l1_day", "l3_neon"}


def log(msg):
    unreal.log("A119 VERIFY " + msg)


def near(a, b, tol=0.6):
    return abs(a - b) <= tol


def comp_row(c):
    loc = c.get_relative_location() if hasattr(c, "get_relative_location") else c.get_editor_property("relative_location")
    rot = c.get_editor_property("relative_rotation")
    scale = c.get_editor_property("relative_scale3d")
    mesh = c.get_editor_property("static_mesh")
    names = [str(n) for n in c.get_material_slot_names()]
    mats = {}
    for n in names:
        m = c.get_material(c.get_material_index(n))
        mats[n] = m.get_name() if m else None
    return {
        "mesh": mesh.get_name() if mesh else None,
        "loc_cm": [round(loc.x, 1), round(loc.y, 1), round(loc.z, 1)],
        "yaw": round(rot.yaw, 1),
        "scale": [round(scale.x, 4), round(scale.y, 4), round(scale.z, 4)],
        "visible": bool(c.is_visible()),
        "materials": mats,
    }


def seat_of(rink, team):
    """GetPenaltyBoxSeat through Python: (location, rotation), or None when the function answers false.
    (Unreal's Python folds a bool return with out parameters into the out values, or None on false; a 3-tuple form is handled too.)"""
    r = rink.get_penalty_box_seat(team)
    if r is None:
        return None
    if isinstance(r, tuple) and len(r) == 3:
        return (r[1], r[2]) if r[0] else None
    return (r[0], r[1])


def main():
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    rink = eas.spawn_actor_from_class(unreal.RinkActor, unreal.Vector(0, 0, 0))
    rink.set_flags(unreal.ObjectFlags.RF_TRANSIENT) if hasattr(rink, "set_flags") else None
    with open(os.path.join(OUT, "arena.json")) as fh:
        blender = json.load(fh)
    report = {"sizes": {}, "tiers": {}, "checks": {}}
    ok = True

    def build(length, width, tier, goal=None):
        cfg = unreal.JamGameConfig()
        r = cfg.get_editor_property("rink")
        r.set_editor_property("length_m", float(length))
        r.set_editor_property("width_m", float(width))
        if goal:
            r.set_editor_property("goal_width_m", float(goal[0]))
            r.set_editor_property("goal_height_m", float(goal[1]))
        cfg.set_editor_property("rink", r)
        look = cfg.get_editor_property("look")
        look.set_editor_property("tier", tier)
        cfg.set_editor_property("look", look)
        rink.build(cfg)

    comps = lambda: {n: rink.get_editor_property(n) for n in ("arena_bench_neg", "arena_bench_pos", "arena_box_neg", "arena_box_pos")}
    comps2 = lambda: {n: rink.get_editor_property(n) for n in ("arena_goal_neg", "arena_goal_pos", "arena_goal_net_neg", "arena_goal_net_pos", "arena_table")}
    nets_visible = lambda: bool(rink.get_editor_property("nets").is_visible())
    with open(os.path.join(OUT, "import.json")) as fh:
        imported = json.load(fh)["pieces"]

    # ---- 1. places, scale, meshes and materials on three rinks (tier l2_night)
    for (length, width) in ((120, 60), (90, 45), (140, 70)):
        build(length, width, "l2_night")
        y0 = (width * 0.5 + BOARD_T + GAP) * 100.0
        bx = 0.0833 * length * 100.0
        rows = {n: comp_row(c) for n, c in comps().items()}
        exp = {
            "arena_bench_neg": ("SM_Arena_TeamBench", -bx, y0, 0.0, length * 0.1 / 12.0, "MI_Toon_Blue"),
            "arena_bench_pos": ("SM_Arena_TeamBench", bx, y0, 0.0, length * 0.1 / 12.0, "MI_Toon_Orange"),
            "arena_box_neg": ("SM_Arena_PenaltyBox", -300.0, -y0, 180.0, 1.0, "MI_Toon_Blue"),
            "arena_box_pos": ("SM_Arena_PenaltyBox", 300.0, -y0, 180.0, 1.0, "MI_Toon_Orange"),
        }
        size_ok = True
        for n, (mesh, x, y, yaw, sx, team) in exp.items():
            row = rows[n]
            good = (row["mesh"] == mesh and near(row["loc_cm"][0], x) and near(row["loc_cm"][1], y) and near(row["loc_cm"][2], 0.0)
                    and near(abs(row["yaw"]), yaw, 0.1) and near(row["scale"][0], sx, 0.001) and row["scale"][1] == 1.0 and row["scale"][2] == 1.0
                    and row["visible"] and row["materials"].get("team") == team
                    and all(row["materials"].get(k) == v for k, v in EXPECT_LOOK.items()))
            row["as_expected"] = good
            size_ok = size_ok and good
        seat = seat_of(rink, 0)
        seat1 = seat_of(rink, 1)
        # where the sim pins a boxed skater today (UJamSim::BoxPos): x = -3 (team 0), rink z = -(W/2 - SkaterRadius - 0.3), on the ice
        sim = (-300.0, -(width * 0.5 - SKATER_R - 0.3) * 100.0)
        entry = {"components": rows, "shown": bool(rink.is_arena_dressing_shown()), "code_benches_visible": bool(rink.get_editor_property("benches").is_visible())}
        if seat and seat1:
            loc = seat[0]
            entry["seat_team0_cm"] = [round(loc.x, 1), round(loc.y, 1), round(loc.z, 1)]
            entry["seat_team0_yaw"] = round(seat[1].yaw, 1)
            entry["seat_team1_cm"] = [round(seat1[0].x, 1), round(seat1[0].y, 1), round(seat1[0].z, 1)]
            entry["sim_box_pos_team0_cm"] = [sim[0], round(sim[1], 1)]
            entry["seat_minus_sim_cm"] = [round(loc.x - sim[0], 1), round(loc.y - sim[1], 1)]
            seat_ok = near(loc.x, -300.0 - 35.0, 1.0) and near(loc.y, -y0 - 62.0, 1.0) and near(loc.z, 10.0, 1.0) and near(seat[1].yaw, 90.0, 0.1)
        else:
            seat_ok = False
        entry["seat_ok"] = seat_ok
        size_ok = size_ok and seat_ok and entry["shown"] and not entry["code_benches_visible"]
        if length == 120:
            pl = blender["placement_m"]
            same = all(near(rows[n]["loc_cm"][0], p["x"] * 100.0) and near(rows[n]["loc_cm"][1], p["y"] * 100.0)
                       for n, p in (("arena_bench_neg", pl["SM_Arena_TeamBench"][0]), ("arena_bench_pos", pl["SM_Arena_TeamBench"][1]),
                                    ("arena_box_neg", pl["SM_Arena_PenaltyBox"][0]), ("arena_box_pos", pl["SM_Arena_PenaltyBox"][1])))
            entry["matches_arena_json"] = same
            size_ok = size_ok and same
        # ---- lane A119: the scorer's table and the goals on this rink, for six goal sizes
        rows2 = {n: comp_row(c) for n, c in comps2().items()}
        t = rows2["arena_table"]
        table_ok = (t["mesh"] == "SM_Arena_ScorerTable" and near(t["loc_cm"][0], 0.0) and near(t["loc_cm"][1], -y0) and near(abs(t["yaw"]), 180.0, 0.1)
                    and t["scale"] == [1.0, 1.0, 1.0] and t["visible"] and t["materials"].get("frame") == "MI_Toon_NetFrame"
                    and all(t["materials"].get(k) == v for k, v in EXPECT_LOOK.items()))
        entry["table"] = dict(t, as_expected=table_ok)
        goals = {}
        goals_ok = True
        gl = (length * 0.5 - 4.0) * 100.0     # FJamRinkConfig::GoalLineX
        for goal in ((4.0, 0.61), (6.0, 0.61), (4.0, 1.22), (5.5, 0.61), (2.5, 0.61), (5.0, 1.0)):
            build(length, width, "l2_night", goal)
            g = {n: comp_row(c) for n, c in comps2().items() if n != "arena_table"}
            trig = rink.get_editor_property("goal_trigger_pos")
            ext = trig.get_scaled_box_extent()
            tloc = trig.get_editor_property("relative_location")
            row = {"components": g, "code_frame_visible": nets_visible(), "shown": bool(rink.is_arena_goal_shown()),
                   "sim_volume_cm": {"half_width": round(ext.y, 2), "height": round(ext.z * 2.0, 2), "front_x": round(tloc.x - ext.x, 2), "depth": round(ext.x * 2.0, 2)}}
            good = row["shown"] and not row["code_frame_visible"]
            for n, sign in (("arena_goal_neg", -1.0), ("arena_goal_pos", 1.0)):
                c = g[n]
                m = imported.get(c["mesh"] or "", {}).get("goal")
                if not m:
                    good = False
                    continue
                half = m["opening_half_width_m"] * 100.0 * c["scale"][0]
                height = m["opening_height_m"] * 100.0 * c["scale"][2]
                c["opening_world_cm"] = {"half_width": round(half, 2), "height": round(height, 2)}
                c["opening_error_cm"] = round(max(abs(half - goal[0] * 50.0), abs(height - goal[1] * 100.0), abs(half - ext.y), abs(height - ext.z * 2.0)), 2)
                good = (good and c["opening_error_cm"] <= 2.0 and near(c["loc_cm"][0], sign * gl) and near(c["loc_cm"][1], 0.0) and near(c["loc_cm"][2], 0.0)
                        and near(c["yaw"], -90.0 * sign, 0.1) and c["scale"][1] == 1.0 and c["visible"]
                        and near(abs(tloc.x - ext.x), gl, 0.6)
                        and c["materials"] == {"frame": "MI_Toon_NetFrame"})
                # the net: its own mesh on the frame's transform, cords on B's MI_Toon_NetMesh, and no outline stencil (the frame has it)
                nc = g[n.replace("arena_goal_", "arena_goal_net_")]
                comp = rink.get_editor_property(n)
                ncomp = rink.get_editor_property(n.replace("arena_goal_", "arena_goal_net_"))
                c["custom_depth"] = bool(comp.get_editor_property("render_custom_depth"))
                nc["custom_depth"] = bool(ncomp.get_editor_property("render_custom_depth"))
                good = (good and nc["mesh"] == m.get("net_mesh") and nc["loc_cm"] == c["loc_cm"] and nc["yaw"] == c["yaw"] and nc["scale"] == c["scale"] and nc["visible"]
                        and nc["materials"] == {"net": "MI_Toon_NetMesh"} and c["custom_depth"] and not nc["custom_depth"])
            row["ok"] = good
            goals_ok = goals_ok and good
            goals["%gx%g" % goal] = row
            log("size %dx%d goal %gx%g mesh=%s scale=%s opening_error_cm=%s ok=%s" % (length, width, goal[0], goal[1], g["arena_goal_pos"]["mesh"], g["arena_goal_pos"]["scale"],
                                                                                 g["arena_goal_pos"].get("opening_error_cm"), good))
        entry["goals"] = goals
        size_ok = size_ok and table_ok and goals_ok
        entry["ok"] = size_ok
        ok = ok and size_ok
        report["sizes"]["%dx%d" % (length, width)] = entry
        log("size %dx%d ok=%s shown=%s seat0=%s seat-sim=%s" % (length, width, size_ok, entry["shown"], entry.get("seat_team0_cm"), entry.get("seat_minus_sim_cm")))

    # ---- 2. tiers: built on each tier, and switched to each tier without a rebuild
    for tier in TIERS:
        build(120, 60, tier)
        built = bool(rink.is_arena_dressing_shown())
        code = bool(rink.get_editor_property("benches").is_visible())
        vis = [bool(c.is_visible()) for c in comps().values()]
        want = tier not in BARE
        vis2 = [bool(c.is_visible()) for c in comps2().values()]
        good = built == want and code == (not want) and all(v == want for v in vis) and bool(rink.is_bare_tier(tier)) == (not want)
        good = good and all(v == want for v in vis2) and bool(rink.is_arena_goal_shown()) == want and nets_visible() == (not want)
        report["tiers"][tier] = {"shown": built, "code_benches_visible": code, "pieces_visible": vis, "goals_and_table_visible": vis2, "code_frame_visible": nets_visible(), "ok": good}
        ok = ok and good
        log("tier %s shown=%s code_benches=%s ok=%s" % (tier, built, code, good))
    build(120, 60, "l2_night")
    switched = {}
    for tier in ("p3_clay", "l1_day", "l2_night", "l3_neon", "p4_broadcast"):
        rink.set_zone_rings_for_tier(tier)
        switched[tier] = bool(rink.is_arena_dressing_shown())
        switched[tier + ":goals"] = bool(rink.is_arena_goal_shown()) and not nets_visible()
    report["checks"]["switch_without_rebuild"] = switched
    sw_ok = all(switched[t] == (t.split(":")[0] not in BARE) for t in switched)
    ok = ok and sw_ok

    # ---- 3. the neutral value: jam.rink.arena 0 gives back A25's code benches (the sink calls SetConstructionVisible; no frame ticks here, so call it)
    rink.set_zone_rings_for_tier("l2_night")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.arena 0")
    rink.set_construction_visible(True)
    off = {"shown": bool(rink.is_arena_dressing_shown()), "code_benches_visible": bool(rink.get_editor_property("benches").is_visible()),
           "pieces_visible": [bool(c.is_visible()) for c in comps().values()], "seat_returned": seat_of(rink, 0) is not None,
           "goals_and_table_visible": [bool(c.is_visible()) for c in comps2().values()], "goals_shown": bool(rink.is_arena_goal_shown()), "code_frame_visible": nets_visible()}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.arena 1")
    rink.set_construction_visible(True)
    on = {"shown": bool(rink.is_arena_dressing_shown()), "code_benches_visible": bool(rink.get_editor_property("benches").is_visible()),
          "goals_shown": bool(rink.is_arena_goal_shown()), "code_frame_visible": nets_visible()}
    # and the construction switch hides everything, as before
    rink.set_construction_visible(False)
    none = {"shown": bool(rink.is_arena_dressing_shown()), "code_benches_visible": bool(rink.get_editor_property("benches").is_visible()),
            "goals_shown": bool(rink.is_arena_goal_shown()), "code_frame_visible": nets_visible()}
    rink.set_construction_visible(True)
    report["checks"]["neutral_cvar"] = {"arena_0": off, "arena_1": on, "construction_off": none}
    cv_ok = (not off["shown"] and off["code_benches_visible"] and not any(off["pieces_visible"]) and not off["seat_returned"]
             and on["shown"] and not on["code_benches_visible"] and not none["shown"] and not none["code_benches_visible"]
             # lane A119: arena 0 gives the code frame back and hides the goals and the table; construction off keeps the bare code frame (as before A119)
             and not off["goals_shown"] and off["code_frame_visible"] and not any(off["goals_and_table_visible"])
             and on["goals_shown"] and not on["code_frame_visible"] and not none["goals_shown"] and none["code_frame_visible"])
    ok = ok and cv_ok
    log("neutral cvar ok=%s %s" % (cv_ok, json.dumps(report["checks"]["neutral_cvar"])))
    log("switch without rebuild ok=%s %s" % (sw_ok, json.dumps(switched)))

    eas.destroy_actor(rink)
    report["verdict"] = "pass" if ok else "fail"
    with open(os.path.join(OUT, "verify.json"), "w") as fh:
        json.dump(report, fh, indent=2)
    log("PASS" if ok else "FAIL")


try:
    main()
except Exception as exc:   # the editor must quit whatever happens
    import traceback
    unreal.log_error("A119 VERIFY FAIL exception: %s" % traceback.format_exc())
unreal.SystemLibrary.quit_editor()
