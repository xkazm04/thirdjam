# SOAK-4: a soak baseline that is true of the package the operator plays (lane A174, track S1, evolution 4)

Sat 14 Nov 2026, morning. Build: this worktree (a107) at main ca493723 (package H28's commit c5dc15e5 plus all-off rules), full build 290 s. Headless, `-Multiprocess`, 3 processes at most.

## 1. Drift before: baseline as it stood on main (modes 3f5cbda + A156, takes 41df3e6), 36 cells, 3,600 matches, 0 faults
**0 of 36 cells bit-identical; 21 numbers moved over the 10 % gate.** The regular cuts and arenas were not in the baseline at all (no "before" for 24 cells).
Modes: every one drifted but within sampling noise except sudden-death (win 33 > 45, 2.55 sigma) and shotgun (49 > 60, 2.2 sigma), cage 78 > 69 (2.2 sigma). Takes: the ladder was re-set, so they moved a lot (regular class takes 3 to 6 now win 98 to 100, were 73 to 78;
novice take 3 44 > 90, take 4 41 > 76; take 1 novice 98 > 78; take 2 and 3 to 6 matches are 30 to 60 % shorter). Per cell (baseline > now, frames, goals only "now": the old baseline did not record goals):


### modes (regular class at the mode level)
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| m-arsenal | DRIFT | 62 > 65 | 10471 > 9718 | 6.3 > 5.8 | 2.36-1.71 now | 0 > 0 |
| m-bruisers | DRIFT | 59 > 54 | 4628 > 4532 | 3.7 > 3.9 | 1.36-1.17 now | 0 > 0 |
| m-cage | DRIFT | 78 > 69 | 12676 > 12547 | 7.2 > 7 | 4.35-3.2 now | 0 > 0 |
| m-fog-of-war | DRIFT | 81 > 81 | 5981 > 6132 | 5.1 > 5.5 | 2.68-1.25 now | 0 > 0 |
| m-glass-house | DRIFT | 41 > 44 | 11085 > 10015 | 6.3 > 5.9 | 1.92-2.15 now | 0 > 0 |
| m-marathon | DRIFT | 77 > 74 | 24478 > 24459 | 15.9 > 16 | 7.61-5.08 now | 0 > 0 |
| m-outnumbered | DRIFT | 56 > 63 | 5963 > 5456 | 2.1 > 2 | 2.33-1.75 now | 0 > 0 |
| m-pond | DRIFT | 72 > 73 | 13071 > 13023 | 0 > 0 | 5.84-3.9 now | 0 > 0 |
| m-power-play | DRIFT | 52 > 60 | 16653 > 16882 | 12.4 > 13 | 5.61-4.8 now | 0 > 0 |
| m-shotgun | DRIFT | 49 > 60 | 6148 > 6399 | 4.2 > 4.7 | 2.31-1.89 now | 0 > 0 |
| m-sniper | DRIFT | 41 > 49 | 5086 > 4495 | 4.4 > 3.9 | 2.08-1.99 now | 0 > 0 |
| m-sudden-death | DRIFT | 33 > 45 | 2499 > 2834 | 0.9 > 1 | 0.45-0.55 now | 0 > 0 |

### take 1
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-1-novice | DRIFT | 98 > 78 | 6544 > 6950 | 1.5 > 1.8 | 1.65-0.69 now | 0 > 0 |
| t-take-1-regular | DRIFT | 99 > 95 | 4060 > 4285 | 2 > 1.6 | 1.94-0.29 now | 0 > 0 |
| t-take-1-good | DRIFT | 100 > 99 | 3182 > 3534 | 2 > 2.3 | 1.98-0.12 now | 0 > 0 |

### take 2
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-2-novice | DRIFT | 99 > 88 | 11861 > 8531 | 1.5 > 2 | 1.85-0.43 now | 0 > 0 |
| t-take-2-regular | DRIFT | 100 > 98 | 12179 > 8201 | 3.2 > 2.4 | 2.96-0.36 now | 0 > 0 |
| t-take-2-good | DRIFT | 100 > 97 | 12634 > 8153 | 3.3 > 3.4 | 3.41-0.42 now | 0 > 0 |

### take 3
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-3-novice | DRIFT | 44 > 90 | 21174 > 11443 | 5.5 > 2.9 | 2.76-1.06 now | 0 > 0 |
| t-take-3-regular | DRIFT | 73 > 98 | 14500 > 6487 | 8.7 > 2.5 | 2.97-0.43 now | 0 > 0 |
| t-take-3-good | DRIFT | 94 > 100 | 9833 > 5468 | 6.6 > 3.4 | 3-0.23 now | 0 > 0 |

### take 4
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-4-novice | DRIFT | 41 > 76 | 12493 > 9029 | 3.4 > 2.5 | 2.15-0.94 now | 0 > 0 |
| t-take-4-regular | DRIFT | 76 > 98 | 11333 > 6091 | 7.3 > 2.4 | 2.93-0.26 now | 0 > 0 |
| t-take-4-good | DRIFT | 94 > 100 | 8849 > 5291 | 6.2 > 3.4 | 2.98-0.21 now | 0 > 0 |

### take 5
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-5-novice | DRIFT | 42 > 73 | 20593 > 15579 | 5.3 > 3.4 | 2.53-1.52 now | 0 > 0 |
| t-take-5-regular | DRIFT | 78 > 100 | 14256 > 5812 | 8.8 > 2.1 | 3-0.14 now | 0 > 0 |
| t-take-5-good | DRIFT | 91 > 100 | 9732 > 4448 | 7.1 > 2.1 | 3-0.12 now | 0 > 0 |

### take 6
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-6-novice | DRIFT | 44 > 66 | 21174 > 15330 | 5.5 > 4 | 2.39-1.8 now | 0 > 0 |
| t-take-6-regular | DRIFT | 73 > 99 | 14500 > 7524 | 8.7 > 3.2 | 2.99-0.33 now | 0 > 0 |
| t-take-6-good | DRIFT | 94 > 100 | 9833 > 4879 | 6.6 > 3.1 | 3-0.14 now | 0 > 0 |

### take 7
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-7-novice | DRIFT | 41 > 62 | 26467 > 20409 | 6.1 > 6.1 | 2.21-1.78 now | 0 > 0 |
| t-take-7-regular | DRIFT | 76 > 88 | 20522 > 13909 | 8.9 > 7.4 | 2.8-0.99 now | 0 > 0 |
| t-take-7-good | DRIFT | 93 > 97 | 12979 > 11140 | 6.7 > 6.2 | 2.96-0.58 now | 0 > 0 |

### take 8
| cell | digest | win % | frames | kd of human | goals for-against | unfinished |
|---|---|---|---|---|---|---|
| t-take-8-novice | DRIFT | 6 > 5 | 15405 > 8986 | 4.8 > 2.9 | 0.64-2.91 now | 0 > 0 |
| t-take-8-regular | DRIFT | 24 > 29 | 14602 > 9151 | 8.9 > 6.7 | 1.56-2.51 now | 0 > 0 |
| t-take-8-good | DRIFT | 47 > 42 | 13005 > 9198 | 9.8 > 7.9 | 2.03-2.24 now | 0 > 0 |

bit-identical 0 of 36

## 2. Drift after: re-taken on ca493723, then replayed
Re-taken 36 cells, plus two new families added to the gate (`drift.mjs`: regular cuts `r-*` and arenas `a-*`; `--only cuts|arenas`, `--cells <labels>`): 60 cells x 100 seeds, 0 faults. Replay of all 60: **60 of 60 bit-identical, 0 moved** (every cell played twice with one digest).
Cost: re-take 36 cells about 10 min, 24 new cells about 8 min, a full 60-cell drift run about 15 min with 3 workers (it was 5 m 27 s for 36 cells at 16 workers on a quiet machine).

## 3. The soak: 46,800 matches, 3 workers, 0 faults
Three plans on `hockey.regular` (3 classes x 3 levels) and `hockey.arena.large`, seeds 1001-4600 (`plan-soak4*.json`; the chunk was 500 for the first, 1500 for the others so a process lives 2 to 4.5 minutes). Wall 638 s + 556 s + 1,348 s = 42 min (about 18 matches per second).
- Crash, hang, ensure, error line, missing seed: **0** (faults.jsonl empty in all three). Matches that did not finish (result not win/loss/draw or over=false): **0**. Non-finite numbers in 46,800 lines: **0**. Nothing beyond the harness cap (36,000 frames): longest match 33,000.
- **Matches that hit the 30,000-frame hard stop (frames >= 30,000): 96 of 46,800 (0.21 %)**, and they are almost all the novice class on hockey.regular (the modes lane's one in 600 was this):

| cut, level, class | matches | at or over 30,000 | share | longest |
|---|---|---|---|---|
| regular, pro, novice | 3,600 | 51 | 1.42 % | 33,000 |
| regular, rookie, novice | 2,100 | 21 | 1.00 % | 33,000 |
| regular, all_star, novice | 2,100 | 10 | 0.48 % | 33,000 |
| regular, all_star, regular | 2,100 | 6 | 0.29 % | 32,306 |
| regular, pro, regular | 3,600 | 4 | 0.11 % | 33,000 |
| regular, all_star, good | 2,100 | 3 | 0.14 % | 31,896 |
| large arena, pro, novice | 3,600 | 1 | 0.03 % | 30,618 |
| every other cell (regular rookie regular/good, pro good, large arena 8 cells) | 29,100 | 0 | 0 | 26,764 |

  Each seed is in `tools/soak/out/soak4-report.txt` (run `node tools/soak/soak-report.mjs tools/soak/out/soak4 tools/soak/out/soak4b tools/soak/out/soak4c` for the LONG lines: cut, level, class, seed, frames, result). 9 of the 96 ended on a win, so the stop awards by score. A novice bot that does not score is what runs the clock; a human who idles could too. Not a fault of the build, a finding on the match clock.
- Memory per process (`memwatch.mjs`, a sample every 10 s, 23 processes that lived 125 to 272 s, warm-up of 40 s left out): first quarter 1,819 MB mean, last quarter 1,835 MB mean, change +0.3 to +1.2 % (worst +1.2 %), maximum working set 1,862 MB. Flat. A process warms from 0.7 to 1.8 GB in its first 40 s (a start-up plateau, not a leak). Limit of the proof: processes live 2 to 4.5 minutes, not an hour.
- Take 3 (the ladder) is hash-identical to `hockey.regular` at rookie (digest 9e510e887acf6443 in `t-take-3-regular` and `r-regular-regular-rookie`): the ladder's take 3 is the plain regular cut; levels now: takes 1 to 4 rookie, 5 to 7 pro, 8 all_star.
- Void rows: none. Lane A170 stopped some processes on the machine at about 07:28; my driver, memwatch and children were alive afterwards (checked by process id), no row re-run.

## 4. The drift gate on every landing
Measured (3 workers, machine at 32 GB): one regular cell + one mode + one take (`r-regular-regular-pro`, `m-cage`, `t-take-5-regular`), 300 matches: **54 s** (3 bit-identical); one cell alone 36 s. The cost is the process start (about 20 to 30 s) plus 100 matches. Proposed step 7b of `premerge.ps1`, on every landing, no switch, after the build and before the automation filter:

```
node tools/soak/drift.mjs --uproject <this tree> --max 3 --cells r-regular-regular-pro,m-cage,t-take-5-regular     # about 55 s, exit 1 on any moved number
```
A pass needs the baseline's digests: a landing that changes the sim on purpose re-takes just those cells (`--cells ... --update-baseline --build "<commit> why"`, 1 minute) and lands `baseline.json`, which the Director sees in the diff. The expected-moving landings (sim, brain, cartridges) keep running the existing `-Drift` step (60 cells, about 15 min, off by default). A landing that must stay hash-identical (docs, tools, rules switched off) then gets a bit-identical check in about a minute. `drift.mjs` exits 1 on a digest-different cell only if a number moved over the gate; propose the step read the JSON `identical` count instead (3 of 3) so any hash change fails. Not wired (the Director decides).

## 5. The baseline as committed: 60 cells x 100 seeds, all taken on ca493723 (2026-11-14 05:33 UTC and after)
| family | cells | taken on |
|---|---|---|
| modes `m-*` (12, regular class at the mode's level, marathon capped 36,000 with sprint) | 12 | ca493723 |
| takes `t-take-N-novice|regular|good` (8 takes x 3) | 24 | ca493723 |
| regular cuts `r-*`: regular at rookie, pro, all_star; boost, chaos, raw at pro; x novice, regular, good | 18 | ca493723 |
| arenas `a-*`: large, wide at pro; x novice, regular, good | 6 | ca493723 |

Win rates by cell are in `tools/soak/baseline.json` (`win`, `frames`, `kd`, `gf`, `ga`, `idle_s`, `digest`). The entry baseline is kept as `tools/soak/out/baseline-entry-3f5cbda-41df3e6.json` (not in the landing).

## 6. Files
`tools/soak/baseline.json`, `drift.mjs` (families, `--cells`, goals `gf`/`ga`), `drift-table.mjs`, `memwatch.mjs`, `soak-report.mjs`, `plan-soak4.json`, `plan-soak4b.json`, `plan-soak4c.json`, `docs/experiments/SOAK-4.md`, `status/A174.md`. Tests: drift.test.mjs 7 of 7.
