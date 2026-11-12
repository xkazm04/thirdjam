# Lane N3: audio ready-making (device B), 2026-11-12 about 14:45

Nobody has listened to any of this. Every statement is from measurements (ffmpeg loudnorm, sample maths) and from the source mp3 files.

## Result
- Script: `C:/Users/kazda/jam/audio-lab/make_ready.mjs` (Node + ffmpeg only, about 30 s for everything, reads the 105 source mp3, never touches existing lab files).
- Output: `C:/Users/kazda/jam/audio-lab/ready/` = 99 WAV (48 kHz, 16-bit; 95 mono, 4 stereo music), `README.md` (per-file table), `manifest.json`.
- Rerun: `node C:/Users/kazda/jam/audio-lab/make_ready.mjs` (options: `--only=<regex>` for a test run that does not rewrite README, `--allow-quiet=<regex>` to force takes that need more than 20 dB).

## Counts (files written)
| class | n | target |
|---|---|---|
| one-shot SFX | 13 | -16 LUFS, -1.5 dBTP |
| very short hits (<0.4 s after trimming; UI clicks, PuckBoards_01, SlapShot_02, Liam "Cut") | 5 | peak -3 dBFS |
| loops / room tone | 4 (2 projector, 2 skate stride) | -26 LUFS |
| voice lines | 73 | -18 LUFS, -1.5 dBTP |
| music | 4 (2 loop files, 2 full-length with fade) | -20 LUFS |
| **not produced** | 8 | see below |

## Cannot be used without regeneration (needs more than 20 dB gain; not amplified)
- `SFX_Clapperboard_01` (silent), `_02` (+51 dB), `_03` (+47 dB), `_05` (+34 dB): only `_04` is usable.
- `SFX_PuckPost_01` (+28 dB), `_02` (+22 dB): both takes of the post hit are too quiet.
- `SFX_SoundStageRoom_01` (+28.6 dB), `_02` (+28.4 dB): room tone needs about +28 dB to reach -26 LUFS, so by the 20 dB rule neither is produced. If the operator wants one anyway: `node make_ready.mjs --only=SoundStageRoom --allow-quiet=SoundStageRoom` (raises the mp3 noise floor as well; listen first).
So there is currently no usable post hit, no room tone, and exactly one clapperboard.

## Flags on written files
- At or over full scale at source (clipped, or just mp3 overshoot; TP +0.1 to +3 dB on the mono fold): BodyCheck_02, Clapperboard_04, GoalHorn_01 and _02, PuckBoards_02, RuleChangeSting_01, SkateCarve_01 and _02, SkateStrideLoop_01, SlapShot_01. Clipping that is in the source stays in the file; check by ear.
- Peak-limited (a plain gain to the -1.5 dBTP ceiling was applied because linear loudnorm cannot reach the LUFS target under that ceiling; no limiter or compressor used, so the result sits under the target): BodyCheck_01 (8.8 LU under -16), SkateCarve_02 (7.1), SlapShot_01 (5.6), BodyCheck_02 (4.6), SkateCarve_01 (3.9), Clapperboard_04 (3.0), PuckBoards_02 (1.3), RuleChangeSting_02 (1.2); voice: Liam_Action (4.4), C2_TenSeconds (3.1), Liam_PlacesEveryone (2.7), Liam_CutPrintIt (1.8), and eight others within 1.2 to 1.5 LU. Smaller shortfalls are in the numbers but not flagged. These hits are transient-heavy; raise them in the engine only by a per-asset volume multiplier if the mix needs it, or accept the lower level.
- Real stereo content folded to mono (side/mid above -20 dB): most SFX (the sources are stereo 44.1 kHz mp3). Folded with each channel at -3 dB summed; check for phasey or hollow sound.
- Loops: SFX loops are cross-faded 150 to 400 ms (chosen per take by score); DC offset removed (about -65 to -90 dBFS before, below -150 after). Seam numbers are in the README. By the numbers all four are continuous (sample step 0.1x to 0.5x of the normal step), but ProjectorLoop_02 has more high-frequency activity near the seam than elsewhere (+5.6 dB roughness) and the stride loops are rhythmic, so the last 20 ms is quieter than the first 20 ms after the seam by design. All loops: ear check.
- Short-clip loudness: for clips under 0.4 s no LUFS exists; result LUFS shows n/a.
- Trimming at -60 dB RMS shortens tails (PuckBoards_01 1.0 -> 0.32 s, SlapShot_01 1.0 -> 0.43 s, UiClick_01 0.48 -> 0.08 s). If a tail sounds cut, it is the threshold, change `trimEdges` in the script.

## Music
- MenuTheme: tempo 110 BPM (onset autocorrelation, strength 0.80), intro ends about 4.3 s, fade-out starts 42.1 s. Loop file = 5.33 s to 40.24 s (34.9 s, 16 bars of 4/4), end cross-faded 1 s equal-power into the audio before the loop start. Seam: sample step 15.6x before, 0.0x after.
- MatchLoop: tempo read as 100 BPM (autocorrelation said 200, halved by an octave guard; unreliable), fade-out starts 58.0 s. Loop file = 26.85 s to 55.66 s (28.8 s, 12 bars). Seam: sample step 5.3x before, 1.1x after.
- Full-length versions (`MUS_MenuTheme_01.wav` 44.7 s, `MUS_MatchLoop_01.wav` 59.8 s) keep the fade-out; total unique music 104.5 s, under the 180 s limit.
- **Honest caveat:** the loops were found by envelope and spectrum matching with a 4/4 assumption; envelope correlations are only 0.5 to 0.6 and the spectral distances 5 to 6.5 dB, i.e. a moderate match, not a proven one. The seam may be audible (a chord or phrase cut, a groove hiccup) and the tempo (especially MatchLoop) may be wrong. The loop files do not contain the intro. They need the operator's ear before use; fall back to the full-length file played once, or to an engine-side loop with a tail.

## Not verified (nobody listened)
Whether any trimmed tail or fade sounds natural; whether the cross-fades are audible; whether clipped sources sound distorted; whether the mono fold changes the character; whether music loops land musically; whether the voice or sound content is what the prompts asked for. Engine import (Bink q40, sound classes, looping flag) was not tried: there is no Unreal on this machine. Engine settings per file are in the README column "engine".
