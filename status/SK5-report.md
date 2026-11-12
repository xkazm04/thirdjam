# SK5 report: polish on the shipped figure, haze, puck on white and cream ice, glass hit, jersey, motion blur (device B)

Lane SK5, 2026-11-12, 20:36 to 21:45 machine clock. Main tree `C:/Users/kazda/jam/thirdjam`, editor on port 8000 (pid 6924, mine), left through `ExitEditor` with no dirty package (last world: an untitled blank map). No build. One read-only git command at the coordinator's request (`git show origin/main:handoff/A-to-B-0011.md`). Checkpoint: `status/SK5-checkpoint.md`.

**Housekeeping**
- `/Game/Maps/Rink` was never saved. My test figures, stand-in pucks and camera, tagged `SK5`, were discarded through `new_blank_map(False)`.
- `_Scratch/` does not exist. Cvars set back: `jam.Cut hockey.regular`, `jam.ScriptHuman 0`.
- No `NS_` system was touched by me.
- Another editor shared the GPU, so I took no frame numbers.
- Scratch (`$SK5`): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/sk5`. It holds every script (`bp5.mjs`, `bp5b.mjs`, `guard.mjs` for the Blueprints; `ri_tb1.py` the import; `mark.py` and `disc5.py` the puck kit; `glass.py`; `jersey.py`; `haze.py` and `mblur.py` the rig; `mdrv.py` the match driver), the backups in `bk/`, and the full-size images in `img/`.

**Warning: an outside write reverted `BP_SkaterVisual_TB1` on disk at 20:58:08.**
- The file went back to SK4's exact 1,711,491 bytes, with none of my events in it.
- At the same moment (20:57:57 to 20:58:08), `VFX/MI_Fx_Chip/_Puff/_Ring/_Spark/_Star`, `M_Fx_Tier`, `NS_blade_spray` and `NS_pickup_idle` were rewritten in this tree.
- **I did not write any of those files.** It looks like a sync or checkout from outside the editor.
- My editor still held my version in memory. I re-saved it at 21:40 and confirmed my events are in the file on disk.
- **Whoever syncs this tree must not overwrite `Skaters/BP_SkaterVisual_TB*` again.**

## Verdict

| Item | Result |
|---|---|
| 1 Re-import (72 takes) + aim/correction re-measure | **PASS** |
| 2a Poke -> `Stumble_Short` at 6 frames | **PASS** (match + test) |
| 2b Rest on `Idle`, hips level | **PASS** |
| 2c Trip -> hold -> `GetUp_Side_Right` | **PASS** (test call; nobody sends a trip) |
| 2d Front knockdown direction | **PASS**: already correct, unchanged |
| 2e Knockdown strength -> play rate 0.95..1.15 | **PASS** (bus binding, match) |
| 2f Reaction events + snap fix | **PASS** (0.12 s blend-in, not the 2-frame one: see below) |
| 3 Haze on | **PASS**: strength 0.8 kept |
| 4 Puck on white and cream ice | **PASS** broadcast; **PARTIAL** clay (depth of field); night unchanged |
| 5 Glass `HitWobble` | **PASS** |
| 6 Proof | **PASS** (match strips; the poke strip is a test figure) |
| 7 Jersey night fill 2.5 (coordinator) | **PASS**: night-only master gate |
| 8 Motion blur off on the night rig (coordinator) | **PASS** |
| 9 Divide by zero in the knockdown path (coordinator) | **PASS**: fixed on all four figures |
| Validation `/JamAssets/Skaters/` | **PASS**: 320 loaded, 320 valid, 0 invalid, 0 warnings, every blend space full |

## 1. Re-import, aim and correction

- **Import:** `skf/export5/p2_cel/SK_Skater_p2_cel_loopcut.fbx`, animation only. 70 takes replaced in place, 2 new (`Idle` 1.983 s loop, `Stumble_Short` 0.333 s = 20 frames), 0 failures.
- **Blend spaces:** `BS_SkaterTB1v2_Skate` 14 samples / 0 empty, `_Aim` 24 / 0 empty.
- **Re-measure on 15 test figures** at the cell centres in Glide, driven every tick in PIE, 8 samples each:
  - blade error 0.0 cm, blade bottom 1.6 cm everywhere;
  - correction used, max: pitch 3.7 deg, yaw 3.1 deg, slide 2.7 cm (SK3: 3.5 / 3.1 / 2.4).
  - Same as SK3 and SK4 reported.

## 2. `BP_SkaterVisual_TB1` and `ABP_SkaterTB1`

**(a) Stumble timing.**
- `PlayPoke(false)` plays `Poke`. Once 0.1 s of game time has passed, a new event `PlayCut` cuts to `Stumble_Short` with blend 0, at the phase it should be at.
- Match log: `JAMEVENT poke ... frame=272 kind=miss`, then `SK5POKE false f 32933`, then `SK5STUMBLE dt 0.1 f 32939`. **The cut lands 6 frames after the call.**
- The stumble runs to frame **+26**. The sim's stumble runs from +0 to +20, so **the visual ends 6 frames after the sim**. To end together, `Stumble_Short` would have to start at the poke frame itself.

**(b) Rest.**
- The AnimBP's `Idle` state now plays `Idle` (it was `Idle_Faceoff`).
- **Correction at rest** (6 figures × 8 samples): pelvis 76.5..77.0 cm (it was 65.8). Max correction 2.9 deg pitch, 2.2 deg yaw, **1.9 cm slide** (it was 7.7 cm p90 in SK4's match, 10.2 cm max on SK3's rig).
- **Faceoff:** the faceoff pose is `Faceoff_Ready` through `bStFace` and is unchanged. At the opening faceoff the centres stand at pelvis 67 cm and the wingers on `Idle` at 76.8 cm (`sk5-faceoff-rest`).
- `Idle_Faceoff` is now played by nothing; I kept the asset.

**(c) Trip.** `SetTripped(true)` plays `Trip` and holds; `SetTripped(false)` plays `GetUp_Side_Right` (it was `GetUp_Front`).

**(d) Front hit.** A push against the facing plays `Fall`. The figure keeps facing the checker and falls backward along the push, away from the checker. This is device A's "back to the push", so I did not change it.

**(e) Strength.**
- The Blueprint now binds `JamEventBus.OnJamEvent`. A `knockdown` event whose Who is its own skater's sim index (`GetAttachParentActor` cast to `JamSkater`) stores the strength.
- The knockdown then plays at **0.95 + 0.2 × strength**, multiplied by SK4's learned time-scale.
- The event arrives before `SetKnockedDown` in the same frame. The handler also re-rates a knockdown that has already started.
- Match: strength 1.0 → rate 1.15; 0.89 → 1.13; 0.17 → 0.98.

**(f) Reaction events, tested from Python.**
- **What each event plays now:**

  | Event | Plays | Blend-in |
  |---|---|---|
  | `PlayHitReaction`, push from behind / left / right | `Check_Hit_Back` / `_Left` / `_Right` | **0.12 s** |
  | `PlayStickHit(false)` | `Poke_Hit` | **0.12 s** |
  | `PlayStickHit(true)` | `Lift_Hit` | 0.08 s |
  | `PlayBoardHit` | `Board_Hit` | 0.08 s |
  | `SetBoardPin(true/false)` | `Board_Pin` loop / stop | – |
  | `PlayCheck` | `Check_Deliver_Left` / `_Right`, by the nearest opponent's side | – |

- **The snap is in the engine.** Measured per frame on the bones at frame 3 to 4 of the take:
  - clavicle 47 to 66 deg in one frame on all three `Check_Hit_*` (`Check_Hit_Left` and `_Right` are worse than `_Back`);
  - `Poke_Hit` stick 72 to 80 deg.
- **A 2-frame blend-in does not help** (52 deg), because the step is inside the take, not at its start.
- **A 0.12 s blend-in** brings it to 27 to 30 deg (stick 46 deg), close to the neighbouring frames, and the held extreme (frames 5 to 8) still reads.

**Divide by zero (item 9).**
- The cause: `0.55 / LastGapS` inside a `SelectFloat`, which evaluates both branches. `LastGapS` is 0 until a skater's first get-up.
- The fix: guarded with `FMax(LastGapS, 0.1)`.
- **Shared edit:** the same guard went into `BP_SkaterVisual_TB2`, `_TB2_P3` and `_TB2_P4`, which share SK4's function.
- Before: 6 `Divide by zero: Divide_DoubleDouble` lines in this session. After: 4 knockdowns in a match, 0 such lines, and no `Accessed None` or runtime error.

## 3. Haze on (after 21:00)

- B15's switch: `Haze_L2_Night_1..2` are no longer hidden in game. The rig was saved at 21:07; B16's method, after discarding Rink.
- Frozen frame (seed 1, t 2.16 to 2.18) at 10 / 24 / 40 m plus the game camera, before and after (`sk5-haze-24-game`, `sk5-haze-10-40`).
- **Luma at 24 m:**

  | Box | Before | After |
  |---|---:|---:|
  | Above the far boards | 20.8 | 101.3 |
  | Sky high | 10.6 | 39.1 |
  | **Far net** | 73.3 | **73.2** |
  | Far orange figure | 116.2 | 122.9 |

  Game camera: far net 138.6 → 140.7, band above it 76.1 → 88.5.
- **Verdict:** the puck, both teams and the far net still read in front of the far card. The net sits on the ice below the card's bright band. **`MI_HazeCard_End` Strength stays 0.8.**

## 4. The puck on white and cream ice

**What was added.**
- **`SM_PuckDisc`** now has 4 slots:
  - `Halo` and `Beacon`, unchanged;
  - **`Ring`**: a dark annulus 25 to 35 cm, 0.3 cm over the dome;
  - **`Marker`**: two crossed dark cards, 10 cm × 45 cm.
- **`VFX/M_Fx_PuckMark`** (new): unlit, masked, two-sided, colour (0.035, 0.045, 0.07). Its opacity mask is 1 only when **|`MPC_Look.Clay`| > 0.5** and the camera is beyond `FadeStart`.
- Instances: `Look/MI_Fx_PuckRing` (`FadeStart` 0), `Look/MI_Fx_PuckMarker` (`FadeStart` 1200 cm).
- **Night is pixel-identical** before and after: the mask is 0 when Clay is 0.

**Stills:** stand-in puck built as `InitPuck` builds it; dark-band and pool spots; 6 / 24 / 40 m; tiers through `jam.livecut` scratch cartridges (`sk5-puck-night-before-after`, `sk5-puck-clay-bcast`).

| Tier | Verdict |
|---|---|
| Night | Unchanged. |
| Broadcast | **Clear win.** A ring at 6 m; a dark post on the puck at 24 m and 40 m, where before there was only a speck. |
| Clay | **Partial.** The ring reads at 6 m. The post helps at 24 m. At 40 m the tier's depth of field (f/3.2) blurs puck, post and all into the cream. |

**Puck body on clay.**
- At 6 m the body reads luma 38 on clay against 9 on night, as a dark warm brown. `MI_Toon_Puck` is already black (0.012).
- The lift comes from the clay path of `M_Toon`: the roughness goes to the clay target, and the halo's yellow is reflected in the body. The "pale" look at distance is the depth of field mixing the puck into the ice.
- **There is no instance value that fixes it, so I left it.** A master change, not made: a `ClayKeepDark` term in `ClayMixBaseColour` / `ClayMixRoughness` that keeps near-black bases black and glossy on clay. Or exclude the puck from the clay depth of field.

**Night carrier at 24 m (A-to-B-0011).** In the real match, at 24 m, the orange carrier in the dark side band has the halo and glow card on its blade (`sk5-carry-24`). **The puck and its carrier can be found at a glance.**

## 5. Glass `HitWobble`

- `M_Glass` gained scalar **`HitWobble`** (0..1, default 0), wired into the `Glass` custom node with `Time`.
- At 0 the output is exactly as before: the new terms are multiplied by 0.
- At 1:
  - a diagonal bright sweep moving along the panel at 3 bands per second;
  - the top edge 6× brighter;
  - +0.6 base glow;
  - opacity +0.45 in the sweep and +0.3 at the top;
  - no vertex motion.
- Overridden at 0 on `MI_Toon_Glass` and `MI_Bcast_Glass`. `MI_Glass`, the one the game uses at night, inherits it.
- Still pair: `sk5-glass-hitwobble`.

## 6. Proof (`devlog/shots/20261112-2138*-B-sk5-*`, `...214357-B-sk5-motionblur`)

| Still | What |
|---|---|
| `sk5-strip-check-left` | Match: `Check_Deliver_Left`, victim `Knockdown_Back`, held, early `GetUp_Front` |
| `sk5-strip-front-fall` | Front check at strength 0.89: `Fall` away from the checker at rate 1.13, held |
| `sk5-strip-poke` | Test figure: `Poke`, cut to `Stumble_Short`, back to Glide |
| `sk5-faceoff-rest` | Rest at the faceoff |
| `sk5-jersey-24`, `sk5-carry-24`, `sk5-haze-*`, `sk5-puck-*`, `sk5-glass-hitwobble`, `sk5-motionblur` | Items 3, 4, 5, 7, 8 |

**Not shown:**
- A poke strip in a match: my driver's injected left-mouse poke never registered for the human after the first session. The match poke is proven by the log above.
- A rest still after the whistle.

## 7. Jersey night fill (A-to-B-0011)

**Why a gate and not the instance value.**
- The Blueprint writes `Emissive` 2.0 (boost 3.0) on the jersey dynamic instance at `SetBoost`, so an instance value alone never reaches the game.
- `SK3EmissiveGate` passes it straight through on clay, so 2.5 would have leaked into clay.

**Change.**
- New master scalar **`EmissiveNight`** on `M_SkaterTB1Jersey`, default 0 = off.
- At night (|Clay| < 0.5) the fill becomes Orig + (`EmissiveNight` - 2.0). Broadcast stays 0, clay is unchanged.
- `MI_SkaterTB1_JerseyDeco` `EmissiveNight` = **2.5**: night fill 2.5, boost 3.5.
- `M_SkaterTB2` was not touched.

**Dark side band at 24 m, saturated cloth pixels, sRGB Rec.709 luma (`sk5-jersey-24`):**

| Jersey | Before luma | After luma | RGB before → after |
|---|---:|---:|---|
| Blue | 19.3 | **49.9** | (5, 11, 140) → (4, 51, 176) |
| Orange | 58.2 | **88.0** | (126, 43, 12) → (171, 71, 11) |

The orange keeps its hue.

## 8. Motion blur

- `PPV_L2_Night`: `override_motion_blur_amount` True, amount 0.5 → **0**; `override_motion_blur_max` True, max 5 → **0**.
- **Separate save:** this went in a second save at 21:42 (the coordinator's message came after the haze save). Both changes are in the rig: the haze cards were read back as shown.
- Game camera during a forced hard turn: smeared before, crisp after (`sk5-motionblur`; not the same instant).

## Validation of `/JamAssets/Skaters/`

- After all edits: 320 assets loaded, 0 failures.
- `validate_assets_with_settings`: 320 checked, 320 valid, 0 invalid, 0 warnings.
- Every blend space full: 4 Skate × 14 samples, 4 Aim × 24 samples, 0 empty.
- No Error or Warning line from the load or the validation.

## Assets created or changed (bytes, `Game/Plugins/JamAssets/Content/`)

**Created:**

| Asset | Bytes |
|---|---:|
| `Skaters/AS_SkaterTB1v2_Idle` | 240,455 |
| `Skaters/AS_SkaterTB1v2_Stumble_Short` | 145,622 |
| `VFX/M_Fx_PuckMark` | 11,938 |
| `Look/MI_Fx_PuckRing` | 6,487 |
| `Look/MI_Fx_PuckMarker` | 6,499 |

**Changed:**

| Asset | Before | After | What |
|---|---:|---:|---|
| `Skaters/AS_SkaterTB1v2_*` (70) | 72 files 10,320,451 total, incl. new | | re-imported from `skf/export5/p2_cel` |
| `Skaters/BP_SkaterVisual_TB1` | 1,711,491 | 1,940,533 | `PlayCut`, `OnBusEvent` (bus binding), stumble cut, trip get-up, strength rate, 0.12 s reactions, divide guard; vars `HitStr DownRate StumbleT0` |
| `Skaters/BP_SkaterVisual_TB2` | 1,685,566 | 1,688,066 | divide guard only (shared function) |
| `Skaters/BP_SkaterVisual_TB2_P3` | 1,685,168 | 1,687,686 | same |
| `Skaters/BP_SkaterVisual_TB2_P4` | 1,685,977 | 1,688,460 | same |
| `Skaters/ABP_SkaterTB1` | 649,067 | 648,729 | `Idle` state plays `Idle` |
| `Skaters/M_SkaterTB1Jersey` | 54,536 | 55,212 | `EmissiveNight` night gate |
| `Skaters/MI_SkaterTB1_JerseyDeco` | 8,270 | 9,876 | `EmissiveNight` 2.5 |
| `Props/SM_PuckDisc` | 24,386 | 28,212 | + `Ring`, `Marker` slots |
| `Look/M_Glass` | 19,728 | 21,134 | `HitWobble` |
| `Look/MI_Toon_Glass` | 7,025 | 10,233 | `HitWobble` 0 exposed |
| `Look/MI_Bcast_Glass` | 7,799 | 11,573 | `HitWobble` 0 exposed |
| `Rigs/LR_L2_Night.umap` | 417,811 | 417,991 | haze cards shown; `PPV_L2_Night` motion blur 0 |

**Ledger rows to add:**
```
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Idle.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p2_cel loopcut, standing idle with hips at Glide height, 2 s loop, animation-only import (lane SK5) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1v2_Stumble_Short.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p2_cel loopcut, 20-frame stumble starting at Poke frame 6, animation-only import (lane SK5) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckMark.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | unlit masked two-sided dark puck mark, shown only when abs(MPC_Look.Clay) > 0.5 and the camera is beyond FadeStart (lane SK5) |
| Game/Plugins/JamAssets/Content/Look/MI_Fx_PuckRing.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Fx_PuckMark, FadeStart 0; slot Ring of SM_PuckDisc (lane SK5) |
| Game/Plugins/JamAssets/Content/Look/MI_Fx_PuckMarker.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Fx_PuckMark, FadeStart 1200 cm; slot Marker of SM_PuckDisc (lane SK5) |
```

**Ledger rows to update (notes only):**
- `AS_SkaterTB1v2_*` (70): "re-imported from skf/export5/p2_cel (lane SK5)".
- `BP_SkaterVisual_TB1`: "+ short stumble cut, Idle at rest, trip get-up side, knockdown strength from the event bus, 0.12 s reaction blend, divide guard (lane SK5)".
- `BP_SkaterVisual_TB2`, `_TB2_P3`, `_TB2_P4`: "+ divide guard on the learned time-scale (lane SK5)".
- `ABP_SkaterTB1`: "Idle state plays Idle (lane SK5)".
- `M_SkaterTB1Jersey`: "+ EmissiveNight night-only fill gate (lane SK5)". `MI_SkaterTB1_JerseyDeco`: "EmissiveNight 2.5 (lane SK5)".
- `SM_PuckDisc`: "+ dark ring 25-35 cm (MI_Fx_PuckRing) and 10 x 45 cm dark marker cards (MI_Fx_PuckMarker), clay and broadcast only, 28212 bytes (lane SK5)".
- `M_Glass`: "+ HitWobble 0..1, default 0 (lane SK5)". `MI_Toon_Glass`, `MI_Bcast_Glass`: "HitWobble 0 exposed (lane SK5)".
- `LR_L2_Night`: "... 2 end haze cards shown (lane SK5), motion blur off in PPV_L2_Night (lane SK5)".

**Rows to remove:** none.

## What device A must know

1. **Strength:**
   - `SetKnockedDown`'s direction is normalised (`KnockDir.GetSafeNormal()`), so it carries no strength.
   - B's figure now reads the `knockdown` event from `JamEventBus.OnJamEvent`, matching Who to `JamSkater.GetSimIndex()` of its attach parent.
   - **This relies on the event being broadcast before `ApplySimFrame` in the same frame**, as it is today. The handler also covers the reverse order.
2. **The stumble ends 6 frames after the sim's:** visual +6..+26 against sim +0..+20.
3. **Glass is a single `Glass` mesh component** on `RinkActor_0`. A dynamic instance with `HitWobble` = 1 lights **all** the glass, not the panel nearest the hit.
   - To localise it, A needs per-panel components, or I add `HitCentre` / `HitRadius` parameters (one more lane edit).
   - The sweep moves with `Time`, so a 12-frame pulse shows about 0.6 of a band pass.
4. **No calls arrived that are not in `B-ASKS-2.md`.** `PlayHitReaction`, `PlayBoardHit`, `SetBoardPin`, `PlayStickHit` and `SetTripped` are still sent by nobody in the match.
5. **`jam.ScriptHuman 1` silently overrides `DebugInjectKey`:** no check or poke landed until I set it back to 0.
6. **Clay puck:** the depth of field (f/3.2) hides the puck beyond about 30 m whatever its kit is. That is a camera / post choice.

## What I could not verify

- **The poke-to-stumble cut in a strip from a real match.** It is in the log (6 frames); the strip is a test figure.
- **A rest still after the whistle.**
- **The trip in a match:** nobody sends `SetTripped`.
- **Frame cost** (the GPU was shared).
- **The motion-blur before and after at the same instant:** the frames are different turns.
- **The puck kit on clay at 40 m beyond the depth of field.**
- **Cooked behaviour of the bus binding.** It works in PIE; Blueprint delegate binding to a game-instance subsystem should cook as it is.
- **Whoever reverted the Blueprint at 20:58, and whether the `VFX/` and `NS_` files that changed then are as their owners want them.**

## What the lead must decide

1. **Find and stop the process that rewrote `Skaters/BP_SkaterVisual_TB1` and `VFX/*` in this tree at 20:58.** My saves at 21:40 are the right version.
2. **Stumble:** accept the 6-frame overrun, or start `Stumble_Short` at the poke frame itself (it would then skip `Poke`'s jab).
3. **Reaction blend 0.12 s, or ask Blender for a 3-frame rise** on `Check_Hit_*` and `Poke_Hit`, which halves the step at the source.
4. **The clay puck:** a `Look/` master term (`ClayKeepDark`), or exclude the puck from the clay depth of field.
5. **Glass:** per-panel components (A) or a `HitCentre` parameter (B) to localise the hit.
6. **Remove the `SK5POKE` / `SK5STUMBLE` / `SK5KDSTR` and `SK4KD` / `SK4UP` log lines** before shipping if a quiet log is wanted.
7. Paste the ledger rows.
