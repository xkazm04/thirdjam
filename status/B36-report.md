# B36 report: the black beyond the boards, worn boards and chipped cap on the clay tier (device B)

Lane B36, device B, machine clock 17:33 to 18:15 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 3fd4ec9, read only). Editor on port 8002 (pid 26672, mine), left through `ExitEditor` at 18:09:45 (`LogExit: Exiting.`, no play session open, dirty list empty just before, process gone). The QA lane's editor was never addressed.

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (18,966 bytes, 11-12 11:10). `LR_P3_Clay.umap` and `LR_L2_Night.umap` are byte for byte as B34 left them: both fixes are in materials only. Nothing of A's, `Skaters/`, `VFX/`, `Audio/` changed.
- `jam.Cut hockey.regular` set back; no other console variable changed. 0 "overflow" lines in the log. `Game/Saved/Screenshots/WindowsEditor/` is empty.
- Scratch, with backups of all eight changed files from before the lane in `bk/`: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b36`.
- **Order.** The message that put the void first reached me at 17:49; the board wear had been saved at 17:48. So both are done and saved; the void was built, tuned and proven after that message, on top of the wear.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| A | Black beyond the boards, regular rink | **PASS**: the apron is a dim warm plank floor, luma about 30 at the wall to 45 beside the boards (ice 145 to 166 in the same views); black pixels 9.0 % of the frame → 0.4 % at the side board | `b36-void-corner-ab`, `b36-void-side-ab` |
| A | Same on Pond and the wide arena | **PASS on Pond. PARTIAL on the wide arena**: the apron is fixed, but the upper barn wall is dark there (before and after) | `b36-void-pond-ab`, `b36-void-wide` |
| A | Simple shapes in the apron | **Left out** (reason below) | none |
| A | Calm ice, figures, night | **PASS**, with one thing to know about blue helmets (below) | tables below |
| 1 | `B34BoardWear` in `M_Toon`, bit for bit off elsewhere | **PASS**: a static switch; the unswitched shader has the same 549 instructions as before | `b36-boards-ab`, `b36-net-end`, `b36-night-unchanged` |
| 2 | Chips on the red cap, `M_Trim` | **PASS**: base colour only, clay only, also behind a static switch | `b36-net-end` |
| 3 | The toned-down values | **DONE as decided, with one warning: "joints 0.7" makes the joints fainter, not darker** (below) | `b36-boards-ab` |
| 4 | Calm ice and readability | **PASS**: ice regions unchanged; orange against the worn board 45 or more (was 60 to 67) | `b36-figures-at-boards`, `b36-game-ab` |
| 5 | Tone | **PASS by eye**: grey-black smears, grey-brown dirt, no red-brown | `b36-net-end` |
| 6 | Motion | **PASS**: no shimmer | `b36-motion` |

## A. The black beyond the boards

**What it was.** All of the black is the barn floor between the boards and the wall (15.5 m wide at the sides, 16 m at the ends). The wall is already lit down to its foot by its own wash. The floor is a very dark wood (`MI_ClayBarn_Floor`) and no lamp reaches it, so it read luma 0 to 6.

**What I did: no light, one material node.** `M_ClayBarn` gains the Custom node `B36FloorSpill` on the emissive output (after `B26BarnWash`): the floor's own wood colour as a warm fill, strongest beside the boards and falling toward the wall, as if light spilled over the boards. It is off unless `SpillK` > 0, and only `MI_ClayBarn_Floor` sets it.

| Parameter (on `MI_ClayBarn_Floor`) | Value | Meaning |
|---|---|---|
| `SpillK` | 7.2 (master 0) | strength; **the one value to turn**: about 5.5 gives luma 25 to 34 |
| `SpillFar` | 0.55 | share left at the wall |
| `SpillLenM` | 5 | falloff length from the boards |
| `SpillColour` | (1, 0.62, 0.36) | warm tint |
| `SpillRink` | (36.885, 29.605, 1.22, 0.76), master value | the rink's footprint in the floor cube's local coordinates |

- **Why not lamps:** a fill in the material casts no shadow and no highlight and puts nothing on the ice or on the figures. It needs no per-rink placement and adds no light to the eighteen the rig has.
- **Rink sizes:** the fill is computed in the floor mesh's local coordinates, so it follows the rig's scale. It assumes the 2 : 1 rinks that ship.
- The floor keeps its plank grain, so it reads as a floor, not a plane (`b36-void-corner-ab`).
- **Shapes in the apron: left out.** With a fill and no real light, boxes would be evenly bright on every face and read as flat cut-outs. Doing them properly needs a shaded material or a real fill light; I was in doubt, so they are not there.

**Apron, before and after** (camera at game height 3.5 m and pitch -17.7 degrees; corner view 15 m from the corner, side view 8.5 m from the board; I could not reproduce the QA stills' exact camera, which was in play):

| Rink | View | Frame under luma 12, before → after | Apron luma after | Ice in the same view, before → after |
|---|---|---|---|---|
| Regular 90 x 45 | corner | 8.5 % → 1.7 % | median 29 | 148.1 → 148.1 |
| Regular | side board | 9.0 % → 0.4 % | 32 at the wall, about 45 beside the boards | 145.5 → 145.5 |
| Pond 60 x 30 | corner | 3.8 % → 0.3 % | median 29 | 166.1 → 166.1 |
| Pond | side board | 5.7 % → 0.1 % | 30 to 36 | 147.5 → 147.5 |
| Wide 140 x 70 | corner | 21.4 % → 10.8 % | 30 to about 45 | 125.6 → 125.6 |
| Wide | side board | 18.4 % → 8.7 % | 30 to about 45 | 146.6 → 146.6 |

- The boards, cap and ice outside the apron moved by 0.43 to 0.56 between before and after (noise), 0.95 in the wide side view.
- **Wide arena, not fixed:** what stays dark there is the upper barn wall at the top of the frame (luma about 10, before and after; 26 on the regular rink, 32 on Pond). The wall's wash has a fixed height in centimetres and the walls are 1.56 times taller on that rink. Not the apron, and I did not touch it.

**Figures against the new background.** Plainly: at game-camera height a figure at the boards is **not** in front of the apron. The clay figure is about 1.45 m tall and the cap top is at about 1.3 m; from 3.5 m up, its helmet ends at or just under the cap line (0 to 6 team-coloured pixels above it, at 7 m and at 12 m, side board and corner). Its background stays the board and the red cap.

| Position (game height, paired runs) | Team | Jersey / board beside it, before → after the floor |
|---|---|---|
| Side board | blue | 47 / 176 (-129) → 47 / 176 (-129) |
| Side board | orange | 118 / 177 (-59) → 119 / 176 (-57) |
| Far corner | blue | 39 / 149 (-110) → 39 / 149 (-110) |
| Far corner | orange | 106 / 148 (-43) → 106 / 149 (-42) |

What does change is the band just above the cap, next to the helmet:

| Position | Team | Helmet top luma | Apron just above the cap, before → after | Gap, before → after |
|---|---|---:|---|---|
| Side board | blue | 45 | 1 → 33 | +44 → **+12** |
| Side board | orange | 133 | 1 → 33 | +131 → +99 |
| Far corner | blue | 37 | 0 → 34 | +36 → **+3** |
| Far corner | orange | 104 | 0 → 34 | +103 → +70 |

**To know:** a blue helmet now has nearly the same luma as the apron. At the game camera the red cap (luma about 100) lies between the two, so they do not touch. A lower camera (a replay or a low follow) would put a blue helmet in front of the brown floor, where it separates by colour only. `SpillK` 5.5 would bring the gap back to about +10 to +20; a darker floor reads less.

**Motion, 30 consecutive frames of a frozen frame:** held camera over the side board: apron 0.14 mean difference, temporal std p99 0.46 (wall foot 0.21 / 2.29, boards 0.25 / 1.54, ice 0.29 / 0.85). Pan across the corner at 1 degree per frame: aligned residual 2.52, with no "before" run to compare it to (the pan along the boards reads 1.71 by the same method); on the still's frames the apron does not change, but I did not judge it at full size.

## 1. Board wear in `M_Toon`

**How it is switched: a static switch parameter, `BoardWear`, master default off.** The new Custom node `B34BoardWear` is wired only to the switch's True input; the False input is the old chain (`B19ToonBase`), and the switch feeds base colour. A static switch is resolved when the shader is generated, so for every instance that does not override it the wear code is not in the shader at all. That is why it cannot leak, where a runtime parameter would leave the code in every `M_Toon` user.

Evidence:

| Material | Pixel shader instructions before | After |
|---|---:|---:|
| `M_Toon` (the permutation every other user compiles) | 549 | **549** |
| `MI_Toon_KickPlate` | 549 | 549 |
| `MI_Toon_Board` (switch on) | 549 | 696 |
| `M_Trim`, `MI_Trim_PuckHalo`, `MI_Trim_Beacon` | 221 | **221** |
| `MI_Trim_Blue` (switch on) | 221 | 313 |

Cost: one more shader permutation for `MI_Toon_Board` and one for the two cap instances. Not cooked.

**What the node does** (clay only; it can only darken, the last line is a per-channel minimum with the input):
- **Plaster tint** `WearTint` (0.9, 0.8625, 0.7805): the plaster reads (0.72, 0.69, 0.64) on clay. I did **not** change `TeamColour` on the instance, because night uses it too.
- **Dirt band:** up to 25 % darker at the kick strip's top, gone 18 to 42 cm above it, uneven along the board.
- **Puck smears:** grey-black rubber (0.035, 0.035, 0.04), 3 to 8 cm, three layers, dense under 50 cm, none above 90 cm, three times denser on the end boards within 3 to 9 m of the middle (behind the nets). I thinned them once after the first look (0.42 → 0.30).
- **Stick scuffs:** horizontal, 20 to 60 cm long, 10 to 16 % darker, up to 65 cm.
- The red band, the top face and everything above 104 cm are untouched (the wear fades out between 101 and 104 cm).
- Every mark fades out as it nears pixel size: the smears from 1.2 cm per pixel toward their own average darkening, the scuffs from 0.8 cm per pixel to nothing.

**Night unchanged** (before = stills at 17:35 before anything was touched; noise = a second untouched night session):

| Night view | After the wear | After everything | Unchanged scene |
|---|---:|---:|---:|
| Boards 8 m | 0.47 | 0.48 | 0.49 |
| 24 m | 0.80 | 0.64 | 0.79 |
| Game camera, outside figure outlines | 0.43 | 0.52 | 0.50 |

**Two other `M_Toon` users on clay** (pixels of that part only, before against after the wear):

| Part | Material | Pixels | Mean difference | Luma |
|---|---|---:|---:|---|
| Kick strip, boards 8 m | `MI_Toon_KickPlate` | 36,247 | 0.61 | 154.3 → 153.9 |
| Kick strip, along the boards | `MI_Toon_KickPlate` | 15,318 | 0.55 | 150.0 → 149.7 |
| Net frame and mesh, net-end view | `MI_Toon_NetFrame`, `MI_Toon_NetMesh` | 15,669 | 0.50 | 102.7 → 102.6 |

All at noise (0.5 to 0.6).

## 2. Chips on the red cap, `M_Trim`

New Custom node `B34CapChips` after `B23TrimBase`, behind the static switch `CapChips` (on only in `MI_Trim_Blue` and `MI_Trim_Orange`; device A's dynamic instances inherit it, as the stills show). Chips are blobs of a 6 cm three-dimensional noise, so they run over the cap's edges without a seam; dark primer; `ClayCapChips` 1, `ClayCapChipThr` 0.75 (I raised it from 0.72 after the first look). Base colour only: the cap's red self-fill is unchanged, so a chip on an unlit face is a darker red, not grey. Chips fade out from 2.5 cm per pixel. Not done: "denser at the corners" (the material does not know where the corners are).

From the far end at the game camera: cap luma 99.5 → 99.7, kick strip 162.3 → 161.9. Both read as the rink's edge as before.

## 3. The values

| Asset | Parameter | Before | After |
|---|---|---|---|
| `MI_Toon_Board` | `PanelVar` | 0.06 | 0.15 |
| | `JointDark` | 0.45 | **0.7** |
| | `ClayBumpMul` (dents) | 0.3 | 0.3 |
| | plaster on clay | (0.8, 0.8, 0.82) | (0.72, 0.69, 0.64) through `WearTint` |
| | `WearDirt`, `WearSmear`, `WearScuff`, `WearKickTopCm` | none | 0.25, 1, 1, 25 |
| `MI_ClayBarn_Wall` | `WashColour` | (1, 0.55, 0.28) | (0.8, 0.43, 0.21) |

**Warning on the joints.** `JointDark` is a multiplier on the joint's colour: 0.45 is a dark joint, 0.7 is a fainter one. My B34 test still used 0.8 and my report called that a stronger joint; that was wrong. I applied 0.7 as decided, so the panel joints are now fainter than before. If the intent was darker joints, the value is about 0.35.

## 4. Calm ice and readability

**Ice luma, game camera at the faceoff, thirteen regions (median):**

| | near | nearL | midL | midR | mid | farC | farL | farR | sideL | sideR | netFront | cornerL | cornerR |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Before | 193 | 185 | 168 | 167 | 171 | 199 | 182 | 178 | 139 | 138 | 152 | 154 | 157 |
| After the wear | 193 | 185 | 168 | 167 | 171 | 199 | 182 | 178 | 140 | 139 | 151 | 153 | 156 |
| After the wear and the floor | 193 | 185 | 168 | 167 | 171 | 199 | 182 | 178 | 139 | 138 | 151 | 153 | 156 |

**Figures, paired method** (teams swapped at the same spawns with the same role, frozen at 0.6 s before they move; my first pass at 2.17 s was wrong because the two teams' figures had skated to different places, and I retook the "before" before changing anything). Figures stand 2 m from the board.

| Position | Team | Jersey / ice, before → after the wear | Jersey / board behind it, before → after the wear |
|---|---|---|---|
| Left wing | blue | 47 / 179 (-132) → 47 / 179 (-132) | 48 / 183 (-135) → 48 / 171 (-123) |
| Left wing | orange | 119 / 179 (-60) → 119 / 179 (-60) | 120 / 185 (-65) → 128 / 174 (**-46**) |
| Right wing | blue | 48 / 177 (-129) → 48 / 177 (-129) | 46 / 185 (-139) → 46 / 170 (-124) |
| Right wing | orange | 117 / 177 (-60) → 117 / 177 (-60) | 119 / 186 (-67) → 127 / 172 (**-45**) |
| Far corner | blue | 46 / 153 (-106) → 46 / 153 (-106) | 46 / 173 (-128) → 46 / 161 (-115) |
| Far corner | orange | 116 / 153 (-37) → 117 / 153 (-36) | 115 / 175 (-60) → 118 / 164 (**-46**) |

- Against the ice nothing changed.
- Against the board, orange lost about 15 to 20 and stands at 45 or more (target 25). The board is 11 to 15 darker. Part of the loss is measurement: the orange reading itself moved by up to 8 between the runs although the jersey did not change (13 % fewer pixels passed the test).

## 6. Motion (wear)

30 consecutive frames of a frozen frame, before against after:

| Run | Crop | Before: mean difference / temporal std p99 | After |
|---|---|---|---|
| Held game camera | far boards | 0.27 / 2.62 | 0.25 / 2.46 |
| Held game camera | side boards | 0.29 / 2.57 | 0.29 / 2.60 |
| Held game camera | whole frame | 0.25 | 0.24 |
| Pan along the boards, 1 degree per frame | boards and ice, aligned residual | 1.76 | 1.71 |

No shimmer. A run in play was not taken.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/Look/`)

| File | Before | After |
|---|---:|---:|
| `M_ClayBarn.uasset` | 20,892 | 26,467 |
| `MI_ClayBarn_Floor.uasset` | 9,123 | 12,185 |
| `MI_ClayBarn_Wall.uasset` | 11,890 | 11,356 |
| `M_Toon.uasset` | 60,521 | 67,992 |
| `MI_Toon_Board.uasset` | 15,462 | 18,747 |
| `M_Trim.uasset` | 20,682 | 24,965 |
| `MI_Trim_Blue.uasset` | 8,718 | 10,265 |
| `MI_Trim_Orange.uasset` | 8,914 | 10,461 |

No asset created or deleted. `M_ClayBarn` pixel shader: 319 instructions after; before not read.

## Ledger rows (all eight replace the rows of the same path)

```
| Game/Plugins/JamAssets/Content/Look/M_ClayBarn.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | barn wood master: world triplanar grain, dents, roughness (lane B19); B26BarnWash: faint warm emissive wash rising from the floor, off by default (lane B26); B36FloorSpill: warm fill of the floor outside the rink's footprint, strongest beside the boards, in the mesh's local coordinates, off unless SpillK > 0 (lane B36) (26467 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Floor.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ClayBarn, floor (lane B19); lit apron outside the boards: SpillK 7.2, SpillFar 0.55, SpillLenM 5 (lane B36) (12185 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_ClayBarn_Wall.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ClayBarn, plank walls (lane B19); WashK 7, WashHeightCm 300 (lane B26); WashColour (0.8, 0.43, 0.21) (lane B36) (11356 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + Construct (B13), clay pass 2 / broadcast mode (B14), CelConstructGate (B15), B19ToonBase: clay-only base override and painted board panels with joints (B19), B34BoardWear behind the static switch BoardWear (default off, so no other instance compiles it): clay-only dirt band, rubber puck smears, stick scuffs and plaster tint on the boards (B36) (67992 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; kick plate and rail band (B13-B15); clay: panels with joints every 2.44 m, red band (B19); static switch BoardWear on, PanelVar 0.15, JointDark 0.7, WearTint (0.9, 0.8625, 0.7805), WearDirt 0.25 (B36) (18747 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Trim.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + clay cap path B23TrimBase/Emis/Rough: matte painted cap without team glow where ClayCap > 0, Clay >= 0.5 only (B23); B34CapChips behind the static switch CapChips (default off): chipped paint on the clay cap, base colour only (B36) (24965 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Blue.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; ClayCap 1: red board cap on the clay tier (B23); static switch CapChips on, ClayCapChips 1, ClayCapChipThr 0.75 (B36) (10265 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_Orange.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; ClayCap 1: red board cap on the clay tier (B23); static switch CapChips on, ClayCapChips 1, ClayCapChipThr 0.75 (B36) (10461 bytes) |
```

## What I could not verify

- A cook and a packaged build: two shared masters changed and two static switches add shader permutations.
- Frame cost of the wear node (147 more instructions on board pixels) and of the cap chips.
- The saved assets in a fresh editor process (saves returned true, the dirty list was empty, the after-stills come from fresh play sessions after the saves).
- The exact camera of the two QA stills; the apron in play (all stills are frozen frames).
- The worn boards on Pond, the wide arena and Cage looked at closely (the obstacles carry `MI_Toon_Board` and so the wear); the wear behind the near net.
- A blue helmet in front of the apron from a low camera: the game camera never shows it, so the numbers above are for the band beside the helmet.
- Motion in play; the pan across the corner has no "before" run.
- Broadcast tier: not run. It uses `M_Toon` with the switch off, so by the shader count it cannot have changed; no still.
- The net-end and figures-at-boards stills were taken after the wear and before the floor fill; neither view shows the apron.

## Decisions for the lead

1. **Joints:** 0.7 (fainter, as applied) or about 0.35 (darker, which I believe was the intent). One value on `MI_Toon_Board`.
2. **Apron brightness:** `SpillK` 7.2 (luma 30 to 45, blue helmets within 3 to 12 of it) or about 5.5 (25 to 34, helmets about 10 to 20 apart). One value on `MI_ClayBarn_Floor`.
3. **Wide arena's dark upper wall:** a lane to make the wall wash follow the rig's scale, or accept (only the 140 m rinks).
4. **Shapes in the apron:** a lane with a shaded prop material, or leave the floor empty.
5. **Smear density and chips:** as saved, or thinner; `WearSmear`, `WearScuff`, `WearDirt` and `ClayCapChipThr` are single values. Setting the two static switches off returns boards and cap to exactly what they were.
6. Paste the ledger rows; cook check before pushing (shared masters `M_Toon` and `M_Trim` changed).

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-180919-B-b36-void-corner-ab.jpg`, `-void-side-ab.jpg`, `-void-pond-ab.jpg`, `-void-wide.jpg`
- `20261113-180919-B-b36-boards-ab.jpg`, `-game-ab.jpg`, `-net-end.jpg`
- `20261113-180919-B-b36-figures-at-boards.jpg`
- `20261113-180919-B-b36-night-unchanged.jpg`
- `20261113-180919-B-b36-motion.jpg`
- `20261113-180919-B-b36-hero-game.png`
