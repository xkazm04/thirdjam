# The difficulty curve, pass 7: the ladder on the integrated tree, law 3 kept (lane A133, track C0 evolution 2, 2026-11-14 00:10-)

Scope (brief `briefs/A133-C0-evolution-2.md`; R-0053 to R-0055): main e44df14 (the kept puck, the take grace, the brain's second evolution, the shove rule,
the standing start, the spin's price) with lane A112's first curve applied on top (`CURVE-6.md`). Targets (R-0046): the novice at rookie 85, the regular
player at pro 48, the good player at all_star 26, on hockey.regular and hockey.arena.large, within 3 points; law 3 is a gate (R-0053 point 1) and the hunt
radius is not a curve lever (R-0055 point 1). Driver `tools/perf/results/a133/curve.mjs` (lane A112's; A53's seeds on hockey.regular, 1..n on the large
rink); rows in `tools/perf/results/a133/cells.jsonl` (raw matches in `raw/`, not landed). Status with timestamps: `status/A133.md`.

## 1. Law 3 under a candidate profile, before a build: `Bar.Curve.Law3`
A new test outside the gate's filter (`JamCarrierSkillTests.cpp`, end): law 3's carry drill (the stop begun at 8, 9.5, 10, 11, 12 m and the straight
charge, 200 seeds each) with `JAM_LAW3_TUNES="level:tune|level:tune"` applied on the brain; run by `tools/perf/results/a133/law3.sh <log> "<arms>"`. It lets a
curve number be held against the gate on the build that measures it.

| level | tune (on lane A112's profile) | 8 m | 9.5 | 10 | 11 | 12 | straight |
|---|---|---|---|---|---|---|---|
| pro | A112's profile (hunt 20, poke 2, checks 1) | **21.0** | 84.5 | 98.0 | 99.0 | 92.5 | 0 |
| pro | hunt 10 | 70.5 | 100 | 99.5 | 89.5 | 71.0 | 0 |
| pro | hunt 9 | 98.5 | 99.5 | 95.5 | 89.5 | 68.0 | 0 |
| pro | **hunt 8** | **100** | 97.0 | 94.0 | 83.5 | 56.5 | 0 |
| pro | hunt 8 + shot 40 / checks 0.5 / poke 0 / sprint 0.7-1 / lift cooldown 45 | 96.5 | 97.5 | 95.5 | 85.5 | 66.5 | 0 |
| pro | hunt 8 + speed 1.1 | 98.5 | **73.0** | 52.0 | 33.5 | 31.0 | 0 |
| pro | **hunt 8 + carry through pressure** (the profile) | **100** | 97.0 | 94.0 | 83.5 | 56.5 | 0 |
| all_star | hunt 30 (A112) / 25 / 20 | 0 | 39.5 | **57.5** | 89.5 | 100 | 0 |

Read: the hunt is the only pro number the drill sees inside 8 m (R-0055 confirmed: 10 m already fails); the shot, the checks, the poke, the sprint and the
lift cooldown leave it at 96.5 % or more; speed moves the window in (9.5 m 73 %, under lane A121's 95 % bar at 9.5 m): speed is out at pro on the regular rink.
All_star's hunt does not matter to the drill (its defender starts inside 20 m): 57.5 % from 10 m, the bar is 50.

## 2. The six cells on the tree as it stands (A112's profile, whistle off; 600 a cell)

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | 79.3 (75.9-82.4) | 99.5 (98.5-99.8) |
| regular at pro | 48 | 58.5 (54.5-62.4), law 3 broken (hunt 20) | 69.7 (65.9-73.2) |
| good at all_star | 26 | 24.3 (21.1-27.9) | 38.3 (34.5-42.3) |

(The integration lane's 98.2 / 67.8 / 39.3 and 99.8 / 79.5 / 82.0 were the same tree before A112's profile and with the whistle on; all_star is harder than pro
on the large rink once A112's profile is in: R-0055 point 3 holds from here on.)

## 3. The levers (600 a cell, `-aitune` on build 2; every row in section 6)

| level / rink | lever | win % | read |
|---|---|---|---|
| pro / regular | hunt back to 8 (law 3) | 74.0 | the price of law 3: +15.5 |
| pro / regular | **carry through pressure** (`opp.hk_pressure_s=0.3;opp.hk_better_by_m=12`, all_star's numbers: the carrier moves the puck only when an opponent arrives inside 0.3 s, passes only to a mate 12 m nearer the net) | **51.0** | the lever: the pro bot stops giving the puck back with a pass into the human's forecheck |
| pro / regular | shot reach 40 / 60 / 80 (30) | 66.5 / 60.5 / 53.3 | second lever; with the carry nothing more (51.3) |
| pro / regular | sprint 0.7 + lift 45; wind-up 6; mates pass 0.2; never yield; deke; pass out; all_star's sense; gap control; red 1.1 | 70.8; 71.0; 72.2; 76.5; 75.3; 73.5; 74.0; 74.0; 74.2 | flat: red skill is no lever on the regular rink |
| rookie / regular | poke after 8 / 10 frames (6) | 84.3 / **86.8** | aggression first: a later poke |
| rookie / regular | shot reach 15 / 18 (25) | 88.5 / 86.8 | the same reach as the poke |
| pro / large | carry (red 1.065); + red 1.1 / **1.13** / 1.16 | 69.8; 55.5 / **47.8** / 41.2 | the carry is no lever on 120 x 60: red skill is, steeply |
| rookie / large | poke 10 + red 1.03 / 1.08 / **1.12** / 1.16 | 97.3 / 91.3 / **84.7** / 75.7 | |
| all_star / large | speed 1.38 / **1.42** (1.34) | 33.8 / **27.5** | speed last, as the brief orders; the only large-rink number all_star has |

## 4. The profile as written (`DifficultyProfile.cpp`, `JamAICore.h` ProOppSkill)

| level | number | before (A112) | after |
|---|---|---|---|
| rookie | poke after | 6 frames | **10** |
| rookie | large-rink red skill | 0.98 | **1.12** |
| pro | hunt radius | 20 m | **8 m** (law 3, R-0055) |
| pro | carry through pressure (`Hk.PressureSec` / `Hk.BetterByM`, the opposing side only; the human's mates keep 1.4 s / 3 m) | 1.4 s / 3 m | **0.3 s / 12 m** |
| pro | large-rink red skill | 1.065 | **1.13** |
| all_star | large-rink speed | 1.34 | **1.42** |

Unchanged from A112: pro poke 2, checks from 1 m/s; rookie shot reach 25; all_star hunt 30, shot reach 60; every level's mates at the human's pace.
Neutral values: `JamAI::PreA95Tune` (the levels before the vision) gains `opp.hk_pressure_s=1.4;opp.hk_better_by_m=3` for pro; `JamBaseline::BrainTune(Level,
bLargeRink)` carries each level's numbers before lanes A112 and A133 (the pinned-hash tests' one helper, R-0055 point 7), and `JamBaseline::WithWhistle(G)` the
cuts' whistle before lane A112 (content, not a rule: a pin made on a shipped cut calls it after `MakeNeutral`).

## 5. The six cells as built, 2,000 a cell (build 3; pro large on build 4)

| cell | target | hockey.regular | hockey.arena.large |
|---|---|---|---|
| novice at rookie | 85 | **85.3** (83.6-86.7) | **84.8** (83.2-86.3) |
| regular at pro | 48 | **49.1** (47.0-51.3) | **49.9** (47.7-52.1) at red 1.12 (44.9 at 1.13) |
| good at all_star | 26 | 29.6 (27.6-31.6): **3.6 over** | **26.4** (24.5-28.4) |

No match unfinished. All_star is harder than pro on both rinks (R-0055 point 3). Five cells within 3 points; all_star on the regular rink is 3.6 over
(inside the 4-point bar of `EVOLUTION-COMMON.md`). It read 24.3 at 600 (A53's first six seed bands) and 29.6 at 2,000: the seed sample, as lane A112 saw
(29.7 -> 32.0). Its remaining levers at 2,000: lift cooldown 20: 29.6; carry through pressure 0.15 s: 29.5; red skill 0.95: 28.9 (all inside each other's
intervals; not taken: a number that passes a threshold on noise is not a re-set). Lane A112 found the aggression levers saturated there and speed at its
optimum (1.4): the cell is the brain's (section 7).

## 6. The gate on build 4

- Full `Jam` filter: **206 pass, 0 fail** (`Game/logs/a133/jam-3.log`). Node tests: 243 pass, 1 fail = `BarkData.inl` against `barks.json`, the worktree's
  CRLF copy that lanes A102, A121 and A138 reported (not this lane's).
- Law 3 (`Jam.Core.Movement.StopAndGo`, R-0053): pro, the stop begun at 8 m **200 of 200**, the straight charge 0; the stop at 9.5 m 97.0 % past, 99.0 % kept;
  all_star 0 / 39.5 / 89.5 / 100 % from 8 / 9.5 / 11 / 12 m (bar 50 where its window is), straight 0.
- Determinism: hockey.regular seed 1 frame 600 `ef34dd88c7c62b47` (lane A138's; a profile number moves bot matches, not the sim's own step).
- Lane A112's seven reds: `ContactBoardMatch` (its neutral arm now gets the pro numbers before A112 through `JamBaseline::BrainTune`), `CutsRoundTrip` and
  `PromptParity` (cached cuts rebased onto the whistle-off hockey.regular by `tools/perf/results/a133/rebase-cuts.mjs`, then `gen-designer-data.mjs`),
  `SeasonRoundTrip` (A112's source fix, built), `ModeKeys.Sprint` and `Effects.BotMatchWithEffects` (green on the integrated tree with no edit: their red
  was A112's own tree), `StopAndGo` (pro's hunt back to 8 m). New on the integrated tree and fixed: `Jam.Bot.Hockey2.Neutral` and `Jam.Core.MovementNeutral`
  (their pins blew the whistle: `JamBaseline::WithWhistle`; the large pin also needs pro's large red 1.04: `BrainTune(Level, bLargeRink)`),
  `Jam.Designer.ReplyParity` (the checked-in corpus `tools/fuzz/replies/manifest.json` rebased by `rebase-replies.mjs`: 41 of 56 texts `penalties` true ->
  false, Node re-judged all 56, no verdict changed), `Jam.Core.Movement.LateAnswer` and `.SpinPrice` (law 3's drill: the hunt), and
  `Jam.Effects.BotMatchWithEffects` again on build 3 (hockey.boost is 120 m and plays all_star: at large speed 1.42 seeds 1 to 3 end 3:0 near frame 2,300,
  before a pickup is taken; its seed fallback now runs to 12, seed 4 takes 2: the property, not a number, is the test).
- Tests re-pinned: none. Helpers added to: `JamBaseline.h` (`BrainTune(Level, bLargeRink)`, `WithWhistle`).

## 7. Revert and neutral
Every number of this pass is a profile number: `PreA95Tune` and `JamBaseline::BrainTune` give back the levels before it as a tune (proved by the neutral tests
above, green). To revert one level, restore its line in `DifficultyProfile.cpp` / `JamAICore.h` (the "before" column of section 4).
