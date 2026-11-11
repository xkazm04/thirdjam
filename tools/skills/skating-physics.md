---
name: skating-physics
description: Build the deterministic hockey simulation in C++ - skater movement with lateral grip, rink boards, skater contacts and checks, the puck with board bounce, drag, speed cap and shots - stepped at a fixed 60 Hz with no Chaos and no Character Movement Component, plus the determinism test. Use on device A from H1 (M0 greybox) onward.
track: gameplay (device A); every skater visual track sits on top of it
evidence: docs/SKATERS-RESEARCH.md sections 4 and 5; Godot source third-life/engine/kernel/api/tl_biped.gd
---

# Recipe: skating and puck physics (kinematic, fixed 60 Hz)

One plain C++ simulation owns all gameplay motion. Actors only display it. This is a port of the Godot hockey that already passed its bot tests,
with the numbers of `docs/HOCKEY.md` section 2 and the cartridge `game.skater`, `game.puck`, `game.shot`, `game.rink`.

## Why not Chaos or the Character Movement Component

Chaos async tick and substepping are marked "still experimental" in the 5.9 physics settings, and no determinism switch is documented; the
CMC steps on the render delta and has no forward/lateral grip split. Both would need fighting, and neither gives the same `state_hash` per seed.
Chaos is allowed for cosmetics only (tipped figures, debris), never for anything the hash or the bots read.

## Structure

- `FJamSim` (plain struct in `JamCore`, no UObject): `Frame`, `FRandomStream Rng` (seeded from the cartridge seed, the **only** randomness),
  `TArray<FJamSkater> Skaters`, `FJamPuck Puck`, score, rule state, cartridge values. `void Step(const TArray<FJamInput>& Inputs)` advances
  exactly `DT = 1.0 / 60.0`. `uint64 StateHash() const` hashes positions and velocities quantised to 1 mm and 1 mm/s, plus frame, score, possession.
- Driver: a world subsystem (or the game mode) ticks with an accumulator: `Acc += DeltaSeconds; int n = 0; while (Acc >= DT && n < 4) { Sim.Step(Inputs); Acc -= DT; ++n; }`.
  Pawns read `Sim` state and set their transforms, interpolating with `Alpha = Acc / DT` between the previous and current sim state.
- Inputs are sampled once per sim frame (human: Enhanced Input values latched at the step; bots: computed from the sim state inside the step,
  stateless per tick as in Godot). Iterate skaters in a fixed order (by id), never over a `TMap` or a `TSet`.
- Use `double` in the sim (Unreal's `FVector` is double in UE5) and work in **metres** in the sim; convert to centimetres only when writing transforms.

## Skater step (per skater, per frame)

```
// inputs: move (2D, |move| <= 1), brake (bool), shoot, check
if |move| > 0.05:  yaw = rotate_toward(yaw, atan2(move.y, move.x), turn_rate * DT)        // turn_rate 5 rad/s
fwd   = (cos yaw, sin yaw)
along = dot(v, fwd);  side = v - fwd * along
if |move| > 0.05 and !brake and along < speed:  along = min(speed, along + accel * |move| * DT)   // speed 8, accel 12
along = move_toward(along, 0, (ice_mu * 9.8 + (brake ? brake_decel : 0)) * DT)            // ice_mu 0.015 -> 0.147 m/s2
side *= max(0, 1 - (lateral_grip + (brake ? brake_decel * 0.5 : 0)) * DT)                 // lateral_grip 3 /s
v = fwd * along + side
if knocked_down: v *= max(0, 1 - 2.0 * DT); no thrust, no turn, no shot, until knock_frames reaches 0
p += v * DT
```

`rotate_toward` = move the angle toward the target by at most `step`, wrapping through +-pi. `move_toward(a, b, d)` = step a toward b by at most d.
Godot named the braking value `skate_brake`; the cartridge has no key for it yet (ask the Director; until then a constant 6 m/s2).
`ice_mu` maps to `game.rink.ice_friction` (state that in the cartridge contract).

## Rink (rounded rectangle) and contacts

- Rink half extents `hx = length/2 = 30`, `hz = width/2 = 15`, corner radius `rc = 5`. Signed distance to the board for a point p (inside negative):
  `q = abs(p) - (hx - rc, hz - rc); d = length(max(q, 0)) + min(max(q.x, q.y), 0) - rc`. Normal = gradient (normalise `sign(p) * max(q,0)` in the
  corner, the axis normal on the straight).
- Skater vs board (radius `r_s = 0.4`): if `d + r_s > 0`: push in by `d + r_s` along `-n`; remove the outward normal velocity and keep 30% as a
  bounce: `vn = dot(v, n); if vn > 0: v -= (1 + 0.3) * vn * n`.
- Skater vs skater (both `r_s`): overlap -> separate by half the overlap each (masses equal at 80) and exchange the normal relative velocity
  with restitution 0.2. **Check**: if A pressed check within the last 6 frames, `check_enabled`, and A's speed along the contact normal
  `> check_speed_min`, then B gets `knock_frames = knockdown_frames`, B's visual `SetKnockedDown(true)`, and B drops the puck. Emit the
  `check` and `knockdown` events.
- Goals: posts as two circles (r 0.05) at `x = +-26, z = +-goal_width/2`, back and side walls of the net as segments for the puck only; the goal
  trigger is a box behind the line (`|x| > 26`, `|z| < goal_width/2`, puck only). Scoring side = the opposite team. Pause `goal_pause_frames`, faceoff reset.

## Puck step (4 fixed substeps per frame)

At 30 m/s the puck moves 0.5 m per frame, four times its radius: step it in `N = 4` substeps of `DT/4` (0.125 m each) so it cannot pass a post.

```
for i in 1..4:
  v *= max(0, 1 - drag * DT/4)                 // game.puck.drag in 1/s; Godot had 0.0; start at 0.1
  if |v| > max_speed: v *= max_speed / |v|     // 30 m/s
  p += v * DT/4
  board: d = sdf(p); if d + r_p > 0: p -= (d + r_p) * n; vn = dot(v, n); if vn > 0: v -= (1 + e) * vn * n; v_t *= (1 - 0.1)
         // e = game.rink.board_restitution (start 0.6), r_p = 0.12; v_t = tangential part
  posts and net walls: circle/segment push-out with the same reflection, e = 0.3
  carried? if a skater has possession: p = skater.p + fwd * 0.6, v = skater.v (no integration)
```

- **Possession** (a proposal; the Godot pack's exact rule was not checked, read `third-life/site/ice-hockey-three-against-20261111-1/pack` if in doubt): a free puck within 0.6 m of a skater's stick point, moving slower than 12 m/s relative to it, is taken; the
  nearer skater wins a tie, then the lower id. A shot or a knockdown releases it.
- **Shot**: allowed if the puck is within `reach_m` (1.5) of the skater, the angle between `fwd` and the puck direction is within 60 deg
  (`dot >= 0.5`), and `cooldown_frames` (24) have passed. Puck `v = aim * shot.speed` (18 m/s, regardless of mass), `aim` = facing, or the
  precision-boost cone's target. Emit `shot`. The visual gets `PlayShoot()` on the same frame.
- **Drop on knockdown**: puck `v = skater.v * 0.5 + rng_unit_vector() * 1.0` (the seeded stream; never `FMath::Rand`).

## Project settings (gameplay does not depend on them, set them anyway)

- Physics: leave Tick Physics Async off; gameplay actors have `SimulatePhysics` off and collision `QueryOnly` or `NoCollision`.
- General > Framerate: do not force a fixed frame rate for play (the accumulator does the fixing); smooth frame rate is fine.
- Test runs of the packaged game: `-deterministic` (= `-UseFixedTimeStep -FixedSeed`) with `-FPS=60` makes one render frame equal one sim step,
  which makes screenshots per frame reproducible; the sim itself does not need it.

## The determinism test (write it before the feel tuning)

- `Jam.Sim.Determinism` (C++ automation test, no world, no map): build the sim from the `hockey.regular` cartridge at seed 1; run 3,600 frames
  with the player bot and AI bots; record `StateHash()` every 60 frames. Run it again in the same process: all 60 hashes equal. Seed 2 must
  differ from seed 1 by frame 600 (proves the seed is used).
- `Jam.Sim.Bounds`: no skater or puck outside the rink SDF by more than 1 cm at any frame of the 3,600.
- `Jam.Sim.PuckCap`: a shot at 18 m/s plus a carried boost never exceeds 30 m/s; a puck fired at a corner at 30 m/s comes back (no tunneling).
- `Jam.Sim.Feel` numbers (from Godot, the acceptance of the port): from rest, full input, the skater reaches 8 m/s in about 0.67 s (40 frames);
  released at 8 m/s it still moves at > 7 m/s after 5 s of glide (0.147 m/s2); a 90 deg turn at full speed takes 19 frames (5 rad/s) and the
  sideways slip halves in about 14 frames (grip 3).
- Then the milestone tests in `docs/HOCKEY.md` section 7 (`PlayerBotWins` at seeds 1, 2, 7 with the same hash per seed) run on the same sim.

## Pitfalls

- Any float that touches the hash must come from the sim; reading an actor transform back into the sim breaks determinism (interpolated values).
- `TMap` iteration order and pointer-sorted arrays differ between runs; sort by id.
- `FMath::FRand`, `FMath::RandRange` use a global stream: forbidden in the sim. Pass `Rng` everywhere.
- Fast-math and different builds (Development vs Shipping, different CPUs) can change the last bits of floats; the hash quantises to 1 mm to
  absorb that. Compare hashes within one build; the gate runs on device A.
- The godot formula multiplies `side` by `(1 - k*DT)`: with large k (brake + grip > 60) it would flip sign; the `max(0, ...)` clamp is required.
- Units: the sim is metres and seconds-per-frame; Unreal transforms are centimetres. Convert in exactly one function.
