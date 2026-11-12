# P4 motion set: skating and the stick verbs from text-to-motion, the Meshy library, Manny clips and (for the operator) Rokoko Vision

Lane A29, device A, 2026-11-12 16:57-19:00 local (R-0025 point 3). Reads with `docs/experiments/ANIMATION-SOURCES.md` (the research), `contracts/skater-visual.md`
(C-0001 v3: the verbs), `docs/experiments/BLENDER.md` section 5 (the toy figure's 17 actions, the same list), `tools/mcp/ANIMATION.md` section 4 (the retarget recipe).
Tools: `tools/p4motion/` (`clip.py`, `gate.mjs`, `retarget.mjs` + `ue_retarget.py`, `capture.mjs`, `bonemap-<vendor>.json`, `jobs.json`, `meshy-ledger.jsonl`).
Clips: `Game/Content/P4/Anim/AS_P4_*` (in place, on the UE5 Manny skeleton `SK_Mannequin`), rigs and retargeters under `Game/Content/P4/Anim/Rigs/`, the vendor-skeleton
sources under `Game/Content/P4/Anim/Src/<Vendor>/`, the proof level `Game/Content/P4/Anim/Proof/L_P4_Proof`. Stills: `devlog/shots/20261112-*-A-a29-*.png`.

Operator decisions folded in (17:0x-17:1x): the Fab "Hockey Player Animated" pack is **descoped** (not in any route or table); falls and collision reactions are a
first-class verb group (knocked_down from the side / from behind / a trip, get_up, a stumble on a mistimed poke, a board collision); Rokoko Vision accepts YouTube video
and 30 seconds of Vision budget go to the contact moves only (section 6).

## 0. The answer in six lines

1. **Text-to-motion (Meshy prime) gives a skating stride that passes the gate** once it is cut to its best cycle and the loop is blended closed: `AS_P4_skate_fwd`,
   1.33 s, loop error 0 m, root in place, retargeted onto the Manny with every core bone mapped. 10 credits and about 2 minutes of tool time per locomotion clip.
   **Not yet watched playing correctly**: the retarget leaves the pelvis translation 80 m off (section 8, blocker 1, with the fix path); rotations are right.
2. **The Meshy animation library covers the falls group** on the `ue5_mannequin` rig (names match the Manny core; 3 credits a clip, 4 s): knocked down from the front,
   from behind, a trip, a hit reaction, a stumble, two get-ups, idle.
3. **The Manny clips that ship with the engine** are the day-one fallback for hit reactions and falls (`MM_HitReact_*`, `MM_Death_*`, `MM_Attack_02` as a check), already on
   the target skeleton, no retarget.
4. **The stick verbs with a contact moment** (shot wind-up and release, pass, poke, lift, check, faceoff draw) have text-to-motion placeholders at best; the registry rule
   wants two views or a side camera: **the operator films them for Rokoko Vision** (section 6) and drops the FBX under `Game/Content/P4/Anim/Vision/`; the same clip.py,
   gate and retarget path takes it from there (`bonemap-rokoko.json` to be written from the first file's bone list).
5. **A motion gate runs before any clip enters a build** (section 3): loop closure, foot contact, root motion lint, contact window. Thresholds are in this file.
6. What did not work and is written down so nobody repeats it (all measured 17:20-17:55 on UE 5.9 through MCP): a SceneCapture2D driven from editor Python under
   Simulate exports black; an `execute_python` that contains `await asyncio.sleep` leaves every later viewport capture black; scrubbing with `set_animation` +
   `set_position` across calls blackens too, `override_animation_data(seq, True, False, t, 0)` per call renders; and a level whose lights are Stationary with no built
   lighting renders black in Simulate on a fresh editor instance: the proof level's sun and sky light are Movable now. A force-stopped editor leaves
   `Saved/Autosaves/PackageRestoreData.json`, and the next editor on the same tree (any lane's) blocks on a modal "Restore Packages" dialog that stops every MCP tool.

## 1. Verb table (C-0001 v3 and the falls group), source, gate, contact window

Filled per clip by `tools/p4motion/jobs.json`; "src" is the vendor skeleton: *Ref* = Meshy text-to-motion Reference (52 bones), *MRig* = Meshy `ue5_mannequin` rig (28),
*Manny* = engine clip on `SK_Mannequin`. Gate columns are the measured numbers; the verdict is PASS / WARN / FAIL as `gate.mjs` prints them. A contact window is the frame
range (at 60 fps, in the in-place clip) in which the attack "hits"; the visual times the impact effect on it.

Gate numbers (metres, metres per second) as `gate.mjs` printed them at 17:44-17:55; "raw cut err" is the honest loop number for a text-to-motion cycle: the pose
distance between the first and last frame of the best cycle *before* the 6-frame blend closes it (the blend makes the gate's loop closure 0 by construction; a raw cut
error above 0.15 m means the blend has to hide a visible snap, judge it by eye on the strip). Reactions keep their vertical drop; their horizontal knock-back is stripped
and shows up as "slide" (the feet move with the thrown body), which is a WARN by design, never a FAIL.

| Verb (C-0001 v3, falls group) | Clip `AS_P4_*` | Source | Route | Cr. | Frames @60 (s) | Gate | Numbers | Contact window | Note |
|---|---|---|---|---|---|---|---|---|---|
| idle | `idle` | MRig | library 0 Idle | 3 | 240 (4.0) | PASS | float 0.012, slide 0.04 | | plain standing idle; a hockey "ready" idle is a Rokoko Create prompt (section 5) |
| skate cycle | `skate_fwd` | Ref | text-to-motion prime 4 s, cut 45..85, blend 6 | 10 | 80 (1.33) | PASS | raw cut err 0.10, float 0.053, glide 0.73 | | one stride cycle; the raw clip travels 0.49 m/s, so the visual scales the play rate by speed as the toy does |
| skate backward | `skate_bwd` | Ref | text-to-motion prime 4 s, cut 13..34, blend 6 | 10 | 42 (0.70) | PASS | raw cut err 0.22, float 0.024, glide 0.44 | | C-cuts; short cycle, blend hides a 22 cm snap: judge on the strip |
| crossover left | `crossover_l` | Ref | text-to-motion prime 4 s, cut 54..85, blend 6 | 10 | 62 (1.03) | PASS | raw cut err 0.30, float 0.025, glide 0.39 | | the lean stays code (component roll, BLENDER.md 5); raw cut err 0.30: the model did not loop, the blend does the closing |
| crossover right | `crossover_r` | Ref | text-to-motion prime 4 s, cut 52..79, blend 6 | 10 | 54 (0.90) | WARN | raw cut err 0.15, float 0.061, glide 3.38 (raw travel 5.3 m in 4 s) | | a fast crossover: the planted-foot glide is 3.4 m/s, so this one plays at speed 3-6 m/s only |
| glide | `glide` | Ref | text-to-motion prime 3 s, cut 9..29, blend 6 | 10 | 40 (0.67) | PASS | raw cut err 0.14, float 0.037, glide 1.39 | | |
| stop | `stop` | Ref | text-to-motion prime 3 s, re-prompted once ("both feet on the ice from the very first frame"), whole clip | 10 (+10 for the first, FAIL: started airborne) | 178 (2.97) | PASS | float 0.029, glide 0.23 (raw travel 0.33 m) | | one-shot, not a loop; the first try is gone |
| carry (stickhandling) | `carry` | Ref | text-to-motion prime 4 s, cut 28..108, blend 6 | 10 | 160 (2.67) | PASS | raw cut err see jobs.json, float 0.027, glide 0.49 | | pass 2 (Director, 18:03): a slow carry stance, stick low, small stickhandling |
| aim left / right | none (code) | | | | | | | | per-bone blend of the hand/stick as the toy does; no clip |
| poke | `poke` | Ref | text-to-motion prime 2.5 s, whole clip | 10 | 148 (2.47) | WARN | slide 1.02 (a lunge step, stripped) | 40..90 at 60 fps (20..45 raw); peak hand speed 4.0 m/s at raw frame 36 | pass 2; Rokoko Vision take 4 replaces it when filmed |
| lift | none | | Rokoko Vision (take 5) | | | | | | |
| pass | none | | Rokoko Vision; fallback: the shot's follow-through at rate 1.5 (BLENDER.md 5) | | | | | | |
| shoot wind-up / release | none | | Rokoko Vision (take 1); E1's text-to-motion slap shot (MESHY.md task 01a1cd15-affa) as a placeholder, not imported | | | | | | |
| check (thrown) | `check_manny` | Manny | `MM_Attack_02` duplicated, force_root_lock | 0 | 60 (1.0) | see `jobs.json` after the capture run (gate in the editor, class attack) | | 20..40 (the lunge, from peak hand speed) | Rokoko Vision (take 2) for the real one |
| knocked_down, from the front (check into the chest) | `knocked_down_back` | MRig | library 187 Knock Down | 3 | 150 (2.5) | WARN | slide 1.38 (knock-back 0.58 m stripped), drop 0.58 | | falls on the back, holds on the ice |
| knocked_down, from behind | `knocked_down_behind` | MRig | library 180 Shot in the Back and Fall | 3 | 280 (4.67) | PASS | slide 0.07, drop 0.62 | | falls forward, long hold: cut at 2.5 s in the visual |
| knocked_down, hit from the front, falls forward | `knocked_down_front` | MRig | library 184 Shot and Fall Forward | 3 | 130 (2.17) | WARN | slide 0.25, drop 0.56 | | |
| knocked_down, variant | `knocked_down_side` | MRig | library 190 Knock Down 1 | 3 | 196 (3.27) | WARN | slide 0.61, drop 0.61 | | a wilder fall: lower foot reaches 1.03 m (a leg in the air) |
| knocked_down, from the side (true side falls) | `fall_left_manny`, `fall_right_manny` | Manny | `MM_Death_Left_01` / `MM_Death_Right_01` duplicated, force_root_lock | 0 | 66 / 66 (1.1) | editor gate, see `jobs.json` | | | the only side-direction falls in the set |
| knocked_down, a trip | `trip_fall` | MRig | library 366 Falling Down | 3 | 136 (2.27) | WARN | slide 0.38, drop 0.63 | | |
| checked from behind at speed | `hit_in_back_skating` | MRig | library 416 Hit in Back While Running | 3 | 150 (2.5) | WARN | slide 1.11 (the run itself, stripped) | | the figure runs, not skates: use only the hit half (frames 40..150) |
| short fall | `fall_2` | MRig | library 503 Fall 2 | 3 | 40 (0.67) | PASS | slide 0.13 | | too short on its own; a first half for a trip |
| check received, stays up | `hit_reaction`; `hit_reaction_manny` | MRig; Manny | library 178 Hit Reaction; `MM_HitReact_Front_Hvy_01` | 3; 0 | 98 (1.63); 48 (0.8) | WARN; editor gate | slide 1.18 (a 0.74 m stagger, stripped) | | |
| stumble (mistimed poke) | `stumble` | MRig | library 519 Sliding Stumble | 3 | 682 (11.4) | PASS | slide 0.08 | | 11 s long: the stumble is in the first 2 s, cut there in the visual |
| board collision | none | | Rokoko Vision (take 2, the wall); fallback: `hit_reaction` at rate 0.6 facing the boards | | | | | | |
| get_up | `get_up_1`, `get_up_2` | MRig | library 344 / 345 Stand Up | 3 + 3 | 496 (8.27); 184 (3.07) | PASS; PASS | slide 0.03 / 0.04, rise 0.63 / 0.36 | | 344 is slow (8 s, play at 3x); 345 is the usable one |
| faceoff ready | none | | Rokoko Vision (take 3); Rokoko Create prompt 4 | | | | | | |
| draw | none | | Rokoko Vision (take 3) | | | | | | |
| goalie stances (3) | none | | Rokoko Create prompt 5 (centre); left/right by mirroring and a hip offset in the visual | | | | | | |
| foul reaction | reuse `check_manny` at rate 0.6 (BLENDER.md 5) | | | | | | | | |

Every clip is **in place**: `clip.py` removes the horizontal travel of the root bone frame by frame (Z stays: a fall still drops), and the gate fails any clip whose pelvis
still moves more than 1 cm horizontally. The sim owns displacement.

## 2. The pipeline, step by step (what a builder runs)

```
node tools/meshy/meshy.mjs motion --prompt "..." --duration 4 --mode prime --wait --ledger tools/p4motion/meshy-ledger.jsonl    # or: animate --rig-task <id> --action N
node tools/meshy/meshy.mjs download <task> <scratch>/<clip> --only fbx
"C:/Program Files/Blender Foundation/Blender 4.2/blender.exe" -b --factory-startup --python tools/p4motion/clip.py -- --in <fbx> --name AS_P4_<verb> \
    --summary <work>/AS_P4_<verb>.summary.json --out-fbx <work>/AS_P4_<verb>.fbx [--trim a,b] [--loop-blend 6]
node tools/p4motion/gate.mjs <work>/AS_P4_<verb>.summary.json --class locomotion|attack|reaction|pose [--loop] [--contact a,b]
powershell -File tools/run/editor.ps1 -Project Game -McpPort 8016 -WaitMcp          # only while retargeting or capturing (R-0009)
node tools/p4motion/retarget.mjs --fbx <work>/AS_P4_<verb>.fbx --clip AS_P4_<verb> --vendor meshy|meshyrig [--src_mesh SK_P4_MeshyRig]
node tools/p4motion/capture.mjs --clips AS_P4_<verb>,... --width 720                   # 8 frames over 2 s of sequence time, strip under devlog/shots/
powershell -File tools/run/editor.ps1 -Project Game -Stop
```

`clip.py` recognises the skeletons by name (`meshy-reference`, `ue5-manny` for the Meshy rig, `mixamo`, `rokoko-hips`); a Rokoko Vision FBX needs its profile confirmed
from the first file's bone list (the script prints it) and `bonemap-rokoko.json` written like the two existing tables.

The retarget (`ue_retarget.py`): legacy `FbxFactory` import with 60 fps sampling, no materials or textures; the first import of a vendor keeps its mesh and skeleton as
`SK_P4_<Vendor>` / `SKEL_P4_<Vendor>` under `Src/<Vendor>/` and later clips import against that skeleton; the source IK Rig is built **from the bone table** (chains named
exactly as the Manny's auto-generated chains, so `auto_map_chains(EXACT)` maps them); the target rig is the Manny's auto definition plus auto FBIK; the retargeter gets the
default op stack (pelvis motion, FK chains, IK chains, IK solve, root motion), both rigs aligned; `IKRetargetBatchOperation.duplicate_and_retarget` writes `AS_P4_<verb>`.
The totality report lists every Manny core bone (pelvis, spine 1-5, neck 1-2, head, clavicles, arms, hands, legs, feet, balls) that no mapped chain covers: **empty for both
vendors**. Known unmapped on purpose: metacarpals and fingers for the 28-bone rig (gloves), twist bones (follow their parent), `ik_*`, `root` (never moves).

Timings measured: Meshy text-to-motion 8-19 s server, library animate 3-5 s; Blender clip.py 6-9 s; import + rig + retarget 6 s per clip in the editor; capture 2 s per frame.

## 3. Gate thresholds (`tools/p4motion/gate.mjs`, change both places)

| Check | Applies to | PASS | WARN | FAIL | How it is measured |
|---|---|---|---|---|---|
| loop closure | locomotion (and any `--loop`) | first vs last pose <= 0.05 m | <= 0.10 m | above | max over feet, toes, head, hands of the pelvis-relative distance between the first and last frame; the gate also searches every (start, end) pair at least 20 frames apart and prints the best cut for `clip.py --trim` |
| foot float | locomotion, pose | lower foot <= 0.06 m above the ice | <= 0.10 m (a hop) | above | ice = the lowest foot/toe height over the clip; per frame the lower of the two feet |
| glide | locomotion | implied travel <= 8 m/s (the sim's top speed) | | above | root travel of the raw clip over its length; planted-foot speed is reported (mean and p90) as the clip's natural glide, which the visual's play rate must match |
| foot slide | attack, reaction, pose (in place) | planted feet move <= 0.15 m/s (p90) in the raw clip | above | | a foot is planted when within 0.04 m of the ice; a reaction that is thrown back warns, by design |
| root motion lint | every clip | pelvis horizontal excursion after the strip <= 0.01 m | | above | on the stripped clip; the raw travel is printed next to it |
| contact window | attack | window inside the clip and the frame of peak hand speed inside it | peak outside the window | no window, or outside the clip | the window is written in `jobs.json` next to the clip |

Classes: `locomotion` (loops), `attack` (shot, pass, poke, lift, check, draw), `reaction` (knocked down, hit, stumble, get up), `pose` (idle, faceoff ready, goalie stances).

## 4. What each route cost (Meshy balance 786 at 16:59; other lanes spend from the same pool)

| Route | Clips | Credits | Wall time per clip | Human steps |
|---|---|---|---|---|
| Meshy text-to-motion (prime) | 6 strides (one FAIL: `stop`) | 60 | 8-19 s server + 8 s Blender x2 passes + 6 s retarget | none |
| Meshy animation library on the `ue5_mannequin` rig | 12 reactions and idles | 36 | 3-5 s server + 8 s Blender + 6 s retarget | none |
| Manny engine clips | 18 copied (death x6, hit react x8, attack x3, idle); 4 duplicated in place | 0 | none | none |
| **Total this lane** | 18 generated clips, 4 Manny duplicates, 2 IK rig pairs, 1 proof level | **96 credits** (balance 786 at 16:59; other lanes draw on the same pool) | about 2 h of lane time, of which 35 min went to the black-capture hunt | none |
| Rokoko Create text-to-motion (web, five free clips a month) | up to 5 | 0 | the operator's | prompts in section 5; the operator exports FBX |
| Rokoko Vision (web, 30 s a month on the jam perk) | 3 segments | 0 | the operator's | section 6 |

## 5. Rokoko Create: the five prompts (web account, five free text-to-motion clips a month; the operator pastes them, exports FBX, drops them under `Game/Content/P4/Anim/Create/`)

Picked for what a text-to-motion model is most likely to get right (whole-body, no prop contact, no partner): the stick verbs with a clear silhouette.

1. **shoot_windup**: "ice hockey player in a low crouch winds up for a slap shot: both hands on a hockey stick, raises the stick high behind the right shoulder, weight on the back leg, holds the top of the wind-up for a moment" (2.5 s).
2. **shoot_release**: "ice hockey player swings a hockey stick down and through a slap shot, weight transfers to the front leg, the stick follows through up and across the body, then settles" (2 s).
3. **pass**: "ice hockey player makes a quick forehand pass with a hockey stick: a short sweeping motion low along the ice from right to left, no wind-up, then back to a ready stance" (1.5 s).
4. **faceoff_ready**: "ice hockey centre at the faceoff dot, feet wide, deep crouch, stick blade on the ice in front, head up, waiting, small weight shifts, loops" (3 s).
5. **goalie_centre**: "ice hockey goalie ready stance: knees bent, feet wide, chest forward, stick blade flat on the ice in front of the right skate, glove hand open at shoulder height, small balance sway, loops" (3 s).

Then: `clip.py` with the profile the file reports, gate class `attack` (1-3) or `pose` (4-5), retarget with a `bonemap-rokoko.json` written from the bone list.

## 6. For the operator to film (Rokoko Vision, single camera, 30 s a month on the perk)

Vision wants one performer, the whole body in frame the whole time, a static camera, a plain background, at least 24 fps, HD. Strides are **not** filmed (R-0025: text-to-motion
first, and the stride already passes the gate); the 30 seconds go to the three contact moves, off-ice in shoes on a hard floor with a stick (a tennis ball or nothing for the
puck). Ice is not needed: the sim owns the displacement, the clip only owes the body.

**Camera.** Phone on a tripod at hip height (about 1 m), 4 m from the performer, landscape, 1080p at 30 or 60 fps, exposure locked, no zoom, no pan. The performer stays
inside a 2 x 2 m square marked on the floor. Plain wall behind, no other people in frame. Two views are needed for the moves along the camera axis: **film every move twice,
once from the side (camera perpendicular to the shot direction) and once from the front three-quarter (45 degrees)**; Vision takes one video at a time, so the side view is
the one uploaded (it carries the depth of the wind-up and the lunge) and the three-quarter is the reference the retarget is judged against.

**What to perform, in order, with a 1-second still "ready" pose before and after each take** (the gate cuts on them):

| # | Take | Length | Performer does | Vision budget |
|---|---|---|---|---|
| 1 | slapshot x2 | 10 s | ready stance with the stick low, wind-up high behind the shoulder, swing down and through, follow through, back to ready; twice, slowly the first time | 10 s |
| 2 | body check into the boards | 10 s | stand side-on to the wall at 1.5 m, two quick steps, shoulder into the wall (a thick mat on the wall), rebound to a stance; twice | 10 s |
| 3 | faceoff crouch and draw | 10 s | wide feet, deep crouch, blade on the floor, hold 2 s, draw the blade sharply back and to the left, stand up; twice | 10 s |
| 4 | poke check | 5 s | stance, one-handed stick thrust forward and down, back to stance; twice | spare |
| 5 | stick lift | 5 s | stance, lift the blade up under an imaginary stick in front, back; twice | spare |
| 6 | knocked down from the side and get up | 8 s | stance, fall sideways onto a mat, lie still 1 s, get up to a stance | spare |

Upload order if the 30 seconds run out: 1, 2, 3; 4-6 go next month or through Create. Export from Vision as FBX on the Manny ("Unreal Engine 5" target) if the account
allows the retarget export, else the default Rokoko skeleton; either lands under `Game/Content/P4/Anim/Vision/` with the take number in the file name.

### From YouTube (the operator's alternative; Vision accepts a video file)

Candidates found 17:05-17:17 (web research; the pages were read through a text proxy, so **camera view, static camera, fps and the exact timestamps are not verified by
watching**: the operator scrubs each one before upload). Vision needs one performer alone, full body, static camera, side-on for the contact moves. **None of the ranked
picks is Creative Commons**: every one is "Standard YouTube Licence" (inferred: no CC row on the page), so the clip would be used as a motion reference of someone else's
video, which `ANIMATION-SOURCES.md` section 0 flagged as a rights question. The only CC-licensed on-topic find is a 1:32 practice clip of unknown framing.
**Director's decision (18:03): the operator's own filming is the rights-clean route; the three clips below stay as reference only** (what the moves look like from
the side, how long a take runs), never as Vision input.

| Move | Candidate | Segment (evidence) | View / solo (inferred) | Licence | Risk |
|---|---|---|---|---|---|
| slapshot, pick 1 | "Slap Shot In Two Minutes", iTrain Hockey, youtube.com/watch?v=8hY9iRwFy3g (2:08, 2024) | 0:34-1:03 "Shot mechanics and stages" (chapter, verified); the two repetitions inside: guessed | on-ice, tripod, one demonstrator; angle unknown | Standard (inferred) | cuts to the coach talking inside the chapter |
| slapshot, pick 2 | "SLAP SHOT, Off-Ice Shooting Correction + Tips", iTrain Hockey, youtube.com/watch?v=KLtU7cOUjKY (4:12, 2020) | 3:53-4:12 "Demonstration" (chapter, verified) | off-ice shooting pad, solo | Standard (inferred) | 19 s only |
| slapshot, CC | "3 slapshots and deflections", Greg Lewis, youtube.com/watch?v=ZwmukSB9iA4 (1:32, 2015) | timestamp unknown, operator to scrub | unknown; "deflections" implies a second player | **Creative Commons Attribution (reuse allowed)** (verified) | content unknown |
| body check, pick 1 | "How To Properly Make A Body Check Against The Boards", Mountain High Hockey, youtube.com/watch?v=7oYsZ8FY9Do (1:50, 2018) | 0:08-0:43 "Proper Technique", 0:43-1:13 "Finishing The Check" (chapters, verified) | on-ice, checker + receiver | Standard (inferred) | two bodies: the receiver occludes the checker; mask or pick the clearest repetition |
| body check, pick 2 | "PRO HOCKEY TRAINING: Body Checking Basics", NSDA Hockey, youtube.com/watch?v=qQGgKYFZKs4 (13:08, 2019) | 4:57-6:11 "Checking along the wall" (chapter, verified) | on-ice drill, two players | Standard (inferred) | other players may enter the frame |
| body check, official | "Body Checking (Off-Ice Lesson)", USA Hockey, youtube.com/watch?v=XvNwI6AVaqU (12 min, 2011) | 6:02-6:52 "Hit the Wall", 6:52-8:08 "Pinning" (chapters, verified) | dryland against a wall, a youth group | Standard (inferred) | SD, several people |
| faceoff, pick 1 | "PRO HOCKEY TRAINING: Face Offs", NSDA Hockey, youtube.com/watch?v=Yijz7Vp4Y9E (10:02, 2019) | 0:26-1:00 "Proper stance and hand grip", 1:53-3:21 "Sweeping and power techniques" (chapters, verified) | on-ice at the dot, one centre probable | Standard (inferred) | an opponent at the dot; the drop may be simulated |
| faceoff, pick 2 | "How to Win Faceoffs in Hockey", How To Hockey (Coach Jeremy), youtube.com/watch?v=NwKIZfh8s6c (6:24, 2012) | 1:05-3:44 "Tips", 3:44-end "Tricks" (chapters, verified) | on-ice | Standard (inferred) | older, cut-heavy |
| faceoff, pick 3 | "Simple Face off Fundamentals", Patrick Arbour Hockey, youtube.com/watch?v=l21TGgbTWEY (0:59, 2017) | whole clip, operator to scrub | short single-skill clip | Standard (inferred) | quality unknown |

Which first and why: **8hY9iRwFy3g for the slapshot** (newest HD footage of one demonstrator on a tripod, chapter-marked mechanics), **7oYsZ8FY9Do for the check** (the
only tutorial made for the boards specifically, with chapter-marked technique and finish; expect to mask the receiver), **Yijz7Vp4Y9E for the faceoff** (chapter-marked
stance and draw on ice, 2019). Spend: about 10 s each, contact moves only.

## 7. Recommendation per action class

| Action class | Source | Why | Gate class |
|---|---|---|---|
| cyclic locomotion (stride, backward, crossovers, glide) | Meshy text-to-motion prime, 4 s, cut to the best cycle, loop blended | the stride reads as skating and closes with a 6-frame blend; 10 credits, no human | locomotion |
| stop | Meshy text-to-motion, 2.5 s one-shot | | locomotion, no loop |
| contact moments (shot, pass, poke, lift, check, draw) | Rokoko Vision from the operator's own side-view footage (section 6); text-to-motion only as a placeholder | the registry rule: a contact move needs two views or a side camera; Meshy's slap shot from E1 has vague hands without a stick | attack, with a contact window |
| falls and collision reactions | Meshy library on the `ue5_mannequin` rig (187, 180, 184, 366, 416, 178, 519, 344, 345), Manny `MM_Death_*` / `MM_HitReact_*` as the fallback already on the skeleton | hand-keyed, hold on the ground, 3 credits | reaction |
| poses (idle, faceoff ready, goalie stances) | Rokoko Create prompts (section 5), library idle as the fallback | pose loops are what text-to-motion does best | pose |

## 8. Blockers, in order

1. **The retargeted clips' pelvis translation is wrong, so no clip has been watched playing correctly yet.** Measured 17:57-18:00 in the editor: the retarget output
   `AS_P4_skate_fwd` has its pelvis at (-27.7 m, 19.8 m, 80.0 m) at t 0 and `AS_P4_knocked_down_back` at (11.7, 0.7, -83.2 m), while the Manny's own `AS_P4_check_manny`
   sits at (0.05, 0.21, 0.86 m). The bone **rotations** retarget (thigh_l moves), the IK rig maps every core bone, but the Pelvis Motion op scales the source pelvis
   travel by the target/source pelvis-height ratio, and the source skeleton as imported has its pelvis height along the wrong axis (`AS_P4_skate_fwd_src` pelvis at
   (1.2, -81.3, -3.6) cm: Y is up, so the "height" the op reads is 3.6 cm and the forward travel lands on Z). It is not the Blender round trip: the vendor's untouched
   FBX gives the same (test `AS_P4_skate_fwd_v`, deleted), and baking the armature's 90-degree object rotation before export made it 100x worse (`_u`, deleted).
   Fix path, in order: (a) import the source with the skeleton converted to Z-up (`FbxImportUI` `bForceFrontXAxis` / the Interchange pipeline, or a Blender export with
   `axis_up='Z'` and the armature rotation applied **without** `bake_space_transform`), check `Pelvis` z is about 90 cm at t 0 before building the rig; (b) failing that,
   zero the pelvis translation on the *output* clip (UE 5.9 Python exposes no `AnimDataController` on `AnimSequence`; the Blender route can do it on a re-export);
   (c) failing both, retarget with the Pelvis Motion op removed from the op stack, which keeps the figure on the ground at the cost of the pelvis bob and the fall drop.
   About 30 minutes with the editor; the eyes judgement waits on it. The proof strips under `devlog/shots/20261112-175258-A-a29-*.png` were captured with this
   pelvis offset, so they show the lit floor and a figure far off camera: **not evidence of motion**, kept so the capture path is on record.
2. **The contact moves have no clip until the operator films** (or decides on the YouTube route). Until then `check` is `MM_Attack_02` and the shot is E1's text-to-motion
   slap shot, both placeholders.
3. **The clips play on the Manny, not yet on A28's figure.** A28 has no status file yet; when its `SK_P4_*` on the Meshy `ue5_mannequin` preset lands, the `meshyrig` bone
   table applies to it unchanged (same names) and `retarget.mjs --target <its mesh>` makes the set.
4. **`Game/Content/Characters/Mannequins/` (72 MB)** was copied from the engine templates because no UE5 Manny was in the project; it is nobody's folder in
   `ownership.json`. The Director decides where it lives (A28 needs it too).
