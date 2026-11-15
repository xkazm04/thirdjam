# Play me: Sunday 10:00 (draft by lane A213 for the Director; PLAY-ME.md is the Director's)

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
(a tap is flat, **hold to lift**; the line on the ice grows and turns yellow when full); without = cross-check (shoves the man in front off the
puck; **hold to brace**). A button held while the puck changes hands does nothing until you let go and press again.

## What changed since last night (your three points)

1. **Bots that play.** Their passes arrive (65 to 76 % completed, was about 40), a mate skates onto the pass in flight, the second forward goes
   ahead on the weak side, and against a carrier inside 35 m one defender stands in the shooting lane, for both sides.
2. **The puck is yours on touch, and is seen.** A loose puck that reaches your stick or your body is yours. Your ring doubles the instant you
   have it, a mate's ring is your colour, an opponent's is orange, a loose puck has a ring of its own; the pill at the bottom left flips to
   SHOOT / PASS.
3. **Hits that land and are seen.** A body check pulls you up to a metre onto a man in front within about 2.5 m (it lands about nine times in
   ten at rookie); the cross-check shoves any man in front and the carrier loses the puck; the hit shows on the man who took it (the crouch
   becomes the hit, a contact ring, the downed oval).

## Five things to try in the first five minutes

| Try | You should see |
|---|---|
| Win the drop (LMB at DROP, not before), then tap RMB with your stick on a mate 8 m away | the pass arrives on his blade; his ring turns your colour and pulses |
| With the puck, hold RMB half a second, stick toward a mate far up the ice | a line on the ice grows from the puck and ends on a ring under him; let go: a hard pass a third of the rink long |
| With the puck, hold LMB | a line grows past the puck and turns yellow when full; let go: a lifted shot, twice as fast as last night |
| Without the puck, skate at their carrier and press RMB inside a couple of metres | you lunge onto him, he stumbles or goes down, the puck comes loose with its own ring; the first times the pill says "RMB  CHECK HIM" |
| Without the puck, LMB at a man in front; then hold LMB and skate into a man | a shove (his puck comes off); held, the pill reads BRACED and the hit stops him |

## Known limits, plainly

- Your mates still shoot less than they should (about one shot a minute) and seldom carry inside 22 m; the open man ahead is found about a third
  of the time.
- The bots' own checks at you land about one time in three; they do not dodge at rookie.
- A missed cross-check has no commentator line yet (the voice comes from device B); a pass is praised only when it is received.
- The lines of a held pass or shot are drawn fainter where they cross your own figure (the camera looks over your shoulder).
- Glow Ice (and every Director's mode) now plays the level lit on HOME, like PLAY NOW.
