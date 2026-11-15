# Lane A218, track F0 (possession), Sunday: a puck inside his ring is his, whatever his stick points at (device A, Opus)

You are lane A218 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069** at the end of `contracts/RULINGS.md` (the operator played package H39 at 10:00), `status/A216.md` and `status/A208.md` (the
possession track's night: its family and laws are yours), `status/A207.md` PASS READY 2c / 3 (the passer's own-release lock is the pass lane's;
that lane is closed: its lines in the pickup are yours now).

Your worktree is `.claude/worktrees/f0b`, on main (the Director names the commit). Status file `status/A218.md`. Two heavy processes at most.
Nothing of yours runs from the main checkout. Submission is tonight at 22:00: this is a FAULT of the package, not an evolution.

## His words (R-0069, verbatim)
"puck is not controlled by player if in his circle area, then player runs right through the pack and still does not have control"

The night called this fixed ("loose pucks taken 99.6 %") with scripted hands whose stick always points at the puck and who never hold a button.
A person's cursor is somewhere else. **You first reproduce it as he meets it, then cure it, then show it on the same reproduction.**

## What the Director already read in the code (JamSim.cpp; check each, there may be more)
1. `OwnPuckClears` (about line 3595): while `Puck.PassFrames > 0 && Puck.PassBy == him` his own pass is never his again, by arc, body, catch or
   carry: and a pass counts as "in flight" until another skater touches it, 180 frames pass, or it slows under `PassCatchMinMps` = 1.0 m/s
   (line 4215). A person taps RMB with nobody on the line (or the mate does not take it), skates after his slow puck and runs through it for up
   to three seconds. The shot's lock is 32 frames while it leaves him.
2. `TryLoosePickup` (about line 3675): the stick's arc is a wedge of `PickupArcDeg` 60 degrees either side of his FACING or his blade's line,
   inside `PickupArcM` 1.5 m. A puck inside the ring drawn under him (radius 1.1 m, a full circle) but beside or behind him is not his.
3. `MayPlayLoose` (about line 3659): no pickup at all while `S.bCrossBraced` (he holds LMB without the puck: the brace he asked for), while he
   cannot act, while `DeflectCooldown` runs after a press took his puck, while a draw is open.
4. The "his own puck only when it comes back at him" line in TryLoosePickup (`Puck.LastTouch == S.Index && Dot(Puck.Vel - S.Vel, To) >= 0`):
   a puck he lost or dropped that moves the way he skates is refused until he outruns it.
5. `BodyMeetsPuck`: his body takes a loose puck only under `PickupBodyCatchMps` 18 m/s of relative speed, and never his own just-released puck.

## The rule (the Director's ruling, R-0069 point 2)
**For the HUMAN: a loose puck that is inside the ring drawn under him, or that his body skates through, becomes his on that frame**, whatever
his stick points at, at any speed he can skate (a full circle of the ring's radius round him, and his body), including his own pass or shot
once it is no longer leaving him (it has slowed under about 6 m/s relative to him, or he has caught up with it, or 20 frames have passed since
the release: whichever comes first; the lock exists only so the release gets off his stick), including while he holds LMB or RMB (a held
brace picks the puck up and the brace ends; the held button then does nothing until released, as the rules already say). Exceptions that stay:
a puck riding with another carrier is taken by a press, not by touch (R-0048); a puck over the blades' height; an opponent's hard shot rings
off (the sting); during the faceoff's draw the draw's own rule decides. The bots keep the night's rules unless a case is plainly the same fault
for them (say which you changed for both). Behind keys where it is a number (`stick.pickup.*`), neutral = main.

## Reproduce first (the first 30 minutes), then cure, then prove
1. A staged test for each case above with a PERSON's hand: the stick aimed 90 and 180 degrees away from the puck; he skates through a still
   puck at walking pace, at full speed and at a sprint; a puck beside him and behind him inside 1.1 m; after his own tapped pass to nobody
   (he chases it at once); after his own shot's rebound; while holding LMB; while holding RMB; just after a poke took it from him. One table:
   the case, frames from "inside his ring" to "his" on main (or "never"). Write it as `SEEN` lines the minute it exists: the Director reads it.
2. The cure, smallest sure change; the same table after: every row "his" within 2 frames (the release cases: within the bound of the rule).
3. Tests in `Jam.Core.Possession.*` for every row; the full `Jam` filter in a cleared environment; Shipping compiles; law 3 as built (pro
   94.0 / 99.5 / 93.0 / 83.5 at 8 / 8.5 / 9 / 9.5 m, the straight charge 0); `Jam.Net.HashCoverage`; the escape scan's classes that the night
   brought to 0 stay 0 (`Bar.Hard.PuckEscape`: outside the rink, stuck in a net, wall goals) over 120 matches.
4. Then, with the person-path instrument (`-run=JamHandCells`, on main: `tools/perf/results/a217/hand.mjs`): rookie regular and pro regular at
   100 seeds before and after: the win rate may rise (he keeps more pucks): say by how much.

`PUCK FAULT READY` with the table before and after, the files by name, the gate: **by 12:30 by the machine clock**. Keep the files closed
after the line. If a case cannot be cured by then, land the others and name it.
