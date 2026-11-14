# B42 report: the effects at game size (device B)

Lane B42, device B, machine clock 02:21 to 03:04. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` c81ea3a4), port 8002, one GPU editor session (pid 25688), left through `ExitEditor` with no dirty package; that process is gone (the two `UnrealEditor` processes still running are the GPU-less editors of other lanes; not touched). No cook commandlet was running when I started saving. **The two figure Blueprints and everything under `Skaters/` were not opened or saved**; the working tree shows eight changed files, all under `VFX/`. No commit, no push, no build, no cook. `r.Editor.Viewport.OverridePIEScreenPercentage` was 1, set to 0 for the stills, read back at 1; `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` back to 0 / 0 / empty. Backups of the eight systems as they were at lane start: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b42` (`spec42.mjs` all systems, `patch42.mjs`, `share.py` the measure, `fz.sh`, `brf.sh`, `brk.sh`, `tk.py`, all frames in `shots/`).

**How the frame share is measured.** Two captures of the same frozen frame at the game camera, 1920 x 1085 (the play session at the window's resolution): one with the effect, one without (the effect's component hidden, or my spawn cleared). After a 1 px blur, a pixel counts when any channel differs by more than 12 of 255; the share is counted pixels over all pixels. The two captures are seconds apart in a frozen world, so anti-aliasing settling adds noise: a frame with no effect left measures 0.2 % (the +20 row below). Two un-paused runs of the same match do not line up pixel for pixel (I tried: the figure sits 30 px apart), so the numbers are from frozen pairs and the strips are un-paused.

## Verdicts
| Job | Verdict | Still |
|---|---|---|
| 1. The stop's spray as a fan | **PASS on the numbers, strong to the eye.** The human's stop from 11.0 m/s: **3.98 % of the frame at +6**, 3.49 % at +12, 0.24 % at +20 (noise: nothing left). Dark slush streaks and a light haze thrown ahead of the skates along the travel. **0.7 % of the figure's own pixels changed at +6.** Bots' stops use the same system with a factor on the distance to the camera; that factor was not measured | `b42-brake-fan` |
| 2. Abilities distinct and seen | **PASS by my own spawns; the game fired none.** At +4, frozen, with a direction: shotgun **2.46 %**, hip check **3.29 %**, shield **3.91 %**, slow puck **3.61 %** (before: 0.01 / 0.05 / 0.23 / 0.10 %). Told apart by colour and shape: a grey-blue blast ahead with white shards; an ochre arc; a slate-blue ring with white shards that holds about 50 frames; a pale frost patch. **In play the shotgun and the hip check get no direction** (see "For the other device"), so they would be round | `b42-ability-kinds` |
| 3. Chips as streaks | **PASS for `NS_check`, `NS_knockdown`, `NS_board`; one emitter left.** Every chip in those three is now a velocity-aligned streak 3 to 5.5 cm wide and 9 to 22 cm long (ratio 2.9 or more by construction), ice white; the check's bead ring is a ring of streaks. A real hit un-paused shows white dashes and no beads. The cross-check mark sits at 0.72 m, below the number. **Left:** the cross-check's small snow kick (6 round chips 4-6 cm): my patch of it did not take and I did not rebuild `NS_lift` | `b42-chips-streaks` |
| 4. The slide trail capped | **Done by count, not judged by eye.** About 210 live -> **about 100** (64 puffs + 38 streak quads). No real slide long enough to show the streak occurred in my runs, so "without visibly thinning" is not shown | |
| 5. The goal | **Partly.** Blue chalk-dust plumes rise from the net and spread along the end over the stands, with a cream fountain of streaks over the net. From centre ice (41 m): 1.17 % at +10, **3.00 % at +40, 3.31 % at +70**, 0.62 % at +100. From 14 m: 6.86 % at +10, 10.84 % at +40. It does not hold the full 108 frames at strength (fading from about +80), it rises above the boards rather than along them, and it was only seen by my own spawn, frozen: the game scored no goal while I watched | `b42-goal` |

Before and after, same measure (frozen, game camera, % of the frame):
| Effect | Before | After |
|---|---:|---:|
| stop's spray at +6 | not measured this way (B40's build: a dozen flecks) | 3.98 |
| shotgun / hip check / shield / slow puck at +4 | 0.01 / 0.05 / 0.23 / 0.10 | 2.46 / 3.29 / 3.91 / 3.61 |
| check with knockdown at +2 | 0.48 | 0.28 |
| hard board crash at +3 | 0.59 | 0.46 |
| cross-check at +3 | 0.33 | 0.20 |
| goal from centre ice at +40 | not measured (my "before" spawn was behind the camera) | 3.00 |

## What device A already draws on the same moments
Read in `status/A145.md`: a hit flash (a point light at the hit, 300 cd, 450 cm), board caps that hop, and on a goal a wave of hopping caps with two red lights running along the end boards. My goal effect is dust in the air above the net and the boards, in the team's colour: it adds no light and nothing on the ice. My board crash stays at the foot of the boards (chips and a snow cloud), under their flash. I did not see the two together: no real hard board hit or goal was captured in this lane.

## Values
**`NS_brake`** (two emitters, one burst 0.04 s after activation; `S` from the owner's speed clamped to 14 m/s; `D = clamp(1.3 - camera distance / 30 m, 0.35, 1)`):
- Cloud: `D x (10 + 8 S)` soft puffs 45-70 cm, blue-grey (0.60, 0.67, 0.82), alpha 0.8.
- Shave: `D x (22 + 18 S)` soft slush streaks 16 x 60-85 cm, dark blue-grey (0.30, 0.36, 0.52), thrown at 12 to 35 m/s with heavy drag from 60 cm ahead of the skates. White streaks did not read on the clay ice at all (my first two builds), so the fan is dark.
- Gone by +20. The velocity rule holds (clamped; nothing on a time step under 0.5 ms, under 2 m/s or over 20 m/s).

**`NS_ability`** (seven emitters; kind by `User.Variant`, only when Strength is above 0.5):
| Kind | What | Count | Life |
|---|---|---:|---|
| 0 shotgun | grey-blue puffs 80-125 cm and white shards 7 x 40 cm, from 1 m ahead along `User.Dir` at 15 to 60 m/s | 24 + 24 | 12 to 19 frames |
| 1 hip check | ochre chalk puffs 60-90 cm on a ring of 95 cm at 0.5 m, shifted 70 cm and pushed along `User.Dir` | 22 | 13 to 19 |
| 2 shield | slate-blue chalk puffs 55-75 cm on a ring of 125 cm at the skates, and 10 white shards rising from it | 30 + 10 | 45 to 54 (shards 21 to 33) |
| 3 slow puck | pale frost puffs 60-85 cm on two rings (55 and 115 cm) at the skater | 26 | 30 to 42 |

**`NS_check`, `NS_knockdown`, `NS_board`**: rebuilt with B30's and B35's counts and speeds; every chip emitter now has a non-uniform size and velocity alignment, colour ice white (the check's ring keeps 10 % of the team colour). The dot ring's material changed from `MI_Fx_Spark` to `MI_Fx_Chip`.

**`NS_lift`**: mark and splinters lowered from 1.0 m to 0.72 m; stars 26-34 -> 20-26 cm.

**`NS_slide_trail`**: spray 16 -> 8 puffs every 0.05 s, 5-10 cm (was 4-8); scrape one quad every 0.04 s (was 0.03).

**`NS_goal`** (rebuilt; `User.Variant` added): Left and Right, 34 soft puffs each, 1.7-2.7 m, the team colour lightened by 35 %, thrown from 1.5 m over the net outwards and up at 7 to 22 m/s, gravity -900, life 96 to 108 frames; Fountain, 26 cream streaks 12 x 55 cm rising over the net. A conceded goal (Variant 1) gets 60 % of the counts. The goal event carries: the puck's position in the net, Strength 1, the scoring team's colour, Variant 0 scored / 1 conceded.

## Peak particle counts (from spawn counts and lifetimes; none measured live)
| System | Before | After |
|---|---:|---:|
| `NS_brake` per stop | 52 | **58** |
| `NS_ability` shotgun / hip check / shield / slow puck | 24 / 12 / 24 / 22 | **48 / 22 / 40 / 26** |
| `NS_check` / `NS_knockdown` / `NS_board` (hard skater crash) | 49 / 23 / 39 | unchanged |
| `NS_lift` | 27 | unchanged |
| `NS_slide_trail` live at full slide speed | about 210 (the critic's ledger: 220) | **about 100** |
| `NS_goal` | 160 (the critic's ledger) | **94** |
Worst moment (a hit into the boards with a slide, a goal, two stops): 49 + 23 + 39 + 100 + 94 + 2 x 58 = **about 420** (the ledger's 600).

## Not verified
- The game's own `ability` event (none fired) and a real goal (none scored): both shown by my own spawns only. The goal un-paused: the net was out of view in the live frames.
- The stop's fan for bots at distance (the distance factor), at 14 m/s, on night, and against the new timber stands (the stops I saw were in open ice).
- The figure coverage at +12 (the measure gave noise there; in the frame the fan is off the figure).
- Whether the fan is too strong for calm ice: it is twice the asked share.
- Every effect of this lane on night. The abilities' dark colours (slate blue, grey-blue) are recoloured by the soft material off clay; not looked at.
- The slide trail at half the puffs in a real long slide.
- The hard board crash and the cross-check mark in a real event after this lane's change (frozen spawns only); my effects together with device A's hit flash and goal wave.
- Why the size patch through the tool did not change the sprites' shape (I rebuilt the three systems instead); the same patch on the cross-check's snow kick is therefore without effect.
- Cook: not run. Eight assets changed.

## For the figure lane (Blueprint asks; I touched neither Blueprint)
1. After each `ActorComponent.Activate (BrakeFx, true)` in `StopTick` (both sites), add one node: `NiagaraComponent.SetVariableFloat (BrakeFx, "User.Strength", StopV0 / 14.0)` (on the `SetBraking` path: `Speed / 14.0`). Tell me when it is in: `NS_brake` then sizes from that value instead of the speed at the commit (one expression on my side).
2. Optional: `NiagaraComponent.SetVariableVec3 (BrakeFx, "User.Dir", the unit travel direction)` at the same place, so the fan follows the travel at the start of the stop, not the owner's velocity 0.04 s later.

## For the other device
1. **`UJamEffectsBinder` does not pass the direction for `ability`** (nor `deke`): `User.Dir` is set only for check, knockdown, lift and poke. Add `Ability` and `Deke` to that line. Until then the shotgun's blast and the hip check's arc are round.
2. **Shotgun: `Where` at the blade** on the frame the shot leaves; today the event is at the skater's centre on the F press.
3. **Slow puck: `Where` at the puck**, and the system attached to the puck for its `duration_frames`, or a second event at its end. Today the frost is planted at the skater.
4. **Goal:** keep the red light off the ice (radius 350) as advised; nothing else needed. If the wave's lights run along the end boards under my dust, say if the two together are too much.

## Decisions for the operator
1. The stop's fan is dark slush, 4 % of the frame for a third of a second, on every hard stop of every figure. Keep, or halve it (2 % still passes).
2. The four abilities in four colours (grey-blue, ochre, slate blue, frost) on the lavender ice: accept the palette, or name the colours.
3. The goal as blue dust above the end: keep as the payoff, or ask for something that hugs the boards (that needs the rink's size in the event).
4. The check's ring is now white streaks with a tenth of the team colour: the team is no longer told by the ring. Accept, or raise the tint (salmon returns with it on the orange side).

## The next three evolutions, each with its proving number
1. **The fan sized by the stop's start speed and tuned down for bots**, once the figure lane passes `User.Strength`. Proof: 1.5 to 2.5 % of the frame at +6 for the human at 9 to 11 m/s, under 0.5 % for a bot 30 m away.
2. **The goal along the boards and for the full time.** Proof: at least 3 % of the frame from centre ice at every sampled frame from +20 to +100, and under 0.1 % of the ice changed outside the crease.
3. **The abilities in the game's own events with direction**, after the other device's binder line. Proof: 4 of 4 kinds named from an un-paused still without a caption, each at 2 % or more at +4.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with `.txt`)
`20261114-030208-B-b42-brake-fan.png`, `20261114-030208-B-b42-ability-kinds.png`, `20261114-030209-B-b42-chips-streaks.png`, `20261114-030209-B-b42-goal.png`.

## Ledger rows (full rows, the ledger's 11 columns; replace the eight existing rows)
```
| Game/Plugins/JamAssets/Content/VFX/NS_brake.uasset | vfx | 0.53 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | the hard stop's fan: one burst 0.04 s after activation of 10 to 18 shadowed-snow puffs and 22 to 40 soft blue-grey slush streaks 16 x 60-85 cm thrown ahead of the skates along the owner's travel, sized by the owner's speed clamped to 14 m/s and scaled down with the distance to the camera, nothing on a near-zero time step or outside 2 to 20 m/s, gone by +20 frames; played by BrakeFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B40, B42) (552130 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_ability.uasset | vfx | 1.55 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event ability on a use (Strength above 0.5), by User.Variant: 0 shotgun a grey-blue blast cone and white shards along User.Dir (48), 1 hip check an ochre chalk arc on the lunge side (22), 2 shield a slate-blue chalk ring with white shards holding about 50 frames (40), 3 slow puck a patch of pale frost at the skater (26); a grant (Strength 0) shows nothing; User.Dir read, default zero (lanes B40, B42) (1628462 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_check.uasset | vfx | 0.72 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event check: 3-frame clack star, a ring of 20 to 30 ice-white streaks flung out flat on the ice (was team-colour dots), low streak chips along User.Dir; strength max(0.5, User.Strength); peak 49 (lanes B3, B17, B30, B42) (751942 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.51 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event knockdown: low streak chips and 3 to 5 small shin-high puffs along User.Dir, gone by about +10 frames; peak 23 (lanes B3, B17, B18, B30, B42) (532261 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.94 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event board: Variant 0 low puck chips as streaks; Variant 1 a skater's crash scaled by (Strength - 0.5) x 2: four streak chips and one small puff at 0.5, 26 streak chips along the board foot and a snow cloud at 1.0, leaving the face along User.Normal; no shake line; peak 39 (lanes B3, B17, B30, B33, B35, B42) (990399 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.27 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event lift as a cross-check: Variant 0 three pale-hot stars 20-26 cm and five dark brown-red spokes 7 x 24 cm at 0.72 m (below the number on the back), shrinking and fading over 10 to 12 frames, eight splinters gone by +6 frames, a small snow kick at the skates, peak 27; Variant 1 two tiny chips; User.Dir read, default zero (lanes B3b, B17, B33, B35, B37, B40, B42) (1334643 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_slide_trail.uasset | vfx | 0.48 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | slide trail while a knocked-down figure slides: 8 small soft blue-grey puffs every 0.05 s and one velocity-aligned soft streak quad every 0.04 s fading over 1.5 s, about 100 live at full slide speed; speed capped at 8 m/s and quad length at 90 cm, nothing spawned on a near-zero time step, above 12 m/s, on a 30 cm jump or in the first 0.1 s; played by SlideFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B30, B32, B33, B35, B42) (498910 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.72 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | event goal: chalk dust in the scoring team's colour (User.Colour, lightened) thrown from the net out along the end boards both ways and up, rising and falling over 108 frames, and a cream fountain of streaks over the net; Variant 1 (conceded) at 60 % of the count; peak 94; User.Variant added (lanes B3, B42) (751010 bytes) |
```
