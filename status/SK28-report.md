# Lane SK28 report: the sets' sounds, the real props, export33's five takes, the dash by numbers, the first cuts

Device B, tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, on dev-b dfabb6be), one editor without a GPU (`-nullrhi -unattended`, port 8004, pid 26428, 0 `LogD3D12RHI` lines). No commit, no push. 76 files changed or new, all under `Game/Plugins/JamAssets/Content/Skaters/`. The editor left through `ExitEditor` at 05:38 and its process is gone.

**All six jobs are done and pass by their numbers. Nothing was seen or heard: every result is a bone position, a prop vertex or a log line.** Three things in it need a decision or a correction elsewhere:

1. **The pin's victim must be called on the attacker's frame 16, not 14.** Called on 14, the victim's legs are 24 to 39 cm away from the roller when it arrives.
2. **The figure Blueprints grew, they did not shrink.** The cuts took about 187 nodes out of each; the lane's new work put about 350 in. Net: +165 nodes and +0.34 MB a figure.
3. **The figures' log lines are off by default now.** Any lane that reads `SK4KD`, `SK6REL`, `SK25` or `SK26` lines must first run `ke * SetLog 1`.

## Verdict per job

| Job | Verdict | The number that decides it |
|---|---|---|
| 1. The sets' sounds | **PASS by log**, unheard | 14 of 14 MetaSounds: the engine started one voice per call on both figures, each on the audio table's frame |
| 2. The brake effect's strength | **PASS by read-back**, unseen | `User.Strength` 0.7143 after a stop from 10 m/s, 0.6429 after `SetBraking` at 9 m/s (the speed over 14); 0.5 before |
| 3. The sets' final exports | **PASS**, with the pin's timing above | dive and weapons re-imported by name; the lab's derived offsets seat every prop with no correction; the dash export is the one already in the tree |
| 4. Export33's five takes | **PASS** | flutter 0 on both figures; `GetUp` by itself rises 5.3 cm a frame at most on the night figure, as the lab gives it |
| 5. The dash by numbers | **Done** | hand-over to `Stride` 34: skates 1.8 cm (clay), 3.2 cm (night); both skids hold 30 of 30 frames; film script written |
| 6. The cuts | **Done**, see point 2 above | the seeded match's 6 knockdown and stumble lines and 232 figure lines identical before and after |

Runs behind this: combined test sessions on test figures (clay: 34 figures, 335 frames; night: 37 figures, 420 frames; 0 Blueprint errors in each) and one seeded clay match before and after (seed 1, to sim frame 7200, 4 goals, 0 errors both times).

## Job 1. The sounds

Each sound is a `PlaySound2D` in one event (`Snd`), or a `SpawnSound2D` where the component is kept. Timed sounds ride a timer set half a frame short, so they fire on their frame.

**The engine's volume.** I set `LevelEditorPlaySettings.enable_game_sound` to false in memory before every play session and back to true at the end. No console variable was needed. It did not break the log: the engine still starts every voice.

**Proof.** With `log LogAudioMixer Verbose` the engine prints `Initializing mixer source voice: MS_<name>` for each sound it starts. I counted those against the figure's own `SK28 snd` lines, and measured each line's engine frame against the frame the move started.

| MetaSound | Table's frame | Started on (clay / night) | Calls / engine voices in the run |
|---|---|---|---|
| `MS_dive_block` | dive frame 0 | 0 / 0 | 3 / 3 (left, right, front) |
| `MS_dive_block_hit` | the hit | 0 / 0 after `PlayDiveBlockHit` | 1 / 1 |
| `MS_slide_tackle` | frame 0 | 0 / 0 | 1 / 1 |
| `MS_dash_start` | frame 0 | 0 / 0 | 7 / 7 |
| `MS_dash_loop` | `Dash_Start` frame 10 | 10 at rate 1.0; 9 at rate 1.10 (the take's frame 10) | 7 / 7 spawned, 7 faded out |
| `MS_dash_skid` | skid frame 2 | 2 / 2 | 3 / 3 |
| `MS_dash_wipeout` | frame 13 | 13 / 13 | 1 / 1 |
| `MS_boost_take` | frame 0 | 0 / 0 | 1 / 1 |
| `MS_giant_poke`, `MS_giant_sweep` | frame 0 | 0 / 0 | 1 / 1 each |
| `MS_mallet_swing` | frame 4, hit or miss | 4 / 4 | 2 / 2 |
| `MS_mallet_bonk` | frame 15, hit only | 15 / 15; none on the miss | 1 / 1 |
| `MS_cannon_fire` | frame 8 | 8 / 8 | 1 / 1 |
| `MS_cannon_kick` | frame 8, in place of the fire sound | 8 / 8; no fire sound on the kick | 1 / 1 |

- **The dash loop** is kept in `DashSnd` and faded out over 0.05 s in `EndDash` (every reason but the bump) and at a knockdown. Its `PitchShift` is set to 12 x log2 of the dash's rate (0.0 at rate 1.0, 1.72 at 1.10).
- **The skid.** Spawned on frame 2. If the brake is held it gets `HoldSeconds` 5, and `Release` is sent when the hold ends. Not held, it plays its default 0.28 s. In the run the two held skids logged `Release sent` on the frame the hold ended (30 frames).
- **The first run was one frame late** on four timed sounds (swing 5, bonk 16, wipeout 14, and the kick's hide). Timers set to whole frames fire on either side of the frame. Setting them half a frame short fixed all four.
- The rolling pin has no sound in the table and plays none.

## Job 2. The brake effect's strength

`SetVariableFloat(BrakeFx, "User.Strength", StopV0 / 14.0)` follows the first `Activate(BrakeFx)` in `StopTick`, and `BrkV0 / 14.0` follows the second (the `SetBraking` path). Read back from the component on test figures, the same on both:

| Case | Speed at the brake | `User.Strength` before | After |
|---|---|---|---|
| The figure's own stop, 10 m/s falling at 15 m/s² | `StopV0` 10.0 | 0.5 (the effect's default) | 0.7143 |
| `SetBraking(true)` at 9 m/s | `BrkV0` 9.0 | 0.5 | 0.6429 |
| A skid out of a dash with the brake held | - | 0.5 | 0.5: no brake effect starts, the skid outranks the stop |

## Job 3. The sets' final exports

### What came in

| Source | Gate | Per style |
|---|---|---|
| `sets-lab\weapons\export` (READY 04:39) | PASS, 26 rows, 0 failing | 9 takes re-keyed, 1 equal (`Fall_Bonk`), 3 new (`Pin_Carry`, `Pin_Roll`, `Pin_Miss`) |
| `sets-lab\dive\export` (READY 04:28) | PASS, 20 rows, 0 failing | 5 re-keyed (`Dive_Block_Left`, `_Right` now 40 frames; the three slides), 3 equal, 2 new (`Fall_Legs_Front`, `Fall_Legs_Back`) |
| `sets-lab\weapons\export\props` | not gated | 5 meshes x 2 styles; vertex counts as the lab's table (184, 244, 164, 52, 264); every material slot bound by name |
| `sets-lab\dash\export` | PASS, 20 rows | **kept, nothing re-imported**: READY is 03:07:20 and it is the loop-2 export lane SK26 imported; the tree's take lengths match its table |

Assets under `Skaters/`: 935 before, 953 after. All load, all 16 Blueprints compile, nothing left unsaved.

### The dive and the victims

- `Dive_Block_Left` and `_Right`: 40 frames, lying from frame 37. Joins into the lying loop 0.7 to 0.8 cm; the jolt and the get-ups as before.
- **`PlaySlideTackleHit(float DirX, float DirZ)`** is declared on the victim. `Dir` is the push, the tackler's travel. Push against the victim's facing (the tackler at his front half): `Fall_Legs_Front`. Otherwise: `Fall_Legs_Back`.
- Both orders work. The call and `SetKnockedDown(true, ...)` may arrive in either order within the same frame; four test figures gave the same result:

| Victim | Take, frames shown | Then | Join into the loop (clay / night) |
|---|---|---|---|
| Tackler at the front, hit first or knockdown first | `Fall_Legs_Front` 2 to 18 | `Lie_Loop_Front`, `GetUp_Front` | 2.0 / 2.8 cm |
| Tackler behind, either order | `Fall_Legs_Back` 2 to 18 | `Lie_Loop`, `GetUp` | 1.4 / 1.5 cm |

### The props

**The lab's derived offsets seat the props. I corrected nothing.** The four parts ride `stick_socket` with offset 0 and the quaternion of correction 2; the keg rides `hand_l` with the report's offsets, its rotation rows turned into a quaternion the usual way. (The same conversion gives back correction 2's own quaternion from its rows.)

| Check | Clay | Night | The lab's number |
|---|---|---|---|
| Giant carry, lowest point over `Stride` | 14.9 cm | 16.2 cm | 14.9 / 16.2 |
| Rolling pin, lowest point (`Pin_Miss`) | 19.2 cm | 18.3 cm | 19.2 / 18.3 |
| Roller's top at contact | 46.0 cm | 45.4 cm | 45 to 48 |
| Keg's front face at `Cannon_Fire` frame 8, against the lab's muzzle | 0.0 cm sideways, 0.0 up, 3.8 ahead | the same | the 3.8 cm is along the barrel: the mesh's front face against the lab's muzzle point |
| Crank against keg, nearest vertices | 0.9 cm, every measured frame | 0.9 cm | the two attach rules agree to 0.2 cm |
| Dropped keg's centre | (17.8, -150.7, 21.0) | (17.1, -153.6, 21.0) | the same, to 0.0 cm; axis along his forward; lowest point 0.2 cm |
| Giant poke / sweep, lowest point | 0.6 / 1.6 cm | 0.6 / 0.5 cm | nothing under the ice |
| Mallet, lowest point (`Mallet_Miss`) | 15.8 cm | 18.6 cm | - |

- **The kick.** Both parts shown on frames 0 to 10; on frame 11 both go and the static keg appears, in the same frame. The stick comes back at the get-up. The figure then has no weapon (`WeaponKind` 0). The dropped keg stays 4 s, or until the next `SetWeapon`.
- **Weapon 4** is the rolling pin: `SetWeapon(4)` with `Pin_Carry` as a fifth switch of the carry layer, `PlayWeapon(4, ..., bHit)` plays `Pin_Roll` or `Pin_Miss`, and `PlayWeaponHit(4, ...)` gives the victim `Fall_Legs_Front`.
- **The pin's contact.** With the victim at the lab's spacing (0.96 m clay, 1.04 m night), the roller's nearest point to the shin's bone line:

| Attacker's frame | Victim still standing (clay / night) | Victim called on frame 14 (clay / night) |
|---|---|---|
| 14 | 13.0 / 15.6 cm | 12.4 / 15.7 cm |
| 16 (contact) | 5.8 / 8.1 cm: on the shin | 23.8 / 28.9 cm: the legs have left |
| 18 | 6.7 / 8.9 cm | 31.8 / 38.4 cm |

  The weapons report gives frame 14 for `Trip`. `Fall_Legs_Front` has its contact on its own frame 2 and is played from there, so its call belongs on the attacker's frame 16.
- **In a dash the prop is hidden** and the stick shown, as in any take that is not the weapon's own. So the giant carry's 1.4 cm margin over `Dash_Loop` does not arise in the engine today.

## Job 4. Export33, the five takes it clears

`Lie_Loop_Front`, `GetUp`, `GetUp_Front`, `GetUp_Side_Left`, `GetUp_Side_Right` re-keyed by name from the loopcut file, both styles. Nothing else of export33 was touched (149 takes in the file). The engine-side smoothing of lane SK26 is gone with the old keys.

- **Flutter: 0 on both figures.** A 200-frame knockdown shows the whole loop (139 frames): frames on which the left hand jumps over 1 cm against its neighbours 0, largest one-frame move of the hand 1.5 cm (clay), 1.4 cm (night). It was 9 and 11.
- **The get-ups' largest one-frame rise of the head:**

| Take | By itself, no blend (clay / night) | As played after the loop (clay / night) |
|---|---|---|
| `GetUp` | 5.8 / **5.3 cm** (the lab: 5.3) | 9.7 / 9.3 cm |
| `GetUp_Front` | 7.1 / 6.4 cm | 12.3 / 10.8 cm |
| `GetUp_Side_Left`, `_Right` | 7.3 / 6.5 cm | 7.5 / 8.5 cm |

  As played it is higher because `PlayUp` blends 5 frames out of the lying loop: the start is held back, then catches up. I did not change the blend (see decisions).

## Job 5. The dash, by bone positions

Test figures in open ice (no fall in the run chose a wall take), skating at 8 m/s under the takes, a seventh figure untouched for scale (its skates move 6.7 cm a frame at most on clay, 7.5 night). The root bone stays on the actor in every take (0.00 cm): the path is the pawn's.

| Stretch | Frames shown | Skates' step at the join (clay / night) | Note |
|---|---|---|---|
| into `Dash_Start` | 0 to 9 | 3.2 / 3.4 cm | |
| into `Dash_Loop`, three cycles | 0 to 67 at rate 1.0 | 1.0 / 0.9 cm | inside the loop the skates move up to 15.2 / 16.4 cm a frame; three cycles take 68 frames (below) |
| into `Dash_End` | 0 to 10 | 15.2 / 16.4 cm | the loop's own pace, not a pop |
| **onto `Stride` frame 34** | 34 to 46 | **1.8 / 3.2 cm** | as last time; the next frame 8.2 / 10.6 cm is the stride's own push |
| `Stride` back to the locomotion | - | 2.2 cm (clay) | |
| into either skid | 0 to 11 by the readout, the rest under the 9-frame blend out | 9.4 / 10.2 cm | |
| the skid's hold | frozen on frame 8 (the take's hold is 6 to 9) | 30 of 30 frames still, both sides, both figures | ends within 2 frames of `SetBraking(false)`, or after 2 s |
| into the wipeout | 0 to 19, then `Lie_Loop_Front` | 9.4 / 10.2 cm; into the loop 0.3 cm | own release, 86 frames |
| into the bump, and back | 0 to 7 by the readout, then the loop | 13.9 / 14.5 cm in; 18.2 / 19.7 cm back | the head moves 11.5 and 13.2 cm in the two frames of the return (clay) |

**The bump's return is the take's own.** Played by itself the take's head steps are 8.0, 12.7, 12.2, 2.9, 4.9, 5.5, 11.9 cm: it comes back to the loop's pose in two fast frames. The engine adds nothing there.

**A loop cycle is 23 frames in the engine, not 24.** The loopcut take is 23 intervals long and the montage wraps from its last key to its first with no frame between. Three cycles take 68 frames. This is how lane SK26 imported it; I did not change it.

The filming script is `status\SK28-film.md`.

## Job 6. The cuts

Done, both Blueprints:
- **The test trigger asks once** whether this is the human's figure (20 lookups of the game mode before, 1 now), then calls `SK26Do`.
- **Every print goes through one event, `Log`, behind `bLog`** (53 print nodes before, 1 now). `SetLog(bool)` switches it; off by default.
- Two sets in `ReceiveBeginPlay` of variables nothing reads (`ChkT`, `StumF`).

Not cut: `SetTripped`. Nothing calls it, but handoff B-to-A-0028 offers it to the other device, so it is not plainly dead.

| | Clay before | Clay after | Night before | Night after |
|---|---|---|---|---|
| Nodes in the event graph | 4,192 | 4,357 | 4,103 | 4,268 |
| Asset bytes | 9,685,732 | 10,021,696 | 9,476,476 | 9,812,787 |
| The trigger alone, by the text's count | 330 | 145 | 330 | 145 |

- The cuts took about 187 nodes out of each figure (about 0.43 MB at 2,311 bytes a node). The lane's new events and edits put about 350 in. I did not build the cuts alone, so the split is from the text's counts, not from two saved assets.
- **The log switch saved no bytes.** A print node became a call node and the strings are still built. What it buys is a quiet log.
- **The seeded match** (clay, seed 1, to sim frame 7200), before and after: the sim's 3 knockdown and 3 stumble lines identical; the figure's 232 lines (falls, loops, get-ups, releases, reactions, whips, check sizes) identical; end minus release 0.0 on all three knockdowns; 0 errors; 4 goals both times. The before run has 8 more figure lines, all from the first second, before `ke * SetLog 1` took effect in the after run.

## Not verified

- **Anything by eye.** No frame of this lane was rendered. `status\SK28-film.md` lists what the GPU must film: the props in the hands, the kick, the victims, the dash and its held skid, the bump's return, the get-ups' start.
- **Anything by ear.** The sounds started; their level, the loop's fade, the skid's body-to-tail jump and the pitch at other rates are unheard.
- **The brake effect on screen.** The parameter reads back; the component reports not active without a GPU.
- **The skid's `Release`.** The trigger is sent; that the MetaSound's tail then plays was not measured.
- **The pin's pair with the call on frame 16.** I measured the roller against the standing pose and against the fall's first frame, not a run with the call moved.
- **A real slide-tackle pair.** The victims were tested alone. Whether the victim lands clear of the slider depends on the game's push.
- **The cooked size** of the figures. The table is the editor asset.
- **The night level.** The night figure was tested on the clay level's ice.

## For the Blender set lanes

- **Weapons:** the offsets of correction 2 are right as derived, keg included: numbers above. Nothing to change.
- **Weapons:** the keg mesh's front face is 3.8 cm ahead of your muzzle point at frame 8. If the puck should leave from the face, move the point.
- **Weapons:** `Fall_Legs_Front` under the pin needs the call on your frame 16. If you want frame 14 kept, the victim take needs 2 lead-in frames.
- **Weapons and audio:** the audio table fires `MS_mallet_bonk` on frame 15; your contact is frame 17. One of the two is 2 frames off.
- **Dash:** `Dash_Bump_Left` and `_Right` return in two frames (head 12 cm a frame). A third return frame would take the snap out.
- **Dash:** the engine loops `Dash_Loop`'s loopcut file in 23 frames (last key straight to first). Say whether the cycle is meant to be 24; if so the engine needs the file with the repeated last frame.
- **Dive:** the slide's stick arm is still moving 20 cm a frame at frame 18 and stops into the loop (9.4 cm at the join on the night figure, 3.5 on clay). A settle of two more frames would land it.
- **Figure lab:** `GetUp_Quick` still rises 21.8 cm (clay) and 20.4 cm (night) in one frame as played. It was not in export33's re-time.

## For the other device: every call by name for the three sets

All on the skater's visual, called by name. `Dir` is in world x and z as in the contract. `Frames` is the whole time down until he is up; under 64 it takes the default.

| Call | Inputs | What it plays | When to call |
|---|---|---|---|
| `PlayDiveBlock` | `float DirX, float DirZ, int32 Frames` (default 103) | `Dive_Block_Left` or `_Right` by the side of `Dir`, 40 frames, lying from 37, own get-up | the dive's first frame; a `SetKnockedDown(true)` while he is down is adopted |
| `PlayDiveBlockFront` | `float DirX, float DirZ, int32 Frames` (default 101) | `Dive_Block_Front`, 38 frames, face down from 35 | the dive's first frame |
| `PlayDiveBlockHit` | none | the 12-frame jolt, then the loop again | the frame the puck strikes him; only while lying in a side dive |
| `PlaySlideTackle` | `int32 Frames` (default 85), `int32 Side` | `Slide_Tackle_Left` (Side 0 or more) or `_Right`; contact frame 10; on the back from 19 | the slide's first frame |
| `PlaySlideTackleHit` | `float DirX, float DirZ` (the tackler's travel) | the victim's `Fall_Legs_Front` or `_Back`, from its frame 2 | on the victim, on the tackler's frame 10, in the same frame as `SetKnockedDown(true, ...)`, either order |
| `SetWeapon` | `int32 Kind`: 0 none, 1 giant stick, 2 mallet, 3 puck cannon, 4 rolling pin | the carry and the prop; the stick hidden | on pickup and on loss; also removes a dropped keg |
| `PlayWeapon` | `int32 Kind, float DirX, float DirZ, int32 Frames, bool bHit` | 1: `Giant_Poke` if `Dir` is within 20 degrees of his facing (contact 9), else `Giant_Sweep` (contact 10 to 14). 2: `Mallet_Swing` on a hit (contact 17), else `Mallet_Miss`. 3: `Cannon_Fire` when not hit (puck at 8); `Cannon_Kick` when hit: he goes down (`Frames`, default 89), the keg drops at 11 and his weapon is gone. 4: `Pin_Roll` on a hit (contact 16), else `Pin_Miss` | the move's first frame; `Frames` is used by the kick only |
| `PlayWeaponHit` | `int32 Kind, float DirX, float DirZ` | on the victim. 2: `Fall_Bonk`. 4: `Fall_Legs_Front`. Other kinds: nothing | in the same frame as `SetKnockedDown(true, ...)`, either order. Mallet: the attacker's frame 16. **Pin: the attacker's frame 16** |
| `PlayDash` | `int32 Frames` | `Dash_Start`, the loop for the frames given (rate 0.7 to 1.4), `Dash_End`, then `Stride` | the dash's first frame |
| `SetBoostLevel` | `bool bOn, float SpeedScale` | on: `Dash_Start` and the loop until off, rate 0.9 to 1.3; off: `Dash_End` | named so because `SetBoost(FName Kind)` exists |
| `EndDash` | `uint8 Reason, float DirX, float DirZ` | 0: `Dash_End`. 1: `Dash_Skid_Left` (DirZ 0 or more) or `Dash_Skid`, held on frame 8 while `SetBraking(true)` is on (2 s at most). 2: `Dash_Bump_Left` or `_Right`, then the loop for the time left. 3: `Dash_Wipeout`, face down, own get-up (default 86 frames) | the frame the dash ends; for a held skid call `SetBraking(true)` in the same frame and `SetBraking(false)` at the release |
| `PlayBoostPickup` | none | `Boost_Take` on the arms, 12 frames | the pickup's frame |
| `SetLog` | `bool bOn` | the figure's log lines on or off (off by default) | lanes only: `ke * SetLog 1` |

Three notes to send with it:
- **The slide's victim lands clear only if the game keeps the carrier's velocity and adds a sideways push of about a metre** (the dive lane gives 1.0 to 1.2 m). Without it the victim falls onto the slider.
- **Every sound of the three sets now plays from the figure.** If the game also fires any of the fourteen MetaSounds on a bus event, it will play twice.
- After `PlayWeapon(3, ..., true)` the figure has dropped its weapon by itself. `SetWeapon(0)` after it is harmless and removes the keg on the ice at once.

## Decisions for the operator

1. **The dropped keg's life.** It lies 4 s and goes without a fade. Longer, shorter, or until someone picks it up (then the game owns it)?
2. **The get-ups' blend.** Shortening the 5-frame blend out of the lying loop to 2 would bring the as-played rise down toward the takes' 5 to 7 cm. It touches every get-up, the early ones too, so I left it for a filmed comparison.
3. **The log's default.** Off, as the brief's "behind a test bit". If the QA lanes would rather have it on until submission, it is one class default (`bLog`).
4. **A brake spray on the dash skid.** None starts today. Wanted?
5. **The prop in a dash.** It is hidden for the dash and returns after. Keep the swap, or carry through the dash (the giant stick then has 1.4 cm over the ice on clay)?

## The next three evolutions

1. **Film this lane on the GPU** with `status\SK28-film.md`, ears on. Proving numbers: one still per row; the kick's strip shows no frame with two kegs; each of the 14 sounds judged against the knockdown's level.
2. **The take choices as tables**, the real size cut: `FallPick`, `PlayGetUp`, `PlayLie`, `ChkCut`, `WeaponPick` as arrays indexed by kind and size, and the trigger with `SetTestMode` in a child Blueprint. Proving numbers: under 3,900 nodes and under 9.0 MB a figure; the seeded match's 232 figure lines identical.
3. **The pairs in one test**: tackler and victim, mallet and victim, pin and victim on two figures at the labs' spacings with the calls on their frames. Proving numbers: the pin's roller within 7 cm of the shin line on frame 16 with the victim on its frame 2; the slide's victim at least 30 cm clear of the slider's bones at rest with a 1 m push.

## Ledger rows

76 full rows in the ledger's 11 columns (path, kind, size_mb, origin, tool, licence, url, date, device, shipped, note): the 2 figure Blueprints, the 2 animation Blueprints, 28 re-keyed takes, 8 takes saved again with equal keys, 10 new takes, 10 re-keyed export33 takes, 8 new prop meshes, 2 re-imported keg meshes, 6 material instances saved again. 58 replace a row already in `ASSETS.md`; 18 are new.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 9.36 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python |  |  | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes by the knockdown's length (light under 90 frames, heavy from 130), the snap get-up under 70 frames, GetUp_Wall_Back, the wall falls shifted to the boards with the real normal in the corners, PlayStumble and SetBraking declared by name, a test switch for the late get-ups (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the pawn's PlayStumble covers every stumble (the check-bus read dropped), the stop driven by SetBraking's level and gated (7 m/s at the start, held 6 frames for a bot with 8 s between its stops, from the first frame for the human), lying loops from the frame the fall holds its end pose (lane SK24); +a plain collision gets no whip; the whip's two weights (WhipHeadW, WhipArmW); the check's and the cross-check's sizes (closing speed on the press, the Strength's band when the bus starts the take); the calls PlayDiveBlock, PlayDiveBlockFront, PlayDiveBlockHit, PlaySlideTackle, SetWeapon, PlayWeapon, PlayWeaponHit, PlayDash, SetBoostLevel, EndDash, PlayBoostPickup by name, riding the knockdown's machinery; the WeaponMesh component on stick_socket; the test trigger SK26Test (lane SK26); the sets' fourteen sounds by PlaySound2D or SpawnSound2D at the audio table's frames (the dash loop kept and faded out, the skid held and released); the brake effect's User.Strength after both Activate nodes; the dives' hold at 37; the tackle's and the pin's victims (PlaySlideTackleHit, PlayWeaponHit kind 4, Fall_Legs_Front or _Back from frame 2); the four real props on stick_socket and the keg on hand_l (components WeaponKeg, KegDrop), the kick's dropped keg; the rolling pin as weapon 4; the skid's hold while the brake is held; every print behind SetLog(bool), off by default; the test trigger's lookup asked once (lane SK28) (9812787 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 9.56 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) |  |  | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4); stumble, board falls, quick get-ups, hard stop and deke as BP_SkaterVisual_TB1 (lane SK21); the hard stop gated to an event: 7 m/s at the start, 3.5 m/s lost, 1.5 s between the human's stops, 12 s between a bot's (lane SK22); StopTick plays MS_brake (PlaySound2D) where the hard stop commits (lane AU11); fall sizes, snap get-ups, GetUp_Wall_Back, the wall shift, PlayStumble and SetBraking by name and the late get-ups' test switch as BP_SkaterVisual_TB1 (lane SK23); + BrakeFx Niagara component (NS_brake), activated at the two MS_brake nodes: the StopTick commit and the SetBraking pose (lane B40); the stumble by the pawn's call only, the gated level-driven stop and the lying loops as BP_SkaterVisual_TB1 (lane SK24); +the whip's constant and weights, the check sizes, the eleven set calls, the WeaponMesh component and the test trigger as BP_SkaterVisual_TB1 (lane SK26); the sets' sounds, the brake effect's strength, the victims' calls, the real props with WeaponKeg and KegDrop, weapon 4, the skid's hold, the prints behind SetLog and the trigger's single lookup as BP_SkaterVisual_TB1 (lane SK28) (10021696 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 0.83 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4); +the carry layer on the aim layer's input: four bool switches (Giant_Carry, Mallet_Carry, Cannon_Carry, Boost_Take), mesh space from spine_01 (lane SK26); the carry layer's fifth switch: bCarry5 with Pin_Carry, under the boost beat (lane SK28) (871620 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.83 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3); +the carry layer's four switches as ABP_SkaterTB1 (lane SK26); the carry layer's fifth switch (bCarry5, Pin_Carry) as ABP_SkaterTB1 (lane SK28) (867464 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (109405 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Poke.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Poke by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (135315 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Sweep.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (138117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (109431 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Swing.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (161527 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Miss.uasset | anim | 0.18 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (187813 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (109431 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Fire.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Fire by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (142215 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Cannon_Kick.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Cannon_Kick by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p2_cel (lane SK28) (160007 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p2_cel (lane SK28) (173758 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p2_cel (lane SK28) (173776 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p2_cel (lane SK28) (149516 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p2_cel (lane SK28) (147934 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Slide_Tackle.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Slide_Tackle by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p2_cel (lane SK28) (149424 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Bonk.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Fall_Bonk by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/weapons/export/p2_cel (lane SK28) (141601 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Hit_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Hit_Left by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p2_cel (lane SK28) (123694 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Hit_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Hit_Right by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p2_cel (lane SK28) (124544 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dive_Block_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Dive_Block_Front by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p2_cel (lane SK28) (163162 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28) (109370 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Roll.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Roll by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28) (146056 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Miss.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Miss by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28) (157904 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Legs_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Fall_Legs_Front by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28) (145768 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Fall_Legs_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p2_cel, take Fall_Legs_Back by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28) (147478 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take Lie_Loop_Front by name on SKEL_SkaterTB1v2; the left arm's rotation keys smoothed in the engine over the first move (they alternated 3 to 6 cm at the hand frame to frame); a re-import brings that back until the lab fixes it (lane SK26); re-keyed by name from skf/export33/p2_cel loopcut: the left arm's flutter fixed in the take, the engine-side smoothing of lane SK26 gone (lane SK28) (252065 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p2_cel loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (165544 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p2_cel loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p2_cel loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (164930 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p2_cel loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (174102 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_GetUp_Side_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p2_cel loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1v2 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p2_cel loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (171781 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (109450 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Poke.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Poke by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (138232 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Sweep.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (141834 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (109468 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Swing.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (161564 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Miss.uasset | anim | 0.18 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (192650 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (109468 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Fire.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Fire by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (140650 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Cannon_Kick.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Cannon_Kick by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/weapons/export/p3_clay (lane SK28) (160042 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p3_clay (lane SK28) (177728 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p3_clay (lane SK28) (177746 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p3_clay (lane SK28) (150342 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p3_clay (lane SK28) (150360 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Slide_Tackle.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Slide_Tackle by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name from sets-lab/dive/export/p3_clay (lane SK28) (150250 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Bonk.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Fall_Bonk by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/weapons/export/p3_clay (lane SK28) (143462 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Hit_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Hit_Left by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p3_clay (lane SK28) (123720 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Hit_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Hit_Right by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p3_clay (lane SK28) (124570 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dive_Block_Front.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Dive_Block_Front by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); compared with the final export: keys equal, saved again by name from sets-lab/dive/export/p3_clay (lane SK28) (166932 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28) (109404 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Roll.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Roll by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28) (146090 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Miss.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Miss by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28) (157930 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Legs_Front.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Fall_Legs_Front by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28) (148386 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Legs_Back.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/dive/export/p3_clay, take Fall_Legs_Back by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28) (147504 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lie_Loop_Front.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take Lie_Loop_Front by name on SKEL_SkaterTB1_P3; the left arm's rotation keys smoothed in the engine over the first move (they alternated 3 to 6 cm at the hand frame to frame); a re-import brings that back until the lab fixes it (lane SK26); re-keyed by name from skf/export33/p3_clay loopcut: the left arm's flutter fixed in the take, the engine-side smoothing of lane SK26 gone (lane SK28) (251878 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p3_clay loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (175981 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export30/p3_clay loopcut, take GetUp_Front by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p3_clay loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (177645 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Left by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p3_clay loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (186928 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | skf/export29/p3_clay loopcut, take GetUp_Side_Right by name through the data controller on SKEL_SkaterTB1_P3 (keys only; re-import by name only) (lane SK25); re-keyed by name from skf/export33/p3_clay loopcut: re-timed, 36 frames, the rise at the start spread (lane SK28) (184601 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_GiantStick_TB1v2.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_GiantStick_p2_cel.fbx, the giant stick, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (76497 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_GiantStick_TB1_P3.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_GiantStick_p3_clay.fbx, the giant stick, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (76498 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_Mallet_TB1v2.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_Mallet_p2_cel.fbx, the mallet, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (77008 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_Mallet_TB1_P3.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_Mallet_p3_clay.fbx, the mallet, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (77006 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_RollingPin_TB1v2.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_RollingPin_p2_cel.fbx, the rolling pin, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (74970 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_RollingPin_TB1_P3.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_RollingPin_p3_clay.fbx, the rolling pin, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (74990 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_Crank_TB1v2.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_Crank_p2_cel.fbx, the puck cannon's crank, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (71817 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_Crank_TB1_P3.uasset | mesh | 0.07 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_Crank_p3_clay.fbx, the puck cannon's crank, material slots bound by name to MI_Prop_*; rides stick_socket (lane SK28) (71839 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_TB1v2.uasset | mesh | 0.08 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_p2_cel.fbx, the puck cannon, 4 material slots bound by name (lane SK26); re-imported from the final export: the keg alone (the crank is its own mesh now), rides hand_l (lane SK28) (83484 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Prop_PuckCannon_TB1_P3.uasset | mesh | 0.08 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/props/Prop_PuckCannon_p3_clay.fbx, the puck cannon, 4 material slots bound by name (lane SK26); re-imported from the final export: the keg alone (the crank is its own mesh now), rides hand_l (lane SK28) (83509 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayTimber.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.62, 0.42, 0.22) for the props' slot ClayTimber (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4861 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayTape.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.90, 0.86, 0.74) for the props' slot ClayTape (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4849 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayFelt.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.78, 0.20, 0.14) for the props' slot ClayFelt (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4849 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayCream.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.92, 0.86, 0.72) for the props' slot ClayCream (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4855 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayRed.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.78, 0.20, 0.14) for the props' slot ClayRed (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4843 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Prop_ClayPuck.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | a copy of MI_SkaterTB1_Stick (parent M_Toon) with TeamColour at sRGB (0.16, 0.14, 0.13) for the props' slot ClayPuck (lane SK26); colour set again to the same value and saved by the import of lane SK28 (lane SK28) (4849 bytes) |
