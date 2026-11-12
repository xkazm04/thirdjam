# Lane A44: the Designer learns tonight's keys and gains three cuts (device A, from H12.4)

You are lane A44 of the Third Jam (team Hokej). The Designer (a sentence becomes a cut, offline library for judges, CLI transport for development) is the theme's proof; tonight the game
gained keys it does not know: `game.shot.one_timer`, `game.skater.breakaway`, `game.stick.deke` (A39), `game.commentary` (A32), `game.camera.boards_margin_m / end_height_m / far_net_bias`
(A40), `game.stick.foul_grace_frames` (A16), `look.trim_emissive` and tier `p4_broadcast` (A21, A31), the events `breakaway`, `save`, `post`, `carry`, `period_end`, `clock`,
`season_take`, `deke`. Read first: `Game/Source/JamDesigner/**` (the prompt builder, `DESIGNER_GAME_EDITABLE`, the validation chain, `Tools/gen-cut-library.mjs` with the `base_sha256`
guard, `Tools/gen-designer-data.mjs`), `docs/experiments/CUTS.md` (A37's three proposed cuts and the empty spots in the fun space), `docs/experiments/PAYOFFS.md` section 2 (the
Designer sentences A39 wrote), `docs/experiments/COMMENTARY.md`, `contracts/depth.md` (C-0005), `schema/README.md`, R-0004 (CLI transport), R-0017, R-0021 point 2 (hockey words),
R-0029, `tools/barks/filter.mjs` (every player-facing string passes it).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a44` (the Director creates it at the main that carries A39's payoffs; build it yourself, unity off; headless; the CLI
transport uses the operator's Claude Code subscription: `claude -p --model claude-haiku-5-5` as `gen-cut-library.mjs` does, at most 40 calls). Run no git command.

You own: `Game/Source/JamDesigner/**`, `Game/Content/Designer/**` (the library, regenerated only through the generator), `docs/experiments/DESIGNER-2.md`, `status/A44.md`.

## Job
1. **Editable keys**: add the new keys to `DESIGNER_GAME_EDITABLE` with their ranges, a one-line meaning each in the system prompt (hockey words only on anything a player reads), and
   a paraphrase test per key family (`Jam.Designer.*`): "make one-timers deadly", "give breakaways a burst", "let me deke the last man", "a quiet commentator", "camera higher at the
   end boards", "no cheap fouls after a strip", "the broadcast look" each produce the right key with a sensible value and nothing else changed.
2. **Three new library cuts** from A37's proposals (`no_shoot` in front of both nets "carry it in"; `goal_against` boost "help whoever is losing"; `goal_shield` x2 on a clock with
   first-to-three) plus one per payoff ("One-Timer Night", "Breakaway Cut", "Deke Cut"), written as sentences for the generator; the library regenerated to 36 cuts with the base guard,
   every cut through `tools/cuts/sweep.mjs` (A37's) and the text check; names and pitches hockey first.
3. **Repair chain**: the haiku -> haiku repair -> sonnet chain stays; measure first-try validity on the 36 (A7 had 24 of 30) and the token cost; if the prompt grew past 70 KB,
   trim the schema excerpt to the editable subset.
4. Tests: `Jam.Designer.*` green headless; `gen-designer-data` PASS; node tests.

Report: the key table, the six new cuts with their sweep verdicts, first-try validity and cost, the file list. Time box 90 minutes.
