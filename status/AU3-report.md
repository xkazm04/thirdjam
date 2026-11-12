# Lane AU3 report: crowd instead of horns (device B), 2026-11-13 about 00:50

- Tree `C:/Users/kazda/jam/wt/b-vfx`, no git command, no build. Editor on port 8002: started 00:23:37, left through `ExitEditor` at 00:47:45 (`LogExit: Exiting.`, no fatal line, no dirty package at exit).
- `/Game/Maps/Rink` was never saved (mtime still 2026-11-12 12:40) or reloaded. Nothing under `_Scratch/` (the folder does not exist). Every save was made with PIE stopped, and the file times were checked after it (00:42:06-07).
- **Nobody has listened to any of this.** Every audio statement comes from recordings of the PIE output (AudioMixer output recording, 48 kHz stereo). They were measured with ffmpeg (peak, true peak, EBU R128 integrated loudness on a silence-padded clip) and numpy (envelopes, spectra, the crowd lane's tonality test ported line for line from `crowd/proc.mjs`).
- The recordings are in the scratch folder `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au3/`: `events.wav` (the ten triggers, bed), `trim.wav` (final levels), `v1.wav` (Variant 1 check), `match.wav` (bot match). They were removed from `Game/Saved`.

## Verdicts

| Item | Verdict | Short evidence |
|---|---|---|
| 1. Import | **PASS** | 16 `CRD_*` stereo waves (all takes except `GoalFor_b`) in `Audio/Crowd/`, q40, `SC_Crowd`, no attenuation; `Bed_a`/`Bed_b` looping. `SC_Crowd` made and added to `SCM_Volume` (volume 1.0). It has no parent: no class in the project has one, and there is no master class (as AUD found). |
| 2. `MS_goal` | **PASS** | Horn removed (2 saws, mixer, low-pass, envelope, multiply; mixer input 5 gain 0). Variant 0 picks one of three cheers at random, with no immediate repeat. Variant 1 plays the groan. The burst stays under the crowd. Final level at Strength 1: -16.2 to -16.5 LUFS (old -17.2: within 1.0 dB). |
| 3. `MS_period_end` / `MS_win` / `MS_lose` / `MS_save` | **PASS** | Period end: horn gone, applause in, -20.2 LUFS (old horn -20.2). Win: `Win_b` layered over the arpeggio. Lose: unchanged, because no goal-against take reads as a murmur by its numbers. Save: "ooh" exactly 6.0 dB under the thump at Strength 1 and 0.3. |
| 4. `MS_crowd_bed` | **PASS** | New looping stereo source in `SC_Crowd`: `Level` (default 0.5, clamped, 0.5 s smoothing) and `Excite` (default 0: up to +6 dB gain and a low-pass opening from 3 to 9 kHz). Seam measured clean. |
| 5. Swap-ready | **PASS** | Every take is a named graph input (section 4). |
| 6. Proof in the real game | **PASS, with one gap** | Ten binder triggers gave all three cheers (c 4, a 3, d 3) with no immediate repeat. Two Variant 1 triggers gave the groan. No horn: the horn band at 233-237 Hz has no peak. 90 s bot match mix: -22.5 LUFS (last night -23.0). The goal in play cheered (`GoalFor_c`). **Gap:** this tree's binder does not send Variant 1 yet, so a conceded goal cheers too (section 6). |

## 1. What was built

All four event MetaSounds were **mono** and had to become **stereo** to carry the stereo takes. Trap for later lanes:
- `MetaSoundSourceBuilder.set_format(STEREO)` from Python (on `MetaSoundEditorSubsystem.find_or_begin_building`) converts the graph's interface. It does **not** change the asset's `output_format` property, which stays MONO.
- In PIE the result is silence and the log shows `Metasound generator expected 1 number of channels, found 0`.
- Setting `output_format` = STEREO afterwards re-applies the interface and **drops the output edges** (`Removed existing targeted output ... Desired edges may have been removed`). They then have to be reconnected.
- That was done for all four, and all five MetaSounds report STEREO with both outputs connected.
- The old mono path now feeds both channels at 0.7071 (equal power), so the synth parts keep their old level.

| Asset | Graph now | Gains (mixer inputs) |
|---|---|---|
| `MS_goal` | Burst (5 saw voices, unchanged, x (a + b x Strength) via `Multiply_2440`). `Random Get (WaveAsset:Array)` on input `GoalForCheers` (`No Repeats` 1, `Enable Shared State` true), triggered by OnPlay; its `On Next` plays a stereo Wave Player. A second stereo Wave Player on input `GoalAgainstGroan`, on OnPlay. Weights: groan = clamp(`Variant`, 0, 1), cheer = 1 - that. New float input `Variant` (default 0). OnFinished at 4.1 s. | L `AudioMixerMono3_d102`, R `AudioMixerMono3_ff38`: Gain 0 burst **0.35** (was 0.7071: **burst -6 dB**), Gain 1 cheer **1.2**, Gain 2 groan **1.2**. The crowd is fixed; `Strength` scales only the burst. |
| `MS_period_end` | All horn nodes removed (3 saws, mixer, low-pass, envelope, multiplies, Strength gain). A stereo Wave Player on input `PeriodEndApplause`. OnFinished at 3.1 s. `Strength` input kept, unused (it was already fixed gain) | `AudioMixerMono2_0d0a` / `_df0d` Gain 1 **1.4** |
| `MS_win` | Arpeggio and thump unchanged (clapper still muted). A stereo Wave Player on input `WinCheer` added over them. OnFinished 0.955 -> 6.1 s | `AudioMixerMono2_9c3b` / `_c639`: synth 0.7071, wave **1.0** (fixed) |
| `MS_save` | Thump, clack and 90 Hz sine unchanged. A stereo Wave Player on input `SaveOoh`, scaled by the same Strength gain (`Add_a4b3`), so it stays 6 dB under at every Strength. OnFinished 0.25 -> 2.05 s | `AudioMixerMono2_d2a8` / `_8c87`: synth 0.7071, ooh **0.68** |
| `MS_crowd_bed` (new) | Stereo, looping. Input `BedLoop` (WaveAsset), a looping Wave Player. `Level` -> Clamp 0..1 -> InterpTo 0.5 s; `Excite` -> Clamp -> InterpTo 0.5 s. Gain = 2 x Level x (1 + Excite), so Level 0.5 is the file level and Excite 1 is +6 dB. Two low-pass biquads, cutoff 3000 + 6000 x Excite Hz. Class `SC_Crowd` | - |

**How the random pick is built:** UE's `Array / Random Get` node on a `WaveAsset:Array` graph input, with `No Repeats` 1 and `Enable Shared State` on.
- Shared state makes every `MS_goal` instance draw from one shared stream, so the next goal never repeats the last cheer.
- Epic's note: shared state applies in PIE and in the game, not in the MetaSound editor preview.
- `No Repeats` is clamped to half the array size, so it is 1 with three takes and 0 with one or two. With two takes, immediate repeats come back.
- No log line names the pick, so the cheer was identified from the recording. The fingerprint is the 10 ms loudness envelope over 1.5-3.9 s, correlated with each take's envelope (search +-100 ms). The winner was 0.985-0.998 against 0.91-0.97 for the others.

## 2. Levels (final gains, music and skate loop muted during the trigger test, binder path)

| Sound | Strength / Variant | Peak dBFS | Loudness LUFS | Note |
|---|---|---|---|---|
| `MS_goal` | 1.0 / 0 (binder), three plays | -5.5, -2.9, -4.7 | -16.4, -16.5, -16.2 | Old goal -4.2 / -17.2: **within 0.7-1.0 dB**. 0-1.5 s: -14.2 to -14.9; 1.5-4.5 s: -20.6 to -25.0 |
| `MS_goal` | 0.2 / 0 | -6.4 | -16.9 | Strength scales only the burst (crowd fixed) |
| `MS_goal` | 1.0 / 1 | -6.5 | -19.0 | The groan identified as `GoalAgainst_a` (envelope corr 0.99 vs 0.78-0.84) |
| `MS_goal` cheer alone | - | - | about -17.5 (derived, not isolated) | File -16 + 1.6 dB gain - 3.1 dB. The 3.1 dB is the offset between file and game measured on `MS_period_end` (file -20, gain +2.9 dB, measured -20.2) |
| `MS_period_end` | any | -10.3 | **-20.2** | Old horn -13.1 / -20.2 |
| `MS_win` | 1.0 | -2.6 | -16.2 over 6.5 s | Old -7.6 / -15.3 over 0.955 s. Envelope: -12.5 dB RMS at 0-0.5 s, then decays to -37 by 5 s (Win_b tail) |
| `MS_lose` | 1.0 | -8.3 | -17.6 | Unchanged |
| `MS_save` | 1.0 | -4.5 | -25.2 over 2.4 s | thump 0-0.3 s **-22.8**, ooh 0.35-2.0 s **-28.8**: **6.0 dB under** |
| `MS_save` | 0.3 | -8.8 | -29.4 | thump -26.9, ooh -32.9: 6.0 dB under |
| `MS_crowd_bed` | Level 0.5, Excite 0 | -12.3 | -31.7 (19 s) | -1.6 dB vs the file at the same loop positions (3 kHz low-pass) |
| `MS_crowd_bed` | Level 0.5, Excite 1 | -9.9 | -28.1 | Measured 0.8-5 s after the step. +4.9 dB vs the file at the same positions, i.e. most of the +6 dB, while InterpTo settles. The 4-9 kHz vs 0.1-2 kHz balance goes from -22.9 to -12.3 dB: the filter opens |
| `MS_crowd_bed` | Level 1, Excite 0 | -9.0 | -25.6 | +6.1 dB over Level 0.5, as designed |

Trim history:
- **First pass:** crowd 1.0 and burst 0.7071 gave -14.9 to -15.7 LUFS with peaks to -0.4 dBFS.
- **Crowd 0.8:** barely moved it, because the integrated loudness is set by the 0-1.5 s window, where burst and cheer overlap.
- **Final:** lowering the burst 6 dB (the brief says the burst stays *under* the crowd) and setting the crowd to 1.2 gave -16.2 to -16.5 with peaks of -2.9 to -5.5.
- **The -16 LUFS "cheer on its own" and a sum within 2 dB of -17.2 cannot both hold:** a cheer at -16 alone already puts the sum at -16 or more. I kept the sum in range (cheer about -17.5) instead. Crowd Gain 1 at 1.4 would put the cheer at about -16 and the sum at about -15.3 (edge of the 2 dB window). Lead decision in section 8.

**Bed loop seam** (9.0 s file):
- Seam at 9.0 and 18.0 s after the onset: the lowest 10 ms RMS within +-100 ms is -41.4 dB, against 40 random positions in the bed (median -40.2, 10th percentile -42.1). No gap.
- The largest sample step within +-10 ms of the seam is 2.09 x the bed's 99th-percentile step. For the random positions: median 0.85, 90th percentile 2.09, maximum 2.72. So the seam step is inside the bed's own range, but at its high end.

## 3. Proof in the real game

**Ten `goal` triggers through the binder.** `UJamEffectsBinder.handle_event('goal', payload)`, Strength 1.0, 5 s apart, at time dilation 0.01. The binder spawned `MS_goal` at the location and set `Variant` 0 from `VariantFor`. Picks in order:

**c, a, d, a, c, a, c, d, c, d**. All three appear (c 4, a 3, d 3) with no immediate repeat.
- These ten were recorded with the crowd at 0.8 and the burst at 0.7071, before the final trim. The pick logic was not touched by the trim.
- Three more binder triggers after the trim also cheered.

**Two at Variant 1** (spawned at location exactly as the binder does, then `SetFloatParameter("Variant", 1)`): both are the groan. A third test with Strength 0 was the groan too.

**No horn:**
- The crowd lane's tonality test on the first second (`proc.mjs` rule) of the final goals: **tone-ish, not tonal**. Prominence 20.6-26.9 dB (the tonal threshold is 30), persistence 0.15-0.65, ratio 0.08-0.26. Strongest components: 275 Hz (the burst's saws) or about 1.1 kHz (the `GoalFor_c` whoop flagged by the crowd lane).
- For comparison, the old goal with the horn (B16 `after2.wav`) is **tonal**: prominence 48.5 dB at 229 Hz.
- A horn-specific check: band energy at 231-239 Hz (the horn's 233.1 / 236.6 Hz) against its +-20-60 Hz neighbourhood, over 0.05-0.95 s. Results: **-19.6 to -25.8 dB** in the triggers and -15.1 dB in the match goal, i.e. no peak. The old goal with the horn measures **+6.1 dB**.
- Before the final trim, with the burst at full level, the same test read "tonal" (prominence 32-38 dB at 275 Hz). The burst-only goal from before the horn (B16 `before.wav`) reads tonal too (53.6 dB at 275 Hz). That is the burst's saws, not a horn.

**Bot match** (fresh PIE on `Rink`, the scripted human, the live bot match; `MS_music_match` and the commentary were already playing from device A's code):
- The bed was started from Python at Level 0.5, because A does not start it yet.
- **Mix over the first 90 s: -22.5 LUFS, peak -2.1 dBFS.** Last night: -23.0 (the music alone), so +0.5 LU. The bed at -31.7 adds about that.
- **Goal in play** at 106.1 s (frame 4673, `who=3`, `kind none`): the cheer `GoalFor_c` played (envelope corr 0.957 vs 0.85-0.89 for the others). The goal window is -16.4 LUFS / -2.0 dBFS over the music. RMS goes from -25.8 dB before to -15.8 dB at the goal.
- The binder in this tree has **no `goal` case in `VariantFor`**, so it sends 0 and the default path cheered, as required.
- No `JAMEVENT period_end` fired in the 125 s (the match emitted `period take_2` after the goal). `MS_period_end` was proven by binder trigger only.

## 4. Swap instructions (one minute per situation)

Every take is a graph input on the MetaSound. Change the input's default value in the MetaSound editor: open the asset, select the input in the Members panel, edit "Default Value". Through MCP: `AttachBuilderToAsset`, then `SetDefaults` on the graph input, then `CommitBuilderToAsset`. Then save with PIE stopped.
- Wave paths have the form `/JamAssets/Audio/Crowd/CRD_<Situation>_<letter>.CRD_<Situation>_<letter>`.
- The alternates are all imported, normalised by lane AU2 to the same loudness per situation, so a swap keeps the level.

| Situation | Asset | Input to change | Now | Alternates imported |
|---|---|---|---|---|
| Goal for (random) | `MS_goal` | `GoalForCheers` (WaveAsset **array**: 1 to N entries) | `[CRD_GoalFor_d, CRD_GoalFor_c, CRD_GoalFor_a]` | `CRD_GoalFor_e` (`_b` not imported: flagged tonal) |
| Goal against | `MS_goal` | `GoalAgainstGroan` | `CRD_GoalAgainst_a` | `_b`, `_c` |
| Save "ooh" | `MS_save` | `SaveOoh` | `CRD_Save_b` | `_a`, `_c` |
| Period end | `MS_period_end` | `PeriodEndApplause` | `CRD_PeriodEnd_a` | `_b` |
| Win | `MS_win` | `WinCheer` | `CRD_Win_b` | `_a` |
| Bed | `MS_crowd_bed` | `BedLoop` | `CRD_Bed_b` | `_a` |

- All alternates have the same length as the default, so no end time needs changing.
- If a longer take is ever used, raise the `Delay Time` of the OnFinished delay: `MS_goal` `TriggerDelay_ca3c` (4.1), `MS_period_end` `TriggerDelay_0bbc` (3.1), `MS_win` `TriggerDelay_a166` (6.1), `MS_save` `TriggerDelay_6139` (2.05).
- Fewer than three goal cheers turns off no-repeat (see section 1).

## 5. Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/Audio/`)

| File | Before | After |
|---|---:|---:|
| `Crowd/CRD_*.uasset` (16 new) | - | 7,881,346 total (rows below) |
| `MS_goal.uasset` | 218,585 | 301,343 |
| `MS_period_end.uasset` | 133,553 | 88,257 |
| `MS_win.uasset` | 248,509 | 295,744 |
| `MS_save.uasset` | 171,738 | 222,481 |
| `MS_crowd_bed.uasset` | new | 137,202 |
| `SC_Crowd.uasset` | new | 1,049 |
| `SCM_Volume.uasset` | 2,723 | 3,032 |

- The repository grows by about **8.2 MB**.
- Cooked estimate at Bink q40 (AUD's rule of thumb, 60 s stereo = 0.80 MB): 67 s of stereo in total, so about 0.9 MB if all 16 are cooked, or about 0.47 MB for the 8 referenced takes (35 s).
- Not measured, no cook was run.

Ledger rows. **Replace** the existing rows for `MS_goal`, `MS_period_end`, `MS_win`, `MS_save` and `SCM_Volume`; the rest are new:

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalFor_a.uasset | audio | 0.5 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd goal cheer, stereo 48 kHz 4.0 s, -16 LUFS, Bink q40, SC_Crowd; in MS_goal GoalForCheers (486161 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalFor_c.uasset | audio | 0.5 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd goal cheer with clapping tail, stereo 4.0 s, -16 LUFS, Bink q40, SC_Crowd; in MS_goal GoalForCheers (492230 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalFor_d.uasset | audio | 0.5 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd goal cheer with sticks on boards, stereo 4.0 s, -16 LUFS, Bink q40, SC_Crowd; in MS_goal GoalForCheers (493012 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalFor_e.uasset | audio | 0.5 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate goal cheer (small warm crowd), stereo 4.0 s, -16 LUFS, Bink q40, SC_Crowd; not referenced (474683 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalAgainst_a.uasset | audio | 0.4 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd groan then polite applause, stereo 3.0 s, -20 LUFS, Bink q40, SC_Crowd; MS_goal GoalAgainstGroan (Variant 1) (364457 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalAgainst_b.uasset | audio | 0.3 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate goal-against (visiting fans over a murmur), stereo 3.0 s, -20 LUFS, Bink q40, SC_Crowd; not referenced (324698 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_GoalAgainst_c.uasset | audio | 0.3 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate goal-against (long groan, murmur), stereo 3.0 s, -20 LUFS, Bink q40, SC_Crowd; not referenced (280384 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Save_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate save ooh, stereo 2.0 s, -20 LUFS, Bink q40, SC_Crowd; not referenced (203916 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Save_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd gasp and applause, stereo 2.0 s, -20 LUFS, Bink q40, SC_Crowd; MS_save SaveOoh, 6 dB under the thump (241539 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Save_c.uasset | audio | 0.3 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate save ooh, stereo 2.0 s, -20 LUFS, Bink q40, SC_Crowd; not referenced (251018 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_PeriodEnd_a.uasset | audio | 0.4 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | warm applause settling to a murmur, stereo 3.0 s, -20 LUFS, Bink q40, SC_Crowd; MS_period_end PeriodEndApplause (398030 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_PeriodEnd_b.uasset | audio | 0.4 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate period-end applause, stereo 3.0 s, -20 LUFS, Bink q40, SC_Crowd; not referenced (378081 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Win_a.uasset | audio | 0.7 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate win celebration, stereo 6.0 s, -17 LUFS, Bink q40, SC_Crowd; not referenced (679091 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Win_b.uasset | audio | 0.8 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | sustained cheering and applause, stereo 6.0 s, -17 LUFS, Bink q40, SC_Crowd; MS_win WinCheer (774127 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Bed_a.uasset | audio | 1.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg, crossfaded loop) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | n | alternate crowd bed loop, stereo 9.0 s, -30 LUFS, looping, Bink q40, SC_Crowd; not referenced (978741 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Bed_b.uasset | audio | 1.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU2 proc.mjs (ffmpeg, crossfaded loop) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/crowd/ledger.jsonl (prompts; status/B-audio-crowd.md) | 2026-11-13 | B | y | crowd bed loop, stereo 9.0 s, -30 LUFS, looping, Bink q40, SC_Crowd; MS_crowd_bed BedLoop (1061178 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_goal.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, now stereo; horn removed; burst (Strength-scaled, -6 dB) under the crowd: Variant 0 one of three cheers at random (GoalForCheers, Random Get, no repeat, shared state), Variant 1 the groan (GoalAgainstGroan); -16.2 to -16.5 LUFS at Strength 1 (lane AU3) (301343 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_period_end.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, now stereo; arena horn removed, crowd applause (PeriodEndApplause) at fixed gain, -20.2 LUFS, 3.1 s (lane AU3) (88257 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_win.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, now stereo; arpeggio and thump kept, crowd celebration (WinCheer) over it, 6.1 s, -16.2 LUFS (lane AU3) (295744 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_save.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, now stereo; padded thump + clack + 90 Hz thump, crowd ooh (SaveOoh) 6 dB under it at any Strength; -25.2 LUFS at 1.0 (lane AU3) (222481 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_crowd_bed.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-13 | B | y | MetaSound source, stereo looping crowd bed (BedLoop); inputs Level 0..1 (default 0.5, 0.5 s smoothing) and Excite 0..1 (up to +6 dB, low-pass 3 to 9 kHz); SC_Crowd; -31.7 LUFS at the defaults (lane AU3) (137202 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_Crowd.uasset | audio | 0.0 | original | UE 5.9 editor (Python) |  |  | 2026-11-13 | B | y | sound class for the crowd waves and MS_crowd_bed; no parent (no class in the project has one); adjuster in SCM_Volume (lane AU3) (1049 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SCM_Volume.uasset | audio | 0.0 | original | UE 5.9 editor (Python) |  |  | 2026-11-13 | B | y | sound mix with volume adjusters for SC_SFX, SC_Ambience, SC_Music, SC_VO, SC_Crowd (lane AU3 added SC_Crowd) (3032 bytes) |

## 6. What device A must do

1. **Send the goal variant.** `VariantFor` has no `goal` case, so `MS_goal` always gets `Variant` 0, and **a conceded goal cheers today**. Add a `goal` case to `VariantFor` (or pass the variant from `HandleEvent`, which has the payload): 1 when the scoring team, read from the net side as in `ColourFor`, is not the human's team; else 0.
2. **Start the bed.** At match start, together with the music: `SpawnSound2D(MS_crowd_bed)` (no auto-destroy) and keep the handle. Set `Level` with `SetFloatParameter` (0..1; 0.5 is the file level, about -31.7 LUFS; 1.0 is +6 dB). Stop it at match end.
   - It loops by itself and is 2D. Its class is `SC_Crowd`.
   - A pause or menu can lower `Level` (it smooths over 0.5 s).
3. **Drive `Excite`** (0..1, smoothed 0.5 s inside): for example 0.5 with the puck in the attacking third, 1.0 for about a second on a shot or a breakaway, then back to 0. At 1 the bed is up to +6 dB and brighter (low-pass 9 kHz instead of 3 kHz).
4. Nothing else changes for the binder: same asset names, same `Strength` / `Variant` float inputs. `MS_goal` is still spawned at the location; it has no attenuation, so the stereo crowd plays unspatialised.

## 7. What I could not verify

- **Anything audible.** That the cheers, groan, ooh, applause and bed sound like a crowd and fit the moments; that no take holds a horn, whistle, word or synthetic artefact (the tests only catch sustained narrow tones); that the burst at -6 dB still reads under the crowd; that the bed seam is inaudible.
- **The cheer's level on its own.** Derived (-17.5 LUFS), not isolated. The graph cannot silence the burst through an input.
- **`period_end` in a live match.** It did not fire in 125 s; it was triggered through the binder only.
- **A conceded goal through the binder.** Variant 1 was tested by spawning `MS_goal` the way the binder does, because the binder does not send it.
- **Shared-state no-repeat in a packaged build.** Seen in PIE only (10 triggers, no repeat).
- **Cooked sizes.** No cook was run.
- **The crowd waves' class.** The waves sit in `SC_Crowd`, but a wave played by a MetaSound Wave Player follows the MetaSound's class (`SC_SFX` for the four event sounds). So `SCM_Volume`'s crowd adjuster moves only `MS_crowd_bed`; that is engine behaviour as I understand it, not measured.
- **The bed's Excite reach.** Measured +4.9 dB, not the full +6, 0.8-5 s after the step. Whether InterpTo reaches +6 dB later was not measured.

## 8. What the lead must decide

1. **The goal's level rule.** The brief's two numbers conflict: a cheer at -16 LUFS on its own, and a sum within 2 dB of -17.2.
   - Now: cheer about -17.5 and sum -16.2 to -16.5.
   - Louder crowd: `MS_goal` mixer Gain 1 at 1.4 gives a cheer of about -16 and a sum of about -15.3.
   - I also lowered the burst 6 dB so the crowd leads ("the burst stays under it"). Keep, or bring the burst back (Gain 0 at 0.7071; the sum then rises about 1 dB)?
2. **The burst on a conceded goal.** It plays under the groan too, as the brief said. A celebratory burst on a concession may read wrong; it could be muted at Variant 1.
3. **`MS_lose`: nothing added.** By its numbers none of the goal-against takes cut to 1.5 s reads as a murmur:
   - The first 1.5 s of all three are steady sounds (envelope range 6-7 dB for `_a`/`_b`), at -18.6 to -19.1 LUFS, with spectral centroids of 1.7-3.0 kHz.
   - `GoalFor_d`, a cheer, has a 2.2 kHz centroid, so the centroid does not separate a groan from a cheer.
   - `_c` fades out inside 1.5 s (43.7 dB envelope range).
   - So nothing marks a "disappointed murmur" by numbers. It needs an ear (try `CRD_GoalAgainst_a` 0-1.5 s, -18.8 LUFS).
4. **Picks.** The operator should listen and choose: the defaults went in as AU2 pre-selected. `GoalFor_c` and `_a` are tone-ish (whoops near 1 kHz), and the in-match goal was `_c`. Swaps per section 4.
5. **`MS_period_end` class.** It is now pure crowd but stays in `SC_SFX`. Move it to `SC_Crowd` if the crowd slider should cover it.
6. **The 8 unreferenced alternates (3.6 MB on disk).** Keep them for the swap, or delete them after the operator picks.
7. **Licences.** As AU2: ElevenLabs paid-plan terms. The organisers' rule on AI-generated audio is still to be confirmed.
