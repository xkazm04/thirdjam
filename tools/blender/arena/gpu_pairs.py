"""tools/blender/arena/gpu_pairs.py (Third Jam, track F3, lane A136): what the board caps cost on the GPU.
Reads the log of an off-screen stills run made with ARENA_STILLS_REACT=caps ARENA_STILLS_GPU=1 (engine_stills.py): ten ProfileGPU pairs from one
game-camera view, `jam.rink.caps 0` then `jam.rink.caps 1`, alternating. From each profile's "Graphics pipeline" table it takes three rows' inclusive
draws, primitives and time: the whole frame (<root>), BasePass and CustomDepth (the two passes the caps are drawn in: they cast no shadow). Prints the
medians with the caps off and on and their differences. The machine is shared, so the whole frame's time is noisy: the passes' own rows are the number.
  python tools/blender/arena/gpu_pairs.py Game/logs/a136-caps-stills.log
"""
import json
import re
import statistics
import sys

ROWS = ("<root>", "BasePass", "CustomDepth")


def main(path):
    mark = re.compile(r"A119 STILLS gpu (jam\.rink\.caps (\d)|ProfileGPU)")
    # ... time ms | incl draws | dispatches | prims | verts | percent | bar | time ms | name |
    row = re.compile(r"ms\s*\S\s*(\d+)\s*\S\s*(\d+)\s*\S\s*(\d+)\s*\S\s*(\d+)\s*\S\s*[0-9.]+%.*?([0-9.]+) ms\s*\S\s*(.+?)\s*\S\s*$")
    state, armed, graphics = None, False, False
    got = {0: [], 1: []}
    cur = None
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            m = mark.search(line)
            if m:
                if cur and state is not None and "<root>" in cur:
                    got[state].append(cur)
                    cur = None
                if m.group(2) is not None:
                    state = int(m.group(2))
                    armed = False
                else:
                    armed, graphics, cur = True, False, {}
                continue
            if not armed:
                continue
            if "pipeline" in line and "GPU" in line:
                graphics = "Graphics pipeline" in line
                continue
            if not graphics:
                continue
            m = row.search(line.rstrip())
            if m and m.group(6) in ROWS and m.group(6) not in cur:
                cur[m.group(6)] = {"draws": int(m.group(1)), "prims": int(m.group(3)), "ms": float(m.group(5))}
    if cur and state is not None and "<root>" in cur:
        got[state].append(cur)
    if not got[0] or not got[1]:
        print("A136 GPU could not read pairs from %s (caps off %d, caps on %d)" % (path, len(got[0]), len(got[1])))
        return 2
    res = {"tool": "thirdjam.arena.gpu_pairs/1", "frames_off": len(got[0]), "frames_on": len(got[1]), "rows": {}}
    for name in ROWS:
        off = [g[name] for g in got[0] if name in g]
        on = [g[name] for g in got[1] if name in g]
        if not off or not on:
            continue
        r = {}
        for key in ("ms", "draws", "prims"):
            a, b = statistics.median(x[key] for x in off), statistics.median(x[key] for x in on)
            r[key] = {"off": round(a, 3), "on": round(b, 3), "difference": round(b - a, 3)}
        r["ms"]["off_min_max"] = [min(x["ms"] for x in off), max(x["ms"] for x in off)]
        r["ms"]["on_min_max"] = [min(x["ms"] for x in on), max(x["ms"] for x in on)]
        res["rows"][name] = r
        print("A136 GPU %-12s caps off %7.3f ms (%d draws, %d prims) -> caps on %7.3f ms (%d draws, %d prims): %+.3f ms, %+d draws, %+d prims; off %s on %s"
              % (name, r["ms"]["off"], r["draws"]["off"], r["prims"]["off"], r["ms"]["on"], r["draws"]["on"], r["prims"]["on"], r["ms"]["difference"],
                 r["draws"]["difference"], r["prims"]["difference"], r["ms"]["off_min_max"], r["ms"]["on_min_max"]))
    passes = sum(res["rows"][n]["ms"]["difference"] for n in ("BasePass", "CustomDepth") if n in res["rows"])
    res["passes_difference_ms"] = round(passes, 3)
    res["under_0_1_ms"] = passes < 0.1
    print("A136 GPU the two passes the caps are drawn in: %+.3f ms (%s); %d + %d frames" % (passes, "under 0.1 ms" if passes < 0.1 else "NOT under 0.1 ms", len(got[0]), len(got[1])))
    print(json.dumps(res))
    return 0


sys.exit(main(sys.argv[1]))
