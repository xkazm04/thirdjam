# The night of 14 to 15 November: what the operator said after playing H34, and the rules of the night (read after EVOLUTION-COMMON.md)

**R-0067, his words at 20:25, verbatim:**
"1. AI is the main flaw, it does not try to play hockey to achieve a goal, all teammates/oponents are not interested in game, disengaged. They want to
pass but no pass reaches the end (inprecise direction or speed), passess need to be able to be hold enough to cross 1/3 of arena length on max, shoot
speed should double 2. Puck does not stick with players, it moves through them without locking 3. Tackles/Crosschecks are difficult to land as they
need to land to thin space of the player, crosscheck has bug in animation displaying technical blue rectangle instead of animating player with stick
into its position. Tackle is also unrecognizable and animation too quick ... the game itself does not look bad, but effectively is unplayable and both
devices need to work towards these goals overnight intensively to perfect the game"

**What it means for every lane tonight**
- **The freeze of 20:00 is lifted for these goals.** The game he played is not a game yet: a player must feel, in the first minute, that ten skaters
  want the puck and the net, that a pass arrives, that the puck belongs to whoever touches it, and that a hit he aims lands and is seen.
- **The operator's eye is the bar, not the scripted player's win rate.** Every lane states its goal as something a person at the keyboard sees or
  feels, measures it with a number, and proves the number moved. The ladder (85 / 48 / 26) is re-set by the curve lane AFTER the game plays; no lane
  holds back a change that makes the game play because it moves a cell. Law 3 (the stop begun at 8 m beats a charging defender; a straight charge does
  not) is the one design law that stays a gate; if your change breaks it, say so at once with the rows: the Director decides.
- **He plays the default first: PLAY NOW at ROOKIE on the regular rink, mouse and keyboard.** Measure there first, then pro. A level may be easier by
  the bots' skill of execution (a slower shot release, a wider miss, a later read) and never by their not caring: at every level every skater without
  a job goes to the puck or to a place that matters.
- **Changes ship ON tonight.** Keep the discipline that makes landing possible: a cartridge key or a brain switch with a neutral value that replays
  the game of before to the hash (100 seeds a level), the full `Jam` filter green in a cleared environment, test-guards, Shipping compiles. Tests that
  pinned the old behaviour compare against the neutral explicitly. Lockstep protocol 9 and the wire stay; a new sim state field is hashed and covered
  by `Jam.Net.HashCoverage` (say it in your READY section).
- **Several lanes edit `JamSim.cpp` and `JamAICore.cpp` tonight** (passing and shooting; possession; contact; the brain). Keep each change in its own
  function with one call line; name every function and line you touch in the status file; never reformat or move code you do not change.
- **Deliver in waves.** `READY 1` by 23:15 by the machine clock with the single biggest thing a player would feel, built and gated; then keep going:
  `READY 2` by 02:30, `READY 3` by 06:00. Packages are cut about 23:45, 03:15 and 07:00. Write `SOURCE OPEN` before the first edit after a READY
  line and never leave a half-saved state on disk for more than a build: the Director exports on the READY line.
- **Stills and strips**: from the game camera, off screen, with `-dpcvars=r.RDG.AsyncCompute=0` (the editor's first-match GPU crash, R-0066 point 5).
  You do not own the cursor. No brand, league, team or film names or looks. Teen rating.
- The machine clock is the truth (`date +%H:%M`). Stop only processes whose ids you recorded. No git. Never `Game/Plugins/JamAssets` (device B's).
