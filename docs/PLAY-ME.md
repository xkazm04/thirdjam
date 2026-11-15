# Play me: Sunday night, package H46: the walked and cured candidate (R-0069 to R-0073)

**Start it:** double-click `Game/Packaged/PLAY-LATEST.cmd` (every release rewrites it to the newest package), or unzip
`Game/Packaged/Hokej_Hokej_H46/Hokej_Hokej_H46.zip` into a fresh folder and run the exe. HOME shows the package's name in the bottom-right
corner (`H46`): if it says anything else, or nothing, you are in another build. If Windows asks about the firewall, that is the joined
match's socket: Cancel is fine for solo play. The Development twin with the drills and the console (tilde key) is `Game/Packaged/Accept_H46`.

**DLSS is back** (the plugin was fetched again at 17:14 after the Director's cleanup had emptied its folders): H44, H45 and H46 have it;
H42 and H43 do not.

**From here only faults are fixed** (R-0073): the final package is cut at 22:30 from the last green landing; you submit about midnight.

**Why the tag and the launcher exist:** the package you played at 10:31 this morning was H34 (Saturday 19:02), not H39: Windows' own start
record says so, and H35 to H39 were never started from the desktop. The grey / blue bar at a cross-check was H34's. Nothing on screen told
you which build it was. Now it does.

**The fallbacks:** `Hokej_Hokej_H45` (18:49, main c7057ed0): H46 before the walk's cures (coloured balls at the opening faceoff, a
streak behind a sliding man, a triangle in the picture, the hint under the MATCH card). `Hokej_Hokej_H44` (18:14, main bd655dc0, with DLSS): H45 without the evening's three (the HUD in one look, the
modes' illustrations, the neon arena in three more modes) and without device B's goal dust and whiff. `Hokej_Hokej_H43` (16:08): the last
package with the full acceptance run through the cursor (no DLSS). `Hokej_Hokej_H41` (14:14, with DLSS): before the new cross-check take.

## Your points of this morning, and where each stands

| Your words | In this package | Not done, said plainly |
|---|---|---|
| "puck is not controlled by player if in his circle area, then player runs right through the puck and still does not have control" | A loose puck inside the ring drawn under you, or one you skate through, is yours on that frame: whatever your stick points at, at any speed, with LMB or RMB held. Your own pass or shot is locked only while it is still leaving you. Reproduced first with a hand like yours (stick aimed away, buttons held): 21 of 27 cases were not yours before, none after. | Bots keep the old pickup rules. A puck riding with another carrier is still taken by a poke or a hit, not by touch. An opponent's medium shot that passes inside your ring is now your puck (a block): under one a match. |
| "Crosscheck still displays blue line/rectangle in front of player. This should player never see" and "no aim line needed, the player animation should reflect the crosscheck movement and that should be all" | Nothing is drawn at a hit any more, for you or for bots: no arc, chevron, dashes, burst, ring round a downed man, plate beside you, swell of your ring, pass line or stop line. Only the figure. A tap thrown into a man now plays the cross-check take (H39 replaced it with a collision flinch one frame later). A held cross-check keeps the stick level across the shoulders through skating, braking and standing. | Device B re-made the take this afternoon and A stopped sending a miss into the hold: press-and-hold is now ONE movement (the stick comes up at the press and stays across the shoulders), the tap is a coil at chest height and a bar across the shoulders at contact, with no lift over the head, and the hold ends without the gloves thrown up. **Not done:** a tapped cross-check into AIR still reads weakly as a miss (the stick just comes down: asked of device B); a bot's cross-check was not caught near the camera on any film; seen on device B's strips and A's editor films, not yet in a package by a person. |
| "For modes selection the previous images are deformed, rather create list with titles on left side ... and preview on right half" | MODES is a list of the twelve names A to Z on the left (then ALL RINKS and MAKE YOUR OWN under a rule); the chosen mode's picture whole in its own proportions on the right with its name, its line and the level it plays. Keys, pad and mouse. The cards had squashed 640 x 360 pictures into 320 x 88. | The pictures are 640 x 360 drawn larger: a little soft. A real pad's stick on the list was not tried (the D-pad walk was). |
| "Improve visually panel to show key bindings for player and keep it visible throughout the game, switchable in settings to take off permanently" (to device B, 10:50) | One panel bottom left all match, in the menu's look: a keycap and a verb a row. With the puck SHOOT, PASS, DEKE, SPIN; without CROSS-CHECK, CHECK, POKE; always SKATE, SPRINT, BRAKE (ABILITY while you hold one). It follows rebinds and the pad's names; the row of a held button lights. It replaces the 8-second strip and the small pill. SETTINGS has `CONTROLS PANEL  ON / OFF`, saved. | No switch inside a match (the pause card has no settings screen). Not seen with a real pad attached. Gaining the puck swaps the top half at once; losing it swaps after a fifth of a second, or at once when you press a button, so it does not blink on contested pucks. |

## Cured in H46 (the faults a lane saw walking the cooked H45 through every screen and all twelve modes)

- No burst of coloured balls at the opening faceoff, a poke, a lift or a shot blocked by a body; no streak behind a knocked-down man's
  slide; no man-down triangle in the picture; the yellow hint waits while the MATCH card is up; no faceoff prompt while you sit in the box.
- MAKE YOUR OWN in a packaged game uses only a key you type (or the environment variable `ANTHROPIC_API_KEY` if you set one), else its
  saved games: never a command-line tool found on the machine.
- The Cage's edge line on the neon arena sits evenly; small texts (OWN 24 M, SETTINGS, MODES, MAKE YOUR OWN's list).
- Not cured: the camera at the side boards; in the clock modes the period's card can show while play runs on.

## New in H45 (your orders of 17:16, R-0072; device B's push of 18:05)

- **The match HUD in the front end's one look:** the score and period plates, the MATCH card, the hints, the commentary strip, the
  distance labels, the minimap, the bark bubble, the YOU tag; the end card's stats as a table; the labels hidden while a card is up.
- **An illustration for every mode** (Codex image generation, one painted family, no lettering) in place of the screenshots on MODES.
- **The neon arena in four modes:** Arsenal (lime), Marathon (ice cyan) and Cage (red-pink, its drums rimmed) beside Glow Ice (white);
  the same rules and bots as before, only the look. (The Cage's edge line is evened in the landing after H45.)
- **While you sit in the box** (Glow Ice's foul) the camera follows the play and a plate counts `IN THE BOX n` down.
- **No effect at a hit** (the white streaks, the balls on Glow Ice): the figures only; the sounds stay.
- **The sprint bar left the ice:** the panel's SPRINT row is lit while you sprint and dimmed when the legs are spent.
- **Device B:** the goal's dust no longer covers the camera; a cross-check into air now reads as a miss (he lurches on, a skate thrown
  wide); a check pressed during a miss plays its wind-up.
- **CREDITS** names NVIDIA DLSS (text, no logo) and the tools whose output ships.

## Also new since H39

- **For the trailer:** Fog of War draws its fog (far skaters and the far boards fade, the near play stays clear); Glow Ice's penalty boxes
  are dark with the sheet; Pond's board trim reads as paint, not as a light; a boxed skater leaves no ring on the ice; no engine message
  can appear on screen in any packaged build.
- **A miss is a take:** a check or cross-check that meets nobody plays device B's whiff.

- **Device B (merged at 13:45, 15:10 and 16:00):** a goal on Glow Ice is a burst of light in the scoring team's colour; Pond has a day sky and
  sun; stands on the small rink; the shotgun's fan; the pass's take lets go on the frame the puck leaves; a white trail behind a shot puck, readable from the
  far end; three commentator lines for your cross-check that meets nobody ("Shoves at air." and two more: not yet heard by anyone).
- **The pass has no line on the ice any more** (your word): the mate who will get it is told by his ring turning your colour.
- **The first-minute hints** ("RMB CHECK HIM", "RMB PASS: A MATE IS OPEN") now light the panel's row instead of a pill.

## Controls (generated from the live bindings)

<!-- controls:begin -->
| Control | Mouse and keys | Pad | What it does |
|---|---|---|---|
| SKATE | W A S D or arrows | left stick | you glide: speed is built, not given |
| AIM THE STICK | the mouse | right stick | the blade follows the cursor |
| SHOOT (WITH THE PUCK) | left mouse button | RT | a tap is a flat shot; hold to lift it, let go to fire |
| CROSS-CHECK (WITHOUT) | left mouse button or middle mouse or mouse 4 | RT or LB | shoves the man in front off the puck; hold it to brace |
| PASS (WITH THE PUCK) | right mouse button or Q | RB or X | to the mate your stick points at; hold for power |
| CHECK (WITHOUT) | right mouse button | RB | the body check: it takes you onto a man in front |
| ABILITY | F | R3 (stick click) | the mode's trick or a pickup's, when the slot is lit |
| SPIN | E | B | spin off a man who is on you |
| DEKE | mouse wheel | Y | the carrier's fake |
| SPRINT | Shift | LT | while the legs last |
| BRAKE | S against your motion, or Ctrl | stick back, or L3 | stop hard, then go: it beats a man who charges |
| POKE | Space | A | jab the puck loose |
| SIGNALS | 1 2 3 4 | D-pad | a call to your mates |
| RESTART | R | View | the match again |
| PAUSE | Esc | Start | the pause card: resume, a rule change, controls |
<!-- controls:end -->

**RMB** with the puck = pass to the mate your stick points at (**hold for power**); without = body check. **LMB** with the puck = shoot (a tap is
flat, **hold to lift**); without = cross-check (**hold to brace**). A button held while the puck changes hands does nothing until you let go and
press again.

## Five things to try in the first five minutes

| Try | You should see |
|---|---|
| Look at HOME's bottom-right corner, then open MODES | `H46`; a list of names on the left, each mode's own illustration whole on the right |
| Aim the stick anywhere and skate over a loose puck; then stand with one beside or behind you inside your ring | it is yours on that frame, both times |
| Tap RMB with nobody on the line, then skate after your own slow puck | you pick it up as you reach it (before: you ran through it) |
| Without the puck, tap LMB into a carrier; then hold LMB and skate, brake, stand | the figure's stick comes up; held, it stays level across the shoulders through the brake; nothing drawn on the ice |
| Watch the bottom-left panel as the puck changes hands; then SETTINGS, CONTROLS PANEL OFF | the top half swaps between WITH and WITHOUT THE PUCK; OFF removes it and stays off next start |

## The levels

Read through a headless copy of a person's hand (it presses the two buttons as you do), not played. Wins for the player each level is meant
for; pro at 600 matches a cell after the set, the others at 300:

| Level (target) | Regular rink | Large rink | This morning (H39) |
|---|---|---|---|
| ROOKIE (85 %) | 99 | 93 | 92 / 82 |
| PRO (48 %) | 55 | 46 | 54 / 52 |
| ALL-STAR (26 %) | 32 | 43 | 27 / 34 |

The puck rule made every level easier (pro was 71 / 58 straight after it): you keep and take more pucks. Set 4 brought pro back: its presser
now commits at 8 m inside a wider hunt. Not met: rookie is close to a walkover for the scripted hand (no lever found that keeps it a
newcomer's level); all-star was not moved (its levers raise knockdowns) and is easy on the large rink; at PRO you are knocked down about 1.8
times a minute (1.45 this morning: you carry the puck more, so you are hit more). The twelve modes at pro inherit the set and were not re-measured.

## Known limits, plainly

- A tapped cross-check into air reads weakly as a miss. Waiting on device B.
- Pro's knockdowns; rookie and all-star easier than their targets (above).
- Your passes under pressure still arrive about one time in three to one in two.
- A bot can press against a net's frame for a second or two before it goes round.
- At the side boards the camera goes steep and close (seen on film; not cured: not a small change).
- When orange scores on the neon arena the goal's burst reads nearer smoke than light (device B's; asked).
- A carried puck can sit inside another body for about three frames after a steal (the one class hardening still finds: 50 events in
  2,400 matches).
- The large rink and the twelve modes were measured, not looked at as closely as PLAY NOW on the regular rink.

## How it was proven

- **Tests:** the full automation filter green on every commit: 398 of 398 on the commit H46 is cut from. (That landing's own gate ran only
  214 of them: the disk had fallen under the gate's floor and the run was cut short and wrongly called green. Found at 14:15, the gate
  fixed to fail in that case, the filter re-run in full: 373 green.) Test bars moved today, with
  the reason in each commit: tests that pinned the marks at a hit, the 8-second strip and the pill now pin their absence and the panel; the
  commentary bank's count; `Jam.Bot.Hockey4.PressInHunt` (pro's presser is ON in the hunt since set 4); the ladder's pinned matches.
- **Reproduced as you meet it, then cured, then shown on the same reproduction:** the puck (a table of 27 cases with a person's hand); the
  marks at a hit (every sim frame filmed from the game camera in the PACKAGED game, H39 above and the cure below, three looks, two sizes:
  `docs/experiments/bar/`); MODES and the panel (stills at both sizes: `docs/experiments/modes/`, `docs/experiments/panel/`).
- **The package:** the cooked Development twin played a whole scripted match off screen (no window), no fatal line, the sim's hashes logged.
  The acceptance run through the cursor and the buttons was not made on H40, H41 and H42 (you were at the machine and it takes the mouse);
  **H43 had it in full** at 16:02 while the machine was idle: ten steps and 18 verbs green, then the menu walked and the Shipping zip
  started and played, both green. H44 and H45 had the off-screen match only. **H46 had it in full** at 21:00: ten steps green, the menu walked, the Shipping zip
  started and played.
- **The walk (19:50):** the cooked H45 through every screen, all fourteen MODES rows, all twelve modes at rookie and PLAY NOW at
  rookie and pro, off screen with the scripted hand: no crash, every screen can be left, every mode plays to its end card in the look of
  its picture; six faults a judge would see, five cured in H46 and shown on film before and after.
- **Hardening on this sim (18:14):** 2,400 escape-scan matches (no puck outside the rink, none stuck in a net, no goal through a net's
  side), 600 fuzz matches twice (all end, hashes equal), 44 lockstep matches with loss and jitter and both seats human (0 desync),
  10,700 soak matches (0 faults), 46 of 46 cells identical on a second run.
- **Still said plainly:** all of it was measured and filmed by scripted hands. Nobody has played H40 to H46 with a mouse. You are the first.
