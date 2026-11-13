# Lane A66: `Jam.Bot.SeasonTakes` fails on take-3 and take-4 since A51 merged: root cause and fix (device A, hotfix, Opus)

You are lane A66 of the Third Jam (team Hokej). The Director merged lanes A51 (stick handling: the carried puck lives on the blade and follows the aim within
`game.stick.reach_m`, a sweep shields it from a poke, the opposing bots read a carried puck `game.stick.sweep.bot_react_frames` late by level, a spin on E), A52 (the
puck's height, already on main), A55 (static analysis fixes: math helpers moved into `JamAI::AIMath` in `JamAICore.h`) and two report lanes. The gate is red on one test:

```
FAIL take-3 (Rough Cut) seed 1: 3:1 in 4770 frames, WIN, faceoffs 5 without a touch 0, AI shots 0, worst board -0.400 m, hash 491548d3643fd493 same
FAIL take-4 (Set Pieces) seed 1: 3:1 in 5033 frames, WIN, faceoffs 5 without a touch 0, AI shots 0, worst board -0.390 m, hash c148b401649dea07 same
```

The clause that fails: the AI must shoot at least once in a match of a minute or more (`Game/Source/JamCore/Tests/SeasonBotTests.cpp` line 55-58). Both takes run **pro**
bots with `rules.penalties: true` (`Game/Content/Cartridges/ladder/take-3.cartridge.json`, `take-4.cartridge.json`; take-4 adds four pillars and a timer with a
first-to-three hook); the other ten take x seed cells pass, and `Jam.Bot.ProCharge` (27 of 37 shots charged on hockey.regular pro) passes. The same test was green on
main 9d2e66c (before A51); it went red with A51 + A55 applied. The Director's reading, yours to prove or refute: the pro bots never shoot because the carried puck's new
position (on the blade, up to `stick.reach_m` 1.2 m from the body, following the bot's stick aim) or the carrier rule (`CarrierIndex` carries inside the stick's reach)
breaks one of the brain's shot conditions (`D < W.ShotReach`, `bFaceOk`, `S.bCanShoot`, the charge hold in `JamAICore.cpp` around lines 300-322 and 696-730), or the bots'
delayed read (`PuckSeen`) feeds the carrier's own decision; the penalties rule may matter (a foul call resets something). A second reading: the human won 3:1 with the AI
goal counted without a shot (a deflection), so the bots did reach the net.

Read first: `docs/experiments/STICK.md` (A51's model and its own test results), `Game/Source/JamCore/AI/JamAICore.cpp` (`RecordPuck`, `PuckSeen`, `Think`, `Attack`,
`StickPlay`, `ChargeWindowFor`), `Game/Source/JamCore/Private/JamSim.cpp` (`CarrierIndex`, `StepStick`, the carried puck's offset, `TryShoot`), `JamSimAdapter.cpp`,
`Game/Source/JamCore/Tests/SeasonBotTests.cpp`, `Game/Source/JamCore/Tests/BotTests.cpp`, `Game/Source/JamCore/Private/Tests/JamStickHandlingTests.cpp`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a66` (detached at main 9d2e66c **with the whole merged tree applied uncommitted**: build it yourself, unity
off; the editor binary there is stale until you build). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not
start. Open no interactive editor: automation runs headless the way `tools/gate/premerge.ps1` does; start with the filter `Jam.Bot.SeasonTakes` alone (about 2 minutes),
then `Jam.Core+Jam.Bot+Jam.Playtest+Jam.Net+Jam.Tackling` before you report.

You own: the fix in `JamAICore.cpp` and/or `JamSim.cpp` (the smallest change that restores the pro bots' shot while keeping A51's five tests, `Jam.Bot.ProCharge`,
`Jam.Bot.PassesToHuman`, `Jam.Playtest.NoviceScoresAtRookie`, `Jam.Bot.HumanHasSomethingToDo` and `Jam.Core.Determinism` green), a test if the cause had none
(`Jam.Bot.ProShootsWithPenalties` or the existing SeasonTakes), a section 8 in `docs/experiments/STICK.md` with the cause and the fix, `status/A66.md`.

## Job
1. Reproduce: run `Jam.Bot.SeasonTakes` headless on your build; add a temporary log line per bot per 600 frames in take-3 seed 1 (carrier, distance to the puck, facing
   dot, bCanShoot, the charge hold, the window) to see which condition holds the shot; remove the line before you report.
2. Fix the cause. If the cause is a calibration (a reach, a frame count), change the key's default and say so; if it is a logic fault (a delayed read used by the carrier,
   a carrier rule that drops the puck off the shooter), fix the logic.
3. Run the suites named above; paste the SeasonTakes lines for take-3 and take-4 seed 1 with the AI shots count.
4. Report the cause in two sentences, the diff summary, the test results, the hash if it moved.

Time box 45 minutes. Say at 20 minutes what the condition is, even before the fix.
