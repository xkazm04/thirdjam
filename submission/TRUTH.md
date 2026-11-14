# The truth table, pass 2: every claim the submission makes, against the game as it is on Saturday 14 Nov 08:00 (lane A180, track D1, main 9e452219)

**What this is.** Lane A142 wrote this table on Friday night against main `41df3e6`. About fifty landings later the game is package **H30** (main `95ae15a6`, cut 07:03,
acceptance green, checked in a real window: R-0063). This pass re-reads every row of Friday's table against this main and adds the rows the night made.
Nobody ran a game for it: `TRUE` means "the cartridge, the code, a measured document or a still of the real window says so", never "I played it".
The operator plays H30 at 10:00; nothing here assumes his answers: where a sentence needs one it carries a slot `[O<n>]`, and the question is in `submission/README.md` section 1.

**Verdicts.** `TRUE` = Friday's corrected sentence still stands. `CHANGED` = it stood or was tagged `[waits: ...]` on Friday and the game has moved: the words change
(most `waits` have landed). `FALSE` = the sentence as it stood on the page this morning is not true of this main. `ASK` = an operator's slot.

**Sources, short names.** `PM:n` = `docs/PLAY-ME.md` line n. `R3:n` = `docs/design/REVIEW-3.md` line n (11,708 measured matches on the commit of package H28, the same game as H30 but for the stop mark's shape and the menu's words).
`R-00nn` = `contracts/RULINGS.md`. `M6` = `docs/experiments/MODES-6.md`. `card` = the mode's `game.meta` in `Game/Content/Cartridges/modes/*.cartridge.json`
(what the menu prints). `A155:n` = `status/A155.md` line n (the lane that drove the real window). `B:Hnn` = `status/B.md` entry. `TR` = `status/TRAILER-report.md`.
`still nn` = `submission/shots/a155-nn-*.png` (main checkout).

## 0. What must be findable: the game this morning, one line each

| ID | Fact | Status | Source |
|---|---|---|---|
| N01 | **The level switch reaches the match.** ROOKIE / PRO / ALL-STAR under PLAY NOW; the lit level plays PLAY NOW and the cut cards; the season's takes and the modes set their own; a fresh profile is rookie; the match card and the pause card show the level in force. The menu's own words: rookie "New to hockey games? Start here: rookies rarely hit."; pro "Played one before? Pros hit as a habit."; all-star "The wall. All-stars hunt the puck. Beat pro first." | TRUE | R-0059 point 1; R3:175-177; PM:16, PM:32-33; `MenuFlow.cpp:401-410`; A155:79; still `docs/experiments/img/a164/first-1080-rookie-who.png` |
| N02 | **Who each level is for** (no number on the page): a newcomer wins most rookie matches; a regular player wins about half at pro; all-star is the wall (a good player wins fewer than one in three). Anyone who has played a hockey game wins rookie every time in under two minutes. | TRUE (the page says "about half", never a percentage: pro is 52 today and 48 if the operator takes the dial, R-0063 point 2) | R-0062; PM:33-37; R3:120-124 |
| N03 | **One stop distance at every level: five stick lengths (8 m).** The stop begun there beats the defender 200 of 200 at rookie, pro and all-star; a straight charge 0 of 200 at each. At pro the window is 8 to 9.5 m: later you are flattened, earlier it fails half the time. | TRUE | R-0058 point 1; R3:178-179, R3:241-247; PM:41-42 |
| N04 | **Hits obey closing speed, in real matches.** A bump, a stumble, or down for 1 to 2.5 s by the speed brought; beside a man at his speed the check is a nudge, head-on at full speed he goes down; a standing skater holds a side hit; with the puck you cannot check. | TRUE | R3:185, R3:223-231; PM:22, PM:58; R-0060 point 2; A155:28, A155:33; stills 06, 07 |
| N05 | **No whistle outside one mode.** The regular game, the season and eleven of twelve modes have no fouls and no box; Glass House keeps the whistle ("The only whistle left"). The first screen prints "Hockey with the whistle taken out." | TRUE | PM:17; R3:182-183 (no foul in 8,600 matches of the regular cut, the large rink and the takes); R-0059 point 2; `MenuFlow.cpp:379`; card |
| N06 | **Space takes the puck, and a miss locks the stick.** About one press in ten takes it (it was one in twenty-five); a miss beside a carrier locks your stick for a third of a second instead of tripping you; STICK LOCKED shows only for a miss at a puck an opponent carries within about 3 m. | TRUE, with the limit that it is **one press in ten, not on demand** | R3:146-148, R3:180-181; PM:19, PM:88; `JamSim.cpp:1596`, `:1646-1657`; seen by hand in a window: A155:66 |
| N07 | **You keep the puck until somebody presses for it** (the rule): an unpressed blade or a body's touch does not take it; half a second of grace after winning it, shown as a yellow ring under the carrier that closes while no stick can take his puck. | TRUE as the rule; **the carry is short** (N15) | PM:18; R-0050 point 1; A155:51 |
| N08 | **Nothing is drawn on a downed player**, and he is not knocked down again the moment he gets up (two seconds of grace). | TRUE | R3:186 (0 of 2,432 frames); PM:23; R-0051 point 1; A155:69 |
| N09 | **Two marks on the ice.** A ring in the other team's colour round a loose puck: one of them arrives when you do. A chalk line five stick lengths ahead of you while you carry: it turns the defender's colour and thickens at the last moment a stop still works against him. **The stop line is an offer, not an order**: a scripted player who stopped at every lit line won half as often; the stop beats one defender with open ice behind him. Nobody has played with the marks yet. | TRUE; new tonight; unplayed | PM:21, PM:43-46, PM:79; R3:65-75; A155:81; still `docs/experiments/img/a171/stop-line-lit-and-hint-strip-f000131.jpg` |
| N10 | **Twelve modes**, each with its one-line identity as the menu prints it and as measured (section 1, PG30 to PG42). Team sizes this morning: five a side: Arsenal, Bruisers, Glass House, Pond, Shotgun, Sudden Death; four a side: Cage; three a side: Fog of War, Marathon, Sniper; uneven: Power Play (five on three), Outnumbered (three on five). | TRUE | cards; M6 table; R3:29, R3:290-305 |
| N11 | **The clay rink with timber stands and a clay crowd**, from the first match: the regular cut, the wide and large arenas and takes 1 to 4 are clay; takes 5 to 8 change the scene (neon, night, night, neon); the modes keep their own looks. | TRUE by cartridge, by device B's report and by one window still; **not judged by a reviewer's eye** (R3:326): the operator's at 10:00 (`[O3]`) | R-0054 point 1; PM:15; `hockey.regular.cartridge.json` `look.tier` `p3_clay`; `ladder/take-*.cartridge.json`; B:H45.0, B:H46.6; still a171 (above) |
| N12 | **The season: eight takes and a ramp.** The Read-Through, The Big Room, Rough Cut, Set Pieces (rookie, clay); Second Wind, The Last Word, Slow Motion (pro, eased); The Final Cut (all-star). Takes 1 to 4 are a newcomer's school (he wins more than four in five); the steps a person can feel are take 5 and the finale. For a regular player there is no contest before the finale. | TRUE | R-0057; R3:163-164, R3:280-288; PM:62-63; `ladder/take-1..8.cartridge.json` |
| N13 | **The joined match.** Two processes on one machine play through the real lobby to the final card, with a rematch. **Two physical machines have never been tried.** A real firewall prompt and Wi-Fi loss: not tested. Same network, UDP 7700, protocol 8; 1,000 soak matches without a desync. | TRUE, said exactly so | PM:67-68; `docs/experiments/JOIN-GUIDE.md:3`, `:73`, `:59`; R-0055 point 6 |
| N14 | **The package chain is real.** `Hokej_Hokej_H30.zip` from main `95ae15a6`, acceptance run green (18 verbs through the cursor and the buttons); the fresh zip unzipped and played through the window. Which package is submitted is the operator's (`[O15]`). | TRUE | `status/A.md:453`; R-0063 point 1; A155:68, A155:75 |

### Measured, and therefore NOT to be said on the page

| ID | Do not say | Because | Source |
|---|---|---|---|
| N15 | that the carry is long, or that the puck sticks | his carry's median is about a second (0.78 s at pro; the bar was 1.5); the puck is on nobody's blade more than half of play; at rookie one in six of a newcomer's carries slides off his own blade and one in ten goes to an opponent's bare touch | R3:103-107; PM:86 |
| N16 | that Space works on demand | one press in ten (N06) | R3:147 |
| N17 | that timing a check beats mashing | mashing RMB lands about 20 checks a match against 4.5 timed and wins more; the cooldown that fixes it is built and switched off | R3:113-115; PM:47 |
| N18 | that the stop wins matches, or "obey the line" | in matches the stop gets him past a defender 0.67 times a match; plain speed does it five to ten times as often; stopping at every lit line halves the win rate | R3:248-250; PM:43-45 |
| N19 | that the bots shoot like hockey players | opponents release from a median of 36 m at pro and score from the red line | R3:130-133; PM:84 |
| N20 | any win rate as a number | pro's dial, the switched-off rules and the modes' re-tune can each move a cell by 3 to 10 points within a day | R-0063 points 2 to 4; `docs/EVOLUTION.md` board 08:20 |
| N21 | that every mode is a different game | Bruisers and Cage have drifted towards the regular game (hits 1.33x, checks 1.47x at 200 seeds) | R3:295-296, R3:310-311 |
| N22 | that you are rarely on the ice | he lies down about 9 % of play at pro; two knockdowns in three land within half a second of winning the puck | R3:94-97; PM:80 |
| N23 | a horn, an ink outline, spot pools, a replay or a low camera | no document shows a horn bound or the outline in the packaged look; a package has only the game camera | TR "Not verified" and its opening note; Friday's PG45, PG46, VS08 |

## 1. `submission/page.md`: Friday's rows re-read

"Friday's sentence" is the corrected sentence A142 left in the file (the one a reader met this morning), not the older one it replaced.

| ID | Friday's sentence (short) | Verdict | Source line | The sentence now |
|---|---|---|---|---|
| PG01 | "Hockey with the whistle taken out" `[waits: A133]` | CHANGED | N05 (landed) | kept, no tag; the page adds "one mode keeps it" |
| PG02 | "clay figures"; no rink look promised (O3) | CHANGED | N11 | "clay skaters on a clay rink, timber stands, a clay crowd"; `[O3]` only for his eye's yes |
| PG03 | the mouse is your stick | TRUE | `status/A.md:453` (18 verbs through the cursor); `MenuFlow.cpp:373` | kept |
| PG04 | once a period the Director's Call moves one dial "(the ice, the boards, how fast the bots skate, how hard the shot flies)" | CHANGED | `DirectorsCallWidget.cpp:232` (one change per period), `:89-93` (the dials shown first: skating speed, shot speed, ice friction, a pickup number, how sharp the bots are), `:489-491` (the on-screen CALL READY pill waits for the first goal; the pause card of still 11 offers the call at 0 : 0) | "skating speed, shot speed, the ice, how sharp the bots are"; "the boards" and "how fast the bots skate" dropped (in the cut's list, not among the five the card shows first) |
| PG05 | hits obey momentum | TRUE | N04 (now in real matches) | kept |
| PG06 | "in our drill a standing pro defender loses to it" | CHANGED | N03 | "from five stick lengths it works at every level" |
| PG07 | five a side, and the bots pass | TRUE | `hockey.regular` (5 and 5); R-0052; PM:24, PM:28 | kept; "your mates pass to you" added |
| PG08 | nobody is polite | TRUE | (voice, no fact) | kept |
| PG09 | cuts are picked in the menu; the call moves one dial | TRUE | PG04; still 02 | kept |
| PG10 | "Real hits. The speed you bring is the hit you get." | TRUE | N04 | kept |
| PG11 | "Two skaters gliding the same way just bump" | CHANGED | PM:58; drill 22, A155:28 (the nudge: a bump, both skate on); R3:230 (in matches thirteen knockdowns came "the same way", at 6 m/s of closing) | "beside a man **at his speed** the check is a nudge": the speed between the bodies decides, not the direction |
| PG12 | "head-on at full speed is the biggest hit, and the one who throws it pays for it too" | CHANGED | R3:230 (head-on, both moving: 146 frames down, the longest); `docs/VISION.md:24` says the attacker pays as a rule, and no document of this morning measures it | "head-on at full speed is the biggest hit in the game"; the second half dropped |
| PG13 | with the puck you cannot check | TRUE | A155:33 (the plate NOT WITH THE PUCK, `refused=1`) | kept |
| PG14 | S against your motion brakes; `[waits: A135]` for the prompt | CHANGED | A155:80 (the first faceoff's strip: BRAKE PRESS AGAINST YOUR MOTION); `InputMapping.cpp:99` (Ctrl brakes too); N09 (the stop line) | tag gone (landed); "or hold Ctrl" |
| PG15 | then burst away | TRUE | PM:20 (a quick first step after a stop) | kept |
| PG16 | "a defender at full sprint is always faster than you in a straight line" | CHANGED | N03 (a straight charge: 0 of 200); R3:248-249 (in matches he passes more defenders by plain speed than by a stop) | "charge straight at a defender who is set for you and you do not get through" (the drill's fact, not a law of every race) |
| PG17 | bots carry, pass, do not charge a wall; brakes on purpose and a sprinting carrier `[waits: A138]` | CHANGED | R-0052; R3:234 (252 reverse brakes a match at pro, was 2,146; 10.6 s of sprint with the puck) | tag gone (landed); the limit N19 said beside it |
| PG18 | "the harder the level, the more it hits, and the top one also skates faster. `[waits: A133]` The ladder is being re-set tonight" | FALSE | N01, N02; R-0062 (the ladder is set; the switch works) | the three levels in the menu's own words; who each is for; no number |
| PG19 | abilities on F: shotgun, hip check, shield, slow puck | TRUE | R3:294 (Arsenal: 4.85 tricks on F a match); cards | kept; "each trick fires three times" (card) |
| PG20 | Pond is a cut | TRUE | card | kept |
| PG21 | skate, aim with the mouse, hold left to charge, let go to fire | TRUE | still 03 (the controls screen) | kept |
| PG22 | "Poke the puck (it rarely wins alone)" | CHANGED | N06 | "Space jabs for the puck: about one press in ten takes it; a miss beside a carrier locks your stick for a moment" |
| PG23 | body-check a carrier into the boards | TRUE | PM:25 (a board hit rocks the boards); still 08 | kept |
| PG24 | middle mouse lifts the puck off the carrier you are on top of; the shove not promised | TRUE | still 03 ("on top of the carrier: lift the puck off him"); R-0055 (nobody is shoved from behind) | kept |
| PG25 | go down, slide, get up | TRUE | N04, N08 | kept, with the sizes |
| PG26 | the season: eight takes, one twist each, rookie up to all-star; take 3 at rookie `[waits: A133]` | CHANGED | N12 (landed and re-made) | the school, the two steps, the finale; no number |
| PG27 | the controls table, keyboard and mouse | TRUE | still 03; `InputMapping.cpp:79-127` (R3:325) | kept |
| PG28 | the pad column; "mapped, not play-tested" | CHANGED | still 03: brake on the pad is "stick back, or L3", restart is "View"; the screen itself says "The pad is mapped; no person has play-tested it yet"; PM:94-95 (the pad's walk through the menu could not be proven) | two cells follow the screen |
| PG29 | Esc: pause, Time Out, Director's Call | TRUE | still 11 | kept |
| PG30 | Pond: skate and pass, nobody hits | TRUE | card "Pond hockey. Pass, skate, nobody gets hit." (5 v 5 on 60 x 30 m, 3:00); R3:301 (passing 1.65x, no knockdowns) | kept |
| PG31 | Cage: eight bodies in a tight box, the boards hit back | TRUE | card (60 x 28 m, 4 v 4, boards hit 2x, four pillars); R3:296 (less distinct than it was: N21) | kept |
| PG32 | Sniper: a full-height net and a low guard, lift it | TRUE | card (net 1.22 m, pads at 0.15 m, 3 v 3); R3:304 (goals per shot 1.93x) | kept; "three a side" |
| PG33 | Marathon: "eight minutes on long ice" | FALSE | card "Three on three on endless ice. Bring your legs." (140 x 70 m, 6:00); M6 section 5 | "three on three on endless ice: bring your legs" |
| PG34 | Power Play: "four against three" | FALSE | card "Five on three. Move it, fire it, repeat."; M6 table (5 v 3) | "five on three" |
| PG35 | Shotgun: one blast, every skater within a metre of its path goes down, yours too | TRUE | card "Tight ice, ten bodies. F clears the lane." (back every 20 s); R3:303 (bodies down 1.59x); `docs/VISION.md:34` (teammates on the path go down too) | kept |
| PG36 | Sudden Death: one goal ends it, against the top level `[waits: A133]` | CHANGED | card "One goal ends it. The meanest bots alive." (5 v 5 on 120 x 60 m); R3:305 (38 to 50 s matches) | tag gone; five a side |
| PG37 | Outnumbered: three against five, F is a shield | TRUE | card (F: shield, 3 s; a won puck is safe 1.5 s) | kept |
| PG38 | Glass House: every hit is a gamble, fouls cost ten seconds | CHANGED | card "The only whistle left. Foul and sit 10 s."; R3:298 | "the only whistle left: foul and sit ten seconds" |
| PG39 | Arsenal: grab a boost, get a trick on F | TRUE | card (each trick fires 3x); R3:294 | kept |
| PG40 | Bruisers: five a side, heavy bodies, no whistle | TRUE | card (5 v 5 on 70 x 35 m, 130 kg, first to 2) | kept |
| PG41 | Fog of War: night and fog, slip by, then hit `[waits: A139]` | CHANGED | card "Slow bots in the fog. Slip by, then hit." (bots 0.07 s late, 3 v 3, night, fog); R3:297 (breakaways 2.9x; kept at three a side on purpose, M6) | tag gone (landed); "slow-reacting bots" is the card's and is true |
| PG42 | the twelve by name, in the menu's order | TRUE | still 02 (order: Pond, Cage, Sniper, Marathon / Power Play, Shotgun, Sudden Death, Outnumbered / Glass House, Arsenal, Bruisers, Fog of War) | kept |
| PG43 | (removed Friday: "call a mode mid-match") | TRUE | still absent | |
| PG44 | DIRECT A CUT: without a key the nearest of the cuts already shot; with your own key a new one | TRUE | Friday's sources (`MenuWidget.cpp`, `JamDesignerSubsystem.cpp`), not re-checked in a window; TR "Not verified": a typed sentence without a key is unproven, the chip is the safe route | kept; "or pick one of the sentences on the screen" |
| PG45 | (removed Friday: four spot cones) | TRUE | still absent; N23 | still not claimed |
| PG46 | (removed Friday: the ink outline) | TRUE | still absent; N23 | still not claimed |
| PG47 | blue against orange | TRUE | stills 04, 09 | kept |
| PG48 | a booth talks over every hit; some hit lines are captions only `[waits: device B, BF3]` | CHANGED | B:H41.0 (412 of 412 line ids have a wave); PM:96 (whether you hear them all depends on B's audio at this commit: say if the booth is silent); captions are in every match still | "a booth with a caption for everything"; that it is heard: `[O16]` |
| PG49 | "No crowd in the stands yet" (a limit), `[waits: device B, BF2]` | FALSE | N11 | the stands and the crowd are on the page; the limit is deleted |
| PG50 | the rink answers: a lamp, the net shakes, a bench or a box rattles | TRUE | A155:32 (lamp 3.00 to 17.00, net shake 54 frames); PM:25; still 09; PM:94 (device B's goal effect did not show in the window check: not claimed) | kept; "the boards rock" |
| PG51 | Unreal Engine 5.9, C++, Blueprint, Niagara, MetaSounds | TRUE | unchanged | kept |
| PG52 | written and run by AI coding agents directed by the team | TRUE | `briefs/*`, `status/*` (lanes to A180) | kept |
| PG53 | the dev log: our side's turns and B's notes `[waits: device B, BS1]` | ASK | PM:119 (question 8): device B waits for his word on exporting its transcripts | `[O8]` |
| PG54 | AI-generated content declared, not claimed as sourced | TRUE | a policy; the ledger gate was last run on Friday afternoon's tree (`submission/README.md`, pass 2) | kept; `[O6]` covers the rerun |
| PG55 | "Original: the rink, the materials, the effects, every MetaSound, everything procedural" (O6) | ASK | the night added timber stands, a clay crowd, a roof and walls (B:H45.0, B:H46.6), fourteen MetaSounds, three figure sets: their ledger rows are unchecked | `[O6]`, wider than on Friday |
| PG56 | sourced: the engine's own content and one upscaling library | TRUE | `credits.md` (unchanged; same caveat as PG54) | kept |
| PG57 | team names and roles | ASK | the operator's | `[O1]` |
| PG58 | "Unzip `Hokej_Hokej.zip` `[waits: the Director's real package run]`" | CHANGED | N14 (landed) | tag gone; the zip's final name and number: `[O15]` |
| PG59 | Cancel on the firewall prompt: not tested | TRUE | still not tested: A155:46, :68, :75: no firewall prompt of ours appeared on this machine in three fresh unzips, so Cancel was never pressed; PM:4 repeats the claim | "we expect Cancel to be fine; not tested"; device B's clean-machine test settles it (`[O7]`) |
| PG60 | first launch compiles shaders for a few seconds | TRUE | A155:68, :75 (window after 1 to 5 s; a rendered match 8 s after Return) | kept |
| PG61 | the joined match and its limit | TRUE | N13 | kept, said exactly |
| PG62 | "The puck is yours until someone presses for it. **Nobody gets it by brushing past.**" | FALSE | the second sentence: N07 against N15: at rookie one in ten of a newcomer's carries goes to an opponent's bare touch (R3:106) | the rule, the yellow ring, and "carries are still short" among the limits |
| PG63 | two seconds after you get up a hit is a bump at most | TRUE | N08 | kept |
| PG64 | known limits | CHANGED | PM:77-96 | section "Known limits" rewritten: five of Friday's seven are gone or changed |

## 2. `submission/form.md`

| ID | Friday's row | Verdict | Source | Now |
|---|---|---|---|---|
| FM01 | team name, game name, track, theme | TRUE | unchanged | kept |
| FM02 | members: the operator's blank | ASK | the operator's | `[O1]` |
| FM03 | pitch = page A; blurb = page B (60 words) | CHANGED | both rewritten in this pass; B counted again | new A and B |
| FM04 | the zip `[waits: the Director's real package run]` | CHANGED | N14 | the chain is real; which package and its upload name: `[O15]` |
| FM05 | video: `hokej-v1.mp4` today; v2 does not exist | TRUE | `submission/video/cutlist.md` (v1 is the old build, three a side: no longer the game); TR part two (the edit is one command on device B, proved on stand-ins; nothing filmed) | the film is cut by device B's command from takes that do not exist yet: `submission/video/SHOTLIST.md`, `[O17]` |
| FM06 | dev log path; rebuilt Sunday; hosting blank | TRUE | unchanged | `[O8]` |
| FM07 | prizes: the operator's | ASK | no decision recorded | `[O4]` |
| FM08 | the tools list | ASK | unchanged | `[O5]` |
| FM09 | DLSS plugin, on by default on RTX GPUs | TRUE | Friday's `DLSS.md` (not re-checked this morning) | kept |
| FM10 | the model API: the key typed by the player, none bundled | TRUE | Friday's sources (not re-checked) | kept |
| FM11 | sourced assets: 62 rows, 1 shipped | CHANGED | unproven on this main: `credits.md` was generated on Friday afternoon's tree; the night merged device B's arena, crowd, sounds and sets | "regenerate `credits.md` and rerun the ledger gate on the tree that ships" (`[O6]`, Sunday) |
| FM12 | `docs/SUBMISSION.md` skeleton | TRUE | not touched by this pass (outside `submission/**`) | |

## 3. `submission/tagline.md`

| ID | Line | Verdict | Source | Note |
|---|---|---|---|---|
| TG01 | Hockey with the whistle taken out. | CHANGED | N05 (true now) | the banner; the game prints it on its own first screen |
| TG02 | Clay skaters. Real hits. | CHANGED | N11, N04 (true now) | the store line |
| TG03 | Pass it. Or get flattened. | TRUE | PM:24; R3:94 | |
| TG04 | Rewrite the rules, then try to survive them. | TRUE | PG04 | the theme line |
| TG05 | Five on five, and nobody is polite. | TRUE | N10: of the regular game, the season and six modes | |
| TG06 | The speed you bring is the hit you get. | TRUE | N04 | the law; R-0054 point 5's fallback, no longer needed as one |

## 4. The video (`video/script-v2.md`, `video/cutlist.md`)

| ID | Friday's row | Verdict | Source | Now (`video/SHOTLIST.md`) |
|---|---|---|---|---|
| VS01 | every shot on "the next package" | CHANGED | N14 | H30 Shipping (or the package named by `[O15]`); drills on its Development twin `Accept_H30` |
| VS02 | the same-direction shot: no drill stages it (O9) | CHANGED | answered: `HokejGameMode.cpp:165` (`jam.ctrl.Drill 22`: 0 = the human throws the nudge, 1 = the hit); A155:26-28; stills 06, 07 | drill 22, both halves; Development only (R-0054 point 4) |
| VS03 | head-on by drill 6 | CHANGED | the same | drill 22's second half, or by hand in Shipping (TR shot 5) |
| VS04 | the stop-and-go by `jam.ctrl.Drill 21` | TRUE | PM:56; still 05 (the coach line: STOP WHEN HE IS FIVE STICK LENGTHS AWAY) | kept; in Shipping by hand at the lit stop line |
| VS05 | the brake's spray `[waits: device B, BF1]` | CHANGED | B:H45.3, B:H45.9 (the stop's fan, about 3 % of the frame in B's own spawn); TR "the spray reads small"; no window still of H30 shows it | not promised in a caption |
| VS06 | Shotgun on F; the burst `[waits: device B, BF3]` | CHANGED | still 12 (the caption "One blast and nobody is left standing."); A155:34 (no visible blast in the stills; the HUD kept saying ARMED after the shot, on H27); B:H43.9 (no `ability` event fired in B's play) | must be recorded and looked at before it is promised |
| VS07 | Esc, TIME OUT, the Director's Call | TRUE | still 11 (the pause card); the call card on clay: no capture | must be recorded |
| VS08 | a goal: the lamp, the net; no horn claimed | TRUE | A155:32; still 09 | kept |
| VS09 | five a side at the faceoff | TRUE | still a171; still 09 | kept |
| VS10 | the bots pass | TRUE | PM:24 | kept |
| VS11 | the menu's stale "SCENE 3 v 3" line | CHANGED | gone: stills 01, 02: the slate reads SCENE ROUGH | |
| VS12 | the end card with device B's logo | CHANGED | exists: `devlog/shots/20261113-205819-B-br4-banner-1920.png` (TR shot 11) | generated in the edit from the banner |
| VS13 | 30 to 60 s, 1080p, the theme in the first seconds, no brand words | TRUE | TR part two: A45, A30, C45 pass at 1920 x 1080, 60 fps on stand-ins | |
| VS14 | audio from device B | CHANGED | a recipe, not a take: TR "Filming recipe": ffmpeg on B cannot record the game's sound; the system's own game recorder can; not run | `[O17]` |
| VS15 | `cutlist.md` is the record of `hokej-v1.mp4` | TRUE | unchanged | kept as history |

## 4b. `submission/package/README.txt` (the text file that ships beside the exe; not in Friday's table)

| ID | Its sentence this morning | Verdict | Source | Now |
|---|---|---|---|---|
| RT01 | "Then the menu. Pick a cut and play." | CHANGED | N01 (the first screen is PLAY NOW with the level switch) | PLAY NOW and the three levels |
| RT02 | "Play With A Friend" | CHANGED | still 01 (the button reads BRAWL WITH A FRIEND) | the menu's name |
| RT03 | "Solo play does not need it, so Cancel is fine." | FALSE | as a tested fact: PG59 (never pressed) | "Solo play does not need network access."; the Cancel claim returns only after device B's test |
| RT04 | "Space: poke the puck off a carrier" | CHANGED | N06, N16 | "jab for the puck (about one jab in ten takes it)" |
| RT05 | "Skaters going the same way bump" | CHANGED | PG11 | "beside a skater at his speed a check is a nudge" |
| RT06 | "you win by stopping and cutting" | FALSE | N18 | "Stop five stick lengths short of a defender, cut, go." |
| RT07 | "Five a side, and the other four pass. Once per period the Director can change one rule." | CHANGED | PG07, PG04 | "your mates pass"; "the Director's Call (Esc) moves one dial"; "No whistle (one mode, Glass House, keeps it)" added |
| RT08 | the gamepad line | CHANGED | PG28 | "The pad is mapped; no person has play-tested it yet." added |

The other lines in it (how to start, what you need, the controls list, the credits paragraph) were re-read and stand. The eight takes of the season are all five a side (`ladder/take-1..8.cartridge.json`, five skaters a team), which is what TG05 rests on.

## 5. Counts (this pass)

Friday's 96 rows re-read, plus TG06: by the verdict column of sections 1 to 4 (the `awk` line is in `status/A180.md`).
The new facts of section 0: 14 that must be findable (N01 to N14), 9 that must not be said (N15 to N23).

| | TRUE | CHANGED | FALSE | ASK |
|---|---|---|---|---|
| page (PG01 to PG64) | 38 | 18 | 5 | 3 |
| form (FM01 to FM12) | 6 | 3 | 0 | 3 |
| tagline (TG01 to TG06) | 4 | 2 | 0 | 0 |
| video (VS01 to VS15) | 7 | 8 | 0 | 0 |
| the zip's README.txt (RT01 to RT08, new) | 0 | 6 | 2 | 0 |

After this pass `page.md`, `form.md` and `tagline.md` carry no `[waits: ...]` tag and no sentence marked FALSE above; what is open is a slot `[O<n>]`.
Find the slots with `grep -n "[[]O[0-9]" submission/page.md submission/form.md submission/tagline.md` (a slot is `[O<n>: ...]`, with the colon).
