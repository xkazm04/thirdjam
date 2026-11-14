# Lane E3, first hand-back (21:52): the new calls declared with stand-ins and proved headless; the puck moved onto the blade's face

Tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, dev-b 27a0ba99, rebuilt: PASS in 4.1 s, nothing to compile). Headless editor pid 29976, port 8004,
started 21:26. It is **still running** for the second hand-back, with `Slate.bAllowThrottling 0`. Game sound was already off before I started and is still off.
**Nothing was seen on a GPU.** `GPU-FREE.txt` has not existed since 21:49, and another UnrealEditor (pid 18808, not mine) holds the GPU. No git commands.
Assets were changed only under `Game/Plugins/JamAssets/Content/Skaters/`: 6 changed, 4 new.

## 1. Done / not done
| Item | Verdict | Deciding number |
|---|---|---|
| `PlayReceive()` | **Done**, stand-in take, both figures | `Receive` plays from the call tick for 6 frames. The blade comes back 7.5 cm and the hand rises 3.4 cm, then the figure returns to the carry pose |
| `SetPassCharge(float Charge01)` | **Done**, both figures | Charge 0 to 1 moves the blade 59 cm back, 132 cm to the side and 43 cm up, in step with the charge. `PlayPass` clears it on the same tick |
| `SetShotCharge(float)` | **Checked: it already grew with the charge.** Unchanged for the shot alone | Blade height at charge 0.2 / 0.47 / 0.6 / 1.0: 36 / 121 / 146 / 201 cm (`Shot_Slap` frames 3 to 16) |
| `PlayCallForPass()` | **Done**, stand-in take | Two blade taps, up to 47 cm, 0.22 s |
| Turning to the puck, no call from A | **Done**, both figures | Head yaw against the figure's own look target: within 1 degree on a steady bearing. Clamped at 60 degrees. 0 while carrying. Back to 0 within 8 frames of a knockdown |
| Ready glide (`Ready_Glide`) | **Not done**: the take does not exist yet (`export42` has no `READY.txt` at 21:49) | |
| Job 2: the puck on the blade | **Done (socket moved)**. Measured before and after | The drawn puck in the stick's frame: before (5.62, -146.75, 9.46), on the blade (the stick cuts the puck). After (11.78, -144.38, 7.38) = lane HP's face offset to 0.01 cm |
| Seeded clay match | **Identical** | 492 = 492 sim lines. JAMSIM hash at 7200 `cfc677349d89475b`, the same as XC12's run on the same code. 0 Blueprint errors, 0 `Error:` lines |
| Jobs 3 and 4 (imports, GPU film) | **Not started**: no `READY.txt` in export40 to 42, and the GPU is taken | |

## 2. Per item

### The new calls (both `BP_SkaterVisual_TB1` and `_TB1_P3`, Blueprint DSL; script `e3bp.mjs`)
- **`PlayReceive`** (custom event, no inputs):
  - Plays `AS_<style>_Receive` on DefaultSlot: priority 3, blend in 0.03 s, out 0.1 s.
  - Skipped while down, while getting up, or under a take of higher priority.
  - Stand-in: Poke frames 0-1-2-3-3-2-1-0 (8 keys, 0.117 s): the blade draws back toward the body and the knees give.
- **`PlayCallForPass`** (no inputs):
  - Plays `AS_<style>_CallForPass`: priority 2 (it never overrides a pass, shot or hit), blend 0.05 / 0.12 s.
  - Stand-in: `Shot_Slap` frames 0-3-0 twice (14 keys, 0.217 s).
- **`SetPassCharge(Charge01)`** stores `PassChg` (clamped 0..1) and calls the new `ChargeMix`.
- **`ChargeMix`:**
  - Sets `ChargeTime` = max(shot × 0.26667, pass × 0.05) and `ChargeAlpha` = max(clamp(shot × 5), pass), then `PushAnim`.
  - The pass charge therefore rides the ABP's existing charge evaluator (`Shot_Slap` at `ChargeTime`): frames 0 to 3, a short backswing to the side, growing with the charge.
- **`SetShotCharge`** now ends in `ChargeMix`. With no pass charge it is arithmetically the old code.
- **`PlayPass(Strength)`** sets `PassChg` = 0 and calls `ChargeMix` before the take (as before).
- **New variables:** `PassChg`, `LookYaw`, `LookDbg`, `PuckSeekT` (double), `bNoLook` (bool, default false, Instance Editable: a per-figure off switch), `PuckRef` (Actor).
- **`LookTick`**, run from `ReceiveTick` on every tick:
  - It finds the puck by class `/Script/JamCore.JamPuck` (`GetActorOfClass`, retried at most once a second while none is found).
  - Target = the puck's bearing from `spine_03` against the mesh's forward axis, clamped to ±60. Past ±150 (puck behind) it keeps the side it is already turned to, so it does not flap.
  - Target 0 while carrying (`bPuck`), down, getting up, a goalie, at a faceoff, under a take of priority 5 or more (hits, falls, celebrations), or within 60 cm of the puck.
  - Smoothed at 8/s (25/s while down or getting up).
  - The ABP gets `LookRot1` = 0.4 × yaw on `spine_03` and `LookRot2` = 0.6 × yaw on `head`, so the head ends on the full yaw.
- **The ABPs** (`ABP_SkaterTB1`, `ABP_SkaterTB1_P3`, script `abplook.mjs` + `mbcfg.py`):
  - Two new Transform (Modify) Bone nodes, `AnimGraphNode_ModifyBone_7` (`spine_03`) and `_8` (`head`), at the end of the chain after `ModifyBone_6`, before ComponentToLocal.
  - Additive rotation in component space, alpha 1. New rotator variables `LookRot1` and `LookRot2`.
- **Proving numbers** (headless test figures, 19 in the game's live match. Log `logs\t_T2.json`):
  - **Head yaw** (quaternion difference from a twin figure with `bNoLook`, pure Z rotation, tilt 0.0):
    - steady bearing +18.2: head turn 17.9
    - bearing 158 (behind): turn 60.0 (clamp)
    - spinning figure (1.5 deg/tick): follows to ±60, keeps -60 while the puck passes behind (raw 173.6), then swings to +60 over about 20 frames
  - **The carrier** (`SetHasPuck(true)`): head turn 0.0 on every tick.
  - **Knockdown at tick 100:** look yaw -16.2 → -9.4 → -5.5 → -1.1 (t105) → 0.0 (t112).
  - **The sign:** positive yaw turns the head toward a positive bearing (checked on L1: bearing +18, head +18).
  - `PlayReceive` / `PlayCallForPass` / `SetPassCharge` / `SetShotCharge`: same results on the night figure (rows `Rn`, `CPn`, `PCn`, `SCn`).
- **Still:** none (headless). I looked at the per-tick bone recordings through `anaT.py` and at the log, not at pictures.

### Job 2: the puck on the blade (`SK_SkaterTB1_P3`, `SK_SkaterTB1v2`, socket `puck_socket`, bone `stick_socket` unchanged)
- **What I found today:** the socket is already on the stick bone, so it rides the blade, not the mesh root.
  - In a play session, the drawn puck sat exactly on it: 496 of 496 carrying frames within 0.0 cm, straight and turning, run `pk_before`.
  - In the stick's frame it sat at the old offset, 6.0 cm from the blade centre along the blade. That is inside the blade: lane HP's "the stick cuts the puck".
- **The change:** lane HP's offsets.
  - clay (11.78, -144.38, 7.38), was (5.62, -146.74, 9.46)
  - cel (13.55, -153.63, 12.04), was (6.20, -155.81, 10.39)
  - The height does not matter: `ApplyRenderAt` uses x and y only.
- **After** (seeded clay match `clay1`, 4,255 frames with a figure's `bPuck` on):
  - When drawn at the socket, the puck sits at (11.78, -144.38, 7.38) in the stick frame, median of every carrying frame: the face offset to 0.01 cm.
  - The puck centre is now 7.8 cm from the blade centre (was 6.0, along the blade), off its face.
- **A finding for A, not a socket matter.** In the clay match only 2,059 of 4,120 tracked frames have the puck within 1 cm of the carrier's socket.
  - Of the other 2,061, only 8 match the previous frame's socket (so it is not a lag) and 9 a held puck.
  - The rest are frames where the figure has `bPuck` on but the game draws the sim's puck: `Sim->CarrierIndex()` is not that skater.
  - The human (JamSkater_0) is within 1 cm on 1,059 of 1,224 mid-carry frames. Bots range from 18 to 46 %.
  - So "the puck does not stick" is partly a carrier that the sim does not count as one while the figure is told it has the puck (A-to-B-0032 point 2, possession lane).
- **Not verified:** the 1 to 1.8 cm clearance from the blade face (HP's Blender number). The engine has no blade mesh geometry headless, so I proved only that the engine places the puck exactly where HP put the socket.

## 3. Not verified
- Nothing seen on a GPU: the look turn at the game camera, the stand-in receive (7.5 cm: possibly unreadable at 20 m), the call-for-pass taps, the pass charge pose, and the puck against the blade.
- HP's note that a puck carried straight at 1.25 m hides behind the hips at the human's camera (A's): to film.
- `bNoLook` on the human figure: the human also turns to the puck when not carrying (I left it on).
- A's own calls in a match: none happen yet (0 `E3` log lines in the clay match).

## 4. For device A (exact names; custom events on both `BP_SkaterVisual_TB1` and `BP_SkaterVisual_TB1_P3`, by name behind your exists-checks)
| Call | Inputs | When to call | What the figure does |
|---|---|---|---|
| `PlayReceive()` | none | once, on the frame a pass reaches the receiver's stick (the possession change of a pass) | a 6 to 8 frame cushion take, priority 3 (stand-in now; the real `Receive` re-keys the same asset) |
| `SetPassCharge(float Charge01)` | 0..1, a double is fine | **every frame** while the pass button is held (power / max), and **0 once** on release or cancel | a pass backswing that grows with the charge. `PlayPass(Strength)` stays the release and clears the charge itself |
| `SetShotCharge(float Charge01)` | 0..1 | already on the interface (v3): every frame while charging, 0 after | **already grows with the charge** (blade 36 / 121 / 146 / 201 cm high at 0.2 / 0.47 / 0.6 / 1.0). Unchanged for the shot alone. `PlayShoot(Charge)` releases from the matching backswing |
| `PlayCallForPass()` | none | once, when a skater without the puck asks for it (bot or human button), at most every 0.5 s | two stick taps, priority 2 (never over a pass, a shot or a hit) |
| nothing (looking at the puck) | none | | each figure finds the puck itself (class `JamPuck`) and turns chest and head toward it, clamped to 60 degrees, smoothed. Off while it has the puck (`SetHasPuck(true)`), down, in a hit take, a goalie or a faceoff. Per-figure off switch: the bool `bNoLook` |
| socket | | | `puck_socket` moved onto the blade's forehand face on both meshes (same bone `stick_socket`). Nothing to change in `GetPuckSocketWorld` |

- **For the possession lane:** in the seeded clay match, half of the frames where a figure is told `SetHasPuck(true)` draw the sim's puck, not the socket (`CarrierIndex` disagrees). The human is within 1 cm 86 % of the time, bots 18 to 46 %. If the puck should look stuck, draw it at the socket for every skater the figure calls carrying, or send `SetHasPuck` from `CarrierIndex`.
- Log lines when the figure's log is on: `E3 receive` and `E3 call for pass`.

## 5. Decisions for the operator
1. **The look turn is on for every skater, the human's figure included** (chest 40 % and head 60 % of the bearing, up to 60 degrees). Keep it on the human, or set `bNoLook` on his figure?
2. **The stand-ins are subtle:** the receive moves the blade 7.5 cm. Ship them until export42's `Receive` / `CallForPass` / `Pass_Charge` arrive, or hold the calls back?

## 6. The next three steps
1. Import export40 / 41 / 42 by name as each `READY.txt` appears. Wire `Ready_Glide` (in place of `Glide` without the puck) and `Pass_Charge` (a second charge evaluator), and prove the contact frames still land on call + Frames (4 / 8 / 12).
2. Take the GPU when `GPU-FREE.txt` appears. Film from A's game camera and at 20 m side-on: the hits beside the old ones, a receive, a carry with the puck on the blade (and HP's hip question), a skater turning to the puck.
3. Repeat the seeded match after the imports (sim lines identical, 0 errors).

## 7. Ledger rows (10: 6 changed, 4 new; the changed rows replace these files' rows)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 11.64 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | as the XC12 row, and: PlayReceive() (AS_SkaterTB1v2_Receive, priority 3), PlayCallForPass() (AS_SkaterTB1v2_CallForPass, priority 2), SetPassCharge(float) and ChargeMix (the pass charge on the shot's charge evaluator: ChargeTime max(shot*0.26667, pass*0.05), ChargeAlpha max(clamp(shot*5), pass)); SetShotCharge and PlayPass through ChargeMix; LookTick from ReceiveTick: chest and head turned to the puck (GetActorOfClass JamPuck), clamped 60 deg, smoothed 8/s, off while carrying/down/getting up/goalie/faceoff/take priority >= 5, ABP LookRot1/LookRot2; variables PassChg, LookYaw, LookDbg, PuckSeekT, bNoLook (Instance Editable), PuckRef (lane E3) (12200175 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 11.83 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | as the XC12 row, and: PlayReceive, PlayCallForPass, SetPassCharge/ChargeMix, SetShotCharge and PlayPass through ChargeMix, LookTick and its variables as BP_SkaterVisual_TB1, on the P3 takes and ABP_SkaterTB1_P3 (lane E3) (12409314 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 1.02 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | as the E2 row, and: two Transform (Modify) Bone nodes after ModifyBone_6 before ComponentToLocal: ModifyBone_7 spine_03 (LookRot1) and ModifyBone_8 head (LookRot2), additive rotation in component space, alpha 1; rotator variables LookRot1, LookRot2 set by the figure's LookTick (lane E3) (1071029 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 1.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | as the E2 row, and: the look nodes ModifyBone_7 (spine_03, LookRot1) and ModifyBone_8 (head, LookRot2) as ABP_SkaterTB1 (lane E3) (1066901 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1_P3.uasset | mesh | 1.80 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export7c/p3_clay SK_Skater_p3_clay_loopcut.fbx, mesh only (12.0k tris, 8254 vertices): bands, running stitches 8.0 x 2.8 cm (38), vertex colours Col; own skeleton, sockets kept; Pads slot = MI_SkaterTB1_P3_PadsFelt (lanes SKI3, SK6, SK7); puck_socket moved onto the blade's forehand face, bone stick_socket, (11.78, -144.38, 7.38) was (5.62, -146.74, 9.46) (lane HP's offset, applied by lane E3) (1891232 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1v2.uasset | mesh | 1.02 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export/p2_cel SK_Skater_p2_cel_loopcut.fbx (T-B1 v2); sockets stick/head on their bones, puck_socket on bone stick_socket (lane SK2), moved onto the blade's forehand face (13.55, -153.63, 12.04) was (6.20, -155.81, 10.39) (lane HP's offset, applied by lane E3) (1067537 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Receive.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's Receive: AS_SkaterTB1v2_Poke frames 0,1,2,3,3,2,1,0 (8 keys, 0.117 s: the blade draws back, the knees give), on SKEL_SkaterTB1v2 (re-import by name re-keys it); PlayReceive (lane E3) (128909 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CallForPass.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's CallForPass: AS_SkaterTB1v2_Shot_Slap frames 0-3-0 twice (14 keys, 0.217 s: two stick taps), on SKEL_SkaterTB1v2 (re-import by name re-keys it); PlayCallForPass (lane E3) (136638 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Receive.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's Receive: AS_SkaterTB1_P3_Poke frames 0,1,2,3,3,2,1,0 (8 keys, 0.117 s), on SKEL_SkaterTB1_P3 (re-import by name re-keys it); PlayReceive (lane E3) (138116 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CallForPass.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's CallForPass: AS_SkaterTB1_P3_Shot_Slap frames 0-3-0 twice (14 keys, 0.217 s), on SKEL_SkaterTB1_P3 (re-import by name re-keys it); PlayCallForPass (lane E3) (145886 bytes) |

## Lab
`C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\e3\`
- Scripts: `standins.py`, `abplook.mjs`, `mbcfg.py`, `vars.py`, `vt2.py`, `ie.py`, `e3bp.mjs`, `sockset.py`, `e3t.py`, `pkrec.py`, `e3m.sh`, `anaT.py`, `anaP.py`, `anaP2.py`, `anaP3.py`, `simcmp.py`
- Logs: `logs\t_T2.json`, `pk_before.json`, `pk_clay1.json`, `full_clay1.txt`
- Backups of the six changed `.uasset` files and the DSL before and after: `bk\uasset\`, `bk\*.pre.dsl`, `bk\*.post-e3.dsl`

---

# Second hand-back (GPU, 21:53 to 22:22): filmed; the socket re-sized for the puck A actually draws

GPU editor pid 15212, port 8004, 21:53 to 22:22. Left through `ExitEditor`; the process is confirmed gone. `GPU-FREE.txt` = "free from E3 at 22:22". No editor of mine is running.
Stills: `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-222200-B-e3-*.png`, 13 stills, each with a `.txt` caption. I looked at every strip with the Read tool.

## Verdict per item for the 23:30 merge
| Item | Verdict | What I saw |
|---|---|---|
| (a) The puck on the blade | **Merge, with the re-sized offsets** | **A draws the puck at radius 12 cm** (`InitPuck`: `Mesh` scale 0.24 × 0.24 × 0.08), not 3.81. With HP's offset the blade still cuts through the drawn puck (`-socket-clay-top.png`, middle). I re-sized HP's direction for r 12: clay (19.42, -141.45, 4.80) and night (25.31, -150.14, 14.68). The puck now rests against the blade face from above (`-socket-clay-top.png` right, `-socket-night-shipped.png`) |
| (a) HP's question: does a straight carry hide behind the hips? | **Yes** (A's) | `-carry-gamecam-hidden.png`: at A's game camera a figure carrying straight ahead at 1.25 m shows **neither stick nor puck**. A carry at 35 degrees / 1.45 m shows the stick to the left. For A: carry the human's puck off-centre (about 30 degrees to the forehand side) or further out, so it is seen past the hips |
| (b) The turn to the puck | **Merge for bots; ship it OFF on the human's figure** | At the game camera the human's own helmet visibly turns over his shoulder toward the puck (`-look-gamecam-onoff.png`). On the player's own figure that reads as looking away from where he skates. At 20 m (about 55 px) a head turn is too small to read (`-look-20m-live.png`). It adds a little life up close and costs nothing. I saw no snap, but 1 Hz stills cannot exclude one. Headless, the smoothing is 8/s and holds its side behind the skater (first hand-back) |
| (c) `PlayReceive` stand-in | **Does not read** (harmless; merge the call so A can wire it, the look comes with export42's `Receive`) | Game camera: no visible change (blade behind the body), `-receive-gamecam.png`. 20 m: a barely visible stick draw-back |
| (c) `PlayCallForPass` stand-in | **Reads weakly at the game camera, not at 20 m** | The stick swings out and up to the left (frame 0.05) and reads. The taps themselves are small, and at 20 m the stick goes behind the body (`-callforpass-*.png`) |

## New: `SetLookAtPuck(bool bOn)` on both figures
- A custom event; it sets `bNoLook = !bOn`.
- **For A:** call `SetLookAtPuck(false)` once on the human's figure at spawn, by name behind the exists-check. The bots keep the default (on).
- Not called in the seeded match (it was added after it). It touches nothing else.

## Not verified
- Snaps in the turn at full frame rate: only 1 Hz stills and the headless per-frame numbers.
- The puck against the blade in a turn at the shipped offsets on the GPU. The rig froze a striding figure; in the match the human's figure is often told it has the puck while down or being hit (`-carry-match.png`, frames 2 and 4: the stale `SetHasPuck` already reported).
- The night figure in a match.

## Ledger rows (they replace the first hand-back's rows for these six files)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 11.65 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | as the XC12 row, and: PlayReceive() (AS_SkaterTB1v2_Receive, priority 3), PlayCallForPass() (AS_SkaterTB1v2_CallForPass, priority 2), SetPassCharge(float) and ChargeMix (the pass charge on the shot's charge evaluator: ChargeTime max(shot*0.26667, pass*0.05), ChargeAlpha max(clamp(shot*5), pass)); SetShotCharge and PlayPass through ChargeMix; LookTick from ReceiveTick: chest and head turned to the puck (GetActorOfClass JamPuck), clamped 60 deg, keeps its side past 150 deg, smoothed 8/s, off while carrying/down/getting up/goalie/faceoff/take priority >= 5, ABP LookRot1/LookRot2; SetLookAtPuck(bool) sets bNoLook; variables PassChg, LookYaw, LookDbg, PuckSeekT, bNoLook (Instance Editable), PuckRef (lane E3) (12213504 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 11.85 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | as the XC12 row, and: PlayReceive, PlayCallForPass, SetPassCharge/ChargeMix, SetShotCharge and PlayPass through ChargeMix, LookTick, SetLookAtPuck and their variables as BP_SkaterVisual_TB1, on the P3 takes and ABP_SkaterTB1_P3 (lane E3) (12422525 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 1.03 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | as the E2 row, and: two Transform (Modify) Bone nodes after ModifyBone_6 before ComponentToLocal: ModifyBone_7 spine_03 (LookRot1) and ModifyBone_8 head (LookRot2), additive rotation in component space, alpha 1; rotator variables LookRot1, LookRot2 set by the figure's LookTick (lane E3) (1077081 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 1.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | as the E2 row, and: the look nodes ModifyBone_7 (spine_03, LookRot1) and ModifyBone_8 (head, LookRot2) as ABP_SkaterTB1 (lane E3) (1072962 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1_P3.uasset | mesh | 1.80 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export7c/p3_clay SK_Skater_p3_clay_loopcut.fbx, mesh only (12.0k tris, 8254 vertices): bands, running stitches 8.0 x 2.8 cm (38), vertex colours Col; own skeleton, sockets kept; Pads slot = MI_SkaterTB1_P3_PadsFelt (lanes SKI3, SK6, SK7); puck_socket on bone stick_socket moved off the blade to rest the drawn puck (radius 12 cm) against the forehand face: (19.42, -141.45, 4.80), was (5.62, -146.74, 9.46) (lane HP's direction, sized for the drawn puck by lane E3, seen on the GPU from above) (1891232 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1v2.uasset | mesh | 1.02 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export/p2_cel SK_Skater_p2_cel_loopcut.fbx (T-B1 v2); sockets stick/head on their bones, puck_socket on bone stick_socket (lane SK2), moved off the blade to rest the drawn puck (radius 12 cm) against the forehand face: (25.31, -150.14, 14.68), was (6.20, -155.81, 10.39) (lane HP's direction, sized for the drawn puck by lane E3, seen on the GPU from above) (1067537 bytes) |
