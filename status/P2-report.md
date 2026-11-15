# Lane P2: the pass release on the frame the puck leaves (hand-back 11:45)

Tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, dev-b 10d2e337), rebuilt (PASS, 94.5 s).
- Headless editor pid 35108, port 8004, 10:58 to 11:23.
- GPU editor pid 38660, port 8004, 11:34 to 11:39. `GPU-FREE.txt` taken at 11:33 ("free from S4 at 11:33") and written back at 11:39 ("free from P2 at 11:39").
- Both editors left through ExitEditor; both processes confirmed gone.
- No git commands. Game sound was off before I started and stayed off.

Changed only `Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset` and `BP_SkaterVisual_TB1_P3.uasset`. Both were saved at 11:16, and that saved version is the one proven below. The "old" take that I filmed was put into memory only and thrown away on exit, so the files on disk are unchanged since 11:16.

## 1. Done / not done
| Item | Verdict | Deciding number |
|---|---|---|
| `PlayPass` starts `Pass_Snap` at its frame 5, both figures | **Done** | Montage position on the call frame **5.00 frames** (start 0.0834 s; the log rounds it to 0.083), then 6.00 / 7.02 / 7.98. Seeded match: 45 of 46 passes read 5.00 / 6.00 / 7.02 / 7.98 (before: 0 / 1.02 / 1.98 / 3.0). The 46th is cut by a `Receive` on the next frame, the same in the old run |
| High charge blends into the release with no pop | **Done**: start at frame 5, **2-frame blend (0.0333 s)**, the charge pose held under the blend | Full charge: blade forward speed **+45.0 cm on call+1, +30.8 on call+2, +1.4 after**. It only moves forward. Old: +45 (a pop toward the carry pose) then **-30.8** (back) then forward again until call+5 |
| Tap: frame 5, short blend from the carry, no wind-up | **Done** (2-frame blend) | The blade never goes behind the carry: forward position 112.7 cm or more (carry 117.5). Old tap: back to 54.6 cm on call+2 (the wind-up), after the puck had gone |
| Nothing else changes | **Done** | No other call, take or ABP was touched. 0 notifies, so no contact frame moved. Sim unchanged (next row) |
| Seeded clay match: sim identical, no Blueprint errors | **Done** | **738 = 738 sim lines, identical**. JAMSIM at 7200 `1d1fb181b85f1d4d` in both runs. 0 Blueprint runtime errors, 0 `Error:` lines. All four Blueprints BS_UP_TO_DATE |
| GPU film, tap and full pass, A's game camera, old beside new | **Done**, looked at | Below |
| Anim notifies question (A-to-B-0035 item 7) | **No**: nothing of B's hangs on an anim notify | Below |

## 2. Per item

### What changed (both `BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3`, Blueprint DSL, script `p2bp.mjs`; every block read back equal)
- **`PlayPass(Strength)`**:
  - `PassHoldW` = `PassLast` if a charge above 0 came within 0.1 s, else 0. This catches A's `SetPassCharge(0)` whether it comes before or after `PlayPass` on that frame. Then it clears `PassLast` and `PassChg`.
  - Under the same gate as before (not down, not getting up, priority 3) it plays `Pass_Snap` on DefaultSlot with start time **0.0834 s** (frame 5) and blend in `SelectFloat(0.0333, 0.0333, PassHoldW >= 0.3)`. Both values are 2 frames today; the select is a tuning hook.
  - Blend out 0.05, BlendOutTriggerTime **0.0**, which was -1. Without this change, a take started at frame 5 begins blending out on its first frame and the follow-through never shows at full weight. Now frames 5 to 9 play in full and then fade over 0.05 s.
  - The montage goes into `PassMon`. If the take is refused, `PassHoldW` = 0. Then `ChargeMix`.
- **`SetPassCharge(c)`**: as before, and for c > 0 it also stores `PassLast` = c and `PassLastT` = the time.
- **`ChargeMix`**: the ABP's `PassW` = **max(`PassChg`, `PassHoldW`)** × (not down). The pass-charge layer sits before the slot, so holding it keeps the charge pose under the montage's blend-in instead of dropping it to the carry pose on the call frame. That drop was the old pop.
- **`PassHoldTick`** (new; called from `ReceiveTick` only while `PassHoldW` > 0): it lets go of the hold once `PassMon` is invalid, not playing, or past 0.1217 s, so the hold never shows under the blend-out.
- **New variables**: `PassLast`, `PassLastT`, `PassHoldW` (double) and `PassMon` (AnimMontage reference).

### Headless proof (test figures, both figures, both call orders; logs `logs\t_old.json`, `t_new3.json`, `t_new4.json`)
- Clay, full charge, `PlayPass` then `SetPassCharge(0)` (FP). The order the other way round (FC) gives the same numbers.
  - Montage position (frames): call 5.00, then 6.00 / 7.02 / 7.98.
  - Blade, forward / up in cm: call 40.4 / 11.7 (the charge pose), +1 85.4 / 13.1, +2 116.2 / 36.0, +3 117.6 / 43.5, +4 118.0 / 46.3. Then back to the carry by +7.
  - Forward speed **peaks on call+1 (45.0 cm per frame)**.
- **Why +1 and not the call frame itself.** In the match A's calls arrive after that frame's animation update; the old take read position 0.0 on its call frame in all 47 passes. So the call frame always shows the previous pose, and call+1 is the first frame the figure can move. The test rig calls at the same point of the frame.
- Night figure: +46.9 then +30.9. Middle charge 0.4: +11.1, +12.5.
- With the human's carry aim (45 degrees, 1.6 m, 4 m/s):
  - full charge: forward +44.3, +30.5;
  - tap: forward +16.1. The blade also sweeps 50 to 70 cm sideways per frame from the carry side to the front. That is the take's centred pose, the same with the old take.
- Seeded clay match (clayA, scripted human, hit-stop 3, to frame 7200+): the numbers in the table above (`logs\full_before.txt`, `full_after.txt`, `rp_before.json`, `rp_after.json`).
  - No skater in this match ever charged a pass (bots pass with 0 charge frames), so in a match it is the tap path that runs.

### GPU film. I looked at all three strips with the Read tool.
Clay figure, carrying at 4 m/s with the human's carry aim. The world ran in slomo and was paused exactly on each game frame. The puck flies on from the call at 14 m/s (tap) or 23.3 m/s (full). Stills:
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-113900-B-P2-pass-full-old-new.png` (full pass; OLD top, NEW bottom; call-1 to call+6)
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-113900-B-P2-pass-tap-old-new.png` (tap)
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261115-113900-B-P2-pass-full-zoom.png` (full pass, closer crop: call, +1, +2, +3, +6)

**What a person sees at the game camera:**
- **OLD, full pass:** the puck is already a metre gone while the stick is still winding. On call+1 it jumps back to his right side, on +2 it is out on his left again, and on +3 it crosses. The stick goes through only around +4 to +5.
- **OLD, tap:** a backswing to his right on +1 to +3, after the pass.
- **NEW, both:** the stick goes through on call+1, in one move from his left (charge or carry) to in front of him, with no backswing and no jump. Then it stays in front of him for +2 to +4 and is back at the carry on his left by +6.
- **The caveat (E3 said it of `Pass_Snap` too): from behind, the follow-through itself is hidden by his back** on +2 to +4. The timing now matches the puck, but the follow-through stick does not show from A's camera. At 20 m side-on it would (E3's strip). I did not film the night figure.

### Anim notifies (A-to-B-0035 item 7): **No**
- **No take has a notify.** I checked all 1,046 animation sequences under `/JamAssets` and `/Game` with `AnimationLibrary.get_animation_notify_events`: 0 notifies. There are no montage assets either; every take is a dynamic montage.
- **The ABPs' event graphs** (`ABP_SkaterTB1`, `_P3`) hold only `BlueprintUpdateAnimation` and `TryGetPawnOwner`.
- **The figure Blueprints** have no notify or montage-notify events.
- **Where B's sounds come from:** events or timers, never notifies.
  - Direct calls: `Snd` (`PlaySound2D`), `DashSndOn`, `SndSkid`, `MS_brake` in `StopTick`.
  - Timers (`SetTimer`; world time, not the figure's `CustomTimeDilation`): `DiveSnd`, `WeaponSnd`, `SndSwing`, `KickDrop`, `PlayWeapon`, `DashBegin`, `DashLoopGo`, `EndDash`, `PlayBoostPickup`, `PlayCheck`.
- **The only things that read a take's clock** are threshold tests with `>=` or `<`, so a 7-frame jump steps past them rather than missing them:
  - `ReceiveTick` (the down hold, `HoldAt` / `DownHold`, and the get-up end estimate);
  - `OnBusEvent` (wind check under 0.0667 s);
  - `WindHoldTick`, `StopTick`, `PlayCheckWhiff`.

## 3. Not verified
- The night figure (`_TB1`) on the GPU: headless numbers only.
- The game camera at full frame rate. The strips are stills of exact game frames.
- A charged pass inside a real match: none happened in the seeded match. The charged path is proven on test figures only.
- The pass during a hit-stop. The hold is released by the montage's own position, not by wall time, so it should hold, but this is not tested.

## 4. For device A
- Nothing to change: `PlayPass(Strength)` and `SetPassCharge(0)` on the release frame, in either order, as now.
- The figure now shows the release pose (`Pass_Snap` frame 5) on the first frame it can, call+1. It cannot show it on the call frame itself, because your calls come after that frame's animation update; the whole figure lags the sim by that one frame anyway.
- A charge above 0 sent within 0.1 s before `PlayPass` is taken as that pass's wind-up and blended out of over 2 frames.

## 5. Decisions for the operator
1. From behind, the follow-through is hidden by the skater's back: the timing is right, but the stick does not show. Ship the timing as it is, or ask the figure lab for a `Pass_Snap` whose follow-through ends out to the side, as a re-key by name (no wiring change)?
2. The take is short after frame 5 (4 frames and a 3-frame fade), so the stick is back at the carry by call+6. Keep it short so the carry returns at once, or hold the follow-through longer? A longer hold costs one number in `PlayPass` (BlendOutTriggerTime or play rate).

## 6. The next three steps
1. The lead merges the two figure Blueprints (cook-only check first, per the cook rule).
2. If the operator wants the follow-through seen from behind: the figure lab re-keys `Pass_Snap` frames 5 to 9 out to the forehand side. Then re-film this strip.
3. Film the night figure and a charged pass in a live match once A's human charges one.

## 7. Ledger rows (they replace these files' earlier rows)
| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 11.99 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-15 | B | y | as the fourth-hand-back E3 row, and: PlayPass starts Pass_Snap at its frame 5 (start 0.0834 s, blend in 0.0333 s, blend out 0.05, BlendOutTriggerTime 0.0) so the release pose shows on the frame after A's call; the last pass charge sent within 0.1 s (PassLast/PassLastT, kept by SetPassCharge) is held under the blend (PassHoldW; ChargeMix sets the ABP's PassW = max(PassChg, PassHoldW)) and let go by the new PassHoldTick (from ReceiveTick) once the take PassMon is past 0.1217 s or not playing; new variables PassLast, PassLastT, PassHoldW, PassMon (lane P2) (12573981 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 12.19 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-15 | B | y | as the fourth-hand-back E3 row, and: PlayPass from Pass_Snap frame 5 with the charge pose held under a 2-frame blend, SetPassCharge/ChargeMix/PassHoldTick and the variables PassLast, PassLastT, PassHoldW, PassMon as BP_SkaterVisual_TB1, on AS_SkaterTB1_P3_Pass_Snap (lane P2) (12783224 bytes) |

## Lab
`C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\d3\p2\`
- Scripts: `p2bp.mjs` (the edit), `oldpp.mjs` (old/new toggle used for the film, never saved), `vars.py`, `vars3.py`, `e3t.py` + `mkP.py` + `t.sh` + `anaT.py` (test figures), `recP.py` + `m.sh` + `anaR.py` + `simcmp.py` (seeded match), `fp.py` + `film.sh` + `grid.py` (film), `notif.py` (notify scan).
- The DSL before and after: `bk\*.pre.dsl`, `bk\*.post.dsl`.
