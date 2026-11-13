# SK17 report: the cross-check (Lift, Lift_Hit), Poke_Hit and the side get-ups from export20 (device B)

Lane SK17, device B, machine clock 16:10 to 16:31. Tree `C:\Users\kazda\jam\wt\b-vfx` (`dev-b` b227917), editor on port 8000 (pid 18428), left through `ExitEditor`; the process is gone. No git writes, no cook. Saved: `BP_SkaterVisual_TB1`, `BP_SkaterVisual_TB1_P3` and 10 takes. `/Game/Maps/Rink` not saved; no `_Scratch/`; `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.hitstop` read back at their defaults. Port 8002 (the effects lane) not touched; nothing under `VFX/` or `Audio/` touched. Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\sk17` (`bp17.mjs` the Blueprint change, `import20.py`, `react17.py` / `react17.sh` the match strips, `lift17.mjs` the counts and the geometry, `t7.sh` the test figures, `bk17/` the two Blueprints and the 152 takes as they were at lane start).

## Verdicts

| Item | Verdict |
|---|---|
| 1. Import export20 | **PASS**: 5 takes per figure by name (`Lift`, `Lift_Hit`, `Poke_Hit`, `GetUp_Side_Left`, `GetUp_Side_Right`); each copy verified against the source at copy time; clay's 28 aim holds kept |
| 2. How the Blueprints play them | **Changed**: `Lift` now plays 20 of 20 (it was 9 of 20), `Lift_Hit` 20 of 20, `Poke_Hit` 2 of 2 (it was 0 of 2) in the cross-check drill |
| 3. Real matches, counts and geometry | **Measured**, with one real problem: in 10 of 20 clean cross-checks the attacker faces more than 60 degrees away from the victim |
| 4. How it reads | **The victim reads, the attacker mostly does not** (plain verdict below) |
| 5. Side get-ups' frame 3 | **PASS** on the one side knockdown seen: lowest joint 1.6 cm above the ice (it was -2.2) |
| 6. Audio lines | **Clean** for `lift`, `poke`, `check`, `knockdown`, `board`; one `JAMFX missing` pair for `deke` |

## What the Blueprints did before, and what is saved now (both Blueprints, the same text)

| Call | Before | Saved now |
|---|---|---|
| `PlayLift` (attacker) | `Lift` at priority 3, blend-in 0.08 s | `Lift` at **priority 6**, blend-in **0.03 s**, a log line (`SK6RX lift ... played / blocked`); if it will play, `bPinned` is cleared first |
| `PlayStickHit(true)` (victim of a clean lift) | skipped if the figure is down or within 0.6 s of its last reaction; else `Lift_Hit` at priority 6, blend-in 0.08 s | skipped only if the figure is down; `Lift_Hit` at priority 6, blend-in **0.03 s** |
| `PlayStickHit(false)` (victim of a clean poke) | same 0.6 s rule; `Poke_Hit` at priority 6, blend-in 0.12 s | skipped only if the figure is down; `Poke_Hit` at priority 6, blend-in **0.03 s** |
| `PlayPoke` (attacker) | `Poke` at priority 3 | unchanged |

- **A held key does not re-send `PlayLift` every frame.** The sim gives a lift a 30-frame cooldown (`LiftReadyFrame`), so the 24-frame take cannot be restarted inside itself. No guard was needed, and none was added.
- **The 0.6 s rule: both stick hits now cut through it** (they still set its clock, so a bump right after does not replace them). You asked for `Lift_Hit`; I did the same for `Poke_Hit` because in the first drill match both clean pokes on a carrier were skipped by the rule (2 of 2), and a stick hit comes once per lost puck, not every frame. Say if you want the poke back under the rule.
- **Why priority 6 for `Lift`.** With priority 5 (my first try) the attacker's take was blocked in 11 of 20 cross-checks: 8 times by its own `Lift_Hit` (in the drill the two figures cross-check each other 2 to 10 frames apart) and 3 times by `Board_Pin`. At 6 it replaces both.
- **`bPinned` is cleared** when `Lift` replaces a pin, so the pin's end does not stop the cross-check half way.
- **Still above them: the body check (priority 7).** A figure in its own check lunge shows neither `Lift` nor `Lift_Hit` (numbers below).
- **Found on the way:** `PlayCut` blends every take out over 0.2 s. For a 24-frame take that means the take stops being the active montage at frame 12; frames 12 to 24 are a blend back to skating. `Lift` and `Poke_Hit` have their action in frames 0 to 8, so the hit is intact; their authored recovery is not what plays. Not changed.

## 3. Real matches: counts

| Match (hit-stop 3) | `lift` clean / miss | Attacker's `Lift` played / blocked | Victim's `Lift_Hit` played / blocked / skipped | Clean pokes on a carrier: `Poke_Hit` played / skipped |
|---|---|---|---|---|
| clay, cross-check drill (drill 2), first Blueprint state (`Lift` priority 5, poke under the rule) | 20 / 0 | 9 / 11 | 20 / 0 / 0 | 0 / 2 |
| clay, cross-check drill, **final Blueprint** | 20 / 0 | **20 / 0** | **20 / 0 / 0** | **2 / 0** |
| night, cross-check drill, final Blueprint | 20 / 0 | 20 / 0 | 20 / 0 / 0 | 2 / 0 |
| clay, drill 6 (the human carries at the bots and body-checks), final Blueprint | 8 / 0 | 5 / 3 | 4 / 4 / 0 | none occurred |

- `JAMVIS stickhit` lines equal the sim's clean events in every match (20, 20, 20, 8 lifted; 2, 2, 2, 0 poked).
- SK12's count for comparison: `PlayStickHit` 45 sent = 18 played + 27 skipped.
- **Drill 6, the 3 and 4 blocked:** all are a bot cross-checking the human one or two frames before the human's body check lands on that bot. Both are then in a check lunge (priority 7), which outranks the stick takes. In the one case I have frames for, `Lift_Hit` started and the lunge replaced it two frames later.
- **No miss happened** in any match: the drill presses only in range, and the bots did the same. The miss still is a test figure.
- In the cross-check drill 12 of the 20 are the bot on the human and 8 the human on the bot.

## 3b. Geometry of the 20 clean cross-checks (clay, final Blueprint; night is the same match and gives the same numbers)

| Measure | Value |
|---|---|
| Distance between the two figures at the hit | 80 to 171 cm, median 82 |
| Attacker's facing against the line to the victim: within 30 degrees | 6 of 20 |
| 30 to 60 degrees | 4 of 20 |
| 60 to 90 degrees | 3 of 20 |
| over 90 degrees (the victim is behind the attacker's shoulder) | **7 of 20** |

- **The stick is thrust at empty ice in half the cases** (10 of 20 are more than 60 degrees off; in 7 the attacker has its back to the victim). The sim's lift tests the distance between the two only, not the direction. In drill 6 (8 cases) it is better: 5 within 30 degrees, 1 each in the other three bands.
- The take always shoves along the figure's facing. I did not turn the figure toward the victim in this lane (see the decisions).

## 4. How it reads from the game camera

- **The victim reads, clearly, at game size.** By +2 its stick is knocked up overhead with both arms up, it stays up through +8, then two stumbling steps; it stays on its feet. Nothing else in the game looks like that: the body check throws the victim down, the poke only jars the stick sideways.
- **The attacker reads weakly.** The game camera sits behind the human. When the human faces the victim (the operator sheet), its own body hides the shove: you see the stick go level and the shoulders drive, not the thrust. When it does not face the victim (the clay and night strips), the level stick is plainly seen pointing past or away from the victim.
- **Is it clearly different from the body check and the poke?** By the victim, yes. By the attacker alone, no: from behind, the two-handed shove and the check lunge are both "the figure drives forward", and the difference is the stick held level across the chest.
- **Does it read as a cross-check?** When the two face each other within about a stick's length: yes, as "he got his stick knocked up and stumbled". In the other half it reads as the victim reacting to nothing the attacker visibly did.
- The poke (`sk17-poke`, test figures): `Poke_Hit` moves from the first frame, the stick is knocked out to the side by frame 2, a half step by 6; smaller and quicker than `Lift_Hit`. It was not seen at the game camera in a real match.

## 5. The side get-ups in a real match

`JAMLOW`, clay, one `Knockdown_Left` with `GetUp_Side_Left` (the get-up starts on sim frame 56):

| Take frame | 0 | 1 | 2 | 3 | 4 | 5 |
|---|---|---|---|---|---|---|
| `foot_r` above the ice (cm), export20 | 11.3 | 11.4 | 11.7 | 9.4 | **1.6** | 1.6 |
| the same with export18 (SK16) | | | | **-2.2** | | |

- The lowest joint over the whole knockdown is 1.6 cm above the ice (it was -2.2 on clay, -3.8 on night).
- Pelvis at 90 % minus the release in that match: -3, -3, -3, -2, -2, -8, -2, -3 (the -8 is the read-out's low baseline again).
- `GetUp_Side_Right` did not play in a real match; night was not measured for this.

## 6. Audio and effect lines

- `JAMFX missing`: one pair in the session, `NS_deke` and `MS_deke` ("logged once, skipped"). Nothing missing for `lift`, `poke`, `check`, `knockdown`, `board`.
- No line with `MetaSound` or `LogAudio` and `Error` in the four matches.

## Files changed (`Game/Plugins/JamAssets/Content/Skaters/`)

| What | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 3,536,417 | 3,559,799 |
| `BP_SkaterVisual_TB1_P3` | 3,740,912 | 3,764,357 |
| 5 `AS_SkaterTB1v2_*` (`GetUp_Side_Left`, `GetUp_Side_Right`, `Lift`, `Lift_Hit`, `Poke_Hit`) | 800,266 | 818,890 |
| 5 `AS_SkaterTB1_P3_*` (the same five) | 853,742 | 875,150 |

Import against the lane-start state: night 71 equal and 5 copied; clay 43 equal, 5 copied, 28 aim holds kept; no take failed its check after the copy. 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs are up to date, no dirty package at exit, the scratch import folder is gone. Note: the Blender lane's READY file lists two takes against export19; against the SK15 state (export18) it is these five, as your brief said.

## Stills (`C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`, each with `.txt`)

- `20261113-162813-B-sk17-crosscheck-clay.jpg`: SK17: real cross-checks on clay at A's game camera, every 2nd frame from the call to +24 (export20, the SK17 Blueprint). Top: the human on a bot 1.4 m away and 58 degrees off its facing: the bot's stick is knocked overhead, the human's shove goes past it. Bottom: a bot on the human at 0.8 m; two frames later the human's own body check lands and replaces the reaction.
- `20261113-162814-B-sk17-crosscheck-night.jpg`: SK17: a real cross-check on night (drill 2, A's game camera), every 2nd frame from the call to +24: the human on a bot that is 1.5 m away and behind its shoulder (137 degrees off the facing). The bot's Lift_Hit plays; the human's shove points away from it.
- `20261113-162815-B-sk17-crosscheck-miss.jpg`: SK17: the cross-check with nobody in reach. No miss happened in the real matches (the drill presses only in range), so this is the Lift take on a clay test figure gliding at 6 m/s at a game-like camera, slow motion, labelled by the take's frame; from frame 12 the take is blending out.
- `20261113-162815-B-sk17-crosscheck-sheet.jpg`: SK17 operator sheet: the best real cross-check on clay (the human on a bot at the boards, 0.8 m, facing it; A's game camera): call, +2, +4, +8, +14, +24. The victim's stick is knocked up overhead and it stumbles; the attacker is seen from behind.
- `20261113-162816-B-sk17-poke.jpg`: SK17: a poke with the new Poke_Hit on clay test figures (no poke on the human was captured in the real matches): blue pokes, orange is the carrier and plays Poke_Hit, slow motion, labelled by the frame of Poke_Hit.

## Ledger rows

Replace the two Blueprint rows (their notes plus this clause) and the ten take rows:
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.39 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lane SK17): Lift at priority 6 with a 0.03 s blend-in, Lift_Hit and Poke_Hit cut through the 0.6 s reaction rule with a 0.03 s blend-in (3559799 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 3.59 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls and cross-check as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16) (3764357 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17) (174102 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17) (174149 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p2_cel loopcut, take Lift by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17) (151941 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p2_cel loopcut, take Lift_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17) (162025 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Poke_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p2_cel loopcut, take Poke_Hit by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK17) (156673 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17) (186928 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17) (186969 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p3_clay loopcut, take Lift by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17) (161227 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Hit.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p3_clay loopcut, take Lift_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17) (174042 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Poke_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | skf/export20/p3_clay loopcut, take Poke_Hit by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK17) (165984 bytes) |
```

## What I could not verify

- Cook: not run (yours).
- A second import run as proof (nothing copied): not run; each take was compared with its source right after the copy.
- A miss in a real match: none happened. A poke's victim at the game camera in a real match: not captured (the two clean pokes were not on the human).
- `Lift` and `Lift_Hit` on bots at `all_star`: not run; `pro` only.
- `GetUp_Side_Right` and the night side get-up heights with export20.
- The takes in motion: strips of every 2nd frame and slow-motion test figures only.
- Frames 12 to 24 of `Lift`, `Lift_Hit` and `Poke_Hit` as authored (they play inside the blend-out).
- Whether clearing `bPinned` on a cross-check leaves a pinned figure without its pin pose for long: not looked at.

## Decisions for you

1. **The attacker's direction** (10 of 20 more than 60 degrees off). Three ways, none applied:
   - (a) B turns the attacker's mesh toward the victim for the take on the sim's clean `lift` event (the event carries the victim's position), limited to about 90 degrees, back to the facing by frame 12. One more lane; visual only.
   - (b) Device A lets a lift land only when the victim is inside a cone in front of the attacker (say 60 degrees each side). That changes the game: about half of today's clean lifts would become misses.
   - (c) Leave it.
   I would take (a), and ask A for (b) at a wide cone (over 90 degrees off should not land).
2. **Stick takes against the check lunge.** Today a figure in its own check lunge shows neither its cross-check nor the reaction to one (drill 6: 3 and 4 of 8). Raising them to priority 7 would cut a lunge mid-hit. I would leave it: the body check is the bigger event.
3. **`Poke_Hit` through the 0.6 s rule**: keep (my choice) or put the poke back under the rule.
4. **The 0.2 s blend-out on short takes**: keep, or shorten to about 0.1 s for `Lift`, `Lift_Hit` and `Poke_Hit` so more of the authored recovery plays. Small; I would try it with the mesh turn in the same lane.

## Notes for device A

1. **A lift has no direction test.** `TryLift` checks the distance to the carrier only, so a cross-check lands on a carrier behind the attacker (7 of 20 in the drill were over 90 degrees off the attacker's facing). Whatever B does with the figure, a lift at someone behind your back looks wrong; a cone would fix it at the source.
2. `PlayLift` arrives once per lift (30-frame cooldown): good, no guard needed on B's side.
3. The clean `lift` event carries the attacker in `who` and the victim's position in `where`; B will use that to aim the figure if decision 1(a) is taken. Please keep it in the frame of the lift.
4. In the cross-check drill the two figures trade cross-checks 2 to 10 frames apart all match long (20 in 28 s). If that is not intended, the bot's lift may want a short delay after being lifted itself.
5. `NS_deke` and `MS_deke` are reported missing once per session (`JAMFX missing`).
