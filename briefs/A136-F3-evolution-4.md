# Lane A136, track F3 (the arena in Blender), evolution 4 (device A, Fable)

You are lane A136 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's doc and status: `docs/experiments/ARENA.md` (sections 1 to 28), `status/A127.md`,
`tools/blender/arena/**` (the parametric script, the import, the placement check, the off-screen engine stills).

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main** (your three evolutions are
on it). It has no binaries: build the editor first (builds no longer queue across worktrees). Your status file: `status/A136.md`. Port 4736. Family:
`Game/Content/Arena/**`, `tools/blender/arena/**`, `RinkActor.*` (placement and the arena's own presentation), `docs/experiments/ARENA.md`, `ASSETS.md` rows.

## Evolution 4: the whole boards answer a hit, and the goal is loud on the shipped look
The game's brand is the hit (R-0041), and the shipped look is `p3_clay`. Two measured gaps from your last evolution:
1. **The whole boards answer a hit**: today a hard skater `board` event finds a piece to rock on about 15 % of the outline (only where a bench, box or table
   stands), and on `p3_clay` the glass and rail are hidden, so nothing moves on most hits. Clay stanchion caps or rail sections around the ring (Blender, the
   same thumb-pressed voice, instanced: under 3,000 triangles added to the scene in total), each rocking at the nearest section by the event's Strength.
   Bar: a hard skater `board` event anywhere on the outline finds a piece within 2 m on 100 % of it; nothing blocks the game camera's view of the ice from
   either side (stills from the game camera's angle on both long sides and both ends); GPU cost under 0.1 ms.
2. **The goal on clay is as loud as on night**: 4.0 % of game-camera pixels change on the goal's frame on `p3_clay` against 15.3 % on `l2_night`. Reach 10 %
   or more on clay by the default (the lamp's light, its colour against the warm barn, the goal light's size or place, the net's shake), without washing out
   the puck or the figures in the goal pause.
3. **The bus in a live match**: your tests drive `ARinkActor` through a bus they create. Prove the real binding: a headless game process (`-game -nullrhi`, your
   port, a scripted human or a drill that scores) logs one `JAMRINK react goal` line per `JAMEVENT goal`, and one board line per hard hit.
4. If time remains: **the box is used**: the door swings on entry and exit and the box's clock hand runs with the penalty's frames (modes that keep the whistle).

For device B in your report: every material you reference by name, and the lamp instance you wished for. Never edit `Game/Plugins/JamAssets`.
The gate: the full `Jam` filter green, the hash unchanged (nothing here is sim), the ledger check, `jam.rink.arena 0` and `jam.rink.react 0` give back the
earlier arena. Look at your stills before you report. Time box 90 minutes; interim in the status file.
