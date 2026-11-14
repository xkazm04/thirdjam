# Lane FILM3 report: the figure branch filmed on the GPU, the missed-check rule off by default, the merge verdict

## Verdict: merge `lane/b-fig` into dev-b, together with this lane's two Blueprints (named change: `bAutoCheckMiss`)

Commit the two Blueprints this lane changed (`BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`, uncommitted in the tree), then merge. None of the four reasons for "not" holds:

- **Blueprint errors in a normal match: 0.** The whole editor log has 0, across 3 matches and 6 film sessions.
- **Stuck figures: none seen.**
- **Get-ups: all on the release.** End minus release was 0.0 for 4 of 4 knockdowns.
- **The miss rule fires 0 times by default.** In the same seed it fired 3 times when switched on.

Two things to know (neither blocks the merge):

1. **The match's sim lines no longer equal SK29's, and the figures are not the cause.** On this tree (dev-b merged in) seed 1 differs from SK29's run. The sim hash is equal at frame 600 and different at frame 1200. The first changed knockdown/stumble line is SK29's stumble at sim 1873, which does not happen here. Three runs of the same seed on this tree gave the same 19 lines and the same hashes: rule off with stills, rule off without stills, and rule on in memory. So the figure Blueprints do not touch the sim, and the change came with the dev-b merge. I did not run dev-b alone to prove that.
2. **`Dash_Wipeout_Light` snaps on its frames 2 to 3.** The arms fly out in one frame: `hand_l` moves 68 cm (clay) / 50 cm (night). It still reads as a catch. It cannot happen in today's game, because nothing calls `EndDash(4)`.

Tree `C:\Users\kazda\jam\wt\b-fig`:
- **Merge:** `git merge --no-edit dev-b` gave merge commit 7fd2e542, no conflict. That was the only git operation.
- **Build:** passed in 128.7 s.
- **Editor:** GPU, port 8004, pid 38320, 10:55:55 to 11:26. 58 `LogD3D12RHI` lines. Left through `ExitEditor` (`LogExit: Exiting.`); process confirmed gone. No dirty package at exit.
- **Settings put back:** `r.Editor.Viewport.OverridePIEScreenPercentage` was 0 for the stills and is back to 1. `Slate.bAllowThrottling` was 0 during the runs and is set to 1 at the end. Game sound was **already off when I arrived** (`enable_game_sound` false before my first call); I left it **on**, as the brief asks.
- **Not done:** SK30-film shot 13 (quick get-ups); no stills from the game camera for job 3.

## Job 1. The missed-check rule off by default

**What changed (both figure Blueprints):**
- A new Boolean `bAutoCheckMiss`, default **false**.
- One line in `PlayCheck` changed:
  - before: `(if (> (- now ChkBusT) 0.1)`
  - after: `(if (and bAutoCheckMiss (> (- now ChkBusT) 0.1))`
- Nothing else in the Blueprint DSL differs; the DSL before and after is in my lab folder (`film3\bk\`).
- `PlayCheckMiss()` is untouched.
- The variable is Instance Editable, so a test or the game can switch the rule on for one figure.
- Compiled and saved: `BP_SkaterVisual_TB1` 10 599 364 bytes, `_P3` 10 808 409 bytes.

**Proof.** Seed 1 clay cartridge (`clayA`, regular cut), scripted human:

| Run | `PlayCheck` calls | check bus events | misses by the rule | knockdown/stumble/check sim lines | BP errors |
|---|---|---|---|---|---|
| m1: rule off (default), GPU, with stills, to sim 7300 | 9 | 6 | **0** | 19 | 0 |
| m2: rule off, no stills, to 7156 | 8 | 5 | **0** | 19, identical to m1 | 0 |
| m3: rule switched on in memory for all 10 figures | 8 | 5 | **3** (skaters 5, 6, 7) | 19, identical to m2 | 0 |
| SK29 (`full_gpu29d`, before the dev-b merge) | 7 | 5 | 0 | 21 (differs from frame 1873) | 0 |

- **The miss stumble never plays by default.** It is 0 in m1 and m2; "SK30 check miss" appears 0 times.
- **Not equal to SK29's lines**, for the reason given in the verdict. With the rule on or off, the lines are identical on this tree.
- **On test figures (seen)**, PlayCheck with no bus event:
  - With the rule off: the same clean return as a landed check. Clay: `Check_Deliver_Right_Light` for 8 frames, then skating, for both. Night: `Check_Deliver_Right`, 19 frames, for both.
  - `PlayCheckMiss()` by hand at +7 frames: `Stumble_Back` 467 to 488.
  - Rule switched on for one figure: the stumble by rule, from 6 frames after the call.

Stills:
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-miss-by-hand-clay.png`
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-miss-rule-on-clay.png`
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-miss-by-hand-night.png`

## Job 2. The play-through (match m1, GPU, un-paused, about 25 sim frames a second while capturing)

- **Played to sim 7300.** The regular cut's clock runs out at 7200: `JAMSIM frame=7200 score=1:2 phase=1`.
  - Goals at 784 (orange), 4233 (orange) and 7179 (blue).
  - The match did not reach a "first to 3" end, so I have no BEATEN card. After 7200 the sim kept running (HUD cues to 7519).
  - Started by console as QA5 did (`jam.Cut`, `jam.ScriptHuman 1`), not from the menu.
- **Errors:** 0 Blueprint errors, 0 `Error:` lines. The one warning is my own `Screenshot` console call.
- **Knockdowns (all the human's), each with end minus release 0.0:**

| Sim frame | Length | Fall | Get-up |
|---|---|---|---|
| 2895 | 62 frames | `Knockdown_Right_Light` | `GetUp_Snap_Side_Right` |
| 3471 | 69 frames | `Knockdown_Left_Light`, replaced by `Board_Fall_Front` | `GetUp_Wall` |
| 4498 | 79 frames | `Knockdown_Right_Light` | `GetUp_Quick_Side_Right` |
| 5743 | 69 frames | `Knockdown_Left_Light`, replaced by `Board_Fall_Front` | `GetUp_Wall` |

  Also: 1 stumble shown of 1 sent; stops on 4 figures.
- **What a viewer sees** (I looked at all 33 periodic captures and the 97 burst stills):
  - The stands and the crowd sit behind the play.
  - The goal light floods the corner red.
  - The knockdown: falls, lies, gets up and skates away.
  - The stop: a crouch across the travel with a faint spray.
  - The board fall lies against the boards.
  - No figure is stuck and nothing pops.

Stills:
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-111538-B-film3-match-stands-goals.png` (stands twice, both kinds of goal)
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-111538-B-film3-match-knockdown.png`
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-111538-B-film3-match-stop.png`
- `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-111538-B-film3-match-boardfall.png`

## Job 3. SK30's film script

**How it was filmed:**
- Test figures in a drill session, side-on camera, 4 m away, 50 deg.
- Moving figures are on a treadmill: the speed drives the animation, but the body stays in place.
- Every shot was done on the clay figure, then on the night figure (`BP_SkaterVisual_TB1`, filmed on the clay level).
- Frame labels in the strips are the tick, the take and its frame.
- Numbers come from per-frame bone recordings of the same sessions. The blade point was recorded only in the night sessions.

| Shot | Verdict | What a viewer sees, and the number |
|---|---|---|
| 1 Stick lock, skating | **reads, weakly** | Clay: the lift is hard to see from this angle. Night (measured): blade 5.7 cm before, 17.4 cm at its highest (+11.7), back to 5.7 by tick 52. |
| 2 Stick lock standing, cut short | **reads** | The blade rises off the ice from frame 3 and both figures hold it. The figure cut at 140 is down by 146 (night: 17.0, 13.4, 8.3, 5.8 cm over ticks 140 to 146): a smooth fall, no snap. |
| 3 Take grace | **reads** | The shoulders turn over the stick side for about 20 frames, back by tick 256. The blade stays on the ice (night 5.7 to 4.9 cm). |
| 4 Both layers under a knockdown | **reads (no difference)** | The two figures fall, lie and get up alike: **0.00 cm** in every joint, ticks 320 to 440, clay and night. |
| 5 Missed check, called by hand | **reads** | Check, then a deep overbalanced lurch with one leg kicked back, then a crouch and back to skating by +29 frames. No fall. |
| 6 Landed check (control) | **reads** | Clean return. With the rule switched on and no bus event, the figure lurches like shot 5. |
| 7 Mallet swing with its victim | **reads; the drum does not read as too high** | Zoomed side-on on frames 13 to 17, the drum passes at the attacker's **hip/waist**, well below the shoulder. The 0.6 cm margin from SK30 is measured to the lower upper-arm joint in a deep crouch; it does not show on screen. The drum meets the victim's hips at frames 16 to 17 and the victim sits down hard. `NS_mallet_bonk` at the B44 offset makes a cream puff at the victim's hips. On frames 18 to 25 it **covers half the victim**. |
| 7b Mallet miss | **reads** | An over-swing, then back to the carry. |
| 8 Pin with `Fall_Legs_Back` | **reads** | Legs taken from the front; the victim falls **away** onto his back. Head ahead of the attacker's root: 80 / 79 cm at contact, never nearer than 92 cm during the fall and lie, 173 / 175 cm lying (clay / night). The pin itself is hidden between the bodies at contact. `NS_pin_roll` (root moved 9 cm back): **not seen**. |
| 9 Weapon into a dash | **reads** | Recorded: prop shown on **0 of 92** dash frames, mallet and pin, clay and night; back at tick 527. On screen: stick in hand through the dash, mallet back after `Dash_End`. The still asked on tick 409 still shows the mallet. Under FILM2's "screenshots land a frame late" rule that would be dash frame 0, which contradicts the recording; I did not settle it. The pin figure was hidden behind the mallet figure. |
| 10 Skid's hold | **reads as a held skid; the sway is too small to see in stills** | `Dash_Skid_Left` held on take frames 7 to 9. Largest one-frame step 1.0 / 1.1 cm. `NS_dash_skid` (attached to the root): grey dabs behind the skates for about 12 frames, calm. |
| 11 `Dash_Carry` | **reads as stick-handling** | Stick forward with the blade on the ice (4.1 to 4.2 cm clay, 5.7 to 5.8 cm night, on every carry frame), not raised. The legs are the dash's. `NS_dash_streak` (pelvis) and `NS_dash_push` (at the skates, frames 2 and 14 of each cycle, `Variant` 1 left / 0 right): **not seen side-on**. |
| 12 `Dash_Wipeout_Light` | **reads, with a snap** | A catch with the arms flung forward, a crouch, then back to skating; no fall. Head lowest 70 cm. On frames 2 to 3, `hand_l` moves 68 cm (clay) / 50 cm (night) in one frame. The dash loop itself moves at most 16 to 18 cm a frame. It is visible as a snap of the arms. |
| 13 Quick get-ups | **not filmed** | Not in this brief's list; the time went elsewhere. |

Stills (all in `C:\Users\kazda\jam\wt\b-fig\devlog\shots\`, each with a `.txt` caption):
- `20261114-112525-B-film3-lock-skating-clay.png`
- `20261114-112525-B-film3-lock-standing-clay.png`
- `20261114-112525-B-film3-lock-standing-night.png`
- `20261114-112525-B-film3-grace-clay.png`
- `20261114-112525-B-film3-layers-knockdown-clay.png`
- `20261114-112525-B-film3-miss-by-hand-clay.png`
- `20261114-112525-B-film3-miss-by-hand-night.png`
- `20261114-112525-B-film3-miss-rule-on-clay.png`
- `20261114-112525-B-film3-mallet-clay.png`
- `20261114-112525-B-film3-mallet-frame13-17-zoom-clay.png`
- `20261114-112525-B-film3-mallet-miss-clay.png`
- `20261114-112525-B-film3-mallet-night.png`
- `20261114-112525-B-film3-pin-clay.png`
- `20261114-112525-B-film3-pin-night.png`
- `20261114-112525-B-film3-mallet-into-dash-clay.png`
- `20261114-112525-B-film3-skid-hold-clay.png`
- `20261114-112525-B-film3-dash-carry-clay.png`
- `20261114-112525-B-film3-dash-carry-zoom-clay.png`
- `20261114-112525-B-film3-wipeout-light-clay.png`
- `20261114-112525-B-film3-wipeout-light-night.png`

## Not verified

- **Seeing:**
  - Nothing of job 3 is seen from the game camera; every shot is side-on and close.
  - I judged from stills, not from watching motion: the skid's sway, the wipeout's snap in motion, and whether the effects show from above.
  - Sound: I listened to nothing.
- **Where the sim divergence comes from:** that it came with dev-b is inferred, because three runs on this tree agree with each other. dev-b alone was not run.
- **The menu:** the match was started by console, not from the menu.
- **The match's end:** it ran to sim 7300 without a 3-goal result.
- **Night:** the night figure was filmed on the clay level; no night match.
- **The effects:**
  - Spawned by Python at the B44 places, not by the figure.
  - `NS_pin_roll`, `NS_dash_push` and `NS_dash_streak` were not seen, so I cannot say whether they work or only do not show from this angle.
  - `NS_dash_wipeout`, `NS_dive*`, `NS_slide_tackle`, `NS_cannon_fire` and `NS_giant_sweep` were not spawned.
- **Weapon into a dash:** the still on tick 409 against the recording, as in the shot 9 row.
- **Blade numbers on clay:** not recorded; the recorder gained the blade point only after the clay sessions.

## For the engine lane (as the figures play now, 60 fps)

| Take / layer | Frames | Starts on | Ends on | Trigger | Must never |
|---|---|---|---|---|---|
| `Stumble_Back` as the missed check | 22 (from its frame 2) | `Check_Deliver_*` frame 6 to 7 | skating | `PlayCheckMiss()` only. The figure's own rule (no bus event within 0.108 s) runs only if `bAutoCheckMiss` is true; default false | play by default after a check; play while down or getting up |
| `Check_Deliver_*` with no bus event | clay 8, night 19 | standing/skating | skating | `PlayCheck` | turn into a stumble unless `bAutoCheckMiss` is on |
| `Dash_Wipeout_Light` | 14 played (230 to 243), then a blend out | `Dash_Loop` | skating | `EndDash(4,…)` | a fall; a one-frame arm snap (today 68 cm at frames 2 to 3: to fix) |
| Others | as in `status\SK30-report.md` section 4, unchanged | | | | |

## For the other device

- **`PlayCheckMiss()` (no inputs) on `BP_SkaterVisual_TB1` / `_TB1_P3`:** call it on the attacker when the sim decides a check missed, at the earliest 7 frames after `PlayCheck`. The figure no longer decides misses by itself.
- **The automatic rule is still there:** `bAutoCheckMiss = true` on an instance switches it back on. It is Instance Editable; the game can set it as a property.
- **Seed 1 on dev-b now plays differently from 06:40 this morning.** The sim hash at frame 1200 is `02b5afd69a5fa672`; SK29 had `1bc5d4f14f39e53b`. If that is not intended, a change that came through dev-b alters the sim: check the files that came in with the merge, for example `HokejAcceptDrill.cpp`, `JamCarrierMarks.h` and the camera.

## Decisions for the operator

1. **The missed check, now off.** Should the sim decide misses (it calls `PlayCheckMiss`), or should they stay out?
   - Stills: `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-miss-by-hand-clay.png`, `...-miss-rule-on-clay.png`.
2. **The mallet's drum height:** it reads at the hip, not the shoulder. No change needed, unless you see it otherwise.
   - Still: `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-mallet-frame13-17-zoom-clay.png`.
3. **The bonk's puff covers half the victim for about 8 frames.** Keep it, or make it smaller (calm, readable ice)?
   - Still: `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-mallet-clay.png`.
4. **The light wipeout's arm snap:** blend 0.1 s, or ask the dash lab for a softer frame 2?
   - Still: `C:\Users\kazda\jam\wt\b-fig\devlog\shots\20261114-112525-B-film3-wipeout-light-clay.png`.

## The next three evolutions

1. **The light wipeout without the snap:** blend in 0.1 s (`EndDash` reason 4). Proving number: the largest one-frame joint step on frames 0 to 5 at or under 20 cm, on clay and night.
2. **The effects seen from the game camera:** the dash push, streak and pin line at the camera's height and distance. Proving number: each one's share of the frame at its peak, against the frame without it; above 0.3 % to count as seen.
3. **Shot 13 and a night match:** the quick get-ups filmed, and seed 1 on the night level. Proving numbers: 0 Blueprint errors, end minus release 0.0 for every knockdown, the front quick get-up's head rise per frame.

## Ledger rows (both Blueprints' bytes changed)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 10.60 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event (lane SK22); StopTick plays MS_brake (lane AU11); fall sizes, snap get-up, GetUp_Wall_Back, wall shift, PlayStumble and SetBraking by name, late get-ups' test switch (lane SK23); + BrakeFx Niagara component (NS_brake) (lane B40); the pawn's PlayStumble covers every stumble, the level-driven gated stop, lying loops from the fall's end pose (lane SK24); whip weights, check sizes, the eleven set calls, WeaponMesh on stick_socket, test trigger SK26Test (lane SK26); the timed get-up on its frame, UpBlend (lane SK29); SetStickLock and SetTakeGrace (layers through the ABP's LockRot/GraceRot, not under a take of priority 5 or more), PlayCheckMiss and the miss rule (no check bus event within 0.108 s of PlayCheck), the pin's victim Fall_Legs_Back, the skid's hold sway (SkidSway 1.5), UpBlendQ, test calls Tune and TestBusCheck (lane SK30, previous run); SetHasPuck stored in bPuck and Dash_Carry at the dash loop's start with the puck, EndDash reason 4 plays Dash_Wipeout_Light, the carried prop hidden and the stick shown from the dash's first frame (lane SK30); the miss rule behind bAutoCheckMiss (bool, default false, Instance Editable): PlayCheck arms it only when true, PlayCheckMiss unchanged (lane FILM3) (10599364 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 10.81 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12 to SK19, QA3); + SlideFx (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m while down (lane SK16); SK2Tick (lane QA4); stumble, board falls, quick get-ups, hard stop, deke (lane SK21); the gated hard stop (lane SK22); MS_brake (lane AU11); lane SK23's calls; + BrakeFx (NS_brake) (lane B40); lane SK24's stop and lying loops; lane SK26's calls, WeaponMesh and test trigger; the timed get-up's start and blend (lane SK29); the three calls, the miss rule, the pin's victim, the skid's sway, UpBlendQ and the test calls as BP_SkaterVisual_TB1 (lane SK30, previous run); Dash_Carry with the puck, EndDash reason 4 (Dash_Wipeout_Light), the prop hidden from the dash's first frame, as BP_SkaterVisual_TB1 (lane SK30); the miss rule behind bAutoCheckMiss (default false, Instance Editable) as BP_SkaterVisual_TB1 (lane FILM3) (10808409 bytes) |

## Files

- **Lab folder (scripts, recordings, strips):** `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\film3\`. In it:
  - Blueprint edit: `miss.mjs`
  - Film harness and its sessions: `w3.py`, `gen3.py`
  - Match runners: `m3.sh`, `qah3.sh`
  - Analysis: `cmp3.py`, `ana3.py`, `fstrip3.py`
  - Logs and recordings: `logs\full_m1.txt`, `full_m2.txt`, `full_m3.txt`, `logs\w6_{cA,cW,cD,nA,nW,nD}.json`
- **Copied in:** the earlier lanes' helpers came from `...\scratchpad\sk25\`, with paths repointed. That folder was not changed.
