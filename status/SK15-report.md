# SK15 report: the tackling Blueprints against device A's new build, export18, the boards rule (device B)

Lane SK15, device B, machine clock 13:30 to 14:25. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` 2099faa), two editor sessions on port 8002. The first (pid 21692) was closed while I was cut off between 14:05 and 14:11, not by me (its log shows a clean exit at 14:08; everything had been saved at 13:53). The second (pid 36740, 14:11 to 14:24, for the missing measurements, the proof run and the end check) left through `ExitEditor`; the process is gone. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3` and 28 takes. `/Game/Maps/Rink` not saved; no `_Scratch/`; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill` and `jam.hitstop` read back at their defaults before the exit; `jam.vis.lowlog` was on in my sessions only and is not saved. Port 8000 not touched; nothing under `VFX/` touched. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk13` (`bp15.mjs`, `bp15b.mjs` the Blueprint changes; `import18.py`; `react15.py` / `react15.sh` the match strips; `fig15.py`, `t2d.sh`, `t2x.sh`, `t2y.sh` the test figures; `low15.mjs` the `JAMLOW` reader; `bk15/` the two Blueprints and the 152 takes as they were at lane start).

## Verdicts

| Item | Verdict |
|---|---|
| 1. Hit-stop against A's unreduced frames | **PASS**: get-up end minus release 0.00 at `jam.hitstop` 0, 3 and 6 |
| 2. export18, the get-up timing, the lying poses | **PASS**: 14 takes per figure by name; pelvis at 90 % minus release is -3 to -1 on 15 of 16 knockdowns (it was -12); the bodies rest on the ice in all four poses. One leftover for the Blender lane: a foot joint 2.2 cm under the ice early in `GetUp_Side_Left` |
| 3. The lunge in the frozen frame | **PASS**: the lunge is never under frame 5 at the hit (it was 0 or 1 in two of eight) |
| 4. B takes alternate | **PASS**: front `Fall`, `Fall_B`, `Fall`, `Fall_B`; behind `Knockdown_Back`, `_B`, `Knockdown_Back`; side spin on every third |
| 5. Side falls next to the boards | **PASS for side falls** (a real one 0.6 m from the side boards, test figures at the side boards and in a corner). **Open:** a hit from behind or the front toward a wall still goes through it |
| 6. Nothing but the release ends a knockdown | **PASS**: 12 reactions sent 8 times during a knockdown change nothing |
| 7. Contact shade while down | **NOT ACHIEVED**: the plane grows and follows the body as asked, but it has no visible effect; the material is the limit |

| Chain item (real matches on clay) | Verdict |
|---|---|
| Every item above seen in a real match | **Mostly**: items 1 to 5 seen in real matches (the boards rule once, at the side boards). The corner, item 6 and item 7 on test figures only |
| The human knocked down | **NOT SEEN**: no bot checked the human, on `pro` or on `all_star` |
| `JAMVIS` pairs | **PASS**: every count pairs |
| A's `PlayCheck` per press | **Still every held frame**: 138 calls in 16 presses |

## The Blueprint changes as saved (both Blueprints unless said)

1. **`ReceiveTick`:** the hit-stop shift is gone entirely (`DownT0`, `UpT0`, `UpEndEst` and `LastHitT`). I dropped `LastHitT` too: the rule is 36 frames long and a figure that is down skips reactions anyway.
2. **`OnBusEvent`, new branch on `check`:** when the figure is the checker and its lunge is not active, is not a check, or is under 4 frames in, it starts `Check_Deliver_Left` or `_Right` at frame 6 with no blend. The side comes from the event's contact point.
3. **`KnockDown`, B takes:** the strength clause is gone. Two new counters per figure, `KdFront` and `KdBack`: front hits alternate `Fall` / `Fall_B`, hits from behind alternate `Knockdown_Back` / `Knockdown_Back_B`, side hits spin when the figure's knockdown count mod 3 is 1, as before.
4. **`KnockDown`, the boards, new events `WallFix`, `WallAt`, `FindRink`:**
   - Before a side knockdown the figure tests a point 1.5 m along the knock direction against the rink. If it is beyond the boards it plays `Knockdown_Back` along its own facing; if the point 1.5 m ahead is also beyond the boards, `Fall` along its facing (the body goes backward); if both are, `Knockdown_Back` turned toward centre ice.
   - **It is not a physics trace.** The boards have no collision (a line trace finds nothing). `FindRink` reads the half-sizes of the rink actor's `Ice` component once (4500 x 2250 cm here), and `WallAt` tests against that box with the corners rounded at 3.75 m. The corner radius is a constant in the Blueprint; see the notes for A.
5. **`SetBoardPin`:** the off branch stops a montage only if the figure is neither down nor getting up.
6. **`PlayUp`:** the get-up's blend-out is 0.05 s (it was 0.25 s). With 0.25 s the last 15 frames of the take were already blending to the standing pose, so the figure stood earlier than the take says.
7. **Clay figure only, the contact shade:** while down, `ContactShadow` is scaled to 1.9 x 1.0 m, `Darkest` 0.25, and each tick it is placed under the pelvis and turned along the body (pelvis to head); at the release it returns to 0.5 x 0.7 m, `Darkest` 0.45, at its old place. New variables `bShade`, `ShadeYaw`.

Both compile `BS_UP_TO_DATE`. Against the lane-start text these events differ: `ReceiveTick`, `KnockDown`, `SetBoardPin`, `OnBusEvent`, `PlayUp`, and the three new ones.

## 1. The hit-stop against A's unreduced frames

Three seeded clay matches (drill 6). First (export17 takes): `jam.hitstop` 0 until 18 s, 6 until 40 s, then 3. Second (export18): 3 throughout. Third (export18, no captures): 0 until 18 s, then 6. Sent frames are 90 in every case.

| `jam.hitstop` | Knockdowns my captures did not touch | Get-up end minus release (frames, `SK6REL`) |
|---|---|---|
| 0 | 3 (1 with export17, 2 with export18) | 0.00, 0.00, 0.00 |
| 6 | 4 (2 with export17, 2 with export18) | 0.00, 0.00, 0.00, 0.00 |
| 3 | 15 (4 with export17, 11 with export18) | 0.00 fifteen times |

- Knockdowns frozen for my captures read 0.81, -0.14 and -0.17 in the first match and 0.53 in the second; that is my `slomo`, as in SK14.
- By A's measure the stop no longer moves anything either: pelvis at 90 % minus release is -2, -2 at stop 0, -2, -3 at stop 6 and -3 (median) at stop 3 (section 2).

## 2. export18: get-up timing and lying poses

**Import.** 14 takes per figure by name (the ten falls and the four get-ups); night 62 equal, clay 34 equal and the 28 aim holds kept. Proof run before exit: night 76 of 76 equal, clay 48 equal with the 28 aim holds kept, nothing copied; 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs up to date, no dirty package, no scratch folder. Get-ups are 36 frames (0.6 s); `PickLens` / `SetLen` read the lengths from the assets, so the schedule moved by itself: the get-up starts at frame 54 of 90 (it was 46), lying is about 28 to 30 frames.

**Pelvis at 90 % of standing minus the release, by `JAMLOW`** (A's measure; clay, real matches):

| Takes, Blueprint | Knockdowns | Values (frames) |
|---|---|---|
| export17, blend-out 0.25 s (first match) | 9 | -11, -12, -12, -12, -12, -16, -12, -12, -11 |
| export18, blend-out 0.05 s, hit-stop 3 (second match) | 11 | -1, -3, -3, -2, -3, -7, -2, -3, -3, -3, -3 |
| export18, blend-out 0.05 s, hit-stop 0 (third match) | 2 | -2, -2 |
| export18, blend-out 0.05 s, hit-stop 6 (third match) | 2 | -2, -3 |
| export18, bot on bot, no hit-stop (`all_star` match) | 1 | -3 |

- A's baseline of -12 reproduces here with export17.
- With export18 it is -3 to -1 on 15 of 16, inside the target of -3 to 0; the median is -3.
- The one -7: the read-out took its own standing height as 66.5 cm for that knockdown instead of about 73.8 (the figure was read while crouched), so its 90 % line was 6.6 cm lower. The same knockdown read -16 with export17.
- Pelvis at the release frame: 72.8 to 73.8 cm in all eleven of the second match.
- The get-up starts at frame 56 of 90 by `JAMLOW` (it was 47 to 48).

**Lying joint heights above the ice (cm), clay.** "Test" is my reading on test figures (lowest of the pair for hands and feet); "`JAMLOW`" is A's read-out in a real match (median over the lying window).

| Pose | Take | Joint | export17 test | export17 `JAMLOW` | export18 test | export18 `JAMLOW` |
|---|---|---|---|---|---|---|
| on the back | `Fall` | pelvis / head / hands / feet | 31.8 / 36.9 / 16.9 / 15.6 | 31.8 / 36.9 / 16.9 / 15.6 | 19.0 / 23.3 / 14.9 / 15.6 | 19.0 / 23.3 / 14.9 / 15.6 |
| face down | `Knockdown_Back` | pelvis / head / hands / feet | 31.8 / 33.8 / 18.4 / 23.6 | 31.8 / 33.8 / 18.4 / 23.6 | 18.6 / 22.0 / 15.6 / 23.6 | 18.6 / 22.0 / 15.6 / 23.6 |
| on its right side | `Knockdown_Left` | pelvis / head / hands / feet | 41.4 / 44.5 / 14.3 / 11.2 | 41.4 / 44.5 / 14.3 / 11.2 | 28.4 / 38.4 / 13.3 / 11.3 | 28.4 / 25.2 / 13.3 / 11.3 |
| on its left side | `Knockdown_Right` | pelvis / head / hands / feet | 41.4 / 44.5 / 12.6 / 11.2 | not seen | 28.4 / 38.4 / 25.2 / 11.3 | 28.4 / 25.2 / 25.2 / 11.3 (one, bot on bot) |

- The two measures agree to 0.1 cm wherever both exist, except the head on the side poses with export18 (38.4 mine, 25.2 `JAMLOW`): mine is the head joint, A's is the lowest bone with "head" in its name. export18's clay pelvis numbers are SKF's (19.0, 18.6, 28.4).
- **One thing under the ice by `JAMLOW`:** in `GetUp_Side_Left`, about 4 frames into the get-up, the right foot joint is 2.2 cm under the ice plane (two of three side knockdowns; 0.8 cm above in the third). In `Knockdown_Right`'s chain the lowest is -2.6 cm (one knockdown; I did not look up which frame). A's limit was 2 cm. For the Blender lane.
- A's numbers were for the night figure with SK12's takes (24.5 and 38); clay's export17 values were higher (31.8 and 41.4). I did not measure night in this lane.
- **Does the body rest on the ice now? Yes** in the four low side stills: trunk and hips touch the ice line on the back, face down and on both sides. On the left side the stick lies across the legs above the ice, as SKF said.
- The export17 row of `sk15-lying-low` is out of focus (my camera had no focus distance) and its fourth tile has a game figure in front; the hover still shows in three tiles. The numbers above are the measure.
- `sk15-getup-timing` is from a test figure with a fixed side camera, take frames 18 to 31 of 36 with the pelvis height in each tile: 67 % of standing at release -18, 76 % at -12, 85 % at -5. Frames 32 to 36 were not captured (my slow-motion steps ran out). The real-match frames I had for this follow the human and lose the victim, so I did not use them.

## 3. The lunge in the frozen frame

First match, 11 knockdowns by the human, press to hit / lunge frame at the hit:

| Press to hit (frames) | 0 | 7 | 26 (held) | 16 | 1 | 7 | 7 | 5 | 7 | 6 | 2 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Lunge frame at the hit | 6 | 7 | 7 | 16 | 6 | 7 | 7 | 5 | 7 | 6 | 6 |

- The three presses under 4 frames (0, 1, 2) are at frame 6 at the hit: the bus event arrives in the hit frame and the restart is in place before the stop holds the figure.
- In the held frame the human is now low with the shoulder in (`sk15-hitstop-lunge`, the same sim frame before and after).

## 4. B takes

- Victim 3, front hits in order: `Fall`, `Fall_B`, `Fall`, `Fall_B`. Victim 4, from behind: `Knockdown_Back`, `Knockdown_Back_B`, `Knockdown_Back`.
- Side: `Knockdown_Left`, `Knockdown_Left`, then `Knockdown_Left_B` once (the figure's count mod 3). That is the first side spin in a real match since the B length was wired; its get-up ended on the release like the others.

## 5. Side falls next to the boards

| Case | Result |
|---|---|
| Real match, sim frame 2932, `side_l`, pawn 0.6 m from the side boards (the SK14 fault) | rule fired (`SK15WALL`), `Knockdown_Back` along the facing; nothing through the wall |
| Test figure 0.6 m from the side boards, knocked at the wall | `Knockdown_Back` along the wall; nearest joint (a foot) 21 cm inside |
| Test figure in a corner, 0.6 m inside the arc, knocked into the corner | `Knockdown_Back` along the arc; nearest joints (a hand, a foot) 19 to 20 cm inside |
| Test figure in open ice, same hit | plain `Knockdown_Right`, unchanged |
| **Not covered:** test figure 0.6 m from the end boards, hit from behind toward them | `Knockdown_Back` dives into the wall: head and arms through it |

- The "both ways blocked" branch (turn toward centre ice) cannot be reached in this rink: a point 1.5 m behind the figure is always inside when the point ahead is not. It is there for small rinks.
- My first corner test stood outside the arc (my mistake in the position); the row above is the repeat on the arc.
- Joint centres, not skin: a hand 19 cm from the wall has its mitt close to it.

## 6. What can interrupt a knockdown

- **I did not find how A's case happened.** In the Blueprint text a pin cannot start while the figure is down, and `KnockDown` clears `bPinned`, so the pin's off branch should not have run. I could not reproduce it either. If A sees it again, the `SK6RX` lines around that frame name the call.
- **What I closed:** the pin's off branch was the only montage stop outside the knockdown code; it now also refuses while the figure is down or getting up.
- **Every other entry** goes through `PlayAct` / `PlayCut` or its own test of `bDown`; I found no other stop or play that can run while the figure is down.
- **A finding on the way:** the flag `bUpRun` is never set for longer than the tick that starts the get-up (0 of 396 recorded get-up ticks in the second match). So everything rests on `bDown`, which holds until the release. Since the get-up now ends on the release that is enough, but the flag does not do what its name says.
- **Proof on test figures:** pin on before the knockdown; then pin on, hit, board hit, both stick hits, shoot, check, pass, poke, lift, foul and pin off sent 8 times through fall, lying and get-up. Against an untouched figure knocked down in the same frame, the montage and its position are identical on all 738 ticks.
- Pins did occur in the real matches (2 on, 2 off in each), none during a knockdown as far as the logs show; the proof is the test above.

## 7. Contact shade while down

- The Blueprint does what was asked (section above), and the plane's place and direction read back right (under the pelvis, turned along the body).
- **It does not show.** On the wing, a lying figure captured with the plane hidden and with it shown at 3.0 x 1.8 m and `Darkest` 0 differs by 0.4 grey levels of 255 on average, 14 at most. `sk15-contact-shade` shows the three positions with the standing-size and the lying-size plane: no visible difference.
- **Why:** `M_SkaterContact` darkens in a round spot that falls off fast. Its dark core is under the body; what reaches past the silhouette is too faint. Size and `Darkest` cannot fix that; the falloff has to be flatter (a material change, mine from SK8, not done here).

## The chain in real matches (clay, drill 6)

- First match (export17, hit-stop 0 / 6 / 3, 68 s): 11 knockdowns by the human. Takes: `Fall` 2, `Fall_B` 2, `Knockdown_Back` 3, `Knockdown_Back_B` 1, `Knockdown_Left` 2, `Knockdown_Left_B` 1.
- Second match (export18, hit-stop 3, 80 s): 12 knockdowns by the human. Takes: `Fall` 3, `Fall_B` 2, `Knockdown_Back` 3 (one by the boards rule), `Knockdown_Back_B` 1, `Knockdown_Left` 2, `Knockdown_Left_B` 1. Get-ups: `GetUp` 5, `GetUp_Front` 4, `GetUp_Side_Left` 3.
- Third match (export18, hit-stop 0 then 6, 35 s, no captures): 4 knockdowns.
- `all_star` with the scripted human (60 s, hit-stop 3): one knockdown, bot on bot (`Knockdown_Right`, `GetUp_Side_Right`, get-up end minus release 0.00); the human was not checked; 49 collisions had no landed check.
- `JAMVIS` against the Blueprint (first match): knockdown 11 / 11, hits 31 = 22 played + 2 blocked + 7 skipped, pin on 2 / 2, pin off 2 / 2, board 16 / 16, stickhit 9 / 9, `JAMHITSTOP` 9 for the 9 knockdowns after the stop was switched on. Second match: knockdown 12 / 12, hits 33 = 24 + 2 + 7, pin 2 / 2, board 18 / 18, stickhit 11 / 11, `JAMHITSTOP` 12 / 12.
- `PlayCheck`: 138 calls in 16 presses (first match), 152 in 20 (second); A still sends it on every held frame.
- Bots: none knocked anyone down in the drill 6 matches on `pro`; one bot-on-bot knockdown on `all_star`.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 2,985,941 | 3,399,787 |
| `BP_SkaterVisual_TB1_P3` | 3,044,161 | 3,595,511 |
| 14 `AS_SkaterTB1v2_*` | 2,315,562 | 2,274,986 |
| 14 `AS_SkaterTB1_P3_*` | 2,439,353 | 2,398,073 |

The 14 takes: `Fall`, `Fall_B`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Knockdown_Back`, `Knockdown_Back_B`, `Knockdown_Left`, `Knockdown_Left_B`, `Knockdown_Right`, `Knockdown_Right_B`, `Trip`. Nothing else under `Skaters/` changed; `M_SkaterContact` is untouched.

## Stills (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with `.txt`)

- `20261113-140009-B-sk15-hitstop-lunge.jpg`: SK15: the held frame when the human's press and the hit fall in the same sim frame (clay, real match, sim frame 158, hit-stop 3, A's game camera). Left: SK14, the lunge at frame 0, the human upright. Middle and right: SK15, the lunge restarted at frame 6 on the sim's check event, first and third held frame.
- `20261113-140014-B-sk15-boards-side.jpg`: SK15: a side knockdown 0.6 m from the side boards in real matches on clay, the same sim frame (2932). Top: SK14, Knockdown_Left, the body goes into the wall. Bottom: SK15, the boards rule plays Knockdown_Back along the facing, the body stays on the ice (export17 takes in this strip).
- `20261113-140023-B-sk15-boards-corner.jpg`: SK15: clay test figures, export18. Left: in a corner, 0.6 m inside the arc, knocked into the corner: Knockdown_Back along the arc, nothing through the wall. Right: not covered by the rule: a hit from behind toward the end boards from 0.6 m, head and arms in the wall.
- `20261113-140037-B-sk15-lying-low.jpg`: SK15: the four lying poses on clay from a camera 9 cm above the ice, export17 above export18. The export17 row is out of focus and its fourth tile has a game figure in front; export18 rests on the ice in all four (pelvis joint 28.4, 19.0, 18.6, 28.4 cm).
- `20261113-140048-B-sk15-contact-shade.jpg`: SK15: contact shade under a lying clay figure at mid ice, on a wing and in a corner zone. Top: the standing-size plane (the corner tile is after the get-up). Bottom: the lying-size plane (1.9 x 1.0 m, Darkest 0.25, under the pelvis). No visible difference: the plane's dark core is under the body.
- `20261113-140104-B-sk15-tackle-clay.jpg`: SK15 operator sheet, real match on clay (drill 6, hit-stop 3, export18, A's game camera): press and hit in one frame with the lunge in at frame 6, the held frames, Fall from frame 2, slide with the trail, lying on the ice, the 36-frame GetUp to the release at +90. The camera follows the human, so the victim leaves the frame after +40.
- `20261113-142300-B-sk15-getup-timing.jpg`: SK15: export18's 36-frame GetUp on a clay test figure (the SK15 Blueprint, knockdown of 90 frames, fixed side camera), take frames 18 to 31, that is 18 to 5 frames before the release; each tile carries the pelvis height read from the figure. At release -5 the pelvis is at 85 % of standing (it stood at -12 before). Frames 32 to 36 were not captured.

## Ledger rows

Replace the two Blueprint rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.24 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14); + SlideFx Niagara component (NS_slide_trail) (lane B30); tackling (lane SK15): no clock shift under the hit-stop (device A sends the unreduced knockdown frames), the checker's lunge snaps to frame 6 on the sim's check event, B takes alternate per kind (side spin every third), a side fall next to the boards becomes a fall along the facing (rink size from RinkActor's Ice bounds, corner radius 3.75 m), a pin's end cannot stop a montage while down, get-up blend-out 0.05 s (3399787 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 3.43 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow scaled to 1.9 x 1.0 m under the pelvis while down (lane SK15; no visible effect yet, see the report) (3595511 bytes) |
```
Replace the 28 take rows:
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (153423 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (155702 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (167912 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (167298 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (174102 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (174149 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (156482 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (156212 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (161716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (164676 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (161255 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (164571 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p2_cel loopcut, take Trip by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK15) (163769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (154853 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (167537 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (178349 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (180013 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (186928 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (186969 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (168341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (157380 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (172819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (166206 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (172261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (166088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export18/p3_clay loopcut, take Trip by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK15) (174746 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- The human knocked down by a bot.
- Night: nothing in this lane was looked at or measured on the night figure. Its Blueprint has the same changes (not the contact shade) and its takes the same import.
- Hit-stop 0 and 6 with export18: two knockdowns each (plus one and two with export17).
- The boards rule in a real match: once, at the side boards, with export17 takes. The corner and the open-ice control: test figures.
- The "both ways blocked" branch of the boards rule: never reached.
- Item 6 in a real match: no pin fell inside a knockdown. A's original case: not reproduced, cause not found.
- `Knockdown_Right`, `Fall_Forward`, `Trip` in a real match by the human's check; `Knockdown_Right` once bot on bot.
- The 0.05 s blend-out in motion: frames and `JAMLOW` only.
- Which frame of `Knockdown_Right`'s chain has the -2.6 cm.
- The first editor of this lane (pid 21692) was not closed by me: its log shows a clean exit at 14:08, while I was cut off. Everything had been saved at 13:53 and nothing was edited after that.

## Decisions for you

1. **Hits from the front or from behind toward a wall.** The same test can cover them, but every take throws the body along the knock direction, so the only safe choices are a fall along or against the facing, and here the facing points at the wall. Options: (a) play a side take turned along the wall for these (the body goes along the boards); (b) the Blender lane builds an in-place crumple for "against the boards"; (c) leave it. I would take (a) in the next lane; it needs the wall's direction, which the box test can give.
2. **Contact shade.** Give `M_SkaterContact` a flatter falloff (a plateau to about 70 % of the radius) and a `Darkest` near 0.5 while down; then the Blueprint part already saved will show. One short lane with stills on the wing and in a corner.
3. **Get-up blend-out 0.05 s.** It makes the take's own timing count. If the last three frames look abrupt in motion, 0.08 to 0.1 s is the range to try; I judged it from frames only.
4. **The rink's size in the Blueprint.** It works from the rink actor's `Ice` bounds and a constant corner radius. If A exposes the rink (below), I replace both.

## Notes for device A

1. `KnockdownFrames` unreduced is right with this Blueprint: 0.00 at 0, 3 and 6. Please do not reduce it again.
2. `PlayCheck` still arrives on every held frame (138 calls in 16 presses, 152 in 20).
3. **The boards have no collision**, so the figure cannot trace for them. It reads the `Ice` component's bounds of `RinkActor` and assumes a 3.75 m corner. A `BlueprintPure` on the pawn or the rink actor giving half-length, half-width and corner radius (or simply "distance to the boards along a direction") would replace that.
4. The figure now listens to the `check` bus event (checker's index in `Who`, contact point in `Where`) to put the lunge in at the hit. Please keep that event in the hit frame and before the hit-stop takes hold of the pawn.
5. A hit from behind or the front that pushes the victim into a wall still shows the body in the wall (decision 1). If the sim can keep a knocked-down pawn about 1.2 m off the boards along the knock direction, the take fits without any change here.
6. `JAMLOW` works on B's figures as is; my numbers above use it.

