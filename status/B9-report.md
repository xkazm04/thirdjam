# B9 report: the P3 clay-miniature look tier, pass 1 (device B)

Lane B9, device B, 2026-11-12, 14:05 to 14:55. I drove the port-8000 editor on the main tree through `tools/mcp/mcp.mjs`. A second editor (port 8002, `wt/b-vfx`) shared the GPU for most of the session.

At the end:
- The editor runs on `/Game/Maps/Rink`, PIE is stopped, and no package is dirty.
- `Rink` was never saved, and nothing under `/Game/` was written.
- Nothing is under `_Scratch/`.
- `jam.Cut` is back to `hockey.regular`.

Git: I ran one read-only `git log -1` at the start by mistake (HEAD was `47d0bb0`). I ran no other git command.

## Blocker (read first)

At **14:39** the tree was merged while my editor was running (`.git/ORIG_HEAD` and every cartridge are stamped 14:39:13). The new cartridges are rejected by the old binaries, so PIE no longer builds a rink. Two examples:
- `JAMCART hockey.regular at /game/rules/penalties: must be exactly false but found true`
- `hockey.raw at /game/stick: unknown key "stick"`

I exited the editor cleanly and rebuilt. **The rebuild fails on device A's Source:**

```
C:\Users\kazda\jam\thirdjam\Game\Source\JamCore\Effects\JamEffectTypes.h(124): Error: Unable to find 'class', 'delegate', 'enum', or 'struct' with name 'EJamTarget'
Result: Failed (OtherCompilationError)   (UHT; log Game/logs/build-ThirdJamEditor-Development-20261112-144121.log)
```

`FJamEffectHudEntry` has a `UPROPERTY` of type `EJamTarget`, and `EJamTarget` is not a `UENUM`. I restarted the editor on the old binaries, so it is up. **No real-game PIE was possible after 14:39.**

This is why these are missing:
- the humanoid-visual clay stills;
- the 10 / 24 / 40 m puck tests;
- the 60 m team test.

Everything taken before 14:39 is valid.

## Verdict per deliverable

| # | Deliverable | Verdict | Evidence |
|---|---|---|---|
| 1a | `MPC_Look` | **PASS** | `/JamAssets/Look/MPC_Look`. Scalars: `Clay` 0, `ClayBump` 0.8, `FeltRim` 0.5, `ClayFill` 0.35, `ClayTile` 40 (cm), `ClayRough` 0.85. Vector: `LookTint` (1.0, 0.94, 0.84). |
| 1b | Masters read it | **PASS** | `M_Toon`, `M_Ice`, `M_Trim`, `M_PropCel`, `M_SkaterTB2`. All compile with no error expressions, and a live `Clay` 1 in PIE changes the picture. Graph changes are in the next section. |
| 1c | Proof that `Clay` 0 leaves L2 unchanged | **PASS, within the noise floor** | Table below. Two humanoid views differ only where the bots had moved. |
| 2a | `BP_LookParams` | **PASS** | Seven instance-editable variables. BeginPlay writes them to `MPC_Look`; EndPlay writes the MPC defaults back. Verified in PIE: right after the stream-in, `Clay`=1, `ClayBump`=2, `ClayFill`=0.15, `ClayTile`=25. |
| 2b | `LR_P3_Clay` | **PASS** | New file `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap`. Actors: `Key_P3_Clay`, `Fill_P3_Clay_1..3`, `Sky_P3_Clay`, `PPV_P3_Clay`, `LookParams_P3_Clay`. Made with `new_level` plus a save, then `Rink` opened in a separate call (no crash). |
| 2c | Two textures | **PASS** | Codex, about 1 min each. `T_Clay_Thumb_N` (BC5, `TC_Normalmap`) and `T_Felt_Fibre` (BC1, `TC_Default` with no alpha, linear). Both 1024, RDO Medium, no virtual texture. B8's `apply_import_rules.py` reports "no change" for both (dry run, 10 textures, 0 changed). |
| 3 | Real game with the clay rig streamed | **PARTIAL** | Toy figures, regular cut, game camera at the faceoff, plus a far-net view: **done**. The mid-ice still: my capture did not advance play, then the build break hit. Humanoid visuals (`visual-b2`) under clay: **not done** (blocker). |
| 4 | Readability | **PARTIAL** | From the valid stills only (below). The 10/24/40 m puck series and the 60 m team test were never captured. My first attempt failed because A's puck simulation resets the puck every tick; moving it while the game is paused works. The second attempt hit the blocker. |
| 5 | Frame cost | **PASS (unconfirmed, two editors up)** | Table below. |
| 6 | What device A must add | **PASS** | Section below. |

R-0016 items folded in:
- **Item 1.** `M_Ice` has the new scalar **`GoalLineOffsetCm`** (default 400). It replaces the constant `400.0` in both the colour (`IceColour+Lines`) and roughness Custom nodes; all other inputs are wired as before. I added it *after* the proof captures. At 400 the arithmetic is the same, but it was **not re-measured**, because no PIE was possible.
- **Item 2.** Every clay test and every frame pair hid `Rink`'s `SkyLight_0` and `DirectionalLight_0` in the PIE world first (`clayload.py`). The L2 halves of the frame pairs did too. The Clay-0 proof stills did not, because they compare the masters under identical conditions.
- **Item 3.** Nothing in the rig must stay unscaled. `Key_P3_Clay` is a **directional** light, so position scale does not affect it and its intensity must **not** be multiplied by scale²; A's code only scales local lights, which is correct. The three `Fill_` spots sit at ±56 m (rims) and -52 m (bounce), so they scale correctly with the rink. The depth-of-field focal distance (7 m) is a camera distance and must **not** scale; it is in the PPV, which A's code does not touch.

## How the masters read `MPC_Look`

The original graphs are untouched; the `MPC_Look` reads are added after them. Every addition sits behind a Custom node of the form `if (Clay <= 0.0) return Orig;`. At `Clay` 0, base colour, roughness and emissive are the original expression, and the normal is (0,0,1), which is what an unconnected tangent-space normal is.

| Master | Base colour | Normal | Roughness | Emissive |
|---|---|---|---|---|
| `M_Toon`, `M_PropCel`, `M_SkaterTB2` | desaturate 20 % × `LookTint` | thumb-dent normal, triplanar in **local space** so it does not swim when a figure moves. Tile `ClayTile`, strength `ClayBump`. | → `ClayRough` | cel output × `ClayFill`, plus a **felt rim** |
| `M_Ice` | desaturate 15 % × `LookTint` | same normal at 4 × tile, ½ strength (hand-smoothed clay) | → 0.5 | unchanged (ambient ice fill) |
| `M_Trim` | × `LookTint` | none | none | **unchanged**, so trims, beacon and puck halo keep A's values |

The felt rim is Fresnel (exp 2.5) × felt fibre (triplanar) × `FeltRim` × `LookTint` × lerp(base, white, 0.5). It is gated by the material's own `RimStrength` / 0.6, so the puck and the dark room (`RimStrength` 0) get none. The materials do not know the stencil, so `RimStrength` is the proxy for "stencil-1 surface".

Rig values (in `LookParams_P3_Clay`, not in the MPC defaults): `Clay` 1, `ClayBump` 2.0, `FeltRim` 0.5, `ClayFill` 0.15, `ClayTile` 25, `ClayRough` 0.85, `LookTint` (1, 0.94, 0.84).

**If two rigs are loaded, the last `BP_LookParams` to run BeginPlay or EndPlay wins.** One case matters: if the clay rig unloads *after* another clay-carrying rig loads, its EndPlay writes the defaults back. `LR_L2_Night` carries no `BP_LookParams`, so streaming L2 after P3 unloads correctly returns to `Clay` 0. Hiding the P3 rig without unloading it (what I did in PIE) does **not** reset the MPC: my test script sets it by hand.

## Proof that `Clay` 0 leaves the night look unchanged (measured)

Method:
- Real game, `l2_night`, PIE frozen at the faceoff (`slomo 0.0001`), HUD widgets removed.
- Five fixed views per cut: `game` (pawn camera), `over`, `obl`, `net`, `top`.
- **base**, **base2** and **base3** are three separate PIE sessions with the original masters (restored from backup and reloaded).
- **after** is the saved clay masters at `Clay` 0.

Columns:
- mean |d| = mean absolute RGB difference, in 8-bit levels.
- tileMax = the largest change in mean luma over a 4 × 4 grid of tiles.
- Each noise-floor cell is the worse of base2 and base3 against base.

| Cut | View | Noise floor mean\|d\| | Noise floor tileMax | Noise floor frame luma Δ | **After mean\|d\|** | **After tileMax** | **After frame luma Δ** |
|---|---|---:|---:|---:|---:|---:|---:|
| toy | game | 1.96 | 1.03 | 0.10 | 3.01 | 8.71 * | 0.13 |
| toy | over | 0.65 | 0.99 | 0.23 | 0.52 | 0.68 | 0.01 |
| toy | obl | 1.22 | 2.74 | 0.16 | 1.29 | 2.72 | 0.06 |
| toy | net | 0.57 | 0.66 | 0.11 | 0.52 | 0.35 | 0.05 |
| toy | top | 0.08 | 0.00 | 0.00 | 0.08 | 0.03 | 0.01 |
| hum | game | 4.46 | 34.92 | 1.68 | 2.19 | 11.49 | 1.12 |
| hum | over | 0.72 | 1.37 | 0.02 | 0.87 | 1.37 | 0.07 |
| hum | obl | 1.20 | 4.05 | 0.23 | 1.07 | 2.08 | 0.14 |
| hum | net | 0.33 | 0.27 | 0.02 | 1.08 | 8.42 * | 0.67 |
| hum | top | 0.08 | 0.00 | 0.00 | 0.08 | 0.00 | 0.00 |

\* A diff map shows these two are bots standing in different places at the freeze; the noise floor has the same effect in `hum game` (34.9).

The static geometry and the player figure show no change above the session noise: the ice, boards, nets, trims, room and the figure that does not move. Whole-frame luma moves by at most 0.13 levels in the toy cut. The `top` view, which has no figures and no temporal noise, differs by 0.03 levels at most. So **L2 changes by less than 1 level**.

Stills: `b9-l2-before`, `b9-l2-after`, `b9-l2-after-hum`.

## Paired frame numbers (unconfirmed, two editors up)

- All pairs: same PIE session, 14:34:43 to 14:36:52, toy regular cut, live play, game camera.
- `GetFrameTimings`, 20 s per run after a 5 s drop, about 90 samples, viewport realtime off.
- The other editor (port 8002) was up the whole time.
- "p3" means `LR_P3_Clay` streamed in, with `LR_L2_Night`'s 211 actors hidden (lights off, PPV disabled) but still loaded. Rink's own lights were hidden in every run.

| Run (in order) | GPU ms median | GPU p99 |
|---|---:|---:|
| p3 | 12.67 | 14.03 |
| p3, `r.DepthOfFieldQuality 0` | 12.18 | 13.39 |
| l2 | 12.42 | 13.79 |
| p3 | 12.37 | 14.58 |
| l2 | 12.09 | 13.47 |

- **Clay against night: about +0.25 ms** (12.37 / 12.67 against 12.09 / 12.42).
- **The depth of field alone: about 0.5 ms**: 12.67 → 12.18 in a back-to-back pair. In one `TriggerGPUProfile` on p3, the `DOF(Alpha=No)` node is **0.80 ms** (gather at 699 × 395) and `CoC=[-35;4]`.
- **Shadows:** `ShadowDepths` is **1.52 ms** with the one directional key. The night rig's four spots measured 0.84 to 1.0 ms in B1c's runs. A whole-scene directional virtual shadow map costs more than four local spots.
- These are device B numbers (N1X, x64 under emulation).

## Readability (what I could measure before the blocker)

| Test | Result |
|---|---|
| Puck findable | **Better than L2 on lit ice.** The near-black puck on cream "clay ice" is high contrast: 22 × 11 px at centre ice from the faceoff camera (`b9-clay-faceoff`), sharp because it sits near the focus band. There are no dark gaps between pools, the place where L2 loses the puck. Not measured at 10/24/40 m. |
| Blue vs orange | Separate in every clay still, including the orange pair at the far end. Not measured at 60 m. The warm tint and 20 % desaturation pull orange slightly toward the cream ice. Blue stays clear. |
| Far net with the depth of field on | Readable but softer. The beacon post is the clearest far mark. The net reads as a lavender box with a faint red edge. **This is where clay loses to cel:** no ink outline, the red frame washes out under the warm grade, and the depth of field blurs it about 4 px. The background blur is capped by the optics, not by a cvar: with a 100 mm sensor, f/1.4, 7 m focus and FOV 70, the circle of confusion at infinity is about 4 px (profiler `CoC=[-35;4]`). The near ice gets the stronger tilt-shift blur (up to 35 px). |
| Where clay loses to cel | Overall: no outline, so figures separate from the ice only by colour and the key shadow. The far net frame. The trims: their emissive values are unchanged at the same exposure, but against bright cream ice they read as thin gold lines, not glowing strips. The background is a black void (pass 2). |

## Is it clay?

**Honest verdict: not yet. Today it reads as a "warm matte toy / plaster tabletop", not as clay, and not as "cel without ink" either.**

What works:
- The tilt-shift depth of field. It is the strongest miniature cue: the far end goes soft like a macro photo of a model.
- The soft key shadows, the warm grade, and the cream ice.

What does not work yet:
- The thumb dents and fingerprints are visible only close to the camera. At game distance the figures look like smooth painted plastic.
- The felt rim is barely visible at FeltRim 0.5.
- The saturated jersey blue reads as plastic.
- The figures' shapes are A's capsule toys: hard cylinder edges, not rounded clay forms. No shader can fix that.

## Device A must add

1. Tier value **`p3_clay`** in `LookTier` / `game.look.tier` (`contracts/look-tiers.md`) and in `ULookTierSwitcher`.
2. Map path **`/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay`** (new file `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap`).
3. Cook-list line in `Game/Config/DefaultGame.ini`, `[/Script/UnrealEd.ProjectPackagingSettings]`: `+MapsToCook=(FilePath="/JamAssets/Rigs/LR_P3_Clay")`.
4. **Trims and beacon:** exposure stays at EV100 4.2, so the `Emissive` values are physically correct as they are. But the clay ice is about 3 stops brighter than the night ice, so the strips lose their glow. For P3, set `MI_Trim_Blue` / `_Orange` from 30 to about **10 to 15**, so they read as painted strips not neon; or keep 30 and accept a gold line. The **beacon at 40 stays as it is**: it is the most readable far mark. Since A spawns these, the change has to be per tier in A's code (or B adds `TrimGain` to `MPC_Look` in pass 2).
5. **Fix the UHT error above** so device B can run the game again.
6. **`RinkHalfLength`, `BlueLineFrac` and `GoalLineOffsetCm`** on the ice instance (R-0016): `GoalLineOffsetCm` now exists.
7. **Directional key:** keep `ApplyRigScale` to local lights only.
8. **Unload, do not hide:** `SetTier` must *unload* the previous rig, so that `BP_LookParams` EndPlay resets `MPC_Look`.

## What pass 2 needs

- Re-run the full readability set and the humanoid stills once A's Source builds. The scripts are ready: `stills.sh` (puck moved after pausing) and `clayload.py`.
- Re-measure the `Clay` 0 proof including `GoalLineOffsetCm` (`proofrun.sh after` and `proof.py`; the base sets are kept).
- Stronger clay at game distance:
  - a larger dent tile on figures (40 to 60 cm) with a mip-biased sample;
  - a cavity / AO darkening from the height map;
  - a softer, lower-saturation team palette for P3 (needs A, or `TeamColour` scaling in the MPC);
  - a visible felt rim (`FeltRim` about 1.5) on jerseys only.
- A depth-of-field focus that tracks the player. Today it is a fixed 7 m in the PPV. Tracking needs A's camera, or a tick in `BP_LookParams` writing the PPV focal distance.
- The tabletop: the warm wood "table" plane far below the floor, and a blurred workshop backdrop to replace the black void.
- `TrimGain` in `MPC_Look`, read by `M_Trim` (item 4 above).
- A cheaper key shadow (1.5 ms): a directional light with a shadow distance limited to the rink, or one large shadowed spot.

## What I could not verify

- Everything after the 14:39 merge: humanoid clay stills, mid-ice, puck at 10/24/40 m, teams at 60 m. My `p3toy_*` set was overwritten by an empty-rink run, so it is not filed.
- `GoalLineOffsetCm` on screen.
- `BP_LookParams` EndPlay through a real unload. I only hid the rig.
- Single-editor and 4090 frame numbers.
- Whether the old-binary editor I left running behaves differently once A's fix lands. After the fix, rebuild and restart the editor.

## Files (bytes on disk) and ledger rows

| File | Bytes | Before |
|---|---:|---:|
| `Game/Plugins/JamAssets/Content/Look/MPC_Look.uasset` | 2,716 | new |
| `Game/Plugins/JamAssets/Content/Look/BP_LookParams.uasset` | 92,534 | new |
| `Game/Plugins/JamAssets/Content/Look/Textures/T_Clay_Thumb_N.uasset` | 1,738,751 | new |
| `Game/Plugins/JamAssets/Content/Look/Textures/T_Felt_Fibre.uasset` | 969,183 | new |
| `Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap` | 32,591 | new |
| `Game/Plugins/JamAssets/Content/Look/M_Toon.uasset` | 31,139 | 14,344 |
| `Game/Plugins/JamAssets/Content/Look/M_Ice.uasset` | 37,335 | 24,554 |
| `Game/Plugins/JamAssets/Content/Look/M_Trim.uasset` | 14,393 | 11,433 |
| `Game/Plugins/JamAssets/Content/Arena/M_PropCel.uasset` | 33,620 | 16,871 |
| `Game/Plugins/JamAssets/Content/Skaters/M_SkaterTB2.uasset` | 43,287 | 27,598 |
| `status/B9-report.md` | this file | new |
| `devlog/shots/20261112-14450*-B-b9-*.png` and `.txt` | 7 stills (12.45 MB) plus captions | new |

Shipped-size estimates are analytic, before RDO: about 1.4 MB for the BC5 normal and 0.7 MB for the BC1 felt. The textures need `bFinalUsesRDO` in A's config (A-to-B-0004).

Rows to paste into `ASSETS.md`:

```
| Game/Plugins/JamAssets/Content/Look/MPC_Look.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | material parameter collection for look tiers: Clay, ClayBump, FeltRim, ClayFill, ClayTile, ClayRough, LookTint (lane B9) |
| Game/Plugins/JamAssets/Content/Look/BP_LookParams.uasset | other | 0.1 | original | UE 5.9 editor via MCP (Blueprint DSL) |  |  | 2026-11-12 | B | y | rig actor that writes MPC_Look on BeginPlay and restores defaults on EndPlay (lane B9) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Clay_Thumb_N.uasset | texture | 1.7 | ai-generated | Codex image generation (height map) + numpy height-to-normal |  | status/B9-report.md#prompts | 2026-11-12 | B | y | clay thumb-dent normal map, 1K tileable, BC5 + RDO Medium (lane B9) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Felt_Fibre.uasset | texture | 0.9 | ai-generated | Codex image generation |  | status/B9-report.md#prompts | 2026-11-12 | B | y | felt fibre mask, 1K tileable, BC1 linear + RDO Medium (lane B9) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | P3 clay rig: PPV_P3_Clay (tilt-shift DOF, warm grade, EV100 4.2, no toon blendables), Sky_P3_Clay, Key_P3_Clay (only shadow caster), Fill_P3_Clay_1..3 (shadowless), LookParams_P3_Clay (lane B9) |
```

Update the notes on the existing master rows: `M_Toon`, `M_Ice`, `M_Trim`, `M_PropCel`, `M_SkaterTB2` "+ MPC_Look reads (B9)"; `M_Ice` also "+ GoalLineOffsetCm (R-0016)". All stay at 0.0 MB.

## Prompts

Generator: a copy of `prep-reference/tools/gen.mjs`, changed to write square images to the scratch folder (`scratchpad/b9/gen/`; prompts in `jobs.json` there). No reference image was attached.

- Style line (both jobs): "Game texture for a stylised clay-miniature look. Square image 1024 x 1024, seamlessly tileable on both axes (the left edge continues into the right edge, the top into the bottom), flat orthographic top-down view, even diffuse lighting with no cast shadows, no vignette, no perspective, no objects, no text, no letters, no numbers, no logos, no watermark, no border."
- `T_Clay_Thumb_Height`: "A greyscale HEIGHT MAP of hand-modelled plasticine surface: soft shallow thumb dents 3 to 8 cm wide scattered irregularly, faint concentric fingerprint ridge lines inside some dents, a few gentle smears and tool marks. Mid grey (50 percent) is the flat surface, lighter is raised, darker is pressed in. Low contrast, smooth gradients, no hard edges, uniform density across the whole square so it tiles without visible repeats. Pure greyscale, no colour."
- `T_Felt_Fibre`: "A greyscale texture of close-up wool felt: dense fine matted fibres in random directions, soft fuzzy micro-texture, very small scale (hundreds of fibres across the square), slight mottling, uniform density with no larger features, so it tiles without visible repeats. Mid grey average, low to moderate contrast. Pure greyscale, no colour."

Post-processing (`scratchpad/b9/mk_tex.py`):
1. Both images: made seamless with a half-offset cross-fade.
2. Height map only: a Gaussian blur of 1 px, then the low frequencies removed (σ 64 px, which takes out the generator's baked light gradient).
3. Height map only: converted to a normal map at strength 0.25 per level.

## Stills (`devlog/shots/`)

- `20261112-144506-B-b9-clay-faceoff`: game camera at the faceoff, final values.
- `20261112-144506-B-b9-clay-farnet`: far net from 20 m, depth of field on.
- `20261112-144506-B-b9-clay-oblique`: mid-ice oblique, earlier tuning step.
- `20261112-144506-B-b9-clay-overexposed`: the first untuned light.
- `20261112-144507-B-b9-l2-before` / `-l2-after` / `-l2-after-hum`: the `Clay` 0 proof.

Scratch scripts, all re-runnable, in `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b9/`:
- `mats.py`: master wiring, idempotent.
- `goaloff.py`: adds `GoalLineOffsetCm`.
- `bp.dsl`, `bp_create.mjs`, `bpfin.py`: the Blueprint.
- `rig.py`, `bake.sh`: the rig and its values.
- `clayload.py`: PIE rig swap and Rink light hide.
- `tune.py`: live tuning.
- `proofrun.sh`, `proof.py`: the proof.
- `soak.mjs`: frame timings.
- `stills.sh`, `move.py`: stills and readability.
