# The P2 figure bake-off: toy_skeletal vs humanoid_b1 vs generated_b2 (lane A30, H11.9-H12.9)

Status: measurement notes, 2026-11-12 19:00-19:50 CET, worktree `a30` at main f89014c (device B's four figures merged, A26's skeletal toy figure the
default). R-0027 point 2: the shipped P2 figure is decided from this table. The lane measured and changed no figure. Stills in
`docs/experiments/img/bakeoff/`, eyes readings in `docs/experiments/eyes/a30-*.json`, proof cartridges `Game/Content/Cartridges/hockey.test.figure-{toy,b1,b2}.cartridge.json`
(`hockey.regular` plus `game.skater.visual`), status in `status/A30.md`.

## 0. The answer in five lines

1. **`humanoid_b1` for the shipped default, `generated_b2` as the alternative** (B's proposal, R-0027 point 2, confirmed by measurement). Gemini picked the B figure over
   the skeletal toy in all 12 pairs at every distance in both scenes (confidence 0.8-1.0); between the two B figures it split 4 : 2 for `generated_b2` on toy-ness (no
   cage, dot face, flat cel steps) while `humanoid_b1` took both near faceoff pairs and keeps the hockey read (cage, black pants, numbers legible at 24 m), which is why
   the lane still puts b1 first and b2 one cartridge value away; the Director decides which reading the P2 bar means (section 3).
2. **`toy_skeletal` as merged on f89014c draws the engine's default grid material**: `Game/Content/Figures/` on main lacks A26's `M_ToonFigure`, `MI_SkaterFigure_Trim` and
   `MI_SkaterFigure_Stick` (the flagged copies of B's `M_Toon`); the editor log says `Material with missing usage flag was applied to skeletal mesh /Game/Figures/SK_SkaterToy`
   288 times, and all six figures are grey (`a30-toy-play-10m.jpg`). The shipped default is therefore broken on main today, independent of this bake-off's verdict.
3. Cost separates nothing: on the same frozen frame the GPU frame is 2.8-3.0 ms for all three (BasePass 0.10-0.12 ms, ShadowDepths 0.22-0.23 ms); the game thread
   spends 40-95 us per figure per frame on each of them (six figures 0.25-0.6 ms inside a 0.9-1.6 ms world tick).
4. `Jam.Bot.VisualSwap` is green for all three values (headless `UnrealEditor-Cmd.exe` UE 5.9, `-nullrhi`, worktree build, 19:29:02-19:29:36, hash `1871c1d348fa8b41` on seed 1
   for `generated_b2`, `humanoid_b1` and `toy_skeletal`; `Game/logs/visualswap-a30.log`).
5. What each figure does best: `humanoid_b1` has the clearest hockey read (cage visor, mittens on the shaft, readable back numbers at 24 m, a real sit-down fall);
   `generated_b2` has the cheapest base pass and the softest toy look but a blank face and a mitt over its number; `toy_skeletal` has the most legible verb poses of the
   three when seen in silhouette (a flat face-down fall, a thrust poke) and a frame-counted animation path, but no materials on main, a mirrored number text and no legs.

## 1. Method (the same for all three figures)

- Build: `ThirdJamEditor Development` from the worktree, unity off, 66 s, no other UnrealBuildTool alive. One editor from the worktree on MCP port 8019 (19:07-19:31),
  stopped with `editor.ps1 -Stop` (own pid only); never more than two `UnrealEditor*.exe` on the machine.
- Cartridges: `hockey.test.figure-{toy,b1,b2}` are `hockey.regular` with `game.skater.visual` = `toy_skeletal` / `humanoid_b1` / `generated_b2` (one key added,
  id and title changed; `schema/hockey.test.mjs` preset list extended by one line; 36 schema tests pass).
- Frames: `jam.ScriptHuman 1`, `jam.Cut hockey.test.figure-X`, PIE, seed 1. **Faceoff scene**: the real faceoff, frozen at sim frame 60 (the countdown is 90 frames; the
  centres are at the dot). **Play scene**: A26's frame, frozen at 222 (A23's global time dilation 0.0001 harness; the blue skater nearest the puck is at
  (-7.06, 3.15) m and the puck at (-5.81, 5.37) m in every session, so the three figures stand on the same sim state). **Verbs**: a third session frozen at 223 (the harness
  lands within one frame of its target; 223 in all three verb sessions). The sim never sees the visual (VisualSwap hash).
- Lab camera: A23's geometry, 10 / 24 / 40 m behind the blue skater nearest the puck, pitch -17.7, FOV 70, `HighResShot 2560x1440`; the night rig as shipped.
- Verbs: the six figures nearest the puck get, by hand through the `IJamSkaterVisual` calls, `PlayPoke(true)`, `PlayLift()`, `SetHasPuck(true)` + `SetShotCharge(0.8)`,
  `SetFaceoff(true)`, `SetGoalieStance(true, -1)`, `SetKnockedDown(true, (1, 0))`. The skeletal figure counts `UpdateMotion` calls as frames, so it was advanced by 6 / 8 / 1 / 1 / 1 / 40
  calls (A26's method). B's figures are an actor attached to the skater (`JamPawns.cpp InitSkater`) driven by a delta-time AnimBP, so they were given
  `CustomTimeDilation` 10000 x {1, 1, 6, 6, 6, 6} on the skater and its visual actor for 0.13 s of wall time while the world stayed at dilation 0.0001 (the sim did not step:
  the frame read 223 before and after). Each verb is shot from 5 m, 35 degrees off the figure's facing, pitch -17.7, aimed at the chest; the six stills are tiled into one
  3 x 2 sheet per figure for the eyes `figure` preset.
- Cost: `ProfileGPU` three times at the 10 m lab camera on the frozen play frame (PIE viewport at its window size, RTX 4090; whole frame = the Scene row over its share);
  `stat dumpframe -ms=0.02` three times in a live 20 s bot match per figure (the strip session), inclusive rows of the figure's own functions; the `SKELVIS` probe lines
  for the skeletal figure.
- Clip: eight viewport captures over 20 s of the same live bot match (`CaptureGameViewport`, 1280 px, every 2.5 s, from frame ~500), tiled 4 x 2 (`a30-X-strip.jpg`), judged
  with the eyes `bug-hunt` preset.
- Eyes (R-0024 point 2; Gemini, advice not instructions): `ab-pair` on every pair of figures at 10 / 24 / 40 m in both scenes (18 calls, A = the first-named figure, one order
  per pair), `figure` on each 5 m sheet (3), `readability` at 24 m on the three play stills (1), `bug-hunt` on the three strips (3). JSON in `docs/experiments/eyes/a30-*.json`.

## 2. Stills (`docs/experiments/img/bakeoff/`, 2560 x 1440 JPEG from the PNG captures)

| | toy_skeletal | humanoid_b1 | generated_b2 |
|---|---|---|---|
| play 10 / 24 / 40 m (frame 222) | `a30-toy-play-{10,24,40}m.jpg` | `a30-b1-play-{10,24,40}m.jpg` | `a30-b2-play-{10,24,40}m.jpg` |
| faceoff 10 / 24 / 40 m (frame 60) | `a30-toy-faceoff-{10,24,40}m.jpg` | `a30-b1-faceoff-{10,24,40}m.jpg` | `a30-b2-faceoff-{10,24,40}m.jpg` |
| verbs at 5 m (frame 223) | `a30-toy-verb-{poke,lift,charge,faceoff,goalie,down}-5m.jpg` | `a30-b1-verb-*-5m.jpg` | `a30-b2-verb-*-5m.jpg` |
| verb sheet (3 x 2) | `a30-toy-verbs-sheet.png` | `a30-b1-verbs-sheet.png` | `a30-b2-verbs-sheet.png` |
| 20 s bot-match strip (4 x 2) | `a30-toy-strip.jpg` | `a30-b1-strip.jpg` | `a30-b2-strip.jpg` |

## 3. The table: eyes verdicts with confidence, and the lane's own reading

`ab-pair` (gemini-3.1-pro-preview, about $0.03 a call). The verdict names the figure Gemini found closer to the painted-cel toy hockey bar; sky, ice, pools and construction
were a tie in every call (identical frames), so the verdict is the figures row, with the outline row where it differed.

| Pair (A vs B) | Scene | 10 m | 24 m | 40 m |
|---|---|---|---|---|
| toy_skeletal vs humanoid_b1 | play | **b1** (1.0) | **b1** (1.0) | **b1** (0.95) |
| toy_skeletal vs humanoid_b1 | faceoff | **b1** (0.8) | **b1** (0.95) | **b1** (0.8) |
| toy_skeletal vs generated_b2 | play | **b2** (0.9) | **b2** (0.99) | **b2** (0.95) |
| toy_skeletal vs generated_b2 | faceoff | **b2** (0.9) | **b2** (0.95) | **b2** (0.95) |
| humanoid_b1 vs generated_b2 | play | **b2** (0.95) | **b2** (0.95) | **b2** (0.95) |
| humanoid_b1 vs generated_b2 | faceoff | **b1** (0.9) | **b1** (0.95) | **b2** (0.95) |

What Gemini said, pair by pair (the sentences, not the digits, are the reading):

- toy vs b1 and toy vs b2, every distance, both scenes: the B figure "applies the required deep blue and warm orange jersey colours" while A "uses untextured grey
  placeholders" / "dark grey silhouettes"; at 24 and 40 m against b2 the outline row also went to B ("Still A's dark grey models swallow their own outlines"). What the
  toy keeps: at 24 m "the flat cel lighting steps on the character models are slightly easier to read without saturated colours"; otherwise only the shared environment.
  These twelve verdicts measure the missing materials (section 0 point 2), not the figure's geometry; A26's own pass (FIGURE.md section 5) is the reading of the toy
  figure with its materials, and even there the `figure` preset put it at 3 / 5 as a hockey player.
- b1 vs b2, 4 : 2 for `generated_b2`. For b2 in play: "flat cel lighting steps to the jerseys and simple toy faces, whereas A uses smooth gradients and realistic
  athletic cages" (10 m); "the blue foreground figures in B retain visible yellow sticks and blue helmets, whereas A's overly thick black lines swallow these interior
  colours" (24 m); "thicker, more pronounced black outlines ... thin in the distance, whereas A's outlines are uniformly thin" (40 m, and the same at the 40 m faceoff).
  For b1 at the faceoff: "the black outlines in A make the toy figures pop" (10 m); "distinct black outlines on the players that correctly scale down ... whereas B lacks
  them entirely" (24 m). What the loser keeps: b1's "black pants and gloves provide stronger contrast against the bright ice", b1's cel steps and the puck trail; b2's
  "softer look" and its thinner stroke that "prevents the black stroke from swallowing the stick shaft and hands". Caution (EYES.md wrong 1): both figures share the same
  post-process outline on the same frame, so the outline sentences describe how each mesh takes the same ink (b2's smoother, larger surfaces hold a thicker line), not two
  outline settings; the split is really toy-ness (b2: no cage, dot face, flat steps) against hockey-ness (b1: cage, black pants, readable numbers), and the Director
  chooses which the P2 bar means. No position-bias rerun (one order per pair, budget).

`figure` on the 5 m verb sheets (does the sheet read as a hockey player and as a toy, full size and 128 px; proportions):

| Figure | Hockey player (full / 128 px) | Toy (full / 128 px) | What it said | Head / shoulders / stick of the height |
|---|---|---|---|---|
| toy_skeletal | **3 / 2** (skates: no) | 4 / 3 | "a peg gliding on a flat base"; "the uniform dark grey colour blends into its own shadows"; asks for skates, the intended blue / orange, a 50 % thicker shaft | 0.33 / 0.35 / 1.3 |
| humanoid_b1 | **4 / 3** (all five parts) | 5 / 4 | "all essential hockey equipment present in the correct places"; the top row (upright poses, big back numbers) reads best; asks for a thicker shaft (2 -> 4 px), fewer and thicker cage bars, lighter skate blades; the sliding pose breaks the silhouette | 0.35 / 0.5 / 1.0 |
| generated_b2 | **4 / 2** (all five parts) | 5 / 5 | "massive heads, stubby limbs, bright flat colours ... a plastic toy or bobblehead at all scales"; the stick vanishes when downscaled, the blade "lacks the L-shape", the vertical lift reads as a staff; asks shaft 0.02 -> 0.08 of the height, blade 0.10 -> 0.25, shoulders 0.30 -> 0.40 | 0.4 / 0.3 / 0.85 |

`readability` at 24 m on the three play stills (puck in frame; scores 1-5 for puck / carrier / team colours / far net / zone strips, overall): **toy_skeletal 4 / 3 / 1 / 5 / 4,
overall 2** ("all figures share an identical untextured dark grey mannequin appearance"; the carrier only by the stick pointing into the puck ring); **humanoid_b1 4 / 4 / 5 / 5 / 5,
overall 4** (first look: "foreground bright blue toy skaters"); **generated_b2 4 / 4 / 5 / 5 / 5, overall 4** (first look: "saturated blue helmets"). The noise named on all three is
the stepped light pools and the shadow pools between them, which belong to the rig, not the figure. b1 and b2 tie on readability.

`bug-hunt` on the three 20 s strips (flash model, severity 1-5, clean = false on all three). None of the findings is the figure's: on every strip the top finding is the
game camera inside or behind the dasher boards when play runs along the glass (severity 4, "untextured black void and sliced board geometry", twice on b2's strip) and the
bare stanchions "without panels between them" (B-to-A asks, A31 spawns the glass with them); then the bark bubble over the `MATCH 1` banner and the duplicated
instruction text (HUD, severity 2), an empty orange target box (b2 strip, 2). Figure-related: on the toy strip "a disconnected hockey stick lies orphaned on the ice" and "a
skater submerged through the ice plane" (both the grey figure's dark body against the dark ice shadow in a 1280 px capture with motion blur: the knocked-down pose and a
figure half in a shadow pool; not seen at 2560 px), "models intersecting through one another's torsos" (the faceoff scrum, all figures); on b1's strip the 5 m chase camera
fills the frame with the blue jersey (severity 3; the camera, not the figure). Nothing floats, nothing z-fights on any figure.

The lane's own reading of the same stills:

| | toy_skeletal | humanoid_b1 | generated_b2 |
|---|---|---|---|
| 10 m, play | six dark-grey checker figures (default material), the lean and the stride read, the carried puck sits on the blade; the back number is a white text whose mirrored back side floats beside the figure ("12", "13") | chunky kids with cage visors, mittens on the shaft, jersey stripes and knee pads; the blue "1" reads from behind; the blade is on the ice under the puck | the same chunk with a smoother body, round helmet, dot eyes and no cage; the stick in two hands; the orange pair in front reads as two players at once |
| 24 m, play | silhouettes with sticks; the team is unreadable (grey on grey), the text numbers are the only colour on the bodies | both teams read instantly; "1" and "3" readable on the blue backs; blades and the puck glow separate | both teams read; the numbers are smaller and the right mitt covers part of them (SK2's known point); faces vanish into a blank dot |
| 40 m, play | small dark capsules with sticks, the colour mass gone | small skaters, two colours, stick lines still visible | the same at this size; b1 and b2 are not separable at 40 m without the cage |
| faceoff (frame 60) | the six grey figures at the dot in the crouch, the near blue centre's blade hidden under the body | the blue centre crouches at the dot facing the orange centre over the red line, sticks down, the three wings in a ready stance | identical staging, the crouch a little shallower, the stance reads |
| poke, lift, charge at 5 m | the poke is a thrust with the mittens staying on the shaft (A22 pass 3), the lift raises the stick, the wind-up pulls the blade back over the shoulder; the forearm gap of A22's rig shows on the poking hand | the poke is a forward crouch with the blade along the ice, the lift takes the stick to vertical overhead (reads as a lift, over-sized), the wind-up is not visible from behind the "1" at this angle | the same takes retargeted; the poke crouch is lower, the lift the same vertical, the wind-up likewise hidden from behind |
| faceoff ready, goalie, down at 5 m | faceoff crouch and goalie block are distinct; the fall is flat on the ice face-down with the stick out: the clearest "down" of the three | faceoff crouch and goalie block distinct; the fall is a sit (legs forward, hands down), readable as down but upright | the same three takes; the sit is the same |
| 20 s strip | grey figures through a whole shift, the motion blur of the viewport captures, the HUD and bark lines unchanged; nothing floats | colours and numbers read through the clip, the carry puts the puck on the blade in motion; nothing floats | the same; one capture at 5 m from the chasing camera shows the blue "1" filling the frame cleanly |

## 4. Cost

GPU on the same frozen play frame (frame 222, lab camera 10 m, PIE viewport at its window size, RTX 4090; three `ProfileGPU` samples; whole frame = Scene row over its share; ms):

| Figure | sample 1: frame / BasePass / ShadowDepths / CustomDepth | sample 2 | sample 3 | median frame / BasePass / ShadowDepths |
|---|---|---|---|---|
| toy_skeletal (default material on main) | 3.04 / 0.118 / 0.231 / 0.010 | 2.68 / 0.122 / 0.224 / 0.011 | 3.57 / 0.124 / 0.237 / 0.010 | **3.04 / 0.122 / 0.231** |
| humanoid_b1 | 2.92 / 0.122 / 0.233 / 0.009 | 2.68 / 0.118 / 0.233 / 0.015 | 2.82 / 0.130 / 0.228 / 0.009 | **2.82 / 0.122 / 0.233** |
| generated_b2 | 2.72 / 0.100 / 0.225 / 0.012 | 2.79 / 0.102 / 0.225 / 0.013 | 3.47 / 0.097 / 0.231 / 0.013 | **2.79 / 0.100 / 0.225** |

The frame moves by 0.3-0.9 ms between samples of the same figure (post-processing and the Lumen update are the noise), so the figures are inside each other's noise; the
base pass is the only row that is stable, and there `generated_b2` is 0.02 ms cheaper than the other two. The toy figure's base pass is drawn with the default material, so its
number is not the figure's real cost (A26 measured 0.140-0.160 ms with the toon material).

Game thread, live bot match, six figures, `stat dumpframe` three times per figure (inclusive rows of the figure's own work; us per figure per frame = the six-figure row / 6):

| Figure | World tick (3 dumps) | The figure's rows (six figures, ms) | Per figure per frame |
|---|---|---|---|
| toy_skeletal | 1.09 / 1.41 / 0.90 ms | `UpdateMotion` (native: sampling, aim blend, `RefreshBoneTransforms`) 0.259 / 0.565 / 0.226; text render 0.039 / 0.036 / 0.038 | **38-94 us** (SKELVIS probe at frame 1200: 22-30 us for the five bots, 60 us for the human's figure with its text render) |
| humanoid_b1 | 1.47 / 0.99 / 1.43 ms | BP `UpdateMotion` incl. ubergraph 0.264 / 0.115 / 0.158; `SkinnedMeshComp Tick` (AnimBP) 0.164 / 0.108 / 0.165; `SK2Tick` 0.134 / 0.045 / 0.087 | **45-94 us** |
| generated_b2 | 1.58 / 1.02 / 1.07 ms | BP `UpdateMotion` 0.177 / 0.146 / 0.151; `SkinnedMeshComp Tick` 0.167 / 0.092 / 0.105; `SetStickAim` + `SK2Tick` 0.22 / 0.07 / 0.07 | **51-94 us** |

Six figures cost 0.25-0.6 ms of game thread on any of the three, under the 1 ms world tick and far under the 16.7 ms frame; the spread between dumps of one figure is larger than
the spread between figures. Nothing here decides the bake-off.

## 5. Best and worst per figure

- `humanoid_b1`: best: the hockey read at every distance (cage, mittens on the shaft, knee pads, numbers readable at 24 m, two hands on the stick in every take, the puck on
  the blade in motion); the richest original motion (31 takes). Worst: the lift take goes to vertical overhead (reads as a celebration at a glance), the fall is a sit rather
  than a sprawl, no far-reach aim holds at the sides yet (SK2 fault 1, B is adding the grid), the charged wind-up is only seen from the side.
- `generated_b2`: best: the cheapest base pass, the softest toy proportions, the same nine calls; the faces are plain so nothing reads wrong at 40 m. Worst: the blank
  dot face and no cage at 10 m (the figure reads as a doll, not a hockey player), the right mitt over the back number (B12), otherwise b1's motion faults, retargeted.
- `toy_skeletal`: best: the frame-counted drive (no delta time anywhere: the same pose at any frame rate), the flat face-down fall, the thrust poke, the stride and lean
  authored into the cycle, 11-15 bones. Worst: on main it has no materials (section 0 point 2), the number is a two-sided text whose back side shows beside the figure, no
  legs (the skates are a block), A22's forearm gap at 5 m; and with A26's materials restored it still read 3 / 5 as a hockey player in A26's own `figure` call.

## 6. Recommendation

Ship **`humanoid_b1`** as the default (`game.skater.visual` absent = `humanoid_b1` in `JamCartridge.h`, `schema/DEFAULTS.md`, `schema/README.md`, the schema description,
`JamCoreTests` "absent visual" assertion, regenerate `JamSchemaData.inl` and `parity.json`: the same five one-liners A26 touched for `toy_skeletal`, one lane); keep
**`generated_b2`** as the alternative the operator can pick in a cartridge, and keep `toy_skeletal` and `toy` as values under VisualSwap. Before any later reading of the toy
figure, the three material assets from A26's worktree (`M_ToonFigure`, `MI_SkaterFigure_Trim`, `MI_SkaterFigure_Stick`) must reach `Game/Content/Figures/` on main; until then
the default on f89014c draws grey figures in every cut that leaves the key out, which is every shipped cut.

## 7. Files

- new (worktree a30): `docs/experiments/FIGURE-BAKEOFF.md`, `docs/experiments/img/bakeoff/*` (42: 18 lab stills, 18 verb stills, 3 sheets, 3 strips),
  `docs/experiments/eyes/a30-*.json` (25), `Game/Content/Cartridges/hockey.test.figure-{toy,b1,b2}.cartridge.json`.
- one line: `schema/hockey.test.mjs` (the preset list; outside the listed scope, as A26's line was).
- logs (worktree, not for the tree): `Game/logs/editor-20261112-190736-0.log` (ProfileGPU, dumpframe, SKELVIS), `Game/logs/visualswap-a30.log`.
- main tree: `status/A30.md`.
- untouched: every figure, `Game/Plugins/JamAssets`, the contracts, the sim.

## 8. Errors met

- B's figures are actors attached to the skater, not a `SkaterVisual` component (`InitSkater` takes both): the first verbs pass on `humanoid_b1` found no visual and was rerun.
- B's AnimBPs take delta time, so a frozen world shows the first frame of a forced montage; per-actor `CustomTimeDilation` for a fraction of a second gives them time while the
  sim stays frozen (the method above). The skeletal figure needs the opposite: explicit `UpdateMotion` calls.
- The MCP `CaptureGameViewport --save-images` needs a Windows path (the first toy strip saved nothing and was rerun); `py.mjs` results come back as escaped JSON and a Python
  script whose last statement sits inside `if/else` returns `""`; the editor log must be read as UTF-8 for the ProfileGPU box characters; `stat dumpframe` rows nest, so the
  inclusive `UpdateMotion` row already contains the ubergraph and `PushAnim` rows under it.
- The time-dilation harness lands on the target frame or one after it (222 in the play sessions, 223 in the verb sessions): the sessions are consistent with each other,
  which is what the comparison needs, but a reader of A26's "frame 223" should know both numbers are the same sim state one frame apart.
