# Lane A200, track F1 (contact), evolution 11 (device A, Opus)

You are lane A200 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; points 1 and 5), the track's `status/A192.md` and
`docs/experiments/CONTACT-11.md` (the wind-ups as shipped: 4 frames before a body check's window, 4 before a pressed cross-check's shove; the tests
re-staged for them; law 3's late-stop lines that got harsher), `docs/experiments/BRAIN-7.md` (the bots now plan with a free skater's true speed at pro
and all_star), and `handoff/A-to-B-0027.md` point 1 with device B's answer (device B proposes the frames from call to contact about 15:45).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A200.md`. Port 4800. Family as the
track's: contact in `JamSim.cpp/.h`, the `game.contact` keys, the contact tests and drills, the pressers' timing in the brain where it reads the
wind-up (say each line), `JamPlaytester`'s check and brace timing, `docs/experiments/CONTACT-12.md`. The curve lane re-sets the ladder today on main as
it stands: **nothing of yours ships ON without the Director's word: every new number is delivered at its neutral value with the measured table.**

## Evolution 11: how long must a wind-up be for an eye, and what does that cost
His words: "Animations of crosscheck and body check are short and not clear". Four frames are 67 ms: under what an eye reads. The Director's question:
if the wind-up is 6, 8 or 10 frames, for bots only or for everyone, what happens to the game?
1. **Human and bot apart**: the wind-up's length is one key today for both. Add (neutral = same as today) a second pair so the bots' wind-up can be
   longer than the human's (`contact.bot_check_windup_frames`, `contact.bot_cross_windup_frames`, absent = the shared value; schema, limits, DEFAULTS,
   README, both generators; `Jam.Net.HashCoverage` holds; a guest and a host agree: the sim decides by the skater's controller flag it already has: say
   which field). The human's own press must not feel late: his stays 4.
2. **The table** (600 paired seeds, the ladder's instrument on this tree, regular rink, the three levels; then the large rink for the arm you would
   ship): bots' wind-up 4 (as main), 6, 8, 10, 12 for the body check, the cross-check, and both: the win rate, the human's knockdowns a match, the bots'
   landed checks a match, how often the human gets out of a hit he could see coming (the scripted human reacts to a wind-up he faces with a turn or a
   brake after 6 frames of reaction: add that to the script as a switch, off by default, and say what it is worth: this is the number that says
   whether a longer wind-up is a mechanic or only a delay).
3. **Law 3 with each arm**: `Bar.Curve.Law3`'s rows at pro and all_star (8 / 8.5 / 9 / 9.5 m, the straight charge): the gate rows must hold; and
   evolution 10's open problem: the late-stop lines came out harsher because the bot presses 4 frames' worth early: the bots press early only while
   the carrier is not braking: build it behind a brain switch (off), and say what the lines read with it.
4. **Tests at other lengths**: the tests you re-staged for 4 frames: make each read the wind-up from the cartridge it builds (so 8 does not need a
   third re-staging); the full `Jam` filter green at the neutral values AND at bots 8 / 8 (run it with an override, say how).
5. **Device B's frames**: when B's hand-back names its frames (the Director forwards it), give the arm that fits them and its row of the table.

Write `WINDUP TABLE` in the status file as soon as item 2's regular-rink table is in (**wanted by 16:15**: the Director decides the length and tells the
curve lane), `CONTACT READY 3` with the file list when items 1, 3 and 4 are closed, built, the full `Jam` filter green, Shipping compiles (**by 17:00**);
`SOURCE OPEN` before any later source change. `-Multiprocess`; three measurement processes at most; stop only processes whose ids you recorded. Time
box 180 minutes.
