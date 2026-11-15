# The truth table, pass 6: the evening's changes against the game as it will be submitted (lane A225, track D1, Sun 15 Nov 2026 20:48 to 21:25)

**What this is.** Only the claims that the evening made false, or that can now be checked. The game read: the candidate **H45** (main `c7057ed0`, with DLSS: `docs/PLAY-ME.md` of H45,
"New in H45" and "Known limits") plus lane A235's fixes on main `bcbd1287` (`status/A235.md`: no effect at any contact, no slide streak, no man-down arrow, the prompt waits for the card, no
faceoff cue while boxed, a cooked game never takes the CLI route). The best evidence: lane A234's walk of the cooked candidate (`status/A234.md`, its table; `submission/shots/H45-PICKS.md`).
No game, no build, no git, no web. Pass 5 (below) stays as written; a pass-5 row named here takes the pass-6 verdict.
Verdicts as in pass 5. `walk` = `status/A234.md` (main checkout), its row or SEEN line.

## 6.1 `submission/page.md`

| ID | Claim | Verdict | Source | Now |
|---|---|---|---|---|
| P6-PG01 | Arsenal, Cage and Marathon play on the neon arena (four neon modes): the page's mode lines are the cards (they name no look); "Glow Ice: neon ice"; the rink: "Later takes and some modes change the scene: neon, night, fog" | TRUE | walk: MODES and the Arsenal, Cage, Marathon rows ("neon, lime centre", "red-pink centre, four rimmed drums", "cyan centre"); Pond and Outnumbered in daylight | kept |
| P6-PG02 | how it was made: "concept and menu images from an image-generation model" | FALSE NOW | the fourteen MODES illustrations (generated, not screenshots): ledger rows `T_UI_ModeArt_*`, walk MODES row | "concept, menu and mode pictures from an image-generation model" |
| P6-PG03 | "A panel at the bottom left lists the keys all match"; "Nothing else is drawn on the ice: no line ahead of you, no mark at a hit" | TRUE (of the final source) | walk (H45): the panel; no bar under his ring (the SPRINT row); nothing at a landed hit on clay and night; its W10 (slide streak), W13 (balls at a contact), W15 (man-down arrow) are cured on bcbd1287 (A235 jobs 1 to 3, each with a before / after film) | kept |
| P6-PG04 | Glow Ice: the only whistle, foul and sit ten seconds | TRUE | walk: "whistle, box, IN THE BOX n"; A235 W7 (no faceoff cue while boxed) | kept |
| P6-PG05 | limit: "On Glow Ice the camera can jump to the ice beside a boost beacon after your cross-check lands" | FALSE NOW | not in H45's known limits; the camera limit of H45 is the side boards (PLAY-ME; walk W8: seven of fourteen runs) | "At the side boards the camera goes steep and close, and you can lose the rink." |
| P6-PG06 | (added) a carried puck inside another skater for a few frames after a steal | TRUE | PLAY-ME H45 known limits (hardening: 50 events in 2,400 matches) | one limit line added |
| P6-PG07 | (added) orange's goal burst on the neon ice reads as smoke | TRUE | PLAY-ME H45 known limits | one limit line added |
| P6-PG08 | limits header: "true of package H43" | FALSE NOW | the candidate is H45 | "true of the candidate H45" |
| P6-PG09 | How to run: "about 271 MiB" | FALSE NOW | `Hokej_Hokej_H45.zip`: 296,480,940 bytes (283 MiB), 432 MB unpacked | "about 283 MiB" (the Director writes the final count) |
| P6-PG10 | MAKE YOUR OWN: "or you pick one of the sentences on the screen" (P5-PG49) | TRUE | walk: "list of saved sentences"; A235 W3 (four shown, none cut) | kept |
| P6-PG11 | "Score, and a red lamp lights over the net" (P5-PG57) | TRUE | walk: "GOAL banner with the lamp lit"; `h45-04-goal-banner-lamp.png`. "the net shakes", "the boards rock": still CANNOT | kept |
| P6-PG12 | "The two games stay identical frame by frame"; "a thousand soak matches never drifted" (P5-PG54) | TRUE | PLAY-ME H45 "Hardening": 44 lockstep matches with loss and jitter, both seats human, 0 desync; 10,700 soak matches, 0 faults. "the bots on your side plan with you": still CANNOT | kept |
| P6-PG13 | the twelve mode lines against each mode's walked match | 11 TRUE, 1 CANNOT | walk table: Pond no knockdown in a whole match; Sudden Death ends at the first goal; Power Play five against three; Outnumbered three against five, the shield; Sniper five against four, the tall net; Marathon 140 x 70 m, 6:00; Cage four a side, four drums; Bruisers first to 2; Fog of War night and haze; Arsenal boosts, F tricks; Glow Ice the whistle. Shotgun's blast: the hand never pressed F (still CANNOT, P5-PG44) | kept |
| P6-PG14 | the booth: a line for most things, shown as captions | TRUE | walk: booth lines on every picked frame ("In! Blue scores!", "Gravity wins again.") | kept; heard: `[O16]` |
| P6-PG15 | MAKE YOUR OWN: "With your own key (typed under SETTINGS, never bundled with the game)" | TRUE | A235 job 6: a cooked game never takes the CLI route; the key is the typed one, or the environment variable `ANTHROPIC_API_KEY` (JamDesignerSubsystem.cpp:359), else the saved games; walk: "No API key: the Designer answers from the games it already made" | kept (the page does not claim the key is only typed) |
| P6-PG16 | Windows 10 or 11 (DirectX 12) only, no web build | TRUE | A232 NO-GO (R-0074) | kept |
| P6-PG17 | first launch compiles shaders a few seconds; Cancel on the firewall | CANNOT | the walk ran a Development copy off screen; HOST not pressed; no clean-machine test | unchanged |

## 6.2 `submission/form.md`

| ID | Claim | Verdict | Source | Now |
|---|---|---|---|---|
| P6-FM01 | zip "about 271 MiB zipped, 421 MB unpacked (H41)" | FALSE NOW | H45: 296,480,940 bytes, 432 MB | "about 283 MiB, 432 MB (H45)" |
| P6-FM02 | OpenAI Awards: "IF LANDED" fourteen MODES illustrations; "62 ... 50 of them ship" | FALSE NOW | landed on main c7057ed0; `ASSETS.md` at bcbd1287: 76 rows by Codex image generation or a Codex concept sheet, 64 shipped, 14 of them `T_UI_ModeArt_*` (all shipped); walk: each illustration whole, three on the neon arena that those modes play | the mark removed, a bullet "The MODES screen (14)", counts 76 / 64, the proof paragraph rewritten |
| P6-FM03 | tools: Codex "menu and HUD art, in-game textures, the concept sheets" | FALSE NOW (incomplete) | as P6-FM02 | "... the fourteen MODES illustrations ..."; proof "Codex rows 76 (64 shipped)" |
| P6-FM04 | NVIDIA: "the CREDITS screen does not name DLSS"; "Open: ... has none" | FALSE NOW | `MenuWidget.cpp:1207` "Uses NVIDIA DLSS technology, (c) NVIDIA Corporation."; walk CREDITS row: "the DLSS line" | "names NVIDIA DLSS as text since H45"; "Closed" |
| P6-FM05 | Windows 10 or 11 (DirectX 12) only; no web build; sourced: four fonts and the DLSS library | TRUE | H45 carries DLSS (PLAY-ME) | kept |
| P6-FM06 | the operator's fields: names and e-mails, the video link, the dev log link, the OpenAI model name, the Solo Driver tick | ASK | `status/A225.md` top | |

## 6.3 `submission/package/README.txt` (ships inside the final package)

| ID | Line | Verdict | Source | Now |
|---|---|---|---|---|
| P6-RT01 | "Windows 10 or 11, 64 bit. A graphics card that does DirectX 12." | FALSE NOW (asked: "only") | R-0074, A232 | "Windows 10 or 11 (DirectX 12) only, 64 bit, ..." |
| P6-RT02 | "Under 450 MB of disk when unpacked" | TRUE | H45: 432 MB | kept |
| P6-RT03 | "The same keys show in a panel at the bottom left all match" | FALSE NOW | the panel shows the keys for now: with the puck SHOOT, PASS, DEKE, SPIN; without CROSS-CHECK, CHECK, POKE; always SKATE, SPRINT, BRAKE (PLAY-ME) | "A panel at the bottom left shows the keys you need now, with or without the puck, all match." |
| P6-RT04 | Shift: sprint | FALSE NOW (incomplete) | the sprint bar left the ice: the panel's SPRINT row dims when the legs are spent (PLAY-ME New in H45) | "sprint, while the legs last (the panel's SPRINT row dims when they are spent)" |
| P6-RT05 | the credits paragraph names Unreal Engine and no DLSS | FALSE NOW | DLSS ships in H45; its licence asks for a text credit (DLSS.md 1.4) | "On RTX graphics cards it uses NVIDIA DLSS technology (upscaling, on by default)." (text, no logo) |
| P6-RT06 | the controls rows and the pad line | TRUE | PLAY-ME H45 controls table (generated from the live bindings): every key and pad button matches | kept |
| P6-RT07 | keys for MAKE YOUR OWN | TRUE (silent) | the README does not talk about keys, so it carries no key line | nothing added |
| P6-RT08 | "Solo play does not need network access" | CANNOT | the clean-machine test has not run (`[O7]`) | kept |

## 6.4 `tagline.md`, the video

| ID | Claim | Verdict | Source | Now |
|---|---|---|---|---|
| P6-TG01 | "Six taglines, each true of package H43"; the banner "true of package H43" | FALSE NOW (stale package) | the six lines read again against H45: true (P5 rows; walk) | "the candidate H45" |
| P6-VS01 | SHOTLIST row 1: "No still of HOME of H43" | FALSE NOW | `submission/shots/h45-01-home-rookie-lit.png` | CHANGED (pass 6) |
| P6-VS02 | row 2: "the chosen mode's picture"; stills `modes/01..09` | FALSE NOW | the pictures are generated illustrations (walk); `modes/*` show the screenshots of before | CHANGED: "illustration (generated art, not a screenshot)" |
| P6-VS03 | row 4: faceoff stills | FALSE NOW | `h45-02-faceoff-stands.png` | CHANGED |
| P6-VS04 | row 6: the hit's stills | FALSE NOW | `h45-03-hit-man-in-the-air.png`, `h45-12-cross-check-sends-him-flying.png` | CHANGED |
| P6-VS05 | row 12: the goal's still | FALSE NOW | `h45-04-goal-banner-lamp.png`, `h45-05-glow-ice-goal.png` | CHANGED |
| P6-VS06 | the header: no effect at any contact, the page's pictures named | added | A235; `H45-PICKS.md` | a pass-6 note |
| P6-VS07 | `script-v2.md`'s pass-5 pointer ("gone from package H43") | TRUE | gone from H45 too | kept |

## 6.5 What the walk did not settle (still CANNOT)
Hit sizes by closing speed (P5-PG04, TG02, RT20), the get-up grace (PG19), the poke's one in ten (PG23, PG65, RT14), the season's step (PG38, PG73), the bots' five behaviours (PG39),
Shotgun's blast (PG44), the carry's length (PG64), mashing (PG66), when he is knocked down (PG67), big rinks (PG70), Bruisers and Cage (PG74), the net's shake and the boards' rock,
the bots planning with a joined human, the shader time (PG81, RT02), Cancel on the firewall (RT07, P6-RT08). The walk was the scripted hand off screen; nothing was heard.

## 6.6 Counts (pass 6)
Rows read: 39 (50 claims: P6-PG13 counted as its 12 mode lines). TRUE: 27 (P6-PG13 eleven of them; P6-PG06 and P6-PG07 are true limits added from PLAY-ME). FALSE NOW and rewritten: 18. CANNOT: 3 (P6-PG13 Shotgun, P6-PG17, P6-RT08). ASK: 1 (P6-FM06). A note added: 1 (P6-VS06). Pass-5 rows moved by the walk: P5-PG49, P5-PG54, P5-PG57 to TRUE (in part, as said).

---

# The truth table, pass 5: every claim of the submission against package H43 (lane A225, track D1, Sun 15 Nov 2026 16:10 to 17:30, main a12ead1b)

**What this is.** Pass 4 (below, headed "pass 2": Saturday 08:00, package H30) is a day and a half and thirteen packages old. Since then the operator played H34 and H39 (R-0067, R-0069, R-0070) and the game moved under the page:
the controls were remapped (R-0064: LMB shoots with the puck and cross-checks without it, RMB passes with it and checks without it), the front end was rebuilt (no "cuts"), nothing is drawn
at a hit, the stop line is off, a loose puck inside the ring under him is his, MODES is a list with a preview, one controls panel replaces the strip and the pill, LADDER SET 4, Glass House
became Glow Ice, Fog of War draws fog. The candidate is **H43** (main `a12ead1b`, cut 16:08, acceptance run, menu row and ship row green by the Director's word; **no DLSS plugin in it**).
This pass ran no game, no build and no git. It read the code at `a12ead1b`, the status files, the play sheet of H41 and the stills on main, and listed (never started) the H41 / H42 / H43 folders.

**Verdicts.** `TRUE` = true of H43 as written. `FALSE NOW` = not true of H43; the sentence is rewritten in its file (column "now"). `CANNOT` = cannot be checked by reading; the column says what would
check it; the words stand. `ASK` = an operator's slot. `DLSS` = the sentence depends on the DLSS plugin being in the package: both versions are written (section 5.6).

**Sources, short names.** `PM` = `docs/PLAY-ME.md` (H41's sheet; its controls table is generated from the live bindings). `MW:n` = `Game/Source/JamGame/Private/MenuWidget.cpp` line n. `MF:n` = `MenuFlow.cpp`.
`DC:n` = `DirectorsCallWidget.cpp` (the pause card). `SW` = `SettingsWidget.cpp`. `HM:n` = `HUDMarks.cpp`. `card` = the mode cartridge's `game.meta`, title and teams. `A2nn` = `status/A2nn.md`.
`still panel/01` = `docs/experiments/panel/01-1080-without-puck.jpg`; `still modes/01` = `docs/experiments/modes/01-1080-open-first-arsenal.jpg`. `R3` = REVIEW-3 (measured on H28's commit: older than every rule of today).

## 5.1 `submission/page.md`

| ID | Claim (short) | Verdict | Source | Now |
|---|---|---|---|---|
| P5-PG01 | pitch: whistle taken out, clay skaters, the speed you bring is the hit you get | TRUE | MF:390 (the banner when the first cut has no penalties); `hockey.regular` penalties false, tier `p3_clay` | kept |
| P5-PG02 | blurb: rough five-a-side hockey for clay skaters, whistle taken out | TRUE | `hockey.regular` 5 v 5; MF:383 | kept |
| P5-PG03 | the mouse is your stick | TRUE | MF:383 | kept |
| P5-PG04 | blurb and page: a nudge beside a man at his speed, a flattening head-on; one button, three sizes; a standing skater holds a side hit | CANNOT | last seen as drill 22 on Accept_H27 (A155); the night changed contact (A184 held cross-check, A209 the check finds its man) and no lane re-measured the sizes by closing speed today | check: `jam.ctrl.Drill 22` on Accept_H43, both halves, or the shot 5 / 6 recording |
| P5-PG05 | stop five stick lengths short of a defender, cut, go | TRUE | A221 law3-s4: pro 94.0 % at 8 m, straight charge 0; all-star 99.0 / 0 | kept |
| P5-PG06 | blurb: "Twelve cuts change the rules." | FALSE NOW | MW:901 MODES "TWELVE WAYS TO PLAY"; the front end says no "cut" (R-0064 point 3) | "Twelve modes change the rules." (60 words kept) |
| P5-PG07 | once a period you move one dial | TRUE | DC:236 "One change per period. Move one slider, press APPLY." | kept |
| P5-PG08 | nobody is polite | TRUE | voice | kept |
| P5-PG09 | five clay skaters a side, clay rink, timber stands, a clay crowd; nobody blows a whistle | TRUE | still panel/01 (stands and crowd); `looks/seen-clay90-playnow-1080-h40-vs-tree.jpg` | kept |
| P5-PG10 | when it is you on the ice, you get up | TRUE | knockdowns end (PM levels) | kept |
| P5-PG11 | PLAY NOW, first match at ROOKIE, the switch reads PRO and ALL-STAR, the level you light is the level you get | TRUE | MW:850-862; MF:31 | kept |
| P5-PG12 | blue against orange, you under the yellow chevron | TRUE | still panel/01; HUDWidget.cpp:246 | kept |
| P5-PG13 | W A S D; the blade follows the cursor | TRUE | PM controls | kept |
| P5-PG14 | hold the left button to load a shot, let go to fire | TRUE | PM: with the puck, a tap is flat, hold to lift | kept |
| P5-PG15 | skate at a carrier and press the right button; Q passes | TRUE | PM: RMB without the puck checks; Q passes (RMB too) | kept |
| P5-PG16 | "The first faceoff lists the keys along the bottom of the screen." | FALSE NOW | A222: one panel bottom left all match replaces the 8-second strip and the pill; still panel/01 | "A panel at the bottom left lists the keys all match." |
| P5-PG17 | carry the puck and you cannot check | TRUE | PM: with the puck RMB passes and LMB shoots | kept |
| P5-PG18 | nothing is drawn on you while you are down | TRUE | R-0070; A219 (`jam.hit.Down` 0, `Jam.Hud.NothingAtAHit`) | kept |
| P5-PG19 | for two seconds after you get up a hit is a bump at most | CANNOT | R-0051's rule; no key or test found by this pass | check: the contact suite's get-up grace test, or a drill |
| P5-PG20 | the puck leaves you by "a shot, a pass, a poke, a lift or a check" | FALSE NOW | PM: the cross-check "shoves the man in front off the puck"; no lift is bound | "a shot, a pass, a poke, a shove or a check" |
| P5-PG21 | win it and a yellow ring closes under you: half a second no stick can take it | TRUE | `stick.take_grace_frames` 30 (schema/DEFAULTS.md:38); HUDWidget.cpp:1863 (the ring in yellow) | kept |
| P5-PG22 | wait for his ring to close, then Space jabs | TRUE | PM (POKE, Space) | kept |
| P5-PG23 | about one jab in ten takes the puck | CANNOT | R3 (H28); the night's possession lanes and A218's ring changed who keeps a puck | check: the person-path instrument's poke rows on a12ead1b |
| P5-PG24 | a miss beside a carrier locks your stick | TRUE | JamSim.cpp:1130-1137 (`StickLockLeft`); its plate is no longer drawn (`jam.hud.ContactPlate` 0) and the page does not promise one | kept |
| P5-PG25 | S against the motion or Ctrl brakes hard; the first step after a stop is quick | TRUE | PM; `skater.burst.*` (DEFAULTS:62-64) | kept |
| P5-PG26 | charge straight at a set defender and you do not get through; the stop works from the same distance at every level | TRUE | A221 (straight 0 at pro and all-star; pro 94.0, all-star 99.0 at 8 m) | kept |
| P5-PG27 | heading "Two marks on the ice" | FALSE NOW | HM:34 (`jam.hud.StopLine` 0): one mark is left | "What the ice tells you" |
| P5-PG28 | a ring in the other team's colour round a loose puck; brake short or let him have it | TRUE | HM:28 (`jam.hud.Marks` 1: the contested ring stays) | kept |
| P5-PG29 | a chalk line five stick lengths ahead while you carry, turning the defender's colour | FALSE NOW | HM:34 (default 0, R-0070); A219 | gone; in its place two things seen today: a loose puck inside your ring is yours (A218, R-0069 point 2), the target mate's ring turns your colour (PM "also new"); "nothing else is drawn on the ice" (R-0070, A219) |
| P5-PG30 | "The line is an offer, not an order. It beats one man with open ice behind him" | FALSE NOW | no line is drawn | gone with P5-PG29 |
| P5-PG31 | ROOKIE: "New to hockey games? Start here: rookies rarely hit." | FALSE NOW | MF:421 "...Start here: rookies are a step slower." (lane A213: rookie bots do hit) | the menu's line |
| P5-PG32 | a newcomer wins most rookie matches; a player who has played before wins it | TRUE | A221 / PM: rookie 99 / 93 for the person-like hand ("close to a walkover") | kept |
| P5-PG33 | PRO's line; "about even for someone who has" | TRUE | MF:423; A221: pro 55.2 regular, 46.0 large (600 seeds) | kept |
| P5-PG34 | ALL-STAR's line | TRUE | MF:422 | kept |
| P5-PG35 | "plays PLAY NOW and the plain cuts; the season's takes and the Director's modes bring their own, and the match card tells you which" | FALSE NOW | MF:29-38 (`jam.menu.ModeLevel` 1: a mode plays HOME's level); still modes/01 "PLAYS AT ROOKIE, THE LEVEL LIT ON HOME"; DC:373 (the pause card says YOUR LEVEL or THIS MODE'S OWN) | "plays PLAY NOW and every mode; the season's takes bring their own, and the pause card tells you" |
| P5-PG36 | the eight takes by name | TRUE | `ladder/take-1..8` titles | kept |
| P5-PG37 | takes 1 to 4 clay against rookies; then neon and night at pro; the finale against all-stars | TRUE | `ladder/take-*`: rookie x4 `p3_clay`; 5 neon, 6-7 night at pro; 8 neon all_star | kept |
| P5-PG38 | a newcomer feels the step at take five; a regular walks through seven | CANNOT | R3 (H28) | check: the person path on the eight takes |
| P5-PG39 | the bots play as a side: mates pass and chase; the other five carry, pass when closed, brake on purpose, sprint with the puck | CANNOT | the night's brain waves (A206) changed all of it; no list of today says these five things | check: a recorded match; the brain's last report |
| P5-PG40 | "They also shoot from much too far out" | FALSE NOW | A221: goals against from 28 m or more 3.6 % at pro regular (set 4); R-0066 point 1 (no long shot with a mate open nearer) | "They also lean on a net's frame now and then" (PM known limits) |
| P5-PG41 | heading "Twelve cuts"; "A cut is a rule set... Pick one under CHOOSE YOUR CUT." | FALSE NOW | MW:901, MW:1242 (MODES; no CHOOSE YOUR CUT); A220 | "Twelve modes"; "A mode is... Pick one under MODES." The list re-ordered A to Z as the menu shows it |
| P5-PG42 | Pond, Cage, Marathon, Power Play, Sudden Death, Outnumbered, Arsenal, Bruisers (eight lines) | TRUE | card (pitch, facts, teams: 5v5, 4v4, 3v3, 5v3, 5v5, 3v5, 5v5, 5v5) | kept |
| P5-PG43 | Sniper: "three a side" | FALSE NOW | card: 5 v 4, "5 v 4, shot 52 m/s" | "five on four" |
| P5-PG44 | Shotgun: one blast clears the lane; anyone near its path goes down, yours too | CANNOT | card (F: shotgun shot) TRUE; who goes down is VISION and R3 (H28); device B widened the blast today (B:H60.0) | check: shot 11 of the recording |
| P5-PG45 | Glass House: the only whistle left, foul and sit ten seconds | FALSE NOW | card `hockey.mode.glass-house`: title "Glow Ice", "Dark ice drawn in light. Boost, but skate clean.", penalties true, 600 frames, tier `l3_neon` | "**Glow Ice**: neon ice, the only whistle left. Foul, sit ten seconds." |
| P5-PG46 | Fog of War: night and fog | TRUE | card tier `l2_night`, fog 0.04; A223: fog is drawn now (`jam.look.FogScale` 8, LookTierSwitcher.cpp:34) | kept |
| P5-PG47 | "Esc is TIME OUT. ... the Director's Call ... move one dial ... hit APPLY" | FALSE NOW | DC:197-216 (PAUSED: RESUME, CHANGE A RULE, RESTART, CONTROLS, QUIT TO MENU); DC:236; DC:84-98 (the four dials unchanged) | "Esc pauses it. Once a period CHANGE A RULE is yours: move one slider (...), press APPLY" |
| P5-PG48 | DIRECT A CUT: one sentence; without a key the nearest cut; with a key a new cut | FALSE NOW | MW:1078, MW:1174, MW:1538 (MAKE YOUR OWN inside MODES; "without a key it answers from games it made earlier") | "MAKE YOUR OWN, under MODES, ... answers from the games it made earlier ... writes a new game" |
| P5-PG49 | "or you pick one of the sentences on the screen" | CANNOT | the chips were seen on H27 (TR); the screen is "as it was (lane A8)" by MW:1078 | check: shot 3 of the recording |
| P5-PG50 | the controls table, 15 rows | 10 TRUE, 4 FALSE NOW, 1 CANNOT | PM controls (generated from the bindings): FALSE: cross-check ("on top of the carrier, lift the puck off him": it is LMB without the puck, MMB or mouse 4, "shoves the man in front off the puck; hold it to brace"), pass ("held, a saucer": RMB with the puck or Q, to the mate the stick points at, hold for power), ability ("the cut's trick": "the mode's"), pause ("Time Out, Director's Call": resume, a rule change, controls). CANNOT: poke's "one in ten" (P5-PG23) | the four rows follow PM; shoot and check rows say with / without the puck |
| P5-PG51 | the same list is under CONTROLS and on the pause card | TRUE | MW:1186 (CONTROLS); DC:213 (CONTROLS on the pause card, rebinding) | kept |
| P5-PG52 | "BRAWL WITH A FRIEND" (Two machines, limits, How to run: three places) | FALSE NOW | MW:905 "PLAY WITH A FRIEND", "ONE HOSTS, ONE JOINS BY CODE" | "PLAY WITH A FRIEND" |
| P5-PG53 | a full seat: "sprint, saucer, brake, spin, check and F" | FALSE NOW | PM: no saucer; the held pass is power | "sprint, pass, brake, spin, check and F" |
| P5-PG54 | the bots on your side plan with you; the two games identical frame by frame; played on one machine to a rematch, 1,000 soak matches | CANNOT (as of H43) | N13 (Saturday); no join run read today | check: the soak driver on a12ead1b; the join drill |
| P5-PG55 | two physical machines have never been tried | TRUE | nothing in A.md or B.md says otherwise | kept; `[O13]` |
| P5-PG56 | the rink: clay rink, stands both sides, crowd; benches, boxes, goal frames | TRUE | still panel/01; A223 clay films | kept |
| P5-PG57 | a red lamp lights over the net and the net shakes; the boards rock | CANNOT | A155 (H27) | check: shot 12 of the recording |
| P5-PG58 | later takes and some modes change the scene: neon, night, fog | TRUE | takes 5 to 8, Glow Ice, Fog of War (fog drawn now), Pond and Outnumbered on the day look | kept |
| P5-PG59 | the booth: captions on screen | TRUE | still panel/01 ("Orange playing keep-away.") | kept; that it is heard: `[O16]` |
| P5-PG60 | Unreal Engine 5.9, C++, Blueprint, Niagara, MetaSounds; written and run by AI agents directed by the team; rules measured in simulated matches | TRUE | unchanged; PM "How it was proven" | kept |
| P5-PG61 | AI-generated content declared: images, figures and animation, voice and sound | TRUE | credits.md (16:21): Codex 52 + 10, Rodin 48 + 10, Meshy, Tripo, ElevenLabs 415 + 103 + 1 rows | kept |
| P5-PG62 | "Sourced: the engine's own content and one shipped graphics-upscaling library" | FALSE NOW / DLSS | H43 carries no DLSS (`find` in `Hokej_Hokej_H43/Windows`: no nvngx or DLSS file); credits.md: the other shipped sourced rows are four Roboto fonts of the engine | without DLSS: "the engine's own content, its font among it"; with: "... and one shipped graphics-upscaling library" (in the comment beside it) |
| P5-PG63 | limits header: "true of package H30" | FALSE NOW | H43 is the candidate | "true of package H43" |
| P5-PG64 | carry is short: about a second; off your own blade with nobody near | CANNOT | R3 (H28); since then A208 (the puck belongs to whoever touches it), A218 (the ring), device B's puck on the blade | check: carry median on the person path at a12ead1b; if unchecked by 19:00, delete the line rather than keep a stale number |
| P5-PG65 | Space is a gamble: one in ten | CANNOT | as P5-PG23 | as P5-PG23 |
| P5-PG66 | hammering the right button lands more checks than timing it | CANNOT | the cooldown that would stop it still ships off (`contact.check_cooldown_frames` 0, JamBaseline.h:58); the measure is R3's | check: mashed against timed presses with the person path |
| P5-PG67 | at pro you spend real time on the ice, most often as you win the puck | CANNOT | the first half TRUE (PM: about 1.8 knockdowns a minute at pro); "most often as you win the puck" is R3's | check: knockdowns by frames since the pickup |
| P5-PG68 | the stop beats one defender; "stopping at every lit line loses you more than it wins" | FALSE NOW | no lit line (HM:34) | the second half removed |
| P5-PG69 | opponents shoot from the far side of the red line and it goes in too often | FALSE NOW | A221: 3.6 % of goals against from 28 m or more at pro regular | "Under pressure your passes still arrive only one time in three to one in two." (PM known limits) |
| P5-PG70 | on the big rinks half a minute without the puck | CANNOT | R3:154 | check: touches a minute on the large rink, person path |
| P5-PG71 | a skater can stand at a downed body and push it along | TRUE (by code) | `contact.downed_radius_m` 0 (DEFAULTS:132; no cartridge sets it) | kept |
| P5-PG72 | the two marks were new on the last night, unplayed | FALSE NOW | one is gone; the operator has played since | "A bot can lean on a net's frame for a second or two before it goes round." (PM known limits) |
| P5-PG73 | the season is a school then a wall | CANNOT | R3 | as P5-PG38 |
| P5-PG74 | Bruisers and Cage closer to the regular game than their cards | CANNOT | Bruisers moved to 56 x 28 m; PM: "the twelve modes at pro inherit the set and were not re-measured" | check: the modes' distinctness table at set 4 |
| P5-PG75 | corners: the camera looks down steeply, the net leaves the picture; Glass House: the box below the picture | FALSE NOW | main f7d0b97c and b3284ba0 (the camera at the boards remade); the mode is Glow Ice; A223 films: the camera stands at the box | "On Glow Ice the camera can jump to the ice beside a boost beacon after your cross-check lands." (PM known limits) |
| P5-PG76 | the pad is mapped, no person has play-tested it | TRUE | PM ("not seen with a real pad attached") | kept |
| P5-PG77 | Cancel on the firewall prompt not tested | TRUE | nothing in B.md from H57 on reports the clean-machine test | kept; `[O7]` |
| P5-PG78 | the joined match on one machine, never across two | TRUE | as P5-PG55 | kept |
| P5-PG79 | How to run: zip "today `Hokej_Hokej_H30.zip`" | FALSE NOW | `Game/Packaged/Hokej_Hokej_H43/Hokej_Hokej_H43.zip`, PLAY-LATEST.cmd names H43 | "today the candidate is `Hokej_Hokej_H43.zip`" |
| P5-PG80 | `ThirdJam.exe`; Windows 10 or 11, DirectX 12; vc_redist path; README.txt in the zip | TRUE | H43 listing: `Windows/ThirdJam.exe`, `Engine/Extras/Redist/en-us/vc_redist.x64.exe`, `README.txt` (note: H43's README.txt is the text before this pass: section 5.4) | kept |
| P5-PG81 | first launch compiles shaders for a few seconds | CANNOT | A155 (H27); H43 not started by this lane | check: the ship row's log of H43 |
| P5-PG82 | slots O1, O3, O5, O6, O8, O13, O15, O16, O7 | ASK | `status/A225.md` FOR THE OPERATOR | |

## 5.2 `submission/form.md`

| ID | Claim | Verdict | Source | Now |
|---|---|---|---|---|
| P5-FM01 | team name, game name, track, theme | TRUE | unchanged | kept |
| P5-FM02 | "The game described is package H30 (main 95ae15a6)" | FALSE NOW | the Director's word: H43, main a12ead1b | "package H43 (main a12ead1b, cut 16:08 without the DLSS plugin)" |
| P5-FM03 | pitch and blurb = page A and B, blurb 60 words | TRUE | counted again after P5-PG06: 60 | kept |
| P5-FM04 | Zip: "Today: Hokej_Hokej_H30.zip ..." | FALSE NOW | H43 | H43, main a12ead1b |
| P5-FM05 | "About 267 MiB zipped (H29's measure)" | FALSE NOW / DLSS | H43 zip 253,477,779 bytes (242 MiB), 365 MB unpacked; H41 (with DLSS) 284,254,514 bytes (271 MiB), 421 MB | both sizes written |
| P5-FM06 | the clean-machine test not yet done on any package | TRUE | as P5-PG77 | kept; `[O7]` |
| P5-FM07 | Video: nothing of the final film exists; v1 is Thursday's | TRUE | SUNDAY: recording from about 18:00 | kept; `[O17]` |
| P5-FM08 | members, dev log hosting, prizes | ASK | `[O1]`, `[O8]`, `[O4]` | |
| P5-FM09 | "whether a Rodin or Tripo asset is shipped (... shipped n, on Friday's tree)" | FALSE NOW | the ledger at a12ead1b: 41 Hyper3D Rodin rows `shipped` y; Tripo3D 0 of 10; Meshy 0 shipped | the counts; Rodin is listed (SUBMISSION.md's rule); vendor naming stays `[O5]` |
| P5-FM10 | tools: Unreal, Claude Code, Codex image generation, Meshy / Rodin / Tripo, ElevenLabs, Blender, ffmpeg, the Anthropic API (key typed by the player) | TRUE | credits.md (16:21); SW:251 ("API KEY... kept on this machine only") | kept |
| P5-FM11 | "NVIDIA DLSS 4.5 ...: Settings > DLSS, on by default on RTX GPUs" | FALSE NOW / DLSS | SW has no DLSS row (AI LEVEL, LOOK, API KEY, VOLUME, VOICE SIGNALS, BARKS, CONTROLS PANEL); DLSS is set by `UJamDlssSubsystem` (DlssJam.cpp); H42 and H43 carry no DLSS | with: "on by default on RTX GPUs, set by the game itself (no switch in SETTINGS)"; without: the line is left out |
| P5-FM12 | "credits.md was generated on Friday afternoon's tree" | FALSE NOW | regenerated by this lane at 16:21 | said so; the ledger gate not rerun (`[O6]`) |
| P5-FM13 | sourced: "62 ledger rows, 1 shipped (the DLSS library)" | FALSE NOW / DLSS | credits.md (16:21): 66 sourced, 5 shipped: 4 Roboto fonts + the DLSS library | the new counts; with H43 the four fonts only |

## 5.3 `submission/tagline.md`

| ID | Line | Verdict | Source | Now |
|---|---|---|---|---|
| P5-TG01 | Hockey with the whistle taken out. | TRUE | MF:390; only Glow Ice has penalties | kept |
| P5-TG02 | The speed you bring is the hit you get. | CANNOT | as P5-PG04 | kept: the law as designed (VISION); check as P5-PG04 |
| P5-TG03 | Clay skaters. Real hits. | TRUE | still panel/01; A219 films (the figure does the hitting) | kept |
| P5-TG04 | Rewrite the rules, then try to survive them. | TRUE | CHANGE A RULE (DC:236) | kept |
| P5-TG05 | Five on five, and nobody is polite. | TRUE | `hockey.regular`, the takes, six modes 5 v 5 | kept |
| P5-TG06 | Pass it. Or get flattened. | TRUE | voice; PM (pro knockdowns) | kept |
| P5-TG07 | "one mode, Glass House, keeps the whistle" | FALSE NOW | card: Glow Ice | "Glow Ice" |
| P5-TG08 | "Six taglines, each true this morning" (Saturday's) | FALSE NOW | lines re-read against H43 | "each true of package H43"; two lines added to "Lines NOT to write" |
| P5-TG09 | the narration's "cut", "which the voice guide keeps for the menu" | FALSE NOW | the menu says MODES and CHANGE A RULE, never "cut" | rewritten; still `[O17]` |
| P5-TG10 | `[O3]` clay as the first look | ASK | | |

## 5.4 `submission/package/README.txt` (ships beside the exe)

**H43's `Windows/README.txt` is the text BEFORE this pass** (byte-identical to the file as it was at 16:10). Every FALSE NOW line below is still in the zip of H43 until a package is cut from a tree that holds
this file, or the file is replaced in the zip (FOR THE OPERATOR, status/A225.md).

| ID | Line | Verdict | Source | Now |
|---|---|---|---|---|
| P5-RT01 | intro; HOW TO START 1 and 2 | TRUE | | kept |
| P5-RT02 | first launch compiles shaders a few seconds | CANNOT | as P5-PG81 | kept |
| P5-RT03 | PLAY NOW at the lit level; who each level is for | TRUE | MW:850; MF:421-423 | kept |
| P5-RT04 | Windows 10 / 11, DirectX 12; "About 450 MB of disk when unpacked" | FALSE NOW (the size) | H43 365 MB, H41 421 MB | "Under 450 MB" (true with and without DLSS) |
| P5-RT05 | the VC++ runtime and its path | TRUE | H43 listing | kept |
| P5-RT06 | the firewall: "(BRAWL WITH A FRIEND)" | FALSE NOW | MW:905 | "PLAY WITH A FRIEND" |
| P5-RT07 | "Solo play does not need network access." | CANNOT | the clean-machine test has not run (`[O7]`) | kept (it is what the game is built to do) |
| P5-RT08 | W A S D / arrows, S brakes; mouse aims | TRUE | PM | kept |
| P5-RT09 | "Left mouse: shoot: hold to charge" | FALSE NOW | PM: with the puck shoot (hold to lift); without, cross-check (hold to brace) | both halves |
| P5-RT10 | "Right mouse: body check" | FALSE NOW | PM: with the puck pass (hold for power); without, check | both halves |
| P5-RT11 | middle mouse or mouse 4: cross-check | TRUE | PM | kept |
| P5-RT12 | "Q: pass; hold Q for a saucer pass" | FALSE NOW | PM: hold for power | "pass (hold for power)" |
| P5-RT13 | F, E, mouse wheel, Shift, Ctrl, R | TRUE | PM | kept |
| P5-RT14 | Space: about one jab in ten | CANNOT | as P5-PG23 | kept |
| P5-RT15 | "Esc: pause, Time Out, the Director's Call" | FALSE NOW | DC:197-216 | "pause: resume, change a rule, controls" |
| P5-RT16 | gamepad: "RT shoot, RB check, LB cross-check, X pass" | FALSE NOW | PM pad column: RT shoot or cross-check, RB pass or check | both halves of RT and RB |
| P5-RT17 | the pad is mapped, not play-tested | TRUE | PM | kept |
| P5-RT18 | (added, brief item 3) the controls panel and how to switch it off; rebinding | added | A222 (SETTINGS, CONTROLS PANEL ON / OFF, saved; SW:317); DC:213 and MW CONTROLS (rebind) | two lines |
| P5-RT19 | "No whistle (one mode, Glass House, keeps it)" | FALSE NOW | card: Glow Ice | "Glow Ice" |
| P5-RT20 | hits obey momentum: a nudge beside, flattening head-on | CANNOT | as P5-PG04 | kept |
| P5-RT21 | carrying, you cannot check; stop five stick lengths short; five a side, mates pass | TRUE | PM; A221 | kept |
| P5-RT22 | "Once per period the Director's Call (Esc) moves one dial" | FALSE NOW | DC:206, DC:236 | "CHANGE A RULE (Esc) moves one slider" |
| P5-RT23 | made with UE 5.9 by agents; AI content declared | TRUE | | kept |
| P5-RT24 | "The full list is in the game's CREDITS screen and in NOTICES.txt beside this file" | FALSE NOW | H43's `NOTICES.txt` holds one line (FreeType); the CREDITS screen (MW:1166-1176) names the tools, not a full list | "The tools are named on the game's CREDITS screen." |
| P5-RT25 | fiction, clay figures, no real teams, players or brands | TRUE | | kept |

## 5.5 The video (`video/SHOTLIST.md`, `script-v2.md`, `cutlist.md`)

| ID | Row | Verdict | Source | Now |
|---|---|---|---|---|
| P5-VS01 | shot 1: "the slate menu, the HOKEJ clapper" | FALSE NOW | R-0064 point 3; the front end of A187 / A220 (landing, then HOME) | CHANGED: HOME |
| P5-VS02 | shot 2: CHOOSE YOUR CUT, the twelve cards | FALSE NOW | A220: MODES is a list with a preview | CHANGED: MODES |
| P5-VS03 | shot 3: DIRECT A CUT, ROLL | FALSE NOW | MAKE YOUR OWN, the last row of MODES (MW:1538) | CHANGED |
| P5-VS04 | shot 5: the nudge on Accept_H30, "the camera rides beside it" | FALSE NOW (the package) | H43's twin | CHANGED: Accept_H43; nothing drawn at the hit |
| P5-VS05 | shot 8: "the line ahead turns his colour"; "a beat after the line lights" | FALSE NOW | HM:34 | CHANGED: the stop by eye, no line |
| P5-VS06 | shot 9: Esc, TIME OUT, DIRECTOR'S CALL, "CALL READY pill" | FALSE NOW | DC:197-216, DC:618 ("ESC / START RULE CHANGE READY") | CHANGED: PAUSED, CHANGE A RULE |
| P5-VS07 | shots 4, 6, 7, 10, 11, 12, 13 (as plans: what to record and how) | TRUE | still panel/01 (stands); B:H60.0 (the blast widened: shot 11 still "looked at before it is promised") | kept |
| P5-VS08 | the rules of the film; section 4 (how recorded and cut); section 5 (what not to say) | TRUE | unchanged; section 5 gains the R-0070 line | kept |
| P5-VS09 | section 1 (Saturday's twelve stills) | TRUE as a record | | not re-judged |
| P5-VS10 | `script-v2.md`'s table | TRUE as history (marked superseded since Saturday) | | a pass-5 pointer line added |
| P5-VS11 | `cutlist.md` (the record of `hokej-v1.mp4`) | TRUE as a record | | unchanged |

## 5.6 What depends on DLSS being back

H41 and earlier carry the DLSS plugin; **H42 and H43 do not** (the disk cleanup of 14:29, `status/A.md` 15:21; `Hokej_Hokej_H43/Windows` holds no nvngx or DLSS file). Its restore waits for the operator's word.

| Where | Without DLSS (H42, H43: the text in the file now) | With DLSS back (a package cut after the restore) |
|---|---|---|
| `page.md` How it was made (P5-PG62) | "Sourced (not ours, not scored): the engine's own content, its font among it." | "... its font among it, and one shipped graphics-upscaling library." (in the comment beside the sentence) |
| `form.md` tools (P5-FM11) | the DLSS line is left out; the NVIDIA questions are not answered | "NVIDIA DLSS 4.5 (Unreal plugin v8.9.0, Super Resolution and DLAA, ported to 5.9): on by default on RTX GPUs, set by the game itself (no switch in SETTINGS)"; answers the NVIDIA questions |
| `form.md` Zip size (P5-FM05) | 242 MiB zipped, 365 MB unpacked (H43) | about 271 MiB zipped, 421 MB unpacked (H41) |
| `form.md` sourced assets (P5-FM13) | shipped sourced rows: the four fonts | the four fonts and the DLSS library |
| `package/README.txt` | nothing ("Under 450 MB" holds both ways) | nothing |
| outside this lane's family | `ASSETS.md` marks `Game/Plugins/DLSS/.../nvngx_dlss.dll` `shipped` y: false for H42 / H43; `docs/experiments/DLSS.md` section 3 says "Settings offers Auto / Quality / ... / Off": false either way (no SETTINGS row exists) | the ledger row is right; DLSS.md section 3 is still wrong about SETTINGS |

## 5.7 Counts (this pass)

Claims read: rows of sections 5.1 to 5.5, the controls table counted as its 15 rows, the added README lines (P5-RT18) not counted.

| | read | TRUE | FALSE NOW (rewritten) | CANNOT | ASK |
|---|---|---|---|---|---|
| page | 96 | 50 | 27 | 18 | 1 (the slot row: nine slots) |
| form | 13 | 5 | 7 | 0 | 1 (three slots) |
| tagline | 10 | 5 | 3 | 1 | 1 |
| README.txt | 24 | 10 | 10 | 4 | 0 |
| video | 11 | 5 | 6 | 0 | 0 |
| **all** | **154** | **75** | **53** | **23** | **3** |

(The page row: P5-PG50 is 15 rows: 10 TRUE, 4 FALSE NOW, 1 CANNOT; P5-PG52 is one claim in three places. DLSS-dependent rows are counted as FALSE NOW: P5-PG62, P5-FM05, P5-FM11, P5-FM13.)

---

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
