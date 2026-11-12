# Lane A37: every shipped cut plays (device A, from H11.5)

You are lane A37 of the Third Jam (team Hokej). R-0028 point 3: the 30 offline Designer cuts (`Game/Content/Designer/cached-cuts.json`), the eight season takes
(`Game/Content/Cartridges/ladder/`), and the shipped presets (`hockey.*.cartridge.json`) must each play as hockey: bots score, the human has something to do, the rules end the match,
the look loads. Read first: `contracts/PACK.md`-equivalents here: `schema/README.md`, `contracts/depth.md` (C-0005: effects, rules.hooks, zones, obstacles, variants), `docs/experiments/
LADDER.md`, `Game/Source/JamCore/Tests/{BotTests,SeasonBotTests}.cpp`, `tools/tournament/run.mjs` and the `JamTournament` commandlet, `Game/Source/JamDesigner/Tools/gen-cut-library.mjs`
(how the cuts were made; the `base_sha256` guard), `tools/eyes/README.md`, R-0017, R-0019, R-0021 point 2 (no film words in player-facing text), R-0028.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a37` (detached at the merge commit; build it yourself; headless for the sweeps; one editor on MCP port 8021 only for the
look stills, at most three editors on the machine). Run no git command; the cut library is regenerated only through its generator (A7's tool) with the Director's word; you may
edit single cuts by hand when a rule or a value is wrong, and you say which.

You own: `tools/cuts/**` (the sweep runner and report), `docs/experiments/CUTS.md`, `docs/experiments/img/cuts/**`, `status/A37.md`; `cached-cuts.json` and the ladder cartridges for
hand fixes only (listed in the report).

## Job
1. `tools/cuts/sweep.mjs`: every cut through the commandlet with the real brain at pro and the scripted human, 5 seeds, 36,000 frames max: result, frames, goals, human touches,
   faceoffs without a touch, pickups taken, rule hooks fired, period count, whether the match ended by its own rule; a verdict per cut: plays / ends late / no human touch /
   never ends / loader rejects. JSON plus a markdown table.
2. Fix what fails, by the cheapest honest change: a hook `after_frame`, a timer, a score rule, a zone size; never by removing the cut's idea. Every change listed with before and
   after. A cut whose idea cannot play (the sim lacks it) is marked `deferred` with the missing primitive named for the Director.
3. **Player-facing text**: every cut's `name`, `pitch`, `announce` lines through `tools/barks/filter.mjs` (film words, mild words) and a one-line read: does the sentence say what the
   cut does.
4. **Look**: one still per look tier and per zone kind from a cut that uses it (editor, `jam.playcut <id>`, frozen frame at 24 m), eyes `readability` on each; a cut whose zones or
   pickups are invisible is a bug for the Director (A21's fallbacks).
5. `docs/experiments/CUTS.md`: the table, the fixes, the deferred list, the stills, and three cuts you would add to the library because the sweep shows an empty spot in the
   fun space (sentence + the cartridge keys they would use).

Report: counts (plays / fixed / deferred / rejected), the fixes, the still paths, the three proposals. Time box 2 hours; interim at 45 minutes with the sweep table.
