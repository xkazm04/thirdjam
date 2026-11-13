# SK19 report: a gap between reactions, the early release, the puck trail's ladder, the refused pins (device B)

**The editor crashed once in this lane.** At 18:00 the GPU hung (`DXGI_ERROR_DEVICE_HUNG`) in my first editor (pid 13024), after the regular match was recorded and during a test-figure session. Both Blueprints had been saved at 17:53 with no play session open, so nothing was lost. I stopped the hung process with `editor.ps1 -Stop` (it was mine), started a second editor (pid 37464) for the two sessions that had not run, and that one left through `ExitEditor`; its process is gone.

Lane SK19, device B, machine clock 17:50 to 18:13. Tree `C:\Users\kazda\jam\wt\b-vfx` (`dev-b` b1910f2), port 8000. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`. Nothing under `VFX/`, `Audio/`, `Look/`, `Rigs/` touched. `/Game/Maps/Rink` not saved; no `_Scratch/`; the scratch cartridge deleted; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.hitstop` read back at their defaults before the exit. Port 8002 not touched. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk17` (`bp19.mjs` the change, `t19c.sh` the release test, `bk19/` the Blueprints at lane start) and `...\scratchpad\qa2` (`qa.sh` the match runner, `busy.mjs` the busiest-figure reader).

## Verdicts

| Item | Verdict |
|---|---|
| 1. The reactions at the boards | **PASS**: stick reactions played 141 before, 64 now in the same 104 s match; the busiest figure's busiest two seconds went from 17 take starts to 8 |
| 2. The early release after a foul | **PASS**: the figure is standing 6 frames after the release (it was 30), from every point of the chain; no one-frame snap |
| 3. The ladder behind the puck | **Found, not proven**: it is `NS_puck_trail`; the likely cause is its per-particle visibility from the owner's speed |
| 4. Pins 68 sent, 59 played | **Right as the rules stand**: all 9 are refused by a running check lunge; one side finding below |
| 5. Un-frozen bursts | **Not done** (the crash took the time) |

## The Blueprint changes as saved (both Blueprints, the same text)

1. **The gap.** One number, `RxGap` = 0.35 s, set in `ReceiveBeginPlay`. New variables `RxGap`, `LastRxT`, `RxKind`.
   - `PlayStickHit` (`Lift_Hit`, `Poke_Hit`): held back if the figure started a stick or standing reaction less than `RxGap` ago, or the same kind less than 0.45 s ago (so a take never restarts itself; 0.45 is a literal in this event). When it plays it stamps `LastRxT` and its kind.
   - `PlayHitReaction` (the standing hits): the same gap added to its old 0.6 s rule; stamps `LastRxT`.
   - `PlayLift` (the attacker's shove): not again within `RxGap` on the same figure (log line `SK6RX lift skipped`). The sim's own 30-frame cooldown already keeps lifts further apart than that, so it never fired.
   - Knockdown, check lunge and board pin: untouched.
2. **The early release.** In `KnockDown`, on the release:
   - the get-up is running with more than 6 frames left: its play rate is raised so it ends in 6 frames (log line `SK19REL fast rem_f`);
   - the get-up has not started (the figure is still falling or lying): the last third of the get-up plays at twice the speed, 6 frames, blended in over the event's usual 0.12 s;
   - in both cases the mesh turns back to the pawn's facing over 8 frames instead of 54;
   - 6 frames or fewer left: nothing changes; a trip (no scheduled get-up) is unchanged.

## 1. Reactions, before and after (regular cut on clay, scripted human, the same seeded match, 104 s of play)

| Kind | Sent by the sim | Before: played / held or blocked | After: played / held or blocked |
|---|---|---|---|
| `Lift_Hit` (victim of a cross-check) | 75 | 75 / 0 | **47 / 28** |
| `Poke_Hit` (victim of a poke) | 66 | 66 / 0 | **17 / 49** |
| standing hit | 78 | 43 / 35 (30 by the 0.6 s rule, 5 by priority) | 43 / 35 (the same) |
| `Lift` (the attacker) | 79 | 72 / 7 (by its own check lunge) | 72 / 7; 0 by the gap |
| board hit | 49 | 49 / 0 | 49 / 0 |
| board pin | 68 | 59 / 9 | 59 / 9 |
| knockdown | 15 | 15, get-up end minus release 0.00 on all | 15, 0.00 on all |

- Stick reactions played: **141 before, 64 now**.
- Takes started by the human's figure in the match (every kind): 174 before, 105 now.

**The busiest figure (the human) in its busiest two seconds** (frame: take started):

| | Sequence |
|---|---|
| Before, 17 starts | 0 `Lift_Hit`, 5 `Poke_Hit`, 8 `Board_Pin`, 19 `Poke_Hit`, 30 `Lift_Hit`, 33 `Poke_Hit`, 47 `Poke_Hit`, 55 `Lift_Hit`, 59 `Board_Pin`, 60 `Poke_Hit`, 74 `Poke_Hit`, 85 `Lift_Hit`, 88 `Poke_Hit`, 102 `Poke_Hit`, 114 `Lift_Hit`, 116 `Poke_Hit`, 117 `Board_Pin` |
| After, 8 starts | 0 `Lift_Hit`, 8 `Board_Pin`, 30 `Lift_Hit`, 59 `Board_Pin`, 60 `Poke_Hit`, 85 `Lift_Hit`, 114 `Lift_Hit`, 117 `Board_Pin` |

- The stick reactions are now 25 to 30 frames apart: distinct events.
- **What is left is the pin.** It still cuts a stick reaction 3 to 8 frames in (frames 8 and 117), and a stick reaction cuts the pin one frame after it starts (59, 60). You said the pin keeps its priority; see the decisions.
- Judged from the recorder and the log; I did not look at this match in motion.

## 2. The early release

**Real match** (drill 6 on the regular cut on clay; the human's check from behind draws a foul and the sim releases the victim after 60 frames):

| | Get-up when released | Get-up over after the release |
|---|---|---|
| QA2 (before) | 5 frames in | 30 frames |
| SK19, two fouls (`GetUp_Front`, `GetUp_Side_Left`) | 5 frames in | 3 frames to the blend-out, standing at 6 |

**Test figures** (clay, a 90-frame knockdown released at a set frame by the game clock; pelvis height each frame after the release, standing is 74.7 cm):

| Released at | In | Pelvis at +0, +1, +2, +3, +4, +5, +6 (cm) | At 90 % of standing after |
|---|---|---|---|
| frame 10 | the fall | 21, 24, 29, 39, 49, 62, 72 | 6 frames |
| frame 40 | lying | 19, 21, 27, 38, 49, 62, 72 | 6 frames |
| frame 60 | the get-up, 6 frames in | 32, 47, 50, 54, 64, 73, 75 | 5 frames |
| frame 72 | the get-up, 18 frames in | 50, 55, 58, 60, 66, 74, 75 | 4 frames |
| frame 84 | the get-up, 30 frames in (left alone) | 61, 63, 66, 68, 73, 75, 75 | 3 frames |

- The largest rise in one frame is 13 to 14 cm (from lying, and in the first frame of the sped-up get-up); the rest is spread over the 6 frames. It is a fast rise, not a snap.
- From lying this is a figure springing up in a tenth of a second. It is what was asked; whether it looks right in motion I cannot say from numbers.
- The normal chain is unchanged: 15 of 15 knockdowns in the regular match and 8 of 8 normal ones in the drill end on the release (0.00).

## 3. The ladder of pale arcs behind a fast loose puck

- **What draws it: `/JamAssets/VFX/NS_puck_trail`, emitter `Trail`, a ribbon.** The other device's `AJamPuck` spawns it attached to the puck's mesh, 0.5 cm under it, at absolute scale (`JamPawns.cpp`, the puck kit). The glowing ring is a separate thing: the disc `SM_PuckDisc` under the puck; the thin white ring round a carrier is the other device's HUD.
- **What its own spec says** (lane B18, `status/B18-report.md`, and that lane's edit file): each ribbon particle gets its alpha and its width once, at spawn, from the owner's instantaneous speed `length(Engine.Owner.Velocity)`: alpha 0 at 12 m/s or less, full from 16, and 0 again above 45 m/s (a guard against teleports); width 3 to 10 cm; life 0.2 s; material `MI_Fx_Trail` (on clay a dark olive streak with a pale rim).
- **Probable cause of the ladder, not proven:** the owner's speed is read per frame, and a loose puck's rendered position does not advance evenly per frame (the sim steps at its own rate, the frame rate was low on this machine, and the puck bounces). Particles born on a frame that reads over 45 or under 12 m/s are invisible and thin, their neighbours are visible and wide, so the ribbon shows as separate rungs; the pale rim of the clay material makes each rung an arc.
- **Not read:** the emitter's spawn rule (per frame, by rate, or by distance) and the ribbon's UV setting. I did not find the read tool for Niagara stacks in the time I had.
- **Fixes for the effects lane to try:** take the speed from a smoothed value (or clamp the owner's velocity as B35 did for the slide streak) instead of the instantaneous one; or set alpha in particle update from a system-level smoothed speed, so the whole ribbon fades together; spawn by distance so segments are even.
- Still: `sk19-puck-trail-ladder` (the same un-frozen frame as QA2's; I did not catch a second one).

## 4. The 9 refused pins

- All 9 are bots. In each case the bot's cross-check (`Lift`) had been replaced, 4 to 5 frames after it started, by the bot's own check lunge, and the pin arrived in the lunge's first frames. A running check outranks the pin (the SK12 rule), so the pin is refused. **That is right as the rules stand.**
- A pin over a `Lift` alone does play (checked on a test figure).
- **Side finding:** the bots press check 4 to 5 frames after a cross-check, so in these cases the shove is seen for 4 frames before the lunge takes over. That is the other device's input pattern; on B's side the lunge outranks the shove by your decision.

## 5. Un-frozen bursts

Not done. The match runner captures one un-frozen frame every 2 to 4 s; a burst needs a per-frame capture inside the play session, and the crash took the time for building it. The contact sheets of this lane's regular match are in the scratch folder (`qa2\img\sheet_reg2_*.png`); I did not go through them for the streak, the mark or a crash cloud.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| File | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 3,738,157 | 3,898,568 |
| `BP_SkaterVisual_TB1_P3` | 3,942,681 | 4,102,906 |

Both saved at 17:53 with no play session open, both `BS_UP_TO_DATE`. 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs are up to date, no dirty package at the exit.

## Stills (`C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`)

- `20261113-180852-B-sk19-puck-trail-ladder.jpg` (with `.txt`)
- `sk19-unfrozen-knockdown`, `-crosscheck`, `-board`: not made.

## Ledger rows

Replace the two Blueprint rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.72 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lane SK19): a gap of RxGap 0.35 s between a figure's stick and standing reactions and none restarts itself within 0.45 s, an early release finishes the get-up in 6 frames (3898568 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 3.91 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16) (4102906 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- Everything in motion: recorder, log and per-frame numbers only. In particular the spring-up from lying after an early release, and whether 25 to 30 frames between stick reactions reads as calm enough.
- The night figure: the same Blueprint text, not run in this lane.
- A trip's release (unchanged by construction), and an early release while a hit-stop holds the figure.
- The gap on `PlayLift`: it never fired, because the sim's cooldown is longer.
- The cause of the trail's ladder (item 3): read from the spec, not from the asset.
- Item 5.
- The first editor's last session (releases early and late in the get-up, timed by hand) gave no usable data; the timed test on the second editor replaced it.

## Decisions for you

1. **The pin against the stick reactions.** The pin is now the main thing chopping reactions at the boards (59 in 104 s; it cuts a reaction 3 to 8 frames in and is cut one frame after it starts). Options: (a) the pin waits out a stick or standing reaction younger than `RxGap`, and a stick reaction does not replace a pin younger than `RxGap`; (b) leave it. I would take (a): two conditions in `SetBoardPin` and one in `PlayStickHit`.
2. **Which reaction wins inside the gap.** Today the first one plays and later ones are held, so `Poke_Hit` plays 17 of 66 while `Lift_Hit` plays 47 of 75 (the cross-check usually comes first). If the poke should be seen more, hold a `Lift_Hit` behind a recent `Poke_Hit` for a shorter time; I would leave it.
3. **`RxGap` 0.35 s** is one line in `ReceiveBeginPlay`; a longer gap holds back more. I ran 0.35 only.
4. **The spring-up after an early release**: keep 6 frames, or stretch to 10 if it looks abrupt in motion (one number in two places in `KnockDown`).

## Notes for the other device

1. A foul whistle still releases a knocked-down figure 30 frames early; B's figure now stands in 6 frames instead of 30, but from lying that is a spring-up. Keeping the figure down for its frames would look better.
2. The bots press check 4 to 5 frames after a cross-check; the check lunge then replaces the shove. If the shove should be seen, the bot wants a pause after a lift.
3. 79 lifts and 68 pokes in 104 s is unchanged; B now shows 64 of the 141 stick reactions. Half a second of immunity for the carrier would make the held-back ones unnecessary.
4. `NS_puck_trail` reads the puck's rendered speed frame by frame; if the puck's rendered position can be advanced evenly between sim steps, the trail (and anything else of B's that reads the owner's velocity) is steadier.
