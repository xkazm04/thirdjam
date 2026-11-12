# SKI3 report: the shipped figure's clay version in the engine (device B)

Lane SKI3, 2026-11-12, 22:21 to 22:48 machine clock.
- Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8002. It started at 22:22 and was closed through `ExitEditor` at 22:47 with no dirty package.
- The last world was an untitled blank map. `Rink` was discarded through `new_blank_map(False)`; `Game/Content/Maps/Rink.umap` is still dated 12:40.
- No build, and no git command except a read-only `git log`.
- Another editor (port 8000) shared the GPU, so there are **no frame numbers**.
- Scratch scripts: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/ski3` (`$SKI3`).
- Checkpoint: `status/SKI3-checkpoint.md`.

**Scope change at 22:50 (operator, through the coordinator): clay only.**
- The broadcast figure was **not started. No `TB1_P4` asset exists** (checked on disk).
- The clay materials got the operator's felt-and-clay push (section 2).

**Housekeeping**
- Every file I created is under `Game/Plugins/JamAssets/Content/Skaters/`, and its name contains `TB1_P3` (85 files). A `find -newer` over `JamAssets/Content` lists nothing else.
- I modified no existing asset. I read and duplicated the TB1 assets; `BP_SkaterVisual_TB1` and `ABP_SkaterTB1` were never dirty.
- `_Scratch/` does not exist. `jam.Cut` is set back to `hockey.regular`.
- Test figures and mirror figures existed only in the PIE world (spawned with `summon`), never in the editor world.

## Verdict: `BP_SkaterVisual_TB1_P3` (p3_clay)

| Step | Result | Evidence |
|---|---|---|
| 1 Import | **PASS** | `SK_SkaterTB1_P3` with its own `SKEL_SkaterTB1_P3`: 24 bones, root identity, 163.7 cm tall, faces +X. **72 takes** (`AS_SkaterTB1_P3_*`) and `SM_SkaterTB1_P3_Stick`. Sockets as on the cel figure. Details in section 1. |
| 2 Materials by slot name | **PASS** | Team colour on Jersey and Helmet. Number on a felt patch on the back, crest on the chest. Helmet, gloves, pads and skates in clay. Face, eyes and stick never take the team colour. Details in section 2. |
| 3 AnimBP and Blueprint | **PASS** | Duplicates, not child classes (section 3). Every call and reaction works as on `BP_SkaterVisual_TB1` tonight. The measurements below match or beat the cel figure. |
| 4 On its tier in the real game | **PASS** | A real match on `p3_clay` (`JAMLOOK tier=p3_clay`). The six game visuals were mirrored onto `TB1_P3` figures attached to the same pawns (method below). Stills at the game camera, 8 m and 24 m, in both colours, beside the cel figure in the same frozen frame. Strips of stride, shot, poke, knockdown and get-up. |
| Validation | **PASS** | 85 assets loaded. `validate_assets_with_settings`: 85 checked, 85 valid, 0 invalid, 0 warnings. `BS_SkaterTB1_P3_Skate` has 14 samples, 0 empty; `BS_SkaterTB1_P3_Aim` has 24, 0 empty. `BP_SkaterVisual_TB1_P3` and `ABP_SkaterTB1_P3` recompile to `BS_UP_TO_DATE` with no warning. No Warning or Error line in the editor log during load, validation or compile. |

## 1. Import

**Source and scripts**
- Source: `skf/export5/p3_clay/SK_Skater_p3_clay_loopcut.fbx`, the same kind of file TB1v2 uses.
- Import settings are lane SK2's: no materials, no physics asset, exported time.
- Scripts in `$SKI3`: `imp_p3.py` (the import), `addsock.mjs` (sockets through `SkeletalMeshToolset.AddSocket`), `sock.py` (socket transforms).

**Sockets**
- `stick_socket` and `head_socket` sit on their bones. `head_socket` is world-identity at rest (roll -90, as on TB1v2).
- `puck_socket` is on bone `stick_socket` at (5.62, -146.74, 9.46): 5 cm beyond the blade centre, at puck height, in `Aim_Front`.
- No socket compensation, because the root is identity.

**Blade centre in the `stick_socket` frame** (`fitbc.py`)
- Fitted by least squares over the 24 aim holds at frame 0, so that the blade centre lands on each hold's asked point.
- Checked on the cel figure first: the fit returns (5.57, -149.63, 10.47) against SK3's constant (5.6, -149.57, 10.5), residual 0.02 cm.
- **P3: (5.13, -140.72, 9.65)**, largest residual 0.03 cm. P3 grips 10 cm below the knob, with 1.30 m from grip to heel and a 56-degree lie. With the cel figure's constant, the P3 error would be 7.8 cm.
- **`H0` = 4.6.** The median blade-centre height over the holds is 4.62 cm; the cel figure's 4.46 cm gave its 4.5 by the same rule.

P3's stick blade is in the **Trim** slot (a cream, tape-like blade); the shaft is in the Stick slot.

## 2. Materials: clay, felt, number, crest

Slot order on the mesh: Pads, Jersey, Trim, Skin, Helmet, Gloves, Skate, Blade, Stick. Jersey (index 1) and Helmet (index 4) take the team colour; the helmet gets x0.75, as on the cel figure.

| Slot | Material | What it does |
|---|---|---|
| Jersey | `MI_SkaterTB1_P3_JerseyDeco` (`M_SkaterTB1Jersey`) | **Team colour.** **Crest** at Y -10, Z 113, size 10 (the P3 chest; the cel figure's is at Z 120). **Number off** on the jersey (`NumberBackMask` -2), because the raised felt patch in the geometry covers the back. **Felt push**: `RimStrength` 1.2, which doubles the felt rim on clay only (the rim is gated by RimStrength/0.6; night uses `RimNight`, broadcast 0); `Roughness` 1. |
| Trim | `MI_SkaterTB1_P3_TrimDeco` (`M_SkaterTB1Jersey`) | **Cream (0.9).** Covers the felt patch, the hem, collar, sleeve and sock rolls, **the seams' cross-stitches** and the stick blade. **Back number** through UV0, centred on the patch: `NumberCentreV` 0.344 (z 111 cm, at spine_03, the patch centre), width 0.30, height 0.50, so 16 cm digits on a 21 x 26 cm patch. UV0 on P3: u = 0.025y + 0.5, v = (122 - z)/32, in cm, from `build_skater.py`. Crest off. |
| Helmet, Gloves, Pads, Skate | `MI_SkaterTB1_P3_<slot>` (`M_Toon`) | **Clay.** The cel figure's colours, plus a clay dent bump: `ClayNormalTex` = `T_Clay_Dent_N` (the texture the boards use), `ClayTileMul` 2, `ClayBumpMul` 0.6. Clay roughness and the felt rim come from `MPC_Look`. |
| Skin, Blade, Stick | `MI_SkaterTB1_Skin` / `_Blade` / `_Stick` (referenced, unchanged) | The M_Toon clay path, as on the cel figure (thumb bump, `ClayRough`). |

**Blueprint**
- The jersey MID is made from index 1 and the helmet MID from index 4; both indices are the same on P3.
- `SetNumber` and BeginPlay also write `NumberTens` and `NumberOnes` on the Trim-slot MID (index 2).
- Result in the match: numbers 7, 13, 42, 9, 21 and 88 all appeared on the patches (`ski3-clay-game-p3`), as a dark-outlined digit centred on the cream patch, inside its dark running stitch.

### Against the operator's felt-and-clay concept

I looked at `prep-reference/images/path-p3-t1.png`; nothing was imported from it.

- **Stitched seams: already cream thread.**
  - The P3 export puts the cross-stitches in the **Trim** slot (cream) and only the seam ridge in Jersey (`build_skater.py`, lines 408 and 416).
  - So the hem and collar stitch rows read as cream dashes at the game camera and at 8 m (`ski3-clay-game-p3`, `ski3-clay-8m-concept`). No mask was needed.
- **Felt jersey: partly done.** It has the stronger felt rim and is fully matte.
  - **A fine fibre bump cannot be set from an instance.** `M_SkaterTB1Jersey` has no normal-texture parameter: it was duplicated from `M_Toon` before B14 added `ClayNormalTex`. And `T_Felt_Fibre` is a greyscale mask, not a normal map.
  - The fix needs one of two things:
    - a master change (`M_SkaterTB1Jersey` gains `ClayNormalTex`, `ClayTileMul` and `ClayBumpMul`) plus a felt-fibre **normal** map, converted from B9's fibre mask the way B9 converted the thumb map;
    - or modelled fuzz from the Blender lane.
  - Not done: editing an existing master was outside this lane.
- **Helmet, gloves, pads and skates** are clay with the dent bump. The dents are visible on the helmet at the game camera.
- **"Soft sheen" is not possible per slot.** The clay path forces every M_Toon surface's roughness to `MPC_Look.ClayRough` (0.85); a sheen per slot needs a master parameter.
- **The honest gap to the concept:**
  - The concept's jersey is fuzzy wool with visible fibres and white sleeve and sock bands; ours is smooth matte felt with a rim. The knit texture does not match.
  - What matches the concept's spirit: the face, the helmet's ear guards and the cream trims.

## 3. Duplicates, not child classes, and why

**`ABP_SkaterTB1_P3`** (`$SKI3/abp.py`)
- A duplicate of `ABP_SkaterTB1`, with `target_skeleton` = `SKEL_SkaterTB1_P3`.
- Every sequence player, sequence evaluator and blend-space player is re-pointed by name (`AS_SkaterTB1v2_*` → `AS_SkaterTB1_P3_*`, `BS_SkaterTB1v2_*` → `BS_SkaterTB1_P3_*`):
  - Idle, Fall, Fall_Forward, GetUp, GetUp_Front;
  - Faceoff_Ready, Goalie_Hold, Goalie_Block_Left and _Right;
  - the skate blend space, the four aim players and the Shot_Slap evaluator.
- Unchanged: the graph, the variables, the layered blend from `spine_01` and the `stick_socket` Modify Bone correction.

**`BS_SkaterTB1_P3_Skate` and `_Aim`** (`bs.py`): new blend spaces on the P3 skeleton. The TB1v2 layouts were copied property by property: axes, interpolation, triangulation preference and samples.

**`BP_SkaterVisual_TB1_P3`** (`dsl3.mjs`, `bpfin.py`): a duplicate of `BP_SkaterVisual_TB1`, with its own DSL rewritten:
- the ABP casts and variable sets now target `ABP_SkaterTB1_P3`;
- the 29 take literals point to the P3 takes;
- the jersey MI is the P3 jersey;
- the self-class references (`GetAllActorsOfClass` and the cast in `PlayCheck`) are `BP_SkaterVisual_TB1_P3`;
- the `SK3Fix` blade offset is (5.13, -140.72, 9.65), and the Trim slot carries the number;
- the mesh component uses `SK_SkaterTB1_P3` with `ABP_SkaterTB1_P3_C`; the class default `H0` is 4.6.

Read back: 0 references to `TB1v2` or `ABP_SkaterTB1`, 77 to `TB1_P3`. The class implements `JamSkaterVisual`.

**Why not child classes:**
- `BP_SkaterVisual_TB1` casts to the concrete `ABP_SkaterTB1` in `UpdateMotion`, `SK3Fix` and `PushAnim`, and its events play 29 takes as **literals**. A child Blueprint can override neither.
- A child AnimBP keeps its parent's skeleton.
- Child classes would need three changes to `BP_SkaterVisual_TB1`:
  - an anim interface instead of the cast;
  - the takes as class-default variables (a take table);
  - the blade offset as a variable.
- Until then: **re-run `dsl3.mjs` and `bpfin.py` after every change to `BP_SkaterVisual_TB1`** (about one minute).

## 4. Behaviour against `BP_SkaterVisual_TB1`

**The same as the cel figure.** It is the same graph, so the same calls, priorities, reactions and timing, take for take:
- the stumble cut 0.1 s after the poke;
- knockdown rate 0.95 + 0.2 x strength, read from the event bus;
- the learned get-up, the 0.12 s reaction blend and the divide guard.

**Differences, all deliberate:**
- P3 takes and P3 blend spaces: P3's own proportions and solved holds, including six one-handed far-right holds and the flatter-lie stick with the grip 10 cm below the knob.
- Blade offset (5.13, -140.72, 9.65) and `H0` 4.6, against (5.6, -149.57, 10.5) and 4.5.
- The number sits on the Trim-slot patch, not on the jersey; the crest is 7 cm lower.
- `PlayCheck` looks for the nearest opponent of class `BP_SkaterVisual_TB1_P3`. That is right when the whole match uses this class (the tier figure); it is blind to opponents in a mixed match.

**The reaction calls are not in the C++ interface in this tree.**
- `git log -3 -- Game/Source/JamCore/Public` shows 99a152b, 15b8ba1 and b6d8166. No header has `PlayHitReaction`, `PlayStickHit` or `PlayBoardHit`: lane A45 has not landed here.
- As on the cel figure, `PlayHitReaction`, `PlayStickHit`, `PlayBoardHit`, `SetBoardPin` and `SetTripped` are Blueprint events.
- When A45 lands, B's move of these events on `BP_SkaterVisual_TB1` must be repeated here, or the figure re-derived with `dsl3.mjs`.

## Measurements

PIE on the clay tier, `$SKI3/meas.py`, 8 samples per figure.

**Aim layer.** 36 test figures stood outside the rink and were driven every tick, as in SK5. Figures were placed at the 15 cell centres: angles -54.5, -38, -15, 17.5 and 52.5 degrees, reaches 1.15, 1.375 and 1.525 m.

| | TB1_P3 Glide | TB1 Glide (SK5) | TB1_P3 Stride | TB1 Stride (SK3) |
|---|---|---|---|---|
| Blade error to the asked point | **0.0 cm** (max 0.0) | 0.0 | **0.3 cm** max (mean 0.24) | 2.8 |
| Blade bottom above the ice | **1.0 cm** everywhere | 1.6 | **0.8 to 1.3 cm** | 0.4 to 2.6 |
| Correction used, max pitch / yaw / slide | 3.5 deg / 3.4 deg / 2.0 cm | 3.7 / 3.1 / 2.7 | 3.9 / 4.1 / 4.3 | 4.0 / 3.7 / 4.7 |

"Bottom" is the blade-centre height minus 3.6 cm on P3. That calibration comes from the hold centre at 4.6 cm and SKF's hold blades at 0 to 2 cm up. The TB1 columns subtract 2.9 cm.

**Hands.** Left wrist joint to the shaft line, in Glide, at the exact two-handed holds. The cel figure measures 4.3 to 7.0 cm, and the clay mitten is about 6 cm in radius, so both hands are on the stick.

| Hold | Wrist to shaft (cm) |
|---|---|
| P00 / R100 | 7.0 |
| P70 / R100 | 7.0 |
| N63 / R100 | 6.6 |
| P35 / R130 | 6.3 |
| N30 / R145 | 6.3 |
| P00 / R160 | 5.8 |

**Carry, real match.** 12 consecutive frames on two carries: #9 aiming 7 degrees at 1.6 m, and #21 at -12 to -14 degrees at 1.6 m.
- **Game puck to the P3 blade centre: 4.6 to 5.1 cm** (design 5 cm). The cel figure measured 5.5 to 5.7 cm in SK2 and 3.0 to 6.5 cm in SK3.
- Blade bottom: 0.8 to 1.3 cm.
- Caveat: the game draws the puck at the *hidden cel figure's* `puck_socket`, because the pawn still holds that visual. So this measures P3's blade against the puck where the game puts it. P3's own `puck_socket` was 2.6 to 2.9 cm from that puck.

**Found on the way (it applies to the cel figure as well).**
- The skeletal mesh uses `ONLY_TICK_POSE_WHEN_RENDERED`. Off screen the pose freezes, but `SK3Fix` keeps integrating against the frozen socket.
- Its corrections then run into the clamps (12 deg, 12 deg, 15 cm), and the figure re-enters view with full correction for a few frames.
- My test figures read this way until I set `ALWAYS_TICK_POSE_AND_REFRESH_BONES` on them (PIE only).
- In a match it affects only figures behind the camera.
- The fix: skip `SK3Fix` when `WasRecentlyRendered` is false, in both Blueprints. Not made, because the cel figure is not mine.

## How the figure got into the real game (step 4)

Why a workaround was needed:
- Device A's game cannot select this class yet: `ResolveVisualClass` maps `humanoid_b1` to `BP_SkaterVisual_TB1` on every tier.
- `ResetVisual` cannot be called from a script.
- Python cannot spawn actors through `GameplayStatics` in this build.

The route (`$SKI3/mirror.py`):
1. **Cartridge** `ski3.p3_clay.cartridge.json`: `hockey.regular` with `look.tier p3_clay`, `skater.visual humanoid_b1` and first to 9. Set through `jam.Cut`, then PIE. The log shows `JAMVISUAL humanoid_b1 tier=p3_clay class=.../BP_SkaterVisual_TB1_C` and `JAMLOOK tier=p3_clay`.
2. **One clay figure per game visual.** For each of the six game visuals, a `TB1_P3` figure is spawned into the PIE world with the console command `summon`, attached to the same `JamSkater` pawn, and given its `SetTeam` and `SetNumber`. The cel figure is hidden but keeps posing, so the game's puck stays on its blade.
3. **A Python post-tick callback mirrors the calls every frame:**
   - `UpdateMotion`, with Speed and TurnRate read from the cel figure's ABP and its facing;
   - `SetStickAim`, `SetShotCharge`, `SetFaceoff`, `SetGoalieStance`;
   - `SetKnockedDown`, with the push rebuilt from the cel figure's `DownKind` and `DownYaw`;
   - the one-shots, from the take the cel figure starts: Poke (with its clean flag), Shot_Slap or Shot_Wrist, Pass (with S from the play rate), Lift, Foul_Reaction, Check_Deliver (→ `PlayCheck`), Faceoff_Draw.
   - The P3 figure runs its own Blueprint logic from those calls. It takes the knockdown strength from its own event-bus binding, which works because it is attached to the pawn.
   - Seen in the match log: Shot_Wrist, Lift, Poke and Foul_Reaction. No knockdown: bots do not check in this tree.
4. **Strips:** a `TB1_P3` figure on the ice, driven from Python with `PlayShoot`, `PlayPoke(false)`, `SetKnockedDown(true, 0, 1)` and `SetKnockedDown(false)`, at slomo 0.1 to 0.25.

## Stills (`devlog/shots/`)

| File | What |
|---|---|
| `20261112-224644-B-ski3-clay-game-p3.jpg` | Game camera, clay tier, frozen match: TB1_P3, with #7 on the felt patch. |
| `20261112-224645-B-ski3-clay-game-cel.jpg` | The same frame with the cel figure (what clay shows today). |
| `20261112-224645-B-ski3-clay-8m-blue.jpg`, `-8m-orange.jpg` | 8 m, both colours: TB1_P3 on the left, the cel figure on the right, same frame. |
| `20261112-224645-B-ski3-clay-24m-blue.jpg`, `-24m-orange.jpg` | 24 m, same pairing. |
| `20261112-224645-B-ski3-clay-8m-concept.jpg` | TB1_P3 at 8 m, to set beside the concept `prep-reference/images/path-p3-t1.png` (two files, not composited). |
| `20261112-224646-B-ski3-strip-stride.jpg`, `-strip-shot.jpg`, `-strip-poke.jpg`, `-strip-knockdown.jpg` | Strips of 8, 8, 8 and 12 frames. The knockdown strip's crop cuts off the lowest lying frames. |

## Against the cel figure on the clay tier (honest)

- **Game camera: the clay figure is better.**
  - From behind it reads as a hand-made toy: a cream felt patch with a stitched edge and the number on it, cream stitch rows on the hem, ear guards, a dented helmet and fat rolled trims.
  - The cel figure on clay is a smooth plastic toy with a painted number.
  - Where the cel figure is better: its number is bigger and a little easier to read (painted large on the jersey, against a 16 cm digit on the patch).
- **8 m: clay wins clearly.**
  - The face, the helmet dents and the stitches read. The cream blade reads like taped wood on cream ice.
  - The cel figure's cage and knee pads read more "hockey"; the clay figure reads more "clay miniature", which is the tier's point.
- **24 m: a draw.**
  - The tier's depth of field (f/3.2, focus 11 m) blurs both figures into coloured blobs.
  - **The geometry's clay detail (dents, stitches) does not survive at 24 m.** Only the cream patches (clay) or the white numbers (cel) remain as marks.
  - Team colours separate equally well.
- **Depth of field at the game camera:** the player's own figure is in focus at about 6.5 m and keeps its dents and stitches. Figures beyond about 20 m lose them.

## Assets created (85 files, 15,841,412 bytes, all in `Game/Plugins/JamAssets/Content/Skaters/`)

| Asset | Bytes |
|---|---:|
| `SK_SkaterTB1_P3` | 1,928,839 |
| `SKEL_SkaterTB1_P3` | 12,187 |
| `SM_SkaterTB1_P3_Stick` | 86,730 |
| 72 x `AS_SkaterTB1_P3_*` | 11,026,455 |
| `BS_SkaterTB1_P3_Skate` / `_Aim` | 27,906 / 51,724 |
| `ABP_SkaterTB1_P3` | 644,611 |
| `BP_SkaterVisual_TB1_P3` | 1,994,837 |
| `MI_SkaterTB1_P3_JerseyDeco` / `_TrimDeco` | 12,074 / 12,684 |
| `MI_SkaterTB1_P3_Helmet` / `_Gloves` / `_Pads` / `_Skate` | 10,986 / 10,782 / 10,767 / 10,830 |

**TB1_P4: none exist** (not started, per the 22:50 scope change).

Ledger rows: 85, ready to paste, at the end of this report and in `$SKI3/rows.md`.

## For device A's class-per-tier table

| Visual + tier | Class |
|---|---|
| `humanoid_b1` + `p3_clay` | **`/JamAssets/Skaters/BP_SkaterVisual_TB1_P3.BP_SkaterVisual_TB1_P3_C`** (fallback `BP_SkaterVisual_TB1_C`) |
| `humanoid_b1` + `p4_broadcast` | stays `/JamAssets/Skaters/BP_SkaterVisual_TB1.BP_SkaterVisual_TB1_C`: there is no broadcast figure, and the tier's look is back in experiments |
| `humanoid_b1` on every other tier | `BP_SkaterVisual_TB1_C` (unchanged) |

## What I could not verify

- **The game selecting the class itself.** It needs A's `ResolveVisualClass` entry. In my test the game's calls reached the clay figure through the mirror, one frame late, and the puck was drawn at the hidden cel figure's socket.
- **A knockdown, a check or a stick reaction in a match** on the clay figure. Bots do not check in this tree, and A45's calls are not here. Knockdown and get-up were shown only on the strip figure, driven from Python.
- **Colours and numbers on the strip figure.** The figure summoned for the strips ignored `SetTeam` and `SetNumber` (its `JerseyMID` was None, the symptom SK2 reported for test figures), so the strips show the default blue #7. Game-path figures took colours and numbers correctly: both colours show in the stills.
- **Frame cost** (shared GPU): six figures of about 12k triangles.
- **The fibre bump on the jersey**: it needs a master change (section 2).
- **The night tier on the clay figure** (not asked). It would show the jersey's `EmissiveNight` fill, as the cel figure does.

## What the lead must decide

1. **Send A the table row above** (`humanoid_b1` + `p3_clay` → `BP_SkaterVisual_TB1_P3_C`).
2. **The felt jersey and the concept's white bands.**
   - For fuzz: allow a `Skaters/` master edit (`M_SkaterTB1Jersey` gains `ClayNormalTex`, `ClayTileMul` and `ClayBumpMul`, default off) plus a felt-fibre normal map, or ask the Blender lane for modelled fuzz.
   - The concept's white sleeve and sock bands would need a Blender colour-zone change (moving them to the Trim slot).
3. **Duplicates or child classes.** Keep the duplicate and re-derive it with `dsl3.mjs` after every TB1 change, or refactor `BP_SkaterVisual_TB1` to an anim interface plus a take table, so that tier figures become children.
4. **`SK3Fix` off screen:** gate it on `WasRecentlyRendered` in both Blueprints. It is cheap and removes the run-away into the clamps.
5. **Paste the 85 ledger rows.** `SM_SkaterTB1_P3_Stick` is imported but unreferenced, as `SM_SkaterTB1v2_Stick` was: keep it or delete it.

## Ledger rows (85)

```
| Game/Plugins/JamAssets/Content/Skaters/ABP_SkaterTB1_P3.uasset | other | 0.64 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | duplicate of ABP_SkaterTB1 on SKEL_SkaterTB1_P3, every player re-pointed to the P3 takes and blend spaces (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_FarReach.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Front.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Left.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N30_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N46_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_N63_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P00_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P35_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R100.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R130.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R145.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_P70_R160.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Aim_Right.uasset | anim | 0.12 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Hit.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Board_Pin.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Celebrate.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Deliver_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Back.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Check_Hit_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Crossover_Left.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Crossover_Right.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Draw.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Faceoff_Ready.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Fall_Forward.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Foul_Reaction.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Front.uasset | anim | 0.21 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Left.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_GetUp_Side_Right.uasset | anim | 0.20 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Glide.uasset | anim | 0.24 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Goalie_Block_Left.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Goalie_Block_Right.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Goalie_Hold.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Idle.uasset | anim | 0.26 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Idle_Faceoff.uasset | anim | 0.23 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Back.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Left.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Knockdown_Right.uasset | anim | 0.17 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lean_Left.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lean_Right.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Lift_Hit.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Pass.uasset | anim | 0.14 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Poke.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Poke_Hit.uasset | anim | 0.16 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Shot_Charge.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Shot_Slap.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Shot_Wrist.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stop.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stride.uasset | anim | 0.19 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Stumble_Short.uasset | anim | 0.15 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/AS_SkaterTB1_P3_Trip.uasset | anim | 0.18 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay loopcut, own skeleton SKEL_SkaterTB1_P3 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 1.99 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-12 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB1_P3_Aim.uasset | anim | 0.05 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | 24-hold aim grid at true sample positions, layout of BS_SkaterTB1v2_Aim on the P3 takes (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/BS_SkaterTB1_P3_Skate.uasset | anim | 0.03 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | 14-sample skate blend space, layout of BS_SkaterTB1v2_Skate on the P3 takes (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_Gloves.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Toon, colours of the TB1 slot, clay dent bump T_Clay_Dent_N TileMul 2 BumpMul 0.6 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_Helmet.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Toon, colours of the TB1 slot, clay dent bump T_Clay_Dent_N TileMul 2 BumpMul 0.6 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_JerseyDeco.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_SkaterTB1Jersey: crest at Z 113, back number off (BackMask -2, the patch carries it), RimStrength 1.2 felt rim on clay, Roughness 1 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_Pads.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Toon, colours of the TB1 slot, clay dent bump T_Clay_Dent_N TileMul 2 BumpMul 0.6 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_Skate.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_Toon, colours of the TB1 slot, clay dent bump T_Clay_Dent_N TileMul 2 BumpMul 0.6 (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_TrimDeco.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-12 | B | y | instance of M_SkaterTB1Jersey on the Trim slot: cream felt patch, rolls, cross-stitches; back number via UV0 centred on the patch (CV 0.344, W 0.30, H 0.50), crest off (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/SKEL_SkaterTB1_P3.uasset | other | 0.01 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skeleton of the p3_clay import, root identity (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/SK_SkaterTB1_P3.uasset | mesh | 1.93 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay SK_Skater_p3_clay_loopcut.fbx, 11.6k tris, own skeleton; sockets stick/head on their bones, puck_socket on stick_socket 5 cm beyond the blade; materials by slot name (lane SKI3) |
| Game/Plugins/JamAssets/Content/Skaters/SM_SkaterTB1_P3_Stick.uasset | mesh | 0.09 | original | Blender 4.2 Python script (lane SKF) + UE 5.9 via MCP/Python | | | 2026-11-12 | B | y | skf/export5/p3_clay SM_Stick_p3_clay.fbx (cm); imported, not attached (the stick is skinned into SK_SkaterTB1_P3) (lane SKI3) |
```
