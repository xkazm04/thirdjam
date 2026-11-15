# The bar at a cross-check: found by looking in the package, and gone (lane A219, track F3, Sunday 15 November)

The owner, 14 Nov 20:25 (package H34): "crosscheck has bug in animation displaying technical blue rectangle instead of animating player with stick into
its position". 15 Nov 10:35: "Crosscheck still displays blue line/rectangle in front of player. This should player never see". 11:00: "The bar is there
on both, first gray on tap, blue on hold." 11:40 (R-0070): "no aim line needed, the player animation should reflect the crosscheck movement and that
should be all".

The night's lanes had switched two marks off and counted "no rectangle or slab in 42 hit strips" on the editor's binary. Nobody had looked at a
cross-check in the packaged game, and nobody had checked which package was on his screen. This lane did both.

## What was found

**1. The bar he describes is package H34's, and H34 is what he started this morning.** Windows keeps a record of what a user starts from the desktop
(Explorer's UserAssist counters; `tools/qa/eye2/which-exe.ps1` reads the entries that name ThirdJam, read only). The last ThirdJam exe started by hand:
`Game\Packaged\Hokej_Hokej_H34\Windows\ThirdJam.exe`, 15 Nov 10:31:06, four minutes before his words. No entry exists for H35 to H39. H34 draws a bar
across the stick of a pressed cross-check that goes GREY when the shove meets nobody, and a bar across the chest of a held one in the TEAM's colour
(BLUE): one bar, 140 x 12 x 12 cm, 45 cm in front of the figure at chest height. Sheet `bar/h34-look-01-tap-then-hold-grey-then-blue-f8698-8734.jpg`
is that picture, made in H39's Development twin by switching the two bars back on (`jam.hit.Streak=1,jam.hit.BraceBar=1,jam.hit.Quiet=0`): grey-white
across his shoulders for the frames of the tap, blue from the frame the hold begins. H39 draws neither.

**2. What H39 itself still drew** (its Development twin, off screen, the game camera, its own defaults; every frame read):

| What | When | Colour | Whose | Sheet |
|---|---|---|---|---|
| An arc of 7 rectangles on the ice 1.35 m ahead of the holder | every frame of a held cross-check (from 40 frames after the press) | grey-white (235, 228, 225) on ice (180, 165, 175) | `HitRead.cpp` | `h39-01`, `h39-03`, `h39-13` (Glow Ice) |
| 10 to 16 rectangles in a ring and as shards round the two bodies | 8 frames from the contact of any hit that lands | white | `HitRead.cpp` | `h39-04`, `h39-06` |
| A ring of up to 12 rectangles round a downed man | his 90 to 150 frames down | his team's colour: orange round an opponent, BLUE round a mate and round the human himself | `HitRead.cpp` | `h39-02`, `h39-10` |
| A chevron of two rectangles ahead of a body checker | the wind-up and the window; grey after a miss | white, then grey | `HitRead.cpp` | `h39-07`, `h39-11` (a bot's) |
| A ring of ten rectangles round a stripped puck | 12 frames | gold | `HitRead.cpp` | - |
| A strip 22 cm wide on the ice from the puck along his aim, across his own figure when the mate stands beyond him | while the pass is held (one frame on a tap) | paper-white until the hold's reach finds a mate, then his team's BLUE | `HUDPassMeter.cpp` | `h39-14` |
| A strip across his path ahead of him | while he carries and an opponent closes | paper-white, then the opponent's colour | `HUDMarks.cpp` (the stop's line) | - |
| A plate "BRACED HIT" / "MISSED" at the bottom centre, by his ring | 60 frames after a hit or a miss | dark pill, pale text | `HUDWidget.cpp` (the contact line) | `h39-06`, `h39-07` |
| His ring swollen to 1.5 x (with the doubled carrier ring: a 3 m translucent disc) | 30 frames from a hit that hands him the puck | his team's blue | `HUDWidget.cpp` (the take grace's closing ring) | `h39-04`, `h39-06` |

Device B's figure draws nothing of its own at `PlayCrossCheck` / `SetCrossCheckHold`. A tapped cross-check that misses draws nothing in H39.

**3. The animation did not play when it mattered.** A cross-check is thrown into a man. One to four frames after the press the two bodies meet, the sim
reports a collision, and the pawn sent `PlayHitReaction` to the HITTER: device B's reaction take replaced his cross-check take. Thrown into the carrier
(the drill, press f1338, reaction f1339) not one frame of the take is drawn (`anim-02`); with four frames of air (the hand, press f391, reaction f395)
the arms drop two frames before the contact (`anim-01`). This is his first sentence of 14 Nov: "instead of animating player with stick into its position".

## The rule, in source

**At a check or a cross-check, tapped or held, landed or missed, his or a bot's, on any look, only the figures speak.**

| Switch (default now) | What it took away | 1 gives back |
|---|---|---|
| `jam.hit.Tell 0`, `jam.hit.BraceArc 0` | the chevron, the arc of dashes (and the streak and chest bar inside the same layer) | package H39 |
| `jam.hit.Burst 0` | the ring and shards of rectangles at contact | package H39 |
| `jam.hit.Down 0` | the countdown of rectangles round a downed man | package H39 |
| `jam.hit.PuckFlash 0` | the ring of rectangles round a stripped puck | package H39 |
| `jam.hud.PassMeter 0` | the pass line (the mate's ring, `jam.hud.ChargeLook`, stays) | package H39 |
| `jam.hud.StopLine 0` (new) | the stop's line across his path (the contested puck's rings, `jam.hud.Marks`, stay; the rule's state is still logged) | package H39 |
| `jam.hud.ContactPlate 0` (new) | the plate of the contact line (BRACED HIT, MISSED, TOO SOFT, JUST GOT UP, the stick lock) | package H39 |
| `jam.hud.SwellAtHit 0` (new) | the 1.5 x swell of the ring of a carrier who won the puck by his own hit (a grace won without a hit closes its ring as before) | package H39 |
| `jam.vis.HitTakeGuard 1` (new) | nothing drawn: a figure whose own check or cross-check take runs (the press to 10 frames after the wind-up), or who holds the brace, is not told `PlayHitReaction` for a mere collision; a stumble or a knockdown by a hit goes out as before | 0 = package H39 |

With no layer of the hit read on, `AJamHitRead::Render` does not ask the model for bars and `ApplyBars` never makes a mesh. `AJamHitRead::GameSwitches()`
is the one place the game reads the hit switches. Tests: `Jam.Hit.NoBars` (a missed check, a tap that strips, a 60-frame hold, a braced hit and the 90
frames down, thrown by the human and by a bot: zero bars in every frame, while package H39's switches draw every layer in the same frames),
`Jam.Hud.NothingAtAHit` (the HUD's five defaults and the grace ring's size), `Jam.Hit.TakeKept` (the guard's rule). The hit read's model and its layer
tests are unchanged. All of it is presentation: the sim's hashes of package H39 and of this lane's packages are equal line by line on every film pair.

What still shows at a hit, by design: the figures (device B's takes), the rings under skaters (the carrier's doubled ring when the hit hands him the
puck), the puck's ring, the bottom-left pill (the legend), the edge arrow for a man down outside the frame (`jam.hit.OffScreen`), the camera's kick and
the hit-stop.

## The method (`tools/qa/eye2/`)

| File | What it does |
|---|---|
| `eye2-run.ps1` | One game process, a PACKAGED Development exe under this worktree (`-Exe`) or the editor binary (`-Editor`), `-RenderOffScreen`, no console window, fixed 60 Hz, `r.RDG.AsyncCompute=0`. Menu, level, PLAY NOW or `-Choose modes/<cut>`; then drills with a still every sim frame, or `-ScriptHuman 2 -HitCapture N -WaitSec S`: the person-like hand and the game's own every-frame strips of his presses. Refuses an exe outside the worktree and any Shipping exe. Pid in `Game/logs/a219/<run>.pid`. |
| `films.ps1` | The film set of one exe: three looks (clay, night, Glow Ice) at 1280 x 720 and 1920 x 1080, the hand, the strips. |
| `drill-films.ps1` | A look whose match the hand plays without a cross-check (the night look), filmed with the drills set on the command line (the same match on every start). |
| `pair.py` | A before / after sheet of the same sim frames from two runs. `hashes.py`: the two runs' sim hashes and strips compared. |
| `sheet.py`, `grid.py` | The eye lane's sheet tools, copied. |
| `which-exe.ps1` | Which ThirdJam exes this user started from the desktop, and when last. |
| `auto.ps1` | The automation filter headless in a cleared environment. |

Facts for the next lane: a packaged Development exe starts in 8 s and plays 250 sim frames a second off screen while no still is taken; a still costs
about 0.15 s at 720 and 0.45 s at 1080. A drill switched on by a console command starts on a wall-clock frame, so two such runs show different frames;
a drill set on the command line (`-dpcvars=jam.ctrl.Drill=2`) and the hand's match are seeded and show the same frames on every start, on every exe of
one sim. `-log` on a packaged exe opens a console window: the tool uses `-abslog` alone. The mode names for `jam.menuscreen choose` are
`modes/hockey.mode.<name>`. The hand's match on the night look (Fog of War) has no cross-check in it. The game logs every variable set at its start
(`LogConfig: Set CVar [[...]]`): read it before trusting a film's defaults.
