# Profiling 7: device B's night on the merged tree, and B's unshipped figures out of the cook (lane A161, track Q3 evolution 6, device A, Sat 14 Nov 03:25 - 05:15 CEST)

Follows `PROFILING-6.md` (lane A150). Tree: the Director's local merge `706ed565` = main `e9d0b55` + `origin/dev-b` `8073e1ae` (device B's 25 commits:
timber stands and a clay crowd on `LR_P3_Clay`, effects at game size, new sounds), detached, not on main. Package of this tree: `Game/Packaged/A161`
(Development, with this lane's never-cook block). Every game run is off screen (`bot.ps1 -Offscreen`: `-RenderOffscreen` at 1920 x 1080, no window,
no cursor; port 4761), PLAY NOW's own path with no `-cut` (the operator's `hockey.regular` on clay, five a side). Status log: `status/A161.md`.

## 1. Is the merge safe for the 10:00 package: **MERGE SAFE**

| check | result |
|---|---|
| editor build of `706ed565` as merged | PASS, 328 s (103 actions) |
| full `Jam` filter | **240 of 240** (149.6 s); `Jam.Core.Determinism` hockey.regular s1 f600 **`ef34dd88c7c62b47`** = A150's main hash (B touches no sim) |
| cook-only of the shipped maps (`MapsToCook` + `DirectoriesToAlwaysCook`, the project's list) | exit 0, **0 errors, 0 warnings**, 2247 packages, cook-by-the-book 29.8 s (41 s wall) |
| shader / derived-data pass | none needed: B's content came from the machine's DDC ("Compiling global changed shaders" 2.6 s). The release chain pays tens of seconds on this machine's DDC, more on a cold one |
| after this lane's change: full `Jam` filter, cook-only | 240 of 240, same hash; cook 0 errors, 0 warnings, 1685 packages |

## 2. 183 MB of figures no shipped cut selects (B-to-A-0041 point 5)

**What the 183 MB is.** Source size on disk: TB2 (the `generated_b2` figure and its clay and broadcast versions) 73.4 MB, SkaterGen (the generated
meshes P2C / P3C / P4D / P4E and their 300-odd takes) 98.9 MB, the P4 lab figure 2.8 MB, `Look/P4Lab` 7.0 MB. Most of it is editor-only source data
(raw meshes, source art) the cook never carries: **in the cook the families are 26 MB of loose cooked files.**

**Reference walk** (`tools/size/refwalk.mjs`, new: every `.uasset` / `.umap` read as bytes, each long package name in it an edge; over-approximates,
the safe direction; roots = `MapsToCook`, `DirectoriesToAlwaysCook` minus the never-cook entries, and the long package names in the shipped C++).
Outside the families exactly three packages name one:
- `/Game/P4/Maps/P4_Lab` -> `SK_SkaterTB2_P4`: not shipped (not in `MapsToCook`).
- `/JamAssets/Look/MI_Bcast_SkaterP4` -> `MI_SkaterTB2_P4`: cooked only because `/JamAssets/Look` is always-cook; nothing names it. Never-cooked with them.
- `/JamAssets/Rigs/LR_P4_Broadcast` -> `Look/P4Lab` materials: **shipped** (BROADCAST (P4) is still in the settings' look list). `Look/P4Lab` stays,
  except the P4 lab figure's own four materials (`M_P4_Figure`, `M_P4_FigureE`, `MI_P4_Figure`, `MI_P4_FigureE`): the two masters read SkaterGen
  textures, `MI_P4_FigureE` is named only by `SK_SkaterGen_P4E`, `MI_P4_Figure` by nothing. The first after-cook without them failed with 10 "Content
  is missing from cook" errors from exactly these two masters; `refwalk.mjs` now fails on any cooked package that names a never-cook one.
- The code: `HokejGameMode.cpp:402-404` (`VisualClassPathsFor`, the `generated_b2` branch), chosen only by a cut whose `game.skater.visual` is
  `generated_b2`: `hockey.test.visual-b2` and `hockey.test.figure-b2`, no shipped cut, no designer key.
- The shipped figure (TB1, `BP_SkaterVisual_TB1` / `_TB1_P3`) names none of them.

**The exclusion.** `/JamAssets/Skaters` is one flat folder that also holds TB1, so a folder entry cannot do it. UE 5.9's
`UCookOnTheFlyServer::GetNeverCookPackageNames` matches `DirectoriesToNeverCook` against long package names with
`TDirectoryTree::ContainsPathOrParent`, so an entry may name a single package: **561 entries** (556 in Skaters: every `*TB2*`, `*SkaterGen*`, `*P4Lab*`;
plus `MI_Bcast_SkaterP4` and the four P4 lab figure materials), in a marked block of `Game/Config/DefaultGame.ini` that `tools/size/nevercook.mjs`
writes (`--write`), checks (no flag) and removes (`--remove`, the revert path). A new TB2 / SkaterGen asset from B is cooked until the script runs
again; `refwalk.mjs` names it. Nothing is deleted.

**The tests.** `Jam.Bot.VisualSwap` (loads `BP_SkaterVisual_TB2`) is `EditorContext | CommandletContext`: it never runs in a packaged client and runs
in the editor as before (240 of 240). The fuzz sweep (`Bar.Fuzz.*`) runs the sim only and loads no visual class. A test cut launched in a package
resolves the TB2 class, does not find it, and falls back to the toy figure (`ResolveVisualClass`, its existing warning line).

**Size by folder** (`tools/size/cookonly.sh`: the same cook with loose files, zen store off on the command line only; MB):

| folder (cooked) | before | after |
|---|---|---|
| `JamAssets/Skaters` | 29.1 | **3.1** |
| `JamAssets/Look/P4Lab` | 4.4 | 4.4 |
| shader library (`Content`, staged) | 141.5 | 141.3 |
| everything else | unchanged | unchanged |
| **staged content (total less `Metadata/ShaderLibrarySource`)** | **323.5** | **297.2 (-26.3, -8.1 %)** |
| packages cooked | 2247 | 1685 |

Package `A161` (after): `ThirdJam-Windows.ucas` **144.1 MB** against `A150c` (main `322e6c5`, before B's 25 commits) 155.4 MB: B's night plus this
change is 11.3 MB lighter than the package of record before it.

## 3. The stands and the crowd on clay (B's AR2: 293 actors, 339,596 triangles)

**The switch.** B has no console variable; its switch is "hide the actors labelled `Stand_P3_Clay*` and `Fan_P3_Clay*`". Labels are editor-only and
the actors' names are `StaticMeshActor_N`, so `bot.ps1 -ToggleMeshes 'SM_(Fan|FanRow|Stand)_'` finds them in the match (`getall StaticMeshComponent
StaticMesh`: 195 `SM_Fan_*`, 42 `SM_FanRow_*`, 56 `SM_Stand_*` = 293 in the streamed `LR_P3_Clay` instance) and hides or shows them every 30 s with
`ke <name> SetActorHiddenInGame 1|0`. `ke` finds a name with `FindFirstObject`: with the menu's rig preload on, the source package `/JamAssets/Rigs/LR_P3_Clay`
is also loaded and wins the name (three runs hid nothing: kept as baseline runs); the toggle runs carry `-Dp jam.PreloadRig=0,jam.WarmScene=0`.
**Paired by construction**: both states alternate inside one match, so the machine's load falls on both (`tools/perf/stands.mjs --by time`, 180 guard rows
at each toggle).

| per frame, clay, 5 v 5, 1920 x 1080 | stands and crowd shown | hidden | paired ratio shown / hidden (8 pairs, range) |
|---|---|---|---|
| frame | 16.79 | 16.84 | **1.00** (0.83-1.02) |
| game thread | 2.97 | 2.93 | **1.03** (0.90-1.11) |
| render thread | 4.99 | 4.95 | 1.01 (0.82-1.09) |
| GPU | 5.87 | 5.88 | **1.03** (0.85-1.12) |
| GPU/Basepass | 0.39 | 0.34 | 1.16 |
| GPU/Lights | 0.61 | 0.56 | 1.13 |
| draw calls | 552 | 474 | 1.16 |
| triangles drawn | 305k | 180k | 1.53 |

Medians over runs `a161-st-2` (clean) and `a161-st-3` (another process held the GPU: 22-25 ms in both states; the ratios still pair). In the clean run
alone the stands cost **+0.2 to +0.6 ms of GPU** (4.94-4.98 -> 5.15-5.54) and 0.0 to 0.3 ms of game thread. Board hits (the corner moments, 33): worst
frame in the 10 rows after 18.6 shown / 17.2 hidden, GPU 4.77 / 4.95. No goal fell inside a kept segment; the opening faceoff falls before the
toggles (sim frame 600).

**The frame budget of `PROFILING-6.md` holds**: frame median 16.7-16.8 ms (held at 60 Hz), game thread 2.7-3.0 (budget 1.9-3.0), render thread 4.6-5.0
(3.7-5.0), GPU 4.8-5.9 (4.4-5.7; the clean run's 5.15-5.54 with the stands). The p99 is the loaded machine's (render-thread `EventWait` at an idle GPU,
in both states). Nothing to buy back tonight; if a slower GPU needs it, the switch that buys most is the crowd alone (237 of the 293 actors, the
`SM_Fan*` meshes: most of the 125k triangles in view), which B can also instance (its 150-triangle fans).

## 4. B's five timings (B-to-A-0041 point 7)

| | what | result (paired in one match unless said) |
|---|---|---|
| a | four shadowed key spots on clay, shadow resolution 1.0 vs 0.5 | `r.Shadow.Virtual.ResolutionLodBiasLocal` 0 vs 1 (the VSM form of the lights' resolution scale), toggled every 20 s: **GPU/ShadowDepths 0.38 vs 0.31 ms (ratio 1.23)**, GPU total ratio 0.99 (0.86-1.38), frame 1.00. The project already sets `...LodBiasLocalMoving 1`: movable spots already render at half |
| b | goal frame, 6,000 cd 16 m light vs radius 350 | **not measurable tonight**: `jam.rink.react.light` 6000 vs 0 toggled every 15 s for 180 s; no goal fell, and another process held the GPU (17.7 ms in both states). Wants a scripted goal (a drill that scores) |
| c | the worst moment (board hit with a slide, goal, two hard stops) | board-hit windows (82 in four runs): worst frame in the 10 rows after median 18.6 ms, game thread 3.2, GPU 4.8 (play median 4.8-5.2: under +0.8 ms). A goal with the slide trail together was not seen in about 15 minutes of play (no goal fell in any run) |
| d | the two figure Blueprints' `UpdateMotion` per tick, ten figures | **not measurable tonight**: the CSV carries no Blueprint function timers; it needs an Insights trace with Blueprint named events (`-trace=cpu -statnamedevents`) and its export, outside the time box |
| e | `M_Ice` on clay over the full screen at 1920 x 1080 | upper bound: the whole base pass is **0.37-0.42 ms median (p99 1.6)** on clay at 1920 x 1080, ice included; `M_Ice` cannot cost more than that in the base pass |

## 5. The first match on the merged tree (A150's measurement repeated)

Interleaved `A150c` (main before B) and `A161` (the merge plus this change), three each, PLAY NOW, no cut, off screen:

| pair | A150c: frames over 33 ms in the first 60 / worst | A161 |
|---|---|---|
| 1 | 2 / 107 | 4 / 186 (frame +60, render-thread wait at an idle GPU: the machine) |
| 2 | 19 / 100 | 11 / 125 |
| 3 | 1 / 50 | 1 / 42 |

Synchronous loads from the match start: **0 in 6 of 6**. PSO misses 14-15 graphics + 4 compute in both. The light hold grows from 4-7 ms to 12-20 ms
(the rig's 293 more actors stream in it). No new hitch class; neither arm is clean on this machine.

## 6. Next

1. **A scripted goal for the reaction timings** (B's b and c): a drill cut that scores on a set frame, run with the goal light toggled
   (`bot.ps1 -ToggleHideCmd`); proof: 10 goals a run in each state, GPU on the goal frame paired, the 6,000 cd light's share in ms.
2. **The figure Blueprints' tick cost** (B's d): an Insights trace of ten figures with Blueprint named events, `UpdateMotion` per tick exported per figure;
   proof: a ms-per-tick number for `BP_SkaterVisual_TB1_P3` against the 0.5 ms a figure the frame can spare.
3. **The cook guard**: `refwalk.mjs` and `nevercook.mjs` (check mode) as a gate step, so a new TB2 / SkaterGen asset from B, or a shipped package that
   starts naming one, fails before a package; proof: the gate red on a planted reference, green on the tree.

