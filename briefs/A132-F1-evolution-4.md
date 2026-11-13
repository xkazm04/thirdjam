# Lane A132, track F1 (contact feel), evolution 4 (device A, Fable)

You are lane A132 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (law 1), R-0048 to R-0053 at the end of `contracts/RULINGS.md`, and the track's docs: `docs/experiments/CONTACT-2.md`,
`CONTACT-3.md`, `CONTACT-4.md`, `status/A125.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**, which carries the whole
game for the first time on this track: track F2's body shield and take grace, the brain's second evolution, your own three evolutions. It has no binaries: build
the editor first (builds no longer queue across worktrees). Your status file: `status/A132.md`. Port 4732. Family as the track's: the contact functions in
`JamSim.cpp/.h`, `JamPawns.cpp/.h` (the hit-stop, the figure's read-outs), `CameraFollow.*` (the kick only), `HUDWidget.*` contact texts only, the `game.contact`
keys, `Jam.Core.Contact.*` tests, `docs/experiments/CONTACT-5.md`; plus the one playtester line named in item 2.

## Evolution 4: the hit is the game's event: it lands, and it has a size
1. **The shove under the shields, on main** (your own next-1: it never fired in a match on your old tree): confirm the four lines you named in `TryLift` after
   the merge, and measure: how often a brain lift at a shielded carrier becomes a shove, knockdowns of the human by a cross-check per match, and the three
   cells (600 a cell, paired: the rule on against a test seam that restores the refusal). Bar: the human down by a cross-check no more than once a match
   more than before; say what the cells do (the curve lane is re-setting them tonight: report, do not tune).
2. **The press that lands**: `game.contact.check_reach_m` 1.0 on, together with the playtester pressing its check on **closing speed** instead of its own speed
   along the line (`JamPlaytester.cpp:479-485`: five whiffs in six come from that line; it is the scripted human, so it changes what every measurement of the
   human means: name the before and after for the three cells). Bar: the whiff share under 40 % (60 % today); `Jam.Bot.HumanHasSomethingToDo` green with its
   budget reset from your measurement and the reason written beside it.
3. **The hit's size on screen**: the hold from 2 frames at ratio 1.0 to 6 at 3.0; the camera kick from 6 cm at 60 frames down to 12 cm at 150; a braced bounce
   reads as a bounce. Monotone over the range (a test), `jam.hitstop 0` and `jam.hitkick 0` still switch them off.
4. **Stale brake flags**: `bBraking` / `bReverseBraking` are cleared only in the skater's own step and stay through a faceoff (`JamSim.cpp:1195`, `:677`; the
   acceptance drill counts the flag at `HokejAcceptDrill.cpp:432`): clear them where the phase changes, with a test.

State fields you add: hashed in `StateHash()` if a rule reads them, listed in `SkaterReadouts` if they are read-outs (the common brief says where).
The gate: the full `Jam` filter green; every key's neutral gives back the hashes of the commit you started on. Time box 100 minutes; interim in the status file.
