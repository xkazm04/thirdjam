# Lane VOX2 report: full commentator bank, voice C1, hyped (device B), 2026-11-12 about 16:10

Nobody has listened to any of this; every judgement is from numbers.

## Quota and cost
- Before: 127,679 used of 143,292 (15,613 remaining). After: 131,523 used (**11,769 remaining**, above the 8,000 floor). **Spent 3,844 credits** (ledgers agree: experiment about 215 + bank about 3,629), budget 6,000.
- 340 bank requests, 0 failed, no POST retried. Rate about 0.44 credits per character.

## Length experiment (8 quick lines x 3 treatments; stability .18, style .85; ledger `vox2/exp/ledger.jsonl`, rows `exp/rows.json`)
a = capitals on key word + "!", speed 1.08; b = normal case + "!", speed 1.08; c = normal case + "!", speed 1.15. Seconds after trim:

| line | a | b | c |
|---|---|---|---|
| Robbed! | 0.72 | 0.76 | 1.11 |
| Blue shoots! | 1.09 | 1.76 | 0.85 |
| In! Blue scores! | 2.24 | 2.00 | 2.44 |
| The comeback is on! | 1.48 | 2.39 | 1.97 |
| Speed boost on the ice. | 1.94 | 2.09 | 1.50 |
| Orange is a skater short. | 2.37 | 1.79 | 1.56 |
| The puck can really fly now. | 2.80 | 1.97 | 1.89 |
| One goal in it, ten seconds left! | 2.30 | 2.33 | 2.48 |
| **median / min** | **2.24 / 0.72** | **2.00 / 0.76** | **1.89 / 0.85** |

Hype numbers (mean over the 8): peak-to-loudness 16.0 / 15.7 / 17.1 dB; crest 18.0 / 17.5 / 18.9 dB; pitch spread (Node autocorrelation, semitone sd) 6.3 / 7.6 / 6.7. Loudness range came out 0 for every clip (clips too short for ebur128 LRA; useless). The pitch tracker is crude (octave jumps, F0 pinned at 400 Hz in places). **No treatment is separable as "more hyped" by these numbers**, so (a) is not shown to be clearly hyped; per-line duration also varies a lot between takes (same text, different render), so the table is noisy.

**Rule adopted: every commentator line is spoken in normal capitalisation with the closing mark replaced by "!", stability .18, style .85, speed 1.15 (treatment c), because it is the only one with a median under 2.0 s (1.89) and none under 0.5 s.** (Stoppage lines use the same rule.)

## Bank result
- 340 commentator lines generated, 0 failed (director's 25 not generated). Quick-play lines (287): median 1.66 s, 90th percentile 2.46 s, max 5.46 s; 71 are over 2.0 s. 53 stoppage lines (events period, period_end, season_take, win, lose, and goal.pause.*; the generate-plan.md definition) are exempt from the 2.5 s flag.
- Post-processing as batch 1 (mono 48 kHz 16-bit, -60 dBFS trim, 2/10 ms fades, -18 LUFS, -1.5 dBTP), measured from the written WAV.
- **Flags: 26 lines over 2.5 s** (quick lines), 0 under 0.5 s, 0 with leading silence over 150 ms, 0 more than 4 LU under target. Worst: goal.lead.01 5.46, goal.comeback.01 4.64, goal.first.02 4.49, goal.blue.02 4.09, goal.orange.01 3.60, foul.no_puck.02 3.42, pickup_spawn.reverse_other.02 3.29, faceoff.after_foul.01 3.31. Full list is in `measure.json` and on the page (flagged first). A re-render of a flagged line is about 10 credits and the time varies per take, so redoing the 26 costs roughly 300 credits (about 11,400 would remain); not done, awaiting a decision.
- **Totals: 639 s of audio (340 lines), 58.5 MB of WAV.**

## Files (all under `C:/Users/kazda/jam/audio-lab/vox2/`)
- Manifest: `C:/Users/kazda/jam/audio-lab/vox2/manifest.json` (id, speaker, event, kind, team, priority, cooldown_s, text, wav, seconds; totals).
- Page: `C:/Users/kazda/jam/audio-lab/vox2/listen.html` (grouped by event, flagged first, ticked = keep, "show only flagged", running count, localStorage in try/catch, "Copy rejected ids"). All 340 `wav/` paths it builds exist; the extracted script passes `node --check` (the only newline in strings is via `String.fromCharCode(10)`, no `\n` in JS strings).
- `wav/` (340), `mp3/` (340), `ledger.jsonl`, `measure.json`, scripts `lib.mjs`, `exp.mjs`, `gen.mjs` (resumable, 2 attempts max per id, 5,900-credit guard), `post.mjs`, `page.tpl`.

## Could not verify
Anything audible: whether "!" on calm lines (e.g. "Take two. Same set, new chances!") sounds wrong, whether speed 1.15 sounds rushed or garbled, whether any line is clipped, mispronounced, or drifts in accent, whether "blue"/"orange" are stressed well, whether the hype is real (the numbers did not separate treatments). The 26 long lines are long by duration only; the cause (pauses, stretched words) is unknown. Pitch and LRA numbers are unreliable as noted. Manifest `kind` is omitted for lines with no kind, per the bank.

## Re-render and delivery check (follow-up, about 16:40)
- **Credits spent: 384** (budget 500). Quota after: 131,907 used, **11,385 remaining** (floor 8,000).
- Re-render: the 26 over-2.5 s quick lines got up to two more takes with the adopted rule (`redo.mjs`; take 3 only where take 2 was still over). Every take is kept in `vox2/alt/` (`<name>_t1/_t2/_t3.mp3`); the shortest take of at least 0.5 s is the one in `mp3/`. **24 of 26 are now at or under 2.5 s** (longest kept take among them 2.42 s; takes of each are in `redo-result.json`). Quick-play median still 1.66 s; bank now 606.6 s, 55.55 MB.
- **Still over 2.5 s after three takes (2):**
  - `goal.first.02`, 3.36 s, "And the first one is in the can." (8 words: a shorter text is the fix, lead decides)
  - `lift.clean.01`, 2.52 s, "Lifts the stick, takes the puck!" (6 words, only 0.02 s over; a fourth take was not made because the brief allowed two more)
- `manifest.json`, `measure.json`, `ledger.jsonl` (all POSTs, 0 failed) and the flags on `listen.html` are updated: 2 flagged lines in total now.
- **Delivery check** is the first section of `listen.html`: the 8 experiment lines in (a) v3 as picked, (b) normal case speed 1.08, (c) as the bank was generated, clips in `vox2/check/` at -18 LUFS, one radio group (bank is fine as generated / regenerate as v3 exactly (a) / use (b)), stored in localStorage under its own key (try/catch) and added to the copied text (the button is now "Copy verdict and rejected ids"). Checks passed: all 340 `wav/` paths and all 24 `check/` paths exist; the extracted script passes `node --check`. Still unverified by ear: everything.
