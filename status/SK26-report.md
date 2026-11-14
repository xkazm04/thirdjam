# Lane SK26 report: housekeeping, the check refitted, three new sets in the figure, a fault found in a take

Tree `C:\Users\kazda\jam\wt\b-fig` (`lane/b-fig`, on top of 3aae010e). Port 8004. One editor of mine (pid 39660, started 03:02:49 with `-nullrhi -unattended`; 0 `LogD3D12RHI` lines, line 918 loads `NullDrv`), left through `ExitEditor` and confirmed gone (log closed 04:11:53, pid gone at 04:11:57). The editors on 8002 and 8000 were not touched. Nothing committed, nothing pushed, no cook, no picture.

Everything asked during the lane is in: the three jobs, both additions (the dash set, the whip's two weights), the final dive and weapons exports re-imported, and the physics lane's fault. Numbers are in sim frames at 60 a second.

## Five things first

- **The left hand's flutter in every back fall is in the take, not in the engine.** `Lie_Loop_Front`'s first move, as exported, has the left arm on alternating keys: the hand jumps 2.6 to 3.0 cm (clay) and up to 6.0 cm (night) against its neighbours on 9 and 11 consecutive frames. The whip, the cross-fade and the rule are not involved; rule B only hides it because it never plays that move. I smoothed the arm's keys in the engine: 0 frames after, on both figures. **A re-import by name brings it back** until the lab fixes the take.
- **The begin-play mystery is solved, and it reaches past my own lane.** A summoned figure gets its begin-play values. What wipes them is the headless recipe's own line: a plain `set_editor_property` on the mesh component rebuilds the figure and resets **every** Blueprint variable to its class default. With `unreal.PropertyAccessChangeNotifyMode.NEVER` as third argument the pose ticks and nothing is wiped. Every headless bone measurement made with the plain line ran on a figure with its variables wiped; that includes all ten game figures in SK25's recorded matches. What that puts in doubt is in job 1.
- **Two of the three props did not import.** `Prop_GiantStick` and `Prop_Mallet` are empty files (4,076 bytes, no geometry) in the snapshot and in the final export. The cannon is real. The giant stick and the mallet are shown with the figure's own stick mesh as a stand-in, so every number about their far end is a stand-in's.
- **The lab's way of attaching a prop cannot work.** `stick_socket` turns against `hand_r` inside and between the weapon takes, so a socket on the hand at the stick's rest offset points the wrong way in every take. I attach the prop to `stick_socket` itself and hide the stick's mesh section instead of the bone.
- **The idle cost is not flat.** I could not build the switch: the tool offers no node to read a cached pose. Before and after are both 6 nodes after the slot. Details and the two ways out are in job 1.

## Verdict per job

1. **Housekeeping: three of four done.** The collision's whip is 0. The cross-check's sizes are wired. The begin-play cause is found. The idle cost is not made flat.
2. **The check refitted: done, on little data.** Two clean matches hold 21 landed checks, and only 8 of them had a press that reached the figure first. The old lines (4 and 9 m/s) get 3 of 8. The most the data allows is 6 of 8, and that needs a light line between -0.10 and -0.07 m/s, which is noise. Shipped: **0 and 11 m/s, 5 of 8.** The other 13 checks reach the figure only on the bus, at contact, and now take the Strength's own band.
3. **The first two sets: in, working, with stand-ins for two props.** All calls are declared by name on both figures and proven by logs and bone positions; a console trigger fires every move.
4. **The filming script: written**, `status/SK26-film.md`.
- **Addition A, the dash: done.** Ten takes, four calls, the hand-over to the stride without a pop (the skates move 1.8 cm at the join on clay, 3.2 on night). One name had to change: `SetBoost` is taken.
- **Addition B, the two weights: done.** `WhipHeadW` and `WhipArmW`, default 1.0; the whip measures as before in 11 of 12 cases.
- **The final dive and weapons exports: re-imported by name**, calls adjusted (`PlayDiveBlockFront`, the slide's side).
- **The physics lane's three points: answered** (the flutter found and removed; the stick past the boards explained, not changed; a whip-only test switch added).

## 1. Housekeeping from SK25

### The plain collision's whip

`PlayHitReaction` now starts its whip at 0.0 of the Strength, and a whip of zero size never starts. The constant stays in the graph (one number in `PlayHitReaction`), so the picture lane can film 0 against 0.3. In the two final matches the whips started fell from 365 and 136 to **98 and 42** in 210 s. The human's figure spent 693 of 12,441 frames in a whip (5.6 %, was 13 %) and 315 of 12,552 (2.5 %). What is left is mostly board hits: 83 and 31 in 210 s, each at 0.3 of full.

### The cross-check's sizes

`PlayLift` scans for the nearest opponent as `PlayCheck` does (one shared event, `ChkScan`) and plays `Lift_Light`, `Lift` or `Lift_Heavy` by the same two thresholds. It prints `SK26 lift close <m/s> size <0|1|2>`.

### The idle cost: not made flat

| Per frame, nobody touched, no weapon | Before (SK25) | After (SK26) |
|---|---|---|
| Figure Blueprint, tick | 2 nodes (read the whip flag, branch) | 4 (the same, and the same pair for the weapon level) |
| Animation Blueprint, after the slot | 6: two space conversions that run, four Modify Bone that skip | 6, unchanged |
| Animation Blueprint, on the aim layer's input | 0 | 4 switches that pass the aim pose through |

- **What I tried:** save the slot's pose once and switch between it and the whip chain. The save node can be made; the node that reads it back is not in the tool's palette for this graph (asked four ways, after a compile). I put the graph back as it was.
- **The other way:** move the stick's blade correction, which already pays for one pair of conversions, to after the slot and put the whip beside it. That removes the whip's conversions outright. It also changes what the stick does in the first frames of every take, and the correction is zero when nothing is rendered, so headless I cannot see what I would break. I did not do it blind.
- **What it needs:** two "use cached pose" nodes dropped by hand in an editor with a window (two minutes), then the wiring is mine. Or the second way, with the picture lane looking.

### Why test figures lost their begin-play values

| Step on a summoned figure | `StumF` | `RxGap` | `StopV` | a value set a moment before |
|---|---|---|---|---|
| after `summon` | 16 | 0.35 | 7.0 | 3.18 |
| after moving it, `SetTeam`, `SetNumber`, setting its tags | 16 | 0.35 | 7.0 | 3.18 |
| after `mesh.set_editor_property('visibility_based_anim_tick_option', ...)` | **0** | **0** | **0** | **0** |
| the same set with `PropertyAccessChangeNotifyMode.NEVER` | 16 | 0.35 | 7.0 | 3.18 |

Both figures' poses tick afterwards. The plain set tells the editor a property changed, and the editor rebuilds the Blueprint actor from its class.

**In doubt because of it:**
- **The ragdoll lane's bone measurements** made with the recipe in `RAG-report.md` (Evolution 0): its figures had every variable at the class default from that line on. Its physics numbers do not read those variables, but any reaction gap, stop bar or stumble length on those figures was not the game's.
- **SK25's two recorded matches and its last one:** my recorder set the option on all ten game figures a few seconds in. From there `RxGap` was 0 (no 0.35 s between reactions), `StopV` and the brake bars were 0 (a stop pose on every brake call) and the team numbers the check scan reads may have been 0. SK25's own proofs do not lean on those: the loop's start, cut, travel and release come from values set at each knockdown, and the whip is not gated by them. **The closing speeds in SK25's check table are the ones I would not trust**; job 2 here refits on clean matches.
- **SK25's test figures:** this is why the whip's gains read 0 there and I moved them into the graph. The whip and loop numbers were measured after that and stand.
- **Not in doubt:** every lane before SK25 ran with a window and never set that property.

Both my recorders now use the quiet set. The ragdoll lane and the picture lane should too (it is in the filming script).

### A test switch for the whip alone

`SetTestMode(32)` turns the whip off and leaves the lying rule. `8` forces rule B. `16` does both, as before.

## 2. The check's size, refitted

Seeds 1 and 7920, 210 s each, on the final Blueprints with the recorder's quiet set (the figures' variables intact). The same numbers came out of the two matches I ran before changing anything.

**Every landed check with a press before it** (the figure prints the closing speed on the press; the bus gives the Strength at contact):

| Seed | Contact frame | Press, frames before | Closing speed m/s | Strength | Band | Size at 4 and 9 | Size at 0 and 11 |
|---|---|---|---|---|---|---|---|
| 1 | 1238 | 3 | 0.67 | 0.610 | middle | light | **middle** |
| 1 | 1788 | 4 | -0.07 | 0.485 | middle | light | light |
| 1 | 1888 | 2 | -0.10 | 0.192 | light | **light** | **light** |
| 1 | 5046 | 2 | 6.35 | 0.807 | heavy | middle | middle |
| 1 | 5276 | 3 | 2.65 | 0.666 | middle | light | **middle** |
| 7920 | 1183 | 9 | 7.69 | 0.356 | middle | **middle** | **middle** |
| 7920 | 2504 | 1 | 14.70 | 0.773 | heavy | **heavy** | **heavy** |
| 7920 | 5118 | 11 | 7.03 | 0.306 | light | middle | middle |

**Confusion, rows the Strength's band, columns the size played:**

| | Old lines (4, 9): light | middle | heavy | New lines (0, 11): light | middle | heavy |
|---|---|---|---|---|---|---|
| Strength says light (2) | 1 | 1 | 0 | 1 | 1 | 0 |
| Strength says middle (4) | 3 | 1 | 0 | 1 | 3 | 0 |
| Strength says heavy (2) | 0 | 1 | 1 | 0 | 1 | 1 |
| Right | **3 of 8** | | | **5 of 8** | | |

- **The best any two lines can do on these eight is 6 of 8** (a light line at -0.09 m/s and a heavy line at 11.2). The sixth comes from splitting -0.10 from -0.07 m/s. I did not ship that.
- Closing speed and Strength correlate at 0.38 over the eight. A check at Strength 0.31 closed at 7.0 m/s and one at 0.81 at 6.4: no line separates them.
- **13 of the 21 landed checks had no press at the figure at all** (8 of 13 in seed 1, 5 of 8 in seed 7920). For them the take starts on the bus event, at contact, where the Strength is known: they now take its band directly. That part is right by construction.
- In these two matches the bus set the size by the band for all 21 (seed 1: 2 light, 7 middle, 4 heavy; seed 7920: 3, 3, 2), the eight pressed ones included: they were re-cut at contact. A late edit stops that: when a pressed check is already playing at contact, the bus keeps its size.
- The press comes 1 to 11 frames before the contact, not always 3.
- **With the other device's number on the press** (its own estimate of the Strength it is about to resolve): 8 of 8, if the estimate is the Strength. With the estimate off by one band at the edges, the misses would be the checks near 0.35 and 0.7: two of these eight sit within 0.01 and 0.05 of a line (0.356 and 0.306).
- The cross-check takes the same two lines. In the two matches `PlayLift` chose light 3, middle 17, heavy 4 times (24 presses); there is no Strength for it on the bus to check against.

## 3. The three sets in the figure

### Import by take name

| Set | Source | Takes per figure | New | Re-keyed by the final export | Bad |
|---|---|---|---|---|---|
| Weapons | `sets-lab\weapons\export` (first `snap1`) | 10 | 10 | `Giant_Poke` 17 to 18 keys, `Mallet_Swing` 33 to 37, `Mallet_Miss` 45 to 50, `Cannon_Fire` 21 to 25; the other six unchanged | 0 |
| Dive | `sets-lab\dive\export` (first `snap1`) | 8 | 8 (five from the snapshot, then `Dive_Block_Front`, `Slide_Tackle_Left`, `Slide_Tackle_Right`) | the five re-keyed in place | 0 |
| Dash | `sets-lab\dash\export` | 10 | 10 (`Dash_Loop` from its loopcut file) | - | 0 |

56 new take assets in all. Props: `SM_Prop_PuckCannon_TB1v2` and `_TB1_P3` under `Skaters/Props/` (764 vertices, four slots), and six flat-colour instances of the stick's material (`MI_Prop_ClayTimber`, `ClayTape`, `ClayFelt`, `ClayCream`, `ClayRed`, `ClayPuck`), bound by slot name. The colours are the report's sRGB values turned linear; the parent is the game's `M_Toon`, so they follow the clay or toon look as the stick does.

### The calls, as declared (exact types)

| Call | What the figure does |
|---|---|
| `PlayDiveBlock(float DirX, float DirZ, int32 Frames)` | Direction to the right of the facing: `Dive_Block_Left`; to the left: `Dive_Block_Right`. The take to frame 35, the side lying loop, the side get-up. |
| `PlayDiveBlockFront(float DirX, float DirZ, int32 Frames)` | `Dive_Block_Front` to frame 35, `Lie_Loop_Front`, the front get-up. |
| `PlayDiveBlockHit()` | Only while lying after a side dive: the 12-frame jolt, then the loop again from the jolt's frame 10. |
| `PlaySlideTackle(int32 Frames, int32 Side)` | Side below 0: `Slide_Tackle_Right`, else `_Left`. To frame 19, `Lie_Loop`, the back get-up. |
| `SetWeapon(int32 Kind)` | 0 to 3. Shows the prop, hides the stick, switches the carry layer on. |
| `PlayWeapon(int32 Kind, float DirX, float DirZ, int32 Frames, bool bHit)` | Giant stick: `Giant_Poke` within 20 degrees of ahead, else `Giant_Sweep`. Mallet: `Mallet_Swing` on a hit, `Mallet_Miss`. Cannon: `Cannon_Fire`, or on a hit `Cannon_Kick` with its lying and get-up. |
| `PlayWeaponHit(int32 Kind, float DirX, float DirZ)` | The victim. Kind 2 (mallet): the knockdown that arrives in the same frame, before or after, plays `Fall_Bonk`, then `Lie_Loop` and the back get-up. Other kinds: nothing special. **The other device has to send it.** |
| `PlayDash(int32 Frames)` | `Dash_Start`, N cycles of `Dash_Loop` at the report's rate (kept between 0.7 and 1.4), `Dash_End`, then `Stride` from its frame 34. |
| `SetBoostLevel(bool bOn, float SpeedScale)` | The loop as a level, rate 0.9 to 1.3; off plays `Dash_End`. **Named `SetBoostLevel` because `SetBoost(FName Kind)` already exists in the contract.** |
| `EndDash(uint8 Reason, float DirX, float DirZ)` | 0: `Dash_End`. 1: `Dash_Skid_Left` for DirZ at or over 0, else `Dash_Skid`. 2: `Dash_Bump_Left` or `_Right` the same way, then the loop goes on for the time left. 3: `Dash_Wipeout`, `Lie_Loop_Front`, the front get-up. |
| `PlayBoostPickup()` | `Boost_Take` from `spine_01` up for 12 frames. |

**How a move with lying in it works.** The dive, the slide, the cannon's kick and the wipeout run on the knockdown's own machinery, with the move's take in the fall's place. So the lying loop's start-frame rule, the cross-fade, the get-up by length and the release are the ones already proven.
- If the game sends `SetKnockedDown(true, ..., N)` while the move runs, the figure does not fall again: it takes N as the frames left from that call and re-times the get-up. The game's release then ends it.
- **If the game sends nothing more, the figure ends the move itself.** `Frames` of 64 or more is taken as the whole move. Under that, my default: 30 lying frames and the 36-frame get-up after the take, so 101 frames for a dive, 85 for a slide, 89 for the kick, 86 for the wipeout.
- A lock shorter than the take and get-up together scales the play rate, as a short knockdown does; nothing is cut.

**The weapon level.** While a weapon is held the prop is shown only in the carry pose and in the weapon's own takes. Any other take (a hit, a stumble, a knockdown, a shot, a check) puts the stick back for its length; the kick keeps the cannon until the body lies. One figure-tick test does this, and only while a weapon is held.

**The carry layer** is on the aim layer's input, so it is blended exactly as the aim holds are: mesh space, from `spine_01` up.

### Proof: the dive and the slide (final takes, test figures, both figures)

| Move | Take played from the call | Key frame, from bone positions (clay / night) | Then | End minus release |
|---|---|---|---|---|
| `PlayDiveBlock`, right | `Dive_Block_Left`, frames 0 to 34 | full cover at 10: highest bone 47.2 / 45.2 cm (an upper arm), hips 28.4 / 27.5 | `Lie_Loop_Side_Left` from take frame 35 (start 9, 30 frames, cut 39), `GetUp_Side_Left` | 0.0 (own end at 101) |
| `PlayDiveBlock`, left, 120 frames | `Dive_Block_Right` | the same, mirrored | loop, `PlayDiveBlockHit` at lying frame 15: jolt frames 0 to 9, loop again (start 15, cut 39), `GetUp_Side_Right` | 0.0 (own end at 120) |
| `PlayDiveBlock` then `SetKnockedDown(true, 80)` at frame 35 | `Dive_Block_Left` | - | re-timed to 115 frames, no second fall | 0.0 (the game's release) |
| `PlayDiveBlockFront` | `Dive_Block_Front`, 0 to 34 | cover at 10: highest bone 26.6 / 26.8 cm, hips 19.0 / 17.9 | `Lie_Loop_Front`, `GetUp_Front` | 0.0 |
| `PlaySlideTackle`, +1 | `Slide_Tackle_Left`, 0 to 18 | contact at 10: leading skate 52 / 61 cm ahead of the hips, hips 36.8 / 30.0 cm high | `Lie_Loop` (start 101, 46 frames, cut 39), `GetUp_Quick` | 0.0 (own end at 85) |
| `PlaySlideTackle`, -1 | `Slide_Tackle_Right` | leading skate 57 / 65 cm ahead | the same | 0.0 |

- The lab gives the dive's highest point as 59.6 to 61.4 cm on the skin. I measure bone centres: 47.2 and 45.2 cm at the upper arm, which is that skin less the arm's thickness. I cannot measure the skin.
- **The slide's victim:** nothing new plays. The other device sends what it sends today, which plays `Trip`; by the lab's note the tripped body lands on the tackler from frame 17. The figure does nothing about it.

### Proof: the weapons (final takes, test figures)

| Move | Take from the call | Key frame (clay / night) | Prop shown, stick shown | Prop's lowest point over the ice |
|---|---|---|---|---|
| `SetWeapon(1)`, standing | carry layer | stand-in's far end 229 / 221 cm ahead | prop on, stick off | -1.6 / 6.9 cm |
| `SetWeapon(1)`, skating at 8 m/s, 199 frames | carry layer over `Stride` | far end 230 / 223 cm ahead | prop on, stick off | **-0.6 / 6.9 cm** |
| `PlayWeapon(1)`, ahead | `Giant_Poke` | contact at 9: far end 291 / 284 cm from the hips | on, off | 1.2 / 8.7 |
| `PlayWeapon(1)`, to the side | `Giant_Sweep` | frame 12: 235 / 228 cm | on, off | -2.4 / 4.6 |
| `PlayWeapon(2, hit)` | `Mallet_Swing` | wind-up top at 10: 238 / 244 cm high. Contact at 17: the stand-in's end 122 / 124 cm ahead, 60 to the left, **72 / 75 cm high** | on, off | 57.7 / 60.3 |
| `PlayWeapon(2, miss)` | `Mallet_Miss` | passes the same point at 17 | on, off | 40.0 / - |
| `PlayWeapon(3, miss)` | `Cannon_Fire` | firing at 8: the muzzle at (82.3 ahead, 35.4 right, 85.5 up) / (83.6, 35.9, 88.6) cm | on, off | 65.2 / 68.3 |
| `PlayWeapon(3, hit)` | `Cannon_Kick` | firing at 8: the same point | cannon until the body lies (take frame 23), then the stick | 38.6 / - |
| a hit reaction or a stumble with a weapon held | `Check_Hit_Back`, `Stumble_Back` | - | the stick for the take (the prop shows on its first frame only), the prop back after | - |
| `SetWeapon(0)` | - | - | prop off, stick on | - |

- **The carry is in mesh space.** With the giant stand-in in the hands over 199 frames of stride, its lowest point is 0.6 cm under the ice on clay and 6.9 cm over it on night. The lab's warning for local space is 29 to 91 cm under. The real giant stick is 5 to 15 cm longer than my stand-in and has its own blade; with it the number will move by centimetres, not by the tens.
- **The cannon's muzzle** reads 3.8 cm further ahead and 3.5 cm lower than the lab's first-loop point, sideways exact, on both figures. My point is the middle of the mouth's vertices and the keg has since moved 4 cm out; I do not have the lab's point for the final keg.
- **The mallet's victim at the lab's spacing** (1.08 m ahead, 0.72 m to the attacker's left, facing him; `PlayWeaponHit` and the knockdown on the attacker's frame 16): at the attacker's frame 17 the victim's hips are 19.1 / 20.4 cm from the stand-in handle's end, at the same height (73.2 / 77.3 against 72.1 / 74.7 cm), and the victim is on `Fall_Bonk` frame 1. A drum 36 cm across on that end covers the hip. `Fall_Bonk` plays in both orders of the two calls; `Lie_Loop` and `GetUp` follow; end minus release 0.0.
- "Stick shown" is the engine's own answer for the stick's mesh section. Nothing was drawn.

### Proof: the dash (test figures skating at 8 m/s)

| Move | Takes, in order (clay) | The skates' largest one-frame move at each join, cm (clay / night) |
|---|---|---|
| `PlayDash(96)` | `Dash_Start` 0 to 8.8, `Dash_Loop` four cycles at rate 1.10, `Dash_End` 0 to 10, `Stride` 34 to 46, then the game's stride | into the loop 0.9 / 1.4; into `Dash_End` 13.1 / 14.2; **into `Stride` 34: 1.8 / 3.2**; into the game's stride 2.2 |
| the same, `EndDash(1, +)` after 40 frames | ..., `Dash_Skid_Left` | 9.4 |
| the same, `EndDash(2, +)` after 30 | ..., `Dash_Bump_Left` 0 to 7, the loop again for the time left, `Dash_End`, `Stride` | into the bump 13.9; back into the loop 18.2 |
| the same, `EndDash(3)` after 40 | ..., `Dash_Wipeout` 0 to 19, `Lie_Loop_Front` (start 101, 46 frames, cut 39), `GetUp_Quick_Front` | 9.4; end minus release 0.0 |
| `SetBoostLevel(true, 1.0)`, off after 80 | `Dash_Start`, the loop at rate 1.0, `Dash_End`, `Stride` | into `Stride` 34: 1.8 |

- **The feet do not pop at the hand-over.** For scale: an untouched figure's stride at the same speed moves a skate 6.7 / 7.4 cm a frame at most, the dash loop 16.3 / 17.6. From three frames before the hand-over to 16 after the stride take ends, the largest one-frame move is 10.8 / 10.6 cm; on clay that is in `Dash_End`'s own last frames, and from the join on it is 8.2 cm, inside the `Stride` take.
- **How the loop replaces the stride:** as a looping take over the locomotion, not as a state in the animation Blueprint. One timer event steps the chain (start, loop, end, hand-over), so nothing is added to the tick. The stride comes back by playing `Stride` from frame 34 as a take and fading it into the game's stride over 12 frames; the game's stride is at whatever phase it is at, and the fade hides that (the 2.2 cm above).
- The first version had a pop: each next take started one frame late and the game's stride showed through for a frame (40 cm at the skates). Each timer now fires a frame early.
- On the assets, key against key: `Dash_Start`'s last equals `Dash_Loop` 0, `Dash_Loop` 0 equals `Dash_End` 0, `Dash_End`'s last equals `Stride` 34, `Dash_Wipeout`'s last equals `Lie_Loop_Front` 0: 0.0 cm and 0.0 degrees on both figures.
- **Not built:** the skid's hold for as long as the brake lasts (the skid plays its 20 frames); the dash loop under a weapon's carry.

### The test trigger

`ke * SK26Test <move>` on the console. It fires on the human's figure and on every figure without a pawn; bots ignore it; nothing in the game calls it. Moves 1 to 22 are listed in `status/SK26-film.md`. Proven in a play session: move 4 fired on the six test figures and on the human's figure (`C_0`), on no bot.

### A normal match is unchanged

Seeds 1 and 7920, 210 s each, run on the adopted SK25 Blueprints before any change of this lane, and again on the final ones:

| | Seed 1 | Seed 7920 |
|---|---|---|
| `JAMVIS knockdown` and `JAMVIS stumble` lines, before | 15 | 8 |
| the same lines after, compared as text | **identical** | **identical** |
| Blueprint errors | 0 | 0 |
| Lines from the new calls (dive, slide, weapon, dash) | 0 | 0 |
| Knockdowns recorded, cut clean, end minus release 0.0 | 9, 9, 9 | 4, 4, 4 |

After those two matches I made two small edits: the bus keeps a pressed check's size, and the two light check takes fade out over 6 frames instead of 12 (a 13-frame take with a 12-frame fade was fading from its second frame). The first ran in a 39 s match of seed 1: its seven knockdown and stumble lines identical to the baseline's first seven, no error; but in it one pressed light check had already faded out by the contact and was re-cut to the middle size, which is what the second edit is for. **The second was compiled and saved, not run.**

## The fault in `Lie_Loop_Front`

Frames in which a hand moves more than 1 cm against its neighbours (it lies off the line between the frames before and after, and the two steps point opposite ways), while lying:

| Fall (test figures, hit from behind) | Lying loop | Before: left / right hand | After |
|---|---|---|---|
| `Knockdown_Back`, 110 frames, rule A | `Lie_Loop_Front` from 93 | 9 / 0 | 0 / 0 |
| `Knockdown_Back_Heavy`, 150 frames (both moves, any rule) | from 5 | 9 / 0 | 0 / 0 |
| `Knockdown_Back_Light`, 80 frames, rule A | from 98 | 9 / 0 | 0 / 0 |
| the same three under rule B | from 39 or 44 | 0 / 0 (110, 80); 9 / 0 (150) | 0 / 0 |
| `Fall`, 110 frames (on the back) | `Lie_Loop` | 0 / 0 | 0 / 0 |

- The whip on or off changes none of these numbers.
- On the asset itself, by forward kinematics: clay frames 23 to 31, 2.6 to 3.0 cm; night frames 21 to 33, up to 6.0 cm. The other seven loops: 0.
- **The fix:** the four left-arm bones' rotation keys passed twice through a 1-2-1 filter over frames 20 to 34 (clay) and 18 to 36 (night). The hand still travels 9.4 / 8.6 cm in the move, its largest one-frame move is 1.7 cm, and frames 15 and 39 are 0.13 cm from rest, so the clean windows hold.
- The falls themselves have one to four frames each that trip the same test (a hand reversing 10 to 24 cm at the impact). They are the takes' own, the same with the whip on and off, and I left them.

## The stick past the boards on `Board_Fall_Back`'s frame 1

I did not measure it; this is from how the fall starts. A board fall starts on the take's frame 2, from whatever the figure was doing, and the mesh turns to the take's facing over the next frames. On frame 1 the figure is still close to its ready stance, whose stick reaches about 1.5 m ahead, and the pawn stands 40 to 59 cm from the boards. If the figure faced the boards when it was hit, the stick was already through them before the fall began. **So it is neither the take nor the mesh shift: it is the standing figure's stick at the boards**, and it is there in ordinary play too. What I would do is the second evolution below.

## Sounds: where each would go

Not wired (the MetaSounds are not in this tree). One `PlaySound2D` each, at these places in both figure Blueprints:

| Sound | Event and place |
|---|---|
| the dive | `DiveGo`, kinds 1 and 4 with the front dive, on the call; the landing wants a 10-frame timer |
| the dive's jolt | `PlayDiveBlockHit`, where it prints `SK26 dive hit` |
| the slide | `DiveGo`, kind 2, on the call; the contact wants a 10-frame timer |
| the mallet's bonk | `PlayWeaponHit`, kind 2 (it runs once per bonk in either order) |
| the mallet's swing | `WeaponCut`, the `Mallet_Swing` / `Mallet_Miss` branch |
| the giant stick | `WeaponCut`, the `Giant_Poke` / `Giant_Sweep` branch |
| the cannon's shot | `WeaponCut` for `Cannon_Fire` and `PlayWeapon`'s kick branch; the shot is on frame 8, so an 8-frame timer |
| picking a weapon up, putting it away | `WeaponShow`, the two branches |
| the dash's launch | `DashBegin` |
| the dash loop | `DashNext`, where the loop starts (a looping sound needs a stop in `EndDash` and at the knockdown) |
| the dash's end, skid, bump, wipeout | `EndDash`, its four branches |
| the boost pickup | `PlayBoostPickup` |

## Not verified

- **Anything as a picture.** In particular: the stick really hidden under a prop; the prop's seat in the hands; the carry poses; the first frame after `SetWeapon` (pose and prop appear together, with no blend); the one frame of prop at the start of a reaction.
- **The giant stick and the mallet as themselves.** Every number for them is the stand-in's.
- **The idle cost in milliseconds**, and it is not flat.
- **`Boost_Take` a second time.** The player that shows it is not reset when it is switched on again (the property is closed to the tool), so a second pickup may show only its last frame. Measured: no; it needs eyes or a longer test.
- **The whip's twelfth case.** With the weights at 1.0, eleven of twelve head peaks equal SK25's to 0.01 cm (4.24, 8.45, 13.92; 4.22, 8.39, 13.75; 3.94, 7.83, 12.87; 3.94, 7.83). The last (Strength 1.0 from the right) reads 11.92 cm on frame 3 against 12.87 on frame 4. I take it for the recorder catching the peak a tick apart; I did not run it again.
- **The weights at 0.** The physics lane's child sets them; I measured the default only.
- **The night figure in a match**, as in SK25. Its sets, dash and back falls were measured on test figures.
- **A dive, a slide or a weapon in a real match.** The other device does not call them yet; the trigger was fired in a match once (move 4).
- **A wall near a dive or a slide.** The move turns the board-fall rule off; a dive into the boards plays as on open ice.
- **The lying rule after a re-time.** When the game adopts a dive late, the loop's start frame was chosen for the old length; the cut may then miss its clean window.
- **The skin** against ice and stick, as before: bone centres and prop vertices only.
- The cook.

## For the Blender set lanes

- **Weapons:** `Prop_GiantStick_<style>.fbx` and `Prop_Mallet_<style>.fbx` are 4,076 bytes with no geometry, in `snap1` and in the final `export\props`. The importer says "nothing to import". The cannon (23,932 bytes) is fine.
- **Weapons:** the attach recipe in section 4 does not hold. In your own takes `stick_socket`'s rotation against `hand_r` is (x, y, z, w) 0.698, 0.205, 0.599, 0.334 in `Giant_Carry`; -0.849, -0.522, 0.006, 0.085 on `Mallet_Swing` frame 0 and 0.308, 0.428, -0.725, 0.443 on its frame 8; 0.527, 0.620, 0.241, 0.529 in `Cannon_Fire`. The prop has to ride `stick_socket`. I use your prop frame (euler 90, -62, 0, which in the engine is roll 90, pitch 62) on that bone, and the cannon's mouth lands within 4 cm of your point.
- **Weapons:** the cannon's muzzle point for the final keg, in figure space at `Cannon_Fire` frame 8, so I can check against it.
- **Figure lab (export29):** `Lie_Loop_Front`'s left arm alternates frame to frame through the first move (clay frames 23 to 31, night 21 to 33; 3 to 6 cm at the hand). I smoothed it in the engine; the source needs the fix or the next re-import undoes mine.
- **Dive:** `snap1\dive\p2_cel` held three files the READY did not list. The final export is consistent.
- **Dash:** the folder holds `Dash_Carry` and `Dash_Wipeout_Light`, not in the READY; not imported.
- **All sets:** a loopcut file of 24 keys plays as a 23-frame cycle in the engine (the last key and the first share an instant). That is how every loop here has always come in; say if `Dash_Loop`'s push timing depends on the 24th.

## For the other device

- **`SetBoost` is taken** (`SetBoost(FName Kind)`, the pickup's call). The dash level is `SetBoostLevel(bool bOn, float SpeedScale)` until one of the two is renamed.
- **The exact signatures** are in job 3's table. `EndDash`'s first parameter is a byte; everything called "int" is `int32`.
- **`PlayWeaponHit(int32 Kind, float DirX, float DirZ)` is yours to send** to the mallet's victim, in the same frame as its `SetKnockedDown(true)`, either order.
- **After a move with lying in it** (dive, slide, cannon kick, wipeout) you may send `SetKnockedDown(true, ..., N)` at any frame of it: the figure takes N as the frames left from that call. If you send nothing it gets up by itself (101, 85, 89, 86 frames). Tell me which you will do, so the default can go.
- **The skid's length:** `EndDash(1)` plays 20 frames. A hold for the brake's length needs the length in the call, or `SetBraking` read during the skid.
- **13 of 21 landed checks reach the figure with no `PlayCheck` before the bus event.** For those the take starts at contact, and it now takes the Strength's own band. For the other 8 the press comes 1 to 11 frames before the contact, not always 3.
- **The slide's victim** plays what you send today (`Trip`); a clear-falling take is promised by the set lane.
- Still open from SK25: a Strength on `PlayHitReaction` and on the board hit; the victim's index on `check`.

## Decisions for the operator

1. **The idle cost.** Leave it (two conversions per figure per frame, unmeasured), or give the two-minute hand edit in an editor with a window, or let me move the stick correction with the picture lane watching.
2. **Film the giant stick and the mallet now, with stand-ins, or wait** for their meshes. The takes, the calls and the carry are the real ones; only the object in the hands is not.
3. **The default lying time** when the game sends nothing after a dive, slide, kick or wipeout: 30 frames, mine.
4. **The smoothed `Lie_Loop_Front`** ships as it is, or waits for the lab's own fix.
5. **The check's thresholds** are now 0 and 11 m/s, fitted on 8 presses. With so few I would not defend them; the number on the press from the other device replaces them.

## The next three evolutions

1. **The whip that costs nothing when idle** (carried over). **Proving number: nodes run after the slot for an untouched figure: 3 (save, read, switch), against 6; the head's peaks unchanged to 0.1 cm.** It needs the two nodes by hand.
2. **The stick tucked at the boards.** Shorten the aim's reach when the boards are nearer than the blade, standing and through a board fall's first frames. **Proving number: the stick's farthest point past the boards plane on any frame: 2 cm or less (the physics lane measures 91 and 105 cm on `Board_Fall_Back` frame 1).**
3. **The sets with their real props and their sounds.** Re-import the giant stick and the mallet when they have geometry, wire the fourteen sounds from the table above. **Proving numbers: each prop's lowest point over the ice 0 cm or more through its own takes and the carry over the stride; the mallet drum's end face within 3 cm of the victim's hip on the attacker's frame 17; one sound line in the log per move, 14 of 14.**

## State left behind

- Blueprints saved: `BP_SkaterVisual_TB1.uasset` 9,476,476 bytes (was 7,264,552), `BP_SkaterVisual_TB1_P3.uasset` 9,685,732 (was 7,474,183), `ABP_SkaterTB1.uasset` 845,681 (was 741,085), `ABP_SkaterTB1_P3.uasset` 841,582 (was 736,909). B40's `BrakeFx` and both `Activate` nodes and AU11's sound nodes are kept (counted before and after every write: the same). The figure Blueprints grow by about 2 MB a lane from these writes; I have not looked into why.
- New: 56 take assets (28 per figure), `Skaters/Props/` with 2 meshes and 6 material instances. Changed besides the four Blueprints: `SM_SkaterTB1_P3_Stick` (its two slots bound, as the stand-in) and both figures' `Lie_Loop_Front` (the smoothing). In git: 7 modified, 64 new assets, two files under `status/`.
- New in the figure Blueprints: a `WeaponMesh` component on the figure mesh; 28 new events (`ChkScan`, `ChkCut`, `LiftPlay`, `DiveGo`, `DiveAdopt`, `PlayDiveBlock`, `PlayDiveBlockFront`, `PlayDiveBlockHit`, `DiveHitPlay`, `PlaySlideTackle`, `SlidePick`, `WipePick`, `WeaponTick`, `WeaponShow`, `WeaponPick`, `SetWeapon`, `WeaponCut`, `PlayWeapon`, `PlayWeaponHit`, `DashBegin`, `PlayDash`, `SetBoostLevel`, `DashNext`, `EndDash`, `DashCut`, `PlayBoostPickup`, `BoostBeatEnd`, `SK26Test`); `WhipHeadW` and `WhipArmW` with class default 1.0.
- New in the animation Blueprints: `bCarry1` to `bCarry4`, four switches and four players on the aim layer's input.
- End check: 935 assets load (871 before), 16 Blueprints up to date, no dirty package, `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.hitstop` at their defaults, `Slate.bAllowThrottling` back to 1. Scratch cartridges deleted; they were never in the tree. `/Game/Maps/Rink` not saved.
- The editor log: 0 Blueprint runtime errors over the whole session; 4 `LogScript: Warning` lines, from my two refused tool calls (adding a pin to a blend node, one attach call with too few arguments).
- One more headless trap, for the other lanes: **a play session started with an unsaved, just-compiled Blueprint did not tick** (world time 0.0 again, with throttling already off). Saving before the start cured it every time.

## Ledger rows

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 9.04 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the pawn's PlayStumble covers every stumble (the check-bus read dropped), the stop driven by SetBraking's level and gated (7 m/s at the start, held 6 frames for a bot with 8 s between its stops, from the first frame for the human), lying loops from the frame the fall holds its end pose (lane SK24); +a plain collision gets no whip; the whip's two weights (WhipHeadW, WhipArmW); the check's and the cross-check's sizes (closing speed on the press, the Strength's band when the bus starts the take); the calls PlayDiveBlock, PlayDiveBlockFront, PlayDiveBlockHit, PlaySlideTackle, SetWeapon, PlayWeapon, PlayWeaponHit, PlayDash, SetBoostLevel, EndDash, PlayBoostPickup by name, riding the knockdown's machinery; the WeaponMesh component on stick_socket; the test trigger SK26Test (lane SK26) (9476476 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 9.24 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the stumble by the pawn's call only, the gated level-driven stop and the lying loops as BP_SkaterVisual_TB1 (lane SK24); +the whip's constant and weights, the check sizes, the eleven set calls, the WeaponMesh component and the test trigger as BP_SkaterVisual_TB1 (lane SK26) (9685732 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 0.81 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4); +the carry layer on the aim layer's input: four bool switches (Giant_Carry, Mallet_Carry, Cannon_Carry, Boost_Take), mesh space from spine_01 (lane SK26) (845681 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.80 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3); +the carry layer's four switches as ABP_SkaterTB1 (lane SK26) (841582 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (121309 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Poke.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Poke by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (135315 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Sweep.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (139589 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (123319 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Swing.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (161527 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Miss.uasset | anim | 0.18 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (187813 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Bonk.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Fall_Bonk by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (141601 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (121335 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Fire.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Fire by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (138215 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Kick.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Kick by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (160007 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (170622 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (170640 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Hit_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Hit_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (123694 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Hit_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Hit_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (124544 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Front by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (163162 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (148716 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (147934 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (148624 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Start.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Start by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (125582 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Loop.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Loop (the loopcut file) by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (138572 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_End.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_End by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (126826 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Bump.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Bump by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (124091 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Bump_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Bump_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (124181 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Bump_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Bump_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (123559 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Skid.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Skid by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (142972 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Skid_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Skid_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (144406 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Wipeout.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Dash_Wipeout by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (150226 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Boost_Take.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p2_cel, take Boost_Take by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26) (127054 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Front by name on SKEL_SkaterTB1v2; the left arm's rotation keys smoothed in the engine over the first move (they alternated 3 to 6 cm at the hand frame to frame); a re-import brings that back until the lab fixes it (lane SK26) (252065 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (121354 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Poke.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Poke by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (138808 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Sweep.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (140362 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (124348 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Swing.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (161564 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Miss.uasset | anim | 0.18 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (192650 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Bonk.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Fall_Bonk by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (143462 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Carry.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (121372 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Fire.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Fire by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (138250 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Kick.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Kick by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (157066 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (174400 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (174418 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Hit_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Hit_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (123720 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Hit_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Hit_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (124570 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Front by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (166932 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (150342 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (150360 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (150250 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Start.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Start by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (127016 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Loop.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Loop (the loopcut file) by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (138598 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_End.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_End by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (128100 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Bump.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Bump by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (123157 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Bump_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Bump_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (123247 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Bump_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Bump_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (123265 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Skid.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Skid by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (145014 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Skid_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Skid_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (146448 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Wipeout.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Dash_Wipeout by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (150252 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Boost_Take.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dash/export/p3_clay, take Boost_Take by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26) (128232 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Front by name on SKEL_SkaterTB1_P3; the left arm's rotation keys smoothed in the engine over the first move (they alternated 3 to 6 cm at the hand frame to frame); a re-import brings that back until the lab fixes it (lane SK26) (251878 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_TB1v2.uasset | mesh | 0.08 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_p2_cel.fbx, the puck cannon, 4 material slots bound by name (lane SK26) (83768 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_TB1_P3.uasset | mesh | 0.08 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_p3_clay.fbx, the puck cannon, 4 material slots bound by name (lane SK26) (83790 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayTimber.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.62, 0.42, 0.22) for the props' slot ClayTimber (lane SK26) (4861 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayTape.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.90, 0.86, 0.74) for the props' slot ClayTape (lane SK26) (4849 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayFelt.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.78, 0.20, 0.14) for the props' slot ClayFelt (lane SK26) (4849 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayCream.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.92, 0.86, 0.72) for the props' slot ClayCream (lane SK26) (4855 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayRed.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.78, 0.20, 0.14) for the props' slot ClayRed (lane SK26) (4843 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayPuck.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.16, 0.14, 0.13) for the props' slot ClayPuck (lane SK26) (4849 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/SM_SkaterTB1_P3_Stick.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export5/p3_clay SM_Stick_p3_clay.fbx (cm); imported, not attached (the stick is skinned into SK_SkaterTB1_P3) (lane SKI3); +slots Stick and Trim bound to MI_Prop_ClayTimber and MI_Prop_ClayTape: it stands in for the giant stick and the mallet until their own meshes export (lane SK26) (84274 bytes) |
```
