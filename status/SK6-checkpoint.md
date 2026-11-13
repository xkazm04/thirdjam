# SK6 checkpoint: the figure Blueprints on C-0001 v4 (device B), 01:05

Lane SK6, tree `C:/Users/kazda/jam/wt/b-a45` (branch for `dev-b-a45`: `lane/a45` 1f8d87a + B's `dev-b` content). Editor port 8000 (pid 37264). No build, no git
write. `/Game/Maps/Rink` never saved (no dirty map), `_Scratch/` absent. Another editor was not running on 8002 during my matches, but frame numbers are still not
taken. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk6` (`mv.mjs`, `cd.mjs`,
`cdfix.mjs`, `pin.mjs`, `aim.mjs`, `fix5.mjs`, `ana.mjs`, `rec.py`, `recana.mjs`, logs under `logs/`, DSL backups under `bk/`).

## Part 1: PASS

### 1. State when the editor opened (all seven implement `IJamSkaterVisual`, all `BS_ERROR`)
`BP_SkaterVisual_TB1`, `_TB1_P3`, `_TB2`, `_TB2_P3`, `_TB2_P4`, `_TB2_old`, `_TB2_P3_old`, each with exactly these four errors (verbatim, TB1 shown):

```
LogBlueprint: Error: [AssetLog] ...\Skaters\BP_SkaterVisual_TB1.uasset: [Compiler] Found more than one function with the same name SetBoardPin; second occurance at  <None>
... same name PlayStickHit ... / ... same name PlayHitReaction ... / ... same name PlayBoardHit ...
```
`SetKnockedDown` gave no error: the interface event node had already refreshed to four pins (the body ignored `KnockdownFrames`).
`_TB2_P4` and both `_old` copies also had the four custom events (SK4's older graph), so all seven got the same treatment.

### 2. What each Blueprint implements now (all seven)
- **Interface events** `PlayHitReaction(DirX, DirZ)`, `PlayBoardHit()`, `SetBoardPin(bOn)`, `PlayStickHit(bLifted)` with the bodies of the deleted custom events
  (same takes, same priorities and blend-ins), plus one log line per call: `SK6RX <hit|board|stickhit> <take> played|blocked`, `SK6RX pin on <take> played`,
  `SK6RX pin off stop`, `SK6RX pin ignored on=<b>`, `SK6RX <kind> skipped dt <s>`.
- **`SetKnockedDown(bDown, DirX, DirZ, KnockdownFrames)`**: stores the down time; the fall and get-up takes are picked by direction as before (SK4), their
  lengths read from the takes. With D = KnockdownFrames / 60, fall F (at the knockdown-strength rate), get-up G: scale s = max(1, (F + G) / D); the fall plays
  at rate x s, the get-up starts at D - G / s after the fall began (from `ReceiveTick`, catching up a late tick through the montage start position) at rate s,
  so it ends at D. **KnockdownFrames = 0** (and `SetTripped`, still a Blueprint event, not in the interface): the get-up starts on `SetKnockedDown(false)`.
  The learned gap (`LastGapS` timing, `0.55 / FMax(LastGapS, 0.1)`) is gone; `LastGapS` is only logged.
- Logs: `SK6KD down_f .. fall .. getup .. scale .. up_at_f ..`, `SK4KD <take> rate ..`, `SK4UP <take> at_f .. rate .. start .. end_f .. down_f ..`,
  `SK6UPEND end_f ..`, `SK6REL rel_f .. end_f .. end_minus_rel_f ..` (or `SK6REL fallback`).
- **Reaction cooldown (added, see 3)**: one hit / board / stick-hit reaction per figure per 0.6 s, never while down or while a get-up plays (`bUpRun`);
  `PlayCut` / `PlayAct` / `SetBoardPin` also refuse to start anything over a get-up.
- `_TB2_P4`, `_TB2_old`, `_TB2_P3_old` keep their older takes (P3_old: `Board_Hit` -> `Check_Hit_Back`; face-down falls -> `Fall_Forward`), no knockdown-strength
  bus binding (fall rate 1.0), `PlayAct` (no 0.12 s cut) for the hits.
- Added on R-0032 (coordinator): aim smoothing and a damped blade correction, and (part 2 item 5) the off-screen gate; see below.

### 3. Match proof, humanoid_b1 (`BP_SkaterVisual_TB1`), night, `all_star`, scripted human (`jam.ScriptHuman 1`), seed 1, 7,544 sim frames (~126 s)
| | JAMVIS (device A) | figure line, played | skipped by the cooldown |
|---|---:|---:|---:|
| hit (`PlayHitReaction`) | 104 (106 in run 2) | run 1: 104 / run 2: 72 | 34 |
| board (`PlayBoardHit`) | 54 | 53 (+1 blocked) / 40 | 14 |
| pin on / off (`SetBoardPin`) | 42 / 42 | 42 / 38 (+4 off while not pinned: pin had come on while down) | |
| stickhit (`PlayStickHit`) | 77 (81) | 77 / 32 | 49 |
| knockdowns (`JAMEVENT knockdown`) | 5 | 5 `SK4KD`, 5 `SK4UP` | |
Every JAMVIS line has its figure line in the same engine frame (0 unpaired in run 2).

- **Get-up against the sim's release: 0 frames** on all 5 knockdowns (`end_minus_rel_f` 0.000005), **including the first knockdown of each skater**
  (skaters 0 and 1). 90-frame down time: side knockdowns start the 0.9 s get-up at frame 36; `Fall` (0.6 s) + `GetUp` (1.2 s) is time-scaled x 1.148, get-up at 27.3.
  Under slomo 0.12 (strip): -0.08 frames. Fallback (KnockdownFrames 0, test figures): the get-up starts on the release call.
- **Twitch: yes, before the cooldown.** 104 hit reactions in 126 s (device A counted 37 in 3,300 frames, the same rate), 35 on one skater; 114 of 229 reactions
  came within 0.6 s of the previous one on the same figure; stick hits come in 2-frame bursts (44 stickhit -> stickhit pairs inside 0.6 s, the puck poked back
  and forth). **Set: 0.6 s cooldown shared by hit, board and stick hit** (the brief asked it for hits; the bursts are stick hits). Played after: 72 / 40 / 32.
- Strips (game camera crops): `devlog/shots/20261113-010410-B-sk6-hit-strip.jpg`, `-010411-B-sk6-board-strip.jpg`, `-sk6-stickhit-strip.jpg`,
  `-sk6-knockdown-strip.jpg`. Reads: knockdown and get-up clearly; board hit yes; hit reaction yes at 9 m, small at 16 m; `Lift_Hit` is subtle.

### 4. Other tiers
- **generated_b2, night** (`JAMVISUAL generated_b2 tier=l2_night class=.../BP_SkaterVisual_TB2`): identical sim and identical counts to run 2; 5 knockdowns, all
  0 frames from the release.
- **Clay** (`JAMVISUAL generated_b2 tier=p3_clay class=.../BP_SkaterVisual_TB2_P3`): same counts, 5 knockdowns 0 frames.
- **`_TB1_P3` is not selected by this tree's game**: `humanoid_b1` always resolves to `BP_SkaterVisual_TB1` (HokejGameMode.cpp:167, any tier). `_TB1_P3`,
  `_TB2_P4` and both `_old` copies were proven as test figures in PIE: each played the matching take for hit, board, pin on/off, stick hit, knockdown (90) and
  get-up (`devlog/shots/20261113-010411-B-sk6-tiers-figures.jpg`, right).

### R-0032 stick blink (coordinator's check), default figure, before -> after
Recorded per frame (10 s): test figures driven by `SetStickAim` (sweep +-50 deg / 4 s; step 0 <-> 60 deg every 1.5 s; noise 10 +- 8 deg at 30 Hz, reach 1.3 m),
and the human's figure in the match. Blade position in the skater's frame; frames with a montage or AimW < 1 excluded.

| | before | after |
|---|---|---|
| Step 60 deg: correction peak tilt / turn / slide | 3.5 deg / **12.0 deg (clamp)** / **15.6 cm (clamp)** | 0.5 / 4.1 / 1.5 |
| Step: settle, sign flips | tilt 8-9 frames 1 flip, slide 7-12 frames 1-2 flips: **FAIL** | tilt 0-5 frames 0 flips, slide 0 flips; turn follows the smoothed aim (24 frames to arrive), 1 crossing of +-0.2 deg at the end |
| Noise +-8 deg 30 Hz: blade step p50 / p95 / max | **6.1 / 37.0 / 57.7 cm: FAIL** | 1.07 / 2.45 / 6.97 cm (52 of 369 frames over 2 cm) |
| Sweep: blade step p95 / max | 1.94 / 4.2 cm | 1.94 / 7.1 cm (the sweep's own speed is 1.9 cm a frame) |
| Human in the match: blade step p50 / p95 / max (on ice, no montage) | 0.81 / 3.64 / 6.7 cm (54 usable frames) | 0.14 / 0.49 / 16.9 cm (165 frames, one frame over 2 cm) |
| Flips (stick between two solutions, re-grip) | none seen besides the step overshoot | none |

Cause: the correction is an integrator reading the blade one frame late at gain 0.6, which rings (one-frame-lag loop with gain over 0.25), and the aim layer
follows the raw aim. **Changed in all seven**: `SetStickAim` stores a target; `SK2Tick` follows it critically damped (omega 1/0.06 s, world delta);
`SK3Fix` steps 0.2 of the error per frame (was `FixGain` 0.6) with a 0.3 cm dead zone on height, lateral and radial error. Residual: a +-8 deg 30 Hz input still
moves the blade 2.45 cm a frame at p95; that is the input itself after a 0.06 s filter (a longer time constant would cut it further and lag the aim).

### Validation (01:03)
565 assets under `/JamAssets/Skaters/` loaded, 0 failures; `validate_assets_with_settings`: 565 checked, 565 valid, 0 invalid, 0 warnings; 14 blend spaces, every
sample has an animation (Skate 14, Aim 24); 7 figure Blueprints and 7 AnimBPs compile `BS_UP_TO_DATE` with no warning; no Error or Warning line in the log during
load, validation, compile; no `Accessed None`, divide by zero or Blueprint runtime error in the final session.

### Files changed (all under `Game/Plugins/JamAssets/Content/Skaters/`), bytes
| File | before | after |
|---|---:|---:|
| `BP_SkaterVisual_TB1.uasset` | 1,940,533 | 2,804,420 |
| `BP_SkaterVisual_TB1_P3.uasset` | 1,994,837 | 2,859,016 |
| `BP_SkaterVisual_TB2.uasset` | 1,915,955 | 2,777,253 |
| `BP_SkaterVisual_TB2_P3.uasset` | 1,915,742 | 2,777,372 |
| `BP_SkaterVisual_TB2_P4.uasset` | 1,688,460 | 2,551,602 |
| `BP_SkaterVisual_TB2_old.uasset` | 1,688,227 | 2,551,478 |
| `BP_SkaterVisual_TB2_P3_old.uasset` | 1,687,984 | 2,551,279 |
Nothing else changed in the tree at checkpoint time (checked with `find -newer`). Part 2 later changed nine more files (materials, textures, the TB1_P3 mesh), not needed for v4: see `status/SK6-report.md`. These seven files include part 2 item 5 (the off-screen gate), already applied and proven.

**For the landing window**: take exactly these seven `Skaters/BP_SkaterVisual_*.uasset` from `dev-b-a45`. They need C-0001 v4 (`lane/a45`'s
`JamSkaterVisual.h`); on v3 they would fail to compile (they implement interface events v3 does not have).

Ledger rows (notes update only, sizes in MB):
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 2.67 | ... | 2026-11-13 | B | y | + C-0001 v4: reactions on the interface, get-up timed by KnockdownFrames, 0.6 s reaction cooldown, aim follow 0.06 s, damped blade correction, off-screen gate (lane SK6) |
```
(same note for `_TB1_P3` 2.73, `_TB2` 2.65, `_TB2_P3` 2.65, `_TB2_P4` 2.43, `_TB2_old` 2.43, `_TB2_P3_old` 2.43.)
