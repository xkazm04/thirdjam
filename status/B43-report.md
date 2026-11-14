# B43 report: the effects of the three new animation sets, built without a GPU (device B)

Lane B43, device B, machine clock 05:46 to 06:30. Tree `C:\Users\kazda\jam\wt\b-aud` (`lane/b-aud` at d373bc21), built first (`ThirdJamEditor Win64 Development`: Succeeded, about 3 minutes), port 8006, two editor sessions one after the other (pids 11912 and 36612), both started with `-nullrhi -unattended`, both left through `ExitEditor`; the last one is gone. **The null RHI was in use**: the log loads `NullDrv`, says `rhiname="Null"` and has 0 `LogD3D12RHI` lines (both sessions). The editors on 8004 and 8008 were not touched. I wrote only under `VFX/` in this tree: eleven new systems and three changed ones; no material, no Blueprint, nothing under `Skaters/`. No commit, no push, no cook. `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` back to 0 / 0 / empty, `Slate.bAllowThrottling` back to 1. Backups of the three changed systems as they were: scratch `bk/`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b43` (`spec44.mjs` the eleven systems, `spec44b.mjs` the lift, `patch43.mjs`, `model.mjs` the paper model and `model.txt` its output, `b43_film.py` the filming block, `nb.mjs` the builder).

## What a headless engine can and cannot do for Niagara (nobody had tried)
- **Authoring works.** Systems are created, emitters added, expressions set and **compiled** under the null RHI: all eleven new systems and the three changed ones report `UpToDate`, no errors, no warnings, and saved (the tool's flag and the files on disk).
- **Simulation does not.** In a headless play session `SpawnSystemAtLocation` returns nothing, and `Activate` on a figure's existing Niagara component leaves it inactive; the sim cache says `No valid simulation data`. So **no particle count, no bounds and no spawn test could be read from the engine**. The brief's proof "spawns in a headless play session, counts per frame read from the system" cannot be given by any headless lane.
- What I give in its place: a **paper model** of each system (`model.mjs`): my spec run through my own model of the template's modules (a burst at a spawn time, a location in a sphere or on a flat ring, a velocity from a point, gravity, drag, a random life), 60 steps a second. Its numbers are arithmetic on the spec, not readings.
- For the GPU session: the Niagara sim cache is in Python (`NiagaraSimCacheFunctionLibrary.capture_niagara_sim_cache_immediate`, then `read_position_attribute('Position', emitter, frame)`): with a renderer it will give the real count and box per frame; `tk.py` in the scratch has the function (`m43`).

## Verdict per system (all: compiles, saved, **not simulated, not seen**)
| System | Built as | Verdict |
|---|---|---|
| `NS_dash_push` | 4 to 8 streak chips and 1 to 2 pale dabs thrown back and out from the pushing skate; `User.Variant` picks the side | built; small on purpose (it fires five times a second) |
| `NS_dash_streak` | one thin pale quad (7 cm wide, up to 70 cm long) every 0.03 s behind the hips, only while the owner travels at 6 to 25 m/s | built; **shows nothing on a standing figure, and nothing if the dash is slower than 6 m/s** |
| `NS_dash_skid` | the stop's fan in three bursts over the hold (0, 4 and 8 frames after the start), thrown ahead along `User.Dir` | built from the fan's measured values |
| `NS_dash_wipeout` | a low pale cloud ahead of the chest and at the hips, shavings ahead | built |
| `NS_dive` | a line of streak chips at five points across the body's front and three patches of pale dust, thrown along the facing; then a light trail by the owner's speed | built; the trail needs the game to move the figure |
| `NS_dive_hit` | a cream star for 3 frames, a dozen pieces; needs no direction | built |
| `NS_slide_tackle` | three small sprays ahead of the leading skate, then the contact burst 8 frames after activation | built; one trigger at take frame 2 gives both |
| `NS_mallet_bonk` | a cream star for 3 frames and a cream chalk puff that opens from a 22 cm ring; needs no direction | built; soft and round by construction |
| `NS_cannon_fire` | a cone of pale chalk, eight white shards, four slow wisps along `User.Dir`; no light, no fire colour | built; **a cone, not a true ring** (the builder's ring lies flat on the ice plane) |
| `NS_giant_sweep` | five small sprays on a 2.05 m arc from 44 degrees right to 44 left, one a frame | built; ten emitters, 2.3 MB on disk |
| `NS_pin_roll` | a line of streak chips 1.05 m ahead across 0.9 m and a cream dust | built; no take trigger exists for it yet |

FILM1's leftovers, changed by numbers:
| What | Cause in the system | Change | Verdict |
|---|---|---|---|
| The shotgun's cloud lost its blue | the colour (0.50, 0.56, 0.72) is a dark, barely blue value; many overlapping dabs of it draw as grey | particle colour to (0.62, 0.80, 1.30) | changed; for the GPU to judge |
| The shield's ring missing at +30 | **not found in the system**: life was 45 to 54 frames with a late fade | life to 54 to 60 frames, fade `1 - age^4` | changed; cause open: check on the GPU whether the ring is there at +45 |
| `NS_lift`'s snow kick still round | the emitter kept a uniform size and camera facing; B42's patch of the stack did not take | **`NS_lift` rebuilt whole** from its values (B35, B37, B40, B42), the kick as 3 x 9 cm streaks | rebuilt blind: compare with FILM1's lift on the GPU before it ships |
| The goal's dust fades from +80 | life 96 to 108 frames, fade `1 - age^4`, gravity -900 | life 114 to 126 frames, fade `1 - age^6`, gravity -600; fountain life 78 to 114 frames | changed |

## The counts table (paper model, Strength 1, direction given; live particles at +0, +4, +10, +20 frames; the box is at +4 or +10, cm in the figure's frame from the figure's root)
| System | +0 | +4 | +10 | +20 | Peak | Box |
|---|---:|---:|---:|---:|---:|---|
| `NS_dash_push` (left skate) | 10 | 10 | 1 | 0 | 10 | -14 to 11 ahead, 9 to 29 left, up to 10 |
| `NS_dash_streak` (owner at 14 m/s) | 1 | 3 | 6 | 9 | 10 | -138 to -46 ahead, 0 to 0 left, up to 65 |
| `NS_dash_skid` | 15 | 15 | 43 | 14 | 45 | 123 to 254 ahead, -55 to 55 left, up to 62 |
| `NS_dash_wipeout` | 28 | 28 | 20 | 7 | 28 | 2 to 169 ahead, -46 to 50 left, up to 64 |
| `NS_dive` (owner still) | 37 | 37 | 32 | 0 | 37 | 48 to 107 ahead, -151 to 150 left, up to 44 |
| `NS_dive_hit` | 14 | 13 | 2 | 0 | 14 | -10 to 12 ahead, -8 to 10 left, up to 28 |
| `NS_slide_tackle` | 5 | 10 | 38 | 10 | 38 | 85 to 132 ahead, -20 to 11 left, up to 16 |
| `NS_mallet_bonk` (at the drum face) | 23 | 22 | 22 | 12 | 23 | 65 to 149 ahead, 6 to 76 left, up to 96 |
| `NS_cannon_fire` (at the muzzle) | 30 | 30 | 24 | 2 | 30 | 82 to 275 ahead, -59 to -8 left, up to 111 |
| `NS_giant_sweep` | 8 | 32 | 38 | 0 | 40 | 150 to 222 ahead, -177 to 85 left, up to 20 |
| `NS_pin_roll` | 29 | 29 | 22 | 0 | 29 | 112 to 156 ahead, -49 to 50 left, up to 51 |
| `NS_lift` (rebuilt) | 27 | 27 | 10 | 0 | 27 | -56 to 24 ahead, -27 to 27 left, up to 101 |

Read with care: the model spawns in a full sphere, so some pieces start under the ice (the engine hides those); sprite sizes are not in the box (add up to 25 cm for a dab).

**Against the figure** (a standing body taken as 30 cm ahead and behind, 35 cm to each side, 20 to 190 cm up, at the root): the push stays under 10 cm high beside the skate; the streak starts 45 cm behind the hips and runs back; the skid, the wipeout, the dive's line, the slide's sprays, the sweep and the pin's line all start ahead of the body (24 cm or more) and move away; the cannon's cone starts at the muzzle 79 cm ahead and 35 cm right; the bonk starts on the victim's hip by design and its ring is 40 cm from the centre at +4. The model counts 0 to 2 pieces inside the body box for every system but the lift (8: its mark is on the victim's side by design) and the dive's hit (spawned on the blocker).

**The gates, by evaluating the built expressions** (not by a spawn):
- NS_dash_push zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 5
- NS_dash_streak zero Dir peak 10 | time step 0.0001 peak 0 | Strength 0 peak 10
- NS_dash_skid zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 21
- NS_dash_wipeout zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 28
- NS_dive zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 37
- NS_dive_hit zero Dir peak 14 | time step 0.0001 peak 0 | Strength 0 peak 10
- NS_slide_tackle zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 30
- NS_mallet_bonk zero Dir peak 23 | time step 0.0001 peak 0 | Strength 0 peak 17
- NS_cannon_fire zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 24
- NS_giant_sweep zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 40
- NS_pin_roll zero Dir peak 0 | time step 0.0001 peak 0 | Strength 0 peak 20
- NS_lift zero Dir peak 27 | time step 0.0001 peak 27 | Strength 0 peak 27
- NS_dash_streak owner standing peak 0 | at 5 m/s 0 | at 30 m/s 0
- NS_dive trail owner at 6 m/s: +0: 41 [-14,68,-140,139,-11,38] body 1 | +4: 45 [-58,67,-151,150,-26,44] | +10: 48 [-122,62,-163,139,-40,41] body 1 | +20: 24 [-226,-16,-30,28,-49,33] body 1 | +40: 27 [-266,10,-20,43,-40,21] body 1 || peak 49

So: with no direction the eight directed systems spawn nothing (the two that need none, and the streak, do not read it); on a time step of 0.0001 s every new system spawns nothing; the streak spawns nothing from a standing owner, at 5 m/s, or at 30 m/s; no system reads a raw owner velocity (the streak clamps at 16 m/s, the dive's trail at 8 m/s). `NS_lift` has no such gate, as before.

**Peak counts and the worst plausible moment.** Per system: the Peak column. A dash and a weapon on top of FILM1's worst moment (a hit into the boards with a slide, a goal, two stops: 389): + skid 45 + streak 10 + the largest weapon effect 40 = **484** (the line is 500). All from spawn counts.

**Frame share, predicted, not measured.** From one calibration (FILM1's fan: 34,200 cm2 of sprites measured 2.99 % at the human's distance): skid about 2.9 % at its fullest; wipeout 2.1 %; dive landing 2.1 %; mallet 1.8 %; cannon 1.7 %; sweep 1.0 %; slide contact 1.0 %; pin 0.8 %; streak 0.3 %; push 0.1 %. Expect a half either way.

## Not verified
- **Everything by eye**: size, colour, shape, whether any of it reads, whether anything hides a figure or the puck, on clay or night.
- **Everything simulated**: that the systems spawn, the counts, the boxes, the gates in a running engine. The model is mine, not the engine's.
- The contact burst of `NS_slide_tackle` landing on take frame 10 (it is a spawn time of 0.133 s inside the system).
- `NS_lift` after its rebuild; the shield ring's life; the shotgun's colour; the goal at +100.
- That `M_Fx_Chalk` and `MI_Fx_Chip` draw on the new systems (materials were only referenced, not compiled, here).
- The filming block's spawns (headless they return nothing); its moves, schedule, bone reads and cameras ran.
- Cook: not run. Eleven assets are new (11.3 MB), three changed.

## For the figure lane (what to wire; until then the filming block spawns them)
| System | Take | Frame | Where | How | Inputs |
|---|---|---|---|---|---|
| `NS_dash_push` | `Dash_Loop` (and `Dash_Start`) | loop 2 for the left skate, 14 for the right (start: 3 left, 8 right) | the `foot_l` / `foot_r` bone, dropped to the ice | spawn at location | `User.Dir` = the travel (unit, world x/y); `User.Variant` 0 left, 1 right; `User.Strength` 0 to 1 (the boost's level, optional) |
| `NS_dash_streak` | from `DashBegin` to the dash's end (also off on skid, bump, wipeout) | | a Niagara component on the `pelvis` bone, auto-activate off (as `SlideFx`) | Activate / Deactivate | none (it reads its owner's speed, clamped) |
| `NS_dash_skid` | `Dash_Skid`, `Dash_Skid_Left` | 2 | the root on the ice | spawn attached to the root | `User.Dir` = the travel; `User.Strength` |
| `NS_dash_wipeout` | `Dash_Wipeout` | 13 | the root on the ice | spawn at location | `User.Dir` = the travel |
| `NS_dive` | `Dive_Block_Left` / `_Right` / `_Front` | 9; deactivate at 27 | the root on the ice | spawn attached to the root | `User.Dir` = the figure's facing (toward the shooter) |
| `NS_dive_hit` | `Dive_Block_Hit_*` | 0 | the puck, or the `spine_03` bone | spawn at location | `User.Strength` (optional) |
| `NS_slide_tackle` | `Slide_Tackle_*` | 2 (its contact burst comes 8 frames later by itself) | the root on the ice | spawn attached to the root | `User.Dir` = the slide's travel; `User.Strength` |
| `NS_mallet_bonk` | `Mallet_Swing`, on a hit | 17 | the drum's end face (a socket on the weapon mesh), or the root moved 105 cm ahead, 45 cm left, 72 cm up | spawn at location | `User.Strength` (optional) |
| `NS_cannon_fire` | `Cannon_Fire`, `Cannon_Kick` | 8 | the keg's mouth (a socket), or the root moved 79 cm ahead, 35 cm right, 89 cm up | spawn at location | `User.Dir` = the shot |
| `NS_giant_sweep` | `Giant_Sweep` | 10 | the root on the ice | spawn at location | `User.Dir` = the facing; `User.Variant` 0 if the blade runs from the figure's right to its left, 1 the other way |
| `NS_pin_roll` | `Pin_Roll` | 16 | the root on the ice | spawn at location | `User.Dir` = the facing; `User.Strength` |

Every directed system spawns **nothing** if `User.Dir` is not set: set it on the same node as the spawn.

## For the other device
1. The dash's speed: the streak shows from 6 m/s to 25 m/s of the figure's travel. Say what a dash travels at.
2. The dive and the slide have no travel in their takes: the trail of `NS_dive` and the forward drift of `NS_slide_tackle` follow the figure only if the game moves it.
3. For the dive's hit: the puck's position on the frame it strikes the blocker (the figure only knows its own bones).
4. If any of these moves becomes a bus event, send `Dir` with it; the binder passes `User.Dir` only for check, knockdown, lift and poke today.

## Decisions for the lead
1. `NS_lift` was rebuilt whole and blind to fix six small chips. If that is too much risk for a headline effect before the GPU sees it, take `NS_lift.uasset` from the scratch `bk/` (FILM1's file) when you adopt.
2. `NS_giant_sweep` is 2.3 MB for a one-second effect (ten emitters). It can be five (drop the dust) at about half the size.

## The next three evolutions, each with a proving number
1. **The GPU session of `status/B43-film.md`**: every strip filmed and each effect's frame share measured. Proof: each within a half of its predicted share, and 0 % of the figure's pixels under the streak.
2. **Real counts and boxes through the sim cache** on the GPU. Proof: each system's peak within 10 % of the model's, no piece inside the body box at +4 for the systems that start ahead.
3. **The cannon's chalk as a true upright ring** (an oriented ring location, tried on the GPU). Proof: at +2 at least 80 % of the dabs within 5 cm of a 20 cm circle across the shot.

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Bytes |
|---|---|
| `NS_dash_push` | new, 550,392 |
| `NS_dash_streak` | new, 311,478 |
| `NS_dash_skid` | new, 1,436,799 |
| `NS_dash_wipeout` | new, 757,603 |
| `NS_dive` | new, 2,092,720 |
| `NS_dive_hit` | new, 735,544 |
| `NS_slide_tackle` | new, 1,206,323 |
| `NS_mallet_bonk` | new, 737,219 |
| `NS_cannon_fire` | new, 757,464 |
| `NS_giant_sweep` | new, 2,330,474 |
| `NS_pin_roll` | new, 979,589 |
| `NS_lift` | 1,334,643 -> 1,412,764 |
| `NS_ability` | 1,623,601 -> 1,527,198 |
| `NS_goal` | 751,010 -> 706,833 |

## Ledger rows (full rows, the ledger's 11 columns: add the eleven new ones, replace the rows of `NS_lift`, `NS_ability`, `NS_goal`)
```
| Game/Plugins/JamAssets/Content/VFX/NS_dash_push.uasset | vfx | 0.52 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | one push of the dash, spawned at the pushing skate: 4 to 8 streak chips and 1 to 2 pale chalk dabs thrown back and out; User.Dir = the travel, User.Variant 0 left skate / 1 right, User.Strength; nothing without a direction or on a near-zero time step; peak 10; built without a GPU, not seen (lane B43) (550392 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dash_streak.uasset | vfx | 0.30 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the dash's streak, attached at the hips: one thin pale velocity-aligned quad every 0.03 s behind the hips while the owner travels at 6 to 25 m/s (speed clamped to 16 m/s, nothing on a near-zero time step or a 45 cm jump), about 10 live; built without a GPU, not seen (lane B43) (311478 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dash_skid.uasset | vfx | 1.37 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the dash's skid, attached at the root on Dash_Skid frame 2: three bursts over the hold of 6 to 12 pale streaks and 1 to 3 chalk discs thrown ahead along User.Dir; User.Strength; nothing without a direction or on a near-zero time step; peak 45; built without a GPU, not seen (lane B43) (1436799 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dash_wipeout.uasset | vfx | 0.72 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the dash's wipeout, at the root on Dash_Wipeout frame 13: a low pale cloud ahead of the chest (8 dabs) and at the hips (6) and 14 streak chips thrown ahead along User.Dir; peak 28; built without a GPU, not seen (lane B43) (757603 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dive.uasset | vfx | 2.00 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the dive block, attached at the root on Dive_Block frame 9: a line of 25 streak chips at five points across the body's front and 12 pale dabs thrown along User.Dir (the facing), then a light trail of small soft puffs by the owner's speed (clamped to 8 m/s); peak about 49; built without a GPU, not seen (lane B43) (2092720 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dive_hit.uasset | vfx | 0.70 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the puck strikes the lying blocker, at the puck: a cream star for 3 frames, 6 to 10 streak chips and 3 pale dabs; User.Strength; needs no direction; peak 14; built without a GPU, not seen (lane B43) (735544 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_slide_tackle.uasset | vfx | 1.15 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the slide tackle, attached at the root on Slide_Tackle frame 2: three sprays of 5 streak chips ahead of the leading skate, then the contact burst 8 frames later (8 to 16 chips, 8 pale dabs) along User.Dir; User.Strength; peak 38; built without a GPU, not seen (lane B43) (1206323 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_mallet_bonk.uasset | vfx | 0.70 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the mallet's bonk at the drum's end face on Mallet_Swing frame 17: a cream star for 3 frames, 10 to 16 cream chalk dabs opening from a 22 cm ring, 6 small bits; User.Strength; needs no direction; peak 23; built without a GPU, not seen (lane B43) (737219 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_cannon_fire.uasset | vfx | 0.72 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the puck cannon at the keg's mouth on Cannon_Fire frame 8: a cone of 12 to 18 pale chalk dabs, 8 white shards and 4 slow wisps along User.Dir; no light, no fire colour; peak 30; built without a GPU, not seen (lane B43) (757464 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_giant_sweep.uasset | vfx | 2.22 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the giant stick's sweep at the root on Giant_Sweep frame 10: five sprays (5 streak chips and 3 pale dabs each) on a 205 cm arc from -44 to +44 degrees of User.Dir, one a frame; User.Variant 0 right to left / 1 left to right; peak 40; built without a GPU, not seen (lane B43) (2330474 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pin_roll.uasset | vfx | 0.93 | original | UE 5.9 Niagara via MCP/Python, no GPU |  |  | 2026-11-14 | B | y | the rolling pin at the root on Pin_Roll frame 16: a line of 12 to 21 streak chips 105 cm ahead across 90 cm and 8 cream chalk dabs along User.Dir; User.Strength; peak 29; built without a GPU, not seen (lane B43) (979589 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.35 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event lift as a cross-check: Variant 0 three pale-hot stars 20-26 cm and five dark brown-red spokes 7 x 24 cm at 0.72 m (below the number on the back), shrinking and fading over 10 to 12 frames, eight splinters gone by +6 frames, a snow kick of six streaks at the skates, peak 27; Variant 1 two small streaks; User.Dir read, default zero; rebuilt whole without a GPU in B43, not seen since (lanes B3b, B17, B33, B35, B37, B40, B42, B43) (1412764 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_ability.uasset | vfx | 1.46 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event ability on a use (Strength above 0.5), by User.Variant, small chalk dabs (M_Fx_Chalk): 0 shotgun a blue-grey blast of 44 dabs (0.62, 0.80, 1.30) and 24 white shards along User.Dir, 1 hip check an ochre arc of 46 on the lunge side, 2 shield a slate-blue ring of 60 with 10 white shards, life 54 to 60 frames, 3 slow puck a frost patch of 60 at the skater; a grant (Strength 0) shows nothing; User.Dir read, default zero (lanes B40, B42, FILM1, B43) (1527198 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.67 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | event goal: chalk dust in the scoring team's colour (User.Colour, lightened) thrown from the net out along the end boards both ways and up, 114 to 126 frames of life, and a cream fountain of streaks over the net; Variant 1 (conceded) at 60 % of the count; peak 94; User.Variant read (lanes B3, B42, B43) (706833 bytes) |
```
