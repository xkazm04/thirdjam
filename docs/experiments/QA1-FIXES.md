# QA1 fixes: device B's end-to-end play, the A-side faults in rank order; binder round 3 (lane A47, device A, 2026-11-12)

Owner: lane A47. Inputs: `handoff/B-to-A-0030.md` (QA1's fault table), `B-to-A-0029.md` (the rebuilt contact effects and the three vectors), `B-to-A-0031.md`
(the six newer events' sounds), R-0021 point 3, R-0031, `docs/experiments/COMMENTARY.md` (A32's rules). Everything here is presentation, binder or AI
calibration; the sim gained one read-out (`FJamSimEvent::Dir`) that nothing in the step reads and that is not hashed. `Jam.Core.Determinism` and
`Jam.Core.CartridgeParity` are green on the build (section 4).

## 1. The fault table (B's rank) and the fix per row

| # | Fault | Where it was | Fix |
|---|---|---|---|
| 1 | Motion blur smears every camera turn | nothing on A's side turned the engine default off; B set 0 in the three rigs' volumes | `UJamFollowCamera` carries its own post-process override: `bOverride_MotionBlurAmount`, `MotionBlurAmount 0`, blend weight 1 (constructor). The camera component is the last word under any volume. |
| 2 | Film on the ice: the MATCH clapperboard card at every faceoff, "DIRECTOR'S CALL READY" in every frame | `UJamDirectorsCallHudWidget::BuildTree` loaded `T_UI_MatchCard`; `UJamHudWidget` CallText; the overlay's ready hint | A plain banner on A18's `FJamUiStyle::Panel` plate: "MATCH n" (84 px Black) over "PERIOD n - FACEOFF" (label), one vertical box; the art stays in the menu. "CALL READY" on the HUD; "ESC / START   CALL READY" on the overlay pill. The menu, the Designer and the call card's own title keep the film words (R-0021 point 3). |
| 3a | Lines cut off after a word by a taunt | `FJamCommentaryPicker::Resolve`: any bark at the human cut the playing line | `CutFloorFrames` 30: a bark never cuts a line under 0.5 s old; the line plays on and the bark's hold (150 frames) still applies after it. An older line is cut as before. |
| 3b | A goal went uncalled behind a lower-priority line started 19 frames earlier | the gate (150 frames from the last start, one line at a time) dropped every late line, priority 5 included (A32's open point 1) | `MustPlayPriority` 5: a priority-5 line cuts a playing lower-priority line (`JAMVO_CUT reason=must_play`) and skips the 2.5 s gate behind one. It never cuts another priority-5 line and still waits out a bark hold (B's rule 5). The second beat follows as before. |
| 4 | "Banked it in off the boards!" 5 s into the first match on a plain board hit | `WhenHolds`: `board.bank` held when `E.Frame - LastBoardFrame <= 60`; `Remember` runs after `Candidates`, so on a **board** event this read the *previous* board hit: two hits within a second (the puck rattling along the boards) made every board.bank line a candidate at priority 4 | `board.bank` holds only on a goal event (Observe's goal-time path adds it; the window is unchanged). B's stopgap tags (`kind: bank`, `kind: rim`) are read as conditions, not payload kinds (`IsConditionKind`): without that, kind `rim` could never match the board payload's `skater` / none and the rim lines would be dead, and the early return `Sfx == L.Kind` short-circuited both to "holds". The only other suffixes the picker does not evaluate (`foul.behind`, `foul.no_puck`) are gated by their kind. |
| 5 | The shipped Boost Cut runs the morning's neon rig | `hockey.boost.cartridge.json` `game.look.tier: l3_neon` | `l2_night`. `Game/Source/JamCore/Tools/parity.json` regenerated (it embeds every preset's text). The Designer library guard (`cached-cuts.json` `base_sha256`) hashes `hockey.regular` only and `JamDesignerData.inl` does not embed the boost preset: no `gen-designer-data` run needed. |
| 6 | Ghost figures after `LoadCut` rebuilds (18 after three) | `BuildMatch` called `AActor::Destroy` on each skater; a Blueprint visual (`BP_SkaterVisual_*`) is a child **actor** attached to the skater, which `Destroy` does not take down | The visual goes first (actor `Destroy` or component `DestroyComponent`, A31's `ResetVisual` teardown), the skater after; `JAMVISUAL teardown: ...` and `JAMVISUAL after build: N skaters, M SkaterVisual actors in the world (ghosts=K)` in the log. |
| 7 | The caption stays long after a 2 s line | the HUD held the strip for the line's duration + 90 frames, from `GetLastLine()` alone (a cut line kept its full duration) | `UJamCommentarySystem::GetLineEndFrame()` (the picker's end frame, moved to the cut frame by a cut); the HUD passes hold 0 and the real end: the caption fades over 30 frames from the moment the voice stops. `CommentaryOpacityFor` keeps its old reading as the default for A18's tests. |
| 8 | An own goal bursts in the wrong team's colour | the binder coloured every burst by the team of `Who`; a goal's `Who` is the last toucher, on an own goal the conceding side | For `goal` the binder reads the net's side: `(Where.X > 0) == (AttackGoalX(0) > 0)` is team 0's goal; the colour is the scoring team's. |
| 9 | Pro bots shoot taps (charge 0.00 to 0.07), the slap wind-up never seen | two causes in `FJamBrain::Think`'s charge block: (1) the hold required `W.Carrier == i`, and the carrier of record is the last toucher with the puck in reach, so a bot arriving on a loose puck or a pass was aimed and in reach before it had touched it and its shot left as a tap through `bShoot`; (2) a hold in progress was released the first frame the line-up or carrier test flickered (the puck a few centimetres past reach on a fast carry), after one or two frames (0.03 to 0.07) | an aimed shot in reach charges whether or not the bot is the carrier of record; a hold in progress rides out a frame without the line-up while the bot can still shoot (`bCanShoot`: in reach, cooldown over), releasing at the window's end or when the puck is gone. `Jam.Bot.ProCharge` (new): a default pro match stepped by hand, every bot shot's `LastShotCharge` read from the sim: at least one charged shot (>= 0.3) and at least half of the bots' shots charged. Measured: section 4. This is an AI calibration (R-0021 point 1): bot match outcomes change; the two-run hash agreement tests stay green. |
| 10 | Goal light floats above the glass; menu cards say 90 x 45 for Raw and Boost; the pickup is a capsule | `RinkActor` housing base at glass top + 4 cm (above the rail); `MenuFlow::DescribeCut` fixed strings; `JamPickupActor` ctor | housing base 6 cm under the glass line (clamped over the rail); every card prints `%g x %g m` from its own `game.rink` plus the shipped cut's word (Regular still reads "90 x 45 m, fast"); the fallback token is `/Engine/BasicShapes/Cylinder` at 0.5 x 0.5 x 0.2 m. |

Binder round 3 (B-to-A-0029, 0031), all in `EffectsBinder.cpp` unless said:

| Ask | Done |
|---|---|
| `User.Dir` on `check` and `knockdown` | `FJamSimEvent::Dir` / `FJamEventPayload::Dir` (new read-out, rink (x, z) -> world (x, y), z 0, unit or zero): `check` carries the contact normal (checker towards victim, the `Dir` of `Landed`), `knockdown` the victim's `KnockDir`; the binder sets `User.Dir` on both |
| `User.Normal` on `board` | the puck's and the skater's board events carry `-N` (the board normal into the rink); the binder sets `User.Normal` |
| blade spray every 12 frames | `ToySkaterVisual.cpp` and `SkeletalSkaterVisual.cpp`: `LocalFrame % 12` (one constant each) |
| `carry` silent | `IsSilentEvent` |
| `period_end`, `clock`, `season_take` sound-only and 2D | `IsSoundOnlyEvent`, `IsSound2D` |
| `clock` Kind `sudden_death` -> Variant 1 | `VariantFor` |
| `humanoid_b1` per tier (0030's question, the Director's answer) | `ResolveVisualClass`: `BP_SkaterVisual_TB1_P3` on clay, `_TB1_P4` on broadcast, `_TB1` as the fallback of both (one "not found; trying the next class" line per resolve until B imports them) |

## 2. The commentary rules, as they read now (deltas to COMMENTARY.md section 2)

- Rule 3 (gate): unchanged for priority 1 to 4. A priority-5 line (goal colour calls, `goal.late`, `win`, `lose`, `clock.last_10_tied`) starts even inside the gate
  when the last line was lower than 5: a playing one is cut with a 0.15 s fade (`JAMVO_CUT ... reason=must_play`). It still waits out a bark hold and never cuts a 5.
- Rule 5 (yield): a bark at the human cuts the playing line only when it is 30 frames or older; a younger line plays to its end, and the 150-frame hold still begins at the bark.
- Section 3, `board.bank`: "a goal within 60 frames of a board hit, evaluated at the goal" is now literally so; on a `board` event it never holds.
- The manifest's `kind` on `board.bank.*` (`bank`) and `board.rim.*` (`rim`) are conditions: the picker evaluates them, it does not match them against the payload's kind.
- The caption follows the voice (hold 0, fade 30 frames from the line's end or its cut).

Tests: `Jam.Commentary.CutFloor`, `.MustPlay`, `.BankWhen` (new); `.Stoppage` updated at its "goal inside the gate" step (the goal now cuts in, the beat follows at the
reopened gate); `.BankClean`, `.Selection`, `.Timing`, `.Yield` unchanged.

## 3. Proof (PIE on MCP 8026, `tools/qa/a47run.mjs`)

Two passes on the same deterministic match (`jam.ScriptHuman 1`, `hockey.regular`, seed 1): a **log pass** (60 s of play, then three `jam.Cut hockey.regular`
rebuilds with a Python count of `JamSkater` and `*SkaterVisual*` actors after each, then `JAMEVENT` / `JAMVO` / `JAMVISUAL` / `JAMFX missing` lines) that prints the
sim frames worth a still, and a **still pass** that injects `tools/qa/a47shoot.py` (pauses the game at each sim frame so `CaptureGameViewport` can take the frame with its
UMG: a bark bubble, the banner and the caption are UMG that `HighResShot` does not draw), one still per target, resume, repeat. Results: section 4.

The brief says "three `jam.livecut` rebuilds": `jam.livecut` is `ApplyCartridgeLive` (no rebuild; a changed visual class respawns through `ResetVisual`, which was clean
in QA1). The ghosts came from `LoadCut` -> `BuildMatch` (`jam.Cut <name>` and the Designer's apply), so the proof rebuilds through `jam.Cut`.

## 4. Results (2026-11-12, worktree a47 at main 0e5ca5a, builds 22:55 / 23:00 / 23:09 / 23:16, editor on MCP 8026 from 23:02 to 23:19)

Headless (`a47-tests-4.log`, 23:16): **40/40**: Jam.Commentary 9/9 (BankClean, BankWhen, CutFloor, MustPlay, Selection, Stoppage, Timing, Yield + BankClean's embed check),
Jam.Core 16/16 (Determinism, CartridgeParity on the regenerated parity.json, TacklingEvents with the three `Dir` asserts), Jam.Effects 6/6, Jam.Bot 9/9 (ProCharge: hockey.regular
pro, the brain on all six, seed 1: **24 shots, 19 charged >= 0.3, mean 0.26, best 0.33**; before the fix QA1 saw 0.00 to 0.07). Node: `commentary/check.mjs` PASS (340 lines,
0 errors), `schema/validate.mjs` PASS on the boost cartridge.

PIE (`tools/qa/out/a47-log-1`, `-log-raw`, `-shoot-reg`, `-shoot-raw1`, `-shoot-raw2`; stills in `docs/experiments/img/qa1/`, 1920x1082 JPEG q4):

| Still | Shows | Row |
|---|---|---|
| `a47-banner-f20.jpg` | the plain banner "MATCH 1 / PERIOD 1 - FACEOFF" on the panel plate at frame 20, "ESC / START   CALL READY" bottom left, the take line caption | 2, 7 |
| `a47-banner-period2-f4054.jpg` | the same banner at the period-2 drop after the goal ("PERIOD 2 - FACEOFF"), WON in the ring, "Picture perfect." (the second beat) | 2 |
| `a47-board-skater-f1465.jpg` | blue 42 into the side boards at 0.92: B's Variant 1 burst (puffs, the clack star, the shake line) leaving the board face; "Loud one off the wall." | 8 (User.Normal) |
| `a47-board-puck-f2574.jpg`, `a47-board-f417.jpg` | puck board hits at 0.29 / 0.36: small chips at the contact (faint at that strength) | 8 |
| `a47-goal-burst-f3832.jpg` | 3 frames after blue's goal: the blue ring of NS_goal over the net, "It's in, for blue!" (the colour call, started at 3829) and an opponent's bark bubble that arrived 3 frames later and did **not** cut it; the goal light box on the glass line behind the net | 3a, 6, 8 |
| `a47-goal-call-f3854.jpg` | 25 frames after the goal: the call still playing under the burst; score 01:00 | 3, 7 |
| `a47-windup-f523.jpg` | orange 9 (a pro bot) with the puck at the boards 4 frames before its 0.60 shot; the hold is on (the figure's PlayShoot charge pose is B's TB1 figure's, faint from behind) | 9 |

Log facts (hockey.regular, 60 s, then hockey.raw, 100 s, both `jam.ScriptHuman 1`, seed 1): 13 and 16 commentator lines, 2 + 2 cut by a bark at the human, every cut on a
line 30 frames or older (the first cut in the raw run: `board.hard.03` at 1462 cut at 1537); 0 `JAMFX missing` lines for carry / period_end / clock / season_take and 0 overall;
`JAMVISUAL after build: 6 skaters, 6 SkaterVisual actors (ghosts=0)` after each of three `jam.Cut hockey.regular` rebuilds in the first run and one in the second, and the
Python actor count 6 `JamSkater` / 6 `BP_SkaterVisual_TB1_C` after each; `JAMVISUAL teardown: 6 skaters, 6 visual actors ... destroyed before them`. The first raw run (before
the bark-hold change, 23:03) had the goal at 3829 silenced by a bark at 3768 (hold to 3918, the beat at 3918); the second (23:18) calls it at 3829.

Eyes (`bug-hunt`, gemini-3.8-flash, no thinking, `docs/experiments/eyes/a47-bughunt-{1,2}.json`, $0.011): the banner stills clean except a faint dark rounded rect behind the
"MATCH 1" box (the far-net marker's caption under the banner: A42's marker, severity 2) and the bark bubble over the net marker at the period-2 drop; every board still: the skater's
skate and stick blade clip into the kick plate when a figure leans on the boards (B's TB1 figure against A25's kick plate, severity 3 to 4, not this lane); the goal-burst still: the
shooting figure's stick "floating above the shoulder" (the TB1 shoot pose seen from behind) and the bottom-right head of a figure cut by the frame. Advice, not a bug list.

Not obtained: **a check burst along the hit and a knockdown still.** In 235 s of bot play on three cuts (regular, raw, and an all_star copy) no check landed: `FJamBrain` only ever
writes `In.bCheck = false` (JamAICore.cpp line 582), so bots never body-check (R-0031 point 2 is not on main yet; B-to-A-0029 saw check, knockdown, foul 0 in 92 s for the
same reason), and the scripted human has no check. The `Dir` read-out on check and knockdown is proven by `Jam.Core.TacklingEvents` instead (the sim lands checks headlessly:
Dir = the push, unit, equal to the victim's KnockDir; the board Dir points into the rink). The picture waits for the first landed check in PIE (A33's `tools/qa/tackle.py` drives the
human into a check through the input path, which A49 owns tonight).

## 5. For the Director

1. **Must-play over the bark hold**: B's rule 5 ("the booth yields to a bark at the human") now has one exception, priority 5. Without it the first raw proof had the goal at 3829
   silenced by a steal complaint 61 frames earlier (the hold ran to 3918), which is QA1's fault 3 in another coat; B's acceptance line is "a goal always called". If B wants the bark
   to win on the goal frame itself, that is one condition (`bBarkAtHumanThisFrame` in the must-play test).
2. **Bots never check** (above): `In.bCheck` is never set in `JamAICore.cpp`. Whoever owns R-0031 point 2's lane should know the proof still is waiting on it.
3. `jam.Cut <relative path>` loads the preset of the same family instead of the file (`ResolveCartridgePath`); the QA cartridge path `tools/qa/cartridges/...` resolved to
   `hockey.raw`. Not chased tonight.
4. Files outside the brief's list, each one small: `RinkActor.cpp` (the goal light's base, 1 constant), `MenuFlow.cpp` (the rink-size strings live there, not in MenuWidget.cpp),
   `JamEventBus.h` (`FJamEventPayload::Dir`), `JamAICore.cpp` (the charge hold), `BotTests.cpp` and `JamTacklingTests.cpp` (tests), `SkeletalSkaterVisual.cpp` line 657
   (`% 8` -> `% 12`, A49's file under R-0032: the one constant, nothing else), `parity.json` (regenerated), the `tools/qa/a47*` harness (new).
