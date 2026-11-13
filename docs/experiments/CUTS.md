# Every shipped cut plays (lane A37, R-0028 point 3, 2026-11-12 H11.5)

Status: swept, fixed, re-swept 19:14-20:10. Owner: lane A37. Tools: `tools/cuts/sweep.mjs` (the sweep and its table), `tools/cuts/text.mjs` (player-facing
text through the bark filter), `tools/cuts/stills.mjs` (one still per look tier and zone kind through the editor's MCP), commandlet `-run=JamCutSweep`
(`Game/Source/JamCore/AI/JamCutSweepCommandlet.{h,cpp}`, new). Outputs: `tools/cuts/out/sweep.{md,json,jsonl}`, `tools/cuts/out/text.md`,
`docs/experiments/img/cuts/*-24m.png`, `docs/experiments/eyes/a37-cuts-readability-{1,2,3}.json`.

## What was swept and how

50 cartridges: the 30 offline Designer cuts (`Game/Content/Designer/cached-cuts.json`, written out to `tools/cuts/out/cuts/<slug>.cartridge.json` so the C++
loader reads a file), the 8 season takes and their 4 fallbacks (`Game/Content/Cartridges/ladder/`), the 6 shipped presets and the 2 test presets
(`hockey.*.cartridge.json`). Each: 5 seeds (1-5), 36,000 frames max, the real brain at the cut's own level (24 cuts pro; Family, Rookie, Pond, take-1, take-2
rookie; All-Star, take-8 and its fallback all_star), the scripted human (`PlayerBot`, the same one `Jam.Bot.SeasonTakes` uses). 203 s wall clock, 3 commandlets at a time.

**Finding before any edit (the reason for the new commandlet).** The headless path everyone measured on (`JamTournament` -> `FJamSimAdapter` -> `UJamSim`) never
creates `UJamRuleSystem`, `UJamZoneSystem` or `UJamEffectSystem`; only `AHokejGameMode::BuildMatch` does. So `Jam.Bot.*`, `Jam.Bot.SeasonTakes` and
`tools/sweep` run every cut without its hooks, variants, zones and pickups: a sweep on that path would not have tested what the cuts add. `JamCutSweep`
subclasses the adapter and wires the three systems exactly as the game mode does (rule hook before `Init` so the opening faceoff reaches it, zone source before
`Init`, effects after; per frame the score mirror, the zone update, the effect step; the brain's world view gains the slow-ice circles and `no_shoot` flags the
game mode adds) and appends a `sweep` object to the tournament JSON line: hooks and fires, `end_match` / `end_period` actions, pickups spawned / taken / taken by
the human, effects started, zone entries, periods, announces, shots blocked, checks, knockdowns, fouls, rink revision and size, end cause, effect-system errors.

Verdicts: **plays** | **ends late** (a seed never ended; a first-to cut whose mean runs past 18,000 frames or whose worst seed runs past 27,000; a timer cut whose
worst seed runs past 1.5 x its clock, since frames count goal pauses and faceoffs) | **no human touch** (a faceoff left the scripted human without a touch for
600 frames, or under 3 touches a match) | **never ends** | **loader rejects** | **no goals** (ended by its clock with no goal in any seed). A worst seed over
18,000 frames that is inside the bar is noted as "long".

## Counts

| | first sweep (19:26) | after the fixes (19:50) |
|---|---|---|
| cartridges | 50 | 50 |
| plays | 44 | 47 |
| ends late | 6 (Slow Motion Cut, Marathon Cut, Maze Cut; hockey.boost, hockey.raw, hockey.test.visual-b2) | 3 (the three presets, not mine to edit: see Director) |
| no human touch / never ends / loader rejects / no goals | 0 / 0 / 0 / 0 | 0 / 0 / 0 / 0 |
| fixed by hand | | 3 (listed below) |
| deferred (the sim lacks the idea) | | 0 |

Every one of the 50 loads. No cut leaves the human without a touch (the scripted human took 17-248 touches a match; the lowest, 17, is hockey.chaos, which
sudden-death ends in 37 s). The systems do run: pickups spawned / taken 27 / 19 on Power-Up (human 5), 52 / 29 on Turbo (human 5), 13 / 8 on Giant Puck (human 0),
37 / 23 on hockey.boost (human 6); Second Wind started its effect 21 times in 5 matches; Set Pieces ended 4 of 5 by its first-to-three hook; Sudden Death ended 4
by rule; The Final Cut shrank the rink to 65.6 x 32.8 m and hockey.chaos to 81 x 40.5; take-7 counted 386 zone entries. Checks: 0 in every cut (neither bot nor the
scripted human checks; LADDER.md already says so), so Rough Cut, take-8 and hockey.raw play as their check-less twins (identical hashes to Regular / All-Star).

## The table (after the fixes; `tools/cuts/out/sweep.md`)

| cut | group | level | rules | seeds | W-L-D | ended | end by | frames mean/max | goals | human touches | no-touch faceoffs | pickups taken (human) | hooks/fires | zones/enters | periods | verdict | detail |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Fast Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 8528/13422 | 4.4 | 75.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Slow Motion Cut | cuts | pro | timer 14400 | 5 | 3-2-0 | 5/5 | rule/timer | 12886/15123 | 3.6 | 126.8 | 0 | 0 (0) | 1/3 | 0/0 | 3 | plays |  |
| Family Cut | cuts | rookie | first to 3 | 5 | 5-0-0 | 5/5 | score | 10575/18929 | 3 | 50.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 18929 frames |
| Pinball Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 13191/20476 | 3.8 | 124.4 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 20476 frames |
| Big Glass Cut | cuts | pro | first to 3 | 5 | 2-3-0 | 5/5 | score | 13959/23569 | 4.4 | 66.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 23569 frames |
| Shoebox Cut | cuts | pro | first to 3 | 5 | 2-3-0 | 5/5 | score | 6901/11659 | 4.2 | 83.4 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Open Ice Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 14671/18951 | 4.2 | 112.6 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 18951 frames |
| Glass Ice Cut | cuts | pro | first to 3 | 5 | 2-3-0 | 5/5 | score | 10373/14734 | 4.4 | 85 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Carpet Cut | cuts | pro | first to 3 | 5 | 4-1-0 | 5/5 | score | 9290/15917 | 3.8 | 98.6 | 0 | 0 (0) | 0/0 | 1/291 | 1 | plays |  |
| Power-Up Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 10715/14702 | 4.2 | 113 | 0 | 19 (5) | 0/0 | 0/0 | 1 | plays |  |
| Turbo Cut | cuts | pro | first to 3 | 5 | 2-3-0 | 5/5 | score | 7579/17147 | 4 | 71.2 | 0 | 29 (5) | 0/0 | 0/0 | 1 | plays |  |
| Giant Puck Cut | cuts | pro | first to 3 | 5 | 4-1-0 | 5/5 | score | 10683/14507 | 4.2 | 113.2 | 0 | 8 (0) | 0/0 | 0/0 | 1 | plays |  |
| Neon Cut | cuts | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 8899/17958 | 4 | 93.4 | 0 | 0 (0) | 1/20 | 1/292 | 1 | plays |  |
| Matinee Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Easy Scoring Cut | cuts | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 8004/12200 | 3.2 | 87.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| All-Star Cut | cuts | all_star | first to 3 | 5 | 2-3-0 | 5/5 | score | 8732/11783 | 4.4 | 72.6 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Rookie Cut | cuts | rookie | first to 3 | 5 | 5-0-0 | 5/5 | score | 15081/18906 | 4 | 103.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 18906 frames |
| One Minute Cut | cuts | pro | timer 3600 | 5 | 4-1-0 | 5/5 | timer | 4196/4323 | 2.4 | 44.4 | 0 | 0 (0) | 0/0 | 0/0 | 3 | plays |  |
| Marathon Cut | cuts | pro | timer 18000 | 5 | 2-3-0 | 5/5 | timer/rule | 20283/22697 | 9.4 | 199.4 | 0 | 0 (0) | 1/1 | 0/0 | 3 | plays | long: worst seed 22697 frames |
| Sudden Death Cut | cuts | pro | first to 1 | 5 | 4-1-0 | 5/5 | score/rule | 2435/4996 | 1 | 25.8 | 0 | 0 (0) | 1/4 | 0/0 | 1 | plays |  |
| Pillar Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 14619/15929 | 5 | 139.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Maze Cut | cuts | pro | timer 14400 | 5 | 3-2-0 | 5/5 | timer/rule | 14790/15123 | 2.4 | 182.2 | 0 | 0 (0) | 1/1 | 0/0 | 3 | plays |  |
| Rocket Cut | cuts | pro | first to 3 | 5 | 1-4-0 | 5/5 | score | 12315/17201 | 4.2 | 99.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Big Net Cut | cuts | pro | first to 3 | 5 | 4-1-0 | 5/5 | score | 7715/10683 | 3.8 | 73.6 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Lockdown Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 7908/9700 | 4 | 79.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Pond Cut | cuts | rookie | first to 3 | 5 | 5-0-0 | 5/5 | score | 10581/14425 | 3.2 | 33.6 | 0 | 0 (0) | 0/0 | 1/306 | 1 | plays |  |
| Final Cut | cuts | pro | first to 5 | 5 | 3-2-0 | 5/5 | score | 13439/15422 | 6.4 | 129 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Arcade Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Rubber Boards Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 8113/12378 | 4.2 | 76.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Midnight Cut | cuts | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| The Read-Through | ladder | rookie | first to 2 | 5 | 5-0-0 | 5/5 | score | 9983/18315 | 2.6 | 69 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 18315 frames |
| The Big Room | ladder | rookie | timer 10800 | 5 | 5-0-0 | 5/5 | timer | 11489/11985 | 2.4 | 45 | 0 | 0 (0) | 0/0 | 0/0 | 3 | plays |  |
| Rough Cut | ladder | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Set Pieces | ladder | pro | timer 10800 | 5 | 4-1-0 | 5/5 | rule/timer | 7915/11523 | 3.6 | 86.6 | 0 | 0 (0) | 1/4 | 0/0 | 3 | plays |  |
| Second Wind | ladder | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9081/14959 | 4.2 | 79.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Rubber Boards | ladder-fallback | pro | first to 3 | 5 | 2-3-0 | 5/5 | score | 4849/7015 | 3.4 | 43.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| The Last Word | ladder | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 1/5 | 0/0 | 1 | plays |  |
| One Goal | ladder-fallback | pro | first to 1 | 5 | 4-1-0 | 5/5 | score | 2435/4996 | 1 | 25.8 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Slow Motion | ladder | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 13661/24762 | 4.2 | 85.6 | 0 | 0 (0) | 0/0 | 3/386 | 1 | plays | long: worst seed 24762 frames |
| Wide Shot | ladder-fallback | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 16113/20102 | 4.4 | 119.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 20102 frames |
| The Final Cut | ladder | all_star | first to 3 | 5 | 1-4-0 | 5/5 | score | 9654/13233 | 4 | 83.2 | 0 | 0 (0) | 1/20 | 0/0 | 1 | plays |  |
| Everything We Shot | ladder-fallback | all_star | first to 3 | 5 | 2-3-0 | 5/5 | score | 8732/11783 | 4.4 | 72.6 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Large Rink Cut | presets | pro | timer 10800 | 5 | 3-2-0 | 5/5 | timer | 11917/13512 | 2.6 | 100.2 | 0 | 0 (0) | 0/0 | 0/0 | 3 | plays |  |
| Hockey Arena Wide | presets | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 16113/20102 | 4.4 | 119.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays | long: worst seed 20102 frames |
| Hockey Boost | presets | pro | first to 3 | 5 | 2-2-0 | 4/5 | score/none | 21549/36000 | 4.2 | 160.4 | 0 | 23 (6) | 0/0 | 0/0 | 1 | ends late | 1 of 5 did not end; mean 21549, longest 36000 frames |
| Hockey Chaos | presets | pro | first to 3 | 5 | 2-3-0 | 5/5 | rule | 2225/5158 | 1.2 | 17 | 0 | 0 (0) | 2/11 | 1/82 | 1 | plays |  |
| Hockey Raw | presets | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 19755/30387 | 4 | 163.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | ends late | 0 of 5 did not end; mean 19755, longest 30387 frames |
| Regular Cut | presets | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Hockey Test Visual A26 | preset-test | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 9116/12097 | 3.8 | 82.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | plays |  |
| Hockey Test Visual B2 | preset-test | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 19755/30387 | 4 | 163.2 | 0 | 0 (0) | 0/0 | 0/0 | 1 | ends late | 0 of 5 did not end; mean 19755, longest 30387 frames |

## The fixes (hand edits to `cached-cuts.json`, listed per R-0028 / the brief; `gen-designer-data.mjs` re-run: 30 cuts valid, `JamDesignerData.inl` regenerated)

The pattern is take-4's, already shipped: a clock plus a `first-to-N` hook, so the match keeps its score rule and the clock ends what the score does not (a tie at
the horn goes to the sim's overtime, next goal wins). Each was measured as a variant file (`sweep.mjs --file`) before it went into the library.

| Cut | Before | After | Measured before -> after (5 seeds) |
|---|---|---|---|
| Slow Motion Cut ("make it slower and easier to follow") | `rules: { mode: first_to, score_to_win: 3 }` | `rules: { mode: timer, timer_frames: 14400, hooks: [ { id: first-to-three, when: on_goal, team: scoring, score: { at_least: 3 }, then: [ { do: end_match, team: scoring } ] } ] }` | mean 15,976 / worst 29,074 frames (8.1 min) -> 12,886 / 15,123; 3-2-0; ends by the hook in 3 seeds, the clock in 2 |
| Marathon Cut ("a long match, first to nine") | `first_to 9` | `timer 18000` (the schema's longest clock, 5:00) + `first-to-nine` hook | mean 25,838, 1 of 5 unfinished at 36,000 -> 20,283 / 22,697; 9.4 goals a match; nine reached in 1 of 5, the clock ends the rest |
| Maze Cut ("a maze of boxes on the ice") | 12 boxes of 3 m, one at (0, 0) on the faceoff dot; `first_to 3` | 11 boxes (the centre box removed); `timer 14400` + `first-to-three` hook | mean 20,610 / worst 35,814 -> 14,790 / 15,123; 2.4 goals. The clock alone still left a 27,073 seed and the box alone a 23,115 seed; both were needed |

Not changed, noted as long (worst seed over 5 min, mean under it): Family Cut 18,929, Pinball 20,476, Big Glass 23,569, Open Ice 18,951, Rookie 18,906, take-1
18,315, take-7 24,762, Wide Shot / hockey.arena.wide 20,102. Slow or big rinks at first-to-3 run long at pro; a clock would change their sentence, so they stay.

## Deferred: none. Bugs for the Director instead

1. **Effect trigger `zone_enter` / `zone_exit` is dead everywhere.** `UJamEffectSystem::Step` scans `Sim->GetFrameEvents()` for `zone_enter`, but those events are
   produced by `UJamZoneSystem` (its own `GetFrameEvents()`), which the game mode only broadcasts to the bus. Carpet Cut's `carpet-stick` effect (puck drag x3 on
   entering the band): 291 entries, 0 effects started, in the sweep and in the game alike. The cut still plays its idea through the `slow_ice` zone (strength 0.6
   slows skaters and puck), so it is not deferred. Fix is A14 / A15's seam: the effect system needs the zone system's events (one `OnZoneEvents(const TArray<FJamSimEvent>&)`
   the game mode and the sweep call after `ZoneSystem->Update`).
2. **Team colours are unreadable at 24 m in every still** (eyes `team_colours` 1 of 5 on all 8; my own look agrees: the figures are black silhouettes at this
   distance, the blue / orange only on the scoreboard). Not a cut bug; the figure's team fill at distance (A21 / A26 / B).
3. **Three presets run late at pro**: hockey.boost (mean 21,549, 1 of 5 unfinished), hockey.raw and hockey.test.visual-b2 (mean 19,755, worst 30,387): the 120 x 60
   rink at speed 8 first-to-3 at pro averages 5.5 min (A3's sweep saw the full window on it too). hockey.arena.large on the same rink plays because it is on the
   3:00 clock. The same take-4 pattern (a clock plus the first-to-three hook) would close them; presets are not mine to edit.
4. **The slow-ice look covers the lines** on a big zone: Carpet Cut's 40 x 45 m band is a dense cyan grid over most of the ice; eyes scores the zone strips 2 of 5
   there (5 on plain ice). A21's `MI_Zone_slow_ice` at that size is louder than the rink markings.
5. **A pickup at 24 m is a small cyan ring** (Power-Up still): it renders (A14's actor with B's `SM_Pickup_*` or the fallback) but is easy to miss; no token height reads.

## Player-facing text (`tools/cuts/text.mjs`, 111 lines through `tools/barks/filter.mjs`)

Rule applied: a banned word anywhere, or a hard film word on the ice (scoreboard announce, the match card's note) fails; in the menu the film word is the theme
(R-0021 point 3). The bank's own rules (6-60 characters, no capitalised word mid-line) are notes on a title or a card.

- **Fails (3):** "Sudden Death Cut" (name and sentence: `death` is on the bank's banned list; it is hockey vocabulary and the loader's own sudden-death announce says
  "SUDDEN DEATH: that goal wins it", so I recommend an allow-list entry for the phrase, no rename); take-8's note **"Final cut against the best cast: hits allowed, and
  the set may close in."** carries the hard word `cut` on the match card (R-0021 point 2); a hockey rewrite at the same length for A17 / the Director: "Last game against
  the best they have: hits allowed, and the ice may close in."
- **Notes (77):** every cut name ends in "Cut" (menu text; allowed); take notes carry `take` (take-1, take-7), `frame` (take-3), `props` (take-4) as soft words;
  five notes run 62-72 characters (the card shows them whole; no bark limit applies); Neon Cut's announce "NEON GOAL! PARTY TIME" is clean (the capitals trip the
  name rule only).
- **Does the sentence say what the cut does (one line each):** Fast yes (speed 14.5). Slow Motion yes (speed 8, puck 22, shot 14; now on a 4:00 clock). Family yes
  (slow, rookie, quiet barks). Pinball yes (boards 0.95, 4 bumpers, puck 42). Big Glass yes (160 x 80, friction half). Shoebox yes (50 x 26). Open Ice yes (120 x 60).
  Glass Ice yes (friction 0.002). Carpet **half**: the slow band plays, the "sticky" puck effect never fires (bug 1). Power-Up yes (speed + big_puck pickups).
  Turbo yes (two speed pickup rows, 3 on the ice). Giant Puck yes (radius x3 on pickup; the human took none in 5 matches, the bots 8). Neon yes (l3_neon, fast-ice
  centre, loud barks, an announce per goal). Matinee yes (l1_day). Easy Scoring yes (goal 6 m, shot 24, red skill 0.6: 5-0 for the human). All-Star yes. Rookie yes.
  One Minute yes (3600-frame clock, 2.4 goals). Marathon yes (nine, on the longest clock). Sudden Death yes. Pillar yes. Maze yes (11 boxes). Rocket yes (shot 32,
  puck 40; 1-4 for the human, the hardest cut). Big Net yes (goal 5.5 m). Lockdown yes (goal 2.5 m; still 4 goals a match, the bots find the small net). Pond yes
  (slow, rookie, warm-pond slow circle, quiet). Final **no**: "a sunset championship final" changed only `score_to_win` to 5; no sunset (look tier stays l2_night).
  Arcade **half**: l3_neon only; no retro beyond the neon tier. Rubber Boards yes (0.9). Midnight **no**: the cartridge is Regular with mood words; nothing on the ice
  is spooky or midnight (there is no tier for it). The three look-only misses are Designer limits (one tier enum, no sky or palette control on the ice), not sweep failures.

## The stills (`docs/experiments/img/cuts/`, PIE viewport capture 1275 x 700, camera 24 m behind the blue centre, pitch -17.7, FOV 70, frozen at 4 s of play; Power-Up at 14 s, 24 m from the first pickup)

| Still | Cut | Shows | eyes readability at 24 m (gemini-3.8-flash: puck / carrier / team colours / far net / zone strips, overall) | My read |
|---|---|---|---|---|
| `matinee-cut-24m.png` | Matinee Cut | look tier `l1_day` | 4 / 4 / 1 / 4 / 5, overall 3; first look "yellow-white glare from the boards" | pale ice, bright board tops, black sky: a day tier only by the ice; reads |
| `pillar-cut-24m.png` | Pillar Cut | `l2_night`; 4 pillars | 4 / 4 / 1 / 4 / 3, overall 2; the pillars read as "white cylindrical obstacles" | pillars visible as pale drums at the blue line; night pools on dark ice |
| `neon-cut-24m.png` | Neon Cut | `l3_neon`; fast_ice circle r 6 | 4 / 4 / 1 / 4 / 4, overall 3; "radiant cyan burst in the centre circle" | the fast-ice disc is the loudest thing on the ice; neon boards glow; reads |
| `carpet-cut-24m.png` | Carpet Cut | slow_ice rect 40 x 45 | 4 / 4 / 1 / 4 / **2**, overall 2; "dense cyan grid covering the ice" | the band is visible everywhere, too much so: it hides the lines (bug 4) |
| `take-7-24m.png` | Slow Motion (take-7) | slow circle r 10 + two fast rects, 140 x 70 | 3 / 2 / 1 / 3 / 4, overall 2; "cyan patterned grid" | both zone kinds visible in one frame (grid disc at centre, teal pads on both boards); the far net is 68 m away and tiny |
| `pinball-cut-24m.png` | Pinball Cut | 4 bumpers | 4 / 3 / 1 / 4 / 4, overall 3; "bright magenta curved bumpers" | three magenta bumpers in frame; unmistakable |
| `maze-cut-24m.png` | Maze Cut (fixed) | 11 boxes | 3 / 2 / 1 / 3 / 3, overall 2; "large white maze blocks" | the white boxes read; the centre of the circle is free now; the puck hides between figures |
| `power-up-cut-24m.png` | Power-Up Cut | a pickup on the ice | 1 / 1 / 1 / 4 / 4, overall 2; the pickup listed as "glowing neon cyan ring" | the pickup is a small cyan ring on the red line by the boards; the puck is at the far end (bug 5) |

No cut's zones or pickups were invisible; the one readability failure is the slow-ice material drowning the rink lines on a big zone, and team colours at 24 m
across the board. eyes cost: 3 calls, $0.038.

## Three cuts the sweep says are missing (empty spots in the fun space)

The library's spread: 10 cuts change one number (speed, friction, boards, goal width, shot), 6 change the rink size or shape, 3 pickups, 3 looks, 4 rules of the
match (timer, nine, one, five), 3 difficulty, 1 zones + effect. No cut uses `no_shoot` or `boost_only` zones, `goal_shield` obstacles, a `goal_against` effect
(take-5's `goal_for` makes the leader faster), `mercy_goals`, or `period_count`. Three to add, each a sentence a player would say and the keys it needs:

1. **"no long shots, carry it in" (Carry It In Cut)**: a `no_shoot` box in front of each net, so a goal needs a carry past the backs, and the poke / lift duel
   matters. Keys: `game.rink.zones: [ { id: "slot-west", shape: "rect", x: -36, z: 0, w: 14, d: 20, kind: "no_shoot", strength: 1, visible: true }, { id: "slot-east", ...x: 36 } ]`.
   The brain already refuses to shoot from `no_shoot` (R-0019), so the bots play it; the sweep's `shots_blocked` counter would show the human learning it.
2. **"help whoever is losing" (Comeback Cut)**: every goal against gives the conceding team a speed boost, the opposite of take-5. Keys:
   `game.effects: [ { id: "comeback", kind: "speed", target: "own_team", duration_frames: 420, trigger: "goal_against", stack: "refresh", stats: [ { stat: "skater_speed", multiply: 1.2 } ] } ]`
   plus `game.rules.hooks: [ { id: "comeback-call", when: "on_goal", team: "conceding", then: [ { do: "announce", text: "COMEBACK TIME: THE TRAILING SIDE SKATES FASTER" } ] } ]`.
   Closes the rubber band the sweep shows at pro (3-2 either way, decided by the first two goals).
3. **"put a shield in front of the nets" (Shield Cut)**: `goal_shield` obstacles, the one obstacle kind no cut uses, so scoring means a wrap-around or a bank
   off the shield. Keys: `game.rink.obstacles: [ { kind: "goal_shield", x: -40, z: 0, size: 3 }, { kind: "goal_shield", x: 40, z: 0, size: 3 } ]`, with
   `game.rules: { mode: "timer", timer_frames: 14400, hooks: [ first-to-three ] }` because the sweep says anything that slows scoring at pro needs the clock.

## Note for cut authors: look keys that do nothing on `p3_clay` (A69, B-to-A-0035 point 2)

`look.trim_emissive` has **no effect on `p3_clay`**, by design: B's `M_Trim` has a clay path (a matte red cap) and ignores the dynamic instances' `Emissive` there. A glow on
clay would be `ClayCapGlow` on `MI_Trim_Blue` / `MI_Trim_Orange`, a B-side instance value, not a cut key. On `p3_clay` the glass pane, the glass rail and the stanchions
are not drawn at all (hidden by the rink, A69), and a zone is drawn as an opaque ring (see `docs/experiments/CLAY-RINGS.md`), not as a decal.

## Questions for the Director

- The two new C++ files are in `Game/Source/JamCore/AI/` (A2's folder), additive, no shared file touched: keep as the sweep's commandlet, or fold the wiring
  into `FJamSimAdapter` so `Jam.Bot.*` and `tools/sweep` also run the systems (that would change nothing for regular / large, which have none, but would make the
  season test see take-4's hook and take-5's effect)?
- Bug 1 (zone events into the effect system) is a one-method seam change in A14's and the game mode's step; who takes it?
- The library was hand-edited in three cuts (not regenerated); `base_sha256` is unchanged (hockey.regular untouched); `JamDesignerData.inl` regenerated by the
  embedder, the Jam.Designer tests re-run after the rebuild (status/A37.md).
