# Lane B18 report: device B's share of the QA1 faults (device B)

- Tree `C:/Users/kazda/jam/wt/b-vfx` (built on A's `main` of 20:53). **No git, no build.** Clock 21:40 to 22:17.
- Editor on port 8002 (pid 39416), started 21:41. It left through `ExitEditor` at 22:15:31 (`LogExit: Exiting.`), after StopPIE. **Dirty packages: none**, checked just before the exit.
- `/Game/Maps/Rink` was never saved or reloaded (mtime still 12:40, 18,966 bytes). `LR_L2_Night` was not opened or saved (mtime still 18:42). Nothing is under `_Scratch/`. `Game/Saved/BouncedWavFiles/b18/` was emptied and removed.
- Cvars set back: `jam.Cut hockey.regular`, `jam.ScriptHuman 0`. `bAllowBackgroundAudio` is on: it is a per-user editor setting and nothing is tracked.
- The port-8000 editor shared the GPU, so there are **no frame numbers**.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b18/`. It holds the helpers (`q.mjs`, `calls.mjs`, `ms.cjs` MetaSound builder, `nsbuild.cjs`, `fxpeak.py`, `trailtest.py`, `hook.py`, `ana.py`), every capture under `shots/`, and the wavs under `wav/`.

## Verdicts

| # | Item | Verdict |
|---|---|---|
| 1 | Motion blur off on clay and broadcast | **PASS** |
| 2 | Puck-trail smear | **PASS** (controlled test on three tiers; in play the carried puck shows no smear) |
| 3 | Effects and sounds for the new events | **PASS** for 3 effects and 5 sounds, all fired through the real binder. `carry`: nothing built, by design |
| 4 | Contact puffs hide the figures | **PASS** (`NS_knockdown`). `NS_check` has no puff emitter and is unchanged |
| 5 | VSM queue overflow on clay | **NOT FIXED, not a rig setting.** The warning is intermittent and was not reproduced in 3 of the last 3 clay starts |
| 6 | Small effects in play | **DONE**: shot and pass do not read at the game camera; lift and poke barely read; blade spray reads |

### 1. Motion blur (PASS)

- **Before:** both rig post-process volumes (`PostProcessVolume_1`, unbound) had `override_motion_blur_amount` false, so the engine default (0.5, max 5 %) ran.
- **After:** both volumes have `override_motion_blur_amount` true with amount 0, and `override_motion_blur_max` true with max 0.
- Read back in PIE on both tiers: `mb_override true, mb 0.0, max 0.0`.
- The clay depth of field is unchanged: f/3.2, override on.
- The follow camera's own settings do not override motion blur (`override false`), so the volume decides.
- Stills (bot match, game camera, during turns):
  - `devlog/shots/20261112-221519-B-b18-mb-clay`: before, the figures are smeared; after, they are sharp.
  - `devlog/shots/20261112-221519-B-b18-mb-bcast`: before, the human is streaked on a turn; after, it is sharp.
- **Night tier not touched**, as instructed: the other lane sets it in `LR_L2_Night`. Its blur is still visible in my item 6 night frames: a streak trails a moving skater.

### 2. Puck trail (PASS)

**Root causes found:**
1. **Device A never sets `User.Speed`.** `JamPawns.cpp` spawns `NS_puck_trail` attached to the puck and sets nothing. The default was 10 m/s, so the alpha was always full, whatever the puck did.
2. The emitter's **Loop Behavior was Once with a 5 s duration.** The trail died 5 s after spawn: the PIE probe read `active: false` minutes into a match. QA1's smear was therefore seen early in each match build.

**Changes to `NS_puck_trail`:**
- **Speed:** `max(User.Speed, length(Engine.Owner.Velocity) / 100)` m/s. The trail measures its own speed, and `User.Speed` still works if A ever sets it. Its default is now 0.
- **Alpha and width:**
  - alpha = `saturate((speed - 12) / 4) * step(speed, 45)`;
  - ribbon width = 3 + 7 × alpha cm;
  - so nothing shows at 12 m/s or under, full from 16 m/s, and nothing above 45 m/s (that step hides the one-frame jump when the puck is teleported to a faceoff).
- **Why 12 / 16 m/s instead of 9 / 14:** the cartridge's `game.skater.speed` is **11 m/s, not 8**, and a carried puck moves at the carrier's speed. A pass is 14 m/s (`PassSpeed` default; it shows about half). A shot is at least 18 m/s (`shot.speed`; it shows full).
- **Life:** 0.35 → 0.2 s.
- **Loop Behavior:** Infinite.
- **Material:** `M_Fx_Ribbon` (additive) → new **`MI_Fx_Trail`** (instance of `M_Fx_Tier`, Shape 0 = a tapered streak):
  - night: accent × 12, no rim (today's look);
  - clay: dark olive-brown streak (ClayBright 0.55, rim 0.5);
  - broadcast: dark olive streak (BcastBright 0.45, tint 0.55/0.6/0.5, rim 0.6).
- The colour still comes from `User.Colour`.

**Evidence:**
- `devlog/shots/20261112-221519-B-b18-trail-tiers`: four trail copies circle at 8, 11, 14 and 20 m/s on night, clay and broadcast.
  - 8 and 11 m/s draw nothing.
  - 14 m/s is faint.
  - 20 m/s draws a 4 m streak: yellow-green on night, a darker streak on cream clay and on white broadcast ice.
- In play after the change, carried pucks show only the halo disc, with no trail: broadcast frames `mb-bc-after-5/6` (scratch) and night frames in `b18-play-events`.
- A real shot's trail in play was not caught in a still (see "Could not verify").
- **Before:** QA1's `qa1-r1-strip` and `qa1-b2-strip` (night). There are no before trail stills on clay or broadcast: I edited the asset before capturing them. B17's `b17-before-tiers` shows that the old additive material did not show on those tiers at all.

### 3. New events (PASS)

**Read from the contract:**
- `clock` fires **once per timed take**, at 600 frames (10 s) left, with Kind `last_10s`. It also fires with `sudden_death` when a tie at the horn plays on.
- `season_take`, `period_end` and `clock` all carry Strength 0.
- `carry` fires once per possession, after 300 frames of carrying, with Kind `long` or `coast`. It does not fire every frame, but it is a commentator read-out with nothing to show on the ice.

| Asset | Built | Peak / loudness at Strength 1.0 (0.3), length |
|---|---|---|
| `NS_save` | From `NS_check`. 6 + 10 S ice chips at the puck (5-13 cm), a ring of 10 + 8 S white dots (15 % Colour) flung flat from 18 cm, a 35-60 cm clack star at 25 cm. Life 0.15-0.3 s. Reads `User.Strength`, `User.Colour` | - |
| `MS_save` | `SFX_BodyCheck_02` −2 st, low-passed 700 Hz with a 0.22 s envelope (the padded thump); `SFX_PuckBoards_01` +3 st, low-passed 2.6 kHz (the stick clack); 90 Hz sine thump. Gain 0.45 + 0.55 × S | −8.8 dBFS / **−24.9 LUFS** (−14.3 / −28.6), 0.25 s |
| `NS_post` | From `NS_check` without the ring: a 45-80 cm ping star at 30 cm; 3-4 sparks (`MI_Fx_Spark`, warm white, 10 % Colour) shot up along the post at 3-5.5 m/s with gravity. Life 0.18-0.35 s | `MS_post` (lane AUD) unchanged |
| `NS_breakaway` | From `NS_shot`: 2-3 long thin chips (7 × 90-150 cm, velocity-aligned) streaking flat on the ice at 1.5-2.6 m/s, no gravity, 0.35-0.45 s, ice white with 60 % of Colour. Nothing above 3 cm. Radial: the payload has no direction | - |
| `MS_breakaway` | `SFX_SkateCarve_02`, pitch rising 0 → +5 st over 0.5 s, high-passed 400 Hz, swelling in 0.3 s and out 0.35 s. Gain (0.6 + 0.4 × S) × 1.7 | −12.3 / **−30.6 LUFS** (−15.2 / −33.5), 0.57 s. Quiet by request |
| `MS_period_end` | Arena horn in B16's goal-horn voicing: saws on 233.1 and 236.6 Hz plus 116.55 Hz for body, low-passed 1.3 kHz, attack 0.03 s, 1.4 s. Fixed gain | −13.1 dBFS / **−20.2 LUFS** at any Strength, 1.43 s. The goal burst measured in the same session is −17.1 LUFS, so the horn sits 3.1 dB under it |
| `MS_clock` | Ten soft ticks, one per second (TriggerRepeat, stopped at 9.5 s): a 2.4 kHz band click plus a 1180 Hz sine blip. Input `Variant`: 0 plays, 1 is silent | Ticks measured at 1.06, 2.06 … 10.06 s; −18.2 dBFS peak, about −29.6 LUFS per 2 s window. Variant 1 measured silent (below −240 dB) |
| `MS_season_take` | Two-note rink-organ sting, G4 then C5 at 0.3 s (sine, octave sine, soft square), low-passed 3 kHz, 0.82 s, no film sound | −13.9 dBFS / **−21.1 LUFS**, 0.82 s |

How the sounds were checked:
- Every MetaSound is a one-shot in `SC_SFX`, ending through `OnFinished`, built through the MetaSound toolsets.
- They were recorded from the PIE output submix with the world's audio components stopped and the game frozen. `MS_goal` and `MS_check` were recorded in the same session as references: −17.1 and −26.6 LUFS, matching B16 and AUD.
- All levels are under the goal. Save, clock and breakaway sit in the contact family (−25 to −31). The horn and the sting are louder, on purpose, as stingers.

Run through **device A's real binder** in PIE (`UJamEffectsBinder.handle_event` called on the live binder):

| Event | Effects spawned | Sounds played |
|---|---|---|
| save | +1 | +1 |
| post | +1 | +1 |
| breakaway | +1 | +1 |
| period_end | 0 | +1 |
| clock | 0 | +1 |
| season_take | 0 | +1 |

Stills:
- `devlog/shots/20261112-221520-B-b18-fx-new-night`: rows save, post, breakaway; each at 25 m and 8 m, ages 0.04 / 0.10 / 0.20 s, on a lit pool.
- `-fx-new-tiers`: all three on night, clay and broadcast.

Readings:
- **Night:** all three read at 8 m. At 25 m, save and post are small specks and breakaway is invisible (small by request).
- **Clay and broadcast:** save reads (dark crumbs; blue-grey dots). Post is faint: a grey star on white ice, as with B17's check star. Breakaway barely shows.

**No `NS_period_end`, `NS_clock` or `NS_season_take` was built, and nothing was built for `carry`.** Their missing-asset lines are `Log` verbosity, logged once per editor process. A fix in device A's lookup list is cleaner than spawning empty systems (see "For device A").

### 4. Contact puffs (PASS)

- **`NS_knockdown` Puff:**
  - size min 55 + 30 S → **30 + 15 S** cm;
  - size max 90 + 40 S → **45 + 20 S** cm;
  - alpha scale 0.6 → **0.3** × (1 − age);
  - spray chips unchanged.
- **`NS_check` is unchanged.** It has no puff emitter; the puffs QA1 saw over the figures are the knockdown's, which fire on the same frame. Its chips, ring and star are kept, as instructed.
- `devlog/shots/20261112-221520-B-b18-hit-puffs` shows check plus knockdown at Strength 1 on the human, ages 0.05 / 0.13 / 0.25 s, with the same spawn before and after.
  - Before: by 0.25 s the puff wash greys the legs and hides the 7.
  - After: the jersey colour and the 7 read through the burst.
  - At 0.13 s the white check and knockdown chips still cross the lower body for a moment; the jersey stays blue.

### 5. VSM overflow on clay (no change)

**What is in the clay rig:**
- 49 actors: 38 static meshes, all shadowless; 3 shadowless spot lights; 1 directional key light (the only shadow caster); a sky light; the post-process volume; `BP_LookParams`.
- The rig contributes **0** shadow-casting non-Nanite primitives.

**Every non-Nanite caster in the PIE world on clay is in A's `Rink` (15):**
- 7 `RinkActor` procedural meshes (boards and glass spanning the rink);
- 6 skater skeletal meshes;
- the puck;
- the 120 × 60 m temporary floor `StaticMeshActor_0`, which sits under the ice and still casts shadow.

**When the warning fires:**
- In this session it fired twice, each time a few seconds after a PIE session that **started** on clay (20:43 and 21:06 log time).
- It never fired on a live switch into clay: 6 switches, 0 warnings.

**Count over 30 s after the clay switch:**
- with the floor's shadow turned off in PIE memory: 0;
- control, unchanged: 0;
- a match started on clay, unchanged: 0.

So the warning is intermittent and I could not reproduce it for a before and after. Nothing in the rig was changed for it. The likely trigger is a full VSM invalidation at a fresh start, with large non-Nanite boards, glass and floor in the directional key's clipmap. It is a performance note only.

### 6. Small effects in play (night, bot match)

- **Method:** a Python callback on `OnJamEvent` set time dilation to 0.02 when an event fired; a poller then captured the game camera and released it. In 240 s it caught 2 each of lift, poke, shot and pass, all in one corner scrum.
- **Still:** `devlog/shots/20261112-221520-B-b18-play-events` (lift, lift, poke, shot, pass, pass).

| Effect | At the game camera |
|---|---|
| `lift` | **barely**: a small star and chips at the skates, half hidden by the figure's legs and the HUD ring |
| `poke` | **barely to yes**: the star at the blade reads, the chips do not |
| `shot` | **does not read**: no chip streaks visible in either capture (30 streaks of 10 × 40 cm under the HUD reticle and the skates) |
| `pass` | **does not read** (6 sprites of 3-6 cm; tiny by design) |
| `blade_spray` | **reads**: white fans behind turning skaters (`humanoid_b1` / TB1 figures), visible in the pass frames |

`save`, `breakaway` and `post` did not fire in the 240 s.

## For device A (the lead passes this on)

1. **`carry`: no lookup at all.** Add it to `UJamEffectsBinder::IsSilentEvent`. It is a commentator read-out, so no effect and no sound.
2. **Mark `period_end`, `clock` and `season_take` sound-only** (`IsSoundOnlyEvent`). `MS_` exists for each, and no `NS_` is planned. Today the binder logs a "missing NS_" line for each.
3. **Play `period_end`, `clock` and `season_take` 2D** (add them to `IsSound2D`). They are arena-wide and their `Where` is zero; today they play 3D at centre ice.
4. **`clock` Variant.** Map Kind `last_10s` → 0 and `sudden_death` → 1 in `VariantFor`. `MS_clock` plays its 10-tick sequence at Variant 0 and is silent at 1. Without the mapping, a sudden-death `clock` plays 10 ticks.
   - The ticks run on audio time, not sim time: a pause or slow motion in the last 10 s puts them out of step with the clock.
5. **`NS_puck_trail`.** No code change is needed now: the trail measures its own speed. If A wants exact sim speed, `Trail->SetVariableFloat("Speed", PuckSpeedMps)` each tick works (the larger value wins).
6. **Floor plane** (`StaticMeshActor_0` in `Rink`, under the ice): suggest `CastShadow = false`. It is a needless 120 × 60 m non-Nanite shadow caster; whether it causes the VSM warning is not proven (item 5).
7. **Puck halo disc ghost (seen in passing).** On broadcast a faint pale smear trails the carried puck's halo disc. This looks like B3's Lumen ghost; the disc component still needs `bAffectDynamicIndirectLighting = false` (B3 lead item 3).

## Assets created or changed (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Rigs/LR_P3_Clay.umap` | 135,445 | 135,667 |
| `Rigs/LR_P4_Broadcast.umap` | 464,246 | 465,441 |
| `VFX/NS_puck_trail.uasset` | 311,628 | 316,523 |
| `VFX/MI_Fx_Trail.uasset` | new | 11,519 |
| `VFX/NS_knockdown.uasset` | 533,445 | 500,995 |
| `VFX/NS_save.uasset` | new | 705,031 |
| `VFX/NS_post.uasset` | new | 497,410 |
| `VFX/NS_breakaway.uasset` | new | 287,961 |
| `Audio/MS_save.uasset` | new | 171,738 |
| `Audio/MS_breakaway.uasset` | new | 141,127 |
| `Audio/MS_period_end.uasset` | new | 133,553 |
| `Audio/MS_clock.uasset` | new | 158,917 |
| `Audio/MS_season_take.uasset` | new | 184,167 |

- Not changed: `NS_check`, `Look/`, `Skaters/`, `Props/`, `LR_L2_Night`.
- `NS_save` keeps `NS_check`'s unused `User.Dir` parameter (harmless).

Ledger rows. **Replace** the rows for the two rigs, `NS_puck_trail` and `NS_knockdown`; **add** the rest:

```
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 0.1 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | P3 clay rig: PPV (focus 11 m f/3.2, depth blur, motion blur 0), sky, directional key (only shadow caster), 3 fills, LookParams; tabletop, paper sweep, wire/card light stands with lit lenses (lanes B9, B14, B18) (135667 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_P4_Broadcast.umap | other | 0.5 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | n | broadcast rig: PPV (EV100 4.2, no blendables, motion blur 0), sky, 8 shadowless rect wash lights, 2 shadowed key spots, LookParams (Clay -1), stepped stands, roof trusses, lamp boxes, dark upper volume (lanes B14, B18) (465441 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | puck ribbon trail: speed from max(User.Speed, owner velocity), invisible <=12 m/s, full >=16 m/s, life 0.2 s, loops forever, tier material MI_Fx_Trail; User.Speed (m/s, default 0), User.Colour (lanes B3, B18) (316523 bytes) |
| Game/Plugins/JamAssets/Content/VFX/MI_Fx_Trail.uasset | material | 0.0 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-12 | B | y | instance of M_Fx_Tier for the puck trail: accent glow on night, dark streak on clay and broadcast (lane B18) (11519 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-12 | B | y | event knockdown: snow spray and puffs (puffs 30-65 cm, alpha 0.3 so the figures read through), peak 54 (lanes B3, B17, B18) (500995 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_save.uasset | vfx | 0.7 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | event save: ice chips at the puck, small white dot ring, 35-60 cm clack star; <=0.3 s, peak 35; User.Strength, User.Colour (lane B18) (705031 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_post.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | event post: ping star 45-80 cm plus 3-4 sparks up the post; <=0.35 s; User.Strength, User.Colour (lane B18) (497410 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_breakaway.uasset | vfx | 0.3 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-12 | B | y | event breakaway: 2-3 long thin chips streaking flat on the ice, 0.35-0.45 s; User.Strength, User.Colour (lane B18) (287961 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_save.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-12 | B | y | MetaSound source; save: padded thump (SFX_BodyCheck_02 -2 st LP 700 Hz) + stick clack (SFX_PuckBoards_01 +3 st LP 2.6 kHz) + 90 Hz thump; gain 0.45 + 0.55 x Strength; -24.9 LUFS at 1.0 (lane B18) (171738 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_breakaway.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-12 | B | y | MetaSound source; breakaway: SFX_SkateCarve_02 pitched 0 to +5 st over 0.5 s, HP 400 Hz, swell; -30.6 LUFS at 1.0 (lane B18) (141127 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_period_end.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); period_end: arena horn 1.4 s (Bb3 detuned saws + Bb2, LP 1.3 kHz), fixed gain, -20.2 LUFS (3 dB under the goal) (lane B18) (133553 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_clock.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); clock: ten soft ticks 1 s apart; input Variant (1 = silent) (lane B18) (158917 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_season_take.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); season_take: two-note rink-organ sting G4-C5, 0.82 s, -21.1 LUFS (lane B18) (184167 bytes) |
```

## Stills (`devlog/shots/`, each with a caption `.txt`)

- `20261112-221519-B-b18-mb-clay`
- `20261112-221519-B-b18-mb-bcast`
- `20261112-221519-B-b18-trail-tiers`
- `20261112-221520-B-b18-hit-puffs`
- `20261112-221520-B-b18-fx-new-night`
- `20261112-221520-B-b18-fx-new-tiers`
- `20261112-221520-B-b18-play-events`

## Could not verify

- **Nobody has listened to the new sounds.** They are verified by level, length and tick timing from recordings only.
- **The new events in a real match.** `save`, `post`, `breakaway`, `period_end`, `clock` and `season_take` did not fire in my bot runs (first-to-3 cuts have no clock). They were proven through the live binder's `HandleEvent` with hand-made payloads, not by the sim.
- **A real shot's trail in play** was not caught in a still; the speed gating was proven on test trails driven at known speeds.
- **Item 5's before and after:** the warning is intermittent (2 in this session, then 0 in 3 targeted runs).
- **Partial arc:** the white "arc" of `NS_save` is a small full ring of dots. The Hemicircle input is hidden behind a static switch in the template's shape module.
- **`NS_breakaway` has no direction:** the payload has none, so the streaks are radial. It is barely visible on clay and broadcast.
- No GPU or frame numbers (shared GPU).

## Lead must decide

1. **Pass the device A list above** (items 1-4 at least: carry silent; period_end, clock and season_take sound-only and 2D; the clock Variant).
2. **Trail thresholds 12 / 16 m/s** follow the real skater speed of 11 m/s. A speed-boost pickup (×1.5) lifts a carry to 16.5 m/s, so a boosted carrier now draws the trail. Accept this (it reads as "boosted"), or raise the floor.
3. **`NS_shot` does not read at the game camera in play.** Options: enlarge or brighten its streaks, or accept it, since the sound carries the shot.
4. **Post and breakaway on clay and broadcast are faint** (grey star; thin streaks). Options: accept, or raise their brightness on those tiers.
5. **The night rig's motion blur** belongs to the other lane (`LR_L2_Night`). Item 6's night frames still show the blur: a streak trails moving skaters.
