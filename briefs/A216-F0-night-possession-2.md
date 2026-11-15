# Lane A216, track F0 (possession), the night, part 2: the faceoff and the own net land green, then nothing passes through a body (device A, Opus)

You are lane A216 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
You take over the possession track from lane A208, whose context ran long. Read `briefs/A208-F0-night-possession.md` (the track's brief: its
family and laws are yours), then in `status/A208.md`: `## HANDOFF NOTE (02:17)`, `## STICK READY 2: NOT LANDABLE YET (02:42, build 43)`,
`## FOR A207 (01:50)`, `## FOR A210 and A213 (00:46)`; then **R-0067 and R-0068** at the end of `contracts/RULINGS.md`.

Your worktree is `.claude/worktrees/f0b`. It holds A208's wave 2 (not landed) with main 8cb7e104 merged into it three-way by the Director at
02:47 (no conflicts; everything is staged in the index: do not unstage, do not run git commands that change anything). New under A208's work
since its build 43: lane A209's wave 3 (the faceoff reset clears `CrossHoldFrames` behind `bHoldResetAtFaceoff`; a press buffer with hashed
fields `CheckBufferLeft`, `LiftBufferLeft`), lane A215's drill and driver fix, lane A213's HUD, the hardening suite (`HardTests.cpp`:
`Bar.Hard.PuckEscape`, `Bar.Hard.AcceptRows`). Rebuild first. Status file `status/A216.md` (A208's stays as the record). Port as A208 had.
Two heavy processes at most. Nothing of yours runs from the main checkout.

## 1. FACEOFF READY: the two red tests, then land what is built (the worst thing in the game waits on it)
The night's critic lane: "every puck drop ends with him flat on the ice" (7 faceoffs of 9, rookie too): he stops playing. A208 built the fix and
measured it (the human down in 0 of 18 first faceoffs at rookie, 0 of 17 at pro): `game.stick.faceoff {centre_gap_m 1.8, winner_shield_frames
90, no_hit_frames 45, no_hit_m 6}`; and `game.puck.net {own_wall, mouth_only}` (no goal on yourself with your own play; a goal only through
the mouth from the front: the hardening lane's side-netting goal), the events `possess` and `shot_blocked` Variant 1, a body seats a loose
puck only under `body_catch_mps` of relative speed. The full `Jam` filter on its build 43 was 339 pass, 2 FAIL:
- `Jam.Core.NetTrap` (JamNetTrapTests.cpp:92: "a skater pressed into a net hull without moving for 43 frames"): A208's suspect is the own-net
  wall (a bot carrying at its own goal mouth is blocked while it keeps pushing). Confirm by the one-line re-run it names (own_wall off in the
  test's config). If it is the wall: the wall must not hold a SKATER (only the puck): a carrier whose puck meets his own mouth keeps skating
  (the puck stays on his blade at the line, or drops loose at the post), and the test stays as it is. Do not weaken the test.
- `Jam.Bot.HumanHasSomethingToDo` (BotTests.cpp:202: large rink seed 7, 1 faceoff of 11 without a human touch inside 600 frames, worst 634):
  find what the new layout did there (is the human's centre placed at the dot when the human is NOT the centre? does the no-hit window let a
  bot skate the puck away for ten seconds?). Fix the cause; if it is truly a borderline of the seed, say so with the distribution over 20
  seeds before and after and the Director rules on the bar.
Then on the merged tree: built, the full `Jam` filter green in a cleared environment, Shipping compiles, law 3 as built (pro 94.0 / 99.5 / 93.0
/ 83.5 at 8 / 8.5 / 9 / 9.5 m, straight charge 0), the neutral proof (every key of the track neutral = main's game to the bit: say against
which main build), `Jam.Net.HashCoverage` with the new hashed fields, five-faceoff strips at rookie and pro. Write **`FACEOFF READY`** with
the file list by name (A208's list at the end of its NOT LANDABLE section plus yours): aim 60 to 75 minutes from your start. It is landed
at once. Keep those files closed after the line until the Director says it is on main.

## 2. Nothing passes through a body (the hardening lane's faults; the operator's point 2 in his own words: "it moves through them")
Read them whole in `.claude/worktrees/s3b/status/A212.md` (lines `FAULT 02:08` and `FAULT 02:10`, each with its repeat command; the test is
`Bar.Hard.PuckEscape` in `HardTests.cpp`, in your tree now):
- a carried puck sits inside ANOTHER skater's body for 6 frames and more (10 of 480 matches, in a crowd): the carrier's blade is inside the
  other man, the carry draws the puck to it, `BlockCarried` runs in the substeps yet the frame ends with the puck 0.30 to 0.37 m inside him.
  Wanted: the carry's target never lies inside another body; and what a player SEES: a carried puck pushed into an opponent's body is the
  opponent's (the pickup by body), into a mate's it stops at his edge;
- a mate's LIFTED puck (z 0.55 to 0.67 m, under BodyHeightM 1.8; not a pass in flight) flies through a standing skater with its speed untouched.
  The saucer's lane rule lets a puck over BLADES; a body below 1.8 m still stops or deflects it.
Bar: `Bar.Hard.PuckEscape` over 480 matches: puck inside a body (not a deke) 0, through a body 0, a goal other than through the mouth 0,
own-net goals by own play 0. `STICK READY 3` by 05:00.

## 3. Seams with the pass lane (A207): do not edit, do tell
A207 owns "a passer cannot take his own pass or shot back" and "a person's tap is solved for the mate it points at", in the pickup's code too
(A208's `FOR A207` names the lines; its suspect: every catch sets 4 settle frames during which the carry pulls the puck back whatever its
speed). If item 1 or 2 must touch those lines, write which under `FOR A207` in your status file before you do.

## 4. If time is left
The large rink at pro (the scripted human took 3 % of loose pucks there), the taken share re-read without a passer's own releases, and the
lock (carry within 15 cm at 98.3 %: A208 switched it off because gluing broke A201's late hand, the deke and a hash test: leave it off unless
a way is found that keeps those three green).
