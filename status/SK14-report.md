# SK14 report: the tackling Blueprint changes and export17, proven in real matches (device B)

Lane SK14, device B, machine clock 12:38 to 13:25. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` 934316a), editor on port 8002 (pid 12844), left through `ExitEditor`; the process is gone. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3` and 46 takes. `/Game/Maps/Rink` not saved; no `_Scratch/`; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill` and `jam.hitstop` back to their defaults. Port 8000 (the clay lane) not touched. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk13` (`bp14.mjs` and `bp14b.mjs` the Blueprint changes, `import17.py` the import, `react14.py` / `react14.sh` the match strips, `rec14.py` the per-tick recorder, `pair14.mjs` the counts, `bk14/` the two Blueprints and `bk17/` the 152 takes as they were at lane start).

## Verdicts

| Change | Verdict |
|---|---|
| 1. Get-up against the hit-stop | **PASS**: the get-up ends within 0.05 frames of the release at `jam.hitstop` 0, 3 and 6 (it was -3.0 and -6.0) |
| 2. The human's lunge | **PASS with a limit**: the lunge is no longer restarted; at the hit it is 0, 7, 8, 16, 1, 7, 7, 5 frames in (it was 0 every time). Two of eight still land on frame 0 or 1 |
| 3. `Fall_B`, `Knockdown_Back_B` wired | **PASS**: both seen in real matches, the B length is used |
| 4. A hit pose in the stop | **PARTIAL**: applied (frame 2, no blend under the stop). The held frame reads as a hit from the front and the side when the lunge is 5 or more frames in (6 of 8); it does not when press and hit coincide (2 of 8), and from the front and from behind the human's body hides most of the victim |
| 5. export17 | **PASS**: 23 takes per figure by name, proof run clean, every fall to get-up join exact on the assets |

| Chain item (real matches, hit-stop 3) | Verdict |
|---|---|
| The human checks: press to get-up | **PASS**: 8 knockdowns by the human on clay, every chain complete; one pass on night |
| The human knocked down by a bot | **NOT SEEN**: no bot knocked the human down in any match of this lane |
| Plain side fall from A's camera | **PARTIAL**: one plain `Knockdown_Left`; it reads as a hit from the side through hit +4, then the body is inside the boards |
| A knockdown at the boards | **FAULT FOUND**: a side knockdown 0.6 m from the side boards puts the trunk and head through the wall |
| B30's effects in a real knockdown | **PASS on clay, weak on night**: the figures stay uncovered; the slide trail shows on clay for about a second |
| Reactions pair with `JAMVIS` | **PASS**: every count pairs |

## The Blueprint changes as saved (both Blueprints, the same text)

1. **`ReceiveTick`, first statement:** while the actor's `CustomTimeDilation` is under 0.01, the world delta (`GetWorldDeltaSeconds`) is added to `DownT0`, `UpT0`, `UpEndEst` and `LastHitT`. That covers the whole get-up schedule and the 0.6 s reaction rule (one line each).
2. **`PlayCheck`:** the two `PlayCut` calls run only if the figure has no active montage or `ActPri` is not 7.
   - "Active" is `GetCurrentActiveMontage`, not `IsAnyMontagePlaying`. I changed this after the first match: the 30-frame lunge blends out from frame 18, and with `IsAnyMontagePlaying` a second press inside that blend-out got no lunge at all (seen once: press 7 frames before a hit, no take).
   - A held key now replays the lunge every 18 frames.
3. **`KnockDown`:** `bSpin` is set before `PickLens`. For `DownKind` 3 it plays `Knockdown_Back_B` when `bSpin`, else `Knockdown_Back`; in the front branch `Fall_B` when `bSpin`, else `Fall`.
4. **`PickLens`:** calls `SetLen` with the B take when `bSpin`, for all four kinds. Before, the side spins were scheduled with the plain take's length.
5. **Start at the contact frame:** a new float `DownStart`, set in `KnockDown` to 2 frames (0 for a trip).
   - `PlayDown` starts the take at `DownStart`.
   - `PlayDown` blends in over 0 s when the hit-stop holds the figure at the call (A sets the dilation before `SetKnockedDown`), else over 0.03 s (it was 0.06 s). With any blend the held frames would show the skating pose, because the blend stands still with the figure.
   - `SetLen` stores the fall as its length minus `DownStart`, so the get-up scale and start count the frames that play (24; 26 and 28 for the B takes). The hold test is on the take's position and needed no change.

Both compile `BS_UP_TO_DATE`; the read-back differs from the lane-start text in these six events only (`ReceiveTick`, `PlayCheck`, `KnockDown`, `PickLens`, `SetLen`, `PlayDown`).

## 1. Get-up against the hit-stop (clay, drill 6, the same seeded match at three settings)

Get-up end minus release, in frames, from the Blueprint's own `SK6REL` line:

| `jam.hitstop` | Sent frames | Knockdowns | Knockdowns my captures did not touch | Knockdowns frozen for captures |
|---|---|---|---|---|
| 0 | 90 | 8 | 0.00, -0.05, 0.00, -0.01, 0.00, 0.00 | 0.88, -0.12 |
| 3 | 87 | 9 | 0.00, 0.00, 0.00, 0.00, 0.00, 0.00 | 0.91, -0.07, -0.07, -0.07, -0.08, -0.08, -0.08 (clay); -0.10, -0.12 (night) |
| 6 | 84 | 8 | 0.00, -0.05, 0.00, -0.01, 0.00, 0.00 | -0.07, -0.05 |

- Before the change it was -3.0 at stop 3 and -6.0 at stop 6.
- Stop 3 was run twice on clay (a 49 s pass with 7 knockdowns and a 62 s pass with 9); the untouched values are five from the second pass and one from the first.
- The two columns differ because my strips freeze the whole match (`slomo`) for each capture; that disturbs the estimate by up to a frame even at stop 0. The untouched knockdowns are the measure.
- Fall and get-up in world frames after the call (stop 0 / 3 / 6): the plain fall reaches its hold at 23 / 26 / 29, the B takes at 25 / 28 / 31, the get-up starts at 46 to 47, the release is at 90 in all three.
- The 0.6 s reaction rule got the same line; I did not measure it separately.

## 2. The human's lunge (drill 6, 54 s, 8 knockdowns by the human, final Blueprint)

| Sim frame | Kind, strength | Press to hit (frames) | Lunge frame at the hit | Take |
|---|---|---|---|---|
| 158 | front 1.00 | 0 | 0 | `Fall_B` |
| 821 | front 1.00 | 7 | 7 | `Fall_B` |
| 1275 | behind 0.05 | 26 (key held) | 8 (second lunge of the held key) | `Knockdown_Back` |
| 1801 | behind 0.07 | 16 | 16 | `Knockdown_Back_B` |
| 2679 | front 1.00 | 1 | 1 | `Fall_B` |
| 2907 | behind 0.43 | 7 | 7 | `Knockdown_Back` |
| 2932 | side_l 0.75 | 7 | 7 | `Knockdown_Left` |
| 3092 | front 1.00 | 5 | 5 | `Fall_B` |

- It was 0 every time in SK13. Now the lunge frame equals the frames since the press.
- `PlayCheck` still arrives on every held frame: 127 calls in 14 presses.
- **Two hits of eight still land on lunge frame 0 or 1**, because the press and the contact fall in the same sim frame or the next. There the human stands in the skating pose through the stop. No Blueprint rule on `PlayCheck` can change that; see the decisions.
- With the first form of the rule (`IsAnyMontagePlaying`), frame 2932 had no lunge at all and frame 1275 was in the blend-out; that is why I changed it.

## 3. `Fall_B` and `Knockdown_Back_B`

- Seen in real matches: `Fall_B` 4 times (every front hit), `Knockdown_Back_B` once, each followed by its get-up (`GetUp`, `GetUp_Front`).
- The schedule uses the B length: `SK6KD` prints fall 0.4333 s for both (28 frames minus the 2 skipped), get-up start at frame 43 of 87.
- **Why `Fall_B` on every front hit:** the sim's strength is closing speed over top speed, and a head-on check at speed is 1.00. `bSpin` is "strength 0.95 or more, or every third knockdown of the figure". So the human's front hits always get the B take and plain `Fall` did not play once. See the decisions.
- `Fall_B` reads as knocked back onto the backside with both arms thrown up, then flat on the back. `Knockdown_Back_B` reads as a flat dive forward onto the belly.

## 4. The hit pose in the stop

- The take starts at frame 2 with no blend when the stop holds the figure. The victim's held pose is the snap. At this camera I can see the trunk tilted away from the hit; I cannot see the skate that SKF lifts in frame 2.
- **Does the frozen frame read as a hit from A's camera? It depends on the lunge, not on the victim.**
  - Lunge 5 or more frames in (6 of 8 hits; I have held frames for 7 from the front, 7 from the side and 16 from behind): yes from the front and the side. The human is low with the shoulder in the victim, the victim is tilted away. From behind at 16 it is weak, for the reason two lines down.
  - Lunge at 0 or 1 (2 of 8): no. Two figures stand close; the human is upright.
  - From the front and from behind, the human's own body covers most of the victim, because the camera sits behind the human. The snap is seen clearly only in the side hit and when the victim is off the camera's line.
- Without the stop (stop 0, blend 0.03 s from frame 2): no pop that I can see in consecutive frames (hit, +1, +2, +3). A's camera kick moves the picture in the same frames, so this is a weak judgement.
- I did not try frame 1: frame 2 did not pop.

## 5. export17

- 23 takes per figure differ from the lane-start state and were copied by name: the 21 in SKF's note on both figures, plus `Stumble` and `Stumble_Short`.
- **My comparison was too loose until now.** It let rotation differences under about 1.6 degrees pass as equal, so the first pass skipped clay's `Stumble` and `Stumble_Short`. I tightened it to 0.0003 on every quaternion component and position, compared every frame, and ran it again.
- Proof run after saving: night 76 of 76 equal; clay 46 equal, the 28 aim holds kept, and the two stumbles copied in that run, then saved. A last proof run before exit: night 76 of 76 equal, clay 48 equal with the 28 aim holds kept, nothing copied. 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs are up to date, no dirty package, no scratch folder left.
- Joins on the assets, last key of the fall against the first key of its get-up, all 10 pairs on both figures (the B takes and `Trip` included): 0.000 cm and 0.00 degrees. `Fall` to `GetUp` was 6.3 degrees (night) and 27.5 degrees (clay).

## The chain in real matches (hit-stop 3)

### The human checks
- Clay, drill 6: 8 knockdowns by the human in 54 s. Takes: `Fall_B` 4, `Knockdown_Back` 2, `Knockdown_Back_B` 1, `Knockdown_Left` 1. Get-ups: `GetUp` 4, `GetUp_Front` 3, `GetUp_Side_Left` 1.
- Each fall played from frame 2 to its hold, held, and the get-up ended on the release (table in section 1). Slides were 1.2 to 7.2 m.
- Night, drill 6: the same match; two chains captured, both `Fall_B`.
- Not seen: `Fall`, `Fall_Forward`, `Trip`, the side spins. `Knockdown_Right` played once, bot on bot, in the log only.
- Drill 5 on `all_star` gave nothing: 70 s, two attempts, no check landed. I did not repeat it on `pro`.

### The human knocked down by a bot
- Not seen. Drill 6 on `pro` (five passes of the same match, 30 to 62 s each): the human was never checked. A 70 s match on `all_star` with the scripted human (`jam.ScriptHuman 1`): one knockdown, and it was bot on bot: bot 3 checked the human's team-mate from the side (`side_r`, strength 0.79), plain `Knockdown_Right` then `GetUp_Side_Right`, no hit-stop, get-up end minus release 0.00. 47 collisions had no landed check and the human was not hit.
- So the human's own fall, its hit-stop and the camera on it are not checked with export17 and this Blueprint.

### Plain side fall from A's camera
- Seen twice (sim frames 2932 and 3245, `side_l`, strength 0.75 and 0.61); the first was captured. Plain `Knockdown_Left`, then `GetUp_Side_Left`; the human's lunge was 7 frames in.
- **It reads as a hit from the side through hit +4.** In the held frames the human's shoulder is in the victim's left side and the victim is tilted away; from +3 the victim goes over sideways, away from the checker. It is the clearest of the hits, because the camera sees both figures.
- I cannot judge the rest of the fall: this one happened 0.6 m from the side boards and from +6 the body is inside the wall (next item). The second one was in open ice but not captured.
- `Knockdown_Right` played once, bot on bot with no hit-stop (`all_star` match); it is in the log only, not captured.

### A knockdown at the boards
- **Side boards, `Knockdown_Left`, the pawn 0.61 m from the wall (it never comes closer than 0.47 m):** the take throws the body sideways toward the wall. From hit +6 the trunk, head and arms are inside the boards and only the legs show; while lying (+18 to +40) the upper half is still in the wall. So it is the body, not only the stick.
- **End boards, `Knockdown_Back`, 0.8 m from the wall, sliding along it:** nothing through the wall; the stick lies along the body.
- The side takes carry the body about a metre to the side of the root (by eye). Any side knockdown within that distance of the boards on the far side will do the same. See the decisions.

### B30's effects in real knockdowns
- **During the three held frames:** the ring of team-colour dots lies flat on the ice around the skates and keeps spreading while the figures stand still (the effect is not held by the stop), and the small white star sits between the skates. Nothing is above shin height.
- **From the hit to +12:** the dots and the white chips stay at skate and shin height. They cover the skates and part of the shins of the nearer figure, which I put at under about 5 % of the two figures; no frame has anything over a trunk, head or arm. This is by eye from the frozen frames, not a pixel count.
- **Slide trail, clay:** visible from about +18 as pale, glassy blue-grey dots at the skates and behind the body; at +60 a trail of about 25 dots lies behind the victim. They read as light bubbles, not as a dark scrape. Gone by +84, so the trail stays about 1 to 1.4 s after the hit.
- **Slide trail, night:** a small cluster of white dots at the skates from +18 to +40. Visible, but short: no trail behind the body in the frames I have.

### Reactions against `JAMVIS` (counts, clay drill 6 at stop 0 / stop 3 first pass / stop 6)
| Pair | Stop 0 | Stop 3 | Stop 6 |
|---|---|---|---|
| `JAMVIS knockdown` / `SK4KD` | 8 / 8 | 7 / 7 | 8 / 8 |
| `JAMVIS hit` / played + blocked + skipped | 29 / 21 + 2 + 6 | 29 / 21 + 2 + 6 | 29 / 21 + 2 + 6 |
| `JAMVIS pin` on / `SK6RX pin on` | 2 / 2 | 2 / 2 | 2 / 2 |
| `JAMVIS board` / `SK6RX board` | 9 / 9 | 9 / 9 | 9 / 9 |
| `JAMVIS stickhit` / `SK6RX stickhit` | 7 / 7 | 6 / 6 | 7 / 7 |
| `JAMHITSTOP` / knockdowns | 0 / 8 | 7 / 7 | 8 / 8 |

Night (two knockdowns) and drill 5 on `all_star` (no knockdown; 55 hits = 39 played + 16 skipped, 5 pins, 39 board hits) pair the same way. The stop 3 pass ended at 49 s, one knockdown short of the others.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 2,860,784 | 2,985,941 |
| `BP_SkaterVisual_TB1_P3` | 2,918,758 | 3,044,161 |
| 23 `AS_SkaterTB1v2_*` | 3,778,322 | 3,775,602 |
| 23 `AS_SkaterTB1_P3_*` | 3,996,139 | 3,993,419 |

The 23 takes: `Board_Hit`, `Check`, `Check_Deliver_Left`, `Check_Deliver_Right`, `Check_Hit_Back`, `Check_Hit_Left`, `Check_Hit_Right`, `Fall`, `Fall_B`, `Fall_Forward`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right`, `Knockdown_Back`, `Knockdown_Back_B`, `Knockdown_Left`, `Knockdown_Left_B`, `Knockdown_Right`, `Knockdown_Right_B`, `Stumble`, `Stumble_Short`, `Trip`. Nothing else under `Skaters/` changed.

## Stills (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with `.txt`)

- `20261113-131713-B-sk14-human-check-clay.jpg`: SK14 operator sheet, real match on clay (drill 6, hit-stop 3, A's game camera, export17, the SK14 Blueprint): the human presses 7 frames before the hit, the three held frames, Fall_B from frame 2, slide, lying, GetUp ending on the release. Tiles follow the two figures, then the victim from +18.
- `20261113-131714-B-sk14-fall-b.jpg`: SK14: Fall_B in a real match on clay (front hit at strength 1.00, press and hit in the same sim frame, so the human's lunge is at frame 0 in the held frames), hit to +84, A's game camera.
- `20261113-131715-B-sk14-knockdown-back-b.jpg`: SK14: Knockdown_Back_B in a real match on clay (hit from behind, the human's lunge 16 frames in at the hit), press and hit to +84, A's game camera; the slide trail shows at +26 to +60.
- `20261113-131716-B-sk14-boards.jpg`: SK14: knockdowns at the boards in real matches on clay. Row 1: Knockdown_Left 0.6 m from the side boards, the trunk and head go through the wall and stay there while lying. Row 2: Knockdown_Back 0.8 m from the end boards, sliding along them, nothing through the wall.
- `20261113-131716-B-sk14-side-fall.jpg`: SK14: plain Knockdown_Left from A's game camera in a real match on clay (sim frame 2932, side_l 0.75, lunge 7 frames in). The hit reads as from the side through +4; the pawn is 0.6 m from the side boards and from +6 the body is inside the wall.
- `20261113-131717-B-sk14-slide-trail.jpg`: SK14: B30's slide trail in real slides at A's game camera. Rows 1 and 2 clay (pale blue-grey dots behind the body, about 25 at +60), row 3 night (a short white cluster at the skates).
- `20261113-131718-B-sk14-night.jpg`: SK14: one chain on night in a real match (drill 6, hit-stop 3): press, hit 7 frames into the lunge, the held frames, Fall_B, slide, GetUp.
- `20261113-131837-B-sk14-hitstop-frame.jpg`: SK14: the held frame of A's hit-stop (3 frames) in real matches at A's game camera, six cases on clay and night. The victim is at frame 2 of its fall (the snap). It reads as a hit when the human's lunge is 5 or more frames in; with press and hit in the same frame the human stands upright; from the front and from behind the human's body hides most of the victim.

## Ledger rows

Replace the two Blueprint rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 2.85 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lane SK12): check at priority 7 with 0.03 s blend-in, fall at rate 1.0 held on its last pose, a board pin no longer replaces a running check; + SlideFx Niagara component (NS_slide_trail), activated on KnockDown down, deactivated on release (lane B30); tackling (lane SK14): the get-up schedule and the 0.6 s reaction rule follow the actor's own clock through A's hit-stop, a check take is not restarted while it plays, B takes (Fall_B, Knockdown_Back_B, Knockdown_Left_B, Knockdown_Right_B) on a hit of 0.95 or every third knockdown with their own lengths, falls start at frame 2 with no blend under the hit-stop and 0.03 s otherwise (2985941 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 2.90 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling as BP_SkaterVisual_TB1 (lanes SK12, SK14); + SlideFx Niagara component (NS_slide_trail) (lane B30) (3044161 bytes) |
```
Replace the 46 take rows:
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Board_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (162117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (173072 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check_Deliver_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (170387 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check_Deliver_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (173282 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (155461 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (163511 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Hit_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (163464 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Fall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (153719 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_B.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (151567 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Forward.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (155702 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (175784 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (177794 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (186134 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (186181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (156482 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (156212 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (161716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (164676 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (161255 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (164571 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Stumble by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (147206 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Short.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Stumble_Short by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (151540 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Trip.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p2_cel loopcut, take Trip by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK14) (163769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Board_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (171296 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (182319 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check_Deliver_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (179694 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check_Deliver_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (182529 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check_Hit_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (167712 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check_Hit_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (174704 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Check_Hit_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (174683 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Fall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (165583 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_B.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Fall_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (151141 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Fall_Forward by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (167537 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (189357 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (191277 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (196848 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (199769 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (168341 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back_B.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Back_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (157380 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (172819 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Left_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (166206 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (172261 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_B.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Knockdown_Right_B by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (166088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Stumble by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (158487 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Short.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Stumble_Short by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (162642 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export17/p3_clay loopcut, take Trip by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK14) (174746 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- The human knocked down by a bot.
- In a real match with export17: `Fall`, `Fall_Forward`, `Trip`, the two side spins (so the B length on the side spins is proven by the text only), a plain side fall in open ice past hit +4 (`Knockdown_Right` played once, in the log only).
- Frame 1 as the start of the falls. The pop without the stop is judged from four consecutive frames under A's camera kick.
- The 0.6 s reaction rule under the stop: same line as the get-up, not measured.
- Effect coverage as a pixel count; the slide trail between +60 (there) and +84 (gone).
- Drill 5 on `pro`.
- The stills `sk14-human-check-clay`, `sk14-fall-b`, `sk14-knockdown-back-b` and the clay rows of `sk14-slide-trail` are from the first stop 3 pass, shot before I changed the `PlayCheck` rule to its final form. Those are single presses; the later passes give the same lunge frames for them (0, 7, 16).
- The lunge change on bots: only the human's presses were measured.
- Motion at speed by eye: strips and per-tick numbers only.

## Decisions for you

1. **The lunge at 0 or 1 when press and contact coincide (2 of 8).** B can fix it in the figure: on the sim's `check` bus event with the figure as the checker, if the lunge is under frame 6, restart it at frame 6 with no blend. Then the held frame always shows the shoulder in. It is one more change to `OnBusEvent` in both Blueprints; not applied, not tested. Say if you want it.
2. **`bSpin` on strength 0.95.** As it stands the human's head-on hits always play `Fall_B` and never `Fall`. Either keep it (the big take for the full-speed hit, which is what the rule says), or drop the strength clause so the B takes come every third knockdown only. I would keep it and raise nothing.
3. **Held key replays the lunge every 18 frames.** That follows the sim, which re-arms the check on every held frame. If A sends one `PlayCheck` per press this goes away by itself.
4. **Night slide trail:** short white cluster only. B30's open question (a night-only colour) stands.
5. **Side falls next to the boards.** The side takes throw the body about a metre off the root, so within that distance of a wall on the far side the body goes through it. Three ways, none applied: (a) the figure traces 1.5 m toward the fall side in `KnockDown` and, if it hits the boards, plays the front or back take instead (B, both Blueprints, one more lane); (b) A sends the distance to the boards along the knock direction or a flag; (c) the Blender lane builds side takes that fall in place. I would take (a).

## For device A

1. `PlayCheck` once per press instead of on every held frame (127 calls in 14 presses). The figure no longer needs it, but a held key is a permanent check in the sim, and the lunge loops with it.
2. In two of eight hits the press and the contact are 0 or 1 frames apart, so no lunge can be seen at the hit. A cannot change that without delaying the hit; B's bus-event change above is the cheaper answer.
3. From the front and from behind, the camera behind the human hides most of the victim during the stop. If the stop should show the victim, the camera kick would have to move sideways; I only report what the frames show.
4. Bots landed one check in all these matches. On `pro` (drill 6, 54 s) all 8 knockdowns were the human's, against 29 collisions "no check landed". On `all_star` under drill 5 there was no knockdown in 70 s, against 55 such collisions. On `all_star` with the scripted human there was one in 70 s, bot on bot, against 47. The human was never checked.
5. A side knockdown 0.6 m from the side boards shows the victim's upper body inside the wall (`sk14-boards`). If A can pass the distance to the boards along the knock direction, B can pick a take that stays inside; otherwise B traces for it.
6. `KnockdownFrames - N` can stay: the figure now waits out the stop by itself, and the get-up ends on the release at 0, 3 and 6.

