# STORYBOARD-H17: the shipped build, played and photographed (lane A48, device A, 2026-11-12 22:30 to 23:05 CET)

What the operator sees when he starts `Game/Packaged/Hokej_Hokej_H17/Windows/ThirdJam.exe` (the Shipping stage of H17), not what PIE shows. Every still below is a
1920 x 1080 capture of the Shipping exe's client area, taken every 2 s by `tools/qa/storyboard/capture.ps1` (`tools/qa/out/storyboard/<run>/shots/t###s.png`,
JPEG q88 copies in `docs/experiments/img/storyboard/`). No editor, no PIE, no Development stills: see section 0 for why the second half of the brief's list is missing.

## 0. What the Shipping exe does and does not allow (the facts the brief asked for first)

| Lever the brief names | In `Hokej_Hokej_H17` Shipping | Where it is decided |
|---|---|---|
| `-player=regular` (the playtester as the human) | **does not exist in a running match in any build**: `-player=` is an argument of the tournament commandlet (`JamTournamentCommandlet.cpp:121`); `JamPlaytester` never sits in the human's slot of `AHokejGameMode` | `Game/Source/JamCore/AI/JamTournamentCommandlet.cpp`, `JamPlaytester.h` line 11 |
| `jam.ScriptHuman 1` (the scripted human) | **compiled in** (a `TAutoConsoleVariable`, no `UE_BUILD_SHIPPING` guard in `HokejGameMode.cpp`) but **unreachable**: Shipping has no console, and `-ExecCmds=` / `-dpcvars=` are not applied by a Shipping engine (the human's skater 7 stood at centre ice for 230 s with both on the command line) | `HokejGameMode.cpp:37`, `:536` |
| `-log`, `-abslog` | **no log at all**: `ThirdJam.Target.cs` sets no `bUseLoggingInShipping`, so `UE_LOG` is compiled out; `tools/run/run-packaged.ps1` already notes "Shipping writes no log" | `Game/Source/ThirdJam.Target.cs` |
| `HighResShot`, `jam.playcut`, `jam.pausemenu`, `jam.menuscreen`, `jam.Season.*`, `jam.livecut` | console commands: **unreachable** in Shipping (same reason); the pause menu and the Director's Call open by **ESC** and the menu buttons, which the capture script drives by keys | `JamDesignerConsole.cpp`, `DirectorsCallController.cpp`, `MenuGameMode.cpp`, `Progression.cpp` |
| a map on the command line (`ThirdJam.exe /Game/Maps/Rink -cut=...`) | **ignored**: the film-slate menu boots every time (`GameDefaultMap=/Game/Maps/Menu`); the match is started with ENTER on PLAY THE SEASON or CHOOSE YOUR CUT | `Game/Config/DefaultEngine.ini:3` |
| `-cut=<path>` | **honoured**: `AHokejGameMode::InitGame` reads the command line before the menu's `cut=` URL option, so PLAY THE SEASON loaded `tools/qa/storyboard/hockey.regular.rookie.cartridge.json` (the shipped `hockey.regular` with `ai.difficulty` rookie instead of pro) | `HokejGameMode.cpp:78` |

The Development fallback the brief allows was tried and does not reach the ice tonight: `tools/run/build.ps1 -Target Game -Config Development` in the worktree built in 96.5 s
(`.claude/worktrees/a48/Game/Binaries/Win64/ThirdJam.exe`), but a Development *exe* without a Development *cook* refuses to start (`Failed to initialize ShaderCodeLibrary ...
part of the Global shader library is missing`, `tools/qa/out/storyboard/dev-rookie/game.log` of 22:45), and the same exe placed over the Shipping-cooked content stops at
`Plugin 'StreamlineNGXCommon' failed to load` (22:47). A Development package (`BuildCookRun -clientconfig=Development`) is a 15-minute step that was outside the 75-minute box;
it is the first thing to run tomorrow (finding 1). Consequence: **the stills below show the shipped build with the human standing still** (a first-time player who does not touch
the pad), which is exactly what the operator's first ten seconds look like, and nothing of a carry, a pass, a shot by the human, a goal, a check on the human, a knock-down, a
pickup or a result card. Those seven stills of the brief's sixteen are open (section 2 lists them with the capture plan).

## 1. The runs

| Run | Exe | What | Stills | Keys sent (`capture.log`) |
|---|---|---|---|---|
| `ship-menu` | Shipping | boot to the menu, 14 s | 7 | none |
| `ship-rookie` | Shipping | `-cut=<rookie copy of hockey.regular>`, ENTER at 8 s, 270 s; ESC at 150 s (pause), DOWN ENTER at 156 s (Director's Call), ESC at 164 / 166 s | 115 (t002 to t230) | the 150-166 s keys landed while a browser window of another session sat in front of the game (22:42:50 to 22:43:30); the TIME OUT menu is in t164, the Director's Call screen is not |
| `ship-screens` | Shipping | DOWN x4 ENTER (settings), ESC, DOWN ENTER (cuts), ESC, ENTER (season take 1) | 23 | the 22:40 run (operator away) gave the cuts screen and the season card; its settings stills were overwritten by a 22:44 re-run that File Explorer covered, so **the settings screen is missing** |

The capture script now sends keys only when the game window is the foreground window and reads stills with `PrintWindow(PW_CLIENTONLY | PW_RENDERFULLCONTENT)`, so neither
happens again (status/A48.md H12.9).

## 2. The storyboard (nine stills in the order a first-time player meets them; seven of the brief's sixteen open)

| # | Still (`docs/experiments/img/storyboard/`) | Run, frame | Sim event behind it | What the player should notice |
|---|---|---|---|---|
| 1 | `a48-01-menu.jpg` | ship-menu t012 (12 s after launch) | none (Menu.umap) | The film slate: PROD. HOKEJ, SCENE 3 v 3, TAKE YOURS; the yellow PLAY THE SEASON: TAKE 1 OF 8 is the highlighted default; "Take 1 of 8: The Read-Through" under the list. The STICK / ARROWS move, A / ENTER select, B / ESC back line is cut by the slate's bottom edge and the top checker strip bleeds over the window edge at 1080p (finding 4). |
| 2 | `a48-02-cuts-library.jpg` | ship-screens t020 (CHOOSE YOUR CUT) | none | The Designer's offline library as film strips: REGULAR CUT "Night Rink: 90 x 45 m, fast, first to 3, body checks", RAW CUT "Raw Night: 90 x 45 m, no rules, first to 3, body checks", "6 cuts. Pick one to play it.", BACK. Two of six cuts fit the frame; the other four need scrolling, nothing says so. |
| 3 | `a48-03-season-match-card-hold.jpg` | ship-screens t026, 1 s into take 1 | `period` (take 1 built), faceoff countdown | MATCH 1 clapperboard with COACH'S NOTE: "Take one. Nobody is judging yet: find the net twice." (first to 2), the countdown ring with HOLD under the digit, the pill "press shoot at the drop, not before", the commentator's "Lights are up, the ice is fresh. First period is under way." and the controls strip MOVE WASD / L-STICK, SHOOT SPACE / A, CHECK SHIFT / B, BRAKE CTRL / LT, PAUSE ESC. The card covers the human's figure and the ring touches the PERIOD 1 - FACEOFF line (UI.md section 7 knew). |
| 4 | `a48-04-faceoff-won.jpg` | ship-screens t028 | `faceoff` `won` (the human's centre took the draw while the human held still) | WON in the human's blue inside the ring, the human's team-mate skating left with the puck glow, the bark "Nice stance. Shame it's the only part that moves right." over the far net. The player learns the draw is won without pressing anything at rookie, which contradicts the pill he just read. |
| 5 | `a48-05-match-card-regular.jpg` | ship-rookie t010, frame about 60 | `period` (hockey.regular at rookie, FIRST TO 3), countdown at 1, HOLD | The same card on the regular cut: PERIOD 1: FIRST TO 3 on the scoreboard, the far-net diamond "49 m" and the own-net diamond "own 34 m" with its left chevron, the mini radar bottom right, the "ESC / START DIRECTOR'S CALL READY" pill bottom left. The clapper covers the human completely for 150 frames (finding 3). |
| 6 | `a48-06-you-tag.jpg` | ship-rookie t012, about frame 180 | `faceoff` `drop` just happened; the human stands on the dot | The YOU tag is projected 1.9 m over the human's helmet, which at the F1 camera (6 m back, 3.5 m up) lands on the **orange centre standing behind him**: in this still YOU reads as the opponent's label (finding 2). The team-colour ring under 7 is the only unambiguous "yours". The bark bubble sits across the far-net marker "49 m" (finding 5). |
| 7 | `a48-07-far-net-marker.jpg` | ship-rookie t034, about frame 1500 | play in the far zone, the human untouched | The game the operator sees if he does not move: 7 on the dot inside his ring, the far-net diamond and distance, the own-net chevron, one orange skater crossing at 10 m, the puck nowhere in the frame (the radar is the only place it exists). The ring is the countdown ring's reticle art, not a plain ring (UI.md section 7). |
| 8 | `a48-08-bark-and-commentary.jpg` | ship-rookie t052, about frame 2600 | a bark (`check_received` tone, speaker far) and a commentary line | The bubble "You saw me coming and still chose that. Fascinating" drawn at the top of the frame over the far net with its speaker 40 m away, the commentary strip empty this frame; what a first-time player reads is a sentence with no visible speaker. |
| 9 | `a48-09-pause-time-out.jpg` | ship-rookie t164, ESC at 150 s | pause (`PAUSED` phase), Director's Call ready | TIME OUT, PERIOD 1, RESUME / DIRECTOR'S CALL / RESTART / QUIT TO MENU, "Your call is ready: change one thing about this match.", the ESC / START resume, STICK move, A select, B back line; the bark "You skate like the plan is to be a spectator" and the commentary "Used the boards like a teammate." stay on screen under the pause card. |
| 10-16 | open: faceoff DROP, a carry, a pass received, a shot, a goal with the commentator line, a check landing with the camera punch, a knock-down and get-up, a pickup, the Director's Call card, the far net from the end boards, the win or loss card | | | need a moving human: a Development package with `-ExecCmds="jam.ScriptHuman 1"`, or a Shipping build where the scripted human can be switched on from the command line (section 0). Capture plan is ready: `capture.ps1 -Exe <dev exe> -GameArgs '-cut=... -ExecCmds="jam.ScriptHuman 1"' -Keys '8|{ENTER}','150|{ESC}','156|{DOWN}{ENTER}'` plus `measure.mjs` on the log. |

In 230 s of the rookie run with the human still: score 0:0 throughout (the bots do not score 2 against 3 at rookie in four minutes), no goal, no win or loss card; barks seen
in the stills: "Nice stance. Shame it's the only part that moves right.", "You saw me coming and still chose that. Fascinating", "Pucks come to us, they run away from you",
"Enjoy the view from behind us, it never changes", "Your line is the practice squad. Ours is the real thing", "You skate like the plan is to be a spectator" (six, all
from orange, all mocking a player who has not touched the pad: the first thing the shipped game says to a newcomer is contempt, finding 5); commentary lines seen:
"Welcome to the rink: blue against orange, first period.", "Long carry up the ice.", "Long surge attempt.", "Used the boards like a teammate.", "Banked it in off the
boards!", "Lined one off the wall.", "Wrapped around the boards.", "Orange lets it go!", "Blue on the break, nobody back!", "Blue lets it go!" (ten distinct, read from the
strip in the 2 s stills; none announced a goal because none happened).

## 3. Eyes (tools/eyes, Gemini; advice, not instructions; `docs/experiments/eyes/a48-*.json`)

Four calls, 22:49 to 22:50, $0.087 in all: `readability` (flash, `--no-think`, `--distance 6`: the F1 camera sits 6 m behind the human) on the four play stills, `hud` (flash,
six stills, one retry), `bug-hunt` (flash, six stills), `p2-bar` (pro) on still 6, the best play still. Scores move by one between calls; read the sentences.

| Still | readability (overall; puck / carrier / colours / far net / strips) | hud (legibility; smallest text; collisions it names) | bug-hunt (max severity; what) | Finding? |
|---|---|---|---|---|
| 1 menu | | | 2: the controller prompt clips through the slate's bottom border | with still 2: finding 4 |
| 2 cuts library | | | 3: the stat lines "speed 11 m/s shot 13 m/s ..." clip into the sprocket holes of the film strip; 2: the prompt clips the slate's border | finding 4 |
| 3 season match card | | 3; 14 px; the clapper blocks centre ice and the opposing centre, the coach's note and the reticle overlap the human's upper body, the own-net marker sits on the left margin | | finding 3 |
| 4 faceoff WON | | 3; 14 px; the card blocks the puck and the orange centre after the draw, the coach's note overlaps the helmet, the bark bubble covers the boards and the far net; missing: who won possession, where the puck went | | finding 3, 5 |
| 5 match card regular | 2; 1 / 1 / 4 / 3 / 4 (first look: "giant central Match 1 clapperboard") | 3; 14 px; the card obstructs the centre, PERIOD 1 - FACEOFF collides with the reticle's top, the reticle overlaps the torso; missing: which side is the player's on the scoreboard | **4**: the banner, the reticle "1 HOLD" and the tooltip stack into an unreadable pile | **finding 3** |
| 6 YOU tag | 4; 4 / 4 / 5 / 4 / 5 (first look: "blue player back with blue target reticle"; it reads the orange skater as "labeled YOU ... carrier") | 4; 14 px; "YOU tag is positioned above the orange opponent skater rather than the foreground blue player", the bubble obscures the far net, the puck glow is low contrast on light ice | **4**: the YOU tag over the opposing orange skater | **finding 2**; p2-bar below |
| 7 far-net marker | 3; 1 / 1 / 4 / 4 / 5 (noise: "oversized blue reticle under the foreground skater") | | 3: "the foreground skater has lost its stick mesh" (wrong: the stick rests across the body at idle and reads black on black; R-0021's "the stick must read" is still true at 6 m from behind) | finding 2 (ring art), note for B |
| 8 bark + commentary | 3; 1 / 1 / 4 / 4 / 5 (first look: "commentary banner below the scoreboard") | 4; 14 px; the bubble occludes the far net, the 49 m marker is wedged between the bubble and the net, the own-net marker pinned on the left edge; missing: puck location when play is far, the controls bar during play (it is not: the strip is at the start only) | 3: the bubble covers the lower half of the scoreboard banner; 3: stick mesh (wrong, as above) | **finding 5** |
| 9 pause TIME OUT | | 3; 13 px; the modal covers centre ice and the far-net marker, the commentary capsule and an "own 1 m" marker collide with the helmet, the bark bubble persists over the pause menu | | finding 5 (bubbles during pause) |

`p2-bar` on still 6 at 6 m: **5 / 10**. Lead: "the toy proportions and the basic palette work; the flat global lighting and uniform outlines kill the painted-cel night".
Gaps in order: (1) pools: one big circular gradient on the ice instead of four soft pools with shadow between them; (2) outline: the ink lines do not thin with distance,
far figures become black blobs (EYES.md wrong 1: it quotes the bar, it cannot see the scaling; the far figures in stills 7 and 8 do read as blobs at 40 m, so the sentence
stands as a pixel observation); (3) construction: the boards carry a yellow top and bottom line, not the ochre kick plate, dark rail and team-colour trim of the bar. Works:
the chunky toy proportions and the blue / orange split, the glowing puck disc, the far net findable at once, the night dome above the silhouette line.

Wrong or not ours in the reads: "missing: a period countdown clock" (first-to cuts have none); "missing: which side is the player's on the scoreboard" (fair, the YOU ring
is blue and the left tile is blue, nothing says so); "missing: the controls bar during play" (the strip shows for the first 300 frames by design); the stick mesh (above).

## 4. Measurements

The Shipping exe writes no log (section 0), so the log measurements of the brief (frames, the human's touches, shots, passes received, one-timers, checks, barks and
commentary counts, `step_ms`, `JAMFX missing`, `missing usage flag`, `LoadErrors`, ensures and warnings) could not be taken from the shipped build. `tools/qa/storyboard/measure.mjs`
is written and tested against the format of every line it needs (`JAMEVENT`, `JAMSIM frame= ... step_ms`, `JAMBARK`, `JAMVO`, `JAMFX missing`, `JAMVO missing`, `LoadErrors`,
`Ensure condition failed`, `Warning:`) and runs on the first Development log: `node tools/qa/storyboard/measure.mjs <game.log> --human 0`.

What the stills and the capture log do measure:

| Measure | Value | Source |
|---|---|---|
| Launch to menu | 10 to 12 s (t008 black, t010 menu) in both menu runs | `ship-menu`, `ship-screens` capture.log |
| Menu ENTER to the first match frame | 2 s (ENTER at 8.1 s, MATCH card in t010) | `ship-rookie` |
| Match length captured | 230 s of play at rookie, score 0:0, no result | `ship-rookie` t010 to t230 |
| Human touches / shots / passes / checks | 0 / 0 / 0 / 0 (the human never moved: no scripted human in Shipping) | section 0 |
| Barks on screen | 6 distinct lines in 230 s, every one from orange at the still human | section 2 |
| Commentary lines on screen | 10 distinct in 230 s | section 2 |
| Capture cadence held | 2.0 s between stills over 115 stills (max gap 2.3 s, during a PNG save), no capture gap from the game: no hitch visible at the 2 s grain | capture.log timestamps |
| Window | 1920 x 1080 client area, windowed; the client rect is exactly 1080p (no scaling), 174 to 369 KB JPEG per still | capture.log |
| Frame time | not measurable (no `stat unit` without a console, no log); the GPU soak of FEEL.md stands as the latest number | |
| Errors | none visible (no ensure dialog, no crash in 3 launches and 5 minutes 40 s of running) | |

## 5. Five findings, ranked by what the operator should fix first

1. **The shipped build cannot be played by a script or measured at all** (blocks every proof of this lane and of A34's curve on the real build). Shipping has no log, no
   console, no `-ExecCmds`, and the scripted human is a cvar. Fix: `bUseLoggingInShipping = true` in `ThirdJam.Target.cs` for the jam (Shipping is for the judges, not a
   store) **or** package a Development configuration beside the Shipping one every hour (`tools/run/package.ps1 -Config Development`), and read `jam.ScriptHuman` from
   `FParse::Param(FCommandLine::Get(), TEXT("scripthuman"))` in `AHokejGameMode::InitGame` so it works in every configuration. One line each.
2. **YOU tag over the wrong skater** (still 6, severity 4 in the hud read). The tag is placed 1.9 m above the human's head and projected; at the F1 camera it lands where
   the opponent on the dot stands, so the first still of every match says YOU over an orange figure. Fix: draw the tag at the ring (under the feet, inside the ring's top
   arc) or clamp it to the human's own screen box (the helmet's projected top plus 24 px), never above a figure that is not his; and make the ring a plain ring.
3. **The MATCH card covers the human and the countdown for 150 frames** (stills 3 and 5). The player's first look at his own figure is a clapperboard; the PERIOD 1 - FACEOFF
   line touches the ring and the two pills under it stack over the ring's bottom. Fix: the card in the top band (0.12 to 0.42 VH, under the scoreboard) at 0.7 scale, the
   ring untouched; A18's slots already carry a `BannerBandPx`, so the card takes the band the bubbles are pushed out of.
4. **The menu slate does not fit 1080p** (still 1): the control hint STICK / ARROWS move, A / ENTER select, B / ESC back is cut by the slate's bottom edge and the top
   checker strip bleeds over the window; the cuts screen shows 2 of 6 strips with no scroll cue. Fix: slate height 0.86 VH with the hint line inside it, and "2 of 6, more
   below" (or arrows) on the cuts screen.
5. **The shipped game greets a newcomer with six insults and a bubble over the far net** (stills 4, 6, 8). With `rules.penalties` on and the human still, every bark in 230 s is
   orange mocking the player (`stall` / `check_received` tones), and the bubble is drawn at the top of the frame across the far-net marker when the speaker is 40 m away.
   Fix: no mock lines in the first 600 frames of the season's take 1 (the bark bank has praise and neutral lines; `JamBarks` cooldowns per tone), and a bubble whose
   speaker is off-screen or beyond 24 m goes to the commentary strip with the speaker's number instead of floating at the top.

## 6. Three things that work best

1. **The first ten seconds read as a hockey broadcast**: scoreboard 00 / PERIOD 1: FIRST TO 3 / 00 in the team colours, the far-net diamond with its distance, the radar, the
   controls strip in the player's own key names, and the commentator's "Welcome to the rink: blue against orange, first period." under the card. Nothing has to be explained.
2. **The P2 night look holds at 1080p in the shipped build**: the painted ice with its bands, the glowing board trim, the night dome with the lamp posts and the tree line,
   the toy figures with legible numbers (7, 21, 88) and clean outlines at 6 m and at 40 m; no missing material, no grid, no z-fighting in 150 stills (the bug-hunt read agrees).
3. **The pause card and the season framing**: TIME OUT with PERIOD 1, the Director's Call one line away with "change one thing about this match", the season's COACH'S NOTE
   ("find the net twice") and the cuts library as film strips; the film vocabulary stays in the menu and the cards stay hockey, as R-0021 point 3 asked.

## 7. Files

`tools/qa/storyboard/capture.ps1` (launch, capture every N s, timed keys, foreground guard, PrintWindow), `tools/qa/storyboard/sheet.py` (contact sheets),
`tools/qa/storyboard/measure.mjs` (log measurements), `tools/qa/storyboard/hockey.regular.rookie.cartridge.json` (the shipped regular cut at rookie),
`docs/experiments/img/storyboard/a48-0[1-9]-*.jpg`, `docs/experiments/eyes/a48-{readability,hud,bughunt,p2bar-you-tag}.json`, `status/A48.md`.
