# SOAK-2: the whole game on the vision (lane A107, track S1, device A, 2026-11-13 18:48-)

Build: main ee14438 (the commit with contact by inertia, the movement laws, the hockey brain, the ladder of R-0046), run from the built checkout `.claude/worktrees/clean-main`
(same commit) through headless `UnrealEditor-Cmd -run=JamTournament`. No source edited. Driver: `tools/soak/a107/cells.mjs` (a memory-governed pool: starts a chunk only below
48 GB in use, up to 16 workers, faults per seed with the command that repeats them, requeue after a crash), plans from `mkplan.mjs`, tables from `agg.mjs`, `modes-agg.mjs`, `mkdoc.mjs`.
Raw data: `tools/perf/results/a107/raw/` (does not travel).

## 0. The headless start-up stall and its flag (read first)

Every `UnrealEditor-Cmd` start runs `LaunchUBTValidatePlatformsProc` (`Engine/Source/Developer/TargetPlatform/Private/TargetPlatformManagerModule.cpp:366`, called at 408),
a `Build.bat -Mode=ValidatePlatforms` that takes the UBT mutex. While any lane builds, every commandlet start waits behind it: with 16 workers the first batch ran at about
one chunk per 2-3 minutes (7 commandlets, mine and A100's, sat 3+ minutes after the "TurnkeySupport" log line with 2 s of CPU each; the smoke run of 3 matches took 5 m 17 s).
The engine skips the call when the command line has **`-Multiprocess`** (line ~397). With it: the same 3 matches took 9.5 s, the hashes are identical to the unflagged run
(`a31f6bc64fef07f3`, `1e3bb78feb1e30ad`, `71b35cafd66fe597`), and every batch below ran at full speed while other lanes built. All A107 runs pass it.

## 1. (a) The nine cells on two rinks, 2,000 matches each (36,000 matches, 0 crashes, 0 ensures, 0 error lines, 0 missing seeds)

Seeds: hockey.regular on A53's band (1-100 of each 150), hockey.arena.large 1..2000. Targets: R-0046 (85.3 / 48.3 / 26.3 on both rinks).

| rink | class | level | n | win % | 95 % interval | target | off by | human knockdowns / match | all knockdowns / match | matches with a 30 s no-touch stretch | longest no-touch (s) | match s |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| regular | novice | rookie | 2000 | 85.3 | 83.6-86.7 | 85.3 | 0.0 | 3.63 | 9.2 | 598 (30 %) | 72 | 366 |
| regular | regular | rookie | 2000 | 99.5 | 99.1-99.8 | none |  | 4.08 | 8.1 | 2 (0 %) | 31 | 179 |
| regular | good | rookie | 2000 | 100 | 99.8-100 | none |  | 3.17 | 8.1 | 0 (0 %) | 28 | 120 |
| regular | novice | pro | 2000 | 15.4 | 13.9-17.1 | none |  | 4.15 | 11.6 | 1117 (56 %) | 97 | 329 |
| regular | regular | pro | 2000 | 48.3 | 46.1-50.5 | 48.3 | 0.0 | 8.11 | 17.1 | 112 (6 %) | 60 | 292 |
| regular | good | pro | 2000 | 79.8 | 78-81.6 | none |  | 7.47 | 18.1 | 15 (1 %) | 40 | 214 |
| regular | novice | all_star | 2000 | 2.6 | 2-3.4 | none |  | 3.28 | 9.1 | 976 (49 %) | 91 | 263 |
| regular | regular | all_star | 2000 | 8.8 | 7.6-10.1 | none |  | 6.5 | 13.7 | 91 (5 %) | 48 | 256 |
| regular | good | all_star | 2000 | 26.3 | 24.4-28.2 | 26.3 | 0.0 | 7.82 | 19.1 | 31 (2 %) | 43 | 247 |
| large | novice | rookie | 2000 | 86 | 84.4-87.5 | 85.3 | 0.7 | 2.3 | 4.7 | 222 (11 %) | 64 | 207 |
| large | regular | rookie | 2000 | 97 | 96.2-97.7 | none |  | 2.59 | 6.2 | 14 (1 %) | 45 | 199 |
| large | good | rookie | 2000 | 99.4 | 99-99.7 | none |  | 3.84 | 9.1 | 15 (1 %) | 40 | 201 |
| large | novice | pro | 2000 | 13.3 | 11.8-14.8 | none |  | 1.79 | 5.1 | 1936 (97 %) | 276 | 224 |
| large | regular | pro | 2000 | 48.4 | 46.2-50.5 | 48.3 | 0.1 | 3.69 | 8.7 | 1521 (76 %) | 116 | 218 |
| large | good | pro | 2000 | 80.9 | 79.1-82.6 | none |  | 4.21 | 10.7 | 1023 (51 %) | 100 | 205 |
| large | novice | all_star | 2000 | 2.2 | 1.6-2.9 | none |  | 1.64 | 4.1 | 1910 (96 %) | 180 | 201 |
| large | regular | all_star | 2000 | 5 | 4.1-6 | none |  | 3.64 | 8.3 | 1565 (78 %) | 122 | 203 |
| large | good | all_star | 2000 | 24.9 | 23.1-26.8 | 26.3 | -1.4 | 3.9 | 11.2 | 1040 (52 %) | 109 | 205 |

The six target cells: all inside the 4-point rule. Regular rink 85.3 / 48.3 / 26.3 (to the decimal; A95's numbers); large 86.0 (+0.7) / 48.4 (+0.1) / 24.9 (-1.4).
The twelve cells with no target (six per rink), as played now: regular class at rookie 99.5 / 97.0 (no contest), good at rookie 100 / 99.4, **good at pro 79.8 / 80.9**,
regular at all_star 8.8 / **5.0 (large)**, novice at pro 15.4 / 13.3, novice at all_star 2.6 / 2.2. Going from regular to good gains 31 points at pro and 17 at all_star
on the regular rink; from novice to regular 33 at pro but only 6 at all_star.

Determinism: regular/pro seeds 1-100 and large good/all_star seeds 1-100 re-run in a separate process: 100 / 100 and 100 / 100 hashes equal.

## 2. (d) From every match: knockdowns and the longest stretch without a human touch

The human is knocked down 8.1 times per match (regular class, pro, regular rink: one every 36 s) and 3.7 on the large rink; at all_star 6.5 and 3.6; novice 3.3-4.2 and 1.6-2.3.
The playtest line has `knockdowns` (the human) and `knockdowns_all`; it has **no stumble counter**, so stumbles per match cannot be reported.
Matches past the horn in the nine cells: 0. The per-frame trace (lane A71's `idle.mjs`, 100 seeds, regular class, pro unless stated):

| cartridge | matches with a stretch >= 30 s | p50 / p90 / max longest stretch (s) | who held the puck in the long stretches |
|---|---|---|---|
| hockey.regular rookie | 0 % | 12 / 18 / 23 | loose 57 % |
| hockey.regular pro | 5 % | 19 / 29 / 37 | loose 52 %, opponents 39 % |
| hockey.regular all_star | 3 % | 18 / 26 / 41 | loose 54 %, opponents 36 % |
| hockey.arena.large pro | **71 %** | 38 / 63 / 108 | **opponents 73 %**, a teammate 2 %, loose 24 % |
| hockey.arena.wide pro | **91 %** | 49 / 73 / 121 | **opponents 81 %**, a teammate 1 % |
| take-7 (wide) pro | 54 % | 31 / 47 / 82 | loose 54 %, opponents 44 % |

On the big ice the opposing team holds the puck for most of a half minute and a teammate never does in the long stretches (0 of 16 neighbourhood stretches in each).

## 3. (b) The twelve modes, 200 seeds each, regular class at the mode's level, against hockey.regular at the same level (same seeds)

| mode | level | character number | mode | regular | ratio | differs by a third | win % mode / regular | unfinished | human knockdowns / match |
|---|---|---|---|---|---|---|---|---|---|
| arsenal | rookie | human abilities used / match | 0.94 | 0 | n/a | yes | 93.5 / 100 | 0 | 3.57 |
| bruisers | rookie | hits (landed + taken) / play min | 4.87 | 3.2 | 1.52 | yes | 99.5 / 100 | 0 | 7.04 |
| cage | pro | checks landed / match | 5.06 | 5.88 | 0.86 | **NO** | 25 / 54.5 | 0 | 7.39 |
| fog-of-war | pro | human breakaways / play min | 1.86 | 0.36 | 5.1 | yes | 95 / 54.5 | 0 | 5.45 |
| glass-house | pro | human box seconds / match | 12.11 | 5.69 | 2.13 | yes | 19 / 54.5 | 0 | 4.32 |
| marathon-sprint | pro | human sprint frames / match | 2986.36 | 996.74 | 3 | yes | 2.5 / 42 | 20 | 11.57 |
| marathon | pro | human sprint frames / match | 285.2 | 105.21 | 2.71 | yes | 1 / 54.5 | 14 | 11.63 |
| outnumbered | rookie | shots against / play min | 3.06 | 2.51 | 1.22 | **NO** | 99.5 / 100 | 0 | 4.38 |
| pond | rookie | passes + received / play min | 1.66 | 4.66 | 0.36 | yes | 77.5 / 100 | 0 | 0 |
| power-play | pro | passes + received / match | 7.63 | 16.08 | 0.47 | yes | 45.5 / 54.5 | 0 | 6.47 |
| shotgun | pro | knockdowns (all skaters) / match | 12.03 | 17.82 | 0.67 | **NO** | 22.5 / 54.5 | 0 | 4.17 |
| sniper | pro | human goals per human shot | 0.43 | 0.17 | 2.6 | yes | 14 / 54.5 | 0 | 3.08 |
| sudden-death | all_star | match seconds | 51.53 | 260.51 | 0.2 | yes | 11.5 / 8.5 | 0 | 0.78 |

- **Three modes no longer differ by a third: cage (0.86), outnumbered (1.22), shotgun (0.67).** Cage's character is the checks the human lands (5.06 against 5.88): under the
  contact law ordinary play lands as many hits as the mode does. Shotgun's "every skater within a metre of its path goes down" does fewer knockdowns (12.0) than regular play
  makes by itself (17.8), because regular play now knocks 17 skaters down per match.
- Arsenal's regular value is 0 by construction (regular has no abilities), so it differs trivially.
- **Marathon**: the timer is 36000 frames, equal to the harness cap `-maxframes=36000`, so the default run reads 200 of 200 unfinished (win 0). With a cap of 40000: 184 losses, 2 wins,
  14 unfinished; those 14 re-run to 200000 frames all end between 40010 and **42000 (two exactly at 42000)**: the match plays up to 6000 frames (100 s) past its own horn. The regular
  class wins 1 % (2.5 % with sprint) against 54.5 % on regular: the mode is not winnable by the playtester (the bots sprint; 30 knockdowns per match, 11.6 of them the human's).
- Wins against regular's 54.5 / 100: sniper 14, glass-house 19, shotgun 22.5, cage 25 (low); arsenal 93.5, bruisers 99.5, fog-of-war 95, outnumbered 99.5 (high).
- Pond (rookie) has zero knockdowns by design; every other mode has 3 to 11.6 human knockdowns per match.

## 4. (c) The season's takes, regular class at the take's level, 200 seeds each

| take | level | win % | 95 % interval | unfinished | human knockdowns / match | matches with a 30 s no-touch stretch | longest (s) | match s |
|---|---|---|---|---|---|---|---|---|
| take-1 | rookie | 98 | 95-99.2 | 0 | 2.84 | 0 | 23 | 119 |
| take-2 | rookie | 99.5 | 97.2-99.9 | 0 | 2.67 | 1 | 40 | 197 |
| take-3 | pro | 54.5 | 47.6-61.3 | 0 | 8.42 | 13 | 46 | 305 |
| take-4 | pro | 46 | 39.2-52.9 | 0 | 5.59 | 8 | 52 | 195 |
| take-5-fallback | pro | 44 | 37.3-50.9 | 0 | 8.2 | 9 | 36 | 295 |
| take-5 | pro | 48.5 | 41.7-55.4 | 0 | 7.89 | 9 | 36 | 284 |
| take-6-fallback | pro | 53.5 | 46.6-60.3 | 0 | 1.95 | 4 | 46 | 69 |
| take-6 | pro | 54.5 | 47.6-61.3 | 0 | 8.42 | 13 | 46 | 305 |
| take-7-fallback | pro | 63.5 | 56.6-69.9 | 0 | 8.64 | 143 | 76 | 414 |
| take-7 | pro | 62 | 55.1-68.4 | 0 | 8.86 | 107 | 82 | 392 |
| take-8-fallback | all_star | 8.5 | 5.4-13.2 | 0 | 6.59 | 9 | 41 | 261 |
| take-8 | all_star | 13 | 9-18.4 | 0 | 6.01 | 7 | 44 | 239 |

- 0 crashes, 0 ensures, 0 matches past the horn in 2,400 matches.
- **take-3 and take-6 play identically**: same frames, goals and touches in all 200 seeds (hashes differ in 183 only through the cartridge's own content). take-3's
  `check_speed_min: 5` and `knockdown_frames: 90` and take-6's `variants.last_goal_double` change nothing the playtester sees (win 54.5 both).
- **take-7 (and its fallback) leave the human idle**: 107 and 143 of 200 matches have a 30 s stretch without a touch (max 82 s); the other takes 0 to 13.
- take-8 is 13.0 % (fallback 8.5 %) for the regular class and 28.5 % (fallback 21.5 %) for the good class (section 6): on the ladder's all_star target (26.3) for the class that target is set for. take-1 and take-2: 98 and 99.5 (regular class).

## 5. Other cartridges (regular class, 300 seeds each, 0 faults, 0 unfinished)

| cartridge | level | win % | human knockdowns | matches with a 30 s stretch | longest (s) | match s |
|---|---|---|---|---|---|---|
| boost | all_star | 1.3 | 3.81 | 222 (74 %) | 130 | 238 |
| boost | pro | 46.7 | 5.39 | 252 (84 %) | 127 | 334 |
| boost | rookie | 99.3 | 2.29 | 8 (3 %) | 56 | 188 |
| chaos | all_star | 12.3 | 1.52 | 20 (7 %) | 49 | 61 |
| chaos | pro | 33.3 | 1.46 | 19 (6 %) | 47 | 55 |
| chaos | rookie | 81 | 0.9 | 0 (0 %) | 22 | 52 |
| raw | all_star | 2.7 | 4.26 | 213 (71 %) | 96 | 236 |
| raw | pro | 54.7 | 5.95 | 235 (78 %) | 89 | 344 |
| raw | rookie | 100 | 2.42 | 1 (0 %) | 31 | 162 |
| wide | all_star | 1.7 | 3.28 | 223 (74 %) | 148 | 208 |
| wide | pro | 44 | 4.82 | 278 (93 %) | 121 | 370 |
| wide | rookie | 100 | 1.69 | 7 (2 %) | 52 | 147 |

## 6. The other classes on fresh seeds (201-400, 200 each): every mode and every take with the novice and the good class (9,600 matches, 0 faults, 0 unfinished at cap 36000, marathon 50000)

Win %, novice / good: arsenal 37 / 96.5, bruisers 83.5 / 100, cage 7.5 / 50, fog-of-war 60.5 / 96.5, glass-house 3 / 49, **marathon 0 / 21**, outnumbered 73 / 99.5, pond 40 / 97.5,
power-play 15 / 84, shotgun 6 / 62, sniper **0** / 39, sudden-death 3.5 / 25.5. Takes, novice / good: take-1 84 / 99, take-2 95 / 100, take-3 15 / 81, take-4 22.5 / 73, take-5 14.5 / 83
(fallback 18 / 78.5), take-6 15 / 81 (identical to take-3 again), take-7 24 / 89.5, take-8 4 / 28.5 (fallback 5 / 21.5).
Idle: the novice class has a 30 s stretch in 193-196 of 200 take-7 matches, in 175 of 200 power-play and all 200 marathon matches (novice is a poor player; read as the floor, not a fault).

## 7. Totals

About 56,200 matches: 36,000 (a) + 6,100 (b) + 400 (marathon cap) + 14 (marathon to 200000) + 3,800 (other cartridges, determinism) + 9,600 (other classes) + 300 (large, wide, take-7 traces).
**0 crashes, 0 ensures, 0 error lines, 0 missing seeds, 0 matches that never end** (the longest finished match is a marathon at 42000 frames; every other match ends by frame 33,126, regular/pro seed 2912 of the regular band, under the 36,000 cap).
Hashes agree between two processes on 200 of 200 re-run seeds.
