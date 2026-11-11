---
name: skater-toy-figure
description: Build track T-A, the code-driven toy skater (capsule body, sphere head, cylinder stick) that implements IJamSkaterVisual with lean, bob, stick swing, check and tip-over computed from sim state and frame counters. Use on device A after the seam stubs exist (H1+).
track: T-A (device A, H1-H24, ships in M1)
evidence: docs/SKATERS-RESEARCH.md section 1
---

# Recipe: the toy skater figure (T-A)

You build the look of a skater with **no skeleton and no imported asset**: engine basic shapes, one material, C++ that poses them.
Gameplay owns the pawn and the simulation (`tools/skills/skating-physics.md`); you own only the visual component behind `IJamSkaterVisual`.

## Before you start (read, 5 minutes)

- `contracts/skater-visual.md` (the `IJamSkaterVisual` calls, sockets `stick_socket` `puck_socket` `head_socket`, asset name `BP_SkaterVisual_<track>`),
  `contracts/look-tiers.md` (material parameter names `TeamColour`, `Emissive`, `Wet`, `Scuff`). If either is missing, stop and ask the Director.
- `docs/HOCKEY.md` section 5. Your track is T-A; the asset is `BP_SkaterVisual_TA` (or the C++ class the contract names).
- Paths you may write: `Game/Source/JamGame/**` for C++, `Game/Content/Blueprints/**` for the thin Blueprint, the material where the
  contract says (A owns gameplay-driven materials only if the contract says so; otherwise ask B through `handoff/`).

## Laws

- **Frames, not seconds.** Every animation is a function of `(sim state at frame N, frames since trigger)`. Store the trigger frame. Never use
  `FPlatformTime`, timers, or `GetWorld()->DeltaTimeSeconds` for a pose. dt is the sim's fixed `1/60`.
- **The visual never writes into gameplay.** No collision on the figure's meshes (`SetCollisionEnabled(NoCollision)`), no physics, no tick
  dependency the sim waits on. The state hash must not change if the figure is removed.
- **No `class_name`-style shortcuts, no hand-placed lights.** Only basic shapes from `/Engine/BasicShapes/` and the one material.
- **No logo, no brand, no real team colours by name.** Colours come from the cartridge.

## Steps, in order

1. **Component skeleton (C++).** `UJamToyFigureComponent : public USceneComponent, public IJamSkaterVisual` in `JamGame`.
   In `OnRegister` create, attach and keep pointers to:
   ```
   Yaw        (USceneComponent, rotates to facing)
   └ Lean     (USceneComponent at the skate contact point, z = 0: lean and tip rotate here)
     └ Body   (UStaticMeshComponent, /Engine/BasicShapes/Cylinder, scale ~ (0.55, 0.55, 0.9) -> 0.55 m wide, 0.9 m tall; pivot raised 0.1 m for skates)
       ├ Head (Sphere, d 0.4 m, on top)        -> head_socket = a USceneComponent named "head_socket"
       ├ StickPivot (USceneComponent at the right shoulder)
       │ └ Stick (Cylinder 0.04 x 0.04 x 1.5 m, blade = Cube 0.3 x 0.05 x 0.08 at its end) -> stick_socket at the blade
       └ Number (UTextRenderComponent on the back) only if the material digit is not built
     └ SkateL, SkateR (Cube 0.3 x 0.08 x 0.06, dark grey, 0.12 m apart)
   puck_socket = USceneComponent 0.6 m ahead of the body at z = 0
   ```
   Sizes are a start; the camera is at 52 m, so readable beats realistic. All shapes: `NoCollision`, `CastShadow` on, `bReceivesDecals` off.
2. **Material.** `M_ToyPlastic`: base colour = `TeamColour` (vector), roughness ~0.35, specular 0.5, a fresnel rim added to emissive
   (`Fresnel(exp 4) * TeamColour * 0.3`), `Emissive` scalar multiplying a second emissive term for boosts and the neon tier. One
   `UMaterialInstanceDynamic` per figure for Body and Head (head may use a lighter tint: `TeamColour * 1.3`). Create it by MCP
   `EditorToolset.MaterialToolset` or by hand; it is a material, not a texture, so no import is needed.
3. **Jersey number.** Preferred: in the same material, a seven-segment digit for tens and ones (scalar params named as the contract says;
   propose `NumberTens`, `NumberOnes` if it says nothing), drawn on the back with a mask from the UV region of the cylinder. Fallback in 10
   minutes: the `UTextRenderComponent`, white, world size 0.35, unlit. Do the fallback first, the material digit only if time remains before H20.
4. **The interface, as state setters only.** Each call stores data; posing happens in one place (step 5).
   - `SetTeam(team_id, colour_hex)`: `FColor::FromHex(colour_hex)` -> `FLinearColor::FromSRGBColor` -> `SetVectorParameterValue("TeamColour")`.
   - `SetNumber(n)`: store, update digits or text.
   - `UpdateMotion(speed, turn_rate, facing)`: store the three; `facing` drives `Yaw`.
   - `PlayShoot()`: `ShootStart = SimFrame`. `PlayCheck()`: `CheckStart = SimFrame`.
   - `SetKnockedDown(b)`: on a change, `KnockStart = SimFrame`, `bDown = b`.
   - `SetHasPuck(b)`, `SetBoost(kind)`: store.
   Get `SimFrame` from the sim (a getter on the game mode or the sim subsystem), never count your own ticks.
5. **Pose function.** `FToyPose ComputePose(const FToyInputs& In, int32 Frame)` as a **static pure function** (testable without a world).
   Then `TickComponent` applies the pose (optionally interpolated with the sim's render alpha). Formulas (degrees, metres, frames):
   - Lean: `target = clamp(0.6 * speed * turn_rate, -25, 25)`; follow `lean += (target - lean) * (1 - exp(-12/60))` once per sim frame
     (keep `lean` in the pose state advanced per sim frame, not per render frame).
   - Crouch and bob: body pitch `-(10 + 6 * min(speed/8, 1))`; `phase += speed / 2.4 / 60` per sim frame; `bob_z = 0.04 * min(speed/8,1) * |sin(pi*phase)|`;
     body yaw sway `4 * sin(2*pi*phase) * min(speed/8,1)`.
   - Shot (frames since `ShootStart`, keys from the Godot clip): `0:0, 18:-100, 25:+10, 34:+75, 40:+75, 51:0` deg of StickPivot yaw, smoothstep between keys;
     body yaw follows at 30% of the stick. After 51 frames the shot is over.
   - Check (frames since `CheckStart`): body pitch forward 20 deg and Lean x +0.15 m over 8 frames, back over 12.
   - Knockdown (frames since `KnockStart`, `bDown`): Lean pitch `0 -> 88` over 14 frames, ease-in (`t^2`), then `88 -> 80 -> 88` over 6
     (one bounce), hold. Recovery (`!bDown`): `88 -> 0` over 20 frames ease-out. Direction: the contract's knock direction if it has one,
     else backwards relative to facing.
   - Has puck: StickPivot pitch -10, Stick forward 0.1 m. Boost: `Emissive = 1 + 0.5 * sin(2*pi*Frame/30)`, body scale `1 + 0.04 * sin(...)`.
6. **Thin Blueprint.** `BP_SkaterVisual_TA` derived from the component class only if the contract wants a Blueprint asset to swap; no logic in it.
7. **Review map (dev only, never cooked).** A map with six figures in a row driven by a fake input: speed ramps 0->8, turn -5..5, one shot every
   90 frames, one knockdown every 180 frames, two team colours, numbers 1-6. Keep it out of `+MapsToCook`.

## The test to run

- **Automation test `Jam.Skater.ToyPose`** (C++, no world): for fixed inputs, `ComputePose` at frames 0, 18, 25, 51 of a shot and 0, 14, 20 of a
  knockdown returns the keyed values within 0.01; calling it twice with the same inputs returns identical bytes; lean is clamped at 25.
- **Determinism**: run the bot match at seed 1 with the toy figure and with no visual at all (a null visual): the state hash must be equal.
- **Readability (screenshot)**: at the arena camera, both team colours and the stick direction are visible on a 1080p capture
  (`SceneCaptureToolset` or `tools/run/smoke.ps1` screenshot). The critic's Godot finding was "team colours hard to read from above": if two
  colours fail, raise saturation or add a white head/helmet band, not size.
- **Frame cost**: six figures add under 0.2 ms game thread (stat unit before and after).

## Pitfalls

- Pivots: Unreal's basic cylinder pivot is at its centre, 100 units tall; scale is in units of the 1 m shape. Put lean and tip pivots on scene
  components at the contact point, never rotate the mesh about its centre (the figure would sink into the ice).
- Units: Unreal is centimetres. The formulas above are metres; multiply by 100 when you set locations.
- Do not interpolate the lean follow per render frame: at 144 Hz it would lean faster than at 60 Hz. Advance it per sim frame.
- `FColor::FromHex` accepts `#RRGGBB` and `RRGGBB`; the cartridge wire format decides; validate once, log the bad value, fall back to grey.
- A `UTextRenderComponent` faces one way: put it on the back and accept that it is unreadable from the front.
- Shadows from six small figures are cheap; translucency is not: keep the material opaque.
- If something needs a skeleton, you are in T-B, not T-A. Stop.
