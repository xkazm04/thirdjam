# The commentator (lane A32, device A, 2026-11-12, from B-to-A-0026 and R-0027 point 4)

Owner: lane A32. B's bank (340 recorded lines, `Game/Plugins/JamAssets/Content/Audio/VO/`) and B's rules (`briefs/commentary/README.md`) are the inputs;
this file says what the runtime does with them, the seven events it adds, and how to prove it. Nobody has listened to the mix; every rule below is a frame count.

## 1. Files

| File | Does |
|---|---|
| `tools/commentary/embed.mjs` | writes `Game/Source/JamGame/Private/CommentaryData.inl` from `commentary.json` (the packaged game carries the manifest without a staging rule; the waves cook through `+DirectoriesToAlwaysCook=(Path="/JamAssets/Audio/VO")` in `Game/Config/DefaultGame.ini`) |
| `tools/commentary/check.mjs` | the manifest check: ten keys per line, id `<event>.<situation>.<nn>` whose first segment is the event, team blue/orange/any, integer priority 1..5, cooldown 0 or more, every `sound` on disk as a `.uasset`, no digits, text through the bark filter (`tools/barks/filter.mjs`) with two allowances (section 6) |
| `tools/commentary/commentary.test.mjs` | the committed manifest is clean and embedded; the check rejects what the contract forbids (runs in the premerge gate's `node --test tools/**/*.test.mjs`) |
| `Game/Source/JamGame/Public/JamCommentaryData.h`, `Private/JamCommentaryData.cpp` | `FJamCommentaryLine`, `FJamCommentaryBank` (strict parse of hokej.commentary/1, index by event and id, the embedded copy) |
| `Game/Source/JamGame/Public/CommentarySystem.h`, `Private/CommentarySystem.cpp` | `FJamCommentaryPicker` (engine-free rules), `UJamCommentarySystem` (world subsystem: bus, OnSimFrame, MS_commentary, the two loops), console `jam.Commentary`, `jam.say_line` |
| `Game/Source/JamGame/Private/CommentaryTests.cpp` | `Jam.Commentary.BankClean`, `.Selection`, `.Timing`, `.Stoppage`, `.Yield` (headless, fake manifest) |
| `Game/Source/JamCore/Public/JamEventBus.h`, `Private/JamEventBus.cpp` | the seven event names (`Save`, `Post`, `Breakaway`, `Carry`, `PeriodEnd`, `Clock`, `SeasonTake`) |
| `Game/Source/JamCore/Public/JamSim.h`, `Private/JamSim.cpp` | the six sim read-outs (`CheckSave`, `StepReadouts`, the post ring, `period_end`, `clock`); `season_take` comes from the game mode |
| `Game/Source/JamGame/Private/HokejGameMode.cpp` | one hook in `BuildMatch`: `UJamCommentarySystem::OnMatchBuilt` (loops, season_take) |
| `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md` | `game.commentary { enabled, intensity }`; `JamSchemaData.inl` and the Designer data regenerated |

## 2. The rules as implemented (`FJamCommentaryPicker`)

Per sim frame (60 Hz; nothing reads delta time or a clock): every bus event of the frame goes through `Observe`, every `bark` through `ObserveBark`, then one
`Resolve` at the end of the frame (on `UJamPresentation::OnSimFrame`, which the game mode fires after `DeliverEvents`). `Resolve` starts at most one line.

1. **Candidates** for an event: the bank's lines of that `event` whose `kind` is empty or equals the payload's Kind (for `zone_enter` / `zone_exit` the payload
   carries the zone **id**; the picker maps it to the zone **kind** through `game.rink.zones`), whose `team` is `any` or the event's colour (team 0 blue,
   team 1 orange; the scoring team for `goal`, the human's team for `win` and `lose`, the team of `Who` otherwise), whose `when` holds (section 3), whose
   priority is at or above the intensity floor, whose situation is off its cooldown, and that are not among the situation's last three alternates (never the
   same line twice running). A **stoppage line** (events `period`, `period_end`, `season_take`, `win`, `lose`, situation `goal.pause`: 53 lines) is a candidate
   only while the sim is not in `Playing` (goal pause, faceoff countdown, match over).
2. **Choose**: the highest `priority`; on a tie the more specific line (one point each for a kind, a colour, a conditional situation, a situation about the
   human), then one of the remaining at random (the picker's own seeded stream, never the sim's).
3. **Gate**: a line starts only when 150 frames have passed since the start of the last line, the last line has ended (its `seconds` x 60), and no bark hold
   is on. A late quick-play line is **dropped**, never queued (`JAMVO_SKIP ... reason=gate|playing|yield`, verbose log).
4. **Cooldown** per situation (the id without its last number): `cooldown_s` x 60 frames from the last play of any alternate, scaled by the intensity.
5. **Yield**: a `bark` event addressed to the human (every bark but `goal_against`, the faceoff taunt, and an opponent's `goal_for` complaint about a bot
   teammate's goal) closes the booth for 150 frames (the bubble hold) and cuts a playing line with a 0.15 s fade (`JAMVO_CUT`). A bark and an event on the same
   frame: the bark wins, the event is dropped.
6. **Two-beat goals** (B's rule 6): the colour call plays at the goal (priority 5); when the gate reopens and the sim is still paused (within 300 frames), one
   score-state or mood line follows (`goal.for`, `.against`, `.first`, `.tie`, `.lead`, `.comeback`, `.blowout`, `.match_point`, `.long`, `.own`, `.late`,
   `board.bank`), `goal.pause` only when nothing else fits. The second beat follows even when the colour call fell to a closed gate.
7. **Intensity** (`game.commentary.intensity`): `quiet` plays priority 3 and up with double cooldowns; `normal` the bank as written; `loud` half cooldowns.
   The 2.5 s gate and the one-at-a-time rule never change. `game.commentary.enabled` false, or `jam.Commentary 0`, silences the booth; `jam.Commentary 1` forces it on.
8. **Playback**: `MS_commentary` (one `UAudioComponent`, `SetObjectParameter("Line", wave)`, `Volume` 1, `Play`); the wave itself in `SC_VO` when the MetaSound is
   missing. The 340 waves are requested asynchronously when the subsystem comes up so the first line does not hitch. Log per play:
   `JAMVO id=<id> event=<e> frame=<f> dur=<frames> text="..."`.
9. **Director's lines**: none at runtime (R-0021 point 3); the picker knows one speaker.

## 3. Conditions (`when`), the machine reading

| Situation suffix | Holds when |
|---|---|
| `.blue` / `.orange`, `.<kind>`, `.any`, plain | the team / kind / event matched (nothing more) |
| `.human`, `knockdown.human`, `breakaway.human` | `Who` is the human's skater |
| `pass.to_human` | Kind is `to_<human index>` (the sim's pass event carries the receiver) |
| `.for` / `.against` | the event's team is / is not the human's |
| `shot.hard`, `check.big`, `save.big` | Strength >= 0.8; `board.hard` >= 0.7; `shot.soft` < 0.3 |
| `shot.long` | the shot's x is in the shooter's own half; `goal.long`: the shot that went in (within 180 frames) was |
| `shot.close` | within 4.5 m of the attacked goal line and 2 m outside the mouth |
| `board.rim` | behind a goal line; `check.boards`: within 2 m of the boards |
| `goal.first` | the score was 0:0; `.tie`: equal after; `.lead`: tied before, ahead after; `.comeback`: the scorer was down by 2 or more; `.blowout`: lead of 3 or more; `.match_point`: first_to and one goal from the win; `.own`: the last touch was the conceding team; `.late`: 600 frames or fewer left in a timed take |
| `shot.human_streak` | the human's team's third shot in 600 frames |
| `board.bank` | a goal within 60 frames of a board hit (evaluated at the goal) |
| `faceoff.after_goal` / `.after_foul` | a goal / a foul since the previous drop; `faceoff.early`: the loser's `DrawFrame` is -2 |
| `check.strip` | the puck's previous holder was an opponent within 2 m of the contact and no longer holds it |
| `carry.long` | 30 m or more carried (Strength x rink length); `carry.coast`: Kind `coast`; `carry.protect`: a check within 120 frames and 3 m |
| `pass.blue` / `.orange` | the team's third completed pass in a row (reset when the other team touches the puck); `pass.long`: Strength >= 0.8 |
| `period.take_n` | take 4 or later; `period.take_tied`: take 2 or later with the score tied |
| `period_end.tied` / `.lead` | the score at the horn |
| `clock.last_10_tied` / `.last_10_close` | tied / one-goal gap when 600 frames are left |
| `win.shutout` / `.comeback` / `.close` / `.season` | the loser has 0 / the human's team was down by 2 or more at some point / one-goal margin / the running season take is the ladder's last; `lose.close` / `.blowout` the same margins |
| `directors_call.<key>_up` / `_down` | Kind = the key; Strength >= 0.5 is up |
| anything else | holds (a line B adds later plays once event, kind and team match) |

## 4. The seven events (proposed for C-0003; the Director signs)

| Event | When | Who | Where | Strength | Kind |
|---|---|---|---|---|---|
| `save` | a shot by the other team left within 90 frames, the puck's line still crosses the attacked goal line inside the mouth plus a metre, and the defender about to touch it (blade deflection with an aimed stick, or body) is its team's last man (nearest its own goal line, standing); once per shot | the blocker | the puck | the shot's Strength (exit speed / max) | none |
| `post` | the puck hits a goal post at 3 m/s or more (`CollidePuckCircle` on either post, both nets), at most one per 20 frames | -1 | the post | puck speed / `puck.max_speed` before the hit | none |
| `breakaway` | the carrier has every standing, on-ice opponent behind it (a metre of slack), the net 5 to 45 m ahead, is at or past centre ice and skating at the net (2 m/s or more toward it); once per rush (re-armed when possession changes) | the carrier | the carrier | gap to the nearest defender / 10 m, clamped | none |
| `carry` | one skater has been the carrier (`CarrierIndex`) for 300 consecutive frames; once per possession | the carrier | the puck | metres carried / rink length | `coast` when it started within 10 m of the own goal line and is now past the far blue line (a sixth of the length into the attacking half), else `long` |
| `period_end` | a take ends and the match goes on (the timer crosses a third, or an `end_period` action) | -1 | centre ice (zero) | 0 | `take_<n>` of the take that ended |
| `clock` | 600 frames or fewer left in the current timed take, once per take; and when a tie at the horn plays on | -1 | centre ice (zero) | 0 | `last_10s` / `sudden_death` |
| `season_take` | the game mode built a match whose cartridge is the running season take (`UJamProgression::IsSeasonMatch`), after the `period` event and before the first frame | -1 | centre ice (zero) | 0 | `level_<n>` (1-based ladder index) |

All six sim events are read-outs: the state they keep (`ShotFrame`, `CarryIndex`, `BreakawayFor`, ...) is not hashed and changes nothing in play. The
state hash of a match is the same with and without them (events are not part of `StateHash`). `MS_post` plays through the existing binder; the other six
have no `NS_` / `MS_` yet, so the binder logs one "JAMFX missing" line each at begin play (B may make them, or mark them silent).

## 5. Loops

- `MS_music_match`: one 2D component from `OnMatchBuilt`, `Level` 0.6 at the start, 0.35 while the sim is `Playing`, 0.6 in every other phase (goal pause,
  faceoff countdown, over); set only when the value changes (the sound smooths over 0.5 s). The duck under a line is `SCM_DuckMusic` on B's side. The menu
  map is another world (`MenuGameMode`, not this lane's); its 0.6 is for the menu owner.
- `MS_skate_loop`: attached to the human's skater (re-attached when `LoadCut` rebuilds the actors), `Speed` = |velocity| / `game.skater.speed` every sim frame,
  trigger `Turn` when |yaw rate| passes 3 rad/s at 1 m/s or more, at most once per 15 frames (B-to-A-0007 point 4).

## 6. The content check's two allowances

The commentator register (B's README section 2) runs full sentences in stoppages, and the bark filter caps at 60 characters and bans "death":
`period.take_1.03` (61 chars) and `season_take.level_8.01` (64) are allowed to 64 characters because they are stoppage lines; `clock.sudden.02`
("Sudden death. Do not blink.") is allowed because "sudden death" is the hockey phrase. The check prints these three as INFO and would reject a live-play
line of the same shape. Everything else in the 340 passes the bark filter unchanged.

## 7. Proofs

- Headless: `UnrealEditor-Cmd.exe Game/ThirdJam.uproject -ExecCmds="Automation RunTests Jam.Commentary; Quit" -unattended -nullrhi -nosplash -log`.
- Node: `node tools/commentary/check.mjs`, `node --test tools/commentary/commentary.test.mjs`.
- PIE (editor on MCP port 8018): `jam.ScriptHuman 1`, play 60 s, read `LogJamCommentary` for `JAMVO` lines (spacing >= 150 frames, stoppage lines in a
  pause only, no `JAMVO` start inside a `JAMBARK` hold), then `jam.say_line <id>` on five ids. Results in `status/A32.md`.

## 8. Results (2026-11-12, editor on MCP 8018, 19:43 to 19:47)

Headless: `Jam.Commentary` 5/5 (BankClean, Selection, Timing, Stoppage, Yield); `Jam.Core` 11/11 including `Determinism`, `Jam.Effects` 5/5, `Jam.Rules` 7/7, `Jam.Barks` 3/3
with the read-outs in the sim. Node: check PASS (340 lines, 138 situations, 3 allowances), commentary tests 4/4, schema tests 95/95.

Two 60-second bot matches (`tools/commentary/proof.mjs`, `jam.ScriptHuman 1`, hockey.regular, first_to): run1 17 lines, run2 16 lines (11 picked by the rules, 5 forced by
`jam.say_line`), both pass every check: minimum gap between rule-picked lines 150 frames, no line inside another or inside the 150-frame hold of a bark at the human, every
stoppage line in a pause, 0 missing `VO_Commentator_*`. Excerpt of run2 (`tools/commentary/out/run2/log.txt`):

```
JAMEVENT period ... frame=0 kind=take_1
JAMVO id=period.take_1.01 event=period frame=0 dur=240 text="Welcome to the rink: blue against orange, first period."
JAMEVENT faceoff ... frame=90 kind=drop        JAMBARK who=3 situation=faceoff ... frame=90   (not at the human: no yield)
JAMEVENT breakaway who=3 ... strength=0.112 frame=399
JAMVO id=breakaway.orange.02 event=breakaway frame=399 dur=92 text="Breakaway for orange!"
JAMEVENT goal who=3 ... frame=442                (colour call dropped: 43 frames after the breakaway line, gate closed)
JAMVO id=goal.against.03 event=goal frame=549 dur=88 text="Nothing anyone could do."   (the second beat, at gate + 150, in the goal pause)
JAMEVENT faceoff ... frame=652 kind=drop        JAMEVENT period ... frame=652 kind=take_2  (period.take_2 dropped: 103 frames after the beat)
JAMVO id=lift.clean.03 event=lift frame=719 dur=65 text="Lifted and gone."
JAMVO id=pass.any.01 event=pass frame=905 dur=55     JAMBARK who=4 situation=steal frame=954     JAMVO_CUT id=pass.any.01 frame=955 reason=bark_at_human
JAMEVENT carry who=4 ... strength=0.639 frame=1281 kind=long
JAMVO id=carry.long.02 event=carry frame=1281 dur=60 text="Nobody can catch up."
JAMVO id=lift.clean.02 event=lift frame=1580 ...     JAMBARK steal frame=1628     JAMVO_CUT id=lift.clean.02 frame=1629
JAMVO id=pass.any.02 event=pass frame=1807 dur=78 text="Puck moves quickly."
JAMVO id=board.rim.03 event=board frame=2063 ...     JAMBARK steal frame=2115     JAMVO_CUT id=board.rim.03 frame=2116
JAMEVENT carry who=2 ... frame=2915 kind=long       JAMVO id=carry.long.03 event=carry frame=2915 dur=120
JAMEVENT save who=5 ... strength=0.600 frame=2951   (save.* dropped: 36 frames after the carry line)
JAMVO id=pass.any.01 event=pass frame=3274 dur=55 text="Nice feed."
jam.say_line x5: goal.blue.01 @3387, save.big.02 @3531, breakaway.human.01 @3651, period.take_1.01 @3812, win.shutout.01 @3977
```

Sim read-outs seen in 60 s of first_to play: `breakaway` (x1), `carry` (x4, all `long`), `save` (x1, strength 0.6); `post`, `period_end`, `clock` need a timer cut or a
post hit (none in these two runs; the sim tests cover the code paths only by compiling them, see open question 3). The loops: `JAMVO match built: music=1` in both runs
(MS_music_match playing, Level 0.6 / 0.35 by phase), the skate loop attached to the human's skater; no MetaSound parameter warning in the editor log.

### Open questions for the Director (and B)
1. **A priority-5 call can fall to the gate behind a priority-2 line.** Twice in run2: "Orange scores!" fell because "Breakaway for orange!" had started 43 frames earlier,
   and a save fell 36 frames behind a carry line. B's rule 5 (drop, never queue) is implemented as written; a reading that lets a priority-5 live line cut a lower
   priority quick-play line (the way a bark cuts the booth) would keep the goal calls. The second beat covered the goal in both runs.
2. **Steal barks cut the booth often**: 4 cuts in 60 s (every `steal` complaint is addressed to the human). If the operator hears too many half-lines, the yield could be
   narrowed to barks about the human's own act (check_landed, check_received, signal acks, praise_reply, goal_for by a teammate), leaving `steal` and `stall` to overlap.
3. `post`, `period_end`, `clock`, `season_take` have no `NS_` / `MS_` yet (one "JAMFX missing" line each at begin play; `MS_post` exists and plays). B makes them or the
   binder marks them silent.
4. The worktree checkout carries `BarkData.inl` with CRLF (autocrlf), so `tools/barks/barks.test.mjs` fails there and passes on the main tree; `CommentaryData.inl`'s test
   tolerates CRLF. Not this lane's file; noted for the Director's gate.
5. The eyes-free item is the log only: no movie capture was taken (the editor was shared time on a capped machine); `tools/commentary/out/run2/log.txt` is the record.

## 9. The rough pass (lane A90, device A, 2026-11-13, R-0041 point 4)

Lane A89's sixty lines (`tools/commentary/proposed/a89-lines.json`, voice in `docs/experiments/VOICE.md`) are in the shipped bank as A's second supplement,
`Game/Content/Commentary/rough.commentary.json` (embedded by `node tools/commentary/embed.mjs` into `CommentaryRoughData.inl`; `FJamCommentaryBank::Embedded()` merges it
after the ability supplement: **412 lines, 148 situations**). Ids and situations are A89's, unchanged (they already follow the bank's `<event>.<situation>.<nn>`);
`knockdown.human.04..13` extend B's situation. One priority changed: `check.cross` 4 -> 3 (see the rule). Mild words in the whole bank: **2 of 412 (0.49 %)**
(`check.solid.05` "Hell of a hit.", `check.cross.08` "Damn, ..."), never two in a line; the node test counts it on every gate run.

### The picker rules (`FJamCommentaryPicker`, constants in `CommentarySystem.h`)

| Situation | Holds when | Test |
|---|---|---|
| `check.light` / `check.solid` | the check's Strength < 0.4 / 0.4 to < 0.8 (`check.big` and `knockdown.big` stay >= 0.8; `knockdown.big` needed no code: kind `check` is the knockdown's cause and `.big` the existing rule) | `Jam.Commentary.CheckByStrength` |
| `knockdown.massacre` | a per-team ring of knockdown frames (the knocking team = the victim's opponent, every cause counted); the third inside 1,200 frames. Priority 5 as proposed, so it is must-play | `Jam.Commentary.Massacre` |
| `goal.after_hit` | a goal within 300 frames after a check landed by the scoring team. It is a **second-beat** line: the colour call keeps the goal frame, the payoff follows when the gate reopens in the pause (priority 5 beats the score-state lines there) | `Jam.Commentary.GoalAfterHit` |
| `check.cross` | the sim has no cross-check Kind (A85 owns the sim; no rule added there); the game's cross-check is the stick lift (A49's drill 2, middle mouse). On a `lift` with Kind `clean` the picker offers the `check.cross` lines (event check, kind cross, which no body check's Kind matches) beside `lift.clean`, at the same priority 3; the line logs under `event=lift` | `Jam.Commentary.CrossCheck` |
| **the hit is called** (new timing rule) | a line of a `check` or `knockdown` event may cut a playing line that is not about a hit, of the same or a lower priority, and skips the 2.5 s gate behind one (`JAMVO_CUT reason=hit`); never inside a bark hold, never over another hit's line. A47's must-play and cut-floor rules, A50's random / no-repeat / unheard rules are unchanged | `Jam.Commentary.HitCall` |

Why the last row and the priority change: the first headless pro match (before them) spoke 0 of 6 knockdowns of the human. Every one fell to the gate behind the stick
chatter that led to it (a clean lift's line 7 and 10 frames before, a poke's 50 before) or to a steal / check_received bark hold; pro bots made 111 clean lifts and 98 clean
pokes in 3 minutes, so `check.cross` at priority 4 was the noisiest situation (8 of 41 lines) and is the routine steal, not the bigger knockdown A89 assumed.

### Proof: headless bot matches (`tools/commentary/match.ps1`: `UnrealEditor-Cmd -game -nullrhi`, Rink, `-cut=`, `jam.ScriptHuman 1`, `t.MaxFPS 60`, stopped at frame 10,800)

| Run (out dir under `tools/commentary/out/`) | Cut, bots | Frames | JAMVO | check.light | check.solid | knockdown.big | check.cross | knockdown.human | knockdown.massacre | goal.after_hit | Checks (all knockdowns) / goals |
|---|---|---|---|---|---|---|---|---|---|---|---|
| `pro` (before the hit rule, check.cross p4) | hockey.regular, pro 3 v 3 | 10,775 | 41 | 0 | 0 | 0 | 8 | 0 | 1 | 0 | 6 / 1 |
| **`pro2` (the shipped rules: the 3-minute pro match)** | hockey.regular, pro 3 v 3 | 10,775 | 43 | 0 | 0 | 0 | 8 | 2 | 1 | 0 | 6 / 1 |
| `allstar` | ladder/take-8, all_star 3 v 3 (over at 8,219: orange 3:0) | 8,219 | 33 | 0 | 0 | 1 | 2 | 0 | 1 | 0 | 5 / 3 |
| `bruisers` | modes/hockey.mode.bruisers, 5 v 5 | 10,769 | 33 | 0 | 0 | 0 | 0 | 2 | 1 | 0 | 5 / 1 |
| `drill` (`jam.ctrl.Drill 1`: the human tackles) | hockey.regular, pro (over at 2,950: orange 3) | 2,950 | 19 | 0 | 0 | 2 | 1 | 1 | 0 | 1 | 5 / 3 |
| `bruisers-drill` (`jam.ctrl.Drill 1`) | modes/hockey.mode.bruisers | 10,484 | 37 | 0 | 2 | 0 | 0 | 0 | 1 | 1 | 10 / 2 |

Pro2 in full: poke.clean 5, shot.orange 5, board.rim 4, lift.clean 4, check.cross 8, board.hard 3, pass.any 3, knockdown.human 2, carry.long 2, knockdown.massacre 1, goal.orange 1,
goal.against 1, shot.hard 1, shot.close 1, shot.blue 1, period.take_1 1. The sim gave 111 clean lifts, 98 clean pokes, 6 checks (every one a pro bot knocking down the
scripted human at the end boards, Strength 0.53 to 0.79), 1 goal (orange, no orange check in the 300 frames before). Of the 6 knockdowns of the human: 2 `knockdown.human`
(both by the hit rule, cutting a lift and a cross-check line 10 and 23 frames old), 1 `knockdown.massacre`, 3 dropped inside a bark hold (steal barks 3 and 50 frames
before, a check_received bark on the same frame). The match log is deterministic: `pro` and `pro2` have the same events frame for frame.

Why a situation stayed silent where it did: **check.light** was never spoken in six runs: every bot check lands on the scripted human (`knockdown.human` outranks
the check lines by specificity, VOICE rule 4: when the human is hit the booth winces), and the human's two light checks fell 3 frames inside the gate behind a priority-3
shot line and under the massacre call; the rule is proven by `CheckByStrength` only. **check.solid** speaks when the human lands the hit (bruisers-drill, 2).
**goal.after_hit** needs a goal within 5 s of the scorer's check: none in the plain matches, one in each drill run (as the second beat after the colour call).
`JAMVO_CUT` lines do not appear headless: `CutLine` logs only while the voice component plays, and `-NoSound` plus the unrecorded waves leave it silent; `HitCall` proves the cut.

### New line ids for B's recordings (60 waves, names `VO_Commentator_<id with _>` in `/JamAssets/Audio/VO/`)

`check.light.01..06`, `check.solid.01..07`, `knockdown.big.01..07`, `check.cross.01..10`, `knockdown.human.04..13`, `knockdown.massacre.01..10`, `goal.after_hit.01..10`.
Until a wave exists the line logs and captions with a silent voice (`JAMVO missing ... (logged once)`), as for the ability lines.

### Open for the Director
1. Six `check.cross` lines describe a fall ("sits down hard", "Down he goes", "Sent sprawling", "Bowled over", "the ice caught him", "He did not") but a clean lift knocks
   nobody down. When A85 lands a real cross-check Kind on `check` / `knockdown`, the rule moves there (one line in `Observe`); until then B may record them as they are.
2. Steal barks still hold the booth often (A32 open point 2); one knockdown of the human per match fell to them.
