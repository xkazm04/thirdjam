# Lane AU6 report: the tackle sounds in the engine (device B), 2026-11-13 14:40

- Tree `C:/Users/kazda/jam/wt/b-vfx`. No git write (read-only `git status` and `git log` only), no build, no cook, no outside service. Only files under `Game/Plugins/JamAssets/Content/Audio/` changed, plus this report.
- **Nobody has listened to any of this.** Every audio statement is a measurement of recordings of the PIE output (48 kHz stereo): ffmpeg for peak, true peak and EBU R128 loudness (clip padded with 1 s of silence), numpy for the rest.
- Editor on port 8000, two sessions. First: 13:40 to 14:08; it was closed from outside while I was cut off (`QUIT_EDITOR`, `LogExit: Exiting.`, no fatal line). Second: 14:14 to 14:34, left through `ExitEditor`; pid 40532 is gone. After the restart all three MetaSounds loaded without an error and matched what I had built (inputs, defaults, gains, delays, outputs connected). Nothing had to be restored from the backup.
- `/Game/Maps/Rink` was never saved (still 2026-11-12 12:40). No dirty package at exit. `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.Cut` `hockey.regular` at exit. The recordings were moved out of `Game/Saved`.
- The other editor (main tree, port 8002) was never touched.

## Verdicts

| Item | Verdict | Short evidence |
|---|---|---|
| Import | **PASS** | 26 waves: 20 `SFX_*` mono in `Audio/SFX/` (`SC_SFX`), `CRD_Hit_a/b/c` and `CRD_Roar_a/b/c` stereo in `Audio/Crowd/` (`SC_Crowd`); Bink quality 40, not looping, no attenuation, as the earlier lanes. |
| 1. `MS_check` | **PASS** | One of `Hit_a/b/c` at random, equally loud within 0.3 LU; `HitBig_a` from Strength 0.8. Louder than today at every Strength by loudness (+2.5, +1.1, +3.5 LU at 0.3, 0.6, 1.0). Old sample out, old synth layer muted. |
| 2. `MS_knockdown` | **PASS** | Landing at 0.170 s, slide at 0.200 s, crowd at 0.050 s, get-up at 1.050 s, each measured to the millisecond. No crowd under 0.5, "ooh" from 0.5, roar from 0.8. Old noise sweep removed. |
| 3. `MS_board` | **PASS** | The skater case does reach the sound (Variant 1; section 3). Boom plus glass rattle 0.030 s later, 6.2 dB under. Puck case unchanged within 0.1 dB. |
| 4. Standing bump | **NOT WIRED, as briefed** | No event exists for it. Three waves imported, unreferenced. The ask for device A is in section 8. |
| 5. Classes and mix | **PARTIAL** | All three stay in `SC_SFX`. The chain is 5 to 16 dB over the bed and music in the match, and no sample clips. But the mixes measure -20.5 and -20.8 LUFS, not -23, and the true peaks are -1.1 and -0.9 dBTP. By my estimate this lane adds 0.2 dB to a plain bot match; the rest was already there (section 5). |
| Trigger test | **PASS** | 30 triggers through the binder, every case and Variant; tables in section 4. |
| Real match | **PASS** | Drill 6, 75 s recorded: 13 `check` and 13 `knockdown` in the log, 13 hits found in the recording, each with its landing and get-up; the crowd on the 11 at Strength 0.5 or more. No MetaSound or audio error line. The only `JAMFX missing` lines are `NS_deke` / `MS_deke`, which are not this lane's. |

## 1. Import

- Source: `C:/Users/kazda/jam/audio-lab/tackle/`. Names as AU4 and AU5 proposed: `SFX_<Name>`, `CRD_Hit_<x>` (from `CrowdHit_<x>`), `CRD_Roar_<x>` (from `CrowdRoar_<x>`).
- Imported: `Hit_a/b/c`, `HitBig_a/b`, `Land_a/b/c`, `Slide_a/b`, `BoardsBody_a/b/c`, `Bump_a/b/c`, `GetUp_a/b`, `GlassRattle_a/b`, `CrowdHit_a/b/c`, `CrowdRoar_a/b/c`. Nothing else from AU5.
- Settings read back after the restart: channels 1 (SFX) or 2 (crowd), compression quality 40, looping off, attenuation none, loading behaviour inherited, lengths equal to the WAVs.
- 15 are referenced by a MetaSound, 11 are not (the alternates and the three bumps).

## 2. What each MetaSound plays now

`S` is the `Strength` input. "Big" is 1 when S is 0.8 or more, else 0. The binder's inputs are untouched: `Strength` on all three, and `MS_board` now reads the `Variant` the binder already sends.

### `MS_check` (mono, `SC_SFX`, ends at 0.75 s)

| Input | Type | Default |
|---|---|---|
| `Strength` | Float | 0.5 |
| `Hits` | WaveAsset array | `SFX_Hit_a`, `SFX_Hit_b`, `SFX_Hit_c` |
| `HitTrims` | Float array | 1.0, 1.245, 1.585 (same order as `Hits`: 0, +1.9, +4.0 dB, AU4's loudness offsets) |
| `HitBig` | WaveAsset | `SFX_HitBig_a` |

- On play, `Random Get` picks one of `Hits` (no immediate repeat, shared state, as AU3's goal cheers) and the trim with the same index.
- Output mixer `AudioMixerMono3_9829`:
  - Gain 0 = **0**: the old synth (70 Hz thump and noise). Muted, nodes kept, so one number brings it back.
  - Gain 1 = **1.75**: the hit x trim x max(0.5, S) x (1 - big).
  - Gain 2 = **1.25**: `HitBig` x max(0.5, S) x big.
- 1.75 is +4.9 dB: it makes up the 4.7 LU that `Hit_a` is under the old sample. 1.25 on the big hit makes the step at 0.8 go up in loudness (-25.3 LUFS at 0.79, -24.3 at 0.8).
- **`SFX_BodyCheck_02` is out of `MS_check`.** The mix did not need its tail: the hit is louder than before without it.
- **The old synth layer is dropped (muted)** by the same measurement.

### `MS_knockdown` (now stereo, `SC_SFX`, ends at 2.15 s)

| Input | Type | Default |
|---|---|---|
| `Strength` | Float | 0.5 |
| `Lands` | WaveAsset array | `SFX_Land_a`, `SFX_Land_b`, `SFX_Land_c` |
| `LandTrims` | Float array | 1.0, 0.933, 0.804 (0, -0.6, -1.9 dB) |
| `Slide` | WaveAsset | `SFX_Slide_a` |
| `CrowdOohs` | WaveAsset array | `CRD_Hit_a`, `CRD_Hit_b`, `CRD_Hit_c` |
| `CrowdRoar` | WaveAsset | `CRD_Roar_a` |
| `GetUp` | WaveAsset | `SFX_GetUp_a` |

| Layer | Starts | Delay node | Gain | Scaled by |
|---|---|---|---|---|
| Crowd "ooh" (random, no repeat) | 0.05 s | `TriggerDelay_8955` | 0.8 (Gain 1 of `AudioMixerMono3_b77a` and `_b525`) | S (clamped 0 to 1); only when S is 0.5 to under 0.8 |
| Crowd roar | 0.05 s | `TriggerDelay_8955` | 0.8 (Gain 2 of the same two) | S; only when S is 0.8 or more |
| Landing (random, no repeat) | 0.17 s | `TriggerDelay_470c` | 1.24 (Gain 0 of `AudioMixerMono3_f522`) | trim x max(0.5, S) |
| Slide | 0.20 s | `TriggerDelay_e57b` | 0.70 (Gain 1 of `_f522`) | max(0.5, S) |
| Get-up scrape | 1.05 s | `TriggerDelay_e71c` | 0.98 (Gain 2 of `_f522`) | nothing: fixed |

- The three mono layers are mixed in `AudioMixerMono3_f522`, which feeds both channels at **0.5** (Gain 0 of `_b77a` and `_b525`). 0.5 is what the engine does to a mono MetaSound here (finding 1 below), so these gains read on the same scale as `MS_check`'s.
- Relative to the hit (1.75): landing 3 dB under (as in AU4's preview), slide 8 dB under its delivered relation (AU4's loudness reading), get-up 5 dB under its delivered relation at Strength 1.
- Under Strength 0.5 there is no crowd. The thresholds are `Subtract_29bc` (0.499) and `Subtract_e569` (0.799).
- **The old content is gone.** It was a filtered-noise sweep of 0.5 s starting 20 ms after the trigger. It would have sat on the hit and doubled as a second, synthetic slide.
- The format change followed AU3's trap notes (builder `set_format`, then `output_format`, then the outputs wired). Both outputs are connected; `output_format` reads STEREO, 2 channels.

### `MS_board` (mono, `SC_SFX`, ends at 0.9 s, was 0.4)

| Input | Type | Default |
|---|---|---|
| `Strength` | Float | 0.5 |
| `Variant` | Float (new) | 0 |
| `BoardsBody` | WaveAsset | `SFX_BoardsBody_a` |
| `GlassRattle` | WaveAsset | `SFX_GlassRattle_a` |

- v = `Variant` clamped to 0..1. Output mixer `AudioMixerMono2_0ba0` (gains 1, 1): the old puck graph x (1 - v), and the skater layer x v.
- Skater layer, mixer `AudioMixerMono2_117d`: Gain 0 = **1.4** boom, Gain 1 = **0.7** rattle (6 dB under), rattle delayed 0.03 s by `TriggerDelay_3ea7`; the pair x max(0.5, S), S clamped to 1 (the sim sends over 1 for a skater).
- Following your addition, the rattle is `SFX_GlassRattle_a`, not `SFX_BodyCheck_02`.
- The puck case is the old graph, untouched: `SFX_PuckBoards_01` 0.8 over the synth 0.4.

### Three findings from building

1. **A mono MetaSound comes out at -6.0 dB per channel; a mono layer fed at 0.7071 into a stereo MetaSound comes out at -3.0 dB.** Measured by matching the source file against the recording: `HitBig_a` at gain 1.25 (+1.9 dB) reads -4.1 dB; the boom at 1.4 (+2.9 dB) reads -3.1 dB; the get-up at 0.7 x 0.7071 (-6.1 dB) read -6.0 dB. So AU3's rule "feed the old mono path at 0.7071 to keep its level" raises it by 3 dB. I used 0.5 in `MS_knockdown`. I did not measure or touch `MS_goal`, `MS_win`, `MS_save`, `MS_period_end`; by this their synth layers are 3 dB hotter than before they became stereo.
2. **A stereo wave comes out 3.0 dB under its mixer gain** (roar at 0.8 x 1.0 = -1.9 dB reads -4.9 dB; six cases agree). AU3's file-to-game offset of 3.1 dB is the same thing.
3. **`CommitBuilderToAsset` saves the package to disk** in this editor (the file time follows each commit; no dirty package afterwards). No separate save was needed.

## 3. Does a skater-into-boards case reach `MS_board`? Yes.

- `JamSim.cpp` line 1364: when a skater's speed into the boards is 4 m/s or more, the sim emits `board` with Who = the skater, Kind = `skater`, Strength = normal speed / `game.skater.speed`.
- `EffectsBinder.cpp` lines 199 to 202: `VariantFor` returns 1 for `board` with Kind `skater`, else 0. Lines 405 to 419: the binder plays `MS_board` and sets `Strength` and `Variant`.
- Until now `MS_board` had no `Variant` input, so a skater played the puck crack. Baseline recording: Variant 0 and 1 measured the same.
- In the recordings: the drill match had 5 `board` / `skater` events and the boom was found at all 5; the plain match had 30.

## 4. Trigger test

- PIE on `Rink`, default cut (`hockey.regular`, tier `l2_night`, as AU3 recorded). Each case through `UJamEffectsBinder.handle_event`, at time dilation 0.01, with the music, the bed and the skate loop muted for the test.
- Length is first to last sample over -60 dBFS. "Loudest 100 ms" is RMS. "Old" is the same trigger recorded before any change, same session.
- The takes were identified by matching each source file against the recording.

### `MS_check` alone

| Strength | Take | Peak dBFS | True peak dBTP | LUFS | Loudest 100 ms dB | Length s |
|---|---|---|---|---|---|---|
| 0.3 | `Hit_b` | -11.3 | -11.3 | -29.0 | -24.3 | 0.28 |
| 0.3 | `Hit_a` | -13.5 | -13.5 | -29.3 | -25.1 | 0.19 |
| 0.3 | `Hit_c` | -9.1 | -9.1 | -29.1 | -23.6 | 0.32 |
| 0.3 | old | -14.1 | -14.1 | -31.7 | -25.1 | 0.72 |
| 0.6 | `Hit_a` | -11.9 | -11.9 | -27.7 | -23.5 | 0.20 |
| 0.6 | `Hit_c` | -7.5 | -7.5 | -27.5 | -22.0 | 0.32 |
| 0.6 | `Hit_b` | -9.7 | -9.7 | -27.4 | -22.7 | 0.28 |
| 0.6 | old | -11.0 | -11.0 | -28.7 | -22.1 | 0.80 |
| 0.79 | `Hit_a` | -9.5 | -9.5 | -25.3 | -21.1 | 0.20 |
| 0.8 | `HitBig_a` | -11.3 | -11.3 | -24.3 | -17.9 | 0.59 |
| 1.0 | `HitBig_a` | -9.4 | -9.3 | -22.3 | -16.0 | 0.60 |
| 1.0 | old | -8.4 | -8.3 | -25.8 | -19.2 | 0.95 |

- The three takes are within 0.3 LU of each other at each Strength.
- One number goes the other way: at 0.6 the loudest 100 ms of `Hit_a` and `Hit_b` is 0.6 to 1.4 dB under the old sample's (`Hit_c` is level). By loudness all three are over it.
- Strength 0.3 and 0.5 sound the same (the floor of 0.5).

### `MS_knockdown` alone

| Strength | Crowd | Landing | Peak dBFS | True peak dBTP | LUFS | Loudest 100 ms dB | Length s |
|---|---|---|---|---|---|---|---|
| 0.3 | none | `Land_a` | -15.9 | -15.9 | -34.8 | -28.2 | 1.40 |
| 0.49 | none | `Land_b` | -17.1 | -17.1 | -34.9 | -29.9 | 1.40 |
| 0.5 | `CRD_Hit_c` | `Land_a` | -15.1 | -15.1 | -29.9 | -26.1 | 1.76 |
| 0.6 | `CRD_Hit_a` | `Land_c` | -13.9 | -13.9 | -28.6 | -24.8 | 1.69 |
| 0.6 | `CRD_Hit_b` | `Land_a` | -12.0 | -12.0 | -29.5 | -24.6 | 1.52 |
| 0.6 | `CRD_Hit_c` | `Land_c` | -12.1 | -12.1 | -28.6 | -24.4 | 1.76 |
| 0.79 | `CRD_Hit_b` | `Land_b` | -9.9 | -9.9 | -27.4 | -22.9 | 1.52 |
| 0.8 | `CRD_Roar_a` | `Land_a` | -10.2 | -10.2 | -24.7 | -21.4 | 1.74 |
| 1.0 | `CRD_Roar_a` | `Land_c` | -7.9 | -7.9 | -22.6 | -19.3 | 1.74 |
| 1.0 | `CRD_Roar_a` | `Land_b` | -7.4 | -7.4 | -22.6 | -20.4 | 1.74 |
| 0.3 / 0.6 / 1.0 | old | (noise sweep) | -19.5 / -16.6 / -12.9 | same | -30.2 / -27.9 / -25.5 | -29.4 / -27.0 / -24.4 | 0.51 |

- Length counts from the first sound (crowd at 0.05 s, or the landing at 0.17 s when there is no crowd).
- Under 0.5 the knockdown alone is 4.6 LU quieter than the old sweep by loudness (it is two short events and no crowd); its loudest 100 ms is 1.2 dB louder.

### Check and knockdown on the same call (what the game does)

| Strength | Hit | Peak dBFS | LUFS | Loudest 100 ms dB | Length s |
|---|---|---|---|---|---|
| 0.3 | `Hit_a` | -13.5 | -33.5 | -25.1 | 1.57 |
| 0.6 | `Hit_b` | -9.7 | -28.8 | -22.6 | 1.57 |
| 0.6 | `Hit_a` | -11.9 | -28.3 | -23.3 | 1.81 |
| 0.8 | `HitBig_a` | -9.2 | -23.6 | -17.6 | 1.79 |
| 1.0 | `HitBig_a` | -7.7 | -21.8 | -15.7 | 1.79 |

### `MS_board`

| Variant | Strength | Peak dBFS | True peak dBTP | LUFS | Length s | Old peak / LUFS |
|---|---|---|---|---|---|---|
| 0 puck | 0.3 | -17.4 | -17.4 | -32.1 | 0.20 | -17.6 / -32.1 |
| 0 puck | 0.6 | -14.2 | -14.2 | -28.8 | 0.22 | -14.1 / -28.7 |
| 0 puck | 1.0 | -11.0 | -11.0 | -25.7 | 0.28 | -11.1 / -25.7 |
| 1 skater | 0.3 | -15.2 | -15.1 | -32.1 | 0.56 | -17.6 / -32.1 (the puck crack) |
| 1 skater | 0.6 | -13.6 | -13.5 | -30.5 | 0.56 | -14.1 / -28.8 |
| 1 skater | 1.0 | -9.1 | -9.1 | -26.1 | 0.56 | -10.9 / -25.7 |

- Rattle: found 0.030 s after the boom, 6.2 dB under it. A Strength of 1.6 measures the same as 1.0 (clamped).
- By loudness the skater boom is no louder than the crack it replaces; its peak is about 2 dB higher.

### The timeline inside `MS_knockdown`

Measured by matching each source file against the recording (the lag of the best match), not by eye.

| Layer | Brief | Measured against the hit of `MS_check` (5 joint triggers) | Measured inside `MS_knockdown` alone (10 triggers) |
|---|---|---|---|
| Crowd | 0.05 s | +0.050 s in 4 of 4 with a crowd | reference |
| Landing | 0.17 s | +0.170 s in 4; not separable under the big hit in 1 | 0.120 s after the crowd in 8 of 8 |
| Slide | 0.20 s | +0.200 s in the 1 where it could be found | 0.030 s after the landing in the 2 where it could be found (5 of 8 in the earlier pass) |
| Get-up | 1.05 s | +1.050 s in 5 of 5 | 0.880 s after the landing in 10 of 10 |

- All within 1 ms. The slide is usually masked by the landing, 30 ms before it.

## 5. The real match

- Fresh PIE on `Rink`, default cut, `jam.ScriptHuman 1`, `jam.ctrl.Drill 6`. 75 s recorded from 13.7 s after the match started; `match_60s.wav` is its first 60 s.
- In this run the scripted human did the checking (12 of 13 checks have Who 0); the drill's own line `JAMDRILL kd_human` came once.
- The match ended at 62 s (third goal, `lose`), so the first 60 s are all live play.
- I turned off the editor's "use less CPU in background" for the session (it was holding the unfocused editor near 6 ticks a second while another lane's Blender ran; every sound then started 0.3 to 1.5 s late, old ones too). It is back on.

### Events against the recording

| | Log (`JAMEVENT`) | Found in the recording |
|---|---|---|
| `check` | 13 | 13 hits (11 `HitBig_a`, 2 `Hit_c`), each at the gain its Strength gives |
| `knockdown` | 13 | landing 13 of 13; get-up 13 of 13; crowd 11 of 11 with Strength 0.5 or more (all roars), none on the two under 0.5 |
| `board` / `skater` | 5 | boom 5 of 5 |

- Knockdown strengths: 11 at 0.88 or more, 2 under 0.5. So this run shows the roar and not the "ooh"; the "ooh" is proven by the trigger test only.
- In 12 of 13 the landing sits 0.170 s after the hit (one of the 12 is a weak match: two checks 0.38 s apart). **In one (Strength 0.05, at 8.3 s) the whole knockdown sound started 30 ms after the check's**: landing at 0.200 s, get-up at 1.080 s. The two are separate sounds started on the same frame, and the engine does not promise the same start.
- Level of the hit moment: the 0.3 s from the hit is **5.4 to 16.1 dB over the 0.9 s before it** (bed and music). The two cases at +2.8 and +3.4 dB are second hits 0.3 s after another one.
- Log: no `LogMetaSound` warning or error, no "Metasound generator expected" line, no `LogAudio` warning. `JAMFX missing` appears twice per session, for `NS_deke` and `MS_deke` (lane A39's event, no assets yet).

### The mix

| Recording | Length | Integrated | True peak | Samples at full scale |
|---|---|---|---|---|
| Drill 6 match (`match_60s.wav`) | 60 s | **-20.5 LUFS** | **-1.1 dBTP** | 0 |
| The same, all 75 s (`match_raw.wav`) | 75 s | -20.7 LUFS | -1.1 dBTP | 0 |
| Plain bot match, no drill (`match_nodrill.wav`) | 92 s | **-20.8 LUFS** | **-0.9 dBTP** | 0 |
| AU3 last night, for reference | 90 s | -22.5 LUFS | -2.1 dBFS | |

- **The mix is about 2 LU over the earlier figure, and little of that is this lane.**
  - Plain match: 5 checks, 30 skater board hits, 32 puck board hits, 27 shots in 92 s. With the tackle and skater-board windows cut out, the rest is -21.8 LUFS.
  - Estimate (the engine's own rendering of each new layer placed at the logged event times, compared by energy): the new layers are 9 % of the plain match's energy; with the old sounds at the same events the mix would be 0.2 dB lower.
  - Drill match: the new layers are 35 % of the energy; with the old sounds it would be 1.2 dB lower. Without goal, tackle and skater-board windows it is -23.5 LUFS.
- **Peaks: nothing clips, but there is under 1 dB of room.**
  - The six highest peaks of the drill match all sit on hits of Strength 0.88 or more. At four of them I took my hit out of the mix: what is left still peaks at -4.5 to -6.0 dBFS. Other sounds fire on the same frame: a `lift` at Strength 1 at 39.9 s, the foul whistle at 24.3 s and 55.3 s.
  - The big hit alone peaks at -9.4 dBFS; the old sample peaked at -8.4. So the hit instant is not hotter than before.
  - The plain match's highest peak (-0.9) is in a burst of shots and puck board hits; removing the skater boom there does not lower it.
- The sounds are not panned: at all 13 hits and 5 booms the left and right gains are equal. So making `MS_knockdown` stereo lost no positioning.

## 6. Swap instructions (one minute per sound)

Open the MetaSound, select the input in the Members panel, change "Default Value", save with PIE stopped. Through MCP: `AttachBuilderToAsset`, `SetDefaults` on the graph input, `CommitBuilderToAsset`. Wave paths: `/JamAssets/Audio/SFX/SFX_<Name>.SFX_<Name>` and `/JamAssets/Audio/Crowd/CRD_<Name>.CRD_<Name>`.

| Sound | Asset | Input | Now | Imported alternates | Level note |
|---|---|---|---|---|---|
| Hit | `MS_check` | `Hits` (array) | `SFX_Hit_a`, `_b`, `_c` | all in use | Changing the list: keep `HitTrims` in the same order |
| Big hit | `MS_check` | `HitBig` | `SFX_HitBig_a` | `SFX_HitBig_b` | `_b` is 2.8 LU quieter: Gain 2 of `AudioMixerMono3_9829` from 1.25 to 1.72 |
| Landing | `MS_knockdown` | `Lands` (array) | `SFX_Land_a`, `_b`, `_c` | all in use | Keep `LandTrims` in the same order |
| Slide | `MS_knockdown` | `Slide` | `SFX_Slide_a` | `SFX_Slide_b` | same loudness |
| Crowd "ooh" | `MS_knockdown` | `CrowdOohs` (array) | `CRD_Hit_a`, `_b`, `_c` | all in use | same loudness |
| Crowd roar | `MS_knockdown` | `CrowdRoar` | `CRD_Roar_a` | `CRD_Roar_b`, `_c` | same loudness |
| Get-up | `MS_knockdown` | `GetUp` | `SFX_GetUp_a` | `SFX_GetUp_b` | same loudness |
| Boards boom | `MS_board` | `BoardsBody` | `SFX_BoardsBody_a` | `_b`, `_c` | `_c` is 2.2 LU quieter |
| Glass rattle | `MS_board` | `GlassRattle` | `SFX_GlassRattle_a` | `SFX_GlassRattle_b` | `_b` is 2.1 LU louder: Gain 1 of `AudioMixerMono2_117d` from 0.7 to 0.55 |

- **One take instead of three:** set the array to one entry and its trim array to one number. With one or two entries the no-repeat rule turns itself off (AU3's note).
- **A trim** brings every take of a group to one loudness. `HitTrims`: 10^((-22 - take's LUFS) / 20), so the loudest take (`Hit_a`, -22 LUFS) is 1.0. `LandTrims`: 10^((-21.6 - take's LUFS) / 20), so the quietest take (`Land_a`, -21.6 LUFS) is 1.0. The LUFS are in AU4's table.
- Every alternate fits the end times. If a longer take comes: `TriggerDelay_8dfc` (0.75, `MS_check`), `TriggerDelay_56f0` (2.15, `MS_knockdown`), `TriggerDelay_e871` (0.9, `MS_board`).
- **Levels in one number each:** hit `AudioMixerMono3_9829` Gain 1; big hit Gain 2; landing, slide, get-up `AudioMixerMono3_f522` Gain 0, 1, 2; "ooh" and roar `AudioMixerMono3_b77a` and `_b525` Gain 1 and 2 (set both mixers); boom and rattle `AudioMixerMono2_117d` Gain 0 and 1.

## 7. Files (bytes on disk, `Game/Plugins/JamAssets/Content/Audio/`)

| File | Before | After |
|---|---:|---:|
| `MS_check.uasset` | 171,942 | 264,428 |
| `MS_knockdown.uasset` | 135,832 | 256,988 |
| `MS_board.uasset` | 191,449 | 245,386 |
| 20 new `SFX/SFX_*.uasset` | - | 682,763 |
| 6 new `Crowd/CRD_*.uasset` | - | 1,250,438 |

- The repository grows by about **2.2 MB** (1,933,201 bytes of waves, 267,579 of MetaSounds). The 15 referenced waves are 1,137,756 bytes; the 11 unreferenced are 795,445.
- Cooked estimate by AUD's rule of thumb: 5.1 s of referenced mono and 7.1 s of referenced stereo, about 0.13 MB. Not measured: no cook was run.
- Per file: the ledger rows below carry each file's bytes.

Ledger rows. **Replace** the existing rows of `MS_check`, `MS_knockdown`, `MS_board` and `SFX_BodyCheck_02` (its note changes); the 26 waves are new. `shipped` is `y` only for what a MetaSound references.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body check impact, thump and clack; mono 48 kHz 0.3 s, -22 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_check Hits (random, trim 1.0) (19928 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body check impact, deeper; mono 48 kHz 0.32 s, -23.9 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_check Hits (random, trim 1.245) (18581 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Hit_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body check impact, short and bright; mono 48 kHz 0.365 s, -26 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_check Hits (random, trim 1.585) (23488 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_HitBig_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | big hit, low end and rattle; mono 48 kHz 0.635 s, -16.6 LUFS, -3 dBTP, Bink q40, SC_SFX; MS_check HitBig (Strength 0.8 and over) (53335 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_HitBig_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | alternate big hit, deepest; mono 48 kHz 0.575 s, -19.4 LUFS, -3 dBTP, Bink q40, SC_SFX; not referenced (35983 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body lands on ice, slap-thud; mono 48 kHz 0.315 s, -21.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_knockdown Lands (random, trim 1.0), 0.17 s after the trigger (22884 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body lands on ice, brighter slap; mono 48 kHz 0.3 s, -21 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_knockdown Lands (random, trim 0.933) (24778 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Land_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | body lands on ice, dull and short; mono 48 kHz 0.3 s, -19.7 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_knockdown Lands (random, trim 0.804) (19866 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_a.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | padded body sliding on ice, scrape and hiss; mono 48 kHz 0.835 s, -25.3 LUFS, -14 dBTP, Bink q40, SC_SFX; MS_knockdown Slide, from 0.20 s (60555 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_b.uasset | audio | 0.1 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | alternate slide, hissier; mono 48 kHz 1 s, -25.3 LUFS, -12.3 dBTP, Bink q40, SC_SFX; not referenced (84993 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | skater into the boards, boom; mono 48 kHz 0.6 s, -20.7 LUFS, -3 dBTP, Bink q40, SC_SFX; MS_board BoardsBody (Variant 1) (28322 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | alternate boards boom; mono 48 kHz 0.645 s, -21.2 LUFS, -3 dBTP, Bink q40, SC_SFX; not referenced (26757 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BoardsBody_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | alternate boards boom, deepest and longest; mono 48 kHz 0.695 s, -22.9 LUFS, -3 dBTP, Bink q40, SC_SFX; not referenced (30092 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | standing bump, thud plus skate scrape, layered from two generations; mono 48 kHz 0.45 s, -19.4 LUFS, -3 dBTP, Bink q40, SC_SFX; NOT wired (no bump event yet) (34275 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | standing bump, soft thud; mono 48 kHz 0.3 s, -19.3 LUFS, -3 dBTP, Bink q40, SC_SFX; not wired (no bump event yet) (17703 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Bump_c.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | standing bump, brighter thud; mono 48 kHz 0.3 s, -20.4 LUFS, -3 dBTP, Bink q40, SC_SFX; not wired (no bump event yet) (18586 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GetUp_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | get-up, skate scuffs then a scrape; mono 48 kHz 0.575 s, -28 LUFS, -14 dBTP, Bink q40, SC_SFX; MS_knockdown GetUp, at 1.05 s (40727 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GetUp_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | n | alternate get-up, one skate scrape; mono 48 kHz 0.515 s, -28 LUFS, -15.1 dBTP, Bink q40, SC_SFX; not referenced (36863 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GlassRattle_a.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | glass rattle layer for the boards; mono 48 kHz 0.585 s, -22.6 LUFS, -3 dBTP, Bink q40, SC_SFX; MS_board GlassRattle (Variant 1), 0.03 s after the boom, 6 dB under it (47688 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_GlassRattle_b.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | alternate glass rattle, duller wobble; mono 48 kHz 0.795 s, -20.5 LUFS, -3 dBTP, Bink q40, SC_SFX; not referenced (37359 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | crowd ooh at a hit; stereo 48 kHz 1.79 s, -20.3 LUFS, -11.7 dBTP, Bink q40, SC_Crowd; in MS_knockdown CrowdOohs (random, Strength 0.5 to under 0.8), from 0.05 s (193644 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | crowd ooh at a hit, sudden; stereo 48 kHz 1.515 s, -20.2 LUFS, -9.7 dBTP, Bink q40, SC_Crowd; in MS_knockdown CrowdOohs (random) (167707 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Hit_c.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU4 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU4-report.md) | 2026-11-13 | B | y | crowd ooh at a hit, slower fall; stereo 48 kHz 1.8 s, -20.1 LUFS, -11 dBTP, Bink q40, SC_Crowd; in MS_knockdown CrowdOohs (random) (205363 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_a.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | y | crowd roar on a knockdown; stereo 48 kHz 2 s, -18 LUFS, -10.7 dBTP, Bink q40, SC_Crowd; MS_knockdown CrowdRoar (Strength 0.8 and over), from 0.05 s (210890 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_b.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | alternate crowd roar, longer hold; stereo 48 kHz 2 s, -18.2 LUFS, -12.6 dBTP, Bink q40, SC_Crowd; not referenced (237610 bytes) |
| Game/Plugins/JamAssets/Content/Audio/Crowd/CRD_Roar_c.uasset | audio | 0.2 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU5 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/tackle/ledger.jsonl (prompts; status/AU5-report.md) | 2026-11-13 | B | n | alternate crowd roar, flagged tone-ish; stereo 48 kHz 2 s, -18.2 LUFS, -10.6 dBTP, Bink q40, SC_Crowd; not referenced (235224 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, mono; body check: one of three hits at random (Hits, equal-loudness trims in HitTrims, no repeat), the big hit (HitBig) at Strength 0.8 and over, level x max(0.5, Strength); SFX_BodyCheck_02 removed, old synth layer muted; -27.5 LUFS at Strength 0.6, -22.3 at 1.0 (lane AU6) (264428 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, now stereo; the fall after a check: landing (Lands, random) at 0.17 s, slide (Slide) from 0.20 s, get-up scrape (GetUp) at 1.05 s, crowd from 0.05 s: ooh (CrowdOohs, random) at Strength 0.5 to under 0.8, roar (CrowdRoar) at 0.8 and over, none under 0.5; old noise sweep removed; -28.6 LUFS at Strength 0.6, -22.6 at 1.0 (lane AU6) (256988 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_board.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, mono; Variant 0 (puck): SFX_PuckBoards_01 over the synth, unchanged; Variant 1 (skater into the boards): boom (BoardsBody) with the glass rattle (GlassRattle) 0.03 s later and 6 dB under, level x max(0.5, Strength); new float input Variant (lane AU6) (245386 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BodyCheck_02.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane N3 make_ready.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/ledger.jsonl (prompts; status/B-audio-au1.md) | 2026-11-12 | B | y | mono 48 kHz, Bink q40; no longer in MS_check (lane AU6); still referenced by MS_shot_blocked and MS_save (49828 bytes) |

Licence wording as in the AU2 to AU5 rows. AU1's caveats stand: the output may not be unique, it must be reviewed before distribution (nobody has heard it), and the organisers' rule on AI-generated audio is still to be confirmed.

## 8. Asks for device A

1. **A `bump` event, if the standing bump should sound.** Today nothing reaches a sound.
   - How I know: `JamSim.cpp` lines 1479 to 1485 only set the read-outs `bBumpThisFrame` and `BumpDir` (no `Emit`); `JamPawns.cpp` lines 334 to 338 call the figure's `PlayHitReaction` and log `JAMVIS hit`; `JamEventBus.cpp` has no such event. The second editor session logged 192 `JAMVIS hit` lines against 44 checks (the first: 149 against 23).
   - Ask: a new event `bump` (`JamEvents::Bump`, in `GetKnownEvents`, sound-only in `IsSoundOnlyEvent`), emitted where the sim sets `bBumpThisFrame`, **once per contact, not once per skater** (both skaters are flagged on the same contact).
   - Payload: Who = one of the two (the human if it is involved, else the lower index); Where = the contact point; Strength = closing speed / `game.skater.speed`, clamped to 1; Kind none; Dir = the contact normal. Cosmetic, not hashed, like the skater board read-out.
   - Then B builds `MS_bump` on `SFX_Bump_a` (imported, alternates `_b`, `_c`): ten minutes, no import.
   - Rate: bumps are four to six times as frequent as checks, so the event may want a higher threshold than the animation's 3 m/s.
2. **Nothing else is needed for this lane.** Same asset names; the binder already sends `Variant` 1 for a skater's board hit.
3. For A to know: `MS_knockdown` is now stereo and 2.15 s long (was 0.7 s), `MS_check` 0.75 s (was 1.45 s), `MS_board` 0.9 s (was 0.4 s). One knockdown holds a voice for 2.15 s.

## 9. What I could not verify

- **Nobody has heard this.** That the hits read as body checks, the landing as ice, the roar as a crowd and not as noise; that hit, landing and crowd read as one event; that three takes in rotation sound like variety and not like three different games.
- **Whether the slide and the get-up can be heard in the mix at all.** They are as briefed (8 and 5 dB under their delivered relation). By arithmetic from the gains the get-up sits near -31 LUFS and the slide between -31 and -37, under bed and music measured at about -23.5. The matched filter finds the get-up in 13 of 13; an ear may not.
- **The "ooh" in a real match.** The drill's knockdowns were at Strength 0.88 or more, or under 0.5.
- **Sound against picture.** The sound's times are in seconds; the figure's fall and get-up run in sim frames (90). The match recorded here ran near 58 ticks a second. Below 60 the picture runs slower than the sound, so the get-up scrape comes early. I measured no frame rate, as told, and did not compare sound to picture.
- **The 30 ms start offset** between `MS_check` and `MS_knockdown`: seen once in 18 joint events. I do not know its cause or its rate in a package.
- **The loudness estimate "0.2 dB from this lane"** is arithmetic on energy with rendered templates, not a second recording with the old assets.
- **Finding 1 on AU3's four MetaSounds** is an inference from how mono and stereo sources measured here; I did not record those four.
- **Cooked sizes, a packaged build, shared-state no-repeat outside PIE.** No cook was run.
- **The `ScriptHuman 1` plus `Drill 6` combination**: the brief describes it as "the human checks"; `TACKLING-2.md` describes drill 6 as "the human gets checked". The run gave 13 knockdowns either way.

## 10. Decisions for the lead

1. **The mix is at -20.5 to -20.8 LUFS with under 1 dB of peak room, and this lane is not the cause.** Shots, puck board hits, `lift` / `poke` at Strength 1 and commentary pile up on the same frames. A limiter on the master, or a trim of those sounds, would be a lane of its own. If you want this lane's share down anyway: crowd 0.8 to 0.5 is the largest single step (the roar carries most of the chain's loudness).
2. **How often the roar plays.** Threshold 0.8 as you set it. In today's earlier logs 101 of 230 knockdowns were at 0.8 or more; in the drill 11 of 13; in the plain match 1 of 5. If the roar should be rare, raise 0.799 in `Subtract_e569` (`MS_knockdown`) and `Subtract_cfdd` (`MS_check`) to, say, 0.949.
3. **The skater boom is frequent: 30 in 92 s of plain play**, one every three seconds, each a boom with a glass rattle. It is no louder than the puck crack it replaces, but it is longer and deeper. Options: leave it; lower `AudioMixerMono2_117d` (1.4 and 0.7); or ask A for a higher threshold than 4 m/s.
4. **Crowd level 0.8** is my choice: between AU4's preview (0.5 against a hit at 1.0) and the delivered level. By the gains (not measured side by side) the roar at Strength 1 sits about 5.5 LU under a goal cheer and 3.5 LU over the save "ooh".
5. **Slide and get-up level**: probably masked (section 9). Gains 1 and 2 of `AudioMixerMono3_f522`; +6 dB would be 1.4 and 1.96.
6. **The get-up does not scale with Strength** (the same motion after any hit); the slide and the landing do. Say if it should.
7. **AU3's 0.7071** (finding 1): `MS_goal`, `MS_win`, `MS_save`, `MS_period_end` may carry their old synth layers 3 dB hot. Worth one measuring pass.
8. **The 11 unreferenced waves (0.8 MB)**: keep for the swap, or delete after the operator picks.
9. **`MS_bump`** is not built, as briefed. It can be built now on `SFX_Bump_a` so that A only has to emit the event.
10. **The old synth layer of `MS_check`** is muted, not deleted (Gain 0 of `AudioMixerMono3_9829`; it was 0.35). Delete it if you want the graph clean.

## Recordings and scripts (for the operator's ear)

All in `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/`. The trigger clips start 0.2 s before the trigger and have no music or bed.

- `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/knockdown_s06.wav` (the "ooh", `MS_knockdown` alone)
- `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/knockdown_s10.wav` (the roar, `MS_knockdown` alone)
- `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/check_s06.wav` (`Hit_c`; `check_s06_hit_a.wav` and `check_s06_hit_b.wav` are the other two takes)
- `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/match_60s.wav` (the drill match, full mix)
- **Start here:** `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/chain_s06.wav` and `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au6/chain_s10.wav`: check and knockdown together, which is what the game plays.
- Also there: `check_s03.wav`, `check_s08.wav`, `check_s10.wav`, `knockdown_s03.wav`, `knockdown_s05.wav`, `knockdown_s079.wav`, `knockdown_s08.wav`, `chain_s03.wav`, `chain_s08.wav`, `board_puck_s03/06/10.wav`, `board_skater_s03/06/10.wav`, `match_raw.wav` (75 s), `match_nodrill.wav` (92 s), `trig.wav` (the whole trigger test), `base.wav` (the old sounds, same triggers).
- Scripts: `imp.py` (import), `check.js`, `knock.js`, `board.js`, `gains.js`, `lib.js` (graph edits), `rec_trig.py`, `rec_match.py` (recording), `ana.py`, `tl.py`, `lr.py`, `manal.py`, `mparts.py`, `stem.py`, `pan.py` (measuring). `backup/` holds the three MetaSounds as they were at 13:48.
