# SK7 checkpoint (lane SK7, device B, tree wt/b-a45, editor port 8000 pid 16156)

## 02:51
Saved on disk so far (all under `Game/Plugins/JamAssets/Content/Skaters/`):
- `T_SkaterGen_P3C_BaseColor_v2`, `_Normal`, `_Mask`: re-imported from `skater-lab/skt/out_sk7/*_1024.png` (script `skt/build_maps_sk7.py`: dashes 8.0 x 2.6 cm, gap 4.4, dome 4 mm, felt relief x3, clay dents x2.5, dark felt band 2.4 cm at hem and cuffs).
- `M_SkaterTB2`: group "Felt (SK7)": SheenWhite (0.5), SheenTint (1,1,1), FeltMottle (0), FeltMottleTile (8), ClayBumpScale (1): defaults = old output.
- `M_SkaterTB1Jersey`: group "Felt (SK7)": FeltMix (0), FeltTile, FeltBump, FeltMottle, FeltMottleTile, FeltDarken, HemZ0/HemZ1/HemK, SheenWhite, SheenTint, ClayBumpScale: defaults = old output.
- `MI_SkaterGen_P3C`, `MI_SkaterTB1_P3_JerseyDeco`, `MI_SkaterTB1_P3_TrimDeco`: first felt values saved (normal map ON at strength 1.6; cream trim).
Not saved yet: live tuning in PIE (JerseyLum 0.32, FeltMottle 0.7, NormalStrength 2.2, FeltDarken 0.62, FeltBump 3).
Before stills are in scratch `sk7/img/sk7-*-before.png` (not yet filed in devlog/shots).
Normal map: the SK6 wash did not reproduce in tonight's rig; the map was simply too weak at strength 1 (median tilt 1 degree).
Pre-lane copies of every touched asset: scratch `sk7/bk/`.

## 03:10
Saved on disk (all `Game/Plugins/JamAssets/Content/Skaters/`), dirty list empty after each save:
- `SK_SkaterTB1_P3`: mesh-only re-import from `skater-lab/skf/export7c/p3_clay/SK_Skater_p3_clay_loopcut.fbx` (stitches x2: 8.0 x 2.8 cm, 38 dashes; cuffs x1.3; skeleton and all 72 takes identical to export6, `compare_anim.py` max diff 0.0); Pads slot now `MI_SkaterTB1_P3_PadsFelt` (new).
- `MI_SkaterTB1_P3_PadsFelt` (new): instance of `M_SkaterTB1Jersey`, dark navy felt breezers.
- `MI_SkaterGen_P3C`: normal map ON, NormalStrength 3.5, JerseyLum 0.30, FeltRimBoost 3.5, FeltMottle 0.7, SheenWhite 0.15, SheenTint warm, ClayBumpScale 0.4, RoughClay 0.8.
- `MI_SkaterTB1_P3_JerseyDeco`: FeltMix 1, FeltBump 3, FeltMottle 0.7, FeltDarken 0.62, hem band Z 86 to 93.5 x 0.62, warm sheen. `_TrimDeco`: cream (0.80, 0.70, 0.50), FeltMix 1.
- Night figures on the same masters looked normal in a night match (TB1 and TB2), not pixel-diffed.
Still to do: final stills into devlog/shots, composites, asset validation, cvars back, editor exit, report.

## 03:20 (final)
Editor exited through ExitEditor at 03:13 (pid 16156 gone), dirty lists empty, 645 / 645 skater assets load and validate, cvars restored.
26 stills and composites filed in `devlog/shots/` (`*-B-sk7-*`). Report: `status/SK7-report.md`. Ten files changed under `Game/Plugins/JamAssets/Content/Skaters/` (one new). No git write, no cook.
