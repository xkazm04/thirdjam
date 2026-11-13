# Lane QA2 report: the game on the clay tier as `dev-b` has it now (device B)

**Verdict**
- **Works:** every figure reaction that the sim asked for played, and every knockdown ended on its release, in un-frozen play; the clay look holds on the regular rink, Pond, the wide arena and five a side; the tier switch is clean; the log has no warning or error naming an asset of B's.
- **Worst:** play lives at the boards, and there the top third to half of the frame is black; and the scripted human is knocked down every 7 seconds while the figures at the boards run the same three reactions about three times a second.
- **Not checked:** sound by ear; the slide streak, the cross-check mark, the poke and a hard board crash in un-frozen frames (none was caught at one capture every 2 to 3 s); a packaged build.

- Tree `C:\Users\kazda\jam\wt\b-vfx` (`dev-b` 3fd4ec9). **No save, no git write, no cook.** Clock 17:32 to 17:48. Nothing in the tree changed besides this report and the stills.
- Editor on port 8000 (pid 25480). **It did not leave through `ExitEditor`: it crashed** at 17:44 when I stopped the play session after the menu test (`Assertion failed: false`, `PlayLevel.cpp` line 558, the engine's own check, after a level travel inside the session). The process is gone. All play was finished and nothing was open to save. Three `CrashReportClientEditor` windows are on the machine; I cannot tell which is mine and did not close any.
- Method: play in the editor on `Rink`; cuts were scratch copies of the repository's cartridges with only the tier set to `p3_clay` (outside the repository, deleted). The scripted human (`jam.ScriptHuman 1`) unless said. **All stills are un-frozen captures of the running game**, one every 2 to 3 s, with the HUD as the player sees it. The machine was loaded and play ran slower than real time, so the sessions are shorter in game time than their wall time.
- Sessions (game time): regular 104 s; a tier switch clay to night to clay 13 s; drill 6, 21 s and drill 2, 10 s (tackles and cross-checks close to the camera); the human idle, bots playing, 22 s; Pond 24 s; Bruisers 31 s (five a side by itself); wide arena 17 s; a scratch five-a-side regular 25 s; and the menu, then the regular cut chosen from it (night, as shipped), 12 s. 267 s of play on clay in all.
- Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\qa2` (`qa.sh` the match runner, `logana.mjs` the log reader, every capture under `img\`, each match's whole log slice under `logs\full_*.txt`).

## Faults, ranked by how visible they are to a player

| # | Fault | Visible | Whose | Where seen | Smallest fix I can name |
|---|---|---|---|---|---|
| 1 | **Black beyond the boards.** From the board top to the lit band of the barn wall there is nothing: when play is at the boards, the top third to half of the frame is black | 5 | B, the clay look (rig and barn) | most frames of every clay match at the boards; worse on Pond, where the space between the boards and the wall is larger (`qa2-dark-beyond-the-boards`, `qa2-clay-play-2`) | a dim fill light on the floor and lower wall outside the boards, or a surface there that takes the lamps' light |
| 2 | **The human is knocked down again and again.** Regular cut at `pro`: 15 knockdowns in 104 s, every one of them the human, 10 from the same bot from the same side; seven times within a second of being released. At 90 frames each the human lies or gets up for about a fifth of the match | 5 | the other device (bot tuning) | regular on clay; 3 in 25 s five a side; 2 in 31 s in Bruisers (`qa2-human-down-again`) | a bot does not check a figure that was released in the last second or two; spread the checking over the bots |
| 3 | **The same reactions over and over at the boards.** In 104 s six figures played 141 stick reactions (stick knocked up, or jarred), 59 pin braces, 49 board hits and 43 standing hits: one reaction every 0.36 s. A carrier loses the puck to a lift or a poke 1.4 times a second | 4 | join: the other device's event rates; B's figure plays every one (since SK17 the stick reactions are not thinned) | regular and five a side on clay | the other device: a carrier cannot be lifted or poked again for about half a second; or B puts a gap of about 0.3 s per figure back on the stick reactions |
| 4 | **Sounds by the log: the cross-check, poke and board sounds fire about 2.3 times a second together** (lift 0.76, poke 0.65, board 0.87 per second); the same event twice in one frame 5 times in 104 s (different skaters; never the same skater twice) | 4 (heard; not judged by ear) | sound binder and the same rates as fault 3 | regular on clay | falls with fault 3; else a minimum gap per event in the binder |
| 5 | **The first match showed the rink without its lights for about 3 s**: black sky, glowing boards, pale blue ice, washed figures, during the faceoff countdown; then the clay rig was in | 3 (seen once: the first clay match after the editor started; the log has 2.8 s from match build to "rig loaded"; later matches 0.2 s and clay from the first capture) | join: the other device loads the rig after the match is built; B's rig | the first session (`qa2-first-seconds-without-rig`) | hold the fade-in until the rig reports loaded |
| 6 | **After a foul whistle the fouled figure is still getting up for half a second after the sim released it**: the release came 30 frames early and the get-up played to its end | 3 (every knockdown that draws a foul, penalties on) | join: the other device releases on the whistle; B's figure finishes its take | drill 6 on the regular cut, log only (`SK6REL ... end_minus_rel_f 30`), not in a still | B: on an early release with the get-up running, finish it in a few frames; or the other device keeps the figure down for its frames |
| 7 | **A fast loose puck leaves a ladder of pale arcs behind its ring** | 3 | not determined: the puck's trail effect (B) or the ring (the other device) | Bruisers on clay (`qa2-puck-ring-arcs`); the pale green smear behind a carried puck is still there on night | shorten the trail's life or space its spawn by distance |
| 8 | **The attacker's cross-check does not show 7 times of 79** (its own check lunge outranks it) | 1 | B's figure Blueprint, by decision | regular on clay, log | none wanted (SK17 decision) |
| 9 | **The editor asserts when a play session is stopped after the menu was opened inside it** | not a player's fault | engine, editor only | the menu session | for QA: restart the editor after a menu test |

## What passed

**The tackle chain in un-frozen play**
- Knockdowns: 23 in the clay matches, each with its take and its get-up; **get-up end minus release 0.00 on 22 of 23**, the 150-frame knockdowns of Bruisers included; the 23rd is fault 6.
- Takes seen: `Knockdown_Right` and its spin, `Knockdown_Left`, the spin left, `Fall`, `Fall_B`, `Knockdown_Back`. The wall rule fired 5 times in the regular match and once in Bruisers.
- The lying poses in the un-frozen stills are right: on the ice, along the boards, nothing through the wall.
- Standing hits: 78 sent, 43 played, 5 blocked, 30 held back by the 0.6 s rule.
- Board hits: 49 of 49. Pins: 68 sent, 59 played, 9 refused (figure busy).
- Cross-checks: 77 clean, 2 misses; the victim's reaction played 75 times, none blocked or skipped (2 of the 77 I could not match in the log); the attacker's take 72 of 79; the mesh within 30 degrees of the victim in 54 of 75.
- Pokes on a carrier: 66 of 66 reactions.
- Nothing popped, floated or clipped in the frames I have. That is about 170 stills, not continuous motion.

**The figures otherwise**
- Skating, gliding, the ready stance, the faceoff stance and the idle human: right in the stills; stick in both hands, blade on the ice, the carried puck at the blade.
- Numbers read at the game camera from behind (7, 13, 9, 21). The human has a blue ring and a "YOU" tag at the faceoff, the ring after.
- Every clay match used `BP_SkaterVisual_TB1_P3`; no ghost figures after a tier switch (`ghosts=0`).
- Five a side: 10 figures, readable.

**The look**
- Calm pale ice with lines and circles, boards, kick plate, nets, lamps: no grey default material, no magenta, nothing blown out, on the regular rink, Pond, the wide arena.
- Tier switch clay to night to clay in one match: the right rig and the right figures 4 to 6 s after each switch (my first capture after each); the ambient light was there in those frames.
- From the menu on night: lit from the first capture, 2 s in; no fade or pop.
- The goal: orange sparks over the net.

## The log

- **Warnings and errors naming an asset of B's: none**, in any match.
- **`JAMFX missing`:** `NS_deke` and `MS_deke`, once in the session. Nothing else.
- **MetaSound or Niagara warnings or errors: none.**
- One warning in the menu session, caused by my own console use: `jam.Cut` set by code ignored after being set by console.
- Per match (game seconds; events from the sim):

| Match | s | lift | poke | board | check | knockdown | shot | goal | same event twice in a frame |
|---|---|---|---|---|---|---|---|---|---|
| regular | 104 | 79 | 68 | 90 | 15 | 15 | 32 | 0 | lift 2, poke 2, board 1 |
| five a side | 25 | 25 | 20 | 26 | 3 | 3 | 6 | 0 | lift 2, poke 2, board 1 |
| Bruisers | 31 | 0 | 0 | 2 | 2 | 2 | 2 | 0 | none |
| Pond | 24 | 0 | 0 | 11 | 0 | 0 | 1 | 0 | none |
| wide | 17 | 1 | 0 | 4 | 0 | 0 | 1 | 0 | none |
| human idle | 22 | 3 | 1 | 7 | 0 | 0 | 1 | 0 | board 1 |
| drill 6 | 21 | 3 | 1 | 3 | 3 | 3 | 1 | 1 | none |
| drill 2 | 10 | 6 | 1 | 5 | 0 | 0 | 4 | 0 | none |
| tier switch | 13 | 12 | 3 | 12 | 0 | 0 | 6 | 0 | lift 1 |

(The faceoff's won and lost pair shares a frame by design and is left out.)

## For the other device (not analysed, as seen)

1. The bots knock the human down every 7 s at `pro` and re-check it within a second or two of the release; no bot was knocked down in normal play.
2. Lifts and pokes take the puck 1.4 times a second; nobody keeps it. No goal in 104 s of the regular cut, 32 shots.
3. Play stays in the corners and along the boards; there the camera looks out over the boards into the dark and the open ice is out of frame.
4. The white distance labels ("87 m", "own 9 m") are small on the pale ice and one sat over the goal sparks; the commentary caption box floats over the play.
5. The control hint bar stays over the bottom of the frame for about the first 15 s of every match.
6. The human's ring is large and blue; among five blue figures it is the only mark, and in one five-a-side frame the camera's subject was not obvious.
7. The rig loads after the match is built: on a first load that is 3 s of an unlit rink in the faceoff countdown.
8. A foul whistle releases a knocked-down figure 30 frames early.
9. `NS_deke` and `MS_deke` are asked for and do not exist.

## Not checked

- Sound by ear; whether fault 4 is heard as too much.
- In un-frozen frames: the slide streak, the cross-check hit mark, the poke, a hard board crash, the check lunge at the hit. They last a few frames and my capture pace is one per 2 to 3 s; the frozen-frame evidence is SK18's.
- The goal celebration (one goal, sparks only; no figure reaction seen), the shot wind-up, passing, the faceoff draw: not looked at frame by frame.
- Pickups and rings on the ice: none seen in these cuts.
- Fault 5 in a package or from the menu on clay (the menu starts the regular cut on night; there it was lit from the first capture).
- Fault 6 in a still.
- Pond and Bruisers on their own tiers (they ran on clay, as asked).
- Motion: stills only, no continuous viewing.

## Stills (`C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`, each with `.txt`; all un-frozen)

- `20261113-174540-B-qa2-first-seconds-without-rig.jpg`
- `20261113-174540-B-qa2-dark-beyond-the-boards.jpg`
- `20261113-174541-B-qa2-puck-ring-arcs.jpg`
- `20261113-174542-B-qa2-human-down-again.jpg`
- `20261113-174542-B-qa2-clay-play-1.png` (open ice)
- `20261113-174542-B-qa2-clay-play-2.png` (the corner scrum)
- `20261113-174543-B-qa2-clay-play-3.png` (a goal)
- `20261113-174543-B-qa2-clay-play-4.png` (five a side, a cross-check at the boards)
