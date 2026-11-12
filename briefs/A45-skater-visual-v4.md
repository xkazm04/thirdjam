# Lane A45: C-0001 v4: B's four tackling calls, the down time on the visual, and bots that check (device A, from H12.4)

You are lane A45 of the Third Jam (team Hokej). Ruling R-0031 in `contracts/RULINGS.md` and device B's handoff `handoff/B-to-A-0028.md` (on main after the next merge; else read it with the
read-only `git show origin/dev-b:handoff/B-to-A-0028.md`). Read first: R-0031, B-to-A-0028, `contracts/skater-visual.md` (C-0001 v3), `docs/experiments/TACKLING.md` (A33's read-outs:
`check` Kind and Strength, the skater `board` event, the stumble), `Game/Source/JamGame/Private/JamPawns.cpp` (`ApplySimFrame`: where the v3 calls are made), `Public/JamSkaterVisual.h`,
`Private/SkeletalSkaterVisual.cpp` and `ToySkaterVisual.cpp` (`ReactionFor`), `Game/Source/JamCore/Private/JamSim.cpp` (`ResolvePokes`, `TryLift`, `CollideSkaters`, the board event),
`Game/Source/JamCore/AI/JamAICore.cpp` (the bots' `bCheck`, forced false per R-0017 point 4), `AI/DifficultyProfile.*`, R-0013, R-0023, R-0029.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a45` (the Director creates it at the main that carries the dev-b merge; build it yourself, unity off; one editor on
MCP port 8025 only while proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; the visual calls are cosmetic (Determinism and
VisualSwap unchanged); the bot-check change is a calibration (R-0021 point 1) that moves play: re-record the Determinism baseline, run the playtester grid before and after
(`tools/playtest/run.mjs`, 20 seeds, regular rink) and report the curve move for A3.

You own: `JamSkaterVisual.h` (the v4 calls), `JamPawns.*` (the triggers), `SkeletalSkaterVisual.*` and `ToySkaterVisual.*` (defaults that map to existing reactions), `JamSim.cpp`
only for the read-outs the triggers need (a `pin` state or the poke/lift victim in the event payload), `JamAICore.cpp` + `DifficultyProfile.*` for the bot check (`bCheck` at pro and
all_star under R-0031 point 2's conditions; a profile knob `opp.check_min_closing` and `opp.check_from_behind` false), `contracts/skater-visual.md` (v4 text, appended), `docs/experiments/
TACKLING.md` (section 8), `status/A45.md`.

## Job
1. The four calls and the down time on `SetKnockedDown`, with the triggers from the sim read-outs as B listed them; the toy and skeletal figures map them onto their existing
   reactions (nothing new to author).
2. Bots check at pro and all_star: front or side only, never from behind (the check resolves as a foul there and R-0013 parks the fouler, so the brain must not try), only above
   `check_speed_min` closing, only on the carrier; rookie never. `Jam.Bot` and `Jam.Playtest` gates green in the worktree; the grid before/after reported, not tuned.
3. Proof: a bot-only match on `humanoid_b1` (B's figure) with the log showing `SK4KD`/`SK4UP` lines (B's) and the new calls fired (`JAMVIS hit|board|pin|stickhit` lines you add),
   8-frame strips of one bot check landing on the human and of a board pin, eyes `bug-hunt` on both.
4. Hook for B's glass `HitWobble` in the effects binder (12 frames on the nearest panel's dynamic instance, guarded by a parameter-exists check).

Report: the v4 text, the trigger table, the grid move, the strips, the file list. Time box 75 minutes.
