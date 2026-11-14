# Play me: what to try on Saturday morning, and what to tell the Director after (R-0056)

**The package:** `Game/Packaged/Hokej_Hokej_H28/Hokej_Hokej_H28.zip` (the last `package` line of `status/A.md` names it and its commit). Unzip into a fresh
folder, run the exe. If Windows asks about the firewall, that is the joined match's socket: Cancel is fine for solo play.
**Do not play H27**: its PLAY NOW built the match twice, played pro whatever was lit and drew a black rink. Found in a real window at 03:50, fixed and
verified in a real window at 04:30; the acceptance run now photographs the frame and fails on a black one.

## What changed since the package you last played (H26, Friday 19:46)

| You will notice | Why |
|---|---|
| The rink is **clay** from the first match, with **timber stands and a clay crowd** along both sides | the regular cut and takes 1 to 4 on the clay look (R-0054); device B's stands, merged at 04:44 and timed: no frame cost |
| A **ROOKIE / PRO / ALL-STAR** switch under PLAY NOW, and it reaches the match | the level wire (R-0059); your first PLAY NOW is rookie |
| **No whistle** in the regular game, the season and eleven of twelve modes | fouls and the box are gone there (R-0048); Glass House keeps them |
| **You keep the puck** until somebody presses for it; a **yellow ring** under a carrier closes while no stick can take his puck | an unpressed blade or a body's touch no longer takes it; half a second of grace after winning it, now shown |
| **Space takes the puck** when your stick is aimed roughly at it, and a miss no longer trips you | the poke (A152): a miss locks your stick for a third of a second instead |
| **Stopping works**, from the same distance at every level | S against your motion brakes hard; a quick first step after a stop; five stick lengths (8 m) at rookie, pro and all-star |
| **Two marks on the ice**: a ring in the other team's colour round a loose puck means one of them arrives when you do (take it and you are hit: brake short, or let him have it and hit him); a chalk ring five stick lengths ahead of you while you carry turns the defender's colour when the stop is on | the carrier track (A166): two knockdowns in three land as you win the puck, and the stop was on and unused in one in seven |
| Hits have **sizes**; a **standing skater holds a side hit** | a bump, a stumble, or down for 1 to 2.5 s by the speed brought; the HUD says why a press did not land |
| **Nothing is drawn on you while you are down**; you are not knocked down again the moment you get up | the ring leaves a downed player and returns after the get-up; 2 s of grace |
| Your **mates pass to you** and go for loose pucks | the brain plays as a side; bots brake on purpose |
| The **arena answers**: a goal lights a lamp and shakes the net, a board hit rocks the boards; the ice no longer turns pink on a goal | clay benches, boxes with readable clocks, round caps on every rink; the goal's light at device B's quiet numbers |
| The camera **keeps you in frame**; a yellow chevron marks you | the screen track; in corners it still looks down steeply (trimmed, not cured) |
| The **menu**: PLAY NOW, a controls screen, twelve mode cards you can read; click a level then Return plays it | the menu track |
| Modes: **Arsenal, Glass House and Sudden Death are five a side**; the **Marathon** gives you the puck | the modes track (A156): passes there went from under one to about three a play minute |

## Read this first

- **Your level is the lit one.** PLAY NOW and the cut cards play it; the season's takes and the modes set their own. The match card and the pause card show
  the level in force.
- **The ladder as measured on this package's commit** (2,000 matches a cell, the scripted player the ladder was set with): a newcomer wins **86 %** at
  rookie, a regular player **52 %** at pro, a good player **29 %** at all-star (targets 85 / 48 / 26). Pro is four points
  over on purpose (R-0062): the one dial that brings it down makes the stop-and-go fail one time in five from 9.5 m. On the large rink: 84 / 50 / 26.
- **Which player are you?** The scripted player whiffs about half his checks. One who lands most of them wins 87 / 56 / 31 instead of
  86 / 52 / 29 (R-0060).
  Tell the Director which you are: the curve follows your answer today.
- **The stop works from 8 m**, the distance from the centre to a winger at a faceoff: five stick lengths, at every level. From 3 to 6 m it gets you flattened.
- **Mashing RMB beats timing it today** (measured: about 20 hits a match against 4.6, and four points more wins at pro): a press costs nothing. A cooldown
  and a lunge that fix it are built and switched off, waiting for your word (see the last section).

## Ten minutes, in this order

1. **A first match on the regular cut at rookie.** Does the first minute explain itself? Is the rink what you want a stranger to see first?
2. **The stop-and-go.** Carry at a defender, press **S against your motion** when the chalk ring ahead of you turns his colour (he is about five stick
   lengths away), then go the other way. At pro it
   should work nearly every time from there, and never if you just charge. `jam.ctrl.Drill 21` in a Development build stages it and says the distance on
   screen. This is the game's central skill: does it feel like one?
3. **Two hits.** Skate beside an opponent at his speed and check him (RMB): a nudge. Then hit one head-on at full speed: he goes down. Same button.
4. **Keep the puck, and take it.** Carry with a defender on your back and turn your body between him and the puck: his stick should not reach it. Then
   go for a carrier with **Space** once the yellow ring under him has closed.
5. **Pro, one match; all-star, one match.** Three different games?
6. **The season's take 3 and take 4** (the ramp was re-set in the night: R-0057).
7. **Three modes** (a regular player wins 45 to 80 %): **Shotgun** (ten bodies on tight ice, F clears the lane), **Fog of War** (three a side in the fog:
   slip by slow-reacting bots, then hit; kept at three a side on purpose), **Outnumbered** (three against five: keep the puck, a shield on F). Then one of the
   five-a-side ones: **Arsenal** (tricks on F, three charges each) or the **Marathon**.
8. **A joined match**, if device B is at hand: `docs/experiments/JOIN-GUIDE.md`. Two processes play to the final card with a rematch on this machine; two
   physical machines have never been tried.

## Controls

Mouse aims the stick. **LMB** shoot (a tap is a flat shot, hold to lift). **RMB** check. **MMB** cross-check or lift. **Q** pass (hold for a saucer). **F** ability.
**E** spin. **Wheel** deke. **Shift** sprint. **S against your motion** brake. **Space** poke. **Esc** pause, the Director's call after a goal.
The first screen: **Enter** or the pad's **A** plays one match at the level that is lit; left and right change the level; a click on a level picks it and
Enter still plays.

## Known to be unfinished (do not spend your time finding these)

- **Two marks on the ice are new tonight and nobody has played with them** (see the table): if they read badly or get in the way, say so first.
- **You are on your back about 9 % of play at pro**, and two knockdowns in three land within half a second of your winning the puck: nothing on screen
  warns you that an opponent arrives with it. A mark for that, and the stop's distance drawn on the ice, are being built now.
- **A skater can stand at a downed body and push it along the ice.** The fix (a push-out) is built and off: it moves all-star about four points. The brain
  steering round him is being measured as the cheaper cure.
- **Opponents score from the red line** (their median shot is released from 36 m at pro): the long shot is one of the ladder's dials.
- **On big ice at pro** you can go half a minute without the puck in more than half the matches.
- **Your carry is short** (a second, median): half of the scripted player's carries end on his own shot or pass, so this number mostly measures the
  script; tell us how it feels.
- **STICK LOCKED** (bottom of the screen) shows only when Space misses a puck that an opponent is carrying within about 3 m of you; Space at a loose
  puck or at nothing costs nothing and shows nothing. Measured, never yet seen by an eye in a window.
- **Glass House**: the penalty box is below the camera's picture, so you never see the boxed skater sit; its look (blue ice, a dark hall) is another game's.
- In corners the camera looks down steeply. The attacked net can be out of frame inside the blue line when you work the boards.
- A good player wins Cage, Fog of War and Power Play nine times in ten; Sniper is hard on a newcomer (one in ten).
- The loss card says only "REMATCH?". Device B's goal effect did not show on the ice in the window check.
- Commentary: 412 lines are recorded; whether you hear them all depends on B's audio at this commit: say if the booth is silent.

## What the Director needs from you at 10:00

1. **Feel:** the stop-and-go, the two hits, keeping the puck, Space: right, too easy, too hard? One word each is enough.
2. **The ladder:** three different games? And which player are you: do your checks land?
3. **RMB:** turn on the cooldown (mashing stops paying; costs the ladder under a point) and the lunge (a press commits your body; about three points at
   all-star, paid back by the curve)? Yes or no to each.
4. **The look:** clay with the stands as the first look: yes? Anything that reads badly from the game camera?
5. **Which three modes stay on the page**, if only three could.
6. **New verbs from device B** (figures, sounds and effects exist; none is wired): a **dash**, a **slide tackle**, a **dive block**, and toy weapons for the modes
   (a giant stick, a felt mallet, a puck cannon). Does the last day add verbs at all? If one: which?
7. **The submission's open questions** (`submission/README.md`, section 1, fourteen of them): team names and roles, who submits, the prizes, whether the page
   names tool vendors, the dev log's hosting and device B's transcripts, the P4 removal.
8. **Device B still waits for you** on three things: the zip for a clean-machine test, the video with sound, your word on exporting its transcripts.
9. **The last 24 hours:** what matters most to you now: more feel, more polish, the video, or the page.
