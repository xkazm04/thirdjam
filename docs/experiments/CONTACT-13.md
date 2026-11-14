# Contact 13: a hit he aims lands (lane A209, track F1, the night of 14 to 15 November)

On the f1b tree standing on main b530b4eb (package H34 + docs). The operator, 20:25 (R-0067): "Tackles/Crosschecks are difficult to land as they need to
land to thin space of the player" and "Tackle is also unrecognizable and animation too quick". Status and every number: `status/A209.md`. Drivers:
`tools/perf/results/a209/` (`curve.mjs` = a202's plus `--hand` and `--seam`; `seen.sh`, `chain1.sh`, `chain2.sh`, `hashes.sh`, `same.mjs`,
`hand-table.mjs`, `cells.sh`, `law3.sh`, `ue.sh`, `build.sh`).

## 1. The instrument: a hand like a person's, and the audit of every hit he presses

**The instrument.** `JAM_PLAYTESTER_HAND=person` (`JamHitHand.h`): the scripted human skates as before, but his hits are a person's: he sees a chance
(an opponent on his feet within 4 m inside 60 degrees of where he skates or faces: the carrier always, a man without the puck 35 % of the time when closing
at 1 m/s or more), reacts after 12 to 18 frames, steers at him on eight WASD directions after an aim error of 10 to 20 degrees (the same error on the mouse's
stick), and presses when his eye says the meeting is due (the gap over the closing speed, not the courses), early or late by up to 8 frames: RMB body check
45 %, LMB cross-check tap 40 %, LMB held brace 15 %. Every 20 s or so with nobody within 5.5 m he presses the check at nobody. The audit (`FJamHitAudit`, runs for
every hand) follows each press at the nearest opponent on his feet within 4 m inside 90 degrees of his facing or going until it lands or cannot, and names the
miss's first reason. Read-only: the match is the same with or without it.

**The table** (50 matches a cell, H34's game on this tree; "person" = the hand above, "script" = the ladder's scripted press, perfect aim, 6 / 12 frames
of reaction; % of the presses at a man within 4 m in front):

| | person rookie | person pro | script rookie | script pro |
|---|---|---|---|---|
| **body check** presses a match | 12.8 | 6.7 | 8.3 | 7.6 |
| landed (stumble or down) | **43.0 %** | **34.8 %** | 12.3 % | 11.3 % |
| of those inside 2.5 m and 60 degrees | 47.3 % | 34.2 % | 17.8 % | 12.1 % |
| bump (too soft) | 5.6 | 5.1 | 4.6 | 2.4 |
| miss: the bodies met in the WIND-UP (window not open yet; they bounce apart) | **14.6** | **16.8** | 8.7 | 13.5 |
| miss: never touched, missed by under 0.3 m | **18.1** | **23.4** | 20.7 | 11.9 |
| miss: never touched, 0.3 to 0.6 m | 8.6 | 8.7 | 14.5 | 10.8 |
| miss: 0.6 to 1.0 m / farther | 3.7 / 1.7 | 3.9 / 3.0 | 8.9 / 17.3 | 12.7 / 14.2 |
| miss: met after the window closed (late) | 0.8 | 1.8 | 4.3 | 3.2 |
| miss: refused (carrier / cooldown / stick lock / man went down) | 1.9 | 2.1 | 7.9 | 17.9 |
| **cross-check (tap)** presses a match | 20.4 | 11.1 | 36.5 | 24.4 |
| landed (shove: stumble or down) | **1.3 %** | **2.0 %** | 4.2 % | 6.2 % |
| miss: the man has no puck (the shove hits only a carrier) | **58.7** | **50.4** | 3.8 | 4.4 |
| miss: beyond the stick (centres over 1.8 m) | 15.6 | 21.0 | 8.3 | 6.2 |
| miss: inside the stick, past arm's length (gap over 0.5 m: a lift, no shove) | 10.1 | 10.6 | 4.2 | 19.4 |
| miss: his back / outside the 70-degree cone / shielded | 4.1 / 2.3 / 1.4 | 5.2 / 3.1 / 1.3 | 7.2 / 8.4 / 1.9 | 9.0 / 8.3 / 4.3 |
| bump; strips of the puck (no shove) | 2.5 %; 31 | 3.1 %; 11 | 12.2 %; 26 | 10.8 %; 30 |
| **held brace** episodes (13 frames held) | 321 | 209 | 589 | 383 |
| landed / no body met | 31.2 / 68.5 % | 26.3 / 73.7 % | 7.0 / 93.0 % | 11.5 / 88.0 % |
| presses at nobody (5.5 m clear): landed | 464: 0 | 362: 0 | - | - |
| human on his back, % of play | 2.4 | 6.2 | 2.7 | 8.0 |

**Why they miss, in one line each.**
1. **The body check needs the two 0.40 m bodies to TOUCH inside the window** (centres within 0.80 m between frame 4 and frame 16 after the press). A person
   steering on eight keys misses by less than 0.3 m a fifth of the time ("the thin space"), and a sixth of the time he is ON TIME BUT EARLY: the bodies meet
   during the 4-frame wind-up, bounce apart, and the window opens on air. The check finds no man (`check_reach_m` 0).
2. **The pressed cross-check is not a hit on a man without the puck**: it is the stick lift; the shove comes only on a carrier (or one who carried within 12
   frames) and only at arm's length (gap 0.5 m). A person who presses LMB at the man in front of him gets nothing 98 times in 100.
3. **The held brace lands only on a body that walks into the stick's line** (31 / 26 %): two thirds of the holds meet nobody.
4. Refusals are not the cause for a person (2 %): no cooldown or grace on this cartridge.

**The geometry of a landing hit today** (the person's landed body checks): pressed at 1.55 m (rookie) / 1.81 m (pro) centre to centre, 29 to 33 degrees off
his going, contact 7.5 to 7.8 frames after the press. The bodies' touch: a gap of 0 (centres 0.80 m); no cone on the body check (any bearing that
touches); frames 4 to 16 after the press. Cross-check: carrier only, centres within 1.8 m for the lift and 1.3 m for the shove, 70 degrees of his facing,
not into a back (60 degrees each side of the man's back), the shove 4 frames after the press if still within 1.3 m and the cone.

**What I build next (item 2)**: the check finds its man from the press (the wind-up pulls the checker's root toward the man, at most `check_pull_m`, so the
bodies meet as the window opens; the window's bend `check_reach_m` 1.0 on; the cone reads his facing, his going AND the mouse's aim); a meeting during the
human's wind-up lands (no bounce on air); the pressed cross-check shoves the man in front with or without the puck (without: the shove's own momentum
rule, a stumble at most speeds; with: the strip as today), pulled to arm's length during its wind-up; the human's wind-ups 8 / 8.


## 2. The rule: the hit finds its man (wave 1)

| Key (`game.contact`) | Ships | Neutral | What |
|---|---|---|---|
| `check_pull_m` (new) | **1.0** | 0 | During the HUMAN's body-check wind-up his root is drawn toward the man he pressed at (`UJamSim::PullTarget`: the nearest opponent on his feet inside 60 degrees of his facing, his going or his stick's aim, whose gap the pull and his own speed can cover), spreading what his own closing will not cover over the wind-up's frames left, at most `check_pull_m / wind-up` a frame (0.125 m at 8), aiming the bodies to touch on the wind-up's last frame; his velocity is turned onto the collision course (no speed added). In the window after it, what is left is drawn over the window's frames, at most half of the pull. A meeting inside his wind-up is the hit (`CollideSkaters`' `Can`), and spends the wind-up (`EndWindupOnContact`). His pressed cross-check's man (`CrossTarget`) is drawn to half of arm's length over its wind-up the same way; the shove is taken at the press from arm's length + the pull (0.5 + 1.0 m of gap, judged where his closing will bring the man), lands one frame's pull farther (0.625 m), inside 90 degrees of his facing or aim, and is a **stumble at least** (`ResolveContact`'s new `MinOutcome`). |
| `cross_any_man` (new) | **true** | false | The human's pressed cross-check shoves the man in front with or without the puck (the tail of `ResolveLift`); a man without the puck may be shoved in the back (the thrust only as far as he closes: A138's `CrossThrust`), a carrier's back stays the stick lift. The strip stays the carrier's, on the press as before. |
| `check_reach_m` | 0 -> **1.0** | 0 | A125's bend of the open window, now on (the cone also reads the stick's aim when `check_pull_m` is on). |
| `check_windup_frames` / `cross_windup_frames` | 4 / 4 -> **8 / 8** | 4 / 4 (tonight's) | The human's wind-ups as the bots' body check (8): device B's takes need 6 or more; the pull covers them so the press does not feel late. The bots keep `bot_check_windup_frames` 8 / `bot_cross_windup_frames` 12. |

Bots: nothing changes for them (every rule above reads `FJamSkaterState::bHuman`; their brain aims its own hits). No new state field: the pull is
stateless (reads `CheckWindupLeft`, `CheckWindow`, `LiftWindupLeft`, `CrossTarget`, hashed already). Lockstep: both peers seat the human
(`SeatGuest`), so host and guest agree; protocol and wire unchanged.

**Size by speed, landing by intent**: the pull moves the root, never the speed, so a standing press that reaches his man is a bump (test:
`Jam.Core.Contact.HitFindsMan` 2), a full-speed press a knockdown; the cross-check's shove is a stumble at least ("a shove and a stumble").

Functions and lines in `JamSim.cpp` (A207 / A208 edit the same file): new `UJamSim::PullsHuman`, `PullTarget`, `OutsideCrossCone`, `CrossPullFor`,
`EndWindupOnContact`, `StepCheckPull` (after `CheckReachTarget`); one call line `StepCheckPull(S);` after `StepCheckReach(S);` in the skater's input
step; `CheckReachTarget` one condition (the aim); `CollideSkaters` the `Can` lambda's first term and the two `ResolveContact` lines of the pressed check
(`EndWindupOnContact`); `LandCrossShove` (reach, cone, the no-puck back, the floor); `ResolveLift` the carrier branch's shove distance and the tail's
search; `ResolveContact` signature (`MinOutcome`, default 0) and one line after the outcome.

## 3. The table before and after (the person-like hand, 100 matches a cell, regular rink)

| | H34 rookie | **wave 1 rookie** | H34 pro | **wave 1 pro** |
|---|---|---|---|---|
| body check: landed (stumble or down) of presses at a man within 4 m in front | 43.0 % | **85.8 %** | 34.8 % | **79.2 %** |
| ... any contact (with the bumps a standing press gives) | 50.6 | **93.0** | 40.2 | **84.8** |
| ... of the presses inside 2.5 m and 60 degrees | 47.3 | **88.4** | 34.2 | **81.4** |
| ... met in the wind-up / missed by under 0.3 m | 14.6 / 18.1 | 0.9 / 2.3 | 16.8 / 23.4 | 2.1 / 4.6 |
| cross-check (tap): landed | 1.3 % | **49.9 %** | 2.0 % | **45.2 %** |
| ... of the presses inside 2.5 m and 60 degrees | 1.7 | **61.5** | 2.7 | **52.4** |
| held brace: landed | 31.2 | 34.4 | 26.3 | 32.0 |
| presses at nobody (5.5 m clear): landed | 0 of 464 | **0 of 891** | 0 of 362 | **0 of 770** |
| the human on his back, % of play | 2.4 | 3.1 | 6.2 | 5.9 |
| his knockdowns given a match / the bots' landed checks a match | 5.6 / 5.9 | 17.7 / 6.1 | 2.8 / 10.6 | 9.9 / 10.8 |

The arms of the night (section 2's keys one by one, b3, before the cross-check's later fixes): without the pull the 8-frame wind-up alone makes the body
check WORSE (29.8 / 21.7 %: a quarter of the presses meet in the wind-up and bounce off); the pull with the old 4 / 4 wind-ups lands about the same as with
8 / 8 (85.7 / 83.3 against 86.9 / 80.6): the longer wind-up costs the body check nothing a person feels once the pull covers it.

The bars (brief item 4): presses inside 2.5 m in the cone land 85 % or more: **rookie 88.4 %, met; pro 81.4 % (84.8 % with the bumps), not yet**: wave 2.
No hit from 5 m: met. The human's time on his back at pro 5.9 % (under 8 %): no bot press rate needs to come down.

## 4. Law 3 (`Bar.Curve.Law3`, 200 a row, `JAM_LAW3_AT=8,8.5,9,9.5`)

| row | pro as built | **pro wave 1** | all_star as built | **all_star wave 1** |
|---|---|---|---|---|
| stop at 8 m (the gate, 90 %) | 94.0 | **94.0** | 99.5 | **99.5** |
| 8.5 / 9 / 9.5 m | 99.5 / 93.0 / 83.5 | **99.5 / 93.0 / 83.5** | 99.5 / 99.0 / 93.0 | **99.5 / 99.0 / 93.0** |
| straight charge (must be stopped) | 0 | **0** | 0 | **0** |

Identical: every rule of the night is the human's and never the carrier's, and law 3's human carries.

## 5. The six cells (600 a cell, the ladder's instrument `--check closing`, b9)

| cell | as built (the Director) | **wave 1** | difference |
|---|---|---|---|
| novice at rookie, regular / large | 87.3 / 86.8 | **86.3 / 92.2** | -1.0 / +5.4 |
| regular at pro, regular / large | 48.0 / 47.3 | **64.3 / 64.5** | **+16.3 / +17.2** |
| good at all_star, regular / large | 27.5 / 24.5 | **35.3 / 32.7** | +7.8 / +8.2 |

The scripted human's checks now land (his checks landed a match at pro 12.3, rookie 10.5): pro moves most. Tonight's rule: the curve lane re-sets the
ladder after the game plays; the night's neutral values give back the old cells exactly (section 6).

## 6. The proof

- **Neutral**: with `check_reach_m` 0, `check_pull_m` 0, `cross_any_man` false, `check_windup_frames` 4, `cross_windup_frames` 4 (through the seam
  `JAM_TEST_A151`), the default match is **100 / 100 hash-identical** to this commit's game in every cell (rookie, pro, all_star x regular, large) on b3
  and again on b9.
- The default match's hash changes (the shipped values): expected tonight.
- Tests: the full `Jam` filter in a cleared environment (no `JAM_` variable); `Jam.Net.HashCoverage` (no state field added); test-guards PASS
  (0 faults, 210 files); schema tests 99 / 99; Shipping compile of the Game target PASS.
- Re-staged tests: `JamContactTests.cpp` (the scene maker and the old-game block set the night's keys to neutral; `Jam.Core.Contact.CheckReach` asserts the
  new default 1.0); `Jam.Core.TacklingEvents` (`JamTacklingTests.cpp`: the frame budget reads the presser's wind-up: 8 + it, was 12 at 4). New:
  `Jam.Core.Contact.HitFindsMan` (`JamHitLandsTests.cpp`).

## 7. The acceptance drill (steps 7 and 8; `Game/Source/JamGame/Private/HokejAcceptDrill.cpp`)

- Step 7 k=1 (the body check): its cue already reads the human's wind-up (`Cfg.Contact.CheckWindupFrames`, now 8: the press comes at a gap of closing x 18
  frames, 1.2 m at 4 m/s); with the pull the contact comes on frame 7 or 8 after the press. Unchanged line.
- Step 7 k=2 (the pressed cross-check): the cue now follows the wind-up: `Gap <= cross_reach_m / 2 + closing x cross_windup_frames / 60` (0.52 m at 4 frames
  as before, 0.78 m at 8), so the shove lands at half of arm's length, a stumble at least. Edited (one line, lane A209).
- Step 7 k=3 (the held brace) and the stage that waits for a man on his feet (the Director's fix of 17:15): unchanged, kept.
- Step 8 (be checked): the bot's own wind-up (8), unchanged.
- **Not run on a package by this lane** (no cursor): the Director's release run is the proof.

## 8. Wave 2 (after READY 1): the window, the cross-check's meeting, the brace

Four rules on top of section 2 (same keys; nothing new for a cartridge; all of it the pulled human's only, so the neutral values still replay H34):
1. **The window pulls at once**: after the wind-up a man still within half of the pull (plus what his speed covers in the window) is drawn at
   `0.5 x check_pull_m / 12` a frame until the bodies meet (was: spread over the window's frames left, so the meeting came late or not at all).
2. **The cross-check follows its man**: drawn to half of arm's length by HALF of the wind-up and kept there (was: by its end), and **a meeting of the two
   bodies inside the wind-up lands the shove then** (`UJamSim::CrossMeets` / `LandCrossOnMeeting`, one `else if` in `CollideSkaters`): head-on, the bounce of
   that meeting used to part them before frame 8. The committed shove lands up to two frames' pull farther (0.75 m of gap at 8 frames).
3. **The brace finds the body**: braced, a man in front of him within 0.5 m of gap (`UJamSim::BracePullGapM`) is drawn onto him (a frame's share of the
   pull), so a body passing by the stick's width meets it; the brace's own rule decides the hit (not into a back, by the speed brought).

| person hand, 100 a cell | wave 1 rookie | **wave 2 rookie** | wave 1 pro | **wave 2 pro** |
|---|---|---|---|---|
| body check landed / any contact | 85.8 / 93.0 | 86.2 / 92.6 | 79.2 / 84.8 | 79.4 / 84.8 |
| ... inside 2.5 m: landed / any contact | 88.4 / - | 87.8 / **93.9** | 81.4 / - | 82.2 / **88.0** |
| cross-check landed / inside 2.5 m | 49.9 / 61.5 | **53.0 / 63.1** | 45.2 / 52.4 | **47.2 / 56.2** |
| held brace landed | 34.4 | **43.3** | 32.0 | **36.0** |
| human on his back, % of play | 3.1 | 2.9 | 5.9 | 6.8 |

The bar "inside 2.5 m in the cone, 85 %": with the bumps a standing press gives (a landing by intent, small by speed) rookie 93.9 and pro 88.0 %: met;
stumble or knockdown only: 87.8 / 82.2. The cross-check's remaining misses inside 2.5 m (pro): a carrier's back 12 % (the strip, no shove: the stick
lift from behind stays the brain's steal and law 1's rule), the man out of reach when the shove is due 9 %, a carrier turning his back in the wind-up
4.5 %, shielded (spin, sweep, ability) 4.7 %, the lift's 30-frame cooldown 3 %.

Wave 2's proof (b15): neutral 100 / 100 hash-identical in all six cells (`out/C2-neutral.txt`); law 3 identical to as built (pro 94.0 / 99.5 / 93.0 /
83.5, straight 0; all_star 99.5 / 99.5 / 99.0 / 93.0, straight 0); the full `Jam` filter 329 / 0 in a cleared environment (`Game/logs/a209/gate3.log`);
test-guards PASS; schema 99 / 99. The six cells at 600: rookie 86.7 / 91.0, pro 63.8 / 68.0, all_star 38.2 / 35.8 (regular / large; as built 87.3 / 86.8,
48.0 / 47.3, 27.5 / 24.5).

## 9. The next three evolutions (track F1)

1. **The bots know they are a target** (with A206): a skater without the puck in front of the human whose check winds up (`CheckWindupLeft`, `PullTarget`)
   or whose cross-check is aimed at him (`CrossTarget`) braces or turns his shoulder (the stand brace, the side brace): the hit still lands, its size
   drops. Proving number: the human's knockdowns given a match at rookie from 18.8 back toward 10 to 12 with the landing rate held (85 % or more).
2. **The press buffered through the stick's recovery and the lift's cooldown**: a press inside them is kept (up to 8 frames) and fires when they end
   (one hashed field). Proving number: the person's cooling and cooldown misses (2 to 3 % body, 3 % cross) to 0, the landing rate +2 to 3 points.
3. **The carrier's back**: the human's cross-check into a carrier's back is a shove when he closes (as on a man without the puck: the thrust as far as
   he closes) and stays the strip when he only reaches; with law 3's drill and the brain's steal measured. Proving number: the cross-check inside 2.5 m
   at pro from 56 % to 65 % or more, law 3's gate rows unchanged.
