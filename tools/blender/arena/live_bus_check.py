"""tools/blender/arena/live_bus_check.py (lane A136): reads a game log and checks the rink's answers against the bus's events.
One `JAMRINK react goal` per `JAMEVENT goal` (same sim frame), one `JAMRINK react board cap=` per hard skater `JAMEVENT board` (Strength >= 0.45, same frame).
A board event whose printed strength is within 0.0006 of the threshold is counted either way (the log prints three decimals).
  python tools/blender/arena/live_bus_check.py Game/logs/a136-live-bus.log
"""
import json
import re
import sys

MIN_STRENGTH = 0.45      # ARinkActor::RattleMinStrength


def main(path):
    ev = re.compile(r"JAMEVENT (\S+) who=(-?\d+) where=\(([-0-9.]+),([-0-9.]+),([-0-9.]+)\) strength=([0-9.]+) frame=(\d+) kind=(\S*)")
    rg = re.compile(r"JAMRINK react goal end=(\S+) frame=(\d+)")
    rb = re.compile(r"JAMRINK react board cap=(\d+) of (\d+) strength=([0-9.]+) frame=(\d+) tilt ([-0-9.]+) deg hop ([-0-9.]+) cm, ([0-9.]+) m from the hit")
    rr = re.compile(r"JAMRINK react rattle piece=(\S+) strength=([0-9.]+) frame=(\d+)")
    # lane A145: the box in use. One `react box` line per `foul` event; `react box end` when the counted penalty is over; `box left` when the boxed figure
    # is no longer drawn in its box (the sim released the skater): the two must agree to a frame for the clock's hand to be right.
    rx = re.compile(r"JAMRINK react box team=(\d) who=(-?\d+) frame=(\d+) penalty=(\d+)")
    rxe = re.compile(r"JAMRINK react box end team=(\d) frame=(\d+)")
    rxl = re.compile(r"JAMRINK box left team=(\d) frame=(\d+) last_seen=(\d+) counted_end=(-?\d+)")
    fouls, rbox, rbox_end, rbox_left = [], [], [], []
    # lane A160: a goal is one moment. The engine frame tag of the log line ("[...][123]") of the bus's goal, of the binder's `JAMFX goal` (it starts device
    # B's NS_goal in that call) and of the rink's answer; and the rink's own measured line `JAMRINK goal timing` (first frame off rest of each reaction).
    tag = re.compile(r"^\[[^\]]*\]\[\s*(\d+)\]")
    rt = re.compile(r"JAMRINK goal timing end=(\S+) frame=(\d+): .* lamp \+?(-?\d+) net \+?(-?\d+) caps \+?(-?\d+) wave_light \+?(-?\d+)")
    goal_tags, fx_tags, rgoal_tags, timings = [], [], [], []
    goals, hard, edge, soft, pucks = [], [], [], 0, 0
    rgoals, rcaps, rpieces = [], [], []
    setup = {}
    last = 0
    with open(path, "r", errors="replace") as fh:
        for line in fh:
            tm = tag.search(line)
            etag = int(tm.group(1)) if tm else -1
            if "JAMFX goal " in line:
                fx_tags.append(etag)
                continue
            m = rt.search(line)
            if m:
                timings.append({"end": m.group(1), "frame": int(m.group(2)), "lamp": int(m.group(3)), "net": int(m.group(4)), "caps": int(m.group(5)), "wave_light": int(m.group(6))})
                continue
            m = ev.search(line)
            if m:
                name, frame, kind, strength = m.group(1), int(m.group(7)), m.group(8), float(m.group(6))
                last = max(last, frame)
                if name == "goal":
                    goals.append(frame)
                    goal_tags.append(etag)
                elif name == "foul":
                    fouls.append(frame)
                elif name == "board":
                    if kind != "skater":
                        pucks += 1
                    elif abs(strength - MIN_STRENGTH) < 0.0006:
                        edge.append(frame)
                    elif strength >= MIN_STRENGTH:
                        hard.append(frame)
                    else:
                        soft += 1
                continue
            m = rg.search(line)
            if m:
                rgoals.append(int(m.group(2)))
                rgoal_tags.append(etag)
                continue
            m = rb.search(line)
            if m:
                rcaps.append((int(m.group(4)), float(m.group(7)), int(m.group(1)), int(m.group(2))))
                continue
            m = rr.search(line)
            if m:
                rpieces.append((int(m.group(3)), m.group(1)))
                continue
            m = rx.search(line)
            if m:
                rbox.append((int(m.group(3)), int(m.group(1)), int(m.group(4))))
                continue
            m = rxe.search(line)
            if m:
                rbox_end.append((int(m.group(2)), int(m.group(1))))
                continue
            m = rxl.search(line)
            if m:
                rbox_left.append({"team": int(m.group(1)), "seen_gone_at": int(m.group(2)), "last_boxed_frame": int(m.group(3)), "counted_end": int(m.group(4))})
                continue
            for key in ("JAMRINK arena caps:", "JAMRINK arena dressing shown", "JAMRINK built"):
                if key in line:
                    setup[key] = line[line.index(key):].strip()[:220]
    cap_frames = sorted(f for f, _, _, _ in rcaps)
    need = sorted(hard)
    # every hard hit has a cap answer on its frame; every cap answer belongs to a hard (or threshold) hit
    spare = list(cap_frames)
    missing = []
    for f in need:
        if f in spare:
            spare.remove(f)
        else:
            missing.append(f)
    unexplained = [f for f in spare if f not in edge]
    for f in [f for f in spare if f in edge]:
        edge.remove(f)
    dist = [d for _, d, _, _ in rcaps]
    res = {
        "tool": "thirdjam.arena.live_bus/1", "log": path, "last_sim_frame": last, "setup": setup,
        "goal_events": len(goals), "goal_answers": len(rgoals), "goals_match": sorted(goals) == sorted(rgoals),
        "hard_skater_board_events": len(need), "at_the_threshold": len(edge), "cap_answers": len(rcaps), "missing_answers": missing[:10], "unexplained_answers": unexplained[:10],
        "soft_skater_board_events": soft, "puck_board_events": pucks, "piece_rattles": len(rpieces),
        "farthest_cap_m": round(max(dist), 2) if dist else None, "mean_cap_m": round(sum(dist) / len(dist), 2) if dist else None,
        "caps_within_2m": sum(1 for d in dist if d <= 2.0),
    }
    # lane A145: the box. The sim's release frame is the frame after the last boxed one; the hand's error is the frames between it and the counted end.
    penalty = rbox[0][2] if rbox else 300
    off = [r["counted_end"] - (r["last_boxed_frame"] + 1) for r in rbox_left if r["counted_end"] >= 0]
    res.update({"foul_events": len(fouls), "box_answers": len(rbox), "fouls_match": sorted(fouls) == sorted(f for f, _, _ in rbox), "box_ends": len(rbox_end), "penalty_frames": penalty,
                "boxed_figures_seen_leaving": len(rbox_left), "counted_end_minus_release_frames": off, "worst_hand_error_deg": round(max([abs(o) for o in off] or [0]) * 360.0 / penalty, 2),
                "box_left": rbox_left[:12]})
    need_fouls = int(sys.argv[2]) if len(sys.argv) > 2 else 0
    need_goals = int(sys.argv[3]) if len(sys.argv) > 3 else 1
    # lane A160: the goal's timing. Every timing line is on its goal's bus frame; the worst first frame of each reaction; the engine frame of the three lines.
    timing_frames_ok = sorted(t["frame"] for t in timings) == sorted(goals[:len(timings)]) and len(timings) >= len(goals) - 1
    worst = {k: max([t[k] for t in timings] or [-1]) for k in ("lamp", "net", "caps", "wave_light")}
    unseen = sum(1 for t in timings for k in ("lamp", "net", "caps", "wave_light") if t[k] < 0)
    same_tick = sum(1 for a, b in zip(goal_tags, rgoal_tags) if a == b and a >= 0)
    fx_same_tick = sum(1 for a, b in zip(goal_tags, fx_tags) if a == b and a >= 0)
    res.update({"goal_timing_lines": len(timings), "goal_timing_on_goal_frames": timing_frames_ok, "goal_timing_worst_first_frame": worst, "goal_timing_unseen": unseen,
                "goal_timing": timings[:12], "rink_answer_in_the_goal_events_engine_frame": same_tick, "effect_lines": len(fx_tags), "effect_line_in_the_goal_events_engine_frame": fx_same_tick})
    timing_ok = (not timings and not goals) or (timing_frames_ok and unseen == 0 and worst["lamp"] == 0 and worst["net"] == 0 and worst["caps"] == 0 and worst["wave_light"] <= 1
                                                and same_tick == len(goals))
    ok = (res["goal_events"] >= need_goals and res["goals_match"] and (len(need) >= 1 or need_goals == 0) and not missing and not unexplained and res["caps_within_2m"] == len(dist)
          and res["fouls_match"] and len(fouls) >= need_fouls and res["worst_hand_error_deg"] <= 2.0 and timing_ok)
    res["verdict"] = "pass" if ok else "fail"
    print("A136 LIVE BUS %s: %d sim frames; goal events %d, rink goal answers %d; hard skater board events %d (+%d at the threshold), cap answers %d, farthest cap %s m; piece rattles %d; soft %d, puck %d"
          % ("PASS" if ok else "FAIL", last, res["goal_events"], res["goal_answers"], len(need), res["at_the_threshold"], res["cap_answers"], res["farthest_cap_m"], res["piece_rattles"], soft, pucks))
    print("A145 LIVE BUS box: foul events %d, rink box answers %d (the same frames: %s), counted ends %d, boxed figures seen leaving %d, counted end minus the sim's release %s frames (hand error at most %.2f deg)"
          % (len(fouls), len(rbox), res["fouls_match"], len(rbox_end), len(rbox_left), off, res["worst_hand_error_deg"]))
    print("A160 LIVE BUS goal timing %s: %d goals, %d timing lines (each on its goal's bus frame: %s); worst first frame off rest: lamp +%d, net +%d, caps +%d, wave lights +%d; the rink's answer in the goal event's engine frame %d of %d; `JAMFX goal` lines %d, %d in the goal event's engine frame"
          % ("PASS" if timing_ok else "FAIL", len(goals), len(timings), timing_frames_ok, worst["lamp"], worst["net"], worst["caps"], worst["wave_light"], same_tick, len(goals), len(fx_tags), fx_same_tick))
    print(json.dumps(res))
    return 0 if ok else 1


sys.exit(main(sys.argv[1]))
