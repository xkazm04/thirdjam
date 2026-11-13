# B25 report: the P4 lab at the game camera (device B)

Lane B25 (continuing B24), machine clock 04:08 to 04:41. Tree `wt/b-vfx`, editor on **port 8002**, left through `ExitEditor` with **no dirty package**. No git, no build. `/Game/Maps/Rink` never saved (18,966 bytes, 2026-11-12 12:40); `_Scratch/` absent; lab assets only. SK8's editor on 8000 shared the GPU: no frame numbers, costs are estimates. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b25` (`gcams.sh` five game-like cameras, `blk.py` brightest-block metric, `rig25.py` the persisted rig/material values, `jersey` measurements inline).

## 1. The bars: largely fixed
**What made them:** at the game camera's height the side lamp banks (2 rows x 12 heads per 12 m bank) mirror in the scratched ice as long double rows; the 14 glint lights (4000 cd) and the overhead spots' specular added wide smears. Isolating at centre ice: glints off removed the wide side blobs entirely (saturated ice pixels 0.42 % -> 0.00 %); side banks off changed little there.

**Changes (saved):**
| Where | Before | After |
|---|---|---|
| `M_P4_LampBank` | every head lit | new `Every`/`Group`/`RowOdd`/`RowEven`: lights `Group` heads out of every `Every` (clusters with gaps) |
| `MI_P4_LampBank_Side` | 12 heads x 2 rows, 400 | **every 6th head** (2 small lamps per 12 m bank), Intensity **120** |
| `MI_P4_LampBank_End` | 34 heads x 2 rows, 400 | **pairs every 6 heads**, Intensity **90** |
| Glints (14 point lights, `LR_P4_Broadcast`) | 4000 cd | **120 cd** (still 500 cm vertical capsules, spec only) |
| 14 spot lamps | specular 1.0 | **specular_scale 0.2** (diffuse light unchanged) |
| `M_P4_Ice` / `MI_P4_Ice` | - | new `SpecHigh` **0.35** and `RoughHigh` **0.1**, blended in by camera height **2.5 -> 4 m** (`HighZ0/1`): high cameras see a softer, dimmer reflection; the concept framing (1.2 m) is untouched |

**Measured** (brightest 24 x 24 px block of the ice, luma 0-255; game-like cameras at 4.2 m, pitch -22):
| View | Before (B24 settings) | After |
|---|---|---|
| centre | 246 (rows of saturated pixels wider than 120 px: 37) | **~162-172, no saturated rows** (glint dots only) |
| near the -x end (toward the end bank) | 253, a saturated double row 40 rows tall | **~220**, paired dots, 0 wide rows |
| near the +x end | 239 | **220** |
| near each side board | 213 / 189 | **160 / 148** |
| match frames (game camera) | `b24-hero-game-camera`: 246, a bar ~700 px wide | **213-237**, separated patches and dots |
| wide cut (140 x 70) | - | 222-237, separated patches (the rig is authored for 90 x 45: the wide rink is much darker overall) |

**Against a jersey:** lit jerseys measure luma 50-100 (value 175-240); the brightest reflections are still **~2x a jersey's luma** in a few frames (a lamp's specular hotspot near the player, the end-bank pairs). By value (max channel) they are 220-237 against a jersey's 200-240: comparable. So the reflection no longer reads as a bar and is no longer the brightest *shape* in most frames, but the single brightest *pixels* are still lamp reflections. Shape: separated soft vertical dots and streaks along the boards (`b25-game-boards`), paired dots in the end zone (`b25-game-end`); near the centre some reflections are short soft patches (about a player's width) rather than streaks (`b25-game-best`, left of the players).

**Concept framing kept:** `b25-concept-ab` shows `b24-hero-concept-framing` and `b24-hero-best` against the same cameras after the change: far-bank streaks, spray, lean, ice unchanged; the side banks at the frame's left edge are now dots instead of a row.

## 2. The figures at the game camera
`M_P4_FigureE` gained `TrimGlow` (emissive on mask-G white trim) and `TeamGlow` (emissive on team cloth); `MI_P4_FigureE` now: **RimStrength 0.35** (was 0.15), **TrimGlow 0.25**, **TeamGlow 0.3**, **TeamGamma 0.6** (was 0.8), **TeamHi 1.3** (was 1.1).

**Measured at the faceoff from the game camera, same frame and position** (team-coloured pixels of the jersey):
| | Night tier (`l2_night`, its own figures) | P4 lab before | P4 lab after (TeamGlow 0.35 / gamma 0.6 / Hi 1.3 test) |
|---|---|---|---|
| blue (controlled, near) | rgb (23, 70, 160), luma 66 | (2, 10, 138), luma **17** | (5, 53, 167), luma **51** |
| orange (opponent, mid) | rgb (186, 78, 5), luma 96 | (149, 49, 15), luma **68** | (169, 68, 10), luma **85** |
Saved values (TeamGlow 0.3) sit just under the measured test. A stronger setting (0.45 / 0.5 / 1.35) reached the night tier's luma (73 / 97) but turned blue sky-blue and orange yellow, and the white trims lost contrast: rejected. **So the lab jerseys are still a little darker than the night tier's (about 80-90 % of its luma); that is the price of keeping the colours saturated.**
**Number on the back:** `BP_SkaterVisual_P4Lab` takes `SetNumber` (TB1's graph; the mirror sets 7/13/42/9/21/88), but `M_P4_FigureE` has no number path, so **no number shows**. Adding one needs M_SkaterTB2's number-box nodes ported to `M_P4_FigureE` (not done, time).

## 3. Grounding: decided, **7 shadowed lamps (the +y row), no blob**
At the game camera (`gr14` / `gr7` / `gr7b` / `gr4b` in scratch): 14 shadowed and 7 shadowed read almost the same (soft double shadow under each player); with 7 the shadow is a little lighter but clearly grounds the figure. The blob decal adds almost nothing visible from that height. Saved in the rig: lamps 8-14 (y = +9 m) cast shadows, 1-7 do not. **Expected cost:** half of the 14-lamp figure: about **+0.5-0.8 ms on a 4090, 1.5-2.5 ms mid-range** over unshadowed (estimate, B1c/B20 basis; not measured).

## 4. Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`), all one game frame each from a running bot match with the call-mirroring
- `20261113-043452-B-b25-game-centre.jpg`: centre ice, both teams.
- `20261113-043454-B-b25-game-end.jpg`: -x end zone near the net.
- `20261113-043455-B-b25-game-boards.jpg`: along the side boards.
- `20261113-043457-B-b25-game-ab.jpg`: `b24-hero-game-camera` left, B25 right.
- `20261113-042558-B-b25-concept-ab.jpg`: concept framing and the B24 best camera, before/after.
- `20261113-043511-B-b25-game-best.jpg`: the best game-camera frame for the operator.
Note: the match stills were taken with the earlier figure setting (TeamGlow 0.12, TeamGamma 0.8, rim 0.35, trims 0.25); the saved, lighter team colour (item 2) is not in them.

## Still wrong, plainly
- Near centre ice some side-bank reflections are short wide patches (a player's width), softer but not the concept's many small vertical streaks. The concept has many lamps all around; we have few, bright ones.
- The ice in the end zones and along the boards is dark at the game camera (the spots are aimed at the middle); figures there are lit mostly from above and read darker.
- Jerseys still ~10-20 % darker than the night tier's; no back number.
- The wide cut (140 x 70) is much darker: banks, glints and spots are placed for 90 x 45.
- Players at mid speed still read as sitting from the game camera in some frames (B24).

## Assets changed (bytes, `Game/Plugins/JamAssets/Content/`)
| File | Before -> after |
|---|---|
| `Rigs/LR_P4_Broadcast.umap` | 901,514 -> 901,679 (glints 120 cd, spot specular 0.2, 7 shadowed lamps) |
| `Look/P4Lab/M_P4_Ice` | 55,562 -> 57,856 (`RoughHigh`, `SpecHigh`, `HighZ0/1`) |
| `Look/P4Lab/MI_P4_Ice` | 21,062 -> 22,864 |
| `Look/P4Lab/M_P4_LampBank` | 16,271 -> 18,854 (`Every`, `Group`, `RowOdd`, `RowEven`) |
| `Look/P4Lab/MI_P4_LampBank_Side` | 8,337 -> 9,301 |
| `Look/P4Lab/MI_P4_LampBank_End` | 8,802 -> 9,061 |
| `Look/P4Lab/M_P4_FigureE` | 36,238 -> 36,170 (`TrimGlow`, `TeamGlow`) |
| `Look/P4Lab/MI_P4_FigureE` | 8,190 -> 13,584 |

Ledger rows whose text changes (`shipped` n):
```
| Game/Plugins/JamAssets/Content/Rigs/LR_P4_Broadcast.umap | other | 0.86 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | P4 lab arena: B20's set + stands on M_P4_Stands, side and end emissive lamp banks (clustered), 14 spec-only glint lights at 120 cd, spot lamps specular 0.2, 7 of 14 lamps shadowed (lanes B14, B20, B21, B25) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Ice.uasset | material | 0.06 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab ice master: scratch tiles, deep layer, wet patches, milky layer, near-camera cuts, glossy patch, softer/dimmer specular for high cameras (lanes B20, B21, B25) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_LampBank.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | unlit lamp bank: procedural grid of lamp heads, lit in clusters (lanes B21, B25) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_FigureE.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | P4E figure master: team colour by mask R x (1-G), roughness by mask, weave normal, AO, rim, sheen, trim and team glow (lanes B21, B25) |
```

## What I could not verify
- Frame cost (shared GPU). Motion: whether the dots/streaks shimmer as the camera follows.
- The saved team-colour setting in a match frame (stills predate it; measured only at the faceoff test).
- The other two wide/large cuts beyond one wide run.

## What the lead must decide
1. Accept the game-camera look as is (no bars; reflections still the brightest pixels), or add more, smaller lamps around the arena (the concept's "many small streaks" needs many sources, an asset/rig pass).
2. Port a back number into `M_P4_FigureE` (about 20 min, B).
3. If P4 ships: the rig must scale with the rink (A's switcher), the wide cut is too dark today.
