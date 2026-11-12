# B17 report: effects that make contact read, checked on every look tier (device B)

Lane B17, device B, 2026-11-12. Machine clock 19:18 to 20:52. Checkpoint: `status/B17-checkpoint.md` (19:52).

- Editor on port 8002, tree `wt/b-vfx` (built on A's `main` of 18:13), pid 36424. It left through `ExitEditor` (pass) with no dirty package and PIE stopped; the process is gone.
- No git, no build. `/Game/Maps/Rink` was never saved or reloaded (mtime still 12:40, 18,966 bytes); nothing under `/Game/` was written. Nothing is under `_Scratch/`.
- Not touched: `Skaters/`, `Props/`, `NS_puck_trail` (byte-identical), the rigs, `M_Toon`, `M_Ice`, `M_Trim`, the post-process materials, `M_Fx_Sprite`, `M_Fx_Ribbon`.
- Cvars set back: `jam.Cut hockey.regular`, `jam.ScriptHuman 0`.
- The 8000 editor shared the GPU: **no frame numbers**. Budgets below are particle counts, sprite sizes and lifetimes.

## Method

- **Spawn exactly as the binder does** (`UJamEffectsBinder::HandleEvent`): `SpawnSystemAtLocation(Where, zero rotation)`, then `Strength`, `Colour`, `Variant`, from Python in PIE on `/Game/Maps/Rink`. `Where` is z 0 (z 4 cm for board, shot and goal), as `HokejGameMode` sends it.
- The game is frozen at the opening faceoff (`slomo 0.0001`), and each burst is stepped with `NiagaraComponent.advance_simulation` to exact ages, so a strip's 6 frames are true ages (0.02 to 0.32 s for contact; 0.05 to 1.2 s for the long ones).
- Four instances per strip: pool (x -11.25 m, under a spot) at Strength 1.0 and 0.3 in blue; dark band (x 0) at 1.0 and 0.3 in orange. Toy figures (1.8 m) stand in frame.
- Views: the game camera (25 m away, high behind, pitch -17.7, FOV 70), and 8 m over each pair.
- Tiers: night through the game (`hockey.regular`); clay through a scratch cartridge copy (`game.look.tier p3_clay`, `jam.Cut <path>`, `MPC_Look.Clay` read 1); broadcast by B14's rig swap in PIE (Clay read -1). The default figure (`toy_skeletal`) everywhere.
- Proof in play: a bot match (`jam.ScriptHuman 1`) of 92 s game time. A Python callback on `UJamEventBus.OnJamEvent` set the time dilation to 0.02 the moment a contact event fired; a poller then captured the game camera plus 6 frames from 25 m behind the event and released the dilation.

## 1. Audit table, before (night tier; contact events also on clay and broadcast)

Every effect used `M_Fx_Sprite`: additive, unlit, a soft white disc times 15. Additive can only lighten, so white bursts vanish on the lit pools (luma about 220), and they are invisible on cream clay ice and on white broadcast ice (strip `b17-before-tiers`).

| Effect | Night, 25 m | Size vs a 1.8 m skater | Team colour | Film-set | Clay | Broadcast |
|---|---|---|---|---|---|---|
| `check` | **no** | 8 puffs, 15-45 cm, smaller than the puck halo | lost (50 % white) | no | no | no |
| `knockdown` | barely (dark) / no (pool) | 1 m low ring of chips | no | no | - | - |
| `board` | barely | 1.2-1.6 m orange flash half in the ice, plus dust | flash only | **yes**: a lamp blink, not a hit | no | no |
| `poke` clean / miss | no / no | 4-9 cm sparks / puffs | no | no | no | no |
| `lift` (clean = miss) | no | chips under 10 cm | no | no; no `User.Variant` | - | - |
| `shot` | barely / no | 12-30 chips | no | no | - | - |
| `shot_blocked` | no | 18 chips | weak | no | - | - |
| `foul` 0 / 1 | yes (dark) / barely (pool, pink wash) | 1.4 m red flash + upright ring | 35 % | **yes**: B3b's "red cut light" flash | - | - |
| `faceoff` drop / won / lost | barely / barely / no | 24-sprite ring | won only | no | - | - |
| `pass` | no (tiny by design) | 3-6 cm | no | no | - | - |
| `goal` | **yes** | 160 sparks | yes | no | - | - |
| `win` | barely | confetti from 9 m | trim colours | no | - | - |
| `pickup_spawn` / `pickup_take` | barely / **yes** | column / 60 cm ring | yes | no | - | - |
| `effect_start` / `effect_end` | yes (dark), barely (pool) / barely | ring + 1.6 m flash | yes | flash mild | - | - |
| `zone_enter` / `zone_exit` | no / no (subtle by design) | 40-90 cm glints | yes | no | - | - |
| `directors_call` | **yes** | 25 m sweeping ring of 160 upright puffs | ignores Colour | warm stage-lamp white (1, 0.85, 0.6); the sweep itself is fine | - | - |
| `NS_blade_spray` | **no** (8 m: barely) | 3-6 cm sprites | n/a | no | no | no |

## 2. What changed

### The material: one tier-aware master instead of three systems

**`VFX/M_Fx_Tier`** (new): translucent (alpha, not additive), unlit, two-sided, used with Niagara sprites, mesh particles and ribbons. Two Custom HLSL nodes (emissive and opacity) read **`MPC_Look.Clay`**:

| Tier (`Clay`) | Fill | Rim |
|---|---|---|
| night, cel (0) | particle colour x `NightBright` (12): the painted look | thin ink rim (`InkCol`, the outline's ink 0.012/0.012/0.028), `NightRim` 0.6 |
| broadcast (-1) | darker, bluer: colour x `BcastTint` (0.40, 0.55, 0.82) x `BcastBright` | dark rim (0.01, 0.02, 0.05), `BcastRim` 1.0 |
| clay (1) | crumbs: colour x `ClayTint` (0.86, 0.70, 0.54) x a fake dome shade (lit from top-left, no shadow) x `ClayBright` | brown rim (0.10, 0.06, 0.035), `ClayRim` |

- Shapes by `Shape`: 0 hard-edged disc chip with a rim, 1 ring (unused, see section 7), 2 four-point clack star with four short diagonals and a dark halo, 3 soft puff (no rim).
- No light emission, no lit translucency, no Lumen dependence, no texture.

Instances (new):

| Instance | Shape | Clay | Broadcast | Use |
|---|---|---|---|---|
| `MI_Fx_Chip` | 0 | bright 2.2, rim 0.8 | bright 2.6 | ice chips and snow (they turn blue-grey with a dark rim on broadcast) |
| `MI_Fx_Spark` | 0 | tint neutral (1, 0.93, 0.85), bright 3.5, rim 0.6 | tint neutral 0.8, bright 3.5 | anything in a team or pickup colour (it keeps its hue on broadcast); NightRim 0.35 |
| `MI_Fx_Star` | 2 | bright 3.0, rim 0.6 | neutral tint, bright 1.4 (a grey star with a dark halo on white ice) | clack stars |
| `MI_Fx_Puff` | 3 | bright 3.0 | bright 2.6 | snow puffs |
| `MI_Fx_Ring` | 1 | bright 3.5 | bright 3.5 | built and saved; not used by any system yet (section 7) |

### The systems (user parameters: every name and meaning kept; additions in bold)

All emitters come from `OmnidirectionalBurst` (Self life cycle, Once, Complete) and run on the CPU. All of them are translucent, cast no shadows and sit over the ice.

| System | Emitters now (count at Strength 1.0, size, lifetime) | Reads from the payload |
|---|---|---|
| `NS_check` (rebuilt) | **Chips** 14 + 22 S (36), 7-20 cm, velocity-aligned, 0.2-0.3 s, at skate height (15 cm), thrown out and up. **Ring**: 30 + 14 S (44) chip dots on a 35 cm ring flung outward flat on the ice (6 cm) at 4.8-9.2 m/s with drag 4, so a 2-4 m shock ring in team colour, 0.26 s. **Star**: 1 white clack star, 110 + 70 S cm (1.8 m), at shoulder height (1.4 m), pops from 55 % to 100 % in 0.06 s, 0.25 s | Strength (counts, sizes, speeds); Colour (ring 80 %, chips 20 %); **`User.Dir`** (new Vector3f, default 0): the chip velocity origin is offset by -60 cm x Dir, so with Dir 0 the burst is radial, and with a rink-plane unit vector the chips are thrown along the hit (`b17-after-check-night` uses Dir +x). Total peak 81 particles; nothing lives past 0.3 s, so it does not hide A's puck pop |
| `NS_knockdown` (rebuilt) | Spray 18 + 26 S (44) chips, 7-20 cm, 0.35-0.6 s; Puff 5 + 5 S (10) snow puffs 55-130 cm growing x1.7, alpha 0.6, 0.5-0.8 s. **No slide streak**: the payload has no direction (section 4) | Strength, Colour 10 % |
| `NS_board` (rebuilt for A's Variant) | Variant 0, puck: Chips 8 + 12 S (20), 5-14 cm, 0.22-0.35 s. Variant 1, skater: Body 20 + 26 S (46), 9-25 cm, 35 cm up, 0.3-0.5 s; Puff 4 + 4 S (8), 45-100 cm; Shake 2-3 thin dashes, 150-260 x 9 cm, at 1.18 m (the board top is 1.2 m), 0.14-0.18 s | Strength, **Variant** (0 puck, 1 skater; added); Colour 15 % (Who is -1, so white) |
| `NS_poke` (rebuilt) | Variant 0: Clack star 55-80 cm at 30 cm (blade height), 0.2 s, plus 8 chips. Variant 1, mistimed: 6 snow puffs, 25-40 cm, 0.35-0.5 s | Strength, Variant |
| `NS_lift` (rebuilt) | Variant 0: star 70 cm, plus 16 chips scooped up 0.3-0.45 s. Variant 1: 6 puffs | **`User.Variant` added** (the binder already sends it; clean and miss now differ) |
| `NS_shot` (rebuilt) | 12 + 18 S (30) chip streaks: non-uniform 10 x 40 cm, velocity-aligned, 3.5-14 m/s, 0.18-0.3 s | Strength, Colour 25 % |
| `NS_shot_blocked` (rebuilt) | 14 chips plus 22 ring dots on a 25 cm ring flung out flat, red (1, 0.1, 0.2) with 30 % Colour, 0.25-0.4 s | Colour |
| `NS_foul` (rebuilt) | **The red flash is gone.** A flat ring of 30 dots on the ice, red (1, 0.12, 0.08) with 35 % of the fouler's colour, 0.5 s; Variant 1 adds a second ring 0.18 s later (the double whistle) | Variant, Colour |
| `NS_blade_spray` (rebuilt) | 16 + 24 S (40) chips, 6-17 cm, a sideways fan low to the ice (flattened source, local Y), 1.8-8 m/s, 0.3-0.45 s | Strength (= lean / 35 deg) |
| `NS_goal`, `NS_win`, `NS_pickup_spawn`, `NS_pickup_take`, `NS_pickup_idle`, `NS_effect_start`, `NS_effect_end`, `NS_zone_enter`, `NS_zone_exit`, `NS_faceoff` (Result) | Same emitters, counts and sizes; material `M_Fx_Sprite` swapped for `MI_Fx_Spark` | unchanged |
| `NS_pass`, `NS_faceoff` (Burst) | material swapped for `MI_Fx_Chip` | unchanged |
| `NS_effect_start` Flash | material swapped for `MI_Fx_Puff` (a soft puff, not a lamp flash) | unchanged |
| `NS_directors_call` | Same 160-sprite sweep (15 m/s for 0.8 s, about 25 m across); colour fixed warm lamp white (1, 0.85, 0.6) → neutral ice white (0.86, 0.94, 1.0); material `MI_Fx_Chip` (ink rim) | unchanged (still ignores Colour) |

### Verdicts after

Night: strips `b17-after-night-all` and `b17-after-check-night`. Clay and broadcast: `b17-after-check-clay`, `b17-after-check-bcast`, `b17-after-tiers`.

| Effect | Night | Clay | Broadcast |
|---|---|---|---|
| `check` | **yes**: the loudest contact picture (star, team ring, chips) | **barely to yes**: ring and brown crumbs read through the focus blur; the star is faint | **yes**: ring dots and chips with dark rims; the star is a weak grey |
| `knockdown` | yes | barely (crumbs) | yes (blue-grey chips) |
| `board` Variant 1 (skater) / 0 (puck) | yes / barely (small by design) | not captured | yes / not captured |
| `poke` clean / miss | barely at 25 m (small star by design; yes at 8 m) / barely | - | - |
| `lift` clean / miss | barely / barely | - | - |
| `shot` | yes (streaks) | - | - |
| `shot_blocked` | yes (dark band) / barely (pool) | - | - |
| `foul` | yes (dark band) / barely (pool: red mixed with blue reads pink) | - | - |
| `blade_spray` | yes at 8 m, barely at 25 m; obvious at the follow camera (the play strips) | barely | yes (8 m) |
| `goal`, `pickup_take`, `directors_call` | yes | - | - |
| `effect_start`, `pickup_spawn`, `faceoff` won | barely to yes | - | - |
| `faceoff` drop, `pass`, `zone_*`, `win` | barely / no (small or subtle by design) | - | - |

## 3. Film-themed, and what replaced it

- **`NS_foul` red flash** (B3b called it "a red cut light"): removed. A flat red ring of dots on the ice at the contact; a second ring for the second whistle.
- **`NS_board` orange flash sprite** (a lamp blink half-buried in the ice): removed. Chips off the board face, and for a skater (Variant 1) a burst plus a shake line on the board top.
- **`NS_directors_call` warm lamp white**: now a neutral ice white with an ink rim. It is still the one sweep across the ice; no film imagery was in its shape.
- **`NS_effect_start` flash**: now a soft puff material.
- No clapper, lens flare, grain or cut shape remains in any `NS_`. The film tails in `MS_goal` / `MS_lose` are sound and outside this lane.

## 4. What the binder's payload lacks (for the lead to ask device A)

1. **`check`**: the push direction as a world vector. Niagara user parameter **`User.Dir`** (Vector3f, rink plane: x = rink x, y = rink y (A says "z" in sim terms), z = 0, unit length; zero = no direction). `NS_check` already reads it: radial when zero, chips thrown along it when set. Binder line: `C->SetVariableVec3(TEXT("User.Dir"), FVector(N.X, N.Y, 0))` with N the sim's contact normal from checker to victim (`CollideSkaters` has it as `N` / `Dir`).
2. **`knockdown`**: the same **`User.Dir`** from `KnockDown`'s `KnockDir` (it is already stored on the victim). With it, B adds the slide streak (a stretched chip trail along -Dir), which is not built now.
3. **`board`**: the board normal, **`User.Normal`** (Vector3f, unit, pointing into the rink). The sim has it (`Puck.Pos + N * R` in `JamSim.cpp`, and `BoardDistance` returns `OutNormal`). With it, the chips leave the board face into the rink instead of radially, and the shake line can be aligned along the board.
4. A's new `check` Variant (0 front, 1 side_l, 2 side_r, 3 behind) is **accepted and ignored**: `NS_check` has no `User.Variant`, so setting it is a no-op. It cannot give a world direction, as A says.
5. Optional: `shot` direction (`User.Dir`) would turn the chip streaks into a one-sided spray off the blade.

## 5. Budget notes (no GPU numbers: shared GPU)

| System | Peak particles at S 1.0 | Largest sprite | Lifetime | Blend | Over the ice |
|---|---:|---|---|---|---|
| `NS_check` | 81 (36 chips + 44 ring dots + 1 star) | star 1.8 m (one quad) | ≤ 0.3 s | translucent unlit | yes |
| `NS_knockdown` | 54 | puff 1.3 m x 1.7 = 2.2 m (soft, alpha ≤ 0.6) | ≤ 0.8 s | translucent | yes |
| `NS_board` V1 / V0 | 57 / 20 | puff 1.0 m x 1.6; shake dash 2.6 x 0.09 m | ≤ 0.6 s | translucent | at the board face |
| `NS_poke` | 9 / 6 | star 0.8 m | ≤ 0.5 s | translucent | yes |
| `NS_lift` | 17 / 6 | star 0.7 m | ≤ 0.5 s | translucent | yes |
| `NS_shot` | 30 | 0.10 x 0.40 m | ≤ 0.3 s | translucent | yes |
| `NS_shot_blocked` | 36 | 13 cm | ≤ 0.4 s | translucent | yes |
| `NS_foul` V0 / V1 | 30 / 60 | 17 cm | 0.5 s (+0.18 s) | translucent | yes |
| `NS_blade_spray` | 40 per spawn; the toy re-spawns every 8 frames while lean ≥ 25 deg, so about 3 overlap: **about 120 per hard-turning skater** (was about 115 at the old 38) | 17 cm | ≤ 0.45 s | translucent | yes |
| patched systems | unchanged counts (goal 160, win 180, directors_call 160, the rest ≤ 64) | unchanged | unchanged | **additive → translucent** | yes |

Translucent (not additive) sprites sort and blend per pixel like the old ones and cost about the same per pixel. The largest overdraw is the 1.8 m check star and the knockdown puffs. All lifetimes are 0.8 s or less except `win` and `directors_call` (unchanged). No lit translucency, no light-emitting particles, nothing that needs Lumen (B1c). The worst case on screen is several turning skaters: about 6 x 120 = 720 small chips.

## 6. Proof in play (night, default figure, 92 s of game time)

- Event counts in the 92 s (from the bus callback): goal 2, faceoff 6, period 2, lift 26, pass 7, board 19, poke 16, shot 3, bark 11.
- **No `check`, `knockdown` or `foul` fired.** This tree's bots did not land a body check in 92 s, and penalties are off in `hockey.regular`. Their proof is the binder-equivalent spawn (sections 1 and 2), not play. A's "tackling that reads" code (on `main` since 20:07) is not in this tree.
- One strip per contact event as it happened (game camera frame, then 6 frames from 25 m behind the event): `b17-play-lift` (clean, t 12.0 s), `b17-play-board` (puck, t 21.8 s), `b17-play-poke` (clean, t 30.3 s), `b17-play-shot` (t 36.9 s). Two captures were made per kind; the second ones are in scratch.
- **The blade spray now reads at the follow camera**: the `game` frame of `b17-play-lift` shows the fans of three skaters. Not yet checked on `humanoid_b1` (section 7).
- Check on the other tiers: `b17-after-check-clay`, `b17-after-check-bcast` (binder-equivalent spawn; no check fired in play).

## 7. Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/VFX/`)

| File | Before | After |
|---|---:|---|
| `M_Fx_Tier` | new | 25,591 |
| `MI_Fx_Chip` | new | 8,655 |
| `MI_Fx_Spark` | new | 9,916 |
| `MI_Fx_Star` | new | 10,355 |
| `MI_Fx_Puff` | new | 8,139 |
| `MI_Fx_Ring` | new | 9,954 |
| `NS_check` | 332,103 | 761,660 |
| `NS_knockdown` | 328,807 | 533,445 |
| `NS_board` | 564,054 | 972,237 |
| `NS_poke` | 569,986 | 736,361 |
| `NS_lift` | 331,442 | 733,084 |
| `NS_shot` | 331,864 | 311,617 |
| `NS_shot_blocked` | 323,973 | 519,897 |
| `NS_foul` | 564,393 | 518,762 |
| `NS_blade_spray` | 323,186 | 311,360 |
| `NS_goal` | 331,173 | 317,262 |
| `NS_win` | 542,894 | 531,195 |
| `NS_pass` | 328,503 | 314,769 |
| `NS_pickup_spawn` | 323,118 | 309,671 |
| `NS_pickup_take` | 322,740 | 308,931 |
| `NS_pickup_idle` | 294,296 | 294,329 |
| `NS_effect_start` | 558,510 | 530,819 |
| `NS_effect_end` | 323,877 | 310,063 |
| `NS_zone_enter` | 322,701 | 308,838 |
| `NS_zone_exit` | 323,052 | 309,183 |
| `NS_faceoff` | 570,147 | 542,785 |
| `NS_directors_call` | 318,476 | 307,658 |

- Unchanged: `M_Fx_Sprite`, `M_Fx_Ribbon`, `NS_puck_trail`.
- The VFX folder grows by about +2.1 MB.
- Devlog: 13 stills `devlog/shots/20261112-2041*-B-b17-*`, `-204200-B-b17-play-board` and `-204924-B-b17-after-night-all` (about 2.6 MB with captions).

Ledger rows. **Replace** the existing rows for the 21 changed `NS_` files (match by path), and **add** the 6 materials:

```
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Tier.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | translucent unlit particle master reading MPC_Look.Clay (night ink rim, broadcast darker bluer chips with dark rim, clay shaded crumbs); shapes chip/ring/star/puff by Shape; no texture (lane B17) (25591 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Chip.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Fx_Tier, ice chips and snow (lane B17) (8655 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Spark.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Fx_Tier, team/pickup-coloured sparks, hue kept on broadcast (lane B17) (9916 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Star.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Fx_Tier, clack star with dark halo (lane B17) (10355 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Puff.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Fx_Tier, soft snow puff (lane B17) (8139 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Ring.uasset | material | 0.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | n | instance of M_Fx_Tier, ring shape, unused (flat facing did not render; lane B17) (9954 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_check.uasset | vfx | 0.8 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event check: ice chips (User.Dir optional), team shock ring on the ice, clack star at 1.4 m; 0.25-0.3 s, peak 81; User.Strength, User.Colour, User.Dir (lanes B3, B17) (761660 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event knockdown: snow spray and puffs, peak 54 (lanes B3, B17) (533445 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 1.0 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event board: Variant 0 puck chips, 1 skater burst + puff + board-top shake line; peak 57 (lanes B3, B17) (972237 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_poke.uasset | vfx | 0.7 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event poke: Variant 0 clack star + chips, 1 puff (lanes B3b, B17) (736361 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 0.7 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event lift: Variant 0 clack star + scooped chips, 1 puff; User.Variant added (lanes B3b, B17) (733084 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_shot.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event shot: 30 velocity-aligned chip streaks (lanes B3, B17) (311617 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_shot_blocked.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event shot_blocked: chips + small red ring on the ice (lanes AUD, B17) (519897 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_foul.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event foul: red ring on the ice, second ring for Variant 1; red flash removed (lanes B3b, B17) (518762 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_blade_spray.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | sideways snow fan, at most 40 chips 6-17 cm; User.Strength, User.Colour (lanes B10, B17) (311360 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event goal; tier material MI_Fx_Spark (lanes B3, B17) (317262 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_win.uasset | vfx | 0.5 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event win (confetti); tier material (lanes B3, B17) (531195 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pass.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pass; tier material (lanes B3b, B17) (314769 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_spawn.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_spawn; tier material (lanes B3, B17) (309671 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_take.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_take; tier material (lanes B3, B17) (308931 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_idle.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | pickup idle sparks; tier material (lanes B10, B17) (294329 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_effect_start.uasset | vfx | 0.5 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event effect_start (rising ring + puff); tier material (lanes B3b, B17) (530819 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_effect_end.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event effect_end; tier material (lanes B3b, B17) (310063 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_zone_enter.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event zone_enter; tier material (lanes B3b, B17) (308838 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_zone_exit.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event zone_exit; tier material (lanes B3b, B17) (309183 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_faceoff.uasset | vfx | 0.5 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event faceoff; User.Variant 0 drop, 1 won, 2 lost; tier material (lanes B3b, B17) (542785 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_directors_call.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event directors_call: one neutral ice-white sweep (warm lamp tone removed); tier material (lanes B3, B17) (307658 bytes) |
```

## 8. Six strips for the operator (`devlog/shots/`)

1. `20261112-204138-B-b17-before-night-all` next to `20261112-204924-B-b17-after-night-all`: every effect, before and after, same frame and layout.
2. `20261112-204138-B-b17-after-check-night`: the new check, 6 frames at 25 m.
3. `20261112-204139-B-b17-after-tiers`: check, knockdown and the skater-into-boards on night, clay and broadcast.
4. `20261112-204138-B-b17-before-tiers`: why the material changed (nothing showed on clay or broadcast).
5. `20261112-204140-B-b17-play-lift`: in play; the follow-camera frame also shows the new blade-spray fans.
6. `20261112-204140-B-b17-after-board-skater-8m`: A's new skater-into-boards, Variant 1.

Also filed: `-after-check-clay`, `-after-check-bcast`, `-play-board`, `-play-poke`, `-play-shot`.

## 9. What I could not verify

- **`check`, `knockdown` and `foul` in play**: none fired in the 92 s bot match (penalties off, and A's tackling code is not in this tree). They are verified by binder-equivalent spawns only.
- **`humanoid_b1`**: `NS_blade_spray` was checked on the default figure only (its spawn code in `SkeletalSkaterVisual.cpp` and `ToySkaterVisual.cpp`). Whether `BP_SkaterVisual_TB1` spawns `NS_blade_spray` at all is in `Skaters/`, which I may not open; the asset name does not appear in its `.uasset`.
- **A flat ring lying on the ice**: Niagara sprites with `CustomFacingVector` and a SetParameters `Particles.SpriteFacing` (0, 0, 1) did not render at all, at 4 cm or at 40 cm; camera-facing rendered. The shock rings are therefore dots flung outward flat on the ice. A mesh-renderer plane would give a true painted ring; it was not built. `MI_Fx_Ring` is unused.
- **Not captured**: clay and broadcast for every non-contact effect (night only), and the board puck variant on clay.
- **GPU cost**: none measured (shared GPU). Overdraw and per-pixel cost are estimated, not measured.
- **Motion and temporal behaviour**: frozen frames only, plus four in-play strips at time dilation 0.02.
- **The new `check` Variant (0-3) and `User.Dir` from A's binder**: tested only by setting them myself (Dir +x).
- **Small items**: one input write failed (`NS_directors_call` position offset; harmless, it stays at z 0). During the proof match a stale poller of mine ran for about 3 minutes in parallel; I stopped it, and its captures were overwritten by the clean run.

## 10. Lead must decide

1. **Ask device A** for `User.Dir` on `check` and `knockdown`, and `User.Normal` on `board` (section 4, exact names). With them B adds the knockdown slide streak and board-face chips.
2. **The clack star on broadcast** reads weakly (a grey star on white ice). Options: accept it, since the ring and chips carry the check there, or give broadcast a team-coloured star.
3. **Clay** reads "barely to yes" through B14's focus blur at 25 m. Options: accept, or raise `ClayBright` / `ClayRim` on the instances (one-line changes).
4. **Blade-spray volume**: about 120 chips per hard-turning skater. If several skaters turn at once near the camera, A could raise the 8-frame spawn interval to 12.
5. **A mesh-ring follow-up** (a painted shock ring flat on the ice) if the dot rings are not enough.
6. Paste the ledger rows.
