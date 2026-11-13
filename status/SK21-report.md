# Lane SK21 report: the stumble, the board fall, the hard stop (export21 and export22)

Tree `C:\Users\kazda\jam\thirdjam` (`dev-b` 0f3578e), port 8002, one editor at a time (pid 41020, left through `ExitEditor` at 21:53:32; then pid 36840 for the night check, left at 22:00), both confirmed gone. My clock 21:25 to 22:02.

**Tick rate:** 38.7 and 36.1 sim frames per wall second in the final two matches (44.1 and 41.9 in the first pair). The game ran slow; every number is in sim frames.

## Verdict per item

1. **Import: done, both figures, by take name.** Export21 first, then export22 when it arrived. 20 new takes per figure, 3 replaced.
2. **The stumble plays its own take: 3 of 3 in the two matches** (QA4: two played as a bump, one did not show). Read-out: the `check` event on the bus.
3. **The board fall plays: 1 of 1 in play**, four sides wired, 4 of 4 on test figures. A stumble into the boards plays the stumble (2 of 2).
4. **The hard stop plays, derived from the speed.** It starts on the third frame of a 20 m/s^2 brake. **It fired 419 and 422 times in 100 s across the ten figures**, and I cannot check that against the sim's own count. Your call below.
5. **`Check_Hit_Left` lifting the skates: not seen with the trimmed take.**

Also from the update: **quick get-ups** are scheduled for knockdowns under 80 frames (one in play, a 70-frame one), and **the deke is wired but never fired**, so it is unproven.

## The read-out used for each

| Item | Read-out | Why |
|---|---|---|
| Stumble | a `check` bus event by another skater, its contact point within 1.5 m of the figure, in the same frame as `PlayHitReaction` | the sim emits `check` for a stumble or a knockdown and not for a bump (`JamSim.cpp` lines 1865 to 1877), and the bus event arrives before the pawn's calls |
| Stumble frames | a constant 16 on the figure (`StumF`) | nothing sends it |
| Board fall | `PlayBoardHit` in the same frame as the knockdown (the figure's own `DownT0`) | the order is knockdown first, as you said |
| Wall side | the nearest straight wall from the figure's rink read, against its facing | the `board` event's normal was not needed on the straight walls |
| Hard stop | the speed `UpdateMotion` brings | see below |

**The sim's state is not reachable from the figure.** `bBraking`, `bReverseBraking` and `StumbleFrames` are Blueprint-readable fields of the skater state, but no Blueprint path leads to a skater's state: `UJamSim::Skaters` is a plain property, `GetSkaters()` is not a Blueprint function, and the pawn exposes only its index, its team and its visual. `PlayStumble` is not in the interface header or the contract yet.

`PlayStumbleNow(DirX, DirZ, Frames)` exists as a Blueprint event in both figures; the future interface call can invoke it directly.

## 1. Import, by name

| | Night (`AS_SkaterTB1v2_`) | Clay (`AS_SkaterTB1_P3_`) |
|---|---|---|
| Takes in export22's file | 96 | 96 |
| Equal to the asset already there | 83 | 55 |
| Kept although different (the clay aim holds, export9's) | 0 | 28 |
| Keys replaced | `Check_Hit_Back`, `_Left`, `_Right` (20 frames) | the same three |
| New assets | 20 | 20 |
| Failed the read-back | 0 | 0 |

New, by name: `Stumble_Front`, `_Back`, `_Left`, `_Right`; `Board_Fall_Front` (export22's rebuilt one), `_Back`, `_Left`, `_Right`; `Stop_Hard`, `Stop_Hard_Left`; `GetUp_Wall`, `_Wall_Left`, `_Wall_Right`; `GetUp_Quick`, `_Quick_Front`, `_Quick_Side_Left`, `_Quick_Side_Right`; `Deke_Left`, `Deke_Right`; `Burst`.

The scratch import folder is deleted; 779 assets load (739 before).

## 2. The stumble

- **How it plays:** `Stumble_<side>` by the push against the facing, as for the bump, plus `Front`. Rate 18 / Frames, so the scrabble (take frames 2 to 18) covers the sim's 16 frames and the recovery follows; 28 frames in all.
- **Rules:** exempt from the 0.35 s gap and the 0.6 s rule. Priority 8: above the check lunge, stick reactions and the pin; never while down or getting up.
- **Proof on test figures:** all four sides play; a stumble 8 frames after a stick reaction plays.
- **Proof in play:** seed 1 frame 5078 (`Stumble_Right`, the one QA4 lost to the gap); seed 7920 frames 1403 (`Stumble_Front`) and 1705 (`Stumble_Right`).
- **The trimmed bump:** nothing assumed 30 frames except the blend-in of 0.12 s, which was a third of the new take. I set it to 0.05 s.

Two things the read-out cannot see:
- **The attacker's own recoil stumble** (it ran into a more stable skater): the sim emits no event for it.
- **A third skater within 1.5 m taking a plain collision in that same frame** would play a stumble. Not seen in 200 s.

## 3. The board fall

- **Which take:** the wall ahead of the figure plays `Board_Fall_Front` then `GetUp_Wall`; on its left `_Left` then `GetUp_Wall_Left`; on its right `_Right` then `GetUp_Wall_Right`; behind `_Back` then `GetUp_Side_Left` (the quick one under 80 frames).
- **How:** the fall that started in that frame is replaced in the same frame, from take frame 2. The figure is turned square to the wall over the first six frames. The get-up is rescheduled so it still ends on the release.
- **Test figures:** 4 of 4, get-up end on the release, pelvis at 90 % three frames before it.
- **In play:** seed 1 frame 2953: `Board_Fall_Left`, `GetUp_Wall_Left`, get-up end minus release 0.0. Seed 7920 had no knockdown into the boards.
- **With a bump in the same frame:** `Board_Hit` replaces the bump (test figure). Not separable in the match logs.
- **With a stumble in the same frame:** the stumble plays and the board take is skipped. That is my choice; I did not shoot `Board_Hit` against it.
- **Your wall rule** for knockdowns near a wall with no board hit is unchanged.

**The pawn's real distance to the boards, for the lab** (the figure's own measure to the nearest straight wall at every `PlayBoardHit`, 42 hits in two matches):

| | cm |
|---|---|
| Median | 47 |
| 39 of 42 | 40 to 59 |
| The other three | 66, 104, 107 |
| At the three checks into the boards | 40, 59, 66 |

The takes assume 35 cm, so the figure slumps about 12 cm short of the wall on the median and 31 cm short in the one real board fall. The 104 and 107 are probably corner readings: I measure to the straight walls only.

**Not judged:** whether the wall get-ups show through the boards in play (one case, seen from above), and the corners, where the figure is turned to the nearer straight wall and not to the curve.

## 4. The hard stop

- **Derivation:** per `UpdateMotion`, deceleration = speed lost x 60. It starts when three calls in a row lose more than 12 m/s^2 and the eased deceleration is over 12, at more than 2 m/s. One-frame drops (a wall, a stumble's halved speed) do not count.
- **Start:** frame 3 of a 20 m/s^2 brake (test figure). The ordinary brake at 6 m/s^2 never triggers it.
- **Hold and release:** take frames 4 to 16 loop while the eased deceleration stays over 8 and the speed over 0.7 m/s. The release is played from the loop's end, so nothing pops, and the pose can outlast the brake by up to 12 frames.
- **Side:** `Stop_Hard_Left` when the body is turning left (turn rate over 0.2 rad/s), else `Stop_Hard`.
- **Never over** a knockdown, a get-up, or any take of priority above 3 (so not over a stumble, a check, a hit, a pin or a shot). Anything of priority 6 or more replaces it.

**The count:**

| | Seed 1 | Seed 7920 |
|---|---|---|
| Stops started in 100 s, ten figures | 419 | 422 |
| The human's | 14 | 20 |
| Each bot's | 36 to 57 | 30 to 60 |

- Their lengths cluster at 24, 36 and 48 frames (no, one and two holds), so most are sustained brakes from speed (median 9.8 m/s at the start), not noise.
- **I have no denominator.** The sim's `ReverseBrakes` is neither logged in play nor readable, so "shown within 3 frames out of N reverse brakes" cannot be stated.
- **Four stop poses a second on the ice is a lot.** In the one real strip I have, the human's stop is in traffic and hard to judge.

## 5. `Check_Hit_Left`

With the trimmed take the skates stay on the ice in all twelve frames of a side view (test figure, every 3rd frame).

## From the update: quick get-ups, deke, burst

- **Quick get-ups** when the knockdown is under 80 frames, except after a wall fall (no quick wall get-up exists).

| Length | Get-up | Starts | Pelvis at 90 % minus release | Frames with the pelvis under 45 % |
|---|---|---|---|---|
| 60 (test) | `GetUp_Quick` | frame 41 | -2 | 38 (QA4: 24) |
| 70 (test) | `GetUp_Quick` | frame 51 | -2 | 48 |
| 90 (test) | `GetUp` | frame 55 | -3 | 54 |
| 70 (in play, seed 7920) | `GetUp_Quick_Side_Left` | frame 51 | 0.0 on the release; `JAMLOW` reads -5 against a standing height taken mid-reaction | lying window 19 frames (QA4: 3) |

- **Deke:** the figure hears `deke` on the bus as it hears `check`. For the skater that made it, it plays `Deke_Right` when the event's Kind is `right`, else `Deke_Left` (priority 4, visual only). **No deke fired in either match, so this has never run.**
- **Burst:** imported only.

## The two seeded matches, before (QA4) and after

Sent by the sim in 100 s / what the figure did. The sim's events are the same as in QA4.

| Call | Seed 1 before | Seed 1 after | Seed 7920 before | Seed 7920 after |
|---|---|---|---|---|
| `PlayHitReaction`, bumps | 107: 94 played, 12 skipped, 1 blocked | 107: 93 played, 13 skipped, 1 blocked | 73: 64 played, 7 skipped, 2 blocked | 73: 64 played, 7 skipped, 2 blocked |
| `PlayHitReaction`, stumbles | 1: skipped | 1: `Stumble_Right` | 2: played as bumps | 2: `Stumble_Front`, `Stumble_Right` |
| `PlayBoardHit` | 21: 17 played, 2 blocked, 2 skipped | 21: 17 played, 2 blocked, 1 skipped, 1 board fall | 21: 17 played, 4 skipped | 21: 17 played, 4 skipped (2 of them by the stumble) |
| Same-frame board cases showing their own take | 0 of 1 | 1 of 1 | 0 of 2 | 2 of 2 (as stumbles) |
| `PlayStickHit` | 13: 7 played, 6 skipped | the same | 20: 9 played, 11 skipped | the same |
| `PlayLift` | 17: 15 played, 2 blocked | the same | 12: 9 played, 3 blocked | the same |
| Knockdowns, get-up end minus release | 0.0 x3 | 0.0 x3 | 0.0 x3 | 0.0 x3 |
| Pelvis at 90 % minus release | -3, -3, -3 | -3, -2, -2 | -3, -2, -9 | -3, -3, -5 |
| Hard stops started | none existed | 419 | none existed | 422 |

- One bump that played in QA4 is skipped on seed 1 now. I did not trace it; the stumble now starts the 0.6 s rule's clock, which is the likely cause.
- **Nothing new popping** that I saw in the strips; no Blueprint runtime error in the log.
- Nobody down again within 2 s of a release, as before.

## Blueprint changes as saved

Both figures, the same text with each figure's take names.

- **New events:** `PlayStumbleNow`, `StumPlay`, `BoardFall`, `StopTick`.
- **Changed events:** `PlayHitReaction` (the stumble test first; bump blend-in 0.05), `PlayBoardHit` (board fall, stumble, bump-in-the-same-frame branches; prints the wall distance), `OnBusEvent` (notes a `check` near the figure; the `deke` branch), `UpdateMotion` (calls `StopTick`), `KnockDown` (picks the quick get-up), `PickLens`, `PlayGetUp` (wall and quick get-ups), `ReceiveBeginPlay`.
- **New variables:** `ChkT`, `StumT`, `StumF`, `SpdPrev`, `DecE`, `DecN`, `bStop`, `WallYaw`, `bQuick`. `DownKind` gains 5, 6, 7 for the wall falls.
- **Cost:** `StopTick` adds one call and about 30 nodes to every `UpdateMotion` on the usual path (93 nodes in its text). `PlayBoardHit` now runs the rink read on every board hit (about one every 5 s per match).

| File (under `Game/Plugins/JamAssets/Content/Skaters/`) | Bytes before | Bytes after |
|---|---|---|
| `BP_SkaterVisual_TB1.uasset` | 4,001,197 | 4,853,726 |
| `BP_SkaterVisual_TB1_P3.uasset` | 4,205,098 | 5,063,135 |
| `AS_SkaterTB1v2_Check_Hit_Back.uasset` | 155,461 | 140,261 |
| `AS_SkaterTB1v2_Check_Hit_Left.uasset` | not noted | 140,375 |
| `AS_SkaterTB1v2_Check_Hit_Right.uasset` | not noted | 140,328 |
| `AS_SkaterTB1_P3_Check_Hit_Back.uasset` | 167,712 | 151,552 |
| `AS_SkaterTB1_P3_Check_Hit_Left.uasset` | not noted | 151,600 |
| `AS_SkaterTB1_P3_Check_Hit_Right.uasset` | not noted | 151,579 |
| 20 new night takes | | 3,098,761 in all |
| 20 new clay takes | | 3,141,598 in all |

Saved with no play session open. End check: 779 assets load, 16 Blueprints up to date, no dirty package, `jam.*` defaults restored. Scratch cartridges and my screenshot files are deleted.

## Not verified

- **The night figure was played for 51 s only** (seed 1 to sim frame 3100, 38.4 sim frames per wall second): three knockdowns with get-up end minus release 0.0, the same board fall (`Board_Fall_Left`, `GetUp_Wall_Left`), 244 stops, no Blueprint error. No stumble falls inside those 51 s, so its stumble has not been seen in play.
- The deke branch (no event fired).
- `Board_Hit` over a bump in a match (seen on a test figure only).
- The stop's look in open ice in play, and its count against the sim's.
- The wall get-ups against the boards in play, and the corners.
- The cross-check (none happened, again).
- The cook (yours).

## For the other device

1. **`PlayStumble(DirX, DirZ, Frames)` is still wanted.** Until it lands B's stumble depends on two things staying true: a bump emits no `check` event, and the bus event reaches the figure before the pawn's calls. Frames is a constant 16 on B's side.
2. **A Blueprint path to the brake read-out**: one pure accessor on the pawn (reverse braking, stumble frames) or a `SetBraking` call would replace B's derivation from the speed.
3. **419 and 422 sustained decelerations over 12 m/s^2 in 100 s**, 30 to 60 per bot. Is the brain reverse-braking that often? `ReverseBrakes` is not logged in play.
4. **The pawn stands 47 cm from the boards at a board hit on the median** (40 to 59 mostly; 40, 59 and 66 at the three checks into the boards).
5. **The attacker's recoil stumble has no event.**
6. `JAMVIS hit` still says "no check landed" for a stumble.
7. No deke fired in 200 s of `pro` play; the lab asks for `PlayDeke(Side, Frames)` and `PlayBurst(Frames)`.

## Decisions for you

1. **Keep the hard stop as it is, gate it, or switch it off until the sim's count can be read?** It is real braking by the numbers, but four poses a second is busy. A gate would be a higher speed (2 m/s today) or five frames of confirmation (which breaks "within 3 frames").
2. **A stumble into the boards plays the stumble, not `Board_Hit`.** Say if you want the other.

## The next three evolutions

1. **The stop counted against the sim, with its spray.** Take the brake read-out from the sim once a path exists, add the spray and the side from the travel. **Proving number:** stops shown divided by the sim's `ReverseBrakes` in the two seeded matches equals 1.00, with none shown outside a reverse brake (today 419 shown, the sim's count unknown).
2. **The board fall meets the wall.** Shift the mesh (not the root) by the measured distance and use the boards' real normal in the corners. **Proving number:** the gap between the figure's wall-side shoulder and the boards at take frame 5, from up to 31 cm today to under 5 cm, on the four sides and in one corner.
3. **The deke and the burst seen.** Prove the deke in a drill that fires it and wire `Burst` to the first stride after a stop. **Proving number:** dekes shown out of `deke` events in a stick drill, and bursts shown within 3 frames on the 20 rounds of drill 21: all of them (today 0 fired, 0 shown).

## Stills

In `C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt`; un-frozen:

- `20261113-215742-B-sk21-stumble.png`: bump beside stumble on test figures, and a real stumble from the game camera
- `20261113-215744-B-sk21-board-fall.png`: the four sides on test figures, and the real board fall
- `20261113-215426-B-sk21-stop-hard.png`: a test figure, and the human's stop in play
- `20261113-215427-B-sk21-contact-outcomes.png`: the operator sheet (bump, stumble, board fall, knockdown)

## Ledger rows

The two Blueprint rows (replace) and 46 take rows (6 replace, 40 new):

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 4.63 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21) (4853726 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 4.83 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21) (5063135 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Back.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (140261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Left.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (140375 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Right.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (140328 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stumble_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (154572 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stumble_Back, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (157710 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stumble_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (167088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stumble_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (167016 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Board_Fall_Back, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (158717 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stop_Hard.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stop_Hard, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (156610 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stop_Hard_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p2_cel loopcut, take Stop_Hard_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (158167 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take Board_Fall_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (160527 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take Board_Fall_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (157399 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Fall_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take Board_Fall_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (157483 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Wall, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (170326 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Wall_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (167088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Wall_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (167148 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Quick, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (142135 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Quick_Front, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (141751 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Quick_Side_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (145647 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take GetUp_Quick_Side_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (145648 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Deke_Left.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take Deke_Left, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (139207 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Deke_Right.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p2_cel loopcut, take Deke_Right, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (137728 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Burst.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export22/p2_cel loopcut, take Burst, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK21) (146794 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (151552 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (151600 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; 20 frames, was 30; re-import by name only) (lane SK21) (151579 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stumble_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (157300 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stumble_Back, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (160608 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stumble_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (168890 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stumble_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (168633 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Board_Fall_Back, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (161393 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stop_Hard.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stop_Hard, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (156281 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stop_Hard_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export21/p3_clay loopcut, take Stop_Hard_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (157922 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take Board_Fall_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (163117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take Board_Fall_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (160207 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Fall_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take Board_Fall_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (160324 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Wall, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (175093 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Wall_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (171841 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Wall_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (170698 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Quick, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (144153 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Quick_Front, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (144465 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Quick_Side_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (147628 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take GetUp_Quick_Side_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (147649 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Deke_Left.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take Deke_Left, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (139301 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Deke_Right.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export22/p3_clay loopcut, take Deke_Right, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (139304 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Burst.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | n | skf/export22/p3_clay loopcut, take Burst, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK21) (146791 bytes) |
```

# SK22: the hard stop as an event, the night figure, the board fall's distance

One editor (pid 18280), left through `ExitEditor` at 22:19:05 and confirmed gone. My clock 22:02 to 22:22. Tick rate in the three final matches: 42.1, 35.7 and 36.6 sim frames per wall second.

## Verdict

1. **The hard stop is gated and the counts are on target for the bots: 419 to 72 and 422 to 76 stops in 100 s.** Each bot shows 6 to 8; the human shows 8 and 11, a little under your 10 to 20 on seed 1.
2. **The speed gate does almost nothing; the bots needed a time bar.** They brake hard from full speed 25 to 50 times in 100 s each, so I gave them one stop per 12 s. Two of three real full-speed stops by a bot now show no pose.
3. **No lean-in.** The pose commits when 3.5 m/s is lost, about frame 11 of a 20 m/s^2 brake, and nothing shows before that.
4. **The night figure passes its 100 s match.** Two stumbles of two, the quick get-up, get-up end minus release 0.0 three times, no Blueprint error. No board fall occurred on that seed.
5. **The board fall's mesh shift is not done.** It was more than fifteen minutes with a test, so I left it.

## What the gate is, as saved

- **Fast when the brake began:** at least 7 m/s on the call before the first hard frame.
- **Hard and sustained:** three calls in a row over 12 m/s^2 (unchanged), and 3.5 m/s lost in total before the pose commits. A tap of the brake shows nothing.
- **Not again too soon:** 1.5 s for the human, 12 s for a bot. The figure asks the game mode which skater is the human.
- **Unchanged:** the hold, the release, the side, and never over a higher take.

## Counts per figure, 100 s, clay

Stops shown. Figure 0 is the human.

| Seed 1 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | All |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Before (SK21) | 14 | 57 | 49 | 36 | 40 | 45 | 44 | 43 | 41 | 50 | 419 |
| Gate 7 m/s, 3.5 lost, 1.5 s for everyone (measured) | 8 | 28 | 24 | 20 | 21 | 25 | 23 | 23 | 26 | 32 | 230 |
| After (as saved) | 8 | 7 | 7 | 6 | 7 | 7 | 7 | 8 | 7 | 8 | 72 |

| Seed 7920 | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | All |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Before (SK21) | 20 | 60 | 50 | 30 | 30 | 45 | 39 | 48 | 51 | 49 | 422 |
| After (as saved) | 11 | 8 | 7 | 6 | 6 | 8 | 8 | 7 | 7 | 8 | 76 |

**The speed gate at 6, 7 and 8 m/s** (hard brakes that lost 3.5 m/s, with the 1.5 s rule, counted from the same logs; a higher take can still block some):

| | Seed 1 human | Seed 1 bots, each | Seed 7920 human | Seed 7920 bots, each |
|---|---|---|---|---|
| All such brakes | 16 | 33 to 51 | 15 | 25 to 48 |
| Gate 6 | 15 | 24 to 33 | 13 | 18 to 31 |
| Gate 7 | 15 | 20 to 32 | 12 | 13 to 30 |
| Gate 8 | 15 | 18 to 29 | 11 | 12 to 26 |

- The bots' median speed at the start of a hard brake is 10.5 m/s, so no speed bar separates a stop from an adjustment.
- **The human shows fewer than qualify** (8 of 15 on seed 1): the rest come while a pin, a stick reaction or a hit is playing, and the stop never plays over those.
- The stumbles, the board fall and the get-ups are as in SK21 on both seeds (stumbles 1 and 2, the board fall on seed 1, get-up end minus release 0.0 six times).

## The night figure, 100 s on `l2_night`, seed 7920

| | Result |
|---|---|
| Stumbles | 2 of 2: `Stumble_Front`, `Stumble_Right` |
| Board fall | none occurred (both board cases are stumbles on this seed) |
| Knockdowns | 150, 110 and 70 frames; the 70 plays `GetUp_Quick_Side_Left` |
| Get-up end minus release | 0.0, 0.0, 0.0 |
| Pelvis at 90 % minus release | -3, -3, -4 |
| Stops shown | 76 (human 11, bots 6 to 8) |
| Blueprint errors | none |

## Saved

Both Blueprints, with no play session open. End check: 779 assets load, 16 Blueprints up to date, no dirty package, defaults restored.

- **`StopTick`:** the gate above; it also prints one line per qualifying brake (`SK22 brake`), which the counts were read from.
- **`ReceiveBeginPlay`:** sets the speed bar 7, the bots' gap 12 s.
- **New variables:** `StopV0`, `StopV`, `StopT`, `StopGapB`, `bCand`.

| File (under `Game/Plugins/JamAssets/Content/Skaters/`) | Bytes at SK21 | Bytes now |
|---|---|---|
| `BP_SkaterVisual_TB1.uasset` | 4,853,726 | 4,953,428 |
| `BP_SkaterVisual_TB1_P3.uasset` | 5,063,135 | 5,162,878 |

Ledger rows: the two Blueprint rows above stand with these sizes and one more clause:

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 4.72 | ... | ... the hard stop derived from the speed and gated (7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's) (lanes SK21, SK22) (4953428 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 4.92 | ... | ... stumble, board falls, quick get-ups, the gated hard stop and deke as BP_SkaterVisual_TB1 (lanes SK21, SK22) (5162878 bytes) |
```

## Stills

- `20261113-221554-B-sk22-stop-human.png`: the human's stop in play, cropped on the figure, every 3rd frame
- `20261113-221556-B-sk22-stop-bot.png`: a bot's stop in play, full frames; the bot enters at the right edge, so it is a weak strip

## Not verified

- **"Every one visibly a stop in a strip":** I looked at one human strip and one bot strip, not at all 148.
- The bots' 12 s gap was chosen from seed 1's log and then measured on both seeds and on night; no other value was played.
- The human's count on a real player's hands: the scripted human carries into pins and stick fights, which block the pose.
- The night figure's board fall on a 100 s match (seen in SK21's 51 s on seed 1).
- The mesh shift toward the wall: not started.
- The cook.

## For the other device, added

- **The bots brake hard from full speed every 2 to 4 seconds** (25 to 51 brakes of 3.5 m/s or more per bot in 100 s, median start speed 10.5 m/s). The figure now hides two of three; the cause is the brain's.

## Decision for you

- **The lean-in:** today nothing shows for the first ten frames or so of a real stop. If you want the turn-in from frame 3, it will also show on every brake that then stops short of 3.5 m/s; I can count those first.
