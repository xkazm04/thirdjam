# The screen, part 5: nothing is drawn on top of the key feature (lane A158, track U1, evolution 5, Sat 14 Nov 03:25 to 05:05)

Parts 1 to 4 are in `SCREEN.md`. The operator named the fall, the lying and the get-up the key feature; device B's critic (`handoff/B-to-A-0041.md`,
`status/EYE-report.md` on `origin/dev-b`) found the HUD drawn over them: the ring across the lying body, the caption on it, a bubble over the middle ice, and
the camera looking straight down in corner pile-ups. This part takes the HUD off the downed figure and measures it; the camera's part is a soft cap with its
limits stated. Presentation only: no sim state, no cartridge key, the lockstep hash untouched (`hockey.regular` seed 1: `d1d00f4e973366cc`, bare
`d1554c0c7d967574`, the same in main's own gate log of the commit and in this lane's gate runs).

## What changed, and the neutral value of each

| Rule | Where | Neutral value (the screen of before) |
|---|---|---|
| **No ring under the human while he is down.** The selection ring is not drawn from the first down frame; it comes back over 12 sim frames from the frame the sim releases him (the get-up ends there). The chevron on the helmet stays. | `UJamHudWidget::RingDownAlpha`, `UpdateYours` | `jam.hud.ClearDown 0` |
| **The human's ring does not lie on a downed figure beside him** (he stands over the man he felled): it fades out over 6 frames while its box meets a downed figure's box, and back the same way. | `RingYieldStep`, `StepRingYield` | `jam.hud.ClearDown 0` |
| **No marker on a downed figure**: the carrier's ring, the net badges with their labels and arrows, the puck arrow with its label are not drawn on a frame their box (plus 16 px: the label's plate, a turned chevron) meets a downed figure's box. | `YieldToDowned` (`HUDClearDown.cpp`) | `jam.hud.ClearDown 0` |
| **The caption never lies on the human, up or down, nor on a downed figure.** From its dock on the bottom edge it goes to the slot under the scoreboard (under the overlay's line and the one-time prompt while they show) when a kept box is under the dock; it keeps the place it took until its line ends (no hopping); with both places taken it is not drawn that frame (the voice plays on). | `CaptionPlace`, `UpdateCommentary` | `jam.hud.ClearDown 0` |
| **The contact line steps sideways** along its own row (192 px over the bottom edge) to the nearest place clear of the human and of a downed figure, between the left margin and the ability plate; it stays where it stepped to. Placement only: its words and its timing are track F1's and untouched. | `SideStep`, `PlaceContactLine` | `jam.hud.ClearDown 0` |
| **The bark lane yields to the same boxes and to both nets** (a bubble over a net was the critic's fault 3; the lane already took the bubbles off the middle ice in part 3). | `UJamBarkWidget::Layout` reads `UJamHudWidget::GetKeepClear` | `jam.hud.ClearDown 0` |
| **The own-net marker is not drawn on a net that is in the frame** (the diamond and "own NN m" stood on the goalie and the net: the critic's fault 16); with the net out of the frame it is the edge chevron with its distance, as before. | `ShowOwnMarker`, `UpdateOwnMarker` | `jam.hud.OwnMarkOnNet 1` |
| **The camera's pitch cap** (soft, 55 degrees): see "The camera in corners" below. | `FJamCameraSettings::PitchCapDeg`, `PitchCapDistM`, `KeepHumanInFrame`; `jam.camera.PitchCap`, `jam.camera.PitchCapDist` | `jam.camera.PitchCap 0` |

**A figure's box** is its bones projected onto the screen plus 14 cm at the figure's depth (`FigureBoxOnScreen`: device B's figures are skinned; a figure
without a skeleton gives the box of its meshes' bounds). The bones follow the fall, the slide and the get-up, and they include the stick: the box is the
lying figure, not the 1.8 m post over his feet that the lane's old rule used. Each tick the HUD computes the human's box and every downed skater's
(`UpdateKeepClear`); "down" is the sim's own flag (`FJamSkaterState::IsDown`, read, never written).

No state field was added to `FJamSkaterState` or `FJamPuckState`.

## Proof

**The instrument.** `jam.hud.DownProbe N` (in the game, `HUDClearDown.cpp`): once a sim frame seen, for every downed skater, the figure's box of the last
tick is intersected with the painted box (the cached geometry of the last paint, render transform included) of every text, image, plate and nested widget of
every widget in the viewport that is shown all the way up its tree (the HUD, the bark widget, the Director's Call widget); a full-screen layer is not an
element. One `JAMSCREEN down` line a knockdown: the frames seen, the frames with any element over the box, by element. The human's chevron and YOU tag are
counted apart (the chevron stays, by the brief: it was over his box on every down frame). Driver: `tools/qa/screen/u5-downs.ps1`, one editor-binary game
process a seed, `-RenderOffScreen` (no window, no cursor), 1280 x 720, clay, 90 m, five a side, pro, the scripted human; 135 s a seed.

**The bar (20 captured knockdowns on clay, no HUD element over the downed figure's screen bounds for the length of the knockdown): met.**

| Run (build 3: the HUD rules of the closed state) | Knockdowns of the human | With no element over him on any frame | Frames seen down | Frames with an element over him |
|---|---|---|---|---|
| rule on, seeds 2, 4, 5, 6 (`out/downs3-after-a`) | 11 | 11 | 1,195 | 0 |
| rule on, seeds 7, 8, 9, 10 (`out/downs3-after-b`) | 12 | 12 | 1,237 | 0 |
| **rule on, eight seeds** | **23** | **23** | **2,432** | **0** |
| rule on, seeds 5, 10, 6 again on build 5 (the closed state's own binary, the camera cap on; `out/downs5-after`) | 15 | 15 | 1,578 | 0 |
| rule off (`jam.hud.ClearDown 0`), seed 123, build 2 (`out/downs-clay-before`) | 4 | 0 | 430 | 430 (the ring 430, the puck arrow 36, the carrier's ring 11, the caption 2) |
| rule on, seed 123, build 2: the same four knockdowns (`out/downs-clay-after`) | 4 | 4 | 430 | 0 |
| rule off, seed 1, build 3 (`out/ev5-std-before`) | 1 | 0 | 137 | 137 (the ring 137, the carrier's ring 21) |

- The caption on the human while he is up (the same eight seeds, rule on): 0 of 40,492 frames.
- Rule off and rule on play the same match: seed 1's `JAMSIM` hashes at sim frames 600, 1200, 1800 are `8e0fd36aeea524e2`, `02b5afd69a5fa672`,
  `ebe03901d77c82a3` in both runs (the HUD reads the sim and writes nothing back).
- Build 2 (the markers' pad at 6 px) had one frame in 948 with the puck arrow's plate on his box (seed 2, the knockdown at sim frame 280); the pad is 16 px
  since build 3 and that knockdown is clean.
- **Other skaters' knockdowns** (not the bar; reported): 6 in those eight seeds, 558 frames seen, an element over the downed figure on 158: the human's
  chevron 72 (he stands over the man; the chevron stays), his ring 40 (its 6-frame fade-out; it lay there for 71 of 136 frames in build 1), the Director's
  Call widget's plate 31 (fixed furniture of another widget, not this track's file), the scoreboard 16 and the one-time prompt 8 (fixed places at the top
  edge; the downed skater lay under them from the crane's view).
- **Gate**: the full `Jam` filter, headless: 241 of 241 Success on builds 1 to 5, the last on the closed state (`Game/logs/u1-automation-ev5-run1..5.log`): main's 240 +
  `Jam.Hud.ClearDown` (the ring's two rules, the caption's place, the contact line's step, the own-net marker, boxes). `Jam.Menu.FitsTheScreen` and the
  HUD's tests green. Hash before = after: `hockey.regular seed 1 ... the tree as it stands: 8301 frames, hash d1d00f4e973366cc` and `bare
  d1554c0c7d967574 after 6000 frames` in main's own premerge log of e9d0b55 (`premerge-automation-ue-20261114-030901.log`) and in all five runs.

**Stills** (`docs/experiments/img/a158/`; all taken by the game off screen, read by the lane):

| Still | What it shows |
|---|---|
| `down-seed123-f01429-before-after.jpg` | the same knockdown, 25 frames in. Before: the ring across the lying body, the caption under his hand. After: no ring, the caption under the prompt at the top, the chevron on the helmet. |
| `down-seed123-f02661-before-after.jpg` | a knockdown at the red line with an opponent standing over him: before the ring across both, after nothing on him. |
| `downs-after-12-stills.jpg` | twelve stills of seven knockdowns (seeds 2, 4, 5), rule on: open ice, the side board from the crane, a pile at the red line. |
| `own-marker-f01680-before-after.jpg` | the own-net marker: before, "own 63 m" and its diamond stand on the net; after, the net is there to see. |
| `puck-post-at-faceoff-f00960.jpg` | the "black post" (not fixed here: see the faults table). |

**Not proven off screen** (for the window lane): the contact line's sideways step (the scripted human triggers no contact line in these runs: the rule is
proven by `Jam.Hud.ClearDown`'s cases only); a hand's eye on the ring coming back at the get-up (12 frames); the caption's two places at 1920 x 1080 and at
a 4:3 window (the boxes are computed in Slate units, the stills are 1280 x 720).

## The camera in corners

**Verdict: a trim, not a cure. The cap ships on at 55 degrees (`jam.camera.PitchCap 55`; 0 = the camera of before); it holds its three assertions and
makes no number worse, but a corner pile-up is still seen from steeply above on most of the frames it was.**

What the critic saw is real on this main: in the standard match (seed 1, clay, sim frames 0..5999, `jam.camera.PitchCap 0`) the camera looks down steeper
than 60 degrees on 84 of 5,989 frames on 90 m (steeper than 70: 13; steepest 72.1; the camera 7.47 m up) and on 50 of 5,965 on 140 m (16; 73.0).

**The rule** (`KeepHumanInFrame`, height and pitch alone): where the boards take the camera's arm, the crane and the rig's own lifts give no more height
than the cap's pitch needs to hold the chest (the pitch may sit the middle third's 6.7 degrees over the chest's bearing), never less than keeps the
camera `PitchCapDistM` (5 m: the track's "the figure does not fill the frame") from the chest, never under the open-ice height; the pitch the stage asks
for is no steeper than the cap. The crane's own rate limit (0.2 m a frame) eases it. The yaw and the camera's foot are not touched.

**Measured in the real match** (build 4, the cap's final code; the same match with `jam.camera.PitchCap=0` and at the default 55; paired on the sim frames
both logs carry; `tools/qa/screen/u5-pitch.py`; runs `out/cap4-off-90`, `cap4-on-90`, `cap4-off-140`, `cap4-on-140`):

| Rink | Frames compared | Steeper than 60 deg | Steeper than 70 deg | Steepest | Highest camera | Chest outside the middle third | Frames whose yaw differs | Largest step a frame (pitch, height) |
|---|---|---|---|---|---|---|---|---|
| 90 m | 5,676 | 84 -> 71 | 13 -> 7 | 72.1 -> 71.4 | 7.47 -> 6.85 m | 28 -> 28 | 0 | 11.1 deg, 0.20 m -> the same |
| 140 m | 5,875 | 50 -> 50 | 16 -> 14 | 73.0 -> 73.0 | 6.92 -> 6.69 m | 31 -> 31 | 0 | 10.0 deg, 0.20 m -> the same |

The human is at the same place on every compared frame (the camera feeds nothing back into the scripted human; a hand's path cannot move either: the yaw
is the yaw without the cap on every frame). Where the cap bites it does what it says: sim frames 2297 to 2301 on 90 m: 64.7 -> 58.2 ... 61.7 -> 55.0
degrees, the camera 7.2 -> 5.4 m.

**The model** (`Jam.Camera.KeepsHuman`, 6,000 frames a rink, cap 55; the three assertions: no more frames steeper than 60 degrees than without the cap, he
is held as without it, no snap): 90 m: steeper than 60 degrees 280 -> 260, than 70: 243 -> 240, highest 7.54 -> 6.50 m; 140 m: 12 -> 6, 0 -> 0, steepest
65.9 -> 62.9; 120 m: 9 -> 9. Yaw not the yaw without the cap on 0 frames; chest outside the middle third 13 -> 13, 0 -> 0, 7 -> 7, never outside the frame;
largest step a frame 98 cm and 9.65 degrees, as without. The settings' default is 0 (the tests' rig and the neutral value).

**Why it is only a trim (measured, not guessed).** (1) The model's steep frames are his back at the glass: the camera's foot stands at the boards' inset
and he is under or behind it; no height changes that pitch. (2) In the corner of `pair-cap` (sim frame 2280: the same pose with and without the cap, 6.46 m
up, pitch 62.3) the stage that keeps him in the frame has already brought the foot to 2.07 m behind him; the cap reads the arm from the rig's foot, before
that stage moves it, so it sees more arm than there is and does not bite. Reading the arm after the stage (the next evolution) would give 58 degrees from
5.4 m there, and no more: with 2 m of arm and 5 m to the chest the pitch is 58 degrees; a view under 50 degrees from 5 m needs 3.2 m of arm. A corner does
not have that arm unless the camera turns (ruled out: 322e6c5, a turned camera turns the cursor-aimed skater) or crosses the boards (the arena behind
the boards would have to be safe for a camera: device B's).

Stills: `corner-f02280-cap-off-on.jpg` (the corner the cap does not change), `downs-after-12-stills.jpg` tile a5-f05133 (the side board from the crane).

**Not proven off screen**: how the lower camera feels in motion at a board (the stills are every 120 frames; no snap is the model's and the log's
statement, not an eye's). One number gives the camera of before: `jam.camera.PitchCap 0`.

## The critic's eleven A-side faults on this main

| # | Fault (EYE report) | On this main (e9d0b55) | After this evolution |
|---|---|---|---|
| 1 | the ring over the human, across the body when he is down | present (430 of 430 down frames) | gone while he is down and beside a downed figure (0 of 2,432 frames). Standing, the ring is still a 2D ellipse drawn over his skates and shins: a ring the figure hides needs a mesh on the ice (next evolutions) |
| 2 | the caption on the human | already docked on the bottom edge (lane A111) but it met his box on 2 of 430 down frames | never on him, up or down: 0 of 40,492 + 2,432 frames |
| 3 | taunt bubbles over the net and the middle ice | already gone: the fixed lane beside the score plates (lane A135) | the lane also yields to both nets and to a lying figure's real box |
| 4 | the empty top-centre panel with the period text on its border | already gone: "PERIOD n" over "FIRST TO n" inside the plate (lane A123), in every still | unchanged |
| 5 | an opponent stands in a downed body | not this track's (the sim or the brain); nothing on this main names it | unchanged; still 2 of this part shows it (seed 123, sim frame 2661) |
| 9 | the goal light (shared with B) | not this track's | unchanged |
| 11 | the camera straight down in corners | present: steeper than 60 degrees on 84 of 5,989 frames (90 m), 50 of 5,965 (140 m) | trimmed, not cured: 84 -> 71 and 50 -> 50 (steeper than 70: 13 -> 7, 16 -> 14); see "The camera in corners" |
| 14 | black clock faces on the penalty boxes on clay | not checked: no still of this lane shows the boxes at reading size (the pieces are track F3's) | unchanged |
| 16 | distance markers on the goalie and the net | present for the own net (`own-marker-f01680`); the far-net badge stands over the crossbar by lane A76's rule | the own-net marker is not drawn on a net in the frame |
| 19 | "the lifted puck as a black post" | present, and not a lifted puck: a thin black post stands on the puck AT REST on the faceoff dot (sim frames 960, 3120); a puck in the air is a black dot over its lit disc. About the trail ribbon's width (9 cm; `NS_puck_trail`, "held on a near-zero time step" in ASSETS.md): a vertical ribbon frozen by its own hold rule after the drop | not fixed: `Game/Source/JamCore/Private/JamPawns.cpp` (AJamPuck spawns the trail, line 730) and the asset are outside this track; a finding for the puck's owner and device B |
| 20 | the bare rink in the first frames of a match | not re-shot by this lane (lane A111's light hold and 2250116 are on this main; device B checks it in its next play-through) | unchanged |

## How to repeat (part 5)

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\build.ps1 -Project <worktree>\Game
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-gate.ps1 -Tag ev5                      # the full Jam filter, headless, -Multiprocess
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after  -Seeds "2,4,5,6"   # off screen, one process a seed, 135 s
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode before -Seeds "123"       # jam.hud.ClearDown 0, jam.hud.OwnMarkOnNet 1
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds "1" -StillEvery 120 -StillAt 0 -CamLog -RunName cap-on
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds "1" -StillAt 0 -CamLog -ExtraCVars "jam.camera.PitchCap=0" -RunName cap-off
python tools\qa\screen\u5-pitch.py tools\qa\screen\out\cap-off\seed1\game.log tools\qa\screen\out\cap-on\seed1\game.log
```

`u5-downs.ps1` writes `tools\qa\screen\out\<run>\summary.txt` (the knockdowns, those with no element over the figure, the frames hit by element) and, per
seed, `downs.txt` (one `JAMSCREEN down` line a knockdown), `sim-lines.txt` and `shots\` (two stills of every knockdown of the human, 25 and 75 frames in).
The probe is in the game: `jam.hud.DownProbe N` (lines every N sim frames and one per knockdown), `jam.hud.DownStills N`, `jam.hud.DownStillTag <tag>`.
A match ends at the third goal and the end card stays up: more seeds give more knockdowns, more seconds do not. One process takes about 3 GB.
