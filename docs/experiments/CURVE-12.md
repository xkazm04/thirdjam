# The difficulty curve, pass 12: the ladder after free skaters 15 % faster (lane A189, track C0 evolution 8, 2026-11-14 13:30-)

Scope (brief `briefs/A189-C0-evolution-8.md`, R-0064 point 5): the operator ordered free skaters 15 % faster (`skater.free_speed_mult` 1.15, MOVEMENT-10). Targets
85 / 48 / 26 within 3; law 3 a gate. Driver and rows: `tools/perf/results/a189/` (`curve.mjs` = A182's; `cells.jsonl`; raw matches under `raw/`, not landed).
Instrument: the ladder's (`JAM_PLAYTESTER_CHECK=closing`). Every step with its time: `status/A189.md`.

## Pass 1 on 1f4be055 (overtaken: knowledge only)

The base moved under the pass at 14:35 (the hits' wind-ups, the scripted human's brace/check mix, breakaway 0.3, brain7's true speeds). Its rows:
rookie regular red 0.85 / 0.80 / 0.75: 74.5 / 81.3 / 82.3 (red saturates); with red 0.80: poke 14 80.8, speed 0.80 82.0, shot reach +20 85.2 (2,000
built: 84.0), +15 89.7. All_star regular speed 1.48 / 1.5 / 1.52 / 1.55: 39.0 / 31.3 / 34.7 / 38.8 (no slope; 1.5 at 2,000: 34.0); 1.48 with the brain
dial (second man hunts, step-up 16 / 99) 28.8. All_star large speed 1.55 / 1.62: 31.5 / 26.3 (2,000 at 1.62: 24.6). Pro regular hunt 9-13 (presser
keys): 45.8 / 49.0 / 43.8 / 46.8 / 40.0 (2,000: hunt 11 44.7, hunt 10 49.6); pro large hunt 11 51.8 (2,000). The dial for all_star cannot be a profile
number today: `SetEvolution3` zeroes the opposing side's `StepUpM` in `ApplyDifficultyProfile`.

## LADDER SET 1 on 58af359f (wind-ups 4 / 4, breakaway 0.3, brain7 true speeds at pro and all_star)

| cell (target) | as built (600) | numbers | with them (600) | 2,000 as built with them |
|---|---|---|---|---|
| rookie regular (85) | 71.2 | red skill 0.97 -> **0.80**, shot reach +25 -> **+20** | 83.0 | **83.8** (82.2-85.4) |
| rookie large (85) | 80.5 | the reach (shared) + LargeRedSkill 1.14 -> **1.11** (1.09: 87.7) | 85.0 | **83.0** (81.3-84.6) |
| pro regular (48) | 46.7 | none | | not measured |
| pro large (48) | 56.5 | LargeRedSkill 1.15 -> **1.18** (1.21: 41.7) | 46.2 | **48.7** (46.5-50.9) |
| all_star regular (26) | 30.2 | none found (pass 1: speed has no slope; the brain dial is not a profile number) | | open |
| all_star large (26) | 26.7 | none | | |

Built = tune by seed: rookie regular, rookie large, pro large 600 of 600 each. Law 3 on the build: rookie 8 m 100 %, pro 94.0 (9.5 m 83.5), all_star 99.5; a straight charge 0 at every level. Full `Jam` filter on the build with the numbers, the measuring environment
cleared: 287 of 287. Neutral: `JamBaseline::BrainTune("rookie")` gains `red_skill=0.97` on the regular rink (its large tune already carries its own red);
the large-rink red skills are not tune keys of the baseline (no pin is a large-rink match at rookie or pro).

## LADDER SET 2 on 38766fc9 + the bots' wind-ups 8 / 12 (A200) + evolution 8 (A202, by `brain8=1`)

| cell (target) | wind-ups only (600) | + evolution 8 (600) | numbers moved | 2,000 with them |
|---|---|---|---|---|
| rookie regular (85) | 81.2 | 85.8 | none | **84.8** (83.1-86.3) |
| rookie large (85) | | 86.7 | none | **84.0** (82.4-85.6) |
| pro regular (48) | 53.3 | 41.7 | pro `Hk.PressureSec` 0.3 -> **0.6** (1.0: 50.8) | **46.8** (44.6-48.9) |
| pro large (48) | | 38.7 | the same + pro `LargeRedSkill` 1.18 -> **1.165** (1.15: 52.5) | **46.1** (43.9-48.3) |
| all_star regular (26) | 31.3 | 25.2 | none | **25.9** (24.0-27.8) |
| all_star large (26) | | 25.2 | none | **24.9** (23.1-26.9) |

Evolution 8 made the regular rink harder for the player at pro (-11.6 points) and all_star (-6.1) and easier at rookie (+4.6); it does not
touch law 3 (pro 94.0 / 83.5 / 60.0 / 35.0 / 34.5, straight 0 with and without it). Built = tune for the two pro numbers: 600 of 600 each.
All_star's brain dial is a profile number now (`ApplyDifficultyProfile` keeps a level's `Opp.Hk.StepUpM`; neutral 100 of 100 seeds a cell): on 38766fc9
before the wind-ups, second man alone -3.0, with step-up 16 m -7.4 on the regular rink, flat on the large; not needed in this set.
