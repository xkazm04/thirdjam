# Lane V4 report: the streak to 9 px, and the missed cross-check's voice (device B), 2026-11-15 12:40

Tree `C:\Users\kazda\jam\wt\b-aud` (branch `lane/b-aud`, at dev-b 9529103b; no source change since S4's build, so no rebuild). Port 8006.

**Housekeeping**
- **GPU editor**: pid 38324, from 12:23 to 12:32.
  - GPU-FREE.txt said "free from S4 at 11:45". I wrote "taken by V4" and gave it back as "free from V4 at 12:32".
  - The editor was left through `ExitEditor` and the process is gone.
- **Headless editor** (`-nullrhi -unattended`): pid 4492, from 12:35 to 12:37, for the audio import. Left through `ExitEditor`; the process is gone.
- **Another editor was running that is not mine**: an `UnrealEditor` pid 15860 (with `UnrealTraceServer` 35892), started 12:23:03. I did not touch it.
- **Settings are as I found them**:
  - `enable_game_sound` false;
  - `jam.ScriptHuman` 0;
  - `r.Editor.Viewport.OverridePIEScreenPercentage` 1;
  - `jam.Cut hockey.regular`.
- No dirty package at either exit.
- No commit, push, cook or build.
- Paid service: ElevenLabs only, 33 credits (section 2b).
- The API key was read inside `eleven.mjs` and sent only in its request header. It was never printed, logged or copied.
- Raw files are outside git, in `C:/Users/kazda/jam/audio-lab/cross-miss/`:
  - `mp3/` (the takes), `wav48/` and `wav/` (processed), `wav24/` (what was imported);
  - `ledger.jsonl` and `measure.json`.
- Backup of the material as committed (S4): scratch `v4/bk/M_Fx_PuckTrail.uasset`.
- Scripts are in scratch `v4/`:
  - S4's rig, copied and re-pointed;
  - `mkstreak.py` with `MinRad` 0.01;
  - `xm.mjs` (voice), `imp.py` (import), `cb.sh` (carry run).

## 1. Done / not done

| Item | Verdict |
|---|---|
| Streak core to about 9 px at 30 m (MinRad 0.0072 -> 0.01) | **done, filmed on the GPU, clay and neon, 20/30/35 m, S4 beside V4, measured** |
| Passes stay thinner than shots | **done** (unchanged within 1 px near the camera; a pass's width factor is 0.55 to 0.8 of a shot's at any distance) |
| Carried puck shows nothing | **done** (16.5 m/s carried, both tiers: identical to S4's frames) |
| Three `check.cross_miss` lines in the commentator's voice | **done**: made, checked, imported as `VO_Commentator_check_cross_miss_01..03` with `check_cross_01`'s settings |
| Heard by a person | **not done**: nobody has listened (section 3) |

## 2a. The streak (Job 1)

**What changed**
- `M_Fx_PuckTrail`: the scalar parameter `MinRad` went from 0.0072 to 0.01, rebuilt with S4's own script.
  - Nothing else changed.
  - The material is now 22,118 bytes (it was 21,931).
- `NS_puck_trail` is untouched.
- The white core's width = width factor x taper x max(12 cm, camera distance x 0.01).
  - At 1280 x 720, FOV 70, that is about 9.1 px at any distance beyond 12 m, before the taper.
  - Near the camera the 12 cm floor still rules, so the near frames are unchanged.

**Proving numbers** (S4's rig, measured the way S4 measured: the white core in px 1 m behind the puck, the still minus the same run filmed with no trail):

| Tier | Puck from the camera (speed) | S4 (0.0072) | V4 (0.01) |
|---|---|---|---|
| clay | 20 m (36.1 m/s) | 6.0 px | **9.0 px** |
| clay | 30 m (27.7 m/s) | 6.2 px | **9.2 px** |
| clay | 35 m (26.6 m/s) | 6.2 px | **8.2 px** |
| neon | 20 m | 7.0 px | **10.0 px** |
| neon | 30 m | 7.2 px | **9.2 px** |
| neon | 35 m | 6.2 px | **8.2 px** |

- Core plus dark rim, clay: 9.2 / 10.2 / 9.2 px at 20 / 30 / 35 m (S4: 8.0 / 7.2 / 8.0).
- Length is unchanged: 6.0 / 4.2 / 4.1 m on clay (69 / 19 / 13 px).
- **Passes**, clay, core 1 m behind the puck (both passes are 7 to 14 m from the camera):
  - 14 m/s: 3.8 to 5.0 px, the same as S4;
  - 23.3 m/s: 5.0 to 6.5 px (S4: 5.0 to 6.5).
  - Neon is the same within 0.3 px.
  - At 30 m a pass would be 5 to 7 px against a shot's 8 to 9 px.
- **Carried** at 16.5 m/s:
  - clay: brightest pixel on the path 160.7 against 152.3 one metre aside (S4: 160.7 / 152.0);
  - neon: 33.0 / 0.0 (S4: 33.0 / 0.0).
  - No streak on either tier.

**Stills** (each with a `.txt` caption):
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-shot-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-shot-neon-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-d30-clay-full-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-d30-neon-full-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-pass23-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-pass14-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-123000-B-V4-carry-clay.png`

**What I looked at** with the Read tool: the clay and neon shot sheets (S4 above V4), the full clay 1280 x 720 frame at 30 m (S4 above V4), a x4 zoom of the 30 m puck on clay (side by side), the 23.3 m/s pass sheet, and a x2 crop of the carried puck on clay.
- **20 / 30 / 35 m**: the white wedge under the puck is visibly fuller.
  - At 30 m it is now as wide as the puck's pale disc at its base; in S4's frame it was narrower than the disc.
  - It is still a short tick, because the camera sees the 4 m streak almost end-on.
  - It stays crisp, with thin dark edges on clay.
- **Near frames** (2, 5 and 8 frames after the release): the same long white needle as S4's.
- **Neon**: a fuller white line, no glow cloud.
- **Passes**: the same thin lines as before.
- **Carried puck**: only the puck and its pale disc, no line.
- **Calm**:
  - nothing is drawn over a figure (the ribbon is depth-tested at ice level);
  - no glow, no team colour, no text.

## 2b. The voice (Job 2)

**How it was made**: exactly as lane AU10 made the rough lines that include `check.cross.*`.
- The pipeline: `audio-lab/commentary-rough/scripts/vo.mjs` and `vopost.mjs`, on `audio-lab/vox2/lib.mjs`.
- **Voice**: C1, the designed commentator voice (`audio-lab/commentators/saved.json`), the same id as the whole bank.
- **Generation settings**:
  - `eleven_multilingual_v2`, stability 0.18, style 0.85, speed 1.15;
  - the closing mark sent as "!" ("rule c");
  - output `mp3_44100_128`.
- **Processing**: mono 48 kHz, trim at 0.001 of full scale, 2 / 10 ms fades, -18 LUFS capped at -1.5 dBTP, 16-bit.
- **Conversion**: to mono 24 kHz 16-bit with ffmpeg soxr and no gain change, as lanes AUD and AU11 did.
- **Import**: imported, then 23 sound-wave properties were copied from `VO_Commentator_check_cross_01`. Afterwards no property differed from it:
  - `SC_VO`, compression `PROJECT_DEFINED` quality 40;
  - `LOAD_ON_DEMAND`, not looping, no attenuation, no submix, virtualization `RESTART`;
  - priority, volume and pitch all 1.0, 24000 Hz, 1 channel.
- **Request contents**: only the line's text. Nothing else was sent.

**AU10's per-take checks**: none of the three first takes failed any of them. So there was no second take.
- The checks: clipped, cut off, raw loudness, under target, length against the bank's fit, speed outside the bank's range, under 0.5 s, leading silence.
- Length against the bank's fit: z between +0.1 and +0.8.

| Asset | Text (as sent) | s | LUFS | dBTP | words/s | f0 median |
|---|---|---|---|---|---|---|
| VO_Commentator_check_cross_miss_01 | Shoves at air! | 1.53 | -18.0 | -2.9 | 1.96 | 190 Hz |
| VO_Commentator_check_cross_miss_02 | Cross-check, and nobody home! | 2.08 | -18.0 | -1.7 | 1.92 | 197 Hz |
| VO_Commentator_check_cross_miss_03 | Swings the shaft, finds nothing! | 2.43 | -18.3 | -1.5 | 2.05 | 226 Hz |
| *reference* VO_Commentator_check_cross_01 | Stick across the chest! That is a cross-check. | 3.36 | -19.2 | -1.5 | 2.38 | 171 Hz |
| *reference* check_cross_02 / 03 | | 2.56 / 1.49 | -19.1 / -18.0 | -1.5 / -3.8 | | 190 / 171 Hz |

- All three new lines are under the picker's 2.5 s gate.
- The uasset durations are 1.534, 2.085 and 2.434 s.

**Credit ledger** (`C:/Users/kazda/jam/audio-lab/cross-miss/ledger.jsonl`; by the `character-cost` response header):

| time (UTC) | request | chars | credits | request id |
|---|---|---|---|---|
| 11:34:16 | tts "Shoves at air!" | 14 | 6 | EvMERePx6v4Kot3Wgaw7 |
| 11:34:16 | tts "Cross-check, and nobody home!" | 29 | 13 | ogQmse07Qu3jhIe5Q55F |
| 11:34:17 | tts "Swings the shaft, finds nothing!" | 32 | 14 | gqc0xI9hWlgKW3ZZF6LJ |
| | **total: 3 requests, 3 succeeded, none retried** | 75 | **33 of the 150 allowed** | |

The account's own count agrees: 3,665 used before and 3,698 after (+33), with 127,095 left.

## 3. Not verified
- **Nobody has listened to the three lines.** Every statement in section 2b is a measurement.
  - The voice id and settings are the bank's.
  - The pitch sits in the same band as `check_cross_01..03`.
  - Whether each take actually says its text, without a slip, is unheard.
- The new lines have not been played in a match. They are not in A's bank yet.
- The streak was filmed with the S4 stand-in (a static puck copy on A's camera geometry), not with a real match shot and A's moving camera.
- The cook. The material was only re-parameterised; the graph is the same.
- The night tier (`l2_night`), and scalability tiers below the editor default.
- **The ribbon's width caps the streak.** The ribbon stays 44 cm wide, and that sets two limits:
  - Beyond about 32 m the dark rim at the puck's base starts to be clipped. At 35 m the core is 8.2 px and the rim still shows (9.2 px in total).
  - Beyond 44 m the core itself is capped at 44 cm (about 8 px at 50 m).
  - If the far end of the rink matters, widening the ribbon in `NS_puck_trail` (and `GeomW` in the material to match) is the next step. I did not change it.

## 4. For device A

**Streak**: nothing is required.

**Bank entry (A's file, not edited by me)**: event `lift`, kind `miss`, in the shape of the existing `check.cross.0N` entries (same team, priority and cooldown as `check.cross.01`; `when` rewritten):

```json
{
  "id": "check.cross_miss.01",
  "event": "lift",
  "kind": "miss",
  "team": "any",
  "priority": 3,
  "cooldown_s": 12,
  "when": "the human's cross-check that meets nobody (event lift, Kind miss)",
  "text": "Shoves at air.",
  "sound": "/JamAssets/Audio/VO/VO_Commentator_check_cross_miss_01.VO_Commentator_check_cross_miss_01",
  "seconds": 1.53
},
{
  "id": "check.cross_miss.02",
  "event": "lift",
  "kind": "miss",
  "team": "any",
  "priority": 3,
  "cooldown_s": 12,
  "when": "the human's cross-check that meets nobody (event lift, Kind miss)",
  "text": "Cross-check, and nobody home.",
  "sound": "/JamAssets/Audio/VO/VO_Commentator_check_cross_miss_02.VO_Commentator_check_cross_miss_02",
  "seconds": 2.08
},
{
  "id": "check.cross_miss.03",
  "event": "lift",
  "kind": "miss",
  "team": "any",
  "priority": 3,
  "cooldown_s": 12,
  "when": "the human's cross-check that meets nobody (event lift, Kind miss)",
  "text": "Swings the shaft, finds nothing.",
  "sound": "/JamAssets/Audio/VO/VO_Commentator_check_cross_miss_03.VO_Commentator_check_cross_miss_03",
  "seconds": 2.43
}
```

- The `text` fields are as A proposed. The closing "!" went only to the service.
- The asset paths:
  - `Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_01.uasset`
  - `Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_02.uasset`
  - `Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_03.uasset`
- Priority and cooldown are copied from `check.cross.01`. A may want a shorter cooldown for a miss.

## 5. Decisions for the operator
1. **Listen to the three lines.** They are in `C:/Users/kazda/jam/audio-lab/cross-miss/wav/`, or in the editor under `Audio/VO/`.
   - A retake costs 6 to 14 credits a line.
   - 117 credits of this lane's allowance are left, but the brief allows two takes a line at most, so one more take each.
2. **Streak**: keep 9 px. Alternatively widen the ribbon, so the rim also holds beyond 32 m and the core holds beyond 44 m (a Niagara change, not filmed).

## 6. Next three steps
1. The lead's cook-only run on this tree before pushing dev-b: the re-parameterised material and three new sound waves.
2. A plays one miss in a match after adding the bank entry, and confirms that `JAMVO` finds the three waves.
3. One still from a real match shot in a package build, to confirm the 9 px at A's own camera.

## 7. Ledger rows

`M_Fx_PuckTrail` **replaces** its row. The three VO rows are new.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckTrail.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | translucent unlit two-sided streak for the puck ribbon: inside the 44 cm ribbon a white core of width factor x (1 - U)^0.8 x max(12 cm, camera distance x 0.01) (about 9 px at 1280x720 FOV 70 beyond 12 m; capped by the 44 cm ribbon beyond 44 m) with a dark rim (0.025, 0.03, 0.04) at 0.6 of 0.0018 x distance each side, antialiased with fwidth; core brightness 8.0 on clay, 3.0 off clay (MPC_Look.Clay); opacity = particle alpha; no texture, no team colour (lanes B37, V3, S4, V4: MinRad 0.0072 -> 0.01) (22118 bytes) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_01.uasset | audio | 0.05 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/cross-miss/measure.json (text); cross-miss/ledger.jsonl | 2026-11-15 | B | y | commentator "Shoves at air." (situation check.cross_miss, event lift kind miss; 1.53 s, -18.0 LUFS, -2.9 dBTP; lane V4) (56730 bytes) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_02.uasset | audio | 0.07 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/cross-miss/measure.json (text); cross-miss/ledger.jsonl | 2026-11-15 | B | y | commentator "Cross-check, and nobody home." (situation check.cross_miss, event lift kind miss; 2.08 s, -18.0 LUFS, -1.7 dBTP; lane V4) (70518 bytes) |
| Game/Plugins/JamAssets/Content/Audio/VO/VO_Commentator_check_cross_miss_03.uasset | audio | 0.08 | ai-generated | ElevenLabs TTS (designed voice C1) + ffmpeg (mono 24 kHz) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/cross-miss/measure.json (text); cross-miss/ledger.jsonl | 2026-11-15 | B | y | commentator "Swings the shaft, finds nothing." (situation check.cross_miss, event lift kind miss; 2.43 s, -18.3 LUFS, -1.5 dBTP; lane V4) (85025 bytes) |
