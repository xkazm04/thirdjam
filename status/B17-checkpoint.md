# B17 checkpoint: the event effects audited in the real game, night tier (device B)

Lane B17, device B, 2026-11-12, machine clock 19:18 (start) to 19:52 (this checkpoint). Editor on port 8002 (tree `wt/b-vfx`), PIE on `/Game/Maps/Rink`, `hockey.regular`
(`l2_night`, default figure `toy_skeletal`), `jam.ScriptHuman 1`. No git, no build, nothing saved yet, nothing under `_Scratch/`. The 8000 editor shares the GPU: no frame numbers.

## Method (how the binder fires them, reproduced)

- Not a bot match: every `NS_<event>` was spawned from Python in the PIE world exactly as `UJamEffectsBinder::HandleEvent` does
  (`SpawnSystemAtLocation(Where, zero rotation)`, then `Strength`, `Colour`, `Variant`), at ice level (z 0; the binder's `Where` is z 0, or 4 cm for board / shot / goal).
- The game is frozen at the opening faceoff (`slomo 0.0001`, t = 2.19 s); each burst is stepped with `NiagaraComponent.advance_simulation` to fixed ages, so the 6 frames of a strip are exact.
- Four instances per strip: pool (x -11.25 m, under `Spot_L2_Night_2`) at Strength 1.0 and 0.3 in blue (0.05, 0.3, 1.0); dark band (x 0) at 1.0 and 0.3 in orange (1.0, 0.35, 0.05).
- Views: game camera 25 m away, high behind (pitch -17.7, FOV 70, both pairs in frame, toy skaters 1.8 m in frame for scale); 8 m over the pool pair; 8 m over the dark pair.
- Ages: contact events 0.02 to 0.32 s; faceoff / goal / pickup / effect / zone / directors_call 0.05 to 1.2 s; win 0.2 to 2.6 s. 26 strips x 3 views, in the scratch folder `b17/shots/n_*`.

## Verdicts, night tier, before (all use `M_Fx_Sprite`: additive, unlit, soft disc, Glow 15)

| Effect | 25 m: reads? | Size vs a 1.8 m skater | Team colour | Film-set? | Notes |
|---|---|---|---|---|---|
| `check` | **no** | 8 puffs of 15-45 cm: smaller than the puck halo | lost (50 % white reads white) | no | the loudest contact is the quietest effect: no ring, no star, 0.25 s of white dots |
| `knockdown` | barely (dark) / no (pool) | low ring of chips, 1 m | no | no | radial, no slide |
| `board` | barely | 1.2-1.6 m orange flash half-buried in the ice + dust | flash yes | **yes**: the flash is a lamp blink, not a hit | no chips off the board, nothing on the board top |
| `poke` clean / miss | no / no | 4-9 cm sparks; puffs | no | no | invisible at 25 m |
| `lift` (both) | no | chips under 10 cm | no | no | no `User.Variant`: clean and miss look the same |
| `shot` | barely (dark) / no (pool) | 12-30 chips | no | no | |
| `shot_blocked` | no | 18 chips | weak | no | |
| `foul` 0 / 1 | yes (dark) / barely (pool, pink wash) | 1.4 m red flash + upright sprite ring | 35 % | **yes**: the red flash is B3b's "red cut light" | upright ring, not on the ice |
| `faceoff` drop / won / lost | barely / barely / no | 24-sprite ring at the dot | won only | no | |
| `pass` | no (by design: tiny) | 3-6 cm | no | no | fine as is |
| `goal` | **yes** | 160 sparks | yes | no | the only event that reads |
| `win` | barely (falls from 9 m, spread thin) | confetti | trim colours | confetti is fine | |
| `pickup_spawn` / `pickup_take` | barely / **yes** | column / 60 cm ring | yes | no | |
| `effect_start` / `effect_end` | yes (dark) barely (pool) / barely | ring + 1.6 m flash | yes | flash: mild | |
| `zone_enter` / `zone_exit` | no / no (subtle by design) | 40-90 cm glints | yes | no | |
| `directors_call` | **yes** | 25 m sweeping ring of 160 upright 55-70 cm puffs | ignores Colour | warm lamp-white (1, 0.85, 0.6): a stage-light tone; shape is a fine "rules changed" sweep | |
| `NS_blade_spray` | **no** (8 m: barely in a pool) | 3-6 cm sprites | n/a | no | a stop does not throw a visible fan |

**Systemic finding.** Every effect is additive white or near-white. Additive can only lighten: on the lit pools (luma about 220) white chips vanish, and the
broadcast ice (luma 210-235, no dark band) and the cream clay ice will be worse. Fix: one translucent (not additive), unlit, tier-aware particle master that reads
`MPC_Look.Clay` and draws a thin dark rim, instead of three systems per effect.

## Plan for the rest of the lane (contact first)

1. `M_Fx_Tier` (translucent, unlit, `MPC_Look.Clay`: ink rim on night; darker bluer chips with a dark rim on broadcast; warm shaded crumbs on clay) with instances
   `MI_Fx_Chip`, `MI_Fx_Ring` (flat ring lying on the ice), `MI_Fx_Star` (clack star with a dark halo), `MI_Fx_Puff`, `MI_Fx_Spark` (keeps hue on broadcast).
2. Rebuild `check` (chips at skate level, flat team shock ring, white star at 1.4 m), `knockdown` (spray, puff, flat scuff), `board` (chips plus a shake line at the
   1.2 m board top), `poke` / `lift` (clack star or puff by Variant; `lift` gains `User.Variant`), `shot` (stretched chip streaks), `shot_blocked`, `foul` (flat red ring
   on the ice, a second ring for Variant 1; the red flash goes), `NS_blade_spray` (bigger fan).
3. Swap the rest to the tier material; `directors_call` lies flat on the ice in a neutral ice white.
4. Before/after on the clay tier and the broadcast rig, then a 90 s bot match with live captures.
