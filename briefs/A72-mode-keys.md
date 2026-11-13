# Lane A72: the cartridge keys the modes asked for (device A, from H15, R-0036, Opus)

You are lane A72 of the Third Jam (team Hokej). Lane A67 shipped six Director's modes (`Game/Content/Cartridges/modes/`, `docs/experiments/MODES.md`) from the keys that
exist and listed eleven asks: keys that do not exist and would let a mode's character go further. Your job: the keys worth having, end to end (schema, limits, DEFAULTS,
README, loader, sim or brain, a test each, Designer data regenerated), then the modes re-measured with them.

Read first: `docs/experiments/MODES.md` (the asks A1-A11 with what each would allow), `Game/Content/Cartridges/modes/*.cartridge.json`, `tools/playtest/modes.mjs` (the
measurement), `schema/README.md`, `DEFAULTS.md`, `limits.json`, `game.schema.json`, `Game/Source/JamCore/Public/JamCartridge.h` + `Private/JamCartridge.cpp` (the loader,
the live keys), `Game/Source/JamCore/AI/DifficultyProfile.cpp` + `DifficultyResolve.cpp` (how `game.ai.*` keys override a level: `ai.speed_scale`, `ai.red_skill` are the
pattern), `JamAICore.h` (`FJamBotSkill::ReactionFrames`, `CheckMinClosing`), `Game/Source/JamCore/Private/JamSim.cpp` (`puck.count` is loaded and unread; the timer; the
penalty box frames), `Game/Source/JamCore/AI/JamPlaytester.cpp` (the classes; the sprint is never pressed), `docs/experiments/CONTROLS.md` (the sprint), R-0034, R-0036.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a72` (created at the current main; build it yourself, unity off). Run no git command; never touch
`Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Open no interactive editor (everything headless). Unknown keys are errors; absent
value = omit the key; a sim change is a calibration with the hash before and after written. Lane A70 re-tunes the difficulty profiles at the same time: do not change a
profile's numbers; your keys are overrides a cartridge may set.

You own: the keys below in `schema/` + `JamCartridge.*`, their readers in `DifficultyResolve.cpp` / `JamAICore.*` / `JamSim.cpp` / `JamPlaytester.cpp`, their tests
(`Jam.Core.ModeKeys`), the re-measured modes (`tools/playtest/out/modes-keys.md`), `Game/Content/Cartridges/modes/*.json` where a mode now uses a new key,
`docs/experiments/MODE-KEYS.md`, `status/A72.md`.

## Job, in this order (stop where the time box ends; each key lands whole or not at all)
1. **A4 `ai.react_frames`** (0..60): the opposing bots' reaction delay from the cartridge (the resolver's override of `Opp.ReactionFrames`), and **A5 `ai.body_check`**
   (0..1: 0 never, 1 the level's rule): Blind Side and Cage want both.
2. **A1 `rules.timer_frames`** maximum 36000 (ten minutes), with the HUD clock checked at 10:00; Marathon moves to ten minutes.
3. **A2 `ai.sprint`** (0..1 share of the time a bot sprints when chasing) and **A3** the regular and good playtester classes press sprint on a breakaway or a chase (a
   tooling change; the novice never), so sprint frames can be measured; Marathon's number becomes sprint frames per match.
4. **A8 `rules.penalty_frames`** (60..600): how long a foul sits; Cage uses short penalties.
5. **A10 `puck.count`** (1..3) wired into the sim: a second and third puck step, carry, score and reset like the first (the carrier logic per puck; the brain chases the
   nearest); a mode "Three Pucks" if it plays; Determinism re-recorded. If this is more than 40 minutes of work, write the design and stop here.
6. **A7 `rules.offside` true** (the blue line rule; a whistle and a faceoff) only if time remains.
7. Every landed key: schema + limits + DEFAULTS + README + loader + reader + a `Jam.Core.ModeKeys` case; `gen-schema-data`, `gen-parity`, `gen-designer-data` re-run;
   the modes that use a new key re-measured with `tools/playtest/modes.mjs` (20 seeds) and the numbers in `MODE-KEYS.md`.

Report: the keys landed with their tests, the modes' numbers before and after, the hash if it moved, the file list. Interim at 45 minutes with steps 1-2. Time box 120 minutes.
