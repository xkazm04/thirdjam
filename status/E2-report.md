# Lane E2 report, first hand-back (jobs 1 to 4): the figure's new calls, built and proved headless

Device B, tree `C:\Users\kazda\jam\wt\b-fig` (branch `lane/b-fig`, at dev-b 68bf1cb1, rebuilt: PASS 323.9 s). Headless editor only (port 8004).
No git and no cook. Assets were changed only under `Game/Plugins/JamAssets/Content/Skaters/`: 4 changed, 12 new. **Nothing was seen on a GPU**: the film script is `status\E2-film.md`.

## 1. Done / not done

| Job | Verdict | The number that decides it |
|---|---|---|
| 1. `SetCrossCheckHold(bool bOn)` | **Done**, both figures | hold weight 0.17 a frame: 1.0 six frames after the call, 0 six frames after the off call. Legs 0.00 cm from a control figure through the hold, clay and night. `PlayCrossCheck` from the hold plays `CrossCheck_Release` |
| 2. `SetBoostEquipped(FName Kind, bool bOn)` | **Done** with stand-in props | all 4 kinds shown on and gone off; an unknown kind is ignored. Through a 110-frame knockdown the keg stays 9.05 to 9.16 cm from `spine_03` |
| 3. `PlayCelebrate` | **Done**, with an **int32** input `Which` (0 to 2 scorer, 3 mate) | 4 takes by name, 58 frames each; skipped while down. In a seeded match the game called it 4 times (2 goals × scorer and mate) |
| 4. Proof and film script | **Done** | two seeded clay matches: sim lines and all 12 `JAMSIM` hashes identical, 0 Blueprint errors. Test sessions: 0 errors. Film script written |
| 5. Imports, wind-ups behind a switch | **Not started** (second hand-back) | the boosts lane's export folder exists, but has no `READY.txt` yet (3 `SM_Equip_*_p3_clay.fbx` seen); `export36` does not exist |

## 2. Per item

### Job 1: the held cross-check
- **What changed:**
  - **The animation Blueprints** (`ABP_SkaterTB1`, `ABP_SkaterTB1_P3`):
    - a new Layered blend per bone, `AnimGraphNode_LayeredBoneBlend_0`, after the carry and aim layer (`LayeredBoneBlend_1`) and before `LocalToComponentSpace_0`. Its layer setup is copied from the carry layer: from `spine_01` up, rotation blended in mesh space.
    - its pose comes from `AnimGraphNode_SequencePlayer_9`, which loops `CrossCheck_Hold`.
    - its weight comes from a new float variable `HoldW`.
    - The slot is after this layer, so any take (a fall, a get-up, a check) covers the hold.
  - **New stand-in takes** (both styles):
    - `CrossCheck_Hold`: `Lift`'s contact frame 2, held (2 keys).
    - `CrossCheck_Release`: `Lift` from frame 2 to the end (23 keys, 0.367 s).
    - A re-import by name re-keys them (the import pattern lanes SK26 to SK30 used), so the Blender lane's takes replace them without any wiring.
  - **The figure Blueprints** (`BP_SkaterVisual_TB1`, `_P3`), new events:
    - `SetCrossCheckHold(bOn)` ramps `HoldW` through `HoldTick`, run from the tick while `bHoldRun` is set: 6 frames in, 6 frames out. The weight is forced to 0 while down or getting up.
    - `PlayCrossCheck(Frames)` (int) logs. If the skater is held, or the hold ended under 0.1 s ago, it calls `CrossRelease(1)`.
    - `CrossRelease` ends the hold in 3 frames and plays `CrossCheck_Release` (blend 0.05 / 0.15 s, priority 6). It sets `LiftPlayT`, so a `PlayLift` on the same frame is skipped.
    - **The brace's hit:** a `check` bus event for this skater while held (or within 0.1 s of the hold ending) now plays the release instead of the body check's take (`OnBusEvent`, flag `bRelNow`). With no hold, the old path runs unchanged.
- **Proving numbers** (test figures, headless; clay / night):
  - H1 skating 6 m/s, hold from tick 30 to 150:
    - weight 0.167, 0.333 … 1.0 at tick 36; 0 at tick 156
    - legs (feet, calves, thighs, pelvis, root) **0.00 cm** from the control H0 over ticks 42 to 148
    - arms and stick up to 91.9 / 100.2 cm from H0 (the pose); back to 0.00 after tick 170
    - held pose: hands 94 to 96 cm high, about 60 cm ahead of the hips, 46 cm apart; the blade 110 cm high
    - largest one-frame arm step in the ramps: 19.7 / 21.6 cm (H0 2.2 / 2.5)
  - H5 at 9 m/s: the same ramps; the feet keep striding (6.7 / 7.2 cm a frame).
  - H2 standing, hold on, then `PlayCrossCheck(0)` + `PlayLift()` on tick 90:
    - `CrossCheck_Release` plays ticks 90 to 104, and no Lift take
    - hold weight 0.67, 0.33, 0 over ticks 91 to 93
    - `hand_r` drops 13 cm (clay) / 14 cm (night) on tick 90
  - H3, `PlayCrossCheck(0)` + `PlayLift()` with no hold: today's `Lift_Light` (clay) / `Lift` (night). Unchanged.
  - H4, a knockdown during the hold: weight 0 from tick 61. The fall, the lie and the get-up play as today.
- **Still:** none (headless). I looked at the per-frame bone recordings (`logs\w6_cT.json`, `w6_cT2.json`, `w6_nT.json`, through `ana.py`) and the log lines, not at pictures.

### Job 2: the equipped props (stand-ins: my choice)
- **What changed:** four hidden, no-collision StaticMeshComponents on both figures, placed on the Glide pose and attached (KeepRelative) to their bone when shown:

| Kind | Component | Stand-in | Rides on | Where (cel) |
|---|---|---|---|---|
| `shotgun` | `BoostShotgun` | the puck cannon's keg (`SM_Prop_PuckCannon_*`), 0.75 | `spine_03` | on the back, centre 30 cm (clay 37) behind the joint, tilted 20 deg |
| `hip_check` | `BoostHip` | the mallet (`SM_Prop_Mallet_*`), 0.55 | `pelvis` | hanging at the right hip, 34 cm out |
| `shield` | `BoostShield` | a round timber disc (`/Engine/BasicShapes/Cylinder` 46 × 6 cm, `MI_Prop_ClayTimber`) | `spine_03` | on the back |
| `slow_puck` | `BoostSlow` | the rolling pin (`SM_Prop_RollingPin_*`), 0.6 | `spine_01` | across the lower back |

  - `SetBoostEquipped(Kind, bOn)` shows or hides that kind's prop (helper event `EquipShow`).
  - Off hides only that kind; an unknown kind logs and does nothing. A knockdown keeps the prop attached.
- **Proving numbers:**
  - Shown and hidden on the right ticks: E1 20 to 250. E5: slow on 20, off 100, shotgun on 120; `hip_check` off when not shown changes nothing; `bogus` is ignored.
  - E1 knocked down with the keg on: keg to `spine_03` 9.05 to 9.16 cm on every frame (it rides the body).
  - Clay, standing, centres: keg (-12.7, 0, 101.4) against `spine_03` (9, 0, 106), measured before I moved it 8 cm further back.
- **Not verified:** whether a prop cuts into the skin. I placed them against joints with the skin depths a figure-lab pass measured on the chest (11.6 cel / 20.3 clay); only a GPU still decides (film shot 7). Still: none.

### Job 3: the celebrations
- **What changed:**
  - **Inputs:** `PlayCelebrate(int32 Which)`. A's `CallPlayCelebrate` accepts one input of name, string or number. I chose the number because it alone tells the mate apart: A sends the scorer 0 to 2 by the goal count, and the mate 3.
  - **Takes:** 0 `Celebrate_ArmsUp`, 1 `_StickRaise`, 2 `_KneeGlide`, 3 `_Mate`; anything else plays ArmsUp. They are new stand-ins: copies of `Celebrate`, 73 keys, 1.2 s each. Blend 0.15 / 0.25 s, priority 6.
  - **While down or getting up:** skipped, with a log line.
- **Proving numbers:**
  - C0 to C3: each take by name on ticks 30 to 88. C4, down: skipped.
  - In the seeded match: `JAMFIG celebrate frame=1095 … scorer=5:stick_raise:PlayCelebrate mate=7:knee_glide:PlayCelebrate`; the figure logged `celebrate 1, 2, 3, 3`.

### Job 4: the proof
- **Two seeded clay matches**, both run after the change (cartridge `clayA`, seed 1, scripted human, headless, to sim frame 7156):
  - sim knockdown, stumble and check lines: 26 = 26, **identical**
  - all 12 `JAMSIM` hashes identical (frame 7200 `09e05efd4cd8c0c6`)
  - **0 Blueprint errors** in each; 4 `PlayCheck`, 7 check bus events, 0 misses
- **The new calls in a normal match:**
  - `PlayCrossCheck` 10 times: a log line only, since nobody holds
  - `PlayCelebrate` 4 times
  - `SetCrossCheckHold` 0 times: no input reaches `bCrossHold` on this tree, and the bots ship at -1
  - `SetBoostEquipped` 0 times
- **"Equal to before":** these lines are NOT equal to FILM3's match on the same seed (first difference at sim 1427 against 1503). The reason is A's code merged since then (A184 to A191: speed, controls, contact). The figure feeds nothing into the sim (CallFigure's return is only logged). The two runs prove the sim is deterministic with the new figure. There is no pre-change run on this tree; making one would have needed git.
- **Test sessions** (clay twice, night once): 0 Blueprint errors, 0 `Error:` lines.
- **Film script:** `C:\Users\kazda\jam\wt\b-fig\status\E2-film.md` (7 shots).

## 3. Not verified
- **The GPU:** nothing seen, no still.
  - The stand-in hold pose's look: the blade is 110 cm up; it may read as a raised stick.
  - The props against the skin.
  - The release's 13 cm drop on its first frame.
- **The brace's hit** (bus `check` while held, leading to the release): not exercised. Test figures have no sim index, and no brace happens in a match yet.
- **`PlayCrossCheck` with Frames > 0** (a wind-up): it logs the frames and does nothing more, by design until job 5.
- **The night figure on the night level:** it was tested on clay.

## 4. For device A (exact names; all on `BP_SkaterVisual_TB1` and `_TB1_P3`, by name)

| Call | Inputs | What the figure does |
|---|---|---|
| `SetCrossCheckHold(bool bOn)` | level | the braced upper body over 6 frames in, 6 out; legs untouched; nothing visible under a take, while down or getting up |
| `PlayCrossCheck(int32 Frames)` | frames to contact (today 0) | from the hold (or within 0.1 s of its end): `CrossCheck_Release` (22 frames) and the same frame's `PlayLift` is skipped; otherwise a log line only and `PlayLift` plays as today |
| `SetBoostEquipped(FName Kind, bool bOn)` | `shotgun`, `hip_check`, `shield`, `slow_puck` | that kind's prop on or off; it stays on through falls |
| `PlayCelebrate(int32 Which)` | 0 to 2 the scorer's three, 3 the mate | a 1.2 s take (stand-in); skipped while down |
| test moves | `ke * SK26Test 30` hold on, `31` off, `32` PlayCrossCheck, `33` to `36` celebrate 0 to 3, `37` to `40` equip the four kinds, `41` all off | on the human's figure |

- **A brace that lands:** today it arrives as a `check` bus event, and the figure plays the release from it. If A calls `PlayCrossCheck` on that frame as well, the figure plays it once (it skips the second call).
- **The wind-ups:** I have no frames ask yet. When A answers, job 5 adds the wind-up takes behind a switch.

## 5. Decisions for the operator
1. **The held pose's stand-in is `Lift`'s shove frame**, with the arms out (hands 60 cm forward) and the blade 110 cm up. Ship it until the proper `CrossCheck_Hold` arrives, or wait for the Blender lane's take?
2. **The equipped stand-ins:** keg on the back (shotgun), mallet at the hip (hip_check), timber disc on the back (shield), rolling pin across the lower back (slow_puck). Fine as placeholders until the props lane's equipped props land?
3. **A goal now plays the stand-in `Celebrate` on two figures** in every match, since A's code already calls it. Keep it, or hold `PlayCelebrate` back until the real takes arrive?

## 6. The next three steps
1. Job 5: import `export36` (the four `CrossCheck_*` and four `Celebrate_*` takes) and the boosts lane's equipped props by name, once their `READY.txt` files exist. Wire `_Hold_In` and `_Hold_Out` in place of the weight ramp. Proving numbers: keys equal to the export, the hold's legs still 0.00 cm, 0 errors.
2. Put the wind-up takes for `PlayCheck` and `PlayCrossCheck` behind a switch (`bWindupTakes`, default off), timed to A's frames. Proving number: the take's contact frame lands on A's contact frame (0 frames off).
3. Measure the brace's hit in a match once the controls lane lands (`JAM_PLAYTESTER_BRACE=always`). Proving numbers: every `E2 cross release why 2` has a `check` bus event on the same frame, and 0 body-check takes from a brace.

## Editors and tools
- **Mine:** pid 27948, headless, port 8004, started 14:22:35. It is **still running** for job 5; the map `/Game/Maps/Rink` is dirty from spawning test actors and must not be saved.
  - Game sound off in memory and `Slate.bAllowThrottling` 0; both go back on before I exit.
  - Not touched: UnrealEditor pid 32208 (started 14:22:59, not mine) and Blender pid 27228.
- **Lab:** `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\e2\`
  - scripts: `assets.py`, `abph.mjs`, `abpset.py`, `equip.py`, `e2bp.mjs`, `gen.py`, `w3.py`, `ana.py`
  - the DSL before and after: `bk\`
  - logs: `logs\full_m1.txt`, `full_m2.txt`, `w6_*.json`

## Ledger rows (16: 4 changed, 12 new)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 11.08 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3); SK2Tick pushes the aim values once per call (lane QA4); the stumble's own take told by the check bus event (PlayStumbleNow), board falls on four sides with wall get-ups when PlayBoardHit comes with a knockdown, quick get-ups under 80 frames, the hard stop derived from the speed, the deke on its bus event (lane SK21); the hard stop gated to an event (lane SK22); StopTick plays MS_brake (lane AU11); fall sizes, snap get-up, GetUp_Wall_Back, wall shift, PlayStumble and SetBraking by name, late get-ups' test switch (lane SK23); + BrakeFx Niagara component (NS_brake) (lane B40); the pawn's PlayStumble covers every stumble, the level-driven gated stop, lying loops from the fall's end pose (lane SK24); whip weights, check sizes, the eleven set calls, WeaponMesh on stick_socket, test trigger SK26Test (lane SK26); the timed get-up on its frame, UpBlend (lane SK29); SetStickLock and SetTakeGrace (layers through the ABP's LockRot/GraceRot, not under a take of priority 5 or more), PlayCheckMiss and the miss rule (no check bus event within 0.108 s of PlayCheck), the pin's victim Fall_Legs_Back, the skid's hold sway (SkidSway 1.5), UpBlendQ, test calls Tune and TestBusCheck (lane SK30, previous run); SetHasPuck stored in bPuck and Dash_Carry at the dash loop's start with the puck, EndDash reason 4 plays Dash_Wipeout_Light, the carried prop hidden and the stick shown from the dash's first frame (lane SK30); the miss rule behind bAutoCheckMiss (bool, default false, Instance Editable): PlayCheck arms it only when true, PlayCheckMiss unchanged (lane FILM3); SetCrossCheckHold(bool) ramps the ABP's HoldW over 6 frames (HoldTick), PlayCrossCheck(int) and a check bus event while held play CrossCheck_Release (CrossRelease), SetBoostEquipped(Name, bool) shows four stand-in prop components BoostShotgun/BoostHip/BoostShield/BoostSlow (keg, mallet, timber disc, rolling pin) attached to spine_03/pelvis/spine_03/spine_01, PlayCelebrate(int 0-3) plays Celebrate_ArmsUp/_StickRaise/_KneeGlide/_Mate, test moves 30-41 (lane E2) (11075011 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 11.28 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12 to SK19, QA3); + SlideFx (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m while down (lane SK16); SK2Tick (lane QA4); stumble, board falls, quick get-ups, hard stop, deke (lane SK21); the gated hard stop (lane SK22); MS_brake (lane AU11); lane SK23's calls; + BrakeFx (NS_brake) (lane B40); lane SK24's stop and lying loops; lane SK26's calls, WeaponMesh and test trigger; the timed get-up's start and blend (lane SK29); the three calls, the miss rule, the pin's victim, the skid's sway, UpBlendQ and the test calls as BP_SkaterVisual_TB1 (lane SK30, previous run); Dash_Carry with the puck, EndDash reason 4 (Dash_Wipeout_Light), the prop hidden from the dash's first frame, as BP_SkaterVisual_TB1 (lane SK30); the miss rule behind bAutoCheckMiss (default false, Instance Editable) as BP_SkaterVisual_TB1 (lane FILM3); SetCrossCheckHold, PlayCrossCheck and the brace's release, SetBoostEquipped with four stand-in prop components (placed for the clay body: keg 37 cm, disc 23 cm, pin 27 cm behind their joints), PlayCelebrate(int 0-3) and test moves 30-41 as BP_SkaterVisual_TB1, on the P3 takes and ABP_SkaterTB1_P3 (lane E2) (11283943 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1.uasset | anim | 0.95 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | AnimBP: idle, skate blend, lean, falls, get-up, DefaultSlot (lane B4); +the carry layer on the aim layer's input: four bool switches (Giant_Carry, Mallet_Carry, Cannon_Carry, Boost_Take), mesh space from spine_01 (lane SK26); the carry layer's fifth switch: bCarry5 with Pin_Carry, under the boost beat (lane SK28); two Modify Bone nodes at the end of the whip chain: spine_01 (GraceRot, alpha GraceA) and stick_socket (LockRot, alpha StickA), driven by the figure's SetTakeGrace and SetStickLock layers (lane SK30); the held cross-check's layer: a Layered blend per bone (spine_01 up, mesh-space rotation, as the carry layer's) after LayeredBoneBlend_1, pose a looping Sequence Player on CrossCheck_Hold, weight HoldW (float) set by the figure (lane E2) (952835 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.95 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3); +the carry layer's four switches as ABP_SkaterTB1 (lane SK26); the carry layer's fifth switch (bCarry5, Pin_Carry) as ABP_SkaterTB1 (lane SK28); the trunk and stick Modify Bone nodes (GraceRot/GraceA, LockRot/StickA) as ABP_SkaterTB1 (lane SK30); the held cross-check's layer (HoldW, CrossCheck_Hold of the P3 takes) as ABP_SkaterTB1 (lane E2) (948687 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Hold.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's CrossCheck_Hold: AS_SkaterTB1v2_Lift's contact frame 2 held, 2 keys, on SKEL_SkaterTB1v2 (re-import by name re-keys it); the ABP's hold layer plays it from spine_01 up (lane E2) (118393 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_CrossCheck_Release.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's CrossCheck_Release: AS_SkaterTB1v2_Lift from frame 2 to its end, 23 keys, 0.367 s (re-import by name re-keys it); played by CrossRelease from the hold (lane E2) (149323 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate_ArmsUp.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's Celebrate_ArmsUp: a copy of AS_SkaterTB1v2_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(0) (lane E2) (200084 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate_StickRaise.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's Celebrate_StickRaise: a copy of AS_SkaterTB1v2_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(1) (lane E2) (200104 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate_KneeGlide.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's Celebrate_KneeGlide: a copy of AS_SkaterTB1v2_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(2) (lane E2) (200099 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Celebrate_Mate.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | stand-in until the figure lab's Celebrate_Mate: a copy of AS_SkaterTB1v2_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(3) (lane E2) (200074 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Hold.uasset | anim | 0.13 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's CrossCheck_Hold: AS_SkaterTB1_P3_Lift's contact frame 2 held, 2 keys, on SKEL_SkaterTB1_P3 (re-import by name re-keys it); the ABP's hold layer plays it from spine_01 up (lane E2) (127679 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_CrossCheck_Release.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's CrossCheck_Release: AS_SkaterTB1_P3_Lift from frame 2 to its end, 23 keys, 0.367 s (re-import by name re-keys it); played by CrossRelease from the hold (lane E2) (158609 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Celebrate_ArmsUp.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's Celebrate_ArmsUp: a copy of AS_SkaterTB1_P3_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(0) (lane E2) (216385 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Celebrate_StickRaise.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's Celebrate_StickRaise: a copy of AS_SkaterTB1_P3_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(1) (lane E2) (216405 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Celebrate_KneeGlide.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's Celebrate_KneeGlide: a copy of AS_SkaterTB1_P3_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(2) (lane E2) (216400 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Celebrate_Mate.uasset | anim | 0.22 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | stand-in until the figure lab's Celebrate_Mate: a copy of AS_SkaterTB1_P3_Celebrate, 73 keys, 1.2 s (re-import by name re-keys it); PlayCelebrate(3) (lane E2) (216375 bytes) |

---

# Second hand-back (job 5, parts 1 and 2; 15:05): the boosts lane's pickups and equipped set imported and wired

Same tree and editor (pid 27948, headless, port 8004); on top of the coordinator's commit 89620a6d. There are 48 assets: 46 new and 2 changed (the figure Blueprints).
**Part 3 is not started**: `C:\Users\kazda\jam\skater-lab\skf\export36\READY.txt` does not exist (checked at 15:03).

## 1. Done / not done

| Part | Verdict | Proving number |
|---|---|---|
| 1. Six pickups `SM_Boost_<kind>` | **Done** | 6 imported to `/JamAssets/Props/`. Tris 400 / 840 / 692 / 560 / 608 / 1,124, the same as the manifest. Slots `Clay` and `Accent` bound to 12 new instances of `M_PickupProp` (the material `SM_Pickup_*` use). Sizes are as in the manifest (speed 54.7 × 92.5 × 78.9 cm, base at z 0) |
| 2. Equipped set, wired | **Done** | The socket rule reproduces the lab's placement exactly: in Glide, the lowest prop vertex equals the lab's to **0.01 cm** for all 8 (clay: keg 96.45 against the lab's 96.46, bumper 73.19 / 73.19, shield 88.41 / 88.41, crest 146.24 / 146.24; cel 100.15 / 100.15, 69.35 / 69.35, 92.10 / 92.11, 154.76 / 154.76). The prop in the bone frame equals the socket to 0.002 cm on every frame. Hidden from the knockdown's second frame to the end of the get-up |
| 3. `export36` | **Waiting** | there is no `READY.txt` yet |

## 2. Per item

### Pickups
- **What changed:**
  - `SM_Boost_speed`, `_precision`, `_heavy_puck`, `_big_puck`, `_reverse_other`, `_custom`: FBX static mesh, no collision generated, combined.
  - The FBX importer had suffixed five meshes' slots (`Clay_001` …). I renamed them back to `Clay` / `Accent`.
  - **Colours:** `M_PickupProp` takes no colour parameter; it takes a `BaseColorTex`. So each slot gets an 8 × 8 solid texture `T_Boost_<kind>_<Clay|Accent>` (sRGB) in the manifest's sRGB colour, under an instance `MI_Boost_<kind>_<Clay|Accent>`. Each instance has the same scalars as `MI_PickupProp_speed`: Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6.
  - The older `SM_Pickup_*`, `MI_PickupProp_*` and `T_Pickup_*` are left untouched: A's code loads `SM_Boost_*`.
- **Not verified:** the look. Materials don't compile headless, so the colours, emissive and rim are not seen. Film steps are in `E2-film.md`.
- **The film rows are tested headless:** 12 pickups were present in the play session (6 at 25 m, 6 at 45 m), then removed.

### Equipped set
- **What changed:**
  - **Imported:** `SM_Equip_<shotgun|hip_check|shield|slow_puck>_<p3_clay|p2_cel>` under `/JamAssets/Skaters/Props/`, with eight `MI_Equip_<kind>_<Clay|Accent>`. These are copies of `MI_Prop_ClayTimber` (`M_Toon`, as the weapon props use), each `TeamColour` the manifest's colour in linear.
  - **The four components are re-used:** `BoostShotgun`, `BoostHip`, `BoostShield`, `BoostSlow`. Each now holds the props lane's mesh (clay on `_P3`, cel on the night figure) at scale 1 with no material override.
  - **Relative transform (the socket):** pivot × inverse(the bone's reference-pose component transform), computed in the engine from the skeletal mesh's reference pose, with Unreal Y = −Blender Y. It equals the manifest's Unreal column:

    | Kind | Bone | Clay socket | Cel socket | Rotation |
    |---|---|---|---|---|
    | shotgun | spine_03 | (-32.32, 2.47, 0) | (-29.80, 2.76, 0) | roll -90 |
    | hip_check | pelvis | (0, -5, 0) | (0, 3, 0) | roll -90 |
    | shield | spine_03 | (-21.22, 5.47, 0) | (-18.70, 5.76, 0) | roll -90 |
    | slow_puck | **head** | (0.62, -32.51, 0) | (0.44, -34.97, 0) | roll -90 |

  - **Blueprint logic:**
    - `SetBoostEquipped(Kind, bOn)` sets `bEqShotgun` / `bEqHip` / `bEqShield` / `bEqSlow` and `bEqAny`, then `EquipApply`.
    - `EquipApply` shows each prop (attached KeepRelative to its bone) when it is on and the figure is not down or getting up (`bEqHid` = `bDown` or `bUpRun`).
    - `EquipTick` runs from the tick while `bEqAny` is set and re-applies on a change of down / up.
    - Several kinds can be shown at once; an unknown kind logs and does nothing.
- **Proving numbers** (headless test figures `fa_Q`, clay `w6_cQ`, night `w6_nQ`; 0 Blueprint errors):
  - In the bone frame, every prop sits at its socket on all 260 Glide and 260 Stride frames: spread 0.001 to 0.002 cm.
  - **Glide, the lowest vertex** (all 120 frames; the lab every 2nd frame): equal to the lab's to 0.01 cm, all 8 (table above). The engine therefore places the props where the lab measured them, so the lab's clearances hold in Glide:
    - clay keg: 1.21 cm off the nearest body face, 0 frames inside
    - cel keg: 0.58 cm
    - That is closer than the "about 2 cm" asked; the placement is the lab's, not mine.
  - **Stride:** the bumper and the crest match the lab (72.66 / 72.63, 141.66 / 141.65). **The keg and the shield sit 1.9 to 3.1 cm lower than the lab measured** (clay keg 96.80 against 98.66; shield 88.63 against 91.68; cel keg 100.58 against 101.87). The props ride `spine_03` exactly, so the engine's `Stride` take must bend the upper spine differently from export35's. Clearance in the engine's Stride is therefore **not proved**.
  - **Knockdown** (K0 to K3, `Knockdown_Back` from tick 40 to the get-up's end on 148, released on 150):
    - hidden from tick 41 to 150, shown again on 151, all 4 kinds
    - one frame shown at tick 40, the take's first frame (the hide comes from the next tick)
    - so nothing goes under the ice
  - Figure M skating with all four: all shown from tick 12; the shield off at 250 leaves three.
- **Seeded clay match again** (`m3`, after the change): sim lines 26 = 26, the hash list equal to m1 and m2, 0 Blueprint errors. `SetBoostEquipped` was not called (this cartridge has no abilities).
- **What I looked at:** the per-frame bone and prop-transform recordings, with the props' 178 to 526 mesh vertices placed through them (`anaq.py`), plus the lab's `summary_<style>.txt` and `equip_<style>.json`. No picture.

## 3. Not verified
- **Nothing seen on a GPU:** the pickups' colours and emissive; the props against the jersey.
- **Clearance against the skin in the engine:** there is no skinned-vertex access headless. It is proved only by equality with the lab's placement in Glide; Stride differs (above).
- **The knockdown's first frame:** the prop is still shown on it.
- **A's own call in a match:** no boost cartridge was run. `jam.boost.Preview shotgun` would show it on the human.

## 4. For device A
- Pickups: `/JamAssets/Props/SM_Boost_<kind>` for the six kinds. No code change is needed; materials are bound on the mesh.
- `SetBoostEquipped(FName Kind, bool bOn)`: unchanged name and inputs. The props are now the real ones. They hide themselves while the skater is down or getting up, and come back after, still equipped.
- `/JamAssets/Props` still needs to be in the cook (A188 loads by path).

## 5. Decisions for the operator
1. **The keg sits 1.2 cm (clay) / 0.6 cm (cel) off the jersey in Glide**, as the props lab placed it; the coordinator asked for about 2 cm. Move it back 1 cm? (Change one number per style.)
2. The old `SM_Pickup_*` meshes and textures: delete them now that `SM_Boost_*` exist, or keep them?

## 6. The next three steps
1. Part 3 as soon as `export36\READY.txt` exists: the hits' wind-ups behind a switch, the held cross-check, the celebrations.
2. Hide the prop on the knockdown's own frame (in `KnockDown`), and re-check the engine's `Stride` against export35's.
3. The GPU look at the pickups (25 / 45 m) and the equipped set, from the film script.

## Ledger rows, job 5 (48: 46 new, 2 changed; replace the Blueprints' rows above)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Props/SM_Boost_speed.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_speed.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_speed_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_speed (lane E2) (82518 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_speed_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for speed; BaseColorTex of MI_Boost_speed_Clay (M_PickupProp takes no colour parameter) (lane E2) (11363 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_speed_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_speed_Clay; slot Clay of SM_Boost_speed (lane E2) (6380 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_speed_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for speed; BaseColorTex of MI_Boost_speed_Accent (M_PickupProp takes no colour parameter) (lane E2) (11409 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_speed_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_speed_Accent; slot Accent of SM_Boost_speed (lane E2) (6396 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_Boost_precision.uasset | mesh | 0.09 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_precision.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_precision_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_precision (lane E2) (93569 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_precision_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for precision; BaseColorTex of MI_Boost_precision_Clay (M_PickupProp takes no colour parameter) (lane E2) (11455 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_precision_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_precision_Clay; slot Clay of SM_Boost_precision (lane E2) (6412 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_precision_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for precision; BaseColorTex of MI_Boost_precision_Accent (M_PickupProp takes no colour parameter) (lane E2) (11501 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_precision_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_precision_Accent; slot Accent of SM_Boost_precision (lane E2) (6428 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_Boost_heavy_puck.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_heavy_puck.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_heavy_puck_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_heavy_puck (lane E2) (88111 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_heavy_puck_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for heavy_puck; BaseColorTex of MI_Boost_heavy_puck_Clay (M_PickupProp takes no colour parameter) (lane E2) (11800 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_heavy_puck_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_heavy_puck_Clay; slot Clay of SM_Boost_heavy_puck (lane E2) (6420 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_heavy_puck_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for heavy_puck; BaseColorTex of MI_Boost_heavy_puck_Accent (M_PickupProp takes no colour parameter) (lane E2) (11525 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_heavy_puck_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_heavy_puck_Accent; slot Accent of SM_Boost_heavy_puck (lane E2) (6436 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_Boost_big_puck.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_big_puck.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_big_puck_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_big_puck (lane E2) (85029 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_big_puck_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for big_puck; BaseColorTex of MI_Boost_big_puck_Clay (M_PickupProp takes no colour parameter) (lane E2) (11433 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_big_puck_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_big_puck_Clay; slot Clay of SM_Boost_big_puck (lane E2) (6404 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_big_puck_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for big_puck; BaseColorTex of MI_Boost_big_puck_Accent (M_PickupProp takes no colour parameter) (lane E2) (11479 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_big_puck_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_big_puck_Accent; slot Accent of SM_Boost_big_puck (lane E2) (6420 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_Boost_reverse_other.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_reverse_other.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_reverse_other_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_reverse_other (lane E2) (86721 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_reverse_other_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for reverse_other; BaseColorTex of MI_Boost_reverse_other_Clay (M_PickupProp takes no colour parameter) (lane E2) (11548 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_reverse_other_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_reverse_other_Clay; slot Clay of SM_Boost_reverse_other (lane E2) (6444 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_reverse_other_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for reverse_other; BaseColorTex of MI_Boost_reverse_other_Accent (M_PickupProp takes no colour parameter) (lane E2) (11594 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_reverse_other_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_reverse_other_Accent; slot Accent of SM_Boost_reverse_other (lane E2) (6460 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_Boost_custom.uasset | mesh | 0.10 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Boost_custom.fbx (gate: READY 14:48) by name, static mesh, no collision, base centre at z 0; slots Clay and Accent bound to MI_Boost_custom_Clay / _Accent; loaded by device A's pickup as /JamAssets/Props/SM_Boost_custom (lane E2) (103039 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_custom_Clay.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Clay colour for custom; BaseColorTex of MI_Boost_custom_Clay (M_PickupProp takes no colour parameter) (lane E2) (11387 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_custom_Clay.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_custom_Clay; slot Clay of SM_Boost_custom (lane E2) (6388 bytes) |
| Game/Plugins/JamAssets/Content/Props/T_Boost_custom_Accent.uasset | texture | 0.01 | original | Python (zlib PNG writer) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | 8 x 8 solid colour, sRGB, the boosts manifest's Accent colour for custom; BaseColorTex of MI_Boost_custom_Accent (M_PickupProp takes no colour parameter) (lane E2) (11433 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_Boost_custom_Accent.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | instance of M_PickupProp, scalars as MI_PickupProp_speed (Emissive 5, ClayProp 1, ClayPropSat 1.7, ClayPropGlow 0.3, ClayRim 1.6, ClayRimPower 1.6), BaseColorTex T_Boost_custom_Accent; slot Accent of SM_Boost_custom (lane E2) (6404 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_shotgun_p3_clay.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_shotgun_p3_clay.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_shotgun_Clay / _Accent; shown on its bone by SetBoostEquipped on the clay figure BP_SkaterVisual_TB1_P3 (lane E2) (79761 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_shotgun_p2_cel.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_shotgun_p2_cel.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_shotgun_Clay / _Accent; shown on its bone by SetBoostEquipped on the night figure BP_SkaterVisual_TB1 (lane E2) (79738 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_shotgun_Clay.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Clay colour for shotgun (sRGB to linear); slot Clay of SM_Equip_shotgun_p3_clay and _p2_cel (lane E2) (4879 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_shotgun_Accent.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Accent colour for shotgun (sRGB to linear); slot Accent of SM_Equip_shotgun_p3_clay and _p2_cel (lane E2) (4891 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_hip_check_p3_clay.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_hip_check_p3_clay.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_hip_check_Clay / _Accent; shown on its bone by SetBoostEquipped on the clay figure BP_SkaterVisual_TB1_P3 (lane E2) (83809 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_hip_check_p2_cel.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_hip_check_p2_cel.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_hip_check_Clay / _Accent; shown on its bone by SetBoostEquipped on the night figure BP_SkaterVisual_TB1 (lane E2) (83744 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_hip_check_Clay.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Clay colour for hip_check (sRGB to linear); slot Clay of SM_Equip_hip_check_p3_clay and _p2_cel (lane E2) (4891 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_hip_check_Accent.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Accent colour for hip_check (sRGB to linear); slot Accent of SM_Equip_hip_check_p3_clay and _p2_cel (lane E2) (4903 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_shield_p3_clay.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_shield_p3_clay.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_shield_Clay / _Accent; shown on its bone by SetBoostEquipped on the clay figure BP_SkaterVisual_TB1_P3 (lane E2) (79544 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_shield_p2_cel.uasset | mesh | 0.08 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_shield_p2_cel.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_shield_Clay / _Accent; shown on its bone by SetBoostEquipped on the night figure BP_SkaterVisual_TB1 (lane E2) (79524 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_shield_Clay.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Clay colour for shield (sRGB to linear); slot Clay of SM_Equip_shield_p3_clay and _p2_cel (lane E2) (4873 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_shield_Accent.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Accent colour for shield (sRGB to linear); slot Accent of SM_Equip_shield_p3_clay and _p2_cel (lane E2) (4885 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_slow_puck_p3_clay.uasset | mesh | 0.10 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_slow_puck_p3_clay.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_slow_puck_Clay / _Accent; shown on its bone by SetBoostEquipped on the clay figure BP_SkaterVisual_TB1_P3 (lane E2) (102496 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/SM_Equip_slow_puck_p2_cel.uasset | mesh | 0.10 | original | Blender 4.2 Python script (lane B2 boosts) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | n | sets-lab/boosts/export/SM_Equip_slow_puck_p2_cel.fbx by name, static mesh, prop frame = figure rest frame at the pivot; slots Clay/Accent bound to MI_Equip_slow_puck_Clay / _Accent; shown on its bone by SetBoostEquipped on the night figure BP_SkaterVisual_TB1 (lane E2) (102433 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_slow_puck_Clay.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Clay colour for slow_puck (sRGB to linear); slot Clay of SM_Equip_slow_puck_p3_clay and _p2_cel (lane E2) (4891 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/Props/MI_Equip_slow_puck_Accent.uasset | material | 0.00 | original | UE 5.9 editor via MCP/Python | | | 2026-11-14 | B | n | duplicate of MI_Prop_ClayTimber (M_Toon), TeamColour the boosts manifest's Accent colour for slow_puck (sRGB to linear); slot Accent of SM_Equip_slow_puck_p3_clay and _p2_cel (lane E2) (4903 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 10.67 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-14 | B | y | as the first E2 row above (lanes B4 to FILM3, E2), and: the four Boost* components now hold the props lane's SM_Equip_<kind>_p2_cel at the socket rule (pivot x inverse of the bone's reference-pose component transform: shotgun spine_03 (-29.80, 2.76, 0), hip_check pelvis (0, 3, 0), shield spine_03 (-18.70, 5.76, 0), slow_puck head (0.44, -34.97, 0), roll -90), SetBoostEquipped keeps one bool per kind (bEqShotgun, bEqHip, bEqShield, bEqSlow, bEqAny), EquipApply/EquipTick hide them while down or getting up (lane E2, job 5) (11185004 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 10.87 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-14 | B | y | as the first E2 row above (lanes SKI3 to FILM3, E2), and: the four Boost* components now hold SM_Equip_<kind>_p3_clay at the socket rule (shotgun spine_03 (-32.32, 2.47, 0), hip_check pelvis (0, -5, 0), shield spine_03 (-21.22, 5.47, 0), slow_puck head (0.62, -32.51, 0), roll -90), SetBoostEquipped with one bool per kind, the props hidden while down or getting up (EquipApply/EquipTick), as BP_SkaterVisual_TB1 (lane E2, job 5) (11393953 bytes) |
