# Lane A207, track P1 (the pass and the shot), the night: a pass arrives, a shot is a shot (device A, Opus)

You are lane A207 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then `docs/VISION.md`, `schema/README.md` and `schema/DEFAULTS.md` (the keys: `skater.pass_speed` 14, the shot's speed and charge, the saucer, the
one-timer window, the pass's catch), the pass and shot code in `JamSim.cpp` (`StepPassCharge`, `StepCharge`, the release, the receive / catch, the
one-timer), the controls' `docs/experiments/CONTROLS-3.md` and `CONTROLS-4.md` (the buttons: RMB with the puck passes, held from 13 frames it is a
saucer; LMB shoots, tap flat, hold to lift; the charge rule), and how a bot passes and shoots (`JamAICore.cpp`: the pass decision and its aim).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A207.md`. Port 4807. Family: the
pass's and the shot's physics, charge and aim in `JamSim.cpp/.h`, their `game.skater` / `game.puck` keys and tests, the pass's and shot's input
mapping only where a threshold must change (`InputMapping.*`: say each line), the pass's and shot's HUD read-outs, `docs/experiments/PASS-SHOT.md`.
Not yours: who decides to pass (the brain lane A206: but the AIM and LEAD of a bot's pass is a sim-side helper you provide and it calls), the pickup
of a loose puck (A208), contact (A209).

## His words: "They want to pass but no pass reaches the end (inprecise direction or speed), passess need to be able to be hold enough to cross 1/3 of arena length on max, shoot speed should double"
1. **Measure the pass as it is** (30 minutes): human and bot, tap and held: the puck's speed at release, its speed after 5 / 10 / 20 / 30 m (the
   ice's friction), the distance at which it falls under a skater's speed, the angle error between the aim and the intended man (the human aims with
   the mouse: is the pass sent along the stick, at the cursor, or at a man?), where the receiver is when it arrives (did he skate away?), how often it
   is caught (by the mate, by an opponent, by nobody), per level; the same for bot-to-bot and bot-to-human. The rink is 90 x 45 m by default.
2. **The pass he ordered**: the hold decides the pass's power: a tap is a short crisp pass (arrives fast at 8 to 10 m); **a full hold sends the puck
   at least one third of the rink's length (30 m on the regular rink, scaled with the rink) and it is still moving fast enough to be a pass when it
   gets there** (say the arrival speed; a pass that dies at the receiver's feet is right, one that dies before him is wrong); in between, power grows
   evenly with the hold. Decide what happens to the saucer (it was "held from 13 frames": propose: the hold is power; the saucer is the pass given
   while a stick or body is in the lane, or a second key: take what a hand finds natural, say why, keep it rebindable). A power meter by the stick.
3. **A pass goes to a man**: aim assist: the pass leaves toward the mate nearest the aim line inside a cone (say the cone), led by his velocity and
   the pass's flight time, so it arrives where he will be; with no mate in the cone it goes where the stick points. The receiver's stick takes it
   (a catch radius that forgives: say the number; the possession lane A208 owns the loose-puck pickup: agree the pass's receive with it through your
   status files). The same helper aims the bots' passes (`AimPassAt(Receiver)`: the brain calls it). A pass into an opponent's stick is still his.
4. **The shot twice as fast**: "shoot speed should double": the puck's speed off a shot doubles at every charge (find the keys; if "doubles" breaks
   something measurable (the goalie-less net: every shot from anywhere scores; the puck leaves the rink; tunnelling through the boards or a body at
   60 Hz: check the puck's step against the thinnest collider and sub-step if needed), fix the thing it breaks rather than halving the order), with
   the shot's wind-up and release still readable. Bots' shots take the same speed; rookie's aim error stays its handicap.
5. **Bars** (100 matches at pro and rookie with the scripted player, and 200 staged passes): a held full pass covers 30 m or more and arrives above
   6 m/s; a tap pass at 8 m is caught 95 % of the time by an open mate; passes aimed within 20 degrees of a mate are completed 80 % or more when no
   opponent is in the lane; bot-to-bot completion 60 % or more at pro; the shot's speed is 2.0 x today's at tap, half and full charge; no puck leaves
   the rink or passes through a body or the boards in 200 matches.
6. Law 3 rows with each wave (a faster puck changes the carrier's options).

`PASS READY 1` by 23:15 (items 1 to 4 as far as they are closed: the pass's power and aim first: file list, bars, hash proof at neutral, six cells
at 600 for the curve lane), then waves 2 and 3 as NIGHT-COMMON says. Three measurement processes at most. Time box: the night.
