# B29 report: clay tier pass 9: opaque zone rings, every shipped rink, end lamps, pickups (device B)

Lane B29, device B, machine clock 11:26 to 11:50 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b`, read only), editor on port 8002 (pid 36996, mine), left through `ExitEditor` at 11:45:10 (`LogExit: Exiting.`, PIE stopped, dirty list empty just before, process gone). The other editor was never touched.

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (mtime 11-12 11:10, 18,966 bytes). Nothing of A's changed; the rings were tested by assigning my instances to A's ring components at run time. `M_Ice`, `M_Zone`, `M_Toon`, `LR_L2_Night`, `Props/`, `Skaters/` not touched. `handoff/A-to-B-0015.md` not edited (the answer for it is below).
- `_Scratch/` does not exist; `Game/Saved/Screenshots/WindowsEditor/` is empty. `jam.Cut hockey.regular`, `jam.ScriptHuman 0` set back.
- 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines in this session's log.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b29`.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | Opaque ring material for device A's zone rings | **PASS**: five instances on a small opaque lit master; all five kinds read at a glance; mirror image at most +9 luma | `b29-rings-game`, `b29-rings-10m`, `b29-rings-before-after` |
| 2 | Clay rig on every shipped rink | **PASS** on all eight; nothing to fix | `b29-rink-<name>` (eight) |
| 3 | End lamps larger | **PASS**: twice as wide, no visible mirror image | `b29-end-lamps` |
| 4 | Pickups on clay | answered below, no change | `b29-pickups` |
| | Motion (rings) | **PASS** | `b29-motion` |
| | Night stability | **PASS** | `b29-tier-stability` |

## 1. The zone rings

**New: `M_ZoneRing`** (opaque, default lit, 209 pixel shader instructions) and five instances.
- Base colour: the kind's colour, normalised and a little more saturated, as a matte clay body (`RingBody` 0.5, `RingSaturation` 1.6, `RingRough` 0.6).
- Emissive: the same colour at `RingGlow` 1.0, a little weaker on the side faces than on top. No bloom-size halo: the ring is exactly as wide as A's mesh.
- `ZoneColour` is the only value an instance sets; each `MI_Zone_<kind>_Ring` copies it from `MI_Zone_<kind>`:

| Instance | Colour (linear) | Reads as |
|---|---|---|
| `MI_Zone_boost_only_Ring` | (0.9, 0.8, 0.2) | yellow |
| `MI_Zone_bumper_Ring` | (1, 0.35, 0.1) | orange |
| `MI_Zone_fast_ice_Ring` | as `MI_Zone_fast_ice` | green |
| `MI_Zone_no_shoot_Ring` | as `MI_Zone_no_shoot` | red |
| `MI_Zone_slow_ice_Ring` | as `MI_Zone_slow_ice` | blue |

**Test.** A scratch cartridge on `p3_clay` with all five kinds (three circles, two rects); my instances put on A's five ring components from Python in the running match; one frozen frame, A's material against mine.
- Game camera and 10 m: every kind is a solid coloured ring that reads from the far end; the slow-ice blue is the weakest against the lavender ice but still clear (`b29-rings-game`).
- Mirror image: the ice just in front of the rings changes by +0.4 luma on average, +2 to +3 at the 95th percentile, **+9 at the 99th** (game camera, 10 m and low camera). Under the +15 asked.
- Motion, 30 consecutive frames, held game camera in play: rings 0.16 to 0.37 mean \|d\|, temporal std p99 at most 3.2. Fast turn by eye: no shimmer.

**The exact lines for device A (answer to `A-to-B-0015`).**
- Use on the ring components: `/JamAssets/Look/MI_Zone_boost_only_Ring`, `/JamAssets/Look/MI_Zone_bumper_Ring`, `/JamAssets/Look/MI_Zone_fast_ice_Ring`, `/JamAssets/Look/MI_Zone_no_shoot_Ring`, `/JamAssets/Look/MI_Zone_slow_ice_Ring`. In `RinkActor.cpp` that is the same `LoadLook` call with `MI_Zone_%s_Ring` for the ring's material.
- **No parameter needs to be passed.** `Circle` and `Strength` do not exist on this master; setting them on a dynamic instance does nothing and is harmless. A dynamic instance is not needed at all; the instance can be assigned directly.
- The material does not read the component scale or the UVs, so the ring no longer has to be built in the quad's local frame; A's current mesh works unchanged.
- A kind without a `_Ring` instance should fall back to what A does today.

## 2. The clay rig on every shipped rink

Each cartridge copied to scratch with `look.tier` set to `p3_clay`, run in a real match, two stills each. Sizes as the game logged them (`JAMRINK built`, `JAMLOOK scale`):

| Rink | Size as reported | Rig scale | Ice luma at the game camera (lowest to highest place) | Finding |
|---|---|---:|---|---|
| `hockey.arena.wide` | 140 x 70 m, corner 5.83 | 1.556 | 140 to 197 | even to the boards, walls and lamps follow |
| `hockey.arena.large` | 120 x 60 m, corner 5 | 1.333 | 137 to 193 | same |
| Marathon | 140 x 70 m, corner 5.83 | 1.556 | 136 to 196 | same |
| Cage | 70 x 35 m, corner 2, 4 obstacles | 0.778 | 145 to 200 | same; the four obstacles are lit like the boards |
| Pond | 60 x 30 m, corner 3, goal 6 m | 0.667 | 143 to 210 | same |
| Blind Side | 90 x 45 m, corner 3.75 | 1.000 | 139 to 199 | the regular rink |
| Power Play | 90 x 45 m, corner 3.75 | 1.000 | 138 to 199 | the regular rink (four against three) |
| Sniper | 90 x 45 m, corner 3.75 | 1.000 | 138 to 199 | the regular rink |

- No dark end, no wall cutting a rink, no lamp over the ice at playing height on any of them. On the smallest rink (Pond) the lamps hang lowest; by the rig scale of 0.667 that is side lamps at about 7.7 m, keys at about 8.7 m, and the decorative end shades at about 3.7 m, about 3 m outside the end boards (computed, not measured).
- The lamp sheen uses the same scale factor as the rig. I did not isolate it per rink; nothing in the stills looks displaced.
- Nothing needed fixing in the rig or the ice material, and nothing here is A's to fix.
- The luma boxes are the regular rink's screen regions, so on other sizes they fall on slightly different places; they are a check for dark areas, not a like-for-like table.

## 3. End lamps

The six decorative shades beyond the end boards: scale 1.6 → (3.2, 3.2, 2.0), height 5.5 → 5.6 m. From the faceoff the three far ones are now about twice as wide in the frame (`b29-end-lamps`). The far ice under them changed by +0.5 luma on average and +7 at the 99th percentile against pass 8; no mirror image is visible.

## 4. Pickups on clay

**How they read.** Each pickup is a small prop floating over a flat tan base ring. On the pale ice the base is close to the ice in tone and barely marks the spot; the prop reads by its own colour (yellow spool, purple shapes) but small, and nothing glows. From the far end a pickup is a few pixels. They do not read as the concept's glowing amber capsules.

**Whose assets.** All B's, in the content plugin:
- meshes `JamAssets/Props/SM_Pickup_Base` and `SM_Pickup_<kind>` (`speed`, `precision`, `big_puck`, `heavy_puck`, `custom`, `reverse_other`), plus `SM_Pickup_Token`;
- materials `JamAssets/Props/MI_PickupProp_<kind>` on the props (textured) and `JamAssets/Look/MI_Pickup_Base` (master `M_Pickup`) on the base.
None of it is an engine shape. Device A only places them.

**What B could do with materials alone** (not done; `Props/` was not in this lane):
- the base as a low amber clay ring with the same soft emissive as the zone rings (a clay branch in `M_Pickup` or a clay instance of `M_ZoneRing`), so every pickup has one warm marker on the ice;
- the props with a warm emissive rim and a stronger kind colour on clay, so they read as lit objects rather than painted ones;
- both kept under the same limit as the rings (mirror image under +15 luma, no halo).
What materials cannot do is turn the props into the concept's capsule shape; that is a mesh change.

## Tier stability

**Night**, before (11:28) against after the last save (11:43):

| Still | Mean \|d\| | Tile max | Area crop | Frame luma |
|---|---:|---:|---:|---|
| Boards 8 m | 0.50 | 1.56 | 0.52 | 60.61 → 60.60 |
| Game camera | 1.29 | 11.97 | 0.71 | 136.49 → 136.58 |
| 24 m | 0.68 | 5.41 | 0.46 | 98.78 → 98.79 |

No leak; nothing the night tier uses was changed in this pass.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Look/M_ZoneRing.uasset` | new | 14,892 |
| `Look/MI_Zone_boost_only_Ring.uasset` | new | 7,380 |
| `Look/MI_Zone_bumper_Ring.uasset` | new | 7,203 |
| `Look/MI_Zone_fast_ice_Ring.uasset` | new | 7,253 |
| `Look/MI_Zone_no_shoot_Ring.uasset` | new | 6,742 |
| `Look/MI_Zone_slow_ice_Ring.uasset` | new | 7,142 |
| `Rigs/LR_P3_Clay.umap` | 318,467 | 318,467 (six actors rescaled) |

## Ledger rows

Six **new** rows; the rig row **replaces** the existing one.

```
| Game/Plugins/JamAssets/Content/Look/M_ZoneRing.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | opaque lit master for the clay zone rings (device A's ring mesh): kind colour as a matte clay body with a soft emissive (lane B29) (14892 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_boost_only_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing, colour of MI_Zone_boost_only (lane B29) (7380 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_bumper_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing, colour of MI_Zone_bumper (lane B29) (7203 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_fast_ice_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing, colour of MI_Zone_fast_ice (lane B29) (7253 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_no_shoot_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing, colour of MI_Zone_no_shoot (lane B29) (6742 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_slow_ice_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing, colour of MI_Zone_slow_ice (lane B29) (7142 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.32 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light on the engine's small gray cube (B28), 4 shadowed warm key spots (5200 cd, no specular), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29) (318467 bytes) |
```

## What I could not verify

- The ring instances loaded by A's own code (I assigned them from Python; the values in the stills were set on dynamic instances and then saved as the master's defaults, so the saved assets were not re-shot).
- Rings on a rink of another size, and a ring with a figure standing on it.
- The lamp sheen's position per rink as a measurement.
- Frame cost; a cook (six new assets).
- The saved assets in a fresh editor process (saves returned true, dirty list empty, files on disk).
- My overview camera on the Pond rink sat inside an end shade; that still is not filed.

## Decisions for the lead

1. Answer `A-to-B-0015` with the lines above (I did not edit the handoff file) and let device A switch the ring components to `MI_Zone_<kind>_Ring`.
2. Slow-ice blue on lavender ice is the weakest of the five; a darker or more violet blue is one colour value on its instance, if the operator finds it faint.
3. Pickups: a materials lane on `Props/` and `M_Pickup` (amber base ring, emissive rim on the props), or a mesh lane for capsules, or leave.
4. Paste the ledger rows; cook check before pushing.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-114529-B-b29-hero-game.png`
- `20261113-114529-B-b29-rings-game.png`, `20261113-114529-B-b29-rings-10m.png`, `20261113-114529-B-b29-rings-before-after.jpg`
- `20261113-114529-B-b29-rink-wide.jpg`, `-rink-large.jpg`, `-rink-marathon.jpg`, `-rink-cage.jpg`, `-rink-pond.jpg`, `-rink-blind-side.jpg`, `-rink-power-play.jpg`, `-rink-sniper.jpg`
- `20261113-114529-B-b29-end-lamps.jpg`
- `20261113-114529-B-b29-pickups.jpg`
- `20261113-114529-B-b29-tier-stability.jpg`
- `20261113-114529-B-b29-motion.jpg`
