# The mechanics proven in the package, and the saucer pass (lane A77, device A, R-0036, 2026-11-13)

R-0036 puts gameplay mechanics second after collisions: "the sweep, the spin, the lifted shot, passes, one-timers, dekes: each proven in a package through the real input
path". Lane A51 built the sweep and the spin and fell short of its own bar in the package (7 sweeps read, 3 kept; the spins pressed on a timer, mostly without the puck;
no stills) because a foreign dialog held the foreground. This page is the proof run again the way A73 drove its package (nothing needs the foreground), with one
mechanic added that the lifted puck (A52) makes possible: **the saucer pass**. Status with timestamps: `status/A77.md`. The determinism hash before and after is in
section 7 (a sim rule changed: a puck over the blades passes them).

Code: `Game/Source/JamCore/{Public,Private}/JamSim.{h,cpp}` (the saucer, the blade-height rule), `JamCartridge.{h,cpp}` (`game.pass`), `AI/JamAICore.{h,cpp}` (the pro
bot's saucer), `AI/JamSimAdapter.cpp`, `AI/JamPlaytester.{h,cpp}` (the regular and good players' saucer), `JamNet/JamNetPackets.{h,cpp}` (flag bit 1024, protocol 6),
`JamGame/{Public,Private}/InputMapping.{h,cpp}` (F held), `HokejGameMode.{h,cpp}` (`ReadHumanInput`, the drills 7-11, the stills), `schema/`, the test
`Private/Tests/JamSaucerTests.cpp`, the harness `tools/qa/ctrl/a77-prove.ps1` + `a77-analyze.mjs` + `a77-strip.py` + `a77-cut.json` / `a77-cut-rookie.json`.

## 1. How the package is driven (the lesson of A51's run)

The game (`Game/Packaged/A77-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe`, Development) runs `jam.ctrl.Drill 7..11`. Only the skating is scripted; the stick, E, F
and the left button go through `ReadHumanInput` exactly as a hand's would. The drill writes a `JAMCUE <kind> frame=N ...` line at the moment a person would act (the
defender's blade on the puck, a defender closing, a blade in the pass lane, the human set at 10 m facing the net, the teammate's pass released with its ETA). The driver
tails the log (`-ForceLogFlush`) and acts within about 50 ms, into the game's Slate window and nowhere else: the cursor by `SetCursorPos` plus a posted `WM_MOUSEMOVE`
(Slate reads the OS cursor on the move), E and F by posted `WM_KEYDOWN / WM_CHAR / WM_KEYUP` (F held 220 ms for the saucer), the left button by posted
`WM_LBUTTONDOWN / UP` (held 20 / 135 / 650 ms for the tap, the half and the full charge). The menu's PLAY is one posted Return, the end is `quit` over the remote console
(A73's recipe). The log shows what arrived: `JAMCTRL ... src=mouse`, `btn=spin (E / B)`, `btn=pass held=1`, `btn=shoot held=1`, and the sim's own read-outs
(`JAMCTRL sweep`, `JAMCTRL spin ... carrying=`, `JAMCTRL saucer`, `JAMSHOT`, `JAMPASS ... saucer=`).

## 2. The saucer pass: the model (`JamSim.cpp`)

| Piece | Rule |
|---|---|
| The input | `FJamInput::bPassHeld`, a held level like `bShootHeld`. The human's game mode sends the level and never the `bPass` edge any more (`InputMapping`: `IsPassHeld`, the pass action's Completed event). Bots and scripts keep the edge (a flat pass, as before); a bot that saucers holds the level for `half_frames` and drops it |
| The charge | `StepPassCharge`, after `StepCharge`: held with the puck in shot reach and the pass cooldown over, `PassChargeFrames` grows to `pass.saucer.half_frames` (8). On the release the pass goes out: a **saucer** when the hold reached `half_frames`, a flat pass when shorter (so a tap of F is the pass it always was, one or two frames later than the press). A hold that never had the puck fires nothing. `PassChargeFrames` is hashed only while a pass is held |
| The receiver | `PickReceiver(S, Aim, bSaucer)`: the flat pass still refuses a lane with an opponent within 0.9 m of the segment ("a stick in the lane"). A saucer accepts a lane whose nearest opponent body is at least `SaucerLaneMinM` 0.55 m off the line (the body still stops a puck under 1.8 m: the saucer flies over a **stick**, not a skater) and sits inside the airborne part of the way (more than 0.5 m out, under `SaucerLandFrac` 0.75 of the distance minus 0.5 m). Behind that the puck has landed and a blade takes it again, so a defender standing on the receiver's blade still blocks |
| The flight | `ApplySaucerLift(DistM, Speed)`: the puck leaves at `skater.pass_speed` (14) with `VZ` such that it lands `SaucerLandFrac` of the way to the aim point, solved with the puck's drag the way `ApplyShotLift` does (the discrete `v *= 1 - k h` per substep), capped at the flight whose apex is `pass.saucer.lift_m` (0.35): a short pass is a low arc that lands three quarters of the way, a long pass flies the full 0.35 m arc (0.53 s, about 6.5 m) and slides the rest. The half-substep term is carried so Z is 0 on the landing frame |
| Over the blade | **New sim rule**: a puck with `Z >= BladeClearM` (0.2 m: a blade is a few centimetres tall, the puck's centre line is 1.2 cm up) passes every blade: no deflection, no reception, no poke (`StepPuck`'s blade block and `ResolvePokes` test it). The body under `BodyHeightM` 1.8 and the crease skater's block under `goalie.block_height_m` are untouched. This also changes a lifted **shot**: it now passes a blade it used to be deflected by while it is in the air, which is the honest reading of A52's height (section 7: the hash moved) |
| The landing | the ice bounce as A52 made it (`ice_restitution` 0.3; a 0.35 m arc lands at 2.6 m/s down, hops 3 cm, dies on the next bounce under `BounceDeadM`); the receiver's blade (within 0.6 m) or body (within `stick.reception_m` 0.9, under 14 m/s) seats it as any pass |
| Read-outs | the `pass` event's Kind is `saucer_to_N` (a flat pass stays `to_N`); `FJamSkaterState::bSaucerThisFrame`, `Saucers`; the game mode logs `JAMPASS ... saucer=1` for everyone and `JAMCTRL frame=N saucer receiver=R vz=` for the human. No new interface event: B's `PlayPass` fires as for any pass; the puck visual already draws `Z` (A52) |

## 3. The keys (`schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`; gen-schema-data, gen-parity, gen-designer-data re-run)

| key | default | range | what |
|---|---|---|---|
| `game.pass.saucer.half_frames` | 8 | 0..60 | frames pass (F / X) must be held with the puck in reach for the saucer; a shorter hold or the tap is the flat pass; 0 = off |
| `game.pass.saucer.lift_m` | 0.35 | 0..1 | the saucer's apex over the ice; clears a blade (0.2 m), never a body (1.8 m); 0 = off |

`game.pass` is a new top-level object beside `shot` (the flat pass's own keys stay `skater.pass_speed` / `pass_lead_frames` where A16 put them). Neither key is
Designer-editable (passing stays the host's, like `shot`). The shipped cartridges are unchanged and take the defaults.

## 4. The wire, the brain, the playtester

- **Wire**: `FJamNetQuantInput::EFlag::PassHeld = 1024` (a held level, re-read every frame like `ShootHeld`), `JamNet::Version` 5 -> **6**, still 10 bytes per input
  (two flag bytes since protocol 4). Done because A38's checklist in `JamNetPackets.h` says a lane adding an `FJamInput` field does exactly this and `Jam.Net.InputCoverage`
  fails until it is listed; JamNet is not in this lane's ownership list, so the Director decides whether the bump stays (a protocol-5 guest is ignored, R-0037 point 3).
  `LobbyGameMode::ReadLocalInput` is untouched: a joined match's human passes flat (the edge) and cannot saucer until that lane reads `IsPassHeld`.
- **Brain** (`JamAICore.cpp`, `Think` after `StickPlay`): a carrier whose level charges its shots (pro, all_star; rookie's `ShotChargeFrames` is 0) that wants to pass
  (`In.bPass` from the pass-out, the forecheck yield or the pass to the human) while the sim has **no flat receiver** (`W.CarrierBestPass < 0`) but **a saucer receiver**
  (`W.CarrierBestSaucer >= 0`, the adapter fills it from `BestPass(..., true)`) holds `bPassHeld` for `W.SaucerHalfFrames` and releases it; while it holds, `StickPlay`
  caps the blade's reach as A66's rule does inside shooting range (the puck must stay inside pass reach, 1.5 m, or the release fires nothing). The puck gone, the hold
  is dropped.
- **Playtester** (`JamPlaytester.cpp`): the regular and good players (`bStickHandling`), when a teammate ahead is open but the corridor to it has a stick in it (`Corridor`
  blocked, the case the pass loop used to skip) and the sim has a saucer receiver, hold pass for `half_frames` (the player stops for it) and release. The novice never.

## 5. The counts in the package against the bars (package 2 `Game/Packaged/A77-dev2`, logs `tools/qa/out/ctrl/a77-*/game.log`, verdicts `a77-analyze.mjs`)

Package 1's three runs are void (my `MechDrillMate` line sat between A49's `if` and its `else`, so the human's drill input was overwritten and it stood at the dot:
`status/A77.md` 14:42); the table is package 2. The real input path is proven on every run: the cursor by a posted move (`src=mouse`), E (`btn=spin`), F held
(`btn=pass held=1`), the left button held (`btn=shoot held=1`), each arriving one to three frames after its cue.

| mechanic | the bar (brief) | the count | the proof path | the still |
|---|---|---|---|---|
| **Sweep** (drill 7, pro, 59 s) | ten sweeps past a pro defender, the puck kept | **22 cues, 26 `JAMCTRL sweep` read-outs (met); 10 attempts, kept 30 frames later 3, poked clean 1, knocked 1 (not met: 3 of 10 kept)**; 19 clean lifts in the run | `JAMCUE sweep` (the defender's blade within 0.9 m of the puck) -> a posted cursor crossing in three steps -> `JAMCTRL sweep side= shield_frames=12` -> `JAMDRILL stick_result sweep` | `img/a77/a77-sweep-f878-strip.png` (150 stills over 10 s, every 24th frame shown) |
| **Spin** (drill 8, pro, 81 s) | ten spins with the puck, kept, the stamina bar visible; one checked mid-turn | **9 cues, 8 E edges, 8 spins accepted (stamina 0.75 on each), 5 with the puck (not met: 5 of 10); kept 1 of 5; one spin checked mid-turn, `knock_frames=135` (met)** | `JAMCUE spin` (carrying, a defender within 2.5 m, bar and cooldown ready) -> posted E -> `JAMCTRL spin frames=23 ... carrying=` -> `stick_result spin` | `a77-spin-f190-strip.png`, `a77-spin-f728-strip.png` (24 each, the stamina bar in the HUD) |
| **Saucer** (drill 9, pro, 150 s) | ten saucers over a defender's blade, landed before the receiver | **24 cues, 21 F holds, 4 saucers (not met), over a blade 1, landed before the receiver 1, received 0, taken 3**; the pro bots passed 0 times, so the brain's saucer did not show | `JAMCUE saucer` (a saucer receiver, an opponent within 1.6 m of the lane) -> F posted down 220 ms -> `JAMPASS ... human saucer=1`, `JAMCTRL saucer vz=2.48` -> `JAMDRILL saucer over_blade= max_z= landed_frame= received=` | `a77-saucer-f1620-strip.png` (cleared a blade at the 0.35 apex), `a77-saucer-f1393-strip.png` (taken at 8 cm) |
| **Lifted shot** (drill 10, rookie, 150 s) | ten taps, ten half, ten full from 10 m, the arrival heights against `shot.lift` | **2 cues, 2 left-button holds arrived, 0 shots fired (not met)**; the heights are proven by `Jam.Core.LiftedShot` only (0.050 / 0.450 / 0.950 m from 12 m) | `JAMCUE shot kind=` (at the 10 m spot, facing the net within 12 degrees) -> posted left button held 20 / 135 / 650 ms -> `JAMSHOT` -> `JAMDRILL shot arrival_z= lift_target=` | none |
| **One-timer** (drill 11, rookie, 120 s) | five one-timers off a pass | **0 cues, 0 passes by the scripted teammate (not met)**; the mechanic itself is `Jam.Core.Payoffs.OneTimer` (PASS) | the scripted mate at 6 m out / 6 m aside passes when the sim's pass goes to the human at its spot -> `JAMCUE onetimer eta_frames=` -> a timed left-button tap -> `JAMDRILL onetimer one_timer=` | none |

What the two days' numbers say together (A51's run, A73's counts, these): **against the pro brain the stick lift at contact decides the 1v1, not the poke.** The sweep
shields the poke (1 clean poke in 10 sweeps here, 0 of 10 in the test) and the spin shields every poke (0 clean pokes in the spin run), but `TryLift` has no shield and fires
at 1.7 m body-to-body: 19 and 11 clean lifts in the two runs took the puck inside the shield windows. A sweep or a spin has its moment when the defender's body is held off
(a scripted defender that stops 2 m out, A51's test; a rookie, which never lifts) and rarely against a pro that closes. Whether the spin should shield the lift (one condition
in `TryLift`: not while `SpinFrames > 0`) and whether the sweep's shield should cover a lift from the side the puck left are the Director's calibrations; this lane did not
change the sim for them (the hash record below is the shipped rule).

Why the shot and one-timer drills gave nothing in the box: the human reached the 10 m spot carrying and facing the net inside 12 degrees only twice in 150 s (the
rookie bots take the puck off it by numbers, not by stick), and the two taps that arrived fired no shot (unexplained: the next lane logs `ChargeFrames` and `ShotReadyFrame`
on the release frame); the one-timer's set piece (the teammate at its spot with the puck, the human at its spot, the sim's pass to the human) never formed in 120 s. Both
drills need the puck handed to the right skater by the drill (a `SetPuck` on a cvar, the way the tests do), not won from the bots; that is a drill change, not a mechanic one.

## 6. The stills (`docs/experiments/img/a77/`, every 4th sim frame, the tile label is the sim frame and the offset)

| Strip | What it shows |
|---|---|
| `a77-sweep-f878-strip.png` | the human (blue, the YOU ring) carrying through the sweep drill's crossings, the orange defenders at arm's length, frames 878-1474 (every 24th frame of the 150-still capture; the raw 150 are in `tools/qa/out/ctrl/a77-sweep/stills/`) |
| `a77-spin-f190-strip.png` | the spin pressed with the puck 1.10 m from the defender: the figure turns through +0..+20, the defender at the body, the stamina bar in the HUD on every tile |
| `a77-spin-f728-strip.png` | the second spin with the puck (defender 2.28 m) |
| `a77-saucer-f1620-strip.png` | the saucer that cleared a blade: the release at +4 (the puck leaves the blade lifted), the flight away up the ice |
| `a77-saucer-f1393-strip.png` | the saucer taken 0.29 m from a blade while 8 cm up: the lane rule's margin, section 5 of the status |

Eyes: **blocked** on the account, not the tool: `tools/eyes` returns 402 `RESOURCE_EXHAUSTED` ("Your prepayment credits are depleted"; `img/a77/eyes/a77-sweep-bughunt.json`
holds the refusal). The operator tops the AI Studio project up; until then no lane has a second pair of eyes. I read the strips myself: nothing fires wrong or late in
them; the puck at tile scale is a dot on the blade and the lifted saucer needs a 10 m framing (A52's note) to be seen as lifted.

## 7. Determinism

`Jam.Core.Determinism`, seed 1, frame 600, headless (`Game/logs/a77-det-before.log`, `a77-tests-1.log`):

| cut | before (merged baseline, LIFTED-PUCK.md section 11) | after |
|---|---|---|
| hockey.regular | `ccb658f82e4ca910` | **`ccb658f82e4ca910`** (unchanged) |
| hockey.raw | `e09eebd2cebd1337` | **`e09eebd2cebd1337`** (unchanged) |

The scripted scenes hold no lifted puck passing a blade and send the pass edge, so they hash as before; a bot match with a lifted shot past a blade will differ from
main (the package runs' `JAMSIM` hashes are not comparable across cuts and drills). What can move it: the blade-height rule (a lifted shot passing a blade in the air), and the human's pass now firing on the release instead of the press (the scripted
Determinism scenes send the `bPass` edge, so only the first applies to them). `PassChargeFrames` enters the hash only while a pass is held.

## 8. What the operator will feel

1. **F has a hold.** A tap is the pass it always was. Hold F a seventh of a second and the puck leaves the blade in a low arc, over the defender's stick that was in the
   lane, and lands flat three quarters of the way to the receiver, who takes it as any pass. With the lane open the hold still saucers (the arc is the choice, not the
   lane). A defender standing in the lane with its body still blocks: the saucer beats a stick, not a skater.
2. **The pro bots saucer back.** When their flat lane is closed by the player's stick, a pro or all_star carrier lifts the pass over it; the rookie never does, and the
   lane the player closes with the body stays closed. A puck in the air cannot be poked or lifted: the operator's forecheck has to read the pass, not swing at it.
3. **The spin beats the poke, not the lift.** (Measured, section 5.) A51's spin shields every opponent's poke for 24 frames; `TryLift` has no shield and the pro bots lift at contact (A73 counted
   55 lifts against 2 checks). A spin started with the defender already at the body loses the puck to the lift; a spin started at 2 m, skating past, keeps it. The drill
   measured it (section 5). Whether the spin should also shield the lift is a Director's call (one line in `TryLift`), not made here.
