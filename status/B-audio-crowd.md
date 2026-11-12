# Lane AU2 report: crowd reactions instead of horns (device B), 2026-11-12 about 22:45

Nobody has listened to any of this; I cannot listen. Every statement is from measurements.

## Quota and credits
- Before: used 132,696 of 143,292, remaining 10,596 (stop line 7,000; budget 2,500).
- After: used 133,499, remaining 9,793. **Spent 803 credits** (17 sound-generation calls, all ok, none retried; the ledger header values sum to 803 too). About 11 credits per requested second.
- Ledger: `C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl` (one line per paid call, written by eleven.mjs --ledger).
- Command per take: `node C:/Users/kazda/jam/eleven-tools/eleven.mjs sfx --prompt <text> --seconds <n> --influence 0.5 --out raw/<id>.mp3 --ledger crowd/ledger.jsonl` (beds with --loop). Runner: `crowd/gen.mjs`. One independent generation per variant.

## Prompts (verbatim): common prefix plus the variant text
Common prefix: "Real recording of a small to medium indoor ice hockey rink crowd, a few hundred people, close and lively, not a stadium roar. No horn, no siren, no organ, no music, no announcer, no whistle, no intelligible words."

- GoalFor_a (4 s requested): A sudden explosive eruption of cheering and whoops right from the first instant, loud immediately, short burst of applause, decaying quickly.
- GoalFor_b (4 s requested): A sudden eruption of cheering and whoops right from the first instant, loud immediately, then rising into sustained enthusiastic applause, decaying naturally.
- GoalFor_c (4 s requested): A sudden eruption of cheering right from the first instant, loud immediately, whoops and applause, then rhythmic synchronized clapping at the tail.
- GoalFor_d (4 s requested): A sudden eruption of cheering right from the first instant, loud immediately, whoops and applause with sticks banging on the wooden boards mixed in, decaying naturally.
- GoalFor_e (4 s requested): A sudden eruption of cheering right from the first instant, loud immediately, a smaller warmer crowd of a few dozen people like a pond hockey game, whoops and applause, decaying naturally.
- GoalAgainst_a (3 s requested): A collective disappointed groan, then scattered polite applause.
- GoalAgainst_b (3 s requested): A few visiting fans cheering against a disappointed murmur of the home crowd.
- GoalAgainst_c (3 s requested): A long collective groan of disappointment followed by a low murmur and a few sparse claps.
- Save_a (2 s requested): A sharp collective ooh and gasp from the crowd, then a ripple of applause.
- Save_b (2 s requested): A sudden intake of breath and a short collective gasp, then a quick wave of applause and a few whoops.
- Save_c (2 s requested): A sharp collective ooh of near miss, then a ripple of appreciative applause.
- PeriodEnd_a (3 s requested): Warm applause from the crowd that settles gradually into a murmur.
- PeriodEnd_b (3 s requested): Steady friendly applause with a few whoops that fades and settles.
- Win_a (6 s requested): A long celebration: cheering, applause and whoops that build once to a peak and then decay naturally.
- Win_b (6 s requested): Joyful sustained cheering and applause with whoops, building once and then decaying slowly, with some rhythmic clapping.
- Bed_a (10 s requested, loop flag on): A quiet steady murmur of a waiting crowd, low chatter without words, an occasional single clap, constant level, no peaks.
- Bed_b (10 s requested, loop flag on): A quiet ambient murmur of spectators in an ice rink, soft low chatter without intelligible words, rare scattered claps, constant level, no peaks.

The bed prompts inherit the prefix word "lively"; their own text asks for quiet and constant. No team, operator or project name is in any prompt.

## Takes (48 kHz 16-bit stereo WAV in `C:/Users/kazda/jam/audio-lab/crowd/`)
Onset = time to the first sample over -30 dBFS after trimming. Tonality = largest +-2-bin peak energy over 200 to 4000 Hz band energy / fraction of frames with the same strong peak / mean prominence in dB, over the first second (8192-point FFT).

| file | situation | s | LUFS | true peak dBTP | onset ms | tonality ratio / persist / prom | flags | credits |
|---|---|---|---|---|---|---|---|---|
| GoalFor_a.wav | goalfor | 4 | -16 | -9.2 | 0.8 | 0.124 / 0.19 / 22.1 | tone-ish | 44 |
| GoalFor_b.wav | goalfor | 4 | -16 | -8.2 | 18.7 | 0.3965 / 0.54 / 24.1 | tonal | 44 |
| GoalFor_c.wav | goalfor | 4 | -16 | -6.4 | 0.5 | 0.3805 / 0.35 / 23.7 | tone-ish | 44 |
| GoalFor_d.wav | goalfor | 4 | -16 | -8 | 2.7 | 0.112 / 0.23 / 17.9 | - | 44 |
| GoalFor_e.wav | goalfor | 4 | -16 | -7.4 | 0.8 | 0.1273 / 0.35 / 15.1 | tone-ish | 44 |
| GoalAgainst_a.wav | goalagainst | 3 | -20 | -11.3 | 1.3 | 0.0726 / 0.04 / 12.3 | - | 33 |
| GoalAgainst_b.wav | goalagainst | 3 | -20 | -12.9 | 0.8 | 0.191 / 0.27 / 20.1 | tone-ish | 33 |
| GoalAgainst_c.wav | goalagainst | 3 | -20 | -11.3 | 0.7 | 0.164 / 0.15 / 23.1 | tone-ish | 33 |
| Save_a.wav | save | 2 | -20 | -12.8 | 0.9 | 0.1614 / 0.15 / 17.2 | tone-ish | 22 |
| Save_b.wav | save | 2 | -20 | -7.3 | 0.3 | 0.0668 / 0.08 / 13.8 | - | 22 |
| Save_c.wav | save | 2 | -20 | -9.7 | 1.9 | 0.1885 / 0.15 / 18.3 | tone-ish | 22 |
| PeriodEnd_a.wav | periodend | 3 | -20 | -10.3 | 0.9 | 0.0437 / 0.08 / 12 | - | 33 |
| PeriodEnd_b.wav | periodend | 3 | -20 | -10.3 | 1.4 | 0.0862 / 0.46 / 17 | tone-ish | 33 |
| Win_a.wav | win | 6 | -17 | -6.9 | 1.5 | 0.1147 / 0.23 / 19.1 | tone-ish | 66 |
| Win_b.wav | win | 6 | -17 | -6 | 0.7 | 0.1418 / 0.31 / 21.7 | tone-ish | 66 |
| Bed_a.wav | bed | 9 | -30 | -11.7 | 0.6 | 0.0602 / 0.04 / 12 | - | 110 |
| Bed_b.wav | bed | 9 | -30 | -12.3 | 0 | 0.0298 / 0 / 9.8 | - | 110 |

Positive control: the old SFX_GoalHorn_02 measures ratio 0.366, persistence 0.81, prominence 46.8 dB at 949 Hz. Crowd takes sit at prominence 10 to 24 dB, persistence 0 to 0.54. Rule: **tonal** = prominence >= 30 dB or (ratio >= 0.25 and persistence >= 0.5); **tone-ish** = ratio >= 0.12 or persistence >= 0.4 or prominence >= 18 (weaker; a whoop, shout or clap ring is enough). The thresholds are mine, calibrated on one horn and 17 takes; not a validated detector.

### What the numbers say
- All five goal takes start loud: onsets 0.5 to 2.7 ms except GoalFor_b at 18.7 ms (all under 60 ms; no late starters). Leading silence after trimming is 0 to 0.8 ms (limit 20 ms). GoalFor_b also starts softly (first 150 ms is 11 dB under its average) and has the most tone-like content (ratio 0.40, persistence 0.54, 24 dB): flagged **tonal**; do not use it without listening. GoalFor_c has ratio 0.38 but persistence 0.35 (one strong narrow component, probably a clap or whoop): tone-ish.
- Loudness: all one-shots within 0.1 LU of target; true peaks -6 to -13 dBTP, so the -1.5 dBTP ceiling holds without a limiter (none applied). Crowd noise has a low crest factor, so these are hot in RMS but low in peak.
- Durations match the request after trimming (+-0.2 s); tails fade over 0.2 s (0.08 s for the 2 s takes).
- Beds are 9.0 s (10 s generated, 1 s used for an equal-power crossfade of the end into the start; the seam is inside the file). Seam: RMS of last 250 ms vs first 250 ms: Bed_a -34.6 vs -34.9 dB (-0.3), Bed_b -33.4 vs -34.8 dB (-1.4); sample step across the wrap 0.07x (Bed_a) and 1.46x (Bed_b) of the mean step. Loudest 100 ms over the bed level: Bed_a +0.9 dB, Bed_b -0.9 dB, so no peaks. Whether a single clap repeats noticeably is not verifiable without listening.

## Defaults pre-selected on the page (rule: no tonal flag, then no tone-ish flag, then lowest onset)
| group | pre-selected |
|---|---|
| goal for (up to 3) | GoalFor_d (only goal take with no flag, 2.7 ms), GoalFor_c (0.5 ms, tone-ish), GoalFor_a (0.8 ms, tone-ish). GoalFor_e (tone-ish, 0.8 ms) is next; GoalFor_b excluded as tonal. |
| goal against | GoalAgainst_a (no flags) |
| save | Save_b (no flags, 0.3 ms) |
| period end | PeriodEnd_a (no flags) |
| win | Win_b (both wins are tone-ish; b has the faster onset, 0.7 ms) |
| bed | Bed_b (both clean; b has the lower loudest-100-ms excess, -0.9 dB vs +0.9 dB) |

Variant character (a explosive, b rising, c clapping tail, d sticks on boards, e small warm crowd) is from the prompts only; whether the model produced it is unverified.

## Page
`C:/Users/kazda/jam/audio-lab/crowd/pick.html` (open locally; WAVs sit beside it; relative paths, all 17 verified to exist; the inline script was extracted and passes `node --check`). One group per situation; checkboxes with a limit of three for goal for, radios elsewhere; defaults above; picks in localStorage inside try/catch; Copy picks (clipboard with select-and-copy fallback); Reset to measured defaults. Not exercised in a browser.
Manifest: `C:/Users/kazda/jam/audio-lab/crowd/manifest.json` (file, situation, seconds, LUFS, true peak, onset, flags, tonality, loop seam, prompt, credits). Also `measure.json`, runners `gen.mjs`, `proc.mjs`, `page.mjs`, unprocessed mp3 in `crowd/raw/`.
Processing: decode to 48 kHz stereo float; trim to 10 ms before the first sample over -45 dBFS and end 100 ms after the last over -55 dBFS; 2 ms fade-in; 0.2 s (0.08 s) fade-out; linear gain to the LUFS target (ffmpeg ebur128); alimiter only if true peak would pass -1.5 dBTP (never needed); 16-bit, no dither.

## Provenance
ElevenLabs sound-generation (default model eleven_text_to_sound_v2), prompt_influence 0.5, Creator (paid) plan; **ai-generated**. Licence note from the morning report (AU1): the Terms of Use let paid plans use output commercially and keep their rights in it; output may not be unique across users; output must be reviewed before distribution (s.4(k)); no attribution stated for paid plans; nothing game-specific found for sound effects; the organisers' rules on AI-generated audio still need confirming. Keep the ledger as the provenance record.

## What I could not verify
Everything audible: that the takes are crowds at all (not a stadium roar, not synthetic), that none contains a horn, siren, organ, whistle, announcer or intelligible words (the tonality numbers catch only sustained narrow-band tones; a short whistle or a spoken word would pass), that the variants match their character, that loop seams are inaudible, that clapping tails or board sticks are present, and whether any take sounds synthetic (no spectral evidence beyond the tone test, so no claim). Not tried in a browser or the engine. Tonal thresholds are calibrated on one horn only.
