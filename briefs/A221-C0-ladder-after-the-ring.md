# Lane A221, track C0 (the curve), Sunday: the ladder after the ring (device A, Opus)

You are lane A221 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0069 and R-0070** at the end of `contracts/RULINGS.md`; the track's last set: `status/A217.md` from "LADDER SET 3 GREEN" to its end
(the person's-path instrument `-run=JamHandCells`, the drivers under `tools/perf/results/a217/`, which numbers the set moved, the table
"SET 3 after the net fix"), `docs/experiments/CURVE-13.md`; and `status/A218.md` PUCK FAULT READY (the rule that changed the game this
morning: for the human, a loose puck inside the 1.1 m ring under him is his on that frame: `stick.pickup.ring_m`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A221.md`. Family:
`tools/perf/results/a221/` (copy the track's drivers from `a217/`: do not edit theirs, except `a217/curve-pins.txt` when the set moves the
pinned hashes), the ladder's own values (the level cartridges' `ladder/*`, `opp.*` and `ai.*` numbers the earlier sets moved: NOT the
brain's code, NOT the sim's code, NOT `stick.pickup.*`: the ring stays as landed), tests `Jam.Curve.*`, `docs/experiments/CURVE-14.md`.
Headless (`-nullrhi`); four processes at most. Nothing of yours runs from the main checkout. Submission is tonight at 22:00.

## Why
The ring gives the person every loose puck he reaches. Lane A218 measured through the person's path (100 seeds, regular rink): rookie
92 -> 98 % won, pro 55 -> 73 %. All_star and the large rink are unmeasured. The targets stand: rookie 85, pro 48, all_star 26 (the player
each level is meant for, within 4 at 300 seeds). A ladder whose middle step is won three times in four is no ladder.

## Do
1. **Measure first (to `CURVE READY 1`, within 30 minutes of your start):** the six cells (rookie / pro / all_star x regular / large) at 300
   seeds through the person's path on your tree, the same columns as SET 3's table (win % with its interval, goals for : against, his shots,
   match length median, his passes first touched by a mate, knocked down a minute, on his back %, goals against inside f600), beside SET 3's
   row. And three lines on WHERE the person now wins more (pucks kept after a poke? rebounds? opponents' shots blocked inside the ring
   between 6 and 18 m/s: A218's open point 3: count them a match; if blocked shots are a large share, say so in a `FAULT` line: that
   would be a rule to narrow, the Director's call, not a ladder number).
2. **LADDER SET 4:** bring pro and all_star back to target with the levers the earlier sets used (opponents' pace, shot range and extra,
   pressure / forecheck, the brain lane's one lever per level), harder by better play, never by cheating: no lever that speeds bots past
   the human's top speed, takes the puck out of his ring, or raises knockdowns (pro's 1.45 a minute is already a known limit: the set must
   not raise it; lower is welcome). Rookie: leave it winnable; bring it under 92 only if a lever does it without a goal against inside
   the first 600 frames in more than 1 match of 20. Law 3 stays the gate at pro and all_star (stop begun at 8 m at least 90 %, the straight
   charge 0: the contact track's driver after every candidate). Confirm the set at 600 seeds a cell.
3. The gate: `Jam.Curve.*` (re-pin the person-path hashes), the full `Jam` filter in a cleared environment, Shipping compiles; the neutral
   proof for any new key.

`CURVE READY 1` as soon as the table exists; `LADDER SET 4` with the cartridge files by name, the table before / after, and the gate **by
14:15 by the machine clock**. Keep the files closed after the line. If a cell cannot reach its target by then, land what is green and
name the cell and the lever you would try next.
