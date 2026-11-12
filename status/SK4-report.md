# SK4 report: falls and collisions in the engine, and the puck at distance (device B)

Lane SK4, 2026-11-12, 18:57 to 20:15 local. Main tree `C:/Users/kazda/jam/thirdjam`, editor on port 8000, left through `ExitEditor` with no dirty
package. No git command, no build. Checkpoint: `status/SK4-checkpoint.md`.

**Housekeeping**
- `/Game/Maps/Rink` was never saved (mtime still 11:10); discarded through `new_blank_map`. `jam.Cut` set back to `hockey.regular`.
- Files changed under `Game/` are only in `Plugins/JamAssets/Content/Skaters/`, `Props/SM_PuckDisc`, `VFX/M_Fx_PuckBeacon` and `Look/MI_Fx_PuckBeacon`
  (new instance; `MI_Trim_PuckHalo` and every master untouched). `_Scratch/` does not exist.
- Another editor (port 8002) shared the GPU: no frame numbers.
- Scratch (`$SK4`): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk4`: every script
  (`kd.mjs` writes the Blueprint events for one figure, `ri.py` the import, `puck/` the puck kit), the cartridges, `m_*.json` polls, images in `img/`.

**How it was verified:** PIE, the real game. `jam.Cut` = scratch copies of the SK3 cartridge (`$SK4/b1.cartridge.json` `humanoid_b1`,
`b2.cartridge.json` `generated_b2`, `clay.cartridge.json` `p3_clay`), with `score_to_win` 9 and `penalties` false (every check on a non-carrier is a foul
and sent my checker to the box). **Bots never check** (`JamAICore.cpp:540` sets `bCheck = false`), so the knockdowns in the matches come from the human
skater driven through the real input stack (`UJamPlayerInput::DebugInjectKey`: WASD towards the nearest opponent, LeftShift within 5 m, slomo 0.12
while close). Test figures `SK4_*` in the same PIE world for the direction tests and the sheets.

## Verdict

| Item | T-B1 (scripted) | T-B2 hybrid (R) | P3 clay (P3A) | P4 broadcast (P4B) |
|---|---|---|---|---|
| 1 Import | **PASS** (70 takes) | **PASS** (68) | **PASS** (66) | **PASS** (68, Stride from the main file) |
| 1 Aim layer and correction after re-import | **PASS** (match) | **PASS** (match) | not re-measured | not re-measured |
| 2/4 Knockdowns by direction, hold, get-up | **PASS** (test + match) | **PASS** (test + match) | **PASS** (test) | **PASS** (test) |
| 2 Trip | Blueprint event only (no game call) | same, falls back to `Fall_Forward` | same | same |
| 3/4 Check delivered by side | **PASS** | **PASS** | **PASS** | **PASS** |
| 3/4 Hit, board, pin, stick-hit events | **PASS** (test figures) | **PASS** | **PASS** (Board_Hit substituted) | **PASS** |
| 5 Puck at distance | **PASS** night dark band; **PARTIAL** in the pools and on clay | | | |
| 6 Match proof, 12-frame sequence, sheet | **PASS** | **PASS** | sheet only (no tier key) | sheet only |
| Cook check (`BS_SkaterTB2_P4_Skate`) | **FIXED**, folder validates clean | | | |

## Direction convention (verified)

`SetKnockedDown(true, DirX, DirZ)`: **(DirX, DirZ) is the direction the victim is pushed, i.e. where it falls to** (rink x = Unreal X, rink z =
Unreal Y). Source: `UJamSim::CollideSkaters` passes the contact normal from checker to victim; its own foul test calls a check "from behind" when
`Dot(Dir, VictimForward) > 0.5`. **The take names say where the hit came from:** `Knockdown_Left` = hit from the left = falls to its right (engine FK:
head ends 45 cm to the figure's right on all four figures). PIE, figure facing +X: push (0, 1) → `Knockdown_Left`, lies on its right side; (0, -1) →
`Knockdown_Right`; (1, 0) → `Knockdown_Back` face down ahead; (-1, 0) → `Fall` on its back; (0.6, 0.8) → `Knockdown_Left` (stills `sk4-kd-dirs-tb1`,
`sk4-kd-gen`).

Pick: f = push · facing, r = push · the figure's right. f ≥ 0.707 → from behind (`Knockdown_Back`); f ≤ -0.707 → falls backward (`Fall`); else r > 0 →
`Knockdown_Left`, r < 0 → `Knockdown_Right`. The mesh turns over 6 frames so the fall runs along the push.

## Calls and events to takes

| Call / event (who sends it) | T-B1 (`skf/export4/p2_cel`) | R | P3A | P4B | Priority |
|---|---|---|---|---|---|
| `SetKnockedDown(true)`, from the left / right (pawn) | `Knockdown_Left` / `_Right` | same, `skh/export6/R` | same, `export6/P3A` | same, `export6/P4B` | 9, blocks all |
| `SetKnockedDown(true)`, from behind | `Knockdown_Back` | **`Fall_Forward`** (export6) | **`Fall_Forward`** (export6) | **`Fall_Forward`** (export6) | 9 |
| `SetKnockedDown(true)`, from the front | `Fall` | `Fall` (export6) | `Fall` (export6) | `Fall` (export6) | 9 |
| `SetKnockedDown(false)` after left / right | `GetUp_Side_Left` / `_Right` | same, `export5/R` | **`GetUp`** / `GetUp_Side_Right` (export5) | same, `export5/P4B` | 6 |
| `SetKnockedDown(false)` after behind / trip | `GetUp_Front` | `GetUp_Front` (export5) | `GetUp_Front` (export5) | `GetUp_Front` (export5) | 6 |
| `SetKnockedDown(false)` after front | `GetUp` | `GetUp` (export5) | `GetUp` (export5) | `GetUp` (export5) | 6 |
| `SetTripped(bDown, DirX, DirZ)` (BP event, nobody sends it) | `Trip` → `GetUp_Front` | **`Fall_Forward`** | **`Fall_Forward`** | **`Fall_Forward`** | 9 |
| `PlayCheck()` (pawn), nearest opponent on the right / left | `Check_Deliver_Right` / `_Left` | same (export5) | same (export5) | same (export5) | 4 |
| `PlayHitReaction(DirX, DirZ)` (BP event) from left / right / behind | `Check_Hit_Left` / `_Right` / `_Back` | same (export5) | same (export5) | same (export5) | 6 |
| `PlayBoardHit()` (BP event) | `Board_Hit` | `Board_Hit` (export5) | **`Check_Hit_Back`** | `Board_Hit` (export5) | 6 |
| `SetBoardPin(bool)` (BP event) | `Board_Pin` loop | `Board_Pin` (export6) | `Board_Pin` (export6) | `Board_Pin` (export6) | 6 |
| `PlayStickHit(bool bLifted)` (BP event) | `Lift_Hit` / `Poke_Hit` | same (export5) | same (export5) | same (export5) | 6 |
| `PlayPoke(false)` (pawn) | `Poke`, then `Stumble` (unchanged) | | | | 3 |

Everything else in the four figures is from `export5` (generated) and `export4` (scripted); P4B `Stride` is from the export5 **main** file.
Priorities: knocked down 9 > get-up and reactions 6 > foul 5 > shot / check 4 > poke / lift / pass 3 > draw 2; a one-shot interrupts only an equal or
lower one, nothing plays while down. `PlayHitReaction` uses the same direction convention as `SetKnockedDown` (the push); a hit from the front, which has
no take, plays the side take by the sign of r.

**Left unwired (not imported), per SKH's sixth pass "still not good enough":**
- R and P4B: `Knockdown_Back`, `Trip` (face-down skates 4.9 / 10.4 cm up). P3A: `Knockdown_Back`, `Trip` (torso 6 cm up), `Board_Hit` (172 cm stick pop at
  frame 11), `GetUp_Side_Left` (132 cm stick pop at frame 18).
- The fallback for the face-down falls is the figure's old `Fall_Forward` → `GetUp_Front`, which SKH lists with the same lift (it was already in the game).

**How it works (all four `BP_SkaterVisual_*`, events written by `$SK4/kd.mjs`):**
- The knockdown plays as a dynamic montage (blend in 0.06 s, no auto blend-out); `ReceiveTick` pauses it 0.08 s before its end, so the lying pose holds.
- `SetKnockedDown(false)` plays the matching get-up (blend in 0.12 s on T-B1, 0.3 s on the generated figures, see the seams below).
- **Timing:** the visual records the down time in game seconds on a skater's first knockdown. From then on it starts the get-up early, so the figure is
  standing when the sim releases it, and it time-scales the knockdown if the down time is under 0.55 s. The first knockdown of each skater gets up on the
  call. `GetUpRate` (default 1.0) scales the get-up.
- The ABP's old `bKnockedDown` / `bFallForward` states are no longer driven (the montage covers the whole body).
- `PlayCheck` finds the nearest visual of the other team (`PendingTeamId`, same class) and plays the delivery on that side.
- Each knockdown and get-up writes a log line (`SK4KD <take>`, `SK4UP <take> gap <s>`), which is how the match counts below were taken. It is a
  `PrintString` to the log only; remove it if the log should stay quiet.

## Timing between knockdown and get-up (as the pawn sends it)

- The sim keeps a skater down exactly `knockdown_frames` sim frames (`UJamSim::StepSkater`); default 90 = **1.5 s**, `hockey.raw`-style cuts 150 = 2.5 s.
- Measured on the visual at normal speed: **1.50 to 1.51 s** game time between the two calls (T-B1). Under my driver's slow motion the measured gap fell to
  1.0 s (the sim and the visual's clock disagree under `slomo`), so treat the 1.0 values in the logs as test artefacts.
- **Does a 0.9 s get-up fit?** Yes once the down time is known: knockdown 0.5 s + get-up 0.9 s = 1.4 s ≤ 1.5 s. On a skater's first knockdown, the
  get-up runs 0.9 s after the up call, while the sim already lets it skate.

## Match counts

| Figure | Knockdowns in the match (log) | Takes played |
|---|---|---|
| T-B1 (`humanoid_b1`), 3 runs | 10 | `Knockdown_Left` 2, `Knockdown_Right` 3, `Knockdown_Back` 3, `Fall` 2; each followed by its get-up |
| T-B2 (`generated_b2`), 2 runs | 3 | `Knockdown_Right` 1, `Fall` 2 (and get-ups) |

Total `JAMEVENT knockdown` lines in the session: 13. That is 13 `SK4KD` lines from game visuals; the other `SK4KD` lines are test figures.

12-frame sequences from the game camera: `sk4-seq-tb1`, `sk4-seq-tb2`. Sheet: `sk4-sheet` (four figures × three knockdowns and five reactions).

**Can you tell at the game camera who was hit from where?**
- **Yes for the knockdowns:** left, right and face-down read clearly on all four figures (`sk4-sheet`).
- **But the checker usually stands between the camera and the victim at the moment of impact.** The fall direction reads once the checker moves on
  (`sk4-seq-tb1` frames 9 to 12).
- **The stay-up reactions are subtle at 6 m:** `Check_Hit_*` and `Lift_Hit` are back near Glide within 0.3 s. `Board_Pin` (arms up) and `Board_Hit`
  read best.

## Calls device A does not send that would bring the rest to life

All proposed as new `BlueprintNativeEvent`s on `IJamSkaterVisual` (C-0001 v4), made from `AJamSkater::ApplySimFrame`; the visual already implements them as
Blueprint events with these names and parameters:

1. **`PlayHitReaction(float DirX, float DirZ)`**: on both opponents when two of them collide in `UJamSim::CollideSkaters` with closing speed
   `-Vn` above a threshold (say 3 m/s) **without** a landed check, or on a check that should not knock down (needs a sim rule, e.g. strength < 0.3).
   Dir = the push on that skater (the contact normal away from the other, as for `KnockDir`).
2. **`PlayBoardHit()`**: in `UJamSim::CollideSkaterStatic`, when a skater's normal speed into the boards `Vn` exceeds about 4 m/s (today only the puck
   emits `board`).
3. **`SetBoardPin(bool bOn)`**: true while a skater has been against the boards (`D + R > 0`) for 10 or more frames with an opponent within 1.2 m;
   false when either stops.
4. **`PlayStickHit(bool bLifted)`**: on the carrier who loses the puck to a clean poke (`ResolvePokes`, `PokeResult = 1` on the poker; the victim is
   the old `CarrierIndex()`), `bLifted = false`; and on the carrier stripped by `TryLift` (`O` in that function), `bLifted = true`.
5. **Trip** (`SetTripped(bool bDown, float DirX, float DirZ)`, or a `Cause` passed with `SetKnockedDown`): there is no trip in the sim. A foul that takes a
   leg would need a sim cause (e.g. a poke that hits the skater instead of the puck) and `KnockDown(..., CauseTrip)`.
6. **`PlayCheck` with a side** (`PlayCheck(float DirX, float DirZ)`): the visual works it out from the nearest opponent, which is right in
   practice. Passing the checker's contact normal would make it exact. `bCheckThisFrame` is set on the press, before contact.
7. Bots never press check (`In.bCheck = false`), so in a bot match none of this is seen without a human. Device A should let the AI check.

## Item 1: import, and the aim and correction re-measurement

- **T-B1:** 70 takes from `skf/export4/p2_cel/SK_Skater_p2_cel_loopcut.fbx`: 55 replaced in place, 15 new.
- **R / P3A / P4B:** `Knockdown_Left/_Right`, `Fall`, `Fall_Forward`, `Board_Pin` from `skh/export6/<c>/…_loopcut.fbx`; everything else from
  `export5/<c>/…_loopcut.fbx`.
- **P4B `Stride`, measured in the engine:**

  | File | Wrap step hand / blade (cm) | Mean step hand / blade (cm) |
  |---|---|---|
  | loopcut | 2.48 / 7.78 | 2.27 / 4.17 |
  | main | 0.00 / 0.00 (the last key repeats the first) | |

  The main file's take has the smaller wrap and is imported. Cost: one repeated frame per cycle.
- **Static holds:** the 24 holds imported as 30-frame static takes. The aim layer is unchanged and every aim blend space still lists its 24 samples.

**Match polls (aim layer on, no montage):**

| | T-B1 now | T-B1, SK3 | R now |
|---|---|---|---|
| Polls | 598 | 240 | 412 |
| Blade bottom p5 / median / p95 (cm) | -1.2 / 1.6 / 4.8 | -0.7 / 1.6 / 3.1 | -1.4 / 0.5 / 3.0 |
| Share within -1..+4 cm | 88 % | 93 % | 89 % |
| Glide / stride position error median (cm) | 1.4 / 0.5 | | 1.8 / 0.8 |

**Correction used, p90 (pitch / yaw / slide):**

| State | T-B1 now | T-B1, SK3 test rig max | R now | R, SK3 max |
|---|---|---|---|---|
| Crossover | 7.2° / 9.3° / 12.7 cm | 5.8° / 4.2° / 6.6 cm | 5.4° / 4.2° / 10.6 cm | 12.0 cm slide |
| Idle | 3.0° / 0.1° / 7.7 cm | 6.6° / 3.2° / 10.2 cm | 2.3° / 1.6° / 5.2 cm | 6.1° / 3.5° / 5.2 cm |
| Glide | 0.9° / 1.0° / 2.2 cm | | | |
| Stride | 2.3° / 2.0° / 4.3 cm | | | |
| Lean | 1.2° / 1.4° / 4.1 cm | | | |

**Did the correction's work get smaller?**
- **Hips are level in the engine.** T-B1 pelvis height: crossover 74.5..76.5 cm, lean 74.7..76.5 cm, glide 75..77 cm (SK3: 4 to 10 cm low).
- **Idle: yes, smaller.** Slide p90 7.7 cm against SK3's 10.2 max, pitch 3.0° against 6.6°. The idle pelvis still dips to 65.8 cm: `Idle_Faceoff` was not
  raised in Blender.
- **Crossover: not shown smaller.** The numbers are not comparable: match p90 against SK3's slow test-rig maxima. The crossover position error median is
  10.4 cm (T-B1) and 9.2 cm (R), the lag of a fast-turning carrier, as SK3 described.
- P3 and P4 were not re-measured: they cannot be in a match, and there was no time for the test rig.

## Item 5: the puck at distance

**Changed:**
- **`SM_PuckDisc`** rebuilt (Props, 24,386 bytes, was 32,710).
  - Slot 0 `Halo` (`MI_Trim_PuckHalo`, unchanged): the same 60 cm disc, now **domed**: 0.2 cm at the rim to 2.7 cm in the centre, which the 8 cm puck
    covers.
  - Slot 1 `Beacon`: **two crossed vertical cards**, 28 cm wide and 32 cm tall.
- **New master `VFX/M_Fx_PuckBeacon`** (unlit, additive, two-sided):
  - emissive = `Colour` × `Intensity` × (1 - V)^`GradientPower` × soft horizontal edge × **saturate((camera distance - `FadeStart`) / `FadeRange`)**;
  - defaults: 1500 cm, 1000 cm, colour (0.75, 1.0, 0.15).
- **New instance `Look/MI_Fx_PuckBeacon`**: Intensity 16, GradientPower 1.5.

**Stencil:** the game's `PuckDisc` component keeps stencil 0, no custom depth, no shadow (`AJamPuck::InitPuck`, unchanged). The cards are on that
component, so they are "emissive part, stencil 0" per the convention. The puck body keeps stencil 3. No code change is needed: the game picks the new mesh
up by path.

**Before / after (`sk4-puck-before-after`):**
- **Rig:** a stand-in puck built exactly as `InitPuck` builds it: the cylinder at 0.24 × 0.24 × 0.08 with `MI_Toon_Puck` and stencil 3, plus
  `SM_PuckDisc` 0.4 cm up. It sat at fixed spots in the PIE world. The game resets the real puck every frame, and moving it while paused froze the
  viewport.
- **Camera:** 3.5 m up, FOV 70, 6 / 24 / 40 m away.

| Spot and distance | Before | After |
|---|---|---|
| Dark band, 6 m | the halo ring | the same, with a slightly bigger glow (dome and bloom); the cards are faded out |
| Dark band, 24 m | 34 × 8 px: a 2 to 3 px bright line and the black puck | **34 × 14 px**: a glowing post over the halo |
| Dark band, 40 m | about 16 × 3 px | **about 16 × 8 px** post: findable |
| Pool, 24 / 40 m | the yellow halo vanishes on near-white ice; only the black puck reads (about 6 × 3 px at 40 m) | the same: additive yellow cannot show on white ice. The puck reads as a dark dot, as before |
| Clay, 24 m (`sk4-puck-tiers`, left) | | **weak**: the clay puck body renders pale, so the halo and post are the only cue on cream ice: a faint yellow-green smudge |
| Broadcast rig, 24 m (`sk4-puck-tiers`, right) | | black puck plus halo and post: reads |

## Cook check

- **Fixed:** `BS_SkaterTB2_P4_Skate` had two samples with no animation: (speed 5, turn 0) and (8, 0), the `Stride` samples. Both now point at
  `AS_SkaterTB2_P4_Stride`.
  - The cause was the `Stride` take being replaced: the SK3 state already had it, and my main-file swap also goes through a consolidate.
- **Sample lists, read back after the fix:**

  | Blend space | Samples | Empty |
  |---|---|---|
  | `BS_SkaterTB1v2_Skate`, `BS_SkaterTB2_Skate`, `BS_SkaterTB2_P3_Skate`, `BS_SkaterTB2_P4_Skate` | 14 each | 0 |
  | the four `BS_*_Aim` | 24 each | 0 |

  Skate samples on every figure: (0, 0) and (1, 0) Glide; (5, 0) and (8, 0) Stride; (0, ±3) and (1, ±3) Glide; (3, ±3) Lean_Left / _Right; (5, ±3) and
  (8, ±3) Crossover_Left / _Right. The aim samples are the `Aim_<N|P><angle>_R<reach>` grid.
- **Whole folder:** every asset under `/JamAssets/Skaters/` was loaded (318 assets, 0 failures) and run through `EditorValidatorSubsystem.validate_assets_with_settings`:
  - 318 checked, 318 valid, 0 invalid, 0 warnings;
  - no Error or Warning line in the editor log during the load or the validation;
  - no `Accessed None` or Blueprint runtime error in the log of the whole session.

## Assets created or changed (bytes)

**Created:**

| Asset | Bytes |
|---|---:|
| 15 `Skaters/AS_SkaterTB1v2_<new>` (`Board_Hit`, `Board_Pin`, `Check_Deliver_Left/_Right`, `Check_Hit_Back/_Left/_Right`, `GetUp_Side_Left/_Right`, `Knockdown_Back/_Left/_Right`, `Lift_Hit`, `Poke_Hit`, `Trip`) | 2,366,958 |
| 13 `Skaters/AS_SkaterTB2_<new>` (the same without `Knockdown_Back`, `Trip`) | 2,522,758 |
| 11 `Skaters/AS_SkaterTB2_P3_<new>` (without `Knockdown_Back`, `Trip`, `Board_Hit`, `GetUp_Side_Left`) | 2,151,567 |
| 13 `Skaters/AS_SkaterTB2_P4_<new>` (without `Knockdown_Back`, `Trip`) | 2,562,677 |
| `VFX/M_Fx_PuckBeacon` | 16,622 |
| `Look/MI_Fx_PuckBeacon` | 6,949 |

**Changed:**

| Asset | Bytes | What |
|---|---:|---|
| `Skaters/BP_SkaterVisual_TB1` | 1,711,491 | falls and collisions events, `ReceiveTick` hold, learned get-up timing, check side; 16 variables |
| `Skaters/BP_SkaterVisual_TB2` | 1,685,566 | same |
| `Skaters/BP_SkaterVisual_TB2_P3` | 1,685,168 | same |
| `Skaters/BP_SkaterVisual_TB2_P4` | 1,685,977 | same |
| `Skaters/BS_SkaterTB2_P4_Skate` | 28,146 | Stride samples re-pointed (cook error) |
| `Props/SM_PuckDisc` | 24,386 | domed halo and beacon cards, 2 slots |
| 55 old `AS_SkaterTB1v2_*` | (70 files 9,892,313 incl. new) | re-imported in place from `skf/export4/p2_cel` |
| 55 old `AS_SkaterTB2_*` | (68 files 11,545,121 incl. new) | re-imported in place (`export6` falls, rest `export5`) |
| 55 old `AS_SkaterTB2_P3_*` | (66 files 11,358,186 incl. new) | same |
| 55 old `AS_SkaterTB2_P4_*` | (68 files 11,761,489 incl. new) | same; `Stride` from the export5 main file |

**Ledger rows to add** (format of `ASSETS.md`; one row per take, pattern shown for each set):

```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_<Take>.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export4/p2_cel loopcut, falls and collisions take, animation-only import (lane SK4) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_<Take>.uasset | anim | 0.19 | original | Blender 4.2 Python script (lanes SKF, SKH) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf p2_cel take retargeted onto R, skh/export5/R (Knockdown_Left/_Right, Board_Pin: export6) loopcut, animation-only import (lane SK4) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P3_<Take>.uasset | anim | 0.20 | original | Blender 4.2 Python script (lanes SKF, SKH) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | skf p3_clay take retargeted onto P3A, skh/export5/P3A (Knockdown_Left/_Right, Board_Pin: export6) loopcut, animation-only import (lane SK4) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB2_P4_<Take>.uasset | anim | 0.20 | original | Blender 4.2 Python script (lanes SKF, SKH) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | n | skf p4_broadcast take retargeted onto P4B, skh/export5/P4B (Knockdown_Left/_Right, Board_Pin: export6) loopcut, animation-only import (lane SK4) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckBeacon.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | unlit additive two-sided glow card, fades in with camera distance (FadeStart 1500, FadeRange 1000 cm) (lane SK4) |
| Game/Plugins/JamAssets/Content/Look/MI_Fx_PuckBeacon.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Fx_PuckBeacon, Intensity 16, GradientPower 1.5; slot 1 of SM_PuckDisc (lane SK4) |
```

Take lists per set: T-B1 the 15 new names above; R and P4 the 13; P3 the 11.

**Ledger rows to update (notes only):**
- `SM_PuckDisc`: "domed halo 0.2 to 2.7 cm + two crossed 28 x 32 cm glow cards (slot 1, MI_Fx_PuckBeacon), 24386 bytes (lane SK4)".
- `AS_SkaterTB1v2_*` (55): "re-imported from skf/export4/p2_cel (lane SK4)". `AS_SkaterTB2*` (165): "re-imported from skh/export5, falls and
  Board_Pin from export6 (lane SK4)"; `AS_SkaterTB2_P4_Stride`: "export5 main file (smaller wrap)".
- `BP_SkaterVisual_*` (4): "+ falls and collisions (knockdown by direction, held, matching get-up, learned timing; PlayHitReaction, PlayBoardHit,
  SetBoardPin, PlayStickHit, SetTripped; check side) (lane SK4)". `BS_SkaterTB2_P4_Skate`: "Stride samples re-pointed (lane SK4)".

**Rows to remove:** none.

## Blender-side faults seen in the engine

1. **Seams between the settled export6 knockdowns and the export5 side get-ups** (largest bone distance, last frame to first frame):
   R `Knockdown_Left` → `GetUp_Side_Left` 20.4 cm, `_Right` 8.2 cm; P4B 31.2 / 39.4 cm; P4B `Fall_Forward` → `GetUp_Front` 4.5 cm. T-B1 0.0 on all pairs.
   Covered by a 0.3 s blend on the generated figures. The source fix: settled get-ups whose first frame equals the settled knockdown, without the
   sixth-pass blade lag.
2. **`Idle_Faceoff` hips still about 10 cm low** (T-B1 pelvis 65.8 cm in the match): the correction still slides the stick 7.7 cm (p90) at rest.
3. **Reactions are small at the game camera:** `Check_Hit_*`, `Lift_Hit` and `Poke_Hit` are hard to read at 6 m once 0.2 s have passed. A bigger torso
   throw and a longer extreme would help.
4. No take in the engine went below the ice by eye. No new pops were seen in the takes wired here. The P3 pops are in takes I did not wire.

## What I could not verify

- **P3 and P4 in a match:** there is still no tier key, so they were tested on test figures only.
- **The aim and correction numbers for P3 and P4.**
- **Trip, board hits, pins, stick hits and hit reactions in a match:** device A sends none of them; they were tested on test figures from Python.
- **The learned early get-up across many knockdowns in a match.** It was verified on a test figure (down time learned 1.35 s; the second get-up started
  at 0.5 s). In the match I saw only one knockdown per skater per run at normal speed.
- **Frame cost** (a shared GPU), and `GetAllActorsOfClass` in `PlayCheck` (6 actors, on the check frame only).
- **The puck kit on the real moving puck at 24 to 40 m:** the stills used the stand-in, built exactly as `InitPuck` builds it. The trail was not
  re-checked.

## What the lead must decide

1. Ask device A for the calls above, in this order: `PlayStickHit` and `PlayHitReaction` (cheap, from existing sim state), `PlayBoardHit` /
   `SetBoardPin`, then a trip cause. Also let bots check, or nobody falls in a bot match.
2. The face-down knockdown and `Trip` on the generated figures stay on `Fall_Forward` until the knee and ankle rule lands (SKH). P3's `Board_Hit` and
   `GetUp_Side_Left` wait for the stick-continuity fix.
3. **Puck in the pools and on clay:** the additive post cannot read on white or cream ice. The options are a dark outline ring or a darker clay puck body
   (a `Look/` master change, which was not mine). On night, the dark band is now clearly better at 24 and 40 m.
4. **The early get-up:** keep it (the figure is up when control returns, from the second knockdown on), or have A pass the down time (`knockdown_frames`)
   so the first get-up is right too.
5. Remove the `SK4KD` / `SK4UP` log lines before shipping if a quiet log is wanted.
6. Paste the ledger rows.
