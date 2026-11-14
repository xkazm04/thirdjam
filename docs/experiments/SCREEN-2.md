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
| `puck-post-at-faceoff-f00960.jpg` | the "black post" as this part first saw it (fixed by lane A168: `post-at-faceoff-f00070-before-after.jpg`). |

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
| 19 | "the lifted puck as a black post" | present at every centre-ice faceoff hold: not a lifted puck and not the trail (this part first guessed the trail: wrong) but the upright far-puck marker inside device B's disc mesh `/JamAssets/Props/SM_PuckDisc` | gone by default (lane A168, `jam.puck.Post 0`): see "The black post at the faceoff" below |
| 20 | the bare rink in the first frames of a match | not re-shot by this lane (lane A111's light hold and 2250116 are on this main; device B checks it in its next play-through) | unchanged |

## The black post at the faceoff (lane A168, Sat 14 Nov 05:18 to 05:35)

**What it is.** Device B's mesh `/JamAssets/Props/SM_PuckDisc` (the puck's disc: actor `JamPuck_0`, class `AJamPuck`, component `PuckDisc`, created in
`AJamPuck::InitPuck`, `Game/Source/JamCore/Private/JamPawns.cpp`). The mesh is not only the halo disc of its ledger row ("60 cm x 4 mm"): in the game its
bounds are 70 cm across and 45 cm tall, and the ledger lists four material slots on it: Halo (`MI_Trim_PuckHalo`), Beacon (`MI_Fx_PuckBeacon`), Ring
(`MI_Fx_PuckRing`, FadeStart 0) and Marker (`MI_Fx_PuckMarker`, FadeStart 1200 cm). The upright part is a far-puck marker that its material fades in when
the camera is more than 12 m from the puck. At the centre-ice faceoff the camera stands 13.5 m behind the dot, so the marker stood on the puck through
every HOLD; in play the puck is near the human, inside 12 m, and it is faded out. On the clay tier the puck's kit does not emit, so it read as a black post.

**How it was found.** `jam.hud.NearPuck 1` (`HUDClearDown.cpp`): at each faceoff hold, and 240 frames after each drop, one `JAMSCREEN nearpuck` line per
visible primitive component within 1.5 m of the puck (class, owner, level, mesh path, place, scale, bounds, first material) and a still. At the opening
hold (sim frame 70) two meshes stand there, both the puck actor's own: the puck (Engine Cylinder, scale 0.24 x 0.24 x 0.08: flat) and `PuckDisc` with
`bounds_centre=(-1.9,-1.5,22.8) half_extent=(35.0,35.0,22.6)`. No rink prop, no rig actor, no figure part.

**The fix** (device B's plugin untouched): `jam.puck.Post` 0 (default): the disc component's world scale is (1, 1, 0.01) (`AJamPuck::PlaceDiscOnIce`,
live), so the disc keeps its place and its width (the puck's ring on the ice) and the 45 cm of upright geometry become 4.5 mm under the puck.
`jam.puck.Post 1` = the marker upright, as before.

**Proof.** The game's own line `JAMPUCK disc post=flat scale_z=0.01 half_height_cm 22.60 -> 0.23 (jam.puck.Post 0)`; the dump at the holds of sim frames
70 and 974: `comp=PuckDisc scale=(1.000,1.000,0.010) ... half_extent=(35.0,35.0,0.2)`; with `jam.puck.Post=1` the same match gives `half_extent
(35.0,35.0,22.6)` and the post (`JAMSIM frame=600 hash=8e0fd36aeea524e2` in both). Still: `post-at-faceoff-f00070-before-after.jpg` (the opening hold,
the puck enlarged under each frame). The full `Jam` filter: 242 of 242 (`Game/logs/u1-automation-post-run1.log`; the new test `Jam.Hud.PuckPostFlat`
holds the default). Runs: `tools/qa/screen/out/post-find`, `post-before`, `post-after`.

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds "1" -StillAt 0 -PlaySeconds 26 -ExtraCVars "jam.hud.NearPuck=1" -RunName post-after
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds "1" -StillAt 0 -PlaySeconds 26 -ExtraCVars "jam.hud.NearPuck=1,jam.puck.Post=1" -RunName post-before
```
(The driver prints FAIL for these two: it wants a knockdown, and 26 seconds have none. The lines and the stills are what is read.)

**For device B**: the marker is yours to redo or to drop (`SM_PuckDisc`'s Beacon and Marker slots; `M_Fx_PuckMark`'s FadeStart): on clay it needs a look
that reads as a marker, not a pin, and it should not stand on a puck the player is looking straight at. Until then A keeps it flat. Not measured here:
what a puck far from the camera in play looked like with the marker (it was meant for that case; the HUD's puck arrow covers a puck outside the frame).

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

# Part 6: two marks, two shapes; less of them; Space and F on the first strip (lane A171, track U1, evolution 6, Sat 14 Nov 06:01 to 07:10)

The design review of the package's commit (`docs/design/REVIEW-3.md`, items 1 and 8) put two screen faults at the top of what the operator feels:
"orange rings keep appearing and I do not know which one wants what", and a first strip that names neither Space nor F. Presentation only: the default
match's hash is the hash of before (`hockey.regular seed 1 ... the tree as it stands: 5712 frames, hash a62c05bf23232586`, `bare eec9827f89f90f64 after
6000 frames`: in main's own gate logs of 05:52 and 06:02, in this lane's gate on the tree before the change, and in its gate after).

## What changed, and the way back for each

| Change | Where | The way back |
|---|---|---|
| **A. The stop mark is a line, never a ring.** A bar on the ice across the human's path at the mark's place (8 m from him towards the closing opponent), perpendicular to his travel, 3.2 m wide (two stick lengths); armed: chalk at 0.6, 16 cm thick (never under 4 px); lit: the defender's colour, 40 cm thick and breathing (never under 9 px); white with an ink edge so chalk reads on pale ice. The contested puck keeps its ring. A ring says "this puck"; a line across the way says "stop here". | `HUDMarks.cpp` (`UpdateMarks`, `JamMakeMarkBar`), `UJamHudWidget::StopBarLayout`, `StopMarkBar` | none shipped (the ring's code is gone); `jam.hud.Marks 0` = no marks at all, as before |
| **B1. No contested ring for 60 frames after a faceoff's drop.** | `JamCarrierMarks::FContestRule::AfterDropFrames` (60), `UJamSim::GetDropFrame` (a getter: no state) | `AfterDropFrames = 0` |
| **B2. The rule's thresholds are the review's**: the two close at 12 m/s or more for the contested ring (was 8), at 8 m/s or more for the stop mark (was 3). | `FContestRule::ClosingMps`, `FStopRule::ClosingMps` | 8 and 3; per run: `JAM_MARK_RULE="closing=8;stopclosing=3"` |
| (main's own change, taken over so this build equals main's rule: the stop mark lights from 9.5 m, was 11) | `FStopRule::LitFarM` | main's |
| **C. The first hint strip names the poke and the ability**: `SKATE WASD  BRAKE PRESS AGAINST YOUR MOTION  AIM MOUSE  SHOOT HOLD LMB  PASS Q  POKE SPACE  CHECK RMB  ABILITY F  SPRINT SHIFT`: nine verbs, three spaces between them (it was four), 143 characters (the strip's width and type were sized for the 146 of the old ten-verb line). | `UJamHudWidget::ControlsHintText` | the old string |
| **D. One down rule for the marks.** `UpdateMarks` no longer builds boxes of its own round downed skaters: it reads lane A158's `DownBoxesNow` (every downed figure by its projected bones, made at the top of the same refresh) with the markers' pad (16 px); the line is tested by the box it covers once turned. And the rings of lane A163's take grace (drawn at up to 1.5 x their size after the HUD's own yield had run) are tested again at the size drawn. | `HUDMarks.cpp`, `MarkDownPadPx`; `HUDClearDown.cpp` (`YieldOverlayToDowned`) | `jam.hud.ClearDown 0` for the rings |

## Proof

- **Gate**: the full `Jam` filter, headless: **267 of 267** on the closed state's build (`Game/logs/u1-automation-ev6-run3.log`): main's 265 +
  `Jam.Hud.TooSoon` + `Jam.Hud.ClearEdges` (the line's layout: length, thickness across itself, the minimum, the turn, the box). `Jam.Core.Movement.Marks`
  (lane A166's rule test) is green with one scene added: head-on in the second after a drop is not contested; with `AfterDropFrames 0` it is; the old
  scenes are staged a second after the drop. `Jam.Menu.FitsTheScreen` green. Hashes as above.
- **Shipping compiles**: `tools/run/build.ps1 -Config Shipping` on the same source: `ThirdJam Shipping in 694.2s`, verdict pass (`Game/logs/build-ThirdJam-Shipping-20261114-062808.log`).
- **D, the bar: no mark over a downed figure.** Off screen, clay, 90 m, pro, 1280 x 720, eight seeds (2, 4, 5, 6, 7, 8, 9, 10; `out/ev6b-downs-a`, `-b`):
  the human down **26 times, 26 with no HUD element over his box on any frame** (2,715 frames seen, 0 hit); other skaters down 7 times (613 frames): his
  chevron on 80 frames, the Director's Call plate 31, the scoreboard 14, the prompt 8, an unnamed plate 4; **the contested ring and the stop line over a
  downed figure: 0 frames of 33 knockdowns** (in those matches the ring showed 49 times and the line lit 36 times; four showings were withheld, `shown=0`).
  The carrier's ring lay on the downed human for 11 frames of one knockdown before the rings' second test (build 1); 0 after.
- **A, stills** (`docs/experiments/img/a171/`): `stop-line-lit-and-hint-strip-f000131.jpg` (the line lit, orange, at the left; the human's ring yellow in
  its take grace; the strip under him), `contested-ring-f002391.jpg`, `ring-and-line-side-by-side.jpg`, `marks-11-stills.jpg` (eight rings, three lines).
  **Both marks in one frame cannot happen**: the ring needs a loose puck (`CarrierIndex() < 0`), the line needs the human carrying it
  (`CarrierIndex() == Me`): the side-by-side is two frames. No still of the armed (chalk) line: with the stop's closing at 8 m/s an armed phase lasts at
  most 11 frames in these eight matches (it was the mark's usual state before), and a capture every 4 frames slowed the game before it got there.
- **C, the strip at 1280 x 720**: `stop-line-lit-and-hint-strip-f000131.jpg`: the nine verbs end 52 px inside the strip's right end; the test holds the
  two new verbs, every old one and the 146-character bound.
- **B, re-measured with lane A166's trace** (`Bar.Movement.CarryTrace`, 100 matches a cell, seeds 1 to 100; the same matches as REVIEW-3's and
  MOVEMENT-8's: 3,142 possessions and 154 class knockdowns at rookie, 3,808 and 439 at pro in every row):

| Cell | Rule | Contested ring a match | lit before his touch | precision (down inside 0.5 s) | recall of "hit as he won it" | ... 20 frames or more before | Stop mark lit a match | its recall | its precision |
|---|---|---|---|---|---|---|---|---|---|
| rookie, the novice | as it was (REVIEW-3) | 18.6 | | 15.6 % | | | 15.2 | | |
| rookie, the novice | old thresholds + B1 (`closing=8;stopclosing=3`) | 18.3 | 24.9 % | 15.6 % | 79.2 % (122 of 154) | 14.9 % | 14.4 | 81.2 % | 8.2 % |
| rookie, the novice | **the new defaults** | **11.1** | 16.3 % | **21.5 %** | **71.4 %** (110 of 154) | 10.4 % | **12.0** | 73.4 % | 8.5 % |
| pro, the regular | as it was (MOVEMENT-8) | 19.3 | 30.9 % | 26.8 % | 72.0 % (316 of 439) | 59.5 % | | | |
| pro, the regular | old thresholds + B1 | 15.8 | 21.4 % | 13.5 % | **25.1 %** (110 of 439) | 11.4 % | 13.8 | 73.8 % | 24.7 % |
| pro, the regular | **the new defaults** | **9.9** | 13.5 % | 14.8 % | **17.3 %** (76 of 439) | 7.1 % | **12.3** | 66.0 % | 25.5 % |

  **What the table says, plainly.** At rookie the review's promise holds (18.6 to 11.1 rings a match, right more often, recall 8 points down), and the
  cut comes from the thresholds: the quiet second after a drop takes 0.3 rings a match there. **At pro the quiet second is where the ring was right**:
  it takes 3.5 rings a match and 206 of the 316 knockdowns the ring used to precede: recall 72.0 to 25.1 % before the thresholds, 17.3 % with them. At pro
  the opposing centre flattens him as he wins the draw; the ring said so a quarter of a second before (median lead 15 frames: too late to act on, which
  is the review's own point about it). B1 is in as ordered; whether pro should keep that ring is one number (`AfterDropFrames`), and it is the
  Director's.
- **How long the marks are on the screen** (`tools/qa/screen/u6-marks.py`, the eight matches): the ring: 49 showings, median 5 sim frames, 42 under a
  third of a second; the line lit: 36 showings, median 4 frames, 34 under a third of a second. Before the new rule (five matches, build 1): the ring 48
  showings, median 11 frames, 35 under a third of a second, every one drawn. The window lane's "not visible in the one episode it lit" (15 frames) is
  the ring's usual life, not a fault of that episode: it is drawn, it is thin (the plain chalk ring, about 3 px at 720p beside the human's 12 px ring)
  and it is over before an eye lands on it. One constant would make it read: a least time on the screen once lit (24 frames). Not done here (the order
  was: the ring stays as it is).

## In the tree, switched off (it follows the package, by the Director's order)

Written and gated before the order changed, left in because this lane has no way to take a change out of its tree: `jam.hud.ClearEdges` **0** (1: the
caption's row under the scoreboard clear of the MATCH card, the prompt and the net's distance pill; the prompt under the MATCH card; the own-net marker
above the radar; the Director's Call plates off downed figures), `jam.camera.PitchCapAfter` **0** (1: the pitch cap read again after the keep-human
stage; not measured in the game), and `TOO SOON` on the contact line (inert while `game.contact.check_cooldown_frames` is 0, as shipped; proven by
`Jam.Hud.TooSoon` on a staged sim with the key on; not seen in a match: the scripted human presses no check).

## How to repeat (part 6)

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-gate.ps1 -Tag ev6
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u5-downs.ps1 -Mode after -Seeds "2,4,5,6" -MarkStills 8 -RunName ev6-a
python tools\qa\screen\u6-marks.py tools\qa\screen\out\ev6-a\seed2\game.log tools\qa\screen\out\ev6-a\seed4\game.log
set JAM_CARRY_TRACE=level=rookie;player=novice;seeds=100
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u1-gate.ps1 -Tag trace-rookie -Filter Bar.Movement.CarryTrace     (the CARRYTRACE mark lines)
powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\build.ps1 -Project <worktree>\Game -Config Shipping
```
