# Lane E4: interim report (URGENT cross-check block first; then the blade where A aims; exports 48 / 49 / 50)

## URGENT (lead 13:45, A-to-B-0039): the cross-check, both figures. Proven headless 14:12; GPU strip pending (GPU held by another lane)
No merge of dev-b was done: dev-b since lane/b-fig's base changes nothing under `Game/Plugins/JamAssets/Content/Skaters/`, and the handoff
was read from the repository. No git commands.

| Point | What changed | Proof (headless, clay test figure at 6 m/s; cel the same) |
|---|---|---|
| 2: the hold shows from the take's contact | `HoldTick`: while the hold is on and `CrossCheck_Wind` plays at or past its contact frame (0.165 s), the take is stopped with a 0.1 s blend. The hold layer is already up underneath, so it shows at once. If the call comes after contact, the cut happens on the next tick | PlayCrossCheck(8) at t10, hold at t18 (press + 8): the hold pose (hands 114 cm, the head bone's height) is at full weight at **t25 = press + 15** (before: **press + 29**). The hold-in raises the right glove to 133 cm on t21 to t23 (the stick coming up once). Hold at t30 (late): the cut is on t31 and the pose is full by t36 |
| 1: the tap reads as a shove | export49's `CrossCheck_Wind` and `CrossCheck_Release` imported over the same names, both styles. The take is not cut by `PlayLift`: `CrossWind` stamps `LiftPlayT`, so a `PlayLift` within 0.35 s is skipped, and a `PlayLift` before it is replaced (same priority 6) | Contact frame 10 on **call + 4 / 8 / 12 / 14 = t34 / 38 / 42 / 44**, both figures. Hands at 96 to 99 cm (shoulder height) through take frames 10 to 18 |
| 3: the hold drops on a fast skater | `StopTick` starts no stop take while `bHoldOn`. `HoldTick` stops a stop take (priority 5) that is already playing under a hold | Not reproduced on the test rig, which has a constant speed. Cause found in the graph: the brace slows him, `StopTick` plays `Stop_Hard` on `DefaultSlot`, and that covers the hold layer. Now gated |
| 4: one frame of gloves over the helmet on release | `HoldTick` keeps the ABP's `bHoldOut` false. The hold fades from its own loop pose over 0.1 s instead of the out-take, whose first frame throws both gloves up | Release at t70: hands 113.8, 88.7, 68.8, 73.6 (before: **138 / 128** on t71) |
| whiff | `PlayCrossWhiff()` and `PlayWhiff(int32 Kind)` (0 is the body check's whiff, 1 is the cross-check's), playing export49's `CrossCheck_Whiff`. From `CrossCheck_Wind` at frame F the whiff starts at F-10; from `CrossCheck_Release` at F-3; cross-faded over 3 frames. With nothing playing it starts from 0 with a 4-frame blend. It plays to its end (BlendOutTriggerTime 0) | Wind(8), then the whiff on the miss frame: whiff frames 0 to 14 played. From the release: starts at 0. From nothing: 0 to 14 |
| sim | | Seeded clay match: **738 = 738 sim lines identical**, JAMSIM 7200 `1d1fb181b85f1d4d`, 0 Blueprint errors, 0 `Error:`; all four Blueprints BS_UP_TO_DATE |

**GPU strip (looked at with the Read tool):** `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-142304-B-E4-crosscheck-tap-hold.png`
(game camera, 1280 x 720, clay, 6 m/s). TAP: the shaft is a bar across his shoulders with both gloves past them from contact (+8) to +14.
HOLD (called at press + 8): the bar rises straight into the hold by +15 with no pop, and the release shows no gloves over the helmet.
LATE HOLD (called at +20): one frame of the stick swung upright at +24 before the hold. A no longer sends the hold that late.
A hold-in pop at press + 12 was seen on the first strip. It was fixed (the cut skips the hold-in: `HoldT0` set back 0.1 s, `HoldW0` 0) and re-filmed.

**Still in the new take, for the operator or the lab:** `CrossCheck_Wind` frames 3 to 8 (the coil before contact) put both gloves at **141 cm
against a head at 104 cm**: over the helmet. With A's 8-frame wind-up the take starts at frame 2, so those six frames show first. From
behind they may still read as a lift before the shove. A re-key of frames 3 to 8 lower is the lab's job (HX2), by name, with no wiring change.

Files to merge (all saved): `BP_SkaterVisual_TB1.uasset`, `BP_SkaterVisual_TB1_P3.uasset`, `AS_SkaterTB1{v2,_P3}_CrossCheck_{Wind,Release,Whiff}.uasset`,
plus the rest of export49 (`Board_Hit`, the four heavy falls) and export48 / export50 (`Pass_Snap`, `Receive`, `Ready_Glide`): see the ledger rows.
The Blueprints also carry job 1 (the blade where A aims; below).

Tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, dev-b 9529103b). Rebuilt: PASS in 4.1 s. Headless editor pid 15860, port 8004, started 12:23.
No git commands. Game sound was already off before I started and is still off. `Slate.bAllowThrottling 0` while I work.

## Job 1: the blade where A aims (A-to-B-0037 item 13). Proven headless; GPU film pending (GPU-FREE.txt absent since 12:53)

### Why the socket missed A's point (measured, drill 21 on clay, every frame of the human's figure)
1. **The puck socket is not where the aim layer aims.** The aim layer was built to put the blade centre on the asked point, and it does (within 4 cm).
   E3 then moved `puck_socket` onto the blade's forehand face, which is 15 to 18 cm clockwise of that centre (to the skater's right) and 4 cm further out.
   Result: every aim landed about 6 degrees to the right at 1.6 m (8 degrees at 1.2 m): +45 went to 38.6, -30 to -34.9, +70 to 64.2. Steady error **17 cm**
   (n = 2287 frames, max 19.5). A's 22 / 37 degree reading at 1.2 m is this same offset.
2. **The trail in a hard cut** came from a spring on the aim angle in `SK2Tick` (critically damped, 16.7 rad/s). It added about 17 frames of lag on
   top of A's own 15-degree slew. Measured: **75 to 82 cm** worst per cut over 5 cuts, above 25 cm for 12 frames.
3. **The figure turned one frame ahead of the pawn.** `UpdateMotion` set the mesh's yaw in WORLD space; the pawn's own turn later in the same frame
   carried it once more. In a 5 rad/s turn the whole body stood 4.8 degrees ahead of the pawn, so 13 to 15 cm on the socket in every turning frame.
4. Headless the old `SK3Fix` correction never ran (it needs `WasRecentlyRendered`), and when rendered it aimed the blade centre, not the socket.

### What changed (`BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3`, Blueprint DSL; scripts `e4bp.mjs`, `e4um.mjs`)
- `SetStickAim(AngleDeg, ReachM)`:
  - the sent angle is clamped to +-70 (it was -63..+70); the reach to 1.0..1.6;
  - shown with a one-frame lead (0.65 of the last step, A's own 35 % approach), because the pose is drawn before the next step;
  - never more than 15 degrees a frame;
  - the blend space gets the target minus a learned map offset (`AimOffA`, `AimOffR`). This is where `puck_socket` really stood against the input
    that drew it. It is learned in `SK2Tick` (0.3 a frame), only while the aim layer is the whole pose, no montage, no hold, no pass charge and
    the aim steady. It needs no GPU;
  - past the blend space's ends (-63 / +70) the rest becomes a yaw of the stick about its grip (`AimOver`, at most 12 degrees).
- `SK2Tick`: the spring is gone; it learns the offset.
- `SK3Fix`: the blade height (pitch) is kept. Its yaw term is now `AimOver` (not gated on rendering); its slide term is off (the loop does that job).
- `UpdateMotion` (the skating branch): the mesh yaw is set relative to the figure (the lift's turn only), not in the world. The down and get-up
  branches are unchanged.
- New real variables: `AimSent`, `AimRSent`, `AimStep`, `AimRStep`, `AimLead`, `AimDes`, `AimOffA`, `AimOffR`, `AimOver`.

### Proving numbers (headless)
| Measure | Before | After |
|---|---|---|
| Drill 21 clay, steady carry, socket to A's point | 17.0 cm mean, 19.5 max (n 2287) | **0.4 cm mean, 1.5 max** (n 2223) |
| Per aim: -30 / +45 / +70 at 1.6 m | -34.9 / 38.6 / 64.2 degrees | **-30.0 / 45.0 / 70.0** |
| Hard cuts, worst per cut | 75 / 81 / 81 / 82 / 81 cm | 43 / 20 / 13 / 13 / 21 / 19 / 13 cm |
| Cut frames over 10 cm | about 15 per cut | 2 to 3 (the first frames of a slew: see below) |
| Test figures striding 9.7 m/s, -70..+70 x 1.2..1.6 m, cel and clay | 13 to 22 cm | **0.3 to 0.6 cm mean** on every step (also -70 and +70) |
| Carrying at 1 m/s | | 0.0 cm mean, 0.4 max |
| Not carrying (ready pose, look at the puck), 1 and 9.7 m/s | (30 to 80 cm) | 2.3 to 2.6 cm mean, 22 max on a step change |
| Mesh against the pawn in a turn | 4.8 degrees ahead | 0.0 |
| Shown angle step per frame | | p99 10.2, max 20 degrees (the 20 comes from a knockdown frame, not from the aim) |
| Seeded clay match (clayA, scripted human, hit-stop 3) | 738 sim lines | **738 = 738, identical**, JAMSIM 7200 `1d1fb181b85f1d4d`; 0 Blueprint errors, 0 `Error:` |

- **Left over:** on the first frame of any new slew the socket is one frame behind (up to 15 degrees: 43 cm at 1.6 m). The figure's animation
  updates before A's step in the same frame, so a 15-degree-per-frame target can only be shown a frame late. From the second frame on it is
  within 5 cm. The fix would be tick order (the figure animates after the game mode), which shifts every take one frame earlier. That is not
  done: it is a decision for device A / the operator.
- Reach 1.0 m on the cel figure: the socket cannot come nearer than 1.12 m (the blend space ends at 1.0), so 12 cm out. Clay: 6 cm. A sends 1.2 or more.

## Job 2: export48 (Pass_Snap reworked, lane PS2): imported, wiring checked headless; GPU film pending
- Re-imported by name over `AS_SkaterTB1_P3_Pass_Snap` and `AS_SkaterTB1v2_Pass_Snap` (animation-only, own skeletons): 0.15 s -> **0.4333 s**.
- Wiring: unchanged (P2's start at frame 5, 2-frame blend, BlendOutTriggerTime 0.0 plays to the end).
- Seeded match: 46 passes. **37 played from frame 5 to the end** (21 frames). The other 9 were cut cleanly by the next call: hit reactions 4,
  stops 2, a fall 1, a knockdown 1, a receive 1.
- Legs-glide risk (lead's note): to be judged on the GPU film.

## Not done yet (as of 14:30; this hand-back was sent early for the urgent cross-check)
- Job 1 is not filmed on the GPU. The drill-based rig failed at 13:06 to 13:31 (a marker spawn and a stray script); the replacement rig (`fa.py`,
  a test figure, a debug sphere at A's point) is proven headless: old trail up to **140 cm** in a 90-degree cut, new **7.8 cm max**; steady old 14 to 19 cm, new 0.2 to 0.5 cm.
- Not filmed: Pass_Snap tap/full (the legs-glide check), the whiff, a board hit, heavy vs middle fall, receive, ready glide.
- export50 group 2 (AimRdy holds) not imported.
- Seeded clay match after every block: 738 = 738 sim lines identical, 0 Blueprint errors (runs e4a, e4b, e4c, e4d).

## Ledger rows (replace these files' earlier rows)
| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 12.37 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | as lane P2's row, and (lane E4): SetStickAim without the spring: the sent angle (clamped +-70) is shown with a one-frame lead (0.65 of the last step) and never more than 15 deg a frame, the blend space fed the target minus a learned map offset AimOffA/AimOffR (where puck_socket stood against the input that drew it, learned in SK2Tick at 0.3 a frame while the aim layer is the whole pose and the aim is steady), the rest past -63/+70 as a stick yaw AimOver (at most 12 deg) in SK3Fix, whose slide term is off; UpdateMotion sets the skating mesh yaw relative (the figure no longer turns one frame ahead of the pawn); new PlayCrossWhiff() (CrossCheck_Whiff from Wind frame F-10 / Release F-3, 3-frame blend, from 0 with 4, plays to its end) and PlayWhiff(int32 Kind) (0 PlayCheckWhiff, 1 PlayCrossWhiff); HoldTick stops CrossCheck_Wind at or past contact while the hold is on (skipping the hold-in) and stops a stop take under a hold, keeps the ABP bHoldOut false (no gloves over the helmet on release); StopTick starts no stop take while bHoldOn; new real variables AimSent, AimRSent, AimStep, AimRStep, AimLead, AimDes, AimOffA, AimOffR, AimOver (12966531 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 12.57 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-15 | B | y | as lane P2's row, and (lane E4): SetStickAim without the spring: the sent angle (clamped +-70) is shown with a one-frame lead (0.65 of the last step) and never more than 15 deg a frame, the blend space fed the target minus a learned map offset AimOffA/AimOffR (where puck_socket stood against the input that drew it, learned in SK2Tick at 0.3 a frame while the aim layer is the whole pose and the aim is steady), the rest past -63/+70 as a stick yaw AimOver (at most 12 deg) in SK3Fix, whose slide term is off; UpdateMotion sets the skating mesh yaw relative (the figure no longer turns one frame ahead of the pawn); new PlayCrossWhiff() (CrossCheck_Whiff from Wind frame F-10 / Release F-3, 3-frame blend, from 0 with 4, plays to its end) and PlayWhiff(int32 Kind) (0 PlayCheckWhiff, 1 PlayCrossWhiff); HoldTick stops CrossCheck_Wind at or past contact while the hold is on (skipping the hold-in) and stops a stop take under a hold, keeps the ABP bHoldOut false (no gloves over the helmet on release); StopTick starts no stop take while bHoldOn; new real variables AimSent, AimRSent, AimStep, AimRStep, AimLead, AimDes, AimOffA, AimOffR, AimOver; on the AS_SkaterTB1_P3_ takes (13175724 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Whiff.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/CrossCheck_Whiff.fbx; gate passed) NEW take CrossCheck_Whiff, 16 keys: a cross-check that met nobody from the miss frame (its frame 0 is CrossCheck_Wind frame 10), caught with the blade in front from 8, the stance at 15; played by PlayCrossWhiff / PlayWhiff(1) by name on SKEL_SkaterTB1_P3 (lane E4 import) (133069 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Whiff.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/CrossCheck_Whiff.fbx; gate passed) NEW take CrossCheck_Whiff, 16 keys: a cross-check that met nobody from the miss frame (its frame 0 is CrossCheck_Wind frame 10), caught with the blade in front from 8, the stance at 15; played by PlayCrossWhiff / PlayWhiff(1) by name on SKEL_SkaterTB1v2 (lane E4 import) (133051 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Wind.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/CrossCheck_Wind.fbx; gate passed) take CrossCheck_Wind, 33 keys: both arms forward at shoulder height on frames 10 to 18 (contact 10), the stance from 26; PlayCrossCheck(N) from 10-N unchanged (contact on call+4/8/12/14 proven) by name on SKEL_SkaterTB1_P3 (lane E4 import) (160570 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Wind.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/CrossCheck_Wind.fbx; gate passed) take CrossCheck_Wind, 33 keys: both arms forward at shoulder height on frames 10 to 18 (contact 10), the stance from 26; PlayCrossCheck(N) from 10-N unchanged (contact on call+4/8/12/14 proven) by name on SKEL_SkaterTB1v2 (lane E4 import) (160542 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Release.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/CrossCheck_Release.fbx; gate passed) take CrossCheck_Release, 25 keys: the new punch pose on frames 3 and 11 (contact 3); the call unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (150966 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Release.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/CrossCheck_Release.fbx; gate passed) take CrossCheck_Release, 25 keys: the new punch pose on frames 3 and 11 (contact 3); the call unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (150939 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/Board_Hit.fbx; gate passed) take Board_Hit, 31 keys: a shoulder into the glass, the blade on the ice in every frame; PlayBoardHit unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (167554 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/Board_Hit.fbx; gate passed) take Board_Hit, 31 keys: a shoulder into the glass, the blade on the ice in every frame; PlayBoardHit unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (167527 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/Fall_Heavy.fbx; gate passed) take Fall_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later (17 after play starts), one small bounce; the calls unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (165528 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/Fall_Heavy.fbx; gate passed) take Fall_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later (17 after play starts), one small bounce; the calls unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (162139 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/Knockdown_Back_Heavy.fbx; gate passed) take Knockdown_Back_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (167968 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/Knockdown_Back_Heavy.fbx; gate passed) take Knockdown_Back_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (164579 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/Knockdown_Left_Heavy.fbx; gate passed) take Knockdown_Left_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (174688 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/Knockdown_Left_Heavy.fbx; gate passed) take Knockdown_Left_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (172427 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export49/p3_clay (takes/Knockdown_Right_Heavy.fbx; gate passed) take Knockdown_Right_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (179188 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane HX2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export49/p2_cel (takes/Knockdown_Right_Heavy.fbx; gate passed) take Knockdown_Right_Heavy, 35 keys: thrown higher and longer, the first landing 3 frames later; the calls unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (179167 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pass_Snap.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane PS2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export48/p3_clay (takes/Pass_Snap.fbx; gate passed) take Pass_Snap reworked, 27 keys (0.4333 s): release at 5 unchanged, the finish out ahead-left held 11 to 19, the carry at 26; PlayPass from frame 5 plays it to its end (37 of 46 passes in the seeded match; the rest cut cleanly by the next call) by name on SKEL_SkaterTB1_P3 (lane E4 import) (160634 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pass_Snap.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane PS2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export48/p2_cel (takes/Pass_Snap.fbx; gate passed) take Pass_Snap reworked, 27 keys (0.4333 s): release at 5 unchanged, the finish out ahead-left held 11 to 19, the carry at 26; PlayPass from frame 5 plays it to its end (37 of 46 passes in the seeded match; the rest cut cleanly by the next call) by name on SKEL_SkaterTB1v2 (lane E4 import) (162340 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Receive.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane HB2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export50/p3_clay (loopcut file; gate passed) take Receive, 9 keys: the blade out to the forehand at 2 (contact), the cushion 2 to 6, settled at aim +34 deg by 8; PlayReceive unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (141412 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Receive.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane HB2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export50/p2_cel (loopcut file; gate passed) take Receive, 9 keys: the blade out to the forehand at 2 (contact), the cushion 2 to 6, settled at aim +34 deg by 8; PlayReceive unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (131341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Ready_Glide.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane HB2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export50/p3_clay (loopcut file; gate passed) take Ready_Glide, 120 keys loopcut: the deeper ready crouch (pelvis 36 deg forward, 9 to 10 cm lower); in BS_<style>_SkateReady unchanged by name on SKEL_SkaterTB1_P3 (lane E4 import) (217069 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Ready_Glide.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane HB2) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export50/p2_cel (loopcut file; gate passed) take Ready_Glide, 120 keys loopcut: the deeper ready crouch (pelvis 36 deg forward, 9 to 10 cm lower); in BS_<style>_SkateReady unchanged by name on SKEL_SkaterTB1v2 (lane E4 import) (217035 bytes) |
