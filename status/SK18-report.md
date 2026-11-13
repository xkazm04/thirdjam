# SK18 report: the attacker turns into the cross-check, shorter blend-out, the effects lane's assets in real matches (device B)

Lane SK18, device B, machine clock 16:33 to 16:50. Tree `C:\Users\kazda\jam\wt\b-vfx`, editor on port 8000 (pid 26476), left through `ExitEditor`; the process is gone. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3`, both with no play session open. Nothing under `VFX/` or `Audio/` touched. `/Game/Maps/Rink` not saved; no `_Scratch/`; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.hitstop` read back at their defaults. Port 8002 (the look lane) not touched. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk17` (`bp18.mjs` the Blueprint change, `react17.py` / `react15.py` the match strips, `lift17.mjs` the counts and the geometry, `bk18/` the two Blueprints as they were at lane start).

**Saves during a play session (your question):** none of mine. In every lane my save call comes after the script that stops the play session, and each save was confirmed by the returned flag and by the file's size on disk; SK17's two re-saves after my failed edits were also made with the session stopped.

## Verdicts

| Item | Verdict |
|---|---|
| 1. The attacker turns toward the victim | **PASS**: 4 frames after the hit the mesh is within 30 degrees of the victim in 12 of 20 clean cross-checks (the pawn's facing: 7 of 20); over 90 degrees: 4 (was 7) |
| 2. Blend-out 0.1 s on the short takes | **Done**; no pop into skating in the strips |
| 3a. `NS_lift` | **Thin on clay** (a few pale splinters; the amber cracks cannot be made out), **readable on night**; nothing crosses a trunk for more than a few frames |
| 3b. `NS_board` skater variant | **Confirmed**: no shake line on the board top at Strength 1.00; light and hard differ only by a few more chips and a small puff |
| 3c. Slide streak | **Continuous and readable at +40 on clay and night**; **a fault at +10 and +20**: long thin straight lines cross the frame through the place of the knockdown, on both tiers |
| 3d. `NS_check` / `NS_knockdown` | **Unchanged**: the frozen hit frame shows the low chips and the ring, nothing over the figures |
| 4. Audio lines | **Clean** besides `deke` |
| 5. The human knocked down by a bot | **SEEN, once**: one knockdown of the human in a 90 s `all_star` match (and one bot on bot); the chain is complete and the get-up ends on the release |

## The Blueprint changes as saved (both Blueprints, the same text)

1. **The turn.** New variables `LiftYaw`, `LiftN`, `LiftEvT`, `LiftPlayT`.
   - `OnBusEvent`, new branch on `lift`: when this figure is the attacker and the lift is clean, `LiftYaw` = the angle from the figure's facing to the victim's position (the event's `where`), limited to 60 degrees either way.
   - The turn starts only if the `Lift` take really plays: `PlayLift` (when its take will play) and the bus event each note their time, and whichever comes second in the frame sets `LiftN` to 22.
   - `UpdateMotion`: counts `LiftN` down once per call; in the standing branch the mesh yaw is the pawn's yaw plus `LiftYaw` times a weight that rises to 1 over 3 frames and falls to 0 over the last 10 (frames 12 to 22 of the take).
   - **Nothing the sim reads is touched:** only the mesh component's world rotation, as before; `FacingYaw` stays the pawn's.
   - **A miss does not turn** (no clean event). Beyond 60 degrees the mesh turns its 60 and the take plays as is.
   - **Against the yaw and blade corrections:** the blade and stick corrections (`SK3Fix`) already switch themselves off while any montage plays, the blend-out included, so they are suspended for the take's 24 frames; the turn is back at 0 by frame 22, before they resume. The down and get-up branches of `UpdateMotion` do not use the offset. I did not change the aim layer's weight; in the strips both hands stay on the stick through the turn.
2. **Blend-out 0.1 s.** A new event `PlayCut1` (a copy of `PlayCut` with 0.1 s instead of 0.2 s) plays `Lift`, `Lift_Hit`, `Poke_Hit` and the three standing hits (`Check_Hit_Back`, `_Left`, `_Right`). The check lunge and `Stumble_Short` stay on `PlayCut` (0.2 s): the lunge's restart rule counts on its length.

## 1. The turn, measured (clay, drill 2, the same seeded match as SK17; 20 clean cross-checks)

| Angle to the victim | Pawn facing at the hit (before, this run) | SK17's table | Mesh 4 frames after the hit (after) |
|---|---|---|---|
| within 30 degrees | 7 | 6 | **12** |
| 30 to 60 | 2 | 4 | 3 |
| 60 to 90 | 4 | 3 | 1 |
| over 90 | 7 | 7 | 4 |

- "After" is the mesh's yaw against the line to the victim 4 frames after the hit, when the turn is fully in. In those 4 frames the two figures also move, so single cases shift by more or less than the 60 degrees (a case at -137 reads -38, one at -176 reads -180).
- Every case that was within 60 degrees ends within 25; the cases at 61 to 93 end at 2 to 54.
- `Lift` played 20 of 20, `Lift_Hit` 20 of 20, `Poke_Hit` 2 of 2; no miss.
- Night, the first 12 s of the same match: 10 clean, 10 and 10 played; the mesh within 30 degrees in 4, 30 to 60 in 3, over 90 in 3 (pawn facing: 3, 0, 2 at 60 to 90, 5 over 90).
- **How it reads now** (`sk18-crosscheck-turn`, `sk18-crosscheck-sheet`): with the victim to the side the human turns into it and is seen three-quarters from behind; both hands drive the level stick into the victim's chest by +2, the victim's stick goes overhead, and by +24 the human faces its skating direction again. This reads as a cross-check. When the victim is straight ahead the camera still hides the shove behind the human's back.

## 2. Blend-out

- With 0.1 s the takes play as authored to frame 18 of 24 (`Lift`, `Poke_Hit`) and 22 of 28 (`Lift_Hit`), then blend to skating over 6 frames.
- In the strips at +14, +18, +20, +24 the figures settle into the skating pose without a visible step. Judged from every 2nd frame, not in motion.

## 3. The effects lane's assets in real matches (nothing under `VFX/` edited)

**`NS_lift`, clean** (`sk18-crosscheck-effect-clay`, `-night`):
- Clay: three to six small pale splinters near the victim's trunk and stick from the call to about +8; small white dots at the skates. **I cannot make out the three amber cracks at game size on clay.** The effects lane's "thin" is right.
- Night: small amber marks at the victim's waist at the call and amber splinters at +2 and +4; readable.
- Chips over a trunk: one or two specks for 2 to 6 frames; none stays to +8 or longer.
- A miss: none occurred in any match, so the miss effect is not seen.

**`NS_board`, skater variant** (`sk18-board-light-hard`; Strength from the sim's `board` events):
- Light brush (0.39 and 0.59, the human): two or three small chips at the skates.
- Hard crash (1.00, a bot at the end boards): about six chips and a small puff at the skates.
- **No shake line on the board top at 1.00** (nor at the light ones). Confirmed.
- The difference between light and hard is small at the game camera.

**Slide streak** (`sk18-slide-streak`; the same knockdown on both tiers, a slide of about 7 m):
- **+40:** a continuous soft dark band behind the body on clay and on night; on night it now reads clearly (SK16: faint).
- **+10 and +20: long thin straight lines** run through the place of the knockdown to the edges of the frame, two or more of them, on clay (faint grey) and on night (clear grey). They are gone at +40. They are not there in the hit frame. I cannot say from stills what they are; a first streak segment stretched from somewhere far away would look like this. This may be the "detached patch" seen once on night. For the effects lane.
- Dashed or continuous: continuous at +40. Flicker: not judged (frames 10 or more apart).

**`NS_check` / `NS_knockdown`:** the frozen hit frame on clay shows the flat ring and low chips at the skates and nothing over the trunks, as in SK14 and SK16.

## 4. Audio and effect lines

- `JAMFX missing`: `NS_deke` and `MS_deke`, once per session. Nothing else.
- No line with `MetaSound` or `LogAudio` and `Error` in the matches.

## 5. The human knocked down by a bot

One `all_star` match of 90 s on clay, scripted human, no drill, hit-stop 3:
- **Knockdowns: 2.** One bot on bot (no hit-stop), and **one of the human**: a bot checked it from the side (`side_l`, strength 0.32) near the side boards, with the hit-stop (3 frames).
- The human's chain (`sk18-human-knocked-down`): the held frames show the bot's shoulder in and the human bent away; plain `Knockdown_Left`, a short slide, lying along the boards, `GetUp_Side_Left`. Nothing through the wall (the wall rule did not need to fire).
- By `JAMLOW`: pelvis at 90 % minus the release -1; lying pelvis 28.4 cm; the lowest joint over the whole knockdown 6.0 cm above the ice (the side get-up's frame 3 is clean here too).
- Get-up end minus release reads 0.90, because my captures froze that knockdown; the bot-on-bot one reads 0.00.
- In the same match: 2 checks landed against 66 collisions with no landed check; 20 clean cross-checks (`Lift` played 17, blocked 3; `Lift_Hit` 20 of 20); 8 clean pokes on a carrier (`Poke_Hit` 8 of 8). The bots' cross-checks are squarer than the drill's: the mesh is within 30 degrees of the victim in 16 of 20 (pawn facing: 9), over 90 in 1 (pawn: 3).
- The camera view of the human's own fall is close and from behind, as for every other figure; I saw nothing wrong in it.

## Matches run (hit-stop 3, `pro` bots unless said)

| Match | What for | Counts |
|---|---|---|
| clay, drill 2, 33 s | the turn, `NS_lift` | 20 clean lifts; 20 / 20 / 20 played |
| clay, drill 6, 34 s | frozen hit frame, streak, board hits | knockdown 4 of 4, hits 16 = 11 played + 1 blocked + 4 skipped, pin 2 of 2, board 7 of 7, stickhit 4 of 4; get-up end minus release 0.00 on the two untouched knockdowns |
| clay, drill 6, 26 s | board hits by bots near the human | 5 board hits captured, Strength 0.55 to 1.00 |
| night, drill 2, 12 s | the turn and `NS_lift` on night | 10 clean lifts; 10 / 10 played |
| night, drill 6, 33 s | streak, hit frame | the same counts as clay |
| clay, `all_star`, scripted human, no drill, 90 s | the human knocked down | knockdowns 2 (1 of the human); 20 clean lifts; 8 clean pokes on a carrier |

In drill 6 one of four cross-checks was blocked on both figures by their check lunges, as in SK17 (you chose to leave the lunge on top).

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| File | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 3,559,799 | 3,738,157 |
| `BP_SkaterVisual_TB1_P3` | 3,764,357 | 3,942,681 |

No take changed. 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs are up to date, no dirty package at exit.

## Stills (`C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`, each with `.txt`)

- `20261113-164553-B-sk18-crosscheck-turn.jpg`: SK18: the same real cross-check on clay (the same seeded match, the human on a bot 1.4 m away and about 60 degrees off its facing; A's game camera; call, +2, +4, +6, +8, +12). Top: SK17, the shove goes straight ahead past the victim. Bottom: SK18, the mesh turns toward the victim for the take and the shove goes into it.
- `20261113-164554-B-sk18-crosscheck-sheet.jpg`: SK18 operator sheet: a real cross-check on clay with the mesh turn and the rebuilt effect (the human on a bot to its right; A's game camera): call, +2, +4, +8, +14, +24. The human is seen three-quarters from behind, both hands driving the stick into the victim; the victim's stick is knocked overhead; by +24 the human faces its skating direction again.
- `20261113-164555-B-sk18-crosscheck-effect-clay.jpg`: SK18: NS_lift (B33) on two real cross-checks on clay at A's game camera. A few small pale splinters fly near the victim for the first 6 to 8 frames; the three amber cracks cannot be made out at this size; nothing crosses a trunk for longer than a few frames.
- `20261113-164556-B-sk18-board-light-hard.jpg`: SK18: NS_board skater variant (B33) in real board hits on clay at A's game camera: hit, +2, +4, +8. Top: a light brush (Strength 0.39, the human): two or three small chips at the skates. Bottom: a hard crash (Strength 1.00, a bot at the end boards): a few more chips and a small puff at the skates. No shake line on the board top in either.
- `20261113-164556-B-sk18-crosscheck-effect-night.jpg`: SK18: NS_lift (B33) on a real cross-check on night at A's game camera (call, +2, +4, +8, +14, +24): small amber marks at the victim's waist at the call, amber splinters at +2 and +4; the sparks at +8 are the bot's own cross-check back. Readable; nothing covers a figure.
- `20261113-164557-B-sk18-slide-streak.jpg`: SK18: the slide streak (B33) in a real slide, the same knockdown on clay (top) and night (bottom) at +10, +20 and +40 frames, A's game camera. At +40 a continuous soft dark band lies behind the body on both tiers. At +10 and +20 long thin straight lines cross the frame through the place of the knockdown on both tiers; they are not there at +40.
- `20261113-164934-B-sk18-human-knocked-down.jpg`: SK18: the human knocked down by a bot, the first time seen on B (clay, all_star, scripted human, no drill, hit-stop 3, A's game camera): a check from the side near the side boards, the held frames, Knockdown_Left, slide, lying along the boards, GetUp_Side_Left to the release. Nothing goes through the wall.

## Ledger rows

Replace the two Blueprint rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.56 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18): Lift at priority 6, Lift_Hit and Poke_Hit through the 0.6 s rule, the attacker's mesh turns up to 60 degrees toward the victim for a clean lift, 0.1 s blend-out on the short takes (PlayCut1) (3738157 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 3.76 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls and cross-check as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16) (3942681 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- The turn in motion: strips of every 2nd frame and per-tick numbers only. The mesh's angle at the hit frame itself is the pawn's; the turn is in from 3 frames later.
- The turn on a figure that is knocked down or pinned during the take: not looked at (the down branch ignores the offset by construction).
- The aim layer during the turn: not measured; by eye the hands stay on the stick.
- The blend-out on the standing hits and on `Poke_Hit` at the game camera: not captured in this lane.
- `NS_lift` miss; `NS_poke`: not seen.
- The slide streak's flicker; what the long lines at +10 and +20 are.
- The board shake line at Strength 1.00 from close up (the hard crash was a bot at the end boards, some way from the camera).
- `all_star` cross-checks.

## Decisions for you

1. **The lines through the streak at +10 and +20**: to the effects lane with `sk18-slide-streak`; they show on both tiers in the one slide I shot.
2. **`NS_lift` on clay** is thin at game size. If the cross-check should be felt on clay, the mark wants to be larger or darker there; the figures now carry the read by themselves.
3. **When the device A change lands** (a cone, `Dir` on the event): the turn works from `where` today and needs no change for a cone; with over-90 cases gone, the 60-degree limit would cover nearly every clean lift.
4. **Light against hard board hits** look almost the same. If Strength should read, that is the effects lane's (more chips or a board shake at 1.0).

## Notes for device A

1. The figure now uses the clean `lift` event's `who` and `where` to turn the attacker. Please keep both, in the frame of the lift; `Dir` is welcome but not needed.
2. Still 4 of 20 clean lifts land on a carrier more than 90 degrees off even after a 60-degree turn (7 of 20 by the pawn's facing). A cone in the sim is what removes those.
3. `NS_deke` and `MS_deke` are still reported missing once per session.
4. On `all_star` without a drill the bots landed 2 checks in 90 s against 66 collisions with no check landed, and cross-checked 20 times. If aggression should read as tackles, the body check is still the rare one.
