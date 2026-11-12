# Lane B3 report: puck readability kit, event effects and sounds (device B)

- Worktree `C:/Users/kazda/jam/wt/b-vfx`, branch `lane/b-vfx`.
- Editor on port 8001: started 10:38, exited cleanly through `ExitEditor` at about 11:12.
- Done: part 1, the whole M1 set (including `directors_call` from R-0002), M2/M3, both loops, and the sound classes with their mix. Everything was checked in PIE. Every M1 event was finished and verified by 11:03.
- Commits: `a45d2ca` (part 1), `0e14516` (M1 MetaSounds + NS_goal), `6d17bde` (M1 Niagara), `34118f9` (M2/M3, loops, sound classes), and the commit carrying this report.

## Part 1: puck readability kit

| Asset | Verdict | Evidence |
|---|---|---|
| `/JamAssets/Props/SM_PuckDisc` | **PASS** | **A real static-mesh asset**, not the "scaled engine cylinder" fallback. It is `/Engine/BasicShapes/Cylinder` duplicated, with its mesh-description vertices scaled by 0.6 x 0.6 x 0.004 and rebuilt. Measured actor bounds: extent 30 x 30 x 0.2 cm, so 60 cm across and 4 mm thick, pivot at the centre, scale 1. Slot 0 is `/JamAssets/Look/MI_Trim_PuckHalo` (TeamColour 0.75, 1.0, 0.15; Emissive 18). Simple collision removed. |
| `NS_puck_trail` | **PASS** | Ribbon 10 cm wide, 60 particles/s, life 0.35 s. Alpha = `saturate((User.Speed - 4) / 4)`: off under 4 m/s, full from 8 m/s. It fades along its length. In the stills it is a glowing yellow-green band behind a puck circling at 12 m/s. |
| `MI_Toon_Puck` rim variant | not built (optional) | The brief says "build last or not at all". |

**Device A must set these on its components** (they are component flags, not asset flags):
- Disc: `CastShadow = false`, no custom depth (stencil 0), centre about 0.5 cm above the ice under the puck.
- Disc: also `bAffectDynamicIndirectLighting = false`. With Lumen on, a moving emissive disc left fading ghost copies of itself on the ice. Setting this flag on the PIE disc stopped new ghosts.
- Trail: attach to the puck and set `User.Speed` in **m/s** every tick. The trail loops forever; it never finishes by itself.

Stills (`devlog/shots/`). Puck circling at 12 m/s game time, camera 4 m up, FOV 70, `LR_L2_Night` streamed in:

| Still | Where | Reading |
|---|---|---|
| `20261112-104734-B-b3-puck-dark-1000.jpg` | dark ice between pools, 10 m | disc and trail obvious |
| `20261112-104734-B-b3-puck-dark-2400.jpg` | dark ice, 24 m | small yellow-green dot plus a thin line, still findable |
| `20261112-104734-B-b3-puck-dark-4000.jpg` | dark ice, 40 m | a few px, barely findable |
| `20261112-104734-B-b3-puck-lit-1000.jpg` | lit pool at x 15 m, 10 m | the accent colour reads on white ice; the "white disc vanishes" failure is avoided |
| `20261112-104735-B-b3-puck-lit-2400.jpg` | lit pool, 24 m | still reads |
| `20261112-104735-B-b3-puck-lit-4000.jpg` | lit pool, 40 m | marginal |

## Part 2: one effect and one sound per event

How each asset was checked:
- **Niagara systems:** spawned in PIE from Python at a position, with `Strength` and `Colour` set, then captured with `CaptureGameViewport`.
- **MetaSounds:** played twice in PIE from Python, at Strength 0.2 and then 1.0. The output submix was recorded to a wav (`unreal.AudioMixerLibrary.start_recording_output` / `stop_recording_output`). The wav was measured: peak in dBFS, and length until the level falls below -45 dBFS. Peaks below are "at 0.2 / at 1.0".

### Events, M1

| Event | `NS_` | Evidence | `MS_` | Evidence |
|---|---|---|---|---|
| faceoff | none (by design) | - | **PASS** `MS_faceoff` | Clapper: two band-pass noise clicks (1.8 and 2.6 kHz, 15 ms decay) 40 ms apart, plus a 110 Hz body thump. Peak -14.7 / -11.3 dBFS, 95 ms. Gain is 0.6 + 0.4 x Strength, so it stays audible at the contract's Strength 0. |
| shot | **PASS** `NS_shot` | 12 + 18 x S sprites (12 to 30), life 0.15-0.25 s, ice white tinted 25 % by Colour, upward fan with gravity. Stills at S 1.0 and 0.2. | **PASS** `MS_shot` | Noise high-passed at 2 kHz (30 ms) plus a sine thump at 150 + 60 x S Hz. Gain 0.3 + 0.7 x S. Peak -14.8 / -7.7 dBFS, 65 ms. |
| board | **PASS** `NS_board` | Two emitters: a dust puff of 8 + 16 x S sprites (0.3-0.5 s), and a single 1.2-1.6 m flash sprite in Colour lasting 0.15 s. | **PASS** `MS_board` | 90 Hz sine (120 ms) plus two hits of 650 Hz band noise (the rattle). Peak -14.4 / -6.7 dBFS, 125 ms. |
| goal | **PASS** `NS_goal` | 60 + 100 x S sparks (160 at S 1), 10-20 cm, life 0.6-1.2 s, in Colour, upward fan with gravity and drag. Stills at S 1.0 (orange) and 0.2 (blue). The trim strips do not pulse (lead must decide, point 1). | **PASS** `MS_goal` | Five detuned, low-passed saws on 220 / 277 / 330 Hz with a 0.25 s attack, about 1.5 s. From 1.3 s, a noise tail amplitude-modulated by a 24 Hz LFO (the film-reel flutter). Peak -9.1 / -5.7 dBFS, about 2.07 s. |
| directors_call (R-0002) | **PASS** `NS_directors_call` | 160 warm-white (1, 0.85, 0.6) sprites, 55-70 cm, on a 1 m ring. They sweep outward horizontally at 15 m/s for 0.8 s, so the ring is about 25 m across at the end. No gravity, no drag. | **PASS** `MS_directors_call` | The clapper, then C5 at 0.16 s and G5 at 0.30 s (sine plus a soft square). Peak -11.5 dBFS at both strengths, 655 ms. Gain is constant on purpose: here Strength is a tunable's value, not a loudness. |
| win | **PASS** `NS_win` | 60 + 120 x S confetti (180 at S 1), alternating trim blue (0.05, 0.3, 1.0) and orange (1.0, 0.35, 0.05). Spawned 9 m up across a 9 m sphere, falling 2.4-3 s. GPU sim. Measured inactive between 2.6 and 3.4 s of game time. | **PASS** `MS_win` | C5-E5-G5-C6 arpeggio on a low-passed square in 0.12 s steps, then the clapper at 0.85 s ("that's a wrap"). Peak -10.0 / -7.6 dBFS, 960 ms. |
| lose | none (desaturation is a look-tier parameter) | - | **PASS** `MS_lose` | E4 saw for 0.3 s, then a C4 saw whose pitch falls to about 40 Hz over 0.9 s (tape stop). Peak -10.7 / -8.3 dBFS, 1.26 s. |

### Events, M2 and M3 (done because M1 was finished and verified first)

| Event | `NS_` | Evidence | `MS_` | Evidence |
|---|---|---|---|---|
| check | **PASS** `NS_check` | 8 sprites, 15-45 cm by Strength, radial at 3-10 m/s, 0.18-0.25 s, white mixed 50 % with Colour. | **PASS** `MS_check` | 70 Hz sine (80 ms) plus low-passed cloth noise. Peak -12.9 / -5.7 dBFS, 85 ms. |
| knockdown | **PASS, radial only** `NS_knockdown` | 20 + 30 x S ice chips from a 40 cm ring, low spray with gravity, 0.3-0.6 s. Not "along the fall direction": the payload carries no direction (lead must decide, point 2). | **PASS** `MS_knockdown` | Band-pass noise whose centre falls from 3 kHz to 800 Hz over 0.5 s. Peak -19.7 / -13.9 dBFS, about 0.5 s. |
| pickup_spawn | **PASS** `NS_pickup_spawn` | 60 sparks rising at 3-7 m/s, 0.4-0.6 s, in Colour, no gravity. | **PASS** `MS_pickup_spawn` | Two sine partials, 880 Hz and 2217 Hz, decaying in 0.7 / 0.4 s. Peak -8.8 dBFS, 695 ms. |
| pickup_take | **PASS** `NS_pickup_take` | 64 sprites on a 30 cm ring expanding at 4.5 m/s, 0.45 s, 0.6 m above the ice, in Colour. | **PASS** `MS_pickup_take` | Sine plus a soft square gliding from 400 to 1600 Hz over 0.25 s. Peak -12.2 dBFS, 300 ms. |

### Continuous sounds and mixing (built, not wired: A needs to hand off the hooks)

| Asset | Verdict | Evidence |
|---|---|---|
| `MS_skate_loop` | **PASS** | Looping source, not a one-shot. Inputs `Speed` (float, 0..1, normalised skater speed) and `Turn` (trigger). Band noise centred at 1200 + 2500 x Speed Hz with gain 3 x Speed; `Turn` plays a 150 ms high-passed scrape. Recorded: -20.6 dBFS peak at Speed 1 with a turn; at Speed 0.2 it stays under -45 dBFS (quiet by design, easy to raise). |
| `MS_arena_room` | **PASS** | Looping. 50 Hz and 100 Hz hum plus a band-passed 120 Hz lamp buzz every 7 s. Peak -25.7 dBFS, continuous over 8.5 s. |
| `SC_SFX`, `SC_Ambience`, `SC_Music`, `SCM_Volume` | **PASS** | Three sound classes and one mix with an adjuster per class (volume 1, pitch 1). Every event MetaSound and `MS_skate_loop` uses `SC_SFX`; `MS_arena_room` uses `SC_Ambience`. A's menu slider: `SetSoundMixClassOverride(SCM_Volume, SC_x, volume)`, then `PushSoundMixModifier(SCM_Volume)`. |

### Acceptance checks

- **Sprite cap: PASS.** Spawn counts give these live-sprite peaks (no tool reads live counts): goal 160, win 180, directors_call 160, shot 30, board 25, check 8, knockdown 50, pickup_spawn 60, pickup_take 64. All are at or under 180, below the 200 cap.
- **Self-destruction: PASS.** Every event emitter is life cycle "Self", loops "Once" and has inactive response "Complete".
  - Spawned with `auto_destroy=True`, goal, shot, board and directors_call were inactive 4.5 s later.
  - Win turned inactive at about 3.0-3.4 s of game time.
- **Cost of the goal burst: PASS, with one unexplained spike.** Measured with `PerfToolset.GpuProfilerToolset.TriggerGPUProfile {"minInclusiveMs":0}` in PIE on device B, burst at 10 m from the camera.
  - PIE view `RenderTranslucency`: **0.069 and 0.079 ms** in the frames right after a goal spawn, against a baseline of 0.0017-0.0024 ms (three samples).
  - Two other samples read 0.74 ms in the parent node. Each was the first goal after a fresh PIE start, and the actual draw (`ParallelDraw`) in them was only 0.026-0.028 ms.
  - I read the spike as a first-use pipeline or timing artefact. **That is not proven.**
  - `NS_goal` runs on the CPU, so it adds no GPU simulation pass.
  - Another editor may have been rendering on the machine at the time, so the numbers are noisy.
- **"Play from a test Blueprint"** was done from Python instead, as the lead allowed. Nothing was committed for it.
- **Audio slice: 2.15 MB**, budget 80 MB.
  - The MetaSound `.uasset` files are 110-250 kB each, not "a few kB": the editor graph document dominates. There are no waves.
  - The 5-second captures are in my scratch folder (`.../scratchpad/b3/wav/`, not committed).

## Exposed parameters (device A's binder must use exactly these names)

- **Every event `NS_<event>`** (shot, board, goal, directors_call, win, check, knockdown, pickup_spawn, pickup_take):
  - user parameters **`User.Strength`** (float) and **`User.Colour`** (linear colour, default white);
  - from code: `SetVariableFloat("Strength", s)` and `SetVariableLinearColor("Colour", c)` (Python `set_variable_float('Strength', ...)` worked);
  - Colour is plain LDR 0..1; the glow comes from the sprite material (`Glow` = 15);
  - the parameter is exposed even where it is ignored: `NS_directors_call` ignores both (fixed warm white), `NS_win` ignores Colour (fixed trim colours), `NS_pickup_*` ignore Strength.
- **`NS_puck_trail`**: **`User.Speed`** (float, **m/s**) and **`User.Colour`** (default 0.75, 1.0, 0.15). It has no Strength.
- **Every event `MS_<event>`**: float input **`Strength`**. They are one-shot sources and end themselves through `OnFinished`.
- **`MS_skate_loop`**: float input **`Speed`** (0..1) and trigger input **`Turn`**. **`MS_arena_room`** has no inputs.

## What worked for Niagara (MCP only, plus Python to spawn)

All calls go through `node tools/mcp/mcp.mjs call <Tool> @args.json` (with `UE_MCP_URL=http://127.0.0.1:8001/mcp`). `N.` below means `NiagaraToolsets.NiagaraToolset_System.`. A Niagara system was built with 15-30 calls and no editor UI.

### Calls

1. **Create.**
   - `N.CreateNiagaraSystem {"assetName":"NS_x","assetPath":"/JamAssets/VFX","templateSystem":{"refPath":"/Niagara/DefaultAssets/Templates/Systems/MinimalLightweight.MinimalLightweight"}}`
   - then `N.RemoveEmitter {"emitterRef":{"system":{"refPath":"/JamAssets/VFX/NS_x.NS_x"},"emitterName":"Minimal"}}`
   - then `N.AddEmitter {"system":{...},"templateEmitter":{"refPath":"/Niagara/DefaultAssets/Templates/Emitters/OmnidirectionalBurst.OmnidirectionalBurst"},"emitterName":"Burst"}`.
   - Emitter templates used: `OmnidirectionalBurst` (burst, gravity, drag, size and alpha curves; most events), `ConfettiBurst` (GPU; win), `LocationBasedRibbon` (trail).
   - A second emitter in one system is one more `AddEmitter` (NS_board "Flash").
2. **User parameters.** `N.AddUserVariables {"system":{...},"variablesToAdd":[{"name":"Strength","type":{"classStructOrEnum":{"refPath":"/Script/Niagara.NiagaraFloat"}},"defaultValue":{"struct":{"refPath":"/Script/Niagara.NiagaraFloat"},"value":{"value":1}}},{"name":"Colour","type":{"classStructOrEnum":{"refPath":"/Script/CoreUObject.LinearColor"}},"defaultValue":{"struct":{"refPath":"/Script/CoreUObject.LinearColor"},"value":{"r":1,"g":1,"b":1,"a":1}}}]}`. The names come out as `User.Strength` / `User.Colour`.
3. **Inputs.** `N.SetStackInputData {"stackInputRef":{"system":{...},"emitterName":"Burst","scriptName":"ParticleSpawnScript","moduleName":"InitializeParticle","inputNameStack":["Lifetime Min"]},"inputData":{...}}`. Script names are `EmitterUpdateScript`, `ParticleSpawnScript` and `ParticleUpdateScript`. The `inputData` forms that worked:
   - float: `{"struct":{"refPath":"/Script/Niagara.NiagaraFloat"},"value":{"value":0.6}}` (int: `NiagaraInt32`; bool: `NiagaraBool` with -1 = true);
   - colour / vector: `{"struct":{"refPath":"/Script/CoreUObject.LinearColor"},"value":{"r":..,"g":..,"b":..,"a":..}}`, `Vector3f` `{x,y,z}`;
   - **HLSL expression bound to user parameters** (the key trick):
     - `{"struct":{"refPath":"/Script/NiagaraEditor.NiagaraExt_StackInputData_HlslExpression"},"value":{"hlslExpression":"(int)(60.0 + 100.0 * User.Strength)"}}`;
     - works for Spawn Count, Color (`float4(User.Colour.r, User.Colour.g, User.Colour.b, 1.0)`) and speeds;
     - also works with `Particles.NormalizedAge`, and with `ExecIndex()` for alternating colours (GPU too).
   - enum: `{"struct":{"refPath":"/Script/NiagaraEditor.NiagaraExt_StackInputData_Enum"},"value":{"enum":{"refPath":"<path>"},"enumName":"ENiagara_LocationShapes::NewEnumerator4"}}`.
     - Take the exact enum path and the `NewEnumeratorN` to display-name table from `N.GetStackInputSchema` (field `enumEntries`).
     - Guessed paths fail: the shapes enum is `/Niagara/Enums/Location/ENiagara_LocationShapes.ENiagara_LocationShapes`.
   - inside a dynamic input: `"inputNameStack":["Velocity Speed","Minimum"]` set the min of the template's `RandomRangeFloat`.
4. **Modules.**
   - Add: `N.AddModule {"moduleLocationRef":{"system":{...},"emitterName":"Trail","scriptName":"EmitterUpdateScript"},"moduleAsset":{"refPath":"/Niagara/Modules/Emitter/SpawnRate.SpawnRate"}}` (used for SpawnRate and `/Niagara/Modules/Update/Color/ScaleColor`).
   - Switch off: `N.SetModuleEnabled {"moduleRef":{"system":{...},"emitterName":"Ring","scriptName":"ParticleUpdateScript","moduleName":"GravityForce"},"bEnabled":false}`.
5. **Renderer.** `N.SetRendererData {"rendererRef":{"system":{...},"emitterName":"Burst","rendererIndex":0},"rendererData":{"propertyValues":"{\"Material\":{\"refPath\":\"/JamAssets/VFX/M_Fx_Sprite.M_Fx_Sprite\"},\"bCastShadows\":false}"}}`. `propertyValues` is a JSON string.
6. **Check and save.**
   - `N.GetSystemCompileState {"system":{...}}` returns `aggregateStatus UpToDate`, `bHasErrors false`;
   - then `EditorToolset.AssetToolset.SaveAsset {"assetPath":"/JamAssets/VFX/NS_x"}`.
7. **Read back.**
   - `N.GetEmitterInputValues {"emitterRef":{"system":{...},"emitterName":"Burst"}}` gives every module input with its value;
   - `N.GetStackInputSchema` gives the type and enum entries of one input.
8. **Spawn and test (Python in PIE).**

   ```python
   W = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
   c = unreal.NiagaraFunctionLibrary.spawn_system_at_location(W, unreal.load_asset('/JamAssets/VFX/NS_goal'), loc, unreal.Rotator(0,0,0), unreal.Vector(1,1,1), True, False)
   c.set_variable_float('Strength', 1.0)
   c.set_variable_linear_color('Colour', unreal.LinearColor(1,0.35,0.05,1))
   c.activate(True)
   ```

   - Short effects (0.25 s) were captured with `unreal.GameplayStatics.set_global_time_dilation(W, 0.1)` before the spawn, because a call round trip takes about 0.5 s.
   - Then `SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport {"world":{"refPath":"/Temp/UEDPIE_0_Untitled_1.Untitled"},"maxResolution":1920}` with `--save-images`.
9. **Materials** (Python `MaterialEditingLibrary`):
   - `M_Fx_Sprite`: Unlit, Additive, two-sided, used with Niagara sprites, ribbons and meshes. Emissive = ParticleColor.rgb x ParticleColor.a x `Glow` (15) x `RadialGradientExponential`.
   - `M_Fx_Ribbon`: the same, with the mask `1 - |2v - 1|` across the width.

### Pitfalls

- **Saving while PIE runs fails** with the misleading `SaveAsset` error `Asset does not exist: /JamAssets/VFX/NS_win`, and Python `save_loaded_asset` returns False. Call `StopPIE` first.
- **Editing a system during PIE leaves the PIE component inactive** after the recompile. Call `activate(True)` again.
- **Moving actors from a Python tick** works only with `register_slate_pre_tick_callback` and the mobility set to Movable. Use `GameplayStatics.get_world_delta_seconds` as the step: real time and game time differ a lot on device B.
- **Velocity direction** with the `AddVelocity` "from point" mode follows `Origin Offset`. A positive z offset shoots particles down into the ice; use a negative z.

## What worked for MetaSounds (MCP only; this was the unproven part)

**Graphs can be built fully through MCP.** The first one (`MS_faceoff`) was built, playing and measured about 10 minutes after starting. The toolsets are `MetaSoundToolset.MetaSoundAuthoringToolset` (`A.`), `.MetaSoundDiscoveryToolset` (`D.`) and `.MetaSoundInspectionToolset` (`I.`). All 13 sources were built by one Node script (`msbuild.js`) that turns a node/edge list into these calls.

### Calls

1. **Find classes.**
   - `D.FindNodeClasses {"pattern":"AD envelope","inputs":[],"outputs":[],"maxResults":4}`;
   - then `D.DescribeNodeClass {"namespace":"AD Envelope","name":"AD Envelope","variant":"Audio"}` for the pins. Its arguments are flat `namespace` / `name` / `variant`, not a `className` object.
2. **Class triples that worked** (namespace, name, variant):

   | Class triple | Pins |
   |---|---|
   | `UE, Noise, Audio` | |
   | `AD Envelope, AD Envelope, Audio` and `..., Float` | in: Trigger, Attack Time, Decay Time, Decay Curve; out: Out Envelope |
   | `UE, Biquad Filter, Audio` | in: In, Cutoff Frequency, Bandwidth, Type (`"0"` lowpass, `"1"` highpass, `"2"` bandpass); out: Out |
   | `UE, Sine/Saw/Square, Audio` | in: Frequency; out: Audio |
   | `UE, LFO, Audio` | in: Frequency, Min Value, Max Value; out: Out (Float) |
   | `UE, Multiply, Audio`, `UE, Multiply, Audio by Float`, `UE, Multiply, Float`, `UE, Add, Float` | in: PrimaryOperand, AdditionalOperands; out: Out |
   | `UE, Trigger Delay, ""` | in: In, Delay Time; out: Out |
   | `UE, TriggerRepeat, ""` | in: Start, Period; out: RepeatOut |
   | `AudioMixer, Audio Mixer (Mono, N), ""` | in: In i, Gain i; out: Out |

   The mixer is the trap: `FindNodeClasses` lists `(Mono, 3)` as a suffix, but the registered name is `"Audio Mixer (Mono, 3)"` with an empty variant.
3. **Build a new source.**
   - `A.CreateSourceBuilder {"builderName":"B_x","outputFormat":"Mono","bIsOneShot":true}` gives the interface nodes `UE.Source.OnPlay`, `UE.Source.OneShot.OnFinished` and `UE.OutputFormat.Mono.Audio:0`.
   - `A.AddGraphInputs {"builderName":"B_x","inputs":[{"name":"Strength","dataType":"Float","default":"0.5"}]}` (the node ref is the input's name; `"Trigger"` works as a dataType).
   - `A.AddNodes {"builderName":"B_x","nodes":[{"className":{"namespace":"UE","name":"Noise","variant":"Audio"}}, ...]}` returns the refs in order.
   - `A.SetDefaults {"builderName":"B_x","defaults":[{"node":"<ref>","pin":"Decay Time","value":"0.015"}]}`.
   - `A.Connect {"builderName":"B_x","connections":[{"fromNode":"UE.Source.OnPlay","fromPin":"None","toNode":"<ref>","toPin":"Trigger"}, ...]}`.
   - `A.LayoutGraph {"builderName":"B_x"}`.
   - `A.BuildToAsset {"builderName":"B_x","assetName":"MS_x","packagePath":"/JamAssets/Audio","author":"..."}` writes the asset to disk. A one-shot must wire `OnFinished`; I used a `Trigger Delay` from `OnPlay`.
4. **Change an existing source.**
   - `A.AttachBuilderToAsset {"assetPath":"/JamAssets/Audio/MS_x","builderName":"B_y","page":"Default"}`;
   - `I.FindAllNodes {"builderName":"B_y"}`, then `A.RemoveNodes` on every node whose class namespace is not `Input` / `Output` (removing an input node is refused: `engine refused the removal`);
   - add and connect as above;
   - `A.CommitBuilderToAsset {"builderName":"B_y"}` saves it.
   - The asset's `sound_class_object` survived the commit.
5. **Play and measure (Python in PIE).**

   ```python
   AML = unreal.AudioMixerLibrary
   AML.start_recording_output(W, 6.5)
   c = unreal.GameplayStatics.spawn_sound2d(W, snd)
   c.set_float_parameter('Strength', 0.2)
   await asyncio.sleep(2.9)
   # ... play again at 1.0 ...
   AML.stop_recording_output(W, unreal.AudioRecordingExportType.WAV_FILE, 'MS_x', 'b3')
   ```

   The wav lands in `Game/Saved/BouncedWavFiles/b3/`.
   - Loops: `set_float_parameter('Speed', x)`, `set_trigger_parameter('Turn')`.
6. **Sound classes and mix (Python).**
   - `AssetTools.create_asset(name, '/JamAssets/Audio', unreal.SoundClass, unreal.SoundClassFactory())`;
   - `unreal.SoundMix` with `unreal.SoundMixFactory()`, then `sound_class_effects` = a list of `unreal.SoundClassAdjuster`;
   - on each MetaSound, `set_editor_property('sound_class_object', sc)`.

### Pitfalls

- **The first recording was pure silence (-180 dBFS)**: the PIE window was unfocused and the editor mutes background audio. Fix: `unreal.find_object(None,'/Script/UnrealEd.Default__LevelEditorMiscSettings').set_editor_property('bAllowBackgroundAudio', True)`. It applies at once and is per-user config, nothing committed.
- **`stop_recording_output` refuses absolute paths**: "the path must be relative to the bounce folder". Pass a relative folder name.
- **`EditorAssetLibrary.delete_asset` on a built MetaSound reported success but left the file**, and the object stayed in memory. Deleting the file on disk did not help either: `BuildToAsset` to the same name then fails with `Failed to build asset`. Use the attach / remove / commit path (step 4) to change an existing source.
- **`unreal.AudioMixerBlueprintLibrary` does not exist** in Python; the class is `unreal.AudioMixerLibrary`.

## Files created (bytes on disk; each a single `.uasset`)

| Folder | Files |
|---|---|
| Props | `SM_PuckDisc` 32,710 |
| VFX | `M_Fx_Sprite` 13,711, `M_Fx_Ribbon` 13,748, `NS_puck_trail` 311,628, `NS_shot` 331,864, `NS_board` 564,054, `NS_goal` 331,173, `NS_directors_call` 318,476, `NS_win` 542,894, `NS_check` 332,103, `NS_knockdown` 328,807, `NS_pickup_spawn` 323,118, `NS_pickup_take` 322,740 |
| Audio | `MS_faceoff` 154,032, `MS_shot` 146,628, `MS_board` 154,023, `MS_goal` 228,158, `MS_directors_call` 239,073, `MS_win` 248,461, `MS_lose` 167,504, `MS_check` 142,017, `MS_knockdown` 135,832, `MS_pickup_spawn` 126,005, `MS_pickup_take` 173,745, `MS_skate_loop` 110,569, `MS_arena_room` 119,715, `SC_SFX` 1,039, `SC_Ambience` 1,064, `SC_Music` 1,049, `SCM_Volume` 2,420 |

- Total for the 30 files: 5,918,360 bytes.
- Nothing under `_Scratch/`. The throwaway layout lived in an unsaved `/Temp/Untitled_1` map and was discarded at exit.
- `Look/` and `Rigs/` are untouched: `git status` is clean after the exit. The dirty in-memory `LR_L2_Night` was discarded, not saved.

## Ledger rows (ready to paste into `ASSETS.md`)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Props/SM_PuckDisc.uasset | mesh | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | puck halo disc, 60 cm x 4 mm (engine cylinder vertices rescaled), material MI_Trim_PuckHalo, no collision (32710 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Sprite.uasset | material | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | unlit additive soft-disc sprite, ParticleColor x Glow 15 (13711 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Ribbon.uasset | material | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | unlit additive ribbon with soft edges, ParticleColor x Glow 15 (13748 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | puck ribbon trail; user params Speed (m/s), Colour (311628 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_shot.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event shot (331864 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event board (dust puff + flash) (564054 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event goal (331173 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_directors_call.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event directors_call (ring) (318476 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_win.uasset | vfx | 0.5 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event win (confetti) (542894 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_check.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event check (M2) (332103 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_knockdown.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event knockdown (M2) (328807 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_spawn.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_spawn (M3) (323118 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pickup_take.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pickup_take (M3) (322740 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_arena_room.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (119715 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_board.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (154023 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (142017 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_directors_call.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (239073 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_faceoff.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (154032 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_goal.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (228158 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (135832 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lose.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (167504 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_spawn.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (126005 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_take.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (173745 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (146628 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_skate_loop.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (110569 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_win.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (248461 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SCM_Volume.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound mix (one adjuster per class) for the volume slider (2420 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_Ambience.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1064 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_Music.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1049 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SC_SFX.uasset | other | 0.0 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | sound class (1039 bytes) |

## Stills filed (`devlog/shots/`, JPEG q88, 1920x1085, each with a caption `.txt`)

- **Puck kit:**
  - `20261112-104734-B-b3-puck-dark-1000`, `-dark-2400`, `-dark-4000`, `-lit-1000`;
  - `20261112-104735-B-b3-puck-lit-2400`, `-lit-4000`.
- **M1 effects:**
  - `20261112-110308-B-b3-fx-goal-s10`, `-goal-s02`, `-shot-s10`, `-shot-s02`, `-board-s10`;
  - `20261112-110309-B-b3-fx-directors-call`, `-win`.
- **M2/M3 effects:**
  - `20261112-110908-B-b3-fx-check`;
  - `20261112-110909-B-b3-fx-knockdown`, `-pickup-spawn`, `-pickup-take`.

## Not finished or not verified

- **Nobody has listened to the sounds.** They are verified by recorded level and length, not by ear, and the timbres are first drafts. `MS_skate_loop` at low Speed is below -45 dBFS.
- **The binder path is not run.** No sound or effect has played through device A's `UJamEffectsBinder` or the event bus: the binder does not exist in this tree. Python called the same functions it will call.
- **Sprite counts and GPU cost are partial.** Live sprite counts are by construction, not read back. The GPU cost was measured for the goal burst only: 4 samples with the effect and 4 baselines, noisy, with an unexplained 0.74 ms first-spawn spike.
- **Two brief items are not built:** the optional `MI_Toon_Puck` rim variant, and the music (out of scope per the brief).
- **Trail stills and the shot spray come from one setup each.** The trail stills use a puck moved by a Python tick. The shot spray is radial upward: the payload has no shot direction.

## Lead must decide

1. **Trim strips pulsing on `goal`, and the trim flash on `board`.** Both need a material-parameter animation on A's trim strips (or on `MI_Trim_*`, which `Look/` owns). I only delivered the particle part. Options:
   - A animates `Emissive` on its trim components when the binder fires;
   - B1 adds a time-driven pulse parameter to `M_Trim`.
2. **Direction for `knockdown`, and optionally `shot`.** The payload `{Who, Where, Strength, Kind}` has no direction, so the knockdown scrape is radial. Options:
   - A spawns `NS_knockdown` with a rotation (skater velocity yaw), and B makes the spray directional along local +X;
   - or keep it radial.
3. **Disc lighting flag.** The disc component needs `bAffectDynamicIndirectLighting = false` to avoid Lumen ghost trails. Put it in the handoff with the asset paths.
4. **Is `Colour` on directors_call / win meant to be honoured?** Right now both use fixed colours (warm white; the two trim colours).
5. **Two hooks still owed for the continuous sounds:** `MS_skate_loop` needs `Speed` (0..1) and `Turn`; `MS_arena_room` needs a place to start. The music source is still the operator's decision.
