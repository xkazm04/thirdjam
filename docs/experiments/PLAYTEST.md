# Playtest: a human-like playtester and the fun metrics (lane A34, H11.5-H12.5, device A)

Status: 2026-11-12 19:29, final for R-0028 point 3. Nothing here changes a profile or a sim number; every proposal names the numbers that would move and
what moved when it was tried (`--tune` / `--set`), for A3's worktree and the Director (R-0021 point 1: calibrations are recorded, not approved).

## 1. The playtester

`Game/Source/JamCore/AI/JamPlaytester.{h,cpp}`: `FJamPlaytester` sits in the human's slot and goes through `FJamAIInput` -> `FJamSimAdapter::Step` -> `FJamInput`,
field by field the same path the pawn's input takes in `AHokejGameMode::GatherInputs` (the only difference to a pad is that nobody holds it). Three players,
`-player=novice|regular|good` on the tournament commandlet:

| trait | novice | regular | good | how |
|---|---|---|---|---|
| reaction | 12 frames | 6 | 3 | the decision made on this frame's world is applied that many frames later (a ring, like A3's bot reaction delay) |
| aim noise | 10 degrees | 5 | 2 | the aim point is the goal centre turned by noise x `hash(seed, 2-second window)`; the player lines up on that, so the real error is up to the noise, and the 8-degree aim cone forgives part of it |
| analog stick | ramp 0.12 / frame | 0.2 | 0.35 | the stick moves toward the wanted direction at most this much per frame (a thumb, not a snap) |
| shot | holds shoot (`bShootHeld`, the pawn's charged shot) from 14 m | 20 m | 26 m | releases when the facing is within 8 / 6 / 4 degrees of the aim, or after 24 / 40 / 60 frames of holding regardless (patience); the release is the shot, so an impatient novice shoots badly aimed taps, the good player waits for the full charge when the lane stays open |
| pass | teammate 8 m ahead | 5 m | 3 m | and open (no opponent within 3 m, lane clear), and only under pressure (opponent within 3 / 4 / 5 m) or with the shooting lane blocked |
| poke, lift | yes | yes | yes | against an opposing carrier the blade follows the puck; poke once it is inside blade reach, lift at contact range (A3's scripted human does the same) |
| check | closing direction . carrier facing < 0.75 | < 0.5 | < 0.2 | at 1.8 m, faster than 5 m/s: the novice checks from behind, the good player side-on or head-on only (the sim's foul line is 0.5) |
| draw | 6 + 12 frames after the drop | 4 + 6 | 2 + 3 | presses `bPoke` for the draw; waits through the countdown |
| chase | where the puck is | leads 0.5 s | leads 0.8 s | a loose puck; a puck on the boards is approached from the ice side |
| defend | chases the carrier | goalside | goalside | regular and good go between an opposing carrier and their own net when it is nearer the net than they are |
| support | - | yes | yes | with a teammate carrying: 12 m ahead of it on the other side of the ice |
| attention | 30 frames off every 600 | - | - | the novice stops pressing for half a second every ten seconds (a held shot is let go: that fires it) |

Everything else is shared: the stick is aimed only for pokes, a carried puck is steered round a defender in the lane (3.5 m to the side nearer the middle), nobody
steers into a net mouth. All "randomness" is splitmix64 of (seed, window, salt), so a match replays to the same `state_hash` (Jam.Playtest checks seed 1 twice).

What it is not: it does not read the HUD, does not know the controls hint exists, has no camera (it sees rink coordinates), and plays alone (no team signals).
Its numbers are a floor for a person who can see, and a ceiling for one who cannot find the puck on the ice (FEEL.md: the puck reads at 10 m on day ice).

## 2. The fun metrics (one `thirdjam.playtest/1` line per match)

`touches` (the human became the puck's last toucher; per play minute), `carries_3s` (spells as the carrier of 180+ frames) and `carry_frames`, `shots` and
`shots_on_target` (the puck's line at the shot crosses the goal line inside the mouth), `shots_mates` / `shots_opp`, `goals` for / against and `goals_by_human`
(last toucher at the goal), `lead_changes`, `first_touch` / `first_shot` / `first_goal_for` / `first_goal_by_human` / `first_goal_against` (frames, -1 never),
`faceoffs` and `faceoffs_without_touch` (no human touch inside 600 frames, A2's gate window), `checks_landed` / `checks_taken`, `fouls` (`fouls_from_behind`,
`fouls_no_puck`) and `box_frames`, `pokes` / `pokes_clean`, `lifts` (clean), `passes`, `idle_spells` (spells of 900+ frames without a touch) and `idle_frames`
(frames beyond the 900th of each), `max_no_touch`, `result`, `hash`, and `acts` (frames per decision kind). `tools/playtest/run.mjs` runs the grid (3 players x 3
levels x 2 rinks, 20 seeds, 3 cells at a time, 4 minutes), prints the table and writes `tools/playtest/out/<name>.jsonl` and `.md`.

## 3. The table (20 seeds per cell, profiles and cartridges as on main f89014c; win % is the human side; first goal = by the human, median of the matches where it happened, n = matches with one, <2 min = inside 7,200 frames; fo w/o touch = faceoffs in the 20 matches without a human touch inside 600 frames; idle = spells of 15 s without a touch; box, idle and match in seconds)

| rink | level | player | W-L-D-unf | win % | GF / GA | by human | touch/min (min) | carries 3 s | carry % | shots | SOT % | shots opp | lead chg | first touch s | first shot s | first goal s (n / <2 min) | fo w/o touch | checks L/T | fouls behind/no puck (box s) | pokes (clean) | passes | idle spells (s) | max no touch s | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| regular | rookie | novice | 3-17-0-0 | **15** | 1.1 / 2.8 | 0.8 | 13.1 (8.7) | 3.6 | 14 | 5.5 | 60 | 13.1 | 0.3 | 1.8 | 18.3 | 93.9 (10 / 7) | 28 | 2.6 / 0 | 0.8 / 1.6 (11.8) | 17.2 (4) | 0 | 2.8 (23.1) | 52.8 | 244 |
| regular | rookie | regular | 12-8-0-0 | **60** | 2.2 / 1.6 | 1.7 | 21.2 (15.8) | 4 | 22 | 15.5 | 63 | 11.2 | 0.4 | 1.7 | 22.9 | 62.9 (17 / 14) | 18 | 3 / 0 | 0.5 / 1.7 (10.8) | 25.9 (8.5) | 0.4 | 0.7 (2.9) | 28.4 | 208.3 |
| regular | rookie | good | 14-6-0-0 | **70** | 2.6 / 1.6 | 1.9 | 26.2 (17.8) | 4.4 | 24 | 22.5 | 57 | 9.8 | 0.5 | 1.6 | 11.5 | 58.7 (18 / 13) | 10 | 3 / 0 | 0.1 / 2.4 (12) | 25.4 (11.5) | 1.4 | 0.2 (0.6) | 19.5 | 188.2 |
| regular | pro | novice | 1-19-0-0 | **5** | 1 / 3 | 0.5 | 15.6 (5.8) | 0.9 | 6 | 4.2 | 33 | 13.3 | 0.3 | 1.8 | 39.6 | 77.3 (8 / 6) | 14 | 1.4 / 0 | 0.5 / 0.6 (5.5) | 22.1 (7.1) | 0.1 | 3.7 (43.4) | 64.5 | 223.9 |
| regular | pro | regular | 8-12-0-0 | **40** | 1.9 / 2.3 | 1.5 | 22.1 (13.4) | 1.9 | 11 | 7.1 | 44 | 10.1 | 0.4 | 1.7 | 35.5 | 74.4 (15 / 10) | 9 | 2 / 0 | 0.4 / 0.8 (6) | 26.9 (9.9) | 0.8 | 2.4 (16.4) | 55.5 | 178.6 |
| regular | pro | good | 10-10-0-0 | **50** | 2.2 / 2.2 | 1.6 | 33.3 (17.8) | 1.8 | 14 | 16.4 | 39 | 11.2 | 0.6 | 1.6 | 12.3 | 38.7 (17 / 16) | 14 | 2.9 / 0 | 0.2 / 1.2 (6.8) | 35 (17.3) | 0.6 | 1.3 (5.2) | 29 | 167.2 |
| regular | all_star | novice | 2-18-0-0 | **10** | 0.8 / 2.9 | 0.5 | 11.2 (2.9) | 0.8 | 5 | 1.3 | 44 | 10.6 | 0.3 | 1.8 | 50.5 | 66.1 (8 / 8) | 7 | 0.7 / 0 | 0.1 / 0.5 (2.8) | 8.2 (1.7) | 0.1 | 3 (33.1) | 89.8 | 140.3 |
| regular | all_star | regular | 6-14-0-0 | **30** | 1.3 / 2.3 | 0.6 | 24.3 (12.2) | 1.1 | 11 | 4.2 | 45 | 9.3 | 0.3 | 1.7 | 17.5 | 20.2 (8 / 7) | 7 | 1.5 / 0 | 0.3 / 0.8 (5.5) | 15.8 (4.6) | 0.4 | 1.3 (7.7) | 41 | 112.4 |
| regular | all_star | good | 8-12-0-0 | **40** | 1.7 / 2.3 | 1.4 | 35.2 (21) | 1.4 | 13 | 10.9 | 33 | 12.4 | 0.6 | 1.6 | 13.8 | 43.4 (14 / 11) | 7 | 2.7 / 0 | 0.3 / 1.2 (7) | 27.6 (12.2) | 0.9 | 1.1 (5.5) | 40.2 | 143.8 |
| large | rookie | novice | 6-14-0-0 | **30** | 0.8 / 1.4 | 0.5 | 9.9 (5) | 3.3 | 16 | 2.7 | 64 | 7.3 | 0.2 | 1.8 | 49.6 | 101.1 (7 / 4) | 13 | 1 / 0 | 0.2 / 0.6 (3.8) | 14 (2.9) | 0 | 4 (52.7) | 76.1 | 210.7 |
| large | rookie | regular | 16-4-0-0 | **80** | 2 / 0.9 | 1.7 | 16.7 (10.3) | 6.7 | 26 | 14 | 64 | 7.7 | 0.4 | 1.7 | 14.3 | 68.5 (16 / 10) | 14 | 1.7 / 0 | 0.3 / 0.8 (5.3) | 23.2 (7.6) | 0.1 | 2.4 (20.5) | 54 | 227.4 |
| large | rookie | good | 14-5-0-1 | **70** | 1.8 / 0.7 | 1.1 | 20.4 (11.7) | 5.9 | 26 | 19.3 | 60 | 7.6 | 0.2 | 1.6 | 13.2 | 113.9 (14 / 8) | 29 | 3 / 0 | 0.1 / 2.2 (11.3) | 27.5 (13) | 0.6 | 2.3 (16.3) | 40.1 | 226.1 |
| large | pro | novice | 1-19-0-0 | **5** | 0.4 / 2.2 | 0.3 | 10.4 (3.7) | 0.7 | 6 | 1.5 | 47 | 6.8 | 0.2 | 1.8 | 37.7 | 53.5 (4 / 3) | 7 | 0.6 / 0 | 0.2 / 0.3 (2.3) | 12.6 (3.2) | 0 | 3.8 (84.6) | 97.6 | 208.3 |
| large | pro | regular | 5-15-0-0 | **25** | 1 / 1.9 | 0.6 | 18.9 (5.7) | 1.9 | 11 | 5 | 51 | 9 | 0.3 | 1.7 | 58.3 | 91.4 (9 / 7) | 16 | 1.6 / 0 | 0.3 / 0.9 (6) | 25.8 (9) | 0.6 | 3.7 (49.7) | 85 | 212 |
| large | pro | good | 8-12-0-0 | **40** | 1.2 / 1.4 | 1 | 27.5 (18) | 2.3 | 17 | 11.1 | 44 | 8.4 | 0.2 | 1.6 | 19.8 | 123.3 (13 / 5) | 16 | 2.3 / 0 | 0.1 / 1.3 (7) | 32.6 (16.4) | 0.7 | 3.2 (27.9) | 50.3 | 208.3 |
| large | all_star | novice | 1-19-0-0 | **5** | 0.4 / 2.6 | 0.3 | 10.5 (4) | 0.8 | 6 | 2 | 38 | 9.4 | 0.1 | 1.8 | 62.8 | 120 (5 / 3) | 6 | 0.4 / 0 | 0.1 / 0.2 (1.3) | 11.5 (2.6) | 0.1 | 4.2 (79.1) | 129.6 | 206.5 |
| large | all_star | regular | 2-18-0-0 | **10** | 0.6 / 2.6 | 0.3 | 19.8 (9.2) | 2 | 13 | 6 | 40 | 11.4 | 0.1 | 1.7 | 38 | 120 (5 / 2) | 12 | 1.2 / 0 | 0.1 / 0.8 (4) | 20.6 (7) | 0.4 | 3.6 (43.3) | 89 | 199.4 |
| large | all_star | good | 4-16-0-0 | **20** | 0.8 / 2.2 | 0.5 | 28.7 (12.7) | 1.6 | 12 | 10.6 | 34 | 11.7 | 0.1 | 1.6 | 40.2 | 113.1 (8 / 4) | 11 | 2 / 0 | 0.1 / 0.9 (4.8) | 35.6 (16.4) | 1 | 3.3 (27.1) | 65.4 | 199.9 |

Reading the table against A3's curve (BALANCING.md, the scripted human: regular 85 / 55 / 28, large 87 / 48 / 25): the **good** player lands on 70 / 50 / 40 and
70 / 40 / 20, the **regular** player on 60 / 40 / 30 and 80 / 25 / 10, the **novice** on **15 / 5 / 10** and **30 / 5 / 5**. The curve was calibrated against a bot that
shoots on first touch from 40 m without reaction time; a person who carries and aims is one level below it, and a first-time player is below rookie everywhere.
Checks taken are 0 in every cell: the bots never body check (BALANCING.md), so the human is the only one checking, and most of those checks are fouls (section 5, P4).

## 3a. What a novice experiences in the first two minutes at rookie (hockey.regular, 20 seeds)

- **0-2 s**: he wins the opening draw. The rookie centre jumps early and loses it (R-0023 point 2); the novice's press 18 frames after the drop takes the puck: first touch
  at 1.8 s in all 20 matches. Good.
- **2-20 s**: he loses it. Carrying lasts; the rookie opponents (speed 0.96, red skill 0.88, 60-frame stall tolerance) catch him and take the puck; he chases for 62 % of
  the match, carries for 14 % of play, and his first shot comes at 18 s (median). The puck is loose most of the time, so he touches it every 4.6 s (13.1 / min, worst match 8.7),
  which is fine; holding it is what he cannot do: 3.6 carries over 3 s per match against the good player's 4.4 and 24 % carrying.
- **49 s** (median): the first goal against. 14 of 20 matches concede inside two minutes; 8 of 20 are lost 0:3, three of them within 70 s (seed 2: 0:3 at 66 s, the opponents'
  first goal at 8 s).
- **94 s** (median of the 10 matches where it happened): his first goal. **7 of 20 matches have a human goal inside two minutes: the M1 bar holds in 35 %.** He shoots 5.5
  times, 60 % on target (his shots are from 14 m, so aim noise hardly matters), and 14 % of his shots go in: the rookie back holds the crease as the last man (`LastManGoalie`,
  every level) and the novice shoots into it.
- Along the way: 2.4 fouls (11.8 s in the box: 0.8 from behind, 1.6 on a skater who had just lost the puck), 28 faceoffs in 220 without a human touch inside 10 s (the
  scripted human's gate is 0), 2.8 spells of 15 s without a touch (23 s of the 244-s match), the longest 53 s, and 5 % of the frames in a lapse.
- Result: 3-17. On the large rink 6-14, with 26 % of play time idle and the first goal by the human at 101 s (4 of 20 inside two minutes).

A person sees more than this bot (the puck, the teammates' positions, the HUD) and less (FEEL.md: the puck is a 6-px dot at 10 m on day ice), so 35 % is the honest estimate
of the M1 bar today, not a floor.

## 4. Where the good player is bored at all_star

On hockey.regular the good player is not idle: 35 touches / min, 1.1 spells of 15 s without a touch per match (5.5 s in all), the longest wait 40 s. He is **beaten**: the all_star
bots score every 60 s (2.3 against in a 144-s match; the first against at 28 s median; 18 of 20 matches concede inside two minutes; 5 of 20 lost 0:3), his shots on target fall from
57 % (rookie) to 33 % because the all_star back holds the lane and steps in front, his carries over 3 s fall from 4.4 to 1.4 per match (13 % of play carrying), and 1 shot in 8
goes in. He wins 40 % (the scripted human 28 %), so all_star is not too hard for him; it is a two-minute sprint of chase and forecheck (63 % + 12 % of frames) in which a
carry never lasts. The boredom is the **large** rink: 3.3 spells of 15 s without a touch per match (27 s), the longest 65 s, 15 % of play time with nothing to do, his first
goal at 113 s median (4 of 20 inside two minutes), 8 of 20 lost 0:3, 71 % of the match chasing a puck that is far away. Jam.Playtest.GoodNeverIdlesAtAllStar fails on hockey.regular
(1-3 spells per seed) as the game stands.

## 5. Five calibration proposals (for A3's worktree and the Director), each measured with the novice at rookie on hockey.regular, 20 seeds

Baseline (table): novice 3-17 (15 %), GF / GA 1.1 / 2.8, a human goal inside two minutes in 7 of 20, first human goal median 94 s, 28 faceoffs without a touch, 11.8 s in the box,
longest wait 53 s, match 244 s. Every variant below is one `--tune` or `--set` on the same binary; raw lines in `tools/playtest/out/v<N>.jsonl`.

| # | proposal | now -> proposed | novice win % | goal inside 2 min | first human goal s | GF / GA | fo w/o touch | box s | longest wait s | match s | run |
|---|---|---|---|---|---|---|---|---|---|---|---|
| P1 | **the back's guard post out of the crease at rookie**: `guard_radius_m` / `guard_range_m` | 5 / 45 -> **10 / 20** | **55** | **11 / 20** | 75 | 2.2 / 2.0 | 24 | 10.8 | 51 | 218 | v9 |
| P2 | rookie opponents' pace: `speed_scale` / `red_skill` | 0.96 / 0.88 -> 0.85 / 0.75 | 55 | 3 / 20 | 157 | 2.2 / 1.6 | 17 | 11.0 | 51 | 350 | v1 |
| P3 | rookie opponents' shot range: `opp.shoot_extra_m` | 13 -> 5 | 40 | 8 / 20 | 113 | 1.9 / 2.3 | 38 | 16.0 | 42 | 282 | v2 |
| P4 | no `foul_no_puck` on the human (rule), or `rules.penalties` off in the rookie cut | true -> false | 30 | 7 / 20 | 113 | 1.6 / 2.3 | 0 | 0 | 58 | 240 | v4 |
| P5 | `puck.drag` | 0.5 -> 0.8 | 45 | 9 / 20 | 70 | 1.9 / 2.3 | 23 | 9.8 | **33** | 217 | v12 |
| - | P2 + P3 together (also regular player 85 %; large rink novice 95 %, regular 100 %) | | 65 | 5 / 20 | 126 | 2.4 / 1.5 | 23 | 12.0 | 51 | 390 | v6 |
| - | **P1 + P2 + P3 together: "rookie for a first-time player"** | | **85** | 10 / 20 | 100 | 2.8 / 1.1 | 14 | 7.8 | 80 | 234 | v15 |
| - | tried and not proposed: `hold_radius_m` 4.5 (30 %, 5 / 20), `opp.carry_speed` 0.85 (40 %, 5 / 20, 42 faceoffs without a touch), `opp.block_shots` 0 (25 %, 5 / 20), `shot.speed` 24 (15 %, 8 / 20), checks off (30 %, 6 / 20), `yield_margin_m` 8 / `yield_radius_m` 40 (30 %, 5 / 20) | | | | | | | | | | v3 v5 v10 v11 v13 v14 |

What the numbers say:
- **P1 is the one that moves the two-minute goal** (7 -> 11 of 20, first goal 94 -> 75 s) and it moves the win rate as much as slowing the opponents does, without stretching the
  match. The novice's shots are on target (60 %) and do not go in because the back stands in the crease; with the guard post 10 m out and only when the puck is within 20 m of
  the net, 1 shot in 4 goes in instead of 1 in 7. Caveat: `guard_*` are `FJamAIParams`, so the human's own back moves too (GA 2.8 -> 2.0 anyway, because the opponents shoot
  less often from a lane the back no longer blocks by standing in it). A per-side knob (`opp.guard_radius_m`) would be the clean form; the `LastManGoalie` rule (every level,
  "holds, never pokes" at rookie) stays and is the next thing to measure. Nothing else in the knob set reached 11 / 20.
- **P2 makes rookie winnable, not faster**: slower opponents score less (GA 1.6) and the novice carries twice as long (7.9 carries over 3 s), but the match runs 350 s and the first
  human goal comes at 157 s. A first-time player who wins 55 % of 6-minute matches without scoring in the first two is the R-0025 "fun first" question in one row. Use with P1.
- **P3** (shoot from reach + 5 instead of + 13) halves the opponents' long shots but they hold the puck longer: faceoffs without a human touch 28 -> 38 of 220. At rookie the
  carrier never yields under the human's forecheck (`forecheck_yield_frames` -1); pairing P3 with pro's 20 frames is the measured next step (one `--tune`, 15 s).
- **P4**: two of three fouls on every player, the good one included (0.1 from behind, 2.4 on a skater without the puck per rookie match), are `foul_no_puck` on a carrier who
  lost the puck between the press and the contact (6-frame window, 3-12 frames of human reaction, the lift and the drag take the puck off the blade). Rule proposal: no
  `foul_no_puck` when the victim was the carrier within the last 12 frames; cut proposal until then: `rules.penalties` false in the rookie cut (box 11.8 s -> 0; win 15 -> 30 %).
  The tunable is there already; the schema has it.
- **P5** (`puck.drag` 0.8) is the only cartridge number that shortened the longest wait (53 -> 33 s) and it lifted the win rate to 45 % with the M1 bar at 9 / 20. R-0011 picked 0.5
  against the scripted human's 100 %; the person-shaped bot prefers 0.8. It changes the shot: a tap (18 m/s) travels 22 m before it stops instead of 36, a full charge (26 m/s)
  32 m; from the 90 x 45 blue line a tap barely reaches the goal line. Re-sweep A3's curve with it before adopting (minutes with `tools/sweep`).
- **all_star for the good player**: the two knobs that could give him the puck made it worse (`opp.forecheck_yield_frames` 12 -> 6: 40 -> 20 %, idle unchanged, v7;
  `opp.pass_out` off: 15 %, v8): the carriers dump sooner and the all_star bots retrieve. He wins 40 % on regular against the scripted human's 28, so all_star on the regular rink
  needs no change for a good person; the large rink (20 %, 27 s idle per match, first goal at 113 s) is where a 3-minute timer and 120 m leave a good player with nothing to do,
  and the knob there is the rink, not the profile (R-0010 chose 90 x 45 for the same reason).
- **The gate**: `Jam.Bot.HumanHasSomethingToDo` is 0 faceoffs without a touch for the scripted human and 28 of 220 for the novice at rookie (10 for the good player). Proposal: the
  gate runs with `-player=novice` too, at rookie, with a budget (say 10 % of faceoffs) rather than 0, so the number is watched where a person feels it.

The tests `Jam.Playtest.NoviceScoresAtRookie` and `GoodNeverIdlesAtAllStar` fail on main as it stands (1 of 3 seeds; 1-3 idle spells per seed): they are the brief's bars,
left failing on purpose as the gate for whichever of P1-P5 the Director applies. On the test's own seeds (1, 2, 7) no variant reaches 2 of 3: P1 alone gives a human goal at frame 4496 in seed 2 and none in seeds 1 and 7 (0:3 twice); P1 + P2 + P3 gives 10316 / 2708 / 12850 (3:0, 3:1, 3:2: the novice wins all three and scores inside two minutes once). The bar is 10-11 of 20 seeds under either, so the test's three seeds are a hard draw; the Director may prefer the 20-seed form (`run.mjs`, `scored_2min`) as the gate, with the 3-seed test kept as the fast check.

The package P1 + P2 + P3 lands the **novice** on the curve's rookie target (85 %) with the match still 234 s long and 2.8 goals for; its cost is the regular player at 85 % and the large rink at 95 / 100 % (v6), so the large rink's rookie offsets (R-0009 point 4) need their own numbers, and "rookie" then means first-time player while pro stays the regular player's level (40 / 50 % today for regular / good).

## 6. Three gameplay ideas, with the evidence

1. **A one-timer** (a shot released within 10 frames of receiving a pass gets the full charge bonus without the hold, plus the aim cone doubled for that shot). Evidence: passes
   to the human are nearly absent (teammates pass to the human at rookie only; the human's own passes are 0 / 0.4 / 1.4 per match for novice / regular / good) and shots on
   target fall to 33-39 % at pro and all_star because the back fills the lane in front of a carrier who must hold the button to charge (`acts.charge` 4 % of frames, 60 frames of
   patience for the good player). A pass that arrives in the shooting window turns the support position (today 1-2 % of frames, idle by definition) into the way past the back,
   and gives the all_star good player something a carry cannot: a lane the back has not had time to fill. Measurable here: `shots_on_target` after a pass, `passes`, `acts.support`.
2. **A breakaway burst** (a carrier with no opponent between it and the net inside 25 m gets +20 % top speed for 60 frames, once per possession). Evidence: carries over 3 s per
   match drop from 3.6-4.4 (rookie) to 0.7-1.4 (pro, all_star) and the carry share from 14-24 % to 5-13 %: the opposing bots run at speed 1.0 with the intercept solve and
   catch every carrier, so a carry has no payoff and the good player's conversion is 1 goal in 8 shots. A burst only when the carrier is already past the last man rewards
   the one carry that got through, costs the defence nothing on the rest, and is a thing a person feels on the pad. Measurable: `carries_3s`, `goals_by_human / shots`.
3. **A deke** (stick flick: the puck moves 1 m to the blade side and the crease back / last-man goalie commits to the old line for 15 frames). Evidence: the novice has 60 %
   of his shots on target and 14 % go in; the good player 57 % and 8 % (rookie), 33 % and 13 % (all_star): the shot gate is not aim, it is the back standing in the crease
   (`LastManGoalie` at every level, squared to the carrier). A deke is the carrier's answer to a goalie and today there is none; it also gives the aim noise a reason to exist
   (a novice's deke goes wide). Measurable: `goals_by_human / shots_on_target`.

A rule finding on the way (not a feature): 2 of 3 fouls called on the human are `foul_no_puck` on a carrier who had just lost the puck (the check window is 6 frames, the
human's press is 3-12 frames late by nature, and the puck leaves the blade under the lift or the drag before contact). The good player, who never checks from behind
(0.1 per match), still sits 12 s in the box per rookie match for it. Proposal P4 below.

## 7. How to reproduce

```
node tools/playtest/run.mjs --seeds 1-20 --out tools/playtest/out/grid                 # the table (18 cells, about 4 minutes)
node tools/playtest/run.mjs --rinks regular --levels rookie --players novice --tune "speed_scale=0.85;red_skill=0.75"   # a profile proposal
node tools/playtest/run.mjs --rinks regular --levels rookie --players novice --set "rules.penalties=false"               # a cartridge proposal (variant file, validated)
UnrealEditor-Cmd Game/ThirdJam.uproject -run=JamTournament -cartridge=hockey.regular -difficulty=rookie -player=novice -seeds=1-20 -nullrhi -unattended -nopause -stdout
UnrealEditor-Cmd Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Playtest; Quit" -nullrhi -unattended -nopause -stdout
```
Raw lines of every run in this document: `tools/playtest/out/grid2.jsonl` (the table), `v1`..`v8` (the proposals). Binary: the a34 worktree at main f89014c plus this lane's files.
