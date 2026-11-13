# Lane A127, track F3 (the arena in Blender), evolution 3 (device A, Fable)

You are lane A127 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, and the track's two evolutions in your worktree: `status/A103.md`, `status/A119.md`, `docs/experiments/ARENA.md`,
`tools/blender/arena/**` (the parametric script, the import, the placement check, the off-screen engine stills).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a103` (the track's: evolutions 1 and 2 are committed there as the baseline; 1 is on main, 2 is
landing; its editor is built). Your status file: `status/A127.md`. Port 4727. Family: `Game/Content/Arena/**`, `tools/blender/arena/**`, `RinkActor.*` (placement and
the arena's own presentation), `EffectsBinder.*` only for the one binding named in item 1, `docs/experiments/ARENA.md`, `ASSETS.md` rows for your meshes.

## Evolution 3: the arena answers the game
The pieces stand; nothing in them moves when something happens. The brand is raw and loud (R-0041): a goal should be answered by the building.
1. **A goal lights the lamp and shakes the net**: on the `goal` event (the bus the effects binder already listens to: `contracts/events.md`), the scorer's
   table's red lamp and a goal light behind the scored net flash (emissive up within 2 frames, back by frame 120), and the scored net's cords shake (a short
   damped wobble of the net mesh: a world-position-offset parameter on a dynamic material instance, or a small transform wobble: never a physics body). Presentation
   only: nothing enters the sim. Proof: a test that on each of 5 goals in a scripted match the lamp's emissive parameter rises within 2 frames and is back by 120,
   with no flash without a goal; the hash unchanged.
2. **A hard hit on the boards rattles them**: on a skater `board` event above a strength you choose, the nearest board section's glass rail (or your bench or
   box behind it) wobbles for a few frames, scaled by the event's Strength. Same proof pattern.
3. **The penalty box holds its skater** where a mode keeps the whistle: the boxed skater's figure is drawn at `GetPenaltyBoxSeat` (your own next-1 of evolution
   1: one presentation call in `JamPawns.cpp`, which is track F1's file: name the exact lines you add and keep it to that call). Bar: within 0.2 m of the `Seat`
   socket on all penalty frames of a drill, back on the ice the frame the penalty ends, the hash unchanged.
4. If time remains: a goal light piece in the clay voice behind each net (Blender, under 1,000 triangles), so item 1 has something to light.

Materials: you may make dynamic instances at run time and set parameters that device B's materials already expose; where a material lacks the parameter you need
(an emissive scalar on the lamp), build the lamp's own small material under `Game/Content/Arena/` (A's content) and say so; never edit `Game/Plugins/JamAssets`.
Stills: the off-screen engine stills you built, plus a short sequence (six frames) of the lamp and the net after a goal. The gate: the full `Jam` filter green,
`-Multiprocess` on every headless start, `jam.rink.arena 0` gives back the still arena. Time box 90 minutes; interim in the status file.
