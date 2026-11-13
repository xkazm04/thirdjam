# B37 report: the puck trail, the night streak's pale edge, the lift's spokes (device B)

Lane B37, device B, machine clock 18:15 to 18:40. Tree `C:\Users\kazda\jam\wt\b-vfx`, port 8000, the only editor on the machine throughout. I ran three editor sessions one after another (pids 41436, 5036, 26044), each left through `ExitEditor` with no dirty package; no `UnrealEditor` process is left. Saved, each with no play session open: `VFX/NS_puck_trail`, `VFX/M_Fx_PuckTrail` (new), `VFX/M_Fx_SlideSoft`, `VFX/NS_lift`. No Blueprint saved, no git writes, no build, no cook. `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` back to 0 / 0 / empty; `r.AntiAliasingMethod` and `r.MotionBlurQuality`, which I switched for two test runs, read back at 4 and 4. Backups of the assets as they were at lane start: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b37` (`trail1.mjs`, `trail2.mjs` the trail edit, `mktrail.py` the material, `tk.py` with the un-frozen watchers `watch_shot` and `watch_fx`, `shots.sh`, `ghost.sh`, `item4.sh`, `rec_*.txt`, all frames in `shots/`).

## Verdicts
| Item | Verdict | Still |
|---|---|---|
| 1. The puck trail | **The trail's own fault is fixed; the ladder in the two stills you sent is NOT fixed, because `NS_puck_trail` does not draw it.** The new trail shows no rungs in un-frozen frames (three fast-puck moments on clay, three on night). The ladder of pale crescents behind the ring is still there in play with the new trail, with the trail hidden and stopped, with temporal anti-aliasing off and with motion blur at 0. What draws it: not found | `b37-puck-trail-clay-ab`, `b37-puck-trail-night-ab`, `b37-puck-ladder-tests` |
| 1b. The trail's look | **Clay PASS by eye**: a faint, soft blue-grey curve behind a fast puck, nothing behind a carried one. **Night readable**: one thin soft yellow-green line. A clean hard shot down open ice was not captured | same |
| 2. Night streak without the pale edge | **PASS**: the hard slide's start at +10 and +20 is one soft dark smudge, no stripes; the band at +40 (frozen frames) | `b37-streak-night-start` |
| 3. `NS_lift` spokes 7 cm | **Done and saved; not compared side by side** | |
| 4. Un-frozen bursts | **Partly**: two real knockdowns (no stray line; neither slid far enough to leave a clear streak), two real cross-checks (splinters and snow kick seen at +1 tick, the 3-frame mark not made out: the victim was turned away), one real hard board crash, Strength 1.00, at the far boards (the grey cloud shows at the skates at +10 ticks) | `b37-unfrozen-knockdown`, `-crosscheck`, `-board` |

## 1. The cause, as found
**In the asset** (`NS_puck_trail`, emitter `Trail`, read back through the tools before I changed it): spawn rate 60 per second by time; each particle's alpha and ribbon width set once, in `InitializeParticle`, from `max(User.Speed, length(Engine.Owner.Velocity) / 100)`, unsmoothed, with the 12 / 16 / 45 m/s thresholds; life 0.2 s; alpha then only `1 - age`. **In play** (recorded per tick on the trail component): the speed it reads jumps from tick to tick, for example 25, 28, 2.7, 13.7, 3.2, 2.5, 28 m/s on seven consecutive ticks while the puck is knocked about. So neighbouring particles are born visible and invisible, and the ribbon between them shows as short pale arcs next to the ring. That is the thin arcs beside the ring in the first row of `b37-puck-trail-clay-ab` (before), gone in the same moment after.

**What the trail does not draw.** Two other things are in the stills you sent:
- **The ladder of pale crescents trailing the ring of a fast puck.** Same moment of a real match in five un-frozen runs (`b37-puck-ladder-tests`): present with the new trail, with the trail component hidden and deactivated, with `r.AntiAliasingMethod 0`, with `r.MotionBlurQuality 0`. So it is not the trail, not temporal anti-aliasing ghosting, and not motion blur (that last run trusts the console variable; I did not confirm the play session honoured it). Not tested: the reflection or lighting history of the bright ring on the glossy clay ice, and the puck disc's own halo and beacon materials (`Look/`, `Props/`: not mine).
- **The large pale ovals on the ice near a scrum.** They stay with the new trail and with temporal anti-aliasing off. By their place and timing they are the puffs of the hit effects, which the tier's puff material draws on clay as pale rimmed discs. Not proven by hiding them.

**What I changed in the trail:**
- A new emitter value `Emitter.TrailSpeed`: the raw speed eased over 0.1 s; a time step under 0.5 ms or a raw speed above 45 m/s (a faceoff teleport) leaves it as it was.
- Visibility of the whole ribbon, every frame, from that value: `saturate((TrailSpeed - 12) / 4) x (1 - age)` in the particle update. Nothing is decided per particle at spawn any more (spawn alpha 1, width a constant 9 cm).
- Spawn 60 per second only while `TrailSpeed` is above 10 m/s and the time step is not near zero (still by time: the builder has no spawn-by-distance call). Life 0.2 -> 0.15 s.
- Material `MI_Fx_Trail` -> new **`M_Fx_PuckTrail`**: translucent, unlit, soft across the ribbon's width (`Soft` 1.2). On clay a fixed blue-grey `ClayCol` (0.40, 0.45, 0.58) at `ClayOpacity` 0.35; off clay the particle colour x `NightBright` 2.5 at `NightOpacity` 0.8. `MI_Fx_Trail` is no longer used by anything I know of; left in place.

## 2 and 3
- `M_Fx_SlideSoft`: `EdgeAmt` 0.85 -> 0 (the pale edge off; the other parameters stay, so it can be turned back on).
- `NS_lift`, emitter `Scar`: spokes 5 x 24 -> **7 x 24 cm**. Nothing else.

## Particle counts (from rates and lifetimes; none measured live)
| System | Before | After |
|---|---:|---:|
| `NS_puck_trail` | 12 live at all times | **9 live while the eased speed is above 10 m/s, else 0** |
| `NS_lift` clean / miss | 27 / 2 | unchanged |
| `NS_slide_trail` | about 210 at full slide speed | unchanged |

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with `.txt`)
`20261113-183518-B-b37-puck-trail-clay-ab.jpg`, `20261113-183518-B-b37-puck-trail-night-ab.jpg`, `20261113-183519-B-b37-streak-night-start.jpg`, `20261113-183519-B-b37-unfrozen-knockdown.jpg`, `20261113-183519-B-b37-unfrozen-crosscheck.jpg`, `20261113-183519-B-b37-unfrozen-board.jpg`, and one you did not ask for: `20261113-183803-B-b37-puck-ladder-tests.jpg`. Un-frozen frames are the game's own `HighResShot` requested from a tick callback while the match runs (no HUD in them); only the streak still is from frozen frames.

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Before -> after |
|---|---|
| `NS_puck_trail` | 316,523 -> 335,096 |
| `M_Fx_PuckTrail` | new, 16,085 |
| `M_Fx_SlideSoft` | 19,882 -> 19,658 |
| `NS_lift` | 1,414,652 -> 1,331,105 |
`NS_lift` was first saved while its recompile was still running; I saved it and the trail again at 18:37 with the compile state up to date and no errors.

Ledger rows (replace the rows of `NS_puck_trail`, `M_Fx_SlideSoft`, `NS_lift`; add `M_Fx_PuckTrail`):
```
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.32 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-13 | B | y | puck ribbon trail: the whole ribbon's visibility from a speed eased over 0.1 s (max(User.Speed, owner velocity), held on a near-zero time step or above 45 m/s): invisible <=12 m/s, full >=16 m/s; spawns only above 10 m/s; width 9 cm, life 0.15 s; material M_Fx_PuckTrail; User.Speed (m/s, default 0), User.Colour (lanes B3, B18, B37) (335096 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckTrail.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | translucent unlit soft ribbon for the puck trail: soft across its width; on clay a faint blue-grey at 0.35 opacity, off clay the particle colour brightened (MPC_Look.Clay) (lane B37) (16085 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_SlideSoft.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | translucent unlit soft particle, depth-faded into the ice; on clay the particle colour with a soft round falloff; off clay the dark scrape is a mid blue-grey band with doubled opacity (pale edge off, EdgeAmt 0), the snow darkened (MPC_Look.Clay) (lanes B32, B33, B37) (19658 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.27 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event lift as a cross-check: Variant 0 three pale-hot stars 26-34 cm and five dark brown-red spokes 7 x 24 cm at 1.0 m for 3 frames, eight splinters gone by +6 frames, a small snow kick at the skates, peak 27; Variant 1 two tiny chips; User.Dir read, default zero (lanes B3b, B17, B33, B35, B37) (1331105 bytes) |
```

## What I could not verify
- What draws the crescent ladder behind a fast puck. Three causes are ruled out, none is confirmed.
- That `r.MotionBlurQuality 0` took effect in the play session.
- That the pale ovals are the hit effects' puffs (by eye only).
- The trail behind a clean hard shot in open ice: the six moments I caught are pucks above 16 m/s for two ticks, mostly loose pucks knocked about at the boards. A night "before": not captured.
- The trail on tiers other than clay and night (they get the night look now; B18 had given broadcast a dark streak).
- The trail's colour on night is whatever `User.Colour` defaults to (yellow-green); the other device sets nothing on it.
- The 7 cm spokes against the 5 cm ones; the cross-check mark itself in un-frozen play.
- A real slide long enough to show the streak un-frozen; flicker in motion; cook (not run).

## Decisions for you
1. **The ladder needs an owner.** It is not in `VFX/NS_puck_trail`. My next step would be one run each with the puck disc's beacon and halo hidden and with reflections off on the clay ice; that touches `Look/` and `Props/`, so it is the look lane's or yours to assign.
2. The pale rimmed ovals on clay: if they are the hit effects' puffs as I think, B can move those puffs from `MI_Fx_Puff` to the soft material in one short lane (they would stop reading as rings). Say so.
3. The clay trail is deliberately faint (opacity 0.35). Raise `ClayOpacity` if the operator wants a shot to read more, or set it to 0 for no trail on clay at all.
4. `MI_Fx_Trail` is now unreferenced by the trail: delete it in a lane that runs the cook check, or leave it.
5. One practical note for the next lane: port 8000 stays in `TIME_WAIT` for about two minutes after `ExitEditor`; `editor.ps1` refuses to start until it clears.
