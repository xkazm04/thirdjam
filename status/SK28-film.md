# Lane SK28: what the GPU must film

Everything in lane SK28 was measured without a GPU: bone positions, prop vertices and log lines. Nothing was seen or heard. This is the list for whoever next holds the GPU, in the order I would film it.

## Before filming

- Tree: `C:\Users\kazda\jam\wt\b-fig` (or dev-b after the merge), clay level first, then one pass on the night level.
- The figures' own log lines are now off by default. Switch them on after the figures spawn: `ke * SetLog 1`.
- Every move below is on the human's figure through `ke * SK26Test <n>`. The trigger answers on the human's figure only.
- Sound: play with the game sound on. The headless runs had it off, so every sound below is unheard.
- Camera: the game camera for the read, then one closer pass (3 m, hip height) for the props and hands.

## 1. The props in the hands (job 3)

| Shot | Trigger | Look for |
|---|---|---|
| Giant stick, standing and skating | `SK26Test 5`, skate two laps | the shaft in both hands, the blade out to the right and clear of the ice (measured 14.9 cm over the stride) |
| Giant poke and sweep | `SK26Test 9` facing the target (poke), `SK26Test 10` (sweep) | the blade low over the ice: 0.6 cm in the poke, 0.5 cm (night) and 1.6 cm (clay) in the sweep; no dip through the ice |
| Mallet carry, swing, miss | `SK26Test 6`, then `9` and `10` | the drum's lowest point 15.8 cm (clay) and 18.6 cm (night) in the miss; the bottom hand on the handle |
| Cannon: keg and crank | `SK26Test 7`, then `10` (fire) | the keg at the right hip on the left hand, the crank on top of it in the right hand; the crank turning over frames 1 to 8; the two parts stay together (0.9 cm between them in every measured frame) |
| The kick | `SK26Test 7`, then `9` | both parts gone on frame 11 and a keg lying on its side 1.5 m to his left in the same frame; no frame with two kegs; the stick back in his hands at the get-up; the dropped keg gone 4 s later |
| Rolling pin | `SK26Test 25`, then `9` (roll) and `10` (miss) | the pin raised over the head in the wind-up, the low shove; from behind it barely reads (the set lane says so) |
| Prop in a dash | any weapon, then `SK26Test 12` | the prop goes and the stick comes back for the dash, and the prop returns after the hand-over: is that swap acceptable on screen |

## 2. The victims (job 3)

| Shot | Trigger | Look for |
|---|---|---|
| Slide tackle from the front | `SK26Test 23` | `Fall_Legs_Front`: the skates swept back, onto the forearms and chest, face down, then the front get-up |
| Slide tackle from behind | `SK26Test 24` | `Fall_Legs_Back`: the skates swept forward and up, onto the hips and back |
| A real pair | a bot slide-tackling the carrier, once the other device calls `PlaySlideTackleHit` | whether the victim lands clear of the slider. It does only if the game keeps the carrier's velocity and pushes it about a metre sideways |
| The pin's pair | needs two figures 0.96 m (clay) or 1.04 m (night) apart | the roller on the shins at the attacker's frame 16; the victim's call belongs on that frame, not on 14 |

## 3. The dash (job 5)

| Shot | Trigger | Look for |
|---|---|---|
| Start, loop, end | `SK26Test 12` in open ice at speed | the start into the loop; three loop cycles; the end into the stride. The skates' step at the hand-over measured 1.8 cm (clay) and 3.2 cm (night) |
| Skid, right and left, held | hold the brake, `SK26Test 13` or `14`, let go after half a second | the figure frozen on the skid's frame 8 while the brake is held (measured 30 frames), then the rest of the take; does the frozen pose read as a hold or as a stall |
| Skid, not held | `SK26Test 13` without the brake | the 20-frame skid by itself |
| Bump | `SK26Test 15` and `20` | the take returns to the loop fast: the head moves 12 cm a frame in its last two frames. Is that a snap on screen |
| Wipeout | `SK26Test 16` | face down from frame 20, the quick front get-up |

## 4. The loop and the get-ups (job 4)

| Shot | Look for |
|---|---|
| A long knockdown onto the front (150 frames or more) | `Lie_Loop_Front` with a still left arm: the flutter is 0 by the numbers |
| The four get-ups after a full-length knockdown | the rise at the start: is it the weight coming up, or still a pop. As played the head rises up to 9.7 cm (back), 12.3 cm (front) and 8.5 cm (sides) in one frame, because the 5-frame blend out of the lying loop holds the start back and then catches up; the takes by themselves rise 5.3 to 7.3 cm |

## 5. By ear (job 1)

Each sound starts once per move at its frame by the log. Unheard:

| Sound | Trigger | Listen for |
|---|---|---|
| `MS_dive_block`, `MS_dive_block_hit` | `SK26Test 1`, `2`, `22`; then `3` while lying | level against the knockdown sounds |
| `MS_slide_tackle` | `SK26Test 4`, `21` | |
| `MS_dash_start`, `MS_dash_loop` | `SK26Test 12` | the loop coming in at frame 10 and going at the dash's end (a 0.05 s fade); whether the 0.4 s cycle reads as skating; the pitch at other rates (`SK26Test 18`) |
| `MS_dash_skid` | `SK26Test 13`, held and not held | the body looping while held and the tail at the release; a click at the jump from body to tail |
| `MS_dash_wipeout` | `SK26Test 16` | on the landing (frame 13) |
| `MS_boost_take` | `SK26Test 17` | |
| `MS_giant_poke`, `MS_giant_sweep` | `SK26Test 5`, then `9`, `10` | |
| `MS_mallet_swing`, `MS_mallet_bonk` | `SK26Test 6`, then `9` (swing and bonk) and `10` (swing only) | the bonk on frame 15, on the hit only |
| `MS_cannon_fire`, `MS_cannon_kick` | `SK26Test 7`, then `10` (fire) and `9` (kick, no fire sound) | |

## 6. The brake effect (job 2)

- Brake from full speed and from half speed. The spray should be visibly stronger from full speed: `User.Strength` is the speed at the brake over 14 (0.71 from 10 m/s, 0.64 from 9 m/s by the read-back).
- A skid out of a dash starts no brake spray at all (the skid outranks the stop). Say whether it needs one.

## Stills to file

One still per row of sections 1 to 3 at the named frame, and a strip of the kick (frames 8, 10, 11, 14, 23). Name them `devlog\shots\<stamp>-B-sk28-<what>.png`.
