# The Blender toy skater on the ice as a skeletal visual (lane A26, H10.9-H12.2)

Status: experiment notes, 2026-11-12 16:54-18:40 CET. R-0024 point 1: lane A22's `SK_SkaterToy` goes on the ice as a skeletal visual next to the
code-built `UToySkaterVisualComponent`, and the better one ships after a PIE comparison. Code in `Game/Source/JamGame/{Public,Private}/SkeletalSkaterVisual.{h,cpp}`,
the swap key value `game.skater.visual: "toy_skeletal"`, the proof cartridge `Game/Content/Cartridges/hockey.test.visual-a26.cartridge.json`, stills in
`docs/experiments/img/figure/`, Gemini readings in `docs/experiments/eyes/A26-*.json`.

## 0. The answer in four lines

1. `USkeletalSkaterVisual` drives the 17 imported sequences from the sim frame only (no AnimBP, no delta time): every C-0001 v3 call maps to a sequence and a frame, `Jam.Bot.VisualSwap` is green with the new value and the PIE hash at frame 600 is the same for `toy` and `toy_skeletal` (`034316ec7ef27999`).
2. On the same frozen frame (seed 1, frame 223, camera 10 / 24 / 40 m) the skeletal figure reads as a hockey player where the code figure reads as a bottle: helmet, two arms, mittens, a wooden blade on the ice; at 40 m it is still a skater with a stick.
3. It costs about 0.1 ms of game thread for six figures and no measurable GPU over the code figure on the same frame (section 4).
4. Recommendation: ship `toy_skeletal` as the default once the three defects in section 5 are fixed by A22 and A26 (hands off the shaft in the stick verbs, the number patch, the lean on a tapered body); until then it stays a cartridge value next to `toy`, which remains the fallback.

## 1. How the visual drives the figure

`USkeletalSkaterVisual` is a `UPoseableMeshComponent` on the pawn (the same seat as the toy component, chosen by `AHokejGameMode::ResolveVisualClass`).
Every `UpdateMotion` call (one per 60 Hz sim frame) advances a frame counter, the lean follow and the stride phase, then `ChooseLayer` picks one
whole-body sequence and a frame in it, every bone of that sequence is sampled with `UAnimSequence::GetBoneTransform` at `frame / 60 s`, the stick aim is
blended per bone, and the local transforms go into `BoneSpaceTransforms` followed by one `RefreshBoneTransforms`. Nothing reads delta time or the wall
clock; the same call sequence gives the same pose at any render rate, and the sim never sees the visual (the hash proof below).

| Call (C-0001 v3) | Sequence | Frame rule | Notes |
|---|---|---|---|
| `UpdateMotion` (speed < 0.3 m/s) | `idle` (60, loop) | `LocalFrame mod 60` | |
| `UpdateMotion` (speed >= 0.3) | `skate` (36, loop) | `frac(phase) x 36`, phase += speed / 2.4 / 60 | one stride cycle per 2.4 m, the code figure's bob phase |
| `UpdateMotion` lean | component roll | `-Lean`, Lean follows 2.5 x speed x turn rate (max 35 deg) at 12 / s | identical to the toy; the blade spray hook at 25 deg kept |
| `SetHasPuck(true)` | `carry` (60, loop) | `LocalFrame mod 60` | |
| `SetStickAim(angle, reach)` | `aim_left` / `aim_right` last frame | per-bone blend of `stick_root`, `arm_l`, `arm_r` by `abs(angle) / 70` x layer weight | never a whole-action blend (BLENDER.md risk 2); the reach slides `stick_root` by `(reach - 1.2) x 100 - 30 x t` cm in the torso frame |
| `SetShotCharge(c)` | `shoot_windup` (24) | hold at `c x 23` | aim weight 0.5 while winding up |
| `PlayShoot(charge)` | `shoot_release` (51) | frames since the call | |
| `PlayPass(strength)` | `shoot_release` from 25 | `25 + since x 1.5`, 18 frames | the pawn also sends `PlayShoot(0)` on a pass frame; the pass clears it |
| `PlayPoke(clean)` | `poke` (18) | frames since; mistimed: `get_up` frames 0-7 after it (the stumble) | |
| `PlayLift` | `lift` (20) | frames since | |
| `PlayCheck` | `check` (20) | frames since | |
| `PlayFoulReaction` | `check` at 0.6 | `since x 0.6`, 33 frames | |
| `SetKnockedDown(true, dir)` | `knocked_down` (40) | frames since, held at 39 while down | the sequence falls backward, so the component yaws over 6 frames until its back faces `dir` |
| `SetKnockedDown(false)` | `get_up` (20) | frames since | the yaw eases back over the 20 frames |
| `SetFaceoff(true)` | `faceoff_ready` (30, loop) | `LocalFrame mod 30` | |
| `PlayFaceoffDraw(won)` | `draw` (15) | frames since | |
| `SetGoalieStance(on, side)` | `goalie_left` / `goalie_centre` / `goalie_right` (30, loop) | `LocalFrame mod 30` | |
| `SetBoost(kind)` | material + scale | `Emissive` 2.2 x (1 + 0.5 sin(2 pi frame / 30)), scale 1 + 0.04 sin | `NAME_None` restores 2.2 and scale 1 (R-0016) |
| `SetTeam`, `SetNumber` | materials | `TeamColour` on the base slot, a lighter head, dark trim, wood stick; `NumberTens` / `NumberOnes` on every instance; a text render on the back | |
| sockets | bones | `stick_socket`, `puck_socket`, `head_socket` are bones: `DoesSocketExist` is true, the pawn draws the carried puck at `puck_socket` unchanged | |

Priority when several apply (highest first): knocked down / getting up, check, foul, shot, pass, poke (+ stumble), lift, draw, goalie stance, faceoff ready, wind-up, carry,
skate, idle. The stick aim blend applies at weight 1 on idle / skate / carry, 0.5 on the wind-up, 0 while a stick verb or a fall owns the stick.

Materials: B's `M_Toon` lacks the SkeletalMesh usage flag (a skinned mesh then draws the engine's default grid material, `LogMaterial: missing usage flag SkeletalMesh`),
so `Game/Content/Figures/` holds `M_ToonFigure` (a duplicate of `M_Toon` with the flag), `MI_SkaterFigure_Trim` and `MI_SkaterFigure_Stick` (duplicates of `MI_Toon_Helmet`
and `MI_Toon_Stick` reparented to it). The component makes one dynamic instance per slot from those (B's originals, the toy material and the basic shape material are the
fallbacks), sets `Emissive` 2.2 and `RimStrength` 0.6 as the toy does, and renders custom depth with stencil 1 for the outline. `Game/Plugins/JamAssets` is untouched.

## 2. Proof

- `Jam.Bot.VisualSwap` (headless, 17:15:29-17:15:48, `Game/logs/visualswap-a26-2.log`): a third track `toy_skeletal` added to the test; `generated_b2`, `humanoid_b1` and
  `toy_skeletal` give the same hash `1871c1d348fa8b41` on seed 1 and pass the bot gate.
- PIE (editor from the worktree on MCP 8013, `hockey.test.visual-a26` = the regular cut with `toy_skeletal`, seed 1, `jam.ScriptHuman 1`): `JAMSIM frame=600 hash=034316ec7ef27999`
  with `toy_skeletal` (16:25:12 UTC log time) and the same `034316ec7ef27999` with `hockey.regular` (`toy`) at 16:26:20; same score 0:1 and phase.
- Determinism of the proof itself: three PIE sessions frozen at sim frame 222 by global time dilation 0.0001 (A23's trick) all read frame 223 at the first capture, with the
  blue skater nearest the puck at the same (-7.086, 3.101) m.
- Schema: `node --test schema/hockey.test.mjs schema/rules.test.mjs schema/effects.test.mjs` 51 pass; `gen-schema-data` (3 files) and `gen-parity` (78 cases, 19 valid) rerun.

## 3. Stills (2560 x 1440, `docs/experiments/img/figure/`)

Same frozen frame (seed 1, frame 223), same lab camera (10 / 24 / 40 m behind the blue skater nearest the puck, pitch -17.7, FOV 70, A23's geometry), the night rig as shipped.

| Distance | Code figure | Skeletal figure |
|---|---|---|
| 10 m | `toy-10m.jpg` | `skel-10m.jpg` |
| 24 m | `toy-24m.jpg` | `skel-24m.jpg` |
| 40 m | `toy-40m.jpg` | `skel-40m.jpg` |
| stick verbs, 10 m | `toy-verbs-10m.jpg` | `skel-verbs-10m.jpg` |
| stick verbs, 5 m | `toy-verbs-5m.jpg` | `skel-verbs-5m.jpg` |

The verb stills: on the frozen frame the four nearest figures were given `PlayPoke(true)` + 6 frames, `SetFaceoff(true)`, `SetGoalieStance(true, -1)` and
`SetKnockedDown(true, (1, 0))` + 40 frames by hand through the interface calls (Python in PIE, cosmetic only, the sim untouched), the fifth `UpdateMotion(6 m/s)` x 9, the sixth
`SetHasPuck` + `SetShotCharge(0.8)`. The knocked-down figure and the goalie are outside the 5 and 10 m frames (the goalie is the far orange figure in the crease).

## 4. Cost

Game thread (the `SKELVIS` probe in `ApplyPose`: sampling 11 bones from one sequence, the aim blend, `RefreshBoneTransforms`), means over the first 600 sim frames of the PIE match:
JamSkater_0 50.7 us, JamSkater_1 17.8, _2 15.8, _3 13.8, _4 14.2, _5 13.8 per figure per sim frame, so about 0.13 ms per sim frame for six figures (the human's figure
pays more: its text render child is updated through the same refresh). The code figure animates by setting a dozen scene component transforms; it is cheaper still, but
both are under the noise of the 0.01 ms sim step.

GPU (ProfileGPU, RTX 4090, the PIE viewport at its 1276 x 700 window size, the lab camera at 10 m on the frozen frame 223, three samples per figure, 17:32-17:33;
whole frame = the Scene row divided by its share):

| Sample | Skeletal: frame / BasePass / CustomDepth / ShadowDepths | Code figure: frame / BasePass / CustomDepth / ShadowDepths |
|---|---|---|
| 1 | 3.26 / 0.149 / 0.137 / 0.416 ms | 7.29 / 0.189 / 0.140 / 1.673 ms (the VSM pages re-rendered after the camera jump) |
| 2 | 3.33 / 0.140 / 0.137 / 0.407 ms | 3.52 / 0.192 / 0.137 / 0.407 ms |
| 3 | 4.54 / 0.130 / 0.658 / 0.430 ms (a custom-depth spike in that one sample) | 3.90 / 0.160 / 0.135 / 0.762 ms |
| median | **3.33 / 0.140 / 0.137 / 0.416** | **3.52 / 0.189 / 0.137 / 0.762** |

Base pass draws: 41-45 with six skinned figures against 64-65 with six groups of basic shapes (one mesh with five sections against eleven static meshes per figure), so the
skeletal figure is a little cheaper to draw, not dearer; the shadow depth and custom depth passes are the same within the sample noise. Nothing here is over 0.2 ms
either way; the frame stays at 3.3-3.5 ms at this viewport (A23 measured 6.5-7.0 ms at 2560 x 1354 for the whole look). Earlier samples during play (different frames,
different camera) gave Scene 4.7 ms skeletal / 3.8 ms code figure and are not comparable; the same-frame table above is the one to read.

## 5. What reads better and worse than the code figure

Better, skeletal:
- At 10 m it is a hockey player: dark helmet with a visor, two arms with forward elbows, mittens, a tapered shaft with a wooden blade flat on the ice, white runners under
  the skates, a jersey over a lighter head. The code figure is a bottle with a hoop and a 6 cm rod.
- At 24 m the silhouette still says "skater with a stick" (the arms and the blade are the tell); the code figure says "capsule". At 40 m the skeletal figure is a small
  skater, the code figure a small capsule; both team colours read at every distance.
- The carry pose puts the blade at the puck (`puck_socket` rides the stick bone); the poke reads as a thrust; the faceoff crouch and the goalie block are distinct
  poses, which the code figure cannot do at all (it has no faceoff, goalie, poke, lift or draw pose: those calls are empty on it).
- The motion is a stride (legs do not exist, but the body and the stick swing with the phase) instead of a bob.

Worse, skeletal:
- In the stick verbs the mittens stay at the body while the shaft thrusts (poke) or lifts: the STRETCH_TO arm stretch baked as bone scale (BLENDER.md risk 2) does not
  carry through the FBX bake or the import for those actions, and my aim blend moves `stick_root` with the two arms but not their length. Visible at 5 and 10 m. For A22:
  bake the arm stretch as translation keys on the hand ends (or a two-bone arm), or key the mittens on the stick bone.
- The number: the curved UV patch on the back has no digit material yet (B's `M_Toon` has no `NumberTens` / `NumberOnes` reader), so the number is a text render as on
  the toy; it comes out mirrored and faint (`skel-10m.jpg`, `skel-10m-v2.jpg`), parented to the torso bone and parented to the component root alike (the second try ruled
  out the bone hierarchy; the cause is open, probably the text's facing against this mesh's frame). The code figure's number reads at 24 m; the skeletal one needs a
  digits material on the patch, which also removes the text render.
- The 35 degree lean of the code figure's turn (kept as a roll of the whole component) reads as a fall on a tapered body with feet: at 10 m the leaning blue skater looks
  tipped over. Either cap the roll at 20 degrees for this figure or author the lean into the skate cycle (A22) and keep the roll for the blade spray hook only.
- The head is 0.31 of the height: at 40 m the helmet and the body merge into one dark-topped blob; Gemini's reading of the Blender sheet (Director, 17:17: head 0.25-0.35 of
  the height, a blade line under the skate blocks, a longer thinner stick blade, cuffs on the mittens) is consistent with what the 24 and 40 m stills show.
- Six skinned meshes replace six groups of basic shapes: the figure is 3,808 triangles against about 1,200 for the toy; the GPU table says whether that shows.

Lane A27's eyes tool (R-0024 point 2; Gemini 3.1 pro, advice not instructions; `docs/experiments/eyes/A26-*.json`, four calls, about $0.13):

- `ab-pair` A = code figure, B = skeletal, same frame: **B at 10 m (confidence 0.95), B at 24 m (0.95), B at 40 m (0.9)**. Figures: "chunky toy proportions with distinct round
  heads, big hands, and visible stick blades, whereas A uses literal placeholder capsules" (10 m); "visible arms, big hands, and skates, whereas A uses abstract floating
  cylinders" (24 m); "distinct round heads and arms ... A's featureless cylinders" (40 m). Outline: B at every distance (the geometry gives the ink lines something to draw).
  Sky, ice, pools, construction: tie (identical). What A keeps: instant team identification from the uninterrupted colour mass (10 and 40 m: the black helmet takes jersey
  pixels), and **the visible jersey numbers at 24 m**.
- `figure` on `skel-verbs-5m.png`: hockey player 3 / 5 at full size, 2 / 5 at 128 px ("skates are tiny slivers that vanish when downscaled"; the leaning blue figure seen from
  behind "lacks visible arms and a physical stick, appearing as a blue cylinder"); toy 4 / 5 at both sizes ("peg dolls"). Proportions read: head 0.23 of the height,
  shoulders 0.33, stick 0.83. Changes it asks for, in numbers: skates 0.05 -> 0.15 of the height, stick thickness 4 -> 10 px at this size, head 0.23 -> 0.30, arm geometry
  or contrast on the blue figure. Works: the team contrast, the black helmets with visors, the puck highlight.
- Position-bias check not run (one order per pair); the three verdicts agree with my own reading of the same stills, so I did not spend the second call.

## 6. Recommendation for the shipped default

`toy_skeletal` should become the default once (1) A22 fixes the hand stretch in the stick verbs and authors the lean into the skate cycle, (2) B (or A26) gives the number
patch a digits material reading `NumberTens` / `NumberOnes`, and (3) the roll cap is set for this body. Each is a small change (a Blender script parameter, one material,
one constant); none of them touches the sim, the contract or the cartridges. Until then `toy` stays the default, `toy_skeletal` is one cartridge value away, and the
VisualSwap test keeps both honest. If the Director wants the swap tonight regardless, the figure is already better than the code figure at every distance for
the things a player looks at (who has the puck, where the blade is, who is poking, who is down); the three defects are cosmetic and visible only under 10 m.

## 7. Files

- new: `Game/Source/JamGame/Public/SkeletalSkaterVisual.h`, `Game/Source/JamGame/Private/SkeletalSkaterVisual.cpp`, `Game/Content/Cartridges/hockey.test.visual-a26.cartridge.json`,
  `Game/Content/Figures/M_ToonFigure.uasset`, `Game/Content/Figures/MI_SkaterFigure_Trim.uasset`, `Game/Content/Figures/MI_SkaterFigure_Stick.uasset`, `docs/experiments/FIGURE.md`,
  `docs/experiments/img/figure/*.jpg` (14: the ten of section 3 and `{skel,toy}-{10,24}m-v2.jpg` from the profiling session after the number change),
  `docs/experiments/eyes/A26-*.json` (4).
- one line each: `schema/game.schema.json`, `schema/limits.json`, `schema/README.md`, `schema/DEFAULTS.md`, `schema/hockey.test.mjs` (the preset list), `Game/Source/JamCore/Public/JamCartridge.h`
  (comment), `Game/Source/JamGame/Private/HokejGameMode.cpp` (include, FName, one branch).
- generated: `Game/Source/JamCore/Private/JamSchemaData.inl`, `Game/Source/JamCore/Tools/parity.json`.
- outside the listed scope, for the Director: `Game/Source/JamCore/Tests/VisualSwapTests.cpp` (the third track, 8 lines).
- main tree: `status/A26.md`.

## 7b. Pass 3 (H12.0, the Director's 17:37 and 17:52 messages, A22's third import)

- A22's figure (15 bones, 19 sequences with `skate_left` / `skate_right` carrying an authored 15 degree lean, two-bone arms baked to plain keys, mittens skinned to
  `stick_root`, slots base / number / head / trim / steel / stick) copied from the shared tree. The component assigns materials by slot name (import order as the
  fallback), blends `skate_left` / `skate_right` over `skate` per bone by the lean state (|lean| / 35, which follows the sim's turn rate) and caps the component roll at
  5 degrees (`MaxRollDeg`); the lean no longer reads as a fall and the mittens stay on the shaft (`skel3-verbs-5m.jpg`).
- `toy_skeletal` is the default: absent `game.skater.visual` = `toy_skeletal` (JamCartridge.h, JamCoreTests, DEFAULTS.md, README.md, the schema description; embedded
  schema and parity regenerated), `toy` stays a value for the code figure. `hockey.regular` with no key spawns the skeletal figure; hash at frame 600 unchanged
  (`034316ec7ef27999`); VisualSwap PASS with the three tracks (`1871c1d348fa8b41`).
- `M_SkaterNumber` (unlit, skeletal flag, a seven-segment two-digit mask from `NumberTens` / `NumberOnes` in one Custom HLSL node, no texture, built by Python in the
  editor) renders the whole patch as ink in PIE (`skel3-10m.jpg`), so it is switched off behind `SkeletalSkater::bUseDigitsMaterial = false`: the patch stays white and the
  white text render carries the number, readable from behind (`1` on the blue figure). The earlier "mirrored" digits were the text's back side showing past the shoulder
  of figures facing the camera (the text quad is two-sided), not a bone issue. Open: the patch's UV range (channel 0, 0..1?) before the material goes on.
- Eyes ab-pair, A = pass 2, B = pass 3 (`docs/experiments/eyes/A26-ab-pass3-*.json`): 10 m A (0.95) for the black square and the text artifact; 24 m B (0.85) for the hands
  on the sticks. Stills: `skel3-{10,24}m.jpg`, `skel3-verbs-{10,5}m.jpg`.
- A22's forearm gap (11-16 cm at the extremes of poke / draw / shoot_release / check): visible at 5 m on the poking figure's lower hand, not at 10 m. Not hidden here.
- Cost: 32 us per figure per sim frame (JamSkater_0, 600-frame mean; the side-blend samples a second sequence only while turning); GPU on the frozen frame at 10 m,
  one sample: frame 3.51 ms, BasePass 0.160, CustomDepth 0.348, ShadowDepths 0.427 (pass 2 medians 3.33 / 0.140 / 0.137 / 0.416).

## 8. Errors met

- `Jam.Bot.VisualSwap` first failed on the new value: the C++ loader validates against the schema embedded by `gen-schema-data.mjs`, so a schema edit needs that script and a rebuild.
- Default grid material on the figure: the SkeletalMesh usage flag (section 1).
- Mirrored number glyphs when the text is parented to a bone (section 5).
- `node --test schema/` runs nothing useful on Node 24 (a directory is treated as a file); name the files.
- Git Bash mangles `\\` in inline `node -e` arguments and turns `/c/...` paths over for node unless `MSYS_NO_PATHCONV=1`; the helper scripts live in files with Windows paths.
