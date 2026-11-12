# The season of takes (lane A17, C-0005 section 1): cuts as progression

Status: 2026-11-12 H7, design written first (brief deliverable 1). Owner: lane A17. Data: `Game/Content/Cartridges/ladder/`. Code: `Game/Source/JamGame/*/Progression*`.

## The idea in one paragraph

A season is a shooting schedule. Each **take** is one cut (a cartridge) with ONE new twist on top of everything the player has already seen, and the
Director (the game) gives a one-line **director's note** on the TAKE card before the puck drops. Win the take ("THAT'S A WRAP") and the next take
unlocks; lose (or draw on the clock) and it is a **RETAKE** of the same cut. Difficulty rises rookie -> pro -> all_star across the season, the
twists rise from "a shorter scene" to "the rink shrinks while you play". A played take also appears in "Choose your cut" so the player can replay it
as a free cut (the theme: the season builds your library of cuts).

Judging (PLAN.md 2, "level design and game progression", 5 points): eight distinct levels, a visible difficulty curve with measured win rates
(BALANCING.md), one new idea per level, each level introduced by a line of text, the next level unlocked by winning it, progress saved.

## The ladder

| # | Title (card) | Cut built from | Level | The ONE twist | Director's note (TAKE card, `twist_text`) | Needs |
|---|---|---|---|---|---|---|
| 1 | The Read-Through | Regular Cut 90 x 45, speed 11 | rookie | a short scene: **first to 2** | "Take one. Nobody is judging yet: find the net twice." | today |
| 2 | The Big Room | Large Rink 120 x 60, speed 8 | rookie | **the clock**: most goals in 3:00 (a draw is a retake) | "We moved to the big stage. More ice, and the clock is rolling." | today |
| 3 | Rough Cut | Regular 90 x 45 + checks | pro | **body checks** on (check speed 5 m/s, knockdown 90 frames) | "Drop the manners. Hit them, but keep the puck in frame." | today |
| 4 | Set Pieces | Regular 90 x 45 + 4 pillars | pro | **props on the ice**: four pillars between the blue lines | "The art department dressed the set. Play around the props, or off them." | today |
| 5 | Second Wind | Regular 90 x 45, boards 0.8, neon + effect `second-wind` | pro | **an effect**: every goal gives your team +25 % skating speed for 300 frames | "Rubber boards, and every goal gives your crew a second wind. Bank it in." | A14 |
| 6 | The Last Word | Regular 90 x 45, first to 3 | pro | **a rule variant**: `last_goal_double` | "The last goal is the one they remember. Make it yours." | A15 |
| 7 | Slow Motion | Wide Rink 140 x 70 + zones | pro | **zones**: slow-ice circle (r 10) at centre, two fast-ice lanes on the boards | "Widescreen. The far net is a long way off: take the fast way round." | A15 |
| 8 | The Final Cut | Regular 90 x 45, checks, neon + `shrink_per_goal` 0.1 + 2 bumpers | all_star | **the set closes in**: the rink shrinks 10 % per goal, two bumpers | "Final cut against the best cast: hits allowed, and the set may close in." | A15 |

Fallbacks (R-0014: `takes[].fallback`; the fallback plays when the loader rejects the designed cut). Rule: each designed take 5-8 is its fallback plus the
one primitive that may not have landed (take 6 swaps the variant for first-to-1), so the one director's note fits both and the ladder title stays. Files `take-N-fallback.cartridge.json`:

| # | Fallback title | Fallback cut (today's primitives only) | Level | Fallback twist | Earlier note (superseded: the ladder's note plays) |
|---|---|---|---|---|---|
| 5 | Rubber Boards | Regular 90 x 45, board restitution 0.8, neon look | pro | **live boards**: everything bounces back into play | "Somebody swapped the boards for rubber. Bank it in." |
| 6 | One Goal | Regular 90 x 45, first to 1 | pro | **sudden death**: one goal ends the scene | "One take, one goal. Whoever scores wins the scene." |
| 7 | Wide Shot | Wide Rink 140 x 70, speed 8, first to 3 | pro | **the widest set** | "Widescreen. The far net is a long way off: get there first, three times." |
| 8 | Everything We Shot | Regular 90 x 45, body checks, neon look | all_star | **the best cast**, hits allowed, lights up | "Final cut. Hits allowed, lights up, against the best cast we have." |

At 13:40 all four designed takes 5-8 validate in Node (A14/A15 schema landed); whether they play is decided at load by the C++ loader. Measured curve for the designed 5-8: pending the 14:00 build.


Why this order (the curve):
- Difficulty: rookie 1-2, pro 3-7, all_star 8. Two easy takes teach the controls, five middle takes teach one system each at a fair level
  (the wide rink of take 7 is the hardest of them), take 8 is the final boss. A retake costs one match, so 25 % is a fair final boss.
  (Design had all_star on 7-8; the wide rink measured 0 % at all_star, so take 7 plays at pro.)
- Take 3 note (R-0013): once lane A16's stick duels land (poke checks, lifts, fouls; bots check at pro and all_star, R-0014 2), take 3 becomes "stick duels": Regular, pro, checks and `rules.penalties` on. Same slot, same note.
- Twist rhythm: rules of the match (1, 2) -> the body (3) -> the set (4) -> the player's power (5) -> the rules again, now as a twist (6) ->
  the ice itself (7) -> all of it (8). No take adds two new things; take 8 recombines.
- Every take changes how a goal is scored, not only how the rink looks: shorter scene, the clock, the hit, the bank shot off a pillar, the boost
  after a goal, the double last goal, the slow centre, the shrinking rink.
- Look tier rises too: l2_night for 1-4 and 6-7, l3_neon for 5 and 8 (the boost and the finale), so the season ends on the most dressed set.

## Measured curve (shipped cartridges, scripted human vs AI, `node tools/sweep/run.mjs --runner ue --rinks ladder/take-N`, 13:30 build)

| Take | Level | Seeds | Human W-L | Win % | Goals/match | Match s (mean) | Note |
|---|---|---|---|---|---|---|---|
| 1 The Read-Through | rookie | 1-10 | 8-2 | 80 | 2.3 | 80 | first to 2 |
| 2 The Big Room | rookie | 1-10 | 7-3 | 70 | 2.1 | 202 | a timer draw counts as a loss |
| 3 Rough Cut | pro | 1-10 | 4-6 | 40 | 4.0 | 135 | identical to Regular pro: neither bot checks, so checks are the human's tool only |
| 4 Set Pieces | pro | 1-10 | 5-5 | 50 | 4.3 | 133 | pillars help the human slightly (Regular pro 40) |
| 5 Rubber Boards | pro | 1-20 | 12-8 | 60 | 4.3 | 132 | measured as `hockey.regular --set rink.board_restitution=0.8` (same game) |
| 6 One Goal | pro | 1-10 | 5-5 | 50 | 1.0 | 20 | |
| 7 Wide Shot | pro | 1-20 | 4-16 | 20 | 4.1 | 270 | all_star: 0 of 10 (rejected) |
| 8 Everything We Shot | all_star | 1-20 | 5-15 | 25 | 4.3 | 131 | |

Rejected fallbacks: Glass Ice (friction 0.004, boards 0.75, puck drag 0.3) 30 % at pro and 10 % with drag 0.5; Wide Shot on a 3:00 clock at all_star 0 %;
take 8 with checks + 4 pillars at all_star 90 % (10 seeds; pillars break the all_star lanes). Jam.Bot.SeasonTakes gates takes 1-4 (seeds 1, 2, 7).

## What the player unlocks

- Winning take N unlocks take N+1 (`unlock: "previous_won"`; take 1 is `"always"`). Losing, or a draw on the clock, keeps the player on take N (RETAKE).
- A take the player has **played** (won or not) is listed in "Choose your cut" under its title, so the season fills the cut library.
- Winning take 8 completes the season: the menu entry then reads "Season wrapped" and replays take 8 (results kept).

## Flow and seams

- Menu (A8): one new entry under "Choose your cut": **Play the season** with a subline ("Take 3 of 8: Rough Cut"). It calls `UJamProgression::StartSeason`,
  which starts the current take through `UJamMenuFlow::StartMatch` with the take's cartridge path.
- In the match: the opening TAKE card (A8's overlay) shows the director's note instead of "ACTION" when the running cut is a season take.
- At the end: the game mode reports the result once (`UJamProgression::RecordResult`); A5's win/lose card hint reads
  "R  NEXT TAKE" after a win, "R  RETAKE" after a loss. R goes through the game mode's existing restart path: on a season win it opens the next take, otherwise it restarts.
- `Saved/Progress.json` (`tj.progress/1`): which takes were played and won, attempts, best score. Unknown keys are errors; a missing or rejected file is
  "start of season". Written after every recorded result.

## Data

- `Game/Content/Cartridges/ladder/season.ladder.json`: `{ "schema": "tj.ladder/1", "ladder": { "id", "takes": [ { "title", "cartridge", "twist_text", "unlock" } ] } }`.
  The `ladder` object is C-0005's `game.ladder` verbatim; the envelope adds only the schema tag so the file identifies itself (unknown keys are errors).
  `cartridge` is a preset id: the take's file is `ladder/<id>.cartridge.json` and that file's `id` equals it (`take-1` ... `take-8`).
- `schema/ladder.schema.json` + `schema/ladder.mjs` (validator: schema, 1..12 takes, unique cartridges, take 1 `always`, every cartridge file exists and
  validates as a cartridge) + `schema/ladder.test.mjs`.

## Full versions of takes 5-8 (when A14 / A15 land; C-0005 section 4 names)

```
take-5 "Second Wind": hockey.boost game + "effects": [ { "id": "second-wind", "kind": "speed", "target": "own_team", "duration_frames": 300,
        "stats": [ { "stat": "skater_speed", "multiply": 1.25 } ], "trigger": "goal_for", "stack": "refresh" } ], ai pro, l3_neon
take-6 "The Last Word": regular game + "rules": { ..., "variants": { "last_goal_double": true } }, ai pro
take-7 "Slow Motion": regular game + "rink": { ..., "zones": [ { "id": "centre", "shape": "circle", "x": 0, "z": 0, "r": 8, "kind": "slow_ice", "strength": 0.5, "visible": true },
        { "id": "north-lane", "shape": "rect", "x": 0, "z": 19, "w": 50, "d": 4, "kind": "fast_ice", "strength": 1.4, "visible": true },
        { "id": "south-lane", "shape": "rect", "x": 0, "z": -19, "w": 50, "d": 4, "kind": "fast_ice", "strength": 1.4, "visible": true } ] }, ai all_star
take-8 "The Final Cut": neon regular + "rules": { ..., "variants": { "shrink_per_goal": 0.9 } }, "rink": { ..., "obstacles": [ two "bumper" ],
        "zones": [ the slow centre circle ] }, take 5's effect, ai all_star
```

## Questions for the Director (contract)

1. C-0005 has no `fallback` key per take; this design ships the fallback as the take's cartridge file and swaps the file when A14/A15 land. If the Director
   wants both in data, propose `takes[].fallback: <preset id>` (used when the primary cartridge is rejected by the loader).
2. The word "take" now means both the in-match period (TAKE 1/2/3 of the theme skin) and a season level. The TAKE card keeps the period number and shows
   the season's director's note on its subline; the season level is named by its title ("Rough Cut") in the menu and the end card to keep the two apart.
3. Built-in pickups do not spawn today (the sim stores `game.pickups` but has no spawner; A14 deliverable 3), so pickups are not used in takes 1-4.
