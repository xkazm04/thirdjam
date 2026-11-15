# HARDENING-6: the night of 14 to 15 November (lane A212, track S3)

The game changed more tonight than in the whole day before (R-0067): the pass in flight is puck state (`PassFrames`, `PassBy`), the pass's power by hold,
the shot twice as fast (51.6 m/s at full charge), the pickup by touch with the puck swept against bodies (lane A208), the check's pull and the cross-check
on any man with the human's 8-frame wind-up, the brain's evolution 9. This file says what holds on each new main: no crash, no desync, no lie, and no
acceptance row that will go red in the release chain's window. Yesterday's suite and its method are in `HARDENING-5.md`; the live table is `status/A212.md`.

## The suite as it lands (SUITE READY)

| test / tool | where | in the gate | what it proves |
|---|---|---|---|
| `Jam.Hard.InputFuzzSmoke` | `Game/Source/JamGame/Private/HardTests.cpp` | yes | 4 matches x 3,000 frames of the input fuzz below, twice each |
| `Jam.Hard.BindingsFile` | same | yes | 15 damaged saved-bindings cases: always a valid table, every verb reachable, a rebinding round-trips |
| `Bar.Hard.InputFuzz` | same | no | `JAM_HARD_MATCHES` matches (48 cases: regular, large arena, boost, chaos, the twelve modes x 3 levels) of random presses on every rebindable verb through random rebindings and the two-role resolver, played twice: nothing not finite, nobody outside, every pause on its frame, over by the hard stop, lockstep hash equal |
| `Bar.Hard.PuckEscape` (new) | same | no | the brain alone over the 48 cases x `JAM_HARD_SEEDS`: the escape scan below; fails on any escape |
| `Bar.Hard.AcceptRows` (new) | same | no | the acceptance drill's rows of steps 1 to 3 and 7 to 10 replayed in the sim through the mapping layer; fails on a red row |
| `Bar.Hard.BindingsReach`, `Bar.Hard.RebindRepro` | same | no | yesterday's FAULT 1 / FAULT 2 probes: both pass since main 972ba542 |
| `Bar.Hard.BindingsWrite` / `Bar.Hard.BindingsRead` | same | no | a rebinding survives a restart (two processes) |
| `Jam.Net.SoakPassState` (new) | `Game/Source/JamNet/Private/Tests/JamNetSoakTests.cpp` | yes | a deliberate one-field mismatch in `Puck.PassFrames`, then `Puck.PassBy`, on the guest's machine while a pass is in flight: named at its frame by the field compare, the lockstep hash differs from that frame, the DESYNC card at the next checkpoint |
| soak harness `ToInput` / bot copy / `BraceMode` | same | | the held cross-check (bit 4096) reaches the wire in `Bar.Net.SoakClean` / `SoakLossy` (main's copy drops it: `ToInput` at :183-193 has no `bCrossHold`) |
| `tools/fuzz/*`, `tools/soak/*` | | | the menu fuzz, the bindings-file variants, the GPU-start counter, the frame runner, the two-process rows (they call main's `tools/soak/lockstep/twoproc5.ps1`) |

`Bar.Net.LateFirstPacket` (yesterday's FAULT 3 repeat) is gone from the soak file: lane A204 carried it to `JamNetTests.cpp` as `Jam.Net.LateFirstPacket`
with the fix (both definitions linked into one module and broke the build).

### What changed in the fuzz tonight
- **Long presses.** A quarter of the hand's presses are held about a second (release chance 1.5 % a frame instead of 8 %): the pass's power hold
  (`game.pass.power.full_frames` 30), the held shot and the held cross-check now occur in every match. Read-outs `pass_full_holds` / `shot_full_holds`
  (holds of 30 frames or more) on the `JAMHARD inputfuzz` line.
- **The escape scan** (`FFuzzSim::ScanEscape`, every frame of every fuzz and escape match), four classes, each with its count of matches, events and
  frames and the first case and frame on a `JAMHARD escape` line:
  - `board`: the puck's centre more than 0.05 m past the boards while play runs, under the glass (`UJamSim::GlassHeightM`);
  - `net`: the puck inside a net's box (`UJamSim::InGoal` with its height) for 3 frames or more while play runs (a goal stops play the frame it scores);
  - `wall_goal`: a goal whose puck was beside or behind the net the frame before (came in through a side or the back);
  - `body`: the puck within 0.35 m of the centre of a standing skater who does not carry it, under `BodyHeightM`, for 3 frames or more (the operator's
    "it moves through them").

### The acceptance rows, headless (`Bar.Hard.AcceptRows`)
Each row is staged as `HokejAcceptDrill.cpp` stages it (positions, speeds, the parked skaters, the pinned puck, the hand-over 45 frames before the act),
the opponent's and the mate's scripts are the drill's, and the drill's own verdict is applied. The driver's hand goes through the game's mapping: the
default bindings, `FJamButtonResolver` stepped with `JamControls::ButtonContext` (LMB shoots with the puck and cross-checks without, RMB passes with it and
checks without), the cursor through `AHokejGameMode::CursorAimFromHit` as a still screen point the camera carries with the skater, the sweeps as the
driver plays them (75 degrees each side, 3 m out). Not replayed: the hand's latency (presses land on the cue frame), the camera checks of step 8, the
pause's wall-clock gap of step 10. The cut is the release's: `tools/qa/accept/accept-cut.json`.

## Results by main

### main 015f1618 (A208's pickup on top of 2971a99e), 00:39
| run | result |
|---|---|
| `Bar.Hard.AcceptRows` | every row green as the drill stages it; `checked_w40` red (fault 1) |
| `Jam.Core.PassShotRows` (steps 4 to 6, lane A207's) | green |
| `Jam.Hard.*`, `Jam.Net.HashCoverage`, `Jam.Net.SoakPassState`, `Jam.Net.SoakFieldArrays`, `Jam.Net.LateFirstPacket` | green |
| bindings restart (two processes) | PASS |
| bindings file, 13 variants through the real file | 13 of 13 PASS, no verb unbound |
| escape scan, smoke (4 matches) | `body` 2 of 4 matches on 2971a99e (before the pickup), 0 of 4 on 015f1618 |
| full `Jam` filter (build 7, clean env) | 341 of 341 |
| `Bar.Hard.InputFuzz` 600 | 600 of 600 over, lockstep hash equal 600 of 600, 3,289 pauses on their frame, 17,353 full pass holds, no NaN, nobody outside |
| `Bar.Net.SoakClean` protocol 9, brace and held passes, 12 x 36,000 frames | 0 desync, every edge applied, 3,799 cross-hold and 249 pass-held frames on the wire |
| `Bar.Hard.PuckEscape` 480 | board 0, net 0, wall_goal 1 (fault 2 below), body 18 events in 16 matches (8 while the carrier dekes: A208's design) |
| soak 60 cells x 100 twice | running |

## Faults found tonight
1. **Step 8 `checked` red in the release window (H35, twice): the driver.** `tools/qa/accept/accept.ps1:337` plays `act=lmb_hold` with a blocking
   `Start-Sleep`, so the loop that sends due key-ups (`:282`) cannot release step 7 k=3's W at 1,500 ms: W stays down about 40 frames into step 8's
   Settle and the human skates 4 m toward the opponent, who is still down from k=3 (the drill waits for its man on his feet only in step 7,
   `HokejAcceptDrill.cpp:275`) and stands up beside him. His check comes with no run-up and, since A208's pickup makes the handed puck the human's at
   once, it is a stumble (ratio 0.55), not a knockdown. The sim replays the window to the number (row `checked_w40` of `Bar.Hard.AcceptRows`: outcome 2,
   ratio 0.55); without the held W the same sequence is green. The rows replay the drill's own sequence for steps 7 and 8 since 01:00 (fresh scenes hid it).
2. **A goal through the side netting.** glass-house at pro, seed 9, frame 3478: a teammate's reception moves the puck 0.47 m from outside the net's side
   (y 2.12) into the box (y 1.89), and the goal test counts it. The pickup's arc has a `NetBetween` guard (`JamSim.cpp:3433`); the seat that moved this
   puck has none.

## After SUITE READY (02:00 to 07:00)

### New and changed instruments
- `Bar.Hard.EscapeList` (`Game/Source/JamGame/Private/HardEscapeTests.cpp`, new): the brain alone over the 48 cases; one `JAMHARD owngoal` line per goal
  whose last touch (the frame before) was by the side that conceded (carried in, with a pass of theirs in flight, or a deflected shot), one `JAMHARD body`
  line per puck inside a standing non-carrier for 3 frames (with mate / opponent, deke, settle, height), and the totals.
- `JAM_HARD_TRACE="<case>@<frame>"` (HardTests.cpp): the puck and every skater within 1.5 m of it, frame by frame, 8 frames each side.
- `Bar.Hard.AcceptRows`: steps 7 and 8 replay the drill's own sequence in one match (the Gap, the stand-up wait, the 40-frame Settle, the 45-frame Hand,
  held keys released by time across rows) and mirror lane A215's fixed step-8 stage (the human sends nothing from the Settle to the verdict; step 8
  waits for the man on his feet). `checked_w40` (W held 40 frames into step 8, the H35 window's case) stays as a guard on that stage: green since build 10.
- `Jam.Net.SoakPassState`: the injection lands only on a checkpoint frame with a pass in flight, so the driver compares the differing hash on that very
  frame and the DESYNC is always owed (the faceoff at the dot let an injected difference heal before the next checkpoint).
- `tools/fuzz/frame-run.sh` takes `PORT` (default 4812, not another lane's 4797) and `TAG`; `tools/fuzz/frame-bot.ps1` reads the CSVs from
  `<package>/Windows/ThirdJam/Saved`; `tools/fuzz/menu-fuzz.mjs` takes `--exe <package game binary>` and does not judge a click the game reports as failed.
- `tools/fuzz/gpu-ship.mjs` must be given the Shipping binary itself (`.../Binaries/Win64/ThirdJam-Win64-Shipping.exe`): given the launcher
  (`Windows/ThirdJam.exe`) it stops the launcher by pid and the game child keeps running (03:07: up to 15 games at once; one hang record at 03:25).
- A Shipping exe keeps its profile in `%LOCALAPPDATA%\ThirdJam\Saved` (the operator's) wherever it is unpacked; a Development package keeps its own
  `Saved` inside the package; the editor binary this worktree's `Game/Saved`.

### Faults of the night (all reduced to a repeat; owners in `status/A212.md`)
| fault | found | repeat | state at 07:00 |
|---|---|---|---|
| 1 step 8 `checked` red in the window (the driver's blocking `lmb_hold` kept W down into step 8) | 01:04 | `Bar.Hard.AcceptRows` row `checked_w40` | fixed (A215: driver + stage) |
| 2 a goal through the side netting (a reception seated the puck inside the net's box) | 01:17 | `JAM_HARD_TRACE` glass-house pro 9 @3478 | fixed (A216: mouth-only goal test): 0 of 480 since |
| 3 a carried puck inside another body (the carrier's blade inside the man; after a steal, inside the old carrier) | 02:04, 05:57 | traces in status | A216 `CarriedOutOfBodies` |
| 4 a mate's lifted puck through a body | 02:08 | `JAM_HARD_TRACE` power-play rookie 3 @4570 | A216 |

### Own-net goals (480 brain matches, seeds 1 to 10)
| game | goals | own-net | carried in by the defenders | walked in (< 6 m/s) | their pass in flight | deflected shots |
|---|---|---|---|---|---|---|
| 8cb7e104 (before the wall) | 3,106 | 181 (5.8 %) | 83 | 9 | 50 | 47 |
| b3284ba0 + A216's wall | 2,509 | 64 (2.6 %) | 9 | 2 | 3 | 29 |
