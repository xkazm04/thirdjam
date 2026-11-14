# BRAIN-4: one stop distance at every level, and a mate that finishes (lane A153, track Q2, evolution 4; Sat 14 Nov 02:29-)

Follows `BRAIN-3.md` (lane A134; its section 6 is this evolution's first item) and R-0058 point 1. Tree: main `a9f7443` (the ladder set by CURVE-7, the
whistle out, the carry drill with a stop distance by level, the brain's third evolution). Source: `Game/Source/JamCore/AI/DifficultyProfile.cpp` (the one
all_star line, the tune keys, one call), `JamAICore.h/.cpp` (two rules at their neutral values), `Public/JamCarryDrill.h` (`StopAtForLevel`),
`Public/JamBaseline.h` (one pair a level), tests `Private/Tests/JamCarrierSkillTests.cpp` (`Jam.Core.Movement.StopAndGo`, the window by level) and
`Tests/HockeyBrain4Tests.cpp` (`Jam.Bot.Hockey4.*`). Raw logs under `Game/logs/a153/` (they do not land); scripts under `tools/perf/results/a153/` and
`a134/`; cells in `tools/perf/results/a95/cells.jsonl` (labels `A153*`).

## 1. Item 1: all_star's presser stays on its job inside the hunt (R-0058 point 1). Ships ON.

**The rule.** `Opp.Hk.bPressInHunt = true` in `MakeAllStar()`. The hunt takes every bot inside its radius off its shape job; at all_star the radius is
30 m, so the presser charged the carrier from 30 m out at 1.4 times the pace and the stop-and-go's window sat at 10 to 12 m. With the switch the presser
keeps its Press job (the gap in front of a carrier that goes) and the other four still hunt. Pro and rookie do not carry the switch (it breaks pro's
drill: BRAIN-3.md section 6). The carry drill's distance is 8 m at every level again (`FJamCarryDrill::StopAtForLevel`), and its line reads five stick
lengths at every level.

**Law 3 on the ladder's tree** (`Jam.Core.Movement.StopAndGo` in the gate's run, 200 seeds an arm; `Game/logs/a153/ue-full-1.log`):

| the stop begins at | all_star as shipped | all_star with `opp.hk_press_in_hunt=0` (the game before; the same run) | pro (unchanged) |
|---|---|---|---|
| 8 m | **200 of 200** (bar: 80 % or more) | 0 of 200 (57 lifted, 143 knocked down) | 200 of 200 |
| 9.5 m | 90.0 % (17 timed out with the puck kept) | | 97.0 % |
| 11 m | 47.5 % (86 timed out) | | |
| 12 m | 35.0 % (93 timed out) | 200 of 200 | |
| straight charge | **0 of 200** (200 poked) | | 0 of 200 (200 poked) |

- The test's all_star bar is now R-0058's: the stop begun at 8 m passes 80 % or more (it was "50 % at the best of four distances", which was 12 m). A
  neutral arm runs inside the same test (the tune `opp.hk_press_in_hunt=0`): the switch must be worth 50 points at 8 m (it is worth 100).
- The window by level (the same file's WINDOW lines, the drill's own script with the second move): rookie, pro and all_star each 200 of 200 from 8 m.
- What a player loses at all_star is the long stop: begun at 11 or 12 m the presser contains him and the carry times out with the puck still his.

**Drill 21, script 1, in the real game mode, headless** (`Game/logs/a153/run-drill.ps1`: lane A147's runner; `-game -nullrhi -Multiprocess`; a copy of
hockey.regular with `ai.difficulty` set; 20 rounds; the bar is 18 of 20):

| level | the stop begins at | past | the line on screen |
|---|---|---|---|
| rookie | 8 m | **20 of 20** | STOP WHEN HE IS FIVE STICK LENGTHS AWAY. CUT. GO. STILL ON YOUR SHOULDER? CUT BACK |
| pro | 8 m | **20 of 20** | the same |
| all_star | 8 m (12 m before) | **20 of 20** | the same (EIGHT before) |

**All_star's two cells** (the good player, 600 matches, lane A53's seeds, `tools/perf/results/a95/curve.mjs`; the ladder as set by CURVE-7 at 2,000: 29.6
on the regular rink, 26.4 on the large):

| cell | with the switch off, paired seeds | as shipped | the move | against the ladder's number |
|---|---|---|---|---|
| good at all_star, regular (`A153-as-regular`, `-off`) | 31.2 % (1.64 : 2.46) | **32.7 %** (1.62 : 2.41) | +1.5 | 3.1 over 29.6: inside 4 |
| good at all_star, large (`A153-as-large`) | the same matches | **30.3 %** (2.21 : 3.33) | 0: 100 of 100 hashes equal with the switch off (three a side has no Press job) | 3.9 over 26.4: inside 4, and not this lane's move |

- The regular cell moves a point and a half towards the human (BRAIN-3.md measured one point the other way before the ladder was set: both are inside
  the noise of 600 matches). It stays inside the bar, so no profile number is asked of the curve lane. If the curve lane wants the 1.5 back, the rule
  "aggression first" names the lever: all_star's checks (`opp.check_min_closing`, 2 m/s today), not its speed.
- The 600 seeds read all_star 1.6 over the 2,000-seed number even with the switch off (31.2 against 29.6): that is the seeds, not the tree.
- Pro is untouched: `brain4=0` against the shipped pro match, 100 of 100 hashes equal on the regular rink.

**Neutral value.** The tune `opp.hk_press_in_hunt=0` gives back all_star as it was; `brain4=0` does the same (100 of 100 hashes equal between the two on
the regular rink at all_star), and `brain3=0` / `brain2=0` switch it off with the evolutions it stands on. `JamBaseline::BrainTune()` carries `brain4=0`
at every level. No hash of a shipped cut's default match is pinned: `Jam.Bot.Hockey4.Neutral` plays one all_star match twice and asserts the two hashes
equal, and logs the match with the switch off (seed 1: as shipped 24,893 frames 3:2, off 3,572 frames 0:3).

**The gate.** The full `Jam` filter on the final build (build 2): **236 of 236** (`Game/logs/a153/ue-full-2.log`; 230 on main a9f7443, 6 new; no "missing
usage flag" line). Build 1 (item 1 with the first two neutral rules): 234 of 234, and build 2 as shipped plays build 1's matches (100 of 100 hashes at pro
and at all_star on the regular rink), so every number of this section is the final source's. The gate's node tests: 243 pass, 1 fail that is the
worktree's (`BarkData.inl` against `barks.json`, the CRLF copy). `Jam.Bot.Hockey4.*` (6 new):

| test | before | after |
|---|---|---|
| `PressInHunt` | the presser 10 m goal-side of a carrier at 6 m/s, inside a hunt of 30 m: full stick at the carrier (move -1.07) | under control (-0.27): exactly what a presser with no hunt at all plays; all_star's profile carries the switch, pro's, rookie's and the mates' do not; each of the four tunes switches it off; the shared baseline's all_star tune switches it off |
| `MateFinish` (item 2) | a mate 10 m out, 10 degrees off the net, the lane open but for the last man: carries on | shoots; an opponent on the lane, 22 m out, 30 degrees off: unchanged |
| `CarryClock` (item 3) | a carrier that holds the puck at centre ice for 200 frames: never releases it | with a 2 s clock it shoots or passes from frame 120 |
| `MateDrive` (item 2) | a mate 22 m out with an open lane, the human open and better placed: it hands him the puck (19 pass frames in half a second) | it keeps it (0); a human set up for the one-timer still gets it |
| `Screen` (item 2) | the human's first frame with the puck, an opponent 5 m from him: his mate goes off to offer a lane (-0.90) | it steps between them (1.00); once the frames are up, as before |
| `Neutral` | | `brain4=0` puts every rule of evolution 4 at its neutral value; the same match twice is the same match |

No field was added to skater or puck state. The opposing side's approach changes at all_star only, and only at five a side.

## 2. Items 2 and 3: four rules in the code at their neutral values (measured below as profile tunes; what ships is in `SetEvolution4`, `JamAICore.h`)

| field, tune key | rule | where |
|---|---|---|
| `MateFinishM`, `mate.hk_mate_finish_m` | A mate that carries the puck inside this distance of the opposing net, at most 60 degrees off its axis, with an open lane (no standing opponent within 1.2 m of the line puck to net up to 3 m in front of the net: the last man in his crease does not close a lane) finishes: inside the sim's aim cone it counts as lined up; it does not give the puck to a better-placed mate or to the human by its coin (a human set up for the one-timer still gets it, pressure still moves the puck). | `Attack` |
| `MateDriveM`, `mate.hk_mate_drive_m` | The same open lane out to this distance: the mate takes the puck to the net itself and does not hand it off on the way. | `Attack` |
| `ScreenFrames`, `mate.hk_screen_frames` | For this many frames after the human takes the puck, the mate nearest the opponent nearest him (within 8 m of him, not on him already) stands in that opponent's way, 1.2 m from it on the line to the human, instead of offering a lane. | `Decide`, `JobSteer`, `ScreenPoint` |
| `CarryClockS`, `opp.hk_carry_clock_s` | A team without shape jobs (fewer than five): a carrier that has held the puck this many seconds passes when the sim's pass has a receiver, else shoots it at the net once it faces up the ice. | `Attack` |

Switch: the tune `brain4=0` puts all four at their neutral values and all_star's presser back on the hunt (evolution 3); each rule alone by its key.

## 3. Item 2: a mate that finishes. Measured; nothing ships on.

30 regular matches at pro an arm, traced frame by frame (`tools/perf/results/a153/pt.sh ... 1-30 1`; `tools/perf/results/a153/releases.mjs` for the
mates' releases, `docs/design/review-1/humancarry.mjs` for the carries; about 156 carries after a mate's pass an arm):

| arm | still the human's 2 s after a mate's pass (bar 30) | 1 s | that carry's median | passes he receives a minute | mates' releases from inside 15 m, a match (bar 1.5) | mates' shots a match | goals for |
|---|---|---|---|---|---|---|---|
| as shipped | 18 % | 44 % | 0.90 s | 1.44 | 0.77 | 3.10 | 2.00 |
| the finish inside 15 m (the brief's rule) | 18 % | 45 % | 0.90 s | 1.42 | 0.77 | 3.00 | 2.03 |
| the finish, and a mate holds the puck 45 frames before handing it off (`mate.hk_min_carry_frames=45`) | 24 % | 48 % | 0.92 s | 1.32 | 0.67 | 2.73 | 2.20 |
| the drive at 25 m with the finish | 19 % | 45 % | 0.90 s | 1.39 | 0.80 | 3.07 | 2.10 |
| the screen for 60 frames | 22 % | 48 % | 0.97 s | 1.41 | 0.83 | 2.47 | 2.27 |
| **the screen for 120 frames** | **30 %** | 52 % | 1.07 s | **1.61** | 0.57 | 2.23 | 2.03 |
| the screen for 60 frames with the drive and the finish | 23 % | 47 % | 0.92 s | 1.39 | 0.87 | 2.60 | 2.43 |

1. **The review's premise does not hold: a mate is hardly ever in the slot with the puck.** At pro a mate is the carrier of record inside 15 m of the
   net for 53 frames a match, in three spells of a third of a second (the human: 134 frames). It already releases from there 0.77 times a match: once in
   four spells. The sim's aim cone (8 degrees) is no wider than the brain's own line-up at 10 m, so "a mate inside 15 m with an open lane shoots" changes
   nothing in a match (the same 0.77; the pro cell 50.2 against 50.5 at 600).
2. **Keeping the puck longer does not get a mate there either.** With the hand-off delayed to 45 frames, or forbidden on an open lane inside 25 m, a
   mate's carry still ends lost or loose in 62 % of the cases with a median of 0.6 s: it is stripped like everyone at pro (the poke after 2 frames, the
   check from 1 m/s). The bar "1.5 releases a match from inside 15 m" is a number of the ladder's pressure, not of a mate's decision.
3. **The screen is the rule that moves the human's own numbers.** With the nearest mate standing in his nearest opponent's way for two seconds, the puck
   is still his two seconds after a mate's pass in 30 % of the cases (18 %), and he receives 1.61 passes a minute (1.44). The mates then shoot less (3.1
   to 2.2 a match): the screener is not on its lane. Thirty matches an arm: the 12 points are about two and a half standard errors.
4. **And it is worth more than the ladder's 4 points at pro**, so it ships at its neutral value:

| cell (600) | as shipped | the screen for 120 frames | the move |
|---|---|---|---|
| novice at rookie, regular (ladder 85.3) | 84.7 (2.73 : 1.21) | SCR_RK | |
| regular at pro, regular (49.1) | 50.5 (2.08 : 2.05) | **55.7** (2.21 : 1.92); at 60 frames 55.7 as well (2.21 : 1.93) | **+5.2** |
| good at all_star, regular (29.6) | 32.7 (1.62 : 2.41) | 32.7 (1.70 : 2.40) | 0.0 |
| regular at pro, large | 54.8; idle30 56.2 % | SCR_LARGE | |

   What pays it back, by the curve lane's rule (aggression first): a notch of pro's forecheck. The screen on (`kEvo4ScreenFrames` 120 in `JamAICore.h`)
   with the pro cell re-set is the brain track's proposal for the next curve pass.

## 4. Item 3: the big ice. Measured; nothing ships on.

Regular at pro on hockey.arena.large, 600 seeds an arm (`tools/perf/results/a153/cell.mjs` is `curve.mjs` with `--extra` for `-playersprint`):

| arm | matches with a 30 s stretch without a human touch (bar 25 %) | p50 / p90 of the longest | win | goals |
|---|---|---|---|---|
| the tree as it stands | **56.2 %** | 32 / 49 s | 54.8 | 2.33 : 2.06 |
| the scripted human sprints (`-playersprint`: a person has Shift) | 52.7 % | 31 / 48 s | 56.7 | 2.42 : 2.09 |
| the opposing three's carry clock at 4 s | **40.8 %** | 28 / 43 s | **73.0** | 2.70 : 1.53 |
| the carry clock at 2 s | 31.5 % | 26 / 38 s | 84.8 | 3.02 : 1.17 |
| the sprinting human and the clock at 4 s | 36.2 % | 27 / 41 s | 71.7 | 2.75 : 1.53 |

1. **The number the brief carries (32.0 %) is not this tree's.** It was measured before the ladder was set. CURVE-7 made pro press (the poke after 2
   frames, checks from 1 m/s, the large rink's red skill), and on the big ice that reads as a human who waits: 56.2 % of matches hold a half-minute
   without the puck. R-0055 point 5's bar is 31 points away, and the last ladder pass moved the cell 24 points the wrong way.
2. **The sprint is not the lever** (3.5 points): the instrument is not what hides a better game.
3. **The carry clock is the lever, and it is a ladder dial.** An opposing carrier that must move the puck after four seconds gives the human's side the
   puck back (15 points of the idle share) and gives away 18 points of the cell: an opposing three that cannot hold the puck is a weaker side. It ships
   at its neutral value. With the clock on, the curve lane has 18 points of aggression to spend on the big ice at pro for a human who waits a third less;
   even at 2 s the bar of 25 % is not reached (31.5 %).

## 5. What does not hold, and what is open

1. Item 2's three bars at pro on the regular rink, as shipped: the mates' releases from inside 15 m 0.77 a match (bar 1.5: **not met**, and not a
   brain number); passes received a minute 1.44 (the review: 1.54; **not moved**); still his two seconds after a mate's pass 18 % (bar 30: **not met**
   as shipped; 30 % with the screen, which waits for the curve).
2. Item 3's bar: 56.2 % against 25 % (**not met**; the clock at 4 s reads 40.8 % and waits for the curve).
3. All_star's long stop: begun at 11 or 12 m it no longer passes (47.5 and 35 %; the carry times out with the puck kept). R-0058 rules the one distance;
   a player who stops early at all_star is contained, not beaten.
4. The switch plays only where the Press job exists (five a side). On the big rinks (three a side) all_star approaches as before: the drill and the law-3
   test are regular-rink scenes, so "one stop distance" is proven for the regular game and not for the six-skater cuts.
5. Not run: the Shipping compile (the landing chain's step; the new test file is inside the automation guard from its first line of code), the
   acceptance run (drill 21's all_star line and distance change what a player reads), any hand play.

## 6. How to repeat

```
tools/perf/results/a153/tests.sh Jam <tag>                                              # the gate's filter -> Game/logs/a153/ue-<tag>.log
powershell -File Game/logs/a153/run-drill.ps1 -Script 1 -Name all_star -Cut <copy of hockey.regular with ai.difficulty all_star>   # lane A147's runner
CLS=good tools/perf/results/a153/idle.sh <label> all_star regular 600 "opp.hk_press_in_hunt=0"     # one cell, one summary line
DRIVER=tools/perf/results/a153/cell.mjs tools/perf/results/a153/idle.sh <label> pro large 600 "" --extra "-playersprint"
tools/perf/results/a153/pt.sh <tag> hockey.regular pro regular 1-30 1 "-aitune=mate.hk_screen_frames=120"   # 30 matches traced frame by frame
node tools/perf/results/a153/releases.mjs Game/logs/a153/<tag>.log; node docs/design/review-1/humancarry.mjs Game/logs/a153/<tag>.log
```
