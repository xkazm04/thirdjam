# Lane A145, track F3 (the arena in Blender), evolution 5 (device A, Fable)

You are lane A145 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's doc and status: `docs/experiments/ARENA.md` (sections 1 to 37), `status/A136.md`,
`tools/blender/arena/**` (the parametric script, the import, the placement check, the off-screen engine stills, `live_bus.sh`, `pixel_change.py`, `gpu_pairs.py`).

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (the track's four
evolutions are on it). It has no binaries: build the editor first. Your status file: `status/A145.md`. Port 4745. Family: `Game/Content/Arena/**`,
`tools/blender/arena/**`, `RinkActor.*` (placement and the arena's own presentation), `EffectsBinder.cpp` only for `MatchPaths` (see item 4),
`docs/experiments/ARENA.md`, `ASSETS.md` rows.

## Evolution 5: the hit is seen, and the box is used
1. **A hard board hit is seen from the game camera** (your own next-2): today the caps' ripple changes 0.1 % of the picture at 12 m: the answer is correct and
   invisible. Reach 1 % or more of the game camera's picture on a hard skater `board` event at 12 m, on clay and on night, scaled by the event's Strength:
   rounder caps in team colours with a level of detail (the 24-triangle lumps read as tents), a rink-owned flash or dust on the board's top at the hit, the
   kick plate's own shudder: what reads. Still under 0.1 ms of GPU and 3,000 triangles added in view; nothing hides the ice from the game camera.
2. **A goal runs round the end's caps**: reuse the ripple as a wave from the goal light along the end boards, so the flash does not sag between its pulses
   (10.5 % at the troughs today: 13 % or more on clay), nothing at rest.
3. **The box is used** (the brief's item 4 of evolution 4): on the bus's `foul` event the penalty box's door opens within 12 frames, the boxed figure is at
   the seat (already built), the box's clock hand runs with the penalty's frames (within 2 degrees on every frame), the door opens again at the end; one
   `JAMRINK react box` line per `JAMEVENT foul` in a live headless match on a cut that keeps the whistle. Presentation only.
4. **A new arena asset must not stall the match start**: the performance lane found that the arena's meshes loaded synchronously at match start and added
   them to the preload's `MatchPaths` (`EffectsBinder.cpp`); any mesh you add goes into that list too (one line each: name them), and `tools/perf/syncrows.mjs`
   on a match start shows zero synchronous arena loads.

For device B in your report: every material you reference by name; the lamp instance B is building (handoff A-to-B-0024, ask 2) replaces your dynamic
instance of `MI_Toon_NetFrame` when it arrives: say in one line what the Director must change then. Never edit `Game/Plugins/JamAssets`.
The gate: the full `Jam` filter green, the hash unchanged (nothing here is sim), the ledger check, `jam.rink.arena 0`, `jam.rink.react 0` and `jam.rink.caps 0`
give back the earlier arena. Look at your stills before you report. Check memory before any game process (start none over 50 GB in use).
Time box 100 minutes; interim in the status file.
