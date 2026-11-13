# Night plan 2026-11-13 (01:05 CET): eleven fillers for the idle machine

The operator (01:00): "brainstorm 10 directions to get resource usage optimized by improving quality of the game or code. The RAM should be ideally in 30-40GB usage whole
night." All eleven accepted; landing policy: everything the Director judges (R-0024); packages per landed batch; model by task complexity. Lanes A51 (stick handling,
Fable) and A52 (lifted puck, Fable) run in parallel with the fillers. Editor cap three; headless and packaged games are not editors.

| # | Direction | Lane | Model | Starts | Needs | Report |
|---|---|---|---|---|---|---|
| 1 | Playtester soak: 3 classes x 3 levels x 40+ seeds, 8 headless games | A53 | Sonnet | 01:10 | - | `SOAK-PLAYTEST.md` |
| 5 | Eyes regression grid of the packaged build, repeated per package | A54 | Sonnet | 01:10 | H19 | `EYES-GRID.md` |
| 9 | Static analysis + build hygiene; safe fixes land | A55 | Opus | 01:10 | - | `STATIC-ANALYSIS.md` |
| 2 | Lockstep soak: hundreds of joined matches, loss injection | A56 | Opus | after A51 lands (protocol 5) | A51 | `SOAK-LOCKSTEP.md` |
| 10 | Development vs Shipping hash parity; hourly package + smoke + size | A57 | Sonnet | after A55's interim | - | `PARITY.md` |
| 8 | Unreal Insights traces per tier; hitch and memory lists | A58 | Opus | after A53's interim | - | `PROFILING.md` |
| 3 | Cartridge fuzz: limits, crashes, non-determinism | A59 | Opus | after A52 lands (puck keys) | A52 | `FUZZ.md` |
| 11 | Test coverage map; ten missing tests | A60 | Opus | after A51/A52 land | A51, A52 | `COVERAGE.md` |
| 4 | Bot self-play tuning: parameter search per level | A61 | Opus | after A53's first table | A53 | `SELFPLAY.md` |
| 6 | p3_clay light sweep with eyes against the P3 concept | A62 | Fable | when an editor slot is free (A51/A52 closed) | B19 merged | `LIGHT-SWEEP.md` |
| 7 | Highlight reel tool: clips and a 60 s reel from bot matches | A63 | Sonnet | after A54's interim | H19 | `HIGHLIGHTS.md` |

Machine budget: 30-40 GB resident. The Director's half-hourly `RESOURCES` line in `status/A.md` is the record; a filler is held when free memory is under 10 GB.
