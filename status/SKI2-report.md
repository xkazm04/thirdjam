# SKI2 report: the concept-sheet figures P2C and P3C in the engine (device B)

Lane SKI2, 2026-11-12, 21:52 to 22:46 machine clock. Main tree `C:/Users/kazda/jam/thirdjam` (branch `dev-b`), editor on port 8000 (started by me, left through `ExitEditor` with no dirty package; last world an untitled blank map). No build, no git command except read-only `git log`. Checkpoint: `status/SKI2-checkpoint.md`. Scratch (every script, measurement JSON and full-size image): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/ski2`.

**Scope change at 22:50 (coordinator, from the operator):** clay is the next priority tier, broadcast is rejected. P2C and P3C are done; **P4D is deferred and `BP_SkaterVisual_TB2_P4` still drives the old P4B figure, untouched** (1,688,460 bytes, SK5's file). The P4D import had already run when the message arrived (mesh, skeleton, 72 takes, sockets, loop wraps measured); I deleted all 74 P4D assets, so nothing of P4D remains on disk or in the ledger. P3C got the operator's felt-jersey material pass.

**Housekeeping:** `/Game/Maps/Rink` never saved (mtime 11:10), discarded through `new_blank_map(False)`; `jam.Cut` back to `hockey.regular`; `_Scratch/` does not exist; the only files changed under `Game/` are in `Plugins/JamAssets/Content/Skaters/` (checked with `find -newer`). Another editor shared the GPU: no frame numbers.

## Verdict per figure and step

| Step | P2C (`BP_SkaterVisual_TB2`, night and every other tier) | P3C (`BP_SkaterVisual_TB2_P3`, clay) |
|---|---|---|
| 1 Import | **PASS** | **PASS** |
| 2 Material | **PASS** | **PASS** (+ felt jersey, see below) |
| 3 AnimBP + Blueprint | **PASS** | **PASS** |
| 4 In a match | **PASS** (Knockdown_Left seen on a test figure only) | **PASS** (two match knockdowns; sides on test figures) |

### 1. Import (both)
- Loopcut FBX from `skc/export2/<fig>/`, new skeleton per figure, no physics asset, 72 takes `AS_SkaterGen_<fig>_<Take>`, `animation_length` exported time.
- Loop wraps (hand / blade cm, last to first frame, against the mean step): P2C Stride loopcut 0.56 / 1.03 (mean 0.74 / 1.40), main file 0.00 but one repeated frame: **kept the loopcut**. P3C Stride 0.25 / 0.60 (mean 0.62 / 1.57); every other loop at or under its mean step.
- Sockets added (`AddSocket`): `stick_socket` and `head_socket` on their bones, `puck_socket` on bone `stick_socket` at (5.36, -153.76, 8.96), the values of `SK_SkaterTB2`. The stick bone's rest orientation is identical to R's (socket world rotation 0/0/0 at rest on both), and the stick is the same S4 stick, so R's values are exact.
- Texture: `T_SkaterGen_<fig>_BaseColor`, 2048 source, `MaxTextureSize` 1024, BC1 (`CompressionNoAlpha`), Lossy Medium, VT off, LOD group World. 1K is sharp in the 2 m front close-up (`ski2-p2c-front-close`): no 2K needed.

### 2. Material (instances of `M_SkaterTB2`)

| | P2C `MI_SkaterGen_P2C` | P3C `MI_SkaterGen_P3C` |
|---|---|---|
| Hue key | KeyGain 10, KeyBias 2.3 | KeyGain 8, KeyBias 1.3, Roughness 0.9 |
| What keys (measured on the PNG, then seen in stills) | deep blue jersey, helmet, socks (b - max(r,g) 0.35-0.38); **navy pants and mittens stay navy** (0.16), face, cage, skates, white trim and laces untouched | deep blue jersey, helmet, socks (0.29-0.33); dark navy pants and mittens (0.01), cream stitches, face, cage, skates untouched |
| Number box | Z 88, Y -6, W 40, H 24, NumberWidth 0.45, NumberHeight 0.6 | same |
| Crest | Z 97, Y -7, size 10 | same |
| Jersey Emissive (`SetBoost`) | **1.25 none / 1.75 boost** (A's 1.0-1.5 band; 2.5 went cyan) | 0.5 / 1.0 (unchanged: clay passes Emissive through) |

- Numbers 7, 13, 42, 88 and both crests read in the stills from behind and in front. The number had to go up and left: at Z 85 the right mitten (which rides at the right hip in Glide) covered the second digit.
- **Felt jersey for clay (operator's concept, `prep-reference/images/path-p3-t*.png`, looked at only):** I added four nodes to `M_SkaterTB2` (group "Felt (SKI2)"), all off by default: `SKI2JerseyMask` (the hue key's own mask x `JerseyFelt` x below `FeltMaxZ` in pre-skinned space), `SKI2FeltNormal` (a fine fibre bump from `Look/Textures/T_Felt_Fibre`, triplanar finite differences, blended over the clay dent normal by the mask), `SKI2FeltRimBoost` (multiplies the clay tier's `FeltRim` by 1 + mask x `FeltRimBoost`). Parameters `JerseyFelt` (default 0), `FeltTile` 6, `FeltBump` 0.6, `FeltRimBoost` 1.5, `FeltMaxZ` 100000. With `JerseyFelt` 0 the normal and rim are exactly the old ones (lerp by 0, x 1), and both paths only reach the output through `ClayMixNormal` / `ClayMixEmissiveFelt`, which return the night and broadcast values unchanged when `Clay` <= 0: every other figure is unchanged by construction (not pixel-diffed). The backup of the old master is `$SKI2/bk/M_SkaterTB2.uasset.bak`.
- `MI_SkaterGen_P3C`: JerseyFelt 1, FeltBump 2, FeltRimBoost 4, FeltTile 4 cm, **FeltMaxZ 99 cm** (the helmet is the same blue as the jersey, so the key alone would have made the helmet felt too; the height cut keeps the helmet, face and cage clay). The cream stitches stay out of the key.
- **How close to the concept:** partway. The jersey now has a soft fuzzy sheen and a fine grain that the clay parts do not, and the helmet, gloves and skates keep the thumb-dent clay. It does not reach the concept's figure: the concept's felt is a thick, hairy, dense texture with **thick cream cross-stitches** at every seam and hem; P3C's texture has thin white running stitches, and at 8 m with the clay rig's depth of field the felt reads as a matte sheen, not as fibre. The concept's face is a simple clay face without a cage; P3C has the cage. Getting closer needs the texture (bold stitches) or the mesh, not the material.

### 3. AnimBP and Blueprint (both)
- `ABP_SkaterGen_<fig>`: copy of `ABP_SkaterTB2` on the new skeleton, every player re-pointed by name; the `Idle` state plays `Idle` (SK5), faceoff stays `Faceoff_Ready`. Compiles `BS_UP_TO_DATE`.
- `BS_SkaterGen_<fig>_Skate` (layout of `BS_SkaterTB2_Skate`, 14 samples) and `_Aim` (24 holds, axes -63..70 x 1.0..1.6). The bilinear aim (four reach rows) and the blade-on-ice Modify Bone node come with the AnimBP copy.
- **The Blueprint graph** (written by `$SKI2/gen.mjs`): `BP_SkaterVisual_TB1`'s graph, which is the only one with SK5's behaviour, re-pointed to `AS_SkaterGen_<fig>_*` and `ABP_SkaterGen_<fig>`, with TB2's material path (slot-0 `JerseyMID`, no helmet MID), TB2's blade offset (134.87, 0, -59.23) and the jersey Emissive above. So both figures now have: the nine stick calls, 24-hold aim with the correction, knockdowns by direction (`Knockdown_Back` from behind and `Trip` are now wired, the export2 joins are exact), matching get-ups held and learned early, check delivery by side, `PlayHitReaction` / `PlayStickHit` (0.12 s blend-in) / `PlayBoardHit` / `SetBoardPin` / `SetTripped`, `Stumble_Short` cut 0.095 s after a mistimed poke, `Trip` -> `GetUp_Side_Right`, knockdown play rate 0.95 + 0.2 x strength from the `knockdown` bus event, the `FMax(LastGapS, 0.1)` guard, get-up blend 0.12 s. Mesh component: `SK_SkaterGen_<fig>`, anim class `ABP_SkaterGen_<fig>_C`. CDO `H0` kept (5.8 P2C, 5.2 P3C), `FixGain` 0.6.
- **The interface events were not on the C++ interface:** `Game/Source/JamCore/Public/JamSkaterVisual.h` has `SetKnockedDown(bool, float, float)` only, the last commit there is 99a152b (A39). So `PlayHitReaction`, `PlayBoardHit`, `SetBoardPin`, `PlayStickHit`, `SetTripped` stay Blueprint events, as on the old figures.
- `_old` copies: `BP_SkaterVisual_TB2_old`, `BP_SkaterVisual_TB2_P3_old` are duplicates of the Blueprints as they stood at 22:00, made self-contained (their `PlayCheck` now finds their own class); both compile clean and render beside the new ones in PIE.

**Measured on test figures in PIE (15 cell centres + 6 holds, driven every tick, H0 calibration of the earlier lanes; P2C bottom = centre - 5.3, P3C - 4.7):**

| | P2C Glide | P2C Stride | P3C Glide | P3C Stride |
|---|---|---|---|---|
| Blade error at cell centres, worst (cm) | 0.0 | 0.8 | 0.0 | 0.4 |
| Blade bottom (cm, target -1..+4) | +0.5 | -0.4..+1.7 | +0.5 | 0.0..+1.3 |
| Correction used, max (pitch / yaw / slide) | 2.4 deg / 3.6 deg / 2.9 cm | 3.4 / 4.5 / 5.2 | 2.3 / 2.6 / 2.5 | 2.4 / 3.3 / 3.3 |
| Left wrist to shaft line, two-handed holds (cm) | 6.4-15.5 (mitten r 12.2) | same | 4.8-11.8 (mitten r 11.5) | |

The correction does much less work than on the old figures (SK3: R up to 4 / 3.8 / 6.2 in Glide). The far-right cells (-54.5 deg, 1.525 m) are one-handed holds (43 cm P2C, 59 cm P3C), as on every figure.

**Carry in the bot match** (puck to blade centre, design 5 cm; puck to `puck_socket` under 4 cm): P2C 12 frames with a 13 deg turn **4.9-6.9 cm**, blade bottom -0.7..+0.2; over 862 carried frames 2.9-8.6 cm. P3C 12 frames with a 17.5 deg turn **5.0-5.5 cm**; 1,312 carried frames 2.6-8.7 cm.

**Calls on test figures, montage log (both figures identical):** `PlayPoke(false)` -> `Poke`, `Stumble_Short` 0.083-0.095 s later; push (0,1) / (0,-1) / (1,0) / (-1,0) -> `Knockdown_Left` / `_Right` / `_Back` / `Fall`, held until release, then `GetUp_Side_Left` / `_Side_Right` / `GetUp_Front` / `GetUp`; `SetTripped` -> `Trip` -> `GetUp_Side_Right`; `PlayHitReaction` -> `Check_Hit_Left` / `_Right`; `PlayStickHit(false)` -> `Poke_Hit`; `PlayBoardHit` -> `Board_Hit`; `PlayLift`, `PlayPass`, `PlayFoulReaction`, `PlayShoot(0)` -> `Shot_Wrist`, `SetShotCharge(0.6)` (alpha 1) then `PlayShoot(0.6)` -> `Shot_Slap` from 0.177 s, `SetFaceoff` (pelvis 50 cm, `Faceoff_Ready`) then `PlayFaceoffDraw`, `SetGoalieStance` -1 / +1, `PlayCheck` -> `Check_Deliver_Right`.

### 4. In a match (`generated_b2`, scratch cartridges `t_night` / `t_clay`, human driven through `DebugInjectKey` as SK4/SK5)
- Log names the right class: `JAMVISUAL generated_b2 tier=l2_night class=.../BP_SkaterVisual_TB2...`, `tier=p3_clay class=.../BP_SkaterVisual_TB2_P3...`; six figures each, numbers 7 13 42 / 9 21 88.
- P2C: `Check_Deliver_Left` -> victim `Fall` (filmed by the driver), then in 100 s: `Knockdown_Back` x3, `Knockdown_Right` x2, `Fall` x2, each followed by its get-up (`gap 1.5`), rates 0.99-1.15 from the strength. P3C: `Fall` and `Knockdown_Back` with their get-ups. `Knockdown_Left` was not drawn in a match (the driver always hit from the same side); it plays on the test figures.
- **No error or warning naming the figures' assets during play**, no `Accessed None`, no divide by zero in the whole session. (One transient compiler error was logged when `BP_SkaterVisual_TB2_old` was first duplicated, before its `PlayCheck` was re-pointed to itself; it compiles clean since.)

## Stills (`devlog/shots/`)

| Files | What |
|---|---|
| `20261112-221823/24-B-ski2-p2c-{8m-back,24m-back,8m-front,front-close,back-close}` | P2C beside the old R figure, both colours, numbers and crest |
| `20261112-222314/15-B-ski2-p2c-{chr,trip,kdback,kdleft,kdright,kdfront,poke,kdback-side}` | strips at the game camera |
| `20261112-223516/17/18-B-ski2-p3c-{8m-back,24m-back,8m-front,back-close,chr,kdback,kdleft,trip,poke}` | P3C beside P3A on clay, strips |
| `20261112-224404/05-B-ski2-p3c-felt-{8m-front,8m-back,ab,close}` | the felt jersey at 8 m beside the old clay figure |

## The flagged takes, as a player sees them at the game camera
- **`Check_Hit_Right` (P2C, P3C):** the stick swings up and stands beside the helmet for about 0.2 s; it does not read as going through the helmet.
- **`Trip` (P2C):** the figure goes down on its left hip and the stick ends behind the big helmet; from behind it looks like a stick lying behind the head, not a clip.
- **Face-down skates up (`Knockdown_Back`):** from the side the skates rest a few cm up on the toes; from the game camera (behind) you see the soles and the back: unremarkable.
- P4D's `Fall` and `Aim_P70_R130`: not looked at (P4D deferred).

## Honest comparison at the game camera
- **P2C vs R (night):** close up P2C wins clearly (cage, a real face, mittens, laces). At 8 m both read, both numbers read; P2C's numbers are a little smaller. **At 24 m they are about equal; P2C does one thing worse:** its navy pants and mittens stay navy, so the orange P2C is less orange overall than the all-orange R (R recolours its pants), and the team read rests on the jersey and helmet. The big helmet also covers more of the upper back. The blade sits on the ice much better (correction work halved).
- **P3C vs P3A (clay):** P3A is still more of a clay puppet (rounded forms, white mittens, no cage); P3C reads more as hockey (cage, seams, dark pants and skates), and with the felt jersey it now has two materials like the concept. Motion: P3C is clearly better (no stick pops, holds within 1 cm). At 24 m on clay the depth of field makes them nearly the same.

## Takes wired, held back or replaced
- **P2C and P3C: all 72 imported; wired** as listed in section 3, including `Knockdown_Back` and `Trip` (newly wired on the generated figures; the old ones fell back to `Fall_Forward`). Imported but played by nothing (as on TB1): `Idle_Faceoff`, `Stumble` (replaced by `Stumble_Short`), `Check`, `Celebrate`, `Stop`, `Shot_Charge`, `Aim_Front/_Left/_Right/_FarReach` (the 24-hold grid replaces them), `Fall_Forward` (the face-down fall is `Knockdown_Back`).
- **Held back / replaced: none** on P2C and P3C. P4D (`Board_Hit` -> `Check_Hit_Back`, `Lift` -> `Poke`) was prepared in `gen.mjs` but not applied.

## Validation (`/JamAssets/Skaters/`, after all edits)
480 assets loaded, 0 failures; `validate_assets_with_settings`: 480 checked, 480 valid, 0 invalid, 0 warnings; every blend space full (6 Skate x 14, 6 Aim x 24, 0 empty samples); no Error or Warning line in the log during the load and validation.

## Assets created or changed (`Game/Plugins/JamAssets/Content/Skaters/`, bytes)

**Created:** `SK_SkaterGen_P2C` 5,217,192; `SKEL_SkaterGen_P2C` 14,029; `T_SkaterGen_P2C_BaseColor` 2,747,761; `MI_SkaterGen_P2C` 16,887; `BS_SkaterGen_P2C_Skate` 28,284; `BS_SkaterGen_P2C_Aim` 51,922; `ABP_SkaterGen_P2C` 645,296; 72 `AS_SkaterGen_P2C_*` 13,285,109. `SK_SkaterGen_P3C` 5,222,031; `SKEL_SkaterGen_P3C` 13,106; `T_SkaterGen_P3C_BaseColor` 2,955,602; `MI_SkaterGen_P3C` 19,732; `BS_SkaterGen_P3C_Skate` 28,240; `BS_SkaterGen_P3C_Aim` 51,878; `ABP_SkaterGen_P3C` 645,283; 72 `AS_SkaterGen_P3C_*` 13,211,522. `BP_SkaterVisual_TB2_old` 1,688,227; `BP_SkaterVisual_TB2_P3_old` 1,687,984.

**Changed:** `BP_SkaterVisual_TB2` 1,688,066 -> 1,915,955 (P2C); `BP_SkaterVisual_TB2_P3` 1,687,686 -> 1,915,742 (P3C); `M_SkaterTB2` 66,746 -> 74,020 (felt path, off by default). **Not changed:** `BP_SkaterVisual_TB2_P4`, everything TB1, `Look/`, `Rigs/`, `VFX/`, `Audio/`, `Props/`.

**Ledger rows to add** (takes: one row per take; per-take sizes are 0.13-0.34 MB, listed in `$SKI2` by `stat`):
```
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P2C.uasset | mesh | 4.98 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | y | skc/export2/P2C loopcut FBX; Rodin task c439ca2c-510e-4831-8c7f-0a50b652d62e, Meshy rig 01a1d137-c464-71be-b85a-5eb778cf905a; root, sockets, boot weights by skc/retarget.py; S4 stick (original); sockets stick/head/puck as SK_SkaterTB2 (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterGen_P2C.uasset | other | 0.01 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | y | own skeleton of P2C, 32 bones; Rodin c439ca2c-510e-4831-8c7f-0a50b652d62e, Meshy rig 01a1d137-c464-71be-b85a-5eb778cf905a |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P2C_BaseColor.uasset | texture | 2.62 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | y | P2C base colour, 2048 source, MaxTextureSize 1024, BC1 no alpha, Lossy Medium; Rodin c439ca2c-510e-4831-8c7f-0a50b652d62e, Meshy rig 01a1d137-c464-71be-b85a-5eb778cf905a |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterGen_P2C_<Take>.uasset | anim | 0.13-0.34 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | y | skf/export5 take retargeted onto P2C by skc/retarget.py (skc/export2/P2C loopcut), 72 takes (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P2C_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | Speed x TurnRate, layout of BS_SkaterTB2_Skate (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P2C_Aim.uasset | anim | 0.05 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | 24-hold aim grid (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterGen_P2C.uasset | other | 0.62 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | copy of ABP_SkaterTB2 on SKEL_SkaterGen_P2C, Idle state plays Idle (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterGen_P2C.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_SkaterTB2: KeyGain 10, KeyBias 2.3, number box Z 88 Y -6 W 40 H 24, crest Z 97 Y -7 size 10, Emissive 1.25 (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P3C.uasset | mesh | 4.98 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | skc/export2/P3C loopcut FBX; Rodin task cb0f0a16-7ce4-4b84-87c7-78027c268ee8, Meshy rig 01a1d14e-aaf9-736c-8ec3-ee298af3bd93; root, sockets, boot weights by skc/retarget.py (ankle_z override 0.21); S4 stick (original) (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterGen_P3C.uasset | other | 0.01 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | own skeleton of P3C, 32 bones; Rodin cb0f0a16-7ce4-4b84-87c7-78027c268ee8, Meshy rig 01a1d14e-aaf9-736c-8ec3-ee298af3bd93 |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P3C_BaseColor.uasset | texture | 2.82 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig | | | 2026-11-12 | B | n | P3C base colour, 2048 source, MaxTextureSize 1024, BC1 no alpha, Lossy Medium; Rodin cb0f0a16-7ce4-4b84-87c7-78027c268ee8, Meshy rig 01a1d14e-aaf9-736c-8ec3-ee298af3bd93 |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterGen_P3C_<Take>.uasset | anim | 0.13-0.33 | original | Blender 4.2 script, retargeted | | | 2026-11-12 | B | n | skf/export5 take retargeted onto P3C by skc/retarget.py (skc/export2/P3C loopcut), 72 takes (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P3C_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | Speed x TurnRate, layout of BS_SkaterTB2_Skate (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P3C_Aim.uasset | anim | 0.05 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | 24-hold aim grid (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterGen_P3C.uasset | other | 0.62 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | copy of ABP_SkaterTB2 on SKEL_SkaterGen_P3C, Idle state plays Idle (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterGen_P3C.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | instance of M_SkaterTB2: KeyGain 8, KeyBias 1.3, Roughness 0.9, number/crest as P2C, felt jersey JerseyFelt 1 FeltBump 2 FeltRimBoost 4 FeltTile 4 FeltMaxZ 99 (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2_old.uasset | other | 1.61 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | copy of BP_SkaterVisual_TB2 before lane SKI2 (R figure), PlayCheck re-pointed to its own class (lane SKI2) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2_P3_old.uasset | other | 1.61 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | n | copy of BP_SkaterVisual_TB2_P3 before lane SKI2 (P3A figure), PlayCheck re-pointed to its own class (lane SKI2) |
```
**Rows to update (notes, size):** `BP_SkaterVisual_TB2` 1.83 MB: "drives P2C (SK_SkaterGen_P2C, ABP_SkaterGen_P2C) with TB1's SK5 graph and TB2's material path; Knockdown_Back and Trip wired; jersey Emissive 1.25/1.75 (lane SKI2)". `BP_SkaterVisual_TB2_P3` 1.83 MB: same for P3C, Emissive 0.5/1.0. `M_SkaterTB2` 0.07 MB: "+ felt jersey path (SKI2JerseyMask, SKI2FeltNormal, SKI2FeltRimBoost; JerseyFelt default 0 = unchanged) (lane SKI2)".

## Old assets that could now be deleted (only if the lead says no switch-back; they are what the `_old` Blueprints use)
- **R figure (old P2 generated)**, 70 files, 17,872,239 bytes: `SK_SkaterTB2`, `SKEL_SkaterTB2`, 66 `AS_SkaterTB2_<Take>` (not `_P3_`/`_P4_`), `BS_SkaterTB2_Skate`, `BS_SkaterTB2_Aim`, `ABP_SkaterTB2`, `T_SkaterTB2_BaseColor`; plus `BP_SkaterVisual_TB2_old`. Keep `MI_SkaterTB2_Trim` and `MI_SkaterTB2_Stick` (the new figures use them).
- **P3A figure**, 73 files, 17,735,539 bytes: `SK_SkaterTB2_P3`, `SKEL_SkaterTB2_P3`, `AS_SkaterTB2_P3_*`, `BS_SkaterTB2_P3_Skate/_Aim`, `ABP_SkaterTB2_P3`, `MI_SkaterTB2_P3`, `T_SkaterTB2_P3_BaseColor`; plus `BP_SkaterVisual_TB2_P3_old`.
- Rclay (`MI_SkaterTB2_Clay`, `T_SkaterTB2_Clay_BaseColor`, `T_SkaterTB2_Clay_Normal`, `M_SkaterGen`, 4.2 MB) is referenced by nothing I know of; check before deleting.
- Delete through the editor (reference-checked) and remove their ledger rows.

## What device A must know
- **Class names unchanged**: `BP_SkaterVisual_TB2` now shows P2C, `_TB2_P3` shows P3C, `_TB2_P4` still shows P4B. No code change needed.
- The four reaction calls (C-0001 v4) are not in this tree's interface yet; when A45 lands, B moves the five Blueprint events on all figures onto the interface (the new Blueprints use the same event names and parameters).
- `knockdown_frames` still not passed: the first get-up of each skater is 0.9 s late, as before.
- P2C's back number sits lower and is smaller than R's (the big helmet covers the upper back): readable at 8 m, not at 24 m (neither was R's).

## What I could not verify
- `Knockdown_Left` and `Trip` from the pawn in a match (the driver never hit from that side; nobody sends a trip); hit reactions, board hit, pin and stick hit in a match (A sends none).
- The felt path on every other figure pixel-for-pixel (it is a no-op by construction at `JerseyFelt` 0 and outside clay), and the felt look without the clay rig's depth of field (every clay still is softened by the f/3.2 at 7 m focus).
- Frame cost (shared GPU). Cooked sizes (no cook).
- P4D anything in the engine beyond its import (deleted).

## What the lead must decide
1. **Switch back to an `_old` Blueprint: my recommendation is no for both.** P2C and P3C keep every behaviour of the old figures and add SK5's (the generated figures were behind on it), hold the stick on the ice with half the correction, and P3C removes P3A's stick pops. The one look loss is P2C's navy pants (less orange at 24 m). To switch back: rename `BP_SkaterVisual_TB2_old` to `BP_SkaterVisual_TB2` (after renaming the new one away), or have A point the tier lookup at the `_old` class.
2. If no switch-back: delete the old R and P3A sets listed above with their `_old` Blueprints (35.6 MB on disk).
3. P2C pants: leave navy (it is the concept), or recolour them with a second key (they are navy, so a key would need a separate range).
4. Clay felt: to get nearer the concept's figure, the texture needs bold cream cross-stitches and a fuzzier jersey (a Blender/texture lane); the material path is ready for it.
5. P4D: re-import when broadcast comes back (import plus the Blueprint is about 20 minutes with `$SKI2` scripts; `gen.mjs` already falls back `Board_Hit` -> `Check_Hit_Back` and `Lift` -> `Poke`).
6. Paste the ledger rows.
