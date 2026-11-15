# Lane SM1 report: the goal's smoke over the camera on clay, and the neon rig at other lengths (device B), 2026-11-15

Tree `C:\Users\kazda\jam\wt\b-aud`, branch `lane/b-aud-h61`, port 8006. Machine clock 17:24 to 17:50. Commit `4f159179` on lane/b-aud-h61 (not pushed).
Build: the first run failed on memory (`C1076 internal heap limit`, during a busy machine), the second passed (14 s, incremental).
GPU: one editor (pid 33364), token "SM1 holds since 17:30", "free from SM1 at 17:41", left through `ExitEditor`, process gone, no dirty package.
Nothing written under device A's folders. Scripts and raw stills: scratch `d3\sm1\` (`dustmat.py`, `patch_rise.mjs`, `neoninsp.py`, `veil.py`, `shots\`); backups of both assets before the change: `d3\sm1\bk\` (sha1 M_Fx_GoalDust 5da31b7a..., NS_goal e76cec1e...).

## 1. Done / not done

| Item | Verdict |
|---|---|
| 1. The goal's smoke on clay: fade by distance to the camera, lower and thinner | **done, seen on GPU, committed** |
| 1. Night and day tiers still read | **not verified** (my two runs drew the clay barn: wrong cut names; section 3) |
| 2. The neon rig at 56 / 60 / 70 / 90 / 120 / 140 m | **answered from the level's actors and A's code** (headless reading, no GPU still) |
| Cook-only check | not done (VFX only, no Skaters/AnimBP/deletes; the lead cooks) |

## 2. Per item

### Item 1: the goal's dust (`M_Fx_GoalDust`, `NS_goal`; `M_Fx_GoalGlow` untouched)
- **What changed**:
  - `M_Fx_GoalDust`: one Custom node "SM1 near fade" between the old opacity and the Opacity pin: `A * 0.45 * smoothstep(300, 900, PixelDepth)`. The dust is gone inside 3 m of the camera, full beyond 9 m, and at 0.45 of its old opacity everywhere. The neon world-offset switch is untouched.
  - `NS_goal`, emitters Left and Right only: start height (InitializeParticle Position Offset z) 150 -> 60 cm. Throw origin (AddVelocity Origin Offset z) -120 -> -30 cm, so the dust goes out along the boards, flatter. Gravity stays -600. The Fountain (cream streaks) is not touched.
- **Proving numbers**: "veil" = mean |difference| (0 to 255, whole 1920x1080 frame) against the same timeline with nothing spawned:

  | camera | +6 | +20 | +40 | +70 |
  |---|---|---|---|---|
  | 4 m from the net, old | 56.1 | 44.0 | 30.6 | 23.2 |
  | 4 m from the net, new | **19.7** | **21.5** | 16.4 | 7.4 |
  | game height 15 m out, old | 9.9 | 22.0 | 26.1 | 25.8 |
  | game height 15 m out, new | 4.7 | **10.4** | **13.0** | 9.5 |

  Share of the frame changed by more than 40: camera at the net +6 39.9 % -> 15.9 %. The usual camera at +40: 19.9 % -> 13.3 %.
  A middle try (fade 2.5 to 6 m, 0.5 opacity, start 75 cm) gave 38.1 at the net at +6: not enough, not kept.
- **How**: PIE on `hockey.regular` (clay, 90 m). `NS_goal` was hand-spawned at the +x net (4100, 0, 0), orange, Strength 1, Variant 0, world slowed to 0.05, game camera pinned (LK1's `armfx`). This is a hand-spawn, not a scored goal.
- **What a person sees**: from the usual camera the dust is low puffs over the end boards, and the red lamp behind the net is clear in every frame (old: hidden behind a cream cloud at +20 and +40). With the camera at the net the grey wall over the whole frame at +6 is gone: the lamp, the fans and the net are seen. What is left close up are the Fountain's white streaks over the net (a different emitter, not touched).
- **Still**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-174500-B-SM1-goaldust-ab.png` (rows old / new at the usual camera, old / new at the net; columns +6, +20, +40).
- **Looked at with the Read tool**: the A/B sheet itself. At full size: old usual +20, old at-net +20, new at-net +20, new usual +20, new2 at-net +6, new2 usual +40.

## 3. Not verified
- **Night and day tiers with the new dust.** I ran `jam.Cut hockey.mode.fog-of-war` and `hockey.mode.pond` by short name. Both drew the clay barn (`...-goaldust-tiers.png`, read; its caption says so). LK1's runs passed the full cartridge path. The dust is the same material on every tier, so it is thinner on night and day too. On clay at 60 m it still reads as soft puffs.
- A real scored goal (all stills are hand-spawns). A's goal camera's own position at a goal (F2a frames 6492 to 6555) was imitated with a pinned camera at 4 m, not replayed.
- The Glow Ice goal (M_Fx_GoalGlow) after the change: untouched by design, not re-filmed.
- The neon rig at other lengths on a GPU: no still (the token went to TR1 at 17:41).
- A cook. `enable_game_sound` was set back to true in memory before the exit (the scripts set it false at each run).

## 4. For device A (A230, A-to-B-0043 "One ask") and the engine lane

### Item 2: `LR_L3_Neon` at other rink sizes
Read from the level (87 actors, live dump `d3\sm1\neon_actors.json`), `ULookTierSwitcher` and the cartridges:
- **No length parameter is needed.** A streams every rig with a uniform scale `RinkLengthM / 90` (`ULookTierSwitcher::RigScaleFor`, `RigRinkLengthM = 90`). The rig is authored for 90 x 45 (Glow Ice's rink), not 56 m. Local lights are re-scaled by A (intensity x s², radius x s), so the light pattern holds.
- **No crowd actors and no stands** in `LR_L3_Neon`: it is A's bare tier. The rig is 39 floor grid strips, the board frame line (28 pieces), 4 hall walls with 8 wall lines, 4 spots, sky light, height fog, post volume and `BP_LookNeon` (a marker that writes `MPC_Look.Neon`). The crowd-by-length owners exist only in the clay, night and day rigs.
- **The board frame line** (`Trim_L3_Neon_B*`, `MI_Neon_BoardLine`) is a closed loop inset 2.2 m from the boards at 90 m (straights at x ±42.8 m, y ±20.3 m). It scales as one piece, so it always meets itself. A's own outline at the boards (`NeonArena.cpp`) follows the real rink at any size.
  Inset from the boards at each size (ends / sides):
  - 56 x 28: 1.37 / 1.37 m
  - 60 x 30: 1.47 / 1.47 m
  - 70 x 35: 1.71 / 1.71 m
  - 90 x 45: 2.2 / 2.2 m
  - 120 x 60: 2.93 / 2.93 m
  - 140 x 70: 3.42 / 3.42 m
  - **Cage 60 x 28: 1.47 / 0.47 m**, uneven but inside the boards.
- **Hall**: end walls at ±65 m x s. At 140 the hall is 37 m tall and the lower wall line is 7 m up (at 56: 14.7 m and 2.8 m). The height fog is not scaled, so at 140 m the far end is a little foggier.
- **SAFE TODAY with no change: 56, 60 (60 x 30), 70, 90, 120, 140 m**, i.e. every rink with width = length / 2. That covers Arsenal 90, Marathon 140, Sudden Death 120, Power Play 60, Pond 60, Outnumbered 70, Shotgun and Bruisers 56.
- **Cage (60 x 28)**: usable. The only effect is that B's inset line sits 0.47 m from the long boards and 1.47 m from the ends. A real fix would scale the rig per axis (y by width / 45) in A's `SetTier`, or B would re-place the line. Not done.
- Not seen on a GPU at any size other than 90. A still at 140 and 60 is the next step if A wants proof before moving a mode.

## 5. Decisions for the operator
1. The dust's strength: 0.45 and a 3 to 9 m fade now. 0.5 and 2.5 to 6 m was tried and left a haze over the frame at the net. Lower is one number in `M_Fx_GoalDust`.
2. The Fountain's cream streaks over the net are now the biggest thing at the net camera at +6. Shortening or thinning them is a 5-minute patch, not done.

## 6. Next three steps
1. Re-film the new dust on night and Pond with the full cartridge paths (one 4-minute GPU run), and on Glow Ice to confirm the glow is unchanged.
2. A GPU still of Glow Ice's rig at 140 m and 60 x 28 to back item 2.
3. The lead cooks; push with the 18:30 package.

## 7. Ledger rows
| Game/Plugins/JamAssets/Content/VFX/M_Fx_GoalDust.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | a copy of M_Fx_SlideSoft (unlit translucent soft sprite) with world position offset (0, 0, -100000) x MPC_Look.Neon, so the goal's dust is drawn on every tier and moved out of view on the neon tier; renderer 0 of NS_goal's emitters Left and Right (lane LK1); since SM1 the opacity goes through a Custom node "SM1 near fade": old opacity x 0.45 x smoothstep(300, 900, PixelDepth), so the dust is gone within 3 m of the camera and full beyond 9 m; veil at a camera 4 m from the net at +6 frames 56.1 -> 19.7 (lane SM1) (22391 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.68 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-15 | B | y | event goal: cream-led chalk dust (cream 2.6/2.45/2.1 with a fifth of User.Colour as the accent) thrown from the net along the end boards, 114 to 126 frames of life, and a cream fountain of streaks; Variant 1 at 60 % of the count (lanes B3, B42, B43, B44, B45); emitters Left and Right carry two sprite renderers: 0 on M_Fx_GoalDust (every tier but neon) and 1 on M_Fx_GoalGlow (neon only, by MPC_Look.Neon) (lane LK1); since SM1 the dust starts at 60 cm instead of 150 cm (InitializeParticle Position Offset) and is thrown flatter (AddVelocity Origin Offset z -120 -> -30), about half the rise; Fountain unchanged (lane SM1) (708927 bytes) |

## Follow-up 23:22 to 23:30 (A-to-B-0045 points 1 and 2)
Merged dev-b (050e349f), built, GPU token 23:22 to 23:27 (editor pid 29720, left through ExitEditor, process gone).
- **Day dust (point 1)**: MPC_Look has no scalar that tells day from night (Clay is 0 on both), so off clay the dust now draws its particle colour x 1.8 (was x NightMul): pale cream on Pond instead of grey soot; on Fog of War (night) it reads as pale dust over the blue ice (only new seen, no night baseline). Clay untouched. Not snow-white: pale cream.
- **Orange on Glow Ice (point 2)**: M_Fx_GoalGlow normalises the team colour to its brightest channel, saturates it (pow 2.4), does not lift warm colours to white, and gives them up to 1.4x the emissive. Orange is now a saturated orange glow (was peach puffs); blue was seen after the first pass only.
- Still: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-232800-B-SM1-daydust-neonorange-ab.png` (read with the Read tool, also each pass sheet).
- Not verified: blue after the final pass, Outnumbered, a real scored goal, a cook.

| Game/Plugins/JamAssets/Content/VFX/M_Fx_GoalDust.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | a copy of M_Fx_SlideSoft (unlit translucent soft sprite) with world position offset (0, 0, -100000) x MPC_Look.Neon, so the goal's dust is moved out of view on the neon tier; renderer 0 of NS_goal's emitters Left and Right (lane LK1); opacity x 0.45 x smoothstep(300, 900, PixelDepth), gone within 3 m of the camera (lane SM1); since the SM1 follow-up the emissive off clay is the particle colour x 1.8 (was x NightMul, grey soot on the day look), clay unchanged (lane SM1) (22775 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_GoalGlow.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | unlit additive soft disc for Niagara sprites on NS_goal's emitters Left and Right (renderer 1), in view on the neon tier only by MPC_Look.Neon; takes the team colour back out of NS_goal's dust colour ((particle colour - (2.08, 1.96, 1.68)) / 0.4766) (lane LK1); since SM1 the colour is normalised to its brightest channel, raised to the power 2.4 (more saturated), lifted to white by 0.1 x (1 - warm) and given (1 + 0.4 x warm) of the emissive, warm = saturate(r - b), x 0.84 x a squared radial falloff x the particle alpha, so orange reads as light like blue (lane SM1) (12741 bytes) |
