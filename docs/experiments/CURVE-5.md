# The difficulty curve, pass 5: the vision on one tree and the ladder set once (lane A95, device A, 2026-11-13 16:53-)

Scope (brief `briefs/A95-integration-and-curve.md`, R-0045, `docs/VISION.md`): lanes A92 (contact by inertia) and A93 (movement, sprint, puck distance, the two-sided
sweep shield) applied on main bb18439 (A94's hockey brain, A91's shot calibrations, A86's mouse-style playtester); the gate and the acceptance run green on that tree; the
playtester plays the vision's verbs; the difficulty ladder set once on the real player; plus B-to-A-0037 (the lift cone, `Dir` on lift and poke, the bots' lift cooldown).

## 1. Job 1: the red list of the merged tree, with causes

Merged tree as handed over, built (unity off), the three generators re-run first (`JamSchemaData.inl` and `JamDesignerData.inl` were stale after the merge; `parity.json`
unchanged; schema tests 99 / 99). Full `Jam` filter (`Game/logs/a55-automation-ue-20261113-165608.log`): **153 pass, 2 fail**.

| test | cause | fix |
|---|---|---|
| `Jam.Core.ContactNeutral` | pinned main 8f6b157's hashes (before A94's brain, A86's `W.Carrier`, A91's calibrations) and neutralised only A92's block, so A93's movement rules stayed live | re-pinned (below) |
| `Jam.Core.MovementNeutral` | the same: 8f6b157's hashes, A92's contact model live | re-pinned (below) |

Every other gate test was green on the merged tree, including the two A93 had red alone (A94's brain moved them): `Jam.Playtest.NoviceScoresAtRookie` 12 of 20 (need 10),
`Jam.Bot.HumanHasSomethingToDo` 13 of 120 (budget 15.1 % = 18, the Director's), `Jam.Bot.SeasonTakes` (take-2 AI shots 2 / 3 / 5). Determinism on the merged tree:
hockey.regular `d192973ac7c97102`, hockey.raw `b02f67a2da0314b4` (bb18439: `bd698403ed205fbd`, `0e5637bb401646ae`).

**The re-pin.** The proof is now "the vision's neutral blocks give back main bb18439 on this tree's brain": both tests write A92's block (`contact: {momentum: false,
carrier_can_hit: true}`), A93's block (the six movement keys) and `stick.lift_cone_deg: 180` into the cartridge text, `game.ai.sprint: 0`, set A93's code switch for the
one-sided sweep shield (a rule with no key), and apply `JamAI::PreA95Tune(level)` (every profile number this lane changed, with bb18439's value; new in
`DifficultyProfile.h`). The pins were read from bb18439's own build (`.claude/worktrees/clean-main`, HEAD bb18439, built 16:37):

| match | bb18439 | this tree, neutral blocks |
|---|---|---|
| NetTrap: hockey.regular pro seed 15, the brain and the player bot | 4078 frames, 3:1, `874f67830b20dc4f` | the same (`Jam.Core.MovementNeutral`) |
| MatchAlwaysEnds: hockey.regular all_star seed 387, the regular playtester | 9475 frames, 3:2, `59762ff73c73f6e3` | the same (both tests) |
| MatchAlwaysEnds: hockey.boost pro seed 150, the regular playtester | 8633 frames, 3:2, `1a33b22080d0f0df` | the same (`Jam.Core.ContactNeutral`; the model resolved nothing) |

The playtester's new verbs (section 3) are used only where the cartridge has them (`reverse_brake_decel`, `burst.frames`, `sprint.carry_bonus` above 0), so the
neutral proofs hold with the new playtester too.

<!-- sections below are filled as the rows complete -->

## 2. B-to-A-0037 (the coordinator's addition, before the ladder)

- **The lift cone**: `game.stick.lift_cone_deg` (default 70, range 10..180, 180 = no cone; schema, `limits.json`, `DEFAULTS.md`, `README.md`, `FJamStickConfig::LiftConeDeg`,
  the loader, the three generators; schema tests 99 / 99). `UJamSim::OutsideLiftCone`: a carrier more than the cone off the attacker's facing is neither lifted nor
  shoved; the swing is a miss. It guards the clean lift, A92's cross-check shove and A92's "take in the same breath" grace shove, for bots and the human alike.
  `Jam.Core.LiftCone` (new, `JamContactTests.cpp`): a standing carrier, the attacker beside it at arm's length turned 0 / 30 / 60 / 80 / 110 / 160 degrees off the
  line: clean lift and shove at 0, 30, 60; miss and nothing landed at 80, 110, 160; with the cone at 180 every angle lifts. `Jam.Core.Stick`'s two lift scenes had the
  lifter facing 90 degrees off the carrier; turned to 60 (inside the cone).
- **`Dir` on `lift` and `poke`**: the clean lift carries the unit vector attacker -> victim (asserted in `Jam.Core.LiftCone`), in the frame of the lift; a lift miss
  carries attacker -> victim (shielded) or attacker -> blade; a clean poke attacker -> carrier (or -> blade on a loose puck); a poke miss attacker -> carrier.
  `EffectsBinder.cpp` sends `User.Dir` for `lift` and `poke` as for `check` and `knockdown` (one line). No `Jam.Fx` test lists the directed events.
- **The bots' lift cooldown** (B saw two figures trade cross-checks 2 to 10 frames apart): `FJamBotSkill::LiftCooldownFrames` in the brain, on top of the sim's own
  30-frame swing cooldown: **pro 90 frames (1.5 s), all_star 45 (0.75 s)**, rookie never lifts; tune key `opp.lift_cooldown_frames`. The brain also lifts only inside
  the cone (`FJamAIWorld::LiftConeDeg`), so a bot never wastes a swing the sim would refuse.
- The acceptance run's cross-check row passes with the cone (the drill comes from the front): section 4.

## 3. Job 3: the playtester plays the vision (`JamPlaytester.cpp`, `.h`)

Each verb is used only when the cartridge has it (`FJamAIWorld::ReverseBrakeDecel`, `BurstFrames`, `SprintCarryBonus`, new, filled by the adapter), so a neutral
cartridge replays the old playtester; `JAM_PLAYTESTER_VERBS=old` (env, measurement only) replays it on any cartridge.

- **The brake on purpose** (`BrakeIntent`): when the wanted move reverses against the velocity (the sim's reverse-brake zone, over 120 degrees at more than 1 m/s) the
  class decides whether it means to stop: good and regular always (stop and go); the novice only when its target is within 6 m (the puck it skated past). A reversal
  it did not mean (the analog ramp passes through a reversed input on every about-turn: A93's accidental brakes) is carved into a turn, 115 degrees off the velocity
  on the input's side, with the same push. The intent rides the reaction ring with the input it belongs to.
- **The cut and burst** (`CutPct`: regular 50 %, good 100 % of the two-second windows, novice never): carrying, a defender closing (more than 1 m/s) in front inside
  5 m, not lining up a shot: a 75-degree cut away from it (the other way if the boards are within 4 m) for 10 frames, at most one a second. The ramp turns the
  stick 75 degrees in 4 to 7 frames, so the sim's burst arms (60 degrees inside 10 frames).
- **The sprint with the puck** (`CarrySprintPct`: regular 50 %, good 100 %, novice never): carrying (not charging, passing or cutting), no opponent within 5 m and
  none on the lane 12 m ahead (2 m half-width): Shift held. By default, no flag: `-playersprint` still adds A72's chase sprint. `Jam.Core.ModeKeys.Sprint` now asserts
  that without the flag the human sprints only with the puck.
- Read-outs in the playtest line: `cuts`, `rev_brakes`, `carry_sprint_frames`, `burst_frames`, `bot_rev_brakes`, `bot_carry_sprint_frames`.

**Before and after** (600 matches per cell, hockey.regular, A53's seeds, paired; before = `JAM_PLAYTESTER_VERBS=old` on the same binary; reverse brakes are the sim's
count of stops a match; the first build of the verbs, with the carry sprint on a 15 m lane and no opponent within 6 m):

| class at its target level | win % before | win % after | reverse brakes before -> after | carry sprint s / match | cuts / match |
|---|---|---|---|---|---|
| novice at rookie | 94.7 (92.6-96.2) | 97.0 (95.3-98.1) | 57.4 -> **41.6** (the accidental ones carved) | 0 | 0 |
| regular at pro | 83.8 (80.7-86.6) | 81.5 (78.2-84.4) | 77.9 -> 77.8 | 0 -> 1.2 | (not counted yet) |
| good at all_star | 61.7 (57.7-65.5) | 64.3 (60.4-68.1) | 76.3 -> 80.9 | 0 -> 2.0 | (not counted yet) |

With the lane relaxed to 12 m / 5 m and the cut counter (the build every lever below was measured on): regular at pro 82.3, 13.4 cuts and 2.3 s of carry sprint a
match; good at all_star 62.0, 18.9 cuts and 3.5 s; novice 97.0, no cut, no carry sprint. The verbs change the win share by 2 to 3 points, inside the intervals:
the carrier's weapon is real in the sim (A93) and the classes now use it, but it is not what decides these matches against A94's brain. The bots brake about 1,700
times a match (perfect inputs, A93's finding); that is the brain's and is not changed here.

## 4. Job 2: the acceptance run

`tools/qa/accept/accept.ps1 -Package Game/Packaged/A95-dev -Name a95-run1 -Port 4695` (the merged tree with the re-pins and B-to-A-0037; nothing else running):
**19 ok, 0 fail**. Step 1 skate, sprint, sprint_carry (10.93 m/s with the puck), **brake** (still in 32 frames from 10.66 m/s, back at 11.00) PASS; step 4 shoot_tap /
half / **full** PASS; step 7 **check** (down 134 frames, ratio 2.64, closing 12.38) and **crosscheck** PASS (stick hit 2, puck free, down 74, ratio 1.33, closing 4.96
at A92's cue on `contact.cross_reach_m`: the cue A93's burst had moved is already the lift's range, so the row needed no further re-staging; the drill comes from the
front, inside the 70-degree cone); step 8 **checked** PASS (down 86, ratio 1.57). The final run on the final numbers is section 7.

## 5. Job 4: the levers, one at a time (600 matches per cell, hockey.regular, A53's seeds, every arm on the same matches)

Driver `tools/perf/results/a95/curve.mjs` (A86's, plus the verbs arm and the new read-outs), rounds `levers1.sh` .. `levers11.sh`, rows in
`tools/perf/results/a95/cells.jsonl`, raw matches in `raw/`. Tune keys are `-aitune` (`DifficultyProfile.cpp`; new: `opp.sprint_share`, `opp.lift_cooldown_frames`,
`sprint`). Base = the merged tree with the bots' sprint shares (rookie 0.1, pro 0.35, all_star 0.7) and the playtester's verbs, before any lever.

**What each lever did, in the brief's order** (win share of the target class; the gap to close was rookie -12, pro -32, all_star -37):

| lever | rookie (novice, base 97.0) | pro (regular, base 82.3) | all_star (good, base 62.0) |
|---|---|---|---|
| 1. principles: tighter / earlier (share, late frames) | 0.85 / 8: -3.5 | 1.0 / 0: -1.5 | (already 1.0 / 0) |
| 1. principles: read the human's pressure | | +4.4 (easier) | off: +4.8 (easier): kept on |
| 1. carry-or-pass: pressure arrives inside | | 0.8 s: -2.0; 2.0 s: +3.9 | 0.8 s with better-by 8 and hunt 14: see combinations |
| 1. carry-or-pass: better-placed by | | 8 m: -5.3 | 12 m (with pressure 0.5 s): -0.1 on D2 |
| 1. carry-or-pass: min carry 40 frames | | +0.4 | |
| 2. hits: checks from (m/s) | 6 (was never): -2.7 (3.0 knockdowns a match) | 2 (was 4): -2.0 (knockdowns 7.7 -> 8.7) | 1.5 (was 3): -0.8 |
| 2. hits: hunt radius | | 8 m (was 0): -4.6 | 14 m (was 10): -8.3 |
| 2. hits: the lift cooldown | | | 20 frames (was 45): -1.3 |
| 3. bots' sprint share | 0.3 (was 0.1): -2.5 | 0.7 (was 0.35): -1.5 | 1.0 (was 0.7): -3.7 |
| 4. red skill | 0.9: -5.8; 1.0 with checks from 6 and sprint 0.2: -12.2 (L7-rk-B) | 1.05: -7.3; 1.15: -8.3 | 1.1: +5.5; 0.9 in D: -5; 0.8: +13 (not monotonic) |
| 5. speed | | 1.1: -10.1 | 1.35: +1.5; 1.05 in D: +4; **1.4 in G: -6.5** |
| (other) shot range | | **+13 -> +30 m: -15.6** | 40 -> 25 m: +7.3 (easier); 50 in G: -1.5 |
| (other) shot wind-up | | 18 frames: +3.0; 4 frames: +6.0 | 6 frames: -1.8; 10 on D2: +1.5, with no yield and cooldown 20 on D2: -1.1 |
| (other) forecheck yield never | | -1.0 | -0.5 |
| (other) draw jump 0 | | | -1.0 |

The principle knobs a level owns (how well and how late) move the win share by 2 to 5 points, inside or near the noise: A94's principles cost the opponents about
40 points against the playtester (BRAIN.md, `opp.hk_on=0` 41 % at pro) and no execution setting gives that back, because the cost is in what the principles decide
(pass instead of shoot: opponent shots 14.8 -> 7-10 a match), not in how fast they decide it. The levers that moved the cells are the ones that give the opponents
shots and the puck: the pro bot's shot range (opponent shots 10.3 -> 13.6), the hunt, the carry-or-pass knobs at all_star (carry through the forecheck instead of
passing into it), then red skill and speed. The shot range is not in the brief's list; it is the level's "how well it executes" in the sense the brief gives it
(a pro bot that never shoots is not executing), and it is reported here as a lever, not hidden.

All rows:

| label | class / level / rink | tune | n | win % (95 %) | fwo % | checks taken | opp shots | bot sprint s | scored < 2 min |
|---|---|---|---|---|---|---|---|---|---|
| L1-pro-base | regular / pro / regular | (profile) | 600 | 82.3 (79.1-85.2) | 1.39 | 7.73 |  | 426 | 303 |
| L1-pro-hktight | regular / pro / regular | `opp.hk_share=1;opp.hk_late_frames=0` | 600 | 80.8 (77.5-83.8) | 1.78 | 7.6 |  | 432.3 | 284 |
| L1-pro-hkpress | regular / pro / regular | `opp.hk_read_human_pressure=1` | 600 | 86.7 (83.7-89.2) | 1.38 | 7.61 |  | 441.1 | 289 |
| L1-pro-check2 | regular / pro / regular | `opp.check_min_closing=2` | 600 | 80.3 (77-83.3) | 1.72 | 8.66 |  | 439.2 | 289 |
| L1-pro-hunt8 | regular / pro / regular | `opp.hunt_radius_m=8` | 600 | 77.7 (74.2-80.8) | 1.94 | 8.43 |  | 434.5 | 271 |
| L1-pro-sprint07 | regular / pro / regular | `opp.sprint_share=0.7` | 600 | 80.8 (77.5-83.8) | 1.9 | 7.89 |  | 621.5 | 293 |
| L1-pro-red105 | regular / pro / regular | `red_skill=1.05` | 600 | 75 (71.4-78.3) | 1.95 | 7.52 |  | 448.3 | 255 |
| L1-as-base | good / all_star / regular | (profile) | 600 | 62 (58.1-65.8) | 0.5 | 7.33 |  | 502.9 | 335 |
| L1-as-check15 | good / all_star / regular | `opp.check_min_closing=1.5` | 600 | 61.2 (57.2-65) | 0.48 | 7.67 |  | 503.2 | 321 |
| L1-as-hunt14 | good / all_star / regular | `opp.hunt_radius_m=14` | 600 | 53.7 (49.7-57.6) | 0.66 | 7.97 |  | 519.6 | 299 |
| L1-as-sprint1 | good / all_star / regular | `opp.sprint_share=1` | 600 | 58.3 (54.3-62.2) | 0.7 | 7.63 |  | 609.9 | 287 |
| L1-as-lcd20 | good / all_star / regular | `opp.lift_cooldown_frames=20` | 600 | 60.7 (56.7-64.5) | 0.63 | 7.67 |  | 511.6 | 326 |
| L1-as-red11 | good / all_star / regular | `red_skill=1.1` | 600 | 67.5 (63.7-71.1) | 0.59 | 7.73 |  | 510.4 | 324 |
| L1-rk-base | novice / rookie / regular | (profile) | 600 | 97 (95.3-98.1) | 5.56 | 0 |  | 274.7 | 329 |
| L1-rk-hk085 | novice / rookie / regular | `opp.hk_share=0.85;opp.hk_late_frames=8` | 600 | 93.5 (91.2-95.2) | 5.7 | 0 |  | 280.6 | 322 |
| L1-rk-check6 | novice / rookie / regular | `opp.check_min_closing=6` | 600 | 94.3 (92.2-95.9) | 6.13 | 3.01 |  | 288.4 | 335 |
| L1-rk-sprint03 | novice / rookie / regular | `opp.sprint_share=0.3` | 600 | 94.5 (92.4-96.1) | 6.57 | 0 |  | 416.5 | 345 |
| L1-rk-red09 | novice / rookie / regular | `red_skill=0.9` | 600 | 91.2 (88.6-93.2) | 6.45 | 0 |  | 294.3 | 300 |
| L1-rk-poke14 | novice / rookie / regular | `opp.poke_frames=14` | 600 | 95.5 (93.5-96.9) | 6.14 | 0 | 11.7 | 269.5 | 352 |
| L2-pro-press08 | regular / pro / regular | `opp.hk_pressure_s=0.8` | 600 | 80.3 (77-83.3) | 1.66 | 7.69 | 9.5 | 434.6 | 274 |
| L2-pro-better8 | regular / pro / regular | `opp.hk_better_by_m=8` | 600 | 77 (73.5-80.2) | 1.87 | 7.88 | 10.4 | 458.4 | 267 |
| L2-pro-mincarry40 | regular / pro / regular | `opp.hk_min_carry_frames=40` | 600 | 82.7 (79.4-85.5) | 2.04 | 7.34 | 10.3 | 427.7 | 294 |
| L2-pro-red115 | regular / pro / regular | `red_skill=1.15` | 600 | 74 (70.3-77.3) | 1.43 | 7.63 | 9.3 | 459.2 | 253 |
| L2-pro-speed11 | regular / pro / regular | `speed_scale=1.1` | 600 | 72.2 (68.4-75.6) | 1.97 | 7.47 | 9.7 | 468.8 | 240 |
| L2-pro-combo | regular / pro / regular | `red_skill=1.1;opp.hunt_radius_m=8;opp.check_min_closing=2` | 600 | 67.3 (63.5-71) | 1.59 | 9.56 | 10.7 | 478.7 | 255 |
| L3-rk-combo | novice / rookie / regular | `red_skill=0.9;opp.hk_share=0.85;opp.hk_late_frames=8;opp.check_min_closing=6;opp.sprint_share=0.3` | 600 | 87.8 (85-90.2) | 6.36 | 3.79 | 14.8 | 480.1 | 260 |
| L3-pro-shoot30 | regular / pro / regular | `opp.shoot_extra_m=30` | 600 | 66.7 (62.8-70.3) | 1.56 | 7.07 | 13.6 | 399.2 | 286 |
| L3-pro-press20 | regular / pro / regular | `opp.hk_pressure_s=2.0` | 600 | 86.2 (83.2-88.7) | 1.62 | 7.54 | 8.8 | 417.7 | 290 |
| L3-pro-charge18 | regular / pro / regular | `opp.shot_charge_frames=18` | 600 | 85.3 (82.3-87.9) | 1.73 | 7.53 | 6.8 | 418.4 | 291 |
| L3-as-speed135 | good / all_star / regular | `speed_scale=1.35` | 600 | 63.5 (59.6-67.3) | 0.81 | 7.71 | 11.3 | 541.1 | 312 |
| L3-as-huntsprint | good / all_star / regular | `opp.hunt_radius_m=14;opp.sprint_share=1` | 600 | 54.8 (50.8-58.8) | 0.58 | 7.79 | 10.8 | 593.1 | 279 |
| L3-as-press20 | good / all_star / regular | `opp.hk_pressure_s=2.0` | 600 | 64.3 (60.4-68.1) | 0.56 | 7.61 | 10.4 | 505.2 | 315 |
| L4-as-charge6 | good / all_star / regular | `opp.shot_charge_frames=6` | 600 | 60.2 (56.2-64) | 0.85 | 7.78 | 18.7 | 530.9 | 303 |
| L4-as-yieldnever | good / all_star / regular | `opp.forecheck_yield_frames=-1` | 600 | 61.5 (57.5-65.3) | 0.4 | 7.34 | 10 | 499.3 | 336 |
| L4-as-nopress | good / all_star / regular | `opp.hk_read_human_pressure=0` | 600 | 66.8 (63-70.5) | 0.63 | 7.33 | 10.2 | 486 | 321 |
| L4-pro-A | regular / pro / regular | `opp.shoot_extra_m=30;red_skill=1.1;opp.hunt_radius_m=8;opp.check_min_closing=2` | 600 | 47.2 (43.2-51.2) | 1.45 | 8.32 | 16.3 | 407.2 | 231 |
| L4-pro-yieldnever | regular / pro / regular | `opp.forecheck_yield_frames=-1` | 600 | 81.3 (78-84.2) | 1.65 | 7.61 | 8.8 | 422.6 | 301 |
| L4-pro-charge4 | regular / pro / regular | `opp.shot_charge_frames=4` | 600 | 88.3 (85.5-90.7) | 2.1 | 7.71 | 13.8 | 441.9 | 286 |
| L4-rk-combo95 | novice / rookie / regular | `red_skill=0.95;opp.hk_share=0.85;opp.hk_late_frames=8;opp.check_min_closing=6;opp.sprint_share=0.3` | 600 | 82.8 (79.6-85.6) | 7.25 | 3.83 | 15.7 | 494.4 | 237 |
| L5-as-shoot25 | good / all_star / regular | `opp.shoot_extra_m=25` | 600 | 69.3 (65.5-72.9) | 0.84 | 7.85 | 7.6 | 539.4 | 328 |
| L5-as-draw0 | good / all_star / regular | `opp.draw_jump_pct=0` | 600 | 61 (57-64.8) | 0.66 | 7.52 | 10.5 | 507.5 | 335 |
| L5-as-combo | good / all_star / regular | `opp.hunt_radius_m=14;opp.sprint_share=1;opp.shoot_extra_m=25;opp.draw_jump_pct=0` | 600 | 67.2 (63.3-70.8) | 0.79 | 8.18 | 7.5 | 620.9 | 320 |
| L6-as-C1 | good / all_star / regular | `opp.hunt_radius_m=14;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8` | 600 | 43 (39.1-47) | 0.74 | 7.74 | 12.3 | 594.9 | 282 |
| L6-as-C2 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2` | 600 | 49.7 (45.7-53.7) | 0.82 | 8.05 | 11.2 | 606.9 | 280 |
| L6-as-C3 | good / all_star / regular | `opp.hunt_radius_m=14;opp.sprint_share=1;red_skill=0.9` | 600 | 52.3 (48.3-56.3) | 0.68 | 8.12 | 11.2 | 609 | 284 |
| L7-rk-A | novice / rookie / regular | `red_skill=0.95;opp.check_min_closing=6;opp.sprint_share=0.2` | 600 | 88 (85.2-90.4) | 7.4 | 3.67 | 15.4 | 396.1 | 255 |
| L7-rk-B | novice / rookie / regular | `red_skill=1.0;opp.check_min_closing=6;opp.sprint_share=0.2` | 600 | 84.8 (81.7-87.5) | 6.94 | 3.82 | 15.9 | 408.2 | 258 |
| L8-as-D1 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2` | 600 | 42.8 (38.9-46.8) | 0.62 | 8.12 | 12.4 | 602.4 | 248 |
| L8-as-D2 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;red_skill=0.9` | 600 | 37.8 (34-41.8) | 1.01 | 8.49 | 13.1 | 625.4 | 265 |
| L8-as-D3 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;speed_scale=1.05` | 600 | 46.7 (42.7-50.7) | 0.85 | 7.81 | 12 | 584 | 298 |
| L9-as-E1 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;red_skill=0.8` | 600 | 51.3 (47.3-55.3) | 1.08 | 8.36 | 12.1 | 620.2 | 268 |
| L9-as-E2 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;red_skill=0.9` | 600 | 37.7 (33.9-41.6) | 0.94 | 7.66 | 12.7 | 571 | 240 |
| L9-as-E3 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.check_min_closing=2;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;red_skill=0.8` | 600 | 45.7 (41.7-49.7) | 0.88 | 8.29 | 12.7 | 627.8 | 259 |
| L10-as-F1 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10` | 600 | 39.3 (35.5-43.3) | 0.93 | 8.57 | 16.2 | 632.4 | 235 |
| L10-as-F2 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;red_skill=0.9;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20` | 600 | 41.3 (37.5-45.3) | 1.06 | 8.35 | 12.3 | 610 | 270 |
| L10-as-F3 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=8;opp.hk_pressure_s=0.8;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20` | 600 | 36.7 (32.9-40.6) | 1.13 | 8.38 | 16 | 627.1 | 246 |
| L11-as-G1 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20` | 600 | 32.3 (28.7-36.2) | 1.31 | 8.3 | 16.7 | 617.3 | 230 |
| L11-as-G2 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20;speed_scale=1.4` | 600 | 25.8 (22.5-29.5) | 0.8 | 8.06 | 17.1 | 617.7 | 213 |
| L11-as-G3 | good / all_star / regular | `opp.hunt_radius_m=20;opp.sprint_share=1;opp.hk_better_by_m=12;opp.hk_pressure_s=0.5;opp.check_min_closing=2;red_skill=0.9;opp.shot_charge_frames=10;opp.forecheck_yield_frames=-1;opp.lift_cooldown_frames=20;opp.shoot_extra_m=50` | 600 | 30.8 (27.3-34.6) | 0.89 | 8.26 | 18.3 | 614.6 | 230 |

## 6. The ladder as set (the profiles, `DifficultyProfile.cpp`, `JamAICore.h`)

Every number this lane changed, with the value before (main bb18439); `JamAI::PreA95Tune(level)` holds the "before" column as a tune text, and the two
neutral-proof tests apply it.

| level | number | before | after | what it means (R-0041, R-0045) |
|---|---|---|---|---|
| rookie | red skill (regular / large) | 0.8 / 0.97 | **0.97 / 0.89** | the opposing side's pace |
| rookie | checks from (closing m/s) | never | **6** | "rarely hits": 3.6 checks taken a match by the novice (pro 8.1) |
| rookie | sprint share | 0 | **0.2** | sprints little |
| rookie | principles (share, late) | 0.6, 24 frames | unchanged | knows them, late and loose |
| pro | shot range (reach +) | 13 m | **30 m** | a pro bot shoots when it has the shot |
| pro | red skill (regular / large) | 0.95 / 0.97 | **1.07 / 1.04** | |
| pro | hunt radius | 0 | **8 m** | checks and lifts as a matter of course |
| pro | checks from | 4 m/s | **2 m/s** | under A92's model a low-speed press is a stumble |
| pro | sprint share / lift cooldown | 0 / none | **0.35 / 90 frames** | |
| all_star | speed (regular / large) | 1.2 / 1.2 | **1.4 / 1.22** | |
| all_star | red skill | 1.0 | **0.9** | (pace costs stability: 1.0 and 0.8 both read easier) |
| all_star | hunt radius | 10 m | **20 m** | hunts the carrier |
| all_star | checks from | 3 m/s | **2 m/s** | a beating |
| all_star | sprint share / lift cooldown | 0 / none | **1.0 / 45 frames** | sprints on every chase |
| all_star | better placed by / pressure arrives inside | 3 m / 1.4 s | **12 m / 0.3 s** | executes at once: carries through the forecheck instead of passing into it |
| all_star | shot wind-up | 18 frames | **10** | |
| all_star | forecheck yield | 12 frames | **never** | |
| the human's mates (every level) | sprint share | 0 | **0.35** | a teammate's sprint too |

## 7. The final table (2,000 matches per cell; hockey.regular on A53's seeds, hockey.arena.large seeds 1-2,000; every number as built)

Each final row was played on the profile with the last step given as its tune (the same numbers the build carries: `red_skill` / `speed_scale` / `opp.hk_pressure_s`
override after the profile and the large-rink offsets), then the profiles were written and built, and the gate re-run on them (section 9).

| cell | target | hockey.regular | hockey.arena.large | checks taken a match (reg / large) | fwo % (reg) | opponent shots (reg) |
|---|---|---|---|---|---|---|
| novice at rookie | 85 | **85.3 (83.6-86.7)** `F2-rk-red097` | **86.0 (84.4-87.5)** `F8-rk-large` | 3.63 / 2.30 | 7.26 | 15.3 |
| regular at pro | 50 | **48.3 (46.1-50.5)** `F2-pro-red107` | **48.4 (46.2-50.5)** `F5-pro-large` | 8.11 / 3.69 | 1.54 | 15.7 |
| good at all_star | 25 | **26.3 (24.4-28.2)** `F4-as-press03` | **24.9 (23.1-26.8)** `F6-as-large` | 7.82 / 3.90 | 0.73 | 17.3 |

Every cell is inside its 95 % interval of the R-0029 target on both rinks. The way there at 2,000 (each step one number): rookie regular 82.7 (red 1.0) -> 85.3 (0.97);
pro regular 46.0 (red 1.1) -> 48.3 (1.07); all_star regular 27.9 (speed 1.4, pressure 0.5 s) -> 30.7 (speed 1.5: saturated, back to 1.4) -> 26.3 (pressure 0.3 s; the
other arm, the lift cooldown 30, read 26.7 but would have undone B's cooldown); large: the regular profile played 62.0 / 82.8 / 5.7 (600), then rookie red 0.86 93.0,
0.89 89.0, 0.92 75.1, **0.90 86.0**; pro red 1.06 41.5, **1.04 48.4**; all_star speed 1.25 19.2, 1.22 23.9 (pressure 0.5) / **24.9 (0.3)**. The large rink is far steeper
than the regular one (a hundredth of red skill is 1.3 to 4 points there).

What the ladder now is (R-0041): rookie hits a novice 3.6 times a match (pro 8.1, all_star 7.8 on the good player who dodges more), sprints in a fifth of its chases
and plays the principles late and loose; pro shoots when it has the shot, hunts within 8 m, checks from 2 m/s and lifts every 1.5 s at most; all_star hunts within
20 m, sprints on every chase, carries through the forecheck, never yields the puck, and is the fastest side on the ice (1.4). The bots sprint 6.5 (rookie), 6.7 (pro)
and 9.9 (all_star) minutes of bot-time a match (summed over the nine bots) against none on main bb18439 (`game.ai.sprint` absent = never).

## 8. Job 5: the budgets from the soak

- `Jam.Bot.HumanHasSomethingToDo` (novice at rookie, hockey.regular, 20 seeds): the soak's share of faceoffs without a human touch inside 600 frames on the final
  rookie profile is **7.26 %** (2,000 matches, `F2-rk-red097`); budget = share + 2 points (R-0039) = **9.3 %** (`Faceoffs * 93 / 1000`, was 15.1 %).
- `Jam.Playtest.NoviceScoresAtRookie`: the soak's share of matches with a novice goal inside 7,200 frames at rookie is **44.8 %** (896 of 2,000), under the 50 % that
  10 of 20 asks, so the bar is that share less one standard error of a 20-seed count (20 x 0.448 = 9.0, SE 2.2): **7 of 20** (was 10). The rookie that the novice
  beats 85 % of the time scores later than the old one: the checks from 6 m/s and the sprint take the novice's early chances.
- `Jam.Bot.SeasonTakes`: as the Director left it (the AI-shot clause read over a take's three seeds).

## 9. Job 6: the gate and the acceptance run on the final numbers

- Full `Jam` filter on the built final profiles (`Game/logs/a55-automation-ue-20261113-183303.log`): **156 passed, 0 failed** (155 of main's and A92 / A93's
  tests plus `Jam.Core.LiftCone`). `HumanHasSomethingToDo` 11 of 131 (budget 9.3 %: 12); `NoviceScoresAtRookie` 11 of 20 (need 7); `SeasonTakes` green (take-3, pro
  or harder: the scripted human won 2 of 3); `MovementNeutral` and `ContactNeutral` on bb18439's hashes; `ModeKeys.Sprint` green with the levels' shares.
  Determinism (seed 1, frame 600): hockey.regular `d192973ac7c97102`, hockey.raw `b02f67a2da0314b4` (bb18439: `bd698403ed205fbd`, `0e5637bb401646ae`; the
  profiles do not reach frame 600's hash, A92's and A93's rules do).
- Development package of the final tree (`Game/Packaged/A95-dev`, 76 s) and `accept.ps1 -Name a95-final -Port 4695`, no warning: **19 ok, 0 fail**
  (`tools/qa/out/accept/a95-final/verdict.txt`): brake 32 frames to still, sprint_carry, shoot_full, check (down 134, ratio 2.64), **crosscheck** (stick hit,
  puck free, down 74, ratio 1.33, closing 4.96), checked (down 86), deke / spin / sweep, resume_aim.

## 10. What went back to neutral

Nothing. Every rule of A92 and A93 ships at its default; no key was moved to make a test green. The changes outside the profiles are test fixes with a cause:
the two neutral proofs re-pinned (section 1), `Jam.Core.Stick`'s lifter turned into the new cone, `ModeKeys.Sprint`'s two absent-key assertions (the levels now
sprint; the human sprints only with the puck without `-playersprint`), and the two budgets (section 8).

## 11. For the Director

1. **The shot range at pro** (13 -> 30 m) is outside the brief's lever list and is the largest single move (-15.6 points): A94's carry-or-pass halved the bots'
   shots, and a pro that will not shoot is not executing. Reverting it puts pro back near 65 % for the regular player.
2. **all_star's carry-or-pass numbers** (better placed by 12 m, pressure 0.3 s) mean an all_star carrier hardly passes away from pressure any more: it carries
   through the forecheck, sprinting. That is what beat the good player; it is also less of A94's "passes when pressure arrives". The principle is on; the level
   executes it late on purpose. If that reads wrong on screen, the alternative found was hunt + sprint + red 0.9 at about 38 % (600).
3. **Speed and red skill are not monotonic at all_star** (speed 1.35: flat, 1.5: easier than 1.4; red 0.8 and 1.0 both easier than 0.9): pace costs stability
   under A92 and A93 (a sprinting, faster bot is easier to knock down and carries its puck farther out). The ladder's top is set on measured points, not a slope.
4. **The large rink is far steeper than the regular one** and needed its own offsets on every level (all three were far off with the regular profile: 62 / 83 / 6).
5. **The bots brake about 1,700 times a match** with perfect reversals (A93's finding); untouched here. The human's mates sprint like pro bots at every level.
6. **B-to-A-0037**: the cone and `Dir` are in the sim and the binder; the bots' lift cooldown is pro 90 / all_star 45 frames (rookie never lifts). B should see no
   lift on a victim behind the attacker's shoulder and no bot lifting twice inside 0.75 s.
7. Contracts not edited: `events.md` could say `lift` and `poke` now carry `Dir`; `game.stick.lift_cone_deg` is in the schema docs.

## 12. Files (relative to the worktree; no git command was run)

- Sim and config: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h`, `Public/JamCartridge.h`, `Private/JamCartridge.cpp` (the cone; `Dir` on lift / poke).
- Generated (the three generators): `Game/Source/JamCore/Private/JamSchemaData.inl`, `Game/Source/JamCore/Tools/parity.json`,
  `Game/Source/JamDesigner/Private/JamDesignerData.inl`. Schema: `schema/game.schema.json`, `schema/limits.json`, `schema/DEFAULTS.md`, `schema/README.md`.
- Brain and profiles: `Game/Source/JamCore/AI/DifficultyProfile.cpp`, `DifficultyProfile.h`, `JamAICore.h`, `JamAICore.cpp`, `JamSimAdapter.cpp`, `JamBrain.h`.
- Playtester: `Game/Source/JamCore/AI/JamPlaytester.cpp`, `JamPlaytester.h`.
- Effects: `Game/Source/JamGame/Private/EffectsBinder.cpp`.
- Tests: `Game/Source/JamCore/Private/Tests/JamContactTests.cpp` (`ContactNeutral` re-pinned, `LiftCone` new), `JamMovementTests.cpp` (`MovementNeutral`
  re-pinned), `JamStickTests.cpp` (the lifter's facing), `Game/Source/JamCore/AI/ModeKeysTests.cpp` (`ModeKeys.Sprint`), `Game/Source/JamCore/Tests/BotTests.cpp`
  (`HumanHasSomethingToDo` budget), `JamPlaytester.cpp` (`NoviceScoresAtRookie` bar).
- Docs and measurement: `docs/experiments/CURVE-5.md`, `status/A95.md`, `tools/perf/results/a95/` (`curve.mjs`, `job3.sh`, `levers1.sh` .. `levers11.sh`,
  `final1.sh` .. `final5.sh`, `cells.jsonl`, `raw/`).
- By-products, not part of the change: `Game/Packaged/A95-dev`, `tools/size/history/20261113-171100-Development.json`, `20261113-183518-Development.json`,
  `Game/Build/Windows/FileOpenOrder/*.log`, `Game/logs/a55-*`, `Game/logs/build-*`, `Game/logs/package-*`, `tools/qa/out/accept/a95-run1`, `a95-final`.
