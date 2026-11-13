# SOAK-DRIFT: the shared soak driver and the drift gate (lane A116, track S1, evolution 2)

Two tools, both headless, both under `tools/soak/`. Every track lane can run the gate before its report; every lane that needs many matches can hand the driver a plan.

## 1. The shared driver: `tools/soak/cells.mjs`

```
node tools/soak/cells.mjs <plan.json> --uproject <abs .uproject of a BUILT checkout> --out <dir> [--max 16] [--chunk 100] [--stagger-ms 1000]
```

A plan is a JSON list of cells: `{label, cls, lvl, cart, seeds, extra?, trace?, maxframes?}`; `seeds` is `"1-2000"`, `"3,5,9-12"` or `"regularband:2000"` (A53's band).
Plans of A107's batches are in `tools/soak/a107/plan-*.json` and still run (`plan-a.json` is the 18-cell, 36,000-match matrix).

What it keeps, so a lane does not have to:

| rule | how |
|---|---|
| `-Multiprocess` on every start | in `chunkArgs` (and in every repeat command it prints); a node test checks it |
| the memory band (R-0047) | `mayStart`: under 38 GB in use up to `--max` workers; 38 to 50 GB a trickle of up to 4; over 50 nothing new; over 56 nothing at all |
| a start guard | exits 2 (`could_not_run`) when the exe or project is missing, a label repeats, a plan cell lacks a field, or the first chunk (run alone, twice at most) prints no match line |
| requeue after a crash | the seed that was running when the process died is a fault; the rest of the chunk is requeued after it; a chunk that gave no match at all is retried whole once |
| a per-seed fault ledger | `<out>/faults.jsonl`, one line per fault: label, kind (`crash`, `ensure`, `error_line`, `jamfx_missing`, `missing_seeds`), seed, the line, and `cmd`, the command that repeats that one seed |
| a match per line | `<out>/<label>.jsonl`, the `thirdjam.playtest/1` lines sorted by seed and deduped; `<label>.done`; logs only for chunks with a fault |
| resumable | finished chunks are appended to `<label>.partial.jsonl`; a restart plays only the seeds that are missing; a `.done` cell is skipped (proved: see section 4) |
| kills only its own | on a timeout it kills the one child it started, never anything else |

Exit 0 no faults, 1 faults, 2 could not start. The last stdout line is `{"tool":"thirdjam.soak/1",...}`. Test: `node --test tools/soak/cells.test.mjs` (11 tests, a fake runner stands in for Unreal).

The start stagger matters: the first version started one worker per 4 s, which caps a driver with 35-second chunks at about 22 matches per second however many workers are allowed. The default is now 1 s.
The machine is CPU-bound while other lanes build (100 % load in the proof run), so more workers than cores do not help; the governor watches memory, not CPU.

## 2. The drift gate: `tools/soak/drift.mjs`

```
node tools/soak/drift.mjs --uproject <abs .uproject of a BUILT checkout>              # play and compare with tools/soak/baseline.json
node tools/soak/drift.mjs --uproject <...> --update-baseline --build "<commit> <what>"   # take a new baseline from this build
        [--seeds 100] [--only modes|takes] [--threshold 10] [--max 16] [--out <dir>]
```

It plays, through the shared driver, the **twelve modes** (regular class, at the mode's own level; Marathon with `-playersprint` and `--max-frames 42000`, because its
timer of 36,000 frames equals the harness cap and every match would read "unfinished") and the **eight season takes** (not the fallbacks) with the **novice, regular and good**
class: 12 + 24 = 36 cells of 100 seeds (seeds 1-100). The plan is read from the cartridge folders of the build that is played, so a new mode or take shows as `NEW cell`.

Per cell it keeps the win rate, the unfinished count, the mean frames and a digest of the per-seed hashes; per mode also its character number (`tools/playtest/modes.mjs CHARACTER`).
`tools/soak/baseline.json` is the committed reference. A re-run prints a `MOVED` line for every number that moved more than 10 % **and** by more than a floor (2 win points, or 0.02 of a
character number: one match must not trip it), with the count of binomial sigmas for a win rate; `NEW` and `GONE` cells; and exits 1 if anything moved, a match faulted, or a baseline cell was not played.

The sim is deterministic, so a cell whose digest equals the baseline's is bit-identical and cannot have moved: the gate counts those. A change that touches randomness order moves hashes everywhere
and win rates by sampling noise (about 5 points at 100 seeds around 50 %): read the sigma column before believing a small `MOVED`; a change that moves a number by 4 sigma or a character number by a third is real.

**When to run it.** Before the report of any lane that changes sim, brain, a cartridge or a mode. It needs a built checkout (the worktrees have no binaries: pass the `.uproject` of `clean-main`, or of your own build). It takes about
5 m 27 s for the 3,600 matches (36 cells x 100 seeds, 72 chunks) while other lanes were building and the machine sat at 37 to 39 GB, which kept the governor at 3 to 4 workers; the 8-minute budget holds with room. Each cell is played once per run: a lane that touches only modes uses `--only modes`, only takes `--only takes`. Then, when the moves are intended, run with `--update-baseline --build "<commit> <what changed>"` and land the new `baseline.json` with the lane: the baseline is the record of what the modes and takes were.

Test: `node --test tools/soak/drift.test.mjs` (7 tests: the comparison, the floors, the digest, the plan).

## 3. The baseline as committed (build: clean-main 4da9f36 (vision build, behaves as ee14438: hashes equal A107 36000/36000); 100 seeds per cell, seeds 1-100)

| mode | level | character number | value | win % | human knockdowns / match | matches with a 30 s no-touch stretch % |
|---|---|---|---|---|---|---|
| arsenal | rookie | human abilities used / match | 0.98 | 94 | 3.51 | 0 |
| bruisers | rookie | hits (landed + taken) / play min | 4.88 | 100 | 6.92 | 1 |
| cage | pro | checks landed / match | 4.99 | 18 | 7.48 | 11 |
| fog-of-war | pro | human breakaways / play min | 1.93 | 96 | 5.48 | 2 |
| glass-house | pro | human box seconds / match | 13.18 | 20 | 4.55 | 35 |
| marathon | pro | human sprint frames / match | 3032.77 | 2 | 11.6 | 89 |
| outnumbered | rookie | shots against / play min | 3.06 | 99 | 4.53 | 0 |
| pond | rookie | passes + received / play min | 1.64 | 80 | 0 | 0 |
| power-play | pro | passes + received / match | 7.77 | 41 | 6.5 | 30 |
| shotgun | pro | knockdowns (all skaters) / match | 12.09 | 22 | 4.41 | 25 |
| sniper | pro | human goals per human shot | 0.39 | 10 | 3.19 | 10 |
| sudden-death | all_star | match seconds | 54.48 | 16 | 0.81 | 15 |

Win rate of the regular class in a mode is not comparable to hockey.regular at a different level; the gate only compares a mode with itself. Marathon plays to 42,000 frames: 0 of 100 unfinished (SOAK-2 section 0: at 40,000 14 of 200 were).

| take | level | novice | regular | good |
|---|---|---|---|---|
| take-1 | rookie | 79 | 96 | 100 |
| take-2 | rookie | 94 | 100 | 100 |
| take-3 | pro | 18 | 49 | 73 |
| take-4 | pro | 24 | 45 | 71 |
| take-5 | pro | 15 | 44 | 76 |
| take-6 | pro | 18 | 49 | 73 |
| take-7 | pro | 30 | 60 | 80 |
| take-8 | all_star | 6 | 12 | 34 |

Take-3 and take-6 again play identically (18 / 49 / 73), as SOAK-2 found: the gate will catch the day one of them starts to differ.
The columns `kd` (human knockdowns per match), `idle_s` and `no_touch30_pct` (matches with a 30 s stretch without a human touch) are kept in every baseline cell as read-outs, not gated.

## 4. Proof (build clean-main 4da9f36, which plays as ee14438: the hashes of seeds 1-2 and, below, of 36,000 matches are A107's)

- **The 18-cell matrix, 36,000 matches, through `cells.mjs`: 0 faults, and all 36,000 per-seed hashes equal A107's batch a** (`tools/soak/a107/plan-a.json`). Wall time 26 min in two segments:
  the first run (4 s start stagger) reached 13,000 matches in 11 minutes and was stopped on purpose (the driver is mine; its children finished), to find that the stagger caps it at about 22 matches per second; the second run
  resumed from the partial files (1 s stagger, 100 % CPU load from other lanes) and played the other 23,000 matches in 896 s, 25.6 matches per second, so a straight run is 23.4 minutes. Inside the 25-minute proof number, with the
  caveat that it is CPU-bound while other lanes build: when the machine is idle A95's figure is 24 matches per second per 8 workers.
- **The drift gate**: baseline written in 308 s (3,600 matches, 0 faults). Re-run through the gate: 5 m 27 s, 36 of 36 cells bit-identical, 0 numbers moved, exit 0.
  Negative control (a doctored copy of the baseline, `--from` the same data, no play): Cage's checks landed -20 %, take-3 good +9 points (1.88 sigma) are printed as MOVED and the exit is 1; a +1 point change in take-8 is not flagged.
- Tests: `node --test tools/soak/cells.test.mjs` 11 of 11 (seeds, chunks, the memory band, fault pinning, -Multiprocess in every start, cap passed through, crash requeue with seed and repeat command, start guard exit 2, resume),
  `node --test tools/soak/drift.test.mjs` 7 of 7.

## 5. The read-outs the tracks ask for

- **Knockdowns of the human per match**: in every baseline cell (`kd`) and in every `<label>.jsonl` line (`knockdowns`; `knockdowns_all` for every skater).
- **Who holds the puck during the human's idle stretches**: the playtest line carries only `idle_frames`, `idle_spells` and `max_no_touch`; the holder needs the per-frame trace. Run a cell with `"trace": true` in the plan
  (the driver keeps the logs under `<out>/logs/`) and feed them to `tools/soak/playtest/idle.mjs`; A107's `plan-idle.json` is that cell. The gate keeps the share of matches with a 30 s stretch (`no_touch30_pct`) as the cheap number.
- **Stumbles**: not in the line. Proposed source change (device A, sim owner, not this lane): in `Game/Source/JamCore/AI/JamPlaytester.cpp` where the line is written (about line 883-893, next to `knockdowns_all`) add `"stumbles":%d`,
  counted like `KnockdownsAll` at line 824 from the sim's stumble event or the transition of `StumbleFrames` (`JamSim.h:172`) from 0 to above 0 for the human skater (`ContactStumbles`, `JamSim.h:273`, if it is cumulative); the field is new, so no hash moves. `measure()` in `drift.mjs` then keeps it with one line.

