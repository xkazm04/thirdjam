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
