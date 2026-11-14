# REVIEW-2: the game the operator sits down to at 10:00, against the vision and against REVIEW-1 (lane A148, track F0, Sat 14 Nov 01:24-)

The critic's read of the tree package H27 is cut from (main `2a42d88`: the kept puck, the brain's second evolution, the ladder re-set, the whistle out, the first
look clay), against `docs/VISION.md` and against `docs/design/REVIEW-1.md` on the same seeds. No source, cartridge or test was changed. Status with timestamps:
`status/A148.md`. Scripts and tables: `docs/design/review-2/` (bulk logs in `review-2/raw/`, which does not travel).

**What was run** (this worktree's own build, headless, `-nullrhi -Multiprocess`): 7,312 matches of the playtester (`novice` = a first-time player, `regular` =
someone who has played a hockey game, `good` = the target of all_star) and 45 automation tests with 15 extra drill rows. REVIEW-1's three commands again (the
3 x 3 grid on `hockey.regular`, 40 seeds a cell; the takes, 60 seeds a cell, now all eight at their own level and three classes; the three traced cells, 12
seeds each); the large rink; the twelve modes; six 200-seed cells; and 25 measured variants (a cartridge key or a profile tune, none shipped). **No window**:
the cursor is another lane's. Where a claim is about the screen the evidence is a source line. A number not measured here names the document it was read from.

## 0. Three things for the Director before the operator sits down (each is a sentence in `docs/PLAY-ME.md` or one line of code)

| | What he will meet | Measured | The free fix |
|---|---|---|---|
| **A** | He sets ROOKIE or ALL-STAR in SETTINGS and plays the regular cut: **it is pro every time.** The stored level is read by no game code | no file of `JamGame` or `JamNet` outside `SettingsWidget.{h,cpp}` and two tests names "Difficulty"; `HokejGameMode.cpp:402` and `:244` call `Brain->ApplyCartridge(*Cartridge)` with no level. The same on the main checkout at 01:41 (`:472`, `:314`, read, not built), where PLAY NOW starts `hockey.regular`. A newcomer there: **18 %** (36 of 200), no shot by him in 44 of 200; at rookie 86 % | The sheet's steps 1 and 6 and question 2 cannot be done. Until the level is wired (item 1): **rookie is the season's takes 1 to 3, Arsenal, Bruisers, Pond; pro is the regular cut and takes 4 to 7; all_star is take 8 and Sudden Death** |
| **B** | He presses S "about two stick lengths away" at pro and **is knocked down** | the stop begun at 3 m passes 0 of 200 (down 199), 4 m 6 %, 5 m 7 %, 6 m 7.5 %, 7 m 66 %, **8 m 100 %**, 9.5 m 97 %, 11 m 83.5 % | The sheet says **8 m: the distance from the centre to the winger at a faceoff (7.5 m), five stick lengths**. Sooner is safe, later is a fall |
| **C** | Shotgun, which the sheet sends him to, **whistles him**; big ice is drier than the sheet says | a foul in 15 of 40 Shotgun matches (the regular class); on `hockey.arena.large` at pro a 30 s stretch without the puck in **26 of 40** matches (the sheet: "four in ten") | The sheet says: Glass House, Shotgun, Arsenal, Fog of War, Outnumbered, Sniper and Sudden Death still have the whistle; and "about six in ten" |

## 1. The ten things, ranked by what a first-time player and a judge feel in five minutes

Each: what he feels; the number; the law; the owner; the smallest change before Sunday, with the number that proves it. "Before" is REVIEW-1 (Friday 19:30).

### 1. "I chose ROOKIE. It plays the same." (cost; the first press)
- **Number.** The level on the settings page changes a string in `GameUserSettings.ini` (`SettingsWidget.cpp:44`, the button at `:305-310`) and no match. Every
  cut plays its cartridge's own level; `hockey.regular` says `pro`. The novice on it: **18 %** at pro (200 seeds; 8 % on the 40-seed grid), 0.6 goals by him a
  match, no shot at all in 22 % of matches; **86 %** at rookie. The three levels do differ when the brain is given them: 86 / 51 / 23 % for their target
  classes (200 seeds each; targets 85 / 48 / 26).
- **How it was found, and how to see it in two minutes.** By reading, not by a run (this lane has no window): three searches find no reader of the stored
  level, the menu opens the rink with `cut=<name>` and nothing else (`MenuFlow.cpp:30`, read at `HokejGameMode.cpp:190`), and the game mode passes the brain no
  level. Whoever holds the cursor can confirm it: set ALL-STAR, play the regular cut, watch the opponents' legs. At all_star they skate at 16 m/s (90th
  percentile 16.1 in my traces), at pro at 12.4; the human at 11.2.
- **Law.** None; it hides the ladder R-0046 to R-0055 spent a night on. `docs/experiments/MENU.md` on main (line 11) says PLAY NOW "starts `hockey.regular` at
  the settings' difficulty": it does not.
- **Owner.** U2 (the menu) with the game mode; the Director for the sheet today.
- **Smallest change.** The menu's level reaches the brain for a cut started from PLAY NOW or CHOOSE YOUR CUT (`Brain->ApplyCartridge(*Cartridge, <level>)`, the
  parameter exists: `JamBrain.h:46`); takes and modes keep their own; a joined match keeps the cartridge's (both peers must agree: `LobbyGameMode.cpp:121`);
  a fresh profile starts at ROOKIE (`SettingsWidget.h:31` says `Pro`). **Proof:** a test that a match started at ROOKIE resolves `rookie`; the log line of the
  match start names the level; the novice on PLAY NOW's cut 80 % or more (86 % measured at rookie).

### 2. "The stop got me flattened." (cost; the game's central skill has a window nobody is shown)
- **Number** (`Bar.Movement.StopAndGoSweep`, my rows in `Game/logs/a128/sweep.txt`, 200 seeds a row, the real brain):

  | the stop begins at | 3 m | 4 m | 5 m | 6 m | 7 m | 8 m | 9.5 m | 10 m | 11 m | 12 m | straight |
  |---|---|---|---|---|---|---|---|---|---|---|---|
  | rookie | 95.5 | | **28.5** | | | 100 | | | | | 0 |
  | pro | **0** | 6.0 | 7.0 | 7.5 | 66.0 | **100** | 97.0 | | 83.5 | | 0 |
  | all_star | | | 0 | | | 0 | 39.5 | 57.5 | 89.5 | 100 | 0 |

  A late stop does not lose the puck, it loses the feet: knocked down 199, 179, 153 and 111 times of 200 at 3, 4, 5 and 6 m (pro); at rookie from 5 m 132 of
  200. Inside 5 m no verb beats a pro: the spin 5.5 % and the deke 1.0 % from 4 m (`Jam.Core.Movement.LateAnswer`). The window sits five to six stick lengths out
  (the blade reaches 1.2 m carrying, 1.6 m at full aim), moves out by level, and nothing on the ice marks it. In matches the human class gets past a defender by
  a stop 1.25 times a match at pro (0.5 before), by skating straight 3.3 times (section 4, law 3).
- **Law.** Law 3 holds as a gate (8 m: 200 of 200) and is not yet a thing a person can find.
- **Owner.** The Director (the sheet, the controls screen's line); U1 (a cue); F2 (the window).
- **Smallest change.** The words say 8 m (PLAY-ME, the controls row, drill 21's text). Then a cue, presentation only: the nearest closing opponent on his lane
  wears a ring while he is inside the level's window (7 to 10 m at pro). **Proof:** a still; the hash unchanged. The rule for the curve track after the jam: the
  late stop fails soft (a carrier under 1 m/s is braced against a body check from the front); proof: the stop at 6 m at pro 7.5 -> 60 % or more, at rookie from
  5 m 28.5 -> 80 %, 8 m still 100, straight still 0.

### 3. "Space never takes the puck, and it trips me." (cost; one cartridge key, measured)
- **Number.** Pokes a match and those that take the puck: the novice at rookie 15.2 / 0.4 (**2.6 %**; before 8 %), the regular class at pro 18.1 / 0.8 (4.1 %;
  10 %), the good class at all_star 15.6 / 0.6 (4.0 %; 14 %). In his first match (take 1) a newcomer takes a puck with his stick in 16 of 60 tries. Every
  armed miss is 20 frames without control: the new field `stumbles` reads 6.4 / 6.0 / 5.2 a match (2.0 a minute), and the variant
  `stick.stumble_frames 0` shows whose they are: **6.3 -> 0.3** (rookie), **6.3 -> 0.8** (pro; 200 seeds a cell). All but 0.3 to 0.8 of his stumbles a match are his own missed
  pokes, not hits. The ladder does not need them: 86 -> 84 % and 51 -> 49 % (200 seeds a cell). The take grace is not the cause: at 0 frames his pokes take
  13.1 % and he wins 37 % (the bots gain more).
- **Law.** Law 3, "the mouse is the stick": true for the shot and the pass, not for the duel (REVIEW-1 item 5, unchanged in kind, worse in number).
- **Owner.** The Director for the key (content, as R-0048's two keys were); F2 for the rule.
- **Smallest change.** `"stick": { "stumble_frames": 0 }` in `hockey.regular` and takes 1 to 3 (the schema has the key). **Proof:** `stumbles` 1 or fewer a
  match; the two cells within 4 points (measured: -2, -2). At all_star the same key reads 23 -> 29 % (200 seeds): take 8 keeps the stumble or the curve reads it. After the jam: why 24 pokes in 25 miss (the reach,
  the body shield, the window) split in the playtest line.

### 4. "They score from the red line." (cost; the ladder's largest lever)
- **Number.** The profile lets a bot shoot from its reach + 25 m (rookie), + 30 m (pro: `JamAICore.h:227`, "the single biggest lever") and + 60 m (all_star:
  `DifficultyProfile.cpp:69`). Traced (12 seeds a cell): the opponents' fast releases leave from a median **35.7 m** at pro and **41.8 m** at all_star; 41 %
  and 52 % of them from their own half. Of their traced goals, **7 of 21 at pro and 10 of 40 at all_star were released 25 m or more from the net** (12 of 21
  and 23 of 40 from 15 m or more). With the reach of the first brief (`-aitune opp.shoot_extra_m=13`, 200 seeds): pro 51 -> **61 %**, all_star 23 -> **46 %**:
  half of what makes all_star a wall is the long shot. The human's own mates release from inside 15 m 0.4 times a match.
- **Law.** Law 2 ("pick their moment"); the operator's line that difficulty is aggression.
- **Owner.** C0 (the number) with Q2 (when a bot shoots).
- **Smallest change.** Not free: it is 10 and 23 points of the ladder. If the last curve pass is spent anywhere, here: all_star's reach 60 -> 30 and pro's 30 ->
  20, paid back with the checks and the poke. **Proof:** goals against released from 25 m or more under 10 % of goals at both levels; the three cells within 4.

### 5. "I am on my back a tenth of the match." (cost at pro and all_star; the price of the kept puck)
- **Number.** Knockdowns of the human a minute of play: rookie 0.62 -> **1.02**, pro 1.82 -> **2.38** (2.52 on 200 seeds), all_star 1.93 -> **3.23** (3.38).
  At pro 90 % come while he has the puck and 61 of 82 traced ones at 10 m/s of closing or more, so he lies 146 frames: **7.8 %** of play on the ice at pro
  (4.6 % before), **10.0 %** at all_star (5.7 %), 3.7 % at rookie (1.7 %). The release grace holds (no second fall inside 2 s in the gate's matches); it is the rate and the size.
  `contact.down_frames_max 100`: pro 51 -> 56 %, all_star 23 -> 33 %.
- **Law.** Law 1 is kept to the letter (section 4): head-on at full speed is the biggest hit, and it is the hit he mostly gets. R-0049: "a tackle is an event,
  not a rhythm": one every 24 s at pro, every 18 s at all_star.
- **Owner.** F1 with C0.
- **Smallest change.** None is free. The cheapest: `down_frames_max` 150 -> 120 with the curve's next pass. **Proof:** his share of play on the ice under 6 % at
  pro; cells within 4.

### 6. "Nobody keeps the puck" is still true, for a new reason. (cost; REVIEW-1's first finding, moved a third of the way)
- **Number.** The human's carry, median: 0.87 -> **1.22 s** (novice, rookie), 0.75 -> **0.90 s** (regular, pro), 0.67 -> **0.75 s** (good, all_star); the bar
  was 1.5 s at pro. Carries of 3 s a match: 2.5 -> 2.7, 0.8 -> 1.4, 0.1 -> 0.6 (bar 3). The puck on nobody's blade: 63 -> **60 %**, 60 -> **55 %**, 60 ->
  **60 %** of play (the sim's own `hold_frames` says the same; bar 45). What changed is how a carry ends: lost to a touch 55 -> 38 % at pro, knocked loose by a
  hit 8 -> 14 %. The rule landed; the ladder was then re-set by pressure (pro pokes after 2 frames, checks from 1 m/s), by the carrier that moves the puck
  inside 0.3 s and by the long shot: 86 % of his carries meet an opponent inside 2.5 m, and the puck spends its time in flight.
- **Law.** Law 3 has room at rookie now and little at pro.
- **Owner.** C0 with Q2.
- **Smallest change.** None before Sunday that keeps the cells; item 4's is the nearest (without the long shot the loose share falls 55 -> 53 % and the
  opponents' carry lengthens 0.75 -> 0.83 s). Say it on the page as it is: at rookie you keep it, at pro you earn it.

### 7. "My second match is a waiting room." (cost; the season's order)
- **Number.** Take 2, "The Big Room" (rookie, 120 x 60): a newcomer goes 30 s without the puck in **37 of 60** first tries (13 before), waits 37.9 s of 192
  (12.4), touches it 6.3 times a minute (12.0), wins 83 % (95). Big ice in general: `hockey.arena.large` at pro, the regular class, 26 of 40 matches with such
  a stretch; the opponents hold the puck 42 % of play, his side 21 %. Take 7 (140 x 70): 53 of 60, 98 s of waiting.
- **Law.** Law 2 ("support the puck"): owed by the brain track (R-0055 point 5).
- **Owner.** Q2; the Director for the order.
- **Smallest change.** Content: take 3 before take 2 (his second match is then the regular rink: 90 %, 18 of 60), or take 2's clock 3:00 -> 2:00.
  **Proof:** `Jam.Bot.SeasonTakes` green; the novice's first five minutes with at most one dry half-minute.

### 8. "Take 4 is the wall now." (cost; just outside the ten minutes)
- **Number.** The novice by take: 83 / 83 / 90 / **20** / 12 / 8 / 15 / 3 %. The regular class 88 / 100 / 100 / 60 / 43 / 52 / 38 / 13; the good class 100 /
  100 / 100 / 70 / 72 / 77 / 57 / 42. A newcomer who wins his first three (62 % of them do, in 9.3 minutes) meets take 4 and has no shot in 17 of 60 tries.
  One key the schema already has makes a rung (60 seeds, novice / regular): `ai.react_frames 8` **48 / 92 %**; 16: 68 / 93; `ai.speed_scale 0.9`: 35 / 60;
  `ai.red_skill 0.5`: 45 / 97; `ai.body_check 0.3`: 23 / 70 (the hits are not what beats him: knockdowns 2.9 -> 0.8 and he still loses).
- **Owner.** C0 with the Director (content).
- **Smallest change.** `"ai": { "difficulty": "pro", "react_frames": 8 }` in take 4. **Proof:** the novice 45 to 60 %, the regular class under 95 % (600
  seeds); then takes 5 to 7 stepped the same way (6, 4, 2 frames: not measured).

### 9. "Shotgun blew the whistle on me." (cost; the banner)
- **Number.** Seven of the twelve mode cartridges carry `rules.penalties true`; `MODES-4.md` calls Glass House "the one whistle mode". A foul in 26 of 40
  Arsenal matches (5.7 s in the box), 21 Fog of War, 19 Outnumbered, 18 Glass House, **15 Shotgun**, 13 Sniper, 2 Sudden Death. The regular cut and the
  eight takes: no foul in 6,312 matches (the variants included).
- **Owner.** M1 with the Director.
- **Smallest change.** `rules.penalties false` in the six that are not about it. **Proof:** fouls 0 there; the soak's twelve mode cells re-read.

### 10. "My mates do not finish, and they do not find me more than before." (cost; the best thing of REVIEW-1 has not grown)
- **Number.** Passes received a minute of play: 1.23 -> 0.92 (novice, rookie), 1.73 -> 1.54 (regular, pro), 2.05 -> 1.45 (good, all_star); the per-match
  halving (7.9 -> 4.4 at pro) is mostly the shorter match. The puck still his two seconds after a mate's pass: 32 -> 35 %, 11 -> **19 %**, 5 -> 8 % (the bar
  at pro was 30). One-timers 0.0 to 0.2 a match. His mates shoot 2.3 times a match at pro and release from inside 15 m 0.4 times.
- **Owner.** Q2.
- **Smallest change.** A mate with the puck inside 15 m and an open lane shoots. **Proof:** mates' releases from inside 15 m 1.5 or more a match at pro;
  goals for 2.1 -> 2.4; the cells within 4 (it favours the human: the curve pays it back).

## 2. Five things that are now good and must not be broken in the last day

1. **The whistle is out, and the first hit is a hit.** A foul in 37 of 60 first matches -> 0; 0 fouls and 0 box seconds in every cell of the regular cut and
   the eight takes. The novice chases, so his hits come from behind (11 traced knockdowns at rookie, every one on a skater going his way, at 3.8 m/s of closing): they land now, and short (72 frames).
2. **The first match moves.** Take 1, the novice: first shot 81.9 -> **35.3 s**, a goal by him inside two minutes 31 -> **42 of 60**, the match 4.2 -> 2.2
   minutes; both bars of REVIEW-1 met. Take 3: 22 -> **90 %**. The first ten minutes have no wall: 83, 83, 90.
3. **Law 1 in real matches.** Frames down follow the speed brought (pro, 154 traced knockdowns): under 6 m/s of closing 80 to 89 frames, 6 to 10 about 92,
   10 and up 146; head-on 20.3 m/s, same way 6.6 m/s. The same-speed press is a bump (ratio 0.35 and 0.28), the sprinter into a braced skater is the one who
   staggers, and the HUD has the words (`NOT WITH THE PUCK`, `TOO SOFT`, `JUST GOT UP`, `BOUNCED OFF`).
4. **The stop-and-go is a real skill with a window the level moves.** 100 % at 8 m at pro, 100 % at 12 m at all_star, 0 % straight at every level, the
   mouse counted (the blade left ahead: 31.5 %). It needs to be shown (item 2), not changed.
5. **The bots play the game the player is told to play.** Reverse brakes 2,146 -> **270** a match at pro (467 -> 94 a minute); the bot carrier sprints 13 s
   a match (0); a mate takes the puck off the human's blade 1.0 -> 0.33 times a match; his mates skate at his pace at all_star (15.5 -> 11.7 m/s). And the
   ladder is where R-0046 put it on fresh seeds: 86 / 51 / 23 %.

## 3. Before and after: REVIEW-1's numbers on the same seeds (grid 40 seeds a cell, takes 60, traces 12)

"Moved by" is read from `contracts/RULINGS.md` (R-0048 to R-0055) and the lanes' documents, not measured by arm.

| # | Number | REVIEW-1 | Now | Bar | Moved by |
|---|---|---|---|---|---|
| 1 | the human's carry, median: novice at rookie / regular at pro / good at all_star | 0.87 / 0.75 / 0.67 s | 1.22 / **0.90** / 0.75 s | 1.5 s at pro: **no** | A102 the kept puck, A121 the take grace; taken back in part by C0's re-set |
| 1 | `carries_3s` a match | 2.5 / 0.8 / 0.1 | 2.7 / 1.4 / 0.6 | 3 at pro: no | the same |
| 1 | the puck on nobody's blade, share of play | 63 / 60 / 60 % | 60 / 55 / 60 % | 45 % at pro: no | the same |
| 1 | his carries that end on an opponent's touch / a hit (pro) | 55 / 8 % | 38 / 14 % | | A102 |
| 1 | an opponent's carry, median | 0.63 / 0.60 / 0.65 s | 0.65 / 0.75 / 0.68 s | 1.0 s: no | A115, A133 |
| 2 | take 1, the novice: matches with a foul; seconds in the box | 37 of 60; 5.6 | **0; 0** | 0: **met** | A112's content step (R-0048 point 2) |
| 2, 7 | acceptance rows for the press that did not land, the stumble, the braced bounce | none | not run here (no window); the texts and `PlayStumble` exist in source | 5 rows | A101, A113, A125 |
| 4 | take 1, the novice: first shot; a goal inside two minutes; the match | 81.9 s; 31 of 60; 4.2 min | **35.3 s; 42 of 60; 2.2 min** | 45 s; 40: **met** | A102 (the unpressed blade no longer takes a carried puck) |
| 5 | pokes that take the puck: novice at rookie / regular at pro / good at all_star | 8 / 10 / 14 % | **2.6 / 4.1 / 4.0 %** | reported: worse | A102's body shield, A121 |
| 5 | his stumbles a match (pro); of those his own missed pokes | up to 29 misses | 6.0; all but 0.8 | under 12: met | the playtester presses less (18.1 pokes, 32.6 before) |
| 6 | passes received a minute of play | 1.23 / 1.73 / 2.05 | 0.92 / 1.54 / 1.45 | not under: no | A102 (a touch no longer counts), A115 |
| 6 | still his two seconds after a mate's pass | 32 / 11 / 5 % | 35 / 19 / 8 % | 30 % at pro: no | A115 |
| 8, 9 | bot reverse brakes a match at pro; bot carrier sprint frames | 2,146; 0 | **270; 787** | under 500; 120: **met** | A115 |
| 9 | the pile (three of a side within 6 m of the puck), bot passes a minute, pro | 16.6 %; 8.0 | 13.5 %; 7.8 | 12 %; 11.7: no | A115 |
| - | knockdowns of the human a minute of play | 0.62 / 1.82 / 1.93 | 1.02 / 2.38 / 3.23 | | C0's re-set on the kept puck |
| - | the human's hits that land, a minute | 1.28 / 1.06 / 1.65 | 1.09 / 1.37 / 1.28 | | |
| - | the three target cells (40 seeds; 200 seeds) | 83 / 50 / 30 | 85 / 55 / 18; **86 / 51 / 23** | 85 / 48 / 26: met | A112, A133 |
| - | the novice at take 3; at the first pro take | 22 % (take 3) | 90 % (take 3); **20 %** (take 4) | 60 %: met; the wall moved | A112 |
| - | a mate takes the puck off the human with no pass, a match (pro) | 1.0 | 0.33 (the script cannot tell a one-metre pass from a touch) | 0 | A102 |
| - | the mates' speed at all_star, 90th percentile | 15.5 m/s | 11.7 m/s | his pace: met | A112 (R-0048 point 4) |

Tables: `review-2/data/grid.md`, `season.md`, `large.md`, `modes-regular.md`; cells with every read-out `*.cells.jsonl`; carries `*.carry.json`; the laws
`*.laws.json` (`review1-*.laws.json` is the new script on the first review's raw logs, read from `.claude/worktrees/a100/tools/playtest/out/a100/raw/`).

## 4. The four laws, one by one

**Law 1, a hit obeys momentum: kept, in drills and in matches.** Drills (run here, all green): same way at the same speed 0.35 from behind and 0.28 from the
side (a bump, no event); head-on at 11 m/s each 4.61 (down 150 frames), at 4 and 3 m/s 1.46 (81); a side hit at 3.5 m/s of closing stumbles a skating victim
(0.90) and fells a sprinting one (1.28); walking into a braced skater 0.53, sprinting into him 0.85 with the attacker's own stumble. Matches: the trace has no
contact event, so a knockdown is read from the inputs (`review-2/laws.mjs`: a downed skater gets none; the count agrees with the playtest line, 13.25 against
13.42 a match at pro) and its closing speed from the positions before it (about +-1 m/s).

| pro, 12 matches, 154 knockdowns | under 2 m/s | 2 to 4 | 4 to 6 | 6 to 8 | 8 to 10 | 10 and up |
|---|---|---|---|---|---|---|
| knockdowns | 4 | 4 | 11 | 20 | 18 | 97 |
| frames down, median | 80 | 83 | 89 | 93 | 91 | 146 |

Head-on with both moving: 67, at 20.3 m/s, 146 frames. The same way with both moving: 23 (15 %), at 6.6 m/s, 89 frames: a chase-down, not a nudge. Who
hits whom: the human is the victim in 82 (closing 14.8 m/s, 132 frames, 90 % with the puck), the attacker in 28 (6.3 m/s, 95 frames). The bump and the
stumble cannot be read from the trace. The gate's census (three pro matches on the contact track's pinned tree, not the shipped rules) gives the shape: the
bots take 14 bumps, 8 stumbles and 18 knockdowns; the human takes 0, 2 and 19. **What the law looks like from his side: one size, the largest.**

**Law 2, the brain knows hockey: half.** It brakes on purpose and sprints with the puck (section 2). Passes to the human a minute are level with Friday
(item 10). His idle stretches on the regular rink: 6.9 / 2.4 / 1.3 s a minute (4.2 / 1.6 / 1.1), a 30 s stretch in 11 / 0 / 1 of 40 matches (12 / 3 / 2);
on big ice it is the rule, not the exception (item 7). Shot selection is where it fails:

| the opponents' fast releases (15.5 m/s or more; the sim counts 13.4 / 12.2 / 23.7 a match as shots) | rookie | pro | all_star |
|---|---|---|---|
| a match (a minute of play) | 13.6 (4.8) | 14.8 (5.0) | 35.6 (9.5) |
| distance to the net, median | 28.8 m | 35.7 m | 41.8 m |
| from their own half; of those aimed within 5 m of the net | 37 %; 7 % | 41 %; 7 % | 52 %; 38 % |
| traced goals released at 0-10 / 10-15 / 15-25 / 25-41 m | 7 / 2 / 2 / 2 | 4 / 5 / 5 / 7 | 7 / 10 / 13 / 10 |

The lockstep lane's 273 shots in a match was not reproduced here (two brains, no human; not run); its shape is: at all_star a release every six seconds, half
of them from the wrong half of the rink.

**Law 3, speed change and the stick beat a defender: true in the drill, rare in a match.** The drill is item 2's table. In matches (`laws.mjs`: an opponent
in front of the carrying human, within 6 m, that he leaves a metre behind while still carrying):

| a match | novice at rookie | regular at pro | good at all_star |
|---|---|---|---|
| defenders met; beaten (share) | 14.0; **6.0 (43 %)** | 17.3; **6.0 (35 %)** | 21.3; **8.1 (38 %)** |
| the same on Friday | 21.2; 6.5 (31 %) | 18.1; 4.5 (25 %) | 22.2; 6.3 (29 %) |
| beaten by skating straight / a stop / a cut / the defender was down | 4.25 / 0.42 / 1.33 / 0 | 3.33 / **1.25** / 1.25 / 0.17 | 4.17 / 1.67 / 1.75 / 0.50 |
| not beaten: robbed / knocked loose / he moved the puck | 3.6 / 1.5 / 1.3 | 5.0 / 2.2 / 2.0 | 6.8 / 3.7 / 1.7 |

He gets past two defenders a minute, twice Friday's rate, and more than half of those are plain speed. The stop is the playtester's hand (it brakes by rule,
not by distance): a person who learns 8 m will use it more; nobody is told 8 m.

**Law 4, sprint and the puck's distance: the rules are in, the gamble is not felt.** Drills: the carrier 10.12 m/s, sprinting 10.93, a sprinting defender
12.65 (an 8 m gap is 5.0 m after 2 s); the puck rides at 1.23 m skating and 1.63 m sprinting; the sweep shields 12 frames skating and 6 sprinting. Is the
sprinting carrier easier to rob? **Not in a match.** `Jam.Core.Movement.BotMatchPro` (one match an arm): a stick takes the puck 4.55 times per 1,000 carry
frames from a skating carrier and 2.54 from one that always sprints. In my traces the human at pro loses 27 carries a carried minute at skating pace and 11
above 10.5 m/s, because both he and the bots sprint only on open ice (an opponent within 3 m on 67 % of the skating frames, 41 % of the fast ones). He
sprints with the puck 1.5 s a match, the bots 13 s.

## 5. The first ten minutes of a newcomer (the novice, 60 seeds a take; H27's first press is PLAY THE SEASON)

| When | What happens | Number |
|---|---|---|
| 0:02 | he wins the draw | first touch 1.8 s |
| 0:35 | his first shot | 35.3 s (81.9); no shot at all in 6 of 60 |
| 0:46 | his first goal | 45.6 s (84.9); inside two minutes in 42 of 60 |
| in between | he presses Space ten times and takes one puck in a quarter of his matches; he trips on it four times | 9.7 pokes, 0.3 clean, 4.0 stumbles; a puck by a poke in 16 of 60 |
| | he hits, from behind, and it counts | a hit landed in 50 of 60; 2.5 a match; no foul |
| | he is knocked down | 2.4 times (1.1 a minute; 0.6 before); in 51 of 60 |
| 2:12 | take 1 ends | **83 %** won; the puck is his 16 % of play, on nobody's blade 61 % |
| 2:15-5:40 | take 2, the big room: he waits | 30 s without the puck in 37 of 60; first shot 50.6 s; 83 % won |
| 5:40-9:20 | take 3, the regular rink again | **90 %**; first shot 35.7 s; 4.0 hits landed, down 3.8 times, 6.9 stumbles |
| 9:20 | take 4 is pro | **20 %**; no shot by him in 17 of 60; a goal by him in 20 of 60 |

Verbs he uses in his first match (matches of 60 with at least one): shot 54, pass 51, pass received 51, hit landed 50, poke 58, the puck held 3 s 48, a
reverse brake 57 (14 a match, none of them meant: the hint bar names the brake now, `HUDWidget` `ControlsHintText`). He never cuts, dekes, spins, sprints or
one-times in 480 matches: the model's first-timer does not know those verbs, so their first use is a person's to find. **Where he gets stuck: Space (item 3),
the big room (item 7), take 4 (item 8).** On main, where the first press is PLAY NOW, he gets stuck at once: pro, 18 % (item 1).

The regular class through the same door: 88 / 100 / 100 %, take 1 in 1.2 minutes, then 60 / 43 / 52 / 38 / 13.

## 6. The verdict on `docs/PLAY-ME.md`, line by line

| Line | Verdict |
|---|---|
| The rink is clay from the first match | **True** (`look.tier p3_clay` in the regular, large and wide cuts and takes 1 to 4). Not seen on screen by this lane |
| No whistle in the regular game and the season; Glass House keeps them | **True, and incomplete**: six more modes keep it, Shotgun among them (item 9) |
| You keep the puck until somebody presses for it; half a second of grace | **True as a rule, misleading as a promise**: somebody presses inside a second (item 6) |
| Stopping works; a quick first step | **True** (8 m: 200 of 200) |
| Hits have sizes; the HUD says why a press did not land | **True** in the sim and in source; he will mostly receive the largest (item 5) |
| Not knocked down again the moment you get up | **True** (no second fall inside 2 s in the gate's matches) |
| Your mates pass to you and go for loose pucks; bots brake on purpose | **Half**: the brakes yes; passes a minute are level with H26 (item 10) |
| The arena answers; the camera keeps you in frame; a yellow chevron | In source (`RinkActor.cpp:32-48`, `HUDWidget.cpp:491`); **not judged** (no window) |
| The menu: (from H28) PLAY NOW ... | Not in this tree; on main it starts pro (item 1) |
| 1. A first match on the regular cut at rookie. A newcomer wins about 85 % | **False as an instruction**: the level cannot be chosen. The number is right (86 %) |
| 2. Press S "about two stick lengths away" ... "at pro it should work nearly every time from that distance" | **False**: 0 to 7.5 % from 3 to 6 m, a knockdown; 100 % from 8 m. "Never if you just charge": true (0 of 200) |
| 3. Two hits: a nudge beside him, down head-on | **True** (0.28 and 4.61). It needs him without the puck: with it the press says NOT WITH THE PUCK |
| 4. Keep the puck: his stick should not reach it past your body | **True** for the stick (`Jam.Core.Movement.BodyShield` green); a defender arriving at speed still fells him from behind (ratio 2.29 at 8 m/s) |
| 5. Take 3 is easy now (86 %); the step to take 4 is too steep | **True** (90 %; 20 %) |
| 6. Pro: a regular player wins about half. All-star about a quarter | Numbers **true** (51 %, 23 %); all_star **cannot be reached** on the regular cut |
| 7. Shotgun, Cage, Pond | **True** (Shotgun 1.5 knockdowns by F a match; Cage 92 board hits a match; Pond 0 hits, five a side). For a newcomer: Shotgun 10 %, Cage 23 %, Pond 73 % |
| Controls | **True** against `InputMapping.cpp`; Ctrl also brakes |
| Known: big ice, half a minute without the puck in about four matches in ten | **Low**: 26 of 40 (the regular class), 22 of 40 (the good class) |
| Known: all-star a little soft (30 %) | Not on these seeds: 23 % (200), 18 % (40); the curve lane's 2,000 read 29.6 |
| Known: takes 4 to 7 too close, too far from take 3; the loss card says REMATCH? | **True** (`HUDWidget.cpp:359`) |
| Question 2: did rookie, pro and all-star feel like three different games? | **He cannot answer it**: he will have played pro three times |

## 7. Findings outside `docs/design` (no edit made)

| Where | What |
|---|---|
| `Game/Source/JamGame/Private/HokejGameMode.cpp:402`, `:244`; `SettingsWidget.cpp:44`, `:305`; `Public/SettingsWidget.h:17`, `:31` | The menu's level is stored and never applied. Item 1 |
| `docs/experiments/MENU.md:11` (main) | "starts `hockey.regular` at the settings' difficulty": untrue |
| `docs/PLAY-ME.md:24`, `:23`, `:29`, `:40`, `:52` | The stop's distance; two steps and a question that need the level; the big-ice share. Section 6 |
| `Game/Content/Cartridges/modes/hockey.mode.{arsenal,fog-of-war,outnumbered,shotgun,sniper,sudden-death}.cartridge.json` | `rules.penalties true` in modes that are not about the whistle; `docs/experiments/MODES-4.md:29` says Glass House is the one. Item 9 |
| `Game/Source/JamCore/AI/JamAICore.h:227`, `DifficultyProfile.cpp:31`, `:69` | The shot from reach + 30 / 25 / 60 m. Item 4 |
| `Game/Source/JamCore/Private/JamSim.cpp:1549` with `game.stick.stumble_frames` | The armed miss's stumble: six a match, no cell needs it. Item 3 |
| `Game/Content/Cartridges/ladder/take-2.cartridge.json`, `take-4.cartridge.json` | The waiting room second; the wall fourth. Items 7 and 8 |
| `tools/playtest/run.mjs:71` | `--set` on a cartridge in a subfolder (`ladder/take-4`) writes into `tools/playtest/out/ladder/`, which it never creates: every cell fails with ENOENT |
| `Game/Source/JamCore/AI/JamPlaytester.h:153` | `stumbles` counts the human's own missed-poke stumbles with the hits' (0.3 to 0.8 a match are hits): a soak reader will take it for contact. Split it |
| the playtest line | No contact outcome, no closing speed, no shot position, no sprint flag in the trace: laws 1, 2 and 4 are read sideways (`laws.mjs`). For S1 after the jam |

## 8. What this review could not see

No frame of the game: the clay look, the arena's answers, the chevron, the camera at the boards, the hit's size on screen, sound, the gamepad, a joined
match. No person: the playtester brakes and pokes by rule, so the stop's use in matches and the poke's 4 % are a script's (a hand may do better at both).
The first-use time of a verb other than the touch, the shot and the goal is not in the line. Samples: a 40-seed cell carries about 8 points of noise, a
60-seed cell 6, a 200-seed cell 3.5; the traced medians rest on 150 to 270 carries of the human a cell.

## 9. The track's next three evolutions (F0)

1. REVIEW-3 on the package the operator played, after his priorities: only the items he kept, each with its bar from this file, plus item 1's test. Proof: the
   table of section 3 with a third column.
2. The stop's window as a person finds it: drill 21 script 2 (his own keys) with the distance at his first brake frame logged, three people or three hands of
   the playtester at 6, 8 and 10 m. Proof: the share of first tries that begin inside the window, before and after the cue.
3. The twelve mode cards against the build (the whistle, the facts, the newcomer's win share: Shotgun 10 %, Cage 23 %). Proof: twelve rows, true or the
   corrected line.

## 10. Reproduce

```
node tools/playtest/run.mjs --players novice,regular,good --levels rookie,pro,all_star --rinks regular --seeds 1-40 --jobs 6 --out docs/design/review-2/raw/grid
node tools/playtest/run.mjs --players novice,regular,good --levels rookie --rinks ladder/take-1,ladder/take-2,ladder/take-3 --seeds 1-60 --jobs 6 --out .../raw/ladder123
   (takes 4 to 7 with --levels pro, take 8 with --levels all_star; --rinks large for the big rink; tools/playtest/modes.mjs --seeds 1-40 for the modes)
UnrealEditor-Cmd <uproject> -run=JamTournament -sim=jam -cartridge=hockey.regular -difficulty=pro -player=regular -seeds=1-12 -maxframes=36000 -trace=1
   -nullrhi -Multiprocess -unattended -nopause -nosplash -stdout -FullStdOutLogOutput > raw/pro-regular.log      (arm: -aitune=opp.shoot_extra_m=13)
node docs/design/review-2/laws.mjs <traced log>            # the four laws: knockdowns by closing speed, shots by where, defenders beaten, pace
node docs/design/review-1/humancarry.mjs <traced log>      # carries, who has the puck (REVIEW-1's script, unchanged)
node docs/design/review-2/first.mjs <lines.jsonl ...>      # one line a cell: the clock, per-minute rates, matches in which each verb was used
UnrealEditor-Cmd <uproject> -ExecCmds="Automation RunTests Jam.Core.Contact+Jam.Core.Movement+Bar.Movement.StopAndGoSweep+Jam.Bot.Hockey2; Quit" -nullrhi -Multiprocess ...
   (the sweep's rows: Game/logs/a128/sweep.txt, e.g. "pro brake at 5 m | brake_at=5, level=1, seeds=200, straight=0")
node tools/playtest/run.mjs --players regular --levels pro --rinks regular --seeds 1-200 --jobs 1 --set "stick.stumble_frames=0" --out .../raw/v-stumble0-pro
   (variants: stick.take_grace_frames=15 | 0, contact.down_frames_max=100; take 4: --rinks ladder/take-4 --set "ai.react_frames=8", after mkdir tools/playtest/out/ladder)
```
