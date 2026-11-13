# Lane A93: speed change is the carrier's weapon: braking by reversing, the first-step burst, the carrier's sprint, the puck's distance by pace (device A, from H22, R-0045 laws 3 and 4, Fable)

You are lane A93 of the Third Jam (team Hokej). Read `docs/VISION.md` first: laws 3 and 4 are yours, word for word. Today a skater brakes only on Ctrl, accelerates at one
rate, the carrier cannot sprint at all (R-0041 point 6, now replaced), and the carried puck always rides at the blade's full reach. Your job: the movement that lets a player
beat a defender by stopping, cutting and going, and a sprint that is a gamble.

Read first: `docs/VISION.md`, R-0045, `docs/PLAN-RECONCILED.md`, `docs/experiments/CONTROLS.md` (A49: the bindings, the sprint and the stamina bar, `game.skater.sprint`),
`FEEL.md` (A16: turn rate, lateral grip, glide drag, `brake_decel`), `MECHANICS-2.md` and `MECHANICS-3.md` (A82: `carry_speed_mult` 0.92 and the sprint refused while carrying;
A85: `CarrierReachM()` = max(shot reach, 0.4 + `stick.reach_m`) + 0.05, the puck at the blade), `STICK.md` (A51: the puck on the blade, the sweep and its shield, `StepStick`),
`Game/Source/JamCore/Private/JamSim.cpp` (the skater's movement step, the brake, the stamina, `StepStick`, `CarrierReachM`, `StepSweepShield`, `ResolvePokes`), `JamSim.h`,
`docs/experiments/ACCEPT.md` and `Game/Source/JamGame/Private/HokejAcceptDrill.cpp` (step 1: skate, sprint, sprint_carry; step 3: carry), `schema/`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a93` (created at main 8f6b157; build it yourself, unity off; the Development game package with `-WaitMutex`; no
interactive editor). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`; never kill a process you did not start. Three other lanes edit the sim and
the brain at the same time, each fenced to its functions: **A92** the contact (`TryCheck`, the collision resolve, a new `StabilityOf` that **reads** the velocity and the sprint
flag you set); **A91** the shot's lift and charge and drills 9-11; **A94** the brain; **A86** the difficulty numbers. You own the movement step, the sprint and stamina rule, the
carried puck's distance and the shields' strength by pace; you do not touch the contact functions.

You own: the movement in `JamSim.cpp/.h`; `game.skater.reverse_brake_decel` (20), `game.skater.burst { frames 20, accel_mult 1.8 }`, `game.skater.sprint.carry_bonus` (0.08;
the free skater's `bonus` 0.15 stays), `game.stick.carry_reach_mult` (0.75 not sprinting, 1.0 sprinting), `game.stick.sweep.sprint_shield_mult` (0.5) in `schema/` +
`JamCartridge.*` (names yours to refine; the three generators); the movement and stick tests; the acceptance drill's step 1 and a new brake step (`HokejAcceptDrill.cpp` and
their rows in `tools/qa/accept/accept.mjs`: nothing else there); the pause menu's controls card line for braking (text only, in the widget that holds it);
`docs/experiments/MOVEMENT.md`; `status/A93.md`.

## Job
1. **Braking by reversing**: input against the velocity (the input's dot with the velocity's direction under -0.5) decelerates at `reverse_brake_decel` until the skater is
   still, then the input accelerates the other way; the brake's spray event fires as on Ctrl; Ctrl keeps working. From full speed to still in about half a second.
2. **The first-step burst**: for `burst.frames` after the speed was under 2 m/s or the heading changed by more than 60 degrees inside 10 frames, acceleration is `accel_mult`
   times normal; then it settles. Stop, cut, go must feel fast: measure the time from full speed one way to 6 m/s the other way, before and after.
3. **The carrier's sprint**: allowed; top speed = base x `carry_speed_mult` x (1 + `carry_bonus`), always below a free skater's sprint (base x 1.15): a test proves a sprinting
   defender closes on a sprinting carrier in a straight line; the stamina drains as for anyone.
4. **The puck's distance by pace**: not sprinting, the blade's reach is `carry_reach_mult` 0.75 of full (the puck about 1.2 m from the body at full aim); sprinting, full reach
   and the sweep's shield window halves (`sprint_shield_mult`), so a stick takes it more easily; `CarrierReachM` follows the live reach so shots and passes still fire (A85's
   rule and its test stay green); the aim's direction is always the cursor's.
5. **Neutral values give back today's game**: document the neutral block; `Jam.Core.MovementNeutral` proves a bot match's hash equals main's with it. The default cartridge
   uses the new values.
6. **Tests** (`Jam.Core.Movement.*`): the reverse brake's stopping time; the burst's window; the carrier's sprint slower than a defender's; the puck's distance by pace; the
   weaker shield while sprinting; A51's, A82's and A85's tests green (the ones whose numbers move by this ruling are updated and each change written).
7. **The acceptance run**: step 1 re-staged (the carrier's sprint now raises its speed but stays under a free sprint) and a new step "brake": W for 2 s then S: still inside
   40 frames, then moving backward; green on your Development package; the whole run exit 0 on your tree except rows another lane owns (say which).
8. **A bot match** at pro, 3 minutes headless: the carrier's sprint frames, pokes that took a sprinting carrier's puck against a walking one's; report the numbers.
9. `MOVEMENT.md`: the rules, the keys, the neutral block, the before and after timings, what the player will feel in one paragraph.

Report: the tests, the hash before and after, the neutral proof, the acceptance rows, the timings, the file list. Write `status/A93.md` from the first minute. Interim at 40
minutes with steps 1-4 in the sim. Time box 110 minutes.
