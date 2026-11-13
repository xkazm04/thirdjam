# Contact feel 3: the figure is told what happened, and the human's check finds its man (lane A125, track F1, device A, 2026-11-13)

Four things, in the brief's order: a shield refuses the stick and never the shove (R-0050 point 2); the figure is told a stumble apart from a bump, with the
frames the player has no control (`PlayStumble`); the figure is told the sim's reverse brake (`SetBraking`); and a human's pressed check bends onto the
collision course with the man in front of it (`game.contact.check_reach_m`). Worktree `.claude/worktrees/a101`, on top of evolutions 1 and 2 (`CONTACT-2.md`,
`CONTACT-3.md`).

**What ships on and what ships off.** Items 1 to 3 are on. Item 4 ships at its neutral value 0: it missed the brief's bar (the playtester's whiffs fall from
60 % to 51 % of its presses at 1.0; the bar was under 40 %) and at 1.0 one gate test of another track leaves its budget. What it buys at 1.0 is measured
(section 6); turning it on is one number.

## 1. The shield refuses the stick, never the shove (R-0050 point 2; the Director's order of 21:17)

Track F2 put two rules into `IsPokeShielded` on main (neither is in this worktree): the body shield (`game.stick.body_shield`: the carrier's body between the
defender's stick and the puck) and the take grace (`game.stick.take_grace_frames`: a skater that has just won the puck). `TryLift` asked `IsLiftShielded` (the
same answer) twice: once to refuse the cross-check's shove, once to refuse the lift. So on main a press on the middle mouse button at a carrier who turned its
back, or who had just won the puck, did nothing at all.

    stick refused   = IsLiftShielded(attacker, carrier)       read ONCE, before the contact; every shield anybody puts into IsPokeShielded
    shove refused   = IsShoveShielded(attacker, carrier)      the spin and the sweep, and nothing else (R-0040's bars, lane A92's third limit)
    stick only      = stick refused and not shove refused     on main: the body shield or the take grace; in a tree without them: never
    the shove       resolves when not shove refused, at arm's length, and (not from behind, or stick only)
    the lift        takes nothing when stick refused

- **The shove's guard reads one shield test, `IsShoveShielded`, and never `IsLiftShielded` / `IsPokeShielded`.** No shield a lane adds to `IsPokeShielded` can
  refuse a shove, whatever it is made of. The lift's answer is read by the shove's branch only to **widen** it (next point), never to refuse it.
- **From any side.** Lane A92 kept a lift from behind a pure stick lift ("no shove, no foul: how the brain steals a puck"). Where a stick-only shield refuses
  that lift, the limit has lost its reason, so the shove lands from behind too and obeys the contact law (a hit the victim cannot see finds it unbraced, x0.8).
  With the whistle on it is the from-behind foul it always was. With no shield up, a lift from behind is still the stick lift it was: the game before the lane.
- **The lift's answer is taken before the shove.** A carrier the shove drops loses the puck by the contact law (the knockdown's strip, or the stumble's loose
  puck from `loose_ratio`), never to the lift; reading the shield again after the knockdown (as the old second call did) would have read the geometry of a
  puck that is already loose.
- **The neutral values are the shields' own**: `stick.body_shield false` and `stick.take_grace_frames 0` make "stick only" false everywhere. This lane adds no
  key, and in this worktree "stick only" is never true: the baseline's hashes by construction.
- **The test seam.** `UJamSim::bTestStickRefused` (a plain member, not state, never hashed, false in every game) makes `TryLift` take the stick as refused, so
  `Jam.Core.Contact.ShieldShove` forces the predicate by hand in this tree.

**What the Director checks after the merge** (all in `Game/Source/JamCore/Private/JamSim.cpp`, `UJamSim::TryLift` and the function after `OutsideLiftCone`):

1. `const bool bStickRefused = IsLiftShielded(S, O) || bTestStickRefused;` is the **only** line of `TryLift` that names `IsLiftShielded` (grep: one hit inside
   the function). If the merge left a second one (main's line 1525 `... && !IsLiftShielded(S, O)` or line 1535 `if (IsLiftShielded(S, O) || O.ShieldFrames > 0)`),
   the rule is not in: the first must read `&& !bShoveRefused`, the second `if (bStickRefused || O.ShieldFrames > 0)`.
2. The shove's `ResolveContact` line reads `if (Dot(Dir, O.Forward()) <= 0.5 || bStickOnly)`.
3. `UJamSim::IsShoveShielded` holds the spin's line and the sweep's three and nothing of track F2's: no `IsBodyShielded`, no `TakeGraceLeft`. (It is a copy of
   `IsPokeShielded` as it stood before F2, on purpose: `IsPokeShielded` could not be split here without colliding with F2's lines in it.)
4. `Jam.Core.Contact.ShieldShove` on main: its case 6 (the seam off, straight from behind, the puck in front of the carrier) must log "a shove, the puck not
   taken (the tree's own body shield refused the stick)"; in this worktree it logs "a stick lift, no shove". Case 4b's second half (the seam off, from the
   front) shows what main's take grace does right after `HandPuck`: the same shove either way.
5. `Jam.Core.Contact.StandingBrace` case 5 ("a lift from behind: the puck is lifted ... nobody is shoved") already runs with `bBodyShield = false` on main
   (line 576): it stays green; with the shield on, that scene is now a shove.

## 2. `PlayStumble(DirX, DirZ, Frames)`: a stumble is told apart from a bump

Until now a bump and a stumble both reached the figure as `PlayHitReaction(DirX, DirZ)`. The pawn (`AJamSkater::ApplySimFrame`) now reads, on the frame the
sim's bump read-out fires, whether a hit began a stumble (`AJamSkater::StumbleFramesFor`: the victim's `ContactOutcome` is a stumble, or the skater is the
attacker that bounced, `bRecoilThisFrame`; and `StumbleFrames > 0`), and sends:

    a stumble began by a hit     PlayStumble(DirX, DirZ, Frames)  on a figure that has a function of that name; PlayHitReaction(DirX, DirZ) on any other
    a bump                       PlayHitReaction(DirX, DirZ)       as before

- **Frames** is `FJamSkaterState::StumbleFrames` after the step: the frames the sim still holds the skater without thrust, steering or stick, counted from the
  next sim frame. A body check resolves after the skaters moved: 16 (`game.contact.stagger_frames`). A cross-check resolves before they move: 15 (the frame of
  the hit was the first of the 16). The test steps the sim and finds the control back after exactly that many frames, both ways.
- **By name, not on the interface.** `AJamSkater::CallFigure(Figure, Name, Args)` looks the function up on the figure's class (`FindFunction`), checks that its
  parameters are exactly that many numbers or booleans, writes each by the parameter's own type (a Blueprint event's "float" is a double) and calls it. Nothing
  on `JamSkaterVisual.h` moves, so no figure has to recompile or re-implement anything, and a Blueprint that already declares an event of that name does not
  collide with an interface function (the v4 calls did: `contracts/skater-visual.md`, version history).
- **Log.** `JAMVIS stumble frame=.. skater=.. team=.. human=.. dir=(x, z) frames=.. by=.. ratio=.. recoil=.. call=PlayStumble|PlayHitReaction`, one per stumble.
  The `JAMVIS hit` line still goes out whenever `PlayHitReaction` was the call made.
- Not sent for a knockdown (`SetKnockedDown`) or the stumble a mistimed poke gives its own skater (`PlayPoke(false)` has it).

## 3. `SetBraking(bOn, Decel)`: the sim's reverse brake as a level

    on the change of (bReverseBraking and the skater is moving):   SetBraking(true, Decel)  /  SetBraking(false, 0)       by name, as above; a figure without it gets nothing

- **The condition is the sim's**: `FJamSkaterState::bReverseBraking`, set by `StepSkater` on the frames it applies `skater.reverse_brake_decel` (input more than
  120 degrees against a velocity over 1 m/s). Not the Ctrl brake, and nothing is read from the pawn's own motion.
- **Decel** (`AJamSkater::BrakeDecelFor`) is the speed the sim took off the skater on the frame the brake began, per second: `reverse_brake_decel` x the input's
  size, 20 m/s^2 by default (the test reads 20.00 on every braking frame). It is computed from two consecutive sim states, so this lane adds **no field to the
  skater state** (nothing for the hash, nothing for main's `Jam.Net.HashCoverage` list).
- **A stale flag is not a brake.** The read-out is written by the skater's own step; a faceoff or a period break leaves the last value standing. The pawn sends
  the level only while the sim's velocity is not zero.
- **Log.** `JAMVIS brake frame=.. skater=.. team=.. human=1 on=.. decel=.. speed=.. call=SetBraking|none`, for the human only (a bot reverses some two thousand
  times a match).

## 4. The check finds its man (`game.contact.check_reach_m`)

    while a HUMAN skater's check window is open (12 frames), every frame, before the skaters move:
      man      = the nearest standing opponent with  distance <= 2 radii + check_reach_m,  within 60 degrees of where the skater is going or of its facing
      across   = the man's speed across the line between the two, capped at sin 60 x the skater's speed
      velocity = (unit vector to the man) x sqrt(speed^2 - across^2)  +  (unit vector across) x across                 (UJamSim::CheckReachTarget, StepCheckReach)

- **A collision course, not a homing run.** The velocity takes the man's own speed across the line of sight (constant bearing) and puts the rest along it. A
  crossing man is led; a press that already is on the collision course is left as it was; a hit is never harder than the collision the two were on. The first
  version turned the velocity at the man's position: `Jam.Core.Contact.SprintFalls` (a skater at (5, 3.5) m/s beside a victim at (5, 0)) then knocked down the
  walking victim it should stumble, because all 6.1 m/s had become closing speed. That test is untouched and green again.
- **It bends, it does not add.** The speed is kept, so the hit's strength stays what the contact law makes of the closing speed plus the lunge the model already
  has; a skater standing still gains nothing. The position is never written: every step is the velocity times one frame.
- **The human's only** (`bHuman`, every human-controlled skater, as lane A49's 12-frame window is). The brain aims its own check: with the bend on every skater
  the bots landed a quarter more hits on the human (section 6).
- **Stateless.** The man is found again on every frame of the window: no field in the skater state, nothing to hash. `check_reach_m 0` returns before anything is
  read: the same state hash as before the lane.
- A carrier's press is refused before it gets here; part of the momentum model (`contact.momentum false`: off).

## 5. The tests (`Game/Source/JamCore/Private/Tests/JamContactTests.cpp`, four new)

| Test | What it proves |
|---|---|
| `Jam.Core.Contact.ShieldShove` | the stick refused by hand (`bTestStickRefused`): from behind a standing attacker's thrust stumbles the carrier with the law's ratio 0.625 (240 on an unbraced 384), the puck stays the carrier's, no clean lift, no stick-hit read-out; at 8 m/s: down, ratio 2.292, the puck leaves by the knockdown; penalties on: `foul_from_behind`; from the side and from the front (the take grace's shape): the same shove with and without the seam (0.556 and 0.500), only the take differs; the spin refuses both; the seam off from behind: the old stick lift in this tree |
| `Jam.Core.Contact.FigureCalls` | `CallFigure` on a probe with a Blueprint's parameter types (double, double, int32; bool, float); a wrong parameter count, a non-number parameter, a figure without the function and no figure are not called; `PlayStumble`'s frames: 16 for a body check, 15 for a cross-check, and the control is back after exactly that many steps; a bump 0, a knockdown 0, the attacker's own bounce 16 with the push against its travel; `SetBraking`: the level rises on the first frame of the reversed input, holds 30 frames from 10.6 m/s, Decel 20.00 on every one of them, 0 off |
| `Jam.Core.Contact.CheckReach` | reach 0: a man 1.3 m ahead and 1.1 m aside is a whiff; reach 1: the press turns 38 degrees and lands, no faster than the same press without the reach, every step the velocity times one frame, the hit no harder than the same speed straight on (1.04 against 1.21); outside the cone, beyond the reach, standing still, carrying the puck: nothing bends; a crossing man is led; a bot has no target; the key is 0 when absent |
| `Jam.Core.ContactReachMatch` | `check_reach_m 0`: pro seeds 1 / 2 / 3 hash as evolution 2's defaults did (asserted); six three-minute pro matches at reach 0 and 1.0: presses, whiffs, landed, why a press whiffed (a measurement) |

## 6. The numbers

**The hash.** `check_reach_m 0` (the default), pro seeds 1 / 2 / 3, three minutes: `7fb31c1f16b3f8c4` `a2be45743aebf331` `4f3c95b28136e412` = evolution 2's on its
defaults (asserted in `ContactReachMatch`). Items 1 to 3 change no state: item 1's "stick only" is never true in this tree, items 2 and 3 are the pawn's.

**The playtester's presses, six three-minute pro matches, five a side** (`JAMREACH TOTAL`, build 2: the human's only, collision course):

| reach | presses | whiffs | share | landed (a check event) | share | bot check events | knockdowns a match: human / bot | goals |
|---|---|---|---|---|---|---|---|---|
| 0 | 96 | 58 | **60.4 %** | 28 | 29.2 % | 56 | 5.00 / 5.00 | 9:4 |
| 0.5 | 84 | 45 | 53.6 % | 32 | 38.1 % | 67 | 5.67 / 6.50 | 11:7 |
| 1.0 | 80 | 41 | **51.2 %** | 33 | 41.2 % | 57 | 5.50 / 5.67 | 14:7 |
| 1.5 | 85 | 47 | 55.3 % | 30 | 35.3 % | 57 | 4.17 / 7.00 | 7:10 |

(The brief's "75 %: 30 whiffs against 10 landed" reads 60 % on these six seeds: 58 whiffs against 28 landed and 3 soft.) Ninety presses an arm: the arms differ
by less than their noise; the cells below are the measurement that carries weight.

**Why a press whiffs** (read at the press, reach 0, the 58 whiffs): the nearest man within 1 m and the two closing at under 3 m/s: **49**; the nearest man
beyond 1 m: 5; within 1 m and closing faster: 4. A gap of 1 m in a 12-frame window needs 5 m/s of closing speed: five whiffs in six are presses on a man the
playtester is not closing on (it presses on its own speed along the line, over 4 m/s, whatever the carrier does: `JamPlaytester.cpp:481`). The reach takes the
"closing faster" class to 0 and cannot touch the chase: it adds no speed, and under the contact law a hit at that closing speed is a bump anyway. **So the bar
(under 40 %) is not met and a bend of direction cannot meet it.**

**The first version, for the record** (build 1: every skater, the velocity turned at the man's position; reach 0 / 0.5 / 1.0 / 1.5 / 2.0): whiffs 60.4 / 60.0 /
47.7 / 50.6 / 50.7 %; check events by a bot 56 / 63 / 70 / 67 / 68; knockdowns of the human a match 5.00 / 5.67 / 6.67 / 6.33 / 6.00; goals 9:4 / 8:8 / 5:10 /
9:10 / 6:10. The bots gained more than the human, and `SprintFalls` went red (a harder hit than the collision): both are why the rule is the human's and a
collision course.

**The three target cells, paired, 600 seeds, reach 1.0 against 0** (`tools/perf/results/a101/cells.jsonl`, rows `reach1-vs-0`):

| cell | target | reach 1.0 | reach 0 | paired delta | off target at 1.0 | human's landed checks a match | knockdowns of the human a match |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85 | 83.7 (80.5-86.4) | 85.3 | -1.6 | -1.3 | 10.04 against 7.23 (+39 %) | 3.61 against 3.57 |
| regular at pro | 50 | 47.3 (43.4-51.3) | 51.0 | -3.7 | -2.7 | 8.66 against 5.87 (+48 %) | 8.02 against 8.10 |
| good at all_star | 25 | 28.5 (25.0-32.2) | 25.3 | +3.2 | +3.5 | 9.04 against 6.96 (+30 %) | 7.83 against 7.66 |

No cell is more than 4 points off its target at 1.0. The human lands a third to a half more checks a match and is knocked down as often as before.

**The gate at 1.0** (build 2): 167 PASS, 1 FAIL: `Jam.Bot.HumanHasSomethingToDo` (`BotTests.cpp:221`): "hockey.regular rookie novice, seeds 1-20: 19 of 172
faceoffs without a human touch inside 600 frames (budget 9.3 %: 15)". It runs on the defaults; the reach changes the novice's matches and the count leaves its
budget. That, with the missed bar, is why the key ships at 0.

**To turn it on** (the Director's or the curve track's): `FJamContactConfig::CheckReachM` in `JamCartridge.h` (1.0), the default column in `schema/DEFAULTS.md`
and `schema/README.md`, the row "check_reach_m is 0 (off) when the key is absent" in `Jam.Core.Contact.CheckReach`, the neutral arms of
`Jam.Core.ContactGraceMatch` and `ContactBoardMatch` (add `G.Contact.CheckReachM = 0.0;`), and `HumanHasSomethingToDo`'s budget or its cause.
`ContactReachMatch` sets the key itself and stays as it is.

## 7. A live game process (`-game -nullrhi -Multiprocess`, the game mode's scripted human)

`tools/commentary/match.ps1 -Cut hockey.regular -Name a125-live -Frames 5400` on build 2 (`tools/commentary/out/a125-live/game.log`, 5379 frames): 38
`JAMVIS brake` lines for the human (19 brakes, each `on=1 decel=20.0` then `on=0 decel=0.0`, `call=none`: the toy figure has no `SetBraking`, nothing is sent,
nothing breaks); 112 `JAMVIS hit`; no `JAMVIS stumble` (the 8 landed checks of that match were all knockdowns).

## 8. For device B, and the contract texts this lane proposes (the Director's to write)

**Device B**: declare the two as functions or Blueprint events on the figure (`BP_SkaterVisual_TB1` / `TB2`), with these names and inputs; number types are
free (a Blueprint float is fine); do **not** wait for an interface change and do not expect one: the pawn finds them by name. Until then nothing changes for
the figure: a stumble still arrives as `PlayHitReaction`.

`contracts/skater-visual.md`, two rows for the call table (proposed **v5**):

| Call | Parameters | Unit or range | Meaning |
|---|---|---|---|
| `PlayStumble` **(v5, by name)** | `float DirX`, `float DirZ`, `int32 Frames` | (DirX, DirZ) the push on this skater (unit, rink x / z), as `PlayHitReaction`'s; `Frames` = the sim frames the skater still has no thrust, steering or stick, counted from the next frame: `game.contact.stagger_frames` (16) for a body check, one less (15) for a cross-check, whose hit resolves before the skaters move | one-shot on the first frame of a stumble a hit caused: the victim of a check whose ratio fell in the stumble band, or the attacker that bounced off a braced skater (the push then points against its travel). Sent **instead of** `PlayHitReaction` on that frame; the control returns exactly `Frames` frames later (no call marks the end; a knockdown inside the stumble arrives as `SetKnockedDown`). Not sent for a bump (`PlayHitReaction`), a knockdown, or the stumble a mistimed poke gives its own skater (`PlayPoke(false)`). **Not on the interface**: the pawn calls any function or Blueprint event of this name with three numeric inputs on the visual object; a visual without one receives `PlayHitReaction(DirX, DirZ)` as before |
| `SetBraking` **(v5, by name)** | `bool bOn`, `float Decel` | `Decel` in m/s^2, 0 or more: the speed the sim took off the skater on the frame the brake began, per second (`game.skater.reverse_brake_decel` x the input's size: 20 by default); 0 when `bOn` is false | level, called on the change: true on the frames the sim applies the reverse brake (move input more than 120 degrees against a velocity over 1 m/s: the sim's `bReverseBraking`), false when it ends (the skater is still, the input turns, the skater goes down, a faceoff). Not the Ctrl brake. **Not on the interface**: called by name as `PlayStumble`; a visual without one receives nothing |

Version history line: "v5 (H26+, lane A125, from B's answers to A-to-B-0021 point 3 and A-to-B-0023 ask 4): `PlayStumble` and `SetBraking`, called by name from
`AJamSkater::ApplySimFrame` (`AJamSkater::CallFigure`), so the interface header and every existing visual are untouched; one `JAMVIS stumble` line per stumble
(`call=PlayStumble|PlayHitReaction`), one `JAMVIS brake` line per change for the human (`call=SetBraking|none`)."

`schema` row (in the worktree already): `game.contact.check_reach_m`, number 0..3, default 0.

## 9. Not proven here, findings, and what comes next

**Not proven here.**
- **Item 1's effect on the ladder.** Neither stick shield exists in this worktree, so the rule is proven by a forced predicate and never fires in a match
  here. On main every lift press of the brain at a carrier behind a body shield or inside a take grace, at arm's length, becomes a shove. Lane A92 measured what
  routine shoves do (the human down 10 to 14 times in three minutes when every lift shoved; `cross_reach_m 0.5` is what tamed it). The curve track measures the
  three cells and the human's knockdowns by cross-check on the merged main before a package is cut.
- **The figure's side.** No figure in this tree has `PlayStumble` or `SetBraking`: the live process shows the fallback (`call=PlayHitReaction`, `call=none`);
  the called path is proven on the probe object. Device B's Blueprint event with its double inputs is the same reflection path.
- **Item 5** (the hit's size on screen) was not started.
- The acceptance run (the cursor is another lane's): queued for the Director: rows check / crosscheck / checked on a Development package of main after the
  merge, and a capture of the middle mouse button pressed at a carrier's back.

**Findings outside this track's family.**
1. `Game/Source/JamCore/AI/JamPlaytester.cpp:479-485`: the playtester presses its check on its own speed along the line (`Me.Vel.Dot(N) > 4.0`), not on the
   closing speed. 49 of its 58 whiffs are presses on a carrier it is not closing on. `(Me.Vel - C.Vel).Dot(N)` is what a person learns in a minute, and it is
   the honest way under the whiff bar.
2. `Game/Source/JamCore/Tests/BotTests.cpp:221` (`Jam.Bot.HumanHasSomethingToDo`): the budget of 9.3 % of faceoffs without a human touch sits within four
   faceoffs of today's count (15 allowed; 19 with the reach on): any rule that changes the novice's matches at rookie can turn it red for a reason that is not
   its subject.
3. `Game/Source/JamCore/Private/JamSim.cpp:1195` (`StepSkater`) and `:677` (`FaceoffReset`): `bBraking` / `bReverseBraking` are cleared only inside the
   skater's own step, so they keep their last value through a faceoff or a period break. Readers that count the flag (`HokejAcceptDrill.cpp:432`, the effects
   binder's spray) can see a brake that is not happening; the pawn's `SetBraking` guards against it on its side.
4. The build tool's mutex: this lane's three builds took 973 s, 1,454 s and 796 s, of which about 9, 20 and 10 minutes were spent
   queued behind other lanes' `Build.bat`: more than a third of a 100-minute lane. `-Multiprocess` cured the headless starts; builds still take turns.

**The next three evolutions for this track.**
1. **The shove under the shields, measured where the shields are** (on main, after this lands): the human's knockdowns by cross-check a match and the three
   cells with the shove as built against `|| bStickOnly` removed (from behind stays a refused press). Proving numbers: every cell within 4 points; the human
   down by a cross-check no more than once a match more than today at pro (8.1 knockdowns a match in all).
2. **The hit's size on screen** (this brief's item 5): the hold 2 to 6 frames by the ratio and the camera's kick by the frames down. Proving numbers:
   `HitStopFramesFor` 2 at ratio 1.0 to 6 at 3.0 and above; the kick 6 cm at 60 frames down to 12 cm at 150; a capture of the smallest and the biggest.
3. **The press that lands**: `check_reach_m` 1.0 on, with the playtester pressing on closing speed (finding 1) and finding 2's budget re-measured. Proving
   numbers: the playtester's whiff share under 40 % (60 % today, 51 % with the reach alone), the human's landed checks a match up by a third or more (10.0 /
   8.7 / 9.0 against 7.2 / 5.9 / 7.0), every cell within 4.

## 10. Files

- Sim and key: `Game/Source/JamCore/Private/JamSim.cpp`, `Game/Source/JamCore/Public/JamSim.h`, `Game/Source/JamCore/Public/JamCartridge.h`,
  `Game/Source/JamCore/Private/JamCartridge.cpp`.
- The pawn's figure calls: `Game/Source/JamCore/Private/JamPawns.cpp`, `Game/Source/JamCore/Public/JamPawns.h`.
- Tests: `Game/Source/JamCore/Private/Tests/JamContactTests.cpp` (four tests inserted after `using namespace JamContactTests;`; no existing line edited).
- Schema: `schema/game.schema.json`, `schema/limits.json`, `schema/DEFAULTS.md`, `schema/README.md`; generated: `Game/Source/JamCore/Private/JamSchemaData.inl`,
  `Game/Source/JamDesigner/Private/JamDesignerData.inl` and what else `gen-designer-data` rewrites, `Game/Source/JamCore/Tools/parity.json`.
- Docs and data: `docs/experiments/CONTACT-4.md`, `status/A125.md`, `tools/perf/results/a101/cells.jsonl` (three rows `reach1-vs-0`).
- Not for landing: `Game/logs/a125-*`, `Game/logs/a92-tests-a125-*`, `tools/commentary/out/a125-*`, `tools/perf/results/a101/raw/`,
  `tools/playtest/out/hockey.regular.contact.check_reach_m_0.cartridge.json`, `devlog/out/fixture-shots/*` if the node tests rewrote them.

## 11. The final tree (build 3: `check_reach_m` default 0)

- **Full `Jam` filter: 168 PASS, 0 FAIL** (84 s with `-Multiprocess`; `Game/logs/a92-tests-a125-full2-ue.log`). Evolution 2 had 164: the four new tests are
  the difference.
- **Hashes**: every hash the gate logs (83 distinct lines: `Jam.Core.Determinism` `f3613b5fcd33ce32`, `ContactMatch` pro and all_star, the neutral blocks, the
  modes and takes) is equal to evolution 2's gate log (`a92-tests-a113-full3-ue.log`): none missing, none new. `ContactReachMatch` at reach 0: `7fb31c1f16b3f8c4`
  `a2be45743aebf331` `4f3c95b28136e412` (asserted).
- **A live process on the final tree** (`match.ps1 -Cvars jam.ctrl.Drill=1 -Name a125-drill`, 3191 frames): one stumble, sent as
  `JAMVIS stumble frame=1969 skater=2 team=0 human=0 dir=(-0.72, -0.69) frames=16 by=5 ratio=0.44 recoil=0 call=PlayHitReaction` (the toy figure has no
  `PlayStumble`: the fallback), 28 `JAMVIS brake` lines (`call=none`), 9 knockdowns, 8 hit-stops as before.
- The measurements of sections 6 and 7 were taken on build 2 (the same source with the default at 1.0, and the arms 0.5 and 1.5 in the match test); the cells
  set the key in both arms' cartridges or read the binary's default, so they stand for "1.0 against 0" on the final tree too.
