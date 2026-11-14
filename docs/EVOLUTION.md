# The evolution tracks (Director; live from Fri 13 Nov 18:50; R-0047)

Ten standing tracks. Each lane does one evolution, lands through the gate (and the acceptance run when a player's verb changes), and proposes the next three. The Director
keeps every track busy until the freeze. Rules: `briefs/EVOLUTION-COMMON.md`. The game's laws: `docs/VISION.md`.

**Freezes (R-0047):** rule changes in the sim land until **Saturday 19:00**, each with a neutral value and a green gate; content freezes **Saturday 22:00**; Sunday is the
submission; submit Sunday 22:00. **Cursor owner now: track S2** (after the Director's H26 release run).

## Fable: mechanics and design, and Blender
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **F0 Design eye** | Play the build as a critic and keep the other tracks' queues honest | `docs/design/**` only; no source | Review H26 against VISION: the ten things a first-time player feels in five minutes, ranked, each with the track that owns it |
| **F1 Contact feel** | A hit that reads and feels right at every strength | the contact functions in `JamSim.cpp`, `JamPawns.cpp` (hit-stop), `CameraFollow.*` (kick), the HUD's contact texts | The stumble and the refused press read: HUD line for "not with the puck", the stumble's camera and hit-stop scaled by the ratio, a board check that uses the boards as the second body |
| **F2 Carrier skill** | Beating a defender by stopping, cutting and the stick feels great | the movement and stick functions in `JamSim.cpp`, drills in `HokejGameMode.cpp`, `HokejAcceptDrill.cpp` | Drill 21 (stop-and-go past a standing pro defender) and the tuning it shows: the burst's window, the brake's grip, puck protection with the body between |
| **F3 Arena in Blender** | The rink dressed for 5 v 5 in the clay voice | `Game/Content/Arena/**` (A's own meshes), `tools/blender/arena/**`, `RinkActor.*` placement only | Benches and penalty boxes with doors, built in Blender to the rink's dimensions, placed by `RinkActor`, using B's clay materials by reference |

## Opus: the codebase and the quality of the implementation
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **Q1 Determinism and lockstep** | Two peers stay bit-identical with everything added today | `JamSim.cpp` hash and state only, `Game/Source/JamNet/**` | Audit every field added since protocol 5 (abilities, contact, stumble, burst, sprint, brake, lift cone) for the hash and the wire; re-run A56's soak on protocol 7 |
| **Q2 Review and repair** | Today's fast landings read by an adversary | the ability rule, `JamAICore.*` (the brain), `CommentarySystem.*`, `MenuFlow.*`, cartridge loader | Review A84, A94, A90, A81 for edge cases (an ability during a knockdown, a hip check under the new contact, the brain's reads with nine skaters down, a rejected mode file) and fix what is real |
| **Q3 Performance at five a side** | No hitch, and headroom, with ten figures and the new brain | `EffectsBinder.*`, `HUDWidget.*` tick, the brain's cost, `DefaultEngine.ini` | Profile H26 at 5 v 5 on both tiers with A58's tools; the brain's space read per frame; fix the top offender |

## Sonnet: tests and hardening
| Track | Goal | File family | First evolution |
|---|---|---|---|
| **S1 Soak** | The whole game, tens of thousands of matches, every cut and mode | `tools/soak/**`, reports | The full soak on the new main (nine cells, all twelve modes, the ladder's takes): crashes, matches that never end, idle stretches, modes whose character number broke under the vision |
| **S2 Acceptance matrix** | The acceptance run on everything a player can pick | `tools/qa/accept/**` drivers and cuts only, reports | The acceptance run on each mode and each look tier; a Shipping smoke and clean-profile test of H26; which rows a mode's rules legitimately change |
| **S3 Fuzz and loader** | No cartridge can crash or hang the game | `tools/fuzz/**`, tests under `Game/Source/JamCore/Private/Tests/JamFuzzTests.cpp` | Thousands of schema-valid random cartridges at the limits through the loader and 600 frames of sim, headless: crashes, non-determinism, never-ending matches |

## Device B: the same loop (handoff A-to-B-0022, 19:20; B owns its board)
| Track | Model | Goal |
|---|---|---|
| **BF1 The figure plays the vision** | Fable + Blender | The stumble by direction and strength, the side takes lowered, a hard-stop pose and spray |
| **BF2 The arena and the clay look** | Fable + Blender | Takes over the arena track from lane A103's report: the crowd band, the scoreboard, the stands |
| **BF3 Hits you hear and see** | Fable | The ability effect and sound, a bump and a stumble sound, the brake's spray, the crowd by strength, 72 commentator lines |
| **BQ1 The quality of B's implementation** | Opus | Blueprints against the hit-stop, every take plays when its event fires, cook warnings, ledger sizes |
| **BS1 Tests only a second machine can run** | Sonnet | The clean-machine test of every package, the video with sound, joined play A to B, B's half of the dev log |

A merges `dev-b` at every half-hour check when it is ahead; B merges main before each lane.

## Queue (the Director appends each lane's "next three" here; the top line of a track is what runs next)
**From F0's REVIEW-1 (19:34; R-0048):**
- **F2** now: the carried puck leaves only by an action (`stick.passive_take` false), never to a teammate; then drill 21 with the body between; then a near miss is not a stumble.
- **Curve** (new track C0, Opus, starts on F2's report): re-set the ladder with the puck kept; all_star's speed for the opposing side only.
- **Content** (new track C1, Sonnet, with C0): `hockey.regular` and takes 1-2 without penalties; the first three takes at rookie; the mode cards that still say "checks from N m/s".
- **Q2** next: the brain brakes on purpose (reverse brakes at pro 2,146 to under 500 a match); mates pass to the human where he has room; the bot carrier sprints on an open lane.
- **F1** next after "every outcome reads": the hit's size on screen (hit-stop 2 to 6, the kick by frames down); the braced bounce as a row and a line.
- **S1** next: the shape at 60 seeds a level; the scorecard's read-outs in the playtest line; the season in order with retries.
- **S2** rows to add: bump, refused, stumble, braced, brake with its spray.
- **F3** next: something behind the glass; the box used during a penalty.
- **F0** next: REVIEW-2 after F1, F2 and Q2 land (same seeds); the twelve modes against the vision.
- **Q1** (from A104): abilities for the online guest (running as A110); the hash-coverage lint; all twelve modes soaked on protocol 8.
- **Submission**: the page says "difficulty is aggression, not speed" and "cross-check a skater who is not looking": both false of the build; the whistle line; the video script's shot 5.

**From device B's play-through (B-to-A-0038; R-0049, 19:38):**
- **F1** next (before the hit's size): release grace (`contact.release_grace_frames` 90): nobody is down again within 2 s of release; the human down fewer than five times in 100 s at pro.
- **F2** next (after the passive take): take grace (`stick.take_grace_frames` 30): a carrier keeps the puck at least half a second between stick takeaways.
- **U1 the screen** (new track, Fable, lane A111 in worktree `a100`): the human's marker at five a side, the fade-in waits for the lights, the hint bar and labels, the camera in the corners.

**From A101 (F1, 19:59) and A106 (Q3, 19:59):**
- **F1** running: A113, release grace and the soft hit explains itself. Then: the board check as a play (`contact.board_slam`; landed hits the boards answer per pro match from 1 to 3 or more); a hard turn lowers stability (`contact.turn_stability` 0.85, neutral 1).
- **Q3** running: A114, the match-start frame under 100 ms in 6 of 6 runs, no synchronous lookup in play, the tournament build. Then: the first 60 frames under 33 ms; the figure Blueprint under 0.2 ms a frame (with device B).
- **S2** rows to add from A101: "refused" reading the HUD's contact text; captures of a stumble's hold and a check into the boards.
- **Hygiene** (Sonnet, when a slot is free): `ownership.json` does not know the folders added since Thursday (185 paths "owned by nobody" in the gate's ownership step).

**Board at 20:14 (R-0050). Landed: A104, A101, A106, A103, A107, A109, A105 (main 7f4d501, pushed and verified); landing: A110, A102.**
| Track | Running now | Then |
|---|---|---|
| F1 contact | A113 release grace, the soft hit explains itself | the shield refuses the stick, never the shove; the board check as a play; a hard turn lowers stability |
| F2 carrier | A121 take grace, the stop's grip, the carrier's first step | a late answer inside 7 m (spin and deke windows) |
| F3 arena | A119 clay goal frames, the scorer's table, stills from the game | the boxed skater sits in the box (modes with the whistle) |
| U1 screen | A111 the human's marker, the fade-in, the hint bar, the corner camera | from its report |
| Q1 lockstep | A120 two real processes, the refusal card, barks for the guest, the join guide | from its report |
| Q2 brain | A115 mates pass to the human, the chaser, braking on purpose, reading the shield | loose puck and downed human; slow puck flight; live changes |
| Q3 performance | A114 the match starts without a hitch | the first 60 frames; the figure Blueprint with device B |
| S1 soak | A116 one shared driver, a drift gate for modes and takes | a soak of the packaged game |
| S2 acceptance | A108 the matrix on every mode and tier (cursor owner) | rows: refused, bump, stumble, braced, brake with its spray |
| S3 fuzz | A118 the seven fault classes closed | lockstep fuzz with Q1 |
| C0 curve | starts when A102 is on main (lane A112, new worktree) | re-measure after each rule landing; the last set at the rule freeze |
| M1 modes | starts with C0: cage, outnumbered, shotgun differ again; the marathon's horn; take-3 against take-6 | the twelve modes against the vision |
| F0 design eye | REVIEW-2 when F1, F2, Q2 and C0 have landed (same seeds) | |

**Board at 22:40 (R-0053). Main 41df3e6, pushed and verified. H26 is the newest package; none is cut before I1 and C0 land.**
Landed since 20:03: A104, A101, A106, A103, A107, A109, A105, A110, A102, A111, A114, A113, A119, A118, A116, A121, A120, A122, A127, A123, A108, A129, A124.
Parked on the local branch `wip-batch6` (worktree `i1`): A115 (the brain's second evolution, hooks wired), A125 (the shove under the shields, the figure
calls), with A128 (the standing start, the spin's price) applied on top. Exported and waiting for I1: `tj/a105.patch`, `a125.patch`, `a128.patch`; the curve
lane's first evolution: `tj/c0.patch`.
| Track | Worktree | Running now | Then |
|---|---|---|---|
| **I1 integration** | `i1` | A138: one shared neutral baseline, what broke law 3, law 3 holds with everything on, the take grace's same-frame hole | lands as one; then C0 on its tree |
| C0 curve | (fresh, from I1's landing + `c0.patch`) | waits for I1 (brief A133) | the season's ramp; a re-measure after every rule landing |
| F1 contact | (fresh, after I1) | waits for I1 (brief A132: the press that lands, the hit's size, stale brake flags) | the board check as a play |
| F2 carrier | `a102` | waits for I1 | a second move after the go; the window by level; the spin from 5 m |
| Q2 brain | (fresh, after I1) | waits for I1 (brief A134: defend the kept puck, the puck stays with the human's side, shot selection) | |
| U1 screen | `u1b` | A135: the camera never loses the player | bark text on a lane |
| F3 arena | `f3b` | A136: the whole boards answer a hit; the goal loud on clay | the box is used |
| Q3 performance | `q3b` | A137: the rig preload on main, the first 60 frames, the budget at full resolution | |
| M1 modes | `m1b` | A139: the passing modes pass; the grace keys per mode; the marathon's pace | all twelve on the re-set ladder |
| S1 soak | `a107` + `clean-main` | A126: a fresh baseline on 41df3e6, the drift gate in the gate, new playtest fields | a baseline again when the curve lands |
| S2 acceptance | `a108` | A131: the acceptance run tells the game's faults from the machine's (cursor owner) | the queued rows and stills on a new package |
| Q1 lockstep | `a104` | next: live cuts as lockstep input, a clean rematch | two physical machines with the operator |
| S3 fuzz | `a109` | next: Designer-reply parity, a timer oracle, rule chains | |
| F0 design eye | | REVIEW-2 on the first package after the curve | |



**Board at 03:30 (R-0061). Main e9d0b55 pushed; H27 cut from it at 03:20 (acceptance green on the first try, zip 275.5 MB). In the gate as one landing:
A152 (the poke), A154 (the ladder check), A153 item 1 (one stop distance). The operator plays at 10:00; his package (H28) is cut by 09:00, chain from 07:00.**
| Track | Worktree | Running now | Then |
|---|---|---|---|
| S2 acceptance | `a108` | A155: H27 through the real window: the menu row, the queued rows, the matrix on tonight's modes, twelve stills, the Shipping zip played (cursor owner until 06:30) | H28's rows |
| C0 curve | `c0b` | starts on the landing: A162, the last pass (R-0061): `LADDER SET` by 06:15 | after 10:00: the long shot, the operator's answer on which player he is |
| Q2 brain | `q2b` | A153 items 2 and 3 (a mate that finishes, the big ice): neutral tunes, measured | from its report |
| M1 modes | `m1b` | A156: five a side where the mode allows it, the Marathon's puck, Arsenal's tricks | cartridges may land until 06:45 |
| F1 contact | `f1b` | starts on the landing: A157: nobody stands in a downed body (B's critic), a check's cooldown, the lunge, the board slam: all neutral | turned on with the operator's priorities |
| F2 carrier | `f2b` | starts on the landing: A163: the cues (take grace, stick lock), where the carry ends, time lying down | |
| U1 screen | `u1b` | A158: nothing drawn over a downed player (B's critic), captions off the middle third, the corner camera's pitch | for the 09:00 package |
| F3 arena | `f3c` | A160: the boxes' clock faces on clay, round caps on every rink, the goal in time with B's effect | for the 09:00 package |
| Q3 performance | `q3b` (main + `origin/dev-b` merged locally) | A161: is B's night safe for the package (build, tests, cook, the stands timed), 183 MB of unused figures out of the cook, B's five timings | dev-b merges into main at about 06:45 on its word |
| S3 hardening | `a109` | starts on the landing: A159: a gate step for test code outside its guard, every launch cut reaches its match, fuzz of tonight's keys | |
| S1 soak | `a107` | idle until `LADDER SET`: then a fresh baseline | |
| Q1 lockstep | `a104` | idle (live cuts as protocol 9: not before the operator plays) | two physical machines with the operator |
| D1 submission | `d1` | idle until the operator answers the fourteen questions | the page and the video's shot list after 10:00 |
| Device B | `dev-b` | nine lanes (B-to-A-0040): three new figure sets, stands and crowd, effects, a critic (B-to-A-0041: answered in A-to-B-0025) | last push for the package by 06:30 |


**Board at 06:00 (R-0062). Main ca493723 pushed (265 tests). Package H28 cut at 05:46 from c5dc15e5: acceptance green on the first try with the
black-frame row; in a real window: the faceoff without the post, STICK LOCKED by hand, the menu on a clean profile, the Shipping zip unzipped fresh and
played, the ice marks KEEP. H27 is dead (its PLAY NOW built the match twice: the wrong level, a black rink). The operator plays H28 at 10:00.**
| Track | Worktree | Running now | Wanted by |
|---|---|---|---|
| S2 acceptance | `a108` | A155: the matrix on H28's modes, then its report (cursor owner until 07:15) | 07:15 |
| F0 design eye | `f0b` | A169: REVIEW-3 on c5dc15e5: the play sheet line by line, the ranked ten | sheet verdict 08:15, top ten 09:15 |
| C0 curve | `c0b` | A170: the hunt dial that puts pro at 48 without touching the stop (the brain lane's Press job), for both players of R-0060; the price of the switched-off set | `DIAL READY` 08:30 |
| M1 modes | `m1b` | A173: the twelve modes re-measured on this main; Sniper, Arsenal, the good class | the table 07:30 |
| U1 screen | `u1b` | A171: one down rule for the new marks, TOO SOON, the last overlaps, why the contested ring did not read | after 10:00 |
| F3 arena | `f3c` | A172: the Glass House box in the camera's picture, B's lamp and cords, A's pieces against B's stands | after 10:00 |
| Q3 performance | `q3b` | A175: the first match's start on a cold shader cache, a timed goal, the figure Blueprints' tick for B | after 10:00 |
| S1 soak | `a107` | A174: a drift run, every stale cell re-taken on this main, the hour's soak | after 10:00 |
| F1 contact | `f1b` | idle (A165 landed off): next: the hand that aims for the wall; the whole set on with the curve | the operator's word |
| F2 carrier | `f2b` | idle (A166 landed on: the marks): next: the instrument obeys the stop mark; the pickup that settles with the curve | the operator's word |
| Q2 brain | `q2b` | idle (A167 landed off): next: keep-out plus push-out on main; pro's presser on its job, then the hunt as the curve's dial (A170 measures it) | the operator's word |
| S3 hardening | `a109` | idle (A159 landed): next: the double-build row from inside a match; a Shipping compile in the gate | |
| U2 menu | `u2` | idle (A164 landed): next: every first-screen button clicked by the acceptance run; the no-dead-key rule on the other screens | |
| Device B | `dev-b` | merged at 83c76776 (a46c8081); A-to-B-0025 and 0026 open | its answers |
Switched off and measured, waiting for the operator: the push-out round a downed skater (+2.7 all_star), the check's cooldown with restart (free) and
lunge (+2.5), the board slam, the half-second screen (+1.5 / +2.2 / -1.4), the stop's grace, the pickup that settles, the mates' keep-out, pro's
presser on its job, the three's chaser.


**Board at 08:20 (R-0063). Main = package H30's game (cut 07:03 from 95ae15a6; verified in a real window: the stop line, the hint strip, the menu line,
the fresh zip). H29 is the fallback; H27 and H28 are withdrawn. Every lane has reported; the machine is quiet for the operator's play at 10:00.**
Waiting for his word, each measured and ready as a patch in the Director's folder or switched off on main:
| What | State | What it buys | What it costs |
|---|---|---|---|
| Pro's dial (hunt 11, the presser on its job) | two patches: `tools/perf/results/a170/A170-pro-hunt11.patch` (everywhere at pro) and `-pinned.patch` (the regular game and the season; modes and four cuts pinned) | pro 48.0 / 47.7 (52.4 / 50.2 today); the season's step 4 to 5 from 19.5 to 13 | the human 0.2 points of play more on the ice; unpinned: the modes about ten points harder |
| A hit as he wins the puck is a stumble (`contact.take_shield_frames` 15, scaled to 45) | exported (A177), off | his knockdowns at pro 6.9 to 3.7 a match, lying down 9.1 to 4.5 % of play | pro +2.8, all_star +6.0 (2,000); law 1 off for a head-on hit in that window |
| The push-out round a downed skater with the mates' keep-out | on main, off | spells at a downed body 1,468 to 2 in 200 pro matches | -2.3 / -1.2 / +2.8 |
| The check's cooldown 60 with restart | on main, off | a mashed button lands 2.9 a match (20.4 today; timed 3.3) | half a point |
| The lunge (12 frames, 1.5 m/s), the board slam (30 frames) | on main, off | a press commits the body; a slam lasts 25 frames longer | lunge +2.5 at all_star; slam +1.8 |
| The whole contact set | | he lies 4.4 % of play at pro, 6.1 % at all_star | pro +3.4 (the dial at 12 pays it), all_star +7.3 (no lever) |
| The marks made to read (A178) | exported, held | the contested ring on screen 28 frames (4 today); the stop line three times as thick and always across his path | none (presentation) |
| The modes' re-tune (A173) | exported, held | Sniper passes; Bruisers distinct again; Fog of War and Cage no longer a good player's walkover | measured at hunt 8: pin or re-measure with the dial |
| The arena's seventh evolution (A172) | exported, held | device B's lamp and cords; Glass House proposed on the clay rig | none (presentation) |
| The half-second screen, the pickup that settles, the three's chaser | on main or measured, off | see BRAIN-4, MOVEMENT-7, BRAIN-5 | the pickup: +5.8 / -9.0 / -5.0: the one rule that takes pro and all_star down |
All_star on the regular rink has no profile lever left (checks, speed, red skill, pressure, the commit distance: flat or the wrong way): the brain track's.
