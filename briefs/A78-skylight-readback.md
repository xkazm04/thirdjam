# Lane A78: is the sky-light readback gone on the clay tier? One measurement for device B (device A, from H16, Sonnet)

You are lane A78 of the Third Jam (team Hokej). Device B asks (handoff `A-to-B-0016`, B's answer point 3): the clay rig `LR_P3_Clay` no longer references the engine's
daylight cubemap (lane B28: a specified cubemap on the engine's small gray cube, no real-time capture). Measure once whether the two synchronous sky-light readbacks on
the first lit frames (profiler offender 1: 26-38 ms each, 56-72 ms per match start) are gone on the clay tier. If they are, B does the same to the night rig and A clears
`Rink.umap`'s sky light; if not, B switches both rigs to a movable real-time capture.

Read first: `docs/experiments/PROFILING.md` (A58: offender 1, the method, Insights export, the remote console), `docs/experiments/PERF-FIXES.md` (A65: `tools/perf/start.ps1`,
`startframes.mjs`, the before/after method on match start; the shadow pool and preload are in), `tools/perf/*.ps1`, `tools/perf/*.mjs`, `tools/perf/cuts/perf.p3_clay.cartridge.json`
and `perf.l2_night.cartridge.json`, `handoff/A-to-B-0016.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a78` (detached at main 2678378, which carries B's clay rig after B28 and A65's fixes; build the Development
game package yourself with `-WaitMutex`). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start; open no
interactive editor. Other lanes package concurrently: wait out an AutomationTool already running. One game at a time; GPU figures while another game window is open are
busy and said so.

You own: `tools/perf/results/a78/**`, `docs/experiments/SKYLIGHT.md`, `status/A78.md`. No source edits.

## Job
1. Package Development; run A65's `start.ps1` method three times each on `perf.p3_clay` and `perf.l2_night`: the menu-to-match frame, the first lit frames, and the Insights
   or CSV evidence of the sky-light readback (the `SkyLight` / `ReadbackSkyLight` / `UpdateSkyCaptureContents` events on the first lit frames: present or absent, ms).
2. `SKYLIGHT.md`: the table (tier x run: match-start frame ms, first-lit frames ms, readback present yes/no with ms), the verdict in one sentence for B ("gone on clay with
   the specified cube: do the same to night" or "still there: switch to movable real-time capture"), and the remaining match-start budget per tier.
3. Append the verdict as a line under `## Answer` of `handoff/A-to-B-0016.md` (A's reply to B's point 3), dated.

Report: the table and the verdict, the file list. Time box 40 minutes.
