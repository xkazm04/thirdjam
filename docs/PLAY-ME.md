# Play me: Sunday 10:00, the package after the night (R-0067, R-0068)

**The package:** `Game/Packaged/Hokej_Hokej_H39/Hokej_Hokej_H39.zip` (the last `package` line of `status/A.md` names it and its commit). Unzip
into a fresh folder, run the exe. If Windows asks about the firewall, that is the joined match's socket: Cancel is fine for solo play. HOME lights
ROOKIE (your settings file reads as a first start): start there, then PRO. The Development twin with the drills and the console (tilde key) is
`Game/Packaged/Accept_H39`.
**The fallbacks:** `Hokej_Hokej_H38` (08:08, main 2c34cab4): the same game with the levels as they were built before the ladder was set (rookie about 93 / 90 % wins, pro 54 on the
regular rink and 7 on the large one, all-star 4 / 6) and with the end card missing your own winning goal: acceptance run, menu row and ship row green.
`Hokej_Hokej_H37` (07:20, c26e9152): H38 without the fix for an own goal through the side of the net. `Hokej_Hokej_H35` (02:40): the night's first third only.
From 08:30 only faults are fixed. Submission is tonight at 22:00.

## Your three points of 20:25, and where each stands

| Your words | In this package | Not done, said plainly |
|---|---|---|
| "AI is the main flaw, it does not try to play hockey to achieve a goal ... disengaged" | Every skater has a job. A man keeps each net and steps into a shot's line; a man goes at an unguarded carrier; the second forward goes ahead on the weak side; a defender stands in the shooting lane; clumps split. Bots do not pass under 5 m, straight back, or through their own crease. Nobody is hit for the first moments after a drop, and nobody shoots from beyond 15 m in a match's first ten seconds. | Your mates find the open man ahead about a third of the time. At pro the bots press you hard: you are knocked down about one and a half times a minute. |
| "They want to pass but no pass reaches the end (imprecise direction or speed)" | A pass goes to the man the stick points at, at a speed solved for him; the passer never gets his own pass back; an opponent takes a pass in flight only at his blade; under pressure it leaves on your far side; the mate it is aimed at holds his line and skates onto it. Bots complete about 55 % of their passes (37 % yesterday). | Your own passes under pressure arrive about four to six times in ten (pro about two in three, rookie about half, with the critic's scripted hand). Hold RMB for the far man. |
| "passes need to be able to be hold enough to cross 1/3 of arena length on max" | Hold RMB: half a second is the full pass, a third of the rink; a line on the ice shows how far and ends in a ring under the mate it will reach. | |
| "shoot speed should double" | Done: 52 m/s off the stick at full charge. The added speed fades after the first 8 m, so a far shot at a net with a man in it is a save and a close one is a chance. | The shot has no line on the ice while you charge (it stood up like a slab and was taken out). |
| "Puck does not stick with players, it moves through them without locking" | A loose puck that touches your stick's arc or your body is yours (99 % of touches, 45 % yesterday); a body blocks a shot and a carried puck; nobody scores on himself; a goal counts only through the mouth. The screen says who has it: your ring doubles, a mate's is your colour, an opponent's is orange, a loose puck has its own; the puck you carry is drawn where you can see it. | In a crowd a carried puck can still sit between two defenders whose bodies touch (2 cases in 480 matches). |
| "Tackles/Crosschecks are difficult to land as they need to land to thin space of the player" | RMB without the puck pulls you onto a man in front within about 2.5 m: it lands about nine times in ten at rookie. LMB lands if your stick meets a man within ten frames of the press; a tap into a carrier's back shoves him off the puck (he stumbles, never falls). A tap never braces; a press a moment too early still swings. A bot never shoves you from behind. | The cross-check lands three times in four with a person's hand (half yesterday), not nine in ten: the men it misses skate out of the stick's reach. |
| "crosscheck has bug in animation displaying technical blue rectangle" | Gone: it was A's own mark drawn on the stick and chest. Device B's takes now carry the hits by themselves (the stick brought up across the body, the brace held above the shoulders, the fall flat back along the hit). A white version of the same fault that a new HUD line brought in at 03:00 was found by the night's critic lane and taken out at 05:46. | A missed cross-check has no animation of its own yet. |
| "Tackle is also unrecognizable and animation too quick" | Your wind-up is 8 frames (was 4), device B's follow-through is held 12 to 18 frames, the hit shows on the man who took it, and the end card counts your hits. | |

## Also new since last night

- **The faceoff is a faceoff**: both centres at the dot, a real draw; whoever wins it cannot be knocked down for a second and a half.
- **The camera near the boards** slides in and comes down instead of rising: behind the net and along the wall you see the ice, not the stands.
- **The end**: the last goal is shown with its GOAL banner before the card; the card says your shots, your passes that arrived, your hits, your time with the puck.
- **The first minute teaches**: the pill at the bottom left names what your two buttons do right now, and pulses "RMB CHECK HIM" / "RMB PASS: A MATE IS OPEN" a few times until you have done it.
- **Glow Ice and the other modes** play the level lit on HOME.

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
| Win the drop (LMB at DROP, not before), then tap RMB with your stick on a mate 8 m away | the pass arrives on his blade; his ring turns your colour and pulses |
| With the puck, hold RMB half a second, stick toward a mate far up the ice | a line on the ice grows from the puck and ends on a ring under him; let go: a hard pass a third of the rink long |
| With the puck, hold LMB and let go inside 12 m; then try from the blue line | close: a chance; far: the man in the crease steps into it |
| Without the puck, skate at their carrier and press RMB inside a couple of metres | you lunge onto him, he stumbles or goes down, the puck comes loose with its own ring |
| Chase a carrier and tap LMB into his back; then hold LMB and skate into a man from the front | the tap shoves him off the puck; held, the pill reads BRACED and the hit stops him |

## The levels

Set at 07:57 through a headless copy of a person's hand (it presses the two buttons as you do; the ladder used to be read with a hand that sent
verbs straight to the sim, and since last night's rules the two differed by a class). Wins for the player each level is meant for, 300 matches a cell:

| Level (target) | Regular rink | Large rink |
|---|---|---|
| ROOKIE (85 %) | 92 | 82 |
| PRO (48 %) | 54 | 52 |
| ALL-STAR (26 %) | 27 | 34 |

What was moved: pro's bots no longer shoot from beyond the blue line (goals against you from 28 m or more: 21 % of them before, under 3 % now) and are
weaker on the large rink; rookie's opponents skate a touch faster (rookie had become a walkover once a man kept each net); all-star's bots are slower and
dodge less. Not met: at PRO you are knocked down about one and a half times a minute (the one lever that cured it made pro too easy); rookie on the
regular rink is 7 points easy and all-star on the large rink 8; about three rookie matches in ten run past four minutes.

**Two readings of this ladder, both given.** The critic lane filmed it against the levels as built (package H38) with the same hand on seven
seeds and read rookie as harder and slower to score in, and your pass at pro as worse. The curve lane measured both over 300 matches a cell on the
same seeds: rookie 93.7 % wins as built against 91.7 % with the set, the median match 184 s against 188 s, his shots 3.8 a match in both; pro 49.7 %
against 54.0 %, your passes reaching a mate 36 % in both, knockdowns 1.48 against 1.45 a minute. So over many matches the two play alike at rookie
and pro on the regular rink, and only the set cures pro's far goal (21 % of goals against, 2.7 % with it) and makes all-star and the large rink
playable (one film of the large rink at pro was still a 0:4 rout: one seed against the measure's 52 %). If rookie feels like a grind to you, H38 has
the kinder numbers: say so and the two can be blended (rookie as built, the rest as set).

## What the night's critic says (a lane that only looks, with a scripted hand that presses the buttons as a person does)

At 00:42 on the game you played: "a stranger would not start a second match". At 06:50 on this game: "a stranger wins a close match at rookie, twice
from behind, and at pro loses 0:3 to three shots from the slot with his own man in the crease and can read why on the card; he plays a second match
at both, and says 'I get flattened a lot at pro' and 'my cross-check misses', not 'unplayable'." It is a scripted hand's eye on four films and
fourteen seeds, not a person's: yours is the bar.

## Known limits, plainly

- Your passes under pressure, the cross-check at three in four, and being knocked down often at pro: as in the table above.
- Device B's last asks did not arrive (its night ended at 01:15): the shot's streak is still a small dot from 25 m, and a missed cross-check has no
  take and no commentator line.
- A bot can press against a net's frame for a second or two before it goes round (a known fault of the brain, not cured tonight).
- Right behind your own net with your back to the glass the camera sits close over you (about 1 still in 100).
- The large rink and the twelve modes were measured, not looked at as closely as PLAY NOW on the regular rink.

## How it was proven

- **Tests:** the full automation filter (358 tests at the last landing before the ladder) green in the landing gate of every commit; three test bars
  were moved by the Director with the reason in the commit (a faceoff is now a contest; a pass can end before a checkpoint; a second pickup refreshes an
  effect), and the ladder's landing re-pinned the tests that pin a level's match.
- **Every package** was cut by the release chain: the acceptance drill in a real window (ten steps, 18 verbs through the cursor and the buttons), then the
  Shipping zip started and played by keys, then the menu walked. H35 was held up an hour by the test driver, not the game.
- **Hardening lane, on the sim that ships:** 6,000 soak matches with 0 faults; 600 input-fuzz matches (long holds, pauses) all finished with equal
  hashes; lockstep over 28 long matches with loss and jitter: 0 desync. It found five faults in the night, each fixed before this package: the release
  driver; a goal through the side netting at a reception; a carried puck inside another body; a lifted puck through a mate; an own goal through the side
  netting off a pass at the post (found at 07:04 on the final sim, cured at 07:39, re-scanned: 0 in 600).
- **Critic lane:** six lists through the night on the game's own pictures; its last words are quoted above.
- **Still said plainly:** it was measured and filmed by scripted hands. Nobody has played this package with a mouse yet. You are the first.
