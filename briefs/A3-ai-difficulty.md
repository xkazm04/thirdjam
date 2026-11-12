# Lane A3: three AI difficulties and balancing sweeps (device A, from H5)

You are lane A3 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. The operator wants three levels of AI difficulty. Lane A2 is finishing the bot brain
(`Game/Source/JamCore/AI/**`, `tools/tournament/**`, owner A2 until it reports) and the headless tournament harness; you own the difficulty layer on top of it and the balancing
sweeps. Read first: `GAME.md`, `docs/HOCKEY.md` (2 AI row, 4 `game.ai`, 7 tests), `briefs/A2-ai.md`, `Game/Source/JamCore/AI/JamBrain.h` and `JamAICore.h` (read-only until A2 reports;
check `status/A.md` and the repo for A2's commit), `schema/game.schema.json`, `schema/limits.json`, `schema/README.md`, `Game/Content/Cartridges/*.json`, `tools/skills/skating-physics.md`.

You own: `schema/**` and `Game/Content/Cartridges/**` for the difficulty fields only (lane A4 is finished), `Game/Source/JamCore/AI/Difficulty*` (new files), `tools/sweep/**`,
`docs/experiments/BALANCING.md`. Do not edit A2's files; if the brain needs a hook you cannot add in your own files, write the exact patch into your report for the Director to apply.
Run no git command; headless work only (automation tests and the tournament commandlet; MCP port **8009** only if the editor is unavoidable). Node 24, zero dependencies.

## Deliverables
1. **Schema**: `game.ai.difficulty` enum `rookie | pro | all_star` (absent = `pro`), kept in the limits mirror, the Designer MAY change it (add to `DESIGNER_GAME_EDITABLE` and the system
   prompt: "make it easier / harder" maps here), tests; `game.ai` keeps its explicit numbers as overrides on top of the difficulty profile.
2. **Profiles** (`FJamDifficultyProfile`, data in C++ with the numbers also written to `docs/experiments/BALANCING.md`): per level `speed_scale`, `red_skill`, reaction delay in frames
   (input applied N frames late), shot accuracy (aim error degrees), yield margins (rookie yields more to the human), shot range, stall tolerance, and for `all_star` a lead/intercept
   improvement. Resolution order: profile -> explicit `game.ai` numbers -> Designer values.
3. **Sweeps** (`tools/sweep/run.mjs` driving A2's tournament commandlet once it links; until then, drive A2's headless test harness if it exposes one, else prepare the scripts and the
   analysis against the JSON lines format in `briefs/A2-ai.md`): for each difficulty x {120 x 60 speed 8, 90 x 45 speed 11} x 20 seeds: human-bot vs AI win rate, goals per match, shots,
   touches by the human, match length; the target curve: rookie = human wins about 85% with >= 6 touches per minute, pro about 55%, all_star about 25% but the human still gets >= 4 touches
   per minute (`HumanHasSomethingToDo` holds at every level). Tune the profiles until the curve holds; report the final table.
4. **Tests**: `DifficultyProfilesResolve` (each level resolves to its numbers; explicit `game.ai` overrides win), `DifficultyCurve` (a short tournament asserts rookie wins more than all_star
   at seed 1-5; mark slow).
5. The rink-size evidence the operator asked for: the same sweep table on 120 x 60 vs 90 x 45 at `pro`, with your reading in three sentences.

Report: the profile numbers, the sweep table, test results, every error, the exact patches A2's files would need. Time box 3 hours; report at 60 minutes with the state either way.
