# Integration 1: the sim-and-brain seam, in one tree (lane A138, track I1, 2026-11-13; R-0053)

Four lanes each landed or prepared a rule that passed alone: the carried puck is kept and the body shields it (A102), the take grace and the earned first
step (A121), the standing start and the spin's price (A128), the brain that plays as a side and reads the shields (A115), the shove a shield cannot refuse
(A125). In one tree the gate went red on law 3 and on five pinned hashes (R-0053). This lane owns the seam for one evening: one shared baseline for the
pins, what broke law 3, the rules that make it hold with everything on, and the numbers the curve lane starts from. Status with timestamps:
`status/A138.md`. Raw logs: `Game/logs/a138/` (not landed).

Code: `Game/Source/JamCore/Public/JamBaseline.h` (new), `Public/JamCartridge.h` (three code switches, no key), `Public/JamSim.h`, `Private/JamSim.cpp`
(`TryLift`, `ContactImpulse`, `CrossThrust`, `InTakeGrace`), `AI/JamAICore.h/.cpp` (`StickPlay`, `BodyShields`, the world's two facts),
`AI/JamSimAdapter.cpp` (`BuildWorld`), tests `Private/Tests/JamCarrierSkillTests.cpp`, `JamContactTests.cpp`, `JamMovementTests.cpp`,
`Tests/HockeyBrain2Tests.cpp`.

## 1. One shared baseline for the pinned-hash neutral tests (R-0053 point 5)

`Game/Source/JamCore/Public/JamBaseline.h`, header only:

| Call | What it is |
|---|---|
| `JamBaseline::MakeNeutral(Game)` | every sim rule added since the vision's commit (main ee14438) at its neutral value, one line a rule, in landing order |
| `JamBaseline::BrainTune(Level)` | the same for the brain, as a profile tune: `brain2=0` today, for every level; the curve lane puts a level's old profile numbers here when it moves them |
| `JamBaseline::HooksDead(World)` | the brain's reads of the body shield, the take grace, the shove rule and the spin's exposed end, dead (a commit before lane A115's hooks) |
| `WithBoards`, `WithReleaseGrace`, `WithKeptPuck`, `WithCarrierAnswer` | the evening's steps, each back on, for a pin whose tree already had them; each writes the value that tree had (1.25; 120; false and true; 30, 26, true), not today's default, so a re-tuned default moves no pin |

What `MakeNeutral` sets: `contact.board_bonus` 1, `contact.release_grace_frames` 0, `stick.passive_take` true with the teammate-touch switch,
`stick.body_shield` false, `stick.take_grace_frames` 0, `skater.reverse_brake_decel` 20, `skater.burst.own_stop` false, `contact.check_reach_m` 0, the
shove rule off (`contact.bShieldShove` false: the refusal of main before lane A125), `skater.burst.rest_frames` 0, `skater.spin.exposed_frames` 0, and this
lane's two (`contact.ShoveBehindMatchMps` 0, `stick.bTakeGraceSameFrame` false).

**The next lane adds its neutral in one line** (the comment "THE NEXT LANE" in `MakeNeutral`, one pair in `BrainTune`), and one `With...()` step if a later
pin must replay the tree that had its rule on. No test changes.

The five tests, each calling the helper and un-neutralising only what its pinned tree had (the trees were read from the lanes' worktrees):

| Test | The pinned tree | The call | Pins |
|---|---|---|---|
| `Jam.Core.ContactBoardMatch` | main ee14438 (the vision's commit) | `MakeNeutral`, `BrainTune` | pro seeds 1 / 2 / 3: `6ed8f16ab724086b` `dc1a21fe22e0bc6f` `7d34793757849ca9`: hold |
| `Jam.Core.ContactGraceMatch` | lane A101's (the boards on) | + `WithBoards` | `b59b988423ca43cf` `2333d70ef430d6f3` `4f3c95b28136e412`: hold |
| `Jam.Core.ContactReachMatch` | lane A113's (the boards, the release grace 120) | + `WithBoards`, `WithReleaseGrace` | `7fb31c1f16b3f8c4` `a2be45743aebf331` `4f3c95b28136e412`: hold |
| `Jam.Bot.Hockey2.Neutral` | lane A115's start (the vision's commit, evolution 1) | `MakeNeutral`, `BrainTune` | regular seeds 1, 2 `a31f6bc64fef07f3` `1e3bb78feb1e30ad`; arena.large `6cbdb935ec9cd348` `c23153964bdb9614`: hold |
| `Jam.Core.MovementNeutral`, main bb18439 | before the vision (lanes A92, A93 off) | `MakeNeutral`, then the test's own older block | pro seed 15 `874f67830b20dc4f` (4078 frames, 3:1); all_star seed 387 `59762ff73c73f6e3`: hold |
| `Jam.Core.MovementNeutral`, lane A121's block | lane A102's tree (the kept puck) | + `WithKeptPuck`, `HooksDead` | pro seed 15 `fc59e23598229ebc` (6000 frames, 2:0): holds |
| `Jam.Core.MovementNeutral`, lane A128's block | lane A121's tree | + `WithKeptPuck`, `WithCarrierAnswer`, `HooksDead` | pro seed 15 `c8ae80c1dbdc45b0` (6000 frames, 1:1): holds |

**Every pin holds; none was re-pinned.** Two things the helper had to know that no list of keys says:

- The scripted player bot of `NetTrapMatch` (`FJamPlayerBot`, `JamAICore.cpp`) reads the body shield and the take grace whatever `brain2` says (lane A115's
  hook has no switch there), so a pin from a tree that had the body shield but not the hooks (lanes A102, A121) replays only with `HooksDead` on the world.
- The shove rule had no neutral value: it was "neutral by construction" while the shields were off. It has one now (`bShieldShove` false), and with it
  `TryLift` reads the lift's shield after the contact again, as main did.

## 2. What broke law 3 (the brief's item 2)

`Bar.Integration.Carry` (in `JamCarrierSkillTests.cpp`, outside the gate's `Jam` filter): the carry scene of `Jam.Core.Movement.StopAndGo` (hockey.regular,
the real brain at pro, 200 seeds an arm), each carry's end told apart: lifted, poked, **shoved from behind** (a cross-check on the carrier's back), shoved
from the front or the side, body-checked, timed out with the puck kept. Build 4 (before the way round the body was changed), log `Game/logs/a138/bar-1.log`.

| arm (pro) | stop at 8 m | stop at 9.5 m | straight charge |
|---|---|---|---|
| main before the parked landing: `brain2=0`, the shove rule off | 100 | 97.0 (1 lifted, 5 timed out) | 0 (200 lifted) |
| `brain2=0`, the shove rule on (the thrust whole, or by the closing speed: the same carries) | 100 | 91.0 (2 lifted, **9 shoved from the side**, 7 timed out) | 0 |
| evolution 2 with its shield and grace reads off, the shove rule on | 100 | 91.0 (1 lifted, 10 shoved from the side, 7 timed out) | 0 |
| **everything on: the parked tree** | 99.5 (1 checked) | **72.5 (15 lifted, 40 timed out with the puck)** | 0 |
| everything on, the shove rule off | 99.5 | 72.5 (the same carries) | 0 |
| everything on, the brain's meant brake off | 100 | 73.5 (28 lifted, 25 timed out) | 0 |

What the table says, plainly:

1. **Law 3 as R-0053 words it never broke.** The stop begun at 8 m passes 99.5 to 100 % in every arm and the straight charge 0 % in every arm. The gate's
   "50 %" was `StopAndGo`'s older bar at **9.5 m** (lane A102's clean stop; the far edge of the window since lane A121 moved its centre to 8 m). On the
   Director's red gate of 22:25 that row read: 100 past, 26 lifted, **73 timed out with the puck**, 1 stumble.
2. **Nobody is shoved from behind in the scene**: 0 of 6,000 carries, in any arm. R-0053's reading ("a defender whose lift the shield refuses now shoves
   the carrier in the back") is not what the drill shows. The shove rule's cost is from the **side**, and only with a brain that does not read the shield:
   the lift the body shield refuses becomes a shove on a moving carrier, a stumble with the puck loose (9 or 10 of 200, 6 points at 9.5 m).
3. **The 18.5 points are the brain's shield read.** A defender that reads the shield swings nothing and goes round the body, and lane A115's way round was
   a point beside the carrier led by the carrier's velocity: the defender ends level with a carrier that has stopped and gone and rides beside him to the
   end boards, 1 to 2 m apart, taking nothing and never passed (the traced seed 4: 250 frames side by side). 40 of 200 carries time out with the puck kept.
   By the stop's distance, `hk_round_max_mps` 0: pro 8 m 99.5, 9 m 85.0, 9.5 m 72.5, 10 m 50.5, 11 m 25.5 (108 timed out), 12 m 29.0.
4. The take grace's part of the red gate (a puck lost to a stick 8 frames after it was won) was neither rule: it is the same-frame hole lane A128 found
   (section 3, rule 3).

**After** (build 6, everything on, `Game/logs/a138/bar-2.log`): the stop at 8 m **100** (200 of 200), at 9.5 m **97.0** (1 lifted, 5 timed out), the straight
charge **0** (200 lifted).


## 3. The rules of this lane

Three in the sim (code switches with a neutral value in `JamBaseline.h`; no cartridge key, no state field, nothing new in the hash) and three in the brain.

| Rule | Where | Default | Neutral |
|---|---|---|---|
| A push in the back lands by the closing speed | `UJamSim::CrossThrust`, `FJamContactConfig::ShoveBehindMatchMps` | 3 m/s | 0 (the whole thrust from any side: lane A125) |
| The take grace covers the frame the puck is won in | `UJamSim::InTakeGrace`, `FJamStickConfig::bTakeGraceSameFrame` | true | false (lane A121's rule as it was) |
| The shove rule has a neutral value | `UJamSim::TryLift`, `FJamContactConfig::bShieldShove` | true (lane A125's rule) | false (main's refusal before it) |
| The brain presses no lift on a shielded carrier from behind | `FJamBrain::StickPlay`, world fact `bShieldShove` | on wherever the sim's shove rule is on | dead with `bShieldShove` false or no shield up |
| The brain knows the spin's exposed end | `JamAI::BodyShields`, world fact `SpinExposedFrames` | reads `spin.exposed_frames` | dead at `exposed_frames` 0 |
| The way round the body | `FJamBrain::Attack`, `FJamHockeySense::RoundLeadS`, `RoundMaxMps` (tune keys `hk_round_lead_s`, `hk_round_max_mps`) | the opposing side `RoundMaxMps` 3 m/s, the human's mates 0 (any speed); `RoundLeadS` 0.3 s; `SpinEndShare` 0.5 (tune key `hk_spin_end_share`) | 0.3 / 0 (lane A115's rule); dead with `brain2=0` |

1. **A shove from behind obeys the contact law it claims** (R-0053 point 3; VISION law 1: "two skaters gliding the same way at the same speed exchange
   almost nothing"). Since lane A125 a lift that the body shield or the take grace refuses is a cross-check's shove from any side. Its impulse was
   (closing speed + thrust 3 m/s) x mass, so a defender skating behind a carrier at the carrier's own speed stumbled him with the thrust alone: 240 on an
   unbraced back of 384 = 0.625, a stumble (and the puck loose from 0.7, which a moving back reaches: 240 / 326 = 0.74 at full speed). Now, on a back (the
   push along the victim's facing, the law's "unseen" band, `Dot(HitDir, Forward) > 0.5`) the thrust counts only as far as the attacker is closing:

       thrust on a back = thrust_mps x clamp(closing speed / ShoveBehindMatchMps, 0, 1)             (UJamSim::CrossThrust; ContactImpulse and the speed given)

   Matched speed: impulse 0, a bump, nobody loses a step, no foul. Closing at 1.5 m/s: (1.5 + 1.5) x 80 / 384 = 0.625, a stumble. Closing at 3 m/s or more:
   the whole thrust, as before: the shove from behind that fells is still in the game (it is the from-behind foul where the whistle is on). From the front
   and the side nothing changes (the cross-check to the chest still ignores the standing brace).
2. **A shove from behind that fells is not a play the brain chooses** (R-0053 point 3). `StickPlay` presses no lift on a carrier whose puck a stick cannot
   take (`StickCannotTake`: the body shield or the take grace) while it stands behind that carrier (the push along the carrier's facing above 0.35, the body
   check's own margin), unless the skill checks from behind (`bCheckFromBehind`, no level does). This holds with the shield read on or off (`brain2=0`,
   `hk_read_shield=0`): with the read off the bot still swings its refused lifts from the front and the side, as evolution 1 did, but not into a back.
3. **The take grace's same-frame hole is closed** (lane A128's finding). `ResolvePokes` runs right after `StepPuck`; the winner's grace was counted from
   the end of `Step`. A puck that changed carrier inside a frame could be poked in that same frame. A carrier that is not yet `TakeOwner` (and whose puck was
   not placed by `SetPuck`) now counts as inside its grace. `Jam.Core.Movement.TakeGrace`'s in-play bar is zero again (in 20 pro matches of three minutes: 3 stick takeaways inside 30 frames of a win without the cover, 0 with it
   (177 takeaways in each arm); in the gate's three matches 1 without, 0 with, the shortest hold before a stick takeaway 31 frames. What the three look
   like in the trace (seed 3 frame 8344, seed 7 frame 2007): a clean poke on a frame where the owner of the frame before had held 12 frames and still
   had 19 frames of grace: the puck had gone on to another carrier inside that frame, and the poke took it from the new one, which is the hole as lane
   A128 described it (read from the code and the counts; the single frames were not stepped through).
4. **The brain knows the spin's exposed end.** Lane A128 made the spin's last five frames open to a stick. The brain's shield read is the body-shield
   geometry on its own world, which says "shielded" all through a spin (the puck is dragged to the far side), so the brain of evolution 2 never pressed a
   lift on a spinning carrier (0 presses in 200 carries; lane A128's tree: 188) and the spin with the cut from 6 m was 98 % again. `BodyShields` now returns
   false in the exposed end (and a bot answers a share of the spins it meets, `SpinEndShare` (a fixed coin per bot and spin, as
   `BodyCheckShare`): every exposed end answered is too much (the stick is there on the exact frame: 27 % from 6 m, 12 % from 5 m), none is no price. The
   spin with the cut, pro, from 5 / 6 / 7 m by the share: 0: 60.0 / 99.5 / 100; 0.25: 46.5 / 72.5 / 98.0; 0.4: 42.0 / 65.0 / 97.0; **0.5: 35.0 / 59.5 /
   95.5**; 0.6: 30.5 / 53.5 / 95.0; 0.75: 23.5 / 41.5 / 93.5; 1: 12.0 / 27.0 / 90.5. Lane A128's band (6 m: 50 to 70 %, 5 m at most 60 %) is met from 0.4 to
   0.6; the default is 0.5 at every level, and it is a number the curve lane may set by level).
5. **The shove rule's neutral value** (`bShieldShove` false): what refuses the lift refuses the shove, and a lift from behind is never a shove, as on main
   before lane A125 (the lift's shield is then read after the contact, as main read it). `Jam.Core.Contact.ShieldShove` case 7.

6. **The way round the body is for a carrier that dawdles** (R-0053: "a defender that contains goal-side wins when the carrier dawdles, not when he
   stops and goes"). The brain's shield read goes round a shielding body only while the carrier is slower than `RoundMaxMps` (3 m/s: the Press job's own
   "slow carrier"). A carrier that goes is chased as before the read, still with no refused swing: the defender that was beaten by the stop overruns and
   loses its step. Candidates measured on build 4 (pro; stop at 8 m / 9.5 m / 11 m; straight 0 in each):

   | the way round | 8 m | 9.5 m | 11 m |
   |---|---|---|---|
   | lane A115's rule (any speed, led 0.3 s) | 99.5 | 72.5 (40 timed out) | 25.5 (108 timed out) |
   | led 0.15 s | 99.5 | 71.5 | 24.5 |
   | not led | 100 | 74.5 | 28.5 |
   | **only while the carrier is slower than 3 m/s** (5 and 7 m/s: the same carries) | **100** | **97.0** (1 lifted, 5 timed out) | **86.0** |

   The lead was not the cause; shadowing a carrier that goes was. All_star does not move with any of them (it does not go round: it checks).
   **The cap is the opposing side's.** The human's mates keep lane A115's rule (`P.Mate.Hk.RoundMaxMps = 0` in `DifficultyProfile.cpp`): a mate that only
   chased an opposing carrier won fewer pucks, and the big-ice idle stretch rose by 7 points (section 5). Law 3 is about the human's carry against an
   opposing defender; nothing in it asks a mate to give up the way round.

## 4. Law 3 with everything on (R-0053 point 1), and the gate

The stop's window by level, everything on (200 seeds a cell; past %):

| the stop begins at | 6 m | 7 m | 8 m | 9 m | 9.5 m | 10 m | 11 m | 12 m | straight charge |
|---|---|---|---|---|---|---|---|---|---|
| rookie | 70.0 | 100 | 100 | 100 | 100 | 100 | 100 | 100 | 42.5 |
| **pro** | 6.0 | 65.5 | **100** | 99.5 | 97.0 | 94.0 | 86.0 | 58.0 | **0** |
| the parked tree at pro (`hk_round_max_mps` 0) | 1.5 | 51.0 | 99.5 | 85.0 | 72.5 | 50.5 | 25.5 | 29.0 | 0 |
| **all_star** | 0.5 | 0.5 | 0 | 17.5 | 38.5 | **56.5** | **89.0** | **100** | **0** |

- **Pro: 100 % at 8 m (bar 80), 0 % straight (bar under 20).** The window is lane A128's again, to a point and a half (its tree, before the brain's
  second evolution: 6 m 5.5, 7 m 65.5, 8 m 100, 9 m 99.5, 9.5 m 97.0, 10 m 95.5): the brain now reads the shield and the carrier's skill is worth what it was.
- **All_star: the stop passes 56.5 % from 10 m, 89 % from 11 m, 100 % from 12 m (bar 50).** It passes nothing at 8 m: all_star does not creep, it commits at
  once with its first step (lane A128's finding stands), so its window sits a stride and a half further out. Read as "the stop passes at least 50 % where
  its window is": met. Read as "at 8 m": not met and not attempted: it is the level's approach (its numbers are the curve lane's).
- Rookie's straight charge passes 42.5 %: rookie does not press (R-0050 point 4: the curve lane's call).
- The late answers (pro, from 6 m): the cut alone 53.5, the spin with the cut 59.5, the deke tap with the cut 52.0, the stop 6.0; from 4 m nothing (5.5, 1.0).
- The mouse is the skill: at 8 m the blade left ahead 42.5 against 100; at 9.5 m 83.0 against 97.0.

`Jam.Core.Movement.StopAndGo` now holds R-0053's bars by name ("law 3 (R-0053)": pro 8 m at least 80 %, straight at most 20 %, the gap at least 47
points, all_star's stop at least 50 % at the best of 8 / 9.5 / 11 / 12 m, all_star's straight at most 20 %), and lane A121's 9.5 m bar (95 %) is green again.

**The gate: the full `Jam` automation filter, 203 pass, 0 fail** (build 7, `Game/logs/a138/full-4.log`). Node tests of the gate: 233 pass, 1 fail that is the worktree's (`BarkData.inl` against `barks.json`, the CRLF copy lanes A102 and A121 reported) **** (as handed over with lane A128 applied: 195 pass, 8 fail).
`Jam.Core.Determinism` (seed 1, frame 600): hockey.regular `ef34dd88c7c62b47`, hockey.raw `79d43eb5ea69a468`, before and after (none of this lane's rules
fires in those 600 frames). No field was added to skater or puck state: nothing new in `UJamSim::StateHash`, nothing to list for `Jam.Net.HashCoverage`.

Tests changed: `Jam.Core.Movement.StopAndGo` (R-0053's bars), `.TakeGrace` (the in-play bar back to zero, the hole counted), `Jam.Core.Contact.ShieldShove`
(cases 1 and 3 by the closing speed, case 7 the neutral value), `Jam.Bot.Hockey2.ShieldHook` (a dawdling carrier is rounded, a going one is not), the five
neutral tests (the shared baseline). New, outside the gate: `Bar.Integration.Carry`.

## 5. The numbers the curve lane starts from

The three cells on both rinks as the tree stands (build 6; 600 matches a cell, A53's seeds, `tools/perf/results/a95/curve.mjs`, rows `A138-*` in
`tools/perf/results/a95/cells.jsonl`, raw matches in `raw/`; nothing was tuned: the levels' numbers are the curve lane's):

| cell | R-0046 target | win % (95 %) | goals for : against | knockdowns of the human a match | bot reverse brakes a match | the human's | matches with a 30 s stretch without the puck |
|---|---|---|---|---|---|---|---|
| novice at rookie, regular | 85 | **98.3** (97.0-99.1) | 2.97 : 0.43 | 2.24 | 224 | 15 | 22.0 % |
| regular at pro, regular | 48 | **71.7** (67.9-75.1) | 2.49 : 1.56 | 7.38 | 259 | 38 | 4.2 % |
| good at all_star, regular | 26 | **37.5** (33.7-41.4) | 1.80 : 2.31 | 8.03 | 298 | 50 | 1.3 % |
| novice at rookie, large | | **99.8** (99.1-100) | 5.76 : 0.48 | 2.88 | 156 | 10 | 35.0 % |
| regular at pro, large | | **75.0** (71.4-78.3) | 3.32 : 1.83 | 8.11 | 167 | 21 | IDLE-PRO-LARGE |
| good at all_star, large | | **80.7** (77.3-83.6) | 3.45 : 1.62 | 8.85 | 174 | 29 | 17.7 % |

No match was unfinished. The ladder is 13 / 24 / 11 points above its targets on the regular rink, as R-0050 expected of the kept puck (lane A128's own
tree: 99.2 / 77.0 / 47.5; the brain's second evolution takes 1 / 5 / 10 of that back). Bot reverse brakes stay under 300 a match in every cell (lane A115:
334 at pro, 2,225 before it). **All_star on the big ice wins 80.7 %, above pro's 75.0 %**: on three a side the all_star profile is the weaker opponent
today; the curve lane's first finding on the large rink.

**The 30 s stretch without the puck on the big ice (lane A115's 25 %).** Regular at pro on hockey.arena.large, the share of matches whose longest stretch
without a human touch is 30 s or more (`Game/logs/a138/idle.sh`; rows `A138i-*` are lane A115's seeds 1 to 100, `A138j-*` 600 seeds):

| arm | 100 seeds | 600 seeds | the longest stretch, p50 / p90 |
|---|---|---|---|
| lane A115's conditions on this tree (track F2's seven keys at their neutral values) | 24 % | **25 %** | 24 / 36 s |
| the same with `brain2=0` | 75 % | | 36 / 55 s |
| this tree with `brain2=0` | 87 % | | 43 / 73 s |
| the parked tree's brain (the way round at any speed on both sides, no answer to the spin's end) | 32 % | 40 % (the way round alone) | 28 / 43 s |
| the way round capped on both sides (build 6) | 46 % | 46.3 % | 28 / 42 s |
| **the way round capped for the opposing side only (build 7, the tree as it stands)** | 37 % | **IDLE-PRO-LARGE-B** | 28 / 43 s |

Lane A115's gain is intact where it was measured (25 % on 600 seeds under its conditions; 87 % on this tree without the brain's second evolution). **On
the tree with the kept puck the number is about 40 %, not 25 %**: the median of the longest stretch moves from 24 to 28 s, right under the 30 s line, so the
share over the line is steep. That is track F2's rule (a carried puck leaves only by an action: the human's side takes fewer pucks back: his clean pokes a
match 4.7 to 0.7, his lifts 12.7 to 1.0), not the brain and not this lane. The bar "25 % must not be lost" is met as a property of the brain and not met as
a number on the integrated tree; the lever is the human side's way to take a kept puck (the mates' stick, track Q2's owed evolution in R-0052 point 3).

**What the curve lane must know about law 3** (pro, the stop at 8 m unless said; 200 seeds):

| lever | the stop at 8 m | |
|---|---|---|
| `opp.poke_frames` 2 / 3 / 4 (6 shipped) | 99.5 each | CURVE-6's wall (a pro that pokes after 2 frames) is gone on this tree: the brain holds its stick where the body shields |
| `opp.check_min_closing` 1 | 99.5 | |
| `opp.lift_cooldown_frames` 45 | 99.5 | |
| `opp.sprint_share` 0.7 | 93.0 | |
| **`opp.hunt_radius_m` 12** | **24.0** | the pro leaves its creep and charges: 9.5 m 91.5, 11 m 99.0, 12 m 94.0, 13 m 86.0 |
| **`opp.hunt_radius_m` 20** | **20.0** | 9.5 m 82.5, 11 m 99.0, 12 m 93.0, 13 m 88.0 |
| CURVE-6's whole pro profile (hunt 20, checks 1, poke 2, sprint 0.7, lift cooldown 45) | 5.0 (build 4) | |

**The hunt radius is the lever that breaks the gate, not the poke.** A pro that hunts from 12 m or more plays the carry scene as all_star does: the stop
still beats it, from 9.5 to 13 m instead of 8 to 10 m. As R-0053 words the gate (pro, the stop at 8 m), `hunt_radius_m` stays at its shipped value at pro,
and the ten points CURVE-6 took from it are to be found elsewhere (the poke at 2 frames is free of law 3 now).

## 6. What does not hold, and what is open

1. **All_star at 8 m is 0 %.** The gate's all_star bar is met only where all_star's window is (10 to 12 m). A player who has learned the stop at 8 m on pro
   has to re-learn the distance. Whether that is the level's character or a wall is a design call; the lever is the level's approach, not a rule.
2. **The shove rule (lane A125) is unused by the bots as shipped**: a bot that reads the shield presses no lift the stick cannot take, so only the human
   shoves through a shield. With the read off (`brain2=0`) the bots shove from the side through the body shield (6 points of the 9.5 m stop) and never from
   behind (this lane's rule). The matched-speed rule is proven staged (`Contact.ShieldShove`); in the carry scene it never fires.
3. **The first step and the defender's approach are one system** (lane A128's warning stands): every number in section 4 rests on the pro's creep. A
   brain or curve change to how the defender closes (the hunt radius above is the example) moves the window, and `Bar.Integration.Carry` is the instrument.
4. The three code switches have no cartridge key. Proposed: `game.contact.shove_behind_match_mps` (3, neutral 0). The other two are rules, like the
   teammate-touch switch.
5. Not run: the acceptance run on a package (the cursor is another lane's), drill 21 in the real game mode, the premerge gate's git-based steps (no git
   in this lane), any hand play. An acceptance row that stages a lift on a shielded carrier from behind at matched speed now reads a bump.

## 7. Neutral values and the revert path

- Everything at once: `JamBaseline::MakeNeutral` + `BrainTune()` is main's game at the vision's commit, hash for hash (the pins of section 1).
- This lane alone: `contact.ShoveBehindMatchMps` 0, `stick.bTakeGraceSameFrame` false (sim); `opp.hk_round_max_mps=0;mate.hk_round_max_mps=0;
  opp.hk_spin_end_share=0;mate.hk_spin_end_share=0` gives lane A115's way round and no answer to the spin's end (as the parked tree played); the
  no-shove-from-behind rule has no key (it is R-0053's "not a play the brain chooses") and is dead with `bShieldShove` false.
- The parked tree's own switches stand: `brain2=0`, `stick.body_shield` false, `stick.take_grace_frames` 0, and now `bShieldShove` false.

