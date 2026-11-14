# Lane A185, track F2 (movement), evolution 9 (device A, Opus)

You are lane A185 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (laws 3 and 4), **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing package H31; point 2 is your charter),
the track's `docs/experiments/MOVEMENT-5.md` to `MOVEMENT-9.md`, the curve's `docs/experiments/CURVE-9.md` to `CURVE-11.md` (the ladder as it stands and
its dials) and `docs/design/REVIEW-3.md` (the stop's sweep by distance and level).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A185.md`. Port 4785. Family as the
track's: the movement functions and the `game.skater` keys in `JamSim.cpp/.h` and the cartridges' skater blocks, the carry drill, `Jam.Core.Movement.*`,
`docs/experiments/MOVEMENT-10.md`.

## Evolution 9: skaters without the puck are 15 % faster
His words: "Skaters without puck should be 15% faster than now".
1. **The rule, as ruled**: every skater without the puck, human and bot, at every level, has 15 % more top speed than today, in a glide and in a sprint;
   **the carrier skates exactly as today** (so the carrier is slower against everyone by that much). Find the cleanest place: the shipped cartridges'
   `skater.max_speed` and `sprint` values up 15 % with `skater.carry_speed_mult` (and the sprint's carry factor) down so that carrier speeds are
   unchanged to the digit; or one new key `skater.free_speed_mult` 1.15 (neutral 1) if a cartridge-by-cartridge edit would touch every mode: say which
   and why. Acceleration: say whether "15 % faster" should reach its top in the same time (acceleration up 15 %) or over the same distance: build the
   one that feels like speed, not like ice (the Director's reading: the same time), and say the other's numbers.
2. **What it does to the game's laws**, measured: law 3: the stop begun at 8 m at every level (the defender now closes 15 % faster: `Bar.Curve.Law3`'s
   whole row at rookie, pro, all_star: 8 / 9.5 / 10 / 11 / 12 m and the straight charge): if the window moves, say where it now is and whether the
   carry drill's "five stick lengths" still holds; law 4 (a sprinting carrier's puck is easier to take): unchanged or not; a carrier being caught from
   behind: how often per match before and after (the playtest line's fields or a trace).
3. **What it does to the cells** (the ladder's instrument, 600 paired seeds, both rinks): the six cells before and after; the human's carry length,
   knockdowns a match, time on the ice; the bots' passes completed. You do not re-set the ladder: the curve lane does, after you and the contact lane
   land: give it the table.
4. **The marks and the drill**: the stop line stands 8 m ahead and lights by closing speed and a far edge of 9.5 m (`JamCarrierMarks.h`): with a faster
   defender, are its numbers still the stop's window? One sweep (the review's `JAM_MARK_RULE` arms): the far edge that keeps "a stop begun on the
   light passes at 90 % or more at pro". Drill 21's on-screen distance follows the measured window.
5. Tests updated to the new speeds where they pin one (say each); no pinned hash of a shipped cut.

**This one ships ON.** Write `SPEED READY` with the file list when items 1 and 2 are closed, built, the full `Jam` filter green and Shipping compiles:
**wanted by 14:30 for the 15:30 package**; `SOURCE OPEN` before any later source change. `-Multiprocess`; three measurement processes at most; stop only
processes whose ids you recorded. Time box 130 minutes.
