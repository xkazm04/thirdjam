# Lane A215, track F1 (contact), the night, part 3b: the drill's rows pass in a window, a brace rides a carrier, the faceoff's contact side (device A, Opus)

You are lane A215 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
You take over the contact track from lane A209, whose context ran long: read `briefs/A209-F1-night-hits-land.md` (the track's brief: its family
and laws are yours), `status/A209.md` whole (its three READY sections, the FAULT it fixed, `FOR A206`, what it measured and refused),
`docs/experiments/CONTACT-13.md`, then **R-0067 and R-0068** at the end of `contracts/RULINGS.md`.

Your worktree is `.claude/worktrees/f1b`: it stands on main 015f1618 (lane A208's pickup by touch and the puck's collision with bodies are now
in it) with A209's wave 3 re-applied (in the landing chain now). Status file `status/A215.md` (A209's stays as the record). Port as A209 had.
Family as A209 had, plus the acceptance drill's steps 7 and 8 (`Game/Source/JamGame/Private/HokejAcceptDrill.cpp`, those steps only) and
`tools/qa/accept/` where a row's driver must change. Two heavy processes at most. Nothing of yours runs from the main checkout.

## 1. FAULT first: acceptance step 8 is red in the real window (it blocks every package)
Release H35 (main 015f1618) failed twice on step 8 "Be checked at full speed while standing"; steps 1 to 6, 9, 10 passed; step 7's `check` row
failed once in the second try. Logs: `C:\Users\kazda\AppData\Local\Temp\tj\accept-H35-1.txt`, `accept-H35-2.txt`; the passing run of the
evening `accept-H34-1.txt`.
- H34 (pass): `act none [step=8 ... carrier=-1 puck_dist=2.40 frame=4096]`, then `verb=checked ok=1 down=1 down_frames=150 knock_frames=150
  outcome=3 ratio=3.14 opp_cue=22 closing=-0.01`.
- H35 (fail): `act none [... carrier=0 puck_dist=0.96 frame=4101]`, then `verb=checked ok=0 down=0 knock_frames=0 outcome=2 ratio=0.55
  opp_cue=0 closing=-0.22` (try 2: ratio 0.81, closing -0.26; the row then runs out its 600 frames).
- Read so far: `Hand(Me)` (HokejAcceptDrill.cpp, the lambda near line 239) sets the puck 0.45 m ahead of the human with his velocity; before the
  pickup he never held it in this row (it slid 2.4 m off), now he is the carrier at the act frame; in both runs the human is about 3.7 to 4 m up
  ice 40 frames after being staged at x = 0 (he glides at the opponent standing at x = 7), and in H35 the opponent's cue fires on the act frame
  itself with no closing speed: the hit is a stumble by the momentum rule. A209's guess: the pickup or the puck's collision with a body changed
  what happens in the 45 frames of the hand-over. The hardening lane's headless copy of the row (`Bar.Hard.AcceptRows`, lane A212,
  `.claude/worktrees/s3b`) is green because its staging differs; A212 was told to stage as the drill does and to write a FAULT line with the
  reduction in `.claude/worktrees/s3b/status/A212.md`: read it if it is there.
- Do: find the cause by a trace of those 85 frames (positions and velocities of both, the carrier, every contact); stage the row so it means
  what its name says (he stands, the man comes from 7 m at a sprint, he goes down for the frames the hit earned), with the puck and without
  it if both are true rules of the game; if instead the GAME changed (a bot's full hit on a standing carrier no longer fells), write a `FAULT`
  line with the cause and stop on that point: the Director rules. Then step 7's `check` row (`ok=0 ... closing=6.51 speed=3.92` once): if it is
  the driver's lag (20 ms), make the row robust; if not, a FAULT.
- Prove in a WINDOW, not headless only: a Development package of your tree (`tools/run/package.ps1 -Config Development`, out under your
  worktree) and `tools/qa/accept/accept.ps1 -Package <it>`: all ten steps green twice in a row. The run takes the real cursor for about two
  minutes: the operator is asleep, the machine is yours for that. One window run on the machine at a time: make the directory
  `C:\Users\kazda\AppData\Local\Temp\tj\tj-window.lock` before the run (if it exists, wait: another run is on) and remove it after, pass or
  fail; the Director cuts no package while it exists. Never answer or close a system or security dialog that is not yours.
  `DRILL READY` the minute it is green, with the changed files by name: this alone is landed at once.

## 2. A brace held into a carrier's back is a ride (the eye's item 6: "the held cross-check in a chase does nothing")
The critic lane saw: 160 frames with the braced stick on the carrier's back, figures drawn over each other, the carrier scores; 3 holds in 60 s:
0 down, 1 stumble (`.claude/worktrees/a108/docs/experiments/eye/07-held-cross-check-chase-f302-456.jpg`). Law 1 stays: matched speeds exchange
no knockdown; nobody is felled from behind. The Director's ruling: while a braced stick is in contact with the back of a puck carrier, the
carrier loses speed (about 15 % of his top speed per second of contact, never below the rider's own speed), and after 45 frames of continuous
contact the puck comes off his blade loose (a strip: no fall, no stumble). Behind a key (neutral = off = tonight's game to the bit), bots and
the human under the same rule both ways; write under `FOR A206` the hashed read that says "I am being ridden" so a ridden bot passes or turns.
Tests in `Jam.Core.Contact.*`; the landing table and law 3 unchanged; the six cells before and after.

## 3. The faceoff's contact side, if lane A208 asks
A208 makes the draw's winner un-fellable for 90 frames (the take shield on a draw win) and refuses presses within 6 m of the dot for 45 frames.
If its status file (`.claude/worktrees/f0b/status/A208.md`) carries an `ASK A209`, answer it in yours and make the contact-side change.

## 4. Left from A209
Take A208's re-staging back out of `Jam.Controls.FirstFiveMinutes` now that the tap fault is fixed (it is in your tree now); answer lane A206's
`ASK A209` (`.claude/worktrees/q2b/status/A206.md`: which playtest counter is one per press of the human's check and cross-check: the stats
read 149 to 182 % landed per press).

`DRILL READY` as soon as item 1 is green in a window (aim: within 60 minutes of your start); `HIT READY 3b` (items 2 to 4) by 03:30 by the
machine clock: built, the full `Jam` filter green in a cleared environment, Shipping compiles, law 3 as built, the neutral proof. Time box: the night.
