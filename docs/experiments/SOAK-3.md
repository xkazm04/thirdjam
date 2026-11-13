# SOAK-3: the soak follows main (lane A126, track S1, evolution 3)

Fri 13 Nov 2026, night. The first full reading of the game with the puck kept. Build: `clean-main` at main `41df3e6` (kept puck, body shield, take grace 30, harder stop,
earned first step, release grace 120, boards as a body, the twelve re-made modes, the arena). **Not yet in it:** the brain's second evolution and the re-set difficulty ladder.
So this is the reading "with the puck kept, before the brain and the curve". It is `tools/soak/baseline.json` now (36 cells x 100 seeds, 3,600 matches, 0 faults); the old
one (build `4da9f36`) is kept out of the tree as `tools/soak/out/baseline-old.json` and is what the "old" columns below read. Class `regular` plays the modes; novice, regular
and good play the takes, each at the cartridge's own level (takes 1 and 2 rookie, 3 to 7 pro, 8 all_star). All numbers are over 100 seeds: a win rate carries about 5 points of
sampling noise (1 sigma at 50 %).

## The reading in six lines (for the curve lane C0 and the modes lane M1)
1. **Every take rose, as predicted:** regular class +24 to +34 points on takes 3 to 5, +16 to +34 across 3 to 7; good class +13 to +23 on takes 3 to 7; novice +11 to +27.
   Takes 1 and 2 are now a rout (regular 99 and 100, good 100, novice 98 and 99); the ladder's bottom no longer teaches anything. Take 8 (all_star) rose +12 to +13 (good 34 > 47, regular 12 > 24) and
   is the only take still under 50 for good class; novice stays at 6 %.
2. **Take 3 is no longer a take 4.** Take 3 / 4 / 5 / 6 / 7 for the good class read 94 / 94 / 91 / 94 / 93: the five middle takes are one difficulty (regular: 73 / 76 / 78 / 73 / 76). The
   ladder has a flat middle of five rungs; the curve lane's target cells must come from re-set levels, not from these cartridges.
3. **Take 3 and take 6 are numerically the same game** (every column identical, win 44 / 73 / 94; their per-seed hashes differ, so the cartridges differ in something the
   playtest does not read). Worth a look by whoever owns the ladder cartridges.
4. **The idle share rose almost everywhere.** Idle seconds of the human per match (frames beyond 15 s since his last touch): take 7 novice 133 > 173 s with 98 % of matches holding a
   30 s stretch without a touch; take 3 and 5 novice 44 > 71 and 47 > 71 s; Marathon 96 > 112 s (96 % of matches). The cause is the kept puck: a bot that holds the puck keeps it, the human waits.
   Regular class idle roughly doubles (take 3: 7.8 > 10.9 s; take 7: 45 > 67 s, 69 % of matches with a 30 s dry spell). The modes that had no idle problem now have one:
   fog-of-war 3.2 > 9.3 s (13 % of matches), outnumbered 1.0 > 7.4 s, power-play 22.9 > 38.3 s (57 % of matches). One mode lost its: shotgun 20.0 > 3.6 s.
5. **Knockdowns of the human** moved little for the modes (shotgun 4.4 > 5.1, bruisers 6.9 > 6.2, arsenal 3.5 > 2.7) and rose on take 8 (regular 6.2 > 8.9, good 7.6 > 9.8): all_star bots hit harder
   once they keep the puck. Takes 1 to 2 are low (1.5 to 3.3).
6. **Match length fell on the easy takes** (take 1 regular 6,987 > 4,060 frames, take 1 good 4,513 > 3,182; most modes shorten 10 to 30 %, cage, pond and power-play stay flat: marathon 38,871 > 30,781 as its clock went to 8:00,
   sudden-death 3,269 > 2,355) and rose on the hard ones (take 3 novice 19,289 > 21,174, take 8 novice 13,455 > 15,405).

### Marathon
Its clock is 8:00 now (timer 28800, hard stop 34800), so the 42,000-frame cap and the extra `--max-frames` are gone: the driver's cap is 36,000 (`MARATHON_CAP`), 0 unfinished. `-playersprint` stays
(the character number is sprint frames). Win 2 > 71 % comes from the clock and the tuning, not from the cap. Sprint frames per match 3,033 > 1,869.

## Modes: what moved (regular class at the mode's level, 100 seeds)
| mode | character number | old | new | human win % old | new | knockdowns of human old | new | idle s old | new |
|---|---|---|---|---|---|---|---|---|---|
| arsenal | human abilities used / match | 0.98 | 0.72 | 94 | 92 | 3.51 | 2.68 | 0.81 | 3.31 |
| bruisers | hits (landed + taken) / play min | 4.88 | 4.62 | 100 | 100 | 6.92 | 6.15 | 0.75 | 2.2 |
| cage | checks landed / match | 4.99 | 8.12 | 18 | 57 | 7.48 | 7.38 | 11.47 | 13.03 |
| fog-of-war | human breakaways / play min | 1.93 | 1.88 | 96 | 89 | 5.48 | 5.03 | 3.22 | 9.28 |
| glass-house | human box seconds / match | 13.18 | 9.91 | 20 | 46 | 4.55 | 4.6 | 23.95 | 27.81 |
| marathon | human sprint frames / match | 3032.77 | 1869.35 | 2 | 71 | 11.6 | 12 | 96.14 | 111.63 |
| outnumbered | shots against / play min | 3.06 | 5.14 | 99 | 64 | 4.53 | 3.02 | 1.01 | 7.44 |
| pond | passes + received / play min | 1.64 | 1.06 | 80 | 96 | 0 | 0 | 0.65 | 3.02 |
| power-play | passes + received / match | 7.77 | 4.53 | 41 | 72 | 6.5 | 9.2 | 22.93 | 38.27 |
| shotgun | knockdowns (all skaters) / match | 12.09 | 13.28 | 22 | 64 | 4.41 | 5.05 | 20.04 | 3.56 |
| sniper | human goals per human shot | 0.39 | 0.48 | 10 | 15 | 3.19 | 3.82 | 7.86 | 7.92 |
| sudden-death | match seconds | 54.48 | 39.25 | 16 | 21 | 0.81 | 1.15 | 8.1 | 10.59 |

## Takes: win rate by class, old > new
| take | level | novice old>new | regular old>new | good old>new | kd of human (regular) old>new | idle s (regular) old>new | idle s (novice) old>new |
|---|---|---|---|---|---|---|---|
| take 1 | rookie | 79 > 98 | 96 > 99 | 100 > 100 | 2.85 > 2.02 | 0.48 > 0.92 | 19.43 > 12.68 |
| take 2 | rookie | 94 > 99 | 100 > 100 | 100 > 100 | 2.56 > 3.2 | 2.18 > 8.59 | 13.07 > 23.98 |
| take 3 | pro | 18 > 44 | 49 > 73 | 73 > 94 | 8.35 > 8.66 | 7.77 > 10.93 | 44.31 > 71.16 |
| take 4 | pro | 24 > 41 | 45 > 76 | 71 > 94 | 5.84 > 7.26 | 4.37 > 10.73 | 28.02 > 37.02 |
| take 5 | pro | 15 > 42 | 44 > 78 | 76 > 91 | 7.99 > 8.84 | 7.34 > 11.29 | 46.91 > 71.21 |
| take 6 | pro | 18 > 44 | 49 > 73 | 73 > 94 | 8.35 > 8.66 | 7.77 > 10.93 | 44.31 > 71.16 |
| take 7 | pro | 30 > 41 | 60 > 76 | 80 > 93 | 8.38 > 8.94 | 44.86 > 66.51 | 133 > 172.96 |
| take 8 | all_star | 6 > 6 | 12 > 24 | 34 > 47 | 6.2 > 8.86 | 5.87 > 9.82 | 24.53 > 40.98 |
## The proofs left open from evolution 2
- **Fields (item 2), build of `JamPlaytester.h/.cpp` in `clean-main`:** rebuilt in 48 s; `stumbles`, `hold_frames`, `board_hits`, `knockdowns_by_shotgun` appear in the line (take 3 good, seed 1:
  stumbles 2, hold_frames [1329, 1395, 3482] of 6,210 play frames, board_hits 11, knockdowns_by_shotgun 0). 200 seeds (m-cage and t-take-3-good, 100 each) hash identical to the baseline digests: no hash moved.
- **Drift gate (item 3):** `drift-gate.mjs --force all` on the unchanged build: 36 of 36 bit-identical, exit 0, 475 s (the cap is 480: on a machine at 100 % CPU from other lanes this is on the edge;
  a quiet machine took 327 s in A116). A seeded 12 % change (a doctored baseline, `--from` the run's folder) is caught: `m-cage checks landed 7.25 > 8.12 (+12 %)` and `t-take-3-good win 83.93 > 94 (+12 %, 2.74 sigma)`, exit 1.
  The step is `premerge.ps1 -Drift` (off by default). Not exercised through premerge.ps1 end to end (it parses; the node half is tested).
- **Governor (item 4): the target is missed.** The 36,000-match matrix (`tools/soak/a107/plan-a.json`, 18 cells x 2,000) took **2,214 s (36.9 min)**, 0 faults, against the 20-minute goal. The machine sat at
  100 % CPU from other lanes' builds throughout, so the governor held 3 to 9 workers (progress lines: active 3 nine times, 8 to 9 six times). The A116 figure (23.4 to 26 min at 100 % CPU) was with a
  fixed 16-worker cap and no CPU rule; the CPU rule trades the matrix's time for not starving the other lanes. On a quiet machine it runs all 16. To hit 20 minutes under load pass `--cpu-hi 1.01` (the
  rule then never throttles) at the price of slowing the builds next door. Unit tests (`cpuLimit`, `mayStart`) pass; the 20-minute claim is not proven.

## How long the routine takes (to repeat when the curve lands)
Build `clean-main` 203 s cold (48 s after one file), fresh baseline 535 s, no-op gate re-run 475 s: about 15 minutes for build plus baseline plus the table, the 36,000 matrix about 37 minutes loaded.

## Open: the modes' cells need re-taking on the next main
Lane A139 changed eight mode cartridges after this baseline (Pond, Power Play, Marathon re-made, two of their character numbers now per play minute; Cage, Bruisers, Outnumbered, Sudden Death grace keys).
`baseline.json` holds `m-marathon` and `m-power-play` under the old per-match labels, and the modes' digests will differ: re-take with `node tools/soak/drift.mjs --uproject <clean-main uproject> --only modes --update-baseline`
(12 cells, about 3 minutes loaded). The take cells are unaffected.
