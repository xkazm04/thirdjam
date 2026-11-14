# Lane A152, track F2 (carrier skill), evolution 5 (device A, Fable)

You are lane A152 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (laws 3 and 4), R-0053 to R-0059 at the end of `contracts/RULINGS.md` (your worktree's copy is the newest), the design eye's second review
`docs/design/REVIEW-2.md` (its item 3 is your evolution; items 2 and 6 are your track's too) with `docs/design/review-2/` (its scripts: reuse them), and the
track's docs `docs/experiments/MOVEMENT-2.md` to `MOVEMENT-5.md`, `status/A147.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main. It was built
before: an incremental build is enough. Your status file: `status/A152.md`. Port 4752. Family as the track's: the movement and stick functions in
`JamSim.cpp/.h` (`TryPoke`, `ResolvePokes`, `IsPokeShielded`, the miss's stumble), the `game.skater` and `game.stick` keys, the carry drill, `Jam.Core.Movement.*`
and the stick tests, `JamPlaytester.cpp` (the scripted human's poke decision only), `docs/experiments/MOVEMENT-6.md`.

## Evolution 5: Space is worth pressing
The review measured, on the ladder's commit: **a poke takes the puck 2.6 / 4.1 / 4.0 % of the time** at rookie / pro / all_star (8 / 10 / 14 % on Friday), and
**the human's 6.3 stumbles a match are his own missed pokes** (with `stick.stumble_frames 0` they fall to 0.3 at rookie and 0.8 at pro). Tonight's rules made
the puck hard to take on purpose (an unpressed blade no longer takes it; the body shields it; half a second of grace after winning it), and the one verb left
for taking it by the stick now fails 24 times in 25 and trips the player who tries. The operator will press Space at 10:00.
1. **Trace it first**: 200 of the human's pokes at pro (the review's scripts, or the playtest line with a trace): for each miss, why: out of reach, the body
   shield, the take grace, the carrier's own sweep or spin, the window's timing, a poke at a puck nobody carries. One table. Say what share of misses no
   player could have avoided.
2. **A poke at an unshielded carried puck inside reach takes it** as the law says (law 4: a sprinting carrier's puck is far from him and easier to take): the
   bar: a human-class player's poke takes the puck **at least 15 % of presses at pro** against a carrier that is not shielding and not in its grace, and at
   least 25 % against a sprinting carrier; with the stop-and-go drill's bars unchanged (law 3 is a gate: a poke must not beat a clean stop begun at 8 m).
3. **A miss costs a recovery, not a fall**: the poker's own stumble after a miss becomes a short recovery in which he cannot poke or check again (the frames a
   miss costs today as a stumble, as a stick lock instead; keep the stumble only where the poke hit a braced body or the boards if the sim tells them apart):
   a key with a neutral value that gives back today's stumble. Bar: the human's own-poke stumbles under 1 a match at pro.
4. **The scripted human pokes when a poke can land** (its decision only): so the measurement of "the human" stops paying for presses no player would make.
5. Cells before and after for each rule alone (600 a cell, paired seeds, `-Multiprocess`, three processes at most): the review's own variant read
   86 to 84, 51 to 49 and 23 to 29 for the stumble alone: a rule that moves a cell more than 4 points ships at its neutral value: say which ships on.

**From 08:00 nothing that moves the ladder or a verb lands without an acceptance run on its own package (R-0056)**: write `POKE READY` in the status file
with the file list as soon as items 1 to 3 are green and measured. Use `JamBaseline.h` for neutral tests. State fields: hashed or listed.
The gate: the full `Jam` filter green. Time box 100 minutes.
