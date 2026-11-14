# Lane FILM2 report (phase B, on the GPU): SK28 and SK29 seen for the first time

Device B, tree `C:\Users\kazda\jam\wt\b-fig`, one editor on the GPU (port 8004, pid 40068, 06:06 to 06:41, left through `ExitEditor` and gone). Clay level, un-paused, 60 frames a second. No commit, no push. 17 strips in `devlog\shots\` as `20261114-0634NN-B-film2-<what>.png`, each with a `.txt` caption.

## Verdict

**Merge `lane/b-fig` (SK28 and SK29) into dev-b as is.** A normal match is clean, and nothing I found broken can be reached in today's game.

- **The match:** one seeded clay match on the GPU on the final state (seed 1, the whole 120 s, 4 goals): 0 Blueprint errors, every get-up on the release (3 of 3, end minus release 0.0), the sim's knockdown and stumble lines identical to SK28's.
- **What is broken is in the pairs, and the pairs need calls the game does not make yet:**
  1. the pin's victim falls onto the attacker and lies under his skates;
  2. both slide-tackle victims land on the slider and get up through him.
- **Three things to settle before the other device wires the sets** (none blocks the merge):
  1. a push on the victim for the slide tackle and the pin, or other victim takes;
  2. the skid's hold is a statue for as long as the brake is held;
  3. the dive block's jolt cannot be seen.
- **Export34's middle and heavy checks and `Lift_Heavy`: import them as the defaults.** The attacker never slides backwards (0 frames in the four export34 pairs filmed), the shoulder reaches the chest, and the heavy is its own picture.
- **Export35's new back get-up: do not make it the default yet.** From above its quarter turn reads as a spin on the ice.

How to read the strips: the label on each frame is the take and its frame. Screenshots land one frame after the frame they were asked on; the labels already allow for that. "Game scale" means the close camera's frames shrunk until the figure is about 110 px tall: the size is the game's, the camera angle is not.

## B1. The sets with the real props

| Set | Reads at the game's scale | Closer | Seat, and anything through a body or the ice |
|---|---|---|---|
| Cannon: carry | reads: an orange keg held at the belly | the crank cannot be made out at either size | the keg sits in both arms; nothing through the body |
| Cannon: fire | reads: the keg kicks up and he steps back at frames 8 to 10 | the same | clean |
| Cannon: kick | reads: he is thrown back and a keg lies on the ice beside him | no frame with two kegs; the keg is on the ice in the frame it leaves his arms (11); his stick is back at the get-up | the keg rests on the ice, not in it |
| Giant stick | **reads best of all**: the blade is the biggest thing on the ice | carry with the blade just over the ice ahead; the poke slides it forward; the sweep swings it across | both hands on the shaft at this size; the blade's corner comes down to the ice in the sweep (frame 15; 1.6 cm over it by SK28's numbers), not through it |
| Mallet: swing and bonk | reads: a red drum over the head, then the victim sits down and lies back | at the lab's spacing the two stand almost shoulder to shoulder before the swing; the drum is inside the victim's hip and thigh for frames 18 to 21 | contact as meant; the victim's own stick lies across the attacker's skates the whole time |
| Mallet: miss | reads as an over-swing | he ends turned to the camera, the drum low, then back on the carry | clean |
| Rolling pin: carry and roll | **weak**: a small white roller at the belly; only the wind-up over the head reads | the low shove reads closer | in the hands |
| Rolling pin: its victim | **broken as a pair** | the victim tips forward through the attacker (frames 17 to 21) and lies face down under him; the attacker stands on him through the next move | bodies through each other from frame 17 |
| Dive block, sides | reads: a low dive onto the side, stick out | the act shows closer (a leg up at 17, the stick and glove out) | on the ice, not in it |
| Dive block, struck | **does not read** at either size: frames 2 and 5 of the jolt look like the lying pose | | |
| Dive block, front | reads: flat on the front, arms and stick ahead | the get-up after it shows the new hand-and-knee beat | clean |
| Slide tackle | the slide reads: in low, legs first | | |
| Slide tackle's victims, no travel | **broken as a pair, both** | from the front (`Fall_Legs_Front`): he tips forward over the slider and lands on him, one heap from frame 15. From behind (`Fall_Legs_Back`): his feet fly up and he sits down on the slider's legs. Both pairs get up through each other | bodies through each other from frame 13 to the get-up |
| Dash: start, three cycles, end | reads: a deep crouch, the stick tucked back, long low strides; the rise back to the stride | the hand-over to `Stride` cannot be seen | clean |
| Skids with a hold | read as a two-skate stop turned side-on | **the hold is a statue**: the same picture for 30 frames, no spray, nothing moving | clean |
| Bump | **weak**: a rise and a turn of the shoulders, then the crouch again | his back and number come round at frames 5 to 7 | clean |
| Wipeout | reads: face first, flat, a slide, the quick get-up | | clean |
| Pickup beat | **weak**: the free hand goes to the head for a few frames | | clean |

Stills:
- `20261114-063401-B-film2-cannon.png`
- `20261114-063402-B-film2-giant.png`
- `20261114-063403-B-film2-mallet.png`
- `20261114-063404-B-film2-pin.png`
- `20261114-063405-B-film2-dives.png`
- `20261114-063406-B-film2-slides.png`
- `20261114-063407-B-film2-dash.png`
- `20261114-063408-B-film2-skids.png`
- `20261114-063409-B-film2-bump-wipeout-pickup.png`
- `20261114-063416-B-film2-game-scale-a.png`, `20261114-063417-B-film2-game-scale-b.png`

How the pairs were placed: the mallet's victim 1.08 m ahead and 0.72 m to the attacker's left, called on frame 16. The pin's victim 0.96 m ahead, called on frame 16. The slide's victims with their skates 20 cm (front) and 30 cm (behind) past the slider's leading skate at contact, called on frame 10; the slider's root at 10 cm a frame, slowing to a stop over 20 frames after contact; the victim's root not moved at all.

## B2. The get-ups as they play now, and the light check as a shove

| Strip | What a viewer sees |
|---|---|
| Back get-up, re-timed, no blend | he sits up at an even pace over ten frames, comes onto a knee, rises. No jump at the start |
| Front get-up, hand and knee | a push-up onto all fours with the head low, a held beat, then onto a knee and up. **The best of the four**: it looks like a man collecting himself |
| Side get-ups, export34 | he curls onto his knees with the **helmet on the ice** for about ten frames: it reads as bowing, not as getting up |
| Side get-ups, export35 | the same beat with the head up: all fours, then up. **Better: use these** (they are in the tree now) |
| Quick and snap from the back, export35 | a brisk sit-up and rise; no jump |
| Export35's new back get-up beside the tree's | below |
| Light check as a shove, 80 cm apart | the two stand chest to chest before it starts. The shoulder dips and the glove goes to the chest; **the helmets and chests overlap for frames 3 to 6**. It reads as a shove at close quarters, and the victim rocks back |

**Export35's back get-up beside the tree's, a 150-frame knockdown each:**
- From the side it tells the better story: a roll onto the side, a hand and a knee down, then up.
- **From above it spins.** His legs sweep a quarter circle over the ice in frames 1 to 10 with nothing to cause it, and he rises facing where the tree's take faces. The game looks from high up, so this is what a player would see.
- The lift off the ice (up to 5.5 cm) cannot be seen at these sizes.
- As played its head rises 7.7 cm a frame at most (the tree's: 5.8), and it ends on the release.
- **As a viewer: the tree's take is the cleaner one today.** Export35's is worth having once the turn is out of it.

Beside QA5: QA5 filed no get-up strip, only `20261114-044800-B-qa5-clay-knockdown.png`. I could not replay QA5's takes beside these, so the comparison with QA5 is by the numbers in `status\SK29-report.md` (the head rose 9.3 to 12.3 cm a frame then, 5.3 to 7.9 now).

Stills:
- `20261114-063410-B-film2-getups-export34.png`
- `20261114-063411-B-film2-getups-export35.png`
- `20261114-063412-B-film2-getup-back-export35-beside.png`
- `20261114-063413-B-film2-light-shove.png`

## B3. The film the lab is waiting for

Export34's takes were brought in under a test name for the session (clay) and are deleted again. Both roots were driven by the lab's rule. Body check: the take's frame 0 three frames before contact, the roots 80 cm apart on the contact frame, then the attacker at 0.4 v + 0.15 m/s and the victim at 0.6 v - 0.15. Cross-check: frame 0 two frames before, 120 cm apart, 0.5 v - 1.5 and 0.5 v + 1.5. The victim got a knockdown on the contact frame.

| Pair | Hips moving backwards over the ice | Shoulder to the victim's chest joint on the take's contact frame | What a viewer sees |
|---|---|---|---|
| Export34 middle, 6 m/s closing | **0 frames** | 6.0 cm | the right shoulder goes into the chest, the victim goes down on his back, the attacker skates on. For frames 2 to 4 the attacker's trunk is inside the victim's; afterwards he stands over the victim's legs |
| Export34 middle, 3.85 m/s (the band's low end) | **0 frames** | 6.1 cm | the same picture, slower |
| Export34 heavy, 9 m/s | **0 frames** | 15.4 cm (the victim is already falling away) | **its own picture**: the attacker leaves the ice, the free arm flung up and the stick behind; the victim is thrown level through the air and lands at frame 13; the attacker lands, staggers with the stick high, and is upright by 20 |
| The heavy in the tree today, 9 m/s | 1 frame, 0.5 cm | 55.9 cm: it does not meet | a low lunge with both skates on the ice; it looks like a harder middle |
| Export34 heavy cross-check, 9 m/s | **0 frames** | stick socket to chest 11.8 cm, 120 cm apart | a dive through the shove, then the stick driven up over the head (frames 11 to 13); the victim is thrown furthest of all |

- **Does the attacker slide backwards: no**, in all four of export34's pairs. The tree's own heavy has one frame of 0.5 cm.
- **Does the shoulder meet the chest: yes** for the middle (6 cm). For the heavy the shoulder is within 15 cm of a chest that is already leaving.
- **Does the heavy read as its own picture: yes.** Beside the tree's heavy the difference is plain at a glance.
- **Import them as the defaults: yes**, in a lane without the GPU, by name on both figures, with a seeded match after.

Stills:
- `20261114-063414-B-film2-check-middle-heavy.png`
- `20261114-063415-B-film2-heavy-tree-and-crosscheck.png`

## B4. The physics lane's film

Not done. The time went to the lead's export35 addition and to running the match again after my own overlap (below).

## What went wrong in the running

- **I started one film session while the first GPU match was still running** (06:20). The match was discarded and run again alone, twice.
- **My first framing of the export35 comparison cut one figure off**; I filmed it again wider. The strip filed is the second.
- **In the first session the match's own figures skated through one frame** (the giant sweep's first frame). From the second session on they are hidden.
- The temporary import folder I wrote into the tree by mistake is in `status\SK29-report.md`; it is removed.

## Not verified

- **Sound.** Game sound was off for every session; none of the sets' sounds has been heard.
- **The game's own camera angle.** The game-scale sheets are the close camera shrunk.
- **Motion.** I judged from frames taken un-paused, not from watching it run. The bump's return and the skid's hold are judged from numbers and stills.
- **The night level and the night figure.** Everything here is clay.
- **The left-shoulder middle and heavy, and the half-reach takes.** Only the right-shoulder takes and `Lift_Heavy` were filmed.
- **The checks in a real match.** The pairs were driven by the lab's rule, with no hit-stop and no friction.
- **The brake effect's strength on screen.** I did not film a brake.
- **The keg's crank turning.**

## For the Blender lanes

- **Weapons and dive:** the pin's victim needs a fall away from the attacker. `Fall_Legs_Front` brings him down on the attacker.
- **Dive:** `Dive_Block_Hit_*` cannot be seen. It needs a bigger jolt (the whole body lifting, or the stick knocked round).
- **Dive:** the slide's victims are right in their first frames (the legs go, he tips). The landing is the game's to fix with a push; a take cannot do it alone.
- **Dash:** the skid's hold frame needs something alive: a two- or three-frame shiver to loop, or the spray.
- **Dash:** the bump is a twitch at the game's scale. A bigger shoulder, or a recoil of the head, would carry it.
- **Dash:** the pickup beat is small. The arm fully up for six frames would read.
- **Weapons:** the rolling pin is the weakest prop at the game's scale. A longer or darker roller, or a carry held out to the side like the giant stick's, would help.
- **Figure lab:** export34's middle and heavy checks and `Lift_Heavy` film well: no backward slide, the heavy its own picture. In the middle the attacker's trunk is inside the victim's for frames 2 to 4.
- **Figure lab:** export35's side get-ups are right. Its back get-up: take the quarter turn out (roll without turning on the ice), then it can replace the old one.
- **Figure lab:** the light shove at 80 cm overlaps helmets and chests for four frames. The glove could meet the chest with the trunk held 10 cm further back.

## For the other device

- **The slide tackle and the pin need the victim moved.** With no travel both victims land on the attacker. About a metre sideways for the slide (the dive lane's number); for the pin, away from the attacker.
- **A light check at 80 cm puts the two figures chest to chest.** If the contact distance can be 90 to 100 cm for the light size, the shove has room.
- **The skid's hold has no spray.** `SetBraking(true)` during a dash skid starts no brake effect today. If the game wants spray there it must come from the figure's side; say so and lane SK will add it.
- The calls and their frames are as in `status\SK28-report.md`, with the pin's victim on frame 16.

## Decisions for the operator

1. **Merge SK28 and SK29 now, or hold them for a second look.** I say merge: the match is clean and the broken pairs are unreachable until the sets are wired. Stills: all of the above.
2. **The slide tackle's victim: a push in the game, or accept the heap.** `20261114-063406-B-film2-slides.png`.
3. **The pin: ship it with a push on the victim, wait for another victim take, or drop weapon 4.** It is also the prop that reads least. `20261114-063404-B-film2-pin.png`, `20261114-063416-B-film2-game-scale-a.png`.
4. **Export34's middle and heavy checks as the defaults.** I say yes. `20261114-063414-B-film2-check-middle-heavy.png`, `20261114-063415-B-film2-heavy-tree-and-crosscheck.png`.
5. **Export35's back get-up.** I say not yet: it spins from above. `20261114-063412-B-film2-getup-back-export35-beside.png`.
6. **The skid's hold: add spray, add a small loop, or cap the hold at about 15 frames.** `20261114-063408-B-film2-skids.png`.
7. **The dive block's jolt: bigger, or cut the call.** `20261114-063405-B-film2-dives.png`.
