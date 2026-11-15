# Video script v2, rewritten against main 41df3e6 (lane A142, track D1, 2026-11-13 night): 48 s, for the capture lane and device B

> **Lane A180, Sat 14 Nov 08:00: the shot table below is superseded by `SHOTLIST.md`** (drill 22 now stages the nudge and the hit; the package is H30 or its successor; the stands, the level switch and the stop line exist; device B's edit is one command). The notes on what a Shipping and a Development package can stage still hold.

> **Lane A225, Sun 15 Nov 16:40 (pass 5):** this table is history. Its menu words (CHOOSE YOUR CUT, the cards, TIME OUT, the Director's Call) and the drawn marks it mentions are gone from package H43; the shots are in `SHOTLIST.md` section 3.

Truth rows: `submission/TRUTH.md` VS01 to VS15. Marks: `[waits: X]` = the shot is only filmable when X lands or delivers.
Rules (docs/PLAN.md section 2): 1920x1080 or higher, 30 to 60 s, theme (Director's Cut) visible in the first seconds, no logos or trademark words (captions are ours; engine and vendor names appear nowhere on screen). Voice: `docs/experiments/VOICE.md`, taglines `submission/tagline.md`.
Machine facts from A88: posted keys and mouse buttons reach the game window, the cursor cannot be posted in a Shipping build, recording is PrintWindow-based (`tools/media/take.ps1`), no audio device on this machine.

## Which package
**The next package (H27), not H26.** H26 predates the kept puck, the grace rules, drill 21 and the arena that answers a goal; R-0053 says no package is cut until the curve lane (A133) and the integration lane (A138) land. Every shot below is filmed on that package. Shots 5, 6 and 7 are the film's point and are filmed last.

## What is staged how
- **Shipping** (the build we submit): no console, no log. The cut is chosen with `-cut=<absolute path of the cartridge>`; menus by posted keys; the match plays by itself when the human is idle (the bots are real). The human stands still: a Shipping take cannot show a human's hit (the cursor cannot be posted).
- **Development** (a package of the same tree): `jam.ctrl.Drill N` stages a scene and logs `JAMEVENT` / `JAMCUE` / `JAMDRILL` lines with frame numbers. A88 measured the older Development ice darker than Shipping's: film Development takes only where no Shipping route exists, and say so in the take log (the operator's question O10 asks whether that is allowed).
- **Same seed, same frames, same state**: a moment found in a Development log by its frame can be re-filmed in Shipping by replaying the same cut with the same idle input and cutting at that time. This is the route for every "the bots do X" shot.

## Shots (48 s)

| # | s | what the viewer sees | caption | how it is staged today | waits for |
|---|---|---|---|---|---|
| 1 | 0-3 | the film-slate menu, the HOKEJ clapper | HOKEJ. THE DIRECTOR'S CUT. REAL HITS. | Shipping, `menus` take as A88's (`take.ps1`, 2 s after launch) | nothing |
| 2 | 3-8 | CHOOSE YOUR CUT: the twelve cards, a posted-key move across the rows | TWELVE CUTS. ONE DIRECTOR. | Shipping, posted Down / Right / Return as A88's `menus-H24` take; cards Pond, Cage, Sniper, Marathon, Power Play, Shotgun / Sudden Death, Outnumbered, Glass House, Arsenal, Bruisers, Fog of War | check the take for the stale "3 v 3" line A88 found [waits: capture lane] |
| 3 | 8-12 | five blue and five orange at the faceoff, the drop | FIVE A SIDE. NOBODY IS POLITE. | Shipping, `-cut=` `Game\Content\Cartridges\hockey.regular.cartridge.json` (5 v 5) or Bruisers (`modes\hockey.mode.bruisers.cartridge.json`, 5 v 5 on 80 x 40), idle human; count the skaters in the take (H24 spawned three) | nothing |
| 4 | 12-17 | a pass play by the bots: carrier skates, a teammate opens, the pass lands, a shot | THEY PASS NOW. | Shipping, regular, idle human, 90 s take, cut the window; find the window first in a Development run of the same cut and seed (`JAMEVENT pass`, `contracts/events.md`) | nothing |
| 5 | 17-23 | **same direction**: two skaters glide the same way and touch; both keep their feet. Then **head-on** at full speed: flattened, the hold, the kick. One split frame, the same press twice | SAME WAY: A NUDGE. HEAD-ON: FLATTENED. | **No console drill stages the same-direction half.** `Jam.Core.Contact.SameDirection` is a test, not a drill, and drill 5 (a check from the side at 3 to 6 m/s) is a stumble, not a bump (REVIEW-1 section 4). Head-on half: Development, `tools/qa/ctrl/a73-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 6` (the human carries into the nearest standing pro bot at full speed). Same-direction half: the operator plays it by hand (skate beside a bot at its speed, press the right button), or a drill is written | a drill for the same-direction half [waits: track F1/F2] or the operator's hands (O9) |
| 6 | 23-30 | **stop and go**: the human carries at a standing pro defender, brakes hard, the defender flies past, the human cuts and bursts away, the shot | STOP. THEN GO. | Development: console `jam.ctrl.Drill 21` (`jam.ctrl.CarryScript 1`: the scripted human carries at the defender 12 m ahead, stop begins at 9.5 m, cut 60 degrees, burst; 20 rounds, `JAMDRILL carry` lines; `jam.ctrl.CarryScript 0` is the straight charge for contrast; `2` leaves the keys to you); `HokejGameMode.cpp:73-82`. Cut the best round | the spray and the brake sound are device B's BF1/BF3 [waits: device B]: without them the shot shows the stop and the pass, not a spray |
| 7 | 30-35 | **an ability**: F in the Shotgun cut, a blast through three bodies, the lane goes down like pins | F: SHOTGUN. | Development, `tools/qa/ctrl/a84-prove.ps1` (drill 10 on `modes\hockey.mode.shotgun`, F posted on every `JAMCUE` shot); the bark "One blast and nobody is left standing." is in `Game/Content/Commentary/abilities.commentary.json` | the ability's burst and sound [waits: device B, BF3] (the binder logs one missing effect, `NS_ability`) |
| 8 | 35-39 | **the Director**: Esc, TIME OUT, the Director's Call over the paused match, one slider moved, the scoreboard says the new rule | ONCE A PERIOD, THE DIRECTOR MOVES ONE DIAL. | Shipping, posted Esc in a match (A88's `menus-H24` at 36.0 s); the card reads "One change per period. Move one slider, hit APPLY." | the hint line clipped by the slate border (A88) [waits: U1, A135] |
| 9 | 39-44 | **a goal**: the human scores, the red lamp over the net pulses, the net shakes, "Blue scores!" | BLUE SCORES. THE RINK ANSWERS. | Development, `tools/qa/ctrl/a91-prove.ps1 -Drill 10` (shots from 10 m, all three charges scored in the acceptance run); the lamp and the net are `ARENA.md` section 20 (`jam.rink.react` default on) | nothing; a horn is not claimed |
| 10 | 44-48 | end card: HOKEJ, the tagline | HOCKEY WITH THE WHISTLE TAKEN OUT. [waits: A133] (if A133 does not land: REAL HITS. THE SPEED YOU BRING IS THE HIT YOU GET.) | generated (`tools/media/cut.sh` end card), B's logo if delivered | the logo and banner from device B [waits: device B] |

Total 48 s. Shots 5, 6 and 7 are the three things no other hockey game says on screen; the review of the design eye (`docs/design/REVIEW-1.md` section 4) ranks them in that order of what the film must show.

## What to cut first if time runs out (in this order)
Shot 4 (the pass play), shot 2 (the cards), shot 9 (the goal), shot 8 (the Director). Keep 1, 3, 5, 6, 7, 10 (about 30 s). The review says: keep the stop-and-go, drop the faceoff if a slot is needed.

## What device B must capture, with sound (BS1; A-to-B-0020 point 5 is still open)
The take on this machine is silent: no capture device (dshow lists only a virtual camera). The game's own audio is the half of the film that sells it. Device B, on the next package, 1080p at 30 fps, system audio:
1. One whole minute of regular play, five a side, a human playing by hand: this is the "real" film if it is good; the staged shots above are the fallback.
2. The sounds a staged take cannot be trusted to carry: a body check landing, a goal (horn, lamp, the booth), a shotgun blast, the booth's contact lines (72 of 412 lines have no recording, R-0051, so a silent caption is expected: say which ones).
3. The same-direction nudge and the head-on, played by a human beside each other, if the operator cannot do it (O9).
If B cannot: A's silent cut with the commentator and music muxed in from the cooked wave files (the claim "audio from the game" stays true only if they are the cooked assets used in the take).

## Edit
`tools/media/cut.sh` takes a table of `take, in-point, length, caption` rows (A88's v1 table in `cutlist.md` is the template). Output `submission/video/hokej-v2.mp4`, 1920x1080, 30 fps, H.264 + AAC. v1 (`hokej-v1.mp4`) stays as the fallback until v2 is checked: no logo or trademark word on screen, theme visible in the first seconds (the caption of shot 1 and the slate say Director).
