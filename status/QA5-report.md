# Lane QA5 report: the verdict for the 09:00 package, the play-through, the new sets on screen

## The verdict: (a) merge all of `lane/b-fig`

I played the merged tree (c214e8fe) with the GPU: one whole clay match to its end, most of a second, and 100 s on night. In ordinary play I found **no Blueprint error, no figure stuck in a pose, every get-up ending on the release (14 of 14), and nothing visible that SK25's figures did not have**. The new moves are unused by the game and nothing of them ran in the matches.

Three things the lead should know before cutting the package; none is a reason against (a):

1. **A warning that is not mine and may show in the package.** At the start of every play session the log says `MI_Toon_Blue` and `MI_Toon_Orange` are "missing usage flag InstancedStaticMeshes! Default Material will be used in game." In the editor the crowd in the stands draws correctly (the editor compiles the flag on the spot). In a cooked game that message means the instanced meshes using those two materials get the grey default material. It came with the merge of dev-b (the stands), so it is on dev-b with or without my branch. Somebody should look at the stands in the cooked build, or tick the flag on the two materials and resave.
2. **One of my last SK26 edits had not been run before; it has now.** The light check takes fading over 6 frames: checks and cross-checks were filmed in play (strip filed) and look right.
3. **I ended the Zen server once, by accident, while another lane's editor used it.** My first GPU start hung at "Waiting for ZenServer to be ready"; I stopped my own hung editor with the launcher's `-Stop`, which takes the process tree, and the Zen server it had just launched went with it at 04:34. An editor of another lane (pid 36136) had connected to that server a minute before; it exited at 04:37 with code 0. I do not know whether my stop hurt its work. The second start hung the same way; that time I ended only my editor process, and the third start found the server and was up in 15 s.

Faults inside the new moves (they cannot occur in a normal match; listed, not blocking) are in B2.

## The session

Tree `C:\Users\kazda\jam\wt\b-fig` (`lane/b-fig` c214e8fe), port 8004. GPU editor pid 37956, started 04:37:31 (58 `LogD3D12RHI` lines), left through `ExitEditor` at 04:55:52 and confirmed gone (0 Blueprint errors in its whole log; 940 assets loaded and no dirty package at the end check, the five lab files of B4 among them, since removed). No GPU hang. No commit, no push, nothing written to any asset. `r.Editor.Viewport.OverridePIEScreenPercentage` was 0 for the stills and is back at 1; `Slate.bAllowThrottling` was left at 1 as found (with a window the play world ticks).

The editor ran the play sessions at about 40 sim frames a second while I captured.

## B1. The play-through

**Not from the menu.** I did not drive PLAY NOW; the matches were started as the regular cut by console (`jam.Cut` on a copy of `hockey.regular` with the seed set, `jam.ScriptHuman 1`). The log says `JAMMATCH level=pro source=cartridge`.

| | Clay, seed 1 | Clay, seed 7920 | Night, seed 7920 |
|---|---|---|---|
| Played | to the match's end: frame 6890, the fourth goal, 1 : 3, the BEATEN card | 7,800 frames (three goals, the third take begun) | 6,329 frames (100 s) |
| Blueprint errors | 0 | 0 | 0 |
| `Error` lines of any kind | 0 | 0 | 0 |
| Knockdowns | 3 | 6 | 5 |
| Get-up end minus release | 0.0, 0.0, 0.0 | 0.0 in 6 of 6 | 0.0 in 5 of 5 |
| Stumbles shown of sent | 3 of 3 | 0 sent | 0 sent |
| The human's stops shown, of `on=1` brake lines | 8 of 18 (11 started from 7 m/s) | not counted | not counted |
| Each bot's stops shown | 5 to 8 | not counted | not counted |
| Whips started | 33 | not counted | 43 |
| A figure stuck in a pose | none seen | none seen | none seen |

**Falls and get-ups chosen** (every knockdown of the three sessions):

| Session | Frames down | Fall | Get-up | End minus release |
|---|---|---|---|---|
| Clay 1 | 150 | `Fall_Heavy` | `GetUp` | 0.0 |
| Clay 1 | 150 | `Fall_Heavy` | `GetUp` | 0.0 |
| Clay 1 | 88 | `Knockdown_Left_Light` | `GetUp_Quick_Side_Left` | 0.0 |
| Clay 7920 | 98 | `Knockdown_Left` | `GetUp_Side_Left` | 0.0 |
| Clay 7920 | 137 | `Fall_Heavy` | `GetUp` | 0.0 |
| Clay 7920 | 150 | `Knockdown_Left_Heavy`, replaced by `Board_Fall_Front` | `GetUp_Wall` | 0.0 |
| Clay 7920 | 136 | `Knockdown_Left_Heavy` | `GetUp_Side_Left` | 0.0 |
| Clay 7920 | 83 | `Knockdown_Right_Light`, replaced by `Board_Fall_Front` | `GetUp_Wall` | 0.0 |
| Clay 7920 | 64 | `Knockdown_Right_Light` | `GetUp_Snap_Side_Right` | 0.0 |
| Night | 98, 137, 150, 136, 83 | the same five as clay 7920 | the same | 0.0 in all |

**What I saw, as the operator will:**
- **Clay reads as a match.** The stands and the crowd are in most frames and sit behind the play without fighting it. The score plate, the period card, the key line, the minimap and the commentary line are all there. The goal light floods the corner red on a goal, strongly; for half a second the figures near that net are pink.
- **The first seconds:** the faceoff, the human crouched with the stick out, the ring under him, opponents in their stances. Nothing pops.
- **A knockdown with the lying player:** falls, lies and moves a little, gets up and skates away. At the boards the body lies against them without crossing.
- **A stop with the new fan:** the crouch across the travel reads as a hockey stop, and the pale spray fan is there for about six frames. It is a small, quick effect at this distance: I see it in the strip, I would not swear to seeing it in motion.
- **A board hit:** I have no still of a board hit as such (my watcher has no trigger for it, and 25 happened). The knockdown at the boards and the night board fall are filed in its place.
- **Checks and cross-checks:** six caught near the camera. They read as hits: a lunge, a recoil, the effects lane's flash. No pop between the sizes.
- **Night** is darker and busier (the streaked ice) but nothing is broken: the same falls, the same get-ups.
- **Anything that looks broken:** nothing in the figures. See the material warning above for the stands in a cooked build.

**Warnings in the log** (all sessions): the two material warnings above; "the material instance will recompile every editor launch until resaved" twice; two performance notes about my own `Screenshot` and `ke` console calls.

## B2. The new sets on screen

Fired with `ke * SK26Test <move>` on the human's figure, clay, un-paused, in two sessions (the human standing; the human skating in a match). Each move is filed at the game camera's scale and as a closer crop of the same stills; I set up no second camera.

| Move | As a viewer | Notes |
|---|---|---|
| Dive block left, right | **reads** | The body goes flat across the line with the stick stretched out; lies; gets up. |
| The dive's jolt (`PlayDiveBlockHit`) | **does not read** | At game size frames 50, 52 and 56 look the same. |
| Dive block front | **reads** | From behind: a belly slide away from the camera, arms and stick forward. |
| Slide tackle left, right | **reads** | A turn, a feet-first slide, onto the back, up. **No victim:** there is no victim take yet and the trigger fires the tackler alone. |
| Mallet's victim (`Fall_Bonk`) | **reads** | Sits down hard and lies back. |
| Giant stick: carry | **does not read from behind** | It points away from the camera; only its butt shows at the glove. **A stand-in** (the figure's stick, 1.7 times as large). |
| Giant stick: poke, sweep | **reads** | A long pale stick thrust out to the right; the sweep crosses the body. Stand-in. |
| Mallet: carry, swing, miss | **reads as a stick swung like an axe** | Up beside the helmet, round low and across. **A stand-in with no drum**: it is not a mallet to the eye. |
| Cannon: carry | **reads** | A banded keg at the right hip, clearly a thing being carried. The real prop. |
| Cannon: fire | **reads** | The keg jumps. No puck leaves it (not mine to send). |
| Cannon: kick | **reads** | Fires, sits, rolls onto the side with the keg, gets up; the keg is back in the carry. |
| Dash: start, loop, end | **not judged** | In my session the human dashed along the boards with an opponent on him; the low driving pose shows, the loop and the hand-over to the stride cannot be told apart in that crowd. The feet were measured (SK26), not seen. |
| Dash: skid left, right | **not judged** | One ran at the boards, one under the goal light. |
| Dash: bump | **reads** | A low shoulder lunge. |
| Dash: wipeout | **reads** | A belly flop, lying face down, up. |
| Boost pickup's beat | **reads** | An arm thrown up for a few frames. Fired once. |
| Carries over the stride | mallet stand-in and cannon **read**; giant stick **not seen** | The giant stick's still has an opponent in front of it. |

- **The prop's seat in the hands:** the cannon sits at the right hip under the right arm and stays there through the fire and the kick; it looks held. The two stand-ins sit in the right glove like a stick, because they are one.
- **The stick hidden or not:** hidden in every still with a prop, back in every still without. No still shows both.
- **Broken:** nothing.

**Faults inside the new moves** (none can occur in a normal match):
1. The dive's jolt is too small to see.
2. The giant stick's carry is invisible from the game camera when the figure skates away from it. That may be the stand-in's thin shaft; the real prop has a blade the size of a door.
3. The mallet is a stick until its mesh exports; the giant stick likewise.
4. The slide tackle has no victim.
5. The dash was not seen in the clear; it needs a session with the human alone on the ice.
6. From SK26, still open: `Boost_Take` a second time, the skid's hold, the idle cost.

## B4. The physics lane's newest film

The five files from `C:/Users/kazda/jam/rag-lab/film/loop3/` went into `Skaters/Lab/` for one play session and are removed again (the folder is gone from the tree). I ran `film3_make_clay.py` and filmed its first four pairs as strips, un-paused, from a camera 4.3 m in front of each: knockdowns of 60, 74, 88 and 96 frames, from the front, from behind and from each side.

**Does the red figure's stick still look loose: once it lies, no; in the first six to nine frames of two of the four falls, a little.** Lying, the red twin's stick sits where the blue figure's does. In the forward fall (pair 1) the red stick is flung out flat along the ice, wider than the blue one, around frame 6; in the first side fall (pair 2) it points down toward the camera for a few frames where the blue one already lies along the ice. The backward fall and the other side fall are alike in both.

## Stills

All in `C:\Users\kazda\jam\wt\b-fig\devlog\shots\`, each with a `.txt` caption:

- `20261114-044757-B-qa5-clay-first-seconds.png`
- `20261114-044758-B-qa5-clay-stands.png` (four moments, 2 by 2)
- `20261114-044800-B-qa5-clay-knockdown.png`
- `20261114-044801-B-qa5-clay-stop-fan.png`
- `20261114-044803-B-qa5-clay-goals.png`
- `20261114-044804-B-qa5-clay-match-end.png`
- `20261114-045224-B-qa5-clay-checks.png`
- `20261114-044805-B-qa5-night.png`
- `20261114-044807-B-qa5-night-knockdown.png`
- `20261114-044808-B-qa5-sets-dive-slide-game.png`, `20261114-044809-B-qa5-sets-dive-slide-close.png`
- `20261114-044811-B-qa5-sets-weapons-game.png`, `20261114-044812-B-qa5-sets-weapons-close.png`
- `20261114-044814-B-qa5-sets-dash-game.png`, `20261114-044815-B-qa5-sets-dash-close.png`
- `20261114-045537-B-qa5-rag-film3-pairs.png` (B4)

The periodic stills are the tool's capture of the game view (1502 x 883, with the HUD); the bursts are console screenshots of the window at its own resolution, without the HUD.

## Not done, not verified

- **The menu's PLAY NOW path.** Console start only.
- **B4 was done:** see its section above the stills.
- **Sound.** I listened to nothing; none of the sets' sounds is wired (SK27).
- **A board hit as a still**, and the stop's fan in motion.
- **The dash in the clear**, as above.
- **The cooked build.** I looked at the editor's play session, not at the package. The material warning is the reason to look.
- **The human as a person plays.** Every match was the scripted human.

## For the lead, in one place

- Verdict (a). No file needs taking from an older commit.
- Look at the stands' crowd in the cooked build (`MI_Toon_Blue`, `MI_Toon_Orange`, instanced meshes).
- Tell the lane whose editor was pid 36136 that its Zen server went away at 04:34 for a few seconds.
- Phase A's unfinished jobs (the sounds, the brake effect's strength, two of three matches) are in `status/SK27-report.md`.
