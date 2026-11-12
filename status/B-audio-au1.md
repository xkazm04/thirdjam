# Lane AU1 report: ElevenLabs audio lab (device B), 2026-11-12 about 13:00

Nobody has listened to any of this. Every judgement below is from numbers (duration, LUFS, true peak, loop-edge level) and from the prompt; the listening page is the real test.

## API facts (all pages read 2026-11-12)

| Topic | Fact | Source |
|---|---|---|
| Auth | header `xi-api-key: <key>`; base `https://api.elevenlabs.io` (regional hosts exist) | elevenlabs.io/docs/api-reference/text-to-speech/convert |
| Quota | `GET /v1/user/subscription`: `tier`, `status`, `character_count` (used), `character_limit`, `next_character_count_reset_unix`, `max_credit_limit_extension`, `can_use_instant_voice_cloning`. No commercial-use field in the response | .../api-reference/user/subscription/get |
| Voices | `GET /v2/voices?search=&category=premade&page_size=100` | .../api-reference/voices/search |
| TTS | `POST /v1/text-to-speech/{voice_id}`, body `text`, `model_id` (default `eleven_multilingual_v2`; list via `GET /v1/models`), `voice_settings`, `seed`; `?output_format=` mp3_44100_128 default (also mp3_*, opus_*, pcm_*, wav_*; 44.1 kHz PCM/WAV needs Pro; 192 kbps MP3 needs Creator) | .../text-to-speech/convert, .../models/list, .../overview/capabilities/text-to-speech |
| SFX | `POST /v1/sound-generation`, body `text`, `duration_seconds`, `prompt_influence` (0..1, default 0.3), `loop` (bool), `model_id` `eleven_text_to_sound_v2` (default) or `_v3`. Docs say 0.5 to 30 s on the reference page, 0.1 to 30 on the capability page; **the API rejected 0.3 s with "expected >= 0.5"**, so 0.5 s is the real minimum. Docs say 40 credits per second when a duration is set | .../text-to-sound-effects/convert, .../overview/capabilities/sound-effects |
| Music | `POST /v1/music`, body `prompt`, `music_length_ms` (3000 to 600000), `force_instrumental`, `model_id` (`music_v1` removed 2027-01-31, `music_v2`, `music_v2_5`), `seed`; paid plans only; docs give no credit cost | .../api-reference/music/compose, .../overview/capabilities/music |
| Credits | **The docs' 40 credits/s for SFX did not match what this account was charged** (see below). The `character-cost` response header is returned for TTS and SFX, absent for music | measured |

**Plan:** `creator`, status `active`, no usage-based overage (`max_credit_limit_extension` 0), instant voice cloning available (not used).

**Quota:** before 123,506 used of 143,292, remaining 19,786. Budget set: 25 percent of remaining = 4,946. After everything (including 5 clapperboard retries): 125,737 used, remaining 17,555. **Spent 2,231 credits = 11.3 percent of what remained (45 percent of the budget).** The quota endpoint lags a call or two, so per-call credits are the header values where present (TTS 2 to 7 per line, SFX 6 to 88 per call, scaling with duration); the two music tracks cost roughly 1,380 together (derived from the total, not read directly). Reset: unix 1794564524.

### Licence for shipping generated audio (as far as the docs say)
- Terms of use s.1(c): free plan "may only use the Services for non-commercial purposes"; paid users "may use the Services for commercial purposes" (within the prohibited-use policy). s.4(c)(ii): "you retain all rights in and to your Output". s.10: output "may not be unique across users". s.4(k): you must review output before distribution. No attribution requirement stated for paid plans. Source: elevenlabs.io/terms-of-use.
- TTS capability page: "commercial usage rights are only available with paid plans"; ownership stays with you if you own the input IP. This account is on Creator (paid).
- Music has its own terms (elevenlabs.io/music-terms): media rights for Free through Enterprise Music Lite are "except film, TV, radio, and Studio Games"; a Studio Game is a public video game that is *not* an Indie Game; Indie Game means lifetime game revenue under $500,000 and developer revenue under $1,000,000 last financial year (s.4.8(j), s.4.7). Free needs attribution, other plans do not. Creator is "individuals only" and streaming distribution is permitted. My reading (the page does not say it in one sentence): a jam game with no revenue is an Indie Game and so is not carved out. **Not verified: which music-terms column the Creator tier maps to, and whether an itch.io/Epic-jam submission counts as "made available to the public" under the carve-out. The operator should read s.4 of the music terms and decide.**
- Silent in all the docs I could reach: whether SFX and TTS output has any game-specific restriction; the support-centre article on publishing returned HTTP 403, so it was not read. Voice licence: only stock `premade` voices were used.

## Tools
`C:/Users/kazda/jam/eleven-tools/eleven.mjs` (quota, voices, tts, sfx, music, `--dry-run`, `--ledger`, last stdout line `{"tool":"thirdjam.eleven/1",...}`, POST never retried), `README.md`, plus the throwaway runners `batch1.mjs`, `post.mjs`, `page.mjs`. Ledger: `C:/Users/kazda/jam/audio-lab/ledger.jsonl`. Key was read from `.env`, sent only in the header, never written.

## What was generated
Output folder `C:/Users/kazda/jam/audio-lab/` (mp3), `wav/` (48 kHz 16-bit, silence trimmed at -55 dB peak; mono for voice and SFX, stereo for music; the three SFX loops and the room tone are NOT trimmed). Voices: **Bill** (`pqHfZKP75CvOlQylNhV4`, premade, "Wise, Mature, Balanced", old male American: the "old director"; not truly gravelly, `Callum` is the gravelly one but reads as a trickster) and **Liam** (`TX3LPaxmHKxFdv7VOQHJ`, premade, "Energetic", young male American: the crisp one). Settings: multilingual_v2, stability 0.4 (Bill) / 0.3 (Liam), style 0.5. 24 voice lines, 29 SFX files (26 planned plus 3 retries), 2 music tracks. SFX used prompt_influence 0.5 and one request per file (no variant parameter, so the "two variants" are two independent generations of the same prompt).

Table (LUFS is integrated; for clips under 0.4 s it is undefined, "n/a"):

| file | prompt or text | voice | s (trimmed) | LUFS | peak dBTP | note |
|---|---|---|---|---|---|---|
| MUS_MatchLoop_01.mp3 |  |  | 59.83 | -11.4 | -0.3 | loop edges differ -53 dB |
| MUS_MenuTheme_01.mp3 |  |  | 44.76 | -10.7 | -1.0 | loop edges differ -46.7 dB |
| SFX_BodyCheck_01.mp3 |  |  | 1.48 | -27.2 | -4.2 |  |
| SFX_BodyCheck_02.mp3 |  |  | 1.48 | -18.8 | 0.0 | peak at full scale |
| SFX_Clapperboard_01.mp3 |  |  | n/a | n/a | n/a | too short for LUFS; effectively silent |
| SFX_Clapperboard_02.mp3 |  |  | 0.41 | n/a | -51.8 | too short for LUFS |
| SFX_Clapperboard_03.mp3 |  |  | 0.15 | n/a | -52.8 | too short for LUFS |
| SFX_Clapperboard_04.mp3 |  |  | 0.30 | n/a | 0.3 | too short for LUFS; peak at full scale |
| SFX_Clapperboard_05.mp3 |  |  | 0.72 | -53.2 | -34.3 | very quiet |
| SFX_GoalHorn_01.mp3 |  |  | 2.48 | -7.3 | 0.1 | peak at full scale |
| SFX_GoalHorn_02.mp3 |  |  | 2.32 | -6.7 | 0.1 | peak at full scale |
| SFX_PickupChime_01.mp3 |  |  | 0.80 | -29.8 | -15.6 |  |
| SFX_PickupChime_02.mp3 |  |  | 0.80 | -31.7 | -16.4 |  |
| SFX_ProjectorLoop_01.mp3 |  |  | 5.00 | -26.9 | -7.8 |  |
| SFX_ProjectorLoop_02.mp3 |  |  | 5.00 | -20.0 | -2.4 |  |
| SFX_PuckBoards_01.mp3 |  |  | 1.00 | -26.8 | -5.6 |  |
| SFX_PuckBoards_02.mp3 |  |  | 0.49 | -15.9 | -0.2 |  |
| SFX_PuckPost_01.mp3 |  |  | 0.52 | -46.9 | -27.1 | very quiet |
| SFX_PuckPost_02.mp3 |  |  | 1.48 | -41.9 | -20.7 | very quiet |
| SFX_RuleChangeSting_01.mp3 |  |  | 1.48 | -14.3 | 0.0 | peak at full scale |
| SFX_RuleChangeSting_02.mp3 |  |  | 1.48 | -16.6 | -0.9 |  |
| SFX_SkateCarve_01.mp3 |  |  | 0.85 | -19.4 | -1.0 |  |
| SFX_SkateCarve_02.mp3 |  |  | 0.81 | -22.1 | -0.5 |  |
| SFX_SkateStrideLoop_01.mp3 |  |  | 4.00 | -16.1 | -0.2 | loop edges differ 7.8 dB |
| SFX_SkateStrideLoop_02.mp3 |  |  | 4.00 | -22.8 | -2.6 | loop edges differ -11.6 dB |
| SFX_SlapShot_01.mp3 |  |  | 1.00 | -22.8 | -2.8 |  |
| SFX_SlapShot_02.mp3 |  |  | 0.49 | -37.6 | -13.1 | very quiet |
| SFX_SoundStageRoom_01.mp3 |  |  | 8.00 | -57.7 | -41.2 |  |
| SFX_SoundStageRoom_02.mp3 |  |  | 8.00 | -57.5 | -44.5 |  |
| SFX_UiClick_01.mp3 |  |  | 0.10 | n/a | -5.0 | too short for LUFS; effectively silent |
| SFX_UiClick_02.mp3 |  |  | 0.48 | -23.3 | -0.6 |  |
| VO_Director_Bill_Action.mp3 |  |  | 0.54 | -20.1 | -5.2 |  |
| VO_Director_Bill_Cut.mp3 |  |  | 0.48 | -16.6 | -1.5 |  |
| VO_Director_Bill_CutPrintIt.mp3 |  |  | 1.32 | -19.6 | -3.0 |  |
| VO_Director_Bill_DirectorsCall.mp3 |  |  | 1.01 | -18.4 | -3.2 |  |
| VO_Director_Bill_FromTheTop.mp3 |  |  | 0.77 | -22.4 | -5.8 |  |
| VO_Director_Bill_PlacesEveryone.mp3 |  |  | 1.38 | -21.8 | -4.2 |  |
| VO_Director_Bill_QuietOnTheSet.mp3 |  |  | 0.88 | -21.5 | -7.8 |  |
| VO_Director_Bill_Retake.mp3 |  |  | 0.55 | -21.0 | -6.7 |  |
| VO_Director_Bill_Rolling.mp3 |  |  | 0.58 | -20.3 | -9.4 |  |
| VO_Director_Bill_TakeThree.mp3 |  |  | 0.63 | -20.2 | -7.6 |  |
| VO_Director_Bill_TakeTwo.mp3 |  |  | 0.60 | -20.8 | -6.0 |  |
| VO_Director_Bill_ThatsAWrap.mp3 |  |  | 0.74 | -15.8 | -3.9 |  |
| VO_Director_Liam_Action.mp3 |  |  | 0.51 | -22.3 | -1.5 |  |
| VO_Director_Liam_Cut.mp3 |  |  | 0.41 | -17.9 | -0.6 |  |
| VO_Director_Liam_CutPrintIt.mp3 |  |  | 1.22 | -20.0 | -1.7 |  |
| VO_Director_Liam_DirectorsCall.mp3 |  |  | 0.84 | -21.3 | -3.5 |  |
| VO_Director_Liam_FromTheTop.mp3 |  |  | 0.73 | -22.1 | -6.0 |  |
| VO_Director_Liam_PlacesEveryone.mp3 |  |  | 1.19 | -22.6 | -3.3 |  |
| VO_Director_Liam_QuietOnTheSet.mp3 |  |  | 0.94 | -19.7 | -4.6 |  |
| VO_Director_Liam_Retake.mp3 |  |  | 0.54 | -21.3 | -8.7 |  |
| VO_Director_Liam_Rolling.mp3 |  |  | 0.48 | -21.6 | -7.0 |  |
| VO_Director_Liam_TakeThree.mp3 |  |  | 0.61 | -20.8 | -5.2 |  |
| VO_Director_Liam_TakeTwo.mp3 |  |  | 0.61 | -21.2 | -4.0 |  |
| VO_Director_Liam_ThatsAWrap.mp3 |  |  | 0.91 | -19.5 | -4.1 |  |

## Shortlist from the numbers alone (I cannot listen)
- Clapperboard: only `SFX_Clapperboard_04` (peak +0.3 dBTP, -17 dB RMS, 0.30 s). Takes 01, 02, 03 and 05 are near silent (peaks -57 to -34 dB): the model gave almost nothing for the "clapperboard slate" wording; take 04 used "two wooden sticks slapped together". 01 is entirely silent (wav is empty).
- Slap shot: 01 (-22.8 LUFS, full 1 s) over 02 (-37.6 LUFS, cut to 0.49 s).
- Puck on boards: 02 (-15.9 LUFS, tight 0.49 s, peak -0.2) for punch; 01 (-26.8, 1.0 s) if you want a tail.
- Puck on post: both weak (-47 and -42 LUFS, peaks -27 and -21 dB); 02 is the less bad; it needs +20 dB in the engine and may simply be a faint ring. Regenerate if it sounds thin.
- Goal horn: both hot (about -7 LUFS, 0.1 dBTP, i.e. clipping at the edge); 02 (2.3 s) is closer to the 2.5 s asked. Pull down 10 dB or more.
- Body check: 02 (-18.8 LUFS) louder and fuller than 01 (-27.2); 02 touches 0 dBTP.
- Skate carve: 01 and 02 within 3 dB of each other; no numerical reason to prefer one.
- Rule-change sting: 01 (-14.3) is the stronger; 02 (-16.6) has more headroom.
- Chime and UI click: chimes are quiet (-30 LUFS, peaks -16), fine for a soft cue. UI click 01 is the tight 0.1 s tick (the API minimum was 0.5 s, then trimmed); 02 keeps 0.48 s of body, so 01 is closer to "a tap".
- Voice lines: all 24 sit between -15.8 and -22.6 LUFS with peaks below -0.6 dBTP, so the two voices are interchangeable on the numbers (Bill's "That's a wrap!" is the loudest, -15.8). Lines are 0.26 to 1.4 s after trimming. The choice of voice is a pure ear decision.
- Loops: ProjectorLoop 01 and 02 match at the edges (-0.5 and +0.6 dB over 50 ms), SoundStageRoom 01 and 02 too (-1.1 and +2.3 dB; both about -57 LUFS, a very quiet bed that needs roughly +30 dB, and a lamp hum is hard to confirm without listening). SkateStrideLoop 01 (tail +7.8 dB) and 02 (tail -11.6 dB) do not match at 50 ms, but a stride is rhythmic so a 50 ms window is a rough test: use a short cross-fade in the engine on both. The "loop" flag was accepted by the API without error; whether the seam is truly seamless is unverified.
- Music: both tracks end in a fade (tail 47 and 53 dB below the head), so neither is loop-ready as delivered; trim the fade and cross-fade, or set the engine to loop with a tail. MenuTheme 44.8 s and MatchLoop 59.8 s after trim; both about -11 LUFS (loud for music) at -0.3 and -1.0 dBTP.

Listening page: `C:/Users/kazda/jam/audio-lab/listen.html` (opens locally; the file names must stay next to it; checkbox per take and a "Copy my picks" button). Measurements: `C:/Users/kazda/jam/audio-lab/measure.json`, table `table.md`.

## What failed or was unavailable
- SFX at 0.3 s (UI click): HTTP 400 (min 0.5 s). No credit charged for the two rejected calls as far as the quota shows. Retried at 0.5 s.
- Clapperboard: five takes spent (see above) to get one usable.
- Music was available (`music_v2_5`, `force_instrumental` true) and both tracks were made. Not checked: whether `force_instrumental` truly kept vocals out (no listening).
- Looping: offered (`loop: true`) and accepted; effect unverified.
- Support article on commercial publishing: HTTP 403.

## What I could not verify
Anything audible: voice character ("gravelly" was not achievable from the stock list), whether prompts like "goal horn" or "post ping" came out as intended, loop seams, absence of vocals in the music, no real melody in the music. The music-terms tier mapping and jam-submission status (above). The per-call credit numbers for music. Exact credit price: docs say 40/s for SFX but the account was charged about 11 credits per second.

## Operator decisions
1. Voice: Bill or Liam, or a re-run with another premade (Callum for gravel, Daniel for a broadcaster). Lines are cheap (a full set of 12 costs about 60 credits).
2. Music: keep the two tracks (about 1,380 credits) or not; read s.4 of elevenlabs.io/music-terms first, and decide whether to credit "Created in collaboration with ElevenLabs" (not required on paid plans).
3. Licence: the plan is paid (Creator), so the terms of use allow commercial use of Output; confirm with the organisers' rules on AI-generated audio (theme page says AI use is context only) and keep `ledger.jsonl` as the provenance record.
4. Which takes to import: the engine team needs the picks from the listening page; wavs are in `audio-lab/wav/`.
