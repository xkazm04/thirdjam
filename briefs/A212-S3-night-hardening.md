# Lane A212, track S3 (hardening), the night: the changed game does not crash, desync or lie, and a package can be cut (device A, Opus)

You are lane A212 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0066 to R-0068** at the end of `contracts/RULINGS.md`, the track's `status/A198.md` and `docs/experiments/HARDENING-5.md` as they stand in
your tree (yesterday's suite: the soak plan, the input fuzz `Bar.Hard.InputFuzz`, the bindings-file tests, the menu fuzz, the two-process lockstep
driver, the GPU-start counts), lane A205's freeze check (`.claude/worktrees/c0b/status/A205.md` may be gone: the facts are in R-0066 and
`docs/PLAY-ME.md`), and what tonight's lanes changed in the sim (their READY sections: `status/A206.md`, `A207.md`, `A208.md`, `A209.md` on main).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A212.md`. Port 4812 (two-process
runs 4813 / 4814). Family: `tools/soak/**`, `tools/fuzz/**`, `Game/Source/JamGame/Private/HardTests.cpp`, the soak tests in
`Game/Source/JamNet/Private/Tests/JamNetSoakTests.cpp`, `docs/experiments/HARDENING-6.md`. **You fix nothing in another lane's family: you find,
reduce to the smallest repeat, and write a `FAULT` line the minute it is reduced** (the Director's watcher reads it). **Nothing of yours runs from
the main checkout** (never start an editor on main's `ThirdJam.uproject`: a lane did last night and broke a landing). Loopback only for two
processes; never answer a network or security prompt; a hosting lobby still (OS user name, LAN addresses) never leaves your worktree.

## Tonight the sim changed more than in the whole of yesterday: prove it holds
New since H34: a pass in flight is puck state (`PassFrames`, `PassBy`, soon `PassTo`), the pass's power by hold, the shot twice as fast (the puck
at up to 52 m/s: tunnelling?), the pickup by arc and body with a swept puck, the check's pull and the cross-check on any man, the human's 8-frame
wind-up, the brain's evolution 9. More lands every hour until about 07:00.
1. **Land your own suite first** (it was held out of the freeze landing: the patch is `C:\Users\kazda\AppData\Local\Temp\tj\a198.patch`; your tree
   carries its content already if the Director says so): make `Jam.Hard.*` and `Bar.Hard.*` build and pass on tonight's main (the bindings and
   rebinding tests against the fixed code; the input fuzz with the pass's hold and every new key), drop the copy of `twoproc5.ps1` (main has the
   net lane's) and the committed run outputs; write `SUITE READY` with the file list: the Director lands it.
2. **Soak and determinism on every new main** (the Director moves your tree after each batch of landings): 60 cells x 100 seeds twice: 0 crash,
   ensure, error or NaN line, 60 of 60 bit-identical; then the input fuzz at 600 with the new verbs' holds; a puck-escape scan (the puck outside
   the boards, inside a net's wall, inside a body for more than 2 frames, NaN position) over the soak's rows: the count and the first seed.
3. **Lockstep with the new state**: protocol 9 in one process (clean, 30 % loss, jitter) with the brace AND held passes on: 0 desync; then two
   processes on loopback, a full match with passes, pickups, checks: hashes equal at every checkpoint; a guest who joins with a different build is
   refused with words. `Jam.Net.HashCoverage` covers `PassFrames` / `PassBy` (/ `PassTo`): prove a deliberate mismatch in each is caught.
4. **The frame**: clay and neon, 5 v 5, the game camera, with B's streak and the new marks: mean and worst frame against H34's numbers (17 ms mean,
   19 ms worst at a goal); the first PLAY's hitch.
5. **The release rows**: read `tools/qa/accept/` as it is on main (step 5's held row is now `pass_power`; the ship row presses Return twice) and
   the acceptance drill: for every row say whether tonight's rules still let it pass headless (the pass lane proved steps 4 to 6 in the sim: do the
   same for steps 1 to 3 and 7 to 10 with a scripted press through the mapping layer): a red row found here saves a failed release.
6. From 06:00: 40 starts of the newest Shipping package off screen, the menu fuzz on its Development twin for 10,000 steps, and the bindings file
   through a restart.

One table in the status file, refreshed after each item: the run, PASS or the fault with its repeat. `SUITE READY` within 90 minutes; `HARD READY n`
after each new main; `HARD READY final` at 09:15. Four processes at most. Time box: the night.
