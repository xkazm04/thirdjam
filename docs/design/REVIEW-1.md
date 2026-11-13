# REVIEW-1: H26 against the vision, as a first-time player meets it (lane A100, track F0, Fri 13 Nov 18:47-)

The critic's read of the tree H26 was packaged from (main ee14438: contact by inertia, the movement laws, the hockey brain, the ladder of R-0046), against
`docs/VISION.md`. No source was changed. Status with timestamps: `status/A100.md`. The numbers and the two small scripts that made them: `docs/design/review-1/`.

## 0. The verdict in six lines

1. **Nobody keeps the puck.** A carry lasts under a second at every level (0.75 s at pro) and the puck is on no blade 60 % of play. Law 3 ("the carrier wins by
   changing speed and direction") has no room to happen: the stop alone takes half a second. This is the one finding that costs more than all the others together.
2. **The cause is not the bots' skill.** Slowing the pro bots' poke threefold or removing their hunt leaves the human's carry at 0.70 s. A puck changes hands with
   no press at all: on any aimed blade within 0.6 m once the carrier is above 6 m/s, and on any body (`JamSim.cpp:2336-2399`).
3. **The game's main verb punishes the newcomer.** A first-timer's hit from behind is a silent bump or a foul: he sits in the box in 37 of 60 first matches, under a
   banner that says "Hockey with the whistle taken out".
4. **What the sim decides, the screen does not say.** The stumble, the refused press, the bounce off a braced skater and the brake have no presentation of their own;
   a hit that lays a skater down for 60 frames and one for 150 hold the same 3 frames.
5. **Two things are already the game's own and work:** the knockdown, and the team-mates who give the human the puck 5 to 9 times a match.
6. **The first match can be fixed with two cartridge keys today**, each measured alone on take-1 (section 3, items 4 and 2): `shot.deflect false`: the novice's
   first shot 82 -> 37 s, the match 4.2 -> 2.4 minutes; `rules.penalties false`: a foul in 37 -> 0 of 60 matches.

## 1. How I watched (and what that can and cannot see)

- **No window** (track S2 owns the cursor). The player is the playtester (`Game/Source/JamCore/AI/JamPlaytester.cpp`) in the human's slot: `novice` is the
  first-time player, `regular` someone who has played a hockey game, `good` the target of all_star. It does not read the HUD and has no camera
  (`docs/experiments/PLAYTEST.md`: its numbers are a floor for a person who sees and a ceiling for one who cannot find the puck). Where a claim is about the screen,
  the evidence is a source line or another lane's still, named.
- **Runs** (this worktree's own build of ee14438, headless, `-nullrhi`): the grid `tools/playtest/run.mjs`, 3 classes x 3 levels on `hockey.regular`, 40 seeds a
  cell (360 matches); the season's takes 1, 2 (rookie) and 3 (pro) with the novice and the regular class, 60 seeds each (360 matches); seven traced logs
  (`-trace=1`, 12 seeds each, the target class of each level, and four paired arms); two variants of take-1 (60 seeds each). Tables: `review-1/data/grid.md`,
  `ladder12.md`, `ladder3.md`; per-cell read-outs `*.cells.jsonl`, `*.cell.json`; carries `*.carry.json`.
- **A carry** in the traced numbers is the brain's carrier of record held for 20 frames or more (`review-1/humancarry.mjs`). It agrees with the sim's own counters:
  the human's share of play as the carrier 9.8 % traced against 10.6 % (`carry_frames`), and `carries_3s` is the sim's.
- **Sample sizes.** The carry medians rest on about 270 carries of the human a cell (12 traced seeds, an hour of play); win shares from 12 seeds are weak and are
  quoted as counts. The grid's
  target cells read 83 / 50 / 30 %, CURVE-5's 2,000-match cells 85.3 / 48.3 / 26.3: the same ladder.
- **Stills** read: `docs/experiments/img/a83/five-p3-board-battle.png`, `a80/a80-kd-human-f1674-left-strip.png`, `a84/hud-shotgun-fired-f438.png`,
  `a85/a85-sweep-f548-strip.png`, `modes2/directors-choices-720p.png`, `submission/shots/03-faceoff.png`, `05-knockdown.png`. All are older than H26 (the two
  submission shots are H24, three a side).

## 2. The first five minutes by the clock (the novice on the season, 60 seeds a take)

The season starts at take-1 "The Read-Through" (rookie, five a side, first to 2, penalties on), then take-2 "The Big Room" (rookie, the large rink, a three-minute
clock), then take-3 "Rough Cut" (pro, first to 3).

| When | What happens to a first-time player | Number |
|---|---|---|
| 0:02 | He wins the opening draw (the rookie centre jumps early) | first touch at 1.8 s (median) |
| 0:02-1:20 | He chases. The puck is on nobody's blade most of the time; when he gets it he has it for under a second | carrier 11.2 % of play; 21.1 pokes of which 1.7 take the puck |
| in between | His hits land, and in most matches one of them is whistled | 5.0 checks landed, 1.1 fouls; a foul in 37 of 60 matches; 5.6 s in the box |
| 1:22 | His first shot (median) | 81.9 s; 3 of 60 matches without one; 2.7 shots by him, 10.9 by the opponents |
| 1:25 | His first goal (median of the matches that have one) | 84.9 s; 31 of 60 inside two minutes; 49 of 60 at all |
| 4:12 | Take-1 ends; he has won it four times in five | 80 % won; 2.3 goals in 4.2 minutes; knocked down 2.5 times |
| 4:15-5:00 | The card, then the first minute of The Big Room: more ice, the same chase | take-2: first shot at 70.7 s, 6 of 60 matches without one |
| (8:00) | Past the five minutes: take-3 is pro | **novice 22 % (13 of 60); 22 of 60 matches without one shot by him**; 1.4 shots against 19.8 |

The regular class goes through the same door faster: take-1 in 2.0 minutes (95 %), take-2 in 3.3 (100 %), take-3 at minute 5.5 (48 %).

## 3. The ten things, ranked by what each costs or gives

Each: what he feels; the evidence; the law of VISION it serves or breaks; the owning track; the smallest evolution, with the number that proves it.

### 1. "I never have the puck." (cost; the largest)

- **Evidence.** Traced, the target class of each level on `hockey.regular`:

  | | novice at rookie | regular at pro | good at all_star |
  |---|---|---|---|
  | the human's carry, median / 90th percentile | 0.87 s / 3.27 s | **0.75 s** / 2.03 s | 0.67 s / 1.57 s |
  | carries of 3 s or more a match (`carries_3s`, 40 seeds) | 2.5 | **0.8** | 0.1 |
  | his carries that end on an opponent's touch or a hit | 53 % | **63 %** | 64 % |
  | his carries with an opponent inside 2.5 m at some point | 81 % | 85 % | 80 % |
  | the puck (the brain's carrier of record): on his blade / a mate's / an opponent's / nobody's | 10 / 4 / 23 / **63 %** | 10 / 5 / 25 / **60 %** | 10 / 6 / 25 / **60 %** |
  | an opponent's carry, median; ends lost / passed or released | 0.63 s; 24 / 39 % | 0.60 s; 17 / 48 % | 0.65 s; 23 / 48 % |

  The bots' carries are as short but end by their choice; the human's end by loss. The reverse stop takes 0.5 s and stop-cut-go 0.83 s (`MOVEMENT.md` section 6):
  the median carry is over before the move is. CURVE-5 section 3 already read the consequence: teaching the playtester the brake, the cut and the sprint moved the
  win share by 2 to 3 points ("it is not what decides these matches").
- **What it is not** (paired arms, pro, the regular class, seeds 1-12): the bots' poke after 20 frames instead of 6: carry median 0.70 s. No hunt (radius 0): 0.70 s.
- **What it is.** A carried puck leaves with no press. `Game/Source/JamCore/Private/JamSim.cpp:2336-2365`: the deflection written for shots (C-0005: within
  `DeflectRangeM` 0.6 m of an aimed blade, puck faster than `DeflectMinSpeed` 6 m/s) has no test for a carried puck, so a carrier above 6 m/s loses the puck to any
  opposing blade that is aimed and near; and line 2399: any body the puck touches becomes its last toucher. The comment five lines below (line 2404, R-0021,
  the operator) says a carried puck "leaves only on a shot, a pass, a poke, a lift or a check". The sweep's and the spin's shield (`IsPokeShielded`, line 1337) refuse
  pokes and lifts
  only. Rookie bots never poke and never lift, and still end half of the novice's carries.
- **Sized** with the cartridge key that exists (`shot.deflect false`, paired seeds): pro: carries lost to a touch 55 -> 43 %, `carries_3s` 0.8 -> 2.1, the human's
  shots 5.9 -> 9.3, the median unchanged (0.77 s), wins 5 -> 8 of 12. Rookie: median 0.87 -> 1.00 s, 90th percentile 3.27 -> 4.43 s, lost 47 -> 33 %, wins 8 -> 11
  of 12. So the passive blade is a real share; the body touch and the crowd on every loose puck are the rest. As a global rule it moves a target cell far more than
  4 points.
- **Law.** Breaks law 3 (no room for speed change; "sweeps and dekes shield the puck from sticks" is untrue against a passive stick) and R-0021's own sentence.
  One question is the Director's: law 3 says the shields work "never from bodies", R-0021 says the puck leaves "only on a shot, a pass, a poke, a lift or a
  check". Which of the two rules a body that merely touches a carried puck is not written anywhere.
- **Owner.** F2 (the stick functions). Its first evolution names "puck protection with the body between": of its three parts that is the one that matters; the
  burst's window and the brake's grip measure fine.
- **Smallest evolution.** Inside the sweep's, the spin's and the deke's window the passive blade is refused too (one test in the deflect branch; the window's
  length is unchanged; a body still takes the puck, as law 3 says). Key `game.stick.shield_passive` (neutral false). **Proof:** `Jam.Core.SweepShieldsLift`-style
  test with an aimed, unpressed blade on the puck's path: kept 10 of 10 inside the window (`PLAN-RECONCILED.md`: the sweep kept the puck 4 times in 10 and the
  spin 2, before R-0044 restated their bars to the shield alone); the good class at all_star (it sweeps as a matter of course): carry median 0.67 -> 1.0 s or more,
  `carries_3s` 0.1 -> 1 or more; the three cells within 4 points.
- **The evolution after it** (the rule itself, for the curve to re-set): a carried puck is not deflected off a blade that did not press; key
  `game.stick.passive_take` (neutral true). **Proof:** regular at pro: carry median 1.5 s or more, `carries_3s` 3 or more, the puck on a blade 55 % of play or more.

### 2. "I hit him and got sent off. I hit him and nothing happened." (cost)

- **Evidence.** Take-1, the novice: 5.0 checks landed and 1.1 fouls a match (0.9 from behind, 0.3 on a skater without the puck), **a foul in 37 of 60 matches**,
  5.6 s in the box. Take-2: 1.5 fouls, a foul in 46 of 60, 7.6 s. `hockey.regular` at rookie: 1.9 fouls, 30 of 40 matches, 9.3 s. The novice chases, so he arrives
  from behind: by CONTACT's table a press from behind at the same speed is a bump (ratio 0.35: no event, the short flinch at most once in 20 frames, no word); with
  closing speed it lands and is `foul_from_behind`. A press while he carries is refused: `bCheckRefused` is read by no file outside the sim, its tests and the
  acceptance drill. Every take and `hockey.regular` have `rules.penalties true` (`ladder/take-*.cartridge.json`, `hockey.regular.cartridge.json:150`).
  The banner line and the video's end card: "Hockey with the whistle taken out" (`submission/tagline.md`, `submission/video/script-v2.md` shot 11).
  Take-3's twist text: "Drop the manners. Hit them", with the whistle on.
- **Law.** Law 1 is served in the sim and unspoken on the screen; R-0041 point 4 (the brand) is contradicted by the first match.
- **Owner.** F1 (the HUD's contact texts, already its first evolution for "not with the puck") for the words; the Director for two cartridge keys.
- **Smallest evolution.** (a) F1: one line within 6 frames of every press that did not land, in the voice: the bump ("No speed on it. Cut across him."), the
  refusal ("Not with the puck."), the foul ("From behind: the box."; `HUDWidget.cpp` has no text for a foul or the box today, the whistle and the booth carry
  it). **Proof:** three acceptance rows (press from behind at equal speed; press while carrying; press from behind with 4 m/s closing) each show their line: 3 of 3.
  (b) Director: `rules.penalties false` in take-1 and take-2. **Measured here** (take-1, the novice, 60 seeds, the variant in `tools/playtest/out/a100/`): a foul
  in 37 -> 0 of 60 matches, 5.6 -> 0 s in the box, checks landed 5.0 -> 5.2, won 80 -> 90 %, an own goal at all 49 -> 55 of 60; the pace does not move (first shot
  81.9 -> 84.4 s, the match 4.2 -> 4.3 min: that is item 4's key, not this one). **Proof for the landing:** the same run on the edited takes: no foul, no box second;
  `Jam.Bot.SeasonTakes` green.

### 3. "That hit." (gain; the best two seconds in the build)

- **Evidence.** The acceptance run on the final tree (`CURVE-5.md` section 9): a check at 12.4 m/s of closing lays the carrier down for 134 frames (ratio 2.64), the
  human checked while standing is down 86 (1.57), the cross-check to the chest 74 (1.33). The strip `img/a80/a80-kd-human-f1674-left-strip.png`: the fall, the
  slide, the get-up; the camera held the lying human in the middle third of the picture on 445 of 445 frames (`TACKLING-3.md`). The booth winces, never mocks
  (`submission/shots/05-knockdown.png`: "Down for a second, up for the win"). A novice lands 5.0 checks in his first match and takes 2.5 knockdowns; at pro the regular
  class is knocked down 8.4 times a match (1.8 a
  minute) and lands 4.9. The verb a newcomer finds first, works.
- **Law.** Serves law 1 and the one sentence.
- **Owner.** F1.
- **Smallest evolution that amplifies it.** "The attacker pays" is the part nobody sees: 3 recoils in six matches (`CONTACT.md` section 6), and no line or take of
  its own. A staged, repeatable bounce (`Jam.Core.Contact.AttackerBounces`: sprinting in at 5 m/s, the attacker leaves at 1.53): an opponent sprints into the side of the
  standing, braced human; the human is not laid down; the attacker staggers; the line "Bounced off."
  **Proof:** a new acceptance row `braced` (the mirror of row 8 `checked`, the human without the puck and standing): the human on his feet, `bRecoilThisFrame` on
  the attacker, the line shown. Then the board check (F1's own third part).

### 4. "A minute and twenty before my first shot; four minutes to find the net twice." (cost)

- **Evidence.** Take-1, the novice: first shot at 81.9 s (median), first own goal at 84.9 s, 31 of 60 with a goal inside two minutes, 11 of 60 with none at all,
  2.7 shots by him against 10.9 by the opponents, the match 4.2 minutes for two goals. The twist text asks him to "find the net twice".
- **Law.** None broken by a rule; it is item 1 seen from the first match.
- **Owner.** The Director (the take's cartridge is content; no sim change, no hash of another cut moves).
- **Smallest evolution, measured here.** `"shot": { "deflect": false }` in take-1 (the key exists). The novice on that variant, 60 seeds:

  | take-1, the novice | as shipped | `shot.deflect false` |
  |---|---|---|
  | first shot, median | 81.9 s | **36.8 s** |
  | first own goal, median | 84.9 s | **45.7 s** |
  | a goal by him inside two minutes | 31 of 60 | **45 of 60** |
  | a goal by him at all | 49 of 60 | 58 of 60 |
  | the match | 4.2 min | **2.4 min** |
  | won | 80 % | 90 % |
  | carrier share of play | 11.2 % | 17.2 % |
  | a foul / seconds in the box | 37 of 60 / 5.6 s | 23 of 60 / 3.0 s |

  **Proof for the landing:** the same run on the edited take: first shot under 45 s, a goal inside two minutes in 40 of 60 or more, `Jam.Bot.SeasonTakes` green
  (the AI still needs its shot: 5.8 opponent shots a match on the variant). It costs the human his own passive block of a shot in that take; nobody will miss it in
  a read-through.

### 5. "My poke never works." (cost)

- **Evidence.** Pokes a match, and those that take the puck: the novice in take-1 21.1 / 1.7 (8 %); the novice at rookie 28.7 / 2.3 (8 %); the regular class at pro
  32.6 / 3.3 (10 %); the good class at all_star 31.6 / 4.3 (14 %). An armed miss costs `stick.stumble_frames` 20 frames without control and the booth's
  "Mistimed, and a stumble." (the caption in `img/a83/five-p3-board-battle.png`): the line a newcomer reads most. Meanwhile the opponents need no poke (item 1).
  Why nine in ten miss (the shield, the reach of 0.35 m, the 8-frame window) is not split by any read-out; this is the playtester's hand, and a person's cursor may
  do better or worse.
- **Law.** Law 3, "the mouse is the stick".
- **Owner.** F2.
- **Smallest evolution.** A near miss is not a stumble: the window that ends with the blade within 0.8 m of the puck costs the cooldown only; the stumble stays
  for a swing at nothing. Key `game.stick.near_miss_m` (neutral 0). And the miss's cause in the playtest line (`poke_miss_shield`, `_reach`, `_late`).
  **Proof:** missed pokes that cost him the stumble, a match at pro: from up to 29 (every miss with a carrier near) to under 12; the clean share reported with
  its split; the three cells within 4 points.

### 6. "My team gives me the puck." (gain)

- **Evidence.** Passes received by the human a match: 4.8 (the novice, take-1), 7.0 (rookie), 7.9 (pro), 9.1 (all_star); faceoffs without a human touch 1.5 %
  (`CURVE-5.md`); he is within 6 m of the puck 60 % of play. `BRAIN.md`: 3.7 before the brain, 9.3 after. It is the best thing the brain does for the player, and
  it is half wasted: after a mate's pass he still has the puck a second later in 54 % (rookie) and 36 % (pro) of the cases, two seconds later in 32 % and 11 %;
  one-timers 0.0 to 0.5 a match. (The count includes a puck his blade took off a mate's own stick, 0.4 to 0.8 a match: section 6.)
- **Law.** Serves law 2 (support the puck).
- **Owner.** Q2 (the brain: the mates' pass), then F2.
- **Smallest evolution that amplifies it.** The mates give it to the human when the space read says he is open (3 m free, the read exists) or lead him into the
  free side, not onto a marked blade. **Proof:** the puck still his two seconds after a mate's pass: 11 -> 30 % or more at pro (`humancarry.mjs`,
  `human_after_mate_pass.held_2s_pct`); one-timers 0.2 -> 1 a match; passes received not under 6.

### 7. "A stumble looks like a bump, and a big hit like a small one." (cost)

- **Evidence** (source). The hit-stop is a constant 3 frames, on a knockdown that involves the human only (`JamPawns.cpp:106`, `HokejGameMode.cpp:1221`); the
  camera kick is `max(0.5, Strength)` (`CameraFollow.cpp:410`) and `Strength` is closing speed over `skater.speed` (`JamSim.cpp:1790`), not the ratio that sets
  the frames down (60 to 150): the fall's length varies 2.5 times, its hit hardly. The stumble (16 frames without control, half the speed gone, the puck loose
  from 0.7) plays the bump's flinch (12 frames, 14 degrees), has no hit-stop, no kick, no word; the attacker's recoil the same. At pro A92 counted 20 stumbles of
  the human against 15 knockdowns in three matches: the commonest loss of control is the one with the least on screen.
- **Law.** Law 1: "three outcomes"; "down for longer the bigger the ratio".
- **Owner.** F1 (its first evolution; `CONTACT.md` section 8 has the ask for B's longer take).
- **Smallest evolution.** The hold and the kick read the frames the sim gave: hit-stop 2 at 60 frames to 6 at 150, the kick scaled the same; a stumble holds
  1 frame, kicks at a third, and plays the flinch at full weight for `stagger_frames`. Presentation only: the hash does not move. **Proof:** in the acceptance run
  the rows `check` (ratio 2.6) and `crosscheck` (1.3) log hit-stops that differ by 2 frames or more and kicks by 1.5 times or more; a new row `stumble` (the side
  cross-check from 4 m/s, ratio 0.78) logs a hold of 1 and its line.

### 8. "Nobody told me I can brake, and when I do nothing shows." (cost)

- **Evidence.** The hint bar on the ice lists ten verbs and no brake (`HUDWidget.cpp:349`; still `submission/shots/03-faceoff.png`); the brake is on the pause
  and settings pages only. `bBraking` and `bReverseBraking` are read by no file outside the sim, its tests and the acceptance drill; the figure's spray fires on a
  lean of 25 degrees or more only (`SkeletalSkaterVisual.cpp:670`); no sound is bound. VISION law 3 says "with the spray and the sound". The novice reverses 31
  times in take-1 without knowing it is a verb.
- **A trap in the obvious fix.** The bots reverse-brake 670 to 2,330 times a match in every cell (pro: 2,146, the human 73): about one a second for each bot. A
  spray and a scrape bound to `bReverseBraking` today would fire seven times a second across the ice. Item 9 comes first.
- **Law.** Law 3.
- **Owner.** F2 (the movement functions and their drills) with an ask to B (the spray's and the sound's call on `bReverseBraking` above 4 m/s); the hint bar's
  line is the HUD's.
- **Smallest evolution.** The hint bar says it: `BRAKE  S AGAINST YOUR MOTION` in place of `PAUSE  ESC` (the pause has its own tag in the corner of the same
  screen). **Proof:** a still of the first faceoff. Then, after item 9: the spray and the scrape on the human's reverse brake; **proof:** the acceptance row `brake`
  logs a spray request inside 3 frames of the first brake frame.

### 9. "The bots do not play the game I am told to play." (cost; the model is missing)

- **Evidence.** `bot_carry_sprint_frames` is 0 in all nine cells: no bot ever sprints with the puck, at any level (the sprint shares of CURVE-5 are for chases), so
  law 4's gamble is shown to the player by nobody. The bots' reverse brakes (2,146 a match at pro) are about-turns with perfect inputs, not a stop chosen
  against a defender (`MOVEMENT.md` section 8, `CURVE-5.md` section 11 point 5); the brain's one move with the puck is the 55-degree turn away from a closed lane. At all_star the carry-or-pass numbers (0.3 s, 12 m) make the top level carry through the forecheck
  (`CURVE-5.md` section 11 point 2): the level that should show the principles best shows them least. An opponent's carry is 0.6 s at every level. And the shape the brain won may have slipped with the curve's levers: `shape.mjs` on my pro trace reads the pile
  at 16.6 % of frames and 8.0 bot passes a minute, where `BRAIN.md` had 12.0 % and 11.7 (17.0 % and 5.65 before the brain); 12 seeds against its 60, so a
  read to repeat, not a verdict.
- **Law.** Laws 2, 3 and 4.
- **Owner.** Q2 (the brain).
- **Smallest evolution.** The brain brakes on purpose: a reversal it does not mean is carved into a turn, the rule the playtester got in A95 (`BrakeIntent`).
  **Proof:** `bot_rev_brakes` at pro 2,146 -> under 500 a match; the three cells within 4 points. The next: the bot carrier takes the playtester's two rules
  (sprint on an open lane, the 75-degree cut on a closing defender) by level 0 / 50 / 100 %; **proof:** `bot_carry_sprint_frames` 120 or more a match at pro, an
  opponent's carry median 0.6 -> 1.0 s.

### 10. "The hall is empty." (cost; the first image)

- **Evidence.** `submission/shots/03-faceoff.png` and `05-knockdown.png` (the night rink: a glass rail, then a flat dark field), `img/a83/five-p3-board-battle.png`
  (the day rink: a black band between the boards and the wall). No bench, no box, no crowd, in a game that sends him to "the box" in most first matches.
- **Law.** The one sentence ("rough clay hockey") and R-0041 point 4.
- **Owner.** F3; the crowd's figures are B's.
- **Smallest evolution.** F3's first (benches and penalty boxes with doors) is the right one. **Proof:** the faceoff still from the follow camera with a bench or a
  box in frame on both rinks, and Q3's frame time at five a side within 0.3 ms. The next: something behind the glass on the camera's far side (a clay crowd row or
  stands in B's materials): in the two stills above, the flat dark share of the picture above the boards under 30 %.

### Just outside the five minutes: take-3 is a wall (the Director's)

The novice wins take-1 80 % and take-2 95 %, then meets pro: **22 % (13 of 60), no shot by him in 22 of 60 matches**. At 5.2 minutes a try he needs about 24
minutes to see take-4; a judge will not. The ladder of R-0046 is not the fault (its cells are right); the season's order is. Smallest evolution: take-3 at rookie
with its own roughness written in the cartridge (`rules.penalties false`, the twist text already says so); the first pro take comes later. **Proof:** the novice at take-3 60 %
or more, the regular class 85 % or more (`tools/playtest/run.mjs --rinks ladder/take-3`).

### The scorecard (for REVIEW-2: the same commands, the same seeds)

| # | Number | Today | Bar | Read with |
|---|---|---|---|---|
| 1 | the human's carry at pro, median; `carries_3s` | 0.75 s; 0.8 | 1.5 s; 3 | `humancarry.mjs` on a traced log; the grid |
| 1 | the puck on a blade, share of play (pro) | 40 % | 55 % | `humancarry.mjs` `puck_pct` |
| 2 | take-1, the novice: matches with a foul; box seconds | 37 of 60; 5.6 s | 0; 0 (measured on the variant) | `cell.mjs` on the take's lines |
| 2, 7 | acceptance rows for a press that did not land, a stumble, the braced bounce | none exist | 5 rows green | `tools/qa/accept` |
| 4 | take-1, the novice: first shot; a goal inside two minutes | 81.9 s; 31 of 60 | 45 s; 40 of 60 | `cell.mjs` |
| 5 | pokes that take the puck (regular at pro); misses that stumble him, a match | 10 %; up to 29 | reported with its split; under 12 | the grid |
| 6 | his after a mate's pass, two seconds later (pro) | 11 % | 30 % | `humancarry.mjs` |
| 8, 9 | `bot_rev_brakes` a match at pro; `bot_carry_sprint_frames` | 2,146; 0 | under 500; 120 or more | the grid |
| - | the novice at take-3 | 22 % | 60 % | `run.mjs --rinks ladder/take-3 --levels pro` |

## 4. What makes this game its own (one page)

**The pitch, clause by clause, against the build.** VISION's sentence: "Rough clay hockey where a hit obeys momentum, the puck carrier wins by changing speed and
direction with a stick the mouse controls, and five skaters a side play hockey: they see space, support the puck and pick their moment."

| Clause | In the sim | On the screen, in a match | Verdict |
|---|---|---|---|
| a hit obeys momentum | yes: 0.35 (a bump) to 4.61 (down 150 frames) for the same press; the attacker pays | the knockdown reads; the stumble, the bump, the bounce and the size of a hit do not | **true, half shown** (items 3, 7) |
| the carrier wins by changing speed and direction | the verbs exist: still in 33 frames, the other way in 0.83 s | a carry is 0.75 s; the verbs move the win share by 2 to 3 points | **not yet true** (item 1) |
| a stick the mouse controls | the acceptance run 19 of 19 through the cursor | the stick takes the puck in one poke of ten; a passive stick takes it for free | **true for the shot and the pass, not for the duel** (items 1, 5) |
| five a side play hockey | `BRAIN.md` at pro: bot passes 11.7 a minute, the pile 12.0 % of frames. On the final ladder (`tools/playtest/shape.mjs` on my 12 traced seeds): **8.0 a minute and 16.6 %** at pro; 7.6 and 12.7 % at rookie (it had 9.5 and 11.8 %); 6.7 and 13.6 % at all_star (12.4 and 12.7 %) | the puck is loose 60 % of play; the top level carries through pressure | **true, and thinner than when the brain landed** (items 6, 9) |
| rough | 8.4 knockdowns of the human a match at pro, 1.8 a minute | the whistle is on in the first match and every take | **true on the ice, contradicted by the rules** (item 2) |

**The one line.** The page's line today is "Hockey with the whistle taken out: clay skaters, a mouse for a stick, and a Director who rewrites the rules mid-game."
The first clause is false of the regular cut and of all eight takes. Two honest ways out, the operator's choice: take the whistle out of the regular cut and the
takes (one key each; the fouls then need no box and item 2 half dissolves), or lead with the line the build already keeps in every match:
**"Clay skaters. Real hits: the speed you bring is the hit you get."** (tagline 2 of `submission/tagline.md`, with the law added). Also in
`submission/page.md`: "Difficulty is aggression, not speed" (all_star skates at 1.4, section 6) and "Cross-check a skater who is not looking" (a lift from behind
never shoves, and only the carrier is a target: `CONTACT.md` section 1).

**Three moments the video must show** (one per thing no other hockey game says):

1. **The same press, twice.** Side by side, the same button: skating with him at his speed, a nudge and both skate on (ratio 0.35 from behind, 0.28 from the
   side); head-on at full speed, down for two and a half seconds (4.61, 150 frames) with the hold and the kick. On screen: SAME WAY, A NUDGE. HEAD-ON, FLATTENED.
   The script has it as shots 5 and 6; shot 5 as written uses drill 5, a check from the side at 3 to 6 m/s, which is a bump only while the two move the same
   way: with 3.5 m/s of closing it is a stumble with the puck loose (0.90). Stage `Jam.Core.Contact.SameDirection`'s scene (same direction, same speed).
2. **Stop. He is gone.** The carrier reverses, is still in half a second, the sprinting defender (12.65 m/s, his stability at 0.7) flies past, the first step
   bursts the other way, the shot. This is law 3, and the script's short version ("keep 1, 3, 5, 6, 8, 11") drops it: keep 7 and drop 3. It needs F2's drill 21
   and, to read at all, the spray (item 8).
3. **One rule rewritten, and the lane goes down like pins.** The Director's Call mid-match, then F: the shotgun at 26.8 m/s through three bodies
   (`ABILITIES.md`: 3 of 3 down, the puck through all three). It carries the jam's theme and the brand in four seconds; `img/a84/hud-shotgun-fired-f438.png`.

The spare, if a slot opens: the bounce (an opponent sprints into the braced, standing human at 5 m/s and staggers off at 1.5; item 3): the clay joke that is
also the law.

## 5. The queues, kept honest (lines for `docs/EVOLUTION.md`)

What each track's stated first evolution looks like from the player's side, then the next three, ranked, each with its proving number.

**F1 Contact feel.** The first evolution is right; inside it, the words come first (cheapest, and item 2 is larger than item 7), the board check last.
1. The words for a press that did not land (bump, refusal, bounce), within 6 frames, in the voice. Proof: three acceptance rows green; in a three-minute drill log the count of lines equals the count of bumps, refusals and recoils.
2. The hit's size on screen: hit-stop 2 to 6 and the kick by the frames down; the stumble's own hold and weight. Proof: `check` against `crosscheck` differ by 2 hold frames and 1.5 times the kick; the row `stumble`; the hash unchanged.
3. The braced bounce as a row and a line, then the board check as the second body. Proof: row `braced` green; `Jam.Core.Contact.Boards`: the same side hit that stumbles on open ice puts a skater within 1 m of the boards down; the three cells within 4 points.

**F2 Carrier skill.** Drill 21 is worth having for the video; the tuning it will show is not the burst or the brake (they measure fine) but that the puck does not survive the approach.
1. The shield also refuses the passive blade inside the sweep's, the spin's and the deke's window (`stick.shield_passive`, neutral false). Proof: an aimed, unpressed blade on the path takes the puck 0 of 10 times inside the window; good at all_star carry median 0.67 -> 1.0 s; cells within 4 points.
2. Drill 21 with the body between: the stop-and-go past a pro defender. Proof: the defender overshoots by 2 m or more in 8 of 10; the acceptance row; the video's shot 7 staged from it.
3. The near miss is not a stumble (`stick.near_miss_m`, neutral 0), with the miss's cause in the playtest line. Proof: misses that stumble the human at pro, up to 29 -> under 12 a match; cells within 4 points.
   Held for the curve: the passive take as a rule (`stick.passive_take`); pro carry median 1.5 s, `carries_3s` 3.

**F3 Arena.** Right first evolution: the box is where the rules send him.
1. Benches and penalty boxes with doors. Proof: both in frame in the faceoff still on both rinks; frame time within 0.3 ms at five a side.
2. The far side behind the glass (stands or a clay crowd row in B's materials). Proof: flat dark share above the boards under 30 % in the two reference framings.
3. The box used: the fouled skater is seen in it for his `box_frames`. Proof: a still by name at a foul; no skater mesh through the boards in a 3-minute strip.

**Q2 Review and repair.** Add two reads to the list of A84, A94, A90, A81: a skater takes the puck off its own team's carrier with no pass (`JamSim.cpp:2345`, `:2399`; once a match off the human, 8 to 9 times a match among the opponents), and the two mode cards' "checks from N m/s".
1. The brain brakes on purpose (the playtester's `BrakeIntent` in the brain). Proof: `bot_rev_brakes` at pro 2,146 -> under 500; cells within 4 points.
2. The mates give the human the puck where he has room (the space read's "open", or a lead to his free side). Proof: the puck still his two seconds after a mate's pass 11 -> 30 % at pro; passes received not under 6; one-timers 0.2 -> 1 a match.
3. The bot carrier plays laws 3 and 4 (sprint on an open lane, the cut on a closing defender, by level). Proof: `bot_carry_sprint_frames` 120 or more a match at pro; opponent carry median 0.6 -> 1.0 s.

**S1 Soak.** First, `tools/playtest/shape.mjs` at 60 seeds a level on the final ladder: did the pile and the passes hold after the curve pass (pro read 16.6 % and 8.0 a minute on 12 seeds; the brain's bars were 12.0 % and 11.7)? Then carry the scorecard: add to the playtest line the human's carry median, stumbles of the human, the poke-miss split, so every evolution's soak prints section 3's table. Next: the season in order with retries (the share of novices past take-3 in ten minutes of play: today about one in five tries); the modes' down-time share under the model (Cage read 18 % in A92).

**S2 Acceptance matrix.** Five rows the matrix lacks: `bump` (same way, same speed), `refused` (a press while carrying), `stumble`, `braced`, and `brake` with its spray. Each fails today on the screen's side and passes in the sim: they are F1's and F2's proofs.

**Q1, Q3, S3.** Nothing from the player's side this round. Q3: the words and holds of F1 add HUD work per frame; measure after.

**F0 (this track), next three.**
1. REVIEW-2: the scorecard re-read on the main that carries F1's, F2's and Q2's first evolutions, same seeds; what moved, what did not, the next ranking. Proof: the table of section 3 with a second column.
2. The modes against the vision: each of the twelve cards' facts and character number re-read on H26 (Cage, Bruisers, Fog of War flagged by A92; two cards' "checks from"). Proof: twelve rows, true or the corrected line.
3. The video's three moments on stills from the cursor's owner, frame by frame: does each read in two seconds with the sound off. Proof: three strips named, a yes or the missing piece for each.

## 6. Findings outside `docs/design` (no edit made)

| Where | What |
|---|---|
| `Game/Source/JamCore/Private/JamSim.cpp:2336-2365`, `:2399` | A carried puck changes hands with no press (the shot's deflection has no carried test; any body touch sets `LastTouch`), against the comment at `:2404` (R-0021). Item 1. |
| `Game/Source/JamCore/Private/JamSim.cpp:2345` (and `:2399`) | The reception branch (`bOwn`) has no in-flight test either, and a body's touch has no team test: a skater takes the puck off its own team's carrier and the blade branch counts a pass received. Measured (`review-1/handover.mjs`: the carrier of record changes inside one team with no flight, the puck moved under 1.5 m): a mate takes it off the human 1.0 to 1.2 times a match, the human off a mate 0.4 to 0.8, the opponents off each other 8 to 9 times a match. |
| `contracts/events.md:189` against `JamSim.cpp:1790` | The contract says `check` carries "the ratio of impulse to stability, clamped to 1"; the code sends closing speed over `skater.speed`. One of the two is to be corrected (the Director's text or F1's line). |
| `Game/Source/JamGame/Private/HUDWidget.cpp:349` | The on-ice hint bar has no brake. Item 8. |
| `Game/Source/JamGame/Private/SkeletalSkaterVisual.cpp:670` | The blade spray reads the lean only; nothing reads `bReverseBraking`. Ask to B: a spray and a scrape on it (after Q2's first). |
| `Game/Source/JamCore/Private/JamPawns.cpp:106`, `Game/Source/JamGame/Private/CameraFollow.cpp:410` | Hit-stop constant 3; the kick floored at half strength. Item 7. |
| `Game/Content/Cartridges/modes/hockey.mode.fog-of-war.cartridge.json:236`, `hockey.mode.glass-house.cartridge.json:242` | Card facts "checks from 1 m/s", "checks from 3 m/s": under the model `check_speed_min` scales a hit (x5, x1.7), it is no speed a check lands from (VOICE rule 9). |
| `Game/Content/Cartridges/ladder/take-1..8.cartridge.json` (`penalties`), `submission/tagline.md`, `submission/video/script-v2.md` shot 11 | The whistle is on under the banner that takes it out. Item 2, section 4. |
| `submission/page.md` | "Difficulty is aggression, not speed"; "Cross-check a skater who is not looking". Section 4. |
| `submission/video/script-v2.md` shots 5 and 7 | Shot 5's drill stages a stumble, not a bump; the cut order drops the stop-and-go before the faceoff. Section 4. |
| `Game/Source/JamCore/AI/DifficultyProfile.cpp` (all_star, speed 1.4) | The speed scale is every bot's: at all_star the human's own mates skate 15.5 m/s (90th percentile over half a second) against his 11.3; the opponents 15.9. R-0047 accepted the level; the mates' share of it may not have been seen. |
| `submission/shots/03-faceoff.png` | An opponent's bark at the first faceoff, "Nice stance. Shame it's the only part that moves right.", jokes about the player's skill (VOICE rule 3). For whoever owns the bark bank. |
| the machine | My headless commandlets started one at a time: each sat after the `TurnkeySupport` module load with about 3 s of CPU for 3 to 10 minutes (ten queued at 18:59), then played 60 matches in 7 s. The 360-match grid took 1,156 s, nearly all of it waiting. What I saw while one waited: another worktree's editor start running `UnrealBuildTool -Mode=ValidatePlatforms -OutputSDKs -AllPlatforms`; every editor or commandlet start seems to run it, UnrealBuildTool runs one at a time, and a lane's build holds the same turn. 37 commandlets were up machine-wide. For S1 and every measuring lane: few processes with many seeds each, not many short ones. |

## 7. What this review could not see, and the stills to ask for by name

It saw no frame of H26. Before REVIEW-2 the cursor's owner can settle four things in one Development run (names for the files):
`h26-stumble-human-strip.png` (a pro bot's cross-check stumbles the carrying human: 12 frames from the hit), `h26-bump-behind-strip.png` (the human presses
check from behind at equal speed), `h26-brake-strip.png` (W for two seconds, then S: 40 frames), `h26-take1-faceoff.png` (the first faceoff of take-1 at 1080p,
five a side, the hint bar readable). Not judged at all: sound, the menu's flow on H26, the gamepad, the large rink beyond take-2's numbers, joined play.

## 8. Reproduce

```
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks regular --seeds 1-40 --jobs 6 --out tools/playtest/out/a100/grid
node tools/playtest/run.mjs --players novice,regular --levels rookie --rinks ladder/take-1,ladder/take-2 --seeds 1-60 --jobs 4 --out tools/playtest/out/a100/ladder12
node tools/playtest/run.mjs --players novice,regular --levels pro --rinks ladder/take-3 --seeds 1-60 --jobs 2 --out tools/playtest/out/a100/ladder3
UnrealEditor-Cmd <uproject> -run=JamTournament -sim=jam -cartridge=hockey.regular -difficulty=pro -player=regular -seeds=1-12 -maxframes=36000 -trace=1 -nullrhi ... > raw/pro-regular.log
   (arms: -aitune=opp.poke_frames=20 | -aitune=opp.hunt_radius_m=0 | -cartridge=<hockey.regular with game.shot.deflect false>)
node docs/design/review-1/humancarry.mjs <traced log>      # carries, who has the puck, speeds, after a mate's pass
node docs/design/review-1/cell.mjs <.jsonl or log>          # one line of first-five-minutes numbers per cell
```
