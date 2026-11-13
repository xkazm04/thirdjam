# B-DECISIONS: the operator's decisions through device B, landed on A (R-0033, lane A50)

Lane A50, 2026-11-13. The operator answered device B's open questions at 22:50 and 23:55 (handoffs B-to-A-0032 and 0033, ruling R-0033). This note says what
each decision became in code, what it did not become yet and why, and how it is tested. Numbers are the ruling's.

## 1. The half-height goal: `game.rink.goal_height_m`

"No dedicated goalie ... we can have smaller goals for this case (height in half)."

| Where | What it reads |
|---|---|
| `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`, `validate.mjs` (Designer-editable) | `game.rink.goal_height_m`, number 0.3..1.5, default **0.61** (a real net is 1.22). Generated data re-run (`gen-schema-data`, `gen-parity`, `gen-designer-data`). |
| `FJamRinkConfig::GoalHeightM` (`JamCartridge.h`), the loader (`JamCartridge.cpp`) | the key; rink geometry, so `ApplyLiveConfig` holds a change until the next match like the width |
| `UJamSim::InGoal(P, Side, HeightM)` (`JamSim.cpp`) | a puck centre at or above the crossbar is over the net, not in it; the two-argument form is the puck on the ice |
| `ARinkActor::Build` (`RinkActor.cpp`) | posts and crossbar, the netting's sloped top, the goal trigger boxes stand `GoalHeightM` tall; the back frame keeps its 0.9 : 1.5 proportion under a lower bar (never above it) |

**What the sim does not have.** The sim's puck is two-dimensional: no height state, no lifted shot; the puck pawn's 0.25 m hop on a shot is cosmetic
(`AJamPuck::PopLiftM`). So the step passes the ice height to the goal test and the match hashes do not move (the Determinism baseline is untouched; verified by
the test's scored match and by the suite). The height-aware test is in place for the day a lifted shot enters the sim; the brain has no shot height either, so
"the bots' aim follows" has nothing to read yet. Both are one key away: the lifted shot's height, and the bots' aim reading `GoalHeightM` from the world.

Test `Jam.Core.GoalHeight`: a puck 0.8 m over the line is no goal at 0.61 and a goal at 1.22; at the bar is no goal; on the ice a goal at both; a scored match still
scores; the key loads at 1.22; 0.2 and 2.0 are refused with the pointer `/game/rink/goal_height_m`.

## 2. The goalie stance for any crease skater, the human included

"Last and any player in the team acts like a goalie to cover space and with body and stick."

`JamAI::CreaseStance(W, Id, OutSide)` (`JamAICore.h/.cpp`): skater `Id` stands within 3 m of its own goal centre on the ice side of the goal line, is not down and
not the carrier, and an opposing carrier is inside its blue line (the outer third on its side, `LastManGoalie`'s rule). Side as the back's goalie intent (where the
carrier is, seen from the goalie facing up-ice; centre within a metre). The brain applies it to every bot after `Decide` when the back's rule did not already set
the stance: **steering untouched**, the visual intent only. The game mode (`GatherInputs`) applies the same function to the human on the world the brain just read,
and sends `SetGoalieStance` for every skater through one path with A31's hold (`GoalieHoldFrames`) and the change-only send. `JAMGOALIE` lines now carry
`role=player|back|support|chaser`, so a match log shows the stance on more than the back.

Test `Jam.Core.CreaseStance`: a support in its crease takes the stance with the right side; the back 21 m out, the carrier, a far skater, a carrier outside the blue
line, a skater behind the goal line, a carrying skater and a frozen world do not; through `FJamBrain::Think` the support's output says goalie.

## 3. Commentary: random among the alternates, never the last, prefer unheard

"You can use full mix and randomly change the lines to prevent repetition. All accepted."

The picker already drew at random among the top-priority, most specific candidates and skipped a situation's last three alternates (A32, A47). Added
(`CommentarySystem.cpp`): `HeardThisMatch` (every line id started this match); `ChooseAmong` prefers the top candidates not in it and falls back to all of them
when every one has been heard. The subsystem salts the cut's seed with a per-process match counter and the clock, so two matches of the same cut do not open
with the same line (B-to-A-0032 acceptance); cosmetic only, never the sim's stream; the tests seed the picker directly.

Test `Jam.Commentary.Variety`: a bank of eight alternates of one situation; for three seeds the first eight hits hear all eight once, the next 24 never repeat
the last line and use more than two alternates; the seeds do not all open with the same line.

## 4. Crowd instead of horns: `MS_goal` Variant and the crowd bed

"Goal horns are horrible, rather create variants of crowd cheering."

- `MS_goal` plays `Variant` **0 when the human's team scores, 1 when it concedes** (`EffectsBinder.cpp`). The goal payload carries `Who` = the last touch (the
  wrong team on an own goal) and no team, so the binder resolves the scorer the way the commentary picker does: the team whose score grew since the last
  goal it saw (`ResolveScoringTeam`), the team of `Who` as the fallback, Variant 1 when nothing tells. Logged `JAMFX goal scorer_team= human_team= variant=`.
  Test `Jam.Fx.GoalVariant`.
- `MS_crowd_bed` (`/JamAssets/Audio/MS_crowd_bed`) starts with the music at match start when the asset exists (a quiet load, no "missing" line until B says
  it landed); `JAMVO match built: ... crowd_bed=0|1` says whether it did (`CommentarySystem.cpp`).

## 5. The clay row

B-to-A-0033 point 1: `humanoid_b1` x `p3_clay` -> `BP_SkaterVisual_TB1_P3`, falling back to `BP_SkaterVisual_TB1`; every other tier `BP_SkaterVisual_TB1`. The clay row
was already in `ResolveVisualClass` since A47 (B-to-A-0030); A50 drops the probe of a `BP_SkaterVisual_TB1_P4` that does not exist (one warning line per spawn
on the broadcast lab tier), per the accepted table. `Jam.Bot.VisualSwap` re-run.

## 6. The sprint on the wire

Done on main by the Director before this lane (protocol v4, `bSprint` in `FJamNetQuantInput`, 10 wire bytes, 16-bit mask). A50 only re-ran
`Jam.Net.InputCoverage`, `Jam.Net.PacketRoundTrip`, `Jam.Net.LockstepTwoPeers`.

## Open

- A lifted shot in the sim (a puck height on a charged or deflected shot) would make the half-height net bite in play; today the net is half-height on screen
  and in the triggers, and the sim scores every puck that crosses the line on the ice. The operator's intent ("smaller goals") is then partly visual until that lands.
- B's goalie kit (pads, mask) stays in Blender; the stance and block poses are B's figure's (unchanged).
