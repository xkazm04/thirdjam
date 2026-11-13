# SK11 report: the tackling takes (SKF pass 12) in the engine on the scripted figure (device B)

Lane SK11 (the SK8 to SK10 lane), 2026-11-13, 10:02 to 10:45 machine clock. Tree `C:/Users/kazda/jam/wt/b-a45` (branch `dev-b-a45`, on 4441259). Editor port 8000, mine: pid 28020,
10:02 to 10:41, left through `ExitEditor` (process gone, dirty lists empty just before). No build, no cook, no git write, no Blueprint edit. `jam.Cut` back to `hockey.regular`,
`jam.ScriptHuman` 0; `/Game/Maps/Rink` never saved; no `_Scratch/` or `_SK8TMP/` left. Port 8002 never touched.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk11` (scripts, `img/` every capture, `logs/` recordings, `bk/` the 26 takes as committed).
I imported export11 first (20 takes), took its numbers, then switched to export12 as told. **The saved state is export12 for the 13 takes on both figures.**

## Verdict per item

| # | Item | Verdict |
|---|---|---|
| 1 | Animation-only re-import, 13 takes x 2 figures, by name | **pass**: equal to export12 by take name, 0.0; clay aim holds still export9 |
| 2 | Knockdown timing in real matches | **partial against the target**: get-up 44 frames at rate 1.0 ending 0 frames from the release: yes. The fall is **not** shown at rate 1.0 and **not** to its last frame: the Blueprint plays it at 1.10 to 1.15 and holds it 0.08 s before its end (frame 22 of 26). Lying about 27 frames, not 20 |
| 3 | The two predicted faults | **both there on the skinned figure**, smaller with export12 than with export11 |
| 4 | Game-camera strips and the hit statistics | **partial**: night side and front, clay side and front. **No hit from behind happened** in the match, so no "behind" strip. Statistics for all 5 knockdowns |
| 5 | Which take on `PlayCheck` | `Check_Deliver_Right` or `Check_Deliver_Left`. The plain `Check` take is referenced by nothing. **The attacker's take played in 1 of 5 real knockdowns** |
| 6 | The lump at the faceoff | the top-hand glove on the butt end of the stick |

**The finding that matters most for the tackle:** the bots' check take is almost never seen. `PlayCheck` arrives 1 to 3 frames before the hit, but the Blueprint plays it at priority 4, and a pin or reaction take (priority 6) is usually still running on the attacker, so it is refused. Even when it plays, the hit lands on frame 2 of it.

## 1. The re-import

- 13 takes per figure from `skf/export12/<style>/SK_Skater_<style>_loopcut.fbx`, each re-imported in place through Interchange: `Fall`, `Fall_Forward`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Trip`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Check`, `Check_Deliver_Right`, `Check_Deliver_Left`.
- Stored index: checked for all 26 before importing; none is stale.
- Proof by name (export12 imported once more as scratch assets and compared, 24 bones, every frame for the 13): night 72 of 72 equal, 0.0. Clay: the 13 equal, 31 others equal, and exactly the 28 aim holds differ (they are export9's, as intended). Scratch deleted.
- Lengths now: falls 26 frames (0.4333 s), get-ups 44 frames (0.7333 s), checks 30 frames (0.5 s).
- After it: 731 assets under `/JamAssets/Skaters/` load, 731 valid, none missing; 16 Blueprints and AnimBPs `BS_UP_TO_DATE`; 16 blend spaces full. `git status`: exactly these 26 take files changed.

## 2. Timing

Night tier, default figure, seed 1, `all_star`, scripted human: **5 knockdowns in 99 s** (4 on the human, 1 on a teammate), every one with `knockdown_frames` 90. Recorded on every rendered frame, plus the Blueprint's own log lines. Export11 and export12 give the same timing (same lengths).

| | Target | Measured (all 5) |
|---|---|---|
| Fall as played | 26 frames at 1.0 | reaches **frame 22 of 26** and stops there; played at **rate 1.10 to 1.15** (`SK4KD ... rate 1.102`, `1.105`, `1.106`, `1.15`) |
| Lying (the held frame) | about 20 | **26 to 27 frames** (until frame 46 to 47) |
| Get-up | 44 at 1.0 | **44 frames, rate 1.0**, starts frame 46.0 to 47.0 |
| Get-up end against the release | 0 | **0.0 frames** (end frame 90.0; `end_minus_rel_f` 0.000005 on the three undisturbed ones, -0.18 and -0.39 on the two my captures paused) |

- **Why the fall differs:** two constants in the Blueprint, both older than these takes. `PlayDown` sets `DownHold` to the take's length minus 0.08 s, and the tick pauses the fall there. `KnockDown` sets the fall's rate to 0.95 + 0.2 x hit strength (strengths here 0.76 to 1.0).
- **What that costs:** the last 4 frames of every fall are never shown, and the get-up blends in (0.12 s) from frame 22, not from the fall's last frame. The join SKF built (last frame = first frame) is not what plays.
- **I did not edit the Blueprint.** To meet the target exactly: `DownHold` = length (0.08 -> 0.0) and the rate fixed at 1.0, in `BP_SkaterVisual_TB1` and `_TB1_P3`. Two constants each, no rewiring.
- **Other `knockdown_frames`** (test figures, both Blueprints, export12 lengths): **60**: fall + get-up is 70, so both are sped up by 1.167; get-up starts at frame 22.3 and ends on 60. **120**: rate 1.0, get-up starts at frame 76 (about 56 frames lying), ends on 120. **40**: both at 1.75.
- **Clay** (scripted figure attached to the pawns and fed the game figure's calls, since this tree's game mode does not select it): the same: fall to frame 22, get-up from frame 45 to 46, 44 frames at 1.0, end frame 90.0. The release reaches the copy one tick late through my script.

## 3. The predicted faults (test figures, recorded every rendered frame, scaled to 60 Hz)

**Export11** (what the brief asked about):
- **Shoulder pop: yes.** At frame 2 the clavicles turn 26 to 50 degrees in one frame (`Fall` 40 / 50 night / clay, `Knockdown_Back` 36 / 26, the side takes 47), 14 to 41 the frame after; hands move 40 to 75 cm in that frame.
- **Skate turn in the get-ups: yes.** One frame of 75 to 79 degrees, twice per take: `GetUp` at frames 14 and 23, the side get-ups at 16 and 25. `GetUp_Front` 25 degrees.

**Export12** (what is saved):
- **Arm swing at frames 1 to 2 of the falls:** the upper arm turns **52 to 59 degrees** in one frame in `Knockdown_Back`, **55 to 67** in `Fall`, **58 to 60** in `Knockdown_Left`, **91 to 103** in `Knockdown_Right`, and 37 to 75 the frame after. Hands 41 to 76 cm and head 24 to 50 cm in that frame. Less than Blender's 106 to 149 because the Blueprint blends the fall in over 0.06 s.
- **How it looks:** in the strips it falls on the frames the hit flash covers; it reads as the blow, not as a glitch. I did not watch it at speed.
- **Skate turn in the get-ups:** down to 25 (`GetUp_Front`), 29 to 30 (`GetUp`), 42 to 55 degrees (side get-ups, frame 15). Still one frame, about half of export11.
- **Stick whip in the check:** 43, 64 and 32 degrees a frame at frames 2, 3, 4, still by frame 5 (the take blends in over 0.08 s).
- **The 30 cm throw:** the pelvis is 30 cm off the sim's position from frame 11 of the fall, stays there while lying, and is back at 0 by the end of the get-up; the largest single-frame move is 15 to 18 cm at frame 3. The sim slides the victim 1.1 to 4.7 m in the same time, so the 30 cm does not read as a slide back in the strips.
- **Clipping into the attacker or the boards:** nothing obvious in the four strips; **not checked at full resolution** (the knockdown at frame 988 is 40 cm from the end boards).
- **Side lying pose, arm in the ice:** the lowest arm joint centre is 7.6 to 8.3 cm above the ice. Not visible at the game camera's size. **My close-up missed the figure, so not checked close.**

## 4. How it reads, and the hit statistics

Strips from the real game camera (the match frozen for each capture, frame numbers relative to the hit printed):
- `sk11-tackle-before-after`: the knockdown at sim frame 988 with the export10 takes above, export12 below. Before: the victim curls into a lump. After: thrown flat, legs out, a long lying pose, up by about +69.
- `sk11-knockdown-night-side` (988, from the right), `sk11-knockdown-night-front` (2562, from the front at full strength: `Fall`, `GetUp`).
- `sk11-knockdown-clay-side` (988), `sk11-knockdown-clay-front` (2562; the copy plays `Knockdown_Left` there because my direction rebuild fell on the side branch).
- **Not delivered: `-night-behind`, `-clay-behind`.** No knockdown from behind happened in 99 s of this seed; the one from the left was on a teammate away from the camera.
- The night side strip has 21 of 33 frames (every 6th in the middle): a leftover capture loop of mine took every other frame. The before strip and the other three are complete.

All 5 real knockdowns (night; clay is the same sim):

| Sim frame | Victim | Attacker | Fall take | Centres apart at the hit | `PlayCheck` to hit | Attacker's take at the hit | Victim's travel while down |
|---|---|---|---|---|---|---|---|
| 988 | human | bot 4 | `Knockdown_Right` | 80 cm | 3 frames | none | 356 cm |
| 1464 | human | bot 3 | `Knockdown_Right` | 86 cm | 2 | none | 111 cm |
| 1818 | human | bot 3 | `Knockdown_Right` | 90 cm | 2 | `Board_Hit` (frame 14) | 468 cm |
| 2562 | human | bot 3 | `Fall` | 82 cm | 2 | **`Check_Deliver_Left`, frame 2** | 356 cm |
| 4466 | teammate 1 | bot 4 | `Knockdown_Left` | 83 cm | 1 | none | 282 cm |

- **The hit lands 1 to 3 frames after `PlayCheck`** (3, 2, 2, 2, 1). All five are bots. The scripted human made no check, so I have no number for the human's 0 to 12.
- **Does the lunge read?** With export12 the lunge is at its extreme by frame 3, which fits a hit on frame 1 to 3. But it showed in one knockdown of five, and there the hit flash sits on it. `sk11-check-lunge` shows the take on a test figure in slow motion.

## 5. `PlayCheck`

- `BP_SkaterVisual_TB1` and `_TB1_P3` (and the generated figures' Blueprints): `PlayCheck` finds the nearest opponent and plays **`Check_Deliver_Right`** if that opponent is on the right, else **`Check_Deliver_Left`**, through `PlayAct` at priority 4, rate 1.0, blend-in 0.08 s.
- **The plain `Check` take is referenced by no asset** (asset registry, all five figure families). It is never shown.
- **Why it rarely shows:** `PlayAct` refuses a take whose priority is below the running one. `Board_Pin` and the hit, board and stick reactions run at 6; a check is 4. Bots check out of pins and scrums, so the check is refused (4 of 5 here). The constant is the `4` in `PlayCheck`'s two `PlayAct` calls.

## 6. The lump at the faceoff

It is the **top hand's glove gripping the butt end of the stick**, with the pale knob above it (`sk11-faceoff-lump`). In the faceoff stance the blade is on the ice 78 cm in front and the trunk leans 50 degrees, so the stick runs up past the right hip: the top hand sits 20 cm behind the pelvis at 98 cm, and the knob stands at 107 to 114 cm, higher than the head (100 cm). It comes from the takes (`Idle_Faceoff`, `Faceoff_Ready`), not from a pad or a material. Not changed.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| 13 x `AS_SkaterTB1v2_{Fall, Fall_Forward, Knockdown_Back, Knockdown_Left, Knockdown_Right, Trip, GetUp, GetUp_Front, GetUp_Side_Left, GetUp_Side_Right, Check, Check_Deliver_Right, Check_Deliver_Left}` (total) | 2,272,461 | 2,189,537 |
| The same 13 x `AS_SkaterTB1_P3_*` (total) | 2,426,152 | 2,341,240 |

Nothing else under `Game/` changed. New outside `Game/`: this report and 14 files in `devlog/shots/`.

## Ledger rows to replace

```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (181780 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (178332 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (181990 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (165571 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (168409 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (189553 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (191203 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (196906 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (199807 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (168445 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (172353 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (172217 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p3_clay loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lanes SK8, SK11) (174674 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (170559 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (169063 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 attacker lunge, 30 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (170769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (153677 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (156452 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (175974 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (177709 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (186205 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 get-up, 44 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (186235 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (156487 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (161361 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (161330 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export12/p2_cel loopcut (pass 12 fall, 26 frames), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lanes SK8, SK11) (163716 bytes) |
```

## Stills (`C:/Users/kazda/jam/wt/b-a45/devlog/shots/`, each with a `.txt`)

- `20261113-104242-B-sk11-tackle-before-after.png`
- `20261113-104242-B-sk11-knockdown-night-side.png`
- `20261113-104242-B-sk11-knockdown-night-front.png`
- `20261113-104242-B-sk11-knockdown-clay-side.png`
- `20261113-104243-B-sk11-knockdown-clay-front.png`
- `20261113-104243-B-sk11-faceoff-lump.png`
- `20261113-104243-B-sk11-check-lunge.jpg`

## What I could not verify

- **A cook.** Not run (yours).
- **A knockdown from behind, or from the left on the human, in a real match:** none occurred. `Knockdown_Back`, `GetUp_Front`, `Fall_Forward` and `Trip` were seen on test figures only (numbers above), not from the game camera.
- **The human's check timing** (the scripted human does not check).
- **Clipping and the arm in the ice at close range.**
- **The shoulder and skate numbers before any import:** my first recording failed, so there is no export10 baseline for item 3.
- **Motion at speed by eye.** Strips, slow-motion stills and per-frame numbers only.
- **The clay figure selected by the game** (this tree's game mode does not pick `BP_SkaterVisual_TB1_P3`; I used the call-copying route).

## What the lead must decide

1. Raise the check's priority (the `4` in `PlayCheck`) so the attacker's lunge is seen; without it the new `Check_Deliver_*` takes are almost invisible in play.
2. The fall's two constants (hold 0.08 s before the end, rate by strength), if the fall should play all 26 frames at 1.0 and join the get-up as built.
3. The hit lands on frame 1 to 3 of the check: tell SKF the contact pose must be reached by frame 2, or ask device A for a longer lead.
4. Run the cook-only check; paste the 26 ledger rows.
