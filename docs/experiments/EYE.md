# The eye: the whole game looked at as the operator will look at it (lane A211, track E1, the night of 14 to 15 November)

Five lanes changed the game tonight and each measured its own piece with scripted hands. This lane changes no game source: it plays what the operator
plays (PLAY NOW from the menu's first screen, ROOKIE first, the regular rink, the game camera), films it off screen, reads the pictures as a stranger
would, and hands the Director a ranked list a lane can act on within the hour. The lists themselves (`EYE LIST n`, `SEEN FIXED` / `STILL THERE`,
`EYE READY`) are in `status/A211.md`; this file is the method and the ten stills that travel.

## The tools (`tools/qa/eye/`)

| File | What it does |
|---|---|
| `eye-match.ps1` | One editor-binary game process of the worktree it stands in (it refuses to run from the main checkout), `-RenderOffScreen`, fixed 60 Hz step, `r.RDG.AsyncCompute=0` on every run. Starts on the Menu map, takes the landing and HOME, picks the level on HOME's switch, presses PLAY NOW (`jam.menuscreen play`), and films the match the game's scripted human plays: a still every `-Every` sim frames (`jam.screen.StillEvery`), every sim frame inside `-Windows "a-b,c-d"`. `-Drills "23,2,1"` runs the contact drills in turn; `-HitCapture N` and `-HitCaptureBots 1` take lane A196's every-frame hit strips into the run's own `hits` folder. The game's process id is in `out/<run>/game.json`. |
| `eye-hits.ps1` | The same driver with its own still prefix, so a drill run can film beside a match run in one tree (the game names the periodic stills by tier and frame only: two runs with `-Every` above 0 in one tree would overwrite each other; run one at a time). |
| `sheet.py` | A contact sheet of a run's stills by sim frame: `python sheet.py <shots> <out.jpg> <from> <to> <step> [cols=3] [w=560] [crop=x0,y0,x1,y1] [prefix=...]`. Three columns of 560 px keep a skater at play distance about 13 px tall on a sheet: enough for who is where; use `crop` for the puck, a pose, a mark. |
| `passes.py` | What became of each pass, from a run's `game.log`: the next touch after every `JAMPASS` line (a mate, the passer himself, an opponent, nobody) and how many passes are touched again within 15 frames. |
| `facts.py` | The countable facts of a filmed match, one block per run, for the before and after of a list: score and length, faceoffs with a knockdown of the human inside 60 frames, his knockdowns, his hit presses and contacts, what became of his passes, of his side's and of the other side's (to a mate, to an opponent, back on the passer, touched again within 15 frames), every goal with the frames from its shot (53 = from the blue line), the camera's stands / no-horizon share, the stills with the yellow hint over the button pill. `python facts.py out/<run> ...` |
| `view.py` | What the game camera shows, without an eye: per still the share of ice-coloured pixels. "stands" = under 45 % ice; "top" = the top band of the picture is ice (no horizon). `python view.py <shots> 500 7200 20`. The camera lane's number. |
| `eye-hash.ps1` | The state hashes of one headless start (`-nullrhi`, the Rink map, `-cut=hockey.regular`, `jam.ScriptHuman=<n>`): the before / after proof of a script value in 25 s. A rendered run with value 0 is not repeatable (the machine's cursor aims his stick); a headless one is. |
| `grid.py`, `shift.py`, `steps.py` | Lane A210's sheet helpers, copied unchanged. |

## The method

1. **Two passes over one match.** The match is seeded by its cartridge, so PLAY NOW at one level shows the same frames on every start of one build.
   Pass 1: a still every 20 frames to frame 7200, every 60 after, and the log. Pass 2: the same match with every-frame windows at the events the
   log names (`JAMPASS`, `JAMSHOT`, `JAMRINK react goal`, `JAMVIS knockdown`, `JAMHIT`). A window costs about 7 frames a second of wall time.
2. **Read at play distance first.** Overview sheets at 40-frame steps for the whole match (who is where, what the camera shows), then the strips,
   then crops. A crop is never the first look: a pose that reads only in a crop does not read.
3. **Count, do not sample.** Every item of a list carries how often it was seen of how many situations looked at.
4. **Say what the instrument cannot do.** The rendered game's scripted human (`jam.ScriptHuman 1`, `AHokejGameMode::ScriptedHumanInput`) skates to a
   point behind the puck, passes once in ten seconds and shoots when lined up; it never checks, cross-checks or braces, and it carries the puck into
   its own net when it takes it facing home (4 of the 9 goals in the first three matches filmed). The person-like hand of lane A209
   (`JAM_PLAYTESTER_HAND=person`) lives only in the headless playtester. So a filmed match shows the bots, the puck, the camera, the HUD and the hits
   truthfully, and the score not at all. Hence `jam.ScriptHuman 2` (next section): from EYE LIST 3 on the films are played by the measured hand.

## `jam.ScriptHuman 2`: the measured hand in the rendered game (capture only; the Director's ruling of 15 Nov 01:12)

`Game/Source/JamGame/Private/ScriptHuman2.h / .cpp`, called from the top of `AHokejGameMode::ScriptedHumanInput()` only while the cvar is 2.
- **Who decides:** the headless playtester (`JamAI::FJamPlaytester`: the class the ladder is set with: reaction ring, aim noise, analog ramp, charge and
  release, one-timer, deke, sweep, cut, sprint, brake), built as `RunPlaytestMatch` builds it with no measurement arm set (the check press, the brace
  and the poke read follow the cartridge's switches). Lane A209's person hand (`FJamHitHand`: reaction 12 to 18 frames, an aim error of 10 to 20
  degrees on eight keys, early or late by up to 8 frames; body check 45 %, cross-check tap 40 %, the held brace 15 %) presses the hits.
- **Through whose buttons:** a person's. LMB is held while it charges a shot or holds the cross-check and tapped for a flat shot or a cross-check; RMB
  is held for a saucer and tapped for a pass or a body check; `FJamButtonResolver` with the shipped bindings and `JamControls::ButtonContext` turns
  each press into its verb by the possession of that frame, kills a key held across a change of possession, and starts a hold after the hand's
  thresholds: the path `JamControls::ApplyButtons` gives the keys. A tap the playtester repeats every frame is pressed once, and again after 12 frames.
  The aim of a pass is the stick of the frame the playtester asked for it, held through the release step (a person keeps the mouse on the man
  through the click; without the hold half the hand's passes left along its carrying sweep: `kind=to_-1`).
- **Knobs:** `jam.ScriptHumanKind` (-1 by the level: rookie newcomer, pro regular, all-star good; 0 / 1 / 2), `jam.ScriptHumanHand` (1), `jam.ScriptHumanButtons`
  (1; 0 = the playtester's verbs straight into the input, the headless match's path), `jam.ScriptHumanSprint` (1), `jam.ScriptHumanSeed` (0 = the cartridge's).
- **Repeatable:** no wall clock, no engine randomness; two headless starts of value 2 give the same hashes frame for frame.
  Rendered, three pro runs of four and the rookie pair equal the headless match; one pro run won its third draw 19 frames late and played another
  match from there (cause not found): compare the `JAMSIM` hash lines of both passes before trusting a window.
- **What it does not do:** the ability on F (the playtester's rule for it lives inside `RunPlaytestMatch`), the obedience to the contested ring and the
  stop line (measurement arms, off by default there too), steering through the camera (its move is in the rink's frame; `jam.ScriptHumanCam` belongs to
  value 1). The pill's hold fill does not follow it (`GJamLastHumanButtons` is the keys' own).
- **Filming with it:** `eye-match.ps1 -ScriptHuman 2 ...`; the log's `JAMSCRIPT` lines say the class, the seed and what the hand pressed.

## The ten stills (`docs/experiments/eye/`), from EYE LIST 1

| Still | What it shows |
|---|---|
| `01-faceoff-check-rookie-f135-168.jpg` | ROOKIE, the first puck drop: the other centre's body check lands on the human 44 frames after the drop; he is down. Also the best strip of a hit that reads: wind-up, lean, burst, fall. |
| `02-third-faceoff-down-and-2m-passes-f4944-5032.jpg` | The third drop of the same match: down again for 150 frames, while three orange skaters two metres apart pass the puck seven times in 40 frames. |
| `03-carried-puck-hidden-pro-f2140.png` | PRO: he carries the puck (the pill says SHOOT / PASS) and the puck is behind his hips; the net at the far end has nobody in it. |
| `04-hint-over-the-button-pill-f1620.png` | The yellow "ESC / START  RULE CHANGE READY" painted over the button pill at the bottom left: only "CK" of CHECK shows. From the first goal to the end of every match filmed. |
| `05-goal-from-the-blue-line-pro-f2982-3042.jpg` | PRO: a goal shot from just outside the blue line, about 28 m, unpressed; the puck is a 3-pixel dot. |
| `06-camera-on-the-stands-f3860-4300.jpg` | Twelve stills over seven seconds in his own end: nine show boards, crowd or a straight-down view. |
| `07-held-cross-check-chase-f302-456.jpg` | The held cross-check sprinted up the carrier's back for 160 frames: no hit, the carrier scores. |
| `08-tapped-cross-check-stick-out-f1351-1384.jpg` | A tapped cross-check that missed: the stick stays straight out to the left for 33 frames and more, the same figure as the held brace. |
| `09-rings-f3980.png` | Four kinds of ring and a downed man's bars in one picture. |
| `10-cross-check-that-reads-f1839-1872.jpg` | His tapped cross-check landing: the burst, the man down, the bars round him. Keep this. |

## Facts for the next agent on this track

- A start costs about 30 s to the first match frame; a rookie match 120 to 200 s of wall time at a still every 20 frames; no first-match GPU death in
  the first 10 starts with `r.RDG.AsyncCompute=0`.
- The build tool's three slots were all taken for nine minutes at 00:25: build before you need it. No game process of the tree may run while it links.
- `Game/Saved/Screenshots/WindowsEditor` of the tree is the game's still folder; the drivers move the stills out as they are written.
- Bulk output is under `tools/qa/eye/out/` and never lands.
