# Lane A39: three payoffs: the one-timer, the breakaway burst, the deke (device A, from H11.7)

You are lane A39 of the Third Jam (team Hokej). Ruling R-0029 point 4: lane A34's playtester data shows a carry has no payoff, passes are rare because the back fills the lane while the
carrier charges, and shots on target die on the crease back. Read first: `docs/experiments/PLAYTEST.md` (sections 3 and 6: the numbers and the three ideas), R-0029, R-0013, R-0018,
`docs/experiments/FEEL.md`, `contracts/depth.md` (C-0005), `Game/Source/JamCore/Public/JamSim.h` and `Private/JamSim.cpp` (pass reception, `TryShoot`, charge, the carrier, the last
man), `Game/Source/JamCore/AI/JamPlaytester.cpp` and `JamAICore.cpp` (how the playtester and the brain decide to shoot and pass), `schema/README.md` and `DEFAULTS.md` (keys under
`game.shot`, `game.puck`, `game.skater`), `Game/Source/JamCore/Private/Tests/JamFeelDepthTests.cpp` (the test style).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a39` (the Director creates it at the current main; build it yourself, unity off; headless only; no editor). Run no git
command. Every change is a calibration under R-0021 point 1: deterministic, a cartridge key with a default, recorded in FEEL.md, the Determinism baseline re-recorded if play
changes from frame 1; the playtester grid (`tools/playtest/run.mjs`) before and after is the proof.

You own: `JamSim.cpp` / `JamSim.h` (the three mechanics), `JamCartridge.*` and `schema/` for the keys (`game.shot.one_timer { window_frames, cone_bonus_deg }`,
`game.skater.breakaway { speed_bonus, frames }`, `game.stick.deke { offset_m, commit_frames }`), `JamPlaytester.cpp` and `JamAICore.cpp` (the playtester and the bots use the new
verbs: a deke input `bDeke` on `FJamInput`, serialized for lockstep: say so to the Director for lane A38's checklist), `Private/Tests/JamPayoffTests.cpp` (new), `docs/experiments/
PAYOFFS.md`, `status/A39.md`.

## Job
1. **One-timer**: a shot released within `window_frames` (10) of receiving a pass gets the full charge and `cone_bonus_deg` (8) wider aim tolerance; the event `shot` carries
   `Kind = one_timer`. Test: a scripted pass and shot inside the window scores from 12 m where the same shot outside it is saved by the back.
2. **Breakaway burst**: once the carrier is past the last defender with the net ahead, `speed_bonus` (0.2) for `frames` (60), once per possession; event `breakaway` (lane A32 may
   have added it: reuse). Test: the burst fires once and the carrier reaches the slot before the back.
3. **Deke**: `bDeke` tap moves the puck `offset_m` (1.0) to the blade side within 6 frames and the crease back commits for `commit_frames` (15) (the brain's goalie stance follows
   the puck's old side); a second tap inside 30 frames does nothing. Test: a deke against the last man opens the far side in 2 of 3 seeds.
4. The playtester and the bots use all three (the good player always, the regular sometimes, the novice never; all_star bots use the one-timer and the deke, pro the one-timer).
5. Grid before and after (`tools/playtest/run.mjs`, 20 seeds, regular rink, three players x three levels): the fun metrics must move (shots on target, goals, carries with a payoff)
   and the curve must not leave R-0029's targets by more than 5 points; if it does, propose the profile offset, do not apply it.
6. `docs/experiments/PAYOFFS.md`: the mechanics, the keys, the tests, the before/after grid, the Designer sentence each payoff needs (`DESIGNER_GAME_EDITABLE` note for A7).

Report: the grid before and after, the tests, the file list, the lockstep note. Time box 2 hours; interim at 45 minutes with the one-timer measured.
