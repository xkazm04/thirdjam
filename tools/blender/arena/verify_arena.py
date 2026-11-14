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
Lane A127 (evolution 3) adds, on three rinks (section 4, `e3` in verify.json):
  - a goal light (SM_Arena_GoalLight) outside the end boards behind each net, shown with the boxes, A25's code housings (`GoalLights`) hidden exactly then;
  - the lamps (the table's and the two goal lights' slot `lamp`) on dynamic instances of B's MI_Toon_NetFrame, resting at its own Emissive;
  - a goal at the +x net: the table's lamp and the +x goal light up on the goal's frame, the -x light at rest, the +x net's scale off rest at frame 3,
    everything at rest again at frame 108 (the bar: 120); a hard skater board hit at the -x penalty box rolls that box 3.5 degrees (ARinkActor::RattleMaxDeg) and is over at frame 18;
  - nothing answers on a bare tier, with `jam.rink.react 0`, or with `jam.rink.arena 0`.
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

    def build(length, width, tier, goal=None, whistle=0, corner=None):
        cfg = unreal.JamGameConfig()
        # lane A160: the config's default rules whistle; every shipped cut but Glass House has the whistle off, and so has this check unless asked
        rules = cfg.get_editor_property("rules")
        rules.set_editor_property("penalties", whistle > 0)
        if whistle > 0:
            rules.set_editor_property("penalty_frames", int(whistle))
        cfg.set_editor_property("rules", rules)
        r = cfg.get_editor_property("rink")
        r.set_editor_property("length_m", float(length))
        r.set_editor_property("width_m", float(width))
        if goal:
            r.set_editor_property("goal_width_m", float(goal[0]))
            r.set_editor_property("goal_height_m", float(goal[1]))
        if corner:
            r.set_editor_property("corner_radius_m", float(corner))
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
    # Lane A160: sections 1 to 6 check the arena of evolutions 1 to 5, where a bare tier shows no Blender piece: that is `jam.rink.arena.bare 0` now (the
    # neutral value; every Build reads it). Section 7 runs at the default.
    bare_default = unreal.SystemLibrary.get_console_variable_int_value("jam.rink.arena.bare")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.arena.bare 0")
    # The same for the goal's light: sections 3 to 6 check evolution 4's loud goal (6,000 cd over the net), which is `jam.rink.react.loud 1` with
    # `jam.rink.react.light 6000` now; the default is device B's quiet light (section 7).
    loud_default = unreal.SystemLibrary.get_console_variable_int_value("jam.rink.react.loud")
    light_default = unreal.SystemLibrary.get_console_variable_float_value("jam.rink.react.light")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.loud 1")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.light 6000")
    # Lane A172: and the lamps' and cords' materials of evolutions 3 to 6 (the net frame's red, rest 3 and peak 17; MI_Toon_NetMesh): the neutral values.
    lamp_default = unreal.SystemLibrary.get_console_variable_int_value("jam.rink.lamp.trim")
    cord_default = unreal.SystemLibrary.get_console_variable_int_value("jam.rink.net.cord")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.lamp.trim 0")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.net.cord 0")

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

    # ---- 4. lane A127 (evolution 3): the goal lights, the lamps and the reactions
    def lamp_parent(c):
        m = c.get_material(c.get_material_index("lamp"))
        if not m or not isinstance(m, unreal.MaterialInstanceDynamic):
            return None
        par = m.get_editor_property("parent")
        return par.get_name() if par else None

    def goal_at(x_cm, frame):
        pl = unreal.JamEventPayload()
        pl.set_editor_property("where", unreal.Vector(x_cm, 0.0, 4.0))
        pl.set_editor_property("strength", 1.0)
        rink.react_to_event("goal", pl, frame)

    def hit_at(x_cm, y_cm, strength, frame, kind="skater"):
        pl = unreal.JamEventPayload()
        pl.set_editor_property("who", 0)
        pl.set_editor_property("where", unreal.Vector(x_cm, y_cm, 4.0))
        pl.set_editor_property("strength", strength)
        pl.set_editor_property("kind", kind)
        rink.react_to_event("board", pl, frame)

    def at_rest():
        s1, s2 = rink.get_net_shake_scale(1), rink.get_net_shake_scale(-1)
        rest = rink.get_lamp_rest_emissive()
        return (abs(rink.get_lamp_emissive() - rest) < 1e-6 and abs(rink.get_goal_light_emissive(1) - rest) < 1e-6 and abs(rink.get_goal_light_emissive(-1) - rest) < 1e-6
                and all(abs(v - 1.0) < 1e-6 for v in (s1.x, s1.y, s1.z, s2.x, s2.y, s2.z)) and abs(rink.get_rattle_deg()) < 1e-6 and not rink.is_reacting())

    report["e3"] = {}
    e3_ok = True
    for (length, width) in ((90, 45), (120, 60), (140, 70)):
        build(length, width, "p3_clay")
        ex = (length * 0.5 + BOARD_T + GAP) * 100.0
        gx = (length * 0.5 - 4.0 + 0.5) * 100.0
        lights = {n: rink.get_editor_property(n) for n in ("arena_goal_light_neg", "arena_goal_light_pos")}
        rows = {n: comp_row(c) for n, c in lights.items()}
        place_ok = True
        for n, sign in (("arena_goal_light_neg", -1.0), ("arena_goal_light_pos", 1.0)):
            r3 = rows[n]
            r3["lamp_parent"] = lamp_parent(lights[n])
            good = (r3["mesh"] == "SM_Arena_GoalLight" and near(r3["loc_cm"][0], sign * ex) and near(r3["loc_cm"][1], 0.0) and near(r3["loc_cm"][2], 0.0)
                    and near(r3["yaw"], -90.0 * sign, 0.1) and r3["scale"] == [1.0, 1.0, 1.0] and r3["visible"]
                    and r3["materials"].get("dark") == "MI_Toon_Dark" and r3["materials"].get("kick") == "MI_Toon_KickPlate" and r3["lamp_parent"] == "MI_Toon_NetFrame")
            r3["as_expected"] = good
            place_ok = place_ok and good
        code_lights = bool(rink.get_editor_property("goal_lights").is_visible())
        table_lamp = lamp_parent(rink.get_editor_property("arena_table"))
        rest = rink.get_lamp_rest_emissive()
        entry = {"goal_lights": rows, "code_goal_lights_visible": code_lights, "table_lamp_parent": table_lamp, "lamp_rest_emissive": round(rest, 4), "still_at_start": at_rest()}
        place_ok = place_ok and not code_lights and table_lamp == "MI_Toon_NetFrame" and rest >= 0.0 and entry["still_at_start"]
        # a goal in the +x net at sim frame 5000
        goal_at(gx, 5000)
        seq = {}
        for f in (0, 2, 3, 8, 18, 45, 107, 108, 120):
            rink.advance_reactions(5000 + f)
            s = rink.get_net_shake_scale(1)
            seq[str(f)] = {"light_cd": [round(rink.get_lamp_light_intensity(i), 1) for i in range(3)], "lamp": round(rink.get_lamp_emissive(), 4), "light_pos": round(rink.get_goal_light_emissive(1), 4), "light_neg": round(rink.get_goal_light_emissive(-1), 4),
                           "net_pos_scale": [round(s.x, 4), round(s.y, 4), round(s.z, 4)], "reacting": bool(rink.is_reacting())}
        goal_ok = (seq["0"]["lamp"] > rest + 12.0 and seq["2"]["lamp"] > rest + 12.0 and seq["0"]["light_pos"] > rest + 12.0
                   and all(abs(seq[k]["light_neg"] - rest) < 1e-4 for k in seq) and abs(seq["3"]["net_pos_scale"][1] - 1.0) > 0.05 and seq["3"]["net_pos_scale"][0] == 1.0
                   and seq["107"]["lamp"] > rest and abs(seq["108"]["lamp"] - rest) < 1e-6 and abs(seq["120"]["lamp"] - rest) < 1e-6
                   and seq["120"]["net_pos_scale"] == [1.0, 1.0, 1.0] and not seq["120"]["reacting"] and at_rest()
                   # the lamps' lights: the table's and the +x one on with the flash, the -x one never, all off again at rest
                   and seq["0"]["light_cd"][0] > 0.0 and seq["0"]["light_cd"][2] > 0.0 and all(seq[k]["light_cd"][1] == 0.0 for k in seq)
                   and seq["108"]["light_cd"] == [0.0, 0.0, 0.0] and seq["120"]["light_cd"] == [0.0, 0.0, 0.0])
        entry["goal_pos"] = seq
        # a goal in the -x net lights the -x light and shakes the -x net
        goal_at(-gx, 6000)
        rink.advance_reactions(6003)
        sn = rink.get_net_shake_scale(-1)
        entry["goal_neg_f3"] = {"lamp": round(rink.get_lamp_emissive(), 4), "light_neg": round(rink.get_goal_light_emissive(-1), 4), "light_pos": round(rink.get_goal_light_emissive(1), 4), "net_neg_scale_y": round(sn.y, 4)}
        goal_ok = goal_ok and entry["goal_neg_f3"]["light_neg"] > rest + 10.0 and abs(entry["goal_neg_f3"]["light_pos"] - rest) < 1e-4 and abs(sn.y - 1.0) > 0.05
        rink.advance_reactions(6120)
        goal_ok = goal_ok and at_rest()
        # a hard skater board hit in front of the -x penalty box; a soft one; the puck; an end board
        n0 = rink.get_rattle_count()
        hit_at(-300.0, -width * 50.0, 1.0, 7000)
        box = rink.get_editor_property("arena_box_neg")
        rattle = {"piece": str(rink.get_rattled_piece_name()), "deg_f0": round(rink.get_rattle_deg(), 3), "box_roll_f0": round(box.get_editor_property("relative_rotation").roll, 3), "count": rink.get_rattle_count() - n0}
        rink.advance_reactions(7018)
        rattle["deg_f18"] = round(rink.get_rattle_deg(), 3)
        rattle["box_roll_f18"] = round(box.get_editor_property("relative_rotation").roll, 3)
        n1 = rink.get_rattle_count()
        hit_at(-300.0, -width * 50.0, 0.3, 7100)
        hit_at(-300.0, -width * 50.0, 1.0, 7200, "None")
        hit_at(length * 50.0, 0.0, 1.0, 7300)
        rink.advance_reactions(7400)     # lane A136: the end-board hit now rocks a board cap (no piece): let its ripple end before the rest check
        rattle["soft_puck_endboard_count"] = rink.get_rattle_count() - n1
        rattle_ok = (rattle["piece"] == "ArenaBoxNeg" and abs(rattle["deg_f0"] - 3.5) < 1e-3 and abs(abs(rattle["box_roll_f0"]) - 3.5) < 0.01 and rattle["count"] == 1
                     and rattle["deg_f18"] == 0.0 and abs(rattle["box_roll_f18"]) < 1e-3 and rattle["soft_puck_endboard_count"] == 0 and at_rest())
        entry["rattle"] = rattle
        entry["ok"] = place_ok and goal_ok and rattle_ok
        e3_ok = e3_ok and entry["ok"]
        report["e3"]["%dx%d" % (length, width)] = entry
        log("e3 %dx%d place=%s goal=%s rattle=%s lamp rest %.3f -> %.3f at f0, at rest again f108; light x +-%.1f cm; %s" % (length, width, place_ok, goal_ok, rattle_ok, rest, seq["0"]["lamp"], ex, json.dumps(rattle)))
    # the still arena: a bare tier, jam.rink.react 0, jam.rink.arena 0
    still = {}
    build(120, 60, "l1_day")
    c0 = rink.get_goal_reaction_count()
    goal_at(5650.0, 8000)
    still["bare_tier"] = {"goal_lights_visible": [bool(rink.get_editor_property(n).is_visible()) for n in ("arena_goal_light_neg", "arena_goal_light_pos")],
                          "code_goal_lights_visible": bool(rink.get_editor_property("goal_lights").is_visible()), "reactions": rink.get_goal_reaction_count() - c0, "still": at_rest()}
    build(120, 60, "p3_clay")
    for cvar in ("jam.rink.react", "jam.rink.arena"):
        unreal.SystemLibrary.execute_console_command(world, cvar + " 0")
        rink.set_construction_visible(True)
        c0, r0 = rink.get_goal_reaction_count(), rink.get_rattle_count()
        goal_at(5650.0, 9000)
        hit_at(-300.0, -3000.0, 1.0, 9000)
        still[cvar + " 0"] = {"reactions": rink.get_goal_reaction_count() - c0 + rink.get_rattle_count() - r0, "still": at_rest(), "seat_returned": seat_of(rink, 0) is not None,
                              "goal_lights_visible": [bool(rink.get_editor_property(n).is_visible()) for n in ("arena_goal_light_neg", "arena_goal_light_pos")],
                              "code_goal_lights_visible": bool(rink.get_editor_property("goal_lights").is_visible())}
        unreal.SystemLibrary.execute_console_command(world, cvar + " 1")
        rink.set_construction_visible(True)
    c0 = rink.get_goal_reaction_count()
    goal_at(5650.0, 9500)
    still["back_on"] = {"reactions": rink.get_goal_reaction_count() - c0, "goal_lights_visible": [bool(rink.get_editor_property(n).is_visible()) for n in ("arena_goal_light_neg", "arena_goal_light_pos")]}
    rink.advance_reactions(9700)
    still_ok = (still["bare_tier"]["reactions"] == 0 and still["bare_tier"]["still"] and not any(still["bare_tier"]["goal_lights_visible"]) and still["bare_tier"]["code_goal_lights_visible"]
                and still["jam.rink.react 0"]["reactions"] == 0 and still["jam.rink.react 0"]["still"] and all(still["jam.rink.react 0"]["goal_lights_visible"]) and still["jam.rink.react 0"]["seat_returned"]
                and still["jam.rink.arena 0"]["reactions"] == 0 and still["jam.rink.arena 0"]["still"] and not any(still["jam.rink.arena 0"]["goal_lights_visible"])
                and still["jam.rink.arena 0"]["code_goal_lights_visible"] and not still["jam.rink.arena 0"]["seat_returned"]
                and still["back_on"]["reactions"] == 1 and all(still["back_on"]["goal_lights_visible"]))
    report["e3"]["still_arena"] = dict(still, ok=still_ok)
    e3_ok = e3_ok and still_ok
    ok = ok and e3_ok
    log("e3 still arena ok=%s %s" % (still_ok, json.dumps(still)))
    log("A127 E3 %s" % ("PASS" if e3_ok else "FAIL"))

    # ---- 5. lane A136 (evolution 4): the board caps round the whole ring, and the loud goal's light over the net
    import math
    report["e4"] = {}
    e4_ok = True
    for (length, width, tier) in ((90, 45, "p3_clay"), (120, 60, "p3_clay"), (140, 70, "p3_clay"), (120, 60, "l2_night")):
        build(length, width, tier)
        caps = rink.get_editor_property("arena_caps")
        caps_pos = rink.get_editor_property("arena_caps_pos")   # lane A145: the x >= 0 half (orange); arena_caps holds the x < 0 half (blue)
        n = rink.get_cap_count()
        places = [rink.get_cap_place_m(i) for i in range(n)]
        gaps = [math.hypot(places[i].x - places[(i + 1) % n].x, places[i].y - places[(i + 1) % n].y) for i in range(n)] if n else [99.0]
        mesh = caps.get_editor_property("static_mesh")
        mat = caps.get_material(0)
        mat_pos = caps_pos.get_material(0)
        zs = []
        for i in range(n):
            zs.append(rink.get_cap_transform(i).translation.z)
        # every cap in the boards' band: not outside the outer face, not farther inside than a corner's arc reaches
        outside = sum(1 for q in places if abs(q.x) > length * 0.5 + BOARD_T + 1e-6 or abs(q.y) > width * 0.5 + BOARD_T + 1e-6 or (abs(q.x) < length * 0.5 - 1.6 and abs(q.y) < width * 0.5 - 1.6))
        row = {"caps": n, "instances": caps.get_instance_count() + caps_pos.get_instance_count(), "instances_by_half": [caps.get_instance_count(), caps_pos.get_instance_count()],
               "shown": bool(rink.are_caps_shown()), "visible": bool(caps.is_visible() and caps_pos.is_visible()), "mesh": mesh.get_name() if mesh else None,
               "round": bool(rink.are_caps_round()), "material_pos": mat_pos.get_name() if mat_pos else None,
               "material": mat.get_name() if mat else None, "triangles": rink.get_cap_triangles(), "widest_gap_m": round(max(gaps), 3), "z_cm": [round(min(zs), 2), round(max(zs), 2)] if zs else None,
               "custom_depth": bool(caps.get_editor_property("render_custom_depth")), "cast_shadow": bool(caps.get_editor_property("cast_shadow"))}
        # hard hits: both ends, both long sides, the four corners (a skater's centre 0.45 m inside the boards)
        c = 5.0 * (1.0 - math.sqrt(0.5))
        spots = [(length * 0.5 - 0.45, 0.0), (-(length * 0.5 - 0.45), 3.0), (length * 0.31, width * 0.5 - 0.45), (-length * 0.4, -(width * 0.5 - 0.45))]
        spots += [(sx * (length * 0.5 - c - 0.32), sy * (width * 0.5 - c - 0.32)) for sx in (1, -1) for sy in (1, -1)]
        hits = []
        frame = 20000
        for (x, y) in spots:
            frame += 100
            rink.advance_reactions(frame)
            c0 = rink.get_cap_rock_count()
            hit_at(x * 100.0, y * 100.0, 0.8, frame)
            cap = rink.get_rocked_cap_index()
            h = {"at_m": [round(x, 2), round(y, 2)], "answered": rink.get_cap_rock_count() - c0, "cap": cap, "distance_m": round(rink.get_rocked_cap_distance_m(), 3),
                 "tilt_deg": round(rink.get_cap_tilt_deg(cap), 3), "hop_cm": round(rink.get_cap_hop_cm(cap), 3)}
            rink.advance_reactions(frame + 18)
            h["still_after_18"] = (rink.get_cap_tilt_deg(cap) == 0.0 and not rink.is_reacting())
            hits.append(h)
        row["hits"] = hits
        hit_ok = all(h["answered"] == 1 and h["distance_m"] <= 2.0 and abs(h["tilt_deg"] - 26.0 * 0.8) < 1e-2 and abs(h["hop_cm"] - 22.0 * 0.8) < 1e-2 and h["still_after_18"] for h in hits)
        # the loud goal's light: the +x end's hangs over the net (behind the goal line, inside the end boards)
        lx = rink.get_editor_property("arena_lamp_lights")[2].get_editor_property("relative_location")
        row["end_light_cm"] = [round(lx.x, 1), round(lx.y, 1), round(lx.z, 1)]
        row["end_light_peak_cd"] = [round(rink.lamp_light_peak_at(i), 1) for i in range(3)]
        light_ok = near(lx.x, (length * 0.5 - 4.0 + 3.0) * 100.0) and near(lx.y, 0.0) and near(lx.z, 360.0)
        row["ok"] = (n >= 4 and row["instances"] == n and row["shown"] and row["visible"] and row["mesh"] == ("SM_Arena_BoardCap" if n * 40 <= 6000 else "SM_Arena_BoardCapFar")
                     and row["round"] == (n * 40 <= 6000) and row["triangles"] == n * (40 if row["round"] else 24) and row["material"] == "MI_Toon_Blue" and row["material_pos"] == "MI_Toon_Orange"
                     and 0 < row["triangles"] <= 6000 and row["widest_gap_m"] <= 3.7 and row["z_cm"] == [120.0, 120.0] and outside == 0 and row["custom_depth"] and not row["cast_shadow"] and hit_ok and light_ok)
        e4_ok = e4_ok and row["ok"]
        report["e4"]["%dx%d %s" % (length, width, tier)] = row
        log("e4 %dx%d %s ok=%s: %d caps, %d triangles, widest gap %.2f m, hits %s, end light %s cd %s" % (length, width, tier, row["ok"], n, row["triangles"], row["widest_gap_m"],
            json.dumps([[h["at_m"], h["distance_m"]] for h in hits]), row["end_light_cm"], row["end_light_peak_cd"]))
    # the still boards: a bare tier, jam.rink.caps 0 (the pieces stay), and back
    still4 = {}
    build(120, 60, "l1_day")
    still4["bare_tier"] = {"shown": bool(rink.are_caps_shown()), "visible": bool(rink.get_editor_property("arena_caps").is_visible())}
    build(120, 60, "p3_clay")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.caps 0")
    rink.set_construction_visible(True)
    c0, r0 = rink.get_cap_rock_count(), rink.get_rattle_count()
    hit_at(5955.0, 0.0, 1.0, 30000)
    hit_at(-300.0, -3000.0, 1.0, 30000)
    still4["jam.rink.caps 0"] = {"shown": bool(rink.are_caps_shown()), "visible": bool(rink.get_editor_property("arena_caps").is_visible()), "cap_answers": rink.get_cap_rock_count() - c0,
                                 "piece_rattles": rink.get_rattle_count() - r0, "pieces_shown": bool(rink.is_arena_dressing_shown())}
    rink.advance_reactions(30100)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.caps 1")
    rink.set_construction_visible(True)
    c0 = rink.get_cap_rock_count()
    hit_at(5955.0, 0.0, 1.0, 31000)
    still4["back_on"] = {"shown": bool(rink.are_caps_shown()), "cap_answers": rink.get_cap_rock_count() - c0}
    rink.advance_reactions(31100)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.loud 0")
    rink.set_construction_visible(True)
    lx = rink.get_editor_property("arena_lamp_lights")[2].get_editor_property("relative_location")
    still4["jam.rink.react.loud 0"] = {"end_light_cm": [round(lx.x, 1), round(lx.y, 1), round(lx.z, 1)], "end_light_peak_cd": [round(rink.lamp_light_peak_at(i), 1) for i in range(3)]}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.loud 1")
    rink.set_construction_visible(True)
    still4_ok = (not still4["bare_tier"]["shown"] and not still4["bare_tier"]["visible"] and not still4["jam.rink.caps 0"]["shown"] and not still4["jam.rink.caps 0"]["visible"]
                 and still4["jam.rink.caps 0"]["cap_answers"] == 0 and still4["jam.rink.caps 0"]["piece_rattles"] == 1 and still4["jam.rink.caps 0"]["pieces_shown"]
                 and still4["back_on"]["shown"] and still4["back_on"]["cap_answers"] == 1
                 and still4["jam.rink.react.loud 0"]["end_light_cm"][0] > 6000.0 and still4["jam.rink.react.loud 0"]["end_light_peak_cd"] == [750.0, 3000.0, 3000.0])
    report["e4"]["still_boards"] = dict(still4, ok=still4_ok)
    e4_ok = e4_ok and still4_ok
    ok = ok and e4_ok
    log("e4 still boards ok=%s %s" % (still4_ok, json.dumps(still4)))
    log("A136 E4 %s" % ("PASS" if e4_ok else "FAIL"))

    # ---- 6. lane A145 (evolution 5): the hit flash, the goal wave, the box in use (ARinkActor's own read-outs on three rink/tier pairs)
    report["e5"] = {}
    e5_ok = True
    hit_cd = unreal.SystemLibrary.get_console_variable_float_value("jam.rink.react.hit")
    wave = unreal.SystemLibrary.get_console_variable_float_value("jam.rink.react.wave")
    for (length, width, tier) in ((90, 45, "p3_clay"), (120, 60, "p3_clay"), (90, 45, "l2_night")):
        build(length, width, tier)
        night = 0.2 if tier == "l2_night" else 1.0
        row = {"box_usable": bool(rink.is_box_usable()), "penalty_frames": rink.get_box_penalty_frames(), "hit_peak_cd": round(rink.hit_light_peak_here(), 1)}
        # the door and the hand hang on the box's sockets: their places in the box's own space are Blender's (arena.json sockets, metres)
        want = blender["pieces"]["SM_Arena_PenaltyBox"]["sockets"]
        places = {}
        for prop, sock in (("arena_box_door_neg", "Door"), ("arena_box_door_pos", "Door"), ("arena_box_hand_neg", "Hand"), ("arena_box_hand_pos", "Hand")):
            comp = rink.get_editor_property(prop)
            loc = comp.get_editor_property("relative_location")
            m = comp.get_editor_property("static_mesh")
            places[prop] = {"mesh": m.get_name() if m else None, "at_cm": [round(loc.x, 1), round(loc.y, 1), round(loc.z, 1)], "visible": bool(comp.is_visible()),
                            "ok": bool(m) and comp.is_visible() and all(abs(a - b * 100.0) < 1.0 for a, b in zip([loc.x, loc.y, loc.z], want[sock]))}
        row["places"] = places
        # a hard hit on the +y boards at Strength 0.8: one flash, candelas x Strength (x the night share), gone after 12 frames
        frame = 50000
        rink.advance_reactions(frame)
        hit_at(length * 25.0, (width * 0.5 - 0.45) * 100.0, 0.8, frame)
        lit = [round(rink.get_hit_light_intensity(i), 1) for i in range(4)]
        rink.advance_reactions(frame + 12)
        row["hit"] = {"lit_cd": lit, "after_12": [round(rink.get_hit_light_intensity(i), 1) for i in range(4)]}
        rink.advance_reactions(frame + 18)
        hit_ok = sorted(lit)[:3] == [0.0, 0.0, 0.0] and near(max(lit), hit_cd * night * 0.8, 1.0) and max(row["hit"]["after_12"]) == 0.0 and not rink.is_reacting()
        # a goal in the +x net: the wave's two lights at frame 9, caps in the air, all at rest after the lamp's 108 frames
        frame = 60000
        rink.advance_reactions(frame)
        pl = unreal.JamEventPayload()
        pl.set_editor_property("where", unreal.Vector((length * 0.5 - 3.5) * 100.0, 0.0, 4.0))
        pl.set_editor_property("strength", 1.0)
        rink.react_to_event("goal", pl, frame)
        rink.advance_reactions(frame + 9)
        hopping = sum(1 for i in range(rink.get_cap_count()) if rink.get_cap_hop_cm(i) > 0.0)
        wl = [round(rink.get_wave_light_intensity(i), 1) for i in range(2)]
        rink.advance_reactions(frame + 108)
        row["wave"] = {"lights_cd_f9": wl, "caps_in_the_air_f9": hopping, "end_light_cd": round(rink.lamp_light_peak_at(2), 1), "reacting_after_108": bool(rink.is_reacting())}
        wave_ok = near(wl[0], rink.lamp_light_peak_at(2) * wave * 0.5, 1.0) and wl[0] == wl[1] and hopping >= 1 and not row["wave"]["reacting_after_108"]
        # a foul by skater 0 (no teams in this config: team 1, the box at x = +3): door open by frame 11, the hand half way at half the penalty, at rest after
        frame = 70000
        rink.advance_reactions(frame)
        pl = unreal.JamEventPayload()
        pl.set_editor_property("who", 0)
        pl.set_editor_property("strength", 1.0)
        c0 = rink.get_box_reaction_count()
        rink.react_to_event("foul", pl, frame)
        pf = rink.get_box_penalty_frames()
        rink.advance_reactions(frame + 11)
        d11 = round(rink.get_box_door_deg(1), 2)
        rink.advance_reactions(frame + pf // 2)
        mid = [round(rink.get_box_door_deg(1), 2), round(rink.get_box_hand_deg(1), 2)]
        rink.advance_reactions(frame + pf + 11)
        d_end = round(rink.get_box_door_deg(1), 2)
        rink.advance_reactions(frame + pf + 80)      # the door's second cycle (74 frames) is over
        row["box"] = {"answers": rink.get_box_reaction_count() - c0, "door_deg_f11": d11, "door_hand_deg_half_way": mid, "door_deg_end_plus_11": d_end,
                      "other_box": [round(rink.get_box_door_deg(0), 2), round(rink.get_box_hand_deg(0), 2)], "at_rest_after": [round(rink.get_box_door_deg(1), 2), round(rink.get_box_hand_deg(1), 2), bool(rink.is_reacting())]}
        box_ok = (row["box_usable"] and all(p["ok"] for p in places.values()) and row["box"]["answers"] == 1 and near(d11, 72.0, 0.01) and mid[0] == 0.0 and near(mid[1], 180.0, 0.01)
                  and near(d_end, 72.0, 0.01) and row["box"]["other_box"] == [0.0, 0.0] and row["box"]["at_rest_after"] == [0.0, 0.0, False])
        row["ok"] = bool(hit_ok and wave_ok and box_ok and near(row["hit_peak_cd"], hit_cd * night, 0.5))
        e5_ok = e5_ok and row["ok"]
        report["e5"]["%dx%d %s" % (length, width, tier)] = row
        log("e5 %dx%d %s ok=%s (hit %s wave %s box %s): %s" % (length, width, tier, row["ok"], hit_ok, wave_ok, box_ok, json.dumps(row)))
    # the neutral values: jam.rink.react.hit 0 (no flash), jam.rink.react.wave 0 (no wave), jam.rink.react 0 (the doors stand open as the box was built before)
    still5 = {}
    build(90, 45, "p3_clay")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.hit 0")
    c0 = rink.get_cap_rock_count()
    hit_at(2250.0, 2205.0, 1.0, 80000)
    still5["jam.rink.react.hit 0"] = {"cap_answers": rink.get_cap_rock_count() - c0, "lit_cd": [round(rink.get_hit_light_intensity(i), 1) for i in range(4)]}
    rink.advance_reactions(80100)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.hit %g" % hit_cd)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.wave 0")
    pl = unreal.JamEventPayload()
    pl.set_editor_property("where", unreal.Vector(4150.0, 0.0, 4.0))
    rink.react_to_event("goal", pl, 81000)
    rink.advance_reactions(81009)
    still5["jam.rink.react.wave 0"] = {"caps_in_the_air_f9": sum(1 for i in range(rink.get_cap_count()) if rink.get_cap_hop_cm(i) > 0.0), "lights_cd_f9": [round(rink.get_wave_light_intensity(i), 1) for i in range(2)],
                                       "goal_light_lit": rink.get_goal_light_emissive(1) > rink.get_lamp_rest_emissive()}
    rink.advance_reactions(81200)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.wave %g" % wave)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react 0")
    rink.set_construction_visible(True)
    still5["jam.rink.react 0"] = {"door_deg": [round(rink.get_box_door_deg(t), 2) for t in range(2)], "hand_deg": [round(rink.get_box_hand_deg(t), 2) for t in range(2)]}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react 1")
    rink.set_construction_visible(True)
    still5["back_on"] = {"door_deg": [round(rink.get_box_door_deg(t), 2) for t in range(2)]}
    still5_ok = (still5["jam.rink.react.hit 0"] == {"cap_answers": 1, "lit_cd": [0.0, 0.0, 0.0, 0.0]} and still5["jam.rink.react.wave 0"] == {"caps_in_the_air_f9": 0, "lights_cd_f9": [0.0, 0.0], "goal_light_lit": True}
                 and still5["jam.rink.react 0"] == {"door_deg": [72.0, 72.0], "hand_deg": [0.0, 0.0]} and still5["back_on"] == {"door_deg": [0.0, 0.0]})
    report["e5"]["neutral"] = dict(still5, ok=still5_ok)
    e5_ok = e5_ok and still5_ok
    ok = ok and e5_ok
    log("e5 neutral ok=%s %s" % (still5_ok, json.dumps(still5)))
    log("A145 E5 %s" % ("PASS" if e5_ok else "FAIL"))

    # ---- 7. lane A160 (evolution 6): the dome on every rink, the bare tiers' goals and caps, the box seen from the ice, a goal's timing
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.arena.bare %d" % bare_default)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.loud %d" % loud_default)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.light %g" % light_default)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.lamp.trim %d" % lamp_default)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.net.cord %d" % cord_default)
    report["e6"] = {"jam.rink.arena.bare": bare_default, "jam.rink.react.loud": loud_default, "jam.rink.react.light": light_default}
    box_cd = unreal.SystemLibrary.get_console_variable_float_value("jam.rink.react.box")
    vis = lambda n: bool(rink.get_editor_property(n).is_visible())
    mesh_of = lambda n: (rink.get_editor_property(n).get_editor_property("static_mesh").get_name() if rink.get_editor_property(n).get_editor_property("static_mesh") else None)
    e6_ok = bare_default == 1 and box_cd > 0.0
    # the dome on the regular, the large and the wide rink
    domes = {}
    for length, width, corner in ((90, 45, 3.75), (120, 60, 5.0), (140, 70, 5.83)):
        build(length, width, "p3_clay", corner=corner)
        domes["%dx%d" % (length, width)] = {"caps": rink.get_cap_count(), "round": bool(rink.are_caps_round()), "triangles": rink.get_cap_triangles(), "mesh": mesh_of("arena_caps")}
    report["e6"]["domes"] = domes
    domes_ok = all(d["round"] and d["mesh"] == "SM_Arena_BoardCap" and d["triangles"] == d["caps"] * 40 for d in domes.values())
    # the pond: l1_day (a bare tier), 60 x 30, a 6 m goal, no whistle: its own Blender goal and the caps stand, no piece outside the boards
    build(60, 30, "l1_day", goal=(6.0, 0.61), corner=3.0)
    pond = {"goals_shown": bool(rink.is_arena_goal_shown()), "goal_mesh": mesh_of("arena_goal_pos"), "code_frame_visible": nets_visible(), "caps_shown": bool(rink.are_caps_shown()),
            "caps_round": bool(rink.are_caps_round()), "caps": rink.get_cap_count(), "dressing_shown": bool(rink.is_arena_dressing_shown()), "boxes_shown": bool(rink.are_boxes_shown()),
            "outside_visible": [n for n in ("arena_bench_neg", "arena_bench_pos", "arena_box_neg", "arena_box_pos", "arena_table", "arena_goal_light_neg", "arena_goal_light_pos") if vis(n)],
            "seat": seat_of(rink, 0) is not None}
    report["e6"]["pond"] = pond
    pond_ok = (pond["goals_shown"] and pond["goal_mesh"] == "SM_Arena_Goal_W600H061" and not pond["code_frame_visible"] and pond["caps_shown"] and pond["caps_round"]
               and not pond["dressing_shown"] and not pond["boxes_shown"] and pond["outside_visible"] == [] and not pond["seat"])
    # Glass House: l3_neon with the whistle (600 frames): the two boxes alone outside the boards, a seat, a light in the used box
    build(90, 45, "l3_neon", whistle=600, corner=3.75)
    s1 = seat_of(rink, 1)
    glass = {"boxes_shown": bool(rink.are_boxes_shown()), "dressing_shown": bool(rink.is_arena_dressing_shown()), "penalty_frames": rink.get_box_penalty_frames(),
             "visible": [n for n in ("arena_bench_neg", "arena_bench_pos", "arena_box_neg", "arena_box_pos", "arena_box_door_neg", "arena_box_door_pos", "arena_box_hand_neg", "arena_box_hand_pos",
                                     "arena_table", "arena_goal_light_neg", "arena_goal_light_pos") if vis(n)],
             "seat_team1_cm": [round(s1[0].x, 1), round(s1[0].y, 1), round(s1[0].z, 1)] if s1 else None, "light_peak_cd": round(rink.box_light_peak_here(), 1),
             "light_at_rest_cd": [round(rink.get_box_light_intensity(t), 1) for t in range(2)]}
    frame = 90000
    rink.advance_reactions(frame)
    pl = unreal.JamEventPayload()
    pl.set_editor_property("who", 0)       # no teams in this config: every index is team 1's, the box at x = +3
    pl.set_editor_property("strength", 1.0)
    pl.set_editor_property("kind", "foul_from_behind")
    rink.react_to_event("foul", pl, frame)
    glass["light_cd_f0"] = [round(rink.get_box_light_intensity(t), 1) for t in range(2)]
    rink.advance_reactions(frame + 30)
    glass["light_cd_f30"] = [round(rink.get_box_light_intensity(t), 1) for t in range(2)]
    rink.advance_reactions(frame + 599)
    glass["light_cd_f599"] = [round(rink.get_box_light_intensity(t), 1) for t in range(2)]
    rink.advance_reactions(frame + 600)
    glass["light_cd_f600"] = [round(rink.get_box_light_intensity(t), 1) for t in range(2)]
    rink.advance_reactions(frame + 600 + 80)
    glass["reacting_after"] = bool(rink.is_reacting())
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.box 0")
    rink.react_to_event("foul", pl, frame + 2000)
    rink.advance_reactions(frame + 2030)
    glass["jam.rink.react.box 0"] = {"light_cd_f30": [round(rink.get_box_light_intensity(t), 1) for t in range(2)], "hand_deg_f30": round(rink.get_box_hand_deg(1), 2)}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.react.box %g" % box_cd)
    rink.advance_reactions(frame + 2000 + 600 + 80)
    report["e6"]["glass_house"] = glass
    glass_ok = (glass["boxes_shown"] and not glass["dressing_shown"] and glass["penalty_frames"] == 600
                and glass["visible"] == ["arena_box_neg", "arena_box_pos", "arena_box_door_neg", "arena_box_door_pos", "arena_box_hand_neg", "arena_box_hand_pos"]
                and glass["seat_team1_cm"] is not None and glass["seat_team1_cm"][1] < -2270.0 and near(glass["light_peak_cd"], box_cd * 0.2, 0.5)
                and glass["light_at_rest_cd"] == [0.0, 0.0] and glass["light_cd_f0"][0] == 0.0 and glass["light_cd_f0"][1] > 0.0
                and glass["light_cd_f30"][0] == 0.0 and near(glass["light_cd_f30"][1], box_cd * 0.2, 0.5) and glass["light_cd_f599"][1] > 0.0 and glass["light_cd_f600"] == [0.0, 0.0]
                and not glass["reacting_after"] and glass["jam.rink.react.box 0"]["light_cd_f30"] == [0.0, 0.0] and near(glass["jam.rink.react.box 0"]["hand_deg_f30"], 18.0, 0.1))
    # a bare tier without the whistle: no box, no seat
    build(90, 45, "l3_neon", corner=3.75)
    quiet = {"boxes_shown": bool(rink.are_boxes_shown()), "box_visible": vis("arena_box_pos"), "seat": seat_of(rink, 1) is not None, "goals_shown": bool(rink.is_arena_goal_shown())}
    report["e6"]["neon_without_whistle"] = quiet
    quiet_ok = not quiet["boxes_shown"] and not quiet["box_visible"] and not quiet["seat"] and quiet["goals_shown"]
    # a goal's timing on the regular rink: the first frame each reaction is off rest after the goal's bus frame
    build(90, 45, "p3_clay", corner=3.75)
    frame = 120000
    rink.advance_reactions(frame)
    pl = unreal.JamEventPayload()
    pl.set_editor_property("where", unreal.Vector((90 * 0.5 - 3.5) * 100.0, 0.0, 4.0))
    pl.set_editor_property("strength", 1.0)
    rink.react_to_event("goal", pl, frame)
    for f in range(0, 19):
        rink.advance_reactions(frame + f)
    timing = [rink.get_goal_first_frame(w) for w in range(4)]
    rink.advance_reactions(frame + 200)
    report["e6"]["goal_first_frames"] = {"lamp": timing[0], "net": timing[1], "caps": timing[2], "wave_light": timing[3]}
    timing_ok = timing == [0, 0, 0, 1]
    # the goal's light at device B's numbers (the defaults): beside the lamp outside the end boards, a 3.5 m radius, 800 cd on clay and 320 on night
    lamps = rink.get_editor_property("arena_lamp_lights")
    lx = lamps[2].get_editor_property("relative_location")
    quiet_light = {"p3_clay_peak_cd": [round(rink.lamp_light_peak_at(i), 1) for i in range(3)], "radius_cm": [round(l.get_editor_property("attenuation_radius"), 1) for l in lamps],
                   "end_light_cm": [round(lx.x, 1), round(lx.y, 1), round(lx.z, 1)],
                   "wave_radius_cm": [round(l.get_editor_property("attenuation_radius"), 1) for l in rink.get_editor_property("arena_wave_lights")]}
    build(90, 45, "l2_night", corner=3.75)
    quiet_light["l2_night_peak_cd"] = [round(rink.lamp_light_peak_at(i), 1) for i in range(3)]
    report["e6"]["goal_light"] = quiet_light
    light_ok = (loud_default == 0 and near(light_default, 1600.0, 0.01) and quiet_light["p3_clay_peak_cd"] == [200.0, 800.0, 800.0] and quiet_light["l2_night_peak_cd"] == [80.0, 320.0, 320.0]
                and quiet_light["radius_cm"] == [350.0, 350.0, 350.0] and quiet_light["wave_radius_cm"] == [350.0, 350.0] and quiet_light["end_light_cm"][0] > 4500.0)
    e6_ok = bool(e6_ok and light_ok)
    report["e6"]["goal_light"]["ok"] = light_ok
    e6_ok = bool(e6_ok and domes_ok and pond_ok and glass_ok and quiet_ok and timing_ok)
    report["e6"]["ok"] = {"domes": domes_ok, "pond": pond_ok, "glass_house": glass_ok, "neon_without_whistle": quiet_ok, "goal_timing": timing_ok}
    ok = ok and e6_ok
    log("e6 %s" % json.dumps(report["e6"]))
    log("A160 E6 %s" % ("PASS" if e6_ok else "FAIL"))

    # ---- 8. lane A172 (evolution 7): device B's lamp and cords on the slots `lamp` and `net`
    report["e7"] = {"jam.rink.lamp.trim": lamp_default, "jam.rink.net.cord": cord_default}
    mat_on = lambda prop, slot: (lambda c: (lambda i: (c.get_material(i).get_name() if i >= 0 and c.get_material(i) else None))(c.get_material_index(slot)))(rink.get_editor_property(prop))
    parent_of = lambda prop, slot: (lambda c: (lambda i: (c.get_material(i).get_editor_property("parent").get_name() if i >= 0 and c.get_material(i) and isinstance(c.get_material(i), unreal.MaterialInstanceDynamic) else None))(c.get_material_index(slot)))(rink.get_editor_property(prop))
    build(90, 45, "p3_clay", corner=3.75)
    e7 = {"lamp_trim": bool(rink.is_lamp_trim()), "lamp_parent": {p: parent_of(p, "lamp") for p in ("arena_table", "arena_goal_light_neg", "arena_goal_light_pos")},
          "table_frame_slot": mat_on("arena_table", "frame"), "goal_frame_slot": mat_on("arena_goal_pos", "frame"),
          "cords": {p: mat_on(p, "net") for p in ("arena_goal_net_neg", "arena_goal_net_pos")}, "cord_look": str(rink.get_cord_look_name()),
          "rest": round(rink.get_lamp_rest_emissive(), 3), "rest_now": round(rink.get_lamp_emissive(), 3), "peak_add": round(rink.get_lamp_peak_add(), 3)}
    frame = 150000
    rink.advance_reactions(frame)
    pl = unreal.JamEventPayload()
    pl.set_editor_property("where", unreal.Vector((90 * 0.5 - 3.5) * 100.0, 0.0, 4.0))
    pl.set_editor_property("strength", 1.0)
    rink.react_to_event("goal", pl, frame)
    e7["goal_f0"] = {"table": round(rink.get_lamp_emissive(), 3), "scored_end": round(rink.get_goal_light_emissive(1), 3), "other_end": round(rink.get_goal_light_emissive(-1), 3)}
    rink.advance_reactions(frame + 200)
    e7["after"] = {"table": round(rink.get_lamp_emissive(), 3), "scored_end": round(rink.get_goal_light_emissive(1), 3)}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.lamp.trim 0")
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.net.cord 0")
    build(90, 45, "p3_clay", corner=3.75)
    e7["neutral"] = {"lamp_trim": bool(rink.is_lamp_trim()), "lamp_parent": parent_of("arena_table", "lamp"), "cords": mat_on("arena_goal_net_pos", "net"), "rest": round(rink.get_lamp_rest_emissive(), 3), "peak_add": round(rink.get_lamp_peak_add(), 3)}
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.lamp.trim %d" % lamp_default)
    unreal.SystemLibrary.execute_console_command(world, "jam.rink.net.cord %d" % cord_default)
    build(90, 45, "p3_clay", corner=3.75)
    report["e7"].update(e7)
    e7_ok = (lamp_default == 1 and cord_default == 1 and e7["lamp_trim"] and all(v == "MI_Trim_Lamp" for v in e7["lamp_parent"].values())
             and e7["table_frame_slot"] == "MI_Toon_NetFrame" and e7["goal_frame_slot"] == "MI_Toon_NetFrame"
             and all(v == "MI_Toon_NetCord" for v in e7["cords"].values()) and e7["cord_look"] == "MI_Toon_NetCord"
             and e7["rest"] == 0.5 and e7["rest_now"] == 0.5 and e7["peak_add"] == 8.5 and e7["goal_f0"] == {"table": 9.0, "scored_end": 9.0, "other_end": 0.5}
             and e7["after"] == {"table": 0.5, "scored_end": 0.5}
             and e7["neutral"] == {"lamp_trim": False, "lamp_parent": "MI_Toon_NetFrame", "cords": "MI_Toon_NetMesh", "rest": e7["neutral"]["rest"], "peak_add": 14.0} and e7["neutral"]["rest"] > 0.5)
    report["e7"]["ok"] = bool(e7_ok)
    ok = ok and e7_ok
    log("e7 %s" % json.dumps(report["e7"]))
    log("A172 E7 %s" % ("PASS" if e7_ok else "FAIL"))

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
