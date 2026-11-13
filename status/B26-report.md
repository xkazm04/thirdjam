# B26 report: clay tier pass 6: less mechanical reflections, the side band, the right pool, the walls (device B)

Lane B26, device B, machine clock 04:44 to 05:20 for the work (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 8a96a0b), editor on port 8002 (pid 33776, mine), left through `ExitEditor` at 05:19:03 (`LogExit: Exiting.`, PIE stopped, dirty list empty just before, process gone). Lane SK8's editor was never touched.

Clock note: stills and saves are stamped up to 05:19. Right after the exit `date` read 09:30 CEST; I do not know whether the machine slept or its clock was changed. This report was written straight after.

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (mtime 11-12 11:10, 18,966 bytes). `LR_P3_Clay` not changed this pass (294,972 bytes). `M_Toon` unchanged. `_Scratch/` does not exist; `Game/Saved/Screenshots/WindowsEditor/` is empty.
- `jam.Cut hockey.regular`, `jam.ScriptHuman 0` set back before the exit.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- Scratch (scripts, captures, backups in `bk/`): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b26`.

Method as in pass 5: real matches in PIE through the scratch cartridges, frozen at t 2.17 (faceoff) or 4.6 (zones cut). Every "after" still is from the saved assets.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | Reflections read as a comb | **PASS at the faceoff, PARTIAL in play**: the two rows of identical bars are gone; in play the middle ice still carries a field of soft vertical streaks, now irregular | `b26-game-before` / `b26-game-after`, `b26-game-play`, `b26-low-after` |
| 2 | Dark reflection band along the side boards | **PARTIAL**: softer and lighter, not gone | `b26-boards-ab` |
| 3 | Right key pool the palest part | **PARTIAL**: 186 against 140 before, 162 against 129 now; figures in the pool 3 to 6 % darker | `b26-low-before` / `b26-low-after` |
| 4 | Amber stain under a figure | **No sound way from the ice's side** (one line below) | none |
| 5 | Far wall reads as a lit room | **PASS** | `b26-game-after`, `b26-hero-low` |
| | Motion | **PASS**: no shimmer or popping | `b26-motion` |
| | Night and broadcast stability | **PASS**: no leak (with one caveat on the broadcast reference, below) | `b26-tier-stability` |

## 1. The reflections

`M_Ice`, `B22BulbRefl`: same node, three new vector parameters, new code. Master defaults reproduce lane B22's look exactly (no variation, no height change).

| Parameter (on `MI_Ice_Night`) | Value | What it does |
|---|---|---|
| `BRVary` | (0.7, 0.45, 0.6, 0.9) | per bulb, from a stable hash of string and bulb index: brightness 0.3 to 1.28, streak length ±45 %, a shift along the streak of up to 0.3 of its length; and the disc grows wider and softer with distance from the camera (x 1.9 at 40 m) |
| `BRHigh` | (0.8, 0.3, 220, 350) | from a camera at 3.5 m or higher: intensity x 0.8 and the weakest 30 % of the bulbs fade out; unchanged at 2.2 m or lower; a smooth blend between, by camera height |
| `BRDeep` | (1.0, 0.30, 0.05, 0.25) | where reflections add up the colour moves from (1, 0.40, 0.11) to a deeper gold instead of toward white |
| `PoolBase` (`B23IceEmis`) | 0.7 | away from the lamps the reflections keep 70 %; under a lamp 100 % plus `PoolBoost`: pooled under the lamps, fading between |

Result:
- **Game camera at the faceoff** (`b26-game-after`): no comb. Soft golden reflections of different strengths and lengths, fewer than before, densest under the lamps.
- **Game camera in play** (`b26-game-play`, `b26-motion`): calmer than pass 5, but the middle ice still shows many vertical streaks side by side. The bulbs hang in straight evenly spaced strings, and a reflection can only vary so far before it stops matching the bulb above it.
- **Low camera** (`b26-low-after`): full strength, irregular, deeper gold in the overlaps.

**Motion** (final values, 30 consecutive frames per run, 1/60 s game step on the held and following runs):

| Crop, held game camera in play | Mean \|d\| between consecutive frames |
|---|---:|
| Middle ice with reflections, right / left | 0.24 / 0.26 |
| Side bands, right / left | 0.23 / 0.22 |
| Near ice | 0.22 |
| Walls with the wash, side / far | 0.15 / 0.19 |
| Bulbs | 0.19 |
| Whole frame | 0.21 to 0.25 |

Following camera and the fast turn (3 deg per frame): judged by eye, nothing pops and the dropped bulbs do not flicker (the fade is by camera height, which does not change in a turn).

## 2 and 5. The side band and the walls

One change serves both. `M_ClayBarn` gains `B26BarnWash` (group `Rink wash (B26)`): an emissive of the wood's own colour times `WashColour` (1, 0.55, 0.28), strongest at the floor and fading with height (`WashHeightCm` 300). `WashK` is 0 in the master and **7 on `MI_ClayBarn_Wall` and `MI_ClayBarn_Post`**. No light was added. `M_ClayBarn` is used only by the clay rig's barn.

- **Walls:** the lower planks glow faintly warm above the boards and fall to dark toward the roof; the bulbs still stand against dark wood higher up.
- **Side band:** the ice now reflects warm wood there instead of black. The band is lighter and its edge softer at the game camera and looking back (`b26-boards-ab`); a darker zone along the side boards is still visible.
- I did not raise the ice's roughness or drop its specular near the boards: either would also remove the mirrored boards and the warm band of pass 5 there. It remains the next lever if the band must go completely.

Tried and rejected: `WashK` 30 (the whole barn glowed orange), 0.6 (invisible).

## 3. The right pool

Only the ice's colour inside the key pools changed (`PoolDim` 0.45 → 1.0, `PoolTint` (0.78, 0.66, 0.56) → (0.66, 0.54, 0.44)). The key lights are unchanged: all four are equal (4800 cd, same cones), the right pool is paler only because the low camera stands next to it.

| Low camera, sRGB luma | Before | After |
|---|---|---|
| Right key pool | 186.3, rgb (207, 183, 160) | 161.5, rgb (193, 156, 123) |
| Left near ice | 140.1 | 128.5 |
| Difference | 46 | 33 |
| Figure standing in the pool (two crops) | 97.4 / 74.6 | 94.7 / 69.8 |
| Jersey / helmet, game camera | 128.7 / 81.5 | 126.3 / 79.6 |

The pool is closer to the left, not level with it. Figures in the pool lose 3 to 6 %, at the game camera about 2 %: the darker ice bounces less light onto them. A darker tint (0.60, 0.49, 0.40) brought the pool to 156 and cost the figures more, so I stopped here.

## 4. The stain under a figure

No sound way from the ice: an opaque surface material cannot read the shadow or the lit scene, so the emissive reflection cannot know where a figure stands. Lane SK8's dark contact plane is the right fix: a translucent plane over the ice darkens the emissive too.

## Tier stability

After the last save (05:13 to 05:15), same cartridges and frozen frame.

| Still | Reference | Mean \|d\| | Tile max | Frame luma |
|---|---|---:|---:|---|
| Night, boards 8 m | this session's before-still | 0.46 | 1.36 | 60.36 → 60.37 |
| Night, game camera | this session's before-still | 1.62 | 23.62 | 135.94 → 136.04 |
| Night, 24 m | this session's before-still | 0.88 | 7.37 | 98.33 → 98.50 |
| Broadcast, boards 8 m | pass 5's after-still | 0.51 | 1.22 | 93.30 → 93.36 |
| Broadcast, game camera | pass 5's after-still | 1.16 | 11.48 | 148.01 → 147.99 |
| Broadcast, 24 m | pass 5's after-still | 0.71 | 6.04 | 122.64 → 122.69 |

- **Caveat.** This session's own broadcast before-stills were taken in the second match of a freshly started editor. The boards view differs from pass 5's after-still by 2.17 although the assets were identical (the committed pass 5 state), and from this session's after-still by 2.18; the after-still matches pass 5's by 0.51. The difference map is a broad lighting change over the board and the ice, not a shape of anything I touched. I therefore measure broadcast against pass 5's after-stills. Against this session's before-stills the broadcast numbers are 2.18 (boards), 1.76 (game), 1.05 (24 m).
- The numbers over 1 at the game camera are figure outlines (frozen t 2.19 against 2.18), as in every earlier pass.
- Every new ice path is inside the clay branch of `B22BulbRefl` / `B23IceEmis`; the barn material is not on the other tiers. **No leak.**

## Against `est-p3-barn.png`, point by point

| Estimate | Now |
|---|---|
| Open boards, red top rail | Reached (pass 5) |
| Golden reflections, irregular and soft | **Closer**: irregular, soft, deeper gold; still vertical streaks under straight strings, where the estimate has wide smears |
| Even warm ice, no pale pool | **Closer**: the right pool is 33 above the left, was 46 |
| Light ice up to the boards | **Closer**: the side band is softer, still visible |
| Barn reads as a warm lit room | **Reached** at the lower walls |
| Glowing rings with reflections | Not reached (zones are flat; unchanged this pass) |
| Dark contact under figures | Not from the ice (lane SK8) |

## Assets changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Look/M_Ice.uasset` | 135,803 | 138,819 |
| `Look/MI_Ice_Night.uasset` | 32,282 | 34,035 |
| `Look/M_ClayBarn.uasset` | 17,902 | 20,892 |
| `Look/MI_ClayBarn_Wall.uasset` | 10,240 | 11,890 |
| `Look/MI_ClayBarn_Post.uasset` | 9,937 | 12,384 |

No asset created. Compiled pixel shader instructions: `M_Ice` 1,375 → 1,440 (the additions run per bulb inside the existing loop: three hashes and a few multiplies for each of the 36 bulb tests), `M_ClayBarn` 241 after (before not recorded).

## Ledger rows (all five replace the existing rows of the same path)

```
| Game/Plugins/JamAssets/Content/Look/M_Ice.uasset | material | 0.14 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + scuff zones, scratches, frost (B13), clay pass 2 / broadcast mode (B14), tiles and NightWet (B15), glossy scratched clay-tier ice B19IceMask/Base/Rough/Normal/Spec/Emis (B19), B22BulbRefl analytic bulb-string reflection and ScratchNormFar footprint fade (B22), B23IceFx/Base/Emis key-pool compensation, contact shade and warm band at the board foot (B23), per-bulb variation, far softening, camera-height fade and deeper gold in B22BulbRefl (BRVary, BRHigh, BRDeep), PoolBase, Clay >= 0.5 only (B26) (138819 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B22 bulb reflection layout of LR_P3_Clay's 12 strings, ScratchNormFar 0.85; B23 ClayIceB23 1, contact 14 cm, board band 400 cm; B26 BRVary, BRHigh, BRDeep, PoolBase 0.7, PoolDim 1, darker PoolTint) (34035 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayBarn.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | barn wood master: world triplanar grain, dents, roughness (lane B19); B26BarnWash: faint warm emissive wash rising from the floor, off by default (lane B26) (20892 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Wall.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ClayBarn, plank walls (lane B19); WashK 7, WashHeightCm 300 (lane B26) (11890 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Post.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ClayBarn, posts, trusses, wire, shades (lane B19); WashK 7, WashHeightCm 300 (lane B26) (12384 bytes) |
```

## What device A must know

Nothing changed for A. One fact for any camera work: the reflections are fewer and weaker from a camera at 3.5 m or higher and full below 2.2 m (`BRHigh`), so a lower game camera will show more of them, by design.

## What I could not verify

- Frame cost; cook (no cook run in this lane).
- The saved assets in a fresh editor process. Evidence instead: saves returned true, the dirty list was empty, sizes changed on disk, the new names are in the files, and every after-still was taken after the save.
- Broadcast against a clean before-still of this session (see the caveat).
- The wide and large rinks this pass (pass 5 checked the wide cut; the new terms scale by the same factors or by camera height in plain centimetres).
- A game camera raised by `camera.end_height_m`: it would only stay in the "high" state.
- Whether the weaker reflections lower the puck's or the figures' readability numbers: not measured, judged by eye only.
- TSR at real 60 fps wall time.

## What the lead must decide

1. Whether the game-camera strength is right: `BRHigh.x` (0.8) and `.y` (0.3) on `MI_Ice_Night` are the two levers; 0.6 / 0.3 with far softening 1.2 was nearly empty ice at the faceoff, 1.0 / 0.15 was busy in play.
2. The side band: accept it softened, or a lane that fades the ice's own reflection toward the boards (at the price of the mirrored boards there).
3. The pool: accept 33 above the left, or go darker and accept more loss on figures standing in it.
4. Paste the ledger rows; cook check before pushing.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-051859-B-b26-hero-game.png`, `20261113-051859-B-b26-hero-low.png` (for the operator)
- `20261113-051859-B-b26-game-before.png`, `20261113-051859-B-b26-game-after.png`, `20261113-051859-B-b26-game-play.png`
- `20261113-051859-B-b26-low-before.png`, `20261113-051859-B-b26-low-after.png`
- `20261113-051859-B-b26-boards-ab.jpg` (side boards looking back), `20261113-051859-B-b26-boards-8m-ab.jpg`
- `20261113-051859-B-b26-motion.jpg`
- `20261113-051859-B-b26-tier-stability.jpg`
