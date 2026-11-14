# Contact 5: the hit lands, and it has a size (lane A132, track F1, evolution 4; 2026-11-14)

The first evolution of this track on the tree that carries the whole game (main `e44df14`: the body shield, the take grace, the brain's second evolution, the
quick standing start). Five things: the shove under the shields measured where the shields are; the press that lands (the scripted human presses on closing
speed; `game.contact.check_reach_m` measured with it); the hit's size on screen (the hold, the camera's kick, the bounce's own shape); brake read-outs that no
longer outlive a faceoff; and a drill that stages the vision's law 1 in one take (R-0054 point 3). Status with timestamps: `status/A132.md`. Raw: `Game/logs/a132*`
and `tools/perf/results/a132/raw/` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h` (`TryLift`'s count, `FaceoffReset`), `Public/JamCartridge.h`, `Private/JamCartridge.cpp`
(one code switch, one measurement seam), `Public/JamBaseline.h` (one line), `AI/JamPlaytester.cpp/.h` (the press), `Private/JamPawns.cpp`, `Public/JamPawns.h`
(the hold), `Game/Source/JamGame/Private/CameraFollow.cpp`, `Public/CameraFollow.h` (the kick), `JamCore/Public/JamContactDrill.h` (new: the drill),
`JamGame/Private/HokejGameMode.cpp` (drill 22), tests in `JamCore/Private/Tests/JamContactTests.cpp`, `JamGame/Private/ContactFeelTests.cpp`,
`JamCore/Tests/BotTests.cpp` (one budget).

## 0. What ships, in one table

| Rule | Ships | The three cells (novice at rookie / regular at pro / good at all_star; 600 seeds, paired) | Neutral value |
|---|---|---|---|
| The shove under the shields (`contact.bShieldShove`, lanes A125 / A138; already on main) | **on** | 98.0 / 66.7 / 38.0 with it against 98.7 / 65.8 / 35.3 without: **-0.7 / +0.9 / +2.7** | `bShieldShove` false (`JamBaseline.h`) |
| The scripted human presses its check on closing speed (`contact.bPlaytesterClosingPress`, a code switch, no key) | **on** | 98.2 / 68.0 / 36.0 against 98.0 / 66.7 / 38.0: **+0.2 / +1.3 / -2.0** | false (`JamBaseline::MakeNeutral`, one line) |
| The check finds its man (`game.contact.check_reach_m` 1.0) | **neutral (0)** | on top of the new press 98.3 / 71.8 / 42.5: **+0.1 / +3.8 / +6.5**; alone with the old press 97.3 / 72.3 / 41.5: **-0.7 / +5.6 / +3.5** | 0 (the default, unchanged) |
| The hit's size (hold 2 to 6 frames, kick 6 to 12 cm by the frames down, the bounce's shape) | **on** | presentation only: no match changes | `jam.hitstop 0`, `jam.hitkick 0` |
| Brake read-outs cleared at the faceoff | **on** | read-outs only: no hash moves | none needed |
| The law-1 drill (`jam.ctrl.Drill 22`) | **on** (where drills exist) | off unless called | the cvar at 0 |

The baseline of this tree on these seeds (1 to 600, one process a cell): **98.0 / 66.7 / 38.0** (`tools/perf/results/a132/raw/base-*.jsonl`; lane A138 on A53's
seeds: 98.3 / 71.7 / 37.5). **What a measurement of "the human" means from this commit on**: the playtester's press on closing speed moves the three cells to
**98.2 / 68.0 / 36.0** (before: 98.0 / 66.7 / 38.0): inside each cell's own 95 % interval (about 3.7 points either way at pro) and under the 4-point line.
Numbers the curve lane takes with the old press are comparable to within that.

## 1. The shove under the shields, on main (item 1)

**The four lines of `TryLift` after the merge** (lane A125's check list, `CONTACT-4.md` section 1), read on this tree: (1) `bStickRefused = IsLiftShielded(S, O) ||
bTestStickRefused` is taken once, before the contact; the lift's refusal reads it when the rule is on, and reads the shield again after the contact only in
the neutral arm (lane A138's switch `bShieldShove` false, as main did before lane A125); (2) the shove resolves for `Dot(Dir, O.Forward()) <= 0.5 || bStickOnly`;
(3) `IsShoveShielded` holds the spin's line and the sweep's three and nothing of track F2's; (4) the shove's guard reads `!bShoveRefused`. All four are in.

**Who shoves.** Lane A138 found that a bot which reads the shield presses no lift the stick cannot take, so the rule was proven staged only. In a match it
is the **human's** middle mouse button. `TryLift` now counts it (`UJamSim::ShieldShoveTally`: not state, never hashed, read by no rule; `[0]` a bot's presses,
`[1]` a human's). `Jam.Core.ContactShoveMatch`: 12 three-minute matches a level, the Regular playtester as the human, the whistle off (the test's cut), the rule
on against the refusal restored:

| level | the human's lift presses | at a stick-only shield (body shield or take grace) | became a shove | from behind | bump / stumble / down | a bot's presses at a stick-only shield |
|---|---|---|---|---|---|---|
| rookie | 203 | 49 (4.1 a match) | **37 = 76 %** (every one at arm's length) | 21 | 21 / 8 / 8 | **0** of 16 |
| pro | 250 | 66 (5.5 a match) | **45 = 68 %** | 22 | 26 / 7 / 12 | **0** of 146 |
| all_star | 219 | 39 (3.3 a match) | **19 = 49 %** | 9 | 12 / 4 / 3 | **0** of 129 |

- **A press at a shielded carrier is no longer nothing.** With the refusal restored the same presses (43 / 63 / 37 of them) did nothing at all. With the rule,
  three in four at rookie and half at all_star are a shove: 3.1 / 3.8 / 1.6 a match. More than half of the shoves are a bump (a shove from behind at matched
  speed is one since lane A138), and 8 / 12 / 3 in twelve matches put the carrier down.
- **The bots never shove through a shield**: 0 presses at a stick-only shield in 72 matches (the brain's shield read and lane A138's rule). The brief's
  fear (the human felled by routine shoves, as in lane A92's first model: 10 to 14 times in three minutes) cannot happen on this tree.
- **Knockdowns of the human by a cross-check, a match**: 0.00 / 0.08 / 0.00 with the rule, 0.00 / 0.00 / 0.00 without. **The bar (no more than once a match
  more) is met**: +0.00 / +0.08 / +0.00 (asserted by the test). A bot goes down by a cross-check 1.33 / 1.42 / 0.75 times a match against 0.50 / 0.33 / 0.58.
- **The three cells** (600 seeds a cell, the same seeds in both arms, one process a cell; the seam is `JAM_TEST_SHIELD_SHOVE=0` in the environment: every
  cartridge then loads with `contact.bShieldShove` false):

| cell | the rule on (the tree) | the refusal restored | the rule's effect | the human's check events a match | knockdowns of the human | the human's fouls from behind (the whistle is on in this cut) |
|---|---|---|---|---|---|---|
| novice at rookie | **98.0** (96.5-98.9) | 98.7 (97.4-99.3) | **-0.7** | 2.89 / 2.23 | 2.35 / 2.34 | 0.98 / 0.64 |
| regular at pro | **66.7** (62.8-70.3) | 65.8 (61.9-69.5) | **+0.9** | 2.66 / 1.89 | 7.40 / 7.36 | 0.68 / 0.39 |
| good at all_star | **38.0** (34.2-41.9) | 35.3 (31.6-39.2) | **+2.7** | 3.92 / 3.24 | 7.75 / 7.81 | 0.35 / 0.22 |

  No cell moves more than 4: **the rule ships on**, as it is on main. One cost is real while the whistle is on: a shove from behind that lands is the
  from-behind foul, and the scripted human takes 0.34 / 0.29 / 0.13 more of them a match (97 / 76 / 27 more frames in the box). It goes with the whistle
  (R-0054 point 5). What the cells do is reported, not tuned (the curve lane is setting them).

## 2. The press that lands (item 2): what was done, and the bar that is not met

**The playtester's line** (`Game/Source/JamCore/AI/JamPlaytester.cpp`, the body check in the forecheck): it pressed when its **own** speed along the line to
the carrier was over 4 m/s; it now presses when the **closing** speed along that line is (`(Me.Vel - C.Vel).Dot(N) > 4`): the contact law's own number.
The switch is `contact.bPlaytesterClosingPress` (true; a code switch, no key: it is the instrument, not a rule of the game; no sim code reads it), with its
neutral value in `JamBaseline::MakeNeutral` (false: the pinned matches were played by the old press), and `JAM_PLAYTESTER_CHECK=old` in the environment for a
measurement's before arm.

**The whiff share** (`Jam.Core.ContactReachMatch`: ten three-minute pro matches an arm, the Regular playtester):

| the press | `check_reach_m` | presses | whiffs | landed | whiffs with the carrier within 1 m and closing under 3 m/s on the press frame |
|---|---|---|---|---|---|
| own speed (before) | 0 | 113 | 66 = **58.4 %** | 28 = 24.8 % | 43 |
| **closing speed (ships)** | **0** | 122 | 60 = **49.2 %** | 41 = 33.6 % | 41 |
| own speed | 1.0 | 113 | 61 = 54.0 % | 41 = 36.3 % | 39 |
| closing speed | 1.0 | 117 | 60 = 51.3 % | 45 = 38.5 % | 44 |

- **The bar (under 40 %) is met by no arm.** The press on closing speed takes nine points off the whiffs and lands almost half as many checks again (28 to
  41 in ten matches; over 600 whole matches the regular at pro lands 2.95 a match against 2.66, the good at all_star 4.41 against 3.92); the reach on top of
  it adds landed checks and no fewer whiffs (51.3 %).
- **Lane A125's reading ("five whiffs in six come from that line") does not hold up.** If the old press whiffed because it fired in a chase, a press that
  needs 4 m/s of closing speed should fire far less often. It fires as often (113 and 122 presses), and read against the carrier itself on the press frame
  (`JAMREACH CARRIER`, new in this test; the nearest man is the carrier in all but 7 of 122 presses) 41 of its 60 whiffs still have the carrier within 1 m
  and the two closing at under 3 m/s; 10 have no opposing carrier any more, 7 the carrier beyond 1 m, 2 a faster closing. So the closing speed the
  playtester decides on is gone by the frame the sim opens the window. Not traced here; the two candidates are the bodies meeting before the press (the
  press is allowed from 1 m out and the collision without a check takes the closing speed) and the playtester's own late press.
- **What would meet the bar** is a press on the time to contact, earlier (the gap closing inside the 12-frame window: 1 m needs 5 m/s), which changes the
  instrument again and needs its own trace and its own three cells: the track's next evolution (section 9), cut here at the green point.

**`game.contact.check_reach_m` stays at 0.** Measured on this tree three ways (600 seeds, paired):

| pairing | novice at rookie | regular at pro | good at all_star | the human's landed checks a match |
|---|---|---|---|---|
| reach 1.0 against 0, the old press (build 1) | 97.3 against 98.0: -0.7 | 72.3 against 66.7: **+5.6** | 41.5 against 38.0: +3.5 | 3.48 / 3.69 / 4.73 against 2.89 / 2.66 / 3.92 |
| reach 1.0 against 0, the closing press | 98.3 against 98.2: +0.1 | 71.8 against 68.0: +3.8 | 42.5 against 36.0: **+6.5** | 3.05 / 4.03 / 5.41 against 2.57 / 2.95 / 4.41 |
| both against the tree as handed over | +0.3 | **+5.1** | **+4.5** | |

The reach is worth four to six points to a human who checks (on lane A125's own tree it was -1.6 / -3.7 / +3.2: the kept puck and the shields have made
the check the way to the puck). It is a lever for the curve, not a default: it ships neutral; the default and the schema are untouched.

**`Jam.Bot.HumanHasSomethingToDo`** (novice at rookie, seeds 1 to 20, a budget on the share of faceoffs without a human touch inside 600 frames): 4 of 100
with the closing press (6 of 108 before); over the 600 novice-at-rookie matches 4.44 % (151 of 3,399) against 5.54 % (196 of 3,541); with the reach at 1.0
6.61 % (old press, 255 of 3,860) and 5.88 % (closing press, 212 of 3,606). The budget **stays 9.3 %**, with the measurement and the reason written beside it
(`BotTests.cpp`): R-0039's rule (the share plus 2 points) would set 6.4 %, six faceoffs on a count of four: one standard error of a twenty-seed sample, a red
gate for any unrelated change to a rookie match on the night the curve lane moves rookie. The Director's to overrule: the line is `Faceoffs * 93 / 1000`.

## 3. The hit's size on screen (item 3)

Presentation only: the sim steps every frame whatever these say, lockstep never sees them, no hash moves.

| What | Before | Now | Where |
|---|---|---|---|
| The hold on a knockdown (at `jam.hitstop` 3) | 2 frames at ratio 1.0 to 5 at 3.0 and above | **2 at ratio 1.0 to 6 at 3.0 and above** (3 at 1.4, 4 at 2.0, 5 at 2.34) | `AJamSkater::HitStopFramesFor` |
| The hold on a stumble, a bounce, a bump | 1 or 2, 1, 0 | unchanged | |
| The camera's kick on a knockdown | 12 cm x max(0.5, the check event's strength): by the closing speed over top speed | **by the frames the victim lies: 6 cm at 60 frames to 12 cm at 150**, landed or taken | `UJamFollowCamera::DownKickScale`, `OnJamEvent` |
| The kick on a stumble | 3 to 6 cm by the ratio | unchanged | `StumbleKickScale` |
| The kick on the human's own bounce | 6 cm along the push, settling like a hit | **6 cm out, then back past rest once (about 1.5 cm behind it), then still** | `StartBounceKick`, `KickOffset(..., bBounce)` |

- **Why the frames down and not the event's strength.** The `check` event's strength is the closing speed over `skater.speed`. A cross-check to the chest of
  a standing skater closes slowly and floors it (the thrust ignores the standing brace): it kicked 6 cm, like a brush. The frames down are the contact law's
  own size of the hit (`down_frames_min` at `down_ratio` to `down_frames_max` at `down_full_ratio`), so the hold (by the ratio) and the kick (by the frames)
  now grow together: the biggest hit in the game holds three times as long and kicks twice as far as the smallest knockdown, and lies two and a half times as
  long. A knockdown the model did not resolve (`contact.momentum false`) keeps the event's strength and the base hold, as before.
- **A braced bounce reads as a bounce.** The player ran into a wall: the camera is thrown back along the push and swings forward past rest once before it
  settles: `(1 - t)^1.5 x cos(1.5 pi t)` over the kick's eight frames (cm along the push: 6.00 4.08 1.49 -0.58 -1.50 -1.35 -0.69 -0.15), where a hit of the same size only settles
  (6.00 4.91 3.90 2.96 2.12 1.38 0.75 0.27). The HUD line (BOUNCED OFF), the one-frame hold and the figure's `PlayStumble` with `recoil=1` are as they were.
- **Monotone.** A bump 0 / 0 cm; a stumble 1 to 2 frames / 3 to 6 cm; a knockdown 2 to 6 frames / 6 to 12 cm; never shorter or smaller for a harder hit
  (`Jam.Core.Contact.HitStop`: the sweep over ratio 0.4 to 5.0 and every whole number from 2 to 6 reached; `Jam.Camera.DownKick`: the sweep over 0 to 300
  frames down, the hardest stumble never above the shortest knockdown).
- **The switches.** `jam.hitstop 0` returns 0 for every outcome (the test's rows) and `jam.hitkick 0` is a scale of 0 in `KickOffset` and an early return
  in `OnJamEvent` / `ContactKickObserve` (the test's rows for a knockdown and for a bounce).
- Log lines: `JAMHITSTOP ... frames=` as before; `JAMKICK ... down_frames=` (new field: 0 for a stumble or an unresolved knockdown).

## 4. Brake read-outs do not outlive a faceoff (item 4)

`bBraking` / `bReverseBraking` are read-outs for the figure's stop pose, the spray and the sound (never hashed). `StepSkater` clears them at its start and
sets them again while the skater brakes, and every phase steps every skater, so in the sim a flag was at most one frame stale. But that one frame is the frame
a faceoff is set in, and it lasts as long as the sim is not stepped:

1. **The goal pause's last frame.** The pause holds the brake for everyone; a skater still gliding at more than 1 m/s "brakes" on that frame; `FaceoffReset`
   then puts it on its mark with no speed, in the same step. Readers saw a braking skater standing still on the dot (`HokejAcceptDrill.cpp:432` counts
   `bReverseBraking` a frame; the effects binder's spray reads `bBraking`).
2. **A restart** (`UJamSim::Restart`, a rematch from the menu) in the middle of a reverse brake: both flags stayed up until the first step of the new match,
   which the countdown or a menu can hold back for seconds.

`UJamSim::FaceoffReset` now clears both (one line, where the phase changes; `Restart` and the period's start go through it). `bReverseBrakeLast`, which is
state and hashed, is left as the step wrote it: no hash moves (the gate's hash lines are the baseline's). `Jam.Core.Contact.BrakeFlagsAtFaceoff`: a goal
with `rules.goal_pause_frames 3` (the attacker still gliding fast when the pause ends: the flag was up on 2 pause frames, 0 on the frame the
faceoff is set and 0 through the countdown), and a restart on the frame of a reverse brake (both flags down after it).

## 5. The law-1 drill (`jam.ctrl.Drill 22`; R-0054 point 3)

`docs/VISION.md` law 1: "If tackle is done between players going to same direction with similar speed, the impact is almost none next to a tackle or
cross-check from player facing full speed towards the player." One console command stages both hits side by side, with the same press on the same frame:

    jam.ctrl.Drill 22          three rounds, then the drill switches itself off (JAMDRILL law1 stage / result / summary lines)
    jam.ctrl.LawHuman 0 | 1    0 (default): the human throws the nudge, the camera rides the near lane and the hit lands 5 m beside it;
                               1: the human throws the hit (its six-frame hold and 12 cm kick show)

| Lane | Who | What is pressed | What happens |
|---|---|---|---|
| near (z -2.5 m) | the attacker 0.86 m behind an opponent, both at 8 m/s the same way | check, on the press frame | **a bump** (ratio 0.35, under 0.4): both skate on (the victim 8.00 m/s before, 8.00 m/s half a second later), no line, no hold |
| far (z +2.5 m) | a teammate at 8 m/s, an opponent coming at 8 m/s the other way (closing 16.0 m/s) | check, on the same frame | **down** for 150 frames (ratio 3.26), one frame after the nudge |

- The scene is `FJamLawOneDrill` (`Game/Source/JamCore/Public/JamContactDrill.h`, header only, no sim state), the code `Jam.Core.Contact.LawOneDrill` runs
  headless in four cases (the whistle on and off in the cut, the human on either lane): the gate holds the take. Everyone else is parked in a corner with no
  input and the puck lies in the other corner.
- The four skate for a second first (the press comes at frame 59), so the take has a lead-in; a round is 170 frames after the press (the longest
  knockdown and the get-up), then 90 frames before the next.
- **The whistle is off for a round** (`rules.penalties` through `ApplyLiveConfig`) and put back after it, also when the drill is switched off by hand in the
  middle of one: a check on a skater without the puck is a foul where the whistle is on, and the foul's faceoff would end the take. With the whistle taken out
  of the regular cut (R-0054 point 5) this does nothing.
- **Two things the scene does by hand on the press frame**, so that every take is the same: the press comes when the far lane's two are two frames from
  touching (the hit lands inside the bot's six-frame check window), and the near attacker is seated against its opponent's back at that opponent's own
  velocity (0.78 m between the centres: a move of 8 cm at 8 m/s, less than one frame's travel), because two bodies at matched speed never touch by
  themselves. Nothing else is placed: the outcomes are the contact law's.
- For the video (R-0054 point 4): drills exist only in a Development package; this take is Development footage and goes beside other Development takes.

## 6. The tests

| Test | What it holds |
|---|---|
| `Jam.Core.ContactShoveMatch` (new) | section 1's table; asserts the bar (the human down by a cross-check no more than once a match more with the rule) and that the neutral arm shoves through no stick-only shield |
| `Jam.Core.ContactReachMatch` | four arms (reach 0 / 1.0, the press on own / closing speed), ten seeds; the three pins of evolution 2 through `JamBaseline` hold; asserts the closing press whiffs less than the old one; logs the bar (`JAMREACH BAR ... NOT met`) and the whiff's reason against the carrier (`JAMREACH CARRIER`) |
| `Jam.Core.Contact.HitStop`, `.HitStopPick` | the hold's table 2 to 6, monotone over ratio 0.4 to 5.0, every whole number from 2 to 6 reached; `jam.hitstop 0` off |
| `Jam.Camera.DownKick` (new) | 6 / 9 / 12 cm at 60 / 105 / 150 frames, clamped outside, monotone over 0 to 300 frames, never under the hardest stumble; the bounce crosses rest once and is still by its last frame; a scale of 0 (`jam.hitkick 0`) gives nothing |
| `Jam.Core.Contact.BrakeFlagsAtFaceoff` (new) | section 4's two cases (green with the line; that it is red without it is read from the code, not run: no build was spent on it) |
| `Jam.Core.Contact.LawOneDrill` (new) | section 5's take, four cases (the whistle on / off in the cut, the human on either lane): a bump under `bump_ratio` with both on their feet, a knockdown for `down_frames_max`, inside six frames of each other, no foul, the whistle put back |
| `Jam.Core.ContactNeutral` | one line added to its own neutral list (`bPlaytesterClosingPress = false`): it names its neutral values itself, not through `JamBaseline` (red on build 2 for that reason) |

## 7. The gate, and the hash before and after

- **Before** (build 1, main `e44df14` as handed over): the full `Jam` filter **207 PASS, 0 FAIL** (`Game/logs/a132-tests-base-ue.log`, 167 s).
- **After** (build 3, the tree as it lands): the full `Jam` filter **211 PASS, 0 FAIL** (`Game/logs/a132-tests-full3-ue.log`, 137 s): the four new tests are
  the difference. Law 3's gate `Jam.Core.Movement.StopAndGo` green; `Jam.Net.HashCoverage` green (no state field added).
- **`Jam.Core.Determinism`** (seed 1, frame 600): hockey.regular `ef34dd88c7c62b47` before and after.
- **Every pinned hash holds, none re-pinned**: `ContactBoardMatch`, `ContactGraceMatch`, `ContactReachMatch`, `Jam.Bot.Hockey2.Neutral`, `MovementNeutral` (through
  `JamBaseline::MakeNeutral`, which now names the old press) and `ContactNeutral` (its own list, one line added).
- **The gate's hash lines**: 156 lines carry a hash in the baseline's log; 102 are identical after and one more differs only in its timing (`Determinism`).
  The other 53 are measurement lines of matches the scripted human plays **on the defaults** (the measurement arms of `ContactMatch`, `ContactGraceMatch`, `ContactBoardMatch`, `ContactReachMatch`, `TakeGrace`, "the tree as it stands" in
  `Hockey2.Neutral`, two of `MatchAlwaysEnds`, one of `BundleKeepsRegularHash`): they move because the instrument's default moved, which is the change.
- **At match level**: with `JAM_PLAYTESTER_CHECK=old` this tree replays the baseline's matches: seeds 1 to 100 of each of the three cells, **300 of 300** with
  the same hash and the same last frame as build 1 (`raw/base2-*.jsonl` against `raw/base-*.jsonl`).
- **A live headless game process** (`tools/commentary/match.ps1 -Cvars jam.ctrl.Drill=22`, `-game -nullrhi -Multiprocess`, logs under
  `tools/commentary/out/a132-drill*/`): three `JAMDRILL law1 result ... verdict=pass` lines and `summary rounds=3 pass=3`, with the human on the nudge and
  again on the hit (`jam.ctrl.LawHuman=1`); on the hit the log shows the new size: `JAMHITSTOP frame=151 frames=6 ... ratio=3.26` and `JAMKICK frame=151
  landed strength=1.00 ... cm=12.0 frames=8 scale=1.00 down_frames=150`. The whistle of this cut went off at each stage and came back after each round
  (`JAMSIM live config` lines at frames 90 / 320, 410 / 640, 730 / 960).
- Not run: the node tests of the gate and the premerge script's git steps (no git in this lane); no schema, cartridge or generated file was touched.

## 8. Neutral values and the revert path

- **Everything of this lane that changes a match**: `JamBaseline::MakeNeutral` (one new line, `G.Contact.bPlaytesterClosingPress = false`) with the tree's
  other neutral values is main's game at the vision's commit, hash for hash (the pins above). `check_reach_m` is 0 already; `bShieldShove` false is lane
  A138's line.
- The hit's size: `jam.hitstop 0` and `jam.hitkick 0` (both tested). To have the old size back: `Lerp(2.0, 5.0, ...)` in `AJamSkater::HitStopFramesFor`,
  `DownFrames = 0` in `UJamFollowCamera::OnJamEvent`, and `StartKick` for `StartBounceKick` in `ContactKickObserve`.
- The brake flags: the one line in `UJamSim::FaceoffReset`. The drill: off unless `jam.ctrl.Drill 22` is typed.
- **State fields added: none.** `UJamSim::ShieldShoveTally` is a member of the sim object (a count for tests), not of skater or puck state: nothing for
  `StateHash()`, nothing for `SkaterReadouts`.
- The two measurement seams are environment variables read once a process (`JAM_TEST_SHIELD_SHOVE=0`, `JAM_PLAYTESTER_CHECK=old`), as
  `JAM_PLAYTESTER_VERBS` is: never set in a game or in the gate.

## 9. Not proven here, findings, and what comes next

**Not proven here.**
- **The whiff bar** (section 2): 49.2 % against under 40 %.
- **How the hold, the kick and the bounce LOOK**, and drill 22 in a window: the cursor and the game windows are another lane's. Queued for the cursor's
  owner on a Development package of this tree: (1) `jam.ctrl.Drill 22` with `jam.ctrl.LawHuman 0` and then `1`: the lines of section 7 and a capture of each
  (the take for the video is `LawHuman 0`: the camera rides the nudge and the hit lands 5 m beside it); (2) the acceptance run's rows check / crosscheck /
  checked (the hold on the drill's check at ratio 2.34 is 5 frames now, was 4); (3) a capture of a braced bounce (`JAMKICK ... bounce`), which no drill
  stages yet.
- The figure's side of the size (a longer or harder `PlayKnockdown` for a bigger hit) is device B's: the pawn already sends the frames down.

**Findings outside this track's family.**
1. `tools/qa/ctrl/a92-tests.ps1:17`: the automation runner starts `UnrealEditor-Cmd` without `-Multiprocess` in this tree (the gate's own invocation in
   `tools/gate/premerge.ps1:106` has it). This lane ran a copy with the flag (`tools/qa/ctrl/a132-tests.ps1`).
2. `tools/perf/results/a101/paired.mjs:41` starts six `UnrealEditor-Cmd` at once and `tools/perf/results/a95/curve.mjs:21` eight workers by default: over the
   rule of three measurement processes. `tools/perf/results/a132/cells.mjs` runs one arm, three processes; `pair.mjs` pairs any two arms afterwards.
3. `Game/Content/Cartridges/hockey.regular.cartridge.json:150`: `"penalties": true`: the whistle is still on in the regular cut on this commit (R-0054 point 5
   is the curve lane's). With it on, the shove rule's from-behind fouls are in every number of the human (section 1).
4. `Game/Source/JamCore/AI/JamPlaytester.cpp:892` (`FormatPlaytestJson`): the `thirdjam.playtest/1` line has `checks_landed` and no count of presses or
   whiffs (`FJamSkaterState::ChecksMissed` exists), so the whiff share can only be measured in an automation test of a few matches, not over the 600-match
   cells. Two fields would do it.
5. `Game/Source/JamCore/Tests/BotTests.cpp:225`: R-0039's rule for this budget (the measured share plus 2 points) is one standard error of the test's
   twenty-seed sample; followed to the letter it would be red about one landing in six for reasons that are not its subject. Either the budget is the share
   plus two standard errors (about 8.5 % today) or the test plays more seeds.
6. `Game/Source/JamCore/Private/Tests/JamContactTests.cpp` (`Jam.Core.ContactNeutral`, in this family, fixed) was the last pinned test that names its
   neutral values by its own list (R-0055 point 7); it should call `JamBaseline::MakeNeutral` and keep only its older block, as `MovementNeutral` does.
7. Builds: build 2 of this lane (two shared headers touched: 92 actions) took 22 minutes, 5 waiting for one of the three slots and 17 compiling beside
   three other builds and fourteen editor processes (60.8 GB in use at 00:50, over the band, none of it this lane's); builds 1 and 3 took 3 and 4 minutes.

**The next three evolutions for this track.**
1. **The press that lands, traced.** First the trace (a line at the playtester's decision and at the sim's window: gap and closing speed on the carrier,
   frames between the two), then the press on the time to contact, earlier and from further out. Proving numbers: `Jam.Core.ContactReachMatch`'s whiff share
   **under 40 %** (49.2 % today, 58.4 % before), the row "carrier within 1 m, closing under 3 m/s" under 15 of 60 whiffs (41 today), every cell within 4 of
   98.2 / 68.0 / 36.0, and the two fields of finding 4 so the share is read over 600 matches.
2. **The reach as the curve's lever, not a default.** `check_reach_m` 1.0 lands the human 37 % more checks at pro (4.03 against 2.95 a match) and is worth
   +3.8 / +6.5 points at pro / all_star: hand it to the curve lane with section 2's table, or scale it by level (0.5 was 53.6 % whiffs on lane A125's tree).
   Proving numbers: the human's landed checks a match up by a quarter or more with every cell within 4 of the curve lane's targets after it re-sets them.
3. **The hit's size in the figure and in the acceptance run.** A row `law1` in the acceptance run that calls drill 22 (verdict from `JAMDRILL law1 summary
   pass=3`), a staged braced bounce (the one outcome no drill shows), and device B's knockdown take scaled by the frames down. Proving numbers: the acceptance
   rows green on a Development package; a still of the smallest knockdown (2 frames, 6 cm) beside the biggest (6 frames, 12 cm).

## 10. Files

- Sim: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h`, `Public/JamCartridge.h`, `Private/JamCartridge.cpp`, `Public/JamBaseline.h`.
- The scripted human: `Game/Source/JamCore/AI/JamPlaytester.cpp`, `AI/JamPlaytester.h`.
- The size: `Game/Source/JamCore/Private/JamPawns.cpp`, `Public/JamPawns.h`, `Game/Source/JamGame/Private/CameraFollow.cpp`, `Public/CameraFollow.h`.
- The drill: `Game/Source/JamCore/Public/JamContactDrill.h` (new), `Game/Source/JamGame/Private/HokejGameMode.cpp`.
- Tests: `Game/Source/JamCore/Private/Tests/JamContactTests.cpp`, `Game/Source/JamGame/Private/ContactFeelTests.cpp`, `Game/Source/JamCore/Tests/BotTests.cpp`
  (a comment beside the budget).
- Tools and data: `tools/perf/results/a132/cells.mjs`, `pair.mjs`, `share.mjs`, `run-arms.sh`, `cells.jsonl` (15 rows), `tools/qa/ctrl/a132-tests.ps1`.
- Docs: `docs/experiments/CONTACT-5.md`, `status/A132.md`.
- Not for landing: `Game/logs/a132*`, `Game/logs/build-ThirdJamEditor-*`, `tools/perf/results/a132/raw/`, `tools/commentary/out/a132-*`,
  `tools/playtest/out/hockey.regular.contact.check_reach_m_1.cartridge.json`.
