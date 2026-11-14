# B41 report: the goal moment on clay and night, a lamp and a cord material for device A's pieces, the night rig's banks and fill, the clay check (device B)

Lane B41, device B, machine clock 00:49 to 01:21 (2026-11-14). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 6b9f3425, read only). Editor on port 8002 (pid 39888, mine, the only editor), left through `ExitEditor` at 01:19:09 (`LogExit: Exiting.`, no play session open, dirty list empty just before, process gone). No GPU hang in this lane; no restart.

Housekeeping:
- No commit, no push, no build, no cook. **No frame numbers.**
- Nothing under device A's paths written: `/Game/Arena`, `/Game/Maps`, `Game/Config`, `Game/Source` are unmodified in git. The two new instances were tried on device A's meshes in play sessions only.
- Console variables read back at the end: `r.Editor.Viewport.OverridePIEScreenPercentage 1`, `jam.rink.react.light 6000`, `jam.rink.react.loud 1`, `jam.ctrl.HandPuck 0`, `jam.ctrl.Drill 0`, `jam.ScriptHuman 0`, `jam.Cut hockey.regular`.
- Stills were taken with `OverridePIEScreenPercentage 0` (1920 x 1085).
- Scratch, with the night rig's backup in `bk/`: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b41`.

## Verdicts

| # | Job | Verdict | Still |
|---|---|---|---|
| 1 | The goal moment on clay and night | **DONE.** The fight is device A's point light against the ice, on both tiers. Numbers that keep it off the ice are tested on both tiers and listed for device A. Nothing of B's needed changing | `b41-goal-clay`, `b41-goal-night`, `b41-goal-clay-proposed` |
| 2 | A lamp instance and a cord instance | **DONE**: `/JamAssets/Look/MI_Trim_Lamp` and `/JamAssets/Look/MI_Toon_NetCord`, both shown on device A's meshes on clay and night | `b41-lamp-clay-night`, `b41-net-cords-clay-night` |
| 3 | The night rig: banks and a fill | **DONE**: banks clear of the pieces on three rink sizes; fill in; ice mean +0.01 % at the game camera, +0.24 % by the boards. The fill is uneven across rink sizes | `b41-night-rig-ab`, `b41-night-rinks` |
| 4 | Device A's pieces against the clay rig; clear zones for the stands | **DONE**: nothing of the rig crosses a piece; zones below | `b41-clay-arena-check` |

## 1. The goal moment

**How it was staged.** A real un-paused match with the scripted human standing at the shots drill's spot 10 m from the +x net; the goal is a `goal` event sent on the game's own event bus, so `NS_goal`, `MS_goal`, device A's lamp, its point light and the net's shake all answer as in play. The sim's score does not change, so there is no goal pause in these strips. The view is the game camera's, held still for the 130 frames so the ice can be compared frame to frame. I did not wait for a scored goal.

**What the present goal does to the ice** (median colour per tile over the pixels that are ice before the goal; figures passing through do not count):

| Tier | Frame | Mean ice shift (R, G, B) | Largest tile near the net | Ice pixels moved by more than 12 |
|---|---|---|---|---|
| Clay (peak 6,000 cd) | +4 | +4.9, -2.0, -2.6 | a figure crossing | 16.5 % |
| Clay | +10 | +3.4, -2.7, -3.5 | a figure crossing | 15.7 % |
| Clay | +30 | +4.6, +0.6, +0.5 | R +30 | 11.3 % |
| Clay | +60 | +5.9, +0.7, +0.7 | R +36 | 13.5 % |
| Clay | +108 | +0.3, -0.1, 0.0 | none | 2.2 % |
| Night (peak 1,200 cd) | +4 | +11.3, +0.2, -0.2 | luma +30 (R +52) | 24.1 % |
| Night | +10 | +9.8, +0.3, -0.2 | luma +29 | 23.0 % |
| Night | +30 | +9.2, +0.4, -0.2 | luma +28 | 23.0 % |
| Night | +60 | +11.2, +0.2, -0.2 | luma +30 | 27.0 % |
| Night | +108 | +1.6, -0.2, -0.2 | none | 4.3 % |

The view is from 10 m out, so two thirds of the ice in frame lies outside the light. From a camera inside the scoring end most of the visible ice is inside it, which is what the other lane's captures showed.

**Readability during the flash.**
- Puck (dark core against the ice beside it; from the first clay run, camera not held): -185 before; -181, -193, -190, -174 at +4, +10, +30, +60. Holds.
- Blue jersey (the human) against the ice: -172 before; -166, -156, -131 (+60), -147 (+108). The red light lifts the jersey by up to 41; it stays far from the ice.
- Orange jersey: -54 before. **Not measured during the flash**: every orange figure stood on a line or by the boards in those frames and my rule rejected them.

**What fights, in order.**
1. **Device A's point light against the ice.** It hangs 3.6 m over the ice, 3 m behind the goal line, with a 16 m radius. On clay it turns the end boards, the crease and the slot pink and puts a red streak on the ice under the net (the glossy ice mirrors the lit lamp and boards): a small bright reflection on the playing surface, which the standing direction rules out. On night the cel ramp turns the same light into a hard-edged pink patch about 8 m wide.
2. **The scorer's table's light** (same radius) paints a pink patch on the ice by the table on night. Seen in an early strip where the camera looked that way; not measured.
3. **One of B's end lamp shades turns red** on clay for the 108 frames (it hangs inside the light's 16 m). It reads as a second goal lamp above the boards; it is not on the ice. With the smaller radius below it stays as it is.
4. **`NS_goal`'s burst rises through the lamp** for about its first 15 frames and hides it; by +30 it is above the boards. Mild. I did not change `NS_goal`.
5. The net's shake and the burst do not fight each other.

**The numbers for device A, tested live on both tiers** (console variables and the lights' radius set in the play session, then put back):

| Setting | Now | Tested |
|---|---|---|
| Place of the scored end's light | over the net, 3 m behind the goal line, 3.6 m up (`jam.rink.react.loud 1`) | beside the lamp behind the end boards: x = 45.10 m, z = 3.0 m on the regular rink (`jam.rink.react.loud 0`) |
| `AttenuationRadius` (all three lamp lights) | 1600 | **350** |
| Peak, clay | 6,000 cd | **800 cd** (`jam.rink.react.light 1600` at that placement's half) |
| Peak, night | 1,200 cd | **320 cd** (the same setting) |
| Colour | (255, 36, 16) | unchanged |

Result with the tested numbers: mean ice shift 0.0 at +4 on clay (later frames only show figures crossing tiles), 0.0 to 0.1 in every channel on night; ice pixels moved by more than 12: 0.7 to 3.9 % on clay, 3.6 to 5.7 % on night (moving figures). The lamp, the burst and the net still mark the goal (`b41-goal-clay-proposed`). It is a quieter goal; the lamp material of job 2 is what carries it.

## 2. The two instances

| For | Asset path | File | Parent | Values |
|---|---|---|---|---|
| Slot `lamp` | `/JamAssets/Look/MI_Trim_Lamp.MI_Trim_Lamp` | `Game/Plugins/JamAssets/Content/Look/MI_Trim_Lamp.uasset` | `M_Trim` | `Emissive` 3, `TeamColour` (1.0, 0.04, 0.012), `BaseTint` (0.30, 0.02, 0.015) |
| Slot `net` | `/JamAssets/Look/MI_Toon_NetCord.MI_Toon_NetCord` | `Game/Plugins/JamAssets/Content/Look/MI_Toon_NetCord.uasset` | `M_Toon` | `TeamColour` (0.86, 0.84, 0.78), `GridCm` 0 (no pattern), `Emissive` 0.8, `Roughness` 0.7 |

- **The lamp's parent is `M_Trim`, not `M_Toon`.** `M_Toon`'s `Emissive` is a rim and fill term; `M_Trim`'s is a true emissive, the one the board trims glow with. The parameter is still called `Emissive`, so device A's code needs only the new path.
- **What the values read as** (the lamp alone, no point light):

| `Emissive` | Clay | Night |
|---|---|---|
| 0.5 or under | off: dark red | off: dark maroon |
| 3 (the instance's value) | lit: red | lit: saturated red |
| 8 to 12 | flashing: bright orange-red | flashing: hot orange-red with a small halo |
| 17 | peach, losing the red | orange-yellow |
| 30 | pale, a little halo | orange-yellow |

- Measured lamp luma (mean of the lamp-coloured pixels): night 54 at 3, 105 at 8, 154 at 17 (today's `MI_Toon_NetFrame`: 67 at 3, 130 at 17). Clay: 77 at 0.5, 86 at 3, 95 at 8 (today's instance: 82 to 87 from 3 to 17, no visible change).
- **Plainly:** on clay today's lamp does not answer `Emissive` at all and the new one does; on night today's lamp already brightens, and the new one brightens more and keeps its colour up to about 12. The halo round the lamp is small on both tiers: this is a lamp that visibly lights, not a light source. Whether that lets the point light drop to a third is device A's call after seeing it; with the radius above the question mostly goes away.
- Device A drives rest + 14 today, which lands at 17 (peach / orange-yellow). A peak of 12 (rest 3 + 9) keeps it red.
- **The cords:** plain off-white, no dashes; on night the cel outline still draws each cord.

## 3. The night rig

**The banks.** The two side banks were one 88 m mesh each. Each is now two pieces of the plain side bank mesh, scaled along the boards, leaving a gap:

| Side | Actors | Gap in layout metres | On Pond (x 0.667) | Pieces end at (Pond) | On the wide arena (x 1.556) |
|---|---|---|---|---|---|
| -y (boxes, table) | `Snow_L2_Night_Bank_3`, `_3b` (new) | \|x\| under 8.5 | 5.67 m | 5.26 m (fixed on every rink) | 13.2 m |
| +y (benches) | `Snow_L2_Night_Bank_4`, `_4b` (new) | \|x\| under 12.6 | 8.40 m | 8.03 m | 19.6 m against 18.73 m |

Checked from 7 m up on the regular rink, Pond and the wide arena: no bank crosses a piece (`b41-night-rinks`). `Arena/SM_Con_SnowBank_SideBench` is no longer used by the rig; the file is untouched.

**The fill.** Three shadowless rect lights, no specular: `Fill_L2_Night_Box` (x 0, -y side, 10.5 m wide), `Fill_L2_Night_Bench_1` and `_2` (x -7.5 and +7.5, +y side, 9 m wide). Each sits 30 cm outside the boards' inner face, 2.5 m up, facing straight away from the rink, 450 cd, 7 m radius, colour (170, 195, 255). A rect light lights only what is in front of it, so nothing inside the boards is in its reach by construction.

| Measure (regular rink, frozen frame) | Before | After |
|---|---|---|
| Ice mean luma, game camera (40 % of the frame) | 123.39 | 123.41 (**+0.01 %**) |
| Ice below the board, game height at the -y boards | 87.13 | 87.34 (**+0.24 %**) |
| Pieces above the board, same view | 80.9 | 94.4 |

- The clocks and the table's housing were black at 0 cd and read at 450. I tried 60 (no visible change), 600 and 2,000 (the table's housing blows out) before settling.
- **To know:** from 7 m up the pieces were never ink on black, with or without the fill; the fault is at game height, where only their tops and inner back walls show above the boards.
- **Uneven across rink sizes:** on Pond the fill makes a brighter pool with a curved edge on the box wall; on the wide arena it is weaker and the clocks go dark again. The lights scale with the rig; the boxes do not.
- The "before" at the boards is the saved rig with the three lights switched off live; the banks were already split in it.

## 4. Device A's pieces on clay, and the stands

- **Nothing of the clay rig crosses or hides a piece**: no lamp, shade, post or wall is within 13 m of the benches, boxes or table, and the end shades hang 4 m beyond the goal lights. The pieces stand on the lit apron floor.
- Device A's benches and boxes bind `MI_Toon_Board` to their `board` slot, so they carry the board's red band at 1.06 to 1.20 m and B36's wear. It reads as a trim line. Say if a plain instance is wanted.

**Clear zones for lane AR2, rink coordinates in metres, regular rink 90 x 45** (bounds read from the running game):

| Piece | x | y | Top |
|---|---|---|---|
| Team benches | 2.95 to 12.04, both signs (scale with the rink's length: centre 0.0833 L, length 0.1 L) | +22.72 to +24.41 | 1.90 |
| Penalty boxes | 0.74 to 5.26, both signs (the same metres on every rink) | -24.41 to -22.72 | 2.66 |
| Scorer's table | -0.70 to +0.70 | -24.48 to -22.75 | 2.89 |
| Goal lights | 45.20 to 45.86, both ends (0.2 to 0.86 m outside the end boards) | -0.36 to +0.36 | 2.78 |

**One sentence for AR2:** stands whose front edge is 4 m beyond the boards (|y| from 26.5 m) do not cross the benches, the boxes or the table, whose backs end at 24.48 m, and leave a 2 m aisle; keep |y| under 24.6 m free along the sides and x from 45.0 to 46.1 m free behind each net, and mind that anything lower than 2.9 m directly behind the table and 2.7 m behind the boxes is hidden by them from the game camera.

## Not verified

- A scored goal with the sim's goal pause: the strips are a staged event in running play.
- Orange jerseys against the ice during the flash; the puck inside the pink zone (it was 10 m out).
- The scorer's table's pink patch on the ice as a number.
- The tested light numbers on other rink sizes, and with `MI_Trim_Lamp` bound by device A's own code (I set `Emissive` by hand on my own instance; device A's flash curve was not driving it).
- Sound: `MS_goal` played; I did not judge it against the rest.
- A cook (two new instances, the night rig changed) and a packaged build.
- The saved assets in a fresh editor process (saves returned true, the dirty list was empty, the after-stills come from fresh play sessions after the saves).
- The night banks' "before" from above: I have no still of the crossing itself; the proof is the after-stills and the bounds.
- Cage, the large arena and Marathon on night.
- The night fill in motion (30 frames): not taken.

## For the other device

1. **Lamp:** bind slot `lamp` to `/JamAssets/Look/MI_Trim_Lamp.MI_Trim_Lamp` (file `Game/Plugins/JamAssets/Content/Look/MI_Trim_Lamp.uasset`). Scalar `Emissive`: 0.5 or under reads off, 3 lit (the instance's value, so your "rest" read stays 3), 8 to 12 flashing; from about 15 it loses its red. With your rest + 14 it peaks at 17: a `LampPeakEmissive` of 9 keeps it red.
2. **Cords:** bind slot `net` to `/JamAssets/Look/MI_Toon_NetCord.MI_Toon_NetCord` (file `Game/Plugins/JamAssets/Content/Look/MI_Toon_NetCord.uasset`).
3. **The point lights, tested on clay and night:** `AttenuationRadius` 350 on all three (now 1600); the scored end's light at `jam.rink.react.loud 0`'s place (beside the lamp behind the end boards, x 45.10 m, z 3.0 m on the regular rink); peak 800 cd on clay and 320 cd on night (`jam.rink.react.light 1600`); colour as it is. Measured: mean ice shift 0.0 on clay and 0.0 to 0.1 on night, against R +3.4 to +5.9 (tile +36) on clay and R +9 to +11 (tile luma +30) on night today.
4. **The night rig now has 9 local lights** (3 new rect lights, shadowless) and two more snow bank actors; your switcher's scaling took them without change.
5. Your benches and boxes inherit the worn board look and its red band through `MI_Toon_Board`.

## Decisions for the operator

1. **How loud a goal is.** Today: a red light on the crease and slot (pink ice for 108 frames). Proposed: the lamp, the burst and the net only, nothing on the ice. In between is possible (radius 350 with a higher peak lights the end boards and the lamp post, not the ice).
2. **The lamp's peak:** red at 12, or hot and pale at 17.
3. **The night fill:** 450 cd as saved, or lower; and whether its unevenness on Pond and the wide arena matters for a fallback tier.

## The next three evolutions, each with a proving number

1. **The goal moment after device A's change**, both tiers, a scored goal with its pause: mean ice shift under 1.0 in every channel across the 108 frames, puck and both jerseys at least 25 from the ice in the scoring end, and the lamp's lit pixels at least twice its resting luma.
2. **The night fill follows the pieces on every rink size:** pieces above the board within 10 % of the regular rink's 94 on Pond and the wide arena, ice within 1 % on each.
3. **The stands lane (AR2) against these zones:** zero overlap between any stand or crowd bound and the four zones above on the five shipped rink sizes, and the benches, boxes and table still fully visible from the game camera at the opposite boards.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 419,617 | 429,073 |
| `Look/MI_Trim_Lamp.uasset` | new | 9,737 |
| `Look/MI_Toon_NetCord.uasset` | new | 9,168 |

## Ledger rows

The first replaces the row of the same path; the other two are new.

```
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.41 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | L2 night rig as before; sky light a movable real-time capture of the capture dome SkyCap_L2_Night (lane B34); side snow banks split into four pieces (Snow_L2_Night_Bank_3, _3b, _4, _4b on SM_Con_SnowBank_Side) leaving device A's penalty boxes, scorer's table and benches clear on every rink size; three shadowless rect fills (Fill_L2_Night_Box, Fill_L2_Night_Bench_1, _2: 450 cd, 7 m, facing away from the rink above the board line) for the pieces behind the boards (lane B41) (429073 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Lamp.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_Trim: red lamp for device A's goal lights and scorer's table (slot lamp); Emissive 3 at rest, off at 0.5 or under, flashing 8 to 12; TeamColour (1.0, 0.04, 0.012) (lane B41, A-to-B-0024 ask 2) (9737 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_NetCord.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | instance of M_Toon: plain off-white for the cords of device A's goal nets (slot net), no grid pattern (GridCm 0), TeamColour (0.86, 0.84, 0.78) (lane B41, A-to-B-0024 ask 2) (9168 bytes) |
```

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261114-011347-B-b41-goal-clay.png`
- `20261114-011347-B-b41-goal-night.png`
- `20261114-011347-B-b41-goal-clay-proposed.png`
- `20261114-011347-B-b41-lamp-clay-night.png`
- `20261114-011347-B-b41-net-cords-clay-night.png`
- `20261114-011347-B-b41-night-rig-ab.png`
- `20261114-011347-B-b41-night-rinks.png`
- `20261114-011347-B-b41-clay-arena-check.png`
