# B's asks, round 2 (lane A31, 2026-11-12 19:00 to 20:30): what each handoff gets as its answer

Ruling R-0027 point 4. Handoffs B-to-A-0019, 0021, 0023, 0024, 0025 (all `accepted` by the Director at 18:45; the Director writes the `## Response` blocks,
this file is the lane's evidence). Worktree `.claude/worktrees/a31` at the dev-b merge f89014c; nothing under `Game/Plugins/JamAssets` touched; no git command run.
Stills: `devlog/shots/20261112-194109-A-a31-*.jpg` (captions in the `.txt` next to each). Eyes: `docs/experiments/eyes/a31-tier-strip-bughunt.json`.

## 1. B-to-A-0021: a figure per look tier, jersey numbers

Answer: **done, as asked, with the respawn option.**

- `AHokejGameMode::ResolveVisualClass(Visual, Tier)`: `generated_b2` picks `BP_SkaterVisual_TB2_P3` on `p3_clay`, `_TB2_P4` on `p4_broadcast`, `_TB2` on every other tier;
  a class that fails to load falls to `_TB2`, then to the toy figure. One log line per resolve: `JAMVISUAL generated_b2 tier=p3_clay class=/JamAssets/Skaters/BP_SkaterVisual_TB2_P3.BP_SkaterVisual_TB2_P3_C`.
- A tier change during a match (a Director's Call through `ApplyCartridgeLive`, or the new console command `jam.livecut <cut>`) **respawns the visuals in place with the setup calls
  repeated** (`AJamSkater::ResetVisual`: the old component or attached actor is destroyed, the new one created, `SetTeam`, `SetNumber`, `SetHasPuck`, `SetBoost` sent again, the
  per-frame state cleared so knocked-down / puck / faceoff / goalie stance / boost are re-sent by the next step). Log: `JAMVISUAL respawned 6 skater visuals as ... for tier p3_clay at frame 668 (setup calls repeated)`. On a running match the new figures animate at once (`respawn-unpaused.jpg`, frame 553); in the paused strip stills they sit in B's initial crouch because the AnimBP had not ticked yet (a proof-method artefact, not a match one).
- Numbers: `SetNumber` once at spawn, team 0 gets **7, 13, 42**, team 1 **9, 21, 88** by slot (`AHokejGameMode::JerseyNumberFor`); the human's skater keeps its slot's number
  (slot 0 of team 0 = 7 in the shipped cuts); a fourth skater and beyond keep the old `team * 10 + slot + 1`. `SetTeam(TeamId, Colour)` unchanged.
- Acceptance 1 (PIE, `generated_b2` + `p3_clay`): the log names `BP_SkaterVisual_TB2_P3` and six clay figures skate: `clay-figure-match.jpg`, probe `skaters: BP_SkaterVisual_TB2_P3_C x6`, `MPC_Look.Clay 1.0`.
  Acceptance 2 (a number on every back): `tier-strip-broadcast.jpg` (7 on the human's P4 figure), `pickup-props.jpg` (7, 13, 42 on the skeletal toy's backs); on `humanoid_b1` the
  number is drawn by B's figure from the same call (`b1-allstar-match.jpg`).
- UI textures: as the Director answered (A18's icon kit stays; `T_Crest_*` and `T_Numbers_Varsity` stay, the five `T_Glyph_*` copies can go).

## 2. B-to-A-0024: shot calls, bot charge, goalie stance

Answer: **points 1 to 3 done; point 4 was already live (reach 1.2 since cf315c2); point 5 is section 1.**

- Point 2: `AJamSkater::ApplySimFrame` fires `PlayShoot` **only** on `bShotThisFrame`; a pass is `PlayPass(Strength)` alone. Log (new): `JAMPASS frame=137 skater=0 team=0 strength=0.47 human (PlayPass only, no PlayShoot)`.
- Point 1: the bots charge. `FJamAIInput::bShootHeld` is the level the sim's `StepCharge` reads; the brain holds it for `FJamBotSkill::ShotChargeFrames` (**pro 10**, the default of
  `FJamBotSkill`, so the human's teammates charge 10 at every level; **all_star 18**; **rookie 0**, a tap) while its lined-up shot on the net stands (`bAimed` out of `Attack`), then
  drops it and the sim fires the charged shot on the release; the hold is also dropped the frame the line-up is lost. Board chips, dumps under the human's forecheck and the pass-by-shot
  to the human stay taps on purpose. The sim's charge rule applies to every skater (`speed + charge x charge_speed_bonus`, charge = held frames / `shot.charge_frames_max`). The headless
  gates see it too (`JamSimAdapter` copies `bShootHeld`); `shot_charge_frames` is a new `-aitune` / `--tune-<level>` key for the sweep.
  Proof: `JAMSHOT frame=3044 skater=5 team=1 charge=0.23 bot`, `JAMSHOT frame=4810 skater=4 team=1 charge=0.07 bot` (all_star, `humanoid_b1`, 95 s PIE bot match); at pro the four
  shots of a 55 s window were taps (pro's aimed shot needs the puck inside reach + 13 m; chips and dumps are taps by design).
- Point 3: the brain exposes `bGoalie` and `GoalieSide` (-1 left / 0 centre / +1 right, the carrier's side seen from the goalie facing up-ice; Unreal frame: team 0 faces +x, left is -y)
  from its `LastManGoalie` rule; the game mode (the brain's owner, as C-0001 v3 says) calls `SetGoalieStance(bOn, Side)` on every change, with a 24-frame minimum hold once on
  (the crease rule flickered on/off every 3 frames at its edge in the 19:58 proof). Log: `JAMGOALIE frame=299 skater=2 on=1 side=1`, `... frame=312 skater=2 on=0 side=0`,
  `frame=4021 skater=5 on=1 side=-1`.
- **The curve moved more than 5 points, so the profiles stay as they are (A3's worktree for a re-tune).** Same binary, same 20 seeds, `tools/sweep/run.mjs --runner ue`,
  charge on vs the control with `shot_charge_frames=0` on both sides:

  | rink | level | human win %, charge off (control) | charge on | delta | shots AI opp (off -> on) |
  |---|---|---|---|---|---|
  | regular | rookie | 100 | 100 | 0 | 5.7 -> 5.8 |
  | regular | pro | 55 | **40** | -15 | 8.2 -> 8.9 |
  | regular | all_star | 30 | 35 | +5 | 10.0 -> 10.1 |
  | large | rookie | 100 | 100 | 0 | 4.8 -> 4.8 |
  | large | pro | 45 | **35** | -10 | 4.8 -> 5.0 |
  | large | all_star | 30 | **15** | -15 | 9.2 -> 7.8 |

  Reading: pro and large all_star opponents score more with a 10 / 18 frame wind-up (faster pucks); regular all_star moves up a little (an 18-frame hold is time for the human's poke).
  Rookie is identical in both runs, so its 100 against A3's 85 (60 seeds) is the 20-seed sample, not this change. `Jam.Slow.DifficultyCurve` (5 seeds, rookie beats all_star) still
  passes on both rinks; `Jam.Bot` 6/6 and `Jam.Core` 11/11 green in the worktree (headless 19:17, 19:22 and 19:43 runs, the last on the final build; see `status/A31.md`).
- The default figure: the bake-off (A30) decides; B's acceptance still of `humanoid_b1` in a bot match is `b1-allstar-match.jpg` / `b1-pro-match.jpg`.

## 3. B-to-A-0023: floor, glass, stanchions, rim, CornerRadiusCm

Answer: **points 1 to 5 done; B can remove its 66 `Con_` stanchions and put the rig floor back to -5 cm.**

- Point 1: `ULookTierSwitcher::LowerFloor` makes the floor's component Movable before the move and reads the bounds back:
  `JAMLOOK floor StaticMeshActor_0 lowered by 15.0 cm to a top of -20.0 cm (bounds read back: top -20.0 cm, mobility Movable)`; the PIE probe reads `top_cm -20.0, MOVABLE` on both rinks and every tier.
  B's rig floor (`StaticMeshActor_13..16` at -4 cm on the night rig, `StaticMeshActor_34` at -3 cm on clay) can go back to -5 cm.
- Point 2: the glass carries `/JamAssets/Look/MI_Glass` (`MI_Bcast_Glass` on `p4_broadcast`), set by `ARinkActor::SetGlassForTier` at Build and pushed by `SetTier` on a live
  change: `JAMRINK glass MI_Glass for tier l2_night`, `JAMRINK glass MI_Bcast_Glass for tier p4_broadcast`. (`MI_Toon_Glass`, the name A25 loaded, never existed, so the net placeholder had been standing in.)
- Point 3: option two confirmed. `RinkActor` already spawns the glass stanchions with the glass (GlassRail section 1): 5 cm posts from the board top (1.2 m) to the rail top
  (2.1 m + 4 cm), one every **3.6 m** along the outline (73 on 90 x 45, 98 on 120 x 60; the count rounds so the ring closes), built from the rink config, so they never scale
  with the rig. On `hockey.arena.large` the PIE footprint check lists only B's `Con_L2_Night_Stanchion_59..` (at |x| 59.5 m / |y| 27.6 m, the scaled rig's board line) as actors
  inside the boards: those are the 66 to remove. Still: `rigcheck-large-night.jpg`.
- Point 4: `RimStrength` 1.3 on the toy figure's instances (`ToySkaterVisual.cpp`) and on the skeletal toy's (`SkeletalSkaterVisual.cpp`), both A's; TB1/TB2 are B's.
- Point 5: `CornerRadiusCm` is set on the dynamic ice instance from `game.rink.corner_radius_m` (375 on the regular rink, 500 on the large one, whatever a cut says).
- Point 6: no change on A's side to the puck kit attachment.

## 4. B-to-A-0025: tier `p4_broadcast`

Answer: **done; frame cost, runtime switch and large-rink scale measured.**

- `p4_broadcast` in `ULookTierSwitcher` (`GetKnownTiers` ladder order, map `/JamAssets/Rigs/LR_P4_Broadcast.LR_P4_Broadcast`, trim Emissive 30), `schema/game.schema.json`,
  `limits.json`, `README.md`, `DEFAULTS.md`, `hockey.test.mjs` (95/95), `docs/HOCKEY.md`, the embedded schema (`gen-schema-data.mjs`), settings label **BROADCAST (P4)**,
  cook line `+MapsToCook=(FilePath="/JamAssets/Rigs/LR_P4_Broadcast")`.
- `SetTier` resets `MPC_Look.Clay` to 0 before streaming any rig (`ResetLookCollection`), after the previous rig's unload request; `ULookTierSwitcher::GetLookClay()` reads the value.
  PIE: night **0**, clay **1**, broadcast **-1**, and back to night **0** (the dark bands return) with the live switch, on both rinks.
- Runtime switch between the four tiers (a Director's Call through `jam.livecut` on a paused match at frame 668): night -> clay -> broadcast on `hockey.regular`, night -> broadcast
  on `hockey.arena.large`; each switch unloads the previous rig, streams the next at the rink's scale, pushes the glass and the trim, respawns the figures. Strip at one frame:
  `tier-strip-night.jpg`, `tier-strip-clay.jpg`, `tier-strip-broadcast.jpg`.
- Large rink: `JAMLOOK tier=p4_broadcast ... scale=1.333 (rink 120.0 m)`, `rig loaded: 10 local lights scaled (intensity x 1.778, attenuation x 1.333)`; `broadcast-large.jpg`.
- **Frame cost** (ProfileGPU through the console, RTX 4090, floating PIE window 2560 x 1354, match paused at frame 668, three dumps per tier, the game view's table of each;
  a foreign editor shared the GPU, so whole frames wander):

  | tier | frame (ms) x3 | Scene | ShadowDepths | Lights | PostProcessing | notes |
  |---|---|---|---|---|---|---|
  | `l2_night` | 6.57 / 6.59 / 6.07 | 5.28 / 5.28 / 4.76 | 0.32 / 0.32 / 0.73 | 1.16 / 1.02 / 0.26 | 2.11 / 1.43 / 2.50 | the P2 reference |
  | `p3_clay` | 10.89 / 12.17 / 12.52 | 8.60 / 11.24 / 9.44 | 0.47 / 2.70 / 2.55 | 2.15 / 0.64 / 0.66 | 4.47 / 4.84 / 4.68 | post is the depth of field (f/3.2); shadow depths from the key |
  | `p4_broadcast` | 18.26 / 8.05 / 9.27 | 15.79 / 7.03 / 7.96 | 0.36 / 0.35 / 0.42 | 5.99 / 1.16 / 1.15 | 2.94 / n.a. / n.a. | LumenReflections 4.50 / 2.06 / 3.26 ms (the glossy ice); the first dump is the frame after the rig loaded (a Message Log window opened, Lights 5.99) and is not representative |

  Reading: against the night tier, broadcast costs about **+2.5 ms** of Scene at 1440p (Lumen reflections on the sheen, the eight rect lights at 1.15 ms), clay about **+4 to +6 ms**
  (the depth of field alone 4.5 ms, the shadowed key 2.5 ms). B expected broadcast to be the most expensive tier; at this frame it is clay. The whole game's 8 ms p99 (C-0002) holds
  for night and broadcast at 1440p in a paused PIE view; clay is over it with DoF on. One editor only was not available (a foreign editor, pid 36516, ran throughout).
- The figure class per tier comes with section 1 (`_TB2_P4` on this tier).

## 5. B-to-A-0019: pickup props, rig check

Answer: **props in, the rig check done; the end-board darkening is accepted until the atmosphere pass.**

- `AJamPickupActor`: B's `/JamAssets/Props/SM_Pickup_<kind>` (a built-in kind, else `SM_Pickup_custom`) on `SM_Pickup_Base` at socket `PickupTop`, relative location 0, stencil 1,
  no collision, no shadow, the mesh's own `MI_PickupProp_<kind>`; spun **90 deg/s** (1.5 deg per frame) and bobbed **0..6 cm** over 120 frames, both from the sim frame + alpha
  (never delta time, `Jam.Core.Determinism` untouched). The props replace the token for every kind that has one (the Director's call); the token with `MI_Pickup_<kind>` stays the
  fallback, and the A14 capsule the fallback of the fallback. `IsPropFromB()` for tests. PIE on `hockey.boost` at 26 s and 30 s: the probe reads `lookB true / prop true` on `Pickup_1_speed`, `Pickup_2_precision`, `Pickup_3_heavy_puck`, no `JAMFX missing` line; `pickup-props.jpg`, `pickup-props-probe.jpg`.
- Rig check on the real rink: `hockey.regular` and `hockey.arena.large` from the game camera on `LR_L2_Night`: no `Dress_` actor inside the boards on either (the probe walks every
  `Dress_` / `Con_` actor against the rink footprint: 194 on the large rink, the only hits are B's `Con_L2_Night_Stanchion_*` on the board line); A's floor at -20 cm on both.
  Stills `tier-strip-night.jpg` (regular) and `rigcheck-large-night.jpg` (large).

## Eyes (bug-hunt on the strip, gemini-3.8-flash, $0.011)

Advice, not instructions. On A's side nothing is flagged (no floor, glass or stanchion finding). What it saw, for B: night: the dark side bands read as "a dark blue reflective
artifact" (B's intended bands); on all three tiers the figures' sticks read detached from the hands or clipping through the torso at this frozen frame (B-to-A-0024 says the far-side
aim poses do not exist yet; the aim grid is B's next step); broadcast: "no player cast shadows on the ice" (the two key spots of `LR_P4_Broadcast` do not seem to shadow the figures
at centre ice: for B to check).

## Gates in the worktree

- Build: 19:00 warm 88.6 s, 19:16 30.2 s, 19:22 (schema embed + `jam.livecut`), final after the editor stopped (see `status/A31.md`); unity off everywhere (R-0022).
- Headless `Automation RunTests Jam.Bot+Jam.Core`: Jam.Bot 6/6 (AiBotPlays, DifficultyProfilesResolve, HumanHasSomethingToDo, PlayerBotWins, SeasonTakes, VisualSwap with the same
  state hash on all three visual tracks), Jam.Core 11/11 after the schema embed was regenerated (CartridgeRoundTrip had caught the stale copy), `Jam.Slow.DifficultyCurve` green.
- `schema`: `node --test` 95/95.

## File list (worktree, outside `Game/Plugins/JamAssets`)

Owned by the brief: `Game/Source/JamGame/Private/HokejGameMode.cpp`, `Public/HokejGameMode.h`, `Game/Source/JamCore/Private/JamPawns.cpp`, `Public/JamPawns.h` (the
`ResetVisual` declaration), `Game/Source/JamGame/Private/RinkActor.cpp`, `Public/RinkActor.h`, `Private/LookTierSwitcher.cpp`, `Public/LookTierSwitcher.h`,
`Game/Source/JamCore/AI/JamAICore.cpp`, `Game/Source/JamCore/Effects/JamPickupActor.cpp`, `JamPickupActor.h`, `schema/game.schema.json`, `schema/limits.json`, `schema/README.md`,
`schema/DEFAULTS.md`, `schema/hockey.test.mjs`, `Game/Config/DefaultGame.ini`, `docs/experiments/B-ASKS-2.md`, `status/A31.md` (main tree).
Needed beyond the list, each a few lines, named for the Director: `Game/Source/JamCore/AI/JamAICore.h` (`bShootHeld`, `bGoalie`, `GoalieSide` in `FJamAIInput`;
`ShotChargeFrames` in `FJamBotSkill`; `bAimed` on `Decide` / `Attack`; three state arrays), `Game/Source/JamCore/AI/DifficultyProfile.cpp` (rookie 0, all_star 18, the tune key),
`Game/Source/JamCore/AI/JamSimAdapter.cpp` (one copied field), `Game/Source/JamGame/Private/ToySkaterVisual.cpp` and `SkeletalSkaterVisual.cpp` (rim 0.6 -> 1.3),
`Game/Source/JamGame/Private/SettingsWidget.cpp` (the label), `Game/Source/JamCore/Private/JamSchemaData.inl` (generated), `docs/HOCKEY.md` (the enum line),
`docs/experiments/eyes/a31-tier-strip-bughunt.json`, `devlog/shots/20261112-194109-A-a31-*.jpg|.txt` (10 stills), `tools/sweep/out/a31-charge-20.jsonl`, `a31-nocharge-20.jsonl`.
Scratch (not in the repo): the proof driver `pie.mjs`, `probe.py`, the eight `a31-*.cartridge.json` variants, the ProfileGPU parser.
