# B40 report: the stop's spray, the abilities and the deke, the cross-check mark longer (device B)

Lane B40, device B, machine clock 23:45 to 00:12. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` eab08f23), port 8002, the only editor on the machine: two sessions one after the other (pids 31908 and 31264), each left through `ExitEditor` with no dirty package; no `UnrealEditor` process is left. No GPU hang. Saved, each with no play session open: `VFX/NS_brake`, `VFX/NS_ability`, `VFX/NS_deke` (all new), `VFX/NS_lift`, `Skaters/BP_SkaterVisual_TB1`, `Skaters/BP_SkaterVisual_TB1_P3`. No commit, no push, no build, no cook; nothing under device A's paths written. `r.Editor.Viewport.OverridePIEScreenPercentage` was 1, set to 0 for the stills, read back at 1; `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` back to 0 / 0 / empty. Backups of the two Blueprints and `NS_lift` as they were at lane start: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b40` (`spec40.mjs` the three systems, `lift11.mjs`, `addcomp40.py` and `hook40.mjs` the Blueprint change, `pre_*.dsl` / `post_*.dsl` the Blueprints' text before and after, `tk.py`, all frames in `shots/`).

Numbers are in sim frames (60 a second of game time).

## Verdicts
| Job | Verdict | Still |
|---|---|---|
| 1. `NS_brake`, the stop's spray | **Works, small.** The spray component was active on every stop commit I recorded in clay matches (10 of 10 in one 60 s match, 2 of them the human's; 12 of 12 in the run before it), with the figure at 2.1 to 8.7 m/s over the next four frames, inside the spray's gate. In the human's stop: white shavings at the skates at +4 to +9, nothing at +14 and +40. It reads as a handful of flecks at game size, not yet as a fan. The blue-grey snow puffs were not made out. **Coverage of the figure: not measured**; by eye the flecks lie on the ice beside the boots and none is over the figure | `b40-brake-strip` |
| 2a. `NS_ability` by kind | **Built and separated by `Variant`; shown only by my own spawns.** Shotgun and hip check read at +2 and fade by +8 to +10; the shield's dust ring reads from +3 to +26 and is gone by +36; the slow puck's puff is faint. A grant shows nothing. **The game's own `ability` event is unproven: it never fired** | `b40-ability-kinds` |
| 2b. `NS_deke` | **Built; faint.** A small flick of shavings at +2, gone by +5. **The game's own `deke` event is unproven** | `b40-deke` |
| 3. The cross-check mark longer | **Partly**: full at +2 and +6, a faint remnant at +10, gone at +12 (was gone after 3). It shrinks by a third as it fades, so it does not sit on the trunk at full size. Shown by my own spawn on a standing figure, not in a real cross-check | `b40-crosscheck-mark` |
| 4. Chips as short streaks | **Not done** (the clock) | |

## 1. `NS_brake` and the two Blueprints
- **How it is spawned.** A Niagara component `BrakeFx` (asset `NS_brake`, auto-activate off) on the root of both figure Blueprints, and **one node after each `MS_brake` node**: `ActorComponent.Activate (BrakeFx, reset)`: once where `StopTick` commits the hard stop, once on the `SetBraking` pose path. The Blueprints' text differs from before by exactly those two lines each; both compile `BS_UP_TO_DATE`. Sizes 5,856,029 -> **5,866,902** and 6,065,474 -> **6,076,347** bytes.
- **What the system does.** One burst 0.04 s after activation (the owner's velocity is known by then): `18 + 22 S` ice shavings 4 x 12-18 cm and `6 + 6 S` small blue-grey snow puffs, thrown low along the owner's travel from the skates, gravity -1500, gone within about 16 frames of the burst. `S = saturate((speed - 3 m/s) / 7 m/s)`.
- **The velocity rule.** The speed is `min(length(Engine.Owner.Velocity), 14 m/s)`; nothing spawns on a time step under 0.5 ms, below 2 m/s or above 20 m/s.
- **Sized by the speed at the commit, not at the start of the stop.** The Blueprint hands the system nothing, and by the commit the figure has already lost 3.5 m/s or more. The human's stops committed at 5.8 to 6.6 m/s (v0 9.8 to 10.9), which gives `S` about 0.4.
- **A fault I made and removed.** My first build sprayed for a window of the system's age; a frame 26 frames after a commit still showed fresh flecks, so that window did not close as written. I rebuilt it as a single burst per activation, which cannot repeat. The 12-of-12 run was on the first build, the 10-of-10 run on the single-burst one.

## 2. `NS_ability` and `NS_deke`
What reaches the system today (read in `UJamEffectsBinder` and `JamSim.cpp`): `ability` is spawned at the **skater's position**, Strength 1 on a use and 0 on a grant, Variant 0 for every kind, no direction; `deke` at the **puck**, Strength 0, Variant 0, no direction. So until device A sends the kind, every use plays the shotgun burst.

| Kind (`Variant`) | What plays on a use | Count | Gone by |
|---|---|---:|---|
| shotgun (0) | white shavings 5 x 20 cm and blue-grey puffs bursting low at the skates; from 60 cm ahead along `User.Dir` when a direction is sent | 18 + 6 | +16 |
| hip check (1) | chalk streaks 5 x 22 cm on a ring at hip height (0.9 m), shifted to the lunge side when a direction is sent | 12 | +13 |
| shield (2) | a ring of soft dust puffs 14-22 cm at 55 cm round the skates, drifting out slowly; grey on the clay ice, irregular, no colour, no flat line | 24 | +33 |
| slow puck (3) | a gritty puff of small dust at the skater | 22 | +30 |
| grant (Strength 0) | nothing | 0 | |

`NS_deke`: 8 shavings 4 x 12 cm and 3 small puffs at the puck, gone by +12.

The slow puck is the puff at the start, not a patch dragging behind the puck: the event is one-shot, at the skater, with no duration.

## 3. `NS_lift`
Stars and spokes: life 3 frames -> **10 to 12 frames**; alpha `1 - age^3`; the stars shrink to 65 %, the spokes to 65 % wide and 80 % long. Counts unchanged (27).

## Particle counts (from spawn counts; none measured live)
| System | Peak |
|---|---:|
| `NS_brake` per stop | 24 to 52 |
| `NS_ability` shotgun / hip check / shield / slow puck / grant | 24 / 12 / 24 / 22 / 0 |
| `NS_deke` | 11 |
| `NS_lift` clean | 27 (unchanged) |
With ten figures all stopping in the same quarter second: about 500.

## Not verified
- The game's own `ability` and `deke` events: neither fired. Every ability and deke frame is my own spawn.
- Coverage of the figure by the brake spray as a number.
- The `SetBraking` path: device A does not call it yet, so only the `StopTick` commit was seen.
- The brake spray at 14 m/s (the fastest commit I saw was 8.7 m/s), on night, and from a figure skating backwards.
- The brake's snow puffs (not made out in any frame).
- The ability kinds with a direction in a frame that shows them (the figure hid the shotgun's forward burst from the game camera), and all of them on night.
- The longer cross-check mark in a real cross-check and with two figures in contact (does it cover the attacker's face at +6?).
- The un-frozen ability frames are from an earlier build and have a bot's bump effects in them; the clean evidence is frozen.
- Cook: not run. Three assets are new and two Blueprints changed, so this lane needs the cook check before the push.

## For the other device
1. **`ability`: `Variant` = the kind** (shotgun 0, hip check 1, shield 2, slow puck 3). Already asked; until then every use shows the shotgun burst.
2. **`ability`: `User.Dir`** (unit, world x/y) from the binder as for `check`: the shot's direction for the shotgun, the lunge direction for the hip check. Without it both are rings round the skater.
3. **`ability`, shotgun: `Where` = the blade or the puck**, not the skater's centre, on the frame the shot leaves (today the event is the F press, which may come before the shot).
4. **`ability`, slow puck: `Where` = the puck**, and either a second event when it ends or the system attached to the puck for `duration_frames` (120). Only then can a frost patch drag behind the puck.
5. **`ability`, shield:** nothing needed for the planted ring. If a cue is wanted for the whole 360 frames, send the end as an event.
6. **`deke`: the side** (the Kind is already `left` / `right`): as `Variant` 0 / 1 or as `User.Dir`, so the flick goes to the side the puck leaves.
7. **`SetBraking(bOn, Decel)`:** nothing needed for the spray. Say when it is called, and I will check that path.

## Decisions for the operator
1. The brake spray is deliberately small. Twice as dense still stays under the boots; say if the stop should read more.
2. The shield is a ring of grey dust for half a second. If the operator wants the shield to be visible while it lasts (6 s), that is a different, held effect and needs the end event.
3. The cross-check mark now lasts 10 frames on the victim's trunk at shrinking size. Keep, or go back to 6.
4. Job 4 (chips as short streaks) is still open.

## The next three evolutions, each with its proving number
1. **The brake spray sized by the stop's start speed**: one more Blueprint node sets the system's strength from `StopV0`. Proof: the spray's pixel area at v0 14 m/s is at least twice that at v0 7 m/s in un-frozen frames.
2. **Direction for the shotgun, the hip check and the deke** once device A sends it. Proof: at least 80 % of the chips lie within 30 degrees of the sent direction in a frame at +2.
3. **Chips as short streaks** in `NS_check`, `NS_board` and `NS_knockdown`. Proof: no chip with a length-to-width ratio under 2 in a hit frame on clay.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with `.txt`)
`20261114-001015-B-b40-brake-strip.png` (un-frozen), `20261114-001015-B-b40-ability-kinds.png` (frozen, one un-frozen row), `20261114-001015-B-b40-deke.png` (frozen), `20261114-001016-B-b40-crosscheck-mark.png` (frozen, one un-frozen row).

## Ledger rows (full rows; replace the rows of the two Blueprints and `NS_lift`, add the three new ones)
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 5.60 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40) (5866902 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 5.79 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40) (6076347 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.27 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event lift as a cross-check: Variant 0 three pale-hot stars 26-34 cm and five dark brown-red spokes 7 x 24 cm at 1.0 m, shrinking and fading over 10 to 12 frames, eight splinters gone by +6 frames, a small snow kick at the skates, peak 27; Variant 1 two tiny chips; User.Dir read, default zero (lanes B3b, B17, B33, B35, B37, B40) (1334195 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_brake.uasset | vfx | 0.52 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | the hard stop's spray: one burst 0.04 s after activation of 18 to 40 ice shavings and 6 to 12 small snow puffs thrown low along the owner's travel, sized by the owner's speed clamped to 14 m/s, nothing on a near-zero time step or outside 2 to 20 m/s, gone within about 16 frames; played by BrakeFx on BP_SkaterVisual_TB1 and _TB1_P3 (lane B40) (545322 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_ability.uasset | vfx | 1.13 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event ability on a use (Strength above 0.5), by User.Variant: 0 shotgun a burst of 18 shavings and 6 puffs at the skates (from the blade along User.Dir when sent), 1 hip check 12 chalk streaks at hip height, 2 shield a ring of 24 chalk-dust puffs at the skates for half a second, 3 slow puck a gritty puff of 22; a grant (Strength 0) shows nothing; User.Dir read, default zero (lane B40) (1188559 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_deke.uasset | vfx | 0.50 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event deke: a flick of 8 shavings and 3 small puffs at the puck, gone within 12 frames (lane B40) (520489 bytes) |
```
