# Lane A209, track F1 (contact), the night: a hit he aims lands (device A, Opus)

You are lane A209 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then the track's `docs/experiments/CONTACT-10.md` to `CONTACT-12.md` with `status/A192.md` and `status/A200.md` (the body check's window and
momentum, the pressed and the held cross-check, the wind-ups: the human's 4 / 4, the bots' 8 / 12), `schema/DEFAULTS.md`'s `game.contact` block
(`check_reach_m` is 0: off: "the check finds its man: 1.0 was the measured value"; `cross_reach_m` 0.5), the controls' `CONTROLS-3.md` (LMB without
the puck cross-checks, held it braces; RMB without the puck body-checks), and the hits lane's `docs/experiments/HITS.md` (what is drawn around a hit).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A209.md`. Port 4809. Family as the
track's: contact in `JamSim.cpp/.h`, the `game.contact` keys, the contact tests and drills, the playtester's check timing, `docs/experiments/CONTACT-13.md`.
Not yours: the animation and what is drawn (lane A210 and device B), the brain (A206), possession (A208).

## His words: "Tackles/Crosschecks are difficult to land as they need to land to thin space of the player" and "Tackle is also unrecognizable and animation too quick"
1. **See why they miss** (30 minutes, no code): a scripted hand like a person's (a reaction of 12 to 18 frames, an aim error of 10 to 20 degrees, a
   press that comes early or late by up to 8 frames) at rookie and pro: 50 matches: of his body-check presses and cross-check presses within 4 m of
   an opponent: how many land, and for each miss why (the window had closed, he was outside the gap, the angle between his facing or his aim and the
   man was too wide, the man's side or back, the puck rule: a cross-check only counts "at a carrier"?, the wind-up, a refusal: grace, cooldown,
   soft). The geometry of a landing hit today in a picture: the gap in metres, the cone in degrees, the frames. One table, `SEEN` by 21:15.
2. **A hit lands when a person would say it should**: he presses near a man he is facing or skating at, and it lands: the check finds its man
   (`check_reach_m`: turn it on at what a hand needs: it was measured at 1.0; a short pull of the checker toward the man during the wind-up so the
   bodies meet), a cone of facing or aim wide enough for a mouse (say the degrees), a window long enough for a late hand, and the same generosity for
   the pressed cross-check (it must land on a man with or without the puck when he is in front: say what it does to a man without the puck: a shove
   and a stumble; with the puck: the strip as today) and for the held brace (the body's width, not the stick's line). Momentum still decides how hard
   (a standing shove is a bump; a full-speed hit is a knockdown): the size is by speed, the landing is by intent.
3. **The human's wind-up gives the animation time**: "animation too quick": the human's body check and cross-check take the wind-up the bots have
   (8 frames, device B's takes need 6 or more), with the press BUFFERED so the button does not feel late: the hit is committed on the press, the pull
   toward the man covers the wind-up, the contact lands at its end. Say what the acceptance drill's steps 7 and 8 need (the Director edits the
   release rows; the drill file is yours for those steps: keep the row waiting for a man on his feet).
4. **Bars** (the person-like hand, 100 matches at rookie and pro): presses within 2.5 m of a man inside the cone land 85 % or more (today: your
   table); a press at nobody still whiffs (no hit from 5 m); knockdowns a match stay inside a band a person enjoys (say both sides' numbers: the
   human must not spend more than 8 % of play on his back at pro: if the bots' hits land more too, their press rate comes down in the profile: say
   the number: the brain lane and the curve lane read your file); law 3's gate rows hold (a stop begun at 8 m still beats the charge: the pull must
   not turn a miss by a stopped carrier into a hit: prove it on the drill).
5. Tests: the contact tests read the wind-up and the reach from the cartridge they build; say each re-staged test.

`CONTACT READY 1` by 23:15 (items 1 and 2: file list, the table before and after, the hash proof at neutral, law 3's rows, the six cells at 600
for the curve lane), then waves 2 and 3 as NIGHT-COMMON says. Three measurement processes at most. Time box: the night.
