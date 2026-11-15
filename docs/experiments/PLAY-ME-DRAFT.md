# Play me: Sunday 10:00 (draft by lane A213 for the Director; PLAY-ME.md is the Director's)

*Describes the final sim as it is in the landing gate (06:33): A206 BRAIN READY 4 and 5, A207 PASS READY 3, A208 + A216 possession (STICK READY 3, `possess`, the faceoff at the dot), A209 CONTACT READY 3, A215 CROSS READY, A210 HITS READY 3, A213 SCREEN READY 1-4b. Commit: the Director fills it at the take.*

**The game in three lines.** Five against five on a toy rink, first to three. The mouse aims your stick; the two mouse buttons change meaning
with the puck: with it they pass and shoot, without it they check and cross-check. Your mates and theirs play for the puck and the net; a pass
reaches the man your stick points at; whoever touches a loose puck has it, and the screen says who.

## Controls (generated from the live bindings by `jam.ui.playsheet`: do not edit by hand)

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

In short: **RMB** with the puck = pass to the mate your stick points at (**hold for power**: a tap is short and crisp, half a second crosses a
third of the rink; the line on the ice shows how far); without = body check (it takes you onto a man in front). **LMB** with the puck = shoot
(a tap is flat, **hold to lift**); without = cross-check (shoves the man in front off the
puck; **hold to brace**). A button held while the puck changes hands does nothing until you let go and press again.

## What changed since last night (your three points)

1. **Bots that play.** A man keeps each net now: far shots are saves, close ones are chances. Nobody shoots from beyond 15 m in a match's
   first ten seconds. Your mates defend again; the man you pass to holds his line and skates onto the pass. At a faceoff both centres stand at
   the dot and nobody is hit for the first moments after the drop.
2. **The puck is yours on touch, and is seen.** A loose puck that reaches your stick or your body is yours, and the puck you carry is drawn
   where you can see it. A tap at a mate is aimed for him; under pressure your pass leaves on your far side, away from the man on you; your pass
   never comes back to your own stick; nobody scores on his own net. Your ring doubles the instant you have it (and lets go the moment the pass
   leaves), a mate's ring is your colour, an opponent's is orange, a loose puck has a ring of its own; the pill at the bottom left flips to
   SHOOT / PASS.
3. **Hits that land and are seen.** A body check pulls you onto a man in front within about 2.5 m (it lands about nine times in ten at
   rookie). LMB without the puck lands if your stick meets a man within ten frames of the press; a tap into a carrier's back shoves him off the
   puck (he stumbles, never falls). A bot never shoves you from behind. The hit shows on the man who took it.

## Five things to try in the first five minutes

| Try | You should see |
|---|---|
| Win the drop (LMB at DROP, not before; for a moment after it nobody is hit), then tap RMB with your stick on a mate 8 m away | the pass arrives on his blade; his ring turns your colour and pulses |
| With the puck, hold RMB half a second, stick toward a mate far up the ice | a line on the ice grows from the puck and ends on a ring under him; let go: a hard pass a third of the rink long |
| With the puck, hold LMB | let go: a lifted shot, twice as fast as last night up close |
| Without the puck, skate at their carrier and press RMB inside a couple of metres | you lunge onto him, he stumbles or goes down, the puck comes loose with its own ring; the first times the pill says "RMB  CHECK HIM" |
| Without the puck, LMB at a man in front; then hold LMB and skate into a man | a shove (his puck comes off); held, the pill reads BRACED and the hit stops him |

## Known limits, plainly

- Your passes under pressure still arrive about four times in ten: an opponent close to you takes many. Hold RMB for the far man: a held pass is
  harder and reaches across a third of the rink.
- Your mates find the open man ahead only about a third of the time.
- A missed cross-check has no commentator line yet; a pass is praised only when it is received.
- The held shot has no line on the ice (it stood up as a slab and was taken out); the held pass's line lies on the ice.
- Every Director's mode (Glow Ice too) plays the level lit on HOME, like PLAY NOW.
- After the match's last goal the GOAL banner plays and the card waits a second and a half, then says your shots, your passes that arrived, your
  hits and your time with the puck.
- The levels: rookie __ % / pro __ % / all-star __ % wins for the player each level is meant for (the curve lane sets them; the Director fills
  this line).
