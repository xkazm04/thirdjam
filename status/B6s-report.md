# B6s report: skater polish and a real-match preview (device B)

Lane B6s, 2026-11-12, 12:34 to 12:58 local. Branch `lane/b-skater` (fast-forwarded to `dev-b` 3291d19 first), worktree `C:/Users/kazda/jam/wt/b-skater`.
Editor target rebuilt once (`build.ps1`, PASS in 50 s), editor on port 8002 (pid 16100), closed through `ExitEditor` after a blank map was opened
(`/Game/Maps/Rink` never saved, no dirty map at exit). The port 8000 editor was up the whole time: every frame number below is **unconfirmed, two editors up**.
Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b6s` (`$B6` below).

## Headline

**Device A's game already reads the cartridge key on this tree.** `AHokejGameMode::ResolveVisualClass` maps `game.skater.visual` = `generated_b2` to
`/JamAssets/Skaters/BP_SkaterVisual_TB2.BP_SkaterVisual_TB2_C` and `humanoid_b1` to `BP_SkaterVisual_TB1`, and `AJamSkater::InitSkater` spawns the actor
visual, attaches it to the pawn root and drives it every sim frame. So I did not bolt the visuals on from Python: I ran the real path, with
no code change and no saved map, by setting the console variable `jam.Cut` before PIE. Both figures play full matches through device A's own code
(`hockey.test.visual-b2` for the hybrid; a scratch copy of that cartridge with `"visual": "humanoid_b1"` for T-B1). In play the **hybrid is the better
figure**, with two fixes still needed, below.

## Part 1: T-B2 polish

### a. Mitts: DONE

`M_SkaterTB2` has a second Custom node, `GloveKey` (`MaterialExpressionCustom_2`), after `TeamHueKey`. It feeds Base Color and the `TeamColour` input of
the Rim+Emissive node:
`g = pow(Tex, 0.4545); mask = saturate((min(g.r,g.g) - g.b) * GloveKeyGain - GloveKeyBias); out = lerp(In, GloveColour * dot(g,(0.3,0.59,0.11)) / 0.72, mask)`.
The 0.72 is the measured median luminance of the mitts, so the default colour lands on the mitt's mid-tone and the painted shading stays.

| Parameter | Kind | Default |
|---|---|---|
| `GloveColour` | vector | (0.10, 0.08, 0.07, 1), dark leather |
| `GloveKeyGain` | scalar | 6.0 |
| `GloveKeyBias` | scalar | 1.0 |

Measured on the texture (gamma space, `min(r,g)-b`): mitts 0.32 / 0.37 / 0.39 (p10/p50/p90), face skin 0.07 / 0.09 / 0.12, white about 0. With 6 and 1.0
the mitts key fully and the skin not at all. Checked on blue and orange in PIE with a before/after taken on the same frame (`GloveKeyGain` 0 on the
dynamic instances for "before"): face, white stripes, helmet, skates and stick are unchanged. **One side effect, which I think helps:** the warm
tan knee band above the boot cuff is the same yellow-orange in the texture, so it now turns leather too. Before, it read as an orange accent on both teams.
Still: `devlog/shots/20261112-124227-B-b6s-mitts.jpg` (left before, right after).

### b. Boot mark: NOT A TEXTURE MARK, no patch made

The "three short dark diagonal strokes" are the **modelled lace bars on the boot**: geometry with the outline pass drawn around each bar. They are not texels.
The evidence:
- With the orange figure switched to a flat material (no texture), the lace bars are still there as ridges (left half of the still).
- Close up with the real material, the strokes are bars with outlines and orange inside (right half).
- In `T_SkaterTB2_BaseColor_clean.png`, inside the boot UV region (faces with mesh z between -0.69 and -0.40, 398k px), I searched three ways and found nothing to
  patch: no navy pixel darker than L 0.11; no pixel that fails the hue key inside a navy area (which would show dark on the orange team); and a local
  dark-stroke search turned up only seams and edges. Lane S6 had already flattened the lace tiles to boot navy.

So `T_SkaterTB2_BaseColor_clean2.png` was **not** made and nothing was reimported: a pixel patch cannot remove geometry. Two fixes would: flatten the lace
bars onto the boot in Blender (lane S5's mesh, about 160 small lace islands per lane S6's notes; lane S6 itself suggested this), or accept them as laces.
From the game camera they are never visible (see the match stills). I also saw one small dark dash on the outer upper boot that comes from the texture.
It is a few pixels and is not patched.
Still: `devlog/shots/20261112-124227-B-b6s-boot.jpg`.

### c. Unwired clips, single-node, frozen (left T-B2 late in the clip, right T-B1 early)

| Clip | T-B2 | T-B1 | Verdict |
|---|---|---|---|
| `Celebrate` (1.2 s) | stick held level across the face at chin height in both mitts, arms up | stick raised over the head, both hands, clean | **Fit to use on both.** T-B1 reads better. T-B2's stick sits in front of the face mid-clip. |
| `Stop` (0.6 s) | sideways hockey stop, trailing leg out, body leaning back | low sideways crouch with the stick down | **Fit on both.** Reads as a stop. |
| `Shot_Wrist` (0.35 s) | follow-through, stick across the body | sweep with both hands on the stick | **Fit on both**, but it is short (21 frames): use it as a montage, not as a state. |

Stills: `20261112-124228-B-b6s-clip-celebrate.jpg`, `-clip-stop.jpg`, `-clip-wrist.jpg`. Nothing was wired (the interface has no call for them).

### d. Extra fix found in play: lean smoothing (both figures)

`BS_SkaterTB1_Skate` and `BS_SkaterTB2_Skate` had interpolation time 0 on both axes, so the lean jumped as soon as `TurnRate` jumped. In play `TurnRate` does
jump: the sim's bots turn at the full ±5 rad/s and flip sign between samples about 0.3 s apart, even at 0.4 m/s. I set the axis smoothing
(spring damper, the type already set) to **Speed 0.15 s, TurnRate 0.25 s** on both. Measured afterwards in the match (time dilation 0.2, samples 0.1 s apart): the head's
lateral offset ramps 0 → 7 → 15 → 17 cm over about 0.3 s after a turn starts, where before the jump was immediate.

## Part 2: the real-match preview

### How the visuals were attached and driven (what device A's lane needs to know)

Nothing new is needed in device A's code for this to work. The chain, read from `Game/Source` and verified in PIE:
1. `AHokejGameMode::StartPlay` loads the cartridge named by `-cut=`, the `cut` URL option, or the console variable **`jam.Cut`** (default `hockey.regular`; a
   value ending in `.json` is used as a path).
2. `BuildMatch` calls `ResolveVisualClass(G.Skater.Visual)`. `generated_b2` gives `BP_SkaterVisual_TB2_C` and `humanoid_b1` gives `BP_SkaterVisual_TB1_C`;
   anything else, or a failed load, gives `UToySkaterVisualComponent`.
3. `AJamSkater::InitSkater` (pawn class **`AJamSkater`**, module JamCore; root component `Root`, a `USceneComponent` at ice height) sees an actor class, spawns it at the pawn transform
   with the pawn as owner, `AttachToComponent(Root, SnapToTargetNotIncludingScale)`, then calls `SetTeam(team, G.Teams[team].Colour)`, `SetNumber`,
   `SetHasPuck(false)`, `SetBoost(None)`. No toy component is created, so nothing has to be hidden.
4. Every sim frame (60 Hz), `AJamSkater::ApplySimFrame` calls `UpdateMotion(|Vel| m/s, -YawRate rad/s, FRotator(0, Yaw deg, 0))` (sign already flipped to the
   contract's "positive = left"), plus `PlayShoot` on `bShotThisFrame`, `PlayCheck` on `bCheckThisFrame`, `SetKnockedDown(down, KnockDir.X, KnockDir.Y)` on change,
   and `SetHasPuck` on change. Shots and knock-downs are therefore **forwarded by the game itself**. This cartridge has `check_enabled: false`, so no knock-down
   happened in my runs.

Verified in PIE: all six `AJamSkater` pawns report `GetVisualObject()` = `BP_SkaterVisual_TB2_C` (then `_TB1_C`). The visual sits at offset (0,0,0)
from the pawn root. The AnimBP receives `Speed` 0.2 to 8.9 and `TurnRate` -5 to +5. `LR_L2_Night` is streamed in by the game.

What I ran (editor Python on port 8002; scripts in `$B6`):
- `z_b2.py`: blank map, load `/Game/Maps/Rink` (4 actors, unmodified), `jam.Cut hockey.test.visual-b2`. Then `StartPIE` and `jam.ScriptHuman 1`, so the
  human's skater plays itself (only for these shots).
- T-B1: `jam.Cut $B6/hockey.test.visual-b1.cartridge.json` (copy of `hockey.test.visual-b2` with id, title and `"visual": "humanoid_b1"` changed; the cartridge loaded and validated).
- Faceoff: `GetGameMode().restart_match()` at global time dilation 0.02, then capture. Sequences: time dilation 0.19, 25 `CaptureGameViewport` calls,
  game time logged per frame (0.21 s steps over 5.1 s).

**For the Director and device A:** the swap seam works today with no code change. What is missing is a non-test cartridge choice (contract C-0001
open question 1: the value names in code are `toy` / `humanoid_b1` / `generated_b2`, not the contract's proposed `TA` / `TB1` / `TB2`), and the bot test
with the swapped visual.

### Stills (game camera, 1920 wide, PIE, `devlog/shots/`)

| Still | What |
|---|---|
| `20261112-125215-B-match-tb2h-1.jpg` | hybrid, carrier with the puck and an orange checker, between light pools |
| `20261112-125216-B-match-tb2h-2.jpg` | hybrid, four skaters in a light pool |
| `20261112-125216-B-match-tb2h-3.jpg` | hybrid, two blue overlapping near the camera, TAA ghost on moving limbs |
| `20261112-125216-B-match-tb2h-4.jpg` | hybrid, **faceoff**, four skaters in frame |
| `20261112-125216-B-match-tb2h-5.jpg` | hybrid, five skaters in frame |
| `20261112-125216-B-match-tb1-1.jpg` | T-B1 in play, three orange at distance |
| `20261112-125216-B-match-tb1-2.jpg` | T-B1, four skaters in frame (after a goal) |
| `20261112-125217-B-match-tb1-3.jpg` | T-B1, human striding in a light pool |
| `20261112-125217-B-match-tb1-4.jpg` | T-B1, **faceoff** |
| `20261112-125217-B-match-tb1-5.jpg` | T-B1, four skaters in frame |

### Clips (scratch, not committed)

- **Hybrid: `$B6/match-tb2h.mp4`** (620,970 bytes, 1280 px, 25 frames at 0.21 s of game time = 5.1 s, played at 5 fps), lean smoothing on.
- `$B6/match-tb1.mp4` (620,561 bytes): T-B1, the **same 5 s after `RestartMatch`** (same events at the same game times), for a direct comparison.
- `$B6/match-tb1-vs-tb2h.mp4` (704,223 bytes): both side by side (left T-B1, right hybrid).
- `$B6/match-tb2h-nosmooth.mp4` (685,208 bytes): an earlier hybrid run before the lean smoothing.

### Judgement in play (game camera)

- **Do they read as hockey players?** Yes, both, at every distance the camera gives. The hybrid reads as a round toy player with helmet, mitts, stick and white
  stripes. T-B1 reads as a blockier player with dark breezers.
- **Team colour at distance:** both hold. The hybrid's blue is strong. Its orange turns brown-red in the dark gaps between the four light pools (the hue key
  multiplies team colour by the texture's luminance). T-B1's orange stays orange and its black breezers keep the figure readable in the dark. **Between pools T-B1 reads better.**
- **Slide or pop:** feet slide over the ice by design (no root motion; the pawn moves). That is fine for skating and I saw no foot skating that looks wrong at
  this camera distance. Pops: the lean snapped before the smoothing fix and now eases over about 0.3 s. Resets after a goal teleport the pawn (the game's own doing).
  TAA leaves a ghost on fast-moving limbs near the camera (seen on both figures, and in B4's stills).
- **Stride rate against speed:** believable. Speeds in play are 0 to 8.9 m/s (cartridge top speed 8). The blend gives full stride from 5 m/s at rate 1.0 (one
  cycle per 0.98 s) up to rate 1.3 at 8 m/s (0.76 s per cycle, about 3 m per push). Below 1 m/s it is glide or idle. I could not check foot plant against
  ice travel frame by frame (0.2 s steps are too coarse).
- **Lean direction:** correct. Measured on all six visuals: `TurnRate` -5 (turning right) puts the head 16.7 cm to the figure's right, and +5 puts it 17.2 cm to the left,
  so both lean into the turn. **Weakness:** the bots spin at the full ±5 rad/s even at 0.4 m/s, and the blend space leans fully at 0 m/s too, so a nearly still figure leans
  hard while it turns on the spot. The lean should scale with speed. Moving the lean samples at Speed 0 to a smaller turn rate, or feeding `Speed x TurnRate`, would fix it.
- **Six together:** they hold together at distance. The puck is not on the blade: the sim carries the puck in front of the pawn, while both figures hold the
  stick on the right, so the puck sits left of the blade (seen in the hybrid sequence). Device A could place the carried puck at `puck_socket`, or B could centre the blade.
- **Which is better in play:** the **hybrid**, for charm and team colour in the light pools, and the faceoff and stride read well. T-B1 reads better in the dark gaps and at the far end.
  If the lead keeps the hybrid, the two fixes that matter in play are the dark-gap orange (raise `Emissive` or `JerseyLum` for orange, or light the gaps) and the
  puck/blade offset. The lace outlines do not show at game-camera distance.

### Paired frame time, toy against hybrid (device B, unconfirmed, two editors up)

Medians of 40 `GetFrameTimings` samples, PIE floating 1920 x 1080, match running with `jam.ScriptHuman 1`, all three taken within about 45 s:

| Time | Visual | GPU ms | Game thread ms | Frame ms |
|---|---|---|---|---|
| 12:48:00 | toy (`hockey.regular`) | 12.28 | 14.39 | 16.67 |
| 12:48:22 | hybrid (`hockey.test.visual-b2`) | 12.28 | 14.27 | 16.67 |
| 12:48:45 | toy | 12.43 | 14.91 | 16.68 |

No measurable difference: the frame is capped at 16.67 ms and the GPU is not the limit. This is device B, not the gate.

## Files changed (all under `Game/Plugins/JamAssets/Content/Skaters/`)

| File | Bytes before | Bytes after | Change |
|---|---|---|---|
| `M_SkaterTB2.uasset` | 22,040 | 24,140 | `GloveKey` node, `GloveColour`, `GloveKeyGain`, `GloveKeyBias` |
| `BS_SkaterTB2_Skate.uasset` | 25,144 | 25,417 | axis smoothing Speed 0.15 s, TurnRate 0.25 s |
| `BS_SkaterTB1_Skate.uasset` | 24,880 | 25,140 | axis smoothing Speed 0.15 s, TurnRate 0.25 s |

`T_SkaterTB2_BaseColor` is not changed. Nothing exists under `_Scratch/` (none was created; the folder is absent on disk). No map was saved, and nothing under `/Game/` changed.
Stills: 15 `.jpg` files plus `.txt` captions in `devlog/shots/` (`b6s-*` and `match-*`; 119,063 to 225,744 bytes each).

## Ledger row changes

No new rows. Notes to update:
- `M_SkaterTB2.uasset` (lane B5's row): note becomes "hue-key team recolour plus glove key (GloveColour, GloveKeyGain, GloveKeyBias; lane B6s); copy of M_Toon".
- `BS_SkaterTB2_Skate.uasset` and `BS_SkaterTB1_Skate.uasset` (lane B4's rows): add "axis smoothing 0.15 / 0.25 s (lane B6s)".

## Commits

On `lane/b-skater`: one commit with the three assets, the stills and this report (hash in the final message).

## Not verified

- A knock-down in a real match (this cartridge has `check_enabled: false`). `PlayShoot` and the falls are reached through device A's code, but I did not watch
  either in a match still.
- The contract's same-state-hash check across visuals. The hybrid and T-B1 sequences after `RestartMatch` show the same events at the same game times
  (a goal at about 2.7 s, the reset), which fits, but I did not compare hashes or run bot tests.
- Foot plant against ice travel (frames too coarse). Frame time at device A's gate.
- Whether the port 8002 lingering `LISTENING` (owner pid 0) after `ExitEditor` clears. The editor process is gone, but the line stayed for at least 10 s.

## For the lead and device A to decide

1. Make the visual choice real outside the test cartridge: settle open question 1 of C-0001 (`toy` / `humanoid_b1` / `generated_b2` is what the code accepts),
   then pick the shipped default and run the bot test with it.
2. Hybrid or T-B1 in play. My pick is the hybrid, if (a) orange gets a stronger `Emissive` or the gaps between pools get light, and (b) the carried puck goes to
   `puck_socket` (device A) or the blade moves to centre (device B).
3. The lace bars: flatten them in the mesh (lanes S5/S6), or keep them as laces.
4. Lean at standstill: scale the lean by speed (device B, blend space), or have the bots not spin at full rate when slow (device A).
5. The knee band now follows `GloveColour`. Keep it, or narrow the key to the hands only (that needs a UV or vertex mask).
