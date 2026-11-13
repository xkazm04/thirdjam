# SK12 report: the tackling takes (SKF export14) and the Blueprint changes for them (device B)

Lane SK12 (the SK8 to SK11 lane), 2026-11-13, 10:47 to 11:40 machine clock. Tree `C:/Users/kazda/jam/wt/b-vfx` (branch `lane/b-vfx`, on d6f5bfb). Editor port 8000, mine: pid 35300,
10:47 to 11:35, left through `ExitEditor` (process gone, dirty lists empty just before). No build, no cook, no git write. `jam.Cut` back to `hockey.regular`, `jam.ScriptHuman` 0;
`/Game/Maps/Rink` never saved; no `_Scratch/` or `_SK8TMP/` left. Port 8002 and `wt/b-a45` never touched.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk12` (scripts, `img/`, `logs/`, `bk/` the 144 takes and two Blueprints as committed, and the Blueprints' text before each change).
Export13 went in first, then export14 as told. **The saved state is export14** for every take that differs from export10, plus the two `_B` takes.

## Verdict per item

| # | Item | Verdict |
|---|---|---|
| 1 | Import by take name on both figures | **pass**: night 74 of 74 takes equal export14, clay 46 equal and the 28 aim holds kept (export9); 2 new takes per figure |
| 2a | The check is seen | **pass after one more fix**: priority 7 and blend-in 0.03 s were not enough; `Board_Pin` replaced the lunge in the frame it started. Fixed in `SetBoardPin`; then 2 of 2 |
| 2b | The fall plays as built | **pass**: rate 1.0, held on the lying pose, get-up from frame 46, ends on the release |
| 2c | Spin variants | **pass**: one real spin (`Knockdown_Left_B`) captured in a match |
| 2d | Cooldown and priorities | one thing clearly wrong, fixed (the pin over the check). Nothing swallows a knockdown |
| 3 | Real matches, night and clay | **pass**, thin on knockdowns: this sim gives 1 to 2 per 100 s. **No human check** (the scripted human cannot check) and **no knockdown from behind** |
| 4 | Faults with export14 | arm swing at frame 2 still large on the right side (130 deg); skate turn fixed (28 to 29 deg); **the stick goes through the boards** in a knockdown at the wall |
| 5 | The effect over the hit | `NS_check` (the star) and `NS_knockdown` (the puffs); they cover the figures for about 12 frames |
| + | Faceoff (export14) | **pass**: nothing stands up behind the back; knee spread 44.4 / 46.3 cm; `Faceoff_Draw` plays and the draw is won as before |

## 1. The import

- **Route:** export14 imported once per style as scratch assets on the existing skeleton; every existing take that differed had its keys copied in by take name through the animation data controller; the two new takes were moved in as new assets; scratch deleted. I used this route for all of them because export13/14 have 74 takes: the two new ones shift the order, so the stored index of every take after `Knockdown_Left` would fetch a neighbour through the menu reimport.
- **Changed, per figure (22):** `Board_Hit`, `Check`, `Check_Deliver_Left`, `Check_Deliver_Right`, `Check_Hit_Back`, `Check_Hit_Left`, `Check_Hit_Right`, `Fall`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Right`, `Stumble`, `Stumble_Short`, `Trip`, `Faceoff_Draw`, `Faceoff_Ready`, `Idle_Faceoff`. Counted by comparison with this tree, not from a list: 19 takes differed at export13 (the lane's 18 against export12 plus `GetUp`), and export14 changed 11, of which the three faceoff takes are new to the set.
- **New, per figure (2):** `AS_SkaterTB1v2_Knockdown_Left_B`, `_Knockdown_Right_B`; `AS_SkaterTB1_P3_Knockdown_Left_B`, `_Knockdown_Right_B` (26 frames each).
- **Proof, run last:** export14 imported again and compared by name, 24 bones, every second frame (every frame when copied): night **74 of 74 equal**; clay **46 equal, 28 aim holds different** (export9's, as ordered), nothing to copy, largest difference 0.0.
- **After it:** 735 assets under `/JamAssets/Skaters/` load, 735 valid, none missing; 16 Blueprints and AnimBPs `BS_UP_TO_DATE`; 16 blend spaces full.
- **Standing warning:** for these takes the stored import path and index no longer describe the keys. Re-import them by name only.

## 2. Blueprint changes (`BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3`, the same in both)

| Where | Before | After |
|---|---|---|
| `PlayCheck`, both calls | `PlayAct <take> 4 1.0` (blend-in 0.08 s) | `PlayCut <take> 7 0.0 0.03` (priority 7, blend-in 0.03 s) |
| `PlayDown` | `DownHold` = length - 0.08 s | length - 0.004 s |
| `ReceiveTick`, the hold test | position >= `DownHold` | position + `DeltaSeconds` x `DownRate` >= `DownHold` |
| `KnockDown`, the fall's rate | 0.95 + 0.2 x hit strength | 1.0 (the speed-up for short knockdowns is kept) |
| `KnockDown`, new variable `bSpin` | | true when hit strength >= 0.95 or this figure's knockdown count mod 3 is 1 |
| `KnockDown`, the two side falls | `Knockdown_Left` / `_Right` | the `_B` take when `bSpin`, else as before |
| `SetBoardPin` on | plays `Board_Pin` always | only if no montage plays or the running priority is 6 or less |
| `SetBoardPin` off | stops the montage | only if the running priority is 6 or less |

- **Why the tick test changed:** a montage that reaches its end stops and the figure would stand up for the 20 lying frames. The fall is now paused on the last tick before its end, at any frame rate. At 60 fps that is frame 25 of 26; SKF's falls hold one lying pose from frame 18 to 26, so it is the built pose.
- **Why `SetBoardPin` changed (item d):** bots check out of board pins. In the seed-2 match the pin-on call came in the same frame as `PlayCheck` and replaced the lunge (0 of 2 lunges seen); pin-off three frames later would also have stopped it. After the change: 2 of 2.
- **Spin rule:** the 2nd, 5th, 8th ... knockdown of a figure, or any hit of strength 0.95 or more, when it comes from the side. It reads `HitStr` and the figure's own counter only; nothing the sim reads.
- **Not changed:** `BP_SkaterVisual_TB2*`, `_P4Lab`, both AnimBPs, the anim graph.
- **Side effect to know:** a check started out of a pin replaces the pin loop; the loop does not come back after the lunge.

## 3. Real matches

`all_star`, scripted human. This tree's sim gives far fewer knockdowns than `dev-b-a45`'s did: **seed 1: one in 100 s; seed 2: two in 100 s.** All three are of the human by a bot, from the right, the left, the left.

**Knockdown timing** (`knockdown_frames` 90 every time; Blueprint log lines and a recording of every rendered frame):

| Match | Sim frame | Fall take | Fall rate | Fall reaches | Lying | Get-up | Get-up end minus release |
|---|---|---|---|---|---|---|---|
| night, seed 1, undisturbed | 1006 | `Knockdown_Right` | 1.0 | frame 25, held | 22 frames | `GetUp_Side_Right`, frame 47 to 90.0, rate 1.0 | 0.000005 frames |
| clay, seed 1 | 1006 | `Knockdown_Right` | 1.0 | 25.3 | 21 | frame 46.6 to 90.0, rate 1.0 | +0.2 (my captures paused it) |
| night, seed 2 | 1279 | `Knockdown_Left` | 1.0 | 25.1 | 21 | `GetUp_Side_Left`, 46.3 to 90.0, rate 1.0 | -0.5 (paused) |
| night, seed 2 | 1396 | `Knockdown_Left_B` (spin) | 1.0 | 25.1 | 21 | `GetUp_Side_Left`, 46.3 to 90.0, rate 1.0 | +0.5 (paused) |

Target 26 at 1.0 / about 20 / 44 / 0: met (the fall's last tick is frame 25, the same pose).

**The attacker's lunge:**

| | Lunge seen | `PlayCheck` to hit | Frame of the lunge at the hit | Centres apart |
|---|---|---|---|---|
| seed 1 (night and clay), priority and blend-in changed | yes | 1 frame | 1 | 80 to 96 cm |
| seed 2, two knockdowns, before the pin fix | **no, no** (`Board_Pin` played instead) | 3, 4 | | 82 cm |
| seed 2, the same two, after the pin fix | **yes, yes** | 3, 4 | 3, 4 | 80, 85 cm |

All attackers are bots. **The scripted human never checks** (its script has no check), so there is no press-to-hit number for the human.

**The v4 calls and the takes they play** (`JAMVIS` lines of the sim against the figures' own lines; none unpaired):

| | night, seed 1 (100 s) | clay, seed 1 (100 s) | night, seed 2 (100 s) |
|---|---|---|---|
| `PlayHitReaction` | 77 = 52 played (`Check_Hit_Left` 27, `_Right` 25) + 25 skipped | 77 = 52 (27 + 25) + 25 | 73 = 56 (`_Back` 6, `_Left` 24, `_Right` 26) + 17 |
| `PlayBoardHit` | 30 = 18 `Board_Hit` + 12 skipped | 32 = 19 + 13 | 39 = 28 + 10 skipped + 1 refused (its own check was playing) |
| `SetBoardPin` on / off | 12 = 12 `Board_Pin`; 12 = 11 + 1 ignored | the same | 12 = 12; 12 = 10 + 2 ignored |
| `PlayStickHit` | 45 = 18 (`Lift_Hit` 12, `Poke_Hit` 6) + 27 skipped | 47 = 19 (12 + 7) + 28 | 35 = 12 (9 + 3) + 23 |
| Knockdowns | 1 | 1 | 2 |

"Skipped" is the 0.6 s rule. The clay match logs `JAMVISUAL humanoid_b1 tier=p3_clay class=.../BP_SkaterVisual_TB1_P3_C`: the game selects it. No runtime error line in any match.

**Faceoff (export14), the human at the faceoff in the real match:**

| | Night | Clay |
|---|---|---|
| Knee spread | 44.4 cm | 46.3 cm |
| Top hand: in front of the pelvis / height | 10.6 cm / 86 cm | 11.8 cm / 72 cm |
| Right shoulder height / head height | 99 / 106 cm | 94 / 100 cm |
| Blade centre above the ice | 5.2 cm | 5.2 cm |

In the stills nothing stands up behind the back; the top glove sits at the right hip. `Faceoff_Draw` starts on both centres at each drop (frames 91 and 2192 in the clay match); the human wins both, as it did before.

## 4. Faults, export14 (test figures, every rendered frame, scaled to 60 Hz; close range from the real clay knockdown)

- **Arm swing at frame 2 of the falls** (upper arm, one frame): `Knockdown_Back` 53 deg; `Knockdown_Left` 42; **`Knockdown_Right` 130 to 132**, then 81 to 85; `Fall` 48 (night) and 111 (clay); `Knockdown_Left_B` 66; `Knockdown_Right_B` 123. Hands 38 to 79 cm in that frame. The left side is tamed, the right side is not.
- **Standing hits:** the stick turns 43 to 58 deg a frame at frames 2 to 3 and 73 to 130 at frames 4 to 5; hands 36 to 66 cm a frame; the body is thrown 30 cm and comes back. In the strip it reads as a stagger with the arms flung wide. `Board_Hit`: upper arm 60 deg (night) / 137 (clay) at frame 3.
- **The spin:** the shoulders turn up to 75 to 79 deg in one frame at frame 3 (snap and turn together); I did not separate the plain spin rate. In the real strip the victim is on the ice by +9 like the plain fall; the turn is hard to tell from the game camera.
- **Side get-ups, skate turn:** 28 to 29 deg a frame at most (was 42 to 55 with export12, 75 to 79 with export11). `GetUp_Front` 25, `GetUp` 29 to 30.
- **Check:** the stick turns 32 to 34 deg at frame 2, then 16 a frame; hands 48 to 53 cm at frame 2.
- **Side lying pose against the ice** (`sk12-lying-close`): arm and hip rest on the ice; no sinking visible. The lowest arm joint centre is 9.6 to 10.7 cm above the ice (7.6 to 8.3 with export12).
- **The 30 cm throw at the boards:** the real knockdown is 40 cm from the end boards and the throw runs along the wall. The body stays inside. **The stick does not: its shaft and blade show through the wall from outside** (`sk12-lying-close`, last tile). A throw straight at a wall did not happen in these matches: not verified.

## 5. The effect over the hit

Two Niagara systems of B's fire on the same frame: **`NS_check`** (event `check`: ice chips, the team-coloured ring on the ice, the white "clack star" at 1.4 m) and **`NS_knockdown`** (event `knockdown`: snow spray and puffs). In the strips the star is there from the hit to +12 (5 tiles), over the heads; the puffs cover the victim's legs and trunk from the hit to +12 and thin out by +18 to +21; the ring dots last to +12. So the lunge (frames 1 to 4) and the throw (frames 2 to 11) happen under them. Not changed.

## Files changed and created (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1.uasset` | 2,804,420 | 2,849,176 |
| `BP_SkaterVisual_TB1_P3.uasset` | 2,862,270 | 2,907,210 |
| 22 changed `AS_SkaterTB1v2_*` takes (total) | 3,725,232 | 3,651,472 |
| 22 changed `AS_SkaterTB1_P3_*` takes (total) | 3,970,487 | 3,895,127 |
| new: `AS_SkaterTB1v2_Knockdown_Left_B`, `_Right_B` (total) | | 316,191 |
| new: `AS_SkaterTB1_P3_Knockdown_Left_B`, `_Right_B` (total) | | 318,726 |

`git status`: the two Blueprints, 44 takes modified, 4 takes new. Nothing else under `Game/`. New outside `Game/`: this report and 18 files in `devlog/shots/`.

## Ledger rows

Replace:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 2.72 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lane SK12): check at priority 7 with 0.03 s blend-in, fall at rate 1.0 held on its last pose, spin take on every third side knockdown or a hit of 0.95, a board pin no longer replaces a running check (2849176 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 2.77 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling as BP_SkaterVisual_TB1 (lane SK12) (2907210 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (171296 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (182319 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check_Deliver_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (179694 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check_Deliver_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (182529 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (167712 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (174704 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (174683 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Draw.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Faceoff_Draw by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (151764 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Ready.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Faceoff_Ready by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (154638 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (168401 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (189357 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (191277 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (196848 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (199769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Idle_Faceoff.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Idle_Faceoff by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (235257 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (168341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (172819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (172261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Stumble by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (158487 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Short.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Stumble_Short by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (162642 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Trip by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (174746 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (162117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (173072 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check_Deliver_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (170387 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check_Deliver_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (173282 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (155461 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (163511 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (163464 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Draw.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Faceoff_Draw by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (140676 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Faceoff_Ready.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Faceoff_Ready by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (145361 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (156566 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (175784 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (177794 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (186134 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (186181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Idle_Faceoff.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Idle_Faceoff by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (225995 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (156482 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (161716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (161255 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Stumble by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (147206 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Short.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Stumble_Short by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (151540 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Trip by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only) (lane SK12) (163769 bytes) |
```
New:
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only); spin variant, new asset (lane SK12) (159422 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p3_clay loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; the stored import path and index are older: re-import by name only); spin variant, new asset (lane SK12) (159304 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only); spin variant, new asset (lane SK12) (158148 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export14/p2_cel loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; the stored import path and index are older: re-import by name only); spin variant, new asset (lane SK12) (158043 bytes) |
```
(The `BP_SkaterVisual_TB1` row says shipped `n` today; I wrote `y` since it is the default figure. Check it.)

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with a `.txt`)

- `20261113-113628-B-sk12-tackling-clay.png` (operator sheet)
- `20261113-113629-B-sk12-knockdown-clay.png`
- `20261113-113629-B-sk12-knockdown-night.png`
- `20261113-113629-B-sk12-knockdown-spin.png`
- `20261113-113630-B-sk12-standing-hit.png`
- `20261113-113630-B-sk12-board-hit.png`
- `20261113-113630-B-sk12-check-lunge.png`
- `20261113-113630-B-sk12-lying-close.png`
- `20261113-113631-B-sk12-faceoff-behind.png`

All from real matches and the real game camera, the match frozen for each capture; the strips start on the frame the figure reports the hit (not 6 frames before). The close-ups are the same frozen frames with the camera moved.

## What I could not verify

- **A cook.** Not run (yours).
- **The human's check** (press-to-hit delay, a hit from behind by the human): the scripted human does not check.
- **In a real match:** `Knockdown_Back`, `Knockdown_Right_B`, `Fall`, `Fall_Forward`, `Trip`, `GetUp`, `GetUp_Front`. They were driven on test figures only (numbers in section 4). `Stumble` and `Stumble_Short` were not looked at.
- **A full match after the pin fix:** it was checked on the two seed-2 knockdowns (26 s), not over 100 s; the reaction counts above are from before it. The seed-1 knockdown was not re-run after it.
- **A throw straight at a wall.**
- **Motion at speed by eye:** strips, close stills and per-frame numbers only.
- **`BP_SkaterVisual_TB2*`:** untouched, so the generated figures still refuse the check behind a pin and still stop their falls early.

## Decisions for you

1. **The stick through the boards** in a knockdown at the wall: accept, or ask device A for more room at the wall, or SKF for a stick held in during falls.
2. **The right-side fall's arm swing** (130 deg in one frame, `Knockdown_Right` and `_Right_B`): back to SKF, or accept as the blow.
3. **The spin rule** (every third side knockdown of a figure, or strength 0.95): keep, or strength only.
4. **`NS_check` / `NS_knockdown`** cover the first 12 frames: thin them or lift the star, if the lunge and the throw should be seen.
5. **Knockdown rate:** 1 to 2 per 100 s in this sim; if tackling is the feature, device A may want bots to check more, and the human needs a check to test.
6. The same Blueprint changes for the generated figures (`BP_SkaterVisual_TB2`, `_TB2_P3`), if they stay in the game.
7. Run the cook-only check; paste the ledger rows (2 Blueprints, 44 takes replaced, 4 new).
