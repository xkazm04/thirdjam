# Matches that never end, and the long stretches without a touch (lane A71, device A)

Source: lane A53's soak (`SOAK-PLAYTEST.md`; counts below are from a snapshot of 11,400 results: 53 reached the 36,000-frame cap, 49 of 3,850 on hockey.boost and 4 of 7,550 on
hockey.regular; the later snapshot of 15,700 has 72 capped and 137 finished at 30,000 frames or more) and fresh runs of this lane with a per-frame log.
`tools/soak/playtest/idle.mjs` reads those logs.

## 1. The six reproductions (per-frame log, `-trace=30`; the hashes equal the soak's, so the trace changes nothing)

Command: `UnrealEditor-Cmd Game/ThirdJam.uproject -run=JamTournament -sim=jam -cartridge=<cart> -difficulty=<level> -player=<class> -seeds=<n>-<n> -maxframes=36000 -trace=30 -nullrhi -unattended`.
`-trace=N` now also works on the `-player=` path: it prints the brain's TRACE line plus `since=` (play frames since the human's last touch), `ht=` (the human touched this frame), `car=` (carrier), `lt=` (last toucher).

| cart | class / level | seed | score at the cap | hash before (at the cap) | puck at frame 30,000 | what the bots did | after the fix |
|---|---|---|---|---|---|---|---|
| hockey.regular | novice / rookie | 346 | 2:2 | 00ca6237f2b226db | (-23.1,-8.4), 7.9 m/s, loose, nearest skater 1.6 m | chasing; stall flag on in 6 % of samples; human in the box at (-3,-21.8) | 2:1 win at 30,000, c2ce1c85bb2ffbec |
| hockey.regular | novice / rookie | 349 | 2:2 | 1bcc3217b9795eef | in play, moving | same | 1:2 loss at 30,000, 116bc02816d1c186 |
| hockey.regular | regular / all_star | 387 | 2:2 | 8f549adad97658c8 | in play, moving | same | 2:1 win at 30,000, 67847cb781d5f093 |
| hockey.boost | regular / pro | 150 | 2:0 | 4874f7e7f2dd9b60 | (51.8,0.7), 10.3 m/s, human 0.4 m from it, loose | the human shoots at the opposing end, the goalie saves, the puck leaves the zone again | 2:0 win at 30,000, 32bcd59f6e8e51a5 |
| hockey.boost | good / all_star | 251 | 1:1 | ba520b61ad86c26c | in play, moving | same | 1:1 at 30,000, sudden death, hard stop at 33,000: loss (last touch), 2c2a79f2292cc68c |
| hockey.boost | regular / pro | 737 | 1:1 | 5b8d2eddf102b6cf | in play, moving | same | 0:1 loss at 30,000, 98bc3b3ff2fbaf4f |

(Seeds 346 and 737 read 2:2 / 1:1 at the cap and a different score at 30,000 because a goal fell between 30,000 and 36,000 frames in the old run.)

No match had a puck that nobody could reach. Over the six logs the longest stretch of a loose puck slower than 1 m/s was 210 frames (3.5 s), always with a skater within reach; `stall` was on in
4 to 8 % of the samples and never for long; the puck had no carrier in 62 to 72 % of the play frames and was spread over all three thirds of the ice. No pickup or zone swallowed it (three
pickups lie on the ice in boost the whole match), the human parked only in the penalty box, and no bot stayed in the crease.

## 2. The cause

Not a deadlock: a `first_to` match has no clock, and the scoring rate on the big rink at the hard levels is low enough that the tail of the match length is very long. Finished matches
end at p50 9,144, p90 18,739, p99 29,639 frames, and the hazard falls over a long match: the capped ones have one goal per about 12,000 frames against one per 3,300 in a finished boost
pro/all_star match, with 22 opponent shots against 9, so it is long play without finishing while the goalie stops what comes. 1.3 % of boost matches (the 120 x 60 rink at speed 8) and 0.05 % of regular ones did not end in
10 minutes.

## 3. The fix (one rule, `Game/Source/JamCore/Private/JamSim.cpp`)

`MatchHornFrames = 30000` (8:20) and `MatchHardStopFrames = 33000`, both under the 36,000 cap every tool uses. While the puck is live (Phase Playing):
- at the horn a `first_to` match with a leader is won by the leader; a tie turns to sudden death (`bOvertime`, next goal wins, the existing rule, with its `clock sudden_death` event);
- at the hard stop, in any mode, `EndMatchNow(-1)` ends it: the leader wins, a tie goes to the team that touched the puck last (EndMatchNow's own tie rule; the sim has no draw state).

Hash: it changes only for matches that reach 30,000 frames. The six seeds are in section 1. Of the soak's 15,700 results about 209 (1.3 %: 137 finished late, 72 capped) change; the 200 fresh
regular matches of section 4 all keep their hash (0 differ). The frame count includes the goal pauses.

Test: `Jam.Core.MatchAlwaysEnds` (in `JamPlaytester.cpp`) plays the six seeds twice each and wants every match over before the 36,000 cap with the same hash both times: green.
Filter `Jam.Core+Jam.Bot+Jam.Playtest`: 41 tests, 41 success, 0 failures (includes `BundleKeepsRegularHash`).

For the Director: a draw would be the honest result of a tied hard stop, but the sim, the playtester and the UI know only a winner; last-toucher is deterministic and rare (about 0.1 % of matches).
Whether 8:20 is the right horn for a first-to-3 match with a median of 2:30 is a design call (a 6:00 horn would change about 4 % of matches).

## 4. Idle stretches: 200 hockey.regular matches (seeds 1-22 in every class x level cell, plus seed 23 of novice/rookie and good/all_star), fresh runs, per-frame log

Longest stretch without a human touch per match, in play frames (goal pauses taken out); `idle.mjs` agrees with the sim's own `max_no_touch` within 1 frame at p90.

| | p50 | p90 | p99 | max | matches >= 15 s | matches >= 30 s |
|---|---|---|---|---|---|---|
| all 200 | 986 f (16.4 s) | 1,899 f (31.7 s) | 3,752 f | 4,078 f (68 s) | 118 | 23 (12 %) |

| class | level | n | p50 s | p90 s | max s | >= 30 s |
|---|---|---|---|---|---|---|
| novice | rookie | 23 | 21.3 | 34.6 | 44.1 | 4 |
| novice | pro | 22 | 27.3 | 37.4 | 46 | 6 |
| novice | all_star | 22 | 26.4 | 55.4 | 68 | 10 |
| regular | rookie | 22 | 10.5 | 16.3 | 21.4 | 0 |
| regular | pro | 22 | 15.3 | 24.7 | 27.6 | 0 |
| regular | all_star | 22 | 19.2 | 28.3 | 39.6 | 2 |
| good | rookie | 22 | 10.7 | 15.5 | 23.6 | 0 |
| good | pro | 22 | 12.4 | 16.8 | 19.9 | 0 |
| good | all_star | 23 | 16.8 | 26.1 | 35.5 | 1 |

During the p80-p95 stretches (31 matches, 26-40 s): the puck was loose (no carrier) 74 % of the frames, an opponent carried it 16 %, a teammate 7 %, a faceoff pause 3 %, the human in the box 0 %;
it sat in the human's attack third 35 %, neutral 32 %, own defend third 33 %. All 23 matches with a 30 s stretch are loose-puck stretches. The teammates did not keep the puck from the human: in none
of the 31 stretches did a teammate carry it for 30 % of the frames, and the human received 1.2 teammate passes in those matches against 1.4 in all (`mate.pass_to_human` is not the cause).

The soak's 738 of 7,550 regular matches (9.8 %) agree with the 12 % here; this sample is a third novice, and 20 of the 23 are novice or all_star-level matches.

## 5. Proposal (for the Director; no profile or number changed here)

The long stretches are the human, mostly the novice and any class at all_star, not getting to a loose puck; they are not a puck held from him. The lever is the loose-puck phase (novice at
all_star: median 26 s, p90 55 s). Two things to cost against R-0029 with the playtester (`--tune`): (1) a teammate nearest a loose puck deflects or passes toward the human after about 10 s of
no touch (A46's `mate.pass_to_human` fires only from a carry today); (2) a chase bias that yields a loose puck to the human's side after 15 s without a touch at all_star. Both are profile tunes
for lane A3.
