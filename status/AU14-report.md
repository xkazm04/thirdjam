# Lane AU14 report: sound in an engine without a GPU, and the fourteen sounds of the three new sets (device B), 2026-11-14 03:05

- Tree `C:/Users/kazda/jam/wt/b-aud` (branch `lane/b-aud` at 6b1636e0), built first (570 s, pass). Editor on port 8006 with `-nullrhi -unattended` from 02:57 to 03:03, left through `ExitEditor` (pid 15268 gone, log closed cleanly). No commit, no push, no cook, no service call. The other editors were not touched.
- **Nobody on this device has heard any of this.** Every statement is a measurement of recordings, or a line of the log.
- Written: 30 waves under `Audio/SFX/` and 14 MetaSounds under `Audio/`, nothing else. No map saved, no dirty package at exit, console variables as found, "allow background audio" and "use less CPU in background" put back.

## Verdicts

| Job | Verdict | Evidence |
|---|---|---|
| 0. Sound without a GPU | **YES, all of it works** | The audio device starts on the real output, the play session mixes, a submix recording writes a WAV with the right content at the right level. Sound lanes can run beside the picture lane. |
| 1. Import | **PASS** | 30 waves (both takes of the fifteen), mono, Bink quality 40, `SC_SFX`; `Dash_Loop_*` looping. |
| 2. Fourteen MetaSounds | **PASS** | All fourteen build, save and play by name with no input set. The loop stops cleanly three ways; the skid holds and releases. |
| 3. The mix rule | **PASS** | None is louder than a body hit at 0.6 (-29.4 LUFS). Bonk and cannon sit level with it (-29.5 to -29.6); the rest 1.3 to 3.6 LU under; the dash loop is -38.1 LUFS, 10.6 LU under the music. |
| 4. Audition | **DONE** | `au14_audition.wav` with a cue sheet and one clip per sound, recorded headless. |

## 0. Does sound work without a GPU? Yes.

Log: `C:/Users/kazda/jam/wt/b-aud/Game/logs/editor-20261114-025700-0.log`.

| Step | Result | Evidence |
|---|---|---|
| The null RHI is in use | yes | line 915 `InternalLoadLibrary: 'NullDrv'`; line 919 `rhiname="Null"`; line 922 `gpu=""`; **0 lines of `LogD3D12RHI`**, counted before the first call and again at the end |
| The audio device starts | yes | line 1712 `Creating Audio Device: Id: 1, Scope: Shared`; line 1733 `Initializing audio mixer using platform API: 'WASAPIMixer'`; line 1739 `Using Audio Hardware Device Speakers (Cirrus Logic XU ...)`; line 1745 `Audio device 1 render mode: Realtime` |
| The play session gets its own device | yes | line 2385 `Creating Audio Device: Id: 2, Scope: Unique`; line 2420 `Audio Device (ID: 2) registered with world 'Rink'` |
| The match's sounds play | yes | `MS_music_match`, `MS_crowd_bed` and `MS_skate_loop` were playing in the session |
| A submix recording writes a WAV | yes | `AudioMixerLibrary.start_recording_output` / `stop_recording_output`, as in AU11 and AU12: 10.0 s, -26.5 LUFS (the music and bed of the running match) |
| The content is right | yes | `MS_check` fired at 4.00 s of that recording: the hit take is found at 4.01 s at -5.7 dB against its file; the graph says -5.6 |
| Tick rate | 54.1, 59.3 and 60.0 ticks a second in the three sessions | counted over 4 s each |

- **Nothing warned:** no `LogAudio` or `LogAudioMixer` warning, no MetaSound error, no `JAMFX missing` line in the whole log. Three play sessions.
- **"Allow background audio" was on for the tests**, as in the GPU editor, where a recording is silent without it. A headless editor is never in the foreground, so I expect it to be needed; I did not test with it off.
- **`Slate.bAllowThrottling` was not needed here.** It stayed at 1 and the play world ticked. (The figure lane's trap did not show in this session.)
- **The first play session took 53 s to start** in this never-run tree, and the MCP call reported "Timed out waiting for PIE to start" although the session had started. Later starts took 0.3 s. A lane should check for the play world before deciding the start failed.
- **It plays on the machine's speakers.** The headless editor opens the real output device, so whatever a sound lane triggers is audible in the room. Mute the output device, not the engine, if that matters at night.

## 1. Import

- From `C:/Users/kazda/jam/audio-lab/sets/`: `Dive_Block`, `Dive_Block_Hit`, `Slide_Tackle`, `Dash_Start`, `Dash_Loop`, `Dash_Stride`, `Dash_Skid`, `Dash_Wipeout`, `Boost_Take`, `Giant_Sweep`, `Giant_Poke`, `Mallet_Swing`, `Mallet_Bonk`, `Cannon_Fire`, `Cannon_Kick`, takes `_a` and `_b` of each, as `SFX_<name>`.
- 14 are referenced (the `_a` take of each sound that has a MetaSound). 16 are not: the 14 alternates and both `Dash_Stride` takes (the loop file is used instead of single strides).

## 2. The fourteen MetaSounds and the wiring table

All are mono, `SC_SFX`, no attenuation, and play with nothing set. Inputs common to all: `Strength` (default 1; the level is x max(0.5, Strength)), `Variant` (unused), `FxTrim` (0.70), and one wave input named in the table. A sound that is a hit on a body carries the hit factor inside its gain (its level is set against the body check directly), so one `FxTrim` still moves it with the rest.

| MetaSound | Fire it on | Wave input, gain | Level alone (LUFS / peak dBFS) | Ends after |
|---|---|---|---|---|
| `MS_dive_block` | `Dive_Block` frame 0 (the landing is 0.15 s into the file: frame 9) | `DiveBlock` = `SFX_Dive_Block_a`, 1.0 | -30.7 / -15.2 | 0.72 s |
| `MS_dive_block_hit` | the frame the puck strikes the lying blocker, no delay (the jolt peaks 2 frames later) | `DiveBlockHit` = `SFX_Dive_Block_Hit_a`, 2.0 | -31.4 / -8.9 | 0.35 s |
| `MS_slide_tackle` | `Slide_Tackle` frame 0 (the clatter is at 0.17 s: the contact frame 10) | `SlideTackle` = `SFX_Slide_Tackle_a`, 1.04 | -31.0 / -14.9 | 0.68 s |
| `MS_dash_start` | `Dash_Start` frame 0 | `DashStart` = `SFX_Dash_Start_a`, 0.80 | -33.0 / -17.4 | 0.4 s |
| `MS_dash_loop` | `Dash_Start` frame 10 (0.167 s), and keep the component | `DashLoop` = `SFX_Dash_Loop_a`, 0.49 | -38.1 / -27.6 | when stopped |
| `MS_dash_skid` | `Dash_Skid` frame 2 (the spray starts) | `DashSkid` = `SFX_Dash_Skid_a`, 0.26 | default -36.2 / -26.9; held 1 s -31.6 / -26.1 | 0.3 s after the release |
| `MS_dash_wipeout` | the wipeout take's frame 13 (belly on the ice) | `DashWipeout` = `SFX_Dash_Wipeout_a`, 1.0 | -31.1 / -14.7 | 0.84 s |
| `MS_boost_take` | the boost pickup take's frame 0 | `BoostTake` = `SFX_Boost_Take_a`, 0.52 | -33.0 / -19.7 | 0.4 s |
| `MS_giant_sweep` | `Giant_Sweep` frame 0 (the clack is at 0.20 s: frames 10 to 14) | `GiantSweep` = `SFX_Giant_Sweep_a`, 0.64 | -32.7 / -19.1 | 0.6 s |
| `MS_giant_poke` | `Giant_Poke` frame 0 (the tick is at 0.133 s: frame 8) | `GiantPoke` = `SFX_Giant_Poke_a`, 0.60 | -32.9 / -19.4 | 0.39 s |
| `MS_mallet_swing` | `Mallet_Swing` **frame 4** (its loudest point then meets the contact frame 15); the same for `Mallet_Miss` | `MalletSwing` = `SFX_Mallet_Swing_a`, 0.49 | -32.9 / -21.5 | 0.51 s |
| `MS_mallet_bonk` | the swing's frame 15, only when it hits | `MalletBonk` = `SFX_Mallet_Bonk_a`, 1.57 | -29.5 / -11.1 | 0.5 s |
| `MS_cannon_fire` | `Cannon_Fire` frame 8 (the puck leaves) | `CannonFire` = `SFX_Cannon_Fire_a`, 1.60 | -29.6 / -11.3 | 0.6 s |
| `MS_cannon_kick` | `Cannon_Kick` frame 8, **in place of** the fire sound | `CannonKick` = `SFX_Cannon_Kick_a`, 0.93 | -29.5 / -15.5 | 0.9 s |

**The dash loop: how the caller stops it.** Spawn it and keep the audio component (as the skate loop is kept). Any of three ends it, each tried in the recording:
- `FadeOut(0.05, 0)` on the component when the dash ends: the sound is gone 0.02 s after the 3.00 s it was asked to run.
- `Stop()` on the component.
- If the caller forgets, the input `MaxSeconds` (default 6) ends it from inside: set to 1.0 it measured 1.00 s.
- `PitchShift` (semitones, default 0) follows the take's play rate: rate 0.9 is -1.8, rate 1.3 is +4.5 (12 x log2 of the rate).

**The dash skid: its inputs.** The body of the file (0.1 to 0.6 s) loops, then the tail plays from 0.68 s.
- `HoldSeconds` (default 0.2): how long the body runs before the tail. With nothing set it is 0.2 s of body and the tail, 0.28 s in all, which fits the take without a hold.
- `Release` (trigger): ends the body at once and starts the tail. For a held skid set `HoldSeconds` long (say 5) and send `Release` when the take leaves its hold (`SetTriggerParameter("Release")`). Tried: released at 0.6 s, the sound was 0.66 s long.
- With `HoldSeconds` 1.0 and no trigger it measured 1.08 s.

**Capped gain.** `Dive_Block_Hit_a` is a short, peaky file; AU13's loudness factor alone would pass full scale, so its gain stops at 2.0 (gain x trim 1.4). It ends 2 LU under a body hit and has the highest peak of the set (-8.9 dBFS). `Cannon_Fire_a` needed no cap at this level (gain x trim 1.12).

## 3. The mix rule

Reference in the same recording: `MS_check` at Strength 0.6, -29.4 LUFS.

| Group | Sounds | Level against the body hit |
|---|---|---|
| Level with it | mallet bonk -29.5, cannon fire -29.6, cannon kick -29.5 | 0.1 to 0.2 LU under |
| Hits on a body | dive block -30.7, slide tackle -31.0, wipeout -31.1, dive block hit -31.4 | 1.3 to 2.0 LU under |
| Movement and swings | giant sweep -32.7, giant poke -32.9, mallet swing -32.9, dash start -33.0, boost take -33.0, skid (held) -31.6 | 2.2 to 3.6 LU under |
| The loop | dash loop -38.1, peak -27.6 dBFS | 8.7 LU under; **10.6 LU under the music** (-27.5 LUFS on its own, AU11) |

- Nothing is louder than a body hit.
- `Strength` works where a caller sets it: the bonk at 0.3 plays at the 0.5 floor, 6.0 dB down (-35.5 LUFS).
- All levels are single sounds alone. How they add up in a match depends on how often the figure fires them; nothing fires them yet.

## 4. The audition

`C:/Users/kazda/jam/audio-lab/sets/engine/au14_audition.wav` (31 s), `au14_cue_sheet.txt` beside it, and one clip per line of the sheet (`au14_<sound>.wav`, 19 clips: the fourteen sounds, the loop's two endings, the skid's three forms, the bonk at Strength 0.3, and the body hit for comparison). Each was triggered by spawning the MetaSound by name in a play session with the music, bed and skate loop muted.

## 5. Not verified

- **Anything by ear**, including whether the loop's 0.4 s cycle reads as skating or as a machine, and whether the skid's jump from body to tail clicks.
- **The frames.** The table's frames are AU13's, taken from the takes' descriptions; nothing was played against a figure.
- **Through a Blueprint or the binder.** I spawned each sound from Python. The figure's `PlaySound2D` call and the binder's spawn use the same asset, but neither was run.
- **A match with these sounds**: nothing fires them yet, so no share of the mix.
- **Headless with "allow background audio" off**, and a headless run with no audio device at all (a machine without speakers).
- **A cook.** None was run in this tree.

## 6. For the figure lane

- Wire from the table in section 2: one `PlaySound2D(<MetaSound>)` at the frame given, as `MS_brake` is played. No input needs setting.
- **Two sounds need more than a one-shot call:**
  - `MS_dash_loop`: `SpawnSound2D`, keep the component, `FadeOut(0.05, 0)` when the dash ends. Set `PitchShift` if the loop take's rate is not 1.
  - `MS_dash_skid`: for a held skid, `SpawnSound2D`, `SetFloatParameter("HoldSeconds", 5)`, then `SetTriggerParameter("Release")` when the hold ends. For the plain skid, `PlaySound2D` is enough.
- `MS_cannon_kick` replaces `MS_cannon_fire` on the kick take: do not fire both.
- `MS_mallet_bonk` only on a hit; `MS_mallet_swing` on every swing, at frame 4.
- The victim of a slide tackle or a bonk still gets the sounds it has today (`MS_check`, `MS_knockdown`) if the other device emits them; then the bonk and the hit will stack. See "For the other device".

## 7. For the other device

Bus events that would replace the Blueprint calls. With the name rule the binder needs no code per sound: an event named as the asset plays it, with `Strength` and `Variant` as today.

| Event | Who, Where | Strength | Replaces |
|---|---|---|---|
| `dive_block` | the blocker, its position | 1 | the call at `Dive_Block` frame 0 |
| `dive_block_hit` | the blocker, the puck | puck speed over the maximum | the call on the strike |
| `slide_tackle` | the tackler, its position | closing speed over skater speed | the call at frame 0 |
| `dash_start` | the skater | 1 | the call at `Dash_Start` frame 0 |
| `dash_skid`, `dash_wipeout` | the skater | 1 | the two endings |
| `boost_take` | the taker | 1 | the call on the pickup (the pickup chime plays there today) |
| `giant_sweep`, `giant_poke`, `mallet_swing`, `mallet_bonk`, `cannon_fire`, `cannon_kick` | the user (the bonk: the victim) | 1 | the weapon calls |

- **The loop cannot be a bus event**: it needs a handle to stop. It stays with the figure, or the pawn owns it as it owns the skate loop.
- **Say whether a bonk or a slide tackle also emits `check` and `knockdown`.** If it does, the victim's hit and landing play on top of the bonk; then lower the bonk, or send the `check` with a `Variant` that `MS_check` can mute.
- Events fire on the sim step; the frames in section 2 are the takes'. `mallet_swing` wants 4 frames of delay and `dash_wipeout` 13: either the event comes that much later, or the MetaSound gets a delay input (a small change on B's side).

## 8. Decisions for the operator

1. **The levels are a first setting by rule, not by ear:** bonk and cannon as loud as a body hit, other hits about 1.5 LU under, movement about 3.5 LU under, the dash loop far under the music.
2. **The dive block hit is the sharpest sound of the set** (peak -8.9 dBFS at -31.4 LUFS). If it bites, `Dive_Block_Hit_b` is imported: a fuller take that needs a third of the gain.
3. **The headless editor plays through the machine's speakers** (section 0).

## 9. The next three steps, each with its proving number

1. **The sets heard on the figure.** 0 of 14 are fired by a take today. After the figure lane wires them: 14 of 14 found in a recording, each within one frame (17 ms) of the table's frame.
2. **The mix with the sets in play.** One 100 s match using the dash and a weapon: music + commentary + crowd still at 75 % or more (81 to 84 % today without them).
3. **Takes in rotation where a sound repeats.** 16 imported takes are unused. For the block hit and the dash: no two in a row the same, counted over 20 firings.

## 10. Files and ledger rows

- 30 waves, 977,269 bytes; 14 MetaSounds, 1,656,528 bytes. All new; nothing existing was changed.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au14/` (`imp.py`, `build14.js`, `rec_aud.py`, `an.py`, `job0.py`: the headless recording test).

44 full rows, all new:

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_a.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dive block, a padded body landing flat and sliding; mono 48 kHz 0.615 s, -22 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_dive_block (39754 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_b.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dive block, a padded body landing flat and sliding; mono 48 kHz 0.7 s, -27.4 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (41805 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_Hit_a.uasset | audio | 0.01 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | a puck striking the lying blocker; mono 48 kHz 0.25 s, -28.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_dive_block_hit (9818 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dive_Block_Hit_b.uasset | audio | 0.01 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | a puck striking the lying blocker; mono 48 kHz 0.295 s, -18.5 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (12835 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_Tackle_a.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | slide tackle, slide and clatter; assembled from SlideTackle_1 and Clatter_2; mono 48 kHz 0.575 s, -22.3 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_slide_tackle (42634 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Slide_Tackle_b.uasset | audio | 0.05 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | slide tackle, slide and clatter; mono 48 kHz 0.575 s, -16.1 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (46782 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Start_a.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash start, two blade digs; mono 48 kHz 0.3 s, -21.1 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_dash_start (25767 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Start_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash start, two blade digs; mono 48 kHz 0.25 s, -18.3 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (20169 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Loop_a.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash stride loop, 0.4 s cycle; assembled from DashStride_3; mono 48 kHz 0.4 s, -22.7 LUFS, -9 dBTP, looping, Bink q40, SC_SFX; in MS_dash_loop (33189 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Loop_b.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash stride loop, 0.4 s cycle; assembled from DashStride_3 and DashStride_1; mono 48 kHz 0.4 s, -21.1 LUFS, -9 dBTP, looping, Bink q40, SC_SFX; alternate, not referenced (32039 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Stride_a.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | a single dash stride; mono 48 kHz 0.3 s, -26 LUFS, -9 dBTP, Bink q40, SC_SFX; single stride, not referenced (MS_dash_loop loops the assembled cycle) (23797 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Stride_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | a single dash stride; mono 48 kHz 0.3 s, -21.6 LUFS, -9 dBTP, Bink q40, SC_SFX; single stride, not referenced (MS_dash_loop loops the assembled cycle) (20658 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Skid_a.uasset | audio | 0.07 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash skid, long two-skate skid; mono 48 kHz 0.925 s, -12.5 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_dash_skid (71665 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Skid_b.uasset | audio | 0.08 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash skid, long two-skate skid; mono 48 kHz 0.95 s, -17.7 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (78981 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Wipeout_a.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | dash wipeout, slap, bounce, slide; mono 48 kHz 0.735 s, -21.2 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_dash_wipeout (41890 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Dash_Wipeout_b.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | dash wipeout, slap, bounce, slide; mono 48 kHz 0.7 s, -24.7 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (28155 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Boost_Take_a.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | boost pickup, glove clap or blade tick; mono 48 kHz 0.3 s, -16.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_boost_take (23690 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Boost_Take_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | boost pickup, glove clap or blade tick; mono 48 kHz 0.3 s, -20.3 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (24863 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Sweep_a.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | giant stick sweep, whoosh and clack; assembled from GiantSweep_3 and Clatter_1; mono 48 kHz 0.5 s, -20.2 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_giant_sweep (37358 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Sweep_b.uasset | audio | 0.04 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | giant stick sweep, whoosh and clack; mono 48 kHz 0.5 s, -16.3 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (40118 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Poke_a.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | giant stick poke, whoosh and tick; assembled from MalletSwing_2 and GiantPoke_3; mono 48 kHz 0.288 s, -19.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_giant_poke (23266 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Giant_Poke_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | giant stick poke, whoosh and tick; mono 48 kHz 0.3 s, -21.8 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (18796 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Swing_a.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | mallet swing, a rising whoosh; mono 48 kHz 0.41 s, -17.8 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_mallet_swing (32913 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Swing_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | mallet swing, a rising whoosh; mono 48 kHz 0.3 s, -17.6 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (19881 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Bonk_a.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | mallet bonk, thump and woody knock; assembled from MalletBonk_2 and MalletBonk_5; mono 48 kHz 0.4 s, -22.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_mallet_bonk (20692 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Mallet_Bonk_b.uasset | audio | 0.02 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | mallet bonk, thump and woody knock; mono 48 kHz 0.4 s, -17.8 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (19150 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Fire_a.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | toy puck cannon firing; assembled from GiantSweep_2 and CannonFire_2; mono 48 kHz 0.5 s, -23.1 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_cannon_fire (27079 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Fire_b.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | toy puck cannon firing; mono 48 kHz 0.5 s, -25 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (26448 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Kick_a.uasset | audio | 0.05 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | y | toy puck cannon firing with the carrier sitting down; assembled from GiantSweep_2 and CannonKick_2; mono 48 kHz 0.8 s, -18.6 LUFS, -3 dBTP, Bink q40, SC_SFX; in MS_cannon_kick (45473 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_Cannon_Kick_b.uasset | audio | 0.05 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane AU13 proc.mjs/select.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/sets/ledger.jsonl (prompts; status/AU13-report.md) | 2026-11-14 | B | n | toy puck cannon firing with the carrier sitting down; mono 48 kHz 0.8 s, -17.4 LUFS, -3 dBTP, Bink q40, SC_SFX; alternate, not referenced (47604 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dive_block.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the dive block (drop and slide): SFX_Dive_Block_a (input DiveBlock) at 1.0; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115178 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dive_block_hit.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; a puck striking the lying blocker: SFX_Dive_Block_Hit_a (input DiveBlockHit) at 2.0 (capped: the loudness factor alone would pass full scale); level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115212 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_slide_tackle.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the slide tackle: SFX_Slide_Tackle_a (input SlideTackle) at 1.04; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115196 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dash_start.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the dash start: SFX_Dash_Start_a (input DashStart) at 0.80; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115230 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dash_loop.uasset | audio | 0.13 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the dash stride loop: SFX_Dash_Loop_a (input DashLoop) looping at 0.49 until the caller stops it (FadeOut 0.05 s) or MaxSeconds (6) passes; PitchShift in semitones; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (128125 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dash_skid.uasset | audio | 0.15 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the dash skid: SFX_Dash_Skid_a (input DashSkid) at 0.26: the body loops 0.1 to 0.6 s of the file for HoldSeconds (0.2) or until the trigger Release, then the tail from 0.68 s; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (145831 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_dash_wipeout.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the dash wipeout: SFX_Dash_Wipeout_a (input DashWipeout) at 1.0; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115248 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_boost_take.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the boost pickup: SFX_Boost_Take_a (input BoostTake) at 0.52; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115230 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_giant_sweep.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the giant stick sweep: SFX_Giant_Sweep_a (input GiantSweep) at 0.64; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115187 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_giant_poke.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the giant stick poke: SFX_Giant_Poke_a (input GiantPoke) at 0.60; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115178 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_mallet_swing.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the mallet swing: SFX_Mallet_Swing_a (input MalletSwing) at 0.49; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115196 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_mallet_bonk.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the mallet bonk: SFX_Mallet_Bonk_a (input MalletBonk) at 1.57; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115239 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_cannon_fire.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the toy cannon firing: SFX_Cannon_Fire_a (input CannonFire) at 1.60; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115239 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_cannon_kick.uasset | audio | 0.12 | original | UE 5.9 MetaSounds via MCP/Python (editor without a GPU) |  |  | 2026-11-14 | B | y | MetaSound source, mono, SC_SFX; the toy cannon firing with the carrier sitting down: SFX_Cannon_Kick_a (input CannonKick) at 0.93; level x max(0.5, Strength), Strength default 1, Variant unused, FxTrim 0.70; fired by name, no bus event yet (lane AU14) (115239 bytes) |
