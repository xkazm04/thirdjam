# Play me: what to try on Saturday morning, and what to tell the Director after (R-0056)

**The package:** the newest `Game/Packaged/Hokej_Hokej_H*/Hokej_Hokej_H*.zip` named on the last `package` line of `status/A.md`. Unzip into a fresh folder, run the exe.
If Windows asks about the firewall, that is the joined match's socket: Cancel is fine for solo play.

## What changed since the package you last played (H26, Friday 19:46)

| You will notice | Why |
|---|---|
| The rink is **clay** from the first match | the regular cut and takes 1 to 4 moved to the clay look (R-0054) |
| **No whistle** in the regular game and the season | fouls and the box are gone there (R-0048); Glass House keeps them |
| **You keep the puck** until somebody presses for it | an unpressed blade or a body's touch no longer takes it; half a second of grace after you win it |
| **Stopping works** | S against your motion brakes hard; a quick first step after a stop, a cut or from rest |
| Hits have **sizes** | a bump, a stumble, or down for 1 to 2.5 s, by the speed brought; the HUD says why a press did not land |
| You are not knocked down again the moment you get up | 2 s of grace after a get-up |
| Your **mates pass to you** and go for loose pucks | the brain plays as a side; bots brake on purpose |
| The **arena answers**: a goal lights a lamp and shakes the net, a board hit rocks the boards | clay benches, boxes, goal frames, caps |
| The camera **keeps you in frame** at the boards; a yellow chevron marks you | the screen track |
| The **menu**: (from H28) PLAY NOW, a controls screen, twelve mode cards you can read | the menu track |

## Read this first (from the second design review, `docs/design/REVIEW-2.md`, 7,312 measured matches)

- **The level you pick now reaches the match** (it never did before tonight: the regular cut always played pro, where a newcomer wins 18 %). PLAY NOW and
  the cut cards play your level; **your first PLAY NOW is ROOKIE** even if you had pro saved (the old save cannot be told from a default); change it in
  SETTINGS. The season's takes and the modes set their own level. The match card and the pause card show the level in force. This is true of packages from
  H27 on: check the `package` line in `status/A.md`.
- **The stop works from 8 m**, the distance from the centre to a winger at a faceoff. From 3 to 6 m it gets you flattened.
- The three good things to look for are real: **no whistle** in the regular game and the season, **hits that obey speed**, **the stop-and-go**.

## Ten minutes, in this order

1. **A first match on the regular cut at rookie.** A newcomer wins about 85 % there. Does the first minute explain itself?
2. **The stop-and-go.** Carry at a defender, press **S against your motion** when he is about 8 m away (five stick lengths; against an all-star 12 m, eight), then go
   the other way. At pro it should work nearly every time from that distance, and never if you just charge. `jam.ctrl.Drill 21` in a Development build
   stages it and says the distance on screen. This is the game's central skill: does it feel like one?
3. **Two hits.** Skate beside an opponent at his speed and check him (RMB): a nudge. Then hit one head-on at full speed: he goes down. Same button.
4. **Keep the puck.** Carry with a defender on your back and turn your body between him and the puck: his stick should not reach it.
5. **The season's take 3 and take 4.** Take 3 is easy now (a novice wins 86 %); the step to take 4 is still too steep (the curve lane is on it).
6. **Pro, one match.** A regular player wins about half. All-star is the wall (about a quarter).
7. **Three modes:** Shotgun (F clears the lane), Cage (tight box, the boards hit back), Pond (five a side, pass and skate, nobody gets hit).
8. **A joined match**, if device B is at hand: `docs/experiments/JOIN-GUIDE.md`. Two processes play to the final card with a rematch on this machine;
   two physical machines have never been tried.

## Controls

Mouse aims the stick. **LMB** shoot (a tap is a flat shot, hold to lift). **RMB** check. **MMB** cross-check or lift. **Q** pass (hold for a saucer). **F** ability.
**E** spin. **Wheel** deke. **Shift** sprint. **S against your motion** brake. **Space** poke. **Esc** pause, the Director's call after a goal.

## Known to be unfinished (do not spend your time finding these)

- On big ice (the large and wide cuts, three a side) you can go half a minute without the puck in about six matches in ten at pro. The brain track is on it.
- **Opponents score from the red line** (their median shot is released from 36 m at pro): the long shot is one of the ladder's dials; changing it is the
  curve track's next pass.
- **Space (the poke) rarely takes the puck and a miss trips you**: the carrier track's next evolution.
- You are on your back about 8 % of play at pro. Six modes still blew the whistle on the review's commit (Shotgun among them): the modes lane removes it
  everywhere but Glass House.
- All-star on the regular rink is a little soft (a good player wins 30 %, the target is 26 %).
- The season's middle: takes 4 to 7 are too close to each other and too far from take 3.
- The attacked net can be out of frame inside the blue line when you work the boards.
- The loss card says only "REMATCH?" until the menu track's second evolution lands.
- The start of a match can still hitch once (one frame of 40 to 60 ms).
- Commentary: all 412 lines have a recording from device B; they are in the build only when B's audio is merged at the package's commit.

## What the Director needs from you at 10:00

1. **Feel:** the stop-and-go, the two hits, keeping the puck: right, too easy, too hard? One word each is enough.
2. **The ladder:** did rookie, pro and all-star feel like three different games?
3. **The look:** clay as the first look: yes? Anything that reads badly from the game camera?
4. **Which three modes stay on the page**, if only three could.
5. **The submission's open questions** (`submission/README.md`, section 1): team names and roles, who submits, the prizes, whether the page names tool vendors,
   the dev log's hosting and device B's transcripts, the P4 removal.
6. **The last 24 hours:** what matters most to you now: more feel, more polish, the video, or the page.
