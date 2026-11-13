# B28 report: clay tier pass 8: figures against the ice, lamps in the frame, sky light, clean-up (device B)

Lane B28, device B, machine clock 10:44 to 11:25 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b`, read only), editor on port 8002 (pid 33452, mine), left through `ExitEditor` at 11:16:51 (`LogExit: Exiting.`, PIE stopped, dirty list empty just before, process gone). The skater lane's editor was never touched.

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (mtime 11-12 11:10, 18,966 bytes). `LR_L2_Night` is byte for byte unchanged. `LR_P4_Broadcast`, `M_Toon`, `Skaters/` not touched. `_Scratch/` does not exist; `Game/Saved/Screenshots/WindowsEditor/` is empty.
- `jam.Cut hockey.regular`, `jam.ScriptHuman 0` set back before the exit.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b28`.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | Figures against the ice, both teams, every position | **PASS for standing figures at the eleven positions measured** (every jersey 27 or more from the ice beside it). Lying figures and cast shadows: not shown | `b28-wings-before` / `b28-wings-after` |
| 2 | Lamps in the game camera's frame | **PARTIAL**: three shades are in the frame over the far end, with no visible mirror image; they read small | `b28-lamps-in-frame`, `b28-hero-game` |
| 3 | Sky light costs | **PARTIAL**: clay rig done within the limit; night rig tried and left unchanged; the saving is not realised yet (below) | `b28-skylight-ab` |
| 4 | Clean-up | **DONE**: both assets deleted, no referencer | none |
| 5 | Night tier question | answered below | `b28-night-check-faceoff`, `b28-night-check-play` |
| | Motion | **PASS** | `b28-motion` |
| | Night stability | **PASS** | `b28-tier-stability` |

## 1. Figures against the ice

**How figures got to the positions.** Two scratch cartridges with the spawn points moved (default figure `humanoid_b1`): blue and orange on both wings, in a near and a far corner, in front of both nets; teams swapped between the two. One camera 12 m behind each position at the game camera's height and pitch, frozen at the faceoff frame.

**How I measured.** Jersey = mean luma of the team-coloured pixels of the figure (for orange only the brighter orange, so this is stricter than pass 7's box mean). Ice = median luma of the ice in a ring around the figure.

**What was wrong.** Two things, both from the lights:
- The six side lamps had narrow beams 30 m apart, so the lanes between them were dim (ice 112 to 132 at x ±8 to ±15 m).
- The horizontal fills lit the figures' sides more than the ice, so orange jerseys near the boards were as bright as the ice.

**What changed (rig `LR_P3_Clay`, and one ice value).**

| Part | Before | After |
|---|---|---|
| Side lamps `Lamp_P3_Clay_*` (shadowless, no specular, 11.5 m, y ±13.5 m) | 6 at x -30 / 0 / +30 m, cones 20 / 66, 2600 cd | **8 at x ±11.25 / ±33.75 m, cones 50 / 75, 2100 cd** |
| Side fills `Fill_P3_Clay_1..2` | 2500 cd | 1100 cd |
| End fills `Fill_P3_Clay_3..4` | 2000 cd | 900 cd |
| Wall washes `Fill_P3_Clay_5..6` | 5000 cd | 2500 cd |
| Key spots (the only shadow casters) | unchanged | unchanged |
| `ClayIceBody` on `MI_Ice_Night` | (0.28, 0.37, 0.86) | (0.32, 0.42, 0.97) |
| Lamp sheen in `M_Ice` | 10 lamps | 12 lamps, same strength |

Twelve lamps light the ice now (4 keys, 8 side lamps), two more than the "6 to 10" of the direction; three per lane could not light the lanes evenly.

**Luma by position: jersey / ice beside it (difference).**

| Position | Orange before | Orange after | Blue before | Blue after |
|---|---|---|---|---|
| Faceoff | not measured | not measured | 63 / 171 (-108) | 69 / 188 (-119) |
| Left wing | figure not found | 119 / 175 (-55) | 59 / 186 (-126) | 48 / 174 (-125) |
| Right wing | 123 / 132 (-9) | 112 / 180 (-67) | 49 / 112 (-63) | 52 / 180 (-128) |
| Near corner | 124 / 119 (+4) | 140 / 168 (-27) | not measured | not measured |
| Far corner | 119 / 129 (-10) | 118 / 168 (-50) | not measured | not measured |
| Near net front | 155 / 202 (-47) | 144 / 204 (-60) | 94 / 201 (-107) | 81 / 204 (-122) |
| Far net front | 166 / 201 (-35) | 156 / 202 (-46) | 87 / 201 (-114) | 74 / 204 (-130) |

- Every measured position is now 27 or more from the ice. The smallest is orange in the near corner (-27).
- Figures are a little darker than in pass 7, by design: the player's jersey at the faceoff (pass 7's crop) 106 → 87, helmet 87 → 94. Puck core 14 against ice 142 → 15 against 158.
- Not measured: blue in the corners, orange at the faceoff and at mid ice (three skaters per team, two cartridges).

**Ice evenness (median luma).**

| View | Place | Before | After |
|---|---|---:|---:|
| Faceoff | near / mid | 177 / 154 | 193 / 171 |
| Faceoff | side lanes at centre ice, left / right | 148 / 158 | 139 / 138 |
| Faceoff | far zone, centre / left / right | 181 / 141 / 126 | 199 / 182 / 179 |
| Faceoff | in front of the far net | 125 | 152 |
| Attacking end | corners, left / right | 150 / 153 | 172 / 174 |
| Attacking end | net front / slot | 199 / 205 | 202 / 206 |

Near ice colour is now (201, 191, 201): paler than pass 7's (187, 175, 187), still lavender-grey.

**Shadows and lying figures, plainly.**
- A figure in a side lane has no clearly visible cast shadow, before or after. Only the four keys on the centre line cast shadows; in the lanes they arrive at a slant and the shadowless lamps wash the shadow out. What grounds the figure there is its dark legs and a small dark contact.
- I did not see a figure lying flat. From a standing figure: black trousers and skates are 10 to 35 against ice 170 to 204, so the lower body will separate anywhere; a lying orange jersey shows its lit top to the camera, which is the brightest-orange value in the table (27 to 67 from the ice). That is an estimate, not a measurement.
- A firmer shadow in the lanes would need the side lamps to cast shadows (eight more shadowed spots) or a blob shadow from the figure side. I did not add shadow casters.

## 2. Lamps in the frame

Six decorative shades (`Con_P3_Clay_Shade_13..18`, the same shade mesh, scale 1.6, no light of their own) hang 5.5 m high, 5 m beyond each end board, at y -12 / 0 / +12 m.
- From the game camera at the faceoff the three far ones sit at the top of the frame, each about 40 px wide (`b28-lamps-in-frame`).
- No mirror image of them is visible on the far ice. I could not isolate a number for it: the band under them holds lines and figures (median 164). In motion that band changes by 0.35 between consecutive frames.
- They read as small lamps, not large ones. Larger or lower shades are one value each in the rig; the mirror image must then be checked again.

## 3. Sky light

**What the rigs have.** Both `LR_L2_Night` and `LR_P3_Clay` use a specified cubemap, `/Engine/MapTemplates/Sky/DaylightAmbientCubemap`, with no real-time capture. Night: intensity 2.0, colour (150, 185, 255). Clay: 0.35, (255, 200, 150).

**Clay: changed.** The cubemap is now `/Engine/EngineResources/GrayTextureCube` (an engine resource, nothing new in the project), same intensity and colour. In one frozen frame, switching between the two:

| View | Mean \|d\| | Tile max |
|---|---:|---:|
| Boards 8 m (nothing animated) | 0.82 | 1.32 |
| Game camera | 3.62 | 40.2 |
| Game camera, repeat with nothing changed | 2.10 | 23.2 |

The game-camera frame holds the animated faceoff marker, so its own repeat noise is 2.1; the static view is the valid one and is under 1.0.

**Night: tried, not changed.** The same swap with the same values moves the night picture by 3.55 (boards), 4.13 (24 m) and 4.82 (game camera): brighter and less blue, because the daylight cubemap is itself blue. One calibration step went the wrong way (I passed the colour in the wrong space) and the 20 minutes were up. `LR_L2_Night` is untouched.

**What this buys today: nothing measurable yet.**
- The 16 MB cubemap stays in memory as long as `LR_L2_Night`, `LR_P4_Broadcast` or A's `Rink.umap` reference it.
- Whether the first-frame readback is gone on clay I cannot say: I may not take frame numbers, and a specified cubemap of any size is still processed once.

**Options for night.** (a) Finish the calibration of intensity and colour on the gray cube, two or three more steps of a few minutes each; (b) a small cubemap of B's own with the daylight map's blue gradient, if a way to build a cube asset inside the editor is found (not explored).

## 4. Clean-up

Deleted, after checking referencers in the whole project (none for the mesh; the material was referenced only by that mesh):
- `Arena/SM_Con_ClayBulbString.uasset` (86,474 bytes)
- `Look/MI_ClayBulb_Warm.uasset` (9,085 bytes)

The dormant per-bulb path in `M_Ice` is left as asked.

## 5. The night tier

Plainly: **`l2_night` has no small lights and no small reflections on the ice.** There are no bulbs, and the ice shows no point or streak reflections at the game camera, at the faceoff or in play.

What it does have is the opposite kind of contrast: a few large painted light pools with stepped hard edges on dark blue ice. Ice luma at the faceoff runs from 75 (side lanes) to 220 (pools), against 138 to 199 on clay now. A figure skating out of a pool goes from a white background to a dark blue one within a metre. If anything distracts on that tier it is this, plus the glowing trim on the boards.

What I would change, if he wants it calmer: lift the dark ice and lower the pool peaks so the range is roughly halved, and soften the pool steps. I would not add or remove lights.

## Motion

Final assets, 30 consecutive frames per run, held game camera from the faceoff into play at 1/60 s game step, and a fast turn at 3 deg per frame.

| Crop, held camera | Mean \|d\| | Per-pixel temporal std, p99 |
|---|---:|---:|
| The three end lamps | 0.17 | 1.4 |
| Far ice under them | 0.35 | 5.1 |
| Side lane | 0.29 | 2.1 |
| Near ice | 0.26 | 0.6 |
| Walls | 0.13 | 0.5 |

The middle ice crop (1.18) holds the skaters leaving the faceoff. Fast turn: by eye, nothing shimmers.

## Tier stability

**Night**, before any change (10:46) against after the last save (11:14):

| Still | Mean \|d\| | Tile max | Area crop | Frame luma |
|---|---:|---:|---:|---|
| Boards 8 m | 0.46 | 1.25 | 0.46 | 60.37 → 60.34 |
| Game camera | 1.19 | 14.81 | 0.62 | 136.14 → 136.09 |
| 24 m | 0.64 | 4.56 | 0.42 | 98.52 → 98.45 |

No leak. Broadcast was not run (P4 is out of the plan).

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 294,100 | 318,467 |
| `Look/M_Ice.uasset` | 144,762 | 144,303 |
| `Look/MI_Ice_Night.uasset` | 37,738 | 37,738 (one value changed) |
| `Arena/SM_Con_ClayBulbString.uasset` | 86,474 | deleted |
| `Look/MI_ClayBulb_Warm.uasset` | 9,085 | deleted |

No asset created. `M_Ice` pixel shader: 1,554 → 1,567 instructions.

## Ledger rows

**Delete** the rows of `Arena/SM_Con_ClayBulbString.uasset` and `Look/MI_ClayBulb_Warm.uasset`. The three below **replace** the rows of the same path.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.32 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light on the engine's small gray cube (B28), 4 shadowed warm key spots (5200 cd, no specular), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, B28), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28) (318467 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.14 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice B19IceMask/Base/Rough/Normal/Spec/Emis (B19), B22BulbRefl analytic bulb-string reflection (B22, B26; switched off since B27), B23IceFx/Base/Emis key-pool compensation, contact shade and warm band at the board foot (B23), broad lamp sheen for twelve lamps in B23IceEmis (B27, B28), Clay >= 0.5 only (144303 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B23 pools, contact, board band; B27 calm clay ice: BulbReflB22 0, LampSheen 1.3, neutral pool tint, gloss 0.08, ScratchLift 0.35; B28 body (0.32, 0.42, 0.97)) (37738 bytes) |
```

## What device A must know

1. The clay rig has 18 local lights now (4 shadowed keys, 14 shadowless). The switcher's generic scaling covers them.
2. Six decorative shades hang 5.5 m high, 5 m outside each end board. A camera that leaves the boards at the ends would meet them.
3. The clay rig no longer references `DaylightAmbientCubemap`. `Rink.umap`'s hidden sky light (A's) and `LR_L2_Night` (B's) still do.

## What I could not verify

- Frame cost, the first-frame readback, memory (no frame numbers; no cook in this lane).
- A figure lying flat, and blue figures in the corners, orange at the faceoff and mid ice.
- The mirror image of the end lamps as a number.
- The wide and large rinks: the new lamps and shades are rig actors and scale with the rig; not run.
- The saved assets in a fresh editor process (saves returned true, dirty list empty, sizes changed, after-stills taken after the saves).
- My "before" game-camera still with the default figure is not usable (its values do not match the faceoff view; I did not find out why); the evenness "before" row uses the wings cartridge's game-camera still instead, and the player's "before" values are pass 7's.

## Decisions for the lead

1. **Twelve lamps instead of ten.** Accept, or go back to three per lane and accept dimmer lanes between them.
2. **Figures are darker against a paler ice** (player jersey 106 → 87). This is what separates them; say if the figures should be lifted from the figure side instead.
3. **Shadows in the side lanes**: none to speak of. Shadowed side lamps (cost) or a contact blob from the skater lane.
4. **End lamps**: keep small, or larger / lower with a re-check of their mirror image.
5. **Night sky light**: finish the calibration (option a), and ask device A to clear `Rink.umap`'s sky-light cubemap; only then is the 16 MB gone.
6. **Night tier pools**, if the operator's remark was about that tier.
7. Paste the ledger rows; cook check before pushing (two assets deleted).

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-111726-B-b28-hero-game.png`, `20261113-111726-B-b28-hero-low.png`
- `20261113-111726-B-b28-wings-before.jpg`, `20261113-111726-B-b28-wings-after.jpg`
- `20261113-111726-B-b28-lamps-in-frame.jpg`
- `20261113-111726-B-b28-skylight-ab.jpg`
- `20261113-111726-B-b28-night-check-faceoff.png`, `20261113-111726-B-b28-night-check-play.png`
- `20261113-111726-B-b28-tier-stability.jpg`
- `20261113-111726-B-b28-motion.jpg`
