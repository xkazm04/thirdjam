# Lane A151, track F1 (contact feel), evolution 5 (device A, Fable)

You are lane A151 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (law 1, in the operator's own words: "If tackle is done between players going to same direction with similar speed, the impact is almost
none next to a tackle or cross-check from player facing full speed"; "Stationary player can resist most of side tackles and fouls, logically not crosschecks
directly to face"; "Sprinting and movement should in laws of physics mean less stability"), R-0048 to R-0057 at the end of `contracts/RULINGS.md` (your
worktree's copy is the newest), `docs/PLAY-ME.md` (item 3 of what the operator is told to try at 10:00 is yours: "Two hits"), and the track's docs:
`docs/experiments/CONTACT.md` to `CONTACT-5.md`, `status/A132.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to the newest main (the ladder set,
your four evolutions landed). It was built before: an incremental build is enough. Your status file: `status/A151.md`. Port 4751. Family as the track's:
the contact functions in `JamSim.cpp/.h`, `JamPawns.cpp/.h` (the hit-stop, the figure's read-outs), `CameraFollow.*` (the kick only), `HUDWidget.*` contact
texts only, the `game.contact` keys, `Jam.Core.Contact.*` tests, `JamPlaytester.cpp` (the scripted human's check press and two new read-outs only),
`docs/experiments/CONTACT-6.md`.

## Evolution 5: the operator's hand finds the hit
At 10:00 the operator presses RMB at an opponent. What he feels in that second is this track.
1. **The press that lands, traced** (your own next-1): the scripted human still whiffs 49 % of its checks, and 41 of 60 whiffs have the carrier within 1 m
   closing under 3 m/s on the press frame. Trace one whiff frame by frame against the sim's check window (reach, cone, the window's frames, the lunge, what
   the target does in those frames) and say in one paragraph why a press at a man within a metre does nothing. Then fix what is the sim's (a window that
   closes before contact, a cone that excludes a man beside you, a lunge that goes where he was) with a neutral value, and what is the script's (when it
   presses). Add `check_presses` and `check_whiffs` to the playtest line (read-outs). Bar: whiffs under 40 % over 600 matches at pro; landed checks a match
   up; every cell within 4 points of the curve's tables (85 / 49 / 30), or the rule ships neutral.
2. **A standing skater is braced, but not a wall, and not a rocket**: lane A147 measured a standing skater body-checked at 9 m/s taking it as a bump and
   leaving at 6.6 m/s: neither the operator's law ("can resist most side tackles") nor sense (a bump moves a little speed). Stage the three cases (from the
   side, from the front, from behind; standing, at 9 m/s closing) and make the outcome obey the law: from the side a standing skater holds (a bump, his
   speed after it small: say the number, under 2 m/s), from the front full speed fells him or stumbles him by the ratio, and the attacker pays what the
   table says. Neutral value; cells before and after.
3. **The hit the operator makes is the hit he sees**: with `jam.ctrl.Drill 22` (your law-one drill) and one ordinary match, headless: for the human's own
   landed checks: the HUD's line, the hit-stop's frames and the camera's kick all fire for his hits as for hits on him (log lines); list any outcome that
   is silent on screen.
4. If time remains: the board check as a play (your older next): a stumbling or falling skater that reaches the boards at 3 m/s or more takes them as a
   second hit (`contact.board_slam`, neutral 0).

**From 08:00 nothing that moves the ladder or a verb lands without an acceptance run on its own package (R-0056)**: write `CONTACT READY` in the status file
with the file list as soon as items 1 and 2 are green and measured: the Director lands them before that hour. Use `JamBaseline.h` for neutral tests. State
fields: hashed or listed. Law 3 is a gate. `-Multiprocess`; three measurement processes at most; check memory before each batch. Time box 100 minutes.
