# Play me: Saturday 20:00, the package at the freeze (R-0064, R-0065, R-0066)

**The package:** `Game/Packaged/Hokej_Hokej_H34/Hokej_Hokej_H34.zip` (the last `package` line of `status/A.md` names it and its commit). Unzip into
a fresh folder, run the exe. If Windows asks about the firewall, that is the joined match's socket: Cancel is fine for solo play. The Development twin
with the drills and the console (tilde key) is `Game/Packaged/Accept_H34`.
**The fallback** is `Hokej_Hokej_H33` (17:45, main ea0c3b7a): everything below except the bots' new sense, the second ladder, the neon mode's arena and
the last hour's fixes; its acceptance run, menu row and the unzipped-zip row are green. The package you played at noon was H31.
From 20:00 the game is frozen: only faults are fixed.

## Your notes of noon, and where each stands

| Your words | In this package | Not done |
|---|---|---|
| "Camera around arena borders is autocorrecting unnaturally" | The boards no longer turn the camera, and the pitch correction you felt in corners is off. A stick that stands still keeps its point on the ice while you stand. | The attacked net can still leave the frame when you work the boards inside the blue line. |
| "Possibility to rebind ... Cross check on Left mouse ... Right mouse if on puck: Pass. Hold for saucer pass" | As you ordered: see Controls. Every control can be rebound (SETTINGS or the pause card, CONTROLS); a move that would leave any control without a key is refused with words; a damaged settings file falls back to the defaults. A pill by the stick says what the two buttons do right now. | A hand in a real window on the rebinding screen and on a pad: proven by tests and a scripted hand only. |
| "Crosscheck should be holdable so player with speed momentum can run with held crosscheck into another player" | Hold left mouse without the puck: after 0.2 s you skate braced (slower, turning less) and whoever you run into takes the hit by your speed. BRACED HIT / RUN THROUGH on screen; device B's braced pose on the figure; a streak across the chest and an arc on the ice ahead while you hold. | The bots do not brace (measured: it made them weaker). |
| "Menu HUD is a mess, leave concept of director's cut ... profi ... menu UI and landing experience" | A landing, then HOME: PLAY NOW with the level switch, SEASON, MODES (fourteen cards with pictures), PLAY WITH A FRIEND, CONTROLS, SETTINGS, CREDITS, QUIT (asks once). Device B's barn behind it, its plates and font. The pause card, the end card, the season card, settings, controls and the lobby in the same style. No "cut", no "Director" outside MAKE YOUR OWN (the last card of MODES). | The live arena behind the menu (a still of it stands there). B's frosted logo is switched off: it lost against the backdrop: the plain HOKEJ type stays. |
| "Animations of crosscheck and body check are short and not clear" | Three things. The bots' body check now winds up 8 frames and their cross-check 12 (yours stay at 4 so the button is not late), with device B's wind-up takes on the bots. Around every hit, on the ice: a chevron of light from the press to the contact (grey on a miss), a ring at contact by the speed brought, an oval of bars round a downed man that counts his time down, an arrow at the screen's edge and a mark on the radar for a man down off screen. | Your own figure keeps a short stand-in lean (B's take needs 6 frames or more). No new sound for a braced hit yet. |
| "Special boosts are not seen on ice, not seen equipped on players" | Device B's props turn on a pedestal of light with a ground ring and a radar mark; the keg, the hip bumper, the shield or the crest sits on the skater who holds one; the plate says "F: SHOTGUN" and its state. | The speed boost's prop is cyan and its ring yellow: one of them changes. |
| "Skaters without puck should be 15% faster than now" | Done for human and bots at every level; the carrier is as he was. A breakaway outruns a chaser again. The bots plan with the new speed. | |
| "Puck's white outer ring for visibility can be 50% thinner" | Done (device B: 18 to 9 cm). | |
| "Exercise in one director's mode further concept of neon Tron like arena" | Glass House's slot is now **Glow Ice**: dark ice drawn in light: device B's violet hall with a grid of light lines, A's glowing boards and rink lines, short trails in team colours behind every skater and the puck, the figures' trims lit in their team's colour. It plays with the whistle and speed boosts, first to 4. | Its win rate (a regular player 56 %) was read at 17:00, before the bots' new sense. Cage and Bruisers still share the hall's look. |
| "short celebration animation set after a goal, HUD middle screen large banner saying 'Goal'" | GOAL across the middle band with the new score, on device B's banner art; the scorer and a mate play one of B's four celebrations inside the goal's pause. | The camera stays on you: when you are not the scorer the celebration can be off screen. |

## What else changed since noon, that you did not ask for

- **The bots stopped doing four things a watcher calls broken** (counted over 60 matches, per match): standing at a downed man and pushing him along
  the ice 2.6 -> 0.4, shooting from beyond 30 m with a mate open nearer 2.6 -> 0.4, carrying the puck back into their own zone with open ice ahead
  2.7 -> 0.05, two mates skating in a clump 1.4 -> 0.35. They keep the puck better for it; the ladder was set again on top.
- **The stop line now tells the truth.** It lit about fifteen times a match and a player who obeyed it lost three matches in five. Now it lights about
  twice a match: when the line across your path turns your opponent's colour, he is alone with you: stop, cut to the open side, go. A pale line means
  keep playing. It goes out when a stop would come too late.
- **The ring under a loose puck** means one man is coming for it as fast as you: hold off and let him have it, or take it knowing he hits you. No ring
  means it is a scramble: go.
- **A button held while the puck changes hands does nothing** until you let go and press again, in the game's rules now, not only in the mouse layer.
- **A joined match** no longer dies when one rink loads slower than the other, nor when one player opens the pause card (the card opens over a running
  match and says so); its lines ("rematch asked", "the puck drops") show in the packaged game; a player who quits or loses the network is named on
  the other side within a second or three.
- **QUIT asks once**; SETTINGS and PLAY WITH A FRIEND no longer trap a pad or keyboard player.

## Controls

Mouse aims the stick. **The two mouse buttons change with the puck.**
**LMB**: with the puck: shoot (a tap is a flat shot, hold to lift); without it: cross-check; **hold it** (from 0.2 s) and skate into a man: your speed
is the hit (the pill at the bottom left reads BRACED).
**RMB**: with the puck: pass; **hold it** (from 0.2 s) for a saucer (the pill reads SAUCER); without it: body check.
A press keeps its meaning until you let go, and **a button held while the puck changes hands does nothing until you let go and press again** (a brace
ends when you take the puck). A puck your own side has just passed counts as "with the puck", so a one-timer is still one press.
**Space** poke. **F** ability. **E** spin. **Wheel** deke. **Shift** sprint. **S against your motion** or **Ctrl** brake. **Esc** pause.
Q (pass) and MMB / mouse 4 (cross-check) still work as second keys.
**The pad**: left stick skate (back against your motion brakes), right stick aim, **RT** shoot / cross-check (hold to brace), **RB** pass (hold for a
saucer) / check, **A** poke, **Y** deke, **B** spin, **R3** ability, **LT** sprint, **L3** brake, **Start** pause. The hints on screen follow the
device you touched last.
**Every control can be rebound**: SETTINGS (or the pause card), CONTROLS: click a cell, press the new key or pad button; Esc cancels; a clash swaps
the two and the line says so; a move that would leave a control with no key is refused; RESET TO DEFAULTS asks once more. Saved at once.
The landing: Enter, the pad's A or a click goes on; on HOME **Enter plays one match at the level that is lit**; left and right change the level.

## The ladder on this package

| Level (the scripted player it is set with) | Regular rink | Large rink | Target |
|---|---|---|---|
| Rookie (a newcomer) | **85 %** | 84 % | 85 |
| Pro (a regular player) | **47 %** | 46 % | 48 |
| All-star (a good player) | **26 %** | 25 % | 26 |

Each cell over 2,000 matches on this package's rules (the bots' new sense played by its switch; the built game is replayed against these rows by seed
in the stabilization phase).

Set twice today, the second time on everything above (faster skaters, hits that wind up, bots that play better). Rookie is the newcomer's level; if
you have played a hockey game, yours is pro. The stop begun at 8 to 9 m works at every level (pro 94 %; a straight charge never).

## Ten minutes, in this order

1. **The landing and HOME.** A game somebody shipped? What trips the eye first?
2. **The buttons.** A match at rookie: shoot and pass on the two buttons, lose the puck and hit with the same two. Rebind one control and use it.
3. **The held cross-check.** Without the puck, hold left mouse, sprint at a carrier, run through him.
4. **A hit you take.** At pro, watch a bot wind up on you: do you see it coming now? Can you get out of it?
5. **The boards and the corners.** Carry along the boards and through a corner with the stick aimed: does the picture still move under your cursor?
6. **The stop line.** Carry at one defender with open ice behind him; stop when the line turns his colour; go. Then ignore a pale line.
7. **A boost.** MODES, Arsenal: find a pickup by eye, take it, see it on you, use it on F.
8. **A goal.** The banner and the celebration.
9. **The neon mode** (MODES, Glow Ice).
10. **Pro, one match.** A fair match?

## Known to be unfinished (do not spend your time finding these)

- The third column above.
- **Proven by tests and scripted hands, not by a hand in a real window:** rebinding by a real click and on a pad; the pad's walk through the new
  screens; alt-tab during a joined match; the window's close button telling the other player at once. Two physical machines have never been tried.
- **The twelve modes were re-tuned at 17:00, before the bots' new sense and the second ladder:** every one sat between 38 and 66 % for a regular
  player then; they are measured again in the stabilization phase and may have drifted.
- The acceptance run has five new rows for the buttons (a held button across a change of possession, the slow tap, the saucer's hold, a refused
  rebinding, the pad) that are proven headless and wait for their first run in a window.
- The page's stills and the video shots are from Friday's game and are retaken from this package.

## What the Director needs from you now

1. **The buttons and the held cross-check:** right, or what is wrong, one line each.
2. **The camera at the boards:** cured, better, or the same?
3. **The menu:** right? Anything that must change before the page's stills are taken?
4. **The hits:** do you read a bot's check coming? Is 8 frames enough, too slow, too kind?
5. **The ladder:** which level is yours, and was it a fair match?
6. **The neon mode:** keep it, push it, or drop it?
7. **Anything that broke.** From here only faults are fixed; a fault you name goes first.
