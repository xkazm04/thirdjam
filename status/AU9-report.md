# Lane AU9 report: the matches at full speed, and the cross-check's sound (device B), 2026-11-13 17:27

- Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8000 from 17:19 to 17:25, left through `ExitEditor` (pid 39732 gone). No git write, build, cook or outside service. Changed under `Audio/` only: `MS_lift` and three new waves; plus this report.
- **Nobody on this device has heard any of this.** Every statement is a measurement of PIE output recordings.
- `/Game/Maps/Rink` not saved. No dirty package at exit. `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.Cut` `hockey.regular`; the editor's background throttle is back on. The editor on 8002 was not touched.
- Log: no MetaSound or audio error. The only `JAMFX missing` lines are `NS_deke` / `MS_deke`.

## Verdicts

| Item | Verdict | Evidence |
|---|---|---|
| 1. Both matches at full speed | **PASS** | 58 ticks a second in both. Plain 90 s: -24.2 LUFS, -6.9 dBTP. Drill 60 s: -22.5 LUFS, -4.7 dBTP. Nothing at full scale. Shares measured (section 2). |
| 2. The cross-check's sound | **PASS** | `MS_lift` clean plays one of three cross-check takes at random, -32.2 to -32.6 LUFS, 2.6 to 3.4 LU under a body check at 0.6. It is 4.5 % of the plain match's energy (limit 15 %). The miss is unchanged. |
| 3. Nothing else from AU5 | done | Only `CrossCheck_a/b/c` imported. |

## 1. Tick rates

| When | Ticks per second |
|---|---|
| Before recording, 4 s sample in PIE | 57.7 |
| Plain match, 92 s | 57.8 (5,317 ticks) |
| Drill match, 66 s | 58.1 (3,832 ticks) |
| For reference: AU7's matches | 42 and 55 |
| For reference: AU8's matches | 4 to 20 |

## 2. The two matches, trims on, cross-check in

Default cut, `jam.ScriptHuman 1`; the drill adds `jam.ctrl.Drill 6`.

| | Plain, 90 s | Drill, 60 s |
|---|---|---|
| Integrated | **-24.2 LUFS** | **-22.5 LUFS** |
| True peak | **-6.9 dBTP** | **-4.7 dBTP** |
| Samples at full scale | 0 | 0 |
| Checks and knockdowns | 11 | 13 |
| Clean lifts (cross-checks) / pokes / shots | 65 / 60 / 27 | 10 / 0 / 3 |
| Puck boards / skater boards | 35 / 39 | 2 / 5 |
| Commentary lines | 20 (32 s) | 14 (24 s) |
| Hit moment over the 0.9 s before: min / median / max | 0.9 / 4.5 / 11.6 dB (10 hits) | 4.3 / 7.4 / 14.6 dB (13 hits) |
| Highest peak sits on | a skater into the boards at Strength 1 with a cross-check | a tackle with a foul |

- The plain match is 0.2 LU under the -23 window; the drill is inside it.
- This plain match holds more events than AU7's (65 lifts against 43, 11 checks against 4) because it ran at 58 ticks a second, not 42.
- **The hit moment did not drop.** AU7 had medians of 3.2 dB (plain) and 7.2 dB (drill) before the operator's trims; now 4.5 and 7.4. The hit lost 1.9 dB, but the effects around it lost 3.1.

### Energy shares, measured beside AU8's projection

Same method as AU7 and AU8, now on real full-speed recordings. Percent of the modelled total.

| Track | Plain, measured | Plain, AU8 projection | Drill, measured | Drill, AU8 projection |
|---|---|---|---|---|
| Music | 37 % | 48 % | 23 % | 27 % |
| Commentary | 20 % | 16 % | 16 % | 13 % |
| Crowd bed | 11 % | 13 % | 7 % | 8 % |
| Crowd reactions | 5 % | 2 % | 25 % | 26 % |
| **Music + commentary + crowd** | **73 %** | **79 %** | **72 %** | **74 %** |
| Gameplay effects | 20 % | 17 % | 13 % | 12 % |
| Hits | 6 % | 3 % | 14 % | 14 % |
| **Effects + hits** | **26 %** | **20 %** | **27 %** | **26 %** |
| Interface blips (bark) | 1 % | 1 % | 1 % | 1 % |
| Dominant tracks over effects + hits | **4.5 dB** | 5.9 dB | **4.2 dB** | 4.6 dB |

- The drill lands on the projection. The plain match has a larger effects share than projected because it holds half again as many events.
- The model sums to 124 % of the measured energy in both matches, so read each share to several percent. The skate loop is not in it.
- **Largest single effect in plain play now: the poke**, 60 events, about 9 % of the modelled total. The cross-check (67 lifts) is about 4 %; body checks 4 %; the skater boom 3 %.

## 3. What `MS_lift` plays now

| Input | Type | Default |
|---|---|---|
| `Strength` | Float | 0.5 |
| `FxTrim` | Float | **0.80** (was 0.70: it is a hit now; nothing renamed) |
| `Variant` | Float, new | 0 (clean); the binder already sends 1 for a miss |
| `CrossChecks` | WaveAsset array, new | `SFX_CrossCheck_a`, `_b`, `_c` |
| `CrossCheckTrims` | Float array, new | 1.0, 1.072, 1.479 (0, +0.6, +3.4 dB: AU5's loudness offsets) |

- **Clean (Variant 0):** one take at random (no immediate repeat, shared state) x its trim x the old Strength law (0.35 + 0.65 x S), at `AudioMixerMono2_1ab9` Gain 0 = **0.68**.
- **The old stick layer stays under it** at 0.35 (`Add_d2de`), about 8 dB under where it was. By measurement it adds a short tail: with it, all three takes are 40 dB down after 105 to 115 ms; the files alone are 40 dB down after 130, 70 and 40 ms. So it lengthens takes b and c by 35 to 70 ms and does nothing for take a.
- **Miss (Variant 1):** the old stick sound only, at 0.875 x `FxTrim` 0.80 = 0.70, exactly as before. Measured -40.4 LUFS / -29.4 dBFS, the same as after AU8.
- The whole sound still passes through `FxTrim`.

| Case | LUFS | Peak dBFS | Length s |
|---|---|---|---|
| Clean, `CrossCheck_a` | -32.2 | -15.3 | 0.20 |
| Clean, `CrossCheck_b` | -32.6 | -16.2 | 0.13 to 0.17 |
| Clean, `CrossCheck_c` | -32.2 | -12.3 | 0.28 |
| Miss | -40.4 | -29.4 | 0.12 |
| `MS_check` at 0.6, for comparison | -29.3 to -29.6 | -9.4 to -13.9 | 0.19 to 0.32 |
| The old clean lift (after AU8) | -31.3 | -20.4 | 0.27 |

- The three takes are within 0.4 LU. The clean cross-check is 2.6 to 3.4 LU under the body check at 0.6.
- Against the old clean lift it is about 1 LU quieter by loudness, and its peak is 4 to 8 dB higher: it is a crack now, not a swish.
- Both paths were tried through the binder as well (`lift` clean and miss): same results.
- One of nine clean plays read -33.9 LUFS with a longer tail; a match sound had leaked into that recording window.
- Swap: `CrossChecks` in the Members panel (keep `CrossCheckTrims` in the same order). Level: Gain 0 of `AudioMixerMono2_1ab9`. To drop the old layer under the crack: `Add_d2de` 0.35 to 0, and `Multiply_5947` 0.525 to 0.875 so the miss stays where it is.

## 4. Files (bytes, `Game/Plugins/JamAssets/Content/Audio/`)

| File | Before | After |
|---|---:|---:|
| `MS_lift.uasset` | 193,679 | 293,410 |
| `SFX/SFX_CrossCheck_a.uasset` | new | 16,568 |
| `SFX/SFX_CrossCheck_b.uasset` | new | 17,221 |
| `SFX/SFX_CrossCheck_c.uasset` | new | 16,890 |

Ledger rows: three new waves; the `MS_lift` row **replaces** the existing one.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | cross-check, knock over a thud; mono 48 kHz 0.25 s, -22.4 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_lift CrossChecks (random, trim 1.0) (16568 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | cross-check, bright crack; mono 48 kHz 0.25 s, -23 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_lift CrossChecks (random, trim 1.072) (17221 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_CrossCheck_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | cross-check, short click; mono 48 kHz 0.33 s, -25.8 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_lift CrossChecks (random, trim 1.479) (16890 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lift.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, mono; clean (Variant 0): the cross-check, one of three takes at random (CrossChecks, CrossCheckTrims, no repeat) at 0.68 with the old stick synth 8 dB down under it; miss (Variant 1): the old stick synth, unchanged; FxTrim 0.80 (a hit); clean -32.2 to -32.6 LUFS, miss -40.4 (lanes AU7, AU8, AU9) (293410 bytes) |

## 5. Recordings

In `C:/Users/kazda/jam/audio-lab/tackle/engine/` (AU8's three moved to `engine/au8/`) and in the scratch folder `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au9/`:

- `match_plain_90s.wav`, `match_drill_60s.wav`: full speed, trims on, cross-check in.
- `crosscheck_clean.wav`: three plays alone, takes a, b, c in that order.
- `chain_s06.wav`, `chain_s10.wav`: check and knockdown together at the present levels (-29.9 LUFS / -11.7 dBFS and -23.6 LUFS / -9.8 dBFS).
- Scratch only: `fin.wav` and `t1.wav` (the trigger tests), `backup/MS_lift.uasset` (as it was at 17:19).

## 6. What I could not verify

- **Nobody on this device has heard it.** Whether the cross-check reads as a stick on a padded back, whether it is "lighter than a body check" to an ear (it is 3 LU lighter by loudness, but take c peaks as high as a hit), and whether the old synth under it sounds like a tail or like a second, synthetic sound.
- Whether three takes rotating 65 times in 90 s grate. No repeat is immediate; that is all the graph guarantees.
- The 15 % limit in other matches: measured in this one plain match only (4.5 %).
- The share model overshoots the measured energy by 24 %; most likely the commentary (I assumed its `Volume` at 1) and the music's duck.
- The picture: I did not check that the crack lands on the shove's contact frame.
- Cooked or packaged behaviour. No cook was run.

## 7. Decisions for the lead

1. **The poke is now the loudest frequent effect** (60 in 90 s, about 9 % of the mix, -29.1 LUFS, peak -13.6 dBFS). It was left alone in AU7 and took only the operator's 30 %. If it should sit under the cross-check, its `FxTrim` is one number.
2. **The old stick layer under the crack**: kept at 0.35 because it measurably lengthens two of the three takes. It is a sine glide and noise; if the operator hears a synthetic tone under the crack, drop it (two numbers, section 3).
3. **Take c** (`CrossCheck_c`) carries a +3.4 dB trim to match the others' loudness, so it peaks 3 to 4 dB over them (-12.3 dBFS). If it sticks out, remove it from `CrossChecks` (two takes: the no-repeat rule then switches off).
4. **Plain play at -24.2 LUFS**, 0.2 under the window, with the operator's trims in. The lever for more is still the music `Level` (device A).
