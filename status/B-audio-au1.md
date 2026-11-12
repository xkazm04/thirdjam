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

## Sports commentator voices (addition requested by the lead, 2026-11-12)

### API facts (docs read 2026-11-12)
- Design: `POST https://api.elevenlabs.io/v1/text-to-voice/design`. Body `voice_description` (required; no length limit stated), `text` (100 to 1000 characters, or `auto_generate_text`), `model_id` (`eleven_multilingual_ttv_v2` default, or `eleven_ttv_v3`), `loudness` (-1 to 1, default 0.5), `guidance_scale` (default 5), `seed`. Response `previews[]` of `{generated_voice_id, audio_base_64, media_type, duration_secs}`: **three previews per call** (observed). Source: elevenlabs.io/docs/api-reference/text-to-voice/design.
- Save: `POST /v1/text-to-voice` with `voice_name`, `voice_description`, `generated_voice_id`; returns the voice (`voice_id`). The docs name no slot or plan errors. Source: .../api-reference/text-to-voice/create.
- Cost: the Voice Design guide says you are charged only for the preview-text characters, once, although three samples come back. Observed: 5 calls with a 150-character text. Total spend for the whole addition was 1,455 credits (quota 125,737 -> 127,192 used); TTS header costs summed to 705, so design was about 750 for five calls, roughly 1 credit per preview character. Source: the voice-design capability page (found at elevenlabs.io/docs/eleven-creative/voices/voice-design); the api-reference overview page for it returned 403.
- Slots: `GET /v1/user/subscription` shows `voice_limit` 30, `voice_slots_used` 18 before and 23 after; 7 free; `can_extend_voice_limit` false. Nothing was deleted. The docs say saving a preview uses one slot but not how many a plan includes; the 30 comes from the subscription response.
- Licence for designed voices: not stated on any page I could read. The Terms of Use say paid users may use Output commercially and keep their rights (see above); nothing separate about designed voices, and I could not read the designed-voice or prohibited-use specifics. Unverified; operator should check.

### Descriptions (verbatim, all sent with guidance_scale 5, loudness 0.5, model `eleven_multilingual_ttv_v2`)
- C1 Classic play-by-play: "Native American English, North American broadcast. Male, 45 to 55. Studio quality. Persona: classic sports play-by-play announcer. Emotion: steady, authoritative, building excitement. A deep, steady baritone with a warm resonant timbre and measured pacing that rises quickly in pitch and intensity as the action speeds up."
- C2 High-energy excitable: "Native American English. Male, 25 to 35. Studio quality. Persona: hyper-energetic sports announcer. Emotion: excited, breathless, thrilled. A bright, fast-talking voice with a higher pitch, rapid pacing and punchy delivery, always on the edge of shouting."
- C3 Dry witty colour: "Native English with a mid-Atlantic accent. Male, 40 to 50. Studio quality. Persona: dry witty colour commentator. Emotion: relaxed, amused, wry. A smooth, slightly raspy mid-range voice with a laid-back pace, deadpan timing and a hint of a smile in every line."
- C4 1950s newsreel announcer: "Native English with a mid-Atlantic accent. Male, 40 to 50. Studio quality. Persona: 1950s newsreel and film-studio announcer. Emotion: theatrical, proud, urgent. A clipped, crisp, formal delivery with a projecting chest voice, precise consonants and dramatic pauses."
- C5 Warm sharp female play-by-play: "Native American English. Female, 30 to 40. Studio quality. Persona: confident sports play-by-play announcer. Emotion: warm, sharp, enthusiastic. A clear, quick, confident voice with a warm mid-range timbre, crisp diction and fast pacing that lifts with the action."
- Design preview text (all five, 150 characters): "Blue wins the faceoff and Orange is on the break, nobody back! He shoots... he scores! Big hit along the boards, and that's a wrap on the second take."

### Saved voices (account voice ids) and which preview was kept
| Variant | voice_id | kept preview | spares (not saved; mp3 in `audio-lab/commentators/`) |
|---|---|---|---|
| C1 | 1gm1MeFwA3QfH0NyejOt | 1 | C1_2, C1_3 |
| C2 | FGymx4HciRaNj8j7HIX1 | 1 | C2_2, C2_3 |
| C3 | V18qdcTJ8uPIeabKuiZZ | 2 | C3_1, C3_3 |
| C4 | ZL9GpDFsgoV9hmD3GFe2 | 3 | C4_1, C4_2 |
| C5 | zGmIRV8rOzprpDf4Wutc | 3 | C5_1, C5_2 |

I could not tell the previews apart by ear. The kept preview is a heuristic on duration (fast voices C2, C5: the shortest of the three; deliberate voices C1, C3, C4: the longest, i.e. slower and more pauses), so any of the three could be better. Each saved voice is named "Hokej C<n> <variant>". The ten lines were made with TTS (multilingual_v2, stability 0.35, style 0.4) from the saved voices, not from the preview audio.

### Files and measurements
50 files `C:/Users/kazda/jam/audio-lab/VO_Commentator_C<n>_<Line>.mp3` plus wavs in `wav/`; lines Welcome, Faceoff, Break, Scores, Boards, BigHit, DirectorsCall, TenSeconds, Wrap, Retake. Full per-file duration, LUFS and peak are in `audio-lab/table.md` and `measure.json`. Ranges: loudness -13.4 to -26.6 LUFS, all peaks below -0.4 dBTP. The loudest are C2_Scores (-13.5), C5_Break (-13.4) and C5 generally (-14 to -17); the quietest is C3_TenSeconds (-26.6, peak -10.7), consistent with the relaxed C3 description. C3 and C1 run -17 to -21 and C4 -15 to -18. Durations: Welcome 3.0 to 4.9 s (C2 slowest, 4.9 s: slower than a "fast" voice should be, the numbers contradict the description there), shortest lines about 1 s. Credits: header cost 10 to 26 per line, 141 per voice, 705 for all 50.

### Full commentary estimate
About 120 lines with three alternates in one voice is 480 generations. At the observed 14 credits per line (mean line 33 characters, about 0.43 credits per character) that is roughly 6,800 credits; with longer 50-character lines, about 10,300. Quota remaining 16,100 (resets 2026-11-13), so it fits with 5,800 to 9,300 to spare, but would take roughly 40 to 65 percent of what is left. Do the alternates only for the voices that survive triage.

### Failed or unavailable
Nothing failed. No slot error. The per-voice credit cost of design is derived, not measured per call (the design response carries no cost header).

### Not verified
Everything audible: whether any variant sounds like a sports commentator, whether C2 really is excitable, whether C4 reads as newsreel, whether any voice resembles a real broadcaster by accident (I named none; a designed voice can still land close to a real person, so listen for that), accent drift despite the language prefix, and consistency of one voice across lines. Licence terms for designed voices (above).
