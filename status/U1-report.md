# U1 report: the front end's UI kit (images, display font, two mocks), 14 Nov 14:35

## 1. Done / not done
Done: all names of A-to-B-0028 "The front end" except `T_UI_MenuBackdrop` (GPU lane's) and `L_MenuScene` (not this lane): 11 kit textures, 14 mode pictures, the
font `F_Hokej_Display` (Regular, Bold, Black) with three Font Faces, the GOAL banner, imported headless into `C:\Users\kazda\jam\wt\b-aud` (port 8006), plus
two 1920x1080 mocks. Not done: nothing seen on a GPU; no cook-only check; no git (the lead commits).

## 2. Items
- **Kit textures** under `/JamAssets/UI/` (made by `C:\Users\kazda\jam\ui-lab\work\kit.py`, sources `C:\Users\kazda\jam\ui-lab\out\kit\`). All: UI group, no mips,
  sRGB, never stream. 9-slice margins in texture px (textures are 2x display; in UMG Box margin = px / size):
  | asset | size | margins (L,T,R,B) | compression |
  |---|---|---|---|
  | T_UI_Panel | 256x256 | 32,32,32,32 (0.125) | UserInterface2D |
  | T_UI_Card_Mode | 256x320 | 32,32,32,40 | UserInterface2D (white 2 px top rule: tint it per team) |
  | T_UI_Button_Normal/_Focused/_Pressed/_Disabled | 192x96 | 32,24,32,24 | UserInterface2D |
  | T_UI_Segment_Lit/_Unlit | 128x64 | 16,16,16,16 | UserInterface2D |
  | T_UI_HeaderBar | 2048x176 | stretch to 1920x88, no 9-slice | UserInterface2D |
  | T_UI_Banner_Goal | 2048x640 | Image, display about 1280x400 | BC7 |
  | T_UI_Logo_Hokej | 920x716 | Image (brand lab BR4 logo-alpha-dark, trimmed) | BC7 |
  | Modes/T_UI_Mode_<14 slugs> | 640x360 | Image (display 320x180) | BC1 (Default) |
  Proving numbers: an editor read-back of every texture (size, TEXTUREGROUP_UI, TMGS_NO_MIPMAPS, compression, srgb=True) after the reimport; files on disk with
  the byte sizes in the ledger rows. Estimated cooked size of the whole kit: about 6 MB (+0.5 MB fonts).
- **Mode pictures**: crops of tonight's B engine stills (ar2, ar7, ar14, b41, b42, qa5), one grade (0.88 saturation, 1.12 contrast, cool shadows, vignette,
  ink lower 40 %). rinks = clay and night arena split on a diagonal; fog-of-war = goal still with a procedural haze; make_your_own = stands darkened with a
  grid and a drawn plus. No text, logos or likeness in any picture.
- **Font, the question answered**: option (a) works. UE 5.9 ships Roboto in `Engine\Content\Slate\Fonts`, including **Roboto Bold Condensed**; licence:
  `Roboto.tps` points to `Engine/Source/ThirdParty/Licenses/ROBOTO_License.txt` = **Apache License 2.0**, font data copyright Google 2012 (redistributable,
  text credit in the dev log). Only ONE condensed weight ships (no condensed Regular or Black), so `F_Hokej_Display` (runtime cache) maps
  **Regular = Roboto Regular, Bold = Roboto Bold Condensed (the display face: titles, buttons), Black = Roboto Black** (figures, format tags). Faces:
  `/JamAssets/UI/Fonts/FF_Hokej_Regular|Bold|Black` (data embedded, lazy load). No typeface invented, nothing downloaded.
- **GOAL banner**: drawn in Roboto Bold Condensed Italic (engine font) on an ink ribbon with a blue and an orange slash.
- **Mocks** (Pillow composites from the kit and the same fonts, not engine captures):
  - `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261114-142659-B-ui-mock-landing.png`
  - `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261114-142659-B-ui-mock-modes.png`
  - `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261114-142659-B-ui-kit-sheet.png`
  - `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261114-142659-B-ui-goal-banner.png` (banner over a game still at 1280x400)
- **What I looked at with Read**: A's six menu-2 stills (landing, home, modes, level switch); brand lab final-sheet and logo-alpha-dark on pages; two
  contact sheets of tonight's stills, qa5-sets-weapons-close, b44-eleven-gamecam, b42-goal at full size; the kit sheet twice (fixed fog, rinks, power-play
  HUD strip after the first); the fog/rinks pair; both mocks (modes twice: fixed header divider spacing and cut descriptions); the GOAL banner over a still.

## 3. Not verified
Nothing seen on a GPU: 9-slice in UMG, BC1/BC7 quality, Slate rendering of the font faces. The mocks are drawn by Pillow, so engine kerning and
9-slice scaling may differ slightly. The backdrop in both mocks is a PLACEHOLDER (a clay-barn still, darkened and blurred). No cook-only check run.
Asset names with hyphens (`T_UI_Mode_fog-of-war` etc.) imported and saved fine; not tested in a cook.

## 4. For device A
- Names exactly as 0028: `/JamAssets/UI/T_UI_Logo_Hokej`, `T_UI_Panel`, `T_UI_HeaderBar`, `T_UI_Card_Mode`, `T_UI_Button_Normal|Focused|Pressed|Disabled`,
  `T_UI_Segment_Lit|Unlit`, `T_UI_Banner_Goal`, `Modes/T_UI_Mode_<slug>` (slugs as cartridges, with `make_your_own`), `Fonts/F_Hokej_Display`
  (typefaces "Regular", "Bold", "Black"). Extra: `Fonts/FF_Hokej_*` (Font Faces; `FF_`/`F_` are not in the contract prefix list: A's name for the font).
- Style table values used: ink #0B0E14, panel #141A26/#1A2130, hairline #2E384C, blue #2E6BFF, orange #FF6A13 (pressed #CC4E08), white #F4F6FA, grey text
  #96A0B2. Focus: Button_Focused + an orange outer glow (8 to 10 px) and a 6 px lift on cards.
- Type sizes in the mocks (1080p): PLAY 84 Bold; menu items 46 Bold; small buttons 30 Bold; card titles 36 Bold; body 18 to 23 Regular; format tags 24 Black.
- Add `/JamAssets/UI` to the always-cook list when this is on dev-b (0028 says A does).

## 5. Decisions for the operator
- Is one condensed weight (Roboto Bold Condensed) enough for the display type? If he wants a condensed Black or a different face, nothing else ships with
  the engine; it would need a font he approves (not downloaded by B).
- Mode card pictures are from engine stills; if he wants posed hero shots per mode, that is a GPU lane.

## 6. Next three steps
1. A GPU lane opens the kit in a UMG test widget (9-slice, BC1/BC7, font) and stills it; then cook-only check before the push.
2. Swap the placeholder for `T_UI_MenuBackdrop` in the landing mock when the GPU lane lands it.
3. Optional: a GOAL banner in-animation (scale 1.15 to 1 and a slash wipe, 0.3 s) once A wires it.

## 7. Ledger rows
| Game/Plugins/JamAssets/Content/UI/T_UI_Banner_Goal.uasset | texture | 0.1 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 2048x640 RGBA, UI group, no mips, BC7; GOAL in Roboto Bold Condensed Italic (engine font, Apache 2.0) on an ink ribbon with blue and orange slashes (lane U1) (82787 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Button_Disabled.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 192x96 RGBA, UI group, no mips, UserInterface2D; 9-slice margins L/R 32, T/B 24 px; dim ink at 78 % alpha (lane U1) (11216 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Button_Focused.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 192x96 RGBA, UI group, no mips, UserInterface2D; 9-slice margins L/R 32, T/B 24 px; solid orange with white bar on the left (lane U1) (11324 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Button_Normal.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 192x96 RGBA, UI group, no mips, UserInterface2D; 9-slice margins L/R 32, T/B 24 px; ink fill, hairline, chamfer bottom right (lane U1) (11304 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Button_Pressed.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 192x96 RGBA, UI group, no mips, UserInterface2D; 9-slice margins L/R 32, T/B 24 px; dark orange, top shade, white bar (lane U1) (11353 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Card_Mode.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 256x320 RGBA, UI group, no mips, UserInterface2D; 9-slice margins L/R/T 32 px, B 40 px; white top rule for a team tint (lane U1) (11452 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_HeaderBar.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 2048x176 RGBA, UI group, no mips, UserInterface2D; stretch to 1920x88, no 9-slice; blue/orange split stripe at the bottom (lane U1) (11737 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Logo_Hokej.uasset | texture | 1.1 | original | Blender (brand lab BR4) + Pillow script |  | C:/Users/kazda/jam/brand-lab/out/final/logo-alpha-dark.png; C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 920x716 RGBA, UI group, no mips, BC7; brand lab BR4 logo-alpha-dark.png trimmed and padded (lane U1) (1066698 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Panel.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 256x256 RGBA, UI group, no mips, UserInterface2D; 9-slice margins 32 px all sides (0.125); ink panel, chamfer top right (lane U1) (11155 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Segment_Lit.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 128x64 RGBA, UI group, no mips, UserInterface2D; 9-slice margins 16 px; blue with white underline: level switch lit (lane U1) (11145 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Segment_Unlit.uasset | texture | 0.0 | original | Pillow script (ui-lab kit.py) |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 128x64 RGBA, UI group, no mips, UserInterface2D; 9-slice margins 16 px; ink with hairline: level switch unlit (lane U1) (11078 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_arsenal.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for arsenal: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (266369 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_bruisers.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for bruisers: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (303282 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_cage.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for cage: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (313820 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_fog-of-war.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for fog-of-war: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (281489 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_glass-house.uasset | texture | 0.4 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for glass-house: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (386474 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_make_your_own.uasset | texture | 0.2 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for make_your_own: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (201061 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_marathon.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for marathon: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (278042 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_outnumbered.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for outnumbered: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (336809 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_pond.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for pond: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (339057 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_power-play.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for power-play: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (323396 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_rinks.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for rinks: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (349061 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_shotgun.uasset | texture | 0.2 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for shotgun: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (236105 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_sniper.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for sniper: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (296804 bytes) |
| Game/Plugins/JamAssets/Content/UI/Modes/T_UI_Mode_sudden-death.uasset | texture | 0.3 | original | Pillow script (ui-lab kit.py) from our engine stills |  | C:/Users/kazda/jam/ui-lab/work/kit.py | 2026-11-14 | B | y | 640x360 RGB, UI group, no mips, BC1 (Default); mode card picture for sudden-death: crop of a B engine still of 14 Nov, one grade across all 14 (lane U1) (290610 bytes) |
| Game/Plugins/JamAssets/Content/UI/Fonts/FF_Hokej_Regular.uasset | font | 0.2 | sourced | Roboto shipped with UE 5.9 (Engine/Content/Slate/Fonts/Roboto-Regular.ttf) imported as a Font Face | Apache License 2.0 (Engine/Source/ThirdParty/Licenses/ROBOTO_License.txt; font data copyright Google 2012) | https://www.fontsquirrel.com/license/roboto (from the engine Roboto.tps; nothing downloaded) | 2026-11-14 | B | y | Font Face "Regular" of F_Hokej_Display, lazy load, data embedded (lane U1) (160087 bytes) |
| Game/Plugins/JamAssets/Content/UI/Fonts/FF_Hokej_Bold.uasset | font | 0.2 | sourced | Roboto shipped with UE 5.9 (Engine/Content/Slate/Fonts/Roboto-BoldCondensed.ttf) imported as a Font Face | Apache License 2.0 (Engine/Source/ThirdParty/Licenses/ROBOTO_License.txt; font data copyright Google 2012) | https://www.fontsquirrel.com/license/roboto (from the engine Roboto.tps; nothing downloaded) | 2026-11-14 | B | y | Font Face "Bold" of F_Hokej_Display, lazy load, data embedded (lane U1) (162788 bytes) |
| Game/Plugins/JamAssets/Content/UI/Fonts/FF_Hokej_Black.uasset | font | 0.2 | sourced | Roboto shipped with UE 5.9 (Engine/Content/Slate/Fonts/Roboto-Black.ttf) imported as a Font Face | Apache License 2.0 (Engine/Source/ThirdParty/Licenses/ROBOTO_License.txt; font data copyright Google 2012) | https://www.fontsquirrel.com/license/roboto (from the engine Roboto.tps; nothing downloaded) | 2026-11-14 | B | y | Font Face "Black" of F_Hokej_Display, lazy load, data embedded (lane U1) (166405 bytes) |
| Game/Plugins/JamAssets/Content/UI/Fonts/F_Hokej_Display.uasset | font | 0.0 | sourced | UE Font asset over the three Roboto Font Faces | Apache License 2.0 (Engine/Source/ThirdParty/Licenses/ROBOTO_License.txt; font data copyright Google 2012) | https://www.fontsquirrel.com/license/roboto (from the engine Roboto.tps; nothing downloaded) | 2026-11-14 | B | y | runtime cache; typefaces Regular = Roboto Regular, Bold = Roboto Bold Condensed, Black = Roboto Black (lane U1) (3425 bytes) |
