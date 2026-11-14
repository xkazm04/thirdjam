# Contact 6: the operator's hand finds the hit (lane A151, track F1, evolution 5; 2026-11-14)

On main `22214a2` (the whole game integrated, the ladder set). Three things: **why the scripted human whiffed half of its checks, traced frame by frame**
(its press reaches the sim six frames after it was decided, at a man already passed), with the script's fix and the sim's half of it (the late press);
**a standing skater that is braced** (from the side it holds and the one who ran into it pays; from the front it goes down); and **what the screen says
for the human's own hits** against the hits on him. Status with timestamps: `status/A151.md`. Raw: `Game/logs/a151*`, `tools/perf/results/a151/raw/`,
`tools/commentary/out/a151-*` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h` (`BraceOf`, the braced bounce in `CollideSkaters`, `TryLatePress` and its five state
fields), `Public/JamCartridge.h` (four rule values, none a key yet), `Private/JamCartridge.cpp` (one measurement seam), `Public/JamBaseline.h` (four neutral
lines), `AI/JamPlaytester.cpp/.h` (the press on course and time, `CheckPct`, two read-outs on the playtest line), tests in
`JamCore/Private/Tests/JamContactTests.cpp`, one line in `JamCore/Private/Tests/JamStickTests.cpp`, `JamNet/Private/Tests/JamNetGuestTests.cpp` (the
hash-coverage lists).

## 0. What ships, in one table

Cells: novice at rookie / regular at pro / good at all_star on `hockey.regular`, seeds 1 to 600, one process a cell, paired
(`tools/perf/results/a151/cells.mjs`, `pair.mjs`, rows in `cells.jsonl`). The tree as handed over (`base`): **87.2 / 52.8 / 28.2** (the curve's tables: 85 / 49 / 30).

| Rule | Ships | The three cells, paired | Neutral value |
|---|---|---|---|
| A standing skater is braced: the side brace (`contact.StandSideBrace` 5) and the brace as mass in the bounce (`contact.bBraceExchange`), not for the puck carrier | **on** | the final form against the handed-over tree: 87.0 / 54.3 / 30.3 = **-0.2 / +1.5 / +2.1**. Before the carrier was taken out: the side brace alone -0.9 / +0.5 / +0.5, the bounce alone -1.0 / -0.5 / +0.5, both +0.5 / -0.1 / +2.1 | 0 and false (`JamBaseline::MakeNeutral`) |
| The scripted human presses on course and time (`contact.bPlaytesterCoursePress`, `FJamPlaytesterProfile::CheckPct` 60; the instrument, no key) | **on** | on the final brace: 84.8 / 56.3 / 31.5 = **-2.2 / +2.0 / +1.2**. At 75 % of the chances: -3.0 / +5.6 / +1.5; at every chance: -1.9 / +3.7 / +5.3 | `bPlaytesterCoursePress` false (`MakeNeutral`) |
| The late press (`contact.LatePressFrames` 6) | **neutral (0)** | alone, with the press that arrives late (the old script): **-1.0 / +6.8 / +4.2**; on the new script +1.8 / -1.0 / +1.5 (that script no longer presses late: the instrument cannot see the rule) | 0 (`MakeNeutral`) |
| `check_presses`, `check_whiffs` on the playtest line | **on** | read-outs: 600 of 600 matches a cell hash as before | none needed |

**Everything that ships, against the tree as handed over: 84.8 / 56.3 / 31.5 = -2.4 / +3.5 / +3.3** (inside 4 in every cell). **Against the curve's
tables (85 / 49 / 30) the final tree reads -0.2 / +7.3 / +1.5**, where the handed-over tree read +2.2 / +3.8 / -1.8 on these seeds and this driver: pro was
already 3.8 over its table and is 7.3 over it now. That is the Director's call, with these facts: (1) a paired difference of two 600-seed arms whose
matches diverge has a standard error of about 2.6 points at pro (about 125 results flip each way), so +3.5 is one and a third of them; (2) the direction
that repeats across the arms is "more landed checks, a little more won at pro and all_star, a little less at rookie"; (3) the script's press changes
nothing a player feels (it is the instrument), and one line gives the old one back (`bPlaytesterCoursePress = false` in `JamCartridge.h`: the cells are
then the brace's -0.2 / +1.5 / +2.1); (4) the brace is two lines in the same file (`StandSideBrace = 0.0`, `bBraceExchange = false`).

**The bar of item 1** (whiffs under 40 % over 600 matches at pro, landed checks a match up, the cells inside 4): met with the script at 60 %.

| 600 matches a cell | whiff share (rookie / pro / all_star) | landed check events a match | presses a match |
|---|---|---|---|
| the press as handed over (closing over 4 m/s inside 1.8 m), the handed-over tree | 69.4 / **53.8** / 43.5 % | 3.28 / 3.22 / 3.92 | 9.95 / 10.56 / 8.13 |
| the same press on the final tree (the brace on) | 69.1 / 54.7 / 44.2 % | 3.31 / 3.19 / 3.68 | 10.00 / 10.59 / 7.96 |
| **on course and time, 60 % of the chances: the final tree** | 31.0 / **16.0** / 16.8 % | 4.29 / 4.35 / 4.13 | 3.10 / 3.37 / 3.34 |
| on course and time, every chance | 29.7 / 16.1 / 15.0 % | 5.17 / 5.55 / 5.86 | 4.91 / 5.23 / 5.74 |
| the old press with the late press 6 | 60.6 / 27.8 / 26.2 % | 4.29 / 7.45 / 5.91 | 9.35 / 10.22 / 7.57 |

## 1. The press that lands, traced (item 1)

### 1.1 The paragraph

**Why a press at a man within a metre does nothing.** In the sim a check is not a strike: the press opens a window (12 frames for a human, 6 for a bot)
and a hit exists only if, while it is open, the two bodies collide in the ordinary way (centres under 0.8 m, two radii). Nothing moves the presser: the
"lunge" of 1.5 m/s is a number added to the impulse, never a motion, and `check_reach_m` ships at 0. The scripted human decided its press on "centres under
1.8 m and closing at over 4 m/s along the line between us", which asks neither whether the course meets the body nor when; and **everything the playtester
decides reaches the sim `ReactionFrames` later** (`FJamPlaytester::Think`'s ring: 6 frames for the regular player, 12 for the novice, 3 for the good one).
At the mean closing speed of a whiffed press (11.3 m/s) the 0.83 m between the bodies is gone in 4.4 frames, so the press arrives after the moment it was
for: on the frame the sim got a whiffed press the man was 1.27 m away (centres) and the two were **separating at 4.8 m/s** (57 of 66 whiffs already
separating, 63 of 66 off course). Lane A132's read "within 1 m, closing under 3 m/s on the press frame" was this, seen from the wrong frame: not a chase, a
man already passed. By the state each press was **decided** on: 66 of 135 were off course from the start (the straight course passes the body: 41 whiffs),
54 were on course but the bodies met **before** the press arrived (22 whiffs; 9 landed on a second touch), and the 15 whose contact fell inside the
window as delivered landed 11 times. The window's length is not the fault (after a whiff the bodies met within 30 frames in 3 of 66 cases), nor is the cone
(nothing bends at reach 0). The fault is the script's, and it has a sim half that a person's hand shares: **a window that opens after the contact**. In
18 of 66 whiffs the bodies had met 1 to 6 frames before the press arrived: the press of somebody who presses as he feels the hit.

### 1.2 One whiff, frame by frame

`Jam.Core.ContactPressMatch` (`JAMPRESS WHIFF` lines; the first of pro seed 1, the tree's press), human = skater 0, the man = the opposing carrier:

```
decided 6 frames before, on: centres 1.68 m, closing 9.24 m/s along the line, 15.54 m/s across it, closest approach of the centres 1.45 m (contact under 0.80) => OFF COURSE
the sim got it on:           centres 1.55 m, closing -7.20 m/s (separating), 15.66 m/s across the line
+00 window 12 | human at (22.65, -13.08) vel (-13.11,  0.00) | man at (22.47, -14.74) vel (0.30, -10.50) | centres 1.68 m closing  -8.97 m/s
+01 window 11 | human at (22.43, -13.08) vel (-13.15, -0.53) | man at (22.48, -14.92) vel (0.43, -10.46) | centres 1.83 m closing -10.23 m/s
+03 window  9 | human at (22.00, -13.12) vel (-12.85, -1.42) | man at (22.50, -15.27) vel (0.73, -10.46) | centres 2.20 m closing -11.87 m/s
+06 window  6 | human at (21.39, -13.24) vel (-12.06, -2.68) | man at (22.55, -15.79) vel (1.21, -10.45) | centres 2.80 m closing -12.59 m/s
+11 window  1 | human at (20.47, -13.59) vel (-10.19, -5.07) | man at (22.69, -16.66) vel (2.07, -10.39) | centres 3.79 m closing -11.50 m/s
+12 window  0 | MISSED                                                                                    | centres 3.98 m
=> closest the centres came inside the window: 1.68 m, on its first frame; the bodies never met
```

The human crosses the carrier's path at 13 m/s chasing the PUCK (the playtester steers at the puck or 2.5 m goalside of the carrier, never at the body);
the carrier skates at 10.5 m/s across. When the script decided, the two were 1.68 m apart and closing at 9.2 m/s along the line, on a course that passes 1.45 m
from the body. Six frames later the press arrives; they have crossed and are flying apart. The window counts down on nobody.

### 1.3 The trace in numbers (`Jam.Core.ContactPressMatch`, ten three-minute pro matches; the mechanism, not the verdict)

Each press is read twice: on the state the playtester **decided** on (6 frames before the sim got it) and on the frame the sim **got** it.

| The press of lane A132: 135 presses | presses | landed | whiff | other (took the puck, no window left) |
|---|---|---|---|---|
| off course when decided (closest approach of the centres 0.8 m or more) | 66 | 10 | 41 | 15 |
| on course, but the bodies meet before the press arrives | 54 | 9 | 22 | 22 (+1 too soft) |
| on course, the bodies meet inside the window as delivered (frames 7 to 18) | 15 | 11 | 3 | 1 |
| all | 135 | 30 = 22 % (3.0 a match) | 66 = **48.9 %** | 38 |

The 66 whiffs: decided at centres 1.63 m, closing 11.3 m/s, closest approach 0.95 m (mean); when the sim got the press: centres 1.27 m, closing
**-4.8 m/s**, 57 already separating, 63 off course; off course when decided by 0.8 to 1.0 m 9, 1.0 to 1.3 m 12, 1.3 to 1.8 m 20; the bodies had met 1 to 6
frames before the press arrived in **18**; the bodies met after the window within 30 frames in 3.

## 2. The fixes (item 1)

### 2.1 The script's: the press on course and time (`contact.bPlaytesterCoursePress`, ships on)

`FJamPlaytester::Decide` now presses when the straight course of the two bodies brings the centres within two radii **between 1 and 7 frames after the
press will arrive** (`ReactionFrames` ahead: the first half of the 12-frame window; the second half is the margin for a man who slows), closing at over
4 m/s as before and not into his back. There is no distance gate any more: the time is the gate. It is what a person does who presses for the hit he sees
coming. In the ten matches, at every chance: 65 presses, **51 landed (78.5 %)**, 13 whiffs (20.0 %): every press was on course inside the window when
decided, and the 13 are men who turned or slowed inside the 13 to 19 frames between the decision and the contact. (The test as it lands plays the shipped
60 % of the chances: 42 presses, 34 landed = 81.0 %, 7 whiffs = 16.7 %.)

At every chance the script lands 5.2 to 5.9 checks a match where it landed 3.2 to 3.9, and the all_star cell moves 5.3 points. So the player takes a
**share** of the hits it sees coming: `FJamPlaytesterProfile::CheckPct` (per cent of 2-second windows, the idiom of `OneTimerPct`), **60** for every profile.
Over 600 matches a cell on the final tree: whiffs 31.0 / **16.0** / 16.8 % (69.1 / 54.7 / 44.2 with the old press), landed 4.29 / 4.35 / 4.13 a match
(3.31 / 3.19 / 3.68), cells -2.2 / +2.0 / +1.2 (75 % of the chances moved pro 5.6 points). Measurement seam: `JAM_PLAYTESTER_CHECK=old | closing | course | course:NN`.

**What a measurement of "the human" means from this commit on**: the instrument presses a third as often and lands a third more. Numbers the curve lane
took with the press of lane A132 are comparable to within the deltas above; `JAM_PLAYTESTER_CHECK=closing` gives that press back for a paired arm.

### 2.2 The sim's: the late press (`contact.LatePressFrames`, built and proven, ships at its neutral value 0)

A check is a window that must be open **before** the bodies meet. A hand that presses as it feels the contact is a frame or six late; the bounce has
already parted the two (they leave each other at a fifth of the closing speed) and the press runs out on nobody: MISSED, with the man inside a metre.
That is the 18 of 66. The rule: for `LatePressFrames` frames after a **human** skater's body met an opponent's with no check open, its check press
resolves as that hit (`UJamSim::TryLatePress`): `ResolveContact` on the velocities the two met with (so the closing speed, the brace and the attacker's own
stability are those of the meeting), then the bounce of that meeting; a bump leaves today's velocities alone. The man must still be up, unshielded and
within 0.7 m of touching; the carrier still cannot hit; a bot gets none (its press is its brain's, as with the human's 12-frame window). A weaker touch of
the same man inside the frames does not replace the meeting. State: `LateFrames`, `LateWho`, `LateDir`, `LateVelSelf`, `LateVelOther` on the skater,
hashed in `StateHash()` while `LateFrames` runs (`Jam.Net.HashCoverage` green); `ChecksLate` is a read-out counter (listed).

`Jam.Core.Contact.LatePress` (a standing skater met head-on at 9 m/s): pressed in time: down, ratio 1.166, 67 frames; pressed 1, 3 and 6 frames after the
bodies met: the same outcome and ratio on the press's own frame, a check event, no whiff; 7 frames after: a window and a whiff; at 0: a whiff; a bot's
late press: nothing.

**Why it ships neutral.** With the press that arrives late (the old script is exactly that hand) it turns whiffs into hits wholesale: 600 matches a
cell, whiffs 54.3 -> 27.8 % at pro, landed 3.30 -> **7.45** a match, cells **-1.0 / +6.8 / +4.2**. With the script's new press it does nothing a
measurement can see (+1.8 / -1.0 / +1.5, 506 of 600 pro matches hash-identical), because that script no longer presses late. So the instrument cannot
clear the rule for a real hand: how often a person presses late is the operator's to say at 10:00. Two facts for that decision: (1) a check press has no
cooldown, so a hand that taps RMB every 12 frames already has the window open at every contact: the late press gives one deliberate press what mashing
gives, no more; (2) it also stands a rusher up: a human run into by a bot may press inside 6 frames and the hit resolves on the closing speed the bot
brought, exactly as an open window does today. One line turns it on: `int32 LatePressFrames = 6;` in `JamCartridge.h` (proposed key
`game.contact.late_press_frames`, 0..12).

### 2.3 What was not the fault

- **The window's length** (12 frames): after a whiff the bodies met within 30 frames of the press in 3 of 66 cases.
- **The cone and the reach** (`check_reach_m`, ships 0): it bends a press that is still closing; 57 of 66 whiffed presses arrived after the two had parted.
- **The lunge** (1.5 m/s): it is a term of the impulse, not a motion; nothing in the sim moves a presser toward the man. Left as it is: a lunge that moves
  the body is a new verb (the track's next evolution, with its own cells).

## 3. A standing skater is braced: not a wall, not a rocket (item 2)

### 3.1 Why it left at 6.6 m/s

Two separate things, both read off the code and then staged (`Jam.Core.Contact.StandingSides`, the `JAMSTAND` lines).

1. **The bounce ignored the law.** `ResolveContact` decides bump, stumble or down, and on a bump moves no speed. But `CollideSkaters` then runs its
   exchange on every touching pair, pressed or not: an equal-mass bounce with restitution 0.2, which hands the struck skater **0.6 x the closing speed**
   whatever the contact law just said. Standing, struck at 9 m/s: 5.39 m/s (staged). At 11 m/s: 6.6 m/s, lane A147's number. The brace existed only in the
   outcome, never in the motion.
2. **And by the stability table a standing skater did not resist a side tackle at all.** `stand_brace` 1.5 covers every hit the skater sees, side and
   front alike: a pressed side hit closing at 9 m/s is 80 x (9 + 1.5) / (80 x 6 x 1.5) = **1.17: down**, the same as from the front. The operator's law
   ("Stationary player can resist most of side tackles and fouls, logically not crosschecks directly to face") had no number of its own.

### 3.2 The rule

- **`contact.StandSideBrace` (5; 0 = neutral, the side takes `stand_brace`)**: the brace of a skater under `stand_speed_mps` against a **body** hit from
  the side (the push across its facing, and behind that up to the unseen arc). It falls off linearly to `stand_brace` at the edge of the front arc (the push
  60 degrees off head-on), so there is no cliff between "side" and "front" and the front stays the weak side of a man who stands. A cross-check keeps
  `stand_brace` from the side and ignores the brace to the chest, as before. `UJamSim::BraceOf` is that factor alone; `StabilityOf` multiplies it in (the
  same numbers bit for bit at the neutral value).
- **`contact.bBraceExchange` (true; false = neutral, the equal bounce)**: the brace is mass in the bounce. Between opponents, a skater on its feet that
  stands braced against the push weighs its brace in `CollideSkaters`' exchange; a skater that is down, stumbling (this hit may just have stumbled it),
  moving or carrying the puck weighs 1. With both weights 1 the old line runs, bit for bit.
- **Neither is for the puck carrier** (section 3.4: law 3 is a gate).

Neither is a cartridge key yet (proposed: `game.contact.stand_side_brace` 0..10, `game.contact.brace_exchange` true|false); both have their neutral value
in `JamBaseline::MakeNeutral`, and the measurement seam `JAM_TEST_A151="side_brace=5;brace_exchange=1"` sets them at cartridge load.

### 3.3 The three cases, staged (the attacker glides at the standing skater at 9 m/s; `JAMSTAND`)

| Case | Before | With the rule |
|---|---|---|
| from the side, pressed | **DOWN**, ratio 1.17, 67 frames; leaves at 5.24 m/s; the attacker keeps 3.75 | **BUMP**, ratio 0.35; the standing skater leaves at **0.90 m/s**, on its feet; **the attacker recoils**: stumbles 16 frames and stops (0.00 m/s) |
| from the front, pressed | DOWN, ratio 1.17, 67 frames; leaves at 5.24 m/s | the same: DOWN, ratio 1.17, 67 frames (the front keeps `stand_brace`) |
| from behind, pressed | DOWN, ratio 2.19, 113 frames | the same (unseen: no brace either way) |
| from the side, no press | no hit; the standing skater leaves at **5.39 m/s**; the attacker keeps 3.60 | no hit; the standing skater leaves at **1.80 m/s**; the attacker stops (0.00) |
| from the front, no press | 5.39 m/s; the attacker keeps 3.60 | 4.32 m/s; the attacker keeps 2.52 |
| from behind, no press | 5.39 m/s | 5.39 m/s (the equal bounce: he did not see it) |
| from the side at top speed (11 m/s), pressed | DOWN, ratio 1.39, 77 frames | **STUMBLE**, ratio 0.42 (16 frames); the attacker recoils too: not a wall |
| 15 degrees ahead of the side, pressed, 9 m/s | DOWN, ratio 1.17 | STUMBLE, ratio 0.55 (the brace falling off toward the front) |
| from the front, pressed, 4 m/s | STUMBLE, ratio 0.61 | the same |
| from the side, pressed, 4 m/s | STUMBLE, ratio 0.61 | BUMP, ratio 0.18; leaves at 0.80 m/s |

**The number the brief asked for**: from the side at 9 m/s closing a standing skater takes a bump and leaves it at **0.90 m/s** when the hit was pressed
(the attacker's recoil halves the attacker first) and **1.80 m/s** when it was only run into: both under 2 m/s. From the side nothing closing under 10.5 m/s
(0.4 x 80 x 6 x 5 / 80 - 1.5) is more than a bump; above it he stumbles; nothing from the side fells a man who stands. **The attacker pays what the table
says**: its own stability (422 at 9 m/s) is the lower and the impulse (840) is over it, so it is the attacker who stumbles ("a sprinting attacker who runs
into a braced heavy standing skater is the one who stumbles", VISION law 1), and in the braced bounce it gives up all its speed.

### 3.4 Not for the carrier: law 3 is a gate

The first full gate with the rule on went red on `Jam.Core.Movement.StopAndGo`: "the stop begun at 5 m beats the defender at most 1 time in 10" read
**12.0 %**. A carrier that brakes to a standstill in front of a defender is, for those frames, a standing skater; with the side brace the defender's
side check became a bump and the late stop was saved by standing still. That is the opposite of law 3 (timing is the skill) and of law 1 ("the carrier's
defence is its feet and its stick"). So **the side brace and the braced bounce are for a skater without the puck**: `BraceOf` gives the carrier
`stand_brace` from every side it sees, as before the lane, and the carrier weighs 1 in the bounce. Staged: the same pressed side hit at 9 m/s on a standing
**carrier** is down at ratio 1.17, as before (`JAMSTAND ... the standing skater CARRIES the puck`). With that the law-3 drill reads what it read on the
handed-over tree (section 5).

A second test met the rule: `Jam.Core.Stick` (outside this family) stages its foul grace with a body check from the side at 8 m/s on a standing skater
that has just been stripped: no puck, so braced: a bump, no check event. One line there keeps the brace the scene was staged with
(`G.Contact.StandSideBrace = 0.0;`, `JamStickTests.cpp`, before the grace loop). In this family `Make()`, the scene builder of the older contact tests
(`Boards`, `FigureCalls`, `Levers`, `StandingBrace`: each stages its own rule with a side hit on a standing skater), does the same for both values;
`Jam.Core.Contact.StandingSides` sets them itself and asserts the shipped 5 / true.

### 3.5 What a player meets

- Run into a man who is standing still without the puck, from his side: you stop, he barely moves. Press RMB as you do it at under 10.5 m/s closing: TOO
  SOFT or BOUNCED OFF, and you are the one who stumbles. Hit him from the front, or from behind, and he goes down as before.
- Stand still without the puck and a bot that charges your side bounces off you.
- With the puck nothing changed: a standing carrier is hit as before.
- Bots rarely stand: before the carrier was taken out, 559 of 600 pro matches were hash-identical with the side brace alone, and the braced bounce alone
  moved no cell by more than a point; in its final form the rule moves the three cells -0.2 / +1.5 / +2.1.

## 4. The hit the operator makes is the hit he sees (item 3)

Headless game processes on build 5 (the shipped defaults; the carrier's exemption of build 6 touches no presentation code) (`tools/commentary/match.ps1`,
`-game -nullrhi -Multiprocess`; logs under `tools/commentary/out/a151-*`),
read by `tools/qa/ctrl/a151-feel.mjs`, which lays the sim's `JAMEVENT check` / `knockdown` lines beside `JAMHITSTOP` (the hold), `JAMKICK` (the camera)
and `JAMHUD` (the contact line), frame by frame.

| Scene | The sim | The hold (`JAMHITSTOP`) | The camera (`JAMKICK`) | The HUD's line (`JAMHUD`) |
|---|---|---|---|---|
| `jam.ctrl.Drill 22`, `jam.ctrl.LawHuman 1`: the human's own head-on hit, 3 rounds | 3 check events by the human, outcome down, ratio 3.26, 150 frames | **3 of 3 on the hit's frame** (151, 471, 791), 6 frames, `checker=0` | **3 of 3** `landed`, 12.0 cm, 8 frames | none: no line exists for a hit that lands (the line names what did NOT land) |
| the same drill, `LawHuman 0`: the human's own nudge at matched speed, 3 rounds | 3 bumps, ratio 0.35 (the bot's head-on hit beside it fells its man: a hit between two bots) | none (a bump has no hold; a hit between two bots is not the player's) | none (a bump has no kick) | **none logged, and one is owed: TOO SOFT** (see below) |
| one ordinary match, 10,800 frames (`hockey.regular`, `jam.ScriptHuman 1`): hits ON the human | 5 knockdowns by a check, 3 stumbles | **5 of 5** (2, 3, 4, 6, 6 frames by the ratio); the 3 stumbles 1 frame each | **5 of 5** `taken`; **3 of 3** `stumble_taken` | none exists for a hit taken |
| the same match: the human's own checks | **0 check events by the human in 3 minutes** | | | |

**What the logs prove.** A hit the human lands that puts a man down fires the hold and the kick on the same frame and at the same sizes as a hit that
puts the human down (the hold reads one scale both ways: 2 to 6 frames by the ratio, 6 at the top). Every hit on him had both (8 of 8).

**What stayed silent or unproven (the list the brief asked for):**
1. **The HUD's contact line cannot be read headless.** No `JAMHUD` line came in any process, including the nudge drill, where the human's press resolves
   as a bump three times and `ChecksSoft` counts three: the HUD refreshes in `UJamHudWidget::NativeTick` (`HUDWidget.cpp:704`), and Slate does not tick
   the widget under `-nullrhi`. The line's choice of words is
   covered by `Jam.Hud.ContactLine` (green); that it shows on screen for the human's own TOO SOFT / MISSED / BOUNCED OFF is **queued for the acceptance
   run** on a Development package (drill 22 with `LawHuman 0` must show TOO SOFT at each of the three nudges).
2. **A hit that lands has no words at all**, by design (the hold, the kick and the figure speak). Worth the operator's eye at 10:00: after RMB he reads
   either nothing (it landed) or a reason (it did not).
3. **The human's nudge that is a bump**: no hold, no kick (by design); TOO SOFT is its only sign on screen, and item 1 above says that sign is unproven headless.
4. **A bump the human takes** (a bot's soft hit, or being run into): the figure's hit reaction only; no line, no hold, no kick.
5. **The human's own stumble landed on a bot** (hold 1 or 2 frames, kick `stumble_landed`): proven by `Jam.Camera.StumbleKick` and
   `Jam.Core.Contact.HitStopPick` (green), not seen in a live process tonight, because:
6. **The live scripted human never checks.** `AHokejGameMode::ScriptedHumanInput` (`HokejGameMode.cpp:759`, the PIE script behind `jam.ScriptHuman 1`)
   chases, carries and passes; it is not the playtester. An ordinary headless match therefore has no human check to look at; the drill is the only live
   stage for the human's own hit. (A finding for the tools: the playtester's verbs in the live game would make every feel proof a match instead of a drill.)
7. **New with this lane's side brace**: the human's pressed hit into a standing skater from the side is a bump that bounces him: TOO SOFT or BOUNCED OFF
   (the bounce wins the line), the camera's `bounce` kick, a 1-frame hold. Same route as any bounce; staged in `Jam.Core.Contact.StandingSides`, not seen live.

## 5. Proof

- **The gate**: the full `Jam` automation filter on the final tree (build 6): **223 PASS, 0 FAIL** (`Game/logs/a151-tests-full6-ue.log`); on the tree as
  handed over 220 / 0 (`a151-tests-base-ue.log`). The three new tests: `Jam.Core.ContactPressMatch`, `Jam.Core.Contact.StandingSides`,
  `Jam.Core.Contact.LatePress`.
- **Law 3** (`Jam.Core.Movement.StopAndGo`): green, and all eight `LAW3` lines are the same before and after, character for character (pro: the stop begun
  at 8 m passes 200 of 200; all_star 0 / 39.5 / 89.5 / 100 % at 8 / 9.5 / 11 / 12 m; the straight charge 0). It was red once on the way (section 3.4).
- **The hash before and after**: 189 lines carry a hash in the baseline log, 219 in the final one; 56 are identical; 133 are measurement lines of matches
  played on the defaults, which now have the brace on and the script on course. Every asserted pin holds; none was re-pinned; this lane adds no hash constant.
- **The neutral values, across builds**: the final build with every rule of the lane at its neutral value by the measurement seam
  (`JAM_PLAYTESTER_CHECK=closing JAM_TEST_A151="side_brace=0;brace_exchange=0;late_press=0"`) plays the handed-over build's matches **hash for hash, 600 of
  600 in each of the three cells** (arm `neutral6` against `base`: 1,800 matches). And inside the gate: the pinned tests that call
  `JamBaseline::MakeNeutral` now run through this lane's four neutral lines and still hold; the staged scenes assert the neutral behaviour by its numbers
  (`StandingSides`: 0.6 x the closing speed and a knockdown from the side; `LatePress` at 0: a whiff, nothing kept).
- **The instruments**: `check_presses` and `check_whiffs` change no match (build 3 against build 1: 600 of 600 a cell).
- **State**: `Jam.Net.HashCoverage` green with the five late-press fields hashed behind `LateFrames` and `ChecksLate` listed as a read-out.
- **Queued**: the acceptance run on a Development package of this tree (the brace changes what a check does to a standing skater); in it drill 22 with
  `jam.ctrl.LawHuman 0` for the HUD's TOO SOFT (section 4, point 1).
- **Shipping compiles** (R-0057 point 3): `tools/run/build.ps1 -Project Game -Target Game -Config Shipping` on the final tree: PASS (396 s, 118 actions,
  no error line). The three new tests sit above the file's `#endif` of `WITH_DEV_AUTOMATION_TESTS`.

## 6. The seams and the drivers

- `JAM_TEST_A151="side_brace=N;brace_exchange=0|1;late_press=N"` (`JamCartridge.cpp`): this lane's rules that have no key, set on every cartridge that
  loads. Measurement only, never in a game or the gate.
- `JAM_PLAYTESTER_CHECK=old | closing | course | course:NN` (`JamPlaytester.cpp`): which press the scripted human uses (its own speed; closing speed inside
  1.8 m; on course and time at the profile's `CheckPct`; the same at NN per cent).
- `tools/perf/results/a151/cells.mjs <arm> <n> <set|-> [ENV=value ...]` (three cells, three processes), `pair.mjs <armA> <armB> [label]` (paired table row
  and a line in `cells.jsonl`, with presses, whiffs and landed), `run-arms.sh "<arm spec>" ...` (arms in sequence, memory checked before each).
- `tools/qa/ctrl/a151-tests.ps1 -Filter <filter> -Name <name>` (the gate's invocation with `-Multiprocess`), `tools/qa/ctrl/a151-feel.mjs <game.log>` (section 4).

## 7. Open, and the next evolutions

1. **The late press for a real hand** (`LatePressFrames` 6): the operator's verdict on "RMB at a man within a metre" decides it; if he reports presses
   that do nothing, it is one line, already tested and hashed. Proving number: his own; for the curve, the three cells with a playtester that presses
   late on a share of its hits (a `LatePct` beside `CheckPct`), each inside 4.
2. **The lunge as a motion**: a press that commits the body (a burst of `lunge_mps` along the course for the window's frames, paid for with stability if
   it misses) makes the check an action instead of a state. Proving numbers: the whiff share of a hand-like press (decided at 1.8 m, delivered 6 frames
   late) under 40 % with the late press off; the three cells inside 4; law 3 unchanged.
3. **The board check as a play** (item 4 of this brief, not started): a stumbling or falling skater that reaches the boards at 3 m/s or more takes them
   as a second hit (`contact.board_slam`, neutral 0). Proving numbers: staged, a skater stumbled at 6 m/s 2 m from the boards goes down on them;
   knockdowns of the human a match no more than 0.5 up at pro; the three cells inside 4.
4. Smaller: the scripted human of the live game (`jam.ScriptHuman 1`) never checks; with the playtester's verbs there, item 3 becomes a match instead of a
   drill. The carrier's exemption from the side brace could become a key with the two proposed ones (`game.contact.stand_side_brace`,
   `game.contact.brace_exchange`, `game.contact.late_press_frames`): schema, `limits.json`, `DEFAULTS.md`, `README.md` and the embedded data in one go.
