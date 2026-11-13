# Lane A102, track F2: F2-carrier-skill (device A, evolution 1, Fable)

You are lane A102 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a102` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A102.md`. Your remote-console port if you run a game: 4702.

## Track F2: carrier skill
Family: the movement and stick functions in `JamSim.cpp/.h` (the movement step, the reverse brake, the burst, the sprint, `StepStick`, the carried puck's distance, the sweep and
its shield, the deke, the spin), the drills in `HokejGameMode.cpp` and `HokejAcceptDrill.cpp`, the movement and stick keys, `Jam.Core.Movement.*` and the stick tests,
`docs/experiments/MOVEMENT-2.md`.
First evolution: **the stop-and-go is the carrier's signature.** (a) a headless scenario (the scripted human through `FJamInput` with the mouse-style stick): carry at 9 m/s at a
standing pro defender 12 m away, brake at 5 m, cut 60 degrees, burst; over 200 seeds: the share that gets past with the puck today, against a straight charge; and drill 21 in
`HokejGameMode.cpp` staging the same for a package; (b) tune what it shows inside the keys (`burst.frames`, `burst.accel_mult`, `reverse_brake_decel`, the puck's distance while
braking) so a clean stop-and-go beats a pro defender two times in three and a straight charge one time in five: the gap between the two is the skill the vision asks for;
(c) **puck protection**: while the carrier's body is between a defender's stick and the puck (the defender inside 2 m, the puck on the far side), a poke or a lift from that
defender fails, with no button (`game.stick.body_shield`, default true, neutral false); (d) the three target cells measured; the acceptance run's brake and carry rows on a
package only if you hold the cursor (else say queued).

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
