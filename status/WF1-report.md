# Lane WF1: the cross-check into air reads as a miss; a new press cuts a running whiff (A-to-B-0041 points 1 and 2)

Tree `C:\Users\kazda\jam\wt\b-fig`, branch `lane/b-fig-h61` (base dev-b dc6661d0), commit `6de5c74f` (assets) plus the commit that carries this
report. Not pushed. The tree was rebuilt first (PASS; the first try failed on the compiler's heap while other lanes were building, the second passed in 20 s).

## 1. Done / not done
- **Done, point 1:** `CrossCheck_Whiff` re-keyed in the figure lab (export52), both styles, imported by name. No wiring change: same name, same 16 keys, `PlayWhiff(1)`.
- **Done, point 2:** `PlayCheck` during a running whiff now plays its wind-up at once, on both figures (`BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`).
  `PlayCrossCheck` already replaced a running whiff (measured before the change); it now also clears the whiff mark.
- **Not done:** the body check's whiff (`PlayWhiff(0)`) was not re-keyed (see section 3 for what I measured on it).
- **GPU strips done** (17:46 to 17:57; the token was held 12 minutes and given back): old against new, and the press at whiff+6.

## 2. Per item

### Point 1: the cross-check whiff (`PlayWhiff(1)`)
What a person sees from behind, old against new:
- OLD (export49): two frames after the miss the bar is already down at his hips and he stands there. It reads as "the stick comes down".
- NEW (export52): the bar stays level and keeps going forward for three frames (miss+1 to +3). Then he lurches: the helmet drops, the right
  skate swings out wide to his right off the ice, and the bar tips with one end down (miss+4 to +7). He lands on the wide skate at +8 to +11 with
  the blade on the ice in front, and is back in his stance at +15.

| Measure (engine, headless, test figure at 6 m/s) | Old | New |
|---|---|---|
| Right glove height on miss+1 / +2 / +3, clay (cm) | 91.0 / 71.7 / 57.5 | **93.5 / 83.8 / 76.9** (the bar stays up while it goes on) |
| Lowest helmet height in the take, clay (cm; contact pose 98.8) | 83.5 on miss+4 | **73.2 on miss+6** (10 cm lower) |
| The same, cel | 87.7 | **76.7** |
| Take frames played by `PlayCrossCheck(8)` then `PlayWhiff(1)` on the miss frame | 0 to 14 | 0 to 14 (unchanged; ends 15 frames after the miss) |

| Measure (lab solver, clay / cel) | Contact pose (frame 0) | New frame 3 | New frame 6 |
|---|---|---|---|
| Gloves ahead of the hips' start (cm) | 71 / 79 | **96 / 104** (25 cm further than a hit) | 87 to 99 / 92 to 107 |
| Hips forward (cm) | 0 | 17 | 24 |
| Right skate out from his centre line (cm) | 24 / 22 | 25 / 24 | **44 / 44**, 20 cm off the ice |

- Lab gate (`n2-HX2/gate.sh`, base export46): PASS, 2 rows, 0 failing. Worst one-frame turn 52.0 / 52.1 degrees (limit 60). The first version failed
  the gate (an 85-degree turn of the right upper arm on frame 7); the catch key moved from 10 to 11 and its grip came 17 cm forward.
- No root motion: the hips' shift is inside the take, as in export49.
- Lab strips at the game camera's geometry and side-on, old against new (CPU renders; I looked at both with the Read tool):
  `C:\Users\kazda\jam\skater-lab\skf\export52\sheets\wf1-whiff-p3_clay.png`, `C:\Users\kazda\jam\skater-lab\skf\export52\sheets\wf1-whiff-p2_cel.png`.
- Lab files: `C:\Users\kazda\jam\skater-lab\skf\export52\` (`READY.txt`, `GATE.md`, `<style>\takes\CrossCheck_Whiff.fbx`); the keys are in
  `C:\Users\kazda\jam\skater-lab\skf\d3-WF1\wf1_whiff.py`.

### Point 2: a new press cuts a running whiff
Cause: every whiff take sets the figure's action priority to 7, the body check's own priority. `PlayCheck` plays nothing while a priority-7 take
is running (it takes it for a check already under way), so a check pressed during a whiff was dropped until the whiff ended.

Change (both Blueprints, same edit):
- New real variable `WhiffT`: the time a running whiff ends. Set by `WhiffGo` (now + 0.45 s), `PlayCrossWhiff` (now + 0.3 s), `PlayCheckMiss` (now + 0.4 s).
- `PlayCheck`: while now < `WhiffT` and the figure is not down, it clears `WhiffT` and `WindT0` and sets `ActPri` 0. The check then plays through
  its usual path with its 0.03 s (2-frame) blend. It logs `WF1 check cuts a whiff`.
- `PlayCrossCheck`: clears `WhiffT`. `CrossWind` and `CrossRelease` play without a priority test, 0.05 s (3-frame) blend, as before.
- The AnimBPs (`ABP_SkaterTB1`, `ABP_SkaterTB1_P3`) are not changed: the whiff and the checks are slot takes started by the figure Blueprint.

Proof (headless, the take playing on each tick; calls by name as device A sends them; `SetWindupTakes(true, 8)`):

| Calls | Old | New |
|---|---|---|
| `PlayCrossCheck(8)` t10, `PlayWhiff(1)` t18, `PlayCheck` + `PlayCheckWindup(8)` t24 (whiff+6), clay | the whiff plays to t32; **no check take at all** | **t24: `Check_Deliver_Left_Light_Wind` at frame 2**, to its end at t41 |
| the same, cel | no check take | **t24: `Check_Deliver_Right_Wind` at frame 2**, to t47 |
| `PlayCheck` t10, `PlayWhiff(0)` t30, `PlayCheck` t36 (whiff+6), clay | the whiff plays to t42; no second check | **t36: the check's wind-up at frame 2** |
| `PlayCheck` t10, `PlayWhiff(0)` t30, `PlayCrossCheck(8)` t36, both | t36: `CrossCheck_Wind` at frame 2 | the same (already right) |
| `PlayCrossCheck(8)` t10, `PlayWhiff(1)` t18, `PlayCheck` t34 (whiff+16), both | t34: the check plays | the same |

### Seeded clay match (clayA, scripted human, hit-stop 3), before and after on this tree
**677 = 677 sim lines identical**, JAMSIM 7200 `4ac32edcf3678901`, 0 Blueprint errors, 0 `Error:` in both runs. All four Blueprints BS_UP_TO_DATE.
(The line count differs from E4's 738 because device A's source changed since; the baseline was run on this tree before my first edit.)

### GPU strips, game camera (all three looked at with the Read tool)
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-175227-B-WF1-crosscheck-whiff-old-new-p3_clay.png`: clay, a cross-check into air, the miss frame to +16.
  OLD: the bar is down at his hips by +2 and nothing else happens. NEW: the bar is still level and out on +2, +3 and +4; on +5 to +8 he is bent
  low with the right skate thrown out wide and the bar tipped; on +10 he is crouched on both skates; from +12 he is upright. It reads as a miss
  with no mark. (The old row is export49's file re-imported in memory for the film and not saved.)
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-175227-B-WF1-crosscheck-whiff-new-p2_cel.png`: the cel figure, new take: the same picture.
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-175227-B-WF1-press-at-whiff6-p3_clay.png`: a press 6 frames into the whiff.
  Row 1 (clay, `PlayCheck`): he comes out of the stumble into the check's coil on press+1 to +6 and drives on press+8.
  Row 2 (clay, `PlayCrossCheck(8)`): the bar is across his shoulders by press+2 and the contact pose is on press+8.
  Row 3 (cel, `PlayCheck`): as row 1. The engine's log of these films: the new take starts at its frame 2 on the press tick in all three.
- After the films the editor was closed without saving; `git status` shows no change against the committed assets.

## 3. Not verified
- A moving skater: the film rig skates in place, so the stumble is not seen against real travel over the ice.
- The body check's whiff. On the test rig `PlayWhiff(0)` sent 20 frames after `PlayCheck` on the cel figure starts `Check_Whiff_Right` at frame 12 and
  the take is gone one tick later; on the clay figure it plays 13 frames from 0. Whether device A's real miss frame lands the same way was not measured.
- A held cross-check released on nobody, then a press (A's `miss-03`): proven only through the same calls on the rig, not in A's game.
- `WF1 check cuts a whiff` did not occur in the seeded match (the scripted human pressed no check inside a whiff); the path is proven on the rig only.

## 4. For the engine lane / for device A
- No name or signature changed: `PlayWhiff(int32 Kind)`, `PlayCheck`, `PlayCrossCheck(Frames)`.
- `CrossCheck_Whiff` still ends 15 frames after the miss: A's 16-frame hold-off stands.
- A press inside a whiff now shows from its first frame, so A's 3-frame delay of the release's whiff is no longer needed to protect a body check
  (it does no harm).
- Merge: `BP_SkaterVisual_TB1.uasset`, `BP_SkaterVisual_TB1_P3.uasset`, `AS_SkaterTB1_P3_CrossCheck_Whiff.uasset`, `AS_SkaterTB1v2_CrossCheck_Whiff.uasset`.
  Cook check before the push (the lane touched `Skaters/`).

## 5. Decisions for the operator
- None needed. If the stumble should be bigger or smaller, it is four numbers in `wf1_whiff.py` (`d6` hips forward, `wide` the skate's reach, `roll`, `yaw`) and about six minutes per pass.

## 6. The next three steps
1. Lead: merge the four files, cook check, push for A's 18:30 merge.
2. Device A: film a tap into air and a hold released on nobody followed by a check, against `miss-01` and `miss-03`.
3. If A's film shows the body check's whiff cut short on the cel figure, look at the frame `PlayWhiff(0)` arrives on.

## 7. Ledger rows (they replace these four files' rows)
| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 13.02 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | as lane P2's row, and (lane E4): SetStickAim without the spring: the sent angle (clamped +-70) is shown with a one-frame lead (0.65 of the last step) and never more than 15 deg a frame, the blend space fed the target minus a learned map offset AimOffA/AimOffR (where puck_socket stood against the input that drew it, learned in SK2Tick at 0.3 a frame while the aim layer is the whole pose and the aim is steady), the rest past -63/+70 as a stick yaw AimOver (at most 12 deg) in SK3Fix, whose slide term is off; UpdateMotion sets the skating mesh yaw relative (the figure no longer turns one frame ahead of the pawn); new PlayCrossWhiff() (CrossCheck_Whiff from Wind frame F-10 / Release F-3, 3-frame blend, from 0 with 4, plays to its end) and PlayWhiff(int32 Kind) (0 PlayCheckWhiff, 1 PlayCrossWhiff); HoldTick stops CrossCheck_Wind at or past contact while the hold is on (skipping the hold-in) and stops a stop take under a hold, keeps the ABP bHoldOut false (no gloves over the helmet on release); StopTick starts no stop take while bHoldOn; new real variables AimSent, AimRSent, AimStep, AimRStep, AimLead, AimDes, AimOffA, AimOffR, AimOver; and (lane WF1): a new press cuts a running whiff: new real variable WhiffT (the time a running whiff take ends: WhiffGo sets now + 0.45 s, PlayCrossWhiff now + 0.3 s, PlayCheckMiss now + 0.4 s); PlayCheck, while now < WhiffT and the figure is not down, clears WhiffT and WindT0 and sets ActPri 0 so ChkPlay cuts the whiff with its 0.03 s blend (before: a PlayCheck met ActPri 7 and was dropped until the whiff ended); PlayCrossCheck clears WhiffT (CrossWind / CrossRelease already replace a whiff, 0.05 s blend) (13022358 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 13.23 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-15 | B | y | as lane P2's row, and (lane E4): SetStickAim without the spring: the sent angle (clamped +-70) is shown with a one-frame lead (0.65 of the last step) and never more than 15 deg a frame, the blend space fed the target minus a learned map offset AimOffA/AimOffR (where puck_socket stood against the input that drew it, learned in SK2Tick at 0.3 a frame while the aim layer is the whole pose and the aim is steady), the rest past -63/+70 as a stick yaw AimOver (at most 12 deg) in SK3Fix, whose slide term is off; UpdateMotion sets the skating mesh yaw relative (the figure no longer turns one frame ahead of the pawn); new PlayCrossWhiff() (CrossCheck_Whiff from Wind frame F-10 / Release F-3, 3-frame blend, from 0 with 4, plays to its end) and PlayWhiff(int32 Kind) (0 PlayCheckWhiff, 1 PlayCrossWhiff); HoldTick stops CrossCheck_Wind at or past contact while the hold is on (skipping the hold-in) and stops a stop take under a hold, keeps the ABP bHoldOut false (no gloves over the helmet on release); StopTick starts no stop take while bHoldOn; new real variables AimSent, AimRSent, AimStep, AimRStep, AimLead, AimDes, AimOffA, AimOffR, AimOver; and (lane WF1): a new press cuts a running whiff: new real variable WhiffT (the time a running whiff take ends: WhiffGo sets now + 0.45 s, PlayCrossWhiff now + 0.3 s, PlayCheckMiss now + 0.4 s); PlayCheck, while now < WhiffT and the figure is not down, clears WhiffT and WindT0 and sets ActPri 0 so ChkPlay cuts the whiff with its 0.03 s blend (before: a PlayCheck met ActPri 7 and was dropped until the whiff ended); PlayCrossCheck clears WhiffT (CrossWind / CrossRelease already replace a whiff, 0.05 s blend); on the AS_SkaterTB1_P3_ takes (13231679 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Whiff.uasset | anim | 0.14 | original | Blender 4.2 Python script (lanes HX2, WF1) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | skf/export52/p3_clay (takes/CrossCheck_Whiff.fbx; gate passed) take CrossCheck_Whiff re-keyed by lane WF1 (A-to-B-0041 point 1), 16 keys as before: from the miss frame (its frame 0 is CrossCheck_Wind frame 10) the shove goes on with the bar level to frame 3 (hands 25 cm further ahead, hips 17 cm on), 4 to 7 a stumble over the front skate (hips 8 to 9 cm down, the trunk rolled 9 deg, the back skate swung out 44 cm from his centre line and off the ice), caught on the wide skate at 11 with the blade on the ice in front, the stance at 15; no root motion; worst one-frame turn 52 deg; played by PlayCrossWhiff / PlayWhiff(1) by name on SKEL_SkaterTB1_P3, wiring unchanged (lane WF1 import) (141254 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Whiff.uasset | anim | 0.14 | original | Blender 4.2 Python script (lanes HX2, WF1) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | n | skf/export52/p2_cel (takes/CrossCheck_Whiff.fbx; gate passed) take CrossCheck_Whiff re-keyed by lane WF1 (A-to-B-0041 point 1), 16 keys as before: from the miss frame (its frame 0 is CrossCheck_Wind frame 10) the shove goes on with the bar level to frame 3 (hands 25 cm further ahead, hips 17 cm on), 4 to 7 a stumble over the front skate (hips 8 to 9 cm down, the trunk rolled 9 deg, the back skate swung out 44 cm from his centre line and off the ice), caught on the wide skate at 11 with the blade on the ice in front, the stance at 15; no root motion; worst one-frame turn 52 deg; played by PlayCrossWhiff / PlayWhiff(1) by name on SKEL_SkaterTB1v2, wiring unchanged (lane WF1 import) (141232 bytes) |
