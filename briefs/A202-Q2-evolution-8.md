# Lane A202, track Q2 (the brain), evolution 8 (device A, Opus)

You are lane A202 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing and today's clock: the freeze is 20:00), the track's
`docs/experiments/BRAIN-6.md` and `BRAIN-7.md` with `status/A193.md` (the bots plan with a free skater's true speed; the three findings left open: the
plain lead at 8 m/s, the mates planning at the wrong scale, the red skill no estimate reads), the contact lane's `docs/experiments/CONTACT-11.md` (the
hits' wind-ups, the held cross-check: do the bots use it? `cross_hold_closing` -1 says no), and `docs/PLAY-ME.md`'s "Known to be unfinished" list
(a skater standing at a downed body and pushing it along the ice; opponents scoring from the red line; half a minute without the puck on big ice).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A202.md`. Port 4802. Family as the
track's: `JamAICore.cpp/.h`, the brain's tests (`HockeyBrain*Tests.cpp`), the brain's switches in `JamBaseline.h`, `docs/experiments/BRAIN-8.md`. The
curve lane sets the ladder today on main as it stands: **everything of yours is delivered behind a switch that is OFF (brain8=0 replays main to the
hash: 100 seeds a cell, six cells), with its measured row: the Director decides what goes on before the 18:30 package.**

## Evolution 8: the bots do nothing stupid in front of the operator
Since noon everyone without the puck is 15 % faster, hits wind up, and the bots plan with the new speed. The operator plays again at 20:00. Find what
the bots now do that a watcher would call broken, by trace, not by reading code.
1. **The watch** (first hour): 60 matches traced (pro and all_star, both rinks, and three modes: Arsenal, Shotgun, Marathon): detectors over the
   trace, each with a count a match and the three worst moments (seed, frame): a skater pressing into the boards or a net for more than 60 frames; two
   mates within 1.5 m of each other for more than 90 frames (a clump); a skater standing inside 1.2 m of a downed body for more than 30 frames; a bot
   circling (heading change over 720 degrees in 180 frames without a touch); nobody within 6 m of a loose puck for 90 frames while a bot could reach
   it in 60; a carrier bot skating into his own zone with open ice ahead; a defender leaving the slot empty with the puck below the circles; a shot
   released from beyond 30 m with a mate open nearer; a bot checking a man who has no puck and is 15 m from it; anything else the trace shows. The
   table, sorted by how often and how visible.
2. **The top three, fixed** (each behind brain8's own sub-switch, each with a before / after count from the same seeds and its six-cell row at 600):
   the Director expects the downed-body stand and the long shot among them, but the table decides.
3. **Do the bots brace?** With the held cross-check on, the human can run through a man with a held stick: an opponent who never does it is a tell.
   Measure `cross_hold_closing` at two values at pro and all_star: braced hits a match by bots, the human's knockdowns, the cell. Off by default; the row.
4. **The three open findings of evolution 7** only if cheap and hash-neutral at their switch: the plain lead's 8 m/s, the mates' scale.
5. Tests for each fix (a staged scene per stupidity: the bot does the right thing with the switch, the old thing without).

Write `WATCH TABLE` in the status file by 16:00 (item 1), `BRAIN READY 2` with the file list, the switches and the rows when items 2, 3 and 5 are
closed, built, the full `Jam` filter green and Shipping compiles: **wanted by 17:30**; `SOURCE OPEN` before any later source change. `-Multiprocess`;
three measurement processes at most; stop only processes whose ids you recorded. Time box 200 minutes.
