# Lane SK29 report (phase A): the pin's victim on frame 16, export34 and export35 by name, the get-up as played, the dash loop's lost frame

Device B, tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, on 05ab94a7). No commit, no push. Assets changed only under `Game/Plugins/JamAssets/Content/Skaters/`: 30 changed, 2 new.

A1 to A4 ran in an editor without a GPU (`-nullrhi -unattended`, port 8004, pid 39752, 0 `LogD3D12RHI` lines), left at 06:06. I saw the GPU file at 06:03 (it says free since 05:45), so the lead's export35 addition, the last two changes to the get-up's blend and the seeded match ran in the GPU editor of phase B (pid 40068, left through `ExitEditor` at 06:41 and gone). Phase B is in `status\FILM2-report.md`.

**The played get-up now rises as the take does, the dash loop has its 24th frame back, and the pin's roller is on the shin on frame 16. One find is bad news: the pin's victim falls onto the attacker.**

## Verdict per job

| Job | Verdict | The number that decides it |
|---|---|---|
| A1. The pin's victim on frame 16 | **Measured: the contact is right, the landing is not** | roller to the shin's bone line on frame 16: 4.8 / 6.7 cm (clay, left / right), 8.2 / 8.4 cm (night). The victim's head ends 20 cm (clay) and 26 cm (night) ahead of the attacker's root: on his skates |
| A2. Export34's import-now list | **Done** | 11 takes re-keyed and 1 new per style; by themselves the get-ups rise as the lab gives them |
| A3. The get-up as played | **Done** | `GetUp` 9.7 / 9.3 cm a frame before, 5.8 / 5.3 after (clay / night): the take's own. End minus release 0.0 in 29 of 29 test get-ups and 3 of 3 in the match |
| A4. The dash loop's 23 frames | **Found and fixed** | the loop's wrap dropped one pose a cycle; with the repeated last key the cycle is 24 frames and repeats to 0.00 cm |
| A5. The dash's third export | **Not used: not closed** | `READY.txt` is still 03:07:20; the folder holds files of 03:26 for 13 takes against its list of 10 |
| The lead's addition: export35 | **Done** for the four cleared takes; its new `GetUp` filmed, not imported | quick and snap from the back as played: 8.1 and 7.7 cm a frame (the quick was 21.8) |

The seeded clay match (seed 1, to sim frame 7200, on the GPU, twice on the last two states): 0 Blueprint errors, 4 goals, the sim's 6 knockdown and stumble lines identical to SK28's, and of the figure's 232 lines only the three get-ups' start and release lines differ, as intended (start 0.0 a frame earlier; end minus release 0.0, 0.0, 0.0).

## A1. The pin's victim, called on the attacker's frame 16

Re-run with the call moved, the victim at the lab's spacing (0.96 m clay, 1.04 m night), facing the attacker. Nearest point of the roller to the victim's shin line (knee to ankle joint; the shin's skin is about 6 cm from it):

| Attacker's frame | Called on 16: victim's frame | Roller to shin line, clay (left / right) | Night | Called on 14, clay |
|---|---|---|---|---|
| 14 | standing | 12.0 / 13.3 cm | 15.4 / 15.9 | 12.0 / 13.3 |
| 15 | standing | 6.5 / 7.8 | 9.4 / 9.8 | 16.7 / 12.7 |
| **16** | **2** | **4.8 / 6.7** | **8.2 / 8.4** | 33.8 / 23.8 |
| 17 | 3 | 15.9 / 11.5 | 18.2 / 14.0 | 40.1 / 32.3 |
| 18 | 4 | 34.5 / 25.1 | 40.1 / 30.4 | 39.0 / 31.8 |

- **On frame 16 the roller is on the shins and the legs leave on the next frame.** That part is right now.
- **The victim then falls forward onto the attacker.** `Fall_Legs_Front` takes the feet back and drops him on his front, and he is facing the attacker 1 m away. His chest joint passes 4.7 cm (clay) and 6.1 cm (night) inside the roller on the attacker's frame 22. He ends lying with his head 20 cm (clay) and 26 cm (night) ahead of the attacker's root, where the attacker's skates are.
- Film: `devlog\shots\20261114-063404-B-film2-pin.png`. The attacker ends standing on the lying victim.
- What would cure it is in "For the Blender set lanes" and "For the other device".

## A2. Export34, its import-now list by name

From `skf\export34\<style>\SK_Skater_<style>_loopcut.fbx` (gate PASS, 298 rows, 0 failing). Per style:

| Takes | Result |
|---|---|
| `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Fall`, `Fall_B`, `Poke`, `Check_Light`, `Check_Deliver_Right_Light`, `Check_Deliver_Left_Light`, `Lift_Light`, `Lift` | re-keyed, the same lengths |
| `Check_Deliver_Right_Light_Half` | **new asset: the tree had none.** No Blueprint plays a half-reach take, so it is not wired |
| The other 137 takes of the file | not touched |

The takes by themselves on a test figure (no blend), clay / night:

| Take | Head's largest rise in one frame | The lab's | Other |
|---|---|---|---|
| `GetUp_Front` | 6.0 / 6.4 cm | 6.0 / 6.4 | hips 5.8 / 6.1 (the lab: 5.8 / 6.1) |
| `GetUp_Side_Left`, `_Right` | 7.6 / 7.9 cm | 7.6 / 7.9 | lowest head in the beat 27.6 / 26.8 cm |
| `Check_Light` | - | - | hips ahead of the root at most 12.0 cm (the lab: 12) |
| `Lift_Light` | - | - | hips ahead of the root 0.0 cm |

**Then export35** (gate PASS, 298 rows), by name on both figures: `GetUp_Side_Left`, `GetUp_Side_Right` (the head higher in the beat), `GetUp_Quick`, `GetUp_Snap` (re-timed, the same 20 frames). Its new `GetUp` was not imported: it played in the film session only (`status\FILM2-report.md`).

For the film only: export34's nine middle and heavy takes were in the tree under a test name, clay only (`AS_SkaterTB1_P3_X34_<take>`). I deleted them after the film; the tree holds none of them now. The film's verdict on them is in `status\FILM2-report.md`.

## A3. The get-up as played

Two things held the start back, not one.

1. **The get-up was called one frame late and started on its frame 1.** The tick compares the time down with the start time; at the frame it should fire, the time down is 73.999996 frames against 74. So it fired a frame later and skipped ahead to frame 1. The pose then showed frame 0 for one frame too long and jumped two frames' worth.
2. **The 5-frame blend out of the lying loop** mixed the rising take with the still lying pose, then caught up.

Changes, both Blueprints:
- The timed get-up fires with half a frame of tolerance and never starts before frame 0.
- From the lying loop it starts with no blend (new variable `UpBlend`) for the four 36-frame get-ups and for the quick and snap get-ups from the back. The 5-frame blend stays for the wall get-ups, for early releases, and for the quick and snap get-ups from the front and the sides, which the lab has not re-timed.

**The four 36-frame get-ups, the head's largest rise in one frame as played after a full-length knockdown (clay / night):**

| Take | SK28 (export33, old start) | Before (export34's takes, old start and blend) | After | The take by itself |
|---|---|---|---|---|
| `GetUp` (back) | 9.7 / 9.3 cm | 9.7 / 9.3 | **5.8 / 5.3** | 5.8 / 5.3 |
| `GetUp_Front` | 12.3 / 10.8 | 6.0 / 6.4 | **6.0 / 6.4** | 6.0 / 6.4 |
| `GetUp_Side_Left`, `_Right` (export34) | 7.5 / 8.5 | 7.6 / 7.9 | **7.6 / 7.9** | 7.6 / 7.9 |
| the same with export35's takes | - | - | **6.5** (clay; not measured on night) | - |

- `GetUp`'s first five steps as played: 4.9, 5.3, 5.2, 4.9, 5.2 cm (clay). Before: 1.0, 4.5, 7.9, 9.7, 7.9.
- **The catch-up is gone in every joint, not only the head.** Largest move of any joint in one frame over the first six frames: sides 24.1 / 24.7 cm before, 17.4 / 17.5 after; back 11.2 / 11.0 before, 7.2 / 7.1 after.
- **No pop at the cut.** The frame after the cut is the take's own frame 1 to 0.00 cm in every joint (before, the blend left it 2.7 to 6.8 cm off). The largest joint move on the cut frame is 3.7 to 6.5 cm, at or under the pace of the frames that follow.
- **End minus release 0.0** in every timed get-up measured: 6 of 6 and 6 of 6 (clay and night test figures, 85 to 200 frames down), 10 of 10 and 7 of 7 in the later sessions, 3 of 3 in the match.

**The quick and snap get-ups, as played (clay):**

| Take | Before | After | Blend now |
|---|---|---|---|
| `GetUp_Quick` (back, 85 frames down) | 21.8 cm | **8.1** (export35's take: 8.1) | none |
| `GetUp_Snap` (back, 65 frames down) | not measured (the lab: 12.9 in the take) | **7.7** (export35's take: 7.7) | none |
| `GetUp_Quick_Side_Left` | not measured | 8.8 | 5 frames |
| `GetUp_Quick_Front` | not measured | **14.8** | 5 frames |
| `GetUp_Snap_Side_Left` | not measured | 7.4 | 5 frames |

One trial I took back: with no blend the old `GetUp_Quick` showed its own 25.7 cm step, worse than the blended 21.8. That is why the takes the lab has not re-timed keep the blend.

## A4. The dash loop's 23 frames

**The lost frame was the loop's wrap, and the feet's cycle was not intact.**

- The tree's `Dash_Loop` came from the loopcut file: 24 keys, frames 0 to 23, 23 intervals long. The engine's looping montage goes from the last key straight to the first, so one pose was dropped every cycle.
- Seen in the skates' steps of the take by itself: the first half-stride ends 9.3, 5.2, 7.3 cm; the second ended 9.3, 8.9: one step where there should be two.
- **Fix, one change per figure:** a 25th key equal to key 0 appended to the asset in the engine (what the lab's file without the cut holds). I did not take the file from the dash folder: see A5.

| | Before | After |
|---|---|---|
| Keys, length | 24, 0.3833 s | 25, 0.4 s |
| A frame against the frame 23 later, largest joint apart | 0.00 cm | 15.21 cm (clay), 16.43 (night) |
| A frame against the frame 24 later | 15.21 cm | **0.00 cm** |
| Three cycles in `PlayDash(82)` | 68 frames, the end 3 frames early | 71 frames, `Dash_End` on its frame |
| Skates' step into `Dash_End` | 15.2 / 16.4 cm | 9.3 / 10.3 cm |
| Skates' step onto `Stride` 34 | 1.8 / 3.2 cm | 1.8 / 3.2 cm |

`PlayDash` already counted 24 frames a cycle, so its timeline is now right without a change.

**The same is true of every loop imported from a loopcut file**: the lying loops have 108 keys and wrap in 107 frames. Their motion is small and their clean windows are 15 frames wide, so nothing shows; I did not change them.

## A5. The dash's third export: not used

- `sets-lab\dash\export\READY.txt` is 03:07:20, the list has 10 takes, and the style folders hold 15 files each written at 03:26 (the 10, plus `Dash_Carry`, `Dash_Wipeout_Light`, `Dash_Wipeout_Wall` and two loopcut files). List and folder do not agree, so nothing was taken.
- **A correction to my SK28 report.** I wrote that the dash export in the tree is the gate-passed loop 2. The files in that folder were rewritten at 03:26, after the READY and the gate of loop 2, and lane SK26 imported the dash set after that time. So the ten dash takes in the tree most likely come from the 03:26 files, which no READY or gate covers yet. Their lengths match loop 2's table and their joins measured clean, but the dash lane should say whether the 03:26 files of those ten takes differ from loop 2.

## What went wrong in the running of this lane

- **I saved 298 temporary import packages into the tree.** For the film I left the import's temporary folder (`Skaters/_SK8TMP`) in memory unsaved. At 06:31 my save helper wrote it to disk. The helper now skips that folder, and I removed the folder from disk after the editor closed; `git status` shows nothing under it.
- **`ExitEditor` refused once** (06:05) because those temporary packages were unsaved. I passed `bDiscardUnsavedChanges`; only my own temporary import packages were discarded.
- **One film session ran inside the first GPU match** (my mistake, 06:20). That match is discarded; the match was run again alone, twice, on the last two states of the Blueprint.

## Not verified

- **Night for export35's takes and for the quick get-ups.** The export35 and quick-get-up numbers are clay only.
- **The front and side quick and snap get-ups before this lane.** I have only their after numbers.
- **`Fall`, `Fall_B`, `Poke` against their old keys.** They were re-keyed as the lab asks; I did not measure the forearm's back-and-forth before and after.
- **The lying loops' 107-frame wrap** in play: reasoned from the dash loop's case, not measured.
- **Whether the dash takes in the tree equal loop 2's** (A5).
- **A match on the night level.**

## For the Blender set lanes

- **Weapons and dive:** the pin's victim. `Fall_Legs_Front` drops him toward the attacker. For the pin he needs a fall that goes away from the hit (his feet are taken from the front: `Fall_Legs_Back`'s direction, or `Trip` to the side), or a take in which he is thrown back over the roller.
- **Dash:** say whether the 03:26 files of the ten listed takes differ from loop 2's, and close the third export with a READY that lists all 13.
- **Dash:** send the loop as 25 keys (the repeated last frame kept). The engine needs it; the loopcut file loses a frame.
- **Figure lab:** the same for every loop: the engine's loop wants the file with the repeated last frame. The tree's lying loops are loopcut files and wrap in 107 frames.
- **Figure lab:** `GetUp_Quick_Front` still rises 14.8 cm in one frame as played. `GetUp_Late` and `GetUp_LateLong` are as you list them.
- **Figure lab:** `Check_Deliver_Right_Light_Half` is in the tree but no half-reach take is played by the engine.

## For the other device

- **`PlayWeaponHit(4, ...)` on the attacker's frame 16** is confirmed by a run: the roller is on the shins on that frame.
- **The pin's victim lands on the attacker unless the game moves one of them.** Until the take changes, give the victim the push the slide tackle needs: about a metre, away from the attacker.
- Nothing else changes in the calls. The get-up's timing as the game sees it is unchanged: it still ends on the release.

## Decisions for the operator

1. **The pin's victim:** push him away in the game, or wait for a fall that goes away from the hit. Still: `devlog\shots\20261114-063404-B-film2-pin.png`.
2. **The quick get-ups from the front and the sides** keep a 5-frame blend and the front one still rises 14.8 cm in a frame. Ask the lab to re-time them as it did the back ones, then drop the blend there too.
3. **The other loops' lost frame** (the lying loops wrap in 107 frames, not 108). Invisible today. Fix with the next re-import of each loop, or leave.

## The next three evolutions

1. **Every loop with its repeated last key**, and the start-frame rule's arithmetic checked against it. Proving numbers: each lying loop repeats to 0.00 cm after 108 frames; the cut frames the log prints equal the frames shown; end minus release 0.0 in a seeded match.
2. **The pairs as a test**: pin, mallet and slide tackle on two figures with the victim's push as the game will give it. Proving numbers: no victim joint inside the roller or the attacker on any frame; hips at least 80 cm apart when both lie.
3. **The get-up's first frame on early releases.** They still start two thirds in at double speed (the head rises 20 to 32 cm in a frame). Proving number: under 12 cm a frame on a release 50 frames early, with the get-up still ending on the release.

## Ledger rows

32 full rows in the ledger's 11 columns: the 2 figure Blueprints, 22 takes re-keyed from export34 (4 of them again from export35), 4 re-keyed from export35, 2 dash loops with their 25th key, 2 new takes. 30 replace a row already in `ASSETS.md`; 2 are new.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 9.40 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the pawn's PlayStumble covers every stumble (the check-bus read dropped), the stop driven by SetBraking's level and gated (7 m/s at the start, held 6 frames for a bot with 8 s between its stops, from the first frame for the human), lying loops from the frame the fall holds its end pose (lane SK24); +a plain collision gets no whip; the whip's two weights (WhipHeadW, WhipArmW); the check's and the cross-check's sizes (closing speed on the press, the Strength's band when the bus starts the take); the calls PlayDiveBlock, PlayDiveBlockFront, PlayDiveBlockHit, PlaySlideTackle, SetWeapon, PlayWeapon, PlayWeaponHit, PlayDash, SetBoostLevel, EndDash, PlayBoostPickup by name, riding the knockdown's machinery; the WeaponMesh component on stick_socket; the test trigger SK26Test (lane SK26); the timed get-up starts on its frame (half a frame of tolerance, start never under 0) and, from the lying loop, without a blend for the 36-frame get-ups and the quick and snap get-ups from the back (variable UpBlend; 5 frames kept for the other quick ones, the wall get-ups and early releases) (lane SK29) (9852699 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 9.60 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the stumble by the pawn's call only, the gated level-driven stop and the lying loops as BP_SkaterVisual_TB1 (lane SK24); +the whip's constant and weights, the check sizes, the eleven set calls, the WeaponMesh component and the test trigger as BP_SkaterVisual_TB1 (lane SK26); the timed get-up's start and blend as BP_SkaterVisual_TB1 (lane SK29) (10061596 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (167298 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export24/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export24/p2_cel loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (153423 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Poke.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export9/p2_cel loopcut (pass 9 ready stance), animation-only re-import in place on SKEL_SkaterTB1v2, same asset (lane SK8); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (151745 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (133202 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Right_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (133384 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Left_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (133371 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Lift_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (127866 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export20/p2_cel loopcut, take Lift by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17); re-keyed by name from skf/export34/p2_cel loopcut (lane SK29) (151941 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel and then export35/p2_cel loopcut: the hand-and-knee beat, the head higher in it (lane SK29) (171734 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p2_cel and then export35/p2_cel loopcut: the hand-and-knee beat, the head higher in it (lane SK29) (171781 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export35/p2_cel loopcut: re-timed, the same poses (lane SK29) (140791 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Snap by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export35/p2_cel loopcut: re-timed, the same poses (lane SK29) (140701 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Loop.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Loop (the loopcut file) by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); a 25th key equal to key 0 appended in the engine: the loop is 24 frames now, it was 23 (lane SK29) (139788 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right_Light_Half.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | skf/export34/p2_cel loopcut, take Check_Deliver_Right_Light_Half by name, a new asset on SKEL_SkaterTB1v2; no Blueprint plays it yet (lane SK29) (133450 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (180013 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export24/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export24/p3_clay loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (154853 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Poke.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export9/p3_clay loopcut (pass 9 ready stance), animation-only re-import in place on SKEL_SkaterTB1_P3, same asset (lane SK8); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (160952 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (133228 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Right_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (133410 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Left_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (133397 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Lift_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (127892 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export20/p3_clay loopcut, take Lift by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17); re-keyed by name from skf/export34/p3_clay loopcut (lane SK29) (161227 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay and then export35/p3_clay loopcut: the hand-and-knee beat, the head higher in it (lane SK29) (184560 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export34/p3_clay and then export35/p3_clay loopcut: the hand-and-knee beat, the head higher in it (lane SK29) (184601 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export35/p3_clay loopcut: re-timed, the same poses (lane SK29) (142809 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Snap by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export35/p3_clay loopcut: re-timed, the same poses (lane SK29) (142563 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Loop.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Loop (the loopcut file) by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); a 25th key equal to key 0 appended in the engine: the loop is 24 frames now, it was 23 (lane SK29) (139814 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right_Light_Half.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | skf/export34/p3_clay loopcut, take Check_Deliver_Right_Light_Half by name, a new asset on SKEL_SkaterTB1_P3; no Blueprint plays it yet (lane SK29) (133484 bytes) |
