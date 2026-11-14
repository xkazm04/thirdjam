# Contact 12: how long must a wind-up be for an eye, and what it costs (lane A200, track F1, evolution 11; 2026-11-14)

On the f1b tree standing on main 58af359f (evolution 10's wind-ups 4 / 4 on, free skaters x 1.15, the breakaway bonus 0.3, brain 7). The operator (R-0064
point 3): "Animations of crosscheck and body check are short and not clear". Four frames are 67 ms. The Director's question: at 6, 8, 10 or 12 frames, for
the bots only, what happens to the game? Measured at the neutral values first (the keys equal to the shared ones, the switches off: hash-identical to main);
**the Director shipped the bots' 8 / 12 at 15:53** from the table (section 2); the human keeps 4 / 4; the switches (the dodge, the brain's brake read) stay off. Status: `status/A200.md`. Drivers: `tools/perf/results/a200/` (copies of a184's; `table.mjs`, `law3.sh`, `cells-large.mjs` new).

## 1. The bots' own wind-ups (item 1)

| Key (`game.contact`) | Range | Ships | Neutral |
|---|---|---|---|
| `bot_check_windup_frames` | 0..12 | **8** (the Director, 15:53; absent in a cartridge = 8) | the shared `check_windup_frames` (code: -1) |
| `bot_cross_windup_frames` | 0..12 | **12** (absent = 12) | the shared `cross_windup_frames` (code: -1) |

- The sim picks the length by **`FJamSkaterState::bHuman`** (`FJamContactConfig::CheckWindupFor(bHuman)` / `CrossWindupFor(bHuman)`: four sites in
  `JamSim.cpp`: the press inside a running wind-up, the press, `TryLift`'s told frames, `CrossShove`). `bHuman` is already in `StateHash()` (lane A110) and
  `SeatGuest` sets it on both peers at frame 0, so a host and a guest agree, and the online guest's seat (a human) keeps the shared length. No protocol
  change: the keys ride the cartridge. No new state field (`Jam.Net.HashCoverage` unchanged).
- The human keeps the shared 4: his own press does not feel later.
- Schema (`game.schema.json`, `limits.json`), `DEFAULTS.md`, `README.md`, both generators re-run (`JamSchemaData.inl`, `JamDesignerData.inl`).
- `JamBaseline::MakeNeutral` sets both to -1 (the shared length: the old game replays to the hash). No shipped cartridge (11 cuts, 12 modes, 13 season
  takes) writes a wind-up key: all of them take the code defaults, as 4 / 4 did, so all of them ship 8 / 12. The measurement seam `JAM_TEST_A151` takes `bot_check_windup=N;bot_cross_windup=N` (the gate and law 3
  at a bots' arm; never set in a game or the gate).
- The brain: its press leads by the **bots'** length (`FJamAIWorld::BotCheckWindup()`, filled by the adapter from `CheckWindupFor(false)`;
  `JamAICore.cpp`, the check press, one line: `WindupLeadM`). The pressers' timing lines that read the wind-up: `JamAICore.cpp` `WindupLeadM` (the bot's own
  length); `JamPlaytester.cpp` the human's press reads `W.CheckWindupFrames` (his own, the shared one: unchanged).
- Test: `Jam.Core.Contact.CrossHold` section 5: with 4 / 4 shared and bots 8 / 10, the human's press is told 4 and opens on frame 4, a bot's is told 8 and
  opens on frame 8, a bot's cross-check shove is told 10 and lands on frame 10; the online guest (SeatGuest at frame 0) keeps 4; with the keys absent a bot
  waits the shared length.

## 2. The table (item 2)

600 paired seeds a cell (1..600), the ladder's instrument (`JAM_PLAYTESTER_CHECK=closing`), regular rink, the three cells (novice at rookie / regular
at pro / good at all_star). The human keeps 4 / 4 in every arm. **The baseline first**: 4 / 4 as main = 73.7 / 45.2 / 30.7, hash-identical 600 / 600 a cell
to main's own binary (the curve lane re-sets the ladder on this commit: read the difference column). The paired difference's one sigma is about 2.8 points a
cell. "both N": `bot_check_windup_frames` N and `bot_cross_windup_frames` N; "check N" / "cross N": the one key. "dodge": the scripted human's answer
(below). "saw coming": an opponent within 5 m in front of him (inside 100 degrees of his facing) starts a body check's wind-up closing on him at 1 m/s or
more, or a cross-check's aimed at him; "got out": that man's hit did not land on him (stumble or fall).

| arm | win % (r / p / a) | vs base, paired | human knockdowns a match | bots' landed checks a match | opponents' hits on him | hits he saw coming | of those he got out of, % |
|---|---|---|---|---|---|---|---|
| 4 / 4 (as main), baseline | 73.7 / 45.2 / 30.7 | +0.0 / +0.0 / +0.0 | 3.19 / 7.37 / 7.96 | 6.27 / 11.81 / 12.82 | 3.50 / 8.30 / 8.62 | 9.0 / 16.4 / 17.3 | 61.8 / 53.4 / 56.1 |
| both 6 | 74.0 / 48.0 / 30.5 | +0.3 / +2.8 / -0.2 | 3.15 / 6.84 / 7.50 | 5.91 / 11.40 / 13.04 | 3.52 / 7.85 / 8.21 | 10.5 / 19.1 / 20.4 | 67.5 / 62.7 / 66.1 |
| both 8 | 73.8 / 52.7 / 34.3 | +0.2 / +7.5 / +3.7 | 2.95 / 6.26 / 6.95 | 5.34 / 10.78 / 12.01 | 3.30 / 7.28 / 7.70 | 11.2 / 21.0 / 23.1 | 71.5 / 68.9 / 72.3 |
| both 10 | 73.5 / 55.5 / 34.3 | -0.2 / +10.3 / +3.7 | 2.73 / 5.18 / 6.26 | 5.08 / 9.20 / 10.57 | 3.13 / 6.18 / 7.02 | 12.2 / 22.1 / 25.5 | 75.1 / 75.4 / 77.4 |
| both 12 | 72.2 / 52.0 / 34.5 | -1.5 / +6.8 / +3.8 | 2.45 / 4.38 / 5.14 | 4.61 / 7.93 / 8.80 | 2.79 / 5.37 / 5.92 | 13.2 / 24.4 / 26.7 | 79.7 / 81.2 / 81.7 |
| check 8 | 73.3 / 51.0 / 34.7 | -0.3 / +5.8 / +4.0 | 3.00 / 6.25 / 6.93 | 5.41 / 10.76 / 11.94 | 3.35 / 7.29 / 7.69 | 11.3 / 21.1 / 23.0 | 71.2 / 69.1 / 72.2 |
| check 12 | 71.7 / 51.2 / 33.7 | -2.0 / +6.0 / +3.0 | 2.45 / 4.39 / 5.11 | 4.60 / 7.89 / 8.78 | 2.78 / 5.46 / 5.91 | 13.2 / 24.4 / 26.6 | 79.7 / 81.0 / 81.7 |
| cross 8 | 73.2 / 45.5 / 29.8 | -0.5 / +0.3 / -0.8 | 3.19 / 7.53 / 7.87 | 6.24 / 12.03 / 12.68 | 3.49 / 8.46 / 8.49 | 8.9 / 16.6 / 17.1 | 61.7 / 53.0 / 56.3 |
| cross 12 | 72.2 / 46.0 / 29.5 | -1.5 / +0.8 / -1.2 | 3.22 / 7.45 / 7.84 | 6.31 / 11.89 / 12.63 | 3.53 / 8.35 / 8.46 | 9.0 / 16.5 / 17.1 | 61.7 / 53.3 / 56.3 |
| 4 / 4 + dodge | 69.5 / 46.0 / 28.7 | -4.2 / +0.8 / -2.0 | 3.44 / 7.60 / 8.14 | 6.69 / 12.13 / 12.91 | 3.74 / 8.49 / 8.80 | 9.5 / 16.7 / 17.6 | 61.6 / 52.8 / 55.8 |
| both 6 + dodge | 72.2 / 47.8 / 30.0 | -1.5 / +2.7 / -0.7 | 3.15 / 6.83 / 7.21 | 5.88 / 11.44 / 12.57 | 3.50 / 7.85 / 7.91 | 10.6 / 18.9 / 19.7 | 67.9 / 62.3 / 67.0 |
| both 8 + dodge | 73.7 / 46.7 / 30.2 | +0.0 / +1.5 / -0.5 | 3.17 / 5.91 / 6.76 | 5.64 / 10.43 / 11.94 | 3.52 / 6.91 / 7.47 | 11.8 / 20.5 / 22.8 | 70.9 / 70.1 / 72.7 |
| both 10 + dodge | 74.5 / 48.7 / 30.7 | +0.8 / +3.5 / +0.0 | 2.79 / 5.17 / 6.21 | 5.12 / 9.16 / 10.71 | 3.17 / 6.12 / 7.02 | 12.6 / 22.4 / 25.8 | 75.5 / 76.2 / 77.9 |
| both 12 + dodge | 73.7 / 48.3 / 32.2 | +0.0 / +3.2 / +1.5 | 2.48 / 4.18 / 5.26 | 4.70 / 7.75 / 8.93 | 2.83 / 5.17 / 6.04 | 13.5 / 25.0 / 27.0 | 79.9 / 82.3 / 81.8 |


- **The cost is the body check's.** Check 8: -0.3 / +5.8 / +4.0; check 12: -2.0 / +6.0 / +3.0. At pro the human's knockdowns fall 7.37 -> 6.25 -> 4.39 a
  match and the bots' landed checks 11.8 -> 10.8 -> 7.9: the bot presses the wind-up's ground earlier (the brain's lead grows with it) and a carrier who
  keeps moving is not where the window opens.
- **The bots' cross-check wind-up is nearly free**: cross 8 and 12 within 1.5 points a cell, 80 to 94 % of the seeds hash-identical: a bot's cross-check
  shove lands 0.1 to 0.2 times a match (the strip is the press's, unchanged).
- **The dodge** (`JAM_PLAYTESTER_DODGE=on`, off by default, in the instrument): six frames after a bot's wind-up he can see begins, the scripted human
  flicks the stick across the man's line and away from him (past the analog ramp; the first version, a 75-degree turn through the ramp or a brake, read
  the same) and drops his check press until the hit's window has passed. **It is worth nothing**: the got-out share moves by 0 to 1 point at every length
  (pro 53.4 -> 52.8 at 4, 68.9 -> 70.1 at 8, 81.2 -> 82.3 at 12) and he loses 3 to 6 points of win at pro (he gives up his own play for it). In the 2 to 12
  frames left after six of reaction his body moves tens of centimetres (skating: no sidestep) and the bot's 6-frame window, bending to its man, still
  reaches him. The longer wind-up gets him out more often by itself (53 -> 69 -> 81 % at pro) because the bot presses from farther and misses. **With
  today's verbs a longer wind-up is a delay that weakens the bots' press, not a mechanic a person answers.** It becomes one with an answer verb (next
  evolution 1).
- **Shipped (the Director, 15:53)**: the bots' body check 8 (133 ms, the beat an eye reads), their cross-check 12; the human keeps 4 / 4. The price on this
  table: about -0.3 / +5.8 / +4.0 for the human (the check's; the cross-check's is noise): the curve lane's second pass buys pro and all_star back.


## 3. Law 3 with each arm (item 3)

`Bar.Curve.Law3` (200 seeds a row, the straight charge and the stop begun at 8 / 8.5 / 9 / 9.5 / 10 / 11 / 12 m; `tools/perf/results/a200/law3.sh`; the
bots' keys through `JAM_TEST_A151`), on build3 (free skaters x 1.15 as main):

| row (past the defender, of 200) | pro 4 / 4 | pro 8 / 12 (shipped) | all_star 4 / 4 | all_star 8 / 12 |
|---|---|---|---|---|
| stop at 8 m (the gate's row, 90 %) | 94.0 % | **94.0 %** | 99.5 % | **99.5 %** |
| 8.5 m | 99.5 | 99.5 | 99.5 | 99.5 |
| 9 m | 93.0 | 93.0 | 99.0 | 99.0 |
| 9.5 m | 83.5 | 83.5 | 93.0 | 93.0 |
| 10 m | 60.0 | 60.0 | 87.0 | 86.5 |
| 11 m | 37.0 | 35.0 | 50.0 | 46.5 |
| 12 m | 34.5 | 34.5 | 20.5 | 17.0 |
| straight charge (must be stopped) | 0 | **0** | 0 | **0** |

**The gate's rows hold** (pro 8 m 94.0 %, the straight charge stopped 200 of 200). The late-stop lines are 2 to 3.5 points harsher at 11 and 12 m with the
bots' longer check: the brain's lead grows with the wind-up and a carrier that stops too late is met by a window that opens where he has slowed.

**The brain switch** `FJamHockeySense::bWindupLeadNoBrake` (tune `opp.hk_windup_brake=1`, off): the press leads by the wind-up only while the carrier is not
braking (`FJamAISkater::bBraking`, the sim's read-out). **Every line is identical with it** at 4 / 4 and at 8 / 12 (the defender's presses at pro 11 m:
154 -> 149). The late presses come when the carrier has already slowed under 1 m/s, where the sim no longer calls it braking: the lead is not what makes
those lines harsher; the window's place is. It stays off and in the code as a tune key (cheap to remove).


## 4. Tests at other lengths (item 4)

Every test evolution 10 re-staged for 4 frames now reads the length of the skater that presses, from the cartridge the test builds
(`Contact.CheckWindupFor(<presser>.bHuman)`), so another length needs no re-staging:

| test (file) | the presser | reads |
|---|---|---|
| `Jam.Core.TacklingEvents` (`JamTacklingTests.cpp`) | skater 0 | `CheckWindupFor(Skaters[0].bHuman)` |
| `Jam.Core.Stick` 5, "penalties off", 5b (`JamStickTests.cpp`) | the human | `CheckWindupFor(Skaters[Me].bHuman)` (three lines) |
| `Jam.Core.ModeKeys.PenaltyFrames` (`ModeKeysTests.cpp`) | the human | the same |
| `Jam.Core.SpinCheckedFalls` (`JamStickHandlingTests.cpp`) | the defender (a bot) | `CheckWindupFor(Ds.bHuman)` |
| `Jam.Bot.Hockey.BoxedIsOffIce` (`HockeyBrainTests.cpp`) | a bot slot driven by the test | `CheckWindupFor(Skaters[A].bHuman)` |
| `Jam.Core.Contact.LawOneDrill` (`JamContactDrill.h`) | the hit's and the nudge's attackers | each its own |
| `Jam.Fx.BusForB` (`BusForBTests.cpp`) | the charger | its own |
| acceptance drill step 8 (`HokejAcceptDrill.cpp`) | the opponent (a bot) | `CheckWindupFor(O.bHuman)`; step 7's cue is the human's (shared) |
| `Jam.Rink.PenaltySeat`, `Jam.Rink.HitSeenBoxUsed` (`RinkReactionTests.cpp`) | the human | the shared length (unchanged lines) |

The scenes that switch the wind-ups off (`JamContactTests.cpp`'s scene maker, its neutral cartridge, `Jam.Bot.PlayerBotWins` in `BotTests.cpp`) also set
the bots' keys to -1 (the shared 0).

**The gate at three settings** (build4; `tools/perf/results/a200/ue.sh "Jam" <name> [ENV=...]`): as shipped in a cleared environment (no `JAM_` variable)
`gate1`; the bots at 4 / 4 (`JAM_TEST_A151=bot_check_windup=4;bot_cross_windup=4`) `gateN`: **295 PASS 0 FAIL**; the bots at 8 / 8
(`JAM_TEST_A151=bot_check_windup=8;bot_cross_windup=8`) `gate88`. `gate1` and `gate88` each had 294 PASS and `Jam.Designer.CliTransportParsesResult` failed,
only because the three ran at once (the test writes a fixed temp folder and the three collided); alone it passes (`t3`). The shipped values moved one
test more, `Jam.Core.Abilities.Shield` (`JamAbilityTests.cpp`: its bot charger pressed from 1.4 m and the 8-frame window opened after the bodies met):
re-staged the same way (the charger starts V x its own wind-up / 60 farther out).


## 5. Device B's frames (item 5)

Device B's frames had not arrived when this was written; B is told 8 and 12. **The frames from call to contact as shipped**, for B's takes:
- **A bot's body check**: `PlayCheckWindup(8)` on the press; the window opens 8 frames later and lasts 6 (a bot's): the contact comes 8 to 14 frames after
  the call (the brain presses when the closing speed brings the bodies together about as the window opens). **The human's**: `PlayCheckWindup(4)`, contact
  4 to 16 frames after.
- **A bot's pressed cross-check**: `PlayCrossCheck(12)` on the press (the strip or the miss is on the press's frame); the shove lands **exactly 12 frames
  later** on its man if he is still within the shaft's reach. The human's: 4. The held cross-check has no wind-up (the pose is the wind-up).
When B names its frames, the arm that fits them is a row of section 2 (6, 8, 10, 12 measured for both keys and each alone).

## 6. The proof

- **Neutral**: with the bots' keys at the shared 4, 600 / 600 hash-identical to main's binary in each cell (arm `b4` against `main`), and the default
  match without any measuring variable 100 / 100 hash-identical a level (`n4plain` against `mainplain`, build4 against build0). The gate with the bots at
  4 / 4: 295 PASS 0 FAIL.
- **As shipped** (build4, the cartridge as it is: 8 / 12), regular rink, against 4 / 4: 73.3 / 53.3 / 33.8 = **-0.3 / +8.2 / +3.2**; the human's knockdowns at
  pro 7.37 -> 6.14, the bots' landed checks 11.81 -> 10.65. Large rink (`hockey.arena.large`): 4 / 4 80.5 / 56.5 / 26.7, shipped 85.5 / 58.0 / 37.7 =
  **+5.0 / +1.5 / +11.0** (all_star's knockdowns 9.85 -> 7.53).
- Full `Jam` filter green (above), `Jam.Net.HashCoverage` PASS (no new state field), test-guards PASS (0 faults, 201 files), schema tests 99 / 99, Shipping
  compile of the Game target PASS.
- Read-outs added to the playtest line (measurement): `seen_hits`, `seen_landed`, `dodges`, `bot_hits_on_human`. No skater or puck state field added.

## 7. The next three evolutions

1. **An answer to a hit seen coming** (what makes the long wind-up a mechanic): a sidestep on a tap (a short lateral step, about 0.6 m in 8 frames, with a
   price: no stick for 20 frames), or the brace taken as an answer. Proving number: with the dodge using it, the share of the seen hits he gets out of at
   pro rises by 10 points or more over no dodge at 8 / 12 (today +1), and the bots' wind-up's price at pro halves.
2. **The window where a stopping carrier is**: the bot's check aimed at the carrier's predicted place with his own deceleration (the brake switch showed
   the lead is not the cause). Proving numbers: law 3's late lines back to 4 / 4's (pro 11 m 37.0 %, all_star 11 m 50.0 %, 12 m 20.5 %) within 1 point, the
   gate's 8 m rows unchanged.
3. **The hit held a beat** (R-0064 point 5's other half) and B's frames: on a landed hit both figures hold 3 to 4 frames (presentation only, the sim does not
   wait), the keys set to B's take lengths. Proving numbers: the acceptance run's contact rows green on a package, the sim hash unchanged, the three cells
   within 2 points of section 2's row for B's lengths.
