# Lane SK24 report: the real callers counted, the stop gated on the level, the lying loops

Tree `C:\Users\kazda\jam\thirdjam` (`dev-b` e16210c6), port 8002. One editor (pid 33864), left through `ExitEditor` at 00:39:30 and confirmed gone. No GPU hang and no restart in this lane. Nothing committed, nothing pushed, no cook run.

**Tick rate:** 38.5 and 40.6 sim frames per wall second in the two final clay matches (41.7, 40.2 and 42.2 in the first round and on night). Every number is in sim frames.

## Three things first

- **Job 4, the goal strip, was not done.** The first three jobs took the lane.
- **The matches were run twice.** Round 1 with provisional bars (6 frames for everyone, 1.5 s for the human, 12 s for a bot) gave the histogram; round 2 with the final bars is what the tables report. **Night was run in round 1 only**, so its shown-stop counts are for the provisional bars.
- **Both clay matches ran the full 100 s** this time (last sim frames 6272 and 6288), so the counts are per 100 s without scaling.

## Verdict per job

1. **The real callers: done.** 3 of the sim's 3 stumbles shown, each started once, by the pawn's `PlayStumble`. The `check`-bus read is **dropped**, not filtered. Get-up end minus release is 0.0 for all ten knockdowns on clay and all five on night. No Blueprint error in any log.
2. **The stop is an event again: done for the human, mostly for the bots.** The human shows 10 and 12 stops in 100 s (target 10 to 20). The bots show 2 to 8 each; 12 of 18 are inside 6 to 8, six are under, none over.
3. **The lying loops: done.** Frames with no joint moving in a 150-frame knockdown: 1 (was 88 to 103). The 5-frame cross-fade did not cost the hard line; no hard cut was needed.
4. **The goal strip: not done.**

## 1. The real callers

Two clay matches (`pro`, five a side, scripted human, seeds 1 and 7920) and seed 7920 on night.

### Stumbles

| Match | `JAMVIS stumble` lines | Path | Received by the figure | Take played | Started twice |
|---|---|---|---|---|---|
| Clay, seed 1 | 0 | none | 0 | none | 0 |
| Clay, seed 7920 | 3 | all `call=PlayStumble`, `frames=16` | 3 | 3 | 0 |
| Night, seed 7920 | 3 | all `call=PlayStumble`, `frames=16` | 3 | 3 | 0 |

The three in seed 7920, the same on clay and night:

| Sim frame | Who | Direction sent | Take |
|---|---|---|---|
| 4377 | human | (0.04, 1.00) | `Stumble_Right` |
| 4574 | human | (0.50, -0.86) | `Stumble_Left` |
| 5175 | bot 2 | (-0.67, 0.75) | `Stumble_Left` |

**The bus read is dropped.** `contracts/skater-visual.md` v5 says the pawn sends `PlayStumble` in place of `PlayHitReaction` for every stumble, so the `check` read had nothing left to cover. `PlayHitReaction` no longer looks at the bus and `OnBusEvent` no longer stamps the `check` time. A consequence: a `check` event with `Outcome` `stumble` that arrives without a `PlayStumble` call now plays nothing.

**The take ends on `Frames`** by its play rate (32 / `Frames`), and the strip shows the figure upright by frame 15 to 18. I did not log the montage's end frame.

### Stops against the human's brake lines

| Match | `JAMVIS brake` lines | of them `on=1` | Level spans the figure counted | Stops shown |
|---|---|---|---|---|
| Clay, seed 1 | 24 | 12 | 12 | 10 |
| Clay, seed 7920 | 48 | 24 | 24 | 12 |

Every true line reached the figure. The ones not shown are the gate's work (section 2).

### Knockdowns (final round, clay)

| Seed | Who | Frames down | Fall | Lying loop | Get-up | End minus release |
|---|---|---|---|---|---|---|
| 1 | human | 69 | `Board_Fall_Front` (wall 40 cm, shift 5; in place of `Knockdown_Left_Light`) | `Lie_Loop_Wall` | `GetUp_Wall` | 0.0 |
| 1 | human | 61 | `Board_Fall_Front` | `Lie_Loop_Wall` | `GetUp_Wall` | 0.0 |
| 1 | human | 74 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | `GetUp_Quick_Side_Left` | 0.0 |
| 1 | human | 114 | `Knockdown_Left` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 0.0 |
| 1 | human | 96 | `Fall` | `Lie_Loop` | `GetUp` | 0.0 |
| 7920 | human | 98 | `Knockdown_Left` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 0.0 |
| 7920 | human | 63 | `Board_Fall_Left` (wall 40 cm) | `Lie_Loop_Wall_Left` | `GetUp_Wall_Left` | 0.0 |
| 7920 | bot 3 | 78 | `Knockdown_Right_Light` | `Lie_Loop_Side_Right` | `GetUp_Quick_Side_Right` | 0.0 |
| 7920 | human | 69 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | `GetUp_Snap_Side_Left` | 0.0 |
| 7920 | human | 150 | `Knockdown_Left_Heavy` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 0.0 |

Night, seed 7920: the same five knockdowns (98, 63, 78, 69, 150 frames), all 0.0. A lying loop started in every one of the fifteen. Nobody went down again within 120 frames.

## 2. The stop

### What the level does (final round, clay)

| | Seed 1 | Seed 7920 |
|---|---|---|
| `SetBraking(true)` spans, ten figures, 100 s | 153 | 158 |
| Length: min, median, max | 1, 13, 30 | 1, 11, 30 |
| Speed at the start: 10th percentile, median, 90th | 3.6, 10.6, 12.6 m/s | 3.2, 9.9, 12.5 m/s |

Length histogram, in 3-frame bins:

| Frames | 0-2 | 3-5 | 6-8 | 9-11 | 12-14 | 15-17 | 18-20 | 21-23 | 24-26 | 27-29 | 30 |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Seed 1 | 33 | 19 | 10 | 10 | 11 | 3 | 9 | 28 | 16 | 13 | 1 |
| Seed 7920 | 44 | 14 | 13 | 11 | 10 | 11 | 9 | 21 | 16 | 6 | 3 |

**The bar is 6 frames.** The histogram has two humps: flicks of 0 to 5 frames (52 of 153 and 58 of 158) and stops of 18 to 29 frames, with a thin middle. Six frames cuts the first hump off whole.

### The gate

- A pose only when the figure's own speed at `true` is 7 m/s or more.
- **Bots:** the level must have held 6 frames, and 8 s must have passed since that bot's last stop. Without the time bar a bot would show up to 15 in 100 s; with 12 s it shows 3 to 7.
- **The human:** no time bar, and the pose starts on the level's first frame (the lean-in). The count that allows it: of the human's starts from 7 m/s, 1 of 10 and 2 of 13 lasted under 6 frames. That is 3 of 23, under one in five.
- Until a figure receives its first `SetBraking` call, the older path derived from speed still serves it. It fired 5 times in each match, not counted in the table below.

### Per figure, 100 s (0 is the human)

| Figure | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 |
|---|---|---|---|---|---|---|---|---|---|---|
| Seed 1: level spans | 12 | 11 | 12 | 24 | 22 | 15 | 11 | 7 | 14 | 25 |
| Seed 1: from 7 m/s, held 6 frames | 9 | 8 | 9 | 12 | 13 | 8 | 6 | 3 | 9 | 15 |
| Seed 1: **shown** | **10** | 4 | 6 | 7 | 6 | 6 | 3 | 3 | 7 | 8 |
| Seed 7920: level spans | 24 | 14 | 11 | 18 | 19 | 12 | 14 | 11 | 13 | 22 |
| Seed 7920: from 7 m/s, held 6 frames | 11 | 9 | 3 | 14 | 11 | 5 | 12 | 5 | 8 | 13 |
| Seed 7920: **shown** | **12** | 6 | 2 | 6 | 6 | 4 | 7 | 3 | 7 | 6 |

Totals shown: 60 and 59, against 153 and 158 level spans.

- **The human:** 10 and 12 shown. One in each match is a flick the lean-in let through (3 frames, 4 frames); the other 9 and 11 are stops of 13 to 30 frames. One 1-frame level from 10.8 m/s showed nothing.
- **The bots:** every shown one comes from 7 m/s and a level of 6 frames or more. Six of eighteen are under the target of 6. Four of those six bots made 6 or fewer such stops in the whole match, so the bar is not what holds them down.
- **Night, round 1 bars:** 9 for the human, 2 to 6 per bot, 49 in all.

## 3. The lying loops

### Import, export26, by take name

| | Night | Clay |
|---|---|---|
| Takes in the file | 134 | 134 |
| Equal to the asset already there | 110 | 82 |
| Keys replaced (the sixteen late get-ups) | 16 | 16 |
| New assets (the eight loops, 108 keys each) | 8 | 8 |
| Aim holds kept | none | 28 |
| Bad | 0 | 0 |

855 assets load afterwards.

### Wiring

- Each fall carries the frame where it first holds its end pose: light 13, heavy 28, normal 18 or 19, the spun falls 21 to 25, trip 19, board falls 21 or 22.
- When the fall's montage reaches that frame the loop starts, in place of the pause on the last frame.
- Which loop: wall ahead `Lie_Loop_Wall`, wall left, right and behind their own three, `Knockdown_Left` `Lie_Loop_Side_Left`, `Knockdown_Right` and `Trip` `Lie_Loop_Side_Right`, `Knockdown_Back` `Lie_Loop_Front`, the `Fall` family `Lie_Loop`.
- The get-up starts on the frame it always did and blends in over 5 frames (0.083 s); its end is untouched.
- **Re-import by name is enough** for the loops and the falls, as long as a fall's hold frame does not move. The hold frames are constants in the Blueprint; a changed fall needs its number changed.

### Proof

| Measure | Result |
|---|---|
| Frames with no joint moving 0.02 cm, 150-frame knockdown (11 sockets, test figures: on the back, face down, on the side, at the wall) | 1 (was 88 to 103) |
| Largest joint step at the cut into the get-up (frame 115) | 1.8 to 3.7 cm |
| Get-up end minus release, test figures and fifteen match knockdowns | 0.0 |

The one frame is the tick before the loop took over. I then moved the loop's start one frame earlier and did not measure again.

**At game size:** the limb shift reads. In the play strip the near arm and the legs change between frames 42 and 66. The breathing does not show in stills, and I did not judge it in motion.

## Stills

All in `C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt` caption, un-paused, at the window's resolution (`r.Editor.Viewport.OverridePIEScreenPercentage 0`, put back to 1).

- `20261114-004259-B-sk24-stumble-real-caller.png`: the pawn's two `PlayStumble` calls on the human in play, seed 7920.
- `20261114-004259-B-sk24-stop-level.png`: the human's first level-driven stop, seed 1, frames 133 to 147.
- `20261114-004259-B-sk24-lying-loop-test.png`: three test figures through 150-frame knockdowns.
- `20261114-004300-B-sk24-lying-loop-play.png`: the human lying in play from the game camera, seed 1, frame 5242.

## Not verified

- The frozen-frame count after the loop's start moved one frame earlier. Expected 0; measured 1 before the move.
- The breathing in motion at game size.
- The final bars on night. Both Blueprints are written by one builder, so the logic is the same.
- `Lie_Loop_Wall_Right` and `Lie_Loop_Wall_Back` were not played by any match in this lane. `Lie_Loop_Front` played on a test figure only.
- A stumble with `Frames` 15 (the cross-check). All three were 16.
- The spray (`BrakeFx`, lane B40). Its two `Activate` nodes are kept, but it does not read in my stop stills and I did not look for it.
- Whether each shown bot stop looks like a stop. They are counted by the gate, not by eye.
- The stop pose from behind is small in the strip: the skates turn across the travel by frame 6, the body barely changes.
- The sixteen late get-ups replaced by export26 are still behind the test switch, not wired and not re-shot.
- The cook.

## For the other device

- **A third of the `SetBraking(true)` spans are flicks.** 52 of 153 and 58 of 158 last under 6 frames, many of them 1 frame. The figure ignores them; if they are not meant, they are cheap to drop at the source.
- **The human's level turns on at walking speed.** 11 of the human's 24 true lines in seed 7920 start under 7 m/s, down to 1.1 m/s, some with `decel=0.0` on the `on=1` line (seed 1, frame 156). The figure shows nothing for these.
- **`Decel` is not used by the figure's gate.** It gates on its own speed at the call. Values seen: 26.0, 3.2 and 0.0, against the contract's 20 by default.
- **`JAMVIS brake` is logged for the human only.** The bots' counts above come from the figure's own print (`SK24 brk t v n shown`). A line per bot would let device A check them.
- **A stumble without `PlayStumble` now shows nothing** (section 1). If any path still sends only the `check` event, say so and the bus read comes back with the `Outcome` filter.

## Decisions for the operator

1. **Bots under the target.** Six of eighteen bot counts are 2 to 4, under the asked 6 to 8. Lowering the time bar from 8 s to 6 s would lift some of them, at the cost of the busiest bots going over 8. I kept 8 s.
2. **The human's lean-in from the first frame.** It costs one flick shown per match (1 of 10, 1 of 12). The other choice is the 6-frame bar for the human too: no flick, and every stop starts 6 frames late. I kept the lean-in.
3. **The late get-ups** (SK23's open choice) now have export26's versions on disk. They need a new strip before the choice.

## The next three evolutions

1. **The bots' stop without a time bar.** Replace the 8 s with a test on the stop itself (speed lost while the level holds). Proving number: every bot 6 to 8 shown in 100 s on both seeds, 0 shown from under 7 m/s.
2. **The hold frame read from the take.** Measure each fall's first held frame at import and store it, so a changed fall needs no Blueprint edit. Proving number: 0 frames with no joint moving in a 150-frame knockdown, for all 24 falls on both figures (now 1, measured on four).
3. **The stop that reads from behind.** A deeper pose or a body turn for the game camera. Proving number: the figure's silhouette height drops by at least 8 % within 6 frames of the level's start, measured in the game-camera strip.

## State left behind

- Blueprints saved: `BP_SkaterVisual_TB1.uasset` 6,150,353 bytes (was 5,866,902), `BP_SkaterVisual_TB1_P3.uasset` 6,359,856 bytes (was 6,076,347). B40's `BrakeFx` component and both `Activate` nodes are kept, as is AU11's sound node.
- 32 take assets with keys replaced, 16 new. Nothing else in the repository is changed except the four stills, their captions and this report.
- End check: 855 assets load, 16 Blueprints up to date, no dirty package, `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill` and `jam.hitstop` at their defaults, the screen-percentage override back to 1. Scratch cartridges and screenshot files deleted. `/Game/Maps/Rink` not saved.

## Ledger rows

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 5.87 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the pawn's PlayStumble covers every stumble (the check-bus read dropped), the stop driven by SetBraking's level and gated (7 m/s at the start, held 6 frames for a bot with 8 s between its stops, from the first frame for the human), lying loops from the frame the fall holds its end pose (lane SK24) (6150353 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 6.07 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the stumble by the pawn's call only, the gated level-driven stop and the lying loops as BP_SkaterVisual_TB1 (lane SK24) (6359856 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (139837 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (159818 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (168351 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (171669 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (171448 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (167021 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (166826 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (167288 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_LateLong_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (167230 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (144883 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146748 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146728 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146710 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146559 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146871 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p2_cel loopcut, take GetUp_Late_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK24) (146692 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (234685 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Front.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Front (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (231329 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Side_Left.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Side_Left (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (238104 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Side_Right.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Side_Right (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (241528 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Wall (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (207181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Back.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Wall_Back (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (207221 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Left.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Wall_Left (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (207545 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Right.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p2_cel loopcut, take Lie_Loop_Wall_Right (a 108-frame lying loop), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK24) (207366 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (141810 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (163404 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (171926 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (175252 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (175084 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (170597 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (170567 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (173217 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_LateLong_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (173165 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (146906 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148822 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148739 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148658 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148734 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148919 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export26/p3_clay loopcut, take GetUp_Late_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK24) (148781 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (234367 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Front.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Front (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (234598 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Side_Left.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Side_Left (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (237749 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Side_Right.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Side_Right (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (241165 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Wall (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (207041 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Back.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Wall_Back (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (206961 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Left.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Wall_Left (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (207329 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Right.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export26/p3_clay loopcut, take Lie_Loop_Wall_Right (a 108-frame lying loop), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK24) (207224 bytes) |
```
