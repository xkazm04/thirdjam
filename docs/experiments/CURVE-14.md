# The difficulty curve, pass 14: the ladder after the ring (lane A221, track C0, Sunday 15 November)

Scope (brief `briefs/A221-C0-ladder-after-the-ring.md`): A218's rule (`stick.pickup.ring_m` 1.1: a loose puck inside the ring under the human is
his on that frame) made every level easier. Targets 85 / 48 / 26 for the player each level is meant for, within 4 at 300 seeds. Every step with its
time is in `status/A221.md`; drivers and rows under `tools/perf/results/a221/` (`raw/` and `out/` do not land).

## 1. Instrument
A217's person's path (`-run=JamHandCells`, CURVE-13.md section 1), unchanged in what it plays. Lane A221 added read-outs only (no input, no state):
`opp_shots`, and the human's GAINS (the puck's last toucher becomes him from another skater): from an opponent's shot still in flight by its speed
(<6, 6-18, >=18 m/s), from an opposing carrier, loose after an opponent's touch, after a mate; and of these, those with the puck's centre within
1.2 m of his centre. `tools/perf/results/a221/cells.mjs` runs many cells in one pool of processes (at most four), on hand.mjs's seeds.

## 2. Before the set (main 81e3d1a9 with the ring, 300 a cell; SET 3 after the net fix beside it)

| cell (target) | with the ring (300) | SET 3 (300) | goals for : against | knocked down a minute (SET 3) | GA inside f600, matches |
|---|---|---|---|---|---|
| rookie regular (85) | 99.0 (97.1-99.7) | 91.7 | 2.99 : 0.38 | 0.96 (0.70) | 3 |
| rookie large (85) | 93.0 (89.5-95.4) | 81.7 | 3.72 : 0.90 | 0.92 (0.72) | 11 |
| pro regular (48) | 71.0 (65.6-75.8) | 54.0 | 2.47 : 1.56 | 1.77 (1.45) | 57 |
| pro large (48) | 58.3 (52.7-63.8) | 51.7 | 3.27 : 2.84 | 1.76 (1.65) | 29 |
| all_star regular (26) | 31.7 (26.7-37.1) | 26.7 | 1.67 : 2.40 | 1.53 (1.27) | 47 |
| all_star large (26) | 43.0 (37.5-48.7) | 33.7 | 2.50 : 3.06 | 1.59 (1.50) | 20 |

Where the person wins more: he keeps and takes pucks. A match at pro regular: 7.6 gains off an opposing carrier, 6.4 loose after an opponent's
touch, 6.7 after a mate; 1.9 from an opponent's shot still in flight, of which 0.38 at 6-18 m/s (A218's open point 3: 3.8 % of the opponents' shots,
0.24 per goal against; 7.8 % / 0.44 on the large rink). Blocks are not a large share at any cell (no rule to narrow). Knockdowns rose with the ring at
every cell: he carries more, he is checked more.

## 3. The levers read (person's path, by tune, the set's numbers in brackets)

Pro regular at 100 (73 on these seeds): shot reach +30 63; carrier pressure 0.3 66; forecheck yield never 65; all three 70; **R-0063's dial**
(presser on its job in the hunt, commit 8 m, hunt 11 m) 48, hunt 10 55. At 600 on top of the dial: hunt 12 60.2, pressure 0.3 53.7.
Pro large at 100 (66): the dial 59, hunt 12 56; red 1.10 alone 33; dial + red 1.08 43. All_star (the speed raises his knockdowns on the
large rink, 1.52 -> 1.90 / 2.03 at 1.20 / 1.25, and reads unevenly at 100: regular 1.17 41, 1.19 30, 1.22 12); the brain dial (step-up 16 m,
second man hunts) large 43 -> 35 (300) with knockdowns 1.59 -> 1.66, regular 33 / 31 at 100. Rookie: reach +25 reads 100 at 100.
Law 3 (`Bar.Curve.Law3`, 200 seeds an arm): every pro candidate 93.5-95.0 % at 8 m, straight charge 0; all_star 99.0-99.5 / 0.

## 4. LADDER SET 4

`Game/Source/JamCore/AI/DifficultyProfile.cpp` MakePro: `P.Opp.Hk.bPressInHunt` false -> true, `P.Opp.Hk.PressCommitM` 0 -> 8, `P.Opp.HuntRadiusM`
8 -> 11 (R-0063's dial; ProOppSkill's defaults untouched, so code without a level plays as before), `P.LargeRedSkill` 1.03 -> 1.05.
No new key. Neutral: the tune `opp.hk_press_in_hunt=0;opp.hk_press_commit_m=0;opp.hunt_radius_m=8` (+ `red_skill=1.03` on the large rink)
replays the game before the set to the hash: 50 of 50 seeds a rink against the before rows. Built = tune: 300 of 300 seeds a rink.

| cell (target) | before (300) | after | knocked down a minute before / after | law 3 at 8 m / straight |
|---|---|---|---|---|
| pro regular (48) | 71.0 | **55.2** (51.2-59.1), 600 | 1.77 / 1.82 | 94.0 % / 0 |
| pro large (48) | 58.3 | **46.0** (42.1-50.0), 600 | 1.76 / 1.74 | (the drill is the regular rink's) |
| rookie, all_star (4 cells) | as section 2 | unchanged (their profiles untouched; the pins equal) | | all_star 99.0 % / 0 |

Open: pro regular +7.2 (seeds 1-300 50.3, 301-600 60.0); all_star regular +5.7, all_star large +17, rookie regular +14 / large +8.
The next levers, each with its number: pro's shot reach +25 -> +28 with the dial (pro regular at 600; the far-goal share must stay under 10 %);
all_star's brain dial on the large rink only (needs a level's large-rink step-up number: a code change in `ApplyDifficultyProfile`); rookie: none
found that keeps it a newcomer's level (its far goals are 0 %, its goals against inside f600 1 %).
