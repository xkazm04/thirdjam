# Playtester soak (lane A53, device A)

Status: FINAL Fri Nov 13 2026 10:59:26. 27000 matches done (0 crashed), free memory at this refresh 42.0 GB. Nothing here changes a profile or a sim number.
Final: 20 rounds done early; the 15:00 stop was not needed.

How it ran: `node tools/soak/playtest/soak.mjs --until 15:00 --rounds 20` (8 headless `UnrealEditor-Cmd -run=JamTournament -player=` processes at a time, 50 seeds each, `tools/playtest/run.mjs`'s
command line). Seeds 1-100 of each 150 are `hockey.regular`, 101-150 `hockey.boost`. Cells: 3 classes x 3 levels. A match is deterministic, so each seed is played once; 50 matches take about a minute
after a start-up of about 90 s per process. Tables: `node tools/soak/playtest/aggregate.mjs`. Raw results: `tools/soak/playtest/out/results/`.

## 1. What A3 should change (numbers; the targets are R-0029 point 1, on hockey.regular)

- novice at rookie on hockey.regular: 90.5 % (95 % 89.1 - 91.7, n 2000) against 85: +5.5 points, interval ABOVE +4.1
- regular at pro on hockey.regular: 58.1 % (95 % 55.9 - 60.2, n 2000) against 50: +8.1 points, interval ABOVE +5.9
- good at all_star on hockey.regular: 44.8 % (95 % 42.6 - 47, n 2000) against 25: +19.8 points, interval ABOVE +17.6

Where the brief and the measurement differ: the brief lists targets 80/85, 50/55, 30/25; R-0029 itself says 85 / about 50 / about 25 on the regular rink, which is what the flags use. The soak also
plays one third of its seeds on hockey.boost, which is much easier for the human than regular (see the boost rows); the pooled rows are therefore not the curve.

## 2. The table (class x level; hockey.regular rows are compared with the target, boost and pooled rows are information)

| class | level | cart | n | W-L-D-unf | win % | 95 % interval | R-0029 target | flag | GF / GA | touches (per min) | shots (on tgt) | passes (rcv) | checks L / T | one-timers / dekes / breakaways | scored <2 min | match s (p95) |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| novice | rookie | hockey.regular | 2000 | 1809-182-0-9 | **90.5** | 89.1 - 91.7 | 85 | ABOVE +4.1 | 2.8 / 0.9 | 45.1 (14) | 7.4 (4.3) | 0.1 (2) | 2.4 / 0 | 0 / 0 / 5.3 | 1544 of 2000 | 211.5 (422.2) |
| novice | rookie | hockey.boost | 1000 | 992-8-0-0 | 99.2 | 98.4 - 99.6 |  |  | 3 / 0.4 | 26.7 (11.2) | 5.6 (3.8) | 0 (0.9) | 1.3 / 0 | 0 / 0 / 4.6 | 907 of 1000 | 156.7 (312.8) |
| novice | rookie | all | 3000 | 2801-190-0-9 | 93.4 | 92.4 - 94.2 |  |  | 2.9 / 0.7 | 38.9 (13.1) | 6.8 (4.1) | 0.1 (1.7) | 2 / 0 | 0 / 0 / 5.1 | 2451 of 3000 | 193.2 (390.6) |
| regular | rookie | hockey.regular | 2000 | 1983-17-0-0 | **99.2** | 98.6 - 99.5 |  |  | 3 / 0.3 | 37.4 (20.7) | 12.5 (8.6) | 0.3 (1.4) | 1.6 / 0 | 0 / 2 / 3.9 | 1839 of 2000 | 123.2 (252.8) |
| regular | rookie | hockey.boost | 1000 | 999-1-0-0 | 99.9 | 99.4 - 100 |  |  | 3 / 0.1 | 33.7 (15.2) | 14.6 (9.9) | 0.1 (0.6) | 1.1 / 0 | 0 / 1.8 / 5.6 | 891 of 1000 | 145.1 (321.4) |
| regular | rookie | all | 3000 | 2982-18-0-0 | 99.4 | 99.1 - 99.6 |  |  | 3 / 0.2 | 36.2 (18.8) | 13.2 (9) | 0.3 (1.2) | 1.4 / 0 | 0 / 1.9 / 4.5 | 2730 of 3000 | 130.5 (270) |
| good | rookie | hockey.regular | 2000 | 1998-2-0-0 | **99.9** | 99.6 - 100 |  |  | 3 / 0.1 | 35.5 (22.2) | 16.7 (10.9) | 0.5 (1.3) | 1.7 / 0 | 0 / 3.5 / 2.9 | 1909 of 2000 | 106.9 (218.5) |
| good | rookie | hockey.boost | 1000 | 1000-0-0-0 | 100 | 99.6 - 100 |  |  | 3 / 0.1 | 27.4 (16.2) | 14.9 (10.7) | 0.2 (0.7) | 1.3 / 0 | 0 / 2.2 / 4.2 | 933 of 1000 | 110.6 (235.7) |
| good | rookie | all | 3000 | 2998-2-0-0 | 99.9 | 99.8 - 100 |  |  | 3 / 0.1 | 32.8 (20.2) | 16.1 (10.8) | 0.4 (1.1) | 1.5 / 0 | 0 / 3.1 / 3.3 | 2842 of 3000 | 108.1 (222.4) |
| novice | pro | hockey.regular | 2000 | 409-1591-0-0 | **20.5** | 18.7 - 22.3 |  |  | 1.3 / 2.6 | 41.3 (15.2) | 3 (1.4) | 0.1 (0.8) | 2.5 / 0 | 0 / 0 / 1.6 | 905 of 2000 | 182.6 (348) |
| novice | pro | hockey.boost | 1000 | 469-518-0-13 | 46.9 | 43.8 - 50 |  |  | 2 / 2.1 | 59.5 (16) | 4 (2.4) | 0 (0.9) | 3.1 / 0 | 0 / 0 / 3 | 596 of 1000 | 251.9 (487.7) |
| novice | pro | all | 3000 | 878-2109-0-13 | 29.3 | 27.7 - 30.9 |  |  | 1.5 / 2.5 | 47.4 (15.5) | 3.3 (1.8) | 0.1 (0.9) | 2.7 / 0 | 0 / 0 / 2.1 | 1501 of 3000 | 205.7 (408.5) |
| regular | pro | hockey.regular | 2000 | 1162-838-0-0 | **58.1** | 55.9 - 60.2 | 50 | ABOVE +5.9 | 2.2 / 1.9 | 61.5 (26) | 9.5 (4.9) | 0.6 (1.5) | 3.7 / 0 | 0 / 1.4 / 3.3 | 1460 of 2000 | 163.6 (324.9) |
| regular | pro | hockey.boost | 1000 | 771-215-0-14 | 77.1 | 74.4 - 79.6 |  |  | 2.6 / 1.3 | 79.2 (23) | 11.7 (6.7) | 0.4 (1.2) | 3.4 / 0 | 0 / 1.4 / 5.1 | 678 of 1000 | 230.8 (469.9) |
| regular | pro | all | 3000 | 1933-1053-0-14 | 64.4 | 62.7 - 66.1 |  |  | 2.4 / 1.7 | 67.4 (25) | 10.2 (5.5) | 0.5 (1.4) | 3.6 / 0 | 0 / 1.4 / 3.9 | 2138 of 3000 | 186 (382.2) |
| good | pro | hockey.regular | 2000 | 1491-509-0-0 | **74.6** | 72.6 - 76.4 |  |  | 2.6 / 1.5 | 74.1 (33.1) | 17.5 (8.2) | 1.1 (1.4) | 4.2 / 0 | 0 / 3.3 / 3.8 | 1618 of 2000 | 152 (286) |
| good | pro | hockey.boost | 1000 | 967-33-0-0 | 96.7 | 95.4 - 97.6 |  |  | 2.9 / 0.6 | 65.9 (29.5) | 12.9 (7.7) | 0.5 (0.8) | 2.8 / 0 | 0 / 2.2 / 4.7 | 893 of 1000 | 150.5 (317.6) |
| good | pro | all | 3000 | 2458-542-0-0 | 81.9 | 80.5 - 83.3 |  |  | 2.7 / 1.2 | 71.4 (31.9) | 16 (8.1) | 0.9 (1.2) | 3.7 / 0 | 0 / 2.9 / 4.1 | 2511 of 3000 | 151.5 (299.1) |
| novice | all_star | hockey.regular | 2000 | 129-1870-0-1 | **6.5** | 5.5 - 7.6 |  |  | 0.8 / 2.9 | 29.3 (10.6) | 1.6 (0.6) | 0.1 (0.6) | 1.1 / 0 | 0 / 0 / 0.8 | 449 of 2000 | 184.5 (365.3) |
| novice | all_star | hockey.boost | 1000 | 40-921-0-39 | 4 | 3 - 5.4 |  |  | 0.6 / 2.9 | 33 (7.4) | 1.2 (0.4) | 0 (0.6) | 1.1 / 0 | 0 / 0 / 0.6 | 113 of 1000 | 294.5 (578.6) |
| novice | all_star | all | 3000 | 169-2791-0-40 | 5.6 | 4.9 - 6.5 |  |  | 0.7 / 2.9 | 30.5 (9.5) | 1.4 (0.6) | 0.1 (0.6) | 1.1 / 0 | 0 / 0 / 0.7 | 562 of 3000 | 221.2 (477.2) |
| regular | all_star | hockey.regular | 2000 | 586-1412-0-2 | **29.3** | 27.3 - 31.3 |  |  | 1.5 / 2.5 | 55.1 (22.1) | 6.1 (2.9) | 0.7 (1.2) | 2.4 / 0 | 0 / 0.9 / 2.2 | 1045 of 2000 | 169.6 (339.1) |
| regular | all_star | hockey.boost | 1000 | 240-730-0-30 | 24 | 21.5 - 26.7 |  |  | 1.4 / 2.6 | 59.5 (14.9) | 4.7 (2.5) | 0.5 (1.5) | 2.1 / 0 | 0 / 0.8 / 1.9 | 420 of 1000 | 270.2 (559.8) |
| regular | all_star | all | 3000 | 826-2142-0-32 | 27.5 | 26 - 29.2 |  |  | 1.5 / 2.5 | 56.5 (19.7) | 5.7 (2.8) | 0.6 (1.3) | 2.3 / 0 | 0 / 0.8 / 2.1 | 1465 of 3000 | 203.1 (442.4) |
| good | all_star | hockey.regular | 2000 | 896-1103-0-1 | **44.8** | 42.6 - 47 | 25 | ABOVE +17.6 | 1.9 / 2.2 | 73.5 (29.3) | 13.1 (5.6) | 1.6 (1.8) | 4.8 / 0 | 0 / 2.1 / 2.7 | 1202 of 2000 | 172.2 (337.3) |
| good | all_star | hockey.boost | 1000 | 410-578-0-12 | 41 | 38 - 44.1 |  |  | 1.9 / 2.3 | 71.6 (19.6) | 8.8 (4.2) | 1 (1.5) | 4.5 / 0 | 0 / 2.1 / 2.4 | 560 of 1000 | 245.9 (498.4) |
| good | all_star | all | 3000 | 1306-1681-0-13 | 43.5 | 41.8 - 45.3 |  |  | 1.9 / 2.2 | 72.9 (26.1) | 11.7 (5.1) | 1.4 (1.7) | 4.7 / 0 | 0 / 2.1 / 2.6 | 1762 of 3000 | 196.8 (410) |

## 3. Faults (deduplicated by the first distinct line; reproduce with the command in the row, which needs the matching seed on that cartridge)

| kind | count | first line / detail | cells | reproduce (first seed) |
|---|---|---|---|---|
| human_idle_30s (boost) | 3738 | `2276 frames without a human touch` | good/all_star/boost, good/pro/boost, good/rookie/boost, novice/all_star/boost | `node tools/playtest/run.mjs --players good --levels all_star --rinks hockey.boost --seeds 1003-1003` (seed 1003) |
| human_idle_30s (regular) | 2869 | `2566 frames without a human touch` | good/all_star/regular, good/pro/regular, good/rookie/regular, novice/all_star/regular | `node tools/playtest/run.mjs --players good --levels all_star --rinks regular --seeds 1051-1051` (seed 1051) |
| overtime_without_end (boost) | 108 | `not over after 36000 frames, score 2:2` | good/all_star/boost, novice/all_star/boost, novice/pro/boost, regular/all_star/boost | `node tools/playtest/run.mjs --players good --levels all_star --rinks hockey.boost --seeds 1313-1313` (seed 1313) |
| overtime_without_end (regular) | 13 | `not over after 36000 frames, score 2:2` | good/all_star/regular, novice/all_star/regular, novice/rookie/regular, regular/all_star/regular | `node tools/playtest/run.mjs --players good --levels all_star --rinks regular --seeds 2126-2126` (seed 2126) |
