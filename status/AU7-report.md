# Lane AU7 report: the mix back to target, slide and get-up up, `MS_bump` (device B), 2026-11-13 15:14

- Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8000 from 14:51 to 15:10, left through `ExitEditor` (pid 31408 gone, `LogExit: Exiting.`). No git write, no build, no cook, no outside service. Only files under `Game/Plugins/JamAssets/Content/Audio/` changed, plus this report.
- **Nobody has listened to any of this.** Every statement is a measurement of PIE output recordings (48 kHz stereo, ffmpeg and numpy).
- `/Game/Maps/Rink` not saved (still 2026-11-12 12:40). No dirty package at exit. `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.Cut` `hockey.regular`. The editor's background CPU throttle was off for the tests and is back on. The editor on 8002 was not touched.
- I did not record a full match after each single step (90 s each would not fit the hour). I ranked first, made all trims, then recorded both matches once. The crowd and save "ooh" changes after that were checked by trigger tests and arithmetic only.

## Verdicts

| Item | Verdict | Evidence |
|---|---|---|
| 1. Mix, plain bot match | **PASS, on the lower edge** | 90 s: **-24.0 LUFS** (was -20.8), true peak **-5.9 dBTP** (was -0.9), nothing at full scale. |
| 1. Mix, drill match | **PARTIAL** | 60 s: **-21.6 LUFS** (was -20.5), 0.4 LU over the window; true peak **-3.0 dBTP** (was -1.1), nothing at full scale. |
| 1. The tackle stays on top | **PASS** | Hit moment over the 0.9 s before it, median: drill 5.7 to 7.2 dB, plain 2.3 to 3.2 dB. The highest peak of both matches after the trim is a tackle. |
| 2. Slide and get-up +6 dB | **DONE; they are present, not prominent** | Section 4. |
| 3. Roar threshold | kept at 0.8 | No change. |
| 4. `MS_bump` | **PASS** | Built, `SC_SFX`, 8 to 9 LU under a body check at the same Strength. Silent in the game until A emits `bump`. |
| 5. AU3's 0.7071 | **`MS_win` and `MS_save` corrected; `MS_goal` left; `MS_period_end` has no synth layer** | Section 5. |
| 6. The 30 ms offset | **left** | No cause in the assets, and it did not show again in 19 joint events; section 6. |
| 7. Unreferenced waves | stay | `SFX_Bump_a` is now referenced by `MS_bump`; 10 remain unreferenced. |

No master limiter, no sound class or mix change: after the trims the peaks did not need one.

## 1. What made the level

Plain bot match before the trim (92 s, -20.8 LUFS). Each event sound was recorded alone through the binder; share = count x that sound's energy / the match's energy. Music and bed are 29 s and 9 s recordings of each alone.

| Rank | Source | Count in 92 s | One event alone (LUFS / peak dBFS) | Share of the mix energy |
|---|---|---|---|---|
| 1 | **`lift`, clean, Strength 1 (`MS_lift`)** | 48 | **-17.3 / -6.5** | **53.9 %** |
| 2 | Music (`MS_music_match`) | - | -27.5 LUFS on its own | 19.8 % |
| 3 | `poke`, clean, Strength 1 | 42 | -26.0 / -10.6 | 6.1 % |
| 4 | Skater board boom (Strength mean 0.66) | 30 | -29.5 / -12.1 at 0.66 | 5.8 % |
| 5 | Crowd bed | - | -31.6 LUFS on its own | 5.4 % |
| 6 | Check and knockdown | 5 | -26.8 / -8.4 at 0.7 | 3.5 % |
| 7 | Puck board (Strength mean 0.33) | 32 | -31.7 / -17.0 at 0.33 | 1.7 % |
| 8 | `save` | 2 | -26.9 / -6.2 | 1.6 % |
| 9 | `shot` (Strength mean 0.67) | 27 | -29.5 / -12.5 | 1.3 % |
| 10 | Bark blip, `pass`, `faceoff` drop | 10, 4, 1 | | 0.8 % together |
| - | Commentary (16 lines, 23 s), skate loop | | | not resolved: the rows above already sum to 100 % |

- **One sound was more than half the mix: the stick lift.** At Strength 1 it was 5 LU louder than the big body check (-22.3) and 10 LU louder than a slap shot, 48 times in 92 s. It was also in the top peaks of the drill match (AU6 found a `lift` on the frame of the hits).
- The foul whistle did not occur in the plain match. Alone it is -17.3 LUFS / -10.2 dBFS; left as it is.
- **The music is 4.5 dB quieter than last night.** Alone it now measures -27.5 LUFS; AUD measured -23.0 at `Level` 1. So last night's -22.5 was mostly music, and today's music plus bed is about -26. That is why the plain match lands at the bottom of the window once the lift is tamed.

## 2. Every gain changed

| Asset | Node, pin | From | To | Why |
|---|---|---|---|---|
| `MS_lift` | `AudioMixerMono3_d8cf` Gain 0 / 1 / 2 | 1.4 / 1.4 / 0.9 | 0.4 / 0.4 / 0.25 | -11 dB. Clean lift now -28.1 LUFS / -17.2 dBFS, just under a body check at 0.6 (-27.7). A missed lift is -37.3. |
| `MS_board` | `AudioMixerMono2_117d` Gain 0 (boom) | 1.4 | 1.0 | lower |
| `MS_board` | `AudioMixerMono2_117d` Gain 1 (rattle) | 0.7 | 0.5 | stays 6 dB under the boom |
| `MS_board` | `Clamp_94b4` Min | 0.5 | 0 | no floor |
| `MS_board` | new `Multiply_1c63` (Strength x Strength) between `Clamp_94b4` and `Multiply_45e7` | level x max(0.5, S) | level x S squared | steeper: only hard crashes are loud |
| `MS_knockdown` | `AudioMixerMono3_f522` Gain 1 (slide) | 0.70 | 1.4 | +6 dB |
| `MS_knockdown` | `AudioMixerMono3_f522` Gain 2 (get-up) | 0.98 | 1.96 | +6 dB, still fixed |
| `MS_knockdown` | `AudioMixerMono3_b77a` and `_b525` Gain 1 and 2 (crowd) | 0.8 | 0.6 | as decided |
| `MS_win` | `AudioMixerMono2_9c3b` and `_c639` Gain 0 (synth) | 0.7071 | 0.5 | AU3's 3 dB |
| `MS_save` | `AudioMixerMono2_d2a8` and `_8c87` Gain 0 (synth) | 0.7071 | 0.5 | AU3's 3 dB |
| `MS_save` | the same two mixers, Gain 1 ("ooh") | 0.68 | 0.48 | keeps AU3's "ooh 6 dB under the thump" |
| `MS_bump` (new) | `AudioMixerMono2_35e1` Gain 0 | - | 0.7 | |

Not changed: `MS_check`, `MS_poke`, `MS_shot`, `MS_foul`, `MS_goal`, the music, the bed, every sound class and mix.

The skater boom after the change, puck case untouched:

| Strength | Before (LUFS / peak dBFS) | After | Body check at the same Strength |
|---|---|---|---|
| 0.45 | about -32 / -15 | -42.9 / -25.9 | -29.1 |
| 0.66 | -29.5 / -12.1 | -36.3 / -19.3 | -27.7 at 0.6 |
| 1.0 | -26.1 / -9.1 | -29.0 / -12.1 | -22.3 |

## 3. Both matches, before and after

Default cut, `jam.ScriptHuman 1`; the drill match adds `jam.ctrl.Drill 6`. The sim is deterministic, so the same checks come in the same order before and after.

| | Plain, before | Plain, after | Drill, before | Drill, after |
|---|---|---|---|---|
| Length measured | 90 s | 90 s | 60 s | 60 s |
| Integrated | -20.8 LUFS | **-24.0 LUFS** | -20.5 LUFS | **-21.6 LUFS** |
| True peak | -0.9 dBTP | **-5.9 dBTP** | -1.1 dBTP | **-3.0 dBTP** |
| Samples at full scale | 0 | 0 | 0 | 0 |
| Checks and knockdowns | 5 | 4 | 13 | 12 |
| Hit moment over the 0.9 s before: min / median / max | -3.4 / 2.3 / 3.8 dB | 1.2 / 3.2 / 7.7 dB | 2.6 / 5.7 / 16.1 dB | 3.7 / 7.2 / 14.8 dB |
| Highest peak sits on | shots and puck boards | a tackle (0.73) | a tackle with a foul | a tackle with a foul |

- "Hit moment" is the RMS of the 0.3 s from the hit against the 0.9 s before it. Every hit with room around it in the recording is counted (12 of 13, 12 of 12, 5 of 5, 3 of 4), second hits of a pair included; AU6's "5.4 to 16.1" left two of those out.
- **Why the drill is still 0.4 LU over:** it has 12 knockdowns in 60 s, 10 of them at full Strength. By arithmetic the big hit is 19 % of its energy, the roar 10 %, landing, slide and get-up 8 % together. Crowd 0.6 to 0.5 would give 0.14 dB. Reaching -22.0 needs about 2 dB off the big hit as well, which is the tackle itself. I left that to you.
- **Loudest events after the trim, by energy of one event:** tackle at Strength 1 (665) > foul whistle (361) > `save` (about 300, measured before its "ooh" came down) > tackle at 0.6 (160) > puck board at 1 and skater boom at 1 (78, 77) > `poke` clean (57) > lift clean (36) > shot at 1 (34). So a full tackle is the loudest gameplay event; a tackle at 0.6 sits under the whistle and the `save` (a thump plus a 2 s "ooh").

## 4. Slide and get-up after +6 dB

- **By arithmetic:** the get-up is now about -25 LUFS at any Strength; the slide -25.4 at Strength 1 and -31.4 at 0.5. Music plus bed is about -26. So the get-up clears them by about 1 LU, and the slide only on a full-Strength hit.
- **By the recording:** the matched filter finds both far better than before. Get-up: 0.49 to 0.73 (was 0.13 to 0.49). Slide: 0.22 to 0.42 (was 0.08 to 0.19). Both sit at 1.050 s and 0.200 s after the hit in all 15 hits measured.
- But the 0.4 s in which the get-up plays is not louder than the 0.3 s before it: in the drill -25.5 to -28.4 dB against -24.2 to -28.0. So it is in the mix, level with what surrounds it, not standing out.
- Side effect: the get-up scrape (fixed level) now peaks at about -14 dBFS, and a hit at Strength 0.3 peaks at -9 to -13.5 depending on the take. After a weak hit the scrape is nearly as strong as the hit.

## 5. AU3's 0.7071

I isolated each synth layer by playing the MetaSound with its crowd wave swapped for a 0.08 s click, then compared with the level measured before it became stereo.

| Asset | Synth layer before the fix | The old mono figure | After the fix | Done |
|---|---|---|---|---|
| `MS_win` | -12.3 LUFS | -15.3 LUFS (AUD) | **-15.3 LUFS** | 0.7071 to 0.5 |
| `MS_save` | thump -22.8 LUFS (AU3's own figure) | -24.9 LUFS, -8.8 dBFS (B18) | thump **-25.9 LUFS**, peak -7.5 dBFS; "ooh" -32.0, 6.1 dB under | 0.7071 to 0.5, "ooh" 0.68 to 0.48 |
| `MS_goal` | burst -21.1 LUFS | -20.9 LUFS (B16, burst only) | not changed | see below |
| `MS_period_end` | no synth layer (the horn was removed) | | | nothing to correct |

- `MS_win` was exactly 3.0 LU hot and is back on AUD's number. Whole sound now -17.9 LUFS / -4.0 dBFS (AU3: -16.2 / -2.6).
- `MS_save` whole sound now -28.3 LUFS / -7.5 dBFS at Strength 1 (AU3: -25.2 / -4.5).
- **`MS_goal` left alone.** AU3 set the burst to 0.35 by measuring the finished goal against the old one, not by the 0.7071 rule, so its level was chosen by result. My isolation reads the burst 0.2 LU under the old burst-only goal, where AU3 meant 6 dB under; the click I swapped in may carry part of that. Lowering it to 0.25 is one number (`AudioMixerMono3_d102` and `_ff38` Gain 0) if you want AU3's "-6 dB" literally.

## 6. The 30 ms offset

- Left as it is. `MS_check` and `MS_knockdown` are two sources started on the same frame; nothing in either graph delays its start, and the knockdown's own timeline is exact. The only asset-level cure is one MetaSound for both events, which the binder's one-asset-per-event rule does not allow.
- It did not show again: in this lane's 4 joint triggers the landing is 0.170 s after the hit, and in all 15 match hits the get-up is 1.050 s after it (within 10 ms). With AU6 that is once in 37.

## 7. `MS_bump`

- Mono, `SC_SFX`, no attenuation, ends at 0.5 s. Inputs: `Strength` (0.5), `Variant` (0, not connected), `Bump` (`SFX_Bump_a`; `_b` and `_c` are imported, same length or shorter).
- Level: wave x max(0.5, Strength) x 0.7 (`AudioMixerMono2_35e1` Gain 0).
- Triggered directly (the binder has no such event):

| Strength | Peak dBFS | LUFS | Length s | Body check at the same Strength |
|---|---|---|---|---|
| 0.3 | -20.7 | -37.3 | 0.39 | -29.1 |
| 0.6 | -19.1 | -35.7 | 0.39 | -27.6 |
| 1.0 | -14.7 | -31.2 | 0.39 | -22.3 |

- 8 to 9 LU under the check. At -36 LUFS it is under music and bed (-26); it is built to be felt as a thump, and one number raises it.

## 8. Files (bytes, `Game/Plugins/JamAssets/Content/Audio/`)

| File | Before | After |
|---|---:|---:|
| `MS_bump.uasset` | new | 108,876 |
| `MS_lift.uasset` | 188,906 | 188,954 |
| `MS_board.uasset` | 245,386 | 247,377 |
| `MS_knockdown.uasset` | 256,988 | 256,988 (gains only) |
| `MS_win.uasset` | 295,744 | 295,744 (gains only) |
| `MS_save.uasset` | 222,481 | 222,481 (gains only) |

Ledger rows: `MS_bump` is new; the other six **replace** existing rows.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_bump.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, mono, SC_SFX; standing bump: SFX_Bump_a (input Bump) x max(0.5, Strength) x 0.7, input Variant unused; -35.7 LUFS at Strength 0.6; no bump event yet, so nothing plays it (lane AU7) (108876 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lift.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source synthesised in-engine (no waves); lowered 11 dB (it was 54 % of the match mix): clean lift -28.1 LUFS (lane AU7) (188954 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_board.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, mono; Variant 0 (puck): SFX_PuckBoards_01 over the synth, unchanged; Variant 1 (skater into the boards): boom (BoardsBody) at 1.0 with the glass rattle (GlassRattle) 0.03 s later and 6 dB under, level x Strength squared; -29.0 LUFS at Strength 1 (lanes AU6, AU7) (247377 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, stereo; the fall after a check: landing (Lands, random) at 0.17 s, slide (Slide) from 0.20 s, get-up scrape (GetUp) at 1.05 s, crowd from 0.05 s at 0.6: ooh (CrowdOohs, random) at Strength 0.5 to under 0.8, roar (CrowdRoar) at 0.8 and over; slide and get-up raised 6 dB (lanes AU6, AU7) (256988 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_win.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, stereo; arpeggio and thump (back at their mono level: 0.5 per channel), crowd celebration (WinCheer) over it, 6.1 s, -17.9 LUFS (lanes AU3, AU7) (295744 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_save.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, stereo; padded thump + clack + 90 Hz thump (back at their mono level: 0.5 per channel), crowd ooh (SaveOoh) 6 dB under it at any Strength; -28.3 LUFS at 1.0 (lanes AU3, AU7) (222481 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | standing bump, thud plus skate scrape, layered from two generations; mono 48 kHz 0.45 s, -19.4 LUFS, -3 dBTP, Bink q40, SC_SFX; MS_bump Bump (no bump event yet) (34275 bytes) |

## 9. Recordings

Final set in `C:/Users/kazda/jam/audio-lab/tackle/engine/` and in the scratch folder `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au7/`:

- `match_plain_90s.wav`, `match_drill_60s.wav` (after the trim)
- `chain_s06.wav`, `chain_s10.wav`, `board_skater_s10.wav`, `bump_s06.wav` (alone, no music or bed, 0.2 s of lead)
- `before_after.wav`: 10 s of the drill before (five hits), 1 s of silence, the same five hits after, level as recorded
- I did not delete AU6's files from the `engine` folder: they are now in `C:/Users/kazda/jam/audio-lab/tackle/engine/au6/`.
- Scratch only: `plain_before.wav`, `drill_before.wav`, `tpl.wav` (every event alone, before), `trig2.wav` and `fin.wav` (after), `stems.wav` (music and bed alone), `bump_s03.wav`, `bump_s10.wav`, `backup/` (the MetaSounds as they were at 14:53).

## 10. What I could not verify

- **Nobody has heard this.** In particular whether a lift 11 dB down still reads as a lift, and whether the boom at Strength under 0.6 (-36 LUFS and lower) is still a thump or simply gone.
- **The final crowd and save "ooh" levels in a match.** Both matches were recorded with the crowd already at 0.6; the save "ooh" change came after them and was checked by trigger only.
- **The ranking's precision.** Energy arithmetic from single renders; it sums to 100 % before commentary is counted, so it overshoots by a few percent somewhere.
- **`MS_goal`'s burst** (section 5).
- **`MS_bump` through the binder**: no event exists.
- Cooked build, packaged behaviour. No cook was run.
- One odd line in the log at 14:53: `AudioMixerPlatformInterface Timeout [5 Seconds] waiting for h/w`, twice, during my first template recording (it lost 7 s; I re-recorded). No MetaSound error. The only `JAMFX missing` lines are `NS_deke` / `MS_deke`.

## 11. Decisions for the lead

1. **The drill match at -21.6 LUFS.** Accept it as a stress case (12 knockdowns a minute), or take about 2 dB off the big hit (`MS_check` `AudioMixerMono3_9829` Gain 2, 1.25 to 1.0) and crowd to 0.5. I would accept it: plain play is already at -24.0.
2. **The plain match is at the bottom of the window because A now plays the music 4.5 dB lower than last night.** If the target is -23 in plain play, the music `Level` is the lever (A's), not the effects.
3. **`poke` clean** (42 in 92 s, -26.0 LUFS, peak -10.6 dBFS) is now as loud as a body check at 0.6 and louder than a lift. Untouched; 4 dB off would put it under the tackle and move the plain mix about 0.3 dB further down.
4. **Lift at -11 dB**: my number. It is now just under a 0.6 body check.
5. **`MS_goal` burst**: leave, or 0.35 to 0.25 for AU3's literal -6 dB.
6. **`MS_bump` at 0.7** is under music and bed. If a bump should be heard and not only fill in, raise `AudioMixerMono2_35e1` Gain 0 to about 1.4 (it would then be 2 to 3 LU under the check, less than the 6 you asked for).
7. **The get-up** is now nearly as strong as a weak hit (section 4). If that reads wrong, scale it with Strength after all.
