# B5 report: generated skater, track T-B2 (device B)

Lane B5, 2026-11-12, about 10:45 to 11:10 local (far inside the 14:45 box). Branch `lane/b-skater`. Editor on port 8002 (pid 11976), driven only by this lane.
Scratch scripts: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b5/` (call it `$S`).
Meshy work folder (outside every repo): `C:/Users/kazda/jam/meshy-work/`, ledger `meshy-ledger.jsonl` in it.

## Verdict in one paragraph

The import path works end to end. Figure 1 imports, plays all clips, takes team colour through a hue-key material, sits behind `IJamSkaterVisual` in a
Blueprint with an AnimBP (idle, stride, down; shot and check as montages), and looks like a chunky toy skater under the P2 toy ramp and outline.
**Figure 1 cannot ship**: its texture carries numerals ("29"/"25") on the jersey back and sleeve and small marks on the socks and helmet. All its rows are
`shipped = n`, "test only: number on sleeve". Step 5 (swap in the test cartridge, A's bot tests) is **not verified**: the game has no `JamSkater` pawn,
no code that reads `game.skater.visual`, and no bot tests on `main` yet. I did not touch the cartridge.

## Preconditions

| Precondition | Result | Evidence |
|---|---|---|
| Meshy key, at least 120 credits | PASS | `balance` answered 1204 at start. |
| Blender 4.2 for `inspect.py` | PASS | Inspection ran (headless, harmless TBBmalloc warning). |
| Seams in | PASS | `IJamSkaterVisual` in `JamSkaterVisual.h`, `UTJAnimGraphEdLibrary` callable from Python (`add_state`, `add_transition` worked), `JamSkaterVisual` interface added to the BP. |
| Editor handed over | PASS | Own editor on 8002, 40 GB free at start. |

## Steps and acceptance

| Step | Result | Evidence |
|---|---|---|
| 1 Generate (MESHY.md 6) | PASS | Preview 1 had separate limbs but adult proportions; one retry gave a chunky toy with separate limbs; picked preview 2 (thumbnails filed `tb2-preview-1`, `tb2-preview-2`, operator may overrule). Refine 2K, rig `ue5_mannequin`, stride and shot text-to-motion, three animate calls, all downloaded and inspected. |
| 2 Import | PASS | `SK_SkaterTB2` 170 cm, 20,733 triangles, 14,605 vertices; the root-less skeleton got a bone `Armature` (29 bones, see below); 5 clips on the one skeleton, mesh import off. |
| 3 Material | PASS (with the lead's rework) | `M_SkaterTB2` under `Skaters/TB2/` (copy of `M_Toon`, `M_Toon` untouched), hue-key recolour, stencil 1 on the mesh component. Two-colour still filed. |
| 4 `BP_SkaterVisual_TB2` + AnimBP | PASS | Actor BP implements all 8 calls (verified: all 8 callable from Python on a PIE actor); `ABP_SkaterTB2` states Idle, Stride, Down, `Speed` and `bKnockedDown` variables, DefaultSlot for montages; both compile `BS_UP_TO_DATE`. |
| 5 Swap into test cartridge, bot tests | NOT VERIFIED | `hockey.test.visual-b2.cartridge.json` already has `"visual": "generated_b2"`; nothing in `Game/Source` or `main` consumes it; no bot tests. |
| 6 Ledger | rows below |  |

| Acceptance | Result | Evidence |
|---|---|---|
| Skates, shoots, falls in PIE behind the interface; four stills | PASS | PIE on `Rink` (never saved) + `LR_L2_Night` as sub-level; driven through `set_team`, `update_motion`, `play_shoot`, `set_knocked_down`. Stills: `tb2-skate`, `tb2-shoot`, `tb2-fall`, `tb2-idle-teams`, plus `tb2-teams-hue-key`. |
| Triangles under 25k, one texture at 2K or less, size of `Skaters/TB2/` | PASS | 20,733 triangles; one 2048 x 2048 texture; `Skaters/TB2/` 8,843,000 bytes of `.uasset` (whole `Skaters/` 8,993,819 bytes; mesh 3.6 MB, texture 3.4 MB). |
| Six figures, gpuMs and game-thread ms against six stand-ins | PASS (noisy) | PIE, device B, floating window, two other editors idle, 40 samples each, medians: six figures gpu 12.65 / game 17.87 ms; six cylinder+sphere stand-ins gpu 12.96 / game 17.88; no figures 10.79 / 18.15. Earlier rounds in the same run (shaders still compiling) read 23.5 / 21.1 gpu; the game thread is capped by the editor frame, so the difference is inside the noise. Device B numbers, not the gate. |
| Credits spent | see below | 81 credits in the ledger. |

## Meshy tasks and credits (ledger file `meshy-ledger.jsonl`)

| Call | Task id | Credits |
|---|---|---|
| preview 1 (rejected, adult proportions) | 01a1cf1f-fdd8-7380-9c4f-5c64c72aa5ee | 5 |
| preview 2 (picked, chunky toy) | 01a1cf20-d146-7254-bd64-91833134d9fe | 5 |
| refine 2K (figure 1, used) | 01a1cf21-abca-71e0-8b89-ded695bb2f20 | 10 |
| rig `ue5_mannequin` | 01a1cf22-143c-7425-8d02-8ce566754261 | 5 |
| motion prime 4 s, stride | 01a1cf22-5993-7679-bf8f-c65be3dbae5e | 10 |
| motion prime 3 s, slap shot | 01a1cf22-8a91-7712-8132-b7f0bb2bd0b1 | 10 |
| animate stride | 01a1cf22-c87c-77df-b384-91ea6830dcf4 | 3 |
| animate shot | 01a1cf22-f99e-70ed-a4b5-24c4d37b01f8 | 3 |
| animate library 0, 187, 323 | 01a1cf23-2a80-71d4-a100-9897e4a1d58e | 9 |
| **Subtotal, figure 1 (used)** | | **60** |
| refine 2K, cleaner prompt (figure 2, **not used**) | 01a1cf33-4b14-73a0-829f-49d05ef2a0ae | 10 |
| rig of that refine (not used) | 01a1cf34-1220-7119-9933-1f10662d223c | 5 |
| animate stride on that rig (not used) | 01a1cf34-4fb2-7637-94db-fc79c4698387 | 3 |
| animate shot on that rig (not used) | 01a1cf34-8129-72a4-8f15-c157fab24afb | 3 |
| **Total** | | **81** (limit 130) |

**My mistake, said plainly:** I saw the numerals at about 11:00 and started a regenerate before the lead's "do not regenerate" message arrived. I stopped the
background chain as soon as the message came; the fourth call (library animate, 9 credits) may or may not have been sent (it is not in the ledger). The figure 2
texture is a better figure (cleaner, one small mark on the sleeve) but a different UV layout and a different mesh, so none of it is imported. Its download
URLs expire after 3 days (rig and animation); the refine files are in `meshy-work/REFINE2/`. Balance read 920 at 11:00, which is lower than 1204 - 81, so
other lanes spend on the same key.

## What the import did (the three known risks)

1. **Root-less 28-bone skeleton**: the FBX importer **adds a root bone `Armature`** (29 bones). `Armature` carries a world scale of 100 (the FBX is in metres)
   and a roll of 90; the other bones compensate. Clips have 28 tracks (no `Armature` track), so `Armature` stays at the reference pose.
   `Armature` is therefore the root; `puck_socket` hangs on it.
2. **Pelvis drift**: all travel is on `pelvis` (the track translations are in metres, bone-local: Z is forward, X lateral, Y vertical). Stride drifted 1.8 m forward
   and 0.3 m sideways. Stripped on import by `strip.py` (pelvis X and Z set to frame 0 on Stride, Shoot, Idle, Golf; the vertical bob is kept). **Fall** keeps its
   0.4 m backward and 0.4 m down travel on purpose (it is the fall itself), so the figure slides about 0.4 m back; the pawn does not move. `enable_root_motion` is false on all clips.
3. **Sockets**: `stick_socket` on `hand_r`, `head_socket` on `head` (190 cm above the floor, over the head), `puck_socket` on `Armature` 60 cm ahead of the figure
   on the ice (z 0). All three have relative scale 0.01 so that the world scale of an attachment is 1 (without that, anything attached got 100 x). The mesh faces +Y
   in mesh space; the BP component has relative yaw -90 so the figure faces +X, and `UpdateMotion` sets the mesh world yaw to `Facing.yaw - 90`.

## Clips animated on figure 1's rig, on another mesh with the same bone names (lead question 3)

The clips were imported with `skeleton = SKEL_SkaterTB2`, the skeleton made from figure 1's mesh. A second mesh imported with `skeleton = SKEL_SkaterTB2` shares
the skeleton asset and the clips play on it. Whether they play **correctly** depends on proportions: I could not read the per-bone translation retargeting
mode from Python (no accessor in 5.9 Python found). Meshy's clip files carry a translation key on every bone, so with the default mode the clip's bone lengths win
and a figure with other proportions gets figure 1's limb lengths on the clip. **Not verified.** Safe route for a differently proportioned mesh: import it with its own
skeleton and run the IK Rig and retargeter from `skater-retarget.md`, or set bone translation retargeting to Skeleton for every bone except `pelvis` (Animation
Scaled) in the skeleton editor before using the clips. If the second figure comes from the same Meshy rig call (same preview), bone positions are equal and nothing is needed.
Re-animating the same `motion_task_id` on the new rig (3 credits per clip) always gives matching clips and needs no retarget.

## Material (lead's decision applied)

`M_SkaterTB2` (`/JamAssets/Skaters/TB2/`): hue-key recolour of the one blue texture. `T_SkaterTB2_BaseColor` is sampled, then in gamma space
`mask = saturate((g.b - max(g.r, g.g)) * KeyGain - KeyBias)`, `lum = dot(g, (0.3,0.59,0.11)) / JerseyLum`, `BaseColor = lerp(T, TeamColour * saturate(lum), mask)`.
Emissive is `Emissive * BaseColor` plus the fresnel rim (the `M_Toon` node, its `TeamColour` input fed from the new BaseColor). Parameter names `TeamColour`, `Emissive`
unchanged. Defaults: **KeyGain 6, KeyBias 0.15, JerseyLum 0.08** (the lead's 0.25 was too dark: the figure reads brown under this lighting). Verified with deep blue
(0.02, 0.10, 0.55) and orange (0.95, 0.30, 0.03) side by side (still `tb2-teams-hue-key`): white stripe, face, gloves, skates untouched, helmet follows the team colour.
Two caveats: the key works in gamma space (a pure linear key gives a weak mask); with `Emissive` 0 the orange looks brown in this rig, and it reads orange with `Emissive` 0.5.
The look-tier owner decides the `Emissive` value for figures. The material does not use `NumberTens`/`NumberOnes` and `SetNumber` is a no-op (no number surface on the figure).
`M_Toon` was not touched.

## Blueprint and AnimBP

- `BP_SkaterVisual_TB2` (Actor, `/JamAssets/Skaters/`): components `Mesh` (SK_SkaterTB2, ABP, stencil 1, `OnlyTickPoseWhenRendered`, no collision) and `Stick`
  (engine cylinder, `MI_Toon_Stick`, stencil 1, attached to `stick_socket` in BeginPlay with a baked offset). Events: `SetTeam` -> `TeamColour` on a dynamic material instance;
  `UpdateMotion` -> `Speed` on the AnimBP and mesh yaw; `PlayShoot` -> `AM_SkaterTB2_Shoot`; `PlayCheck` -> `AM_SkaterTB2_Check`;
  `SetKnockedDown` -> `bKnockedDown` (direction ignored); `SetBoost` -> `Emissive` 1 or 0; `SetNumber`, `SetHasPuck` no-ops.
- `ABP_SkaterTB2`: state machine Idle (idle clip) -> Stride (Speed > 0.5, back at < 0.3), Down (`bKnockedDown`, Fall clip, no loop, holds the last frame) -> Idle when it clears; then `Slot DefaultSlot`.
  The brief's "shot" state is a montage in the slot instead of a state, because a one-shot with automatic return is simpler that way. `AM_SkaterTB2_Check` is **a placeholder** (the golf swing); there is no check clip.
- The dynamic material is created in BeginPlay: a `SetTeam` call before BeginPlay would be lost. Spawning through the usual SpawnActor path calls BeginPlay first.
- No `TurnRate` lean, no `SetHasPuck` marker, no get-up clip (clearing the knock-down blends back to idle in 0.2 s).

## What looked wrong

- **Numbers and marks in the texture** (blocking, see above). Also small emblem-like marks on the shins and helmet.
- The stick is a rigid cylinder on `hand_r` with no blade; in the shot clip (two-handed) it passes through the torso, and the wind-up arm swing clips the head.
- Feet slide: not measured. The stride clip loop seam: not measured (the pelvis ends where it starts after the strip, and the clip is 119 frames; I did not compare full poses).
- Lighting on figures is dim in `LR_L2_Night` at the ice edge; orange needs `Emissive`.
- Stills are 1280 px wide (the capture size of this tool), not 1920.

## Verdict on the promotion rule

The look and the animation are good enough for the P2 toy brief once the texture is clean, and the cost is small (one mesh, 8.8 MB, six figures cost nothing measurable
next to six stand-ins on this GPU). But the promotion rule asks for the bot test and frame gate on device A's build, and neither could run: the pawn and the loader do not exist
on this tree. Do not promote on this evidence. Figure 1 is test-only.

## Files under `Game/Plugins/JamAssets/Content/Skaters/` (bytes)

| File | Bytes |
|---|---|
| BP_SkaterVisual_TB2.uasset | 149,340 |
| TB2/ABP_SkaterTB2.uasset | 175,313 |
| TB2/AM_SkaterTB2_Check.uasset | 9,588 |
| TB2/AM_SkaterTB2_Shoot.uasset | 9,346 |
| TB2/AS_SkaterTB2_Fall.uasset | 284,992 |
| TB2/AS_SkaterTB2_Golf.uasset | 310,930 |
| TB2/AS_SkaterTB2_Idle.uasset | 380,110 |
| TB2/AS_SkaterTB2_Shoot.uasset | 316,118 |
| TB2/AS_SkaterTB2_Stride.uasset | 377,715 |
| TB2/M_SkaterTB2.uasset | 23,914 |
| TB2/SKEL_SkaterTB2.uasset | 14,053 |
| TB2/SK_SkaterTB2.uasset | 3,646,128 |
| TB2/T_SkaterTB2_BaseColor.uasset | 3,445,612 |

Nothing under `_Scratch/`; no map was saved (`Rink` and the sub-level were only edited in memory; the editor exits without saving maps).
Clip names: Idle = library 0 (`Idle`), Fall = library 187 (`Knock_Down`), Golf = library 323 (`golf_drive`), Stride and Shoot = text-to-motion.

## Ledger rows ready to paste

Columns as in lane B1's report (path | kind | version | source type | tool | url | (empty) | date | device | shipped | note). The mesh and texture rows are `shipped = n`.

```
| Game/Plugins/JamAssets/Content/Skaters/TB2/SK_SkaterTB2.uasset | skeletal mesh | 0.1 | ai-generated | Meshy | | | 2026-11-12 | B | n | test only: number on sleeve; text-to-3d preview 01a1cf20-d146-7254-bd64-91833134d9fe, refine 01a1cf21-abca-71e0-8b89-ded695bb2f20, rig 01a1cf22-143c-7425-8d02-8ce566754261 (lane B5) |
| Game/Plugins/JamAssets/Content/Skaters/TB2/T_SkaterTB2_BaseColor.uasset | texture | 0.1 | ai-generated | Meshy | | | 2026-11-12 | B | n | test only: number on sleeve; refine 01a1cf21-abca-71e0-8b89-ded695bb2f20 (lane B5) |
| Game/Plugins/JamAssets/Content/Skaters/TB2/SKEL_SkaterTB2.uasset | skeleton | 0.1 | ai-generated | Meshy | | | 2026-11-12 | B | n | made by the import of the rig 01a1cf22-143c-7425-8d02-8ce566754261; goes with the test-only mesh |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AS_SkaterTB2_Stride.uasset | animation | 0.1 | ai-generated | Meshy | | | 2026-11-12 | B | y | text-to-motion 01a1cf22-5993-7679-bf8f-c65be3dbae5e applied by animate 01a1cf22-c87c-77df-b384-91ea6830dcf4; pelvis drift stripped |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AS_SkaterTB2_Shoot.uasset | animation | 0.1 | ai-generated | Meshy | | | 2026-11-12 | B | y | text-to-motion 01a1cf22-8a91-7712-8132-b7f0bb2bd0b1 applied by animate 01a1cf22-f99e-70ed-a4b5-24c4d37b01f8; pelvis drift stripped |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AS_SkaterTB2_Idle.uasset | animation | 0.1 | sourced | Meshy library | preset 0 (Idle) | | 2026-11-12 | B | y | applied by animate 01a1cf23-2a80-71d4-a100-9897e4a1d58e; Meshy paid-plan licence |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AS_SkaterTB2_Fall.uasset | animation | 0.1 | sourced | Meshy library | preset 187 (Knock_Down) | | 2026-11-12 | B | y | applied by animate 01a1cf23-2a80-71d4-a100-9897e4a1d58e; Meshy paid-plan licence |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AS_SkaterTB2_Golf.uasset | animation | 0.1 | sourced | Meshy library | preset 323 (golf_drive) | | 2026-11-12 | B | y | applied by animate 01a1cf23-2a80-71d4-a100-9897e4a1d58e; used only by the check placeholder montage |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AM_SkaterTB2_Shoot.uasset | montage | 0.1 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | wraps AS_SkaterTB2_Shoot (lane B5) |
| Game/Plugins/JamAssets/Content/Skaters/TB2/AM_SkaterTB2_Check.uasset | montage | 0.1 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | placeholder, wraps AS_SkaterTB2_Golf |
| Game/Plugins/JamAssets/Content/Skaters/TB2/M_SkaterTB2.uasset | material | 0.1 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | hue-key team recolour of the Meshy texture; copy of M_Toon, KeyGain/KeyBias/JerseyLum |
| Game/Plugins/JamAssets/Content/Skaters/TB2/ABP_SkaterTB2.uasset | animation blueprint | 0.1 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | idle, stride, down; DefaultSlot |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB2.uasset | blueprint | 0.1 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | implements IJamSkaterVisual (lane B5) |
```

The `shipped` column and the exact column order are my reading of B1's report; check against `ASSETS.md` before pasting. The rows for the mesh, skeleton and texture should be removed again
when figure 2 replaces them.

## Re-runnable scripts, in order (for the next figure)

Run from `C:/Users/kazda/jam/wt/b-skater` with the editor on 8002. `py.sh <file.py>` wraps a Python file for `execute_python`. `mc.mjs` is the call helper for the `.mjs` files.
Things a second figure changes are in the right-hand column.

| # | Script | What it does | A second figure changes |
|---|---|---|---|
| 1 | `imp1.py` | import rig FBX as `SK_SkaterTB2`, materials and textures off, animations off | `W` folder, FBX path `RIG/rigged_character_fbx_url.fbx`, `destination_name`. Pass `ui.skeleton = SKEL_SkaterTB2` to reuse the skeleton |
| 2 | `r.py` | rename `SK_SkaterTB2_Skeleton` to `SKEL_SkaterTB2`, save | names |
| 3 | `imp2.py` | import stride, shot, library FBX as clips onto the skeleton (mesh import off); library file yields one asset per take | three FBX paths (`A_STRIDE`, `A_SHOT`, `A_LIB`), destination names |
| 4 | `r2.py` | rename library takes to Idle, Fall, Golf | take names |
| 5 | `strip.py` | strip pelvis X and Z drift on Stride, Shoot, Idle, Golf | clip list; the axes (X lateral, Z forward) are for this export |
| 6 | `mat.py` then `mat2.py` | import the 2K PNG as `T_SkaterTB2_BaseColor`; duplicate `M_Toon` into `M_SkaterTB2`; then (`mat2_run.py` = defaults line + `mat2.py`) build the hue-key | texture path `REFINE/texture0.base_color.png`; defaults `KG`, `KB`, `JL`; mat2 deletes nodes by name (`Custom_1`, `TextureSampleParameter2D_0`), so run it once on a fresh duplicate, or delete the unused nodes afterwards (`MaterialToolset.DeleteUnusedExpressions`) |
| 7 | `inline_steps.mjs mesh` | `SetMaterial`, three `AddSocket` calls | mesh path |
| 8 | `sock2.py` | socket offsets (`head_socket` local offset, `puck_socket` 60 cm ahead, scale 0.01) | head offset `(0.6745,-0.0158,0.0089)` was computed for this head bone; recompute with `sock.py` for another mesh |
| 9 | `mont.py`, `mont2.py` | montages for Shoot, Check (mont.py also made an unused Fall montage, deleted by mont2.py) | clip names |
| 10 | `a1.py`, `inline_steps.mjs abp`, `a2.py`, `trans.mjs`, `out.mjs`, `a4.py` | AnimBP: create (create via `BlueprintToolset.Create` first), skeleton, variables, state machine, states and transitions, rules, slot, Down no-loop | skeleton and clip names |
| 11 | `inline_steps.mjs bp`, `b1.py`, `b2.py`, `b3.py`, `dsl.mjs bp1.dsl`, `bp2.dsl`, `bp3.dsl`, `c.py` | the visual Blueprint: create, interface, `MID` variable, components, component defaults (mesh yaw -90, stick transform), the event graphs through the Graph DSL, compile and save | mesh asset; stick offset in `b3.py` was computed by `stick.py` for this hand bone |
| 12 | `sp.py`, `drive.py`, `look.py`, `cap.sh`, `startdrive.sh`, `perf.mjs` | test layout, PIE drive, camera, capture, six-figure timing (load `LR_L2_Night` as sub-level first: `lv2.py`) | none |

Caveats: the scripts were run interactively and mostly in this order, not as one chain; `inline_steps.mjs` was assembled afterwards from the commands I ran by hand and has not been run as one piece.
`BlueprintToolset.AddVariable` and the Graph DSL names (`/JamAssets/Skaters/TB2/ABP_SkaterTB2` as a cast target, `Speed`, `bKnockedDown`) depend on the names used here.
In Python the `unreal.Rotator` constructor takes (roll, pitch, yaw): a mistake there flipped the first test render.

## Not finished or not verified

- Step 5 (swap through the cartridge key, bot tests, state hashes): nothing to run it on.
- Frame cost at device A's gate, shipped tier: not measured; no LOD was made (20.7k triangles at one LOD).
- Foot sliding and the stride loop seam: not measured. Fall followed by get-up: only the fall was looked at; `SetKnockedDown(false)` returning to idle was not tested in PIE.
- The retarget mode question above; `SetNumber`, `SetHasPuck`, `TurnRate`, `DirX`/`DirZ` unused.
- The `M_SkaterTB2` emissive default (0): the figure is dim without `Emissive` in this rig.

## Decisions for the lead or operator

1. Replace figure 1 (texture numerals). Figure 2's files are in `meshy-work/REFINE2/` only as a refine, no rig; a new rig and the clips would cost 5 + 15 credits.
2. Who sets the `Emissive` value on skaters per look tier (0.5 reads right on `LR_L2_Night`).
3. Who wires `game.skater.visual` to `BP_SkaterVisual_TB2` in the game mode (device A), and when the bot tests will exist.
4. Whether `AM_SkaterTB2_Check` stays a placeholder, or a hit or shoulder clip gets generated (text-to-motion 10 credits plus 3).
5. The picks of the preview (operator may overrule from the filed thumbnails).

Commits on `lane/b-skater`: b937ff1 (import), then the commit that holds this report (hash in the final message).
