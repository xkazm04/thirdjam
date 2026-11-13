# EYES-GRID-FIXES: the A-side faults of the h19 eyes grid, fixed in rank order (lane A64, device A)

Source: `docs/experiments/EYES-GRID.md` (lane A54, h19 Shipping). Proof: a Development package of the A64 worktree (`Game/Packaged/A64-dev`, gitignored), A54's
`tools/qa/eyesgrid/shoot.ps1` with A54's timeline (card, faceoff 7 s after ENTER, far net with W held, pause, Director's Call), 10 launches, 47 stills in
`docs/experiments/img/a64/` (`<cut>-<tier>-<moment>.jpg`, `-720-` for 1280x720), 20 Gemini calls (bug-hunt and hud on every still, readability at 6 m on
faceoff and far net; flash, no thinking) in `docs/experiments/eyes/a64-*.json`.

Two differences from A54's run, both measured:
- **The card still is taken 1.7 s after ENTER, not 3 s.** In the Development exe the match starts at once and the MATCH card (150 frames) is gone by 3 s; 1.7 s is
  the drop (frame 90), which is the moment the brief asks about. The before column for row 6 is therefore a harder moment than A54's.
- **This run had the foreground** (A54's never did: a Windows Security dialog held it). The cursor is shown and locked to the viewport (A49), so the real mouse aims
  the stick and turns the skater. The driver parks the cursor up-ice for each launch; with that, the regular cut's faceoff and far-net stills match h19's poses.

## Fault table (A54's rank order)

Before = h19 stills of the same cells, after = A64 stills; "stills" counts stills with a row of severity 3 or more that names the fault (regex over Gemini's words,
40 of the 47 new stills have an h19 twin).

| # | Fault (h19 rank) | Where it really was | Fix (file) | Before: stills, max sev | After: stills, max sev | Still |
|---|---|---|---|---|---|---|
| 1 | chaos bumper a flat magenta box (1, sev 5) | not a missing material: `RinkActor.cpp` drew each `bumper` obstacle as an open 20-segment tube with no lid, in B's `MI_Toon_Bumper` (`M_Toon`, hot magenta, **Emissive 2.00**, read at runtime). Open top + one flat tone read as a box | a rubber bollard: dark base and top bands 3 cm proud, a dark lid (`MI_Toon_Dark`, section 3) with a magenta boss; B's Emissive capped at 0.8 by a dynamic instance (`RinkActor.cpp`, `RinkActor.h` `BumperEmissiveCap`) | 3, 5 | 0, - | `hockey.chaos-l2-card.jpg`, `-faceoff.jpg` |
| 2 | controls strip collides with the radar and "PAUSE ESC" (2, sev 4, 53 stills) | the strip was full width (anchors 0..1) and the line 151 characters at 18 pt; the radar frame (216 x 116 px) sits on the same band; source also had SPIN E added since h19 | strip from 24 px to 256 px short of the right edge (radar + 16 px gap), type 16 pt, 4-space gaps (`HUDWidget.cpp`, `HUDWidget.h` `ControlsRightInsetPx`) | 12, 3 | 0, - | `hockey.regular-l2-card.jpg`, `hockey.regular-l2-720-card.jpg` |
| 3 | puck and carrier not in frame (3, sev 4) | the follow camera frames the human only; at the faceoff still (7 s) the play has gone into the human's own end (JAMEVENT breakaway at x -18 m, frame 296), behind the camera | **not fixed.** A puck / far-net yaw lead (`jam.camera.KeepYaw`, written and measured) feeds back: the cursor's point on the ice turns with the camera, the skater turns to it, the pivot yaw follows; with 22 deg the idle skater spun to the side boards and a held W never reached the far net (stills thrown away; a run with `KeepYaw=0` restored h19's pose). Shipped with the lead **off** (default 0, opt-in only). At the drop the puck is in frame with its halo (`*-card.jpg`) | 15, 4 | 19, 4 | `hockey.regular-l2-faceoff.jpg` |
| 3b | far net not in frame (7, sev 4) | A49's end-board framing: at the far goal line the camera faces the end boards, the net sits under the radar in the bottom-right corner | not fixed (same feedback as row 3) | 4, 4 | 4, 4 | `hockey.regular-l2-farnet.jpg` |
| 4 | goal-light housing a floating cube (5, sev 4) | A47's 40 x 30 x 30 cm box straddling the rail | the housing is a 12 x 80 x 12 cm clamp around the glass top rail, the light a red 68 x 8 x 8 cm bar lying on it (`RinkActor.cpp`) | 9, 4 | 0, - | `hockey.regular-l2-farnet.jpg`, `hockey.regular-l1-farnet.jpg` |
| 5 | `l1_day` blown out (6, sev 4) | **A's side, not B's rig parameters**: `LR_L1_Day` and `LR_L3_Neon` are still lane A0's empty placeholder maps (8 KB: a brush and world settings, no light, no post volume). The switcher hid the persistent sky light for them and auto exposure washed the picture (mean luma 177; 200 with the persistent lights alone) | a rig with no light keeps the persistent sky and directional lights, and the follow camera gets -1 EV (`LookTierSwitcher.cpp/.h`, `CameraFollow.cpp/.h` `SetExposureBias`); log `JAMLOOK rig l1_day carries no light ...`, `JAMLOOK camera exposure bias -1.0 EV`. Mean luma 122, 0.4 % clipped (was 177 and 3.3 %); l3_neon the same | 5, 4 | 0, - | `hockey.regular-l1-faceoff.jpg`, `hockey.boost-l1-card.jpg` |
| 6 | MATCH card and WON / YOU ring over the faceoff (14, sev 3) | the card lives in `DirectorsCallWidget.*` (not `HUDWidget.*`): 77 pt, centre y 262, about 510 x 165 px; the ring at the screen centre at 0.66 sat on the human's hip | card 50 pt, tighter padding, centre y 190 (`DirectorsCallWidget.h/.cpp`); countdown / WON ring and the faceoff hint 260 px right of centre (`HUDWidget.cpp/.h` `RingOffsetXPx`) | 10, 3 | 5, 3 | `hockey.regular-l2-card.jpg`, `hockey.boost-l3-card.jpg` |
| 6b | net-distance markers on boards and nets (11, sev 3) | the distance hung 18 + 30 px under the diamond, onto the crossbar | the distance stands beside the diamond (right, or left near the right edge) at the diamond's height (`HUDWidget.cpp`) | 19, 4 | 12, 3 | `hockey.regular-l2-faceoff.jpg` |
| 7 | silent fallback on a rejected `-cut` | `UE_LOG` is compiled out of Shipping (no `bUseLoggingInShipping`) | `JAMCUT rejected <path>: <reason> (falling back to hockey.regular)` to the debugger stream, stderr, `<Saved>/Logs/JamCut.log` (append) and the log; a 4 s toast top-left (`HokejGameMode.cpp`, `HUDWidget.cpp/.h` `ShowToast`) | - | proven | `bad-cut-card.jpg` |

Row 6 and 6b are partial: on the bigger and boosted rinks the far centre still stands behind the smaller card (5 stills), and the marker is beside, not above, the
net at 50 m (Gemini still calls 12 "on the net"). Gemini also names the toast a "debug banner" (it is meant to be seen).

## What is B's
- `Game/Plugins/JamAssets/Content/Look/MI_Toon_Bumper.uasset`: `Emissive` 2.0 flattens the magenta body; about 0.6-0.8 lets the toon ramp show the round. A caps it
  at 0.8 meanwhile (the cap stops acting once B's value is at or under it).
- `Game/Plugins/JamAssets/Content/Rigs/LR_L1_Day.umap`, `LR_L3_Neon.umap`: still empty placeholders; a real day / neon rig (sun or room lights, a post volume with
  exposure) replaces A's stand-in (the switcher drops the -1 EV and hides the persistent lights as soon as the rig carries a light).
- `MI_Toon_GoalLight`: the clamp now takes it; a dark rubber grey reads best on the glass.

## Still open (A)
- Rows 3 and 3b need a decision: a camera that leads to the puck must decouple the cursor aim and the stick from the camera yaw first (A49's input), or the puck
  needs an off-screen pointer on the HUD instead of a camera move.
- Not re-run here: the Jam.Camera / Jam.Menu automation tests (the A40 / A49 camera tests build their own settings with `KeepYawDeg` 0; MenuTests' banner-band
  bounds hold by arithmetic: 121 >= 110, 295 <= 453). The editor target and the Development game target build clean.
