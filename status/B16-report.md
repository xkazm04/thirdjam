# B16 report: what the night rink stands on and in (ground, horizon), plus the two film tails (device B)

Lane B16, device B, 2026-11-12. Machine clock 18:30 to 19:10.

Setup:
- Editor on port 8002, tree `wt/b-vfx`, pid 8904. It left through `ExitEditor` with no dirty package and PIE stopped.
- No git command and no build. `/Game/Maps/Rink` and `/Game/` were never saved. Nothing is under `_Scratch/`.
- The rig was edited the way B15 did it: open `LR_L2_Night` as the editor world, change it, save it, then open `/Game/Maps/Rink` again.
- Not touched: `Skaters/`, `Props/`, `VFX/`, `LR_P3_Clay`, `LR_P4_Broadcast`, `M_Toon`, `M_Ice`.
- Cvars: `jam.Cut` is back to `hockey.regular` after the large-cut check. `jam.ScriptHuman` was set to 1 for the frozen frames and not set back. Cvars are not saved, so both reset when the editor exited. `jam.rink.construction` was not touched.
- The editor on port 8000 shared the GPU, so I took **no frame numbers**.
- **Method:** B15's frozen frame (seed 1, slomo frozen at t 2.18 to 2.20) with cameras at 10 / 24 / 40 m. I added a game-camera view and a high corner view (`hi`: (-75, -55, 30) m, pitch -22, yaw 35).
- **Luma:** sRGB Rec.709 means over fixed crops. For named world points, I projected the point to the screen in PIE and averaged a 9 x 9 px box around it.

**First, for everyone: the default figure renders with the engine's Default Material.** In every still, before my changes and after them, the figures are grey-black.

The editor log says `Material /JamAssets/Look/M_Toon.M_Toon missing usage flag SkeletalMesh! Default Material will be used in game`. The same warning appears for `MI_Toon_Helmet` and `MI_Toon_Stick`, 3,135 warnings in total. Device A's new skeletal toy figure uses `M_Toon`, and `M_Toon` has `used_with_skeletal_mesh` False.

The fix is one flag on `M_Toon`: set it, recompile, save. It does not change what the material outputs. I did not make it, because `M_Toon` is outside my write list. It is urgent: a cooked build would ship grey figures. See "What the lead must decide".

## 1. Ground. Built and saved

### What is in the rig

`LR_L2_Night` went from 154 to 163 actors. The 9 new actors are in folder `Snow`.

| Actor | Mesh | Triangles | Where |
|---|---|---:|---|
| `Snow_L2_Night_Field` | `Arena/SM_Con_SnowField` | 240 | disc of radius 295 m at z -1.5 cm, centred on the rink; the dome is 300 m |
| `Snow_L2_Night_Bank_1` / `_2` | `Arena/SM_Con_SnowBank_End` | 304 each | +x end; the -x end is the same mesh at yaw 180 |
| `Snow_L2_Night_Bank_3` | `Arena/SM_Con_SnowBank_Side` | 416 | -y long side |
| `Snow_L2_Night_Bank_4` | `Arena/SM_Con_SnowBank_SideBench` | 352 | +y long side, broken at the benches |
| `Snow_L2_Night_Mound_1..4` | `Arena/SM_Con_SnowMound` | 112 each | at the tower feet, (+-52, +-30) m |

About 2,060 triangles in total.

**Component settings on all nine:**
- Static mobility, like the `Con_` actors.
- No shadow, no collision (`NoCollision`, so the spring arm never hits them).
- No custom depth (stencil 0).
- `affect_dynamic_indirect_lighting` False, `affect_distance_field_lighting` False.
- No lights were added.
- Every actor sits at the origin or a tower foot, so the rig switch's uniform scale scales the ground and banks with the rink.

**The bank shape.**
- A continuous rounded bank, 0.6 m high and 2 m wide at the base. Profile: (1 - u²)^0.9.
- It runs 0.6 to 2.6 m outside the outer face of the boards, with the crest at 1.6 m.
- It is broken in four places:
  - at each corner, between 30 and 60 degrees of the corner arc, facing the towers;
  - on the +y side for |x| < 13.5 m (A's benches end at 12 m).
- The ends taper over 2.2 m.
- From the game camera the bank sits behind and below the board line, so it never stands between the camera and a net. From inside the rink it is hidden by the boards.

**The meshes were built procedurally by script.**
- A numpy script writes the vertex and triangle lists.
- A Python script builds each `StaticMesh` through `StaticMeshDescription` and `build_from_static_mesh_descriptions`.
- Interchange OBJ import was tried first and rejected: "Bad MeshDescription", and an ensure on missing UVs.
- The mound kept Interchange import data that held a local scratch path. I cleared it and re-saved the mound (77,348 to 14,256 bytes). No file now contains a local path.

### `Look/M_Snow` (new) and `MI_Snow_Night` (new)

`M_Snow` is unlit and opaque, with no texture. One Custom node, `SnowB16`, takes the local position (world to local, so it scales with the rig) and the local normal. In order, it computes:

1. The distance d from the rink outline: a rounded-rectangle SDF of the boards' outer face.
2. The spill: exp(-d / `SpillCm`), weaker toward the ends (`SpillEndKeep`) and scalloped opposite the four pools (`PoolScallop`, pitch 22.5 m). It is posterised into `Bands` flat tones between `SnowShade` (sky-lit) and `SnowLit` (spill-lit).
3. Large soft drifts: smooth sine shapes about 22 m across, cut by one threshold into a crest tone ×`DriftLight`. There is no fine noise.
4. A packed apron within `ApronCm` of the boards (×`ApronTint`).
5. On the banks, the face toward the rink ×`BankLit` and the face away ×`BankShade`. Cut by `BankEdge`.
6. A fade to `FarColour` between `FarStart` and `FarEnd`.

Every edge is anti-aliased with `fwidth`, so the boundaries do not shimmer in stills. Motion is not verified.

| Parameter | `M_Snow` default | `MI_Snow_Night` |
|---|---|---|
| `RinkHX`, `RinkHY`, `RinkR` (cm, the boards' outer face) | 4520, 2270, 395 | default |
| `SpillCm`, `SpillEndKeep` | 900, 0.6 | default |
| `Bands` | 3 | **4** |
| `PoolScallop`, `PoolPitchCm`, `PoolPhaseCm` | 0, 2250, 1125 | **0.2**, default, default |
| `DriftCm`, `DriftCut`, `DriftLight` | 2200, 0.35, 1.12 | default |
| `ApronCm`, `ApronOn`, `ApronTint` | 70, 1, (0.82, 0.84, 0.88) | default |
| `BankLit`, `BankShade`, `BankEdge` | 1.15, 0.7, 0.25 | **1.4, 0.55**, default |
| `FarStart`, `FarEnd` (cm) | 9000, 26000 | default |
| `SnowShade` | (0.10, 0.13, 0.22) | **(0.07, 0.10, 0.20)** |
| `SnowLit` | (0.50, 0.53, 0.60) | **(0.42, 0.50, 0.68)** |
| `FarColour` | (0.03, 0.045, 0.09) | **(0.06, 0.08, 0.15)** |
| `Glow` | 1 | 1 |

### Floors and z-fighting

| Surface | z |
|---|---:|
| My ground | -1.5 cm |
| Device A's `ArenaFloor_UnderIce` (now 220 x 140 m) | -5 cm |
| B13's `Room_L2_Night_1` | -3 cm |
| B13's `SkyB_L2_Night_Ground_1..4` | -3 cm |
| Bank feet | -1 cm |

- The ground is above every older floor.
- Inside the boards it lies under A's ice (top at 0 cm) and is hidden by it.
- No z-fight in any still: 10 / 24 / 40 m, game camera, high corner, bank close-up, large cut.
- The older floors stay, buried under the snow. They are what shows when the snow is hidden.

### Towers stand in the snow

The legs start at z 0, and each foot has a mound 1.75 m in radius and 0.35 m high. See `final-hi`. The end headers are carried by the towers.

## 2. Horizon. Built and saved

### `M_NightSky`: new block in the `NightSky` Custom node, group `Horizon (B16)`

1. A faint glow: `GlowColour` × `HorizonGlow` × exp(-e / `GlowWidth`), where e is the sine of the view elevation.
2. Two silhouette layers by azimuth:
   - Far hills: four sines, height `HillFarH`.
   - Near tree line: low hills of height `HillNearH`, plus triangular tree teeth `TreeH` high, `TreeCount` per turn, with a hashed height per tree and gaps.
3. Below each silhouette the layer colour replaces the sky and the stars. That includes the strip of dome below the horizon behind the snow edge, which closes the black gap there.
4. The edges are anti-aliased by `fwidth(e)`.
5. `Hills` lerps the silhouette block in.

`Hills` 0 and `HorizonGlow` 0 return B15's sky exactly. `MI_NightSky_DarkVolume` (SkyMode 0) is unaffected: `var-skyA-24`.

| Parameter | `M_NightSky` default | `MI_NightSky_Stars` |
|---|---|---|
| `Hills` | 0 (off) | **1** |
| `HorizonGlow`, `GlowWidth`, `GlowColour` | 0, 0.06, (0.10, 0.12, 0.20) | **1.0, 0.08**, default |
| `HillFarH`, `HillNearH`, `TreeH`, `TreeCount` | 0.045, 0.02, 0.012, 900 | **0.075, 0.03, 0.016**, 900 |
| `HillFarColour` | (0.012, 0.02, 0.05) | **(0.07, 0.09, 0.17)** |
| `HillNearColour` | (0.004, 0.007, 0.018) | **(0.035, 0.05, 0.10)** |

**Horizon profile, 24 m frame** (luma, columns 0-400, by screen row):

| | Sky glow above | Far hills | Tree line | Far snow |
|---|---:|---:|---:|---:|
| Saved | 13.4 | 12.4-12.9 | 5.0-5.6 | 11.7-13.5 |

- The sky right at the far-hill line reads 16-19 with the hills off, so the far layer is darker than the sky behind its edge and the tree line is clearly darker.
- B15 had 2-3 luma of sky, then black (0.1-0.4) below the horizon.
- With the hills off and the glow on, a dark 1.7-luma strip stays between the snow edge and the horizon. The silhouette is what closes it.

### Effect on the ice: none measurable

The sky light and Lumen see the glow and the snow, so I measured the ice on fresh PIE sessions, same crops as before.

| View | Crop | Before | After |
|---|---|---:|---:|
| 24 m | dark band, left | 77.7 | 77.7 |
| 24 m | dark band, right | 77.9 | 77.9 |
| 24 m | gap, left | 84.0 | 84.0 |
| 24 m | gap, right | 84.0 | 84.0 |
| 24 m | pool | 218.9 | 218.8 |
| 10 m | left | 129.1 | 129.1 |
| 10 m | right | 127.5 | 127.5 |
| 40 m | dark band, left | 78.2 | 78.2 |
| 40 m | dark band, right | 79.0 | 79.0 |
| Game camera | three crops | 82.1 / 79.2 / 100.8 | 82.2 / 79.1 / 101.9 |
| High view | dark ice at (0, -15) | 69.4 | 69.4 |

- Every change is within 0.1 %, except one game-camera crop at +1.1 %. The game camera moves between frozen frames.
- All are inside the 3 % limit.
- A test at `Glow` 20 lifted the dark ice by 3.5 %. So the snow does reach the ice through Lumen's screen traces, despite `affect_dynamic_indirect_lighting` False. At the saved `Glow` 1 it does not.

## 3. Readability

**Board top, bank and ground** (saved state). The snow stays well under the boards.

| Point | Luma |
|---|---:|
| Board outer face, high view (was 158.3; Lumen sees the snow) | 160.1 |
| Bank crest, 1.6 m out | 72.6 |
| Ground, 2 m out | 73.5 |
| Ground, 10 m out | 38.6 |
| Ground, 30 m out | 18.0 |
| Board top, from the 24 m game frame | 169.0 / 173.3 |
| Ground 10 m beyond the side boards, same frame | 34.7 / 33.1 |

The board line holds, and A's stanchions and rail now read against the snow (`final-40`).

**Puck.** Nothing on the ice changed (the dark band and pools are identical), so B13's puck test stands.
- Dark band at 10 m: halo ring up to 249 against 51.
- Dark band at 24 m: a 4-5 px black puck against 50.
- I tried to move the puck in PIE to repeat it. A's sim owns the puck's position and put it back, so I have no new puck still.

**Figures against the snow.** The game camera sees snow only above the far boards and at the top corners. Behind every figure on the ice there are white boards or ice.
- I could not judge blue against orange: the figures render with the Default Material (see the top of this report).
- Their dark silhouettes separate from the 33-35 luma snow and the 170 luma boards.

**Outline on the banks: none (stencil 0).**
- At stencil 1 (ramp and ink) the unlit bank quantises into the toon ramp's floor and reads as a black slab (`rej-stencil1-bank`). This is the same effect A noted for unlit stencil-1 surfaces.
- At stencil 0 the bank reads by its two painted faces (`final-bank`).

**Ice reflections (screen-space reflections).** No snow, bank or hill appears in the ice in any still, including the low side view at 1 m. The boards hide everything outside, and the bank crest (0.6 m) is below the board top (1.2 m).

**Expected cost** (not measured):
- The ground is one 240-triangle unlit opaque draw, about 40 ALU per pixel (an SDF, about 10 sines, 4 `fwidth`). It covers most of the frame below the horizon and is overdrawn by the ice inside the rink.
- The banks and mounds add about 1.8k triangles of the same shader.
- The sky adds one `atan2` and about 12 sines per sky pixel.
- No light, shadow, translucency or Lumen card was added.
- I expect +0.1 to 0.2 ms on the 4090, most of it the full-frame ground pixels.

## 4. Variants

Saved: ground, banks, hills and glow all on. My judgement:
- **Banks:** they turn the flat apron into a built edge and read from every outside view. From the game camera they are invisible, and they cost nothing.
- **Hills:** they close the black strip under the horizon and give the frame a far plane. The glow alone leaves a dark strip.

| Variant | Stills |
|---|---|
| Saved | `b16-final-game`, `-final-24`, `-final-hi`, `-final-bank` |
| No banks | `-var-nobank-game`, `-var-nobank-24`, `-var-nobank-bank` |
| Hills off | `-var-nohills-game`, `-var-nohills-24` |
| Glow off | `-var-noglow-24` |
| No ground (B15 state) | `-var-noground-24` |
| Dark volume | `-var-skyA-24` |

**One-line switches.** For the rig lines, open `/JamAssets/Rigs/LR_L2_Night` as the editor world and run the line in editor Python. The material lines need no open map.

- **Banks off** (`False` turns them back on):
  ```
  [a.set_actor_hidden_in_game(True) for a in unreal.EditorLevelLibrary.get_all_level_actors() if a.get_actor_label().startswith('Snow_L2_Night_Bank_')]; unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
  ```
- **Whole ground off**, back to B15 (field, banks and mounds): the same line with `'Snow_'`.
- **Hills off** (1.0 turns them back on):
  ```
  mi=unreal.load_asset('/JamAssets/Look/MI_NightSky_Stars'); unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(mi,'Hills',0.0); unreal.MaterialEditingLibrary.update_material_instance(mi); unreal.EditorAssetLibrary.save_asset('/JamAssets/Look/MI_NightSky_Stars')
  ```
- **Glow off:** the same line with `'HorizonGlow'` and 0.0.
- **Dark-volume sky** (SkyA shown, SkyB and all `Snow_` hidden; swap the startswith test to `'SkyB_'` and include `Snow_` to go back). Checked in PIE: it works with the ground hidden (`var-skyA-24`).
  ```
  [a.set_actor_hidden_in_game(not a.get_actor_label().startswith('SkyA_')) for a in unreal.EditorLevelLibrary.get_all_level_actors() if a.get_actor_label()[:5] in ('SkyA_','SkyB_','Snow_')]; unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level()
  ```

## Larger rink check (`hockey.arena.large`, log `scale=1.333`, 120 x 60 m, corner 5 m)

The stills `b16-large-hi`, `-large-corner` and `-large-bench` show:
- The ground disc scales to 393 m inside the 400 m dome, with no hole.
- The bank stays outside the boards everywhere, including the corner gaps.
- The bench break clears the benches.

| Check | 1.333 (large, measured) | 1.556 (wide, computed only) |
|---|---|---|
| Bank inner foot outside the 20 cm boards, straight sides | 0.85 m | 1.05 m |
| Bank break on the bench side | 18.0 m | 21.0 m |
| Bench end | 16.0 m | 18.7 m |

Banks scale to 0.8 m high on the large cut.

## 5. Audio: both film tails removed. Done through the MetaSound toolsets

Method (B3's): `AttachBuilderToAsset`, `RemoveNodes`, `AddNodes`, `SetDefaults`, `Connect`, `CommitBuilderToAsset`. The `Strength` input and the `OnPlay` and `OnFinished` interface are kept. Every other node is untouched, as is the output stage (Strength × k + c, so `Strength` also scales the horn).

**`MS_goal`**
- Removed the reel flutter: `Noise`, `Biquad`, the 24 Hz `LFO`, two `Multiply`, `TriggerDelay`, `AD Envelope`.
- Added a horn on mixer input 5 (`Gain 5` 0.22):
  - two saws at 233.1 and 236.6 Hz (Bb3, about 26 cents apart) through a 2-in mixer;
  - a low-pass biquad at 1300 Hz;
  - an `AD Envelope`, attack 0.012 s, decay 0.9 s, curve 3. That holds near full level, then falls over the last 0.3 s.
- First pass at `Gain 5` 0.35: the horn sat about 6 dB under the burst and the peak rose 2.4 dB. I lowered it to 0.22.

**`MS_lose`**
- Removed the tape stop: the pitch-fall chain `TriggerDelay`, `AD Envelope (Float)`, `Multiply`, `Add` into the second saw's frequency.
- The second note is now a steady C4 (261.63 Hz).
- The sting is now E4 0.3 s, then C4 with its original decay envelope.

**Recorded from the game's audio output** in PIE (`AudioMixerLibrary` output recording, 48 kHz stereo, played at Strength 0.2 and 1.0). Measured with ffmpeg (`astats`, `ebur128`) and a windowed RMS:

| Sound | Strength | Peak before → after (dBFS) | Loudness before → after (LUFS) | Length to -45 dBFS, before → after |
|---|---|---|---|---|
| `MS_goal` | 0.2 | -9.1 → -7.5 | -20.9 → -20.2 | 1.77 → 1.43 s |
| `MS_goal` | 1.0 | -5.7 → -4.2 | -17.6 → -17.2 | 2.07 (B3) → 1.45 s |
| `MS_lose` | 0.2 | -9.8 → -10.7 | -21.7 → -20.0 | 1.21 → 1.23 s |
| `MS_lose` | 1.0 | -8.3 → -8.3 | -18.1 → -17.6 | 1.26 (B3) → 1.24 s |

- **Goal:** RMS in the 0.2-0.5 s window rises 0.5 dB, so the horn sits about 9.6 dB under the burst. It supports rather than leads. The 1.8-2.4 s window goes from -53 dB (the flutter) to silence.
- **Lose:** the 0.9-1.3 s window rises from -30.2 to -29.3 dB, because the C4 holds its pitch instead of sliding into sub-bass.

The wav files are in my scratch folder (`.../scratchpad/b16/aud/before.wav`, `after.wav` at gain 0.35, `after2.wav` as saved). They were moved out of `Game/Saved`. I could not listen to them.

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 398,745 | 417,811 |
| `Look/M_NightSky.uasset` | 20,965 | 28,103 |
| `Look/MI_NightSky_Stars.uasset` | 9,341 | 13,410 |
| `Look/M_Snow.uasset` | new | 28,700 |
| `Look/MI_Snow_Night.uasset` | new | 10,645 |
| `Arena/SM_Con_SnowField.uasset` | new | 16,615 |
| `Arena/SM_Con_SnowBank_End.uasset` | new | 16,213 |
| `Arena/SM_Con_SnowBank_Side.uasset` | new | 16,873 |
| `Arena/SM_Con_SnowBank_SideBench.uasset` | new | 16,494 |
| `Arena/SM_Con_SnowMound.uasset` | new | 14,256 |
| `Audio/MS_goal.uasset` | 228,158 | 218,585 |
| `Audio/MS_lose.uasset` | 167,504 | 144,196 |

`MI_NightSky_DarkVolume` is unchanged (7,994).

## Ledger rows

- **Replace** (match by path): `LR_L2_Night`, `M_NightSky`, `MI_NightSky_Stars`, `MS_goal`, `MS_lose`.
- **Add:** the seven new files.
- **No rows to remove.**

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.4 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | night rig: PPV, sky, 4 spots (inner cone 5), hidden room walls out of Lumen, open night sky (SkyB) and hidden dark-volume variant (SkyA), bare truss on 4 towers, lamp cans, 2 shadowless end fills, 2 end haze cards saved hidden, painted snow ground with banks and tower mounds (Snow_, shadowless, no collision); props unplaced since H8.5 (lanes B1, B1c, B11, B13, B15, B16) |
| Game/Plugins/JamAssets/Content/Look/M_NightSky.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit sky master: dark-volume gradient (SkyMode 0) or procedural star dome and horizon glow (SkyMode 1), no texture (lane B13); horizon glow and two-layer hill and tree-line silhouette, off by default (lane B16) |
| Game/Plugins/JamAssets/Content/Look/MI_NightSky_Stars.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, sky variant B, A23 gradient (lanes B13, B15), hills and horizon glow on (lane B16) |
| Game/Plugins/JamAssets/Content/Look/M_Snow.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit painted night-snow master: spill bands by distance from the rink outline, soft drifts, packed apron, bank faces, far fade; no texture (lane B16) (28700 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Snow_Night.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance for the night rig's snow ground and banks (lane B16) (10645 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_SnowField.uasset | mesh | 0.0 | original | procedural (numpy script + UE 5.9 StaticMeshDescription via MCP/Python) |  |  | 2026-11-12 | B | y | snow ground disc, radius 295 m, 240 triangles (lane B16) (16615 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_SnowBank_End.uasset | mesh | 0.0 | original | procedural (numpy script + UE 5.9 StaticMeshDescription via MCP/Python) |  |  | 2026-11-12 | B | y | snow bank behind one end board, 0.6 m, broken at the tower corners, 304 triangles (lane B16) (16213 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_SnowBank_Side.uasset | mesh | 0.0 | original | procedural (numpy script + UE 5.9 StaticMeshDescription via MCP/Python) |  |  | 2026-11-12 | B | y | snow bank behind the -y long side, 416 triangles (lane B16) (16873 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_SnowBank_SideBench.uasset | mesh | 0.0 | original | procedural (numpy script + UE 5.9 StaticMeshDescription via MCP/Python) |  |  | 2026-11-12 | B | y | snow bank behind the +y long side, broken at the benches, 352 triangles (lane B16) (16494 bytes) |
| Game/Plugins/JamAssets/Content/Arena/SM_Con_SnowMound.uasset | mesh | 0.0 | original | procedural (numpy script + UE 5.9 StaticMeshDescription via MCP/Python) |  |  | 2026-11-12 | B | y | low snow mound at a light-tower foot, 112 triangles (lane B16) (14256 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_goal.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); film-reel flutter tail removed, short detuned-saw arena horn under the burst (lane B16) (218585 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lose.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); tape-stop pitch fall removed, E4 then C4 sting (lane B16) (144196 bytes) |
```

## Rejected, and why

- **Real lights for the spill:** the brief says no lights. A lit snow material would also go through the ramp and Lumen. Unlit with the falloff baked in gives fixed tones at zero light cost.
- **Stencil 1 on the banks:** they quantise to black under the toon ramp.
- **`Glow` 2:** the ground 2 m out reached 120 luma, too close to the board line. `Glow` 20, a calibration test, lifted the dark ice by 3.5 %.
- **The first hill colours:** a 1.4-luma black band.
- **The first hill heights** (0.045 / 0.02): almost invisible from the game camera.
- **`PoolScallop` 0.35:** the spill bands read as lake contour lines.
- **Interchange OBJ import** for the meshes: bad mesh description, and import data with a local path.
- **A bank in front of the benches:** it would cross A's bench walls (back wall at y 23.85 m).

## Six stills for the operator (`devlog/shots/`)

1. `20261112-190601-B-b16-final-game`: the game camera as saved.
2. `20261112-190601-B-b16-final-24` next to `-before-24`: the void filled; the ice is unchanged.
3. `20261112-190602-B-b16-final-hi` next to `-before-hi`: the whole place (snow field, apron, banks, mounds, hills).
4. `20261112-190602-B-b16-final-bank`: the bank close up.
5. `20261112-190603-B-b16-var-nohills-24`: the horizon choice.
6. `20261112-190603-B-b16-var-nobank-bank`: the bank choice.

## What device A must know

1. **`M_Toon` lacks the skeletal-mesh usage flag**, so your skeletal toy figure renders with the Default Material in PIE (3,135 log warnings, plus `MI_Toon_Helmet` and `MI_Toon_Stick`). B owns the fix: one flag on `M_Toon`, saved. Until it lands, every look still shows grey figures.
2. **Your `ArenaFloor_UnderIce`** is 220 x 140 m at -5 cm. B's snow covers it at -1.5 cm everywhere outside the ice, so your floor no longer shows under the night tier. The floor-lowering warning from B13 is still open, but harmless here.
3. **Anything new outside the boards** (gates, a penalty box, moving the benches) must stay inside 0.6 m of the boards' outer face, or B must cut the bank there.
   - The bank runs 0.6 to 2.6 m out.
   - It is broken for |x| < 13.5 m on the bench side (your benches end at 12 m, scaled with the rink) and at the corners facing the towers.
4. **The rig is the only source of the ground.** The other tiers' rigs have none.

## What I could not verify

- Frame cost (the 8000 editor shared the GPU) and the 4090.
- Motion and TSR on the drift edges, the band edges and the tree teeth. Everything is `fwidth`-anti-aliased, but I have stills only.
- The wide cut (computed only).
- Blue against orange figures over the snow, because the figures render grey.
- A new puck still: the sim owns the puck's position.
- The sound by ear.
- The cooked game.

## What the lead must decide

1. **Urgent:** have `M_Toon` (and any other figure master) set `used_with_skeletal_mesh` and saved. It is a one-line change, `M_Toon`'s output is unchanged, and it is outside my write list.
2. **Banks on or off.** On is saved; the switch is above.
3. **Hills and glow on or off.** Both are on as saved.
4. Whether to delete the now-buried `Room_L2_Night_1` and `SkyB_L2_Night_Ground_1..4`. They are only visible when `Snow_` is hidden.
5. **Audio:**
   - The horn level, `Gain 5` 0.22: about 10 dB under the goal burst. 0.35 gives about 6 dB under.
   - Whether `MS_lose` as a plain two-note fall is right.
6. Paste the ledger rows.
