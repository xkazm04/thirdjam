# Lane SK30 report (resumed run): three calls, the weapons' fourth export, the dash's closed export, the skid's sway, the quick get-ups

Device B, tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, on dev-b 06043209). No commit, no push, no cook. Assets changed only under
`Game/Plugins/JamAssets/Content/Skaters/`: 18 changed, 4 new. Nothing here is in the package the operator plays.

## 1. Summary

The previous agent (cut off at about 07:35) had done more than anyone knew: jobs 1, 2 and 4 were built, saved and measured on test figures, and the two
seeded matches were run. Its analysis script crashed before printing the pin's end, the skid and the get-ups; I re-ran that analysis on its recorded data
(no new engine run needed for those numbers). I then did job 3 (the dash's closed export: 10 takes equal to the tree, 2 new takes imported and wired), fixed
one frame where a carried prop showed in a dash, re-ran seed A to check the match is unchanged, and wrote the film script. **Not done:** no still (the
editors were headless, no GPU); night's `Mallet_Miss` not measured; the quick get-ups from the front and sides are not under 10 cm (no variant got there,
so they stay as they were).

## What the previous run had done (found, not redone)

- Its editor, pid 27948 (headless, port 8004, started 07:13), answered at once. **Dirty packages: none, content or map**, so nothing was saved or
  discarded by me. All 18 modified files were already saved by it: the 14 weapon takes at 07:15, the four Blueprints last at 07:28.
- The editor log (`Game\logs\editor-20261114-071307-0.log`) and its work folder (`...\scratchpad\sk25\`, scripts `imp36.py`, `bp38.mjs`, `bp39.mjs`,
  `abp31.mjs`, `abp32.mjs`, `a15.py`; data `logs\w6_s5c.json` clay, `logs\w6_s5n.json` night, `logs\full_s30a.txt`, `logs\full_s30b.txt`) show:
  - **Job 2 import** (07:14): the weapons' fourth export by name, 7 takes per style re-keyed (`Giant_Sweep` 23 keys, `Mallet_Carry` 31, `Mallet_Swing` 37,
    `Mallet_Miss` 50, `Pin_Carry` 31, `Pin_Roll` 31, `Pin_Miss` 41), 6 per style equal and untouched (`Giant_Carry`, `Giant_Poke`, `Fall_Bonk`,
    `Cannon_Carry`, `Cannon_Fire`, `Cannon_Kick`).
  - **Job 1 and the pin's victim** (07:16 to 07:28): `SetStickLock`, `SetTakeGrace`, `PlayCheckMiss` and the miss rule on both figure Blueprints; two
    Modify Bone nodes at the end of each animation Blueprint's whip chain (trunk on `spine_01`: `GraceRot`/`GraceA`; stick on `stick_socket`:
    `LockRot`/`StickA`); the pin's victim switched to `Fall_Legs_Back` (`BonkArm 3`); the skid's hold sway (`SkidSway`, default 1.5 frames); the quick
    get-ups' blend as a variable (`UpBlendQ`, default 0.083 s = the old 5 frames); a test call `Tune(Which, Val)` and `TestBusCheck`.
  - **Test figures** clay and night (07:19 to 07:30), and **two seeded clay matches** (seeds A and B, 07:30 to 07:35, to sim frames 7242 and 7137).

## Verdict per job

| Job | Verdict | The number that decides it |
|---|---|---|
| 1. Three calls | **Done** (previous run), measured | lock: blade +11.0 cm (clay) / +11.7 (night), skates' step unchanged; grace: shoulders turned 17.5 / 17.2 deg, stick 7 cm nearer the hips; both layers under a knockdown 0.00 cm from none; miss: stumble after 7 frames of check, no fall |
| 1. Miss-rule frequency | **Fires too often, one false** | 5 of 12 `PlayCheck` calls in two seeded matches (A 4 of 7, B 1 of 5); one of the five was a check that landed 13 frames after the call |
| 2. Weapons' fourth export | **Done** | drum top under the lower shoulder on every shown frame: by at least 0.6 cm (clay swing frame 14) / 3.3 cm (night); pin's victim falls away: head from 80 to 173 cm ahead of the attacker's root (clay), never nearer than 79 cm after contact |
| 2. Mallet hidden in a dash | **Fixed this run** | prop shown on 1 of 91 dash frames before, **0 of 92** after, stick shown on 92 of 92 (clay and night) |
| 3. The dash | **Done** | `READY.txt` 10:00:49 (newer than 03:07), `GATE.md` PASS 24 rows; 10 takes per style equal to the tree, `Dash_Wipeout_Light` and `Dash_Carry` new and wired; `Dash_Wipeout_Wall` not in the export: not wired |
| 4a. Skid's hold sway | **Done** (previous run), measured | the held take moves between its frames 6.5 and 9.5 and back; joints at most 4.3 cm apart in the hold, largest one-frame step 1.06 cm (clay) / 1.14 (night) |
| 4b. Quick get-ups under 10 cm | **Not reached; left as they were** | `GetUp_Quick_Front` 14.8 cm (clay) / 13.0 (night) with the 5-frame blend; no blend 13.7 with a 13.2 cm pop on the first frame |
| 5. Film script | **Done** | `C:\Users\kazda\jam\wt\b-fig\status\SK30-film.md` (13 shots) |

## 2. Per evolution

### E1. The three calls (job 1)

Test figures in a headless play session (previous run's data, re-analysed). Blade height is the blade point above the ice; the call on tick 30.

| Figure | Stick lock, 20 frames | Clay | Night |
|---|---|---|---|
| S1 skating 6 m/s | blade before / highest | 4.1 / 15.1 cm (+11.0), down by tick 51 | 5.7 / 17.4 cm (+11.7), down by tick 51 |
| S1 | skates' largest one-frame move in the window | 5.7 cm (the stride's own pace) | 6.5 cm |
| S2 standing | blade highest | +10.9 cm | +11.5 cm |
| S9 standing, `SetStickLock(false)` on tick 40 | blade down | by tick 45 (4 frames after the off call) | by tick 45 |
| S7 tuned `LockLift -5` (test only) | blade | pushed 11 cm into the ice: the sign is right, the value is not used | same |

| Figure | Take grace, 30 frames | Clay | Night |
|---|---|---|---|
| S3 skating 6 m/s | shoulders' line, largest turn | -17.5 deg (tick 41), back within 1 deg by tick 61 | -17.2 deg, back by tick 61 |
| S3 | blade to hips (flat) | 89 to 83 cm (-7) | 92 to 86 cm (-7) |
| S3 | head moves at most | 6.6 cm | 6.9 cm |
| S5 against S6 | both layers called during a 110-frame knockdown against none | 0.00 cm (every joint, ticks 20 to 150), blade 0.00 cm | |

| Missed check (`PlayCheck` on tick 30) | Clay | Night |
|---|---|---|
| C1 no bus event | `Check_Deliver_Left` frames 0-6, then `Stumble_Back` frames 2-23, skating on tick 59; hips at most 36 cm ahead of the root, head lowest 64 cm | same shape (`Check_Deliver_Right`), head lowest 67 cm |
| C2 bus event 2 frames after | clean 19-frame check, no stumble | same |
| C3 bus event 2 frames before | clean 19-frame check, no stumble | same |
| C4 `PlayCheckMiss()` outright | `Stumble_Back` frames 2-23, 22 frames | same |

**The miss rule in two seeded clay matches** (logs of the previous run, `full_s30a.txt`, `full_s30b.txt`; a `PlayCheck` is a `SK25 check close` line):

| Match | `PlayCheck` calls | check bus events | misses by the rule | of which false |
|---|---|---|---|---|
| A (to sim frame 7242) | 7 | 6 | 4 | 1: skater 7, the miss started 7 engine frames after the call, the check event came 13 frames after it |
| B (to sim frame 7137) | 5 | 6 | 1 | 0 seen |

Too often for something meant to read as a rare overbalance (42 % of calls), and the 6-frame window is shorter than the sim's own delay at least once.
None of the three calls is made in a match (0 lock or grace lines). Match A, B: 0 Blueprint runtime errors.

Still: none (headless). Film: shots 1 to 6 of the film script.

### E2. The weapons' fourth export (job 2)

Measured on test figures (previous run's data, re-analysed). Drum = the 198 vertices of the mallet's head; shoulder = the lower of the two upper-arm joints.

| Measure | Clay | Night |
|---|---|---|
| Carry (88 / 63 frames): drum top under the lower shoulder by at least | 34.9 cm | 44.5 cm |
| `Mallet_Swing` (on its 31 frames with the prop shown): at least | **0.6 cm** (take frame 14, drum top 88.1 cm) | 3.3 cm (frame 15, drum top 96.8 cm) |
| `Mallet_Miss` (25 frames recorded): at least | 0.6 cm (frame 14) | not recorded |
| Drum centre to the victim's hips, attacker frames 16 / 17 / 18 / 20 | 108.5 / 48.7 / 27.0 / 63.2 cm (victim `Fall_Bonk` from frame 0 at 16) | 110.0 / 48.3 / 22.7 / 62.3 cm |
| Pin's highest point under the lower shoulder by at least | 3.8 cm (`Pin_Roll` frame 24) | 9.8 cm (frame 10) |

**The pin's victim, `Fall_Legs_Back`, called on the attacker's frame 15** (victim 0.96 m / 1.04 m ahead, facing him; distances ahead of the attacker's root):

| Attacker frame | Victim | Head (clay / night) | Hips (clay / night) | Nearest joints of the two (clay) |
|---|---|---|---|---|
| 14 | standing | 80 / 87 cm | 96 / 104 | 30.8 cm |
| 15 | `Fall_Legs_Back` 2 | 80 / 87 | 96 / 104 | 25.4 |
| 16 | 3 | 92 / 100 | 97 / 105 | 16.7 |
| 18 | 5 | 130 / 139 | 102 / 110 | 13.6 |
| 24 | 11 | 159 / 169 | 124 / 132 | 49.6 |
| 40 | `Lie_Loop` | 173 / 183 | 126 / 134 | 59.1 |

After contact the head is never nearer than 79 cm (clay) / 100 cm (night), the hips never nearer than 97 / 105 cm. In SK29 the victim ended 20 cm ahead,
on the attacker's skates. `Fall_Legs_Front` is no longer reached by the pin (`BonkArm 3`); the mallet keeps `Fall_Bonk`.

**The prop in a dash (this run).** The prop was hidden by the 60 Hz weapon tick, one frame late. `DashBegin` now hides it and shows the stick at once.
Test figures at 8 m/s, `SetWeapon(2)` or `(4)` on tick 20, `PlayDash(82)` on tick 60: before, prop shown on 1 of 91 dash frames, stick on 90; after,
**prop on 0 of 92, stick on 92 of 92**, clay and night, mallet and pin; the prop shown on 20 of 20 frames before the dash. Still: none (headless). Film: shot 9.

### E3. The dash's closed export (job 3)

- `C:\Users\kazda\jam\sets-lab\dash\export\READY.txt` is 10:00:49 (newer than 03:07); `GATE.md`: **PASS, 24 take-and-style rows, 0 failing**,
  12 takes per style, READY lists the same 12. `Dash_Wipeout_Wall` is not in it (held back, part 3 of `SET-dash-report.md`): **not wired**.
- Re-import by name of all 12 per style from `<style>\SK_Skater_<style>_<Take>.fbx` (the files with the repeated last frame, not the loopcut files),
  compared on every second frame of every bone (0.001 cm, 0.0005 quaternion):

| Takes | Result (each style) |
|---|---|
| `Dash_Loop` (25 keys), `Dash_Start` 11, `Dash_End` 13, `Dash_Bump`, `_Left`, `_Right` 10, `Dash_Skid`, `_Left` 21, `Dash_Wipeout` 25, `Boost_Take` 12 | **equal to the tree: not touched.** This also closes SK29's question: the tree's dash takes are the closed export, and SK29's 25th key equals the lab's |
| `Dash_Wipeout_Light` (23 keys, 0.367 s) | **new asset** |
| `Dash_Carry` (25 keys, 0.4 s) | **new asset** |

- **Wiring, both figure Blueprints:**
  - `SetHasPuck(bHasPuck)` (the interface call, empty until now) stores `bPuck`. When the dash loop starts (`DashLoopGo`: after `Dash_Start` and after
    a bump), `Dash_Carry` plays instead of `Dash_Loop` if `bPuck` is set. Same length, same timer.
  - `EndDash(4, ...)`: a new reason, the light wipeout: `Dash_Wipeout_Light` (blend in 0.05 s, out 0.15 s), then skating; no knockdown, no bus event.
    Reasons 0 to 3 are unchanged.
- Test figures, clay, 8 m/s, `PlayDash(96)` on tick 40:

| Figure | Takes played |
|---|---|
| K1 (no puck) | `Dash_Start` 40-48, `Dash_Loop` 49-135, `Dash_End` 136-146, `Stride` from frame 34 |
| K2 (`SetHasPuck(true)` on tick 20) | `Dash_Start`, **`Dash_Carry` 49-135**, `Dash_End`, `Stride` 34: the same ticks as K1 |
| K3 (`EndDash(4)` on tick 80) | `Dash_Loop` 49-79, **`Dash_Wipeout_Light` 80-93** then blending out to skating; head lowest 70 cm |
| K4 (`EndDash(0)` on tick 80, control) | `Dash_Loop`, `Dash_End` 80-90, `Stride` 34 |

  - K2 against K1 over the 87 carry frames: legs, hips, spine and head **0.01 cm apart at most**; the stick socket is 36 to 80 cm high in the carry
    against 24 to 36 cm in the loop (hands higher on the stick).
  - K3: the largest one-frame joint step is **31.1 cm on tick 81** (the frame after the cut), against 19.2 cm for the control's `Dash_End`. To be seen.
- **Seed A again** with the wiring (headless, to sim frame 7059): every sim and bus count identical to the previous run of the same seed (112 hits,
  66 passes, 6 checks, 4 misses, 4 goals on the same frames 784, 2349, 5153, 6890, ...), and the figure lines differ only by order within a frame and
  the editor's frame counter; 0 Blueprint errors. **No dash happens in this cartridge**, so the carry and the light wipeout were not exercised in a match.

Still: none (headless). Film: shots 11 and 12.

### E4. The skid's sway and the quick get-ups (job 4)

**Skid's hold** (brake held 67 frames, test figures; before SK30 the hold was a paused frame by construction, not re-measured):

| | Clay | Night |
|---|---|---|
| take frames through the hold | 6.5 to 9.5 and back (`SkidSway` 1.5) | 7.6 to 9.5 (the recording began after the first swing) |
| any two hold frames, joint farthest apart | hand_r 3.3 to 4.3 cm, head 1.1, pelvis 0.3, skates 0.1 to 1.0 | hand_r 3.7 to 4.6, head 1.2, pelvis 0.4 |
| largest one-frame joint step in the hold | 0.82 / 1.06 cm | 0.93 / 1.14 cm |

**Quick and snap get-ups from the front and the sides**, head's largest rise in one frame as played (85 frames down; snap 65). The previous run tried the
blend at 0, 2 and 3 frames against the 5 kept since SK29:

| Blend | Quick_Front | Quick_Side_Left | Quick_Side_Right | Snap_Front | Snap_Side_Left |
|---|---|---|---|---|---|
| 5 frames (kept) | **14.8** / 13.0 cm | 8.8 / 9.8 | 8.9 / 9.9 | 9.5 / 10.2 | 7.4 / 6.6 |
| none | 13.7 / 12.1, **a 13.2 cm pop** on the first frame | 10.1 / 9.1, 27.2 cm pop | 10.2 / 9.2, 26.8 pop | 9.5 / 10.2, 10.4 pop | 6.9 / 6.3 |
| 2 frames | 16.7 / 14.7 | 13.5 / 12.0 | 13.6 / 12.0 | 9.5 / 10.2 | 8.4 / 7.6 |
| 3 frames | 18.7 / 16.5 | 11.8 / 10.6 | 11.9 / 10.6 | 9.5 / 10.2 | 8.7 / 7.7 |

(clay / night; "pop" = the largest joint step on the cut frame). No variant brings the front one under 10 cm without a pop: the 14.8 cm is the take's own
rise, not the blend's. `UpBlendQ` stays 0.083 s, the behaviour SK29 left. The fix belongs to the figure lab (re-time the take as it did the back ones).

## 3. Not verified

- **Nothing was seen on a GPU.** Every number is measured from bone positions in headless play sessions; there is no still.
- **Night `Mallet_Miss`**: not recorded (the night recording ended at tick 114). Clay `Mallet_Miss`: 25 of its frames only.
- **The mallet's carry pose in a dash**: I measured that the prop is hidden, not whether the animation Blueprint's carry layer (`bCarry2`) still bends
  the arms during the dash.
- **`Dash_Carry` and the light wipeout in a match**: seed A has no dash. Whether the game calls `SetHasPuck` today, and how often, is not known.
- **`SkidSway` 3.0**: the test figure set to 3.0 played the same frames as the default, so only the default 1.5 is measured.
- **"Play from one frame earlier"** for the quick get-ups was not tried apart from the blend; SK29's half-frame start rule already applies to them.
- **The miss rule's three other misses in match A** are misses by the sim's account (no check event within seconds); whether they look right is not seen.
- **Night level**: the night figure was tested on the clay level; no night match.

## 4. For the engine lane (the takes as the figures play them now, 60 fps)

| Take | Frames | Starts on | Ends on | Trigger | Must never |
|---|---|---|---|---|---|
| stick lock layer (no take) | `Frames` (0 = 20) | any pose | the same pose | `SetStickLock(true, Frames)`; `false` ends it in 4 frames | play while down, getting up, or under a take of priority 5 or more; move legs or trunk |
| take grace layer (no take) | `Frames` (0 = 30) | any pose | the same pose | `SetTakeGrace(true, Frames)`; `false` ends it in 8 frames | as above; lock the figure |
| `Stumble_Back` as the missed check | 22 (from its frame 2) | `Check_Deliver_*` frame 6 | skating | `PlayCheckMiss()`, or `PlayCheck` with no `check` bus event within 0.108 s | play while down or getting up |
| `Pin_Roll` | 31 (contact 16) | `Pin_Carry` | `Pin_Carry` | `PlayWeapon(4, ..., true)` | the pin above the shoulder (3.8 cm under at the closest) |
| `Fall_Legs_Back` (pin's victim) | 19 to its hold | standing | lying on the back, `Lie_Loop` | `PlayWeaponHit(4, ...)` on the attacker's frame 15 | fall toward the attacker |
| `Mallet_Swing` / `Mallet_Miss` | 37 (contact 17) / 50 | `Mallet_Carry` | `Mallet_Carry` | `PlayWeapon(2, ..., true/false)` | the drum above the shoulder (0.6 cm under at the closest, clay) |
| `Dash_Carry` | 24 a cycle (25 keys) | `Dash_Start` last / a bump's last | `Dash_End` frame 0 | the dash loop starting while `SetHasPuck(true)` | change the legs of `Dash_Loop` (0.01 cm) |
| `Dash_Wipeout_Light` | 22 | `Dash_Loop` (any frame, 3-frame blend) | skating (9-frame blend out) | `EndDash(4, ...)` | a knockdown or a bus event |
| `Dash_Skid` / `_Left` hold | held while braking, up to 2 s | frame 8 | frame 8 | `EndDash(1, ...)` with `SetBraking(true)` | freeze; step more than about 1 cm a frame |

## 5. For the other device (the three calls asked in A-to-B-0026 point 6, and two more)

| Call (on `BP_SkaterVisual_TB1`, `_TB1_P3`) | Inputs | What the figure does | Notes |
|---|---|---|---|
| `SetStickLock(bool bOn, int32 Frames)` | `bOn` true: start; `Frames` 1 to any, 0 = 20 | stick shaken out (3 deg, dies in about 5 frames), blade +11 cm off the ice, back down at the end; a layer, skating goes on | `bOn` false ends it in 4 frames; nothing while down, getting up or under a higher take |
| `SetTakeGrace(bool bOn, int32 Frames)` | as above, 0 = 30 | shoulders turned about 15 deg over the stick side, stick drawn in about 7 cm; a layer | `bOn` false ends it in 8 frames (set in the code; not measured) |
| `PlayCheckMiss()` | none | the attacker's overbalanced step (22 frames) instead of the clean return | also fired by the figure itself when a `PlayCheck` gets no `check` bus event for the same skater within 0.108 s (6.5 frames). **Please call `PlayCheckMiss()` yourself when the sim knows the check missed**: the figure's rule fired on 5 of 12 checks, once on a check that landed 13 frames late |
| `EndDash(4, DirX, DirZ)` | reason 4 | the light wipeout (`Dash_Wipeout_Light`), no fall | new; reasons 0 to 3 unchanged |
| `SetHasPuck(bool)` | as today | the dash loop plays `Dash_Carry` when it starts with the puck | read only at the loop's start; a puck gained mid-loop shows on the next loop start |

`PlayWeaponHit(4, ...)` for the pin should come on the attacker's frame 15 (measured as above). With `Fall_Legs_Back` the victim no longer needs a push.

## 6. Decisions for the operator

1. **The missed check**: 5 of 12 checks in two matches end in the overbalanced step. Keep it this often, have the game decide each miss, or widen the
   figure's window from 6 to 15 frames (which would have saved the one false miss seen).
2. **The mallet's swing passes 0.6 cm under the shoulder on clay** (frame 14). It passes the rule; on screen it may read as shoulder height. Film shot 7
   decides; if it reads high, ask the weapons lab for a 5 cm margin.
3. **The quick get-up from the front** still rises 14.8 cm in a frame: the take's own. Ask the figure lab to re-time it (and the side ones) as it did the
   back ones.

## 7. The next three evolutions

1. **The miss decided by the sim** (or a 15-frame window). Proving number: in two seeded matches no `check` bus event follows a miss within 20 frames
   (0 false misses), with the miss rate printed per match.
2. **The light wipeout's catch**: blend in 0.1 s instead of 0.05 and check the take by itself. Proving number: the largest joint step on the frames after
   the cut at or under the control `Dash_End`'s 19.2 cm.
3. **A dash with the puck in a match**: a cartridge with dashes and boosts, seeded. Proving numbers: how often `Dash_Carry` plays against `Dash_Loop`,
   the stick blade on the ice (under 2 cm) on every carry frame, 0 Blueprint errors.

## Editors

- pid 27948 (the previous agent's, headless, port 8004): attached at 10:07, no dirty package, left through `ExitEditor` at 10:17:50 (`LogExit: Exiting.`),
  process gone.
- pid 33580 (mine, `tools\run\editor.ps1 -ExtraArgs '-nullrhi -unattended'`, port 8004, tree `b-fig`): 10:18:34 to 10:26:23, left through
  `ExitEditor`, process gone, no dirty package at exit. Game sound turned off in memory for every play session and back on before each exit.
- The other editor running on the machine (pid 31700, `wt\b-aud`, port 8006) was not touched.

My scripts and data: `C:\Users\kazda\jam\sk30-lab\` (`imp_dash.py` and its result `imp_dash.out.json`, `wire.mjs`, `hide.mjs`, `hide2.mjs`, the
Blueprints' DSL before and after each change, `an\a15b.py`, `an\dash.py`, `an\md.py`). Test data: `...\scratchpad\sk25\logs\w6_d30c.json`,
`w6_md30c2.json`, `w6_md30n2.json`, `full_s30w.txt`.

## Ledger rows

22 full rows in the ledger's 11 columns: 2 figure Blueprints, 2 animation Blueprints, 14 weapon takes (replace their rows), 4 new dash takes.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 10.10 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event (lane SK22); StopTick plays MS_brake (lane AU11); fall sizes, snap get-up, GetUp_Wall_Back, wall shift, PlayStumble and SetBraking by name, late get-ups' test switch (lane SK23); + BrakeFx Niagara component (NS_brake) (lane B40); the pawn's PlayStumble covers every stumble, the level-driven gated stop, lying loops from the fall's end pose (lane SK24); whip weights, check sizes, the eleven set calls, WeaponMesh on stick_socket, test trigger SK26Test (lane SK26); the timed get-up on its frame, UpBlend (lane SK29); SetStickLock and SetTakeGrace (layers through the ABP's LockRot/GraceRot, not under a take of priority 5 or more), PlayCheckMiss and the miss rule (no check bus event within 0.108 s of PlayCheck), the pin's victim Fall_Legs_Back, the skid's hold sway (SkidSway 1.5), UpBlendQ, test calls Tune and TestBusCheck (lane SK30, previous run); SetHasPuck stored in bPuck and Dash_Carry at the dash loop's start with the puck, EndDash reason 4 plays Dash_Wipeout_Light, the carried prop hidden and the stick shown from the dash's first frame (lane SK30) (10589640 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 10.30 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12 to SK19, QA3); + SlideFx (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m while down (lane SK16); SK2Tick (lane QA4); stumble, board falls, quick get-ups, hard stop, deke (lane SK21); the gated hard stop (lane SK22); MS_brake (lane AU11); lane SK23's calls; + BrakeFx (NS_brake) (lane B40); lane SK24's stop and lying loops; lane SK26's calls, WeaponMesh and test trigger; the timed get-up's start and blend (lane SK29); the three calls, the miss rule, the pin's victim, the skid's sway, UpBlendQ and the test calls as BP_SkaterVisual_TB1 (lane SK30, previous run); Dash_Carry with the puck, EndDash reason 4 (Dash_Wipeout_Light), the prop hidden from the dash's first frame, as BP_SkaterVisual_TB1 (lane SK30) (10798701 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 0.88 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4); +the carry layer on the aim layer's input: four bool switches (Giant_Carry, Mallet_Carry, Cannon_Carry, Boost_Take), mesh space from spine_01 (lane SK26); the carry layer's fifth switch: bCarry5 with Pin_Carry, under the boost beat (lane SK28); two Modify Bone nodes at the end of the whip chain: spine_01 (GraceRot, alpha GraceA) and stick_socket (LockRot, alpha StickA), driven by the figure's SetTakeGrace and SetStickLock layers (lane SK30) (923107 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.88 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3); +the carry layer's four switches as ABP_SkaterTB1 (lane SK26); the carry layer's fifth switch (bCarry5, Pin_Carry) as ABP_SkaterTB1 (lane SK28); the trunk and stick Modify Bone nodes (GraceRot/GraceA, LockRot/StickA) as ABP_SkaterTB1 (lane SK30) (918959 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Giant_Sweep.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export (gate-passed), 23 keys (lane SK30) (138117 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export: carried low, the drum at least 34.9 cm under the shoulder (lane SK30) (109431 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Swing.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export: swung flat from behind the hip, 37 keys, contact 17, the drum at least 3.3 cm under the shoulder on the night figure (lane SK30) (163895 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Mallet_Miss.uasset | anim | 0.19 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export, 50 keys (lane SK30) (195813 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Carry by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28); re-keyed by name from the fourth export, 31 keys (lane SK30) (109370 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Roll.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Roll by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28); re-keyed by name from the fourth export: a low push, 31 keys, contact 16, the pin at least 9.8 cm under the shoulder (lane SK30) (157960 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Pin_Miss.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p2_cel, take Pin_Miss by name, a new asset on SKEL_SkaterTB1v2 (re-import by name only) (lane SK28); re-keyed by name from the fourth export, 41 keys (lane SK30) (173648 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Giant_Sweep.uasset | anim | 0.13 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Giant_Sweep by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export (gate-passed), 23 keys (lane SK30) (141098 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export: carried low, the drum at least 34.9 cm under the shoulder (lane SK30) (109468 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Swing.uasset | anim | 0.16 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Swing by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export: swung flat from behind the hip, 37 keys, contact 17, the drum at least 0.6 cm under the shoulder (frame 14) (lane SK30) (162748 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Mallet_Miss.uasset | anim | 0.19 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Mallet_Miss by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK26); re-keyed from the final export by name (lane SK28); re-keyed by name from the fourth export, 50 keys (lane SK30) (194250 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Carry.uasset | anim | 0.10 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Carry by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28); re-keyed by name from the fourth export, 31 keys (lane SK30) (109404 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Roll.uasset | anim | 0.15 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Roll by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28); re-keyed by name from the fourth export: a low push, 31 keys, contact 16, the pin at least 3.8 cm under the shoulder (lane SK30) (157994 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pin_Miss.uasset | anim | 0.17 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | sets-lab/weapons/export/p3_clay, take Pin_Miss by name, a new asset on SKEL_SkaterTB1_P3 (re-import by name only) (lane SK28); re-keyed by name from the fourth export, 41 keys (lane SK30) (173674 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Carry.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/dash/export/p2_cel (closed, gate PASS), take Dash_Carry by name (the file with the repeated last frame), a new asset on SKEL_SkaterTB1v2, 25 keys, 0.4 s; played at the dash loop's start when SetHasPuck is on (lane SK30) (146190 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Dash_Wipeout_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/dash/export/p2_cel (closed, gate PASS), take Dash_Wipeout_Light by name, a new asset on SKEL_SkaterTB1v2, 23 keys, 0.367 s; played on EndDash reason 4 (lane SK30) (144126 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Carry.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/dash/export/p3_clay (closed, gate PASS), take Dash_Carry by name (the file with the repeated last frame), a new asset on SKEL_SkaterTB1_P3, 25 keys, 0.4 s; played at the dash loop's start when SetHasPuck is on (lane SK30) (146216 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Dash_Wipeout_Light.uasset | anim | 0.14 | original | Blender 4.2 Python script (set lanes) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/dash/export/p3_clay (closed, gate PASS), take Dash_Wipeout_Light by name, a new asset on SKEL_SkaterTB1_P3, 23 keys, 0.367 s; played on EndDash reason 4 (lane SK30) (145624 bytes) |
