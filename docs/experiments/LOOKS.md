# Three looks of A's the trailer shows: the day trim, Glow Ice's penalty boxes, Fog of War's fog (lane A223, track L1, Sunday 15 November)

Device B's look lane filled the day rig and named three faults in A's own parts at the game camera (`handoff/B-to-A-0046.md`). Each was looked at first
on this tree (the game camera, off screen, 1920 x 1080 and 1280 x 720, the cartridge's seeded match played by the person-like hand), then cured by one
variable whose neutral value gives back the picture of before. All three are presentation: no sim file is touched and the sim's hashes are equal.

## What was seen, and the cure

| Look | Seen before | Cause | Cure (default) | Neutral (the picture of before) |
|---|---|---|---|---|
| Pond, Outnumbered (`l1_day`) | The trim on top of the low boards is a light strip: pale yellow on the orange half, pale cyan on the blue half, a halo against white ice | Trim Emissive 30 in daylight (`LookTierSwitcher.cpp`, `GTierTrim`) | `jam.look.TrimDay 10`: the team's colour painted on the boards (amber, blue), no halo | `jam.look.TrimDay 30` |
| Glow Ice (`l3_neon`) | The two penalty boxes are cream slabs on the dark sheet, the biggest and brightest thing in the frame while he sits in one (the camera stands at the box) | The grid arena (lane A197) darkens the ice and the boards; lane A160 stood the boxes on this tier afterwards with their daylight materials | `jam.look.NeonBoxes 1`: the boxes' `board` (the clock's face is part of it), `bench` and `kick` slots take the dark sheet's material; the team's seats, the door's panel and the lit boxed figure stay | `jam.look.NeonBoxes 0` |
| Fog of War (`l2_night`, `look.fog_density` 0.04) | No fog: the far boards, the far goal and the stands 80 m away as crisp as the near ones | `look.fog_density` was parsed into `FJamLook::FogDensity` and nothing read it; the night rig has no fog actor (the contract: fog is L3 only) | `jam.look.FogScale 8`: one height fog owned by `ULookTierSwitcher`, density `look.fog_density` x 8 = 0.32, from 10 m (`FogStartM`), at most 0.9 (`FogMax`), a pale cold blue (0.48, 0.63, 0.90) x `FogBright` 1 | `jam.look.FogScale 0` |

`jam.look.FogFrom 0.03` is the smallest `look.fog_density` that draws: `hockey.boost` and the ladder's neon takes ask 0.02 and keep the picture they had
(the brief: no other look changes). A rig that brings a height fog of its own (`LR_L3_Neon`) keeps it and A draws none: one height fog a scene.
`jam.look.TrimDay` and the `jam.look.Fog*` variables changed at the console are pushed to the live world at once (a console sink), so a value can be judged
by looking without a restart; `jam.look.NeonBoxes` is read when the grid arena is drawn (a rink build or a tier change).

## How the fog's numbers were found (by looking, `tools/qa/eye3/out/t1-fog` .. `t3-fog`)

| Tried | What the film showed |
|---|---|
| colour (0.16, 0.21, 0.30), scale 8, from 10 m | The far boards and the hall take the colour, and the colour is darker than the lit ice: it reads as the hall going dark, not as fog |
| colour x 3 (0.48, 0.63, 0.90), scale 8, from 10 m | **Fog with depth**: the goal 36 m away pale, the far boards and the stands a shape in mist; the opponent 10 m away, his own skater, the puck and the rings crisp. Shipped |
| colour x 4, scale 12, from 8 m | The nearest opponent hazes and the ice loses its blue |
| colour x 6, scale 10, from 12 m, at most 0.85 | The ice washes white |

## The method (`tools/qa/eye3/`)

`eye3-run.ps1` is lane A219's `eye2-run.ps1` with `-Every N -Frames F`: `jam.screen.StillEvery` is set on the command line, so the stills are of the same
sim frames (N, 2N, ...) on every start of one sim, on the editor binary (`-Editor`) or a packaged Development exe of this worktree (`-Exe`).
`looks.py sheet | ab | lum` makes a sheet of chosen frames, a before / after sheet of the same frames, or prints a still's mean luminance.
**One editor film at a time**: every `-Editor` process writes its stills to the one folder `Game/Saved/Screenshots/WindowsEditor`, and two at once take
each other's stills (it happened once here; the run was filmed again). A packaged exe has its own folder and may run beside an editor film.
The neutral film of a look is the same binary with `-ExtraCVars jam.look.TrimDay=30,jam.look.FogScale=0,jam.look.NeonBoxes=0`.

## Proof (sheets in `docs/experiments/looks/`)

- `final-pond-before-after.jpg`, `final-glow-before-after.jpg`, `final-fog-before-after.jpg`: the cooked Development package of the lane's tree, the same binary
  with the three neutral values above its defaults, the same sim frames, 1920 x 1080 above 1280 x 720 (the Pond sheet ends with the trim at full size).
- `unchanged-clay-and-night-1080.jpg`: clay PLAY NOW before and after the lane, and the night look without fog (`hockey.boost`) neutral against default.
- The sim's hashes (one line every 600 frames, 60 s of the seeded match with the person-like hand): equal before / after in every pair of one binary: the
  editor binary at 1080 (source before the lane against after) and at 720 (neutral against default), the package at both sizes; 6 of 6 lines per pair, 12 pairs.
- `seen-*.jpg`: what was seen before a line was written; `forb-*.jpg`: two things of device B's found on the way.

Found on the way, not cured here: the editor binary and the cooked package of one tree play DIFFERENT seeded matches in clay PLAY NOW, Glow Ice and Fog of
War (every hash line from frame 600 differs) and the same match in Pond and Shotgun; package against package is equal. Cut a before / after pair from one binary.

## Tests

`Jam.Look.TrimDay` (10 on `l1_day`, the other four tiers as they were, 30 gives back the day of before) and `Jam.Look.FogKey` (0.04 draws 0.32, 0 and 0.02
draw none, the schema's top value 0.1 stays inside the engine's clamp, `FogScale 0` draws none anywhere): `Game/Source/JamGame/Private/LookTests.cpp`.
`Jam.Rink.NeonArena` gains the boxes: 6 slots dark on Glow Ice's rink, 0 with `jam.look.NeonBoxes 0`, 0 after a switch to clay.
