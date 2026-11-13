# SK13 report: the scripted figures against device A's hit-stop and drills; export16 (device B)

Lane SK13 (the SK8 to SK12 lane), 2026-11-13, 11:53 to 12:36 machine clock. Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b`, 2d02014). Editor port 8002, mine: pid 27012 (11:53 to 12:28)
and pid 26996 (12:29 to 12:31, to play the two new takes); both left through `ExitEditor`, processes gone, dirty lists empty. **No Blueprint was changed or saved.** Animation assets only.
No build, no cook, no git write. `jam.Cut` back to `hockey.regular`, `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.hitstop` 3; `/Game/Maps/Rink` never saved; no scratch folder left. Port 8000 (`wt/b-vfx`, lane B30) never touched.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk13` (`bk/` the 148 takes as they were at the start).
Export15 went in first, then export16 as told. **The saved state is export16.** All strips and numbers below are with export16 unless said.

## Verdict per item

| # | Item | Verdict |
|---|---|---|
| 1 | Hit-stop against the timing | **partial**: the figures hold and the fall then plays 26 frames at 1.0, but **the get-up ends N frames before the release** (3 at the default, 6 at stop 6, 0 at stop 0). One line fixes it, on either side. The held pose is frame 0 of both takes, not a hit pose |
| 2 | Lying poses against the ice | **pass**: nothing of the body is under the ice with export16; the figure's root is at ice height; the stick lies on the ice along the body |
| 3 | The human's check | **found a fault**: while the check key is held the lunge is restarted every frame and stays on frame 0, so the human's lunge always starts after the hit. Press to hit: 1 to 10 frames (one 25). `Knockdown_Back`, `Fall` and the spin met a real match; **no plain side fall and nothing on the right side occurred** |
| 4 | Does a side fall read as from the side | **not answered with a plain side take**: both side hits in the match played the spin. See below |
| 5 | Export16 | **pass** for the import (equal by name, two new takes in, unwired). Spin re-shot. **The knockdown at the boards was not re-shot** (none happened there) |
| 6 | `Board_Pin` still | **pass** |

## 1. The hit-stop (drill 6, the human checks pro bots; the Blueprint's own log lines and a recording of every rendered frame)

| `jam.hitstop` | Sent frames | Figures hold | Fall starts moving | Fall (26 frames) reaches its end | Get-up | Get-up end minus release |
|---|---|---|---|---|---|---|
| 0 | 90 | not at all | frame 1 | world frame 25 | from 47, 44 frames at 1.0, ends 90.0 | **0.000005** |
| 3 (default) | 87 | frames 0 to 3, both figures | frame 4 | world frame 28 | from 43 to 44, ends 87.0 | **-3.0** (8 of 8 knockdowns) |
| 6 | 84 | frames 0 to 6 | frame 7 | world frame 31 | from 41, ends 84.0 | **-6.0** (3 of 3) |

- **What holds:** the victim's fall and the attacker's lunge both stand on frame 0 of their takes for the N frames (`sk13-hitstop`, every frame). That is the pose just before contact, not a hit pose, because the calls arrive in the hit frame and the figure's clock stops at once.
- **The fall after the stop:** 26 frames at rate 1.0, held on its last pose. The 30-frame spin is held on frame 29 and the get-up start does not move, so the spin lies 11 frames at stop 3 (14 at stop 0). No Blueprint change needed for the 30 frames at `knockdown_frames` 90.
- **Why the get-up is early:** the Blueprint schedules the get-up start on world time from the `SetKnockedDown` call. Device A sends `KnockdownFrames - N` and stops the figure's clock for N frames; world time does not stop, so the get-up starts N frames early and ends N frames before the release. The figure then stands for N frames while the sim still holds it.

**Every use of time in the two Blueprints, and whether it survives the stop:**

| Where | Clock | Survives |
|---|---|---|
| `KnockDown` (`DownT0`), `ReceiveTick` (the get-up start test, the get-up end estimate), `PlayUp` (`UpT0`) | world time | **no: the get-up ends N frames early** |
| `ReceiveTick`, the fall's hold test | the tick's own `DeltaSeconds` (dilated) and the montage position | yes |
| The 0.6 s reaction rule (`LastHitT` in `PlayHitReaction`, `PlayBoardHit`, `PlayStickHit`) | world time | yes: a real-time rule, off by at most N frames |
| `PlayPoke` / `SK2Tick` (`StumbleT0`, the 0.095 s stumble delay) | world time | yes, off by at most N frames if a poke falls inside a stop |
| `SK2Tick` aim follow, `SK3Fix` blade correction ramp | world delta time | yes: A sends no aim during the stop |
| `DownAge`, `GetUpFrames` (the yaw turn) | counted per `UpdateMotion` call | yes: A sends none during the stop |
| The spin counter (`KdCount`) | per knockdown | yes |
| Timers, Delays | none in either Blueprint | |

**The fix, either of two (not applied, not tested):**
- **A's side, no Blueprint change:** send the full `KnockdownFrames` (no `- N`). The Blueprint's world-time schedule then ends the get-up on the release; the fall starts N frames late and the lying time shrinks by N.
- **B's side, one statement** at the top of `ReceiveTick` in `BP_SkaterVisual_TB1` and `_TB1_P3`: while the actor's `CustomTimeDilation` is under 0.01, add the world delta to `DownT0`: `(if (< (self.CustomTimeDilation) 0.01) (set self.DownT0 (+ (self.DownT0) (GetWorldDeltaSeconds))))`. The schedule then waits out the stop and ends on the release with A's `- N`.

## 2. The lying poses (export16, test figures knocked down on the real clay ice, both figures)

- **Heights in the match:** the pawn, the six game figures and the test figures all have root z 0.00. A trace finds only the ground slab at -20 cm; the visible ice is taken as z 0 (the SK8 contact plane at +0.4 cm sits on it). I could not read the ice mesh's own height from script.
- **Joint centres above the ice, held lying pose (night / clay):**

| Pose | Take | Pelvis | Lowest joint |
|---|---|---|---|
| on the back | `Fall` | 24.5 / 31.8 cm | 13.7 / 15.6 cm (hand, skate) |
| face down | `Knockdown_Back` | 24.5 / 31.8 | 16.1 / 18.4 (hand) |
| right side down | `Knockdown_Left` | 41.5 / 41.4 | 10.0 / 11.2 (skate) |
| left side down | `Knockdown_Right` | 41.5 / 41.4 | 10.0 / 9.6 (skate, forearm) |

- **The body mesh:** from a camera 10 cm above the ice (`sk13-lying-poses-clay`) the body rests on the ice in all four; nothing is cut by the ice line; the stick lies on the ice along the body. I could not read the skinned mesh's lowest vertex from script. The keys are equal to export16 by name, and SKF's measure on those keys is: body and skates at 0 cm or above, the stick at worst 1.4 to 1.9 cm under in three takes.
- **So:** with export16 the pelvis is not under the ice, and the attach height is not the cause of anything. A's reading was on the old takes; I did not re-measure those.

## 3. The human's check (clay, `jam.ctrl.Drill 6`, 47 s of match, hit-stop 3)

- **9 knockdowns by the human** out of 13 presses. Kinds from the sim: front, behind, side_l, front, behind, behind, side_l, front, front. The victim slides 2.5 to 9.1 m while down. Centres 80 cm apart at every hit.
- **Press to hit** (first `PlayCheck` call of the press to the knockdown): **1, 1, 7, 8, 10, 25, 7, 4, 7 frames.** The 25 is a press held past the window and re-armed.
- **Frame of the lunge at the hit: 0, every time.** `PlayCheck` arrives on every frame while the key is held (145 calls in 13 presses, 4 to 26 a press), and each call restarts the take, because a check at priority 7 is allowed over a check at priority 7. Recorded on a 26-frame press: the lunge is on frame 0 for all 26 frames and through the 3 held frames, and only then plays. **With the human, the lunge always starts after the hit.** Bots send one call, so theirs plays (SK12).
- **The fix (not applied):** in `PlayCheck`, start the take only if no check is already running: wrap the two `PlayCut` calls in `(if (or (not (IsAnyMontagePlaying <anim instance>)) (!= (self.ActPri) 7)) ...)`. Only checks use priority 7.
- **Takes that met a real match for the first time:** `Fall` -> `GetUp` (4), `Knockdown_Back` -> `GetUp_Front` (3), `Knockdown_Left_B` -> `GetUp_Side_Left` (2). Each played to its end, held, and got up at rate 1.0. Strips: `sk13-fall-front`, `sk13-human-check-behind`, `sk13-human-check-side` / `sk13-spin`.
- **Not seen in a match:** plain `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Right_B`, `Fall_Forward`, `Trip`, `GetUp_Side_Right`, and the human being knocked down (the pro bots did not check it in 47 s).

## 4. A's ask: does a side fall read as from the side

Both side hits in the drill match were the victim's 2nd and 5th knockdown, so the Blueprint played the spin; no plain `Knockdown_Left` / `_Right` happened. In the spin strip the victim is thrown across the hitter's path and lands on its side; it reads as a tumble, the direction is not obvious at this size. For the plain side takes the last real strips are SK12's (export14: `sk12-knockdown-night`, `sk12-knockdown-clay`, hit from the right, the body thrown sideways along the boards): there it reads as a side hit. Export16 changes those takes only from the landing on (the stick). **Plain side falls with export16 in a real match: not verified.**

## 5. Export16

- **Import** (by take name through the data controller, as in SK12): against the SK12 state 15 existing takes changed per figure: `Check_Hit_Back`, `Check_Hit_Left`, `Check_Hit_Right`, `Fall`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Knockdown_Back`, `Knockdown_Left`, `Knockdown_Left_B`, `Knockdown_Right`, `Knockdown_Right_B`, `Trip` (export15's 7 and export16's 12, counted by comparison). **New, unwired:** `Fall_B`, `Knockdown_Back_B` (28 frames) on both figures.
- **Proof, run last:** export16 imported again and compared by name: night **76 of 76 equal**; clay **48 equal, 28 aim holds kept** (export9), largest difference 0.0. 739 assets under `/JamAssets/Skaters/` load, 16 Blueprints and AnimBPs up to date, 16 blend spaces full.
- **Joins on the saved assets** (last key of the fall against the first key of its get-up, every bone, both figures): exact (0 cm, 0 deg) for `Fall_B` -> `GetUp`, `Knockdown_Back_B` -> `GetUp_Front`, `Fall_Forward`, `Knockdown_Back`, both side takes and both spins, and `Trip` -> `GetUp_Side_Right`. **One is not exact: `Fall` -> `GetUp`: the thighs and calves differ by a twist of 6.3 deg (night) and 27.5 / 14.5 deg (clay).** Joint positions agree, the twist does not. For SKF.
- **The spin, 30 frames:** see item 1; `sk13-spin` is a real one.
- **The two new takes on test figures:** the joins are exact on the assets (above). Played directly on test figures of both Blueprints (`sk13-new-takes`): `Fall_B` reads as knocked on to the backside with the arms up; `Knockdown_Back_B` reads as a flat dive on the belly with the legs kicked up. Only the first 15 frames were seen: a test call plays them as a one-shot that blends out, so **the lying part and the join into the get-up were not seen in the engine**; that needs the wiring.
- **Not re-shot:** a side knockdown at the boards. Every knockdown in the drill match was in open ice, so whether the stick still shows through the wall is **not checked** with export16. In the lying poses the stick now lies along the body (65 to 90 cm from the pelvis by SKF's measure, against 2 m out before), which should help.

**Blueprint wiring I recommend for `Fall_B` and `Knockdown_Back_B`** (for the lane after B30, both scripted Blueprints):
1. `KnockDown`, where the fall is chosen: for `DownKind` 3 play `Knockdown_Back_B` when `bSpin`, else `Knockdown_Back`; in the last branch (the front hit) play `Fall_B` when `bSpin`, else `Fall`. `bSpin` is already computed for every knockdown (strength 0.95 or more, or the figure's knockdown count mod 3 = 1), so every third knockdown of a figure, or a hard hit, plays the B take of whatever direction it is.
2. `PickLens`: when `bSpin`, call `SetLen` with the B take (28 or 30 frames, not 26). Today the plain take's length is used; it only matters when fall + get-up exceed `knockdown_frames` (under 72 to 74 frames), where the B fall would be cut 2 to 4 frames short.
3. `PlayGetUp` needs no change: each B take ends on its A take's lying pose.

## 6. `Board_Pin`

`sk13-board-pin`: clay tier, real match (seed 1, `all_star`, scripted human), the human pinned at the end boards by a bot, 15 frames after the pin started, game camera, HUD removed.

## The Blueprint changes I recommend (none applied)

1. **Get-up against the hit-stop:** either A sends the full `KnockdownFrames`, or the one statement in `ReceiveTick` (item 1).
2. **The human's lunge:** do not restart a running check in `PlayCheck` (item 3). Without it the human's tackle shows the hit first and the lunge after.
3. **`Fall_B`, `Knockdown_Back_B` and `PickLens`** (item 5).
4. To freeze on a real hit pose, the takes would have to be started a few frames in when the call arrives in a stop (the fall's contact is at frame 2, the lunge's extreme at frame 2). I did not try it.

## Files changed and created (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| 15 changed `AS_SkaterTB1v2_*` takes (total) | 2,478,027 | 2,491,083 |
| 15 changed `AS_SkaterTB1_P3_*` takes (total) | 2,635,227 | 2,648,795 |
| new: `AS_SkaterTB1v2_Fall_B`, `AS_SkaterTB1v2_Knockdown_Back_B` (total) | | 309,635 |
| new: `AS_SkaterTB1_P3_Fall_B`, `AS_SkaterTB1_P3_Knockdown_Back_B` (total) | | 310,377 |

`git status` under `Skaters/`: 30 takes modified, 4 new, nothing else. New outside `Game/`: this report and the `sk13` files in `devlog/shots/`.

## Ledger rows

Replace:
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (167712 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (174704 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (174683 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (168401 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (189357 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (191277 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (196848 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (199769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (168341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (172819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only); spin variant, 30 frames (lane SK13) (166206 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (172261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only); spin variant, 30 frames (lane SK13) (166088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Trip by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK13) (174746 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (155461 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (163511 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (163464 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (156566 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (175784 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (177794 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (186134 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (186181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (156482 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (161716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only); spin variant, 30 frames (lane SK13) (164676 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (161255 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only); spin variant, 30 frames (lane SK13) (164571 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Trip by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK13) (163769 bytes) |
```
New:
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only); new take, not wired into the Blueprint yet (lane SK13) (152997 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p3_clay loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only); new take, not wired into the Blueprint yet (lane SK13) (157380 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only); new take, not wired into the Blueprint yet (lane SK13) (153423 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export16/p2_cel loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only); new take, not wired into the Blueprint yet (lane SK13) (156212 bytes) |
```

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-122841-B-sk13-tackle-human-clay.png` (operator sheet)
- `20261113-122841-B-sk13-hitstop.png`
- `20261113-122841-B-sk13-fall-front.png`
- `20261113-122841-B-sk13-human-check-behind.png`
- `20261113-122842-B-sk13-human-check-side.png`
- `20261113-122842-B-sk13-spin.png` (the same knockdown as the side strip)
- `20261113-122842-B-sk13-lying-poses-clay.png`
- `20261113-122842-B-sk13-board-pin.jpg`
- `20261113-123205-B-sk13-new-takes.jpg` (extra)

## What I could not verify

- **A cook.** Not run.
- **Either get-up fix and the lunge fix:** written, not applied, not tested.
- **In a real match with export16:** plain `Knockdown_Left` / `_Right`, anything on the right side, `Fall_Forward`, `Trip`, the human knocked down, a knockdown at the boards (the stick through the wall).
- **The body mesh's lowest vertex and the ice mesh's height** read directly; the lying result rests on joint heights, the low side stills and SKF's measure of the same keys.
- **The old takes' lying pose** that A's eyes read.
- **Drill 5** (the side approach): not run; drill 6 gave the checks.
- **Motion at speed by eye:** strips, close stills and per-frame numbers only.
