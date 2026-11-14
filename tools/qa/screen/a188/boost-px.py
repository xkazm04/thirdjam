# Lane A188: the pickups' size on screen from the game's JAMBOOST seen lines (jam.boost.Probe N), binned by the camera's distance.
#   python tools/qa/screen/a188/boost-px.py <game.log> [<game.log> ...]
# Prints, per log and per look, the median height and width in pixels (at the run's resolution) of the pickups on screen at 20 / 40 / 60 m
# (+-5 m), and the last JAMBOOST passes line (the human's passes within 5 m of a pickup he did not take).
import re
import statistics
import sys

SEEN = re.compile(r"JAMBOOST seen frame=(\d+) pickup=(\d+) kind=(\S+) look=(\S+) cam_m=([\d.]+) h_px=([-\d.]+) w_px=([-\d.]+) on_screen=(\d) view=(\d+)x(\d+)")
PASSES = re.compile(r"JAMBOOST passes .*")

for path in sys.argv[1:]:
    rows, last = [], None
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = SEEN.search(line)
            if m:
                rows.append(m.groups())
            p = PASSES.search(line)
            if p:
                last = p.group(0)
    print(f"== {path}: {len(rows)} seen lines")
    for look in sorted({r[3] for r in rows}):
        view = next((f"{r[8]}x{r[9]}" for r in rows if r[3] == look), "?")
        out = []
        for d in (20, 40, 60):
            hs = [float(r[5]) for r in rows if r[3] == look and r[7] == "1" and abs(float(r[4]) - d) <= 5 and float(r[5]) > 0]
            ws = [float(r[6]) for r in rows if r[3] == look and r[7] == "1" and abs(float(r[4]) - d) <= 5 and float(r[6]) > 0]
            out.append(f"{d} m: h {statistics.median(hs):.0f} px w {statistics.median(ws):.0f} px (n {len(hs)})" if hs else f"{d} m: none")
        print(f"  look={look} view={view}  " + "  |  ".join(out))
    print(f"  {last}")
