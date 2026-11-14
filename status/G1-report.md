# G1 report: the GPU lane (device B)

# Hand-back 2 (14:54): job 2, the menu backdrop, and job 2b, the UI kit on the GPU

Machine clock 14:40 to 14:54, same tree (now at 765c893e with job 1 committed by the lead). The editor was left through `ExitEditor` (pid 32208 gone), the one allowed command `git checkout lane/b-aud -- Game/Plugins/JamAssets/Content/UI` was run (it **stages** the 32 kit files in this tree's index: the lead cleans it up), and the editor was restarted: **pid 10980, port 8008, running, idle, no play session**. The `Rink` map is dirty in memory only (a test actor was added and destroyed; never saved: leave with `bDiscardUnsavedChanges true`). Put back: `LevelEditorPlaySettings` new window 1536 x 878; `jam.Cut` default; no other cvar touched. `r.SetRes 1920x1080w` was used inside play sessions only.

## 1. Done / not done
- **Done: job 2**: `UI/T_UI_MenuBackdrop` (1920 x 1080) and `UI/T_UI_MenuBackdrop_4K` (3840 x 2160), both imported and saved, untracked new files.
- **Done: job 2b**: the U1 kit seen on the GPU in a throwaway UMG widget (never saved, deleted with its folder `/Game/G1Tmp`).
- Not done: job 3 (neon rig), job 4.

## 2. Items
**Job 2, the backdrop.** Three candidates rendered in a clay play session on `Rink` with every figure and the puck hidden, no HUD (HighResShot 1920 x 1080, camera post-process exposure -0.5, vignette 0.6, white balance 5200 K, saturation 0.9, unsaved):
- A, end-high (behind a net, 10 m up, looking down the rink): good depth, but both faceoff circles and the net sit big and red in the lower half, under the menu's buttons.
- B, corner-high: busy stands down the left edge where the menu column sits; the bench box draws the eye.
- **C, chosen**: centre ice from behind the near boards, 3.8 m up, looking across at the far stands (x 900, y -2300, pitch +3, yaw 90, FOV 90). The interest is in the top half (warm lamps hanging from dark trusses, the stand band with the crowd, the window row) and the ice below is plain and calm, with only a slanting red and blue line low down. Under a 35 % stand-in scrim with A's home menu laid over (candidate sheet) it reads as a barn at night and never fights the panels. No figure, no HUD, no text.
- Import: `/JamAssets/UI/T_UI_MenuBackdrop`, 1920 x 1080, TEXTUREGROUP_UI, no mips, BC7, sRGB, never stream (read back after import). `T_UI_MenuBackdrop_4K` the same at 3840 x 2160 (same camera, cheap: one HighResShot). On disk 1,757,644 and 6,141,167 bytes (uasset with the source PNG; cooked BC7 about 2.1 and 8.3 MB). If 8 MB is too much for the package, drop the 4K one: nothing names it.

**Job 2b, the UI kit on the GPU** (lane/b-aud 8289793a, report U1). A throwaway `WidgetBlueprint` built with `WidgetBlueprintEditorSubsystem` (canvas, images with box brushes, text blocks), shown in a play session at viewport 1920 x 1085 (cropped to 1080): backdrop, a 60 % ink scrim of mine, `T_UI_Panel` boxed to 600 x 400, the four `T_UI_Button_*` at 380 x 96 with `F_Hokej_Display` Bold 30, `T_UI_Segment_Lit`/`_Unlit` at 208 x 46 (ROOKIE / PRO / ALL-STAR), `T_UI_Card_Mode` 320 x 400 with `Modes/T_UI_Mode_pond`, a Regular line and a Black line, `T_UI_Banner_Goal` at 1280 x 400. Box margins from U1's table as fractions (panel 0.125; button 32/192, 24/96; segment 0.125, 0.25; card 0.125, 0.1, 0.125, 0.125) with the brush image size = texture / 2 (the kit is drawn at 2x).
- **9-slice holds**: the panel's top-right chamfer, the buttons' bottom-right chamfers, the card's bottom-right chamfer and its white top rule, the segments' underline all keep their size and 45-degree angle at 600 x 400, 380 x 96 and 208 x 46; no stretched corner.
- **No compression banding**: the panel's fill steps one level at a time top to bottom (25,30,45 to 21,27,38 over 285 px), the buttons likewise (orange 255,121,39 to 255,108,21); the BC7 banner's edges and the BC1 mode picture are clean at 1:1.
- **The font draws**: Bold (Roboto Bold Condensed) for titles and buttons, Black for "5 A SIDE", Regular for the body line, all crisp at 22 to 36 pt; GOAL in the banner texture is sharp at 1280 x 400.
- One thing seen: **`T_UI_Panel` and `T_UI_Card_Mode` are slightly translucent** (the backdrop's lamp shows faintly through the panel's lower half). Fine over a scrimmed backdrop; if A wants them opaque, it is the texture's alpha (U1 lane).
- The method if A wants the same check: the widget appears only through `UserWidget.add_to_viewport` on the widget a `WidgetComponent` created (Python has no `CreateWidget`; a screen-space widget component alone did not draw), and the viewport is captured with `shot showui` (`CaptureGameViewport` leaves UMG out).

**Stills** (each with a `.txt` caption):
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-145300-B-g1-menubackdrop-chosen.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-145301-B-g1-menubackdrop-candidates.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-145302-B-g1-uikit-gpu.png` (the 1920 x 1080 kit still)
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-145303-B-g1-uikit-corners.png`

**What I looked at (Read tool):** device A's `menu-2` home and modes stills first; the three first candidate frames (A had a post in front, fixed by moving the camera), the darker sheet of the three, the 1920 sheet raw and under the scrim with A's menu; the kit still at full size (twice: the window screenshot with its title bar, then the 1080 crop) and the corner zoom sheet.

## 3. Not verified
- The backdrop under A's real scrim and widgets (I used a stand-in scrim); not cooked.
- The kit at other resolutions (DPI scale 1.005 here); hover/press behaviour (images, not real buttons); the HeaderBar and the logo were not placed.
- The 13 other mode pictures.

## 4. For device A
- `T_UI_MenuBackdrop` is ready by the 0028 name; `T_UI_MenuBackdrop_4K` is extra (use it at 4K or drop it).
- For the kit in UMG: a box brush's margin is taken against the brush's image size, so set ImageSize to the texture's size / 2 (button 96 x 48, panel 128 x 128, segment 64 x 32, card 128 x 160), not to the widget's size, or the corners grow with the widget.

## 5. Decisions for the operator
- Backdrop C (calm, interest at the top) over A (more drama, red circles under the menu). Swap to A on request: both rendered with the same grade.

## 6. Next three steps
1. Job 3: `Rigs/LR_N1_Neon`: read how `LR_P3_Clay` / `LR_L2_Night` are loaded by tier name, then build and judge with ten figures.
2. Job 4 if time before 17:30 (the menu scene from camera C).
3. Cook-only check by the lead before the push (UI textures new).

## 7. Ledger rows
| Game/Plugins/JamAssets/Content/UI/T_UI_MenuBackdrop.uasset | texture | 1.68 | original | UE 5.9 editor (HighResShot in PIE) via MCP/Python |  |  | 2026-11-14 | B | y | 1920x1080 RGB, UI group, no mips, BC7, sRGB, never stream; the clay barn at rest from centre ice behind the near boards (x 900, y -2300, 3.8 m up, yaw 90, FOV 90), no figures, puck, HUD or text, exposure -0.5, vignette 0.6, 5200 K; menu backdrop under the scrim, seen on the GPU under the UI kit (lane G1) (1757644 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_MenuBackdrop_4K.uasset | texture | 5.86 | original | UE 5.9 editor (HighResShot in PIE) via MCP/Python |  |  | 2026-11-14 | B | y | 3840x2160 RGB, UI group, no mips, BC7, sRGB, never stream; the same frame as T_UI_MenuBackdrop at 4K (optional, not named by A's code) (lane G1) (6141167 bytes) |

---

# Hand-back 1: the puck's ring

Lane G1, device B, machine clock 14:15 to 14:37. Tree `C:\Users\kazda\jam\wt\b-arena` (`lane/b-arena` at 68bf1cb1, clean at the start). Build `ThirdJamEditor Win64 Development` PASS (366 s). One GPU editor, port 8008, pid 32208, **left running and idle** (no play session, no dirty package) for jobs 2 and 3; the editors on 8004 and 8006 were not touched. Cvars put back: `jam.Cut hockey.regular`, `r.Editor.Viewport.OverridePIEScreenPercentage 1`. No commit, no push, no cook. Scratch (scripts, every capture): `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\g1` (`halve.py`, `ab.sh`, `meas.py`, `bk\SM_PuckDisc_before.uasset` = the HEAD file).

## 1. Done / not done
- **Done: job 1, the puck's white outer ring half as thick.** One file changed: `Props/SM_PuckDisc.uasset`.
- Not done yet: job 2 (menu backdrop), job 3 (neon arena), job 4 (menu scene).

## 2. The item
**Which part draws the white ring.** The pale ring at the game camera is `SM_PuckDisc`'s `Halo` section (flat disc, radius 0 to 30 cm, `MI_Trim_PuckHalo`, overlay `M_PuckGlow` on clay); the puck itself is A's cylinder at scale 0.24 (radius 12 cm). So the white band seen round the puck was 30 - 12 = **18 cm** wide. The `Ring` section (radius 25 to 35 cm, `MI_Fx_PuckRing`) draws nothing visible at these views. Materials untouched.

**What changed.** Geometry only: every vertex of the disc outside the puck's radius moved inward, r' = 12 + (r - 12) / 2 (448 of 546 vertices; z, UVs, slots, materials and triangle count 704 unchanged). Halo rings now at 0 / 6 / 12 / 15 / 18 / **21 cm** (were 0 / 6 / 12 / 18 / 24 / 30); the `Ring` section now 18.5 to 23.5 cm (was 25 to 35), keeping its place relative to the halo's edge. Bounds 47 x 47 x 2.9 cm (were 70 x 70 x 2.9). A sets nothing on the disc (A-to-B-0028: A's flattening is gone from main; `JamPawns.cpp` keeps the disc at world scale 1), so the asset alone carries the change.

**Proving numbers** (same frozen frame, only the disc mesh swapped, GPU, clay and night):
- Ring width: **18 cm before, 9 cm after.** Close camera (3 m, FOV 30): halo 525 px -> 367 px across, puck 210 px = 24 cm, so the pale band is 157 px -> 78 px each side (8.75 px/cm), on clay and night alike.
- Pale halo pixels at the game camera, faceoff crop: clay 395 -> 174, night 338 -> 149. At 30 m (crop round the puck): clay 58 -> 31, night 46 -> 25.
- **Readable at 30 m**: yes on both tiers: the dark puck on a thinner pale strip still reads at the frame centre (clay the weaker of the two: pale ring on pale ice, the dark puck carries it).

**Stills** (all in `C:\Users\kazda\jam\wt\b-arena\devlog\shots\`, each with a `.txt` caption):
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143500-B-g1-puckring-ab-clay.png` (A/B sheet: faceoff, 30 m, close)
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143501-B-g1-puckring-ab-night.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143502-B-g1-puckring-after-clay-faceoff.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143503-B-g1-puckring-after-night-faceoff.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143504-B-g1-puckring-after-clay-30m.png`
- `C:\Users\kazda\jam\wt\b-arena\devlog\shots\20261114-143505-B-g1-puckring-after-night-30m.png`

**What I looked at (Read tool):** both A/B sheets (the band visibly half as wide in every row, the puck unchanged, the clay disc keeps the same cream colour so the `M_PuckGlow` overlay still draws); the full before stills at the game camera on clay and night and the night close-up; the full after 30 m clay frame (the puck reads as a small dark dot with a thin pale rim beside the centre circle) and the full after night 30 m frame.

How measured: PIE on `Rink`, cut = `hockey.regular` with the tier forced to `p3_clay` / `l2_night` by a copy of the cartridge passed to `jam.Cut` (scratch `clay.cartridge.json`, `night.cartridge.json`); world slowed at load, frozen with `slomo 0.0001` at t = 2.02 to 2.04 s (just after the centre drop); stills through `CaptureGameViewport` (1536 x 883), the first capture after each camera move thrown away. 30 m camera: 30 m from the puck, 25 degrees off the long axis (straight behind, the human's own figure covers the puck), 3.5 m up, FOV 70. The "before" mesh for the same-frame A/B was a temporary duplicate (`SM_PuckDisc_G1Before`, never saved, deleted; not on disk).

## 3. Not verified
- A fast-moving puck (the B38 crescent/ladder question on clay): the halo is smaller, the overlay unchanged; not re-run.
- Broadcast tier and any other tier: not looked at.
- The faceoff stills carry the drop's blue puffs over the puck (frozen 0.04 s after the drop); no still of the halved ring on an open-play puck far from the puffs.
- Cook: not run by me.

## 4. For device A
Nothing to wire: `/JamAssets/Props/SM_PuckDisc` keeps its path, slots (`Halo`, `Beacon`, `Ring`, `Marker`) and materials; at scale 1 it is now 47 cm across with a 9 cm pale band round the 24 cm puck. If the cut's `puck_radius` stat changes the puck's size, the band stays 9 cm outside a 12 cm puck only: a bigger puck covers more of it (as before).

## 5. Decisions for the operator
- Half as thick is exact (18 -> 9 cm). If on clay at a distance it now reads too faint, the next step is brightness, not width (`M_PuckGlow` `Glow` 18 is unchanged).

## 6. Next three steps
1. Job 2: the menu backdrop `UI/T_UI_MenuBackdrop` (look at device A's `docs\experiments\menu-2\*.png` first), two or three candidates at 1920 x 1080, import, choose.
2. Job 3: `Rigs/LR_N1_Neon` after reading how `LR_P3_Clay` / `LR_L2_Night` load by tier.
3. Job 4 if time before 17:30.

## 7. Ledger row
| Game/Plugins/JamAssets/Content/Props/SM_PuckDisc.uasset | mesh | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | puck halo disc, 47 x 47 x 2.9 cm (bounds -0.2 to 2.7 cm), 704 triangles, no collision; slots Halo (MI_Trim_PuckHalo, overlay M_PuckGlow), Beacon, Ring, Marker; the pale Halo ring outside the 12 cm puck halved from 18 to 9 cm (vertices outside r 12 cm moved to 12 + (r - 12) / 2: halo radius 30 -> 21 cm, Ring section 25-35 -> 18.5-23.5 cm), seen on the GPU on clay and night (lanes B38, B44, G1) (28360 bytes) |
