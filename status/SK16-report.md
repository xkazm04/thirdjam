# SK16 report: walls for every hit direction, rinks of any size, the contact shade, night, five a side, banner frames (device B)

Lane SK16, device B, machine clock 14:30 to 15:08. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` dda5a63), editor on port 8002 (pid 6528), left through `ExitEditor`; the process is gone. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`, `M_SkaterContact` (it has no instances). `/Game/Maps/Rink` not saved; no `_Scratch/`; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.hitstop` read back at their defaults. Port 8000 (the sound lane) not touched. The five-a-side cartridge was a scratch file outside the repository. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk13` (`bp16.mjs` the Blueprint change, `mat16.py` the material, `t3.sh` / `t4.sh` / `t5.sh` the test figures, `chain16.sh` the matches, `bk16/` the three assets as they were at lane start).

## Verdicts

| Item | Verdict |
|---|---|
| 1. Hits from the front or behind toward a wall | **PASS on test figures** (side boards from behind and from the front, end boards, corner): a side take along the wall, nothing through it. No such hit happened in a real match; the side rule fired once in a real match on clay and once on night |
| 2. Rink size | **PASS**: Pond reads 3000 / 1500 / 300 and the wide arena 7000 / 3500 / 583; the rule fires at their boards and corners and not in open ice; the corner radius comes from the ice material instance |
| 3. Contact shade | **PASS**: 16 to 17 darker (median), 22 to 23 at the 95th percentile, at mid ice, on a wing and in a corner zone; soft edge; standing figures unchanged |
| 4. The night figure | **PASS**: get-up end minus release 0.00 (8 of 8 untouched knockdowns), pelvis at 90 % minus release -3 to -1 on 10 of 11, the frozen frame has the lunge in, the wall case stays inside. Slide trail: clear on clay, faint on night |
| 5. Five a side | **Look passes** with ten figures; two knockdowns at once did not happen in the match I ran |
| 6. Banner stills | **Filed**: three candidates at 1920 x 1080 from real knockdowns on clay, HUD not in the frames |
| 7. The under-ice frames | **Found**: both are in the side get-ups, take frame 3 (below) |

## What was saved

**Both Blueprints**
1. `KnockDown` calls `WallFix` for every knockdown except a trip (it was side knockdowns only).
2. `WallFix`:
   - It reads the rink again on every knockdown (`FindRink`), then tests the point 1.5 m along the knock direction.
   - Side hit toward a wall: as in SK15 (`Knockdown_Back` along the facing, else `Fall` along the facing, else `Knockdown_Back` toward centre ice).
   - **New, hit from the front or from behind toward a wall:** a side take with the mesh kept on the figure's facing, so the body goes down along the wall: `Knockdown_Left` if the point 1.5 m to the figure's left-hand side (facing + 90 degrees) is inside the boards, else `Knockdown_Right` if the other side is, else `Knockdown_Back` toward centre ice. The get-up follows the take (`GetUp_Side_Left` / `_Right`).
   - Log line `SK16WALL kind <n>` when it fires.
3. `FindRink` / `WallAt`: half-length and half-width from the rink actor's `Ice` bounds as before; **the corner radius from the ice's own material instance** (`CornerRadiusCm`, which device A's `RinkActor` sets per rink), with half-width / 6 as the fallback if the parameter is missing. New variable `RinkR`.

**Clay Blueprint only**
4. While down: `ContactShadow` 2.2 x 0.95 m, `Darkest` 0.75, `Flat` 0.7 (it was 1.9 x 1.0 m, 0.25). At the release: 0.5 x 0.7 m, `Darkest` 0.45, `Flat` 0, as before.

**`M_SkaterContact`**
5. One new scalar, `Flat` (default 0): the dark disc keeps full strength out to that fraction of its radius, then fades with the same smooth step. `Flat` 0 is exactly the old material (measured: the old setting gives the old numbers), so standing figures look as they did. Still unlit, Modulate.
6. **A correction to SK15:** I wrote there that the plane "has no visible effect". That was wrong in its wording: the plane did darken the ice, but only in a small spot near the legs (the old falloff is nearly gone at 60 % of the radius), and I read a whole-frame mean as "nothing". The fix asked for is the same.

## 1. Hits from the front or behind toward a wall

Clay test figures 0.6 m from the boards on the regular rink, knocked straight at the wall. Nearest joint centres to the wall while lying (cm inside):

| Case | Take played | Nearest joints | In the still |
|---|---|---|---|
| Side boards, from behind | `Knockdown_Left` along the wall | foot 24, hand 27 | everything inside |
| Side boards, from the front | `Knockdown_Left` along the wall | head 58, trunk 59 | everything inside |
| End boards, from behind (the SK15 fault) | `Knockdown_Left` along the wall | not read (the figure was off camera when I read the joints) | everything inside; one mitt at the kick plate |
| Corner, from behind into the corner | `Knockdown_Left` along the arc | not read (same) | everything inside; one mitt at the kick plate |
| Open ice, from behind (control) | plain `Knockdown_Back` | 10 m away | unchanged |

- Joint centres, not skin: a hand 15 to 25 cm from the wall has its mitt touching the kick plate. Nothing goes through.
- **Real matches:** no hit from the front or from behind landed within reach of a wall. The side case from SK14 and SK15 (sim frame 2932) fired the side rule again on clay and on night (`SK16WALL kind 1`, `Knockdown_Back` along the wall).
- "Before" in `sk16-walls` is SK15's end-boards still; I did not shoot a before for the side boards and the corner.

## 2. Rink size

| Rink | Read by the figure (half-length / half-width / corner, cm) | Side fall 0.6 m from the side boards | Side fall in open ice | Corner |
|---|---|---|---|---|
| regular 90 x 45, corner 3.75 | 4500 / 2250 / 375 | rule fires (SK15, and the real case below) | plain side take | along the arc |
| Pond 60 x 30, corner 3 | 3000 / 1500 / 300 | rule fires: `Knockdown_Back` along the wall, nearest joint 21 cm inside | plain `Knockdown_Right` | rule fires: `Knockdown_Back` along the arc, nearest joints 11 and 19 cm inside |
| wide 140 x 70, corner 5.83 | 7000 / 3500 / 583 | rule fires: `Knockdown_Back` along the wall, nearest joint 21 cm inside | plain `Knockdown_Right` at y = -21.9 m, where the regular rink has its boards | rule fires: `Knockdown_Back` along the arc, nearest joints 15 and 21 cm inside |

- The corner radius comes from the ice material instance, so Pond reads 300 where the half-width rule would have said 250, and the cage (corner 2 m) will read 200. I did not run the cage.
- The Pond figure 0.6 m from Pond's boards stands 8 m inside where the regular rink's boards are; the rule fired there, so the size is not a leftover from another rink.
- **Re-read on a rebuild:** by construction (the rink is read on every knockdown, nothing is kept). I tried to show it with `jam.livecut` from Pond to the wide arena in one match: the look changed but the rink's ice stayed Pond's size in that session, so the figure correctly kept reading 3000 / 1500 / 300. That is a note for device A, not a proof.
- Pond runs on the day tier; my clay test figure looks blown out there. That is the test figure on a tier it is not made for, not a fault of the rule.

## 3. Contact shade

Ice luma with the plane hidden minus with it shown, over the pixels it darkens (lying clay test figure, the SK15 cameras):

| Position | SK15 setting (old falloff, 1.9 x 1.0 m, 0.25) median / 95th / max | SK16 setting median / 95th / max | Pixels darkened, old to new |
|---|---|---|---|
| mid ice | 12 / 32 / 53 | 16 / 22 / 27 | 20,556 to 36,234 |
| wing | 12 / 36 / 65 | 17 / 22 / 26 | 21,089 to 36,997 |
| corner zone | 12 / 37 / 67 | 17 / 23 / 26 | 21,234 to 37,045 |

- Before, the shade was a small dark spot near the legs (peak 53 to 67) and nothing under the trunk and head. Now it is an even soft shape along the whole body, 16 to 17 darker, no peak above 27, no rim.
- Extent: the plane is 2.2 x 0.95 m under the pelvis and turns with the body; at full strength to 70 % of that, faded to nothing at its edge. Beside the trunk it reaches about 10 to 20 cm past the outline at fading strength (by eye from the difference image; not measured in cm).
- Standing figures: `Flat` 0 reproduces the old numbers exactly (12 / 36 / 65 on the wing with the old setting on the new material).
- Other settings tried: `Darkest` 0.88 gave 8 / 10 (too faint), 0.8 gave 14 / 18.

## 4. The night figure

One real match on `l2_night` with the final Blueprint and export18 (drill 6, hit-stop 3, the same seeded match as clay), 12 knockdowns by the human, all with the hit-stop:

| Measure | Night |
|---|---|
| Get-up end minus release (`SK6REL`), knockdowns my captures did not touch | 0.00 eight times |
| The same, knockdowns frozen for captures | 0.87, -0.12, -0.27, -0.27 |
| Pelvis at 90 % minus release (`JAMLOW`), 11 read | -1, -2, -2, -2, -3, -8, -3, -3, -3, -3, -3 |
| Lying, `Fall` (pelvis / head / hands / feet, cm) | 17.7 / 22.2 / 14.2 / 15.3 |
| Lying, `Knockdown_Back` | 17.5 / 21.4 / 15.5 / 22.8 |
| Lying, `Knockdown_Left` | 27.5 / 24.6 / 13.1 / 10.0 |

- The -8 is the same knockdown as clay's -7 / -8: the read-out's standing height is taken low there.
- The pelvis heights are the Blender lane's night numbers (17.7, 17.5, 27.5).
- **The frozen frame:** press and hit in one frame, the lunge is at frame 6 and the human is low with the shoulder in; the victim is snapped (`sk16-night-chain`, top row).
- **A wall case:** the side hit 0.6 m from the side boards fired the rule; `Knockdown_Back` along the wall, nothing through it (bottom row).
- Takes alternate as on clay: `Fall` 3, `Fall_B` 2, `Knockdown_Back` 3 (one by the wall rule), `Knockdown_Back_B` 1, `Knockdown_Left` 2, `Knockdown_Left_B` 1.
- **Under the ice on night:** in `GetUp_Side_Left` the lowest joint reads -3.8 cm on two of three side knockdowns (-0.8 on the third); it is the frame listed under item 7, deeper than clay's -2.2.
- The night figure has no contact plane; item 3 is clay only.

### The reworked slide trail in real slides
The same knockdown on both tiers (from behind, a slide of about 7 m), frames 26, 42 and 60 after the hit:
- **Clay: the streak shows.** Dark smudges at the body at +26, a dark dashed line behind the body at +42, and at +60 a dashed line of roughly 2 to 3 m still clearly there. It reads as dashes, not as one ribbon, as B32 said.
- **Night: faint.** Nothing I can point to at +26 and +42; at +60 a thin dark dotted line behind the body.
- **How long it stays:** still there at +60 on both; my captures stop there, so the fade (B32: 1.4 to 1.6 s) is not measured.
- **Flicker: not judged.** My frames are 16 or more frames apart; flicker needs consecutive frames.
- The snow part is small and low; it covers nothing of the figures in these frames.

## 5. Five a side

- No ten-skater cartridge is in `Game/Content/Cartridges/` on this build, so I made a scratch copy of device A's QA cut on clay with five skaters a side (two more with the roles of the chaser and the back), outside the repository, and deleted it afterwards. The build accepted it: 10 skaters, 10 figures.
- **Look:** four to six figures are in the frame around a hit; each reads apart from the others, team colours hold, the hit ring and chips stay at skate height, nothing piles up visually (`sk16-five-a-side`).
- 5 knockdowns in about 30 s, all by the human, all with the hit-stop; get-up end minus release 0.00 on the two untouched ones.
- **Two knockdowns at once: not seen.** The closest were 102 frames apart on two victims, so the first was just up. Two slide trails or two rings at once are therefore not judged.
- A scrum of all ten in one frame did not happen either.
- `JAMVIS` pairs: knockdown 5 / 5, hits 23 = 17 played + 1 blocked + 5 skipped, pin on 3 = 2 played + 1 ignored (the figure was busy), board 4 / 4, stickhit 8 / 8.

## 6. Banner stills

- **HUD:** I found no switch for the whole HUD in device A's code (only `jam.hud.Pass3` and `jam.hud.PuckMarker`). My match script removes the viewport widgets, so the frames have no HUD; the glowing puck ring is a world object and stays.
- The capture comes out at 1920 x 1085 (asking for more gives the same), so each candidate is the full game-camera frame with 2 and 3 rows cropped top and bottom. No other recomposing, no retouching.
- **Candidate 1:** the held hit frame, lunge 7 frames in, victim snapped, the ring starting; a third figure stands to the right, calm ice all round.
- **Candidate 2:** the same hit 6 frames later: victim thrown back with arms and stick up. This is the liveliest of the three.
- **Candidate 3:** at the side boards, 6 frames after contact, the victim going down along the wall; a second orange figure from the knockdown before lies in the foreground.
- All three are from the camera behind the human, so the human is seen from the back.

## 7. The under-ice frames, for the Blender lane

`JAMLOW` in real matches on clay (export18); the get-up starts on sim frame 56 after the knockdown:

| Take | Take frame | Joint | Height above the ice (cm) |
|---|---|---|---|
| `GetUp_Side_Right` | 1 / 2 / **3** / 4 / 5 | `foot_l` | 10.4 / 4.5 / **-2.6** / 0.5 / 11.5 |
| `GetUp_Side_Left` | **3** (once at 4) | `foot_r` | **-2.2** (two of three knockdowns; 0.8 in the third) |

- It is the same fault mirrored: the foot of the upper leg dips through the ice plane for one frame as the leg swings under the body, about 3 frames into the side get-ups.
- What SK15 listed under `Knockdown_Right` was this frame of its get-up (`GetUp_Side_Right`), not the fall.
- Nothing else in the lying windows or the falls is under the plane in these logs.

## Real matches (hit-stop 3, drill 6)

| Match | Knockdowns | `JAMVIS` knockdown / hits (played + blocked + skipped) / pin / board / stickhit / hit-stop | `PlayCheck` |
|---|---|---|---|
| clay, three a side | 11 | 11 of 11 / 31 = 22 + 2 + 7 / 2 of 2 / 16 of 16 / 9 of 9 / 11 of 11 | 140 calls in 16 presses |
| night, three a side | 12 | 12 of 12 / 33 = 24 + 2 + 7 / 2 of 2 / 18 of 18 / 11 of 11 / 12 of 12 | 149 calls in 19 presses |
| clay, five a side | 5 | 5 of 5 / 23 = 17 + 1 + 5 / 3 = 2 + 1 ignored / 4 of 4 / 8 of 8 / 5 of 5 | not counted |

- Clay, pelvis at 90 % minus release (9 read): -3, -1, -2, -2, -3, -8, -3, -2, -2. Get-up end minus release 0.00 on the four knockdowns my captures did not touch.
- The human was not knocked down by a bot in any of the three.
- The machine was heavily loaded during the clay match (another editor and Blender); captures took up to 20 s each. I cut the planned runs: the night match has fewer frames per knockdown, and a second pass of the wall test figures was dropped.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| File | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 3,399,787 | 3,536,417 |
| `BP_SkaterVisual_TB1_P3` | 3,595,511 | 3,740,912 |
| `M_SkaterContact` | 14,665 | 16,847 |

Nothing else changed. 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs are up to date, no dirty package at exit. `BP_SkaterVisual_TB2_P3` also uses `M_SkaterContact`; with `Flat` at its default 0 its shade is unchanged.

## Stills (`C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with `.txt`)

- `20261113-145224-B-sk16-banner-candidate-1.png`: SK16 banner candidate 1: the held hit frame in a real match on clay (drill 6, hit-stop 3, A's game camera, HUD widgets removed, 1920 x 1080): the human's lunge 7 frames in, the victim snapped at frame 2 of Fall_B, the hit ring starting on the ice. Not retouched.
- `20261113-145226-B-sk16-banner-candidate-2.png`: SK16 banner candidate 2: the same hit 6 frames later (real match on clay, A's game camera, HUD widgets removed, 1920 x 1080): the victim thrown back with arms and stick up, the human low in the lunge, calm ice around them. Not retouched.
- `20261113-145227-B-sk16-banner-candidate-3.png`: SK16 banner candidate 3: a hit at the side boards, 6 frames after contact (real match on clay, A's game camera, HUD widgets removed, 1920 x 1080): the victim going down along the wall by the SK16 wall rule; a second orange figure from the knockdown before lies in the foreground. Not retouched.
- `20261113-145228-B-sk16-walls.jpg`: SK16: hits from behind or the front toward a wall, clay test figures 0.6 m from the boards. Top left is SK15 (end boards: head and arms in the wall). The others are SK16: a side take along the wall at the end boards, in a corner and at the side boards (from behind and from the front); bottom right an open-ice control.
- `20261113-145236-B-sk16-contact-shade.jpg`: SK16: contact shade under a lying clay figure at mid ice, on a wing and in a corner zone. Top: SK15 (old falloff: a dark spot near the legs only). Bottom: SK16 (flat top 0.7, 2.2 x 0.95 m, Darkest 0.75): an even soft shape under the whole body, 16 to 17 darker than the ice beside it.
- `20261113-150239-B-sk16-night-chain.jpg`: SK16: the night figure in a real match (drill 6, hit-stop 3, the SK16 Blueprint, export18, A's game camera). Top: press and hit in one frame with the lunge in at frame 6, the held frames, Fall, lying, GetUp. Bottom: the side hit 0.6 m from the side boards, the wall rule plays Knockdown_Back along the wall, nothing through it.
- `20261113-150242-B-sk16-slide-trail.jpg`: SK16: the reworked slide trail (B32) in a real slide of about 7 m at A's game camera, the same knockdown on clay (top) and night (bottom), 26, 42 and 60 frames after the hit. Clay: a dark dashed streak, clear at +42 and +60. Night: a thin faint streak, visible at +60.
- `20261113-150350-B-sk16-rinks.jpg`: SK16: the wall rule on other rinks, clay test figures. Pond (60 x 30 m, corner 3 m, day tier): a side fall 0.6 m from the side boards turns along the wall, open ice plays the plain side take, the corner follows the arc. Wide arena (140 x 70 m, night tier): at its side boards the rule fires; where the regular rink's boards would be it does not.
- `20261113-150530-B-sk16-five-a-side.jpg`: SK16: five a side on clay for look only (a scratch cartridge outside the repository: the regular rink, pro bots, penalties off, drill 6, hit-stop 3), full frames at A's game camera around two knockdowns by the human. Ten figures on the ice; the figures, their shades, the hit ring and chips stay readable; no two knockdowns overlapped in this match.

## Ledger rows

Replace the three rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.37 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16): every knockdown toward the boards within 1.5 m plays a take along the wall (side hits a fall along the facing, hits from the front or behind a side take), rink half-sizes from RinkActor's Ice bounds and the corner radius from the ice material instance, read on every knockdown (3536417 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 3.57 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling and walls as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16) (3740912 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterContact.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | unlit translucent Modulate disc (Darkest 0.45, smooth falloff; scalar Flat 0 to 1 keeps full strength to that fraction of the radius, 0 = the SK8 shape) for the ContactShadow plane of BP_SkaterVisual_TB1_P3 and _TB2_P3 (lanes SK8, SK16) (16847 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- A hit from the front or from behind toward a wall in a real match: none happened.
- Joint distances to the wall for the end-boards and corner test figures: not read (the figures were off camera at the reading); the stills show them inside.
- The cage (corner 2 m, obstacles) and the other modes: not run. The rule knows nothing of obstacles.
- The re-read after a live rebuild: `jam.livecut` did not change the rink in my session.
- The contact shade's reach past the body in cm; the shade in a real knockdown (test figures only); the shade on `BP_SkaterVisual_TB2_P3` (unchanged by construction, not looked at).
- Slide trail: flicker, and how long it stays past +60.
- Five a side: two knockdowns at once, a scrum of ten.
- The human knocked down by a bot.
- On night: `Knockdown_Right`, `Fall_Forward`, `Trip`; hit-stop 0 and 6 (3 only).
- The "both sides blocked" branch (turn toward centre ice): never reached.

## Decisions for you

1. **The banner frame:** candidate 2 is my pick (the victim in the air, both figures clear, calm ice); 1 is the quieter one; 3 has the boards but a second body in the foreground.
2. **Mitts at the kick plate.** In the wall cases a hand joint ends 15 to 30 cm from the wall, so a mitt can touch the kick plate. Nothing goes through. If you want clear air, the Blender lane could tuck the leading arm in the side takes; I would leave it.
3. **The side get-ups' foot dip** (2.2 to 3.8 cm under the ice for one frame, deeper on night): for the Blender lane, with the frames in item 7.
4. **Night slide streak:** faint in a real slide. B32's colour for night could go darker; that is the effects lane's call.
5. **Five a side:** when device A's ten-skater cartridge lands, one more look for two knockdowns at once.

## Notes for device A

1. **`jam.livecut` and the rink.** Applying `hockey.arena.wide` live to a Pond match changed the look but the rink actor's `Ice` bounds stayed 3000 x 1500 in my session, so anything of B's that sizes itself from the rink keeps the old rink after a live cut. If the rink is meant to rebuild, it did not here.
2. **The accessor** you were asked for can replace my reading of the `Ice` bounds and of `CornerRadiusCm` on the ice material instance. Until then: please keep setting `CornerRadiusCm` on the instance that sits on `Ice` material slot 0, and keep the component's name `Ice`.
3. **The rule knows only the outer boards.** Obstacles (the cage) and the nets are not tested.
4. `PlayCheck` still arrives on every held frame (140 calls in 16 presses on clay, 149 in 19 on night).
5. **A HUD switch** for stills (one cvar that hides every HUD widget) would save each lane its own workaround.
6. Five skaters a side loaded and played from a cartridge with five entries per team on this build.

