# B27 report: clay tier pass 7: calm ice under large lamps, cool ice colour (device B)

Lane B27, device B, machine clock 09:37 to 10:40 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 71c7961 at the end, read only), editor on port 8002 (pid 14144, mine), left through `ExitEditor` at 10:30:33 (`LogExit: Exiting.`, PIE stopped, dirty list empty just before, process gone). The other editor was never touched. No extension of the hour was needed.

This pass follows the operator's direction of 09:47 (calm ice, a few large lamps), which replaced items 2 and 3 of the brief. Item 1 (cool ice) stayed.

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (mtime 11-12 11:10, 18,966 bytes). `M_Toon` unchanged. `_Scratch/` does not exist; `Game/Saved/Screenshots/WindowsEditor/` is empty.
- `jam.Cut hockey.regular`, `jam.ScriptHuman 0` set back before the exit.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- A garland mesh and two halo materials were built for the first brief, then deleted again when the direction changed: nothing of them is left on disk. `MI_ClayBulb_Warm` was re-saved in between and is restored byte for byte.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b27`.

## Verdicts

| Item | Verdict | Still |
|---|---|---|
| Calm ice: no per-bulb reflections on the playing surface | **PASS** | `b27-game-before` / `b27-game-after`, `b27-game-play` |
| Larger reflectors with a subtle effect | **PASS, with one limit**: ten large lamps light the rink; their effect on the ice is one broad sheen of at most +14 luma. From the game camera the lamps themselves are above the frame | `b27-low-after`, `b27-hero-low` |
| Bulb strings | **Removed** (they reflected in the ice whenever the camera framed them) | `b27-low-before` / `b27-low-after` |
| Even light, corners and end zones | **PASS** where measured | `b27-end-zone`, `b27-game-after` |
| Item 1: cool pale ice under warm light | **PASS** | `b27-game-after`, `b27-concept-ab` |
| Readability of puck, lines, figures | **PASS for the puck and the red lines, equal for blue lines, one figure position weaker in luma** (below) | `b27-readability` |
| Scratches fine and quiet | **PASS** (a little paler than before) | `b27-game-after` |
| Motion | **PASS** | `b27-motion` |
| Night stability | **PASS** | `b27-tier-stability` |
| Broadcast stability | **No valid before-still; a direct test shows no leak** (below) | none |

## What changed

**The rig `LR_P3_Clay` (112 actors).**

| Part | Before | After |
|---|---|---|
| Bulb strings | 12 | **0** |
| Key spots `Key_P3_Clay_1..4` (shadowed, x ±11.25 / ±33.75 m, centre line, 13 m) | 4800 cd, cones 16 / 58, specular 1 | **5200 cd, cones 16 / 62, specular 0** |
| New lamps `Lamp_P3_Clay_1..6` | none | spot lights at x -30 / 0 / +30 m on both side lanes (y ±13.5 m), 11.5 m high, 2600 cd, cones 20 / 66, the keys' colour, **shadowless**, no specular |
| Lamp shades `Con_P3_Clay_Shade_1..10` | 4 at scale 1 | **10 at scale 2.2** (six new ones over the new lamps) |
| Fills `Fill_P3_Clay_1..6` | unchanged | unchanged |

Only the four keys cast shadows, as before. No new shadow caster.

**Why the strings went completely.** I first hung four dim strings high along the walls. In play the camera still framed them at times, and every on-screen bulb put a small bright mark on the far ice through the screen-space reflection (about +70 luma, measured). Making the bulbs translucent did not help: the reflection reads the previous frame, translucency included. Any bulb the camera can see reflects, so none is left.

**The lamp discs** (`MI_ClayBulb_Lamp`): `Glow` 60 → 6, so a lamp reads as a warm source, not a white hole, and its mirror image at the low camera is small.

**The ice (`M_Ice`, values on `MI_Ice_Night`, all inside the clay branch).**

| Parameter | Before | After | Effect |
|---|---|---|---|
| `BulbReflB22` | 1 | **0** | the per-bulb reflection is off |
| `LampSheen` (new, in `B23IceEmis`) | none | (1.3, 0.14, 1.6, 1.0) | one broad soft warm sheen per lamp: the lamp's mirror image widened to several metres, capped |
| `LampA`, `LampB` (new, master values) | none | (1125, 3375, 1300), (3000, 1350, 1150) | where the ten lamps hang, in layout cm; scaled with the rink |
| `ClayIceBody` | (0.36, 0.38, 0.52) | **(0.28, 0.37, 0.86)** | blue enough to stay cool under the warm keys |
| `PoolTint` | (0.66, 0.54, 0.44) | (0.72, 0.72, 0.74) | the key pools are dimmed without being warmed |
| `ClayIceGloss` | 0.04 | 0.08 | softer mirror images |
| `ScratchLift` | 0.45 | 0.35 | quieter scratches |

Dormant, to be honest about it: the per-bulb reflection code is still in `B22BulbRefl` (now with a garland-aware layout, pin `BRGar`), and `MI_Ice_Night` still carries layout values from the garland attempt (`BRCount` 0). They describe nothing in the rig. **`BulbReflB22` must stay 0** unless strings come back and the layout is set again.

Compiled pixel shader: `M_Ice` 1,440 → 1,554 instructions (the sheen is ten lamp tests per clay ice pixel; the per-bulb loop of 36 tests no longer runs).

## Luma tables

**Ice colour, game camera at the faceoff (median sRGB).**

| Area | Before | After |
|---|---|---|
| Near ice | (185, 150, 122) | (187, 175, 187) |
| Mid ice | (145, 116, 95) | (167, 150, 158) |
| Side lane | (109, 71, 49) | (173, 154, 167) |

**Evenness: ice luma by place (median, so lines and scratches do not count).**

| View | Place | Before | After |
|---|---|---:|---:|
| Faceoff | near / mid | 155 / 120 | 178 / 154 |
| Faceoff | side lanes, left / right | 82 / 76 | 147 / 158 |
| Faceoff | far zone, centre / left / right | 181 / 140 / 121 | 182 / 141 / 123 |
| Faceoff | in front of the far net | 122 | 125 |
| Faceoff | spread | 76 to 181 | 123 to 182 |
| Attacking end | net front / slot | 189 / 191 | 198 / 204 |
| Attacking end | corners, left / right | 162 / 161 | 150 / 153 |
| Looking back | mid / far | 121 / 139 | 169 / 169 |
| Looking back | side lanes, left / right | 56 / 123 | 160 / 144 |

The corners at the attacking end are the one place that came out a little lower (about 150 against 204 in the slot).

**Readability, game camera at the faceoff: object luma against the ice beside it.**

| Object | Before | After |
|---|---|---|
| Puck core (`generated_b2` run) | 14 against 124 | 15 against 142 |
| Puck core (default figure run) | 15 against 124 | 14 against 142 |
| Player jersey / helmet, `generated_b2` | 125 / 80 | 114 / 81 |
| Player jersey / helmet, default figure | 102 / 84 | 106 / 87 |
| Orange figure at mid ice | 79 against 183 | 87 against 188 |
| Orange figure on the right wing | 82 against 118 | **109 against 112** |
| Orange figure in front of the far net | 106 against 219 | 110 against 212 |
| Red line | 71 against 103 | 109 against 143 |
| Blue circle line | 116 against 155 | 142 against 178 |

- The puck gains 18 luma of contrast.
- The red line keeps its contrast (32 → 34). The blue line loses a little (39 → 36) and some hue separation, since the ice is now bluish.
- The `generated_b2` jersey is 9 % darker (the keys have no specular any more; with it the jersey read 6 higher and a small glint appeared on the ice under the player). The default figure is slightly brighter.
- **The orange figure on the right wing now has the same luma as the ice beside it.** It reads by colour (orange on lavender), not by brightness.
- Not measured: jersey, helmet and puck in a corner and in front of the near net. No figure or puck stood there in the frames I had; the ice values above are what I can give for those places.

**The sheen.** Switched on and off in one frozen play frame at the game camera: at most **+14 luma** (cell means of a 9 x 16 grid), as one broad band across the middle ice rather than ten separate pools; nothing in it is finer than the lamps' spacing.

**Low camera.** Right key pool against left near ice: 162 against 129 before, 174 against 150 now (difference 33 → 25).

## Motion

Final assets, 30 consecutive rendered frames per run; 1/60 s game step on the held and following runs.

| Crop, held game camera in play | Mean \|d\| | Per-pixel temporal std, p99 |
|---|---:|---:|
| Middle ice, right / left | 0.25 / 0.26 | 2.4 / 3.7 |
| Side lanes, right / left | 0.28 / 0.28 | 2.7 / 2.5 |
| Near ice | 0.25 | 0.6 |
| Walls, side / far | 0.14 / 0.17 | 0.4 / 1.8 |
| Whole frame | 0.22 to 0.25 | |

Following camera and the fast turn (3 deg per frame): by eye, nothing shimmers or pops; there is nothing small left on the ice to shimmer.

## Tier stability

**Night**, after the last ice save, against pass 6's after-stills:

| Still | Mean \|d\| | Tile max | Area crop | Frame luma |
|---|---:|---:|---:|---|
| Boards 8 m | 0.38 | 1.31 | 0.40 | 60.37 → 60.38 |
| Game camera | 1.17 | 15.75 | 0.40 | 136.04 → 135.81 |
| 24 m | 0.59 | 2.23 | 0.43 | 98.50 → 98.45 |

No leak. The game-camera number is figure outlines.

**Broadcast.** Pass 6's after-stills are not a valid reference: another lane changed `LR_P4_Broadcast` and the P4 lab materials after them (commit 48598ad), and the frames differ by 13 to 50. I took no broadcast still before my own changes in this session. Instead, in one frozen broadcast frame I switched every clay parameter I changed on the ice back to its old value: the frame changed by 0.49 (game camera) and 0.49 (boards 8 m), against 0.58 and 0.48 for a repeat capture with nothing changed. The night tier, which uses the same ice material, is unchanged. I conclude no leak, but it is not the usual before/after proof.

The night stills were taken before the last rig save (removing the last four strings from `LR_P3_Clay`, which no other tier loads).

## Against the concept `path-p3-t3.png`

| Concept | Now |
|---|---|
| Open boards with a red top rail | Reached (pass 5) |
| Cool pale ice under warm light | **Reached** |
| Warm lit barn | Reached (pass 6), now lit by large lamps |
| Bulb garlands across the rink | **Given up on purpose** (operator's direction) |
| Warm reflection under every light | **Given up on purpose**: one soft sheen instead |
| Glowing rings | Not reached, not this pass |
| Dark contact under figures | Not from the ice (skater lane) |

## Assets changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 294,972 | 294,100 |
| `Look/M_Ice.uasset` | 138,819 | 144,762 |
| `Look/MI_Ice_Night.uasset` | 34,035 | 37,738 |
| `Look/MI_ClayBulb_Lamp.uasset` | 8,139 | 8,809 |

No asset created. Now referenced by nothing in the rig: `Arena/SM_Con_ClayBulbString`, `Look/MI_ClayBulb_Warm`.

## Ledger rows (all four replace the existing rows of the same path)

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.29 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, barn with calm ice (B27): PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky, 4 shadowed warm key spots (5200 cd, no specular), 6 shadowless lamps over the side lanes (2600 cd, no specular), 6 shadowless fills without specular, 10 large lamp shades, LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision (lanes B9, B14, B18, B19, B22, B23, B27) (294100 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.14 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice B19IceMask/Base/Rough/Normal/Spec/Emis (B19), B22BulbRefl analytic bulb-string reflection (B22, B26; garland-aware, switched off since B27), B23IceFx/Base/Emis key-pool compensation, contact shade and warm band at the board foot (B23), broad lamp sheen LampSheen/LampA/LampB in B23IceEmis (B27), Clay >= 0.5 only (144762 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B23 pools, contact, board band; B27 calm clay ice: BulbReflB22 0, LampSheen 1.3, cool body (0.28, 0.37, 0.86), neutral pool tint, gloss 0.08, ScratchLift 0.35) (37738 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBulb_Lamp.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ClayBulb, lamp discs (lane B19); Glow 6 for the large shades (lane B27) (8809 bytes) |
```

## What device A must know

1. The clay rig now has 16 local lights (4 shadowed, 12 shadowless) instead of 10. The switcher's generic scaling of local lights covers the six new ones.
2. Lamps hang at 11.5 m over the side lanes and at 13 m over the centre line, with shades 2.2 times the old size. A camera above about 10 m would meet them.
3. Nothing else changed for A.

## What I could not verify

- Frame cost, and a cook (none ran in this lane).
- Jersey, helmet and puck in a corner and in front of the near net (no such frame).
- Broadcast by a before/after pair (see above).
- The wide and large rinks this pass: the new lamps are rig actors and the sheen scales by the same factor as the rest; not run.
- The saved assets in a fresh editor process. Evidence instead: saves returned true, the dirty list was empty, sizes changed on disk, the new names are in the files, the after-stills were taken after the saves.
- TSR at real 60 fps wall time.

## Decisions for the lead

1. **No bulb strings at all.** They can only return if the clay ice stops using screen-space reflections, or as something the camera never frames.
2. **The lamps are not in the game camera's frame**; the operator sees them only from low or wide cameras. If he wants them in view during play, they must hang lower over the far end, and their mirror images will then be on the ice.
3. **Orange figures at equal luma with the ice** on the wings: accept colour-only separation, or darken the ice a little, or lift the figures' rim.
4. **Key specular**: off now (calm ice under the player, `generated_b2` jersey 9 % darker). One value in the rig to turn back.
5. Clean-up, whenever convenient: `SM_Con_ClayBulbString`, `MI_ClayBulb_Warm` and the dormant per-bulb reflection path in `M_Ice` are unused.
6. Paste the ledger rows; cook check before pushing.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-103104-B-b27-hero-game.png`, `20261113-103104-B-b27-hero-low.png` (default figure, for the operator)
- `20261113-103104-B-b27-game-before.png`, `20261113-103104-B-b27-game-after.png` (`generated_b2`, same frame)
- `20261113-103104-B-b27-game-play.png`
- `20261113-103104-B-b27-low-before.png`, `20261113-103104-B-b27-low-after.png`
- `20261113-103104-B-b27-concept-ab.jpg` (pass 6 and pass 7 at the low camera)
- `20261113-103104-B-b27-readability.jpg`
- `20261113-103104-B-b27-end-zone.png`
- `20261113-103104-B-b27-motion.jpg`
- `20261113-103104-B-b27-tier-stability.jpg` (night only)
