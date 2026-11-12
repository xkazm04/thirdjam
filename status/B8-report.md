# B8 report: asset hygiene and skater tweaks (device B)

Lane B8, 2026-11-12, about 13:40 to 14:00 local. Editor on port 8000 only (no second editor seen; frame numbers: none taken). Run no git command. `/Game/Maps/Rink` was never saved;
at the end the editor is on `/Game/Maps/Rink`, no dirty map, no dirty content package, nothing under `_Scratch/` (created for the sound test, deleted, folder gone from disk).
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b8` (`$B8`).

**Two things were missing from the tree**: `handoff/A-to-B-0004.md` and `docs/experiments/SIZE.md`. I used the rules as written in the lane prompt and `briefs/A13-size-playbook.md`, and created
`handoff/A-to-B-0004.md` with the Response filled in (a header rebuilt from the A13 brief, flagged inside the file).

## Part 1: import rules

### a/b. Textures under `/JamAssets/` (8 in all; no texture under `Look/`, `Props/`, `Rigs/`, `VFX/`, `Audio/`)

All eight: sRGB on, LOD bias 0, virtual texture off, LOD group World, mip gen from group, source alpha channel constant 1 (opaque, so no BC7 needed). Pixel size = source size = built size.

| Texture | px | Compression before | after | Lossy before | after | NoAlpha before | after | MaxSize | `.uasset` bytes before | after | Cooked estimate (BC1, mips) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| Arena/T_CameraCrane_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 967,736 | 968,030 | 699,050 B |
| Arena/T_CameraDolly_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 778,820 | 779,114 | 699,050 B |
| Arena/T_DirectorChair_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 629,243 | 629,537 | 699,050 B |
| Arena/T_FilmLight_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 480,793 | 481,087 | 699,050 B |
| Arena/T_FlightCases_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 829,724 | 830,018 | 699,050 B |
| Arena/T_Slate_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 677,632 | 677,926 | 699,050 B |
| Arena/T_TrussLamps_BaseColor | 1024 | TC_Default | TC_Default | Default | Medium | off | on | 1024 | 774,594 | 774,888 | 699,050 B |
| Skaters/T_SkaterTB2_BaseColor | 2048 | TC_Default | TC_Default | Default | Medium | off | on | 2048 | 2,356,732 | 2,356,931 | 2,796,202 B |

Max size, VT and LOD bias already matched the rules, so the real changes are `CompressionNoAlpha` on (forces BC1 even if a later source gets an alpha channel) and Lossy = Medium.
`.uasset` bytes are the compressed **source** the editor keeps, so they do not predict the shipped size: **the shipped size is only known after a cook.** The cooked estimate is arithmetic
(0.5 byte per pixel BC1 times 4/3 for mips; a BC7 or BC3 texture would be double), not a measurement; the encoder already chooses BC1 for an opaque source, so the format itself is probably unchanged.
**Important:** Medium means Oodle RDO 20, and RDO is applied in the Final (cook) encode only when `[/Script/Engine.TextureEncodingProjectSettings] bFinalUsesRDO=True`.
The engine default is false (`BaseEngine.ini:3743`) and `Game/Config` does not set it, so until device A adds it the Medium flag does nothing (the smaller pak from RDO is not measured).

### c. The script

`$B8/apply_import_rules.py` (full path `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b8/apply_import_rules.py`). Rules: no alpha in source gives
`TC_Default` + CompressionNoAlpha on; real alpha (source alpha min below 1) gives `TC_BC7`; Lossy Medium; VT off; MaxTextureSize 2048 under `/Skaters/`, 1024 elsewhere (a smaller value is kept, 0 becomes the cap).
Textures already using another format (normal map, mask, HDR) keep it; the other rules still apply. It saves each asset it changes and prints one line per texture plus a summary. Idempotent (second run: 0 changed).
Runs inside the editor through MCP `execute_python`:

```
node tools/mcp/mcp.mjs call editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python @call.json --url http://127.0.0.1:8000/mcp --timeout 600
```
with `call.json` holding `{"script":{"content":"exec(open('C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b8/apply_import_rules.py').read())\nREPORT","language":"Python"}}`.
A dry run (no change, no save) puts `DRY_RUN = True` before the `exec`. Optional globals before the `exec`: `ROOT` (default `/JamAssets`), `SKIP` (list of path fragments). The lead may want to copy it under `tools/` (device A's folder, I did not).

**Per-folder defaults:** there is no per-folder import default in the engine. Texture LOD groups are per texture (`lod_group`), and the group's settings live in config (`TextureLODGroups` in the device profiles;
`FTextureLODGroup` has a `LossyCompressionAmount` field, `TextureLODSettings.h:125`). So from B's side the only mechanism is the script above.
**Config for device A (via the lead), `Game/Config/DefaultEngine.ini`:**
```
[/Script/Engine.TextureEncodingProjectSettings]
bFinalUsesRDO=True
FinalRDOLambda=20
```
(Textures set to Medium use lambda 20 whatever `FinalRDOLambda` says; it only matters for textures left on Default.) Nothing else needs config.

### d. Sound

No wave assets exist. Rule for imported waves: SFX mono, Bink, quality 40; music stereo. Tested on throwaway waves under `/JamAssets/_Scratch/` (deleted, folder gone):
- `AssetImportTask` of a 44.1 kHz stereo wav gave `SoundWave`, `compression_quality` 80, `num_channels` 2, compression type `PROJECT_DEFINED`.
- `set_editor_property("compression_quality", 40)` works; `set_sound_asset_compression_type(BINK_AUDIO)` works; saved fine.
- **`num_channels` is read-only**: mono cannot be set after import. Make the source mono first (a 1-channel wav imported as 1 channel; verified) or downmix before import.
So the import script for waves would be: pre-downmix SFX, import, quality 40, Bink. Not added to the texture script (no waves yet).

### e. Meshes

| Mesh | Nanite | Triangles (LOD0) | LODs |
|---|---|---|---|
| Arena/SM_CameraCrane | on | 9,938 | 1 |
| Arena/SM_CameraDolly | on | 9,556 | 1 |
| Arena/SM_DirectorChair | on | 9,374 | 1 |
| Arena/SM_FilmLight | on | 9,722 | 1 |
| Arena/SM_FlightCases | on | 9,488 | 1 |
| Arena/SM_Slate | on | 9,948 | 1 |
| Arena/SM_TrussLamps | on | 9,304 | 1 |
| Props/SM_PuckDisc (not one of the seven) | off | 512 | 1 |
| Skaters/SK_SkaterTB1 (skeletal) | n/a | 5,932 | |
| Skaters/SK_SkaterTB2 (skeletal) | n/a | 19,944 | |

All seven props are Nanite: nothing changed. (Counts through Geometry Script on LOD0.)

## Part 2: skater tweaks

### a. Lean scales with speed (both blend spaces)

Samples before (each of `BS_SkaterTB1_Skate`, `BS_SkaterTB2_Skate`, 12 samples): Lean_Left at (0, +3) and (3, +3); Lean_Right at (0, -3) and (3, -3).
After (14 samples): the two samples at speed 0 are now `Glide` at (0, ±3) and a new `Glide` (rate 0.8, the same as the existing (1, 0) sample) at (1, ±3); `Lean_*` stays at (3, ±3); crossovers at 5 and 8 unchanged.
Result: no lean up to 1 m/s, a ramp between 1 and 3 m/s, full lean from 3 m/s. Axis smoothing kept: Speed 0.15 s, TurnRate 0.25 s (read back after the edit).
Measured in the real match (hybrid, global time dilation 0.5, 120 samples of 5 skaters at TurnRate ±5, `head_socket` lateral offset in component space, cm; before-state not remeasured):

| Speed | head lateral offset (left turn) | (right turn) |
|---|---|---|
| under 0.8 m/s | 0.2 | 0.2 |
| 0.8 to 1.5 | -4.3 (7 samples) | +3.9 |
| 1.5 to 2.5 | -17.9 | +21.2 |
| 2.5 to 3.5 | -30.7 | +30.8 |
| over 3.5 | -27 (some samples back in transition) | +27.6 |

So a figure turning on the spot at 0.4 to 0.6 m/s stays upright and full lean arrives at about 3 m/s. (Before: B6s measured full lean at every speed by design of the samples; I did not re-take it.)

### b. Orange in the dark: `M_SkaterTB2`

Cause, as far as I can show: the Rim+Emissive node outputs `Emissive * TeamColour` (the recoloured pixel colour) with `Emissive` 0.5, and in the dark bands that is all the light the figure has;
orange's dim emissive lands on the toon ramp's dark step and reads brown-red, blue's does not. Tested on the live figure (game colours: blue #1D4ED8, orange #E8651A) with Emissive 0.5, 1.0, 1.5, 2.5 on orange next to blue 0.5:
1.0 already reads orange; higher only glows.
Smallest change that needs no Blueprint edit and survives `SetBoost` (which rewrites `Emissive`): a warm-only multiplier inside the material.

| Asset | Property | Old | New |
|---|---|---|---|
| `M_SkaterTB2` | node `Custom_0` (Rim+Emissive) code | `return F * RimStrength * RimColour + Emissive * TeamColour;` | `float w = 1.0 + WarmFill * saturate(TeamRaw.r - TeamRaw.b); return F * RimStrength * RimColour + Emissive * w * TeamColour;` |
| `M_SkaterTB2` | new inputs on that node | | `WarmFill` (from a new scalar parameter `WarmFill`, default **1.25**), `TeamRaw` (from the `TeamColour` parameter) |

Effect: factor 1.0 for blue (r below b), about 2.0 for the cartridge orange (r minus b = 0.80), so orange behaves like Emissive 1.0 and blue is unchanged. `WarmFill` 0 restores the old look per instance.
Stills (blue left, orange right, hybrid, same camera, WarmFill 0 against 1.25):
- pool: `devlog/shots/20261112-135454-B-b8-orange-before-pool.png` and `-orange-after-pool.png` (orange a touch brighter, blue identical)
- dark band: `-orange-before-band.png` (orange brown-red, blue strong) and `-orange-after-band.png` (orange reads orange, blue unchanged, no glow)

**T-B1** (`M_Toon` instances), checked in the dark band with the same setup: `devlog/shots/20261112-135606-B-b8-tb1-band-check.png`. The orange **jersey stays orange** (Emissive 2.0), so it needs nothing.
The **helmet** (`MI_SkaterTB1_Helmet`, team colour x 0.5) goes dark brown-red in the band, as B4 noted for helmet colour; the jersey carries the team read, so I changed nothing. If the lead wants it fixed: raise `Emissive` on the helmet or make it white (B4 open question).

### c. Stick blade against the carried puck

Method: skinned the stick vertices (material slot Stick) with the mesh's own bone weights and the pose from the clips, on both figures, at 10 times of `Stride` and 7 of `Glide`, blade = lowest 6 cm of the stick. Both rigs are the same.
Old `puck_socket` (on bone `puck_socket`, fixed at about 78 cm ahead, 10 cm left of the pawn): **108 to 140 cm** from the blade in Stride, 12 (T-B1) / 16 (T-B2) cm in Glide: fails the 10 cm rule in both poses.
The stick swings out to the right and 160 to 170 cm ahead in the stride, so no fixed offset on the root can work. Fix: `puck_socket` on both meshes (`SK_SkaterTB1`, `SK_SkaterTB2`) is now re-created on bone **`stick_socket`** (the hand-held stick bone, child of `hand_r`) with relative location (1.1, -142.1, 2.1) and zero rotation
(removed and re-added through `SkeletalMeshToolset`, because `bone_name` is read-only). Measured afterwards, same samples: **at most 6.5 cm** from the blade (4.4 cm in Stride, 6.5 in Glide), world z of the socket 1 to 4 cm (puck on the ice).
Visual check in PIE: five hybrids in different poses with a sphere at `get_socket_location("puck_socket")`: it sits on the blade each time (`devlog/shots/20261112-135455-B-b8-puck-socket.png`).
Consequence for device A: the carried puck at this socket follows the blade, so it leaves the pawn's front line while striding (up to about 1 m to the side, 1.7 m ahead). T-B1 was checked by the numbers only.

### Stills in the real game (game camera, PIE, `jam.Cut`, 1920 wide)

| File (`devlog/shots/`) | What |
|---|---|
| `20261112-135454-B-b8-tb2h-1.png` | hybrid, three skaters at a pool edge |
| `20261112-135454-B-b8-tb2h-2.png` | hybrid, orange and blue in the dark band, orange stays orange |
| `20261112-135454-B-b8-tb2h-3.png` | hybrid, puck still left of the pawn (device A has not moved it yet) |
| `20261112-135605-B-b8-tb1-1.png` | T-B1, pool |
| `20261112-135606-B-b8-tb1-2.png` | T-B1, pool edge |
| `20261112-135606-B-b8-tb1-3.png` | T-B1, dark band |

All other stills of this lane are the 8 listed above under 2b and 2c and `-tb1-band-check` (`b8-*` labels).

## Files changed (bytes)

| File (under `Game/Plugins/JamAssets/Content/`) | Before | After |
|---|---|---|
| `Arena/T_CameraCrane_BaseColor.uasset` | 967,736 | 968,030 |
| `Arena/T_CameraDolly_BaseColor.uasset` | 778,820 | 779,114 |
| `Arena/T_DirectorChair_BaseColor.uasset` | 629,243 | 629,537 |
| `Arena/T_FilmLight_BaseColor.uasset` | 480,793 | 481,087 |
| `Arena/T_FlightCases_BaseColor.uasset` | 829,724 | 830,018 |
| `Arena/T_Slate_BaseColor.uasset` | 677,632 | 677,926 |
| `Arena/T_TrussLamps_BaseColor.uasset` | 774,594 | 774,888 |
| `Skaters/T_SkaterTB2_BaseColor.uasset` | 2,356,732 | 2,356,931 |
| `Skaters/BS_SkaterTB1_Skate.uasset` | 25,140 | 29,139 |
| `Skaters/BS_SkaterTB2_Skate.uasset` | 25,417 | 29,167 |
| `Skaters/M_SkaterTB2.uasset` | 24,140 | 27,598 |
| `Skaters/SK_SkaterTB1.uasset` | 941,511 | 941,463 |
| `Skaters/SK_SkaterTB2.uasset` | 3,744,531 | 3,744,518 |

Also: `handoff/A-to-B-0004.md` (created, 3462 bytes), this report, 12 stills (png) with `.txt` captions in `devlog/shots/` (1.76 to 2.03 MB each). Scratch script (not in the tree): `$B8/apply_import_rules.py`.

## Ledger notes for the lead

- `M_SkaterTB2.uasset`: add "WarmFill (default 1.25): warm team colours get Emissive x (1 + WarmFill*(r-b)) (lane B8)".
- `BS_SkaterTB1_Skate.uasset`, `BS_SkaterTB2_Skate.uasset`: "lean samples at 0 m/s are Glide; full lean from 3 m/s (lane B8)".
- `SK_SkaterTB1.uasset`, `SK_SkaterTB2.uasset`: "`puck_socket` now on bone `stick_socket`, offset (1.1, -142.1, 2.1) (lane B8)".
- The eight textures: note "BC1 (TC_Default, no alpha), Lossy Medium, 1024 / 2048 (lane B8)".

## Not verified

- Shipped texture sizes and the effect of Medium: needs a cook and `bFinalUsesRDO=True`. The cooked sizes above are arithmetic.
- The package-of-the-plugin acceptance line (open on device B): no package was built.
- Lean before-values were not remeasured; the table is the after state only. Poses between 1 and 3 m/s are a linear blend, not checked frame by frame.
- Orange in the dark was proved with detached figures placed in the real match at two spots, plus match stills; the figures' poses were frozen by time dilation 0.0005. No numeric colour measurement (judged by eye).
- `puck_socket` on T-B1: numbers only (same rig, same offset), no still with the marker. Not checked: puck-in-front-of-face versus behind-the-blade orientation (the 10 cm rule is met against the blade's lowest points); knock-down and shot poses.
- Frame cost: no frame numbers were taken.
- Whether the sound import behaves the same for 48 kHz or looping waves; only one 44.1 kHz stereo and one mono test wave.

## For the lead to decide

1. Hand device A the `bFinalUsesRDO=True` lines (and say whether to place `apply_import_rules.py` under `tools/`).
2. The carried puck: device A to use `puck_socket` (it follows the blade now; the puck leaves the pawn front line in a stride). If A prefers the pawn-front carry, the blade would have to be centred instead.
3. T-B1's helmet in the dark (raise `Emissive`, or white/dark helmet).
4. `WarmFill` 1.25 is tuned to the cartridge orange; a different team colour that is warm and dark would get the same boost. Per-instance override is `WarmFill` on the dynamic material.
5. Whether to keep Nanite on `SM_PuckDisc` off (512 triangles, outline stencil 3): left as it is.
