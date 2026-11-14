# Contact 11: the hits last long enough to be seen (lane A192, track F1, evolution 10; 2026-11-14)

On the f1b tree with evolution 9 (CONTACT-10.md) baselined. The operator: "Animations of crosscheck and body check are short and not clear". Four items:
**the wind-ups on at 4 / 4**, **the scripted human mixes the brace and the check**, **the prices on the tree that ships** (free skaters x 1.15), **a braced
hit says so** on the HUD. Device B had not answered A-to-B-0028 when the number was fixed (newest B-to-A: 0043, 11:32): 4 / 4. Cells with the ladder's
instrument (`JAM_PLAYTESTER_CHECK=closing`), seeds 1 to 600, paired. Status: `status/A192.md`. Drivers `tools/perf/results/a184/`.

## 0. What ships

| Key (`game.contact`) | Ships | Neutral |
|---|---|---|
| `cross_hold` | true (evolution 9) | false |
| `check_windup_frames` | **4** | 0 |
| `cross_windup_frames` | **4** | 0 |

**Frames from call to contact, for device B's takes (as shipped):**
- **Body check**: `PlayCheck` and `PlayCheckWindup(4)` on the press; the window opens 4 frames later and lasts 12 frames for the human (6 for a bot): the contact
  comes 4 to 16 frames after the call (the pressers aim it at about 5 to 8: the brain presses when the closing speed brings the bodies together in about 4 frames
  after the window opens; the scripted human 1 to 7 frames after).
- **Cross-check (pressed)**: `PlayLift` and `PlayCrossCheck(4)` on the press; the stick's strip (or miss) is on the press's frame; the **shove lands exactly 4
  frames later** on the man it was aimed at, if he is still within the shaft's reach.
- **Cross-check (held)**: `SetCrossCheckHold(true)` from the 10th frame held; the hit is the frame the bodies meet (no wind-up: the pose is the wind-up);
  `SetCrossCheckHold(false)` on that frame or on release.

**The three cells** (novice at rookie / regular at pro / good at all_star):

| arm | cells | against everything off |
|---|---|---|
| everything off (the tree before evolution 9) | 86.5 / 53.2 / 30.0 | 600 / 600 hash-identical to the tree as it arrived, a cell |
| **as shipped: brace + wind-ups 4 / 4 + the mix** | **86.5 / 56.3 / 28.2** | 0.0 / +3.1 / -1.8 |
| with free skaters x 1.15 (see section 3): everything off | 66.8 / 50.8 / 36.8 | |
| **with free skaters x 1.15: as shipped (for the curve lane)** | **71.0 / 53.7 / 31.0** | +4.2 / +2.9 / -5.8 |
| with free skaters x 1.15: the brace held all match | 64.8 / 41.7 / 28.0 | -2.0 / -9.1 / -8.8 |

## 1. The wind-ups on (item 1)

**A change to evolution 9's cross wind-up**: it delayed the whole swing, so every stick lift (the bots' steal, 40 to 50 a match) came 4 frames late: the spin's
price, the late answer and the body shield's stop-and-go drills moved. Now the **stick's part stays on the press** (the strip, the miss, as before) and only the
cross-check's **shove** is deferred (`UJamSim::CrossShove` / `LandCrossShove`; state `CrossTarget`, `bCrossTargetStickOnly`, hashed while `LiftWindupLeft` runs,
guarded in `Jam.Net.HashCoverage`). The shove lands if its man is still on his feet, out of the box, unshielded, within `cross_reach_m`, inside the cone, and not
into his back (unless a stick-only shield had widened it at the press). And **the wind-ups also run out in a goal pause** (a window opened before a goal
always stayed open in the pause; a wind-up pressed before it now does the same).

**The staged tests** (gate with 4 / 4 first: 256 PASS / 18 FAIL; after the shove change 9 were left). Each: the press, the wind-up's frames, the same assertion
(no assertion was changed):

| test (file) | what changed |
|---|---|
| `Jam.Core.TacklingEvents` (`JamTacklingTests.cpp`) | the checker starts `V x wind-up / 60` farther out; `ClearIce` puts the puck at the side boards: at (Hx, 0) it lay in a net mouth and every scene had been played in the goal pause after a first-frame goal |
| `Jam.Core.Stick` section 5 and "penalties off" (`JamStickTests.cpp`) | the charger starts the wind-up's ground farther out; the scene runs the wind-up's frames longer |
| `Jam.Core.Stick` section 5b, foul grace | the charge starts the wind-up earlier after the strip (so the hit lands as many frames after the strip as before), from the wind-up's ground farther out, 4 frames longer |
| `Jam.Core.ModeKeys.PenaltyFrames` (`ModeKeysTests.cpp`) | the charger starts farther out, the scene runs longer |
| `Jam.Core.SpinCheckedFalls` (`JamStickHandlingTests.cpp`) | the defender presses the wind-up earlier (as the brain does) and not again while a wind-up runs |
| `Jam.Bot.Hockey.BoxedIsOffIce` (`HockeyBrainTests.cpp`) | the victim starts the closing's ground farther out, the scene runs longer |
| `Jam.Core.Contact.LawOneDrill` (`JamContactDrill.h`) | the drill presses the wind-up earlier; the nudge's attacker is seated on the frame the wind-up ends |
| `Jam.Fx.BusForB` (`BusForBTests.cpp`) | the victim stands the wind-up's ground farther out |
| `Jam.Rink.PenaltySeat`, `Jam.Rink.HitSeenBoxUsed` (`RinkReactionTests.cpp`) | the charger starts farther out and keeps skating the wind-up longer |
| `Jam.Bot.PlayerBotWins` (`BotTests.cpp`) | **the one exception**: its three seeds are a chance sample of the PlayerBot's play (R-0011). With both wind-ups on, seed 2 is lost 2:3 (check wind-up alone: won 3:2; cross alone: won 3:0). The test plays them with the wind-ups at 0, said in the code. |

**The acceptance drill** (`HokejAcceptDrill.cpp`): step 7's RMB cue comes the wind-up earlier (`HumanCheckWindowFrames - 2 + check_windup_frames` frames of
closing); step 8's opponent presses the wind-up earlier. The rows and their verdicts are unchanged. Not run (a packaged run needs the cursor): **queued with the
Director**.

**Law 3's drill**: `Jam.Core.Movement.StopAndGo` and `Bar.Curve.Law3` pass with the wind-ups on. The gate's rows are unchanged (pro stop begun at 8 m: 200 of
200; all_star at 8 m: 200 of 200; 9.5 m 92.0 %). **Not every LAW3 line is**: all_star stop begun at 11 m 53.0 -> 46.5 %, at 12 m 28.5 -> 19.0 %; pro at 9.5 m
with the mouse 97.0 -> 96.5 %; the defender presses its check many more times (all_star 8 m: 12 -> 165 presses in 200 runs). The drill's defender is the brain,
which presses on the wind-up's lead (its arm gap grows by closing speed x 4 / 60); against a carrier who is stopping that lead overshoots, most of those presses
are whiffs, and on a stop begun too late (11 m, 12 m) more of them land. The bar the gate holds (the stop at 8 m passes) is untouched; the lines a stop begun
too late reads are harsher. Next evolution 1 makes them identical.

The bots' landed checks a pro match: 10.59 (everything off) against 11.31 as shipped: the anticipation keeps them.

## 2. The scripted human mixes the brace and the check (item 2)

He commits to the brace only on a long run: the carrier 2.5 to 6 m away body to body, closing at 6 m/s or more (`JAM_PLAYTESTER_BRACE_CLOSING` tunes it), not
into his back; he keeps it on that carrier while he still closes on him. On a short approach he presses the body check as before.

| as shipped (no speed) | rookie | pro | all_star |
|---|---|---|---|
| check presses a match (evolution 9: 0) | 7.15 | **6.62** | 6.30 |
| braced hits a match | 1.02 | **1.63** | 0.63 |
| his landed cross-checks (pressed + braced) | 2.81 | 2.52 | 1.04 |
| knockdowns he gives / takes | 1.39 / 3.02 | 2.36 / 7.05 | 1.61 / 8.39 |

The bars (pro: presses at 5 or more, braced hits at 1.0 or more): **met**.

## 3. The prices on the tree that ships (item 3)

`skater.free_speed_mult` is not on this tree. Measured with a **temporary** seam (env `JAM_TEST_FREE_SPEED=1.15`: main's two lines in `StepSkater`, top speed and
thrust x 1.15 for a skater without the puck), removed before the final build. Main's other changes around that key (the carrier marks' new distances, the curve
lane's re-tune, the acceptance drill's walk) are not in it: re-read the cells on the merged tree.

**Holding the brace all match loses**: against the shipped use, -6.2 / **-12.0** / -3.0 (at all_star exactly 3.0); against everything off, -2.0 / -9.1 / -8.8.
On the faster tree a skater who holds it all match fells 13 to 18 men a match and still loses. The prices stay as evolution 9 set them.

**Which numbers scale with speed**: the frames down of a braced hit (law 1's line by closing speed: 67 / 97 / 127 / 150 frames at 4 / 8 / 12 / 16 m/s); the
braced hits a match (sF1 against s1 at pro 1.65 against 1.61) and the knockdowns given. The prices, the tap's 10 frames and the wind-ups' frames do not.

## 4. A braced hit says so (item 4)

`UJamHudWidget::UpdateBraceLine` (`HUDBrace.cpp`, one call after `UpdateCarrierCues`): the human's braced hit that landed (`CrossBraceHits` grew) shows
**BRACED HIT** on the contact line; one he took out of an opponent's brace (the new read-out `CrossBraceTaken` grew) shows **RUN THROUGH**. `JAMHUD ...
contact_line=BRACED_HIT` / `RUN_THROUGH` in the log. Not seen headless (the HUD does not run under `-nullrhi`): a row for the acceptance run is the screen lane's.

## 5. The proof

- Every key off: 600 / 600 hash-identical to the tree as it arrived in each cell (`n1` against `base`).
- The final build (seam removed) plays the shipped cartridge 600 / 600 hash-identical to the measured arm `s1` in each cell.
- Full `Jam` filter, final build, clean environment (no `JAM_` variable): **274 PASS, 0 FAIL** (`Game/logs/a184/gate6.log`).
- `node tools/gate/check-test-guards.mjs`: PASS, 0 faults in 198 files. Schema tests 99 pass. Shipping compile of the Game target: PASS (60 s).
- State added: `CrossTarget`, `bCrossTargetStickOnly` (hashed while `LiftWindupLeft` runs; guards listed); read-out `CrossBraceTaken` (listed).

## 6. The next three evolutions

1. **Law 3's lines identical with the wind-ups on**: the brain's press leads by the wind-up only while the carrier is not braking (or on the predicted gap with
   the carrier's own deceleration). Proving numbers: the 11 LAW3 lines character for character as with the wind-ups off, the bots' landed checks a pro match
   within 0.5 of 10.6.
2. **Device B's frames**: when B answers A-to-B-0028, the keys go to its numbers (up to 12) with the cells and the gate re-run. Proving numbers: the full `Jam`
   filter green, the three cells within 2 of 4 / 4's.
3. **The merged tree's prices**: on main with `skater.free_speed_mult` and the curve lane's re-set, the brace's prices re-measured. Proving numbers: holding all
   match loses by 3 or more at every level against the shipped use, braced hits a pro match between 1.0 and 2.0.
