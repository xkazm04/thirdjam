# Lane SK23 report: fall sizes, the two calls by name, the board fall at the wall, the window's resolution

Tree `C:\Users\kazda\jam\thirdjam` (`dev-b` 20e72c8c), port 8002. One editor at a time; the last two (pid 6832, then pid 29860 for job 5) left through `ExitEditor` at 23:31:57 and 23:41:37, both confirmed gone. My clock 22:53 to 23:45.

**Tick rate:** 44.9, 45.5 and 44.6 sim frames per wall second in the three matches. Every number is in sim frames.

## Three things first

- **Two editors of mine died before the work started.** The first hung on the GPU at the first play start (23:00:17, `DXGI_ERROR_DEVICE_HUNG`); I stopped it with `editor.ps1 -Stop`. The second asserted (23:04:11, `Outer != nullptr`, `UObjectHash.cpp` 1587) when my Blueprint write ran while it was still loading. Nothing was lost: the Blueprints were unsaved and unchanged on disk, the takes already saved. The third editor did the whole lane.
- **Both seeded clay matches ended early.** The scripted human now loses 0 to 3 at sim frame 4661 (76 s of play) and 5372 (88 s). The counts below are for those spans, not for 100 s.
- **No stumble occurred in either match**, so "stumbles shown of stumbles" is 0 of 0 on this build.

## Verdict per job

1. **Import and wiring: done.** Export24 and then export25, by take name, both figures. Fall size, the snap get-up and `GetUp_Wall_Back` are wired and played in the matches. `Stride_Sprint` is imported and not wired.
2. **The two calls: declared on both Blueprints and proven on test figures only.** The real caller is not on main; that path is unproven.
3. **The board fall meets the wall: done, with one caveat.** The mesh shifts to the boards and eases back; no bone crosses the wall. I could not measure the skin's gap, only the bones'.
4. **Resolution: not blocky at the window's resolution, clay and night.** But an editor play session is still drawn at 1398x790 unless the editor's own override is switched off.
5. **The lean-in: counted, not shown.** It would turn in and stop short 6 and 9 times for the human in 76 and 88 s, against 16 and 18 real stops. That is not small.

Export25's addition: `GetUp_Late*` and `GetUp_LateLong*` are imported and reachable through a test switch, **not wired**. Two strips are filed for the choice.

## 1. Import

| | Night | Clay |
|---|---|---|
| Export24: takes in the file | 110 | 110 |
| equal to the asset already there | 70 | 42 |
| keys replaced | 26 | 26 |
| new assets | 14 | 14 |
| Export25: takes in the file | 126 | 126 |
| equal | 106 | 78 |
| keys replaced | 4 | 4 |
| new assets | 16 | 16 |
| Clay aim holds kept (export9's) | 0 | 28 |
| Failed the read-back | 0 | 0 |

- **New, 30 per figure:** `Fall_Light`, `Fall_Heavy`, `Knockdown_Back/Left/Right_Light`, `Knockdown_Back/Left/Right_Heavy`, `GetUp_Snap` and its three variants, `GetUp_Wall_Back`, `Stride_Sprint`, `GetUp_Late` and `GetUp_LateLong` in eight directions each.
- **Keys replaced:** 26 take names on night, 27 on clay (the list is in the ledger rows).
- 839 assets load (779 before).

### What is wired

| Choice | Read-out | Rule |
|---|---|---|
| Fall size | the knockdown's length (`KnockdownFrames`) | light under 90 frames, heavy from 130, the existing fall between |
| Get-up | the same length | snap under 70, quick from 70 to 89, the 36-frame one from 90 |
| After a wall fall | the wall's side | `GetUp_Wall`, `_Wall_Left`, `_Wall_Right`, and now `GetUp_Wall_Back` after `Board_Fall_Back` |

- **A 60 to 80 frame knockdown is still a whole fall and get-up:** 60 frames is `Fall_Light` (16 played), 24 lying, `GetUp_Snap` (20); 80 is the same with 44 lying and `GetUp_Quick`.
- **The wall get-ups' new end turn** (90 degrees to the right after the front fall, to the left after the back one) is handed to the mesh over the take's last three frames. Seen in the strips, not measured.
- **`Stride_Sprint` is not wired.** A clean read-out exists (the speed in `UpdateMotion`), but the stride lives in the figure's animation Blueprint and its blend space, which I did not open in this lane.

### Fall sizes on test figures

| Length | Fall | Get-up | Hips past half height before the release | Frames with the pelvis under 45 % |
|---|---|---|---|---|
| 60 | `Fall_Light` | `GetUp_Snap` from frame 41 | 7 | 46 |
| 80 | `Fall_Light` | `GetUp_Quick` from frame 61 | 13 | 61 |
| 110 | `Fall` | `GetUp` from frame 75 | 27 | 74 |
| 150 | `Fall_Heavy` | `GetUp` from frame 115 | 27 | not recorded to the end |

## 2. The two calls

Both exist as events on `BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3` with the handoff's names and inputs.

- **`PlayStumble(float DirX, float DirZ, int32 Frames)`:** plays `Stumble_<side>` by direction with the whole 32-frame take ending at `Frames`. Called with 16 it played `Stumble_Back` and ended at 16; with 15, `Stumble_Left`.
- **No double start:** a second stumble start in the same frame is refused, whichever of the call or the `check` bus read comes first. Not exercised with a real pair.
- **`SetBraking(bool bOn, float Decel)`:** from the first call a figure receives, its stop follows the level. The pose starts in the frame of `bOn` true (or as soon as no higher take is playing), `MS_brake` plays once per stop, and the derivation from the speed and the 12 s bar are off for that figure.
- **Release on false:** the hold stops looping and the take plays out, so the release comes up to 12 frames after the level drops. In the test the pose was gone 12 frames after `false`.
- **Until a call arrives** a figure behaves as saved before this lane; the sound lane's node is kept.

**The real caller is unproven.** I called both by name from the test harness. Whether device A's call reaches a Blueprint event declared this way I have not seen.

## 3. The board fall at the wall

- **How:** at the board fall the mesh (not the root) moves toward the boards by the figure's distance minus 35 cm, between -15 and +30 cm, over three frames. It holds through the fall and the lying, eases back from the get-up's frame 14, and is at zero five frames after the release.
- **Corners:** inside the corner arc the wall's direction and distance come from the arc, so the figure is turned square to the curve.

Wall-side shoulder bone to the boards at take frame 5, figures 66 cm out (60 cm in the corner):

| Case | Shift | With the shift | Without it |
|---|---|---|---|
| Wall ahead | 30 | 22.9 cm | 52.9 cm |
| Wall behind | 30 | 25.1 | 55.1 |
| Wall on the left | 30 | 23.3 | 53.3 |
| Wall on the right | 30 | 24.7 | 54.7 |
| Corner arc, facing it | 25 | 21.7 | 46.8 |

- **Against the proving number:** I could not measure the skin, so "under 5 cm" is not shown as written. The take's wall plane is now 1 cm from the real boards at 66 cm and on them from 20 to 65 cm; before it was up to 31 cm short.
- **Nothing through the wall, by the bones:** the nearest any bone comes during fall and get-up is 5.0 to 6.3 cm (a hand) with the wall ahead, 10.9 to 16.4 cm on the other sides.
- **Beside the bench:** a board fall under device A's team bench shows nothing past the boards (the still's third row).
- **In play:** one board fall (seed 7920, frame 4933): 40 cm from the wall, shift 5 cm, `Board_Fall_Front`, `GetUp_Wall`, get-up end minus release 0.0.

## 4. The window's resolution

| Session | Drawn | Shown |
|---|---|---|
| Editor play session as it starts by default | 1398x790 | 1920x1085 |
| The same with `r.Editor.Viewport.OverridePIEScreenPercentage 0` | 1920x1085 | 1920x1085 |

- Device A's line is in `DefaultEngine.ini` (twice, lines 12 and 25). It decides the game's path; the editor's own setting still decides an editor play session.
- **At 1920x1085 the moving figures are not blocky**, clay and night (`sk23-resolution-clay`, `-night`, against QA4's and SK20's frames).
- I ran no packaged build; device A measured there.

## 5. The lean-in, counted

A brake that runs three hard frames from 7 m/s or more and then ends before 3.5 m/s is lost is one a turn-in from frame 3 would show on and have to blend out of.

| | Seed 1 (76 s) | Seed 7920 (88 s) |
|---|---|---|
| Human: such short brakes | 6 | 9 |
| Human: brakes that reach 3.5 m/s lost | 16 | 18 |
| Human: stops shown today | 11 | 13 |
| Bots: short brakes, each | 5 to 15 (81 in all) | 2 to 15 (62 in all) |

- The short ones lose 1.7 m/s on the median.
- **Not shown:** one false turn-in for every two or three real stops is not small. The count is one more log line in `StopTick`; the stop itself is unchanged, and both matches replayed with the same knockdowns and get-ups.

## Export25: the late get-ups

Not wired. They play through a test switch on the figure (`SetTestMode`: 1 = `GetUp_Late*` for short knockdowns, 2 = `GetUp_LateLong*` for long ones), off by default.

| Knockdown | Wired choice | Hips past half height before the release | Late take | The same |
|---|---|---|---|---|
| 60 | `GetUp_Snap` | 7 | `GetUp_Late` | 5 |
| 80 | `GetUp_Quick` | 13 | `GetUp_Late` | 5 |
| 150 | `GetUp` | 27 | `GetUp_LateLong` | 5 |

- **Why not wired:** your condition was that it reads at least as well in the engine. On the strip the late take keeps the figure low longer and is up on the release; the last rise is 4 to 6 frames. I could not call that "at least as well" from one strip, so the choice is yours.
- **The spring shows.** In the close crop the body travels about 25 cm in those last frames; at this camera it reads as a jump up, not a slip.

## The matches

Regular, five a side, `pro`, scripted human.

| Seed | Skater | Frame | Length | Hit-stop | Fall | Get-up | End minus release |
|---|---|---|---|---|---|---|---|
| 1 | human | 1169 | 91 | 3 | `Fall` | `GetUp` | 0.0 |
| 1 | human | 2668 | 150 | 5 | `Fall_Heavy` | `GetUp` | 0.0 |
| 1 | bot 1 | 2690 | 126 | 0 | `Fall` | `GetUp` | 0.0 |
| 1 | bot 1 | 3161 | 74 | 0 | `Knockdown_Left_Light` | `GetUp_Quick_Side_Left` | 0.0 |
| 1 | human | 3792 | 124 | 4 | `Knockdown_Left` | `GetUp_Side_Left` | 0.0 |
| 7920 | human | 1935 | 82 | 3 | `Knockdown_Right_Light` | `GetUp_Quick_Side_Right` | 0.0 |
| 7920 | bot 1 | 3749 | 61 | 0 | `Knockdown_Left_Light` | `GetUp_Snap_Side_Left` | 0.0 |
| 7920 | human | 4933 | 138 | 5 | `Board_Fall_Front` (replacing `Knockdown_Left_Heavy` in the same frame) | `GetUp_Wall` | 0.0 |
| 7920 | bot 3 | 5316 | 133 | 0 | `Fall_Heavy` | `GetUp` | 0.0 |

- **Night, seed 7920 (100 s asked, ended at frame 5372 as on clay):** the same four knockdowns as the clay rows of that seed, with the night takes; get-up end minus release 0.0 four times; 70 stops shown.
- **No Blueprint error** in any session's log.
- Nobody was knocked down again within 120 frames of a release.

### Against SK22

| | Seed 1, SK22 (100 s) | Seed 1 now (76 s) | Seed 7920, SK22 (100 s) | Seed 7920 now (88 s) |
|---|---|---|---|---|
| Knockdowns | 3 (human 3) | 5 (human 3, bots 2) | 3 (human 2, bots 1) | 4 (human 2, bots 2) |
| Stumbles | 1 | 0 | 2 | 0 |
| Checks | 4 | 5 | 5 | 4 |
| Stops shown | 72 (human 8) | 58 (human 11, bots 5 to 6) | 76 (human 11) | 70 (human 13, bots 6 to 7) |
| Hard brakes that lost 3.5 m/s | 365 | 272 | 353 | 317 |
| Goals | 0 | 4 | 1 | 3 |

- **Knockdowns did not fall.** There are more of them in less time, and the bots now fall too.
- **Stumbles are gone:** all nine checks were knockdowns.
- **Stops per second are about the same** (0.76 and 0.80 a second shown; about 3.6 hard brakes a second as before).

## Blueprint changes as saved

Both figures, the same text with each figure's take names.

- **New events:** `FallPick`, `FallDo`, `ShiftTick`, `PlayStumble`, `SetBraking`, `SetTestMode`.
- **Changed events:** `KnockDown`, `PickLens`, `PlayGetUp`, `BoardFall`, `UpdateMotion`, `StopTick` (the level path, and the count line of job 5), `StumPlay`, `PlayStumbleNow`, `ReceiveBeginPlay`.
- **New variables:** `FallSize`, `bSnap`, `TurnYaw`, `WallD`, `WallShift`, `ShiftE`, `bBrkLevel`, `bBrkOn`, `bBrkShown`, `StumSpan`, `UpMode`. `DownKind` gains 8 (wall behind).
- **Kept:** the sound lane's `PlaySound2D(MS_brake)` in `StopTick`.

| File (under `Game/Plugins/JamAssets/Content/Skaters/`) | Bytes before | Bytes after |
|---|---|---|
| `BP_SkaterVisual_TB1.uasset` | 4,960,636 | 5,856,029 |
| `BP_SkaterVisual_TB1_P3.uasset` | 5,169,966 | 6,065,474 |
| 30 new night takes | | 4,679,525 in all |
| 30 new clay takes | | 4,769,539 in all |
| 26 night and 27 clay takes with keys replaced | | each in the ledger rows |

Saved with no play session open. End check: 839 assets load, 16 Blueprints up to date, no dirty package, `jam.*` defaults and the session variables I switched restored. Scratch cartridges and my screenshot files are deleted.

## Not verified

- The real callers of `PlayStumble` and `SetBraking`.
- A stumble in play on this build (none occurred), on either path.
- The skin's distance to the boards; the wall get-ups' end turn (seen, not measured).
- The late get-ups in play.
- The fall sizes and get-ups in play from the game camera: the watcher lost the human's figure after the first goal, so the sheet is of test figures.
- A packaged build at the window's resolution.
- The cook (yours).

## For the other device

1. **Both calls are declared by name** on both figure Blueprints, with `float`, `float`, `int32` and `bool`, `float`. Tested by name from B's harness only.
2. **An editor play session is still drawn at 1398x790** with your line in: the editor's own setting decides there. `r.Editor.Viewport.OverridePIEScreenPercentage 0` makes the editor follow the game.
3. **The line is in `DefaultEngine.ini` twice** (lines 12 and 25).
4. **No stumble in 164 s of `pro` play** on two seeds: all nine checks were knockdowns. Is the stumble band still reachable under the new rules?
5. **The scripted human loses 0 to 3 inside 90 s** on both seeds, so a 100 s seeded match no longer exists for B's proofs.
6. **Hard brakes are unchanged** at about 3.6 a second among ten skaters.
7. **In two captures the goal light turns the whole clay ice pink** for its 108 frames. Not judged; for whoever owns that look.

## Decisions for the operator

1. **Long knockdowns: rise slowly over half a second (`GetUp`, as wired) or lie longer and spring up at the release (`GetUp_LateLong`)?** Strip: `sk23-getup-latelong`.
2. **Short knockdowns: keep snap and quick, or switch to `GetUp_Late`?** It has the figure standing without control for 5 frames against 7 and 13. Strip: `sk23-getup-late`.
3. **The fall-size bars** (light under 90 frames, heavy from 130) are mine; say if they should move.

## The next three evolutions

1. **The real callers, counted.** When device A's calls land: **stumbles shown equal the sim's stumbles and stops shown equal its reverse brakes (ratio 1.00 each) in two seeded matches**, with no stop started by B's own derivation.
2. **The sprint stride and the burst in the animation Blueprint**, by speed with hysteresis. **Proving number: the sprint stride shows on at least 95 % of the frames a skater is over the bar, with no switch back inside 20 frames**, in two seeded matches.
3. **The get-up chosen by rule after the operator's answer**, across all lengths. **Proving number: hips past half height no more than 6 frames before the release on every knockdown from 60 to 150 frames** (today 7, 13 and 27).

## Stills

In `C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt`; un-frozen:

- `20261113-232904-B-sk23-fall-sizes.png`
- `20261113-232905-B-sk23-two-calls.png`
- `20261113-232906-B-sk23-board-fall-wall.png` (without and with the shift, the corner, beside the bench)
- `20261113-233019-B-sk23-getup-late.png`
- `20261113-233020-B-sk23-getup-latelong.png`
- `20261113-233243-B-sk23-resolution-clay.png`
- `20261113-233245-B-sk23-resolution-night.png`

## Ledger rows

Full rows: the two Blueprints, 53 takes with keys replaced, 60 new takes.

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 5.58 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23) (5856029 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 5.78 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23) (6065474 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Board_Fall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (159645 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Board_Fall_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (166095 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Board_Fall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (157399 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Board_Fall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (157483 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (162117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Burst.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export24/p2_cel loopcut, take Burst by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (146794 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Celebrate by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (200049 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (153423 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (155702 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (142135 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Quick_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (145648 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (174149 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (169142 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (169456 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (169516 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (156482 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (156212 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (161716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (170628 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (161255 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (170523 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Poke_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Poke_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (156673 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Shot_Slap.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Shot_Slap by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (184308 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Stumble_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (154572 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Trip by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK23) (163769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Fall_Heavy, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (161961 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Fall_Light, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (138713 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Snap, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (142045 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Snap_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (143222 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Snap_Side_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (145749 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Snap_Side_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (145621 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take GetUp_Wall_Back, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (169656 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Back_Heavy, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (163326 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Back_Light, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (139771 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Left_Heavy, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (174196 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Left_Light, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (142596 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Right_Heavy, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (170749 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p2_cel loopcut, take Knockdown_Right_Light, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (142455 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stride_Sprint.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export25/p2_cel loopcut, take Stride_Sprint, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (190882 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (143197 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (142867 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Side_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (146076 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Side_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (146056 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Wall, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (146710 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Wall_Back, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (145887 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Wall_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (146199 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_Late_Wall_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (146020 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (165738 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (164799 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Side_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (170485 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Side_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (170264 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Wall, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (168205 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Wall_Back, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (168010 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Wall_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (168472 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p2_cel loopcut, take GetUp_LateLong_Wall_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK23) (169598 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Board_Fall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (162321 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Board_Fall_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (168685 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Board_Fall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (160207 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Board_Fall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (160324 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (171296 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Burst.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export24/p3_clay loopcut, take Burst by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (146791 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (154853 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (167537 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (144153 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Quick_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (147628 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Quick_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (147649 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (186969 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (172725 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (173025 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (173066 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (168341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (157380 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (172819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (172158 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (172261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (172040 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Hit.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Lift_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (176826 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Poke_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Poke_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (165984 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Shot_Slap.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Shot_Slap by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (193556 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Stumble_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (160608 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Trip by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK23) (174746 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Fall_Heavy, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (165011 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Fall_Light, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (140401 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Snap, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (142563 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Snap_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (144992 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Snap_Side_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (147467 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Snap_Side_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (146026 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take GetUp_Wall_Back, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (172915 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Back_Heavy, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (169958 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Back_Light, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (141281 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Left_Heavy, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (176167 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Left_Light, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (144236 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Right_Heavy, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (177250 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export24/p3_clay loopcut, take Knockdown_Right_Light, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (144032 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stride_Sprint.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export25/p3_clay loopcut, take Stride_Sprint, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (190819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (145170 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (144890 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Side_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (148150 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Side_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (150083 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Wall, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (148658 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Wall_Back, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (148062 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Wall_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (148247 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_Late_Wall_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (148109 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (169324 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (168374 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Side_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (174068 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Side_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (177452 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Wall, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (174149 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Back.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Wall_Back, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (175303 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Wall_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (172033 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export25/p3_clay loopcut, take GetUp_LateLong_Wall_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK23) (174349 bytes) |
```
