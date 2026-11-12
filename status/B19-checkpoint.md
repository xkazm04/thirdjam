# B19 checkpoint: item 1 (the ice) done, item 2 (barn and light) first pass in (device B)

Lane B19, device B, 2026-11-12, machine clock 22:50 to 23:14. Main-tree editor on port 8000. No git, no build. `/Game/Maps/Rink` not saved. No frame numbers (another editor may share the GPU).

**Still for the lead:** `devlog/shots/20261112-231332-B-b19-ck-ice-game` (faceoff camera, through the game on `p3_clay` with `generated_b2`, frozen at t 2.20).
Before: `20261112-231332-B-b19-ck-before-game`. Also `-ck-ice-24` (24 m, bulb reflections in the far ice) and `20261112-231333-B-b19-ck-hi` (high corner in the barn).

## Item 1, the ice: done (tuning continues with the light)

- `M_Ice` gains five Custom nodes behind `if (Clay < 0.5 || ClayIceB19 <= 0) return Orig` (group `Clay ice (B19)`): `B19IceMask`, `B19IceBase`, `B19IceRough`, `B19IceNormal`, `B19IceEmis`. Night (Clay 0) and broadcast (Clay -1) take the old value; the stability stills are taken (before) and are compared at the end.
- Two numpy-made textures: `Look/Textures/T_Ice_Scratch_M` (R long skate scratches, G short dense scratches, B soft undulation; 1024 = 4 m) and `T_Ice_Scratch_N` (grooves + undulation normal). Two layers (4 m and a 9.5 m layer rotated 23 deg) and a 37 m density field, denser in the slots, the centre and along the boards.
- Markings: the same `RinkHalfLength` / `BlueLineFrac` / `GoalLineOffsetCm` path A sets, redrawn with soft edges, red and blue, 15 % under the ice colour. A's own construction markings (faceoff dots and circles) are A's lit sections and stay as they are.
- Glossy: roughness 0.04 between scratches, 0.4 in them, 0.8 in the snow dusting along the boards. The clay PPV now overrides the reflection method to **screen-space reflections** (quality 80, max roughness 0.5): the bulbs, boards and figures reflect where they are on screen. `Wet` from A's `wet_ice` only polishes further on this tier; `ClayIceB19` is the tier-owned switch.
- `MI_Ice_Night`: `ClayIceB19` 1, `SnowAmt` 1, body (0.36, 0.41, 0.50), gloss 0.04.

Numbers at the faceoff camera (sRGB luma): near ice 180 (was 195), mid ice under a key 106 (was 211), blue jersey 83 (74), puck halo 146. The side band by the far boards is 23: too dark, the next step lifts it with the fills.

## Item 2 so far

`LR_P3_Clay` rebuilt and saved (105 actors): the table, sweep and stands are gone; plank walls, posts, trusses, roof, floor, 8 bulb strings (39 bulbs each), 4 lamp shades; 4 shadowed warm key spots (hardened like the night spots) and 4 shadowless warm fills replace the directional key. Next: wall and corner fill, bulb shape, then boards, zones, figures.
