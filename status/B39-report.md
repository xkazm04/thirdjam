# B39 report: chips without the clay rim, the set in un-frozen play, the old trail material deleted (device B)

Lane B39, device B, machine clock 19:25 to 19:36. Tree `C:\Users\kazda\jam\wt\b-vfx` (`dev-b` 108ab3c), port 8000, one editor session (pid 35756), the only editor on the machine, left through `ExitEditor` with no dirty package; no `UnrealEditor` process is left. Saved, each with no play session open: `VFX/MI_Fx_Chip`, `VFX/MI_Fx_Spark`, `VFX/NS_shot`. Deleted: `VFX/MI_Fx_Trail`. No Blueprint saved, no git writes, no build, no cook. `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` back to 0 / 0 / empty; no render setting touched. Backups of the four files as they were at lane start: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b39` (`ub.sh` the un-frozen burst runner, `chain.sh`, `shot.mjs` the shot edit, `tk.py`, `bruisers_clay.cartridge.json`, all frames in `shots/`).

Every still in this lane is **un-frozen**: the game's own screenshot requested from a tick callback while the match runs.

## Verdicts
| Item | Verdict | Still |
|---|---|---|
| 1. Chips and sparks without the rim on clay | **Rim gone; the dots are still round.** `ClayRim` is 0 on both instances. The team-colour ring dots of a check lose the pale rim and read as flat soft dots, not as rimmed bubbles; they are still round discs, because that is the shape the master draws. Night unchanged (same hit, before and after) | `b39-chips-ab` |
| 2a. Knockdown with a slide | **Reads, nothing stray**: dark spray specks and a short soft smudge behind the body; no line, no ring. The slides were short even in Bruisers, so the long continuous band is still shown only in frozen frames | `b39-unfrozen-knockdown-slide` |
| 2b. Cross-check | **Small at game size**: splinters and the snow kick are seen at +1 tick; the 3-frame mark is not made out in these two (one victim turned away, one at the boards) | `b39-unfrozen-crosscheck` |
| 2c. Hard board crash | **Reads**: at Strength 0.90, white chips and the grey snow cloud at the skates at +3 ticks, mostly gone by +9. A light brush (0.53) shows nothing. The hard chips are round white lumps 7 to 14 cm | `b39-unfrozen-board` |
| 2d. Shot | **A fault found and fixed; the trail not seen.** `NS_shot` threw a fan of pale streaks a metre and more over the shooter's head, still there 15 ticks later, in every scrum shot. Made low and short; the same shots after show nothing over the heads. The new puck trail could not be judged: the scripted match gave only short shots (the best a one-timer, Strength 0.87, with no visible trail) | `b39-unfrozen-shot` |
| 3. `MI_Fx_Trail` | **Deleted**: no referencer in the asset registry, no mention in `Game/Source` or `Game/Config` | |

## 1. Chips and sparks
- `MI_Fx_Chip`: `ClayRim` 0.55 -> **0**. `MI_Fx_Spark`: `ClayRim` 0.6 -> **0**. Nothing else changed (`ClayBright`, `NightRim`, `Shape` as they were). `ClayRim` only feeds the clay branch of the master, and the night frames before and after look the same.
- I did not move them to another shape: the master's shape 0 is a hard-edged disc with a shaded side on clay, and the other shapes are a ring, a star and a soft puff. A true fleck needs either a new shape in `M_Fx_Tier` or elongated particles in each system.
- Systems that reference `MI_Fx_Chip` (asset registry): `NS_blade_spray`, `NS_board`, `NS_breakaway`, `NS_check`, `NS_directors_call`, `NS_faceoff`, `NS_knockdown`, `NS_lift`, `NS_pass`, `NS_poke`, `NS_save`, `NS_shot`, `NS_shot_blocked`.
- Systems that reference `MI_Fx_Spark`: `NS_check`, `NS_effect_end`, `NS_effect_start`, `NS_faceoff`, `NS_foul`, `NS_goal`, `NS_pickup_idle`, `NS_pickup_spawn`, `NS_pickup_take`, `NS_post`, `NS_save`, `NS_shot_blocked`, `NS_win`, `NS_zone_enter`, `NS_zone_exit`.
- **Seen on clay after the change:** `NS_check` with `NS_knockdown` (the hit frames), `NS_lift` (the cross-check frames), `NS_board` (the hard crash), `NS_shot` (only that nothing shows at +9 and +15). **Not looked at:** `NS_blade_spray`, `NS_poke`, `NS_pass`, `NS_save`, `NS_faceoff`, `NS_breakaway`, `NS_shot_blocked`, `NS_directors_call` and every spark-only system (goal, win, pickups, zones, foul, post).

## 2d. `NS_shot` (not on your list; found in item 2 and changed)
| | Before (B17) | After |
|---|---|---|
| Count | `12 + 18 S` | `10 + 12 S` |
| Streak size | `(6 + 4 S) x (18 + 22 S)` cm | `(4 + 2 S) x (10 + 10 S)` cm |
| Speed | 3.5-14 m/s from a point 18 cm **below** the ice (so upward) | 2.5-9 m/s from a point at 4 cm (outward) |
| Gravity, drag | -980, 1.0 | -2500, 2.5 |
| Life | 0.18-0.3 s | 0.12-0.2 s |
`S` is now clamped to 0..1. Peak particles 30 -> 22. In B37 I took those streaks over the heads for the lift's splinters; they were the shot's.

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Before -> after |
|---|---|
| `MI_Fx_Chip` | 8,563 -> 10,106 |
| `MI_Fx_Spark` | 9,916 -> 10,487 |
| `NS_shot` | 311,617 -> 293,520 |
| `MI_Fx_Trail` | 11,519 -> **deleted** |

Ledger: **replace** the rows of `MI_Fx_Chip`, `MI_Fx_Spark`, `NS_shot`; **remove** the row of `MI_Fx_Trail`.
```
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Chip.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Fx_Tier, chips and snow (B17); white with a warm shadow side on the clay tier (B22); no rim on clay, ClayRim 0 (B39) (10106 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Spark.uasset | material | 0.01 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_Fx_Tier, team/pickup-coloured sparks, hue kept on broadcast (lane B17); no rim on clay, ClayRim 0 (B39) (10487 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_shot.uasset | vfx | 0.28 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event shot: 10 to 22 short velocity-aligned chip streaks sprayed low off the blade, gone within 0.2 s (lanes B3, B17, B39) (293520 bytes) |
```

## What I could not verify
- The new puck trail behind a long hard shot: none occurred in the scripted matches I ran.
- The cross-check's 3-frame mark in un-frozen play (splinters only).
- A long slide un-frozen: Bruisers knockdowns last longer but the bodies still travelled little.
- A Strength 1.00 board crash up close (my crop of it missed); the 0.90 one is shown.
- The chip and spark change on the systems listed as not looked at, and on tiers other than clay and night.
- The new `NS_shot` up close at +1 to +3 ticks (only that nothing is over the heads at +9 and +15); `NS_shot` on night.
- Cook: not run. One asset was deleted, so this lane needs the cook check before the push.

## Decisions for you
1. **Round dots.** If the flat round dots still read as bubbles to the operator, the next step is in the systems, not the material: make the check's ring dots short streaks along their motion (one lane, `NS_check` and its relatives), or add a fleck shape to `M_Fx_Tier`. Say which, or accept.
2. **The hard board chips** are round white lumps up to 14 cm: same choice as 1, or cut their top size to about 9 cm.
3. **The cross-check mark** is three frames long and I could not catch it un-frozen. If the operator does not see it in play either, lengthen it to 5 frames; I would wait for A's `Dir` first.
4. **A shot drill.** The trail and the shot effect need one long hard shot in open ice to be judged; a drill from A that shoots from the blue line would settle both in one run.
