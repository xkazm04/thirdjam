# HOKEJ: the vision the lanes chase (Director, Fri 13 Nov 15:50 CET; R-0045)

The operator's four topics (13 Nov 15:45), in his words and then as laws with numbers. This page is what a lane reads before it touches the sim, the brain or the
feel. Where it contradicts an older ruling, this page wins (R-0041 point 6 "the carrier cannot sprint" is replaced by law 4 below).

## The one sentence
Rough clay hockey where a hit obeys momentum, the puck carrier wins by changing speed and direction with a stick the mouse controls, and five skaters a side play
hockey: they see space, support the puck and pick their moment.

## 1. Physics: a hit obeys inertia
"If tackle is done between players going to same direction with similar speed, the impact is almost none next to a tackle or cross-check from player facing full speed
towards the player. Applying principles of physics law will make the game much more natural and logical."

- **Impulse.** A contact's strength is the closing speed along the line between the two bodies (attacker's velocity minus victim's, projected on that line) times the
  attacker's mass, plus a small lunge for a pressed check (1.5 m/s) and a thrust for a cross-check (3 m/s). Two skaters gliding the same way at the same speed exchange
  almost nothing; head-on at full speed is the biggest hit in the game.
- **Stability.** Every skater has a stability the impulse must beat. Standing or gliding slowly (under 1 m/s) and braced: high against a body hit from the side
  (x1.5). Moving lowers it with speed; sprinting lowers it more (x0.7); a hard turn, a spin or a full shot wind-up lower it; a hit the victim cannot see (from behind
  its facing) finds it unbraced (x0.8).
- **A cross-check from the front beats a standing skater.** The stick's thrust ignores the standing brace when it comes inside the victim's front arc. (The game's
  words: "to the chest"; the teen rating holds.)
- **Three outcomes, by the ratio of impulse to stability.** Under 0.4: a bump; both keep their feet, a little speed changes hands. 0.4 to 1: a stumble: the victim
  loses half its speed and its control for a few frames, and above 0.7 the puck comes loose. 1 and more: down, for longer the bigger the ratio (60 to 150 frames).
- **The attacker pays.** Momentum is conserved: the attacker loses the speed it gave. A sprinting attacker who runs into a braced heavy standing skater is the one who
  stumbles.
- **The carrier cannot hit.** A skater with the puck cannot body-check or cross-check; its defence is its feet and its stick.

## 2. AI: hockey principles
"AI should know going all the way forward will lead to failure. We should establish hockey principles into the game and AI: awareness of free space around, and free
positioning of team players to decide when to push solo, when to pass. Based on position of other players there should be decision making logic different during
attack and defense."

- **Space.** Every decision reads open ice: the free space around the carrier (the nearest opponent on the lane ahead, the time before it arrives) and around each
  teammate.
- **Carry or pass.** The carrier skates while the lane ahead is open and no better-placed teammate exists; it passes when pressure arrives inside a second, when a
  teammate is open nearer the net, or when the lane closes. Skating straight into a set defence is the wrong decision and the brain knows it.
- **Attack shape.** Without the puck in attack: one skater supports close (an outlet behind or beside the carrier), one goes to the far side or the net front, the
  backs hold the blue line; nobody stands in a teammate's lane.
- **Defence shape.** One pressures the carrier and steers it to the boards; the next covers the nearest pass; the others take the slot and the far side; nobody
  chases behind the play. Gap control: back off at the carrier's speed, close when it slows.
- **Different logic by phase.** Attack, defence and the two transitions (we just won it; we just lost it) each have their own rules; the level decides how well a bot
  executes them, not whether it knows them.

## 3. Gameplay: speed change and the stick
"Gameplay through high acceleration and speed change, and through stick control with mouse. The player with puck cannot tackle or crosscheck others, the skill level
to win will be about changing his speed and direction to avoid others - for this we need to allow fast breaking mechanisms (if player moves forward with 'W', sudden
transition to 'S' will break) and acceleration tempo."

- **Braking by reversing.** Input against the velocity is a hard stop (about 20 m/s^2: from full speed to still in half a second), with the spray and the sound.
- **Tempo.** A first-step burst: for the first third of a second after a stop or a sharp cut, acceleration is nearly doubled; then it settles. Stop, cut, go is the
  carrier's weapon.
- **The mouse is the stick** (R-0013, R-0042): the blade follows the cursor; sweeps and dekes shield the puck from sticks, never from bodies.

## 4. The stick and the sprint, calibrated
"Stick control itself needs calibration, if no sprint the puck should be closer to player. Sprint should be allowed but with caveats: if puck is far from player
easier to get by others, and sprint speed of defenders should be always higher. Sprinting and movement should in laws of physics mean less stability. Stationary
player can resist most of side tackles and fouls, logically not crosschecks directly to face."

- **Puck distance by pace.** Not sprinting, the carried puck rides close: the blade at three quarters of its reach (about 1.2 m from the body at full aim). Sprinting,
  it rides at full reach (1.6 m) and its shields are weaker: easier for a stick to take.
- **The carrier may sprint, and a defender is always faster.** The carrier's sprint adds less (about +8 % on its 0.92) than a skater without the puck (+15 %): a
  sprinting defender always catches a sprinting carrier in a straight line. The carrier escapes by law 3, not by law 4.
- **Pace costs stability** (law 1): the faster and the more extended, the easier to knock over.

## What this means for the plan
Three lanes build this in parallel from now: A92 the contact physics, A93 the movement, sprint and puck distance, A94 the hockey brain. Each rule has a neutral
value that gives back today's game, and each lane proves it. The acceptance run and the gate stay the bar. The curve is tuned once more after all three land (the
levels then differ in how well the bots execute the principles and how hard they hit). The sim freeze stays Saturday 12:00: what is not landed and green by
Saturday 10:00 is cut back to its neutral value.
