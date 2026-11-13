# B30 report: tackling effects that let the figures be seen, and a slide trail (device B)

Lane B30, device B, machine clock 11:43 to 12:26. Tree `wt/b-vfx` (`lane/b-vfx` = `dev-b` 355565d), editor on port 8000 (pid 35784), left through `ExitEditor` with no dirty package; the process is gone (the remaining `UnrealEditor` pid 27012 is the clay lane's editor on 8002). No git, no build, no cook. No P4 asset touched; `/Game/Maps/Rink` not saved; no `_Scratch/`. Backups of the five changed assets as they were at 11:44: scratch `bk/`. Scratch (all scripts): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b30` (`spec30.mjs` the four systems, `nb.mjs` the builder, `tk.py` the test rig, `covm.py` the coverage measure, `dslhook.mjs` the Blueprint hook).

## Verdicts
| Item | Verdict |
|---|---|
| 1. `NS_check` + `NS_knockdown` over the hit | **PASS**: figure coverage at the game camera from the hit to +12 dropped from 21-46 % to 0-8 % (table below); nothing rises over the bodies; the effect is gone by about +10 to +12 |
| 1b. A's `max(0.5, Strength)` | **done** in all three hit systems: 0.5 gives the restrained burst, 1.0 more chips and a wider, faster ring, still low |
| 1c. A's "human involved" `Variant 4` | **not built; not needed now** (reasons below) |
| 2. `NS_board` (skater into the boards; puck chips) | **PASS** with one remark: the snow balls that rose over the glass for 30+ frames are gone; the brief shake line (6 frames) sits at 1.18 m and crosses a figure standing in front of it in open ice (at a real wall it lies on the board top) |
| 3. Slide trail (`NS_slide_trail` + a `SlideFx` component in both scripted Blueprints) | **PASS on clay, weak on night** |
| 4. Clay check (colour, size, bloom, flicker, nothing bright on the surface) | **PASS** from stills: no bloom, nothing bright left lying; flicker not judged (stills only) |
| 5. Frames | test figures driven as SK6 did (deterministic, same poses before and after); the real seed-1 clay knockdown was also run (sim frame 1006, human down at t 16.79 s) but at the end boards and far from the camera: used only to confirm the chain runs |

## 1. `NS_check` and `NS_knockdown` (S = `saturate(max(0.5, User.Strength))`, 60 fps)
| Emitter | Before (B17/B18) | After (B30) |
|---|---|---|
| check Star | 1 white star 110-180 cm at 1.4 m, 0.25 s (15 frames) | 1 star **30-50 cm at 0.6 m** (between the bodies, under the faces), **0.05 s = 3 frames** |
| check Ring | 30 + 14 S team dots 13-24 cm at 6 cm, 0.26 s | **20 + 10 S** dots **7-12 cm at 2 cm** on the ice, flung outward at 420-780 cm/s, **0.2 s**; S widens it |
| check Chips | 14 + 22 S, 7-20 cm at 15 cm, thrown up | **8 + 10 S, 3.5-8 cm at 6 cm**, thrown along the hit (`User.Dir`), 0.12-0.2 s |
| knockdown Spray | 18 + 26 S, 7-20 cm, radius 45 | **8 + 10 S, 3.5-8 cm at 6 cm**, along the hit, 0.12-0.18 s |
| knockdown Puff | 5 + 5 S, 30-65 cm, alpha 0.3 | **3 + 2 S, 18-28 cm at 12 cm** (skate to shin), along the hit, alpha 0.35 fading, **0.13-0.18 s** |
`NS_knockdown` now has `User.Dir` (the binder already sends it). **With the hit-stop:** frames 0-3 show only the flat ring starting on the ice and the small star between the bodies at waist height; the star is gone at frame 3.

**Coverage at the game camera** (test knockdown from the right; figure pixels = difference between frames with and without the figures; covered = pixels the effect changes inside them; both figures together; the two figures overlap on screen, so per figure is not separable at this camera):
| Frame | clay before | clay after | night before | night after |
|---|---:|---:|---:|---:|
| hit | 21 % | **5 %** | 24 % | **8 %** |
| +3 | 23 % | **3 %** | 34 % | **6 %** |
| +6 | 23 % | **3 %** | 29 % | **6 %** |
| +9 | 23 % | **1 %** | 39 % | **3 %** |
| +12 | 23 % | **0 %** | 46 % | **2 %** |
Frames +1, +2, +4 ... were not captured (every 3rd frame); the strips show +15 to +21 with nothing over the figures after.

**Particle counts (peak per spawn, S = 1 / S = 0.5):** `NS_check` 81 -> **49 / 39** (down); `NS_knockdown` 54 -> **23 / 16** (down); `NS_board` skater 57 -> **24 / 18**, puck 20 -> **14 / 10** (down); `NS_slide_trail` new: up to **12 per 0.06 s** at full slide speed, about **50 spray + 170 flecks live** at 4.7 m/s, falling to 0 under 1 m/s (up).

**Variant 4 (human involved):** not built. With the hit-stop on the human's knockdowns, the hit already reads; a bigger burst for the human would cover the human's own figure, which is what this lane removes. If A wants a cue, the cheapest is the ring in the human's team colour at S 1.0 (no new emitter); say so and B adds it behind `Variant 4` in 10 minutes.

## 2. `NS_board`
Puck (Variant 0): **6 + 8 S** chips 3.5-8 cm at 5 cm (was 8 + 12 S, 5-14 cm). Skater (Variant 1): body chips **8 + 10 S, 4-9 cm at 8 cm**, 0.12-0.18 s (were 20 + 26 S, 9-25 cm at 35 cm, the balls that rose over the glass); puffs **2 + 2 S, 18-26 cm at 10 cm**, 0.13-0.17 s (were 45-100 cm at 30 cm); shake line kept at 1.18 m, **6 cm thick, 0.08-0.1 s** (was 9 cm, 0.14-0.18 s). Other effects on puck/board contact (`NS_shot_blocked`, `NS_poke`, `NS_lift`) were **not changed**.

## 3. The slide trail
- **`VFX/NS_slide_trail`** (new; built on a copy of `NS_blade_spray`, so its material path and user parameters are the same family): two emitters, both bursting every 0.06 s **forever while active**, with counts scaled by the component's own speed (`length(Engine.Owner.Velocity)`), zero below 1 m/s:
  - `Spray`: 0-12 per burst (full at 5 m/s), **shadowed blue-grey** (0.62, 0.70, 0.84) chips 5-9 cm at 4 cm, kicked out 60-180 cm/s, gravity, 0.3-0.45 s;
  - `Scrape`: 0-10 per burst (full at 4 m/s), **darker blue-grey** (0.36, 0.44, 0.58) flecks 7-12 cm lying at 1 cm in world space, fading over 1.3-1.7 s: they stay where the body passed, so a sliding body leaves a short dark trail that fades as it slows.
- **Blueprint hook (both `BP_SkaterVisual_TB1` and `_TB1_P3`, visual only):** a new `NiagaraComponent` **`SlideFx`** on the root (asset `NS_slide_trail`, Auto Activate off), and two lines in the `KnockDown` event: `ActorComponent.Activate (SlideFx, reset)` right after `bDown` is set true, `ActorComponent.Deactivate (SlideFx)` right after `bDown` is set false (the release). Nothing else in the graph changed (diff of the read-back DSL: exactly these two lines); takes, priorities, timing untouched; both compile `BS_UP_TO_DATE`. The component sits on the visual actor, so A's hit-stop dilation pauses it with the figure.
- **Seen:** on clay the blue-grey snow and the dark flecks read clearly on the pale ice (`b30-slide-trail-clay`). On night the tier's particle material draws them pale: they read weakly on the lit night ice (`b30-slide-trail-night`). **Not seen in a real match:** the one real clay knockdown slid along the end boards far from the camera; the trail was tested on a test figure moved along the ice at 4.7 -> 2.5 m/s by a script.

## 4. Clay surface check
No effect of the chain blooms (all particles are the tier's translucent unlit materials at low values); nothing bright stays on the ice: the ring dots and chips are gone within 0.2 s, the scrape flecks are darker than the ice. The orange ring dots in the strips are the attacker's team colour (orange in the test). Flicker in motion was not judged.

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with `.txt`)
- `20261113-120927-B-b30-knockdown-clay-before-after.jpg` (the same test knockdown, every 3rd frame, hit to +21, before above, after below)
- `20261113-120928-B-b30-knockdown-night-before-after.jpg`
- `20261113-120928-B-b30-standing-hit-board.jpg` (after only: the old system was already rebuilt when I shot it; the before is SK12's `sk12-standing-hit`)
- `20261113-122154-B-b30-slide-trail-clay.jpg`, `20261113-122154-B-b30-slide-trail-night.jpg`
- `20261113-122437-B-b30-tackle-clay.jpg` (operator sheet: hit, +3, +6, +9, +15, +30, sliding, get-up)
A first `b30-tackle-clay` filed at 12:21:55 had wrong tiles (stale particles from a broken rig state); I deleted it (mine, two minutes old) and re-filed.

## Assets (bytes, `Game/Plugins/JamAssets/Content/`)
| File | Before -> after |
|---|---|
| `VFX/NS_check` | 761,660 -> 756,028 |
| `VFX/NS_knockdown` | 500,995 -> 533,705 |
| `VFX/NS_board` | 972,237 -> 967,241 |
| `VFX/NS_slide_trail` | new, 513,984 |
| `Skaters/BP_SkaterVisual_TB1` | 2,849,176 -> 2,860,784 |
| `Skaters/BP_SkaterVisual_TB1_P3` | 2,907,210 -> 2,918,758 |

Ledger rows. **Replace** the rows of `NS_check`, `NS_knockdown`, `NS_board` and add the `SlideFx` note to the two Blueprint rows (their existing notes plus the clause below); **add** `NS_slide_trail`:
```
| Game/Plugins/JamAssets/Content/VFX/NS_check.uasset | vfx | 0.7 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event check: 3-frame clack star 30-50 cm at 0.6 m, team ring of dots flat on the ice spreading outward, low chips along User.Dir; strength max(0.5, User.Strength); peak 49 (lanes B3, B17, B30) (756028 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event knockdown: low chips and 3-5 small shin-high puffs along User.Dir, gone by about +10 frames; peak 23 (lanes B3, B17, B18, B30) (533705 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.9 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event board: Variant 0 low puck chips; Variant 1 low skater chips, two small puffs at skate height, 6-frame shake line on the board top; peak 24 (lanes B3, B17, B30) (967241 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_slide_trail.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | slide trail while a knocked-down figure slides: shadowed blue-grey snow and darker blue-grey flecks left on the ice, rate by the owner's speed (0 under 1 m/s); played by SlideFx on BP_SkaterVisual_TB1 and _TB1_P3 (lane B30) (513984 bytes) |
```
Clause for the two Blueprint rows: "+ SlideFx Niagara component (NS_slide_trail), activated on KnockDown down, deactivated on release (lane B30)"; sizes 2.73 MB (`BP_SkaterVisual_TB1`, 2,860,784) and 2.78 MB (`_TB1_P3`, 2,918,758).

## What I could not verify
- The new effects in a real match knockdown at the game camera from the hit (the seed-1 knockdown happened at the end boards; I measured on test figures in the same poses the takes give); with A's hit-stop (not in this tree).
- The slide trail on a real slide and on the night tier at close range; the trail on the `generated`/other figures (only the two scripted Blueprints have the hook).
- Flicker and shimmer in motion (stills only); frame cost (no numbers).
- Whether the binder ever sends `check` with Strength above 1 (S is clamped at 1).

## Decisions for you
1. Variant 4: leave it (my recommendation) or have A send it and B tint the ring in the human's colour.
2. Night: accept the weak slide trail, or let B add a night-only darker colour (the tier material lifts particle colour on night).
3. The board shake line at 1.18 m: keep (it marks a skater hitting the boards at a real wall) or drop it.
