# SCREEN-TELLS: the screen says what the game now is (lane A213, track U3, the night of 14 to 15 November)

The operator played H34 at 20:10 and wrote "the game itself does not look bad, but effectively is unplayable" (R-0067). Five lanes changed how the
game plays tonight (A206 brain, A207 pass and shot, A208 possession, A209 contact, A210 hits). This lane changes no rule: it makes the screen say
the game it now is, and nothing else. Presentation only; the sim's hash is untouched (no sim file is edited).

## 1. The rules the screen must tell (from the lanes' READY sections on main 2971a99e)

| Button | With the puck | Without the puck |
|---|---|---|
| LMB | shoot: a tap is flat, a hold lifts it; the shot is twice as fast as yesterday (A207) | cross-check: shoves any man in front, the carrier loses the puck on the press; held from 13 frames: the brace (A209, A199) |
| RMB | pass: a tap is short and crisp; the longer the hold the harder, full at 30 frames (half a second), a third of the rink; it goes to the mate nearest the stick's line inside 25 degrees, led to his blade, else along the stick; it lifts by itself over an opposing blade: no saucer to learn (A207) | body check: 8 frames of wind-up; he is pulled up to a metre onto a man in front within about 2.5 m (A209) |

A button held while the puck changes hands does nothing until released and pressed again; a held pass or shot dies when he loses the puck (A199).
A loose puck that reaches a skater's stick arc or body is his; a carried puck is taken only by a press: poke (Space), a check, a cross-check (A208).

## 2. Inventory (item 1)

Stills: `tools/qa/screen/out/a213-inv*/` (PLAY NOW, rookie, the regular rink, scripted human, 1920 x 1080 and 1280 x 720, off screen, the
screen of before with `jam.hud.Possess 0`, `jam.hud.Words 0`). The rule: nothing covers the puck, the carrier or the net; nothing lies; at most one
line of words on screen at a time outside the scoreboard and the buttons' badge.

| # | Element | What it says, when | True tonight? | A newcomer needs it? | Verdict |
|---|---|---|---|---|---|
| 1 | Scoreboard (two team plates, PERIOD n / FIRST TO n) | always | yes | yes | keep |
| 2 | MATCH n card (PERIOD - FACEOFF - ROOKIE), DirectorsCall widget | the first seconds of a match | yes | yes, once | keep (not a HUD line: it stands over the stands, not the ice) |
| 3 | Faceoff ring (digit, HOLD, DROP, WON / LOST) | every faceoff | yes | yes | keep |
| 4 | Faceoff hint "press shoot at the drop, not before" | the first 3 faceoffs of a session | yes | yes | keep; yields to the controls strip (rule of one line) |
| 5 | Controls strip (SKATE WASD ... SPRINT SHIFT) | the first 8 s of the first match | yes (verbs and keys only) | yes | keep words; it now stands alone (inv-before t001 had four lines of words at once) |
| 6 | Buttons badge (LMB SHOOT / RMB PASS, or CROSS-CHECK / CHECK) | always | **no**: RMB held 13 frames read SAUCER; there is no saucer under the pass's power | yes | **fixed** (A213, item 4): stays PASS |
| 7 | YOU chevron (take-yellow, over the helmet) | always in play | yes | yes | keep |
| 8 | YOU tag | faceoff countdown | yes | yes | keep |
| 9 | Your ring (team colour, 1.1 m) | always | yes, but it said nothing about the puck: the same ring with and without it (inv-before t027: he carries, the puck hidden behind his skates, only the badge tells) | yes | **fixed**: doubled while he carries |
| 10 | Carrier ring (thin white, 0.8 m) | any carrier but him | true, but a mate and an opponent looked alike | yes | **fixed**: in the carrier's team colour |
| 11 | Take grace ring (yellow, 1.5 x closing over 30 frames) | each pickup | true, but yellow hid who had it for the first half second (inv-before t009, t014, t020: a yellow ring under a mate, under an opponent) | the moment, yes; the yellow, no | **fixed**: the closing ring kept, in the carrier's colour |
| 12 | Loose puck | nothing marked it | - | yes (R-0067 point 2) | **added**: a chalk ring round the puck, 40 px least |
| 13 | Contested ring (A166, opponent colour, pulsing) | a loose puck he and an opponent reach head on | yes | yes | keep (the loose ring yields to it) |
| 14 | Stop's distance line (A166) | he carries, an opponent closes | yes (law 3 holds) | yes | keep |
| 15 | Far-net diamond + distance pill, own-net edge chevron + "own NN m" | always | yes | the far net yes; "own NN m" on the left edge rarely | keep; propose: the own-net label only in the own third (it reads as clutter at 79 m) |
| 16 | Off-screen puck arrow + distance | puck off screen | yes | yes | keep |
| 17 | Radar (bottom right) | always | yes | some | keep |
| 18 | Stamina bar, ability plate, effects row | while they apply | yes | yes | keep |
| 19 | Kept prompt "THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT" | 5 s from his first carry, once a session | **no** after he lost the puck (inv-before t014: up while an opponent carries) | yes, while true | **fixed**: drawn only while he carries |
| 20 | Brake prompt "S AGAINST YOUR MOTION, OR HOLD CTRL: STOP, THEN GO" | the first time an opponent closes on him carrying | yes | yes | keep |
| 21 | Contact line (TOO SOFT, MISSED, JUST GOT UP, STICK LOCKED, JUST WON IT, BRACED HIT, RUN THROUGH ...) | after his press | yes (A209 kept the words' cases) | yes | keep; it outranks every other line |
| 22 | Commentator's caption (bottom) | most of the match | yes | flavour | keep; yields to every teaching line (it was a second and third line of words at the faceoff and under the prompts) |
| 23 | Bark bubbles (beside the scoreboard: "You skate like the plan is to be a spectator") | often | yes | flavour | not in the HUD widget (UJamBarkWidget); proposal for READY 2: yield to the HUD's one line too |
| 24 | Pass meter (A207: a line on the ice by the stick while RMB is held) | while held | yes | yes | judged in READY 2 (item 3) |
| 25 | Hit marks (A210: chevron, arc, contact ring, downed oval) | hits | A210's | - | not touched; see the report |
| 26 | Goal banner, end card, toast | as before | yes | yes | keep |

Nothing in the stills of after covers the puck: the loose ring is hollow round it, the rings lie on the ice under every element (z -1), the YOU chevron
ghosts over the puck (lane A123's rule). Seen in both sets: the net's distance pill stands beside the far net, never on its mouth.

## 3. Possession at a glance (item 2)

`Game/Source/JamGame/Private/HUDPossession.cpp` (UpdatePossession, one call line last in `UJamHudWidget::RefreshFromFrame`). From the SIM's carrier
(`UJamSim::CarrierIndex`), never the figure's flag (device B: figures are told they carry on frames the sim's carrier is nobody).

| State | Before tonight | Now (`jam.hud.Possess 1`) |
|---|---|---|
| you carry | your ring (team colour), the same as when you do not | your ring doubled: an outer ring 1.35 x, your team's colour; it stands at your feet, where the carried puck may hide behind your hips at this camera |
| a mate carries | a thin white ring | the ring in your team's colour |
| an opponent carries | the same thin white ring | the ring in his team's colour |
| nobody | nothing | a chalk ring (0.42 m) round the puck on the ice; not while the contested ring (A166) stands round it, not in a countdown, not on a downed figure |
| the pickup | the new carrier's ring yellow for 30 frames (the take grace), closing 1.5 x to 1 x | the same closing ring in the new carrier's colour (the grace's yellow said "grace" and hid who); a 14-frame pulse where no grace runs |

`jam.hud.Possess 0` = the screen of before. Proof: `jam.hud.PossessStills N` (a still N sim frames into each possession).

## 4. The words (item 4)

`jam.hud.Words 1` (default; 0 = the words of before). Changed in `Game/Source/JamGame/Private/InputMapping.cpp`:

| Where | Before | Now |
|---|---|---|
| the badge, RMB held 13 frames with the puck | `RMB  SAUCER` (there is no saucer under game.pass.power) | `RMB  PASS` (the line on the ice shows the power) |
| pause card, LMB line | `(WITH / WITHOUT THE PUCK: HOLD TO HIT)` | `(HOLD: LIFT THE SHOT / BRACE)` |
| pause card, RMB line | `(WITH / WITHOUT THE PUCK)`, rebound: `(HOLD = SAUCER)` | `(HOLD FOR POWER)` |
| CONTROLS, SHOOT | a tap is a flat shot; hold to load it, let go to fire | a tap is a flat shot; hold to lift it, let go to fire |
| CONTROLS, CROSS-CHECK | hold it and skate into a man: your speed is the hit | shoves the man in front off the puck; hold it to brace |
| CONTROLS, PASS | a tap is a flat pass; held, it is a saucer over the sticks | to the mate your stick points at; hold for power |
| CONTROLS, CHECK | the body check: come in fast or it is a bump | the body check: it takes you onto a man in front |
| binding table | PASS (WITH THE PUCK; HOLD = SAUCER), CROSS-CHECK (...; HOLD IT INTO A MAN) | PASS (WITH THE PUCK; HOLD FOR POWER), CROSS-CHECK (WITHOUT THE PUCK; HOLD TO BRACE) |
| the first-match strip | verbs and keys, true as it stands (145 of 146 characters: no room for the holds) | unchanged |

### The Controls text for PLAY-ME (to replace its Controls section's first lines)

Mouse aims the stick. **The two mouse buttons change with the puck.**
**RMB** with the puck: **pass**. It goes to the mate your stick points at (or along the stick when nobody is there). **Hold it for power**: a tap is a
short crisp pass, half a second is a hard pass across a third of the rink; the line on the ice by your stick shows how far, in your colour when it has
found a mate. It lifts over a stick by itself. Without the puck: **body check**: it takes you onto a man in front of you.
**LMB** with the puck: **shoot** (a tap is flat, hold to lift). Without it: **cross-check**: it shoves the man in front off the puck; **hold it** to brace.
A button held while the puck changes hands does nothing until you let go and press again.
**Who has the puck**: your ring doubles when it is yours; a mate's ring is your colour, an opponent's is his; a loose puck has a white ring of its own.
**Space** poke. **F** ability. **E** spin. **Wheel** deke. **Shift** sprint. **S against your motion** or **Ctrl** brake. **Esc** pause.
**The pad**: RT shoot / cross-check (hold to lift / to brace), RB pass (hold for power) / check; the rest as before.

## 5. The eye's lists (A211, EYE LIST 1 and 2), the screen's items

| Eye item | What the eye saw | Fix (switch; 0 = before) | Proof |
|---|---|---|---|
| 4b | from the first goal to the end of every match "ESC / START  RULE CHANGE READY" lay on the buttons' pill (only "CK" showed) | `DirectorsCallWidget.cpp`: the hint stands one row (52 units) above the pill and shows 4 s (240 frames) each time it becomes due (`jam.hud.ReadyHintBrief`) | `tools/qa/screen/out/a213-after2/a213-after2-720-t023.png` (above the pill, 2 : 0), t027 to t051 (the pill reads) |
| 12 | the pill said SHOOT / PASS while he lay on the ice | the pill at 0.3 opacity while he is down (`jam.hud.Words`) | gate (UpdateButtonsBadge); stills in READY 2 |
| 12 | the commentator calls the cross-check a "lift" | not the HUD: the cross-check is the sim's `bLift`, so the commentator draws lines of event `lift` (`briefs/commentary/lines.json` lift.clean.*; voiced text in device B's `Game/Plugins/JamAssets/Content/Audio/VO/commentary.json`) | finding for the commentary owner |
| 10 | the FACEOFF card stayed 2 s into play; the caption sat on the controls strip for 7 s | the card goes at the drop (`jam.hud.CardAtDrop`); one line of words at a time (`jam.hud.OneLine`, section 2) | READY 1 stills |
| 9, list 2 | four ring systems round one pair; five kinds of ring | the contested puck is one ring (the loose puck's place, the opponent's colour): its inner ring and the ring under the coming opponent go; the grace's yellow goes (the carrier's colour); kinds left: your ring (doubled with the puck), the carrier's ring, the puck's ring, the stop's line | READY 1 stills |
| list 2 | the caption under the scoreboard lay on the far net's distance label | the caption is not drawn on a frame its box meets a net, own-net or puck label or the prompt (`jam.hud.OneLine`) | gate; stills |
| 11 | the end card covers the match-ending goal on its own frame | READY 2 | |

## 6. Wave 2 (toward SCREEN READY 2)

| Item | What changes | Switch (0 = before) | Proof |
|---|---|---|---|
| EYE 11 | the win / loss card waits 1.9 s after the match-ending goal (by the clock: the sim's frame stands still in the end phase); the goal's celebration is seen first, then BEATEN / YOU WIN with its keys | `jam.hud.EndWait` | `tools/qa/screen/out/a213-w2b/a213-end-wait-*.png` (no card, the scorers' arms up), `a213-end-card-*.png`; Jam.Hud.Teach (EndCardWaits) |
| 3 | the held pass's target mate gets a ring in the team's colour; the held shot lays a line on the ice from the puck along the stick, 1.5 m at a tap to 4.5 m at the full charge, chalk, yellow when full: the same make as the pass line (0.22 m) | `jam.hud.ChargeLook` (proof `jam.hud.ShotLookDemo N`) | stills `a213-w2*`; Jam.Hud.Teach (the line only grows) |
| 5 | the pill teaches: "RMB  PASS: A MATE IS OPEN" when he carries and a full pass along his facing finds a mate 6 to 30 m ahead; "RMB  CHECK HIM" when an opposing carrier is within 2.6 m inside 60 degrees of his facing; yellow plate, ink type, a slow breath; three times a session each, 900 frames apart, never after he passed / checked | `jam.hud.Teach` (`jam.ui.teachreset`; proof `jam.hud.TeachStills`) | Jam.Hud.Teach (counts, never after, restart) |
| ruling 2 | the bark bubbles yield to the HUD's one line (a bubble is not drawn while a HUD line of words stands; the voice plays) | `jam.hud.OneLine` | BarkWidget.cpp, one line |
| ruling 1 / EYE 12 | the commentator never calls the cross-check a lift: the sim's lift event offers only the cross-check's lines (check.cross on a clean one; a miss plays nothing until B voices `check.cross_miss`) | `jam.comm.LiftIsCross` | Jam.Commentary.CrossCheck (tonight and before) |
| list 3, 1 | a pass is praised when a mate RECEIVES it ("Tape to tape" said for a pass that never left his stick): the pass event is held until his team's PassesReceived grows (3 s at most), else dropped | `jam.comm.PassOnCatch` | gate |
| list 3, 2 | a Director's mode plays the level lit on HOME (Glow Ice played PRO under ROOKIE): modes travel with ?level= as PLAY NOW does | `jam.menu.ModeLevel` | Jam.Menu.LevelWire (the mode step plays HOME's level) |
| list 3, 3 | the contact line's "JUST WON IT" (his press into the other man's take grace) reads HE JUST GOT IT | `jam.hud.Words` | Jam.Hud.CarrierCues |

## 7. Wave 3 (toward SCREEN READY 3)

| Item | What changes | Switch (0 = before) | Proof |
|---|---|---|---|
| play sheet | `docs/experiments/PLAY-ME-DRAFT.md`: the game in three lines, the controls table GENERATED from the live bindings (`jam.ui.playsheet <file>`, then `node tools/qa/screen/a213/play-sheet.mjs <file>`), his three points as they stand on main, five things to try with what he should see, known limits | - | the draft (15 rows from the bindings) |
| lines over his body | a hold's line (the shot's, and A207's pass line: its look) is drawn at 0.55 on a frame its box meets his figure's box, else at full strength. Picked over cutting the line at the figure: a cut line lost its start at the puck, which is where the eye looks | - (the hold lines' own switches) | `screen-tells/r3-01`, `r3-05`, `r3-09` |
| double ring | 1.25 x (was 1.35), the outer ring at 0.75 of the inner's strength | `jam.hud.Possess` | `r3-09` |
| far loose ring | past 22 m from the camera it is drawn in the HUD's yellow, not a darker ink: a dark ring vanishes on the neon rink's black ice, yellow reads on white, blue night and black | `jam.hud.Possess` | `r3-03`, `screen-tells/08` (before) |
| own-net label | "own NN m", its diamond and arrow only while he is in his own third, and never on the radar's frame (on Glow Ice it slid under the radar: "own 8 m" inside the radar) | `jam.hud.OwnThird` | `r3-02` |
| owed stills | the pass prompt firing (`r3-08`, a staged carry: ScriptHuman 0, drill 10, HandPuck); the pill dimmed while he is down: still owed (no knockdown of the human in the staged runs) | - | |
| neon and night | every element read on Glow Ice and on the night look: rings, lines, the pill, the teach plate, the loose and contested rings: all read (blue rings on the night look's blue ice read by their ink edge); fixed: the own-net label under the radar | - | `r3-01`..`r3-07`, `r3-10`; runs `tools/qa/screen/out/a213-{neon1,neon2,night1,night2}/` |

## 8. Wave 4: nothing on any screen lies about tonight's game (main 9e6815a0; the rules due by 08:30 in the last column)

Read against: A206 (bots), A207 (pass and shot), A208 (touch possession), A209 (contact), A210 (hits), and the Director's list of what lands by
08:30 (the faceoff: both centres at the dot, nobody felled for 90 frames after a drop; a pressed cross-check into a carrier's back shoves him; a held
brace on a carrier's back slows him and strips the puck; nobody scores on himself; a pass goes to the mate the stick points at).

| String | Where | Verdict | Note |
|---|---|---|---|
| "New to hockey games? Start here: rookies rarely hit." | HOME, under the level switch (`UJamMenuFlow::LevelMeaning`) | **fixed**: "... rookies are a step slower." | rookie bots check him about 8 times a match, a third land (A209); a level is easier by execution, not by not caring (NIGHT-COMMON) |
| "Played one before? Pros hit as a habit." / "The wall. All-stars hunt the puck. Beat pro first." | HOME | true | |
| "Five-a-side hockey. WASD skates, the mouse is your stick." / "The speed you bring is the hit you get." | HOME | true | the closing speed still makes the hit (A209 law 1) |
| Sniper card: "shot 26 m/s" | mode card facts (`modes/hockey.mode.sniper.cartridge.json` game.meta.facts) | **fixed**: "shot 52 m/s" | shot.speed_scale 2 (A207) doubles every shot: a tap leaves at 52 |
| the other eleven mode cards' pitches and facts | MODES | true | none names a shot speed, a saucer or a lift |
| the controls strip (SKATE WASD ... SPRINT SHIFT) | first match, 8 s | true | verbs and keys only |
| the pill (LMB SHOOT / RMB PASS, CROSS-CHECK / CHECK, BRACED; the teaching pills) | in play | true (READY 1, 2) | SAUCER gone under the pass's power |
| CONTROLS rows, the pause card, the binding table | menu, pause | true (READY 1) | "hold to brace" stays true with the 08:30 brace (it slows and strips on a carrier's back) |
| "press shoot at the drop, not before" | faceoff hint, first 3 faceoffs | true | the faceoff landing (centres at the dot, 90 frames without a felling) keeps the early-press rule; re-read at the landing |
| "THE PUCK IS YOURS UNTIL SOMEONE PRESSES FOR IT" | the kept-puck prompt | true | a carried puck is taken only by a press (A208); shown only while he carries (READY 1) |
| "S AGAINST YOUR MOTION, OR HOLD CTRL: STOP, THEN GO" | the brake prompt | true | |
| the contact line (TOO SOFT, MISSED, JUST GOT UP, BOUNCED OFF, NOT WITH THE PUCK, TOO SOON, STICK LOCKED, HE JUST GOT IT, BRACED HIT, RUN THROUGH) | in play | true | "JUST WON IT" fixed in READY 2 |
| YOU WIN / BEATEN, YOU n : n THEM, the keys | end card | true; **added** the match's lines (section 9) | |
| captions: lift.clean / lift.miss ("Reached for the lift, nothing there") | commentary | **cut** at runtime (READY 2, `jam.comm.LiftIsCross`) | the cross-check is the sim's lift; check.cross plays on a clean one |
| captions: pass.* ("Tape to tape.") | commentary | **fixed** (READY 2, `jam.comm.PassOnCatch`) | said only when a mate receives it |
| captions: faceoff.*, poke.*, check.*, goal.*, the rest | commentary | true | no own-goal line exists (nobody scores on himself by 08:30: nothing to cut) |
| barks (Game/Content/Barks/barks.json) | bubbles | true | no saucer, lift or own-goal word in any bark |
| "In a match Esc shows this list again. The pad is mapped; no person has play-tested it yet." | CONTROLS | true | |
