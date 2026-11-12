# Lane VOX report: commentator C1, first batch (device B), 2026-11-12 about 15:10

Nobody has listened to any of this. Every judgement is from numbers. Page: `C:/Users/kazda/jam/audio-lab/vox/listen.html` (relative `src` into `wav/`; all 58 resolve; the script passes `node --check`).

## Quota and cost

- Quota before: 127,192 used of 143,292 (16,100 remaining). After: 127,679 used (15,613 remaining). **Spent 487 credits** (the ledger's per-call `credits_header` sums to the same 487), budget was 2,500.
- 56 requests (38 first-40 lines, 18 variants) at **0.438 credits per character**; per line 4 to 14 credits (mean 8.7; 12 characters = 5, 32 characters = 14). Voice C1 id from `audio-lab/commentators/saved.json`; `eleven_multilingual_v2`, stability 0.35, style 0.4, default similarity and speed (as AU1's commentator samples).
- Reused, 0 credits (copied word for word from AU1's C1 samples): `period.take_1.01` (Welcome) and `clock.last_10.01` (TenSeconds). The other seven AU1 texts are not in the first 40 or differ (e.g. AU1 "Blue wins the faceoff." vs bank "Blue wins the draw.").
- **Projection for the remaining 300 commentator lines**: 7,409 characters x 0.438 = **about 3,250 credits** (range 3,100 to 3,500); it fits in the 15,613 remaining (about 21 percent of it). The whole 340-line bank would be about 3,620 credits. No director lines were generated (voice not picked).

## What I varied for the delivery variants

Same voice, model and text-to-speech call; only these fields (all via the API: `stability`, `style`, `speed`, and the text):

| Variant | stability | style | speed | text |
|---|---|---|---|---|
| v1 calm | 0.55 | 0.10 | 0.95 | bank text unchanged |
| v2 house | 0.35 | 0.40 | 1.00 | bank text unchanged (the same setting as the first 40) |
| v3 hyped | 0.18 | 0.85 | 1.08 | capitals on the key word and a closing "!": "BLUE SCORES!", "ORANGE SCORES!", "Oh, that is a BIG hit!", "From BEHIND! That is a foul!", "The director has made a CALL!", "THAT'S A WRAP!" |

Six lines: goal for blue = `goal.blue.01`; goal against = `goal.orange.01` (orange scoring; `goal.against.01` "That one stings." is the mood line, not generated as a variant); big check = `check.big.01`; foul = `foul.behind.01`; Director's Call = `directors_call.any.01`; "That's a wrap!" = the bank's director line `dir.win.01` text, spoken here in the commentator's voice (files `VO_Commentator_thats_a_wrap_v1..v3`; the bank has no commentator line with exactly that text). Files: `VO_Commentator_<id>_v1/_v2/_v3`. v3 is the risk: two came out long (`goal_orange_01_v3` 4.46 s, `thats_a_wrap_v3` 3.54 s: the capitals probably made the voice draw the words out or add a pause) and `check_big_01_v3` is 2.62 s.

## Post-processing

Own script `post.mjs` (make_ready.mjs not touched): decode to mono 48 kHz, trim to the first sample above -60 dBFS (2 ms pre-roll) and the last 5 ms RMS above -60 dBFS, fades 2 ms in / 10 ms out, linear gain to -18 LUFS (ebur128), capped at -1.5 dBTP, 16-bit WAV in `vox/wav/`; then re-measured from the written WAV. Where the peak ceiling stopped the gain the file is a little under -18 (worst -20.4 LUFS, `goal_orange_02`); 29 files are within 0.3 LU of target or exactly on it, none is more than 2.4 LU under. Re-measure data: `measure.json`.

Flags: **longer than 2.5 s**: `period_take_1_01` 4.22, `period_take_2_01` 3.27, `period_end_any_01` 3.17 (stoppage lines, fine by the plan), `goal_match_point_01` 2.56, `goal_orange_01_v3` 4.46, `thats_a_wrap_v3` 3.54, `check_big_01_v3` 2.62, `foul_behind_01_v3` 2.52. **Leading silence over 150 ms before trimming**: none (largest 44 ms, `breakaway_blue_01`). **Under 0.4 s**: none (shortest `save_any_01` 0.72 s). Note the 2.5 s voice gate: live-play lines of 2.0 to 2.5 s (`breakaway_blue_01` 2.13, `faceoff_blue_01` 2.23, `faceoff_drop_02` 2.01, `knockdown_any_01` 2.02, `foul_no_puck_01` 2.47, `goal_blue_02` 2.44, `goal_orange_02` 2.29, `directors_call_any_02` 2.28, `lift_clean_01` 2.32, `clock_last_10_01` 2.24) are near the plan's 2.0 s limit for quick lines; the voice is slow ("deliberate" per AU1), so consider the speed setting 1.05 for the rest of the bank.

## The lines

| id | text | s | LUFS | flags |
|---|---|---|---|---|
| period.take_1.01 (reused) | Welcome to the sound stage: blue against orange, take one. | 4.22 | -19 | longer than 2.5 s |
| period.take_2.01 | Take two. Same set, new chances. | 3.27 | -18.2 | longer than 2.5 s |
| faceoff.drop.01 | Puck is down. | 0.94 | -18 |  |
| faceoff.drop.02 | Here comes the drop. | 2.01 | -18 |  |
| faceoff.blue.01 | Blue wins the draw. | 2.23 | -18 |  |
| faceoff.orange.01 | Orange wins the draw. | 1.74 | -18 |  |
| faceoff.after_goal.01 | Back to centre ice. | 1.37 | -18 |  |
| shot.blue.01 | Blue shoots! | 1.26 | -18 |  |
| shot.blue.02 | Shot from blue! | 1.49 | -18.6 |  |
| shot.orange.01 | Orange shoots! | 1.37 | -18.2 |  |
| shot.orange.02 | Shot from orange! | 1.07 | -18 |  |
| shot.hard.01 | What a rocket! | 0.9 | -18 |  |
| save.any.01 | Blocked! | 0.72 | -18 |  |
| post.any.01 | Off the post! | 1.24 | -19.3 |  |
| goal.blue.01 | Blue scores! | 1.4 | -18 |  |
| goal.blue.02 | In! Blue scores! | 2.44 | -20 |  |
| goal.orange.01 | Orange scores! | 1.32 | -18.1 |  |
| goal.orange.02 | In! Orange scores! | 2.29 | -20.4 |  |
| goal.for.01 | What a finish! | 0.99 | -18 |  |
| goal.against.01 | That one stings. | 1.64 | -18.8 |  |
| goal.tie.01 | All square! | 1.38 | -18 |  |
| goal.match_point.01 | One more and that is the match. | 2.56 | -18 | longer than 2.5 s |
| check.blue.01 | Blue finishes the check. | 1.59 | -19.7 |  |
| check.orange.01 | Orange finishes the check. | 1.45 | -18.9 |  |
| check.big.01 | Oh, that is a big hit! | 1.46 | -18.1 |  |
| knockdown.any.01 | Down goes the carrier. | 2.02 | -18 |  |
| poke.clean.01 | Poked away! | 1.05 | -18.1 |  |
| poke.miss.01 | Swing and a miss. | 1.68 | -18.4 |  |
| lift.clean.01 | Lifts the stick, takes the puck! | 2.32 | -18.4 |  |
| foul.behind.01 | From behind! That is a foul. | 2.07 | -18 |  |
| foul.no_puck.01 | No puck there. That is a foul. | 2.47 | -18.6 |  |
| directors_call.any.01 | The director has made a call. | 1.62 | -18 |  |
| directors_call.any.02 | A rule change, live on set. | 2.28 | -20.1 |  |
| clock.last_10.01 (reused) | Ten seconds left in the take. | 2.24 | -19.5 |  |
| period_end.any.01 | And that is the end of the take. | 3.17 | -19.2 | longer than 2.5 s |
| win.any.01 | And that is the match! | 1.39 | -19.1 |  |
| win.any.02 | Victory! What a performance. | 1.75 | -18.6 |  |
| lose.any.01 | Not this time. | 1.15 | -18.8 |  |
| lose.any.02 | The other side takes it. | 1.76 | -18.3 |  |
| breakaway.blue.01 | Blue is in alone! | 2.13 | -18 |  |
| goal_blue_01_v1 | Blue scores! | 1.17 | -18 |  |
| goal_blue_01_v2 | Blue scores! | 1.17 | -18.9 |  |
| goal_blue_01_v3 | BLUE SCORES! | 1.84 | -18 |  |
| goal_orange_01_v1 | Orange scores! | 1.4 | -18 |  |
| goal_orange_01_v2 | Orange scores! | 1.83 | -18 |  |
| goal_orange_01_v3 | ORANGE SCORES! | 4.46 | -20.1 | longer than 2.5 s |
| check_big_01_v1 | Oh, that is a big hit! | 1.36 | -18 |  |
| check_big_01_v2 | Oh, that is a big hit! | 2.2 | -18 |  |
| check_big_01_v3 | Oh, that is a BIG hit! | 2.62 | -18.4 | longer than 2.5 s |
| foul_behind_01_v1 | From behind! That is a foul. | 2.22 | -18 |  |
| foul_behind_01_v2 | From behind! That is a foul. | 2.34 | -18 |  |
| foul_behind_01_v3 | From BEHIND! That is a foul! | 2.52 | -18 | longer than 2.5 s |
| directors_call_any_01_v1 | The director has made a call. | 1.8 | -18 |  |
| directors_call_any_01_v2 | The director has made a call. | 1.73 | -18.5 |  |
| directors_call_any_01_v3 | The director has made a CALL! | 2.16 | -18 |  |
| thats_a_wrap_v1 | That's a wrap! | 1.12 | -18 |  |
| thats_a_wrap_v2 | That's a wrap! | 1.03 | -18 |  |
| thats_a_wrap_v3 | THAT'S A WRAP! | 3.54 | -18 | longer than 2.5 s |

## Could not verify

Anything audible: clipped starts, how "blue" and "orange" are stressed, whether lines ending in "!" sound like questions, whether v3's capitals sound excited or shouted, why the two v3 lines ran long, accent drift, breaths, whether the voice sounds like a real broadcaster. The quick-line length and the gate are numbers only. The 487 credits are the ledger and the quota endpoint agreeing; the projection assumes the same 0.438 per character. Reused AU1 recordings were made in the same session settings but are not byte-regenerated, so a different take of the same voice may differ slightly in level (they were re-normalised like the rest).

Files: `C:/Users/kazda/jam/audio-lab/vox/` (`mp3/`, `wav/`, `ledger.jsonl`, `measure.json`, `table.md`, `listen.html`, scripts `gen.mjs`, `post.mjs`, `page.mjs`, `page.tpl`).
