# B34 report: sky lights to real-time capture on night and clay, ten figures on clay, rougher barn as a proposal (device B)

Lane B34, device B, machine clock 16:27 to 17:20 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 364458f, read only). Editor on port 8002 (pid 17308, mine, started 16:27), left through `ExitEditor` at 17:16:40 (`LogExit: Exiting.`, no play session open, dirty list empty just before, process gone).

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (18,966 bytes, mtime 11-12 11:10, unmodified in git). Nothing of A's, `Skaters/`, `VFX/`, `Audio/` changed. `M_Toon`, `M_Trim`, `M_Ice`, `M_ClayBarn` and their instances not touched.
- `jam.Cut hockey.regular` set back before the exit; no other console variable was changed.
- 0 lines containing "overflow" in this session's log. `Game/Saved/Screenshots/WindowsEditor/` is empty.
- The scratch ten-figure cartridges are deleted. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b34` (rig backups from before the lane in `bk/`).
- One slip, cleaned up: an export path with a wrong prefix made the editor create `C:\c\Users\...` at 16:33; I moved the two files to scratch and removed that folder (it held nothing else).
- Seen at 17:17, not caused by me: no `UnrealEditor.exe` runs on the machine and nothing listens on port 8000 (the skater lane's editor, which I never addressed); an `UnrealEditor-Cmd.exe` (pid 29860, not mine) runs. Port 8002 still shows a listener with owner pid 0 after my exit, as B31 saw on port 8000.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | Sky lights to movable real-time capture, both rigs, picture kept | **PASS on both tiers** (night 0.53 to 0.63, clay 0.50 to 0.58 outside figure outlines; an unchanged scene measures 0.48 to 0.59). The hitch itself: not measured here | `b34-skylight-night-ab`, `b34-skylight-clay-ab` |
| 1 | Tier switch night, clay, night in one session | **PASS**: nothing left behind | `b34-tier-switch` |
| 1 | Flicker or crawl from the capture | **PASS**: none on either tier (frozen frame, 30 frames, with and without) | `b34-motion` |
| 2 | Ten figures on clay | **PASS**, with one fix in the rig (stair-stepped key shadows) and two points for others | `b34-ten-figures-faceoff`, `b34-ten-figures-scrum`, `b34-hero-game` |
| 3 | Rougher barn | **NOT DONE: written proposal and one test still.** Nothing of it is saved | `b34-boards-wear-ab` |

## 1. The sky lights

**What each rig had.** Both sky lights were already Movable, on a specified cubemap, no real-time capture. Neither rig has a sky atmosphere, cloud or fog. Night has a visible star dome (`SkyB_L2_Night_Dome`, material `MI_NightSky_Stars`, not a sky material, so a real-time capture does not see it). Clay is a closed barn with nothing a capture can see.

**What a real-time capture needs, and what I added.** A real-time capture renders only sky atmosphere, clouds, fog and meshes whose material is marked as sky. With none of those the ambient light is black (measured: night at the boards loses 15 luma on average, mean difference 16.9). So each rig got one capture dome:

- `Look/M_SkyAmbient` (new master, unlit, two-sided, **Is Sky**): colour by direction from the dome's centre. The lower half is black. One Custom node, `B34SkyAmbient`.
  - Night mode: nine colours by elevation, read from `DaylightAmbientCubemap` (exported to scratch and averaged per 10 degree band; the export was not imported anywhere), plus one directional term (`AzAmp`, `AzPhase`) because the old cubemap is a cloudy sky that lights walls facing one way about 15 % more.
  - Flat mode (`UseFlat` 1): one colour, for the clay tier's gray cube.
- `Look/MI_SkyAmbient_Night`: `Gain` 1.06, `AzAmp` 0.14, `AzPhase` 1.5708.
- `Look/MI_SkyAmbient_Clay`: `UseFlat` 1, `FlatColour` (1, 1, 1), `Gain` 0.418 (the gray cube's own value).
- One actor per rig, `SkyCap_L2_Night` and `SkyCap_P3_Clay`: the engine's basic sphere at the rig origin, scale 700 (350 m radius), no collision, no shadow, not in distance fields or Lumen. On night it lies outside the star dome (300 m) and is hidden by it; on clay it lies outside the barn. With the dome in the rig and the sky light still on its cubemap, the picture did not move (boards 0.49).

**Values per rig.**

| Rig | Property | Before | After |
|---|---|---|---|
| `LR_L2_Night` `Sky_L2_Night` | Mobility | Movable | Movable |
| | `bRealTimeCapture` | False | **True** |
| | `SourceType` | `SLS_SpecifiedCubemap` | **`SLS_CapturedScene`** |
| | `Cubemap` | `/Engine/MapTemplates/Sky/DaylightAmbientCubemap` | **None** |
| | Intensity, colour | 2.0, (150, 185, 255) | unchanged |
| | Lower hemisphere black | True | True |
| `LR_P3_Clay` `Sky_P3_Clay` | Mobility | Movable | Movable |
| | `bRealTimeCapture` | False | **True** |
| | `SourceType` | `SLS_SpecifiedCubemap` | **`SLS_CapturedScene`** |
| | `Cubemap` | `/Engine/EngineResources/GrayTextureCube` | **None** |
| | Intensity, colour | 0.35, (255, 200, 150) | unchanged |
| `LR_P3_Clay` `Key_P3_Clay_1..4` (item 2) | shadow resolution scale | 0.5 | **1.0** |
| | source radius | 30 cm | **60 cm** |

Neither rig file contains the string `DaylightAmbientCubemap` or `GrayTextureCube` any more. Intensity and colour did not need to change: the match is made in the dome's colours.

**Tile table, before against after.** "Before" = stills at 16:30 (night) and 16:32 (clay), before any change. "After" = a fresh match on the saved rigs. Mean absolute difference, 0 to 255; tile max over an 8 x 8 grid. "Outside figure outlines" excludes the pixels that differ by more than 6 between two "before" captures from two play sessions (figures, faceoff marker, particles), grown by 6 px. The last column is that same pair of "before" captures: what an unchanged scene measures.

| Tier | View | All pixels | Outside figure outlines | Tile max | Frame luma | Unchanged scene |
|---|---|---:|---:|---:|---|---:|
| Night | Game camera | 1.76 | **0.63** | 1.54 | 138.84 → 138.70 | 0.48 (1.75 all pixels) |
| Night | Boards 8 m | 0.53 | **0.53** (no figure in view) | 1.68 | 60.60 → 60.54 | 0.49 |
| Night | 24 m | 0.85 | **0.53** | 1.02 | 97.13 → 97.07 | 0.48 (0.83) |
| Clay | Game camera | 1.21 | **0.54** | 1.15 | 151.78 → 151.81 | 0.51 (0.68) |
| Clay | Boards 8 m | 0.58 | **0.58** (no figure in view) | 2.38 | 101.23 → 101.44 | 0.59 |
| Clay | 24 m | 0.80 | **0.50** | 1.02 | 124.18 → 124.31 | 0.51 (0.63) |

Targets were under 1.0 on night and under 1.5 on clay; both tiers are at what an unchanged scene measures. The mask excludes 7.7 to 11.3 % of the frame.

**Calibration, one frozen frame, old setting against the live capture** (no figure movement between the two):

| Tier | Dome setting | Boards 8 m | Opposite boards | End boards | 24 m |
|---|---|---:|---:|---:|---:|
| Night | gain 0 (no dome light) | 16.88 | | | 11.27 |
| Night | gain 1.0, no directional term | 0.64 | 0.52 | 0.52 | 0.78 |
| Night | **gain 1.06, directional 0.14 (saved)** | 0.54 | 0.54 | 0.52 | 0.80 |
| Clay | gain 0.2158 | 0.95 | 0.97 | 1.23 | 0.84 |
| Clay | **gain 0.418 (saved)** | 0.53 | 0.52 | 0.99 and 1.05 | 0.53 |

The one place that differs: on clay the barn's end wall above the boards reads about 1 luma darker with the capture (tiles -0.5 to -1.7). The same wall moves by up to 1.3 between two captures of an unchanged frame, so I cannot say it is a real difference.

**Tier switch, one session** (night cartridge, `SetTier` called on the game's own switcher: night, clay, night, same frozen frame):

| Compared | Boards 8 m | 24 m | Game camera |
|---|---:|---:|---:|
| Night after the round trip against night before it, same session | 0.38 | 0.69 | 1.32 all pixels (an unchanged frame: 1.10) |
| Night after the round trip against the lane's "before" still, outside outlines | 0.55 | 0.52 | 0.61 |
| Clay reached by the switch against the lane's clay "before" still | 0.59 | 0.48 outside outlines | 0.73 outside outlines |

At each step one sky light is visible (the rig's, real-time) and one dome exists; the previous rig's are gone. `Rink.umap`'s own sky light stays hidden.

**Motion.** 30 consecutive frames of a frozen frame at the held game camera, saved real-time capture against the old cubemap switched back in live:

| Tier | Crop | Real-time capture: mean \|d\| / temporal std p99 | Specified cubemap |
|---|---|---|---|
| Night | whole frame | 0.21 | 0.22 |
| Night | near ice | 0.20 / 0.59 | 0.19 / 0.88 |
| Night | mid ice left | 0.20 / 1.33 | 0.22 / 1.34 |
| Night | boards | 0.21 / 2.75 | 0.20 / 2.70 |
| Clay | whole frame | 0.25 | 0.24 |
| Clay | near ice | 0.28 / 0.62 | 0.28 / 0.58 |
| Clay | boards | 0.16 / 0.95 | 0.15 / 0.76 |
| Clay | walls | 0.20 / 2.43 | 0.18 / 1.92 |

No flicker, no crawl; neither tier was put back. My first night run showed a slow drift (0.4 luma over 29 frames); it was the 2 s after a camera cut, and a repeat with a settled camera gave the numbers above. A motion run in play (the lane's usual one) was not taken.

**The hitch.** Not measured: I may not take frame numbers. Device A measures.

## 2. Ten figures on the clay rink

Scratch cartridge outside the repository (copy of `hockey.regular`, five per team, `p3_clay`, three zone rings, two pickup kinds; deleted). It loaded and played: `JAMVISUAL after build: 10 skaters`. One thing for whoever writes the real one: `spawn_every_frames` under 120 is rejected and the game falls back to the regular cut without ten figures.

**Does the ice read calm?** Yes. At the faceoff eight figures are in the game camera's frame (the two blue defenders are behind it), with two rings and the marker; the ice between them is as empty as with six (`b34-ten-figures-faceoff`). In the scrum at 3.65 s five figures stand within 5 m of the puck and the ice still carries nothing but lines, shadows and the puck marker (`b34-ten-figures-scrum`).

**The puck.** Found at a glance in the scrum: core luma 23 against ice 190, inside device A's yellow-green marker (243).

**The human's figure.** Found at the faceoff and in the scrum only because the camera sits behind it. It has no mark of its own: same blue as four teammates, and the number 7 is on the back only. With five blue figures this is weaker than with three. Not the look's to fix (figure or HUD side).

**Jerseys against the ice, both teams at the same four spots** (two cartridges with the teams swapped; camera at game height behind each spot):

| Position | Blue jersey / ice (difference) | Orange jersey / ice (difference) | Dark parts |
|---|---|---|---:|
| Left wing | 48 / 176 (-128) | 114 / 176 (-62) | 20 |
| Right wing | 53 / 177 (-124) | 115 / 177 (-62) | 15 to 19 |
| Far corner | 46 / 171 (-124) | 115 / 170 (-55) | 17 |
| Near corner | 51 / 172 (-121) | 108 / 172 (-64) | 17 to 18 |

Every position is 55 or more from the ice (target 25). **A correction to B31's "orange in the near corner -24":** my first run gave -20 and -23 there too, with three times too many "orange" pixels: the yellow kick strip and the red line pass the orange test. The table above counts only pixels that differ between the blue run and the orange run at the same spot. The corner is not borderline.

**Shadows.** By eye, no pile of overlapping shadows: each figure throws one or two soft shadows that stay under and beside it. **One fault, the rig's, fixed:** the long shadow of a downed figure had stair-stepped edges about 8 px coarse at the game camera. The four key spots now use shadow resolution scale 1.0 (was 0.5, a caution carried over from the night spots, where B1c found it did not lower page counts) and source radius 60 cm (was 30). The edge is smooth and soft (`b34-ten-figures-scrum`, bottom row). The keys have no specular, so the larger source changes no highlight. 0 overflow lines in the log with ten figures. Cost not measured.

**Lamp sheen, ring glow.** No added clutter seen; not isolated by a measurement in this pass.

**For device A, seen in the same run:**
- At 5.0 s, after the human lost the faceoff and was knocked down, the puck was behind the game camera (puck at x -21.8 m, the human at x -10.3 m with the camera behind it still looking toward +x). One frame, one case.
- Zone rings use the `_Ring` instances in this tree.

## 3. A rougher barn: proposal, not done

I stopped here on time. The boards are `MI_Toon_Board` on the shared master `M_Toon` (37 packages reference it, figures included); the kick strip is `MI_Toon_KickPlate` on the same master; the red cap is the trim strips, `MI_Trim_Blue` / `MI_Trim_Orange` on `M_Trim` (path `B23TrimBase`); the walls are `MI_ClayBarn_Wall` on `M_ClayBarn`. Scuffs need new nodes in `M_Toon`, with a night measurement and a cook behind them; that is not a 10 minute job.

**Test still** (`b34-boards-wear-ab`): existing values only, set on play-session instances, nothing saved. Panel tone variation 0.06 → 0.22, joints 0.45 → 0.8, dent strength 0.3 → 0.6, plaster (0.8, 0.8, 0.82) → (0.68, 0.64, 0.58), wall wash (1, 0.55, 0.28) → (0.72, 0.38, 0.18), cap a little duller.
- Result: boards view changes by 3.60, frame luma 101.4 → 98.1; game camera 2.02. Ice luma is unchanged in all thirteen regions (138 to 199, each within 1).
- What it shows: uneven, dirtier panels and a darker barn already read as "used". Dent strength 0.6 is too much: near the camera the plaster looks blistered. I would keep 0.3 to 0.4.
- What it cannot show: scuffs, puck marks, chipped paint.

**Proposal for a next lane (about 40 minutes with a cook check):**
1. `M_Toon`, one new Custom node `B34BoardWear` after `B19Board`, clay only (`Clay` >= 0.5 and a new switch `BoardWear`, master 0, on only in `MI_Toon_Board`), so night boards stay bit for bit:
   - a dirt band rising from the kick strip: darkening up to 25 % at the foot, gone by 45 cm, broken by low-frequency noise along the board;
   - puck marks: dark short smears, 3 to 8 cm, from a hashed cell pattern, dense in the lowest 50 cm, three times denser within 8 m of each net (the material knows `RinkHalfLength`), none above 90 cm;
   - horizontal stick scuffs at 15 to 40 cm, 20 to 60 cm long, 10 % darker;
   - everything fades out when smaller than a pixel, like the panel joints, so nothing shimmers at 40 m; nothing brightens, so nothing can be brighter than the ice.
2. `M_Trim`, in `B23TrimBase`: chips on the red cap showing a cream or dark primer, a hashed mask of 2 to 6 cm chips on 5 to 8 % of the cap, denser at the corners; base colour only, no emissive change.
3. Values from the test, toned down: panel variation 0.15, joints 0.7, plaster (0.72, 0.69, 0.64), wall wash (0.8, 0.43, 0.21).
4. Proof: night boards before and after (target under 0.6 at the boards view), figures on both wings and in a corner with the paired method of item 2, 30 frames at the game camera and along the boards.

Figure readability after item 3: not re-measured, since nothing was changed.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_L2_Night.umap` | 417,991 | 419,617 |
| `Rigs/LR_P3_Clay.umap` | 318,467 | 320,088 |
| `Look/M_SkyAmbient.uasset` | new | 14,247 |
| `Look/MI_SkyAmbient_Night.uasset` | new | 10,342 |
| `Look/MI_SkyAmbient_Clay.uasset` | new | 9,761 |

`M_SkyAmbient`'s shader instruction count was not read.

## Ledger rows

Three **new** rows; the two rig rows **replace** the rows of the same path.

```
| Game/Plugins/JamAssets/Content/Look/M_SkyAmbient.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | unlit two-sided sky material (Is Sky) for the rigs' capture domes: colour by elevation in nine bands with one directional term, or one flat colour; lower half black; Custom node B34SkyAmbient (lane B34, A-to-B-0016 point 3) (14247 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_SkyAmbient_Night.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_SkyAmbient for SkyCap_L2_Night: gradient mode, Gain 1.06, AzAmp 0.14, AzPhase 1.5708 (lane B34) (10342 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_SkyAmbient_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_SkyAmbient for SkyCap_P3_Clay: flat mode, FlatColour (1, 1, 1), Gain 0.418 (lane B34) (9761 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap | other | 0.42 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | L2 night rig as before; sky light now a movable real-time capture (bRealTimeCapture, SLS_CapturedScene, no cubemap; intensity 2.0 and colour unchanged) of the new capture dome SkyCap_L2_Night (engine sphere, scale 700, MI_SkyAmbient_Night, outside the star dome); no reference to DaylightAmbientCubemap (lane B34) (419617 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.32 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34) (320088 bytes) |
```

The night row's text before "sky light now" should keep whatever the present ledger row says about the rig; I did not read `ASSETS.md`.

## The line for device A (`A-to-B-0016` point 3)

> B34: `LR_L2_Night` and `LR_P3_Clay` sky lights are now `Mobility=Movable`, `bRealTimeCapture=True`, `SourceType=SLS_CapturedScene`, `Cubemap=None`, each fed by a dome actor `SkyCap_<Tier>` in the rig (sky material `/JamAssets/Look/M_SkyAmbient`); neither rig references `DaylightAmbientCubemap` or `GrayTextureCube` any more, and both pictures are within 0.5 to 0.63 mean difference of before. Yours now: the persistent `SkyLight` in `Game/Content/Maps/Rink.umap` is still Stationary, `SLS_SpecifiedCubemap`, `DaylightAmbientCubemap`, intensity 1.0, and still registers at load before the switcher hides it. Please clear or remove it (mind your `l1_day` / `l3_neon` stand-in, which leans on it), then measure the first lit frames on both tiers and the per-frame cost of the two real-time captures; B could not measure either. Keep `r.SkyLight.RealTimeReflectionCapture=1` in every quality profile: at 0 both tiers lose their ambient light (night boards about 15 luma darker).

## What I could not verify

- The hitch at the first lit frame, the per-frame cost of the real-time capture, memory: no frame numbers here.
- The first frames of a match with the real-time capture (whether the ambient light is there from frame one or arrives over a few frames): every still is from 2.2 s or later.
- A cook and a packaged build (two rigs changed, three new assets; the rigs now reference `/Engine/BasicShapes/Sphere` and the new materials).
- The saved assets in a fresh editor process (saves returned true, dirty list empty, sizes changed, the after-stills come from fresh play sessions after the saves).
- Other rink sizes: the dome is a rig actor and scales with the rig; not run.
- A quality profile or platform where real-time sky capture is off.
- `LR_P4_Broadcast`: not touched, not looked at; whether it still references the daylight cubemap was not checked.
- Motion in play; the cost of key shadow resolution 1.0; the key change on night (night has its own spots, untouched).
- Lamp sheen and ring glow with ten figures as numbers; a ten-figure scrum at a net or in a corner.
- Everything in item 3 beyond the one test still.

## Decisions for the lead

1. **Sky lights:** accept; pass the line above to device A. If A finds the per-frame cost too high, the fallback is the old setting from `bk/` in my scratch folder (both rig files as they were at 16:37), which brings the read-back hitch back.
2. **Key shadows on clay** (resolution 0.5 → 1.0, source radius 30 → 60 cm): accept, or ask A to measure shadow cost first; both are one value each on four lights.
3. **The human's figure among five of its own colour** has no mark: ask the skater or HUD side (device A) for one, or accept.
4. **Item 3:** a lane for the proposal (new nodes in the shared `M_Toon`, with a cook check), or the values-only version of the test (four numbers on two instances, no shared master touched), or leave the barn tidy.
5. B31's decision 4 (orange in the corners at the limit) can be closed: it was a measuring error.
6. Paste the ledger rows; cook check before pushing (rigs changed).

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-171620-B-b34-skylight-night-ab.jpg`
- `20261113-171620-B-b34-skylight-clay-ab.jpg`
- `20261113-171620-B-b34-tier-switch.jpg`
- `20261113-171620-B-b34-motion.jpg`
- `20261113-171620-B-b34-ten-figures-faceoff.jpg`
- `20261113-171620-B-b34-ten-figures-scrum.jpg`
- `20261113-171620-B-b34-hero-game.png`
- `20261113-171620-B-b34-boards-wear-ab.jpg` (test still for the proposal)
