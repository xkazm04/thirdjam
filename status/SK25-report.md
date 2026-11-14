# Lane SK25 report: the loops' start frame, the reactive whip, three check sizes, all without a GPU

Tree `C:\Users\kazda\jam\wt\b-fig` (`lane/b-fig` at f21c3f08), built once (`ThirdJamEditor Win64 Development`, pass in 304 s). Port 8004. One editor of mine (pid 33400, started 02:12:37 with `-nullrhi -unattended`), left through `ExitEditor` and confirmed gone (log closed 02:59:38, pid gone at 02:59:40). The editors on ports 8002 and 8000 were not touched. Nothing committed, nothing pushed, no cook, no still, no strip.

**The null RHI was in use:** 0 `LogD3D12RHI` lines in `Game\logs\editor-20261114-021237-0.log`; line 915 loads `NullDrv`, line 919 says `rhiname="Null"`.

**Tick rate:** 60 sim frames per wall second (12,600 frames in 210 s). Every number is in sim frames.

## Three things first

- **A second trap for every headless lane: the play world did not tick at all.** Three play sessions sat at world time 0.0 with the sim at frame 0 while the engine's own frames ran. The cause is Slate's throttling, which holds the play world's tick. `Slate.bAllowThrottling 0` on the console starts it. The ragdoll lane's session ticked without it; mine did not, after an import and Blueprint writes. It cost me about five minutes.
- **Two matches gave 14 knockdowns, not 20.** Seeds 1 and 7920 at 210 s each: 9 and 5. My recorder caught 13 (it was armed after the first one of seed 7920). I added 28 knockdowns on test figures over ten lengths and both rules.
- **The proof matches ran before two late changes.** After them I halved the whip on plain collisions, stopped a weaker whip from restarting a running one, and wired the three check sizes. The lying-loop code did not change after the matches. One 100 s match (seed 1) ran on the final Blueprints: the same six knockdowns as the first 100 s of round 1, 6 of 6 cut clean, end minus release 0.0, no Blueprint error.

## Verdict per job

1. **Export29 and the start-frame rule: done.** The cut lands on a clean frame in 13 of 13 match knockdowns and 28 of 28 on test figures. End minus release is 0.0 in all of them. The travel line (13.5 cm for every lying time from 25 frames) holds for 9 of 11 in the matches; the two misses are the wall loops' second move, 12.1 and 13.1 cm at the wrist bone.
2. **The reactive whip: done, with one cost that is not flat.** Head 4.2, 8.4 and 13.9 cm at Strength 0.3, 0.6 and 1.0, zero on frame 20, root and take timing untouched. The idle cost is two space conversions per figure per frame that I did not remove.
3. **The filming script: written**, `status/SK25-film.md`.
4. **Export30: imported and wired by closing speed, not calibrated.** The size is chosen on the press from the closing speed on the nearest opponent; the two thresholds are guesses until they are set against the Strength the bus reports.

## 1. Import and the lying loops

### Import by take name

| | Night | Clay |
|---|---|---|
| Export29: takes in the file | 134 | 134 |
| equal to the asset already there | 94 | 66 |
| keys replaced (32 get-ups, 8 loops) | 40 | 40 |
| new | 0 | 0 |
| Export30: takes in the file | 142 | 142 |
| equal | 116 | 88 |
| keys replaced | 18 | 18 |
| new (the check and the cross-check, light and heavy) | 8 | 8 |
| clay aim holds kept | none | 28 |
| bad | 0 | 0 |

The tree had export26, so export29's import also carried the twenty-seventh and twenty-eighth passes. 871 assets load at the end (855 before, 16 new).

### The rule as wired

- When the loop starts, the figure knows when the get-up will start, so L is known: the frames from now to the get-up's first frame, rounded up.
- The start frame s is the table's: 0 for L under 16; 15 - L for 16 to 23; rule A 39 - L or rule B 93 - L (kept in their windows) for 24 to 84; 93 - L from 85.
- **A and B alternate by the bus.** Every figure hears every `knockdown` event and counts them; the one knocked down takes the count's parity. So two knockdowns in a row, whoever they hit, get different rules.
- The 5-frame cross-fade into the get-up and the get-up's own start are as in SK24.
- The figure prints `SK25 lie kind K L n s n cut n rule b` at every loop start.

### Proof: the two matches (clay, seeds 1 and 7920, 210 s each)

| Seed | Sim frame | Who | Frames down | Fall | Loop | Rule | Start | L | Cut | Clean | Largest travel cm | Get-up | End minus release |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| 1 | 1916 | human | 75 | `Board_Fall_Front` | `Lie_Loop_Wall` | A | 105 | 18 | 15 | yes | 4.3 (too short for a move) | `GetUp_Wall` | 0.0 |
| 1 | 2311 | human | 112 | `Knockdown_Left_B` | `Lie_Loop_Side_Left` | A | 93 | 54 | 39 | yes | 18.5 forearm | `GetUp_Side_Left` | 0.0 |
| 1 | 5046 | human | 67 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | B | 58 | 35 | 93 | yes | 13.7 skate | `GetUp_Snap_Side_Left` | 0.0 |
| 1 | 5276 | human | 85 | `Board_Fall_Front` | `Lie_Loop_Wall` | A | 12 | 27 | 39 | yes | 14.1 skate | `GetUp_Wall` | 0.0 |
| 1 | 5698 | human | 64 | `Knockdown_Right_Light` | `Lie_Loop_Side_Right` | B | 61 | 32 | 93 | yes | 13.8 skate | `GetUp_Snap_Side_Right` | 0.0 |
| 1 | 5918 | human | 121 | `Knockdown_Right` | `Lie_Loop_Side_Right` | A | 93 | 66 | 51 | yes | 16.9 hand | `GetUp_Side_Right` | 0.0 |
| 1 | 10141 | bot 7 | 84 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | B | 40 | 53 | 93 | yes | 16.9 shin | `GetUp_Quick_Side_Left` | 0.0 |
| 1 | 10163 | bot 1 | 150 | `Fall_Heavy` | `Lie_Loop` | A | 5 | 88 | 93 | yes | 14.7 hand | `GetUp` | 0.0 |
| 1 | 10693 | human | 124 | `Board_Fall_Front` | `Lie_Loop_Wall` | B | 39 | 65 | 104 | yes | **12.1** hand | `GetUp_Wall` | 0.0 |
| 7920 | 1183 | human | 61 | `Board_Fall_Front` | `Lie_Loop_Wall` | A | 0 | 4 | 4 | yes | 3.0 (too short) | `GetUp_Wall` | 0.0 |
| 7920 | 2317 | human | 150 | `Knockdown_Left_Heavy` | `Lie_Loop_Side_Left` | B | 39 | 84 | 15 | yes | 15.3 shin | `GetUp_Side_Left` | 0.0 |
| 7920 | 2504 | bot 2 | 103 | `Knockdown_Left` | `Lie_Loop_Side_Left` | A | 96 | 51 | 39 | yes | 18.4 forearm | `GetUp_Side_Left` | 0.0 |
| 7920 | 4836 | human | 87 | `Board_Fall_Right` | `Lie_Loop_Wall_Right` | B | 63 | 30 | 93 | yes | **13.1** hand | `GetUp_Wall_Right` | 0.0 |

- Start, L and cut are read from the loop's own position on every tick, not from the figure's print; they agree with the print in every row.
- Travel is the farthest any of head, hands, elbows, knees and skates gets from where it was on the loop's first frame, in the mesh's own space, from bone positions.
- **Cut clean: 13 of 13. End minus release 0.0: 13 of 13** (largest 0.000009 frame). **Travel 13.5 cm or more where the lying time is 25 or more: 9 of 11.**
- The two misses are both the wall loops' second move (the free glove lifts). The lab gives 13.5 to 15.3 cm for it; I measure the wrist bone. Whether the lab's point is the glove, I do not know.
- At seed 1, frames 10163 to 10225, two bots lie at once on different rules.
- Seed 7920's knockdown at frame 206 (98 frames) happened before the recorder was armed.

### Proof: test figures, ten lengths, both rules

One pair of figures knocked down together ten times, one on rule A and one forced to rule B; a second pair four times. Final Blueprints.

| Frames down | Fall | Loop | Get-up | Rule A: start, L, cut | travel cm | Rule B: start, L, cut | travel cm |
|---|---|---|---|---|---|---|---|
| 60 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | `GetUp_Snap_Side_Left` | 10, 29, 39 | 18.4 forearm | 64, 29, 93 | 16.9 shin |
| 70 | `Knockdown_Back_Light` | `Lie_Loop_Front` | `GetUp_Quick_Front` | 0, 39, 39 | 16.3 forearm | 54, 39, 93 | 15.5 skate |
| 80 | `Fall_Light` | `Lie_Loop` | `GetUp_Quick` | 98, 49, 39 | 14.5 hand | 44, 49, 93 | 14.0 shin |
| 90 | `Knockdown_Right` | `Lie_Loop_Side_Right` | `GetUp_Side_Right` | 1, 38, 39 | 16.8 hand | 55, 38, 93 | 16.9 shin |
| 100 | `Knockdown_Left_B` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 102, 45, 39 | 18.5 forearm | 48, 45, 93 | 16.9 shin |
| 110 | `Knockdown_Back_B` | `Lie_Loop_Front` | `GetUp_Front` | 96, 51, 39 | 16.6 forearm | 42, 51, 93 | 15.5 skate |
| 120 | `Fall_B` | `Lie_Loop` | `GetUp` | 93, 62, 47 | 14.7 hand | 39, 62, 101 | 14.0 shin |
| 130 | `Knockdown_Right_Heavy` | `Lie_Loop_Side_Right` | `GetUp_Side_Right` | 93, 68, 53 | 16.7 hand | 39, 68, 107 | 16.9 shin |
| 140 | `Knockdown_Left_Heavy` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 93, 78, 63 | 18.5 forearm | 39, 78, 9 | 16.9 shin |
| 150 | `Fall_Heavy` | `Lie_Loop` | `GetUp` | 5, 88, 93 | 14.7 hand | the same (L over 84) | 14.7 hand |
| 75 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | `GetUp_Quick_Side_Left` | 103, 44, 39 | 18.4 forearm | 49, 44, 93 | 16.9 shin |
| 85 | `Knockdown_Left_Light` | `Lie_Loop_Side_Left` | `GetUp_Quick_Side_Left` | 93, 54, 39 | 18.4 forearm | 39, 54, 93 | 16.9 shin |
| 100 | `Knockdown_Left` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 99, 48, 39 | 18.5 forearm | 45, 48, 93 | 16.9 shin |
| 124 | `Knockdown_Left` | `Lie_Loop_Side_Left` | `GetUp_Side_Left` | 93, 72, 57 | 18.5 forearm | 39, 72, 3 | 16.9 shin |

- **Cut clean: 28 of 28. Travel 13.5 cm or more: 28 of 28** (smallest 14.0). End minus release 0.0 in all 40 releases the session logged (largest 0.000009 frame).
- Rule A shows the first move, rule B the second, on the same loop: a forearm against a shin, a hand against a shin. While both lie the two figures differ by 15.9 to 18.6 cm at the forearm.
- The second pair stood 47 cm from the wall and was meant to fall into it. It did not take the wall path, so **the wall loops are proven by the four match knockdowns only**, and two of those are the misses above.
- Night figure: the loop code is the same builder's; not run.

## 2. The reactive whip

### What it is

- **Where:** the spring is computed in the figure Blueprint, only while a whip runs; the pose is changed in the animation Blueprint, after the montage slot, so it lies over whatever take plays. Four Transform (Modify) Bone nodes: `spine_02`, `spine_03`, `neck_01` and the stick arm's `upperarm_r`, each an added rotation in the mesh's space about the horizontal axis across the hit.
- **The spring:** 4 Hz, damping ratio 0.5, in closed form: e^(-12.57 t) sin(21.77 t), scaled so its first peak is 1. It peaks at 0.048 s (frame 3), swings back through zero to 16 % the other way at frame 12, and is faded to exactly zero between frames 12 and 19. On frame 20 the layer is switched off.
- **Size at Strength 1:** 15 degrees on each spine bone, 18 on the neck.
- **The stick arm lags:** its term is 5 degrees times (the spring 3 frames late minus the spring now), so the arm trails the trunk and then follows through.
- **Strength:** read from the bus again, for Strength only. A `check` event within 150 cm of the figure, or the figure's own `knockdown` event, sets it, whichever order the bus and the call arrive in within the frame. With no Strength (a plain collision, a board hit), 0.6 is assumed.
- **By kind:** a stumble takes the Strength whole; a knockdown half; a board hit half (toward the nearer wall); a plain collision (`PlayHitReaction`) half; the attacker 0.4 of his check's Strength, along his hit, on the bus event's frame.
- **No restart by a weaker hit:** while a whip runs, only a stronger one replaces it.
- Nothing runs when there is no contact: the figure's tick tests one flag.
- Test switch: `SetTestMode(16)` turns the whip off on one figure, `SetTestMode(8)` forces rule B.

### Proof: the head's peak displacement, cm (twin figures, one whipped, one not; the whip alone on the idle stance)

| Hit direction (figure faces +x) | Clay 0.3 | Clay 0.6 | Clay 1.0 | Night 0.3 | Night 0.6 | Night 1.0 |
|---|---|---|---|---|---|---|
| +x (from behind) | 4.24 | 8.45 | 13.92 | 4.49 | 8.94 | 14.73 |
| -x (from the front) | 4.22 | 8.39 | 13.75 | 4.47 | 8.88 | 14.55 |
| +y | 3.94 | 7.83 | 12.87 | 4.18 | 8.31 | 13.64 |
| -y | 3.94 | 7.83 | 12.87 | 4.18 | 8.31 | 13.64 |

- Monotonic in every direction. The peak is on the fourth recorded frame; the displacement is along the hit (10.9 to 13.7 of the 12.9 to 13.9 cm, the rest is the arc downward).
- **Frame 19: 0.010 cm or less. Frame 20: 0.000.** The asked line was under 0.5 cm by frame 20.
- The spring-back: 2.1 cm against the hit on frame 12 to 13 at Strength 1.0.
- **Root 0.0000 cm, pelvis 0.0000 cm** in all 24 cases.

### Proof: over a take (clay, Strength 1.0 sent as the bus would, the twin plays the same take with the whip off)

| Call | Take | Head peak cm | Frame 20 | Take position, whip against no whip | Root, pelvis |
|---|---|---|---|---|---|
| `PlayHitReaction` +x | `Check_Hit_Back` | 13.92 | 0.000 | 0.0000 s apart | 0, 0 |
| `PlayHitReaction` -x | `Check_Hit_Right` | 11.62 | 0.000 | 0.0000 | 0, 0 |
| `PlayHitReaction` +y | `Check_Hit_Left` | 13.02 | 0.000 | 0.0000 | 0, 0 |
| `PlayHitReaction` -y | `Check_Hit_Right` | 13.02 | 0.000 | 0.0000 | 0, 0 |
| `PlayStumble` +x, 16 frames | `Stumble_Back` | 13.77 | 0.000 | 0.0000 | 0, 0 |

The four `PlayHitReaction` rows were measured while a collision still took the Strength whole. On the final Blueprints, where it takes half, the same four measure 7.03, 5.93, 6.59 and 6.59 cm; the stumble is unchanged at 13.77. Both orders were exercised: the bus before the call (the hits) and the call before the bus (the stumble, the knockdowns).

### Ice and wall

Bone centres, not the skin. The whipped figure against its twin on the same take.

| Case (Strength 1.0 sent) | Head peak cm | Lowest bone over the whole take: whipped / twin | Farthest bone toward the wall plane |
|---|---|---|---|
| Stumble (`Stumble_Back`) | 13.77 | 11.2 / 13.9 (a hand, still 11 cm up) | n/a |
| Knockdown 110 (`Knockdown_Back`) | 6.94 | 12.83 / 13.96 | n/a |
| Knockdown 110 (`Knockdown_Left_B`) | 6.02 | 10.67 / 10.67 | n/a |
| Knockdown 80 (`Fall_Light`) | 7.04 | 12.97 / 12.97 | n/a |
| Board hit, 47 cm from the plane (`Board_Hit`) | 7.05 toward the wall | unchanged | 14.5 cm short of it, with and without the whip |

- **In a knockdown the whip takes the lowest bone at most 1.1 cm lower than the take's own lowest** (the line is 2 cm). That is with the knockdown at half; at 0.6 it was 1.7 cm.
- **At the wall nothing measured gets nearer the plane than the take already puts it.** At the nearest the pawn stands in play (40 cm, SK23's count) the farthest bone would be 7.5 cm short.
- Not measured: a board fall's whip at the wall, and the shoulder.

### A figure nobody touches

- A whip-enabled figure against a whip-off twin, both untouched, while six others beside them were hit and knocked down: **0 frames with any displacement in 3,063 frames; the largest difference of any of 11 bones is 0.00000 cm.**
- That is 51 s of game time, not the 100 s asked. The session ran at about 30 frames a second under my recorder, and I did not have the time to run it twice as long.
- In a match nobody is untouched: every one of the ten figures whipped (105 to 1,619 frames each in 210 s).

### The idle cost, as node counts

| Path | Nodes added | When idle |
|---|---|---|
| Figure Blueprint, tick | 2 (read the flag, branch) | both run, nothing else |
| Figure Blueprint, the contact calls | 1 call each into `WhipGo` | not on a per-frame path |
| Animation Blueprint, after the slot | 6: LocalToComponent, four Modify Bone, ComponentToLocal | the four Modify Bone nodes skip at alpha 0; **the two conversions run every frame** |

**So the idle cost is not flat.** The two conversions turn every bone's transform there and back once per figure per frame. I did not measure it in milliseconds: headless there is no frame to time. The fix is known (the first evolution).

### What the matches showed (first round, before the two late changes)

| | Seed 1, 210 s | Seed 7920, 210 s |
|---|---|---|
| Whips started | 365 | 136 |
| from plain collisions (`JAMVIS hit`) | 254 | 86 |
| from board hits | 83 | 31 |
| knockdowns, stumbles | 9, 6 | 5, 3 |
| bus `check` events (Strength 0.00 to 1.00) | 13 | 8 |
| Frames the human's figure spent in a whip | 1,619 of 12,615 | 581 of 12,585 |
| Blueprint errors | 0 | 0 |

At seed 1 the human's head was moving 13 % of the time, almost all of it from collisions at the assumed 0.6. That is a tic, which is why a collision now takes half and a weaker hit no longer restarts a running whip. On the final Blueprints (seed 1, 99 s): 192 whips, 171 of them at 0.3; the human spent 840 of 5,924 frames in one (14 %). **The count did not fall**, because equal collisions seldom overlap; the size did, from 8.4 to 4.2 cm.

## 3. The filming script

`status/SK25-film.md`: one Python block that spawns five test figures, then three strips (two lying players on different rules; a whip at 1.0 beside one at 0.3; the back get-up), each with the calls, the frames to capture and a camera position. It also lists seeds and sim frames where the same things happen in a real match.

## 4. Export30: the attacker's three sizes

- **Imported:** eight new takes per figure (`Check_Light`, `Check_Heavy`, `Check_Deliver_Right_Light`, `_Heavy`, `Check_Deliver_Left_Light`, `_Heavy`, `Lift_Light`, `Lift_Heavy`) and 18 changed ones (the stick out of the trunk).
- **Wired:** `PlayCheck` already looks for the nearest opponent to pick the left or right shoulder. It now also takes the closing speed on him: my speed along the line to him minus his along the same line, each figure's speed taken along its facing. Under 4 m/s plays the light take, 9 m/s or more the heavy, between them today's.
- **What can be known on the press:** only that. The figure has its own speed and facing from the motion calls, and can read the other figures' the same way. It does not know whether the check will land, on whom, or how hard.
- **What the bus says three frames later:** the Strength. In the final match (seed 1, 99 s) eight presses printed their closing speed:

| Closing speed m/s | Size played | Strength on the bus | Size the lab's Strength bands would pick |
|---|---|---|---|
| 0.67 | light | 0.61 | middle |
| -0.07 | light | 0.49 | middle |
| -0.10 | light | 0.19 | light |
| 0.52 | light | 0.55 or 0.66 | middle |
| 6.35 | middle | 0.81 | heavy |
| 2.65 | light | 0.67 | middle |
| 7.44 | middle | 0.37 | middle |
| 15.73 | heavy | 0.97 | heavy |

**3 of 8 agree.** The closing speed as the figure can compute it is a poor guide: four checks landed with almost none. The pairing of a press to its bus event is by order in the log; one pair is ambiguous.
- **Not wired:** `Lift_Light` and `Lift_Heavy` (the cross-check), and the bus's fallback start of the check take, which still plays the middle size.

## Not verified (everything here needs eyes)

- Whether the whip reads at the game camera, at any Strength, and whether it reads as a hit or as a nod.
- Whether the two loop moves read as different at game size. By bone positions the two rules differ by 15.9 to 18.6 cm at a forearm.
- The stick arm's lag. With the arm's term at 12 degrees the two hands' distance changed by up to 7.7 cm at Strength 1.0, so I cut it to 5: now at most 1.9 cm over the takes measured. Whether 5 degrees of lag is seen at all, I do not know.
- A stronger whip that replaces a running one starts from zero, so the head can step by the running one's displacement in one frame. Not measured, not seen.
- The whip during the hit-stop frames: the spring runs on game time and does not stop with the take.
- Skin against ice and wall. I measure bone centres, not the mesh. The shoulder was not among the bones measured at the wall.
- The idle cost in milliseconds.
- The night figure in a match. Its whip was measured on test figures only; its loops and check sizes ran in no match.
- The back get-ups of export29 and export30 with the stick: imported and played, never looked at.
- The three check sizes in motion, and whether a light take that ends after 13 frames leaves the attacker standing oddly.
- The cook.
- **Summoned test figures do not get their begin-play values** (`HoldAt`, `StumF`, `BrkMinF` read 0 on them while game figures read 9.0, 16, 6). I did not find why. The whip's constants are therefore in the graph, not in begin-play variables. Earlier lanes' test-figure results that leaned on begin-play values may be touched by this.

## For the other device

- **A plain collision carries no Strength.** `PlayHitReaction` arrives 254 times in 210 s with no bus event beside it. The figure assumes 0.6 and halves it. A Strength (the closing speed over some scale) on that call, or a bus event, would let the whip follow the hit.
- **A board hit carries none either** (83 in 210 s). Same request.
- **The check's size on the press.** The attacker's take has to start three frames before the Strength exists. Either `PlayCheck` gets one float (the sim's own estimate of the Strength it is about to resolve), or tell me the formula and I compute it from what the figure sees.
- **`check` names the attacker, not the victim.** The victim finds itself by being within 150 cm of `where`. A victim index in the payload would remove the guess.
- **Headless sessions need `Slate.bAllowThrottling 0`**, if device A runs any.

## Decisions for the operator

1. **How much whip on a plain collision.** Now 0.3 of full (about 4 cm at the head), 254 times in 210 s across ten figures. Zero is one constant away; so is full size.
2. **The knockdown's whip at half.** A knockdown at Strength 1.0 throws the head about 7 cm, a stumble at the same Strength 14. I halved it to keep bones off the ice in the first frames of a fall. If the film shows the knockdown's whip is lost in the fall anyway, it can go.
3. **The wall loops' second move** measures 12.1 and 13.1 cm at the wrist against the 13.5 line. Either the lab's point is the glove and this is fine, or the lab adds a centimetre and a half.
4. **The check's thresholds** (4 and 9 m/s) are mine. They need either device A's number on the press or a table of closing speed against Strength from longer matches.

## The next three evolutions

1. **The whip that costs nothing when idle.** Cache the slot's pose and blend by a flag, so the two conversions run only while a whip runs. **Proving number: nodes evaluated after the slot for an untouched figure: 2 (the cache and the switch), against 6 today; and the head's peaks unchanged to 0.1 cm.**
2. **The whip clamped by the room it has.** Read the head's and hands' height and the wall distance when the whip starts and scale it down so nothing can reach the ice or the plane; then the knockdown can take the Strength whole. **Proving number: lowest bone in a whipped knockdown no lower than the take's own lowest, at Strength 1.0 undivided, in 20 knockdowns.**
3. **The check's size from a number, not a guess.** With device A's estimate on the press, or thresholds fitted to a table. **Proving number: the size played matches the size the bus's Strength would have chosen (under 0.35, to 0.7, above) in 9 of 10 checks over two matches.**

## State left behind

- Blueprints saved: `BP_SkaterVisual_TB1.uasset` 7,264,552 bytes (was 6,150,353), `BP_SkaterVisual_TB1_P3.uasset` 7,474,183 bytes (was 6,359,856). `ABP_SkaterTB1.uasset` 741,085 bytes (was 648,729), `ABP_SkaterTB1_P3.uasset` 736,909 bytes (was 644,611). B40's `BrakeFx` and both `Activate` nodes and AU11's sound nodes are kept (counted before and after: the same).
- 90 take assets with keys replaced (45 per figure), 16 new (8 per figure). In git: 94 modified, 16 new assets, and the two files under `status/`.
- New in the figure Blueprints: events `WhipTick`, `WhipGoS`, `WhipGo`, `WhipStr`, `ChkPlay`; changed: `PlayHitReaction`, `PlayStumble`, `KnockDown`, `PlayBoardHit`, `OnBusEvent`, `PlayLie`, `ReceiveTick`, `PlayCheck`. New in the animation Blueprints: `WhipRot`, `WhipHead`, `WhipArm`, `WhipA` and six nodes after the slot.
- End check: 871 assets load, 16 Blueprints up to date, no dirty package, `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill` and `jam.hitstop` at their defaults, `Slate.bAllowThrottling` back to 1. Scratch cartridges deleted; they were never in the tree. `/Game/Maps/Rink` not saved.
- The editor log has 0 Blueprint runtime errors over the whole session. It has 20 `LogScript: Warning` lines, all from my own removal of five variables I had added and then replaced by constants.

## Ledger rows

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 6.93 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the pawn's PlayStumble covers every stumble (the check-bus read dropped), the stop driven by SetBraking's level and gated (7 m/s at the start, held 6 frames for a bot with 8 s between its stops, from the first frame for the human), lying loops from the frame the fall holds its end pose (lane SK24); the lying loops start on the frame the lab's table gives (rule A or B, alternating by the bus's knockdown count), a reactive whip on every contact (a damped spring 4 Hz, damping 0.5, pushed into ABP_SkaterTB1's four Modify Bone nodes; Strength from the bus), the attacker's check in three sizes by closing speed (lane SK25) (7264552 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 7.13 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the stumble by the pawn's call only, the gated level-driven stop and the lying loops as BP_SkaterVisual_TB1 (lane SK24); the start-frame rule for the lying loops, the reactive whip and the three check sizes as BP_SkaterVisual_TB1 (lane SK25) (7474183 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 0.71 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4); after the DefaultSlot: LocalToComponent, four Transform (Modify) Bone nodes (spine_02, spine_03, neck_01, upperarm_r; additive rotation in component space; WhipRot, WhipHead, WhipArm, alpha WhipA), ComponentToLocal (lane SK25) (741085 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.70 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3); the whip chain after the DefaultSlot as ABP_SkaterTB1 (lane SK25) (736909 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (165544 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (164930 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Late by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (138493 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_LateLong by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (157450 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_LateLong_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (164799 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (171669 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (169080 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (167021 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (166826 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (168472 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_LateLong_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_LateLong_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (167230 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Late_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (142867 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (146748 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (145384 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (146710 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (146559 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (146871 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Late_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (146692 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (140791 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Quick_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (141751 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Quick_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (145647 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Quick_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (144304 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (174102 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (171781 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Snap by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (140701 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Snap_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (143222 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Snap_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (145749 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Snap_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Snap_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (144277 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (169142 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (169656 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (169456 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Wall_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (169516 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Knockdown_Right_Heavy.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Knockdown_Right_Heavy by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (170749 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (245053 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (252065 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Side_Left.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (248472 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Side_Right.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (248440 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Wall by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (207181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Back.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Wall_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (207221 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Left.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Wall_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (207545 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Wall_Right.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Wall_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (207366 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Stumble_Back by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (157710 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Stumble_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (154572 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Stumble_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (167088 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Stumble_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25) (167016 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left_Heavy.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Left_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (152027 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Left_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Left_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (130683 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Right_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (154440 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Deliver_Right_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Deliver_Right_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (132040 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (154258 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Check_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Check_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (131858 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift_Heavy.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Lift_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (143898 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lift_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take Lift_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK25) (128314 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (175981 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (177645 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Late by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (140466 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_LateLong by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (161036 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_LateLong_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (168374 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (175252 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (172716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (170597 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (170567 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (173217 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_LateLong_Wall_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_LateLong_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (173165 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Late_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (144890 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (148822 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (147395 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (148658 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (148734 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (148919 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Late_Wall_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Late_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (148781 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Quick by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (142809 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Quick_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (143121 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Quick_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (147628 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Quick_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Quick_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (146305 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (186928 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (184601 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Snap by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (141219 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Snap_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (144992 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Side_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Snap_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (147467 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Snap_Side_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Snap_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (144682 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (172725 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (172915 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (173025 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Wall_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (173066 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right_Heavy.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Knockdown_Right_Heavy by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (176130 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (244735 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (251878 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Side_Left.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (248117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Side_Right.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (248077 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Wall by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (207041 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Back.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Wall_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (206961 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Left.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Wall_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (207329 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Wall_Right.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Wall_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (207224 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Back.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Stumble_Back by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (160608 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Front.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Stumble_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (157300 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Stumble_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (168890 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Stumble_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25) (168633 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Left_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (152853 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Left_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (130709 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Right_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (154466 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Deliver_Right_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (132962 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Heavy.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (154284 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Light.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Check_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (132780 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Heavy.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Lift_Heavy (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (143924 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Light.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take Lift_Light (the attacker's hit in three sizes), a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK25) (127892 bytes) |
```
