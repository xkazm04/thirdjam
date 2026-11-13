# B21 report: the P4 lab, second pass: the ice and the skater (device B)

Lane B21, device B, machine clock 2026-11-13 00:54 to 02:50. Tree `wt/b-vfx`, editor on port 8002 (pids 18328, 12412, 35720: restarted twice, see "trouble"), left at 02:46 through `ExitEditor` with **no dirty package** (checked with PIE stopped; file times below). No git, no build. `/Game/Maps/Rink` never saved (18,966 bytes, mtime 2026-11-12 12:40); when the editor marked it dirty after PIE, I loaded the rig with `DISCARD`. `_Scratch/` does not exist. **Not touched:** `NS_blade_spray`, every `BP_SkaterVisual_*` other than my new `BP_SkaterVisual_P4Lab`, every `ABP_*` other than my new `ABP_SkaterGen_P4E`, `M_SkaterTB2`, `M_Ice`, all of A's. The 8000 editor (SK7, another tree) shared the GPU all night: **no frame number was measured on this machine; every cost below is an estimate.** Checkpoint: `status/B21-checkpoint.md`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b21` (`lab.py` every PIE-side setting incl. `b21()`, `mk_mats.py` masters, `rig21.py`/`rig21b.py` rig additions, `mk_near.py` near-cut textures, `spec_spray.mjs`+`nb.mjs` spray, `i3.py`/`tex.py`/`bs.py`/`abp.py`/`gen.mjs`/`b3.py` figure import, `mirror21.py` match mirror, `notes.md`).

**Method:** the real game in floating PIE on `Rink`, QA1's scratch cartridge (`hockey.regular`, `p4_broadcast`, `generated_b2`) by `jam.Cut`, `jam.ScriptHuman 1`; frozen at the opening faceoff (slomo 0.0001, t 2.17-2.20 s) for comparable frames, run at slomo 1 for match frames. Cameras: concept framing (3 m behind, 1.2 m up, pitch -6, FOV 70; for the hero re-aimed to match the review page's estimate: `(-7.85, 1.3, 1.2) m, yaw -4`, skater left of centre, red line across, far bank above), the game camera, a close three-quarter, 8 m and 24 m.

## The morning sheet (`devlog/shots/`)

| File | What |
|---|---|
| `20261113-024341-B-b21-hero-concept-framing.jpg` | blue P4E (loose jersey, new stride) from behind, push-leg extended, powder off the pushing skate, rink ahead, far bank above, lamp streaks in the ice |
| `20261113-024342-B-b21-hero-stop.jpg` | `Stop_Hockey_Left` with `NS_P4Lab_spray` at the blades |
| `20261113-024514-B-b21-hero-game-camera.jpg` | game camera, running bot match (t 10.8 s), both teams |
| `20261113-024514-B-b21-before-after.jpg` | same game camera at the faceoff: yesterday's shipped broadcast (B14) / first lab pass (B20) / now |

All four are single frames from the game; nothing composited from any reference (the before-after is three game frames side by side).

Supporting: `-b21-ck-concept/-game/-tq/-oconcept` (checkpoint, 01:19), `-b21-spray` (close low powder), `-b21-grounding`, `-b21-skater-reflection`, `-b21-figure-close`, `-b21-readability` (8 m, 24 m, orange concept framing), `-b21-match-mirror`, `-b21-export2-faults`.

## Part A: the ice

### 1. Something to reflect (`Rigs/LR_P4_Broadcast`, saved; B20's version backed up in scratch)
- **Stands:** B14's 72 stand boxes get `MI_P4_Stands` (new unlit `M_P4_Stands`): near-black silhouettes, a faint cool fill falling off with height (Fill 0.15), a crowd of dim seat blobs in blue/orange/grey (Crowd 0.35, procedural per 55 cm cell, 25 % empty), 4 % static warm/cool light points (intensity 4), a dim warm concourse band (0.2): the saved `MI_P4_Stands` values, used in every still after the checkpoint. One draw per box, no textures.
- **Lamp banks:** 10 side banks along both long sides at the parapet top (12 m long, y +-41 m, z 11.9 m, 12 heads each) and a 2-row 34-head bank at the -x end, drawn procedurally by `M_P4_LampBank` (unlit, UV grid of discs, Intensity 400); B20's 48-head far bank at +x kept. No new shadow-casting light.
- **Glint lights (new finding):** Lumen software reflections only reflect what is on screen (or in its scene cards), so from the **game camera** (looking down) the ice showed **no** lamp streaks even when made a mirror. 14 **specular-only** point lights at the banks (diffuse 0, no shadow, 4000 cd, source radius 6 cm, **source length 500 cm, vertical**) put long vertical streaks into the ice from any camera. They are what gives the game camera its streaks (`hero-game-camera`, `match-mirror`).
- **Exposure fixed** (EV100 4.2). **Readability:** the brighter surround does not hurt the figures: both jerseys stand out against the darker crowd; the ice under the game camera is a little brighter overall (wet patches darker), the puck keeps its halo (it reads more by its yellow halo than by its body). The glints add bright spots that can sit behind a skater at the game camera (gm frames): a busy background behind the player, not a loss of the figure. Orange at 24 m reads clearly orange on P4E (`readability`), not red-brown.

### 2. Near-camera cuts (`M_P4_Ice` rebuilt, new `T_P4_IceNear_M`/`_N`)
Procedural 1024 tile over 1.4 m (hairline cuts 1-2 texels, wide snow-filled blade cuts, chip flecks), sampled twice at two rotations, fading in within 6 m of the camera (full at 2.7 m), white x1.4, roughness 0.45, own groove normal (NearBump 2). Final: NearAmt 0.55, NearHair 0.45, the old scratch tiles reduced (Fine 0.08, Med 0.3) so the near cuts lead and the far ice is calmer. **Could not judge:** shimmer and fade-band popping in motion (stills only; the fade is smooth by construction, the 1.4 m tile at 1-2 texel lines will alias in motion without TAA settling).

### 3. Body colour, milk, wet patches
Body (0.22, 0.30, 0.43), a low-frequency milky layer (Milk 0.35, colour (0.5, 0.6, 0.72)), wet patches darker (x0.65) and glossy (roughness 0.03), the deep scratch layer kept. Against the references, in words: t2's pale blue-grey with milky depth and darker wet patches is matched in kind; the concept's ice is still **paler overall** where it reflects a lit arena (t1).

### 4. Powder spray `VFX/NS_P4Lab_spray` (new) with `M_P4Lab_Spray` / `MI_P4Lab_SprayMist`
Four emitters: **Powder** 450-1200 grains, 0.6-1.8 cm, velocity-stretched (1 + |v| x 0.009), flat fan (shape 1.6 x 0.35 x 0.12) thrown to local +Y, gravity, drag 2.5; **Dust** 300-600 motes 0.4-1 cm; **Mist** 6-14 soft puffs 22-40 cm, alpha 0.45; **Shavings** 20-50 flat flecks that slide and lie 1.4-2.2 s. Unlit translucent, coloured to the lamps (1.0, 0.97, 0.92) x 4, depth-faded. `User.Strength` scales it. **Trigger for the stills:** PIE refuses Python spawning of actors but `NiagaraFunctionLibrary.spawn_system_at_location` works in PIE: spawned at the skate bone's position with a yaw pointing the fan, stepped with `advance_simulation` to 0.16-0.2 s. In a shipped tier it would replace `NS_blade_spray` on this tier through the figure's existing spray component (one asset path per tier). It reads as white powder, stays under the knee near the blade; the fan is longer (2-3 m at strength 1) than real snow; a few grains are large enough to read as flakes at 1 m.

**Checkpoint stills** at 01:19 (above).

## Part B: the skater

### 5. Import P4E: done, then re-done on `export2` (loose jersey, 75 takes)
- **First (01:20, `export/P4E`):** `SK_SkaterGen_P4E` + `SKEL_SkaterGen_P4E` + 72 takes; sockets (stick/head/puck, SKI2's values); `BS_SkaterGen_P4E_Skate/_Aim`, `ABP_SkaterGen_P4E` (copy of `ABP_SkaterTB2`, all players re-pointed, compiles), `BP_SkaterVisual_P4Lab` (duplicate of `BP_SkaterVisual_TB2`, SKI2's `gen.mjs` graph, mesh and anim class re-pointed, compiles). About 25 minutes.
- **`export2` (02:31, READY.txt 02:28):** imported onto the **same skeleton** (`skeleton` set in the FBX options): 75 takes (`Push_Start`, `Stop_Hockey_Left/Right` new), mesh 36.4k tris. A first in-place re-import attempt (consolidate into the old names) left the AnimBP and blend spaces with "illegal reference to private object" on save; I deleted the P4E mesh/takes/BS/ABP/BP, restarted the editor, re-imported cleanly and re-ran SKI2's scripts: everything compiles and saved, no warnings. Total for export2: about 20 minutes plus the restarts.
- **Textures:** `T_SkaterGen_P4E_BaseColor` (sRGB), `_Normal` (DirectX, normal map), `_Mask` (linear, masks), `_AO` (linear grayscale), 2048 source, **MaxTextureSize 1024 for all stills** (I did not switch to 2048 for the close stills: at 1-3 m 1024 was sharp enough and time went to the import). Trap: the base colour first built with "no miplevels" (grey figure) until its LOD group was changed to World; all four are on World groups now.
- **Shown in a match by SKI3's method:** the six game visuals (`BP_SkaterVisual_TB2_P4`) hidden, one `BP_SkaterVisual_P4Lab` summoned per pawn, attached, team and number set, the game's calls mirrored each tick (`mirror21.py`). In the match: UpdateMotion, aim, Pass, Lift, Shot_Wrist seen; no errors. Knockdowns not drawn in the match (bots did not check); falls were posed as single takes for the fault check. Hero poses: the concept-framing hero is the figure's own AnimBP (blade on the ice, Stride blend) advanced with `custom_time_dilation` while the world is frozen; the stop hero is `Stop_Hockey_Left` as a single take.

### 6. The figure's material (`Look/P4Lab/M_P4_FigureE`, `MI_P4_FigureE`)
Team colour by **mask R x (1 - G)** (G never recolours), shaded by the texture's blue; cloth roughness 0.82 and the weave normal x1.6 by mask A; gloves (mask B 0.3) leather roughness 0.45 with spec 0.55; helmet (B 0.85) roughness 0.16, spec 0.7 (the visor shows the highlight and the face behind it); skates dark with white holders (texture); AO multiplied (0.8); a subtle cool rim (0.15, pow 4) plus a cloth sheen; TeamGamma 0.8. **Orange reads orange** at 8 m and 24 m (`readability`, `hero-game-camera`), not red-brown. Stick: new dark `M_P4_Stick` (no tape: unknown UVs).

### 7. Grounding and reflection
- **Shadow** (`grounding`): all 14 lamps shadowed = the concept's soft multi-shadow, the best grounding; estimated +1-1.5 ms over 2 shadowed lamps on a 4090, 3-5 ms mid-range (B20/B1c basis). 2 shadowed lamps = the figure floats. A **blob decal** (`M_P4_BlobShadow`, one deferred decal per skater) costs ~0.05 ms and grounds the skates but reads as a smudge, not a shadow. Contact shadows need a shadowed light (no effect without). Recommendation: 4-6 shadowed lamps around the play + the blob.
- **Reflection:** a glossy patch under the skater (`PatchAmt`, roughness 0.02, 2.5 m) made **no visible difference**, and even a fully clean glossy ice does not show his reflection from behind at 1.2 m (`skater-reflection`): geometrically it falls under his own skates. Opponents ahead do reflect (visible in the hero). **The concept's under-skater reflection is not reachable from this framing without faking it** (it would need a planar reflection or a mirrored mesh: not worth it).

### 8. `export2` heroes: done (above). Known faults, observed (`export2-faults`): `Knockdown_Back` shows a pale patch over the seat (the skirt's back panel); the stride/glide chest stays upright (no forward lean); `Glide` shows the pale underside of the hem from behind-low; `Stop_Hockey_Right` and `Board_Hit` showed nothing wrong at the frames I sampled (did not step through the 3 bad frames).

## The hero frame against the review estimate (`est-p4-concept-framing.png`), point by point
| Estimate promises | Real frame |
|---|---|
| 7.5-head player, half visor | **matches** (P4E proportions, visor; from behind the visor is a thin edge) |
| loose blue jersey, white yoke, sleeve stripes, striped socks | **matches in kind**: boxy jersey with hanging hem, yoke, stripes, socks; folds are procedural, no drape |
| forward-leaning stride, stick blade on the ice | **half**: the push leg extends back and the blade is on the ice by the puck, but the chest is upright (the takes) |
| pale grey-blue ice, crisp white near scratches | **matches** at the foreground; the far ice is darker and calmer than the estimate |
| lamp reflections as vertical streaks at both sides and the far end | **matches**: far-bank streaks and side glints; fewer and shorter than the estimate's |
| fan of white powder off the pushing skate | **matches in kind**; the fan is low and wide, a bit coarse at the edges |
| soft shadow | **matches** (14 shadowed lamps) |
| faint reflection of the skater | **falls short**: not reachable from behind (above); opponents reflect |

## Judgement (frame costs: estimates only, nothing measured here)

| Concept | Ice | Skater |
|---|---|---|
| t1 (lit arena, crowd) | **~55 %** (B20 45 %): the crowd and banks give the reflections gradients and streaks; remaining: a lit, colourful crowd and arena that light the ice (assets: stands with real seats/people, signage), paler ice | **~65 %** (B20 50 %): proportions, kit, gloves, visor, orange right; remaining: cloth drape, forward lean (takes/technique), skater reflection |
| t2 (dark arena, spot rigs) | **~70-75 %** (B20 60-65 %): powder spray, crisp near cuts, streaks from both cameras; remaining: brighter powder in motion, motion-tested cuts | **~65-70 %**: same as above; the dark arena forgives the stiff chest less |
| t3 (neon, pickups) | **~50 %**: neon dressing and coloured reflections are missing (assets) | **~60 %** |

**Two more days could close:** a shipped broadcast tier of this ice (masters, rig, glints, spray swap, per-tier material selection by A), P4E as the broadcast figure (Blueprint done; A's `ResolveVisualClass` entry), tuning of spray and near cuts in motion, a forward-lean additive on the stride. **Could not close:** real cloth, a lit crowd as rich as t1, the under-skater reflection, t3's neon set.
**Frame cost estimate (4090 / mid-range):** ice material ~0.15 / 0.3 ms (5 extra samples); Lumen SW reflections 0.5-0.9 / 1.5-2.5 ms; 14 glints (unshadowed, spec only) ~0.2 / 0.5 ms; stands and banks ~0.1 ms; spray at ~1000 CPU particles per burst, 2-3 bursts live: ~0.2-0.4 ms CPU; 14 shadowed lamps +1-1.5 / 3-5 ms (4-6 lamps halves it); P4E 36k tris x 6 skinned: small. Total roughly **+2-3 ms on a 4090, +6-9 ms mid-range** over the night tier. Not measured on this machine.

## What a shipped tier needs
- **From device A:** `RinkActor` picks `MI_P4_Ice` (and boards/kick/glass/trim instances) by tier; `ResolveVisualClass` maps `p4_broadcast` to `BP_SkaterVisual_P4Lab` (or a renamed copy); the blade-spray binder uses `NS_P4Lab_spray` on this tier; the rig's side banks and glints scale with large rinks. No project settings.
- **From device B:** rename/finalise the lab assets, a forward-lean additive or re-take of the stride (figure lane), motion tests of near cuts and spray, 4-6 shadowed lamps + blob decal decision, the stick tape.

## Trouble worth knowing
- Lumen SW reflections miss off-screen emitters: use spec-only capsule lights for streaks.
- A material sampling textures inside a Custom node via TextureObjectParameter returned wrong data on the figure; TextureSampleParameter2D nodes fixed it (with the "no miplevels" texture).
- Re-import by consolidating over old assets left illegal private references; delete and import clean on the same skeleton instead.
- `ExitEditor` with `bDiscardUnsavedChanges` then left port 8002 in TIME_WAIT for ~1 min; one restart hit the `Wait for ZenServer?` box (rule 5): I stopped my own editor (pid 36136) and restarted.

## Assets created or changed (bytes, `Game/Plugins/JamAssets/Content/`)
`Rigs/LR_P4_Broadcast.umap` 843,777 -> 901,514; `Look/P4Lab/M_P4_Ice` 41,354 -> 55,562; `MI_P4_Ice` 16,579 -> 21,062; new: `M_P4_Stands` 19,935, `MI_P4_Stands` 10,080, `M_P4_LampBank` 16,271, `MI_P4_LampBank_Side` 8,337, `MI_P4_LampBank_End` 8,802, `T_P4_IceNear_M` 1,215,235, `T_P4_IceNear_N` 1,459,269, `M_P4_FigureE` 36,238, `MI_P4_FigureE` 8,190, `M_P4_Stick` 9,247, `M_P4_BlobShadow` 10,587; `VFX/NS_P4Lab_spray` 967,848, `VFX/M_P4Lab_Spray` 16,683, `VFX/MI_P4Lab_SprayMist` 8,383; `Skaters/SK_SkaterGen_P4E` 5,005,040, `SKEL_SkaterGen_P4E` 14,022, `T_SkaterGen_P4E_BaseColor` 3,666,876, `_Normal` 4,609,135, `_Mask` 100,367, `_AO` 2,524,672, `BS_SkaterGen_P4E_Skate` 27,620, `BS_SkaterGen_P4E_Aim` 51,252, `ABP_SkaterGen_P4E` 644,922, `BP_SkaterVisual_P4Lab` 1,925,709, 75 x `AS_SkaterGen_P4E_*` 137,452-352,589 (14,306,126 total). B20's other `P4Lab` assets unchanged.

Ledger rows (`shipped` n; the rig, `M_P4_Ice`, `MI_P4_Ice` rows **replace** B20's):
```
| Game/Plugins/JamAssets/Content/Rigs/LR_P4_Broadcast.umap | other | 0.86 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | P4 lab arena: B20's set + stands on M_P4_Stands (crowd, fill, light points), side and -x end emissive lamp banks (M_P4_LampBank), 14 spec-only vertical capsule glint lights at the banks (lanes B14, B20, B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Ice.uasset | material | 0.05 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | lab ice master: scratch tiles, deep layer, wet patches (darker), milky layer, near-camera cut layer fading in within 6 m, optional glossy patch; M_Ice's line code (lanes B20, B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Ice.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Ice, B21 final values (lanes B20, B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Stands.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | unlit stands: dark silhouettes, cool height fill, procedural crowd blobs, static light points, concourse band (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_Stands.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_Stands (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_LampBank.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | unlit lamp bank: procedural grid of lamp heads on one face (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_LampBank_Side.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_LampBank, 12 heads, side banks (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_LampBank_End.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_LampBank, 34 heads, end bank (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/T_P4_IceNear_M.uasset | texture | 1.16 | original | Python script (numpy, scratchpad/b21/mk_near.py) |  |  | 2026-11-13 | B | n | procedural tileable near-camera ice cuts 1024: hairlines, snow-filled blade cuts, flecks (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/T_P4_IceNear_N.uasset | texture | 1.39 | original | Python script (numpy, scratchpad/b21/mk_near.py) |  |  | 2026-11-13 | B | n | procedural groove normal for T_P4_IceNear_M (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_FigureE.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | P4E figure master: team colour by mask R x (1-G), cloth/glove/helmet roughness by mask, weave normal, AO, rim and sheen (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/MI_P4_FigureE.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4_FigureE, TeamGamma 0.8 (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_Stick.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | dark composite stick (lane B21) |
| Game/Plugins/JamAssets/Content/Look/P4Lab/M_P4_BlobShadow.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | deferred decal soft blob shadow (lane B21) |
| Game/Plugins/JamAssets/Content/VFX/NS_P4Lab_spray.uasset | vfx | 0.92 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-13 | B | n | P4 lab powder spray: stretched white grains, dust, mist puffs, lying shavings; User.Strength (lane B21) |
| Game/Plugins/JamAssets/Content/VFX/M_P4Lab_Spray.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | unlit translucent lamp-tinted powder sprite (lane B21) |
| Game/Plugins/JamAssets/Content/VFX/MI_P4Lab_SprayMist.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | instance of M_P4Lab_Spray, soft mist (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterGen_P4E.uasset | mesh | 4.77 | ai-generated | Codex concept sheet + Hyper3D Rodin + Meshy rig; jersey garment by skc/p4/tools/build_jersey.py |  |  | 2026-11-13 | B | n | skc/p4/P4E/export2 loopcut FBX, loose jersey, 36.4k tris; Rodin a297c99b-3976-4baa-a8ee-17a77c59d0a2, Meshy rig 01a1d1f6-3b17-75d1-9dcd-afb5d0df719f; sockets stick/head/puck as SK_SkaterTB2 (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterGen_P4E.uasset | other | 0.01 | ai-generated | Meshy rig |  |  | 2026-11-13 | B | n | own skeleton of P4E, 32 bones; Meshy rig 01a1d1f6-3b17-75d1-9dcd-afb5d0df719f (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P4E_BaseColor.uasset | texture | 3.50 | ai-generated | Hyper3D Rodin texture repainted by skc/p4/tools/tex_p4.py |  |  | 2026-11-13 | B | n | export2 base colour, 2048 source, MaxTextureSize 1024; Rodin a297c99b-3976-4baa-a8ee-17a77c59d0a2 (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P4E_Normal.uasset | texture | 4.40 | original | skc/p4/tools/tex_p4.py |  |  | 2026-11-13 | B | n | weave/leather normal, DirectX, 2048 source, 1024 max (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P4E_Mask.uasset | texture | 0.10 | original | skc/p4/tools/tex_p4.py |  |  | 2026-11-13 | B | n | mask R team cloth G trim B gloss A cloth, linear, 1024 max (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/T_SkaterGen_P4E_AO.uasset | texture | 2.41 | original | Blender Cycles bake (skc/p4/tools/bake_ao.py) |  |  | 2026-11-13 | B | n | ambient occlusion, linear, 1024 max (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterGen_P4E_<Take>.uasset | anim | 0.13-0.34 | original | Blender 4.2 script, retargeted |  |  | 2026-11-13 | B | n | skf/export7 take retargeted onto P4E (skc/p4/P4E/export2 loopcut), 75 takes (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P4E_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | Speed x TurnRate, layout of BS_SkaterTB2_Skate (lane B21 with SKI2's scripts) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterGen_P4E_Aim.uasset | anim | 0.05 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | 24-hold aim grid (lane B21 with SKI2's scripts) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterGen_P4E.uasset | other | 0.62 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | copy of ABP_SkaterTB2 on SKEL_SkaterGen_P4E (lane B21) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_P4Lab.uasset | other | 1.84 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | copy of BP_SkaterVisual_TB2 with SKI2's gen.mjs graph on SK_SkaterGen_P4E / ABP_SkaterGen_P4E; not selected by the game yet (lane B21) |
```

## What I could not verify
- Any frame cost (shared GPU; estimates only). Motion: near-cut shimmer, spray in motion, glints while the camera moves, Lumen lag.
- P4E knockdowns and checks in a match (bots did not check); the game selecting `BP_SkaterVisual_P4Lab` itself (needs A); the 3 bad `Board_Hit` frames.
- 2048 textures in the close stills (all stills at 1024).
- The rig on large rinks (banks and glints placed for 90 x 45).

## What the lead must decide
1. Broadcast (P4) worth two more days? My read: the ice is close to t2 now (~70 %) and the skater is ~65 %, at roughly +2-3 ms on a 4090 (estimate). The remaining distance is mostly assets (lit crowd, cloth) and takes (lean), not technique.
2. If yes: A's per-tier ice/figure/spray selection; 4-6 shadowed lamps + blob decal (vs 14); drop the skater-reflection goal.
3. Keep or delete the lab assets (all `shipped` n) and paste the rows.
