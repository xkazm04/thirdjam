# The truth table: every claim the submission makes, against the repository (lane A142, track D1, Fri 13 Nov 2026 night, main 41df3e6)

How to read it. **Now** = the status of the OLD sentence (page v2 of lane A96, form, tagline, video script v2) against main `41df3e6`: `TRUE`, `FALSE`, `UNPROVEN` (the repository does
not say, or says it only for a lane's own tree). **After** = the status of the corrected sentence now in the files: `TRUE` (true of main today), `WAITS <lane>` (true only
when that lane lands: the sentence carries the same tag in the file, as `[waits: ...]`), `ASK` (the sentence is a marked blank or a question for the operator).
Sources are files in this repository; `R-00NN` is `contracts/RULINGS.md`. Nobody ran a game for this table: it is reading, so "TRUE" means "the cartridge, the code or a
measured document says so", never "I saw it". The counts are at the end (section 5).

**What is on main today and what is not (EVOLUTION.md board, R-0050 to R-0053, the briefs of A132 to A139):**
on main: contact by momentum (A92), the movement laws (A93), the first brain (A94), the ladder of R-0046, the kept puck (A102: `stick.passive_take` false, `body_shield` true), the
release grace 120 and the take grace 30 (A113, A121), the harder stop and the earned first step, drill 21, five a side in `hockey.regular`, twelve mode cards, abilities on F, the
joined match (protocol 8), clay goal frames, benches, penalty boxes and the arena that answers a goal (A103, A119, A127), the acceptance matrix.
**Not on main (in a lane tonight):** the whistle off in the regular cut and the takes, take-3 at rookie, the re-set ladder (A112's profile, A133's curve); the brain's second
evolution and the shove rule (A138, the integration lane, parked as `wip-batch6`); the hit's size on screen (A132); the camera that never loses the human and the brake prompt
(A135); the modes' second evolution (A139); the brain's third (A134); more arena (A136); the first 60 frames (A137). **No package is cut until the curve and the integration
lane land (R-0053): the newest package, H26, is older than main.**

## 1. `submission/page.md` (old v2)

| ID | Old claim | Now | Source and finding | After | Corrected sentence (or what happened) |
|---|---|---|---|---|---|
| PG01 | "Hockey with the whistle taken out" (pitch, banner) | FALSE | `hockey.regular` and all twelve takes have `rules.penalties` true; R-0048 point 2 takes the whistle out; REVIEW-1 section 4 | WAITS A133 | kept as the line, tagged `[waits: A133]`; if A133 does not land, the page's fallback strapline (PG10) already stands without it |
| PG02 | "clay skaters" (pitch, tagline, look paragraph) | UNPROVEN | `hockey.regular` look tier is `l2_night`; six modes set `p3_clay`, others `l1_day`, `l3_neon`, `l2_night` (`modes/MODES.md`); R-0041 point 5 calls P3 clay the shipped look; no still of the package shows which one a newcomer meets first | ASK | the page says "clay figures" (the figures are clay in every tier) and does not promise a rink look; O3 |
| PG03 | "a mouse for a stick" | TRUE | `docs/experiments/CONTROLS.md`; `InputMapping.cpp:107`; acceptance run 19 of 19 through the cursor (R-0046) | TRUE | kept |
| PG04 | "a Director who rewrites the rules mid-game" | UNPROVEN | `DirectorsCallWidget.cpp:220`: "One change per period. Move one slider, hit APPLY": the call moves one of up to five tunables of the cut | TRUE | "Once a period the Director's Call lets you move one dial: ice, boards, bot speed, shot speed" (regular lists nine tunables, five are shown) |
| PG05 | "Hits obey momentum" | TRUE | R-0046; `Jam.Core.Contact.SameDirection`; REVIEW-1 section 4: "true, half shown" | TRUE | kept; the limits say the screen shows a knockdown better than a bump (F1/A132) |
| PG06 | "the carrier wins by stopping and cutting, not hiding" | UNPROVEN | the drill: `Jam.Core.Movement.StopAndGo`, bar 80 % at pro (R-0053), `MOVEMENT-3.md`; REVIEW-1 section 4 called it "not yet true" in a match before the puck was kept | TRUE | "Brake with S, cut, go: in the drill a standing pro defender is beaten (the gate's bar is 8 in 10)." No match-level number on the page |
| PG07 | "Five a side, and the bots pass" | TRUE | `hockey.regular.cartridge.json` teams (5 and 5); R-0046; `BRAIN.md` | TRUE | kept |
| PG08 | "Nobody is polite" | n/a | brand voice, no fact | TRUE | kept |
| PG09 | "Once a period the Director rewrites one rule mid-match: a shotgun on F, or a pond with no hits" | FALSE | the call moves a slider (PG04); the shotgun and the pond are cuts picked in CHOOSE YOUR CUT (`MenuFlow.cpp:280`) | TRUE | blurb rewritten: cuts are picked in the menu; the call moves one dial |
| PG10 | "Clay skaters. Real hits." (the page's strapline) | UNPROVEN | hits: PG05; clay: PG02 | TRUE | the strapline is "Real hits. The speed you bring is the hit you get." (REVIEW-1's line, minus the clay) |
| PG11 | "Two skaters gliding the same way just bump" | TRUE | `Jam.Core.Contact.SameDirection`; ratio 0.35 from behind, 0.28 from the side (REVIEW-1 section 4) | TRUE | kept |
| PG12 | "head-on at full speed is the biggest hit, and the one who throws it pays" | TRUE | VISION law 1; R-0045; REVIEW-1: ratio 4.61, down 150 frames | TRUE | kept |
| PG13 | "with the puck you cannot check" | TRUE | VISION law 1 "the carrier cannot hit"; A92 on main (R-0046) | TRUE | kept |
| PG14 | "hit S against your own motion to brake hard" | TRUE | VISION law 3; the hint bar does not list it (REVIEW-1 item 8, `HUDWidget.cpp:349`) | WAITS A135 | verb kept; "the screen tells you" is tagged: A135 item 4 (the brake prompt) |
| PG15 | "then burst away" (first-step burst) | TRUE | VISION law 3; `MOVEMENT-3.md` | TRUE | kept |
| PG16 | "a defender at full sprint is always faster than you in a straight line" | TRUE | VISION law 4 (carrier +8 % on 0.92, defender +15 %); R-0046 | TRUE | kept |
| PG17 | bots "carry when the lane is clear, pass when you close in, have stopped charging a wall of defenders" | TRUE | A94 on main, `BRAIN.md`; REVIEW-1: 8.0 passes a minute at pro | TRUE | kept, shorter; the brain that brakes on purpose, sprints with the puck and reads the shield is `[waits: A138]` |
| PG18 | "Difficulty is aggression first: rookies barely touch you, all-stars hunt whoever has the puck **and outskate you to it**" | FALSE | R-0046 point 3: all_star skates at 1.4 (1.22 on the large rink), aggression alone left the good player near 37 %; REVIEW-1 section 4 names the line false. The ladder is also off until re-set (R-0050: 99.8 / 72.0 / 39.8 against 85 / 48 / 26) | WAITS A133 | "Rookie, pro, all-star: the harder the level, the more it hits; at the top it also skates faster. The ladder is being re-set tonight." `[waits: A133]`; no win rate on the page |
| PG19 | abilities on F: "shotgun, hip-check burst, shield, slow puck" | TRUE | `ABILITIES.md`; Shotgun (mode), Outnumbered (shield, mode), Arsenal (all four, by pickup) cartridges | TRUE | kept, with where each comes from |
| PG20 | "or turn the rink into a pond where nobody hits" (as a call) | FALSE | see PG09; Pond is a cut: `skater.check_enabled` false | TRUE | Pond is a cut in the menu |
| PG21 | "Skate, aim with the mouse, hold left button to charge, let go to fire" | TRUE | `InputMapping.cpp:95`; CONTROLS.md | TRUE | kept |
| PG22 | "Poke the puck off a carrier, sweep it, spin, deke, pass (tap Q flat, hold for a saucer)" | UNPROVEN | verbs exist (CONTROLS.md); REVIEW-1 item 5: a poke takes the puck 1 time in 10 | TRUE | "Space pokes (it rarely wins alone); the wheel dekes; E spins; Q passes, held it is a saucer" |
| PG23 | "Body-check a carrier into the boards" | TRUE | A73, `TACKLING-3.md`; board event `JamSim.cpp:1562` | TRUE | kept |
| PG24 | "Cross-check a skater at arm's length, face to face" | FALSE | CONTROLS.md: the cross-check is a stick lift that strips an opposing carrier within 1.2 m beyond the bodies, from any side; REVIEW-1 section 4: "cross-check a skater who is not looking" is false (only the carrier is a target); the shove rule is A138 | WAITS A138 | "Middle mouse: lift the puck off the carrier you are on top of." The shove under a shield is `[waits: A138]` and not promised |
| PG25 | "watch the clay figure go down, slide, and get up angry" | TRUE | A73 stills (`img/a73`), cutlist row 5; knockdown 60 to 150 frames (VISION law 1) | TRUE | "go down, slide, get up" |
| PG26 | "Take the season (a ladder of rivals)" | UNPROVEN | `LADDER.md`: eight takes, one twist each; no "rivals" | TRUE | "the season: eight takes, one new twist each, rookie up to all-star" (take 3 at rookie is `[waits: A133]`, R-0048 point 3) |
| PG27 | controls table, keyboard and mouse | TRUE | `InputMapping.cpp:79-127`, CONTROLS.md section 1: every row matches | TRUE | kept, one row added: team signals 1 to 4 (CONTROLS.md, `JOIN-GUIDE.md`) |
| PG28 | controls table, pad column | UNPROVEN | bound in the same file; REVIEW-1 section 7: "the gamepad" not judged at all | TRUE | pad column kept; limits line: "the pad is mapped, not yet play-tested by a person" |
| PG29 | "Pause, Time Out, Director's Call: Esc" | TRUE | `DirectorsCallWidget.cpp:173,185` | TRUE | kept |
| PG30 | Pond: "nobody gets hit" | TRUE | `modes/MODES.md`: `skater.check_enabled` false | TRUE | kept |
| PG31 | Cage: "a tight box, live boards, hit or get hit" | TRUE | 60 x 28, restitution 0.95, 4 v 4, `contact.board_bonus` 2 | TRUE | "eight bodies, boards that hit back" |
| PG32 | Sniper: "a big net and a low guard, lift it" | TRUE | `rink.goal_height_m` 1.22, goalie block 0.15 | TRUE | kept |
| PG33 | Marathon: "**ten** minutes of endless ice" | FALSE | timer 28800 frames = 8:00; 140 x 70 (`modes/MODES.md`, A122) | TRUE | "eight minutes on long ice" |
| PG34 | Power Play: "four on three" | TRUE | blue 4 v orange 3 | TRUE | kept |
| PG35 | Shotgun: "F fires a shot that drops the crowd" | TRUE | `ABILITIES.md`: 3 of 3 down, puck through; teammates in the path go down too; cooldown 1200 frames (20 s) | TRUE | "F: one blast, and every skater within a metre of its path goes down. Yours too." |
| PG36 | Sudden Death: "one goal ends it, the meanest bots alive" | UNPROVEN | `first_to` 1, level `all_star`; the level is re-set by A133 | WAITS A133 | "one goal ends it, against the top level" `[waits: A133]` for how hard that is |
| PG37 | Outnumbered: "three against five, get hit, hit back" | TRUE | blue 3 v orange 5, shield on F (3 s every 10 s, A122) | TRUE | kept, F added |
| PG38 | Glass House: "every hit is a gamble, fouls cost 10 s" | TRUE | `rules.penalty_frames` 600, whistle on | TRUE | kept |
| PG39 | Arsenal: "grab a boost, get a trick on F" | TRUE | pickups speed and precision; abilities all `pickup`; MODES.md's "(no card yet)" is stale, the menu lists it | TRUE | kept |
| PG40 | Bruisers: "five a side, no whistle, hit everything" | TRUE | 5 v 5, 80 x 40, mass 130, penalties false | TRUE | "heavy bodies"; "hit everything" cut (the carrier cannot be hit) |
| PG41 | Fog of War: "slow bots in the fog" | UNPROVEN | A122 cut `ai.react_frames` 12 to 6 because 12 won 96.5 %: the bots are no longer slow | WAITS A139 | "night and fog: slip by, then hit"; "slow" removed |
| PG42 | the twelve modes listed by name | TRUE | `UJamMenuFlow::ModeCards` order; `Jam.Menu.DirectorsChoices`; Blind Side and Pinball parked | TRUE | twelve, in the menu's order |
| PG43 | "or call one [mode] mid-match" | FALSE | see PG09 | TRUE | removed |
| PG44 | "Direct a Cut takes one sentence and hands it to the Designer, which turns it into a cut" | UNPROVEN | `MenuWidget.cpp:489`: with no API key the Designer answers from the cuts already shot (nearest sentence); live needs a key typed under SETTINGS; the shipped build has the CLI transport off (`JamDesignerSubsystem.cpp:371`) | TRUE | both halves said: offline it picks the nearest of the cuts we shot; with your own key it writes a new one |
| PG45 | look: "four spot cones on a night rink" | UNPROVEN | `LookTierSwitcher.cpp:317` "four shadowed spots" (clay tier); the regular cut is the night tier | ASK | removed; O3 |
| PG46 | look: "a thick ink outline that thins with distance" | UNPROVEN | no document measures it for the packaged look | ASK | removed; ask B for a still at 3 m and 30 m (O3) |
| PG47 | "blue against orange" | TRUE | cartridge team colours | TRUE | kept |
| PG48 | "a commentator who brags when you hit and winces when you get hit" | UNPROVEN | the bank follows `VOICE.md` rule 4; R-0051: 72 of 412 lines have no recording, most of them the contact lines: the caption shows, the voice does not | WAITS device B (BF3) | "a booth that talks over every hit; some hit lines are captions only until B records them" |
| PG49 | "a crowd that never forgets" | FALSE | REVIEW-1 item 10: "the hall is empty"; the crowd band is device B's BF2, not on main | WAITS device B (BF2) | removed; replaced by what exists: benches, penalty boxes, clay goal frames, a lamp and a net that answer a goal |
| PG50 | (new) the arena answers a goal | TRUE | `ARENA.md` sections 19 to 28; `Jam.Rink.*` 3 of 3; lamp 3.0 to 17.0 on the goal's frame, net shake 54 frames; `jam.rink.react` | TRUE | one sentence |
| PG51 | "Unreal Engine 5.9 (C++ gameplay, Blueprint visuals, Niagara, MetaSounds)" | TRUE | `status/A.md`; R-0049 point 5 (`NS_deke`, `MS_deke`) | TRUE | kept |
| PG52 | "the code, tests, gate and dev log were written and run by AI coding agents directed by the team" | TRUE | `briefs/*`, `status/*`, `submission/README.md` item 7 | TRUE | kept |
| PG53 | "the dev log shows every turn, hour by hour" | UNPROVEN | A96: 490 commits, 123 sessions, 11,999 turns, entries thin before H4; **device B's transcripts: none found** (B has 30 notes) | WAITS device B (BS1) | "the turns of our side's sessions, and B's notes"; counts rebuilt Sunday |
| PG54 | AI-generated content declared, not claimed as sourced | TRUE | `credits.md` (62 sourced rows, 1 shipped); ledger gate PASS, 1,654 rows (A96) | TRUE | kept |
| PG55 | "Original: the rink, the materials, the effects, every MetaSound, everything procedural" | UNPROVEN | ledger origin per row; the arena pieces added since A96 (benches, boxes, goal frames, goal light, table) are A's Blender work (`ARENA.md`) but their ledger rows are unchecked by this lane | ASK | O6: run `node tools/gate/check-assets-ledger.mjs` on the final tree |
| PG56 | "Sourced: the engine's own content and one shipped graphics-upscaling library" | TRUE | `credits.md`, `DLSS.md` (56.2 MB dll) | TRUE | kept |
| PG57 | TEAM: names and roles | n/a | the operator's | ASK | a marked blank |
| PG58 | "Unzip Hokej_Hokej.zip, run ThirdJam.exe; Windows 10 or 11, DirectX 12" | UNPROVEN | script ready, not run for real (`submission/README.md` item 1); clean-profile PASS on H24 only | WAITS Director (the real package run) | kept, tagged |
| PG59 | "Cancel is fine on the firewall prompt for solo play" | UNPROVEN | `submission/README.md` item 9: "only claimed, never tested"; the game opens a socket at start | ASK | said as "not tested"; O7 |
| PG60 | "First launch compiles shaders for a few seconds" | TRUE | clean-profile test (A88): the menu 5 s after launch | TRUE | kept |
| PG61 | (new) a joined match for two machines | UNPROVEN | `JOIN-GUIDE.md`: same network, UDP 7700, protocol 8; two processes on one machine to the end of a match and a rematch (A129); 1,000 soak matches, 0 desync; two physical machines, a real firewall prompt, Wi-Fi loss: not tested | TRUE | written with exactly that limit |
| PG62 | (new) "the puck is yours until someone presses for it" | TRUE | R-0048 point 1, R-0050 point 1: `stick.passive_take` false by default (`JamCartridge.h:154`), `body_shield` true, take grace 30; REVIEW-1 measured the old build's 0.75 s carry; the new carry is in no document | TRUE | no number; the list of what takes it: a shot, a pass, a poke, a lift, a check |
| PG63 | (new) "two seconds after you get up, a hit is a bump" (release grace) | TRUE | R-0051 point 1: 120 frames | TRUE | one line |
| PG64 | (new) known limits | n/a | REVIEW-1 items 7, 8, 10; R-0050 (ladder off), R-0051 (silent lines) | TRUE | listed on the page |

## 2. `submission/form.md` (old)

| ID | Old claim | Now | Source and finding | After | Corrected |
|---|---|---|---|---|---|
| FM01 | team name, game name, track, theme | TRUE | `docs/SUBMISSION.md`, `docs/THEME.md` | TRUE | kept |
| FM02 | Members `TEAM: <names>`; "the older skeleton says 1" | FALSE (the skeleton) | `docs/SUBMISSION.md` row Members says 1, stale: two devices | ASK | skeleton fixed to the blank; O1 |
| FM03 | Pitch = `page.md` A; blurb = B ("exactly 60 words") | FALSE | the old B named a call that is not (PG09) | TRUE | new A and B; B counted at 60 words |
| FM04 | zip `Hokej_Hokej.zip`, top folder `Windows/`, start `ThirdJam.exe`, `README.txt` beside | UNPROVEN | `submission/README.md` item 1 | WAITS Director | kept, tagged |
| FM05 | video `submission/video/hokej-v2.mp4` | FALSE | the file does not exist; only `hokej-v1.mp4` | WAITS capture lane | the form says v1 until v2 is filmed |
| FM06 | dev log `submission/devlog/index.html`, 60 curated pictures | TRUE | the file exists (A96: 7.8 MB) | TRUE | hosting link a blank (O8) |
| FM07 | main prize and modifier "Director decides"; the log "the natural claim" | UNPROVEN | no decision recorded (`submission/README.md` item 8) | ASK | O4 |
| FM08 | the tools list (engine, agents, image, 3D, voice, upscaler, Blender, ffmpeg, the model API) | UNPROVEN | `credits.md`; `docs/SUBMISSION.md` says list Rodin only if the ledger shows a shipped Rodin asset (the `P4/Rodin` rows in `credits.md` are Epic clips, `shipped` n); P4 descoped | ASK | list left as A96 drafted it; O5 |
| FM09 | DLSS 4.5 plugin v8.9.0, on by default on RTX GPUs | TRUE | `DLSS.md` | TRUE | kept |
| FM10 | the model API: key typed by the player, none bundled, CLI transport off in the submitted build | TRUE | `JamDesignerSubsystem.cpp:371,745`; `MenuWidget.cpp:489` | TRUE | kept |
| FM11 | sourced assets: 62 rows, 1 shipped | TRUE | `credits.md` | TRUE | kept; ledger check rerun Sunday |
| FM12 | `docs/SUBMISSION.md`: pitch "Hockey 3v3"; "Members 1"; tools "Codex image generation: reference images only"; "Meshy: generated skater figure" | FALSE | stale since H4.7 | TRUE | the skeleton points to `form.md` and carries the corrected pitch |

## 3. `submission/tagline.md` (the factual taglines)

| ID | Claim | Now | Source | After | Note |
|---|---|---|---|---|---|
| TG01 | "Hockey with the whistle taken out." | FALSE | PG01 | WAITS A133 | banner line, tagged |
| TG02 | "Clay skaters. Real hits." | UNPROVEN | PG02, PG05 | ASK | clay: O3 |
| TG03 | "Pass it. Or get flattened." | TRUE | the bots pass; the knockdown | TRUE | |
| TG04 | "Rewrite the rules, then try to survive them." | UNPROVEN | the call moves one dial; twelve cuts | TRUE | stays as the theme line, with the dial said in the page |
| TG05 | "Five on five, and nobody is polite." | TRUE | PG07 | TRUE | |

## 4. `submission/video/script-v2.md` (old) and `cutlist.md`

| ID | Old claim | Now | Source and finding | After | Corrected |
|---|---|---|---|---|---|
| VS01 | shots 5 to 8 "need H26" | FALSE | H26 predates the kept puck, the graces, drill 21 and the arena; R-0053: no package until the curve and I1 land | WAITS Director (the next package) | "the next package" |
| VS02 | shot 5 "same direction": drill 5 (a check from the side at 3 to 6 m/s) | FALSE | REVIEW-1 section 4: at 3.5 m/s closing drill 5 is a stumble (0.90), not a bump; only the test `Jam.Core.Contact.SameDirection` stages the scene; no console drill does | WAITS F1/F2 (a drill that stages it) | staged by hand or by a new drill; question O9 |
| VS03 | shot 6 head-on: drill 6, human carries into a standing pro bot and is flattened | UNPROVEN | A73: "32 knockdowns in 140 s" (before the grace, the shield, the contact law); "Crunching check!" is in `Game/Plugins/JamAssets/Content/Audio/VO/commentary.json` (B's; recording unknown, R-0051) | TRUE | `a73-prove.ps1 -Drill 6`, re-run on the next package; sound is B's |
| VS04 | shot 7: "no drill stages this; write drill 21" | FALSE | drill 21 is on main: `jam.ctrl.Drill 21` with `jam.ctrl.CarryScript 1` (stop-and-go from 9.5 m, cut 60 degrees), 0 straight charge, 2 your own hands, 3 and 4 the late answers (`HokejGameMode.cpp:73-82`; A102, A121) | TRUE | staged with those commands |
| VS05 | shot 7 "the brake's spray" | UNPROVEN | the spray fires only on a lean of 25 degrees or more; no brake sound is bound (REVIEW-1 item 8); B's BF1 owns the pose and spray | WAITS device B (BF1) | caption kept; the shot shows what is on screen |
| VS06 | shot 8: Shotgun, `a84-prove.ps1`, bark "One blast and nobody is left standing." | TRUE | `tools/qa/ctrl/a84-prove.ps1`; the line is in `Game/Content/Commentary/abilities.commentary.json`; the burst `NS_ability` was missing | WAITS device B (BF3) | staged as before; effect and sound are B's |
| VS07 | shot 9: Esc, TIME OUT, the call's sliders | TRUE | `DirectorsCallWidget.cpp:173,217,220` | TRUE | the clipped hint line (A88) is U1's |
| VS08 | shot 10: a goal by the human, the horn, "Blue scores!" | UNPROVEN | `a91-prove.ps1 -Drill 10`; A91: all three charges scored; no document says a horn is bound | TRUE | a goal, the lamp and the net answering (`ARENA.md` section 20); the horn not claimed |
| VS09 | shot 3: five a side on Bruisers; "H24 spawned three, re-check" | UNPROVEN | cartridge 5 v 5; the package check was never repeated | TRUE | regular is five a side on main; the take is counted by the capture lane |
| VS10 | shot 4: bots pass; find the window by `JAMEVENT pass` | TRUE | `contracts/events.md`; A94 | TRUE | kept |
| VS11 | shot 2: twelve cards; "the menu's SCENE 3 v 3 line fixed" | UNPROVEN | A88 finding; no document says it was fixed | WAITS capture lane | checked in the take |
| VS12 | shot 11: end card with B's logo | UNPROVEN | A-to-B-0019 | WAITS device B | generated card until B's logo arrives |
| VS13 | 48 s, 1080p, 30 fps, theme in the first seconds, no brand words | TRUE | rules digest `docs/PLAN.md` section 2 | TRUE | shot 1's caption now carries "Director" |
| VS14 | audio from device B | UNPROVEN | A-to-B-0020 point 5 open; no capture device on this machine | WAITS device B (BS1) | open question, fallback written |
| VS15 | `cutlist.md` (v1): nine shots of the H24 build and one older Development take | TRUE (as a record) | measured account: H24, three a side, no goal by the human | TRUE | header added: the record of the fallback video, not of main |

## 5. Counts

Counted by script from the rows above (the `awk` line is in `status/A142.md`). 96 rows: 93 with a status and 3 `n/a` (brand voice, the team blank, the limits list).

| | TRUE | FALSE | UNPROVEN | n/a |
|---|---|---|---|---|
| **Before** (the old sentences) | 47 | 16 | 30 | 3 |
| **After** (the corrected sentences) | 66 true of main today (the 3 n/a included) | 0 | 0 | |
| of the 96 after: waiting for a lane (`WAITS`, tagged in the file) | 20 | | | |
| of the 96 after: a marked blank or question for the operator (`ASK`) | 10 | | | |

Each `WAITS` sentence is true of the package that ships only if its lane lands; if it does not, the tagged sentence is deleted (the list is in `submission/README.md`, section "If a lane does not land").
