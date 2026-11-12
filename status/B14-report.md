# B14 report: clay pass 2 and a first broadcast rig (device B)

Lane B14, device B, 2026-11-12, machine clock 16:45 to 17:22. I used the main-tree editor on port 8000 (pid 28036) and exited it through `ExitEditor` with no dirty package. A second editor (port 8002) shared the GPU, so there are **no frame numbers** in this report.

Housekeeping:
- I ran no git command (read-only `git show` only) and did no build.
- `Rink` was never saved, and nothing under `/Game/` or `Skaters/` was written. `LR_L2_Night` was not touched.
- Nothing is under `_Scratch/`.
- `jam.Cut` is back to `hockey.regular`.
- The part-1 checkpoint is `status/B14-checkpoint.md`.

## How the tiers and figures were selected

**`p3_clay`: through the game.** I made a scratch copy of `hockey.regular` with `game.look.tier = p3_clay` and `game.skater.visual = generated_b2`, and set it with `jam.Cut <path>` before PIE. The log shows:
- `JAMLOOK tier=p3_clay map=/JamAssets/Rigs/LR_P3_Clay.LR_P3_Clay scale=1.000 (rink 90.0 m) trim_emissive=12.0`
- `rig loaded: 3 local lights scaled`
- `persistent lights hidden: 2 actors`

In PIE, `MPC_Look` read Clay 1 and the rig's values. Both trim MIDs were at Emissive 12, and the beacons at 40. **Item 1: PASS.**

**`l2_night`: through the game,** with the same cartridge copy set to `l2_night`.

**Broadcast: by script.** There is no tier value for it yet. I started the `l2_night` game, then from Python:
- unloaded the night rig's streaming level (unload, not hide);
- streamed in `LR_P4_Broadcast` at the origin with scale 1;
- put `MI_Bcast_Glass` on A's `Glass` component (PIE only).

**Figures: by script.** The game spawns `BP_SkaterVisual_TB2` for every tier (B-to-A-0021 is still open), and Python cannot spawn actors during PIE. So each game visual was turned into the target figure in place:
- the skeletal mesh, AnimBP class and slot-0 material of that figure's own Blueprint (`SK_SkaterTB2_P3/_P4`, `ABP_SkaterTB2_P3/_P4`);
- a MID with the pawn's team colour.

Rclay is `SK_SkaterTB2` with `MI_SkaterTB2_Clay`. All stills are frozen (`slomo 0.0001`) at the opening faceoff.

## Part 1: clay pass 2

All master changes are clay-only. Every new node returns the old value when `Clay` ≤ 0, and every new parameter defaults to the old constant.

**Proof that the night tier is unchanged:** the `l2_night` overview after all of B14's master changes differs from B13's final overview by mean |d| 0.50 levels, tile max 0.33, and frame luma 0.06. That is within B9's session noise floor.

### 2. Clay at game distance

| What | Change | Old → new |
|---|---|---|
| `T_Clay_Dent_N` (new) | 512 px tileable normal map made by a numpy script: 64 soft elliptical thumb dents with a lip, 10 smears, fine grain. No generated image. BC5. | – |
| `M_Ice` (new parameters) | `ClayNormalTex` | default `T_Clay_Thumb_N` |
| | `ClayIceTileMul` | default 4 |
| | `ClayIceBumpMul` | default 0.5 |
| | `ClayIceRough` | default 0.5 |
| | `ClayScuffMul` | default 1 |
| | `ClayInlay` | default 0 |
| | `ClayInlayCm` | default 3 |
| `MI_Ice_Night` | ClayNormalTex | Thumb → `T_Clay_Dent_N` |
| | TileMul | 4 → 18 (dent tile 1 m → 4.5 m, about a 50 cm dent) |
| | BumpMul | 0.5 → 0.22 |
| | IceRough | 0.5 → 0.8 (matte) |
| | ScuffMul | 1 → 0.35 (the pass-1 blotches) |
| | Inlay | 0 → 1 |
| | InlayCm | 3 → 5 (a dark groove along every marking edge, so the lines read as inlaid strips) |
| `M_Toon` (new parameters) | `ClayNormalTex`, `ClayTileMul` 1, `ClayBumpMul` 1 | – |
| | `ClayFeltRail` 0, `ClayFeltBandCm` 12, `ClayFeltColour` | felt band under the rail top, roughness 1 |
| `MI_Toon_Board` | ClayNormalTex | → `T_Clay_Dent_N` |
| | TileMul | 1 → 6 (1.5 m) |
| | BumpMul | 1 → 0.3 |
| | FeltRail | 0 → 1 (dark red felt band under the rail) |
| `LookParams_P3_Clay` | FeltRim | 0.5 → 1.0 (lighter fuzzy edge on the figures) |

Verdict:
- At 6 m the dents read as soft thumb presses.
- At 24 m they are a faint, even irregularity.
- The markings stay crisp.
- Shimmer in motion was not checked, because I have stills only.

### 3. Arena without props, in clay (`LR_P3_Clay`)

New materials:
- `M_ClayBackdrop` (unlit, two-sided paper sweep: warm (0.55, 0.36, 0.22) at table height down to near black at 45 m, darker below the table) and `MI_Clay_Backdrop`.
- `MI_Clay_Table`, `MI_Clay_Wire` and `MI_Clay_Card` (`M_Toon`).
- `MI_Clay_Lens` (`M_Trim`, Emissive 18, warm).

Actors:
- **Table:** `Con_P3_Clay_Table` (150 × 110 m slab, top at -3 cm) and 4 `TableLeg`s.
- **Backdrop:** `Back_P3_Clay_Sweep` (cylinder, 110 m radius, -12 to 68 m).
- **Light stands:** `Con_P3_Clay_Stand_{Fill1,Fill2,Fill3,Key}_*`. Each stand is a wire pole, base, 3 legs, a cardboard head, a yoke and a lit lens. The key stand is a floor stand with a card softbox, on the side the directional key comes from.
- **Fills moved:** `Fill_P3_Clay_2/3` went from (±56, 0, 13) to (56, 18, 13) and (-56, -18, 13), aimed at centre ice, so no stand stands behind a net.

Constraints held:
- Nothing is inside the boards, and nothing is between the game camera and either net.
- All new meshes are shadowless, with no collision and no distance-field or indirect contribution.
- The only shadow caster is still the directional key.

The rig now has 45 actors.

### 4. Focus

The game camera at the faceoff is at (-16.3, 1.2, 3.5) m, pitch -17.65, FOV 70. The sensor is 100 mm, so f = 71.4 mm.

Distances from the camera:

| Target | Distance |
|---|---|
| Own player | 6.5 m |
| Rink centre | 16.6 m |
| Far blue line | 31.5 m |
| Far net | 57.4 m |
| Near (own) net | 24.7 m *behind* the camera |
| Bottom edge of the frame (foreground) | 5.5 m |

When the player attacks a net, the camera is 11–17 m from it.

**New settings:** focus 7 m → **11 m**, f/1.4 → **f/3.2**, plus depth blur **0.06 km / 6 px** (`PPV_P3_Clay`, not scaled with the rink).

Blur-circle diameter at 1920 px, old → new:

| Point | Old | New |
|---|---|---|
| Foreground | 2.8 | 2.8 |
| Player | 0.8 | 1.9 |
| Centre | 5.8 | 0.9 |
| Far blue line | 7.9 | 1.8 |
| Far net | 8.9 | 2.3 (+ about 2.9 depth blur) |
| Backdrop | 9.5 | 2.5 (+ about 4.3) |

A tracking focus is not needed for this fixed follow camera. If A ever wants one, it has to be the camera's `PostProcessSettings.DepthOfFieldFocalDistance` set each frame to the camera-to-player distance; an `MPC_Look` scalar cannot drive a post volume.

### 5. Figures and the puck

- Six P3 figures in both colours are readable at the game camera and at 24 m.
- Blue and orange are distinct under the warm light.
- The P3 figure reads as a hand-made clay toy. Rclay reads as T-B2 in matte clay paint, so **P3 stays the clay figure**.
- The puck reads clearly on clay ice near the game camera. At 24 m it shows with the halo; without the halo it is still a black dot on cream ice.
- The P3 stick is pale wood on cream ice and reads weakly. That is a figure asset, so I did not touch it.

### Rejected in part 1

- Ice tile 3 m with bump 0.5: it read as a crater field.
- Board bump 0.5: it read as golf balls.
- Any bump on the table slab: the local-space triplanar on a 150 m scaled cube made dunes (the same cause as SKI's giant fingerprint).
- FeltRim 1.2 (I kept 1.0).
- A disc-shaped key head: it read as a lollipop.

## Part 2: `LR_P4_Broadcast`

### 1. The rig

It is authored for a 90 × 45 m rink and has 176 actors.

| Prefix | Count | What |
|---|---:|---|
| `PPV` | 1 | Unbound. Exposure fixed at EV100 4.2, like the other rigs, so trims, beacon and halo keep their meaning. Bloom 0.45, lens flare 0.25, vignette 0.22, contrast 1.06, saturation 1.04, white temperature 6900. No blendables (no ramp, no outline) and no depth of field. |
| `Sky` | 1 | Cubemap, intensity 0.15, cool. |
| `Wash` | 8 | Shadowless rect lights in a 4 × 2 grid at 24 m (x ±11.25 / ±33.75, y ±11.25). 8 × 4 m sources, 4500 cd, attenuation 36 m, cool (236, 244, 255). |
| `Spot` | 2 | The only shadow casters: x ±22.5, z 28 m, 5500 cd, cones 34/46°. Hardened as in B1c: shadow resolution 0.5, no ray-traced, distance-field or contact shadows, attenuation 44 m. |
| `LookParams` | 1 | `BP_LookParams` with Clay = -1. |
| `Con` | 157 | Stepped stands (4 bowls of 18 boxes: 10 lower steps, a concourse wall, 7 upper steps), 5 roof trusses (chords plus 11 diagonals each), 2 lamp rails, 8 lamp boxes with lit faces, 2 key cans. All shadowless and outside the glass. |
| `Back` | 6 | Floor at -3 cm, plus 4 walls and a ceiling as a dark upper volume (`MI_Bcast_Volume`, unlit). |

Why only two shadowed lights: the figures and the puck need one grounding shadow under them, and each shadowed local light adds a virtual-shadow-map pass (B1c measured the night rig's four at 0.84 to 1.0 ms). The even wash comes from the shadowless grid.

**Measured on the ice** (top view, roof hidden for the measurement; sRGB luma):

| Area | Luma |
|---|---|
| Centre band | 210–235 |
| Near the side boards | 186–198 |
| End corners | 173–180 |
| Lines | crisp |

**There is no dark band** (the night tier has 43–53).

### 2. Materials

**The mechanism is `MPC_Look` `Clay` = -1, with no change on A's side.** The P4 rig's `BP_LookParams` writes it on BeginPlay, and its EndPlay writes 0 back.

- **`M_Toon`** gains `BcastToonBase`, `BcastToonRough` and `BcastEmissiveGate`, each gated `if (Clay > -0.5) return Orig`. In broadcast mode:
  - the rim and the emissive fill are off;
  - roughness comes from `BcastRough`;
  - the base can be replaced (`BcastBase`, `BcastBaseMix`);
  - the kick plate takes `BcastKick` and the rail band takes `BcastRail`.
- **`M_Ice`** gains `BcastIceBase`, `BcastIceRough` and `BcastEmissiveGate`. In broadcast mode:
  - the ice colour is `BcastIce` with light scuff (`BcastScuff` 0.12);
  - the markings keep their own colours;
  - roughness is 0.22 for ice, 0.35 for scuff and 0.35 for lines (soft sheen);
  - the ambient emissive is off.
- **Instances** (broadcast values only; inert under the cel and clay tiers):
  - `MI_Toon_Board`: base (0.86, 0.87, 0.89), yellow kick (0.9, 0.62, 0.04), light rail, roughness 0.3.
  - `MI_Toon_NetFrame`: BcastRough 0.3.
  - `MI_Toon_Puck`: BcastRough 0.35.
  - `MI_Ice_Night`: BcastIceRough 0.22, BcastScuffRough 0.35.
- **New instances:** `MI_Bcast_Stands`, `_Truss`, `_LampFace`, `_Volume` and `_Glass` (child of `MI_Glass`: glow 0.08, Fresnel 0.8, edge 2).

The only thing A has to assign is the glass (see "What device A must do").

### 3. Figures

- Six P4 figures in both colours read as players: helmets, pads and laced skates show at 6 m and 24 m. The orange team is distinct from the white ice, and the blue team is strong.
- **`JerseyLum` 0.25** on my own `MI_Bcast_SkaterP4` (child of `MI_SkaterTB2_P4`, with RimStrength 0) keeps the cloth shading:
  - 0.08 is flat;
  - 0.40 darkens the jersey without adding much.
- **No numbers on P4:** SetNumber is still a no-op in the P4 Blueprint, and the other lane has not landed it.
- Under broadcast the figure keeps `M_SkaterTB2`'s Emissive 0.5 from `SetBoost`. That is a cel fill, and only `Skaters/` can drop it.

### 4. Rejected in part 2

- Wash 15000 cd with keys 18000 cd: the ice clipped at 245 and the lines washed out.
- Ice roughness 0.08: the scuff mask turned the lamp reflections into dirty blotches.
- Stands at (0.045, 0.05, 0.065): they read as pure black.

## What device A must do

1. **`p3_clay` already works.** Trim emissive 12 is right. When B-to-A-0021 lands, `generated_b2` + `p3_clay` should map to `BP_SkaterVisual_TB2_P3`.
2. **Broadcast tier:**
   - value **`p4_broadcast`**;
   - map path `/JamAssets/Rigs/LR_P4_Broadcast.LR_P4_Broadcast`;
   - cook line `+MapsToCook=(FilePath="/JamAssets/Rigs/LR_P4_Broadcast")`;
   - trim emissive: the default of 30 reads fine as a team stripe;
   - figure class `BP_SkaterVisual_TB2_P4`.
3. **Per-tier look:** nothing to do. The masters switch on the rig's `MPC_Look` Clay value (1 clay, 0 cel, -1 broadcast). `SetTier` already unloads the previous rig, which is required.
4. **Glass:** assign `/JamAssets/Look/MI_Bcast_Glass` under broadcast and `MI_Glass` otherwise (B13). This is A's `Glass` component; today it is `/Game/Placeholders/M_Net`.
5. **The skater's emissive fill** under broadcast (`SetBoost` None = 0.5) is the skater lane's to decide.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 32,591 | 135,445 |
| `Rigs/LR_P4_Broadcast.umap` | new | 464,246 |
| `Look/M_Ice.uasset` | 45,547 | 61,137 |
| `Look/M_Toon.uasset` | 38,181 | 52,257 |
| `Look/MI_Ice_Night.uasset` | 12,588 | 16,697 |
| `Look/MI_Toon_Board.uasset` | 10,298 | 14,514 |
| `Look/MI_Toon_NetFrame.uasset` | 8,717 | 9,276 |
| `Look/MI_Toon_Puck.uasset` | 7,502 | 8,070 |
| `Look/Textures/T_Clay_Dent_N.uasset` | new | 486,757 |
| `Look/M_ClayBackdrop.uasset` | new | 16,257 |
| `Look/MI_Clay_Backdrop.uasset` | new | 6,574 |
| `Look/MI_Clay_Table.uasset` | new | 9,957 |
| `Look/MI_Clay_Wire.uasset` | new | 9,631 |
| `Look/MI_Clay_Card.uasset` | new | 10,625 |
| `Look/MI_Clay_Lens.uasset` | new | 7,787 |
| `Look/MI_Bcast_Stands.uasset` | new | 9,261 |
| `Look/MI_Bcast_Truss.uasset` | new | 9,512 |
| `Look/MI_Bcast_LampFace.uasset` | new | 7,353 |
| `Look/MI_Bcast_Volume.uasset` | new | 10,202 |
| `Look/MI_Bcast_Glass.uasset` | new | 7,799 |
| `Look/MI_Bcast_SkaterP4.uasset` | new | 14,754 |

`MPC_Look` and `BP_LookParams` are unchanged.

Ledger rows. The `LR_P3_Clay` row replaces the existing one. `M_Ice`, `M_Toon`, `MI_Ice_Night`, `MI_Toon_Board`, `MI_Toon_NetFrame` and `MI_Toon_Puck` keep their rows with the notes "+ clay pass 2 / broadcast mode (B14)" appended.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | P3 clay rig: PPV (focus 11 m f/3.2, depth blur), sky, directional key (only shadow caster), 3 fills, LookParams; tabletop, paper sweep, wire/card light stands with lit lenses (lanes B9, B14) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P4_Broadcast.umap | other | 0.5 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | broadcast rig: PPV (EV100 4.2, no blendables), sky, 8 shadowless rect wash lights, 2 shadowed key spots, LookParams (Clay -1), stepped stands, roof trusses, lamp boxes, dark upper volume (lane B14) |
| Game/Plugins/JamAssets/Content/Look/Textures/T_Clay_Dent_N.uasset | texture | 0.5 | original | Python script (numpy, scratchpad/b14/mk_dent.py) |  |  | 2026-11-12 | B | y | procedural tileable thumb-dent normal map, 512, BC5 (lane B14) |
| Game/Plugins/JamAssets/Content/Look/M_ClayBackdrop.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | unlit two-sided height-gradient backdrop master, no texture (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Clay_Backdrop.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance, warm paper sweep (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Clay_Table.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, tabletop (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Clay_Wire.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, wire stands (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Clay_Card.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Toon, cardboard lamp heads (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Clay_Lens.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Trim, warm lamp lenses (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_Stands.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | instance of M_Toon, dark stands (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_Truss.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | instance of M_Toon, roof truss and lamp boxes (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_LampFace.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | instance of M_Trim, lit lamp faces (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_Volume.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | instance of M_ClayBackdrop, dark upper volume (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_Glass.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | child of MI_Glass, clear broadcast glass (lane B14) |
| Game/Plugins/JamAssets/Content/Look/MI_Bcast_SkaterP4.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | child of MI_SkaterTB2_P4, JerseyLum 0.25, RimStrength 0 (lane B14) |
```

## Six stills for the operator (`devlog/shots/`)

1. `20261112-172129-B-b14-tiers-strip`: the same faceoff frame in night, clay and broadcast.
2. `20261112-170836-B-b14-clay-final-game` (or `-clay-after-close6` from 17:06): clay at the game camera.
3. `20261112-170836-B-b14-clay-final-over`: the clay miniature on its table.
4. `20261112-172126-B-b14-bcast-game`: broadcast at the game camera.
5. `20261112-172127-B-b14-bcast-wide24`: broadcast at 24 m, with stands, kick plate and even wash.
6. `20261112-172127-B-b14-bcast-close6`: P4 figures and the puck at 6 m.

Also filed:
- clay: `b14-clay-before-*`, `-after-*`, `-figs-p3-vs-rclay*`, `-puck-*`, `-final-*`;
- broadcast: `b14-bcast-own/far/over/attack/defend/puck24/top/jerseylum`;
- night: `b14-l2-game`.

## What I could not verify

- Frame cost: two editors were up. The broadcast rig has 8 shadowless rect lights covering most of the screen, plus 2 shadowed spots and Lumen reflections on glossier ice, so it needs a solo measurement.
- Shimmer and temporal behaviour in motion: I have stills only.
- `LR_P4_Broadcast` loaded through the game's switcher, and its rig scaling on large rinks (the switcher scales the wash lights by s² like every local light, which should be right).
- `BP_LookParams` EndPlay restoring Clay 0 after the broadcast rig unloads (the clay rig does the same and the mechanism is unchanged).
- Numbers on P4.
- The figure classes through the game.
- The clay trims at 12 against the new felt band, in motion.

## What the lead must decide

1. Clay or broadcast to push (the operator's choice, from the stills above).
2. Whether the `Clay` = -1 overload is acceptable, or a dedicated `MPC_Look` scalar `Bcast` should be added. The second needs one `BP_LookParams` variable, still no A change.
3. Send A the `p4_broadcast` request: tier value, map path, cook line, figure class and glass instance.
4. Ask the skater lane about the P3 stick contrast on clay ice, and about numbers plus the emissive fill on P4 under broadcast.
5. Paste the ledger rows.
