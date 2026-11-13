# The difficulty curve, pass 3: an aggression lever at all_star (lane A79, device A, 2026-11-13)

Ask (H17, R-0039 open point; R-0041 point 1): a lever at all_star that the good player beats and the regular player does not, so all_star speed can come down
from 1.3 toward 1.2 with the good player still at 25 % (R-0029) and the regular player back near 25-30 %. R-0041 reframed the family: difficulty is aggression,
measure the bots hunting the carrier first (body checks from low closing speeds, the lift, knockdowns), with checks taken by the human per match as the second
number. Built on main c45face, **3 v 3 hockey.regular** (the 5 v 5 switch, A83 / A85, comes after; a final curve pass re-measures on it).

## 1. Method

A70's method (CURVE-2.md section 1): `UnrealEditor-Cmd -run=JamTournament -sim=jam -player=<class> -difficulty=all_star -seeds=lo-hi -maxframes=36000`,
8 headless processes, A53's hockey.regular seeds (1-100 of each 150-band, 20 bands): **n 2,000 per class, the same 2,000 matches for every lever** (paired).
Brain levers through `-aitune`, sim levers through a variant cartridge. Win % with the 95 % Wilson interval (about +-2 points at 25 %, +-1.5 at 13 %).
**Separation = good minus regular**; the brief's end state needs it to shrink (R-0041 reading: the gap must close, then a lower speed lifts everyone).
Checks taken = landed checks on the human a match (equal to its knockdowns in every cell). Two orphaned copies of one driver chain wrote one cell twice:
every figure is deduplicated by seed.

## 2. Every lever (all_star, speed 1.3 unless named; n 2,000 per class)

| lever | novice | regular | good | good - regular | checks taken r / g |
|---|---|---|---|---|---|
| baseline (speed 1.3, checks from 3 m/s, poke after 2, lift on) | 2.6 | 13.3 | 26.8 | 13.5 | 1.97 / 3.00 |
| intercept lead 60 -> 120 frames | 3.3 | 14.2 | 26.0 | 11.8 | 1.98 / 2.91 |
| checks from 2 m/s (`CheckMinClosing`) | 2.7 | 12.7 | 25.4 | 12.7 | 2.00 / 3.05 |
| checks from 1 m/s | 2.8 | 13.3 | 24.9 | 11.6 | 2.02 / 3.07 |
| checks from 2 m/s, no lift | 7.2 | 24.0 | 42.1 | 18.1 | 2.98 / 4.38 |
| poke after 0 frames | 2.6 | 12.3 | 22.6 | 10.3 | 1.99 / 3.27 |
| sim `skater.check_speed_min` 5 -> 3 + checks from 2 (every skater) | 2.8 | 11.4 | 25.6 | 14.2 | 2.45 / 3.61 |
| sim `check_speed_min` 5 -> 2 + checks from 1 (every skater) | 2.4 | 12.1 | 25.5 | 13.4 | 2.72 / 3.90 |
| checks from behind allowed | 3.3 | 14.7 | 29.6 | 14.9 | 3.12 / 4.42 |
| sim `stick.lift_range_m` 1.0 -> 1.5 (every skater) | 3.0 | 13.4 | 21.1 | **7.7** | 2.41 / 4.05 |
| **hunt 10 m** (new `HuntRadiusM`) | 2.0 | 11.8 | 21.4 | 9.6 | 2.07 / 3.04 |
| hunt 20 m | 2.3 | 8.7 | 18.1 | 9.4 | 2.01 / 3.11 |
| hunt 10 m + poke 0 | 2.1 | 8.3 | 17.4 | 9.1 | 1.98 / 3.27 |
| hunt 10 m + poke 0, speed 1.25 | 2.6 | 12.3 | 20.9 | 8.6 | 2.05 / 3.37 |
| **hunt 10 m + poke 0, speed 1.2** | 3.4 | 14.4 | 27.0 | 12.6 | 2.29 / 3.12 |

Findings.
1. **Low-speed checks do nothing for the win share.** A bot's check lands only on the checker's own speed above the sim's `check_speed_min` (5 m/s), so
   `CheckMinClosing` below 3 adds no knockdown. Lowering the sim's minimum (3, 2) does add knockdowns (+0.5 to +0.9 a match on the regular and the good
   player) but moves no win share beyond the interval and does not close the gap: a check costs the regular player as much as the good one.
2. **Checks from behind** make all_star easier for everyone (the bots' fouls hand the human power plays): the wrong way.
3. **The lift is the bots' strongest skill.** Off: every class gains (good +15, regular +11). Its reach 1.0 -> 1.5 m is the best separator measured: good -5.7,
   regular +0.1 (gap 7.7). It is the sim's key and every skater's, the human's lift included: outside this lane (see section 5).
4. **The hunt** (a new skill field, below) is the best brain lever: at 1.3 it costs the good player 5.4 and the regular 1.5 (gap 9.6); 20 m and the instant
   poke stack on it but the gap stays near 9. It adds sticks and lifts on the carrier, not landed checks (checks taken unchanged).
5. **The gap re-opens as speed comes down.** With the hunt and the instant poke, speed 1.2 puts the good player at 27.0 (inside 25 +- 3) and the regular player at
   14.4 (+1.1 on the baseline, inside its interval): the aggression levers take the win rate that speed 1.3 took (R-0041: speed back to 1.2), but no skill lever
   lifts the regular player toward 25-30 at the speed where the good player is 25. The good player's edge is speed and space (shots 10.9 vs 4.8 a match);
   a pressure lever closes the gap only while the bots are faster than it.

## 3. The new field

`FJamBotSkill::HuntRadiusM` (JamAICore.h; default 0 = off, so rookie, pro and the mates do not move): a support bot within this many metres of an opposing
carrier goes at it as a second chaser (`Decide`: the support branch's pounce test, `D < PounceRadius || bHunt`), and plays stick, lift and check as the chaser
does. A yielding bot stays out. Tune key `opp.hunt_radius_m` / `mate.hunt_radius_m`.

## 4. The chosen pair (baked into `MakeAllStar`)

**Speed 1.3 -> 1.2, poke after 2 -> 0 frames, hunt 10 m.** Tune measurement: novice 3.4, regular 14.4, good 27.0; checks taken r / g 2.29 / 3.12.

Re-measured with the numbers built in and no override (n 2,000, the same seeds): **novice 3.4 (2.7-4.3), regular 14.4 (12.9-16.0), good 27.0 (25.1-29.0)**,
equal to the tune row match for match. Checks taken a match: novice 1.04, regular 2.29, good 3.12 (baseline 0.93 / 1.97 / 3.00).

Untouched levels, re-measured on this build (n 2,000): **pro (regular) 51.0 (48.9-53.2), rookie (novice) 85.8 (84.2-87.3)**: equal to CURVE-2's figures.

Gate (`tools/analysis/jamtests.ps1 -Filter "Jam.Playtest+Jam.Bot+Jam.Core.Determinism"`): **12 passed, 0 failed**, Determinism green
(AiBotPlays, BoostPickupsBundle, BundleKeepsRegularHash, Determinism, DifficultyProfilesResolve, HumanHasSomethingToDo, NoviceScoresAtRookie, PassesToHuman,
PlayerBotWins, ProCharge, SeasonTakes, VisualSwap).

## 5. For the ruling

- The brief's end state (good 25, regular 25-30 at all_star) is **not reachable with a skill lever on 3 v 3**: every brain lever that closes the gap at 1.3
  loses it again as speed comes down. What the pass delivers is R-0041's form of it: speed 1.3 -> 1.2 with the aggression levers carrying the difficulty,
  good 27.0, regular 14.4 (unchanged within its interval), novice 3.4.
- **The sim lever that would do more**: `game.stick.lift_range_m` 1.0 -> 1.5 on top of the chosen pair (variant cartridge, every skater): novice 3.0, regular
  **16.3**, good **24.8** (gap 8.5 at speed 1.2; checks taken r / g 2.73 / 3.89). It touches every level and the human's own lift, so it needs a per-side or
  per-level reach in the sim (lane A82's file) and rookie / pro re-measured; asked, not done.
- **Knockdowns every shift** are not within a skill field's reach: a bot's check lands only above the sim's `check_speed_min` (5 m/s) of its own speed.
  Lowering it to 2-3 adds about 0.5-0.9 knockdowns a match with the win shares flat. A per-side `check_speed_min` (the modifier stat `CheckSpeedMin`
  exists) is the sim's lever for "a beating massacre" if the operator wants the knockdown count itself, separately from the win share.
- Everything here is 3 v 3; the 5 v 5 build (A83: all_star good 40 -> 25 with the old profiles) moves the curve, and the final pass re-measures there.
