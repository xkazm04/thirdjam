"""tools/blender/arena/sync_start.py (Third Jam, track F3, lane A145): does an arena asset stall the match start?

The performance lane (A137) found the arena's meshes loaded synchronously on the menu -> match frame and put them into the match preload
(UJamPreloadSubsystem::MatchPaths, EffectsBinder.cpp). `tools/perf/syncrows.mjs` reads that from a packaged, windowed bot run (tools/perf/bot.ps1), which a
lane without the cursor cannot make. This script makes the same menu -> match step HEADLESS and reads the same log lines:

  1. starts ONE game process on the menu map (UnrealEditor-Cmd <project> -game -nullrhi -Multiprocess: no window, no mouse lock) with a cut on the command
     line and the scripted human, on the lane's own remote console port;
  2. waits for the preload's `JAMPRELOAD done` line (or 60 s), then sends the bot's own step over the remote console: `open /Game/Maps/Rink`;
  3. waits for `JAMPRELOAD match start` and some seconds of match, stops the process IT started (never another one);
  4. counts the `JAMPRELOAD sync <package>` lines: every /Game/Arena package among them is a mesh ARinkActor::Build loaded on the game thread.

Run with the system Python from the worktree root (Git Bash or PowerShell):
  python tools/blender/arena/sync_start.py [--port 4745] [--cut tools/perf/cuts/perf.p3_clay_5v5.cartridge.json] [--dp jam.PreloadArena=0] [--name a145-sync-start]
`--dp jam.PreloadArena=0` is the control: the arena leaves the preload and every piece must show as a synchronous load (the instrument sees them).
Prints "A145 SYNC START PASS|FAIL" and a JSON line; exit 0 when no /Game/Arena package was loaded synchronously, 1 when one was, 2 could not run.
"""
import argparse
import json
import os
import re
import socket
import subprocess
import sys
import time

UE = os.environ.get("TJ_UE_CMD", "C:/Program Files/Epic Games/UE_5.9/Engine/Binaries/Win64/UnrealEditor-Cmd.exe")


def read(path):
    try:
        with open(path, "r", errors="replace") as fh:
            return fh.read()
    except OSError:
        return ""


def console(port, cmd, wait=15.0):
    """The remote console's line protocol (tools/perf/bot.ps1): hello, then exec."""
    with socket.create_connection(("127.0.0.1", port), timeout=wait) as c:
        f = c.makefile("rw", newline="\n")
        f.write('{"id":1,"op":"hello","version":1}\n')
        f.flush()
        f.readline()
        f.write(json.dumps({"id": 2, "op": "exec", "cmd": cmd}) + "\n")
        f.flush()
        t0 = time.time()
        while time.time() - t0 < wait:
            line = f.readline()
            if not line:
                break
            if line.startswith('{"id":2,'):
                return line.strip()
    return ""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", type=int, default=4745)
    ap.add_argument("--cut", default="tools/perf/cuts/perf.p3_clay_5v5.cartridge.json")
    ap.add_argument("--dp", default="")
    ap.add_argument("--name", default="a145-sync-start")
    ap.add_argument("--match-seconds", type=float, default=12.0)
    opts = ap.parse_args()
    root = os.getcwd().replace("\\", "/")
    log = "%s/Game/logs/%s.log" % (root, opts.name)
    if not os.path.exists(UE):
        print("A145 SYNC START could not run: no engine at %s" % UE)
        return 2
    if os.path.exists(log):
        os.remove(log)
    cut = opts.cut if os.path.isabs(opts.cut) or not os.path.exists(opts.cut) else "%s/%s" % (root, opts.cut)
    dp = "t.MaxFPS=60,jam.ScriptHuman=1" + ("," + opts.dp if opts.dp else "")
    args = [UE, "%s/Game/ThirdJam.uproject" % root, "-game", "-nullrhi", "-Multiprocess", "-unattended", "-nosplash", "-NoSound", "-log", "-abslog=%s" % log,
            "-cut=%s" % cut, "-dpcvars=%s" % dp, "-RemoteConsoleIP=127.0.0.1", "-RemoteConsolePort=%d" % opts.port]
    proc = subprocess.Popen(args, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    print("INFO sync start: my pid %d, port %d, cut %s, dp %s, log %s" % (proc.pid, opts.port, cut, dp, log))
    res = {"tool": "thirdjam.arena.sync_start/1", "log": log, "dp": dp, "cut": cut}
    try:
        t0 = time.time()
        while time.time() - t0 < 90 and proc.poll() is None and "JAMPRELOAD done" not in read(log):
            time.sleep(1.0)
        text = read(log)
        res["preload_done_line"] = (re.findall(r"JAMPRELOAD done:[^\r\n]*", text) or [None])[0]
        res["preload_requested_line"] = (re.findall(r"JAMPRELOAD requested[^\r\n]*", text) or [None])[0]
        res["menu_seconds"] = round(time.time() - t0, 1)
        if proc.poll() is not None:
            print("A145 SYNC START could not run: the game left before the match (%s)" % log)
            return 2
        time.sleep(3.0)
        answer = ""
        for attempt in range(3):
            try:
                answer = console(opts.port, "open /Game/Maps/Rink")
            except OSError as exc:
                answer = "error %s" % exc
            if '"ok":true' in answer:
                break
            time.sleep(2.0)
        res["console"] = answer[:200]
        t1 = time.time()
        while time.time() - t1 < 60 and proc.poll() is None and "JAMPRELOAD match start" not in read(log):
            time.sleep(0.5)
        time.sleep(opts.match_seconds)
    finally:
        if proc.poll() is None:
            proc.terminate()      # the process this script started, by its own handle
            try:
                proc.wait(15)
            except subprocess.TimeoutExpired:
                proc.kill()
    text = read(log)
    start = re.search(r"JAMPRELOAD match start:[^\r\n]* frame=(\d+)", text)
    sync = [(m.group(1), int(m.group(2))) for m in re.finditer(r"JAMPRELOAD sync (\S+) frame=(\d+)", text)]
    start_frame = int(start.group(1)) if start else None
    arena = [(p, f) for p, f in sync if p.startswith("/Game/Arena/")]
    at_start = [(p, f) for p, f in sync if start_frame is not None and f >= start_frame - 2]
    res.update({
        "match_start_line": start.group(0)[:260] if start else None, "match_start_frame": start_frame,
        "sync_loads_in_all": len(sync), "sync_loads_from_the_match_start": len(at_start), "sync_packages_from_the_match_start": sorted({p for p, _ in at_start})[:40],
        "arena_sync_loads": len(arena), "arena_sync_packages": sorted({p for p, _ in arena}),
        "rink_built": len(re.findall(r"JAMRINK built", text)), "caps_line": (re.findall(r"JAMRINK arena caps:[^\r\n]*", text) or [None])[-1],
        "box_line": (re.findall(r"JAMRINK arena box use:[^\r\n]*", text) or [None])[-1],
    })
    ok = start is not None and res["rink_built"] >= 1 and not arena
    res["verdict"] = "pass" if ok else "fail"
    print("A145 SYNC START %s: match start frame %s, %d synchronous loads from the match start on (%d in the whole log), %d of /Game/Arena %s"
          % ("PASS" if ok else "FAIL", start_frame, len(at_start), len(sync), len(arena), res["arena_sync_packages"]))
    print(json.dumps(res))
    return 0 if ok else (2 if start is None else 1)


sys.exit(main())
