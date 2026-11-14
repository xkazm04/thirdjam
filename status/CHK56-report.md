# Lane CHK56 report: MS_check Variant 5 (braced hit) and Variant 6 (whiff) (device B), 2026-11-14 20:52

- Tree `C:/Users/kazda/jam/wt/b-aud`, branch `lane/b-aud` at 333b965b. Built first (`ThirdJamEditor Development`, 158.8 s, pass). Headless editor
  (`-nullrhi -unattended`) on port 8006, pid 11316, from 20:42 to 20:49; left through `ExitEditor`, process gone. An other editor (pid 21172, started
  20:42) was running and was not touched.
- No commit, push, cook or paid service. No new SoundWave, no ffmpeg render: everything is built inside the MetaSound graph from shipped waves.
- Changed: `Game/Plugins/JamAssets/Content/Audio/MS_check.uasset` only (304,392 -> 368,164 bytes). Backup of the old asset:
  `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/chk56/backup/MS_check.uasset`.
- Editor settings changed for the session and put back: "allow background audio" (off -> on -> off), "use less CPU in background" (on -> off -> on).
  No dirty package at exit.
- **Nobody has heard this.** Every statement below is a measurement of the editor's own output recorded through the submix.

## 1. Done / not done

| Item | Verdict |
|---|---|
| Variant 5, braced hit | **done**: lower-pitched body hit + wooden clack + low thump, level x max(0.5, Strength) |
| Variant 6, whiff | **done**: a stick swish, quiet, no impact |
| Variants 0 to 4 unchanged | **proven sample-exact**: null test of before vs after renders = exact zero difference in all 9 cases |
| Binder sends 5 / 6 | **not B's**: today `UJamEffectsBinder::VariantForPayload` sends 4 for a stumble and 0 otherwise. Until A sends 5 or 6 nothing in the game changes. |

## 2. What was built (graph)

Nothing of the old graph was removed or re-valued: all 52 old nodes are there, **0 pin values changed**, and exactly **two old edges were re-routed**:

| Old edge (removed) | Now |
|---|---|
| `Clamp_e632.Value -> Multiply_7d89.PrimaryOperand` (the stumble gate, "Variant over 3.5") | `Clamp_e632 - Clamp_02ea` (`Subtract_6585`) -> `Multiply_7d89`: the stumble plays at Variant 4 only, not at 5 or 6 |
| `AudioMixerMono2_7ede.Out -> Multiply_bab9.PrimaryOperand` (all sound -> FxTrim) | `AudioMixerMono2_7ede` -> new `AudioMixerMono3_1339` In 0 (gain 1) -> `Multiply_bab9` (FxTrim 0.80, as before) |

For Variant 0 to 4 the new terms are exactly 0 (Clamp_02ea and Clamp_e78b are clamp((Variant - 4.5) x 1000) and clamp((Variant - 5.5) x 1000)), so
`x - 0` and `x x 1 + 0 + 0` give the same floats; the renders confirm it bit for bit.

New graph inputs (WaveAsset): `BracedBody` = `SFX_Hit_b`, `BracedClack` = `SFX_CrossCheck_a`, `Whiff` = `SFX_Mallet_Swing_b`.

New nodes (19):

| Node | Role | Values |
|---|---|---|
| `Subtract_07f7` -> `Multiply_863c` -> `Clamp_02ea` | s5 = Variant is 5 or more | -4.5, x1000, clamp 0..1 |
| `Subtract_2b7a` -> `Multiply_1e10` -> `Clamp_e78b` | s6 = Variant is 6 or more (the whiff's gate) | -5.5, x1000, clamp 0..1 |
| `Subtract_6585` | stumble gate = s4 - s5 | |
| `Subtract_9ad2` | braced gate = s5 - s6 | |
| `Multiply_0cf1` | braced level = `Clamp_d49e` (the hits' max(0.5, Strength), clamp 0.5..1) x braced gate | |
| `WavePlayer_ce75` | **body**: `SFX_Hit_b` (the deeper body-check take) pitched **-3 semitones** (0.38 s, heavier) | Pitch Shift -3 |
| `WavePlayer_dbb4` | **wooden clack**: `SFX_CrossCheck_a` (knock over a thud, the cross-check's own take) pitched **-1 semitone** | Pitch Shift -1 |
| `Sine_1b1c` x `ADEnvelope_d957` (`Multiply_2a8a`) | **low thump**: 55 Hz sine, attack 2 ms, decay 140 ms | 55 Hz; 0.002 / 0.14 s, curve 1 |
| `AudioMixerMono3_999b` | braced layers | body 1.75, clack 1.5, thump 0.7 |
| `Multiply_6984` | braced mix x braced level | |
| `WavePlayer_3356` | **whiff**: `SFX_Mallet_Swing_b` (a rising stick whoosh, no impact) pitched **+2 semitones** (lighter, 0.25 s) | Pitch Shift +2 |
| `Multiply_0942` | whiff x s6 (fixed level, does not follow Strength) | |
| `AudioMixerMono3_1339` | old sound (gain 1) + braced (gain 1) + whiff (gain 0.36) -> FxTrim | 1, 1, 0.36 |

At Variant 5 and 6 the hit path is already muted by the old `Subtract_2187` (1 - "Variant over 3.5"), so neither the random hits, the big hit
(even at Strength 0.75 and over) nor the stumble play: the braced and the whiff replace them. Every layer starts on `OnPlay`, as the hits do. All new
sounds end before the graph's own finish (`TriggerDelay_8dfc` 0.75 s, unchanged): braced 0.35 s, whiff 0.25 s.

Cost: three more Wave Players run (gated to silence) on every check, as the stumble's already does. Not measured; small.

## 3. Numbers

Rendered in the editor world by `create_sound2d(MS_check)` with `Strength` and `Variant` set before play, recorded with
`AudioMixerLibrary.start/stop_recording_output`, one clip per case. For Variants 0 to 3 the take was pinned (`Hits` = one wave, `HitTrims` = its trim)
so the random pick cannot differ between runs. Two renders of the old asset null to exact zero, so the method is sample-exact.

| Case | Before: LUFS / peak dBFS | After: LUFS / peak dBFS (true peak) | Null after vs before |
|---|---|---|---|
| V0, S 0.6, Hit_b | -30.3 / -11.7 | -30.3 / -11.7 (-11.6) | **exact (max diff 0.0)** |
| V1, S 0.6, Hit_b | -30.3 / -11.7 | -30.3 / -11.7 | **exact** |
| V2, S 0.6, Hit_b | -30.3 / -11.7 | -30.3 / -11.7 | **exact** |
| V3, S 0.6, Hit_b | -30.3 / -11.7 | -30.3 / -11.7 | **exact** |
| V0, S 0.3, Hit_a (floor 0.5) | -32.2 / -15.4 | -32.2 / -15.4 | **exact** |
| V0, S 0.6, Hit_c | -29.4 / -9.4 | -29.4 / -9.4 | **exact** |
| V0, S 0.9 (big hit) | -23.5 / -12.2 | -23.5 / -12.2 | **exact** |
| V4, S 0.0 (stumble) | -38.5 / -27.1 | -38.5 / -27.1 | **exact** |
| V4, S 0.9 (stumble) | -34.4 / -23.0 | -33.9 / -23.0 | **exact** (the 0.5 LU is the meter's gating on where the clip sits in the file; the samples are identical) |
| **V5, S 0.3** (braced, floor 0.5) | -36.9 / -25.5 (played the stumble) | **-29.9 / -11.5**, 0.35 s | new |
| **V5, S 0.6** | -35.6 / -24.2 (stumble) | **-28.3 / -9.9**, 0.35 s | new |
| **V5, S 1.0** | -34.1 / -22.7 (stumble) | **-23.8 / -5.5**, 0.35 s | new |
| **V6, S 0.0** (whiff) | -38.5 / -27.1 (stumble) | **-34.9 / -22.5**, 0.25 s | new |
| **V6, S 0.6** | -35.6 / -24.2 (stumble) | **-34.9 / -22.5** | new (fixed level) |

- Braced against a body check at the same Strength 0.6: +1.1 to +2.0 LU (Hit_c -29.4, Hit_b -30.3), lower in pitch, longer (0.35 s against 0.30), with
  the clack and the 55 Hz thump. At Strength 1 it sits about 1 LU under the big hit at 0.9 (-23.5) and has the hottest peak of the check sounds
  (-5.5 dBFS, inside the -3 dBTP rule).
- Layers alone at Strength 1 (first gains, body 2.18 / clack 1.2 / thump 0.35): body -25.5 LUFS, clack -28.5, thump -34.4 (55 Hz weighs little in
  LUFS). I then lowered the body and raised the clack and thump so the wood and the weight carry more (final 1.75 / 1.5 / 0.7).
- Whiff: as quiet as the stumble (-38.5 to -33.9), with a peak 22 dB under full scale and no transient.

WAVs (stereo 48 kHz, the recorder's format), `C:/Users/kazda/jam/wt/b-aud/devlog/shots/`:
`20261114-204934-B-chk56-v0_s06_hitb.wav`, `-v1_s06_hitb.wav`, `-v2_s06_hitb.wav`, `-v3_s06_hitb.wav`, `-v0_s09_big.wav`, `-v4_s09.wav`, `-v5_s03.wav`,
`-v5_s06.wav`, `-v5_s10.wav`, `-v6_s06.wav`, and `20261114-204934-B-chk56-before-v5_s06.wav` (what Variant 5 played before: the stumble). All 42 clips
(before, before2, after, final, and the layer solos) are in
`C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/chk56/wav/`; scripts in `.../chk56/`
(`edit.js` the graph change, `gains.js`, `render.py`/`run.js` the renders, `measure.py` the meter and null test).

What I looked at: the numbers above from `measure.py` (ffmpeg ebur128 + numpy null test), the graph read back by `DescribeGraph` and diffed against
AU12's dump (`scratchpad/au12/MS_check.desc.json`): 52 old nodes present, 0 old pin values changed, 2 edges re-routed, 33 edges added. No still: this
lane makes no picture.

## 4. Not verified

- **Anything by ear**: that Variant 5 reads as "heavier, wooden" and Variant 6 as a whiff.
- **In a match**: no event sends 5 or 6 yet.
- The extra Wave Players' CPU cost; cooked or packaged behaviour (no cook run). The cook pulls in `SFX_Mallet_Swing_b` (19,881 bytes), unreferenced
  until now; `SFX_Hit_b` and `SFX_CrossCheck_a` were cooked already.

## 5. For device A

- To use it: in `UJamEffectsBinder::VariantForPayload`, a `check` that is a held/braced cross-check landing sends `Variant` **5**; a check whose window
  closes unhit sends **6** (if it is played through `MS_check` at all; Strength is ignored by the whiff). Any other value plays as today (0 to 3 a hit,
  4 the stumble). Use exact 5.0 / 6.0.
- A per-take level change is one pin each: `AudioMixerMono3_999b` Gain 0/1/2 (body, clack, thump), `AudioMixerMono3_1339` Gain 1 (braced), Gain 2 (whiff).

## 6. Decisions for the operator

1. The braced hit is about 1 to 2 LU louder than a body check at the same Strength and never becomes the big hit. Louder or a big-hit tier at 0.75 is one
   gain or a few nodes.
2. The whiff has a fixed level (about stumble level). Should it follow Strength?

## 7. Next three steps

1. A sends 5 for the braced landing: in a 100 s clay match every braced check plays `MS_check` at Variant 5, counted from the log (target: all of them,
   0 stumbles or hits at Variant 5).
2. A listen on the GPU machine's speakers: the braced take judged against `v0_s06_hitb` and `v0_s09_big` (one yes/no from the operator).
3. A cook-only run before merging: `MS_check` and `SFX_Mallet_Swing_b` in the cooked list, 0 errors.

## 8. Verdict

**Merge: yes, if you take faults of this size.** It is additive: Variants 0 to 4 are bit-identical, nothing calls 5 or 6 until A's binder does, and the
only new dependency is one shipped wave already in the project.

## 9. Ledger rows

`MS_check` **replaces** its row; `SFX_Mallet_Swing_b` **replaces** its row (now referenced, shipped y); `SFX_Hit_b` and `SFX_CrossCheck_a` replace their
rows for the note only (assets unchanged).

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.35 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; body check: one of three hits at random (Hits, HitTrims), the big hit (HitBig) at Strength 0.75 and over, level x max(0.5, Strength); a stumble (float input Variant 4 from the binder) plays SFX_Stumble_a (input Stumble) at 0.72 x (0.6 + 0.4 x Strength) in place of a hit; Variant 5 a braced hit in place of a hit: BracedBody (SFX_Hit_b, -3 semitones, gain 1.75) + BracedClack (SFX_CrossCheck_a, -1 semitone, 1.5) + a 55 Hz thump (2/140 ms, 0.7), level x max(0.5, Strength); Variant 6 a whiff: Whiff (SFX_Mallet_Swing_b, +2 semitones) at 0.36, no hit; FxTrim 0.80 (lanes AU6, AU8, AU11, AU12, CHK56) (368164 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Swing_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | mallet swing, a rising whoosh; mono 48 kHz 0.3 s, -17.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_check Whiff (Variant 6, +2 semitones, lane CHK56) (19881 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body check impact, deeper; mono 48 kHz 0.32 s, -23.9 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_check Hits (random, trim 1.245) and BracedBody (Variant 5, -3 semitones, lane CHK56) (18581 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_a.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | cross-check, knock over a thud; mono 48 kHz 0.25 s, -22.4 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_lift CrossChecks (random, trim 1.0) and MS_check BracedClack (Variant 5, -1 semitone, lane CHK56) (16568 bytes) |
