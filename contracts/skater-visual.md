# C-0001: skater visual

id: C-0001
version: 1
status: active
owner: Director
parties: A, B
opened: H4

Rules for every contract are in `contracts/README.md` (including the absent-value convention). This file only fills in the specifics.

## Purpose

Gameplay (device A) drives an `AJamSkater` pawn: movement, puck handling, checks, all in the fixed 60 Hz simulation. It never knows what a skater
looks like. Every look (T-A toy figure by A; T-B2 generated rigged and T-B1 retargeted humanoid by B) is a component or actor that implements
`IJamSkaterVisual`, carries the sockets below and exposes the material parameters below. A look is swapped by naming another visual class in a
cartridge, never by touching play code. Source of truth for the C++ spelling: `Game/Source/JamCore/Public/JamSkaterVisual.h`.

## Inputs, producers and first consumers

| Input | Producer | First consumer | Notes |
|---|---|---|---|
| `IJamSkaterVisual` (C++ `UINTERFACE`, module `JamCore`) | A, lane A0 (stub, H4) | A, toy figure T-A; B, `BP_SkaterVisual_TB2` | implement in C++ or Blueprint (all calls are `BlueprintNativeEvent`) |
| `SetTeam` call | A, skater pawn at spawn and at every cartridge load | the visual implementation | colour comes from `game.teams[].colour_hex` |
| `SetNumber` call | A, skater pawn at spawn | the visual implementation | writes `NumberTens`, `NumberOnes` |
| `UpdateMotion` call | A, skater pawn, every simulation frame | the visual implementation | lean, bob, stride rate |
| `PlayShoot`, `PlayCheck` calls | A, skater pawn, on the frame the action happens | the visual implementation | one-shots, no return |
| `SetKnockedDown` call | A, skater pawn, on knockdown and on getting up | the visual implementation | `knockdown_frames` from the cartridge decides the length |
| `SetHasPuck` call | A, skater pawn, on gain and loss of the puck | the visual implementation | |
| `SetBoost` call | A, pickup logic, on take and on expiry | the visual implementation | `NAME_None` on expiry |
| sockets `stick_socket`, `puck_socket`, `head_socket` | B (each skater mesh), A (toy figure) | A, skater pawn (attaches stick, places puck) | missing socket = the visual fails acceptance |
| material parameters (table below) | B (master materials), A (toy figure materials) | the visual implementation, driven from the calls above | |
| `game.skater.visual` cartridge key | A, lane A4 (schema) | A, skater pawn spawn | **open question 1**: not yet in the schema |

## Types and fields

### `IJamSkaterVisual` (C++ interface `UJamSkaterVisual` / `IJamSkaterVisual`, header `JamSkaterVisual.h`)

Calls arrive on the game thread from the simulation tick. Call from C++ as `IJamSkaterVisual::Execute_<Name>(Object, ...)`.

| Call | Parameters | Unit or range | Meaning |
|---|---|---|---|
| `SetTeam` | `int32 TeamId`, `FLinearColor Colour` | `TeamId` 0 or 1 (index into `game.teams`); colour linear RGB, alpha 1 | jersey colour; writes `TeamColour` |
| `SetNumber` | `int32 N` | 0..99 | jersey number; `NumberTens` = N / 10, `NumberOnes` = N % 10 |
| `UpdateMotion` | `float Speed`, `float TurnRate`, `FRotator Facing` | speed m/s >= 0; turn rate rad/s, positive = turning left seen from above; facing yaw in degrees, pitch and roll 0 | every frame |
| `PlayShoot` | `float Charge` | 0..1 | a shot (or a pass, Charge 0) left the stick this frame; Charge 1 = a fully wound-up charged shot (`shot.charge_frames_max` held). **v2 (C-0005, lane A16, H7):** was `none`; implementations must take the parameter (the toy figure scales its swing by 1 + 0.6 x Charge) |
| `PlayCheck` | none | | this skater threw a body check this frame |
| `SetKnockedDown` | `bool bDown`, `float DirX`, `float DirZ`, `int32 KnockdownFrames` | (DirX, DirZ) unit vector on the ice plane in rink coordinates (x along the rink, z across); ignored when `bDown` is false. **v4:** `KnockdownFrames` = the sim's down time in frames (`knockdown_frames` after modifiers) when `bDown`, 0 when false | fall towards that direction; false = get up. The get-up must end `KnockdownFrames` after the fall began: `SetKnockedDown(false)` arrives exactly then |
| `PlayHitReaction` **(v4)** | `float DirX`, `float DirZ` | the push on this skater (unit, rink x / z): the other skater towards this one; classify against the facing as `SetKnockedDown`'s direction (along = `behind`, against = `front`, else `side_l` / `side_r`) | one-shot: a hit taken standing: a check that landed without a knockdown (none in today's sim), or a skater-skater collision at 3 m/s closing or more with no check landed (one per skater per 20 frames) |
| `PlayBoardHit` **(v4)** | none | | one-shot: this skater hit the boards at 4 m/s or more along the normal, or took a landed check that pushed it into boards within 1.5 m (`game.contact.board_bonus` above 1; lane A101); the frame of the skater `board` event, Who = this skater; Strength on the bus: normal speed / `skater.speed` for the former, the check's strength for the latter. On a check into the boards the same frame also carries `SetKnockedDown(true, ...)` or `PlayHitReaction` (a stumble): the figure decides which take wins |
| `PlayStumble` **(v5, by name)** | `float DirX`, `float DirZ`, `int32 Frames` | (DirX, DirZ) the push on this skater (unit, rink x / z), as `PlayHitReaction`'s; `Frames` = the sim frames the skater still has no thrust, steering or stick, counted from the next frame: `game.contact.stagger_frames` (16) for a body check, one less (15) for a cross-check, whose hit resolves before the skaters move | one-shot on the first frame of a stumble a hit caused: the victim of a check whose ratio fell in the stumble band, or the attacker that bounced off a braced skater (the push then points against its travel). Sent **instead of** `PlayHitReaction` on that frame; the control returns exactly `Frames` frames later (no call marks the end; a knockdown inside the stumble arrives as `SetKnockedDown`). Not sent for a bump (`PlayHitReaction`), a knockdown, or the stumble a mistimed poke gives its own skater (`PlayPoke(false)`). **Not on the interface**: the pawn calls any function or Blueprint event of this name with three numeric inputs on the visual object; a visual without one receives `PlayHitReaction(DirX, DirZ)` as before |
| `SetBraking` **(v5, by name)** | `bool bOn`, `float Decel` | `Decel` in m/s^2, 0 or more: the speed the sim took off the skater on the frame the brake began, per second (`game.skater.reverse_brake_decel` x the input's size: 20 by default); 0 when `bOn` is false | level, called on the change: true on the frames the sim applies the reverse brake (move input more than 120 degrees against a velocity over 1 m/s: the sim's `bReverseBraking`), false when it ends (the skater is still, the input turns, the skater goes down, a faceoff). Not the Ctrl brake. **Not on the interface**: called by name as `PlayStumble`; a visual without one receives nothing |
| `SetBoardPin` **(v4)** | `bool bOn` | | level, called on the change: true after 10 consecutive frames within 5 cm of the boards with a standing opponent within 1.2 m (centre to centre); false when either ends, or the skater goes down or sits in the box |
| `PlayStickHit` **(v4)** | `bool bLifted` | false = a clean poke took the puck, true = a stick lift | one-shot on the carrier that lost the puck to an opponent's stick this frame (the poker's own `PlayPoke(true)` / `PlayLift()` play on the other skater) |
| `SetHasPuck` | `bool bHasPuck` | | |
| `SetBoost` | `FName Kind` | enum `BoostKind` or `NAME_None` | `NAME_None` = no boost active |

Rink coordinates: rink x = Unreal X, rink z = Unreal Y; 1 m = 100 Unreal units. The visual must not move the pawn's root; it animates its own
components only, and it must be deterministic for the same call sequence (no wall-clock, no unseeded randomness).

## Enum strings

### `BoostKind` (`FName`, C++ constants `JamBoostKinds::*`, same strings as `game.pickups[].kind`)

`speed`, `precision`, `heavy_puck`, `big_puck`, `reverse_other`

Unknown values are errors. "No boost" is `NAME_None`, never an empty string.

### Sockets (C++ constants `JamSkaterSockets::*`)

| Socket | Constant | Where |
|---|---|---|
| `stick_socket` | `JamSkaterSockets::Stick` | the stick's grip; T-B2 parents it to `hand_r` |
| `puck_socket` | `JamSkaterSockets::Puck` | where a carried puck sits, in front of the blade, on the ice |
| `head_socket` | `JamSkaterSockets::Head` | above the head: **about 19 cm above the helmet's top** on device B's figures (B-answer to A-to-B-0024, 13 Nov); markers (has-puck, boost) attach here; the HUD's chevron comes back 18 cm along the head's axis to sit on the helmet (`jam.hud.MarkerLiftCm` -18) |

### Material parameters (C++ constants `JamMaterialParams::*`)

| Parameter | Kind | Range | Set from |
|---|---|---|---|
| `TeamColour` | vector | linear RGB, alpha 1 | `SetTeam` |
| `NumberTens` | scalar | 0..9 integer | `SetNumber` |
| `NumberOnes` | scalar | 0..9 integer | `SetNumber` |
| `Emissive` | scalar | >= 0 (multiplier) | tier and boost (`contracts/look-tiers.md`) |
| `Wet` | scalar | 0..1 | look tier (`l3_neon` wet ice) |
| `Scuff` | scalar | 0..1 | ice wear (`hockey.raw`) |

A material that lacks a parameter simply ignores it; a parameter spelled differently is a bug.

## Asset names and prefixes

| Asset name | Kind | Folder | Used by |
|---|---|---|---|
| `BP_SkaterVisual_TA` | Blueprint (or C++ component) implementing `IJamSkaterVisual`, toy figure | `Game/Content/Blueprints/` | A, default visual |
| `BP_SkaterVisual_TB2` | Blueprint implementing `IJamSkaterVisual`, generated rigged figure | `Game/Plugins/JamAssets/Content/Skaters/` | B, swap-in |
| `BP_SkaterVisual_TB1` | Blueprint implementing `IJamSkaterVisual`, retargeted humanoid | `Game/Plugins/JamAssets/Content/Skaters/` | B, fallback swap-in |

Pattern: `BP_SkaterVisual_<track>` with `<track>` one of `TA`, `TB1`, `TB2`. Meshes and materials follow `contracts/assets-naming.md`.

## Folders

| Folder | Written by | Read by |
|---|---|---|
| `Game/Source/JamCore/Public/` (`JamSkaterVisual.h`) | A | A, B |
| `Game/Content/Blueprints/` | A | A |
| `Game/Plugins/JamAssets/Content/Skaters/` | B | A (spawn by class path) |

## Data asset names

None yet. The swap goes through a cartridge key (open question 1).

## Acceptance

- [ ] `Game/Source/JamCore/Public/JamSkaterVisual.h` compiles (`tools/run/build.ps1`) and declares the eight calls with these exact names and parameter types (stub: done at H4).
- [ ] Every `BP_SkaterVisual_*` implements `JamSkaterVisual` (Python: `unreal.load_asset(path).generated_class()` implements the interface) and its mesh has the three sockets.
- [ ] A test cartridge naming the visual loads it and the player bot still wins (`docs/HOCKEY.md` section 7); same state hash per seed with any visual, because visuals never feed back into the simulation.

## Version history

- v1 (H4, lane A0): first version, with the compiled stub.
- v4 (H12.4-H14, lane A45, R-0031 point 1, from handoff B-to-A-0028): the tackling calls B built the takes for, all `BlueprintNativeEvent` with empty native defaults
  (a visual that ignores them keeps compiling): `PlayHitReaction(float DirX, float DirZ)` on a hit taken standing (a landed check without a knockdown, which today's sim
  never produces, or a skater-skater collision at 3 m/s closing or more with no check landed; the direction is the push on the skater, classified as `SetKnockedDown`'s),
  `PlayBoardHit()` on the frame of the skater `board` event (4 m/s and up), `SetBoardPin(bool bOn)` as a level (10 consecutive frames within 5 cm of the boards with a
  standing opponent within 1.2 m; off when either ends or the skater goes down), `PlayStickHit(bool bLifted)` on the carrier that lost the puck to a clean poke (false) or a
  lift (true). `SetKnockedDown` gains a fourth parameter, `int32 KnockdownFrames`: the sim's down time when `bDown` (0 when false), so a figure's get-up ends when the sim
  releases the skater (B-to-A-0028 ask 2). The pawn (`AJamSkater::ApplySimFrame`) makes every call from one-frame read-outs on `FJamSkaterState` (`bBumpThisFrame` /
  `BumpDir`, `bBoardHitThisFrame`, `bBoardPinned`, `StickHitResult`; none enters the state hash) and logs one `JAMVIS hit|board|pin|stickhit` line per call. The toy
  (`UToySkaterVisualComponent`) and skeletal (`USkeletalSkaterVisual`) figures map the four onto their existing curves (docs/experiments/TACKLING.md section 8). **B's
  `BP_SkaterVisual_TB1` / `TB2` already declare Blueprint events with these four names** (B-to-A-0028: "built and waiting for a caller"): until B moves them onto the interface
  (its stated step within the hour of the commit), each Blueprint compiles with "more than one function with the same name" for the four, and `Jam.Bot.VisualSwap` reports
  those compile errors (its own checks pass). `SetKnockedDown`'s new parameter is the v2 precedent: B re-implements the event with the four-parameter signature.
- v3 (H9, proposed by lane A16 from handoff B-to-A-0014, awaiting the Director's signature): the stick verbs, all `BlueprintNativeEvent` with empty native defaults
  (a visual that ignores them keeps compiling): `SetStickAim(float AngleDeg, float ReachM)` every frame (0 = forward, positive = the skater's left; reach = skater centre
  to the blade on the ice; the sim's blade is at 0.4 m + aim x `game.stick.reach_m`, so 1.2 m at rest and 2.0 m at full aim with the default 1.6), `PlayPoke(bool bClean)` on the
  frame a poke resolves (false = mistimed, a stumble follows), `PlayLift()`, `PlayPass(float Strength)` (speed / max), `SetShotCharge(float Charge01)` every frame while charging
  (0 otherwise; `PlayShoot(Charge)` releases), `SetFaceoff(bool bReady)` (a centre at the dot before the drop), `PlayFaceoffDraw(bool bWon)` on the drop for the two centres,
  `SetGoalieStance(bool bOn, int32 Side)` (called by the brain's owner, never by the sim), `PlayFoulReaction()` on the fouler. The pawn (`AJamSkater::ApplySimFrame`) makes every
  call from the sim state; the toy figure implements `SetStickAim` and `SetShotCharge`. A carried puck is drawn at the visual's `puck_socket` when the visual exposes one (a mesh
  socket on any component, or a scene component named `puck_socket`): `AJamSkater::GetPuckSocketWorld`, `AJamPuck::ApplyRenderAt`.
- v2 (H7, lane A16, per C-0005 section 5): `PlayShoot(float Charge)`; the toy figure (`UToySkaterVisualComponent`), the placeholder visual and the camera proof scene updated; device B's `BP_SkaterVisual_TB1/TB2` must re-implement the event with the new signature (handoff note in lane A16's report).

## Open questions

1. The swap key: proposed `game.skater.visual` = `"TA" | "TB1" | "TB2"` (enum, absent = `TA`). Needs a schema change by A4 and a Director ruling.
2. HOCKEY.md section 5 writes `SetTeam(team_id, colour_hex)`; the brief and the code take an `FLinearColor` (gameplay converts the hex). Confirm.

- **v5 (H26+, lane A125; device B asked for both under A-to-B-0021 point 3 and A-to-B-0023 ask 4):** `PlayStumble` and `SetBraking` are called **by name** from the pawn (`AJamSkater::CallFigure`), so the interface header and every existing visual are untouched: a figure declares a function or Blueprint event with that name and those inputs and receives the call; a figure without one keeps receiving `PlayHitReaction` for a stumble and nothing for the brake. One `JAMVIS stumble` log line per stumble (`call=PlayStumble|PlayHitReaction`), one `JAMVIS brake` line per change for the human.
