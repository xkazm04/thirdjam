# Acceptance matrix (lane A108, track S2, Fri 13 Nov 20:15 to 22:10; Development package of main ee14438 + nothing, `Game/Packaged/A108dev`)

Run folders: `tools/qa/out/accept/m-*` (`*-d1` = the first, disturbed or never-moved attempt, kept). Cuts: `tools/qa/accept/matrix-cuts/` made by `make-matrix-cuts.mjs`
(mode cut = the mode cartridge with `first_to` 9 and penalties off; tier cut = `accept-cut.json` plus the tier). Runner `run-matrix.ps1`, table `summarize-matrix.mjs`.
The machine was NOT quiet: lane a106 launched `A114-after` ThirdJam.exe beside runs (20:16 to 20:32) and 8 to 12 headless editors ran all evening.

## Rows (final attempt; "d1" = first attempt differed)
| cut | verdict | failing rows and what they are |
|---|---|---|
| hockey.regular, accept cut, run 1 | PASS 19/19 | |
| run 2 (d1: FAIL check, a106 warning) | FAIL | shoot_tap charge 7 frames (band <= 4), shoot_half: frame-count timing under load |
| run 3 (d1 and repeat: FAIL) | FAIL | skate/sprint/sprint_carry/brake: the human never moved (see harness finding 1) |
| hockey.regular cartridge itself | FAIL | pass, saucer, onetimer never reached; run hit 420 s. Hypothesis: it is first_to 3 and the three shoot goals end the match (accept cut is 9). Not a game defect |
| bruisers, cage | PASS (clean repeats) | |
| pond | FAIL check, crosscheck | check: by design (check_enabled false). crosscheck: puck_free per mode |
| sniper | FAIL check, crosscheck | check is a stumble (ratio 0.48, closing 6.9): the rule changed the row, see finding 2 |
| outnumbered | FAIL crosscheck | stickhit ok, puck_free=0 |
| power-play | FAIL shoot_full | shot and goal ok; charge 21 frames, band is >= 26: the mode's `one_timer`/charge numbers move the band by design |
| glass-house | FAIL shoot_tap, check | tap 5 frames; check knockdown=0 (mode raises knockdown threshold to 150 frames, ratio gates) |
| marathon | FAIL brake | top speed 16, stop 46 frames, rev_speed 15.6: the long-ice breakaway speed passes the row's 20 m/s^2 threshold by design |
| shotgun | FAIL check | check=0 at closing 12.25 (ability F interferes? unverified) |
| arsenal | FAIL check, resume_aim 1/2 | check stumble ratio 0.30; resume_aim followed 1 of 2 |
| fog-of-war, sudden-death | FAIL skate (never moved) | harness finding 1 |
| tiers l1_day, l2_night, l3_neon, p3_clay | PASS 19/19 | |
| tier p4_broadcast | FAIL check, crosscheck | check stumble ratio 0.66 closing 9.9; crosscheck puck_free=0: same contact rows, not a render fault |
| H26 Shipping zip, clean profile | PASS | 150 s alive in a match (Period 2, 0-1), 1.65 GB working set, `tools/media/out/a108-clean-h26` |

## Real findings
1. **Harness: the first step is lost when the game starts late.** The log is tailed from `Saved\Logs\ThirdJam.log` (the `-abslog` quirk persists); under load the game took 86 s to the drill and the driver read frames 387..1758 in 0.4 s, so every step-1 verb saw no movement (reg3, fog-of-war, sudden-death). `accept.ps1:258` also throws `Substring` exceptions every loop (cosmetic). Fix: act on a live log only after `begin`, tail with a smaller offset, and throw out a run whose first act lagged its line by > 1 s.
2. **The check and cross-check rows are speed-bound under contact by inertia.** At closing 7 to 10 m/s the result is a stumble (ratio 0.3 to 0.66), a knockdown needs ratio >= 1 (closing ~11 in run 1). The row passes only when the driver's W-hold reaches full speed; a slow start (load) fails it. The drill (F2/F1 family) should start the opponent faster or the rule should accept a stumble with the puck loose.
3. **Shoot_tap/half are frame-counted from wall-clock key holds** and fail by 2 to 3 frames when the game hitches (charge 7 vs <= 4).
4. `p4_broadcast` has no `LR_P4` entry in `EffectsBinder.cpp` (`:26-31`, four tiers) and no `TierLabel`; the tier ran as a fallback look without a failure.
5. The Development exe needs the inner `Binaries\Win64\ThirdJam.exe` for `take.ps1` (the root stub is only a launcher in Shipping).
6. Foreign: lane a106 (or an A114 job) ran packaged games and 8+ editors during the cursor's window.

## Stills (`submission/shots/a108-*.png`, 1920x1080)
01 menu: main menu card (Play the season, choose your cut, direct a cut). 02 faceoff, hockey.regular: Match 1 faceoff, DROP ring, orange line across the centre. 03 season take faceoff, night tier.
04 a blue skater flat on the ice beside "YOU 7", "Folded in half and set down gently. Not gently." 05 net front, lift attempt text. 06 two mates moving with one highlighted, "Into your lane." on a clay-ish look.
07 open ice with two mates near the blue line. 08 boards close, "Sent sprawling by a cross-check." text. 09 Arsenal faceoff, clay look, warm wooden stands. 10 Arsenal, an orange skater with the puck on a pickup, clay figures.
11 Arsenal period 2 faceoff, HOLD ring. 12 H26 clean profile, Period 2, 0-1, live play. Gaps: no real 5 v 5 head-on hit, no stumble hold, no twelve-card menu (the take did not open Choose your cut).

## Re-run by lane A131 (Fri 13 Nov 22:30 to 23:48; same package `A108dev`, the cuts of A108; main's later mode cartridges are NOT in these cuts)
`node tools/qa/accept/summarize-matrix.mjs m- --md` is the table. Final attempt per run: `hockey.regular` x5 PASS with no repeat (load: mean cpu 47 %, 26 GB, 6 to 15 build/editor processes,
a foreign game on the machine in some runs; act lag 4 to 17 ms, tap hold 10 ms = 1 sim frame, 5 of 5); regcart PASS+DESIGN (first_to 3 ends the match after step 4); 12 of 12 modes
PASS, PASS+DESIGN (pond: check, crosscheck, checked by `check_enabled=false`; marathon: brake 47 frames at 16 m/s against band 52) or PASS after one automatic repeat of a `machine` run
(power-play, outnumbered, marathon x2); 5 of 5 tiers PASS (p4_broadcast now passes: its check failure was a late press). The by-design rows are in `tools/qa/accept/mode-table.json`.
Not covered: power-play's and glass-house's shot bands needed no entry (the full hold is now 1.5x, the failures were the sim running at 48 Hz); stale cuts for the changed cartridges (regenerate with `make-matrix-cuts.mjs`).
