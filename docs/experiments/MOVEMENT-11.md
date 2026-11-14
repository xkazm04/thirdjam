# Movement 11: the breakaway escapes again, the stop at the window's centre, and why the late stops stay (lane A194, track F2, evolution 10, 2026-11-14)

Lane A185 (MOVEMENT-10) made the skaters without the puck 15 % faster. Three consequences were left: the breakaway burst (0.92 x 1.2 = 1.104 of the base)
no longer outran a gliding free chaser (1.15); drill 21's stop at 8 m sat on the edge of a window that had narrowed to 8 to 9 m at pro; and the stops begun
too late nearly doubled. Status with timestamps: `status/A194.md`. Drivers and rows: `tools/perf/results/a194/`; logs `Game/logs/a194/` (not landed).
The main it stands on: 7d944ee1.

## 1. The breakaway escapes again: `skater.breakaway.speed_bonus` 0.2 -> 0.3 (ON)

0.92 x 1.3 = 1.196 of the base, over a free skater's 1.15. Neutral value: 0.2 (`JamBaseline::MakeNeutral`, `Jam.Core.Contact.Neutral`). No shipped
cartridge writes the key (the six cuts, twelve modes and twelve takes take the default; one Designer cached cut writes 0.4 of its own).

| measure | 0.2 (main) | 0.3 |
|---|---|---|
| `Jam.Core.Payoffs.Breakaway`: the slot reached / ahead of the chaser | 156 vs 160 frames, 1.54 vs 0.80 m (A185's re-pin: 3 and 0.5) | **147 vs 160 frames, 3.50 vs 0.80 m** (the bars before A185 again: 8 and 1.5) |
| the human's breakaways a match (CarryTrace, regular at pro, 100 matches) | 2.23 | 2.00 |
| ... ended by his shot | 47.1 % | **52.5 %** |
| ... taken (stick, hit, touch) | 38.1 % | **29.5 %** |
| **... caught from behind** (taken by an opponent behind him along his travel) | **20.2 %, 0.45 a match** | **15.0 %, 0.30 a match** |

The cells (600 paired seeds, the ladder's instrument, regular rink; both arms on one build, the bonus by `--set`):

| cell | 0.2 | 0.3 | change |
|---|---|---|---|
| rookie | 67.8 (64.0-71.4) | 69.3 (65.5-72.9) | +1.5 |
| pro | 53.0 (49.0-57.0) | 54.3 (50.3-58.3) | +1.3 |
| all_star | 37.3 (33.6-41.3) | 33.8 (30.2-37.7) | -3.5 |

Every cell inside 4 points. For the curve lane: these are the cells it re-sets from (all_star moves toward its 26).

## 2. The stop at the window's centre (ON)

Law 3's row at pro on this main (`Bar.Curve.Law3`, 200 seeds): 7 / 7.5 / 8 / 8.5 / 9 / 9.25 / 9.5 / 10 m = 38 / 69 / 94 / **99.5** / 94.5 / 89.5 / 78 / 72 %;
all_star 85 / 95.5 / 99.5 / 100 / 99.5 / 98.5 / 96 / 91; rookie 91 / 98.5 / 100 ... 100; the straight charge 0 at every level.

- **Drill 21's stop: 8 -> 8.5 m** (`FJamCarryDrill::StopAtForLevel`, every level): 99.5 / 100 / 100 % at pro / rookie / all_star (was 94 % at pro at 8 m).
  On screen still "FIVE STICK LENGTHS" (8.5 / 1.6 rounds to five). `Jam.Core.Movement.SecondMove`'s 95 % bar for the drill's distance is back.
- **The stop line's place: 8 -> 8.5 m** (`FStopRule::AheadM`, a drawing only: where the line stands is where the drill now stops).
- **The far edge does not want the half metre: it stays 9.0 m.** 9.25 m passes a pro 89.5 %, 9.5 m 78 %.
- The in-match sweep of the far edge with the scripted human obeying the line (`JAM_PLAYTESTER_STOPMARK=brake`, `JAM_MARK_RULE far=8.5 / 9 / 9.5`)
  played the same matches in all three arms: the instrument obeys the built rule and `JAM_MARK_RULE` moves only the trace's own count. It is not a far-edge
  sweep; law 3's row is. What it does say: a player who stops at every lit line wins 29 of 100 at pro against 58 for the one who does not, and is down
  within 1.5 s of 14.3 % of those stops (MOVEMENT-9's "an offer, not an order" still holds).

## 3. The late stops: no movement number widens the window (all neutral)

Wanted: law 3 at pro 90 % or more from 8 to 10 m, the straight charge 0. One code-only number (`FJamSkaterConfig::CarryReverseBrakeMult`, the carrier's
reverse brake as a multiple of `reverse_brake_decel`, 1 = everyone's) and the existing code-only `BurstCarryAccelMult` (the carrier's first step after a
stop), through the new `JAM_LAW3_SET` of `Bar.Curve.Law3`:

| pro, stop begun at | 7 | 7.5 | 8 | 8.5 | 9 | 9.25 | 9.5 | 10 | straight |
|---|---|---|---|---|---|---|---|---|---|
| as shipped | 38 | 69 | 94 | 99.5 | 94.5 | 89.5 | 78 | 72 | 0 |
| carrier's brake x 1.2 | 60.5 | 91 | 99 | 98 | 88.5 | 85 | 77 | 59.5 | 0 |
| carrier's brake x 1.4 | 81 | 97 | 99.5 | 92 | 84.5 | 76 | 67.5 | 63 | 0 |
| carrier's first step 2.2 (1.8 now) | 53 | 83 | 98 | 100 | 94 | 88.5 | 80 | 74.5 | 0 |
| carrier's first step 2.6 | 63 | 92 | 100 | 99.5 | 93 | 86.5 | 82.5 | 71 | 0 |
| brake x 1.2 and first step 2.2 | 78.5 | 97 | 100 | 95.5 | 88 | 82 | 75 | 69.5 | 0 |

(all_star in `Game/logs/a194/l3-*.txt`: the stronger brake costs it 9 to 13 points at 9 to 9.5 m.)

**A stronger brake moves the window toward the defender; it does not widen it.** Its near end (7 to 7.5 m) is a stop that does not finish before the
hit: the brake buys that. Its far end (9.5 to 10 m) is a stop that finishes too soon: the defender, still coming, adjusts and lifts or waits the carrier
out (time-outs 18 and 24 of 200). No carrier number reaches the far end without taking the near end's metres; the far end is the defender's choice,
which is the brain's. Nothing ships: `CarryReverseBrakeMult` stays a code-only 1 (the step multiplies by 1.0 exactly: hash-identical).

## 4. With the brain lane's `brain7` (A193)

The bots taught the new speed: these numbers would move with it: law 3's far end (9 to 10 m: a defender that knows a free skater's speed arrives on
time rather than early or late; the late-stop table of section 3 should be read again on its tree), the breakaway's "caught from behind" (a chaser that
judges its own speed right gives up or commits differently), and all six cells. The drill's 8.5 m and the line's 8.5 m do not depend on it unless the
window's centre moves.

## 5. Proof
- Full `Jam` filter in a clean environment (`tools/perf/results/a194/jam.sh`): build 5: **292 pass, 0 fail**. The test guards: PASS. Shipping compiles.
- Neutral: the breakaway at 0.2 is main (`MakeNeutral`, `Jam.Core.Contact.Neutral` replay their pinned hashes); the drill's distance and the line's place
  are not sim rules (no hash reads them); `CarryReverseBrakeMult` 1.
- Tests moved: `Jam.Core.Payoffs.Breakaway` (bars back to 8 frames and 1.5 m; default 0.3), `Jam.Core.Movement.SecondMove` (the drill at 8.5 m and 95 %;
  one move against two at 9 m, the far edge), `Jam.Core.Movement.Marks` (the line 8.5 m ahead), `Jam.Core.Contact.Neutral` (the breakaway's neutral).

## 6. Next evolutions (ranked)
1. **The far end of the window is the defender's**: with `brain7` on main, read section 3's table again; if the presser now arrives on time, a stop begun
   at 9.5 m should pass 90 % again with no carrier number. Proof: law 3 at pro 90 % or more from 8 to 10 m on `brain7`'s tree.
2. **The line lights for the stop that works**: light it only when the opponent will be between 8 and 9 m when a quarter-second-late press lands
   (closing speed x 0.25 s inside the window), so obeying it stops costing wins. Proof: the obeying instrument within 5 points of the free one at pro.
3. **Breakaway goals**: the breakaway now ends in a shot 52.5 % of the time; the shot from it (the slot at speed) scores how often? Proof: breakaway
   goals a match and their share of the human's goals, before and after the goalie lane's next step.
