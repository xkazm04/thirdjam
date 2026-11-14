# Lane A184, track F1 (contact), evolution 9 (device A, Opus)

You are lane A184 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (law 1: a hit obeys momentum), **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; points 1
and 5), the track's `docs/experiments/CONTACT-7.md`, `CONTACT-8.md`, `CONTACT-9.md` (what waits switched off and its costs), `contracts/skater-visual.md` (the
figure's calls by name) and `handoff/A-to-B-0027.md` (what device B makes for the hits today).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A184.md`. Port 4784. Family as the
track's: the contact functions in `JamSim.cpp/.h` (`TryLift`'s cross-check, `TryCheck`), `FJamInput`, the `game.contact` keys, the scripted human's and the
bots' use of the cross-check (`JamPlaytester.cpp`, the one call site in `JamAICore.cpp`), the contact tests, drill 22, `docs/experiments/CONTACT-10.md`.

## Evolution 9: a cross-check you carry into a man
His words: "Crosscheck should be holdable so player with speed momentum can run with held crosscheck into another player" and "Animations of crosscheck
and body check are short and not clear".
1. **The held cross-check**: a new input level `FJamInput::bCrossHold` (the controls lane A183 sets it from LMB while the human does not carry). While
   it is held and the skater does not carry the puck he is BRACED: his stick is across his body; the first opponent his body meets while braced takes
   the cross-check with the momentum he brings (law 1's line by closing speed, the same table as today's cross-check: say the frames down at 4 / 8 / 12
   / 16 m/s of closing, held against pressed); then the brace is spent until the button is released and pressed again. **It is a commitment, not a
   free shield**: say and measure its prices and pick them so that holding it all match long loses: candidates: no poke, no lift and no pass reception
   while braced; top speed or turning reduced a little; a braced skater who is hit head-on by a faster man pays more. A tap (pressed and released
   inside N frames) is today's cross-check, unchanged. Keys with neutral values that give back today's game exactly (hash-identical default: prove it):
   `contact.cross_hold` (on by default only when measured: see the last paragraph).
2. **The bots and the scripted human**: the scripted human uses the brace as a person would (held from a few metres out when skating at a man without
   the puck: its switch on with the rule); the bots may brace at pro and all_star: a tune key, neutral off, measured.
3. **The hit lasts long enough to be seen** (his animations point, A's half): today the hit lands on the press's frame and the figure's take has no
   time: give the body check and the cross-check a short wind-up the sim owns (`contact.check_windup_frames`, `contact.cross_windup_frames`: the press
   commits, the contact window opens N frames later: 4 to 8), so device B's takes have a beat before the contact; the hit-stop after a landed hit by
   its strength (`JAMHITSTOP` exists: its frames by ratio). Neutral 0. Measure what a wind-up does to landed checks a match for the human and the bots.
4. **The figure calls**: `PlayCrossCheck` / `PlayCheck` on the press with the frames to contact as an argument if the contract's call takes one (read
   it: do not edit the contract: write the exact ask for device B in your report); `SetCrossCheckHold(bool)` is what B builds for the brace: call it by
   name behind a check that the function exists, so the game runs before B's push and uses it after.
5. Cells with the ladder's instrument (600 paired seeds, `JAM_PLAYTESTER_CHECK=closing`): the brace on with its prices: the three cells, the human's
   landed cross-checks a match, his knockdowns given and taken; law 3's row untouched (the stop at 8 m: `Bar.Curve.Law3`).

**This one ships ON** (the operator asked for the verb): the held cross-check and the wind-up at the values you recommend, with the cells said; the curve
lane re-sets the ladder after you and the speed lane land. Write `CONTACT READY` with the file list when items 1 to 4 are closed, built, the full `Jam`
filter green and Shipping compiles: **wanted by 14:45 for the 15:30 package**; `SOURCE OPEN` before any later source change. State fields hashed or
listed. `-Multiprocess`; three measurement processes at most; stop only processes whose ids you recorded. Time box 140 minutes.
