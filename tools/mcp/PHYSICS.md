# Chaos rigid body determinism through MCP

Measured 2026-11-11 against UE 5.9.0 (launcher install) on the throwaway project of `ANIMATION.md` (deleted
afterwards): a sphere on a flat box floor, low-friction bouncy physical material, one impulse at BeginPlay, position
logged at frames 60, 120, 180 of PIE. Tooling only.

## Verdict

**Deterministic with a fixed frame rate: PASS.** Five PIE runs over two editor sessions (one restart in between)
logged identical positions to the 6 decimals `Conv_DoubleToString` prints (centimetres, so agreement to 0.01 um):

| Frame | x (cm) | y | z (cm) | DeltaSeconds |
|---|---|---|---|---|
| 60 | 272.019533 | 0.0 | 50.000009 | 0.016667 |
| 120 | 496.829685 | 0.0 | 50.000005 | 0.016667 |
| 180 | 706.797013 | 0.0 | 50.000011 | 0.016667 |

**Without it: not deterministic.** The first run, before the fixed frame rate reached the running engine, logged
x = 202.922811 / 424.207707 / 843.756179 with `DeltaSeconds` 0.008333 at frame 60 (120 fps while busy) and 0.333333
at frames 120 and 180: an editor in the background is throttled to about 3 fps, the world steps a third of a second
per frame, and frames 120 to 180 took 20 s of wall time. PIE in a default editor depends on window focus.

Not proven: bit-exact equality (only 6 printed decimals compared), packaged builds, more than one body, collisions
between moving bodies, `bEnableEnhancedDeterminism`, async physics. Treat those as open.

## Settings (and how they were set)

| Setting | Value | How |
|---|---|---|
| Project Settings > Engine > General > Use Fixed Frame Rate (`[/Script/Engine.Engine] bUseFixedFrameRate`) | True | `ConfigSettingsToolset.ConfigSettingsToolset.SetSectionProperties {"containerName":"Project","categoryName":"Engine","sectionName":"General","propertiesJson":"{\"bUseFixedFrameRate\":true}"}` |
| Fixed Frame Rate (`FixedFrameRate`) | 60 | same tool, **second call** `"{\"FixedFrameRate\":60}"`: in one call it fails with `'FixedFrameRate' is disabled by its EditCondition (bUseFixedFrameRate); set the values it depends on in an earlier call` |
| Physics > Substepping (`bSubstepping`), Substepping Async, Tick Physics Async (`bTickPhysicsAsync`) | False (defaults, left alone) | read with `GetSectionPropertyValues {"containerName":"Project","categoryName":"Engine","sectionName":"Physics","propertyNames":[...]}` |
| `MaxPhysicsDeltaTime` | 0.0333 (default; 1/60 is below it, so no clamping) | read only |
| `bEnableEnhancedDeterminism` | False (default, not tried) | read only |
| `t.MaxFPS` | not set: with a fixed frame rate the engine uses `FixedFrameRate` as the max tick rate (`UnrealEngine.cpp`, `UpdateTimeAndHandleMaxTickRate`) | - |

The two calls write `bUseFixedFrameRate=True` and `FixedFrameRate=60.000000` under `[/Script/Engine.Engine]` in
`Config/DefaultEngine.ini`, **but the running editor keeps its old value**: the tool sets the class default object, and
the live engine (`/Engine/Transient.UnrealEdEngine_0`) still read `bUseFixedFrameRate=False`. Either restart the
editor (verified: after a restart the live value is True and the numbers above repeat), or set the live object
through Python:

```python
import unreal
eng = unreal.find_object(None, '/Engine/Transient.UnrealEdEngine_0')
eng.set_editor_property('bUseFixedFrameRate', True)    # C++ property names here; snake case raises
eng.set_editor_property('FixedFrameRate', 60.0)
```

Side effect worth having in the jam: with the fixed frame rate live, the background throttle no longer applies, and
MCP calls from the CLI dropped from about 1.2-1.7 s each to 0.06-0.3 s. (Untested alternative for the latency alone:
Editor Preferences > General > Performance > "Use Less CPU when in Background".)

## Recipe (all MCP; Python where noted)

1. Settings: the two `SetSectionProperties` calls above, then restart the editor (or the Python lines above).
2. Physical material (Python; no MCP tool creates one):

   ```python
   at = unreal.AssetToolsHelpers.get_asset_tools()
   pm = at.create_asset('PM_Slick', '/Game/Smoke', unreal.PhysicalMaterial, unreal.PhysicalMaterialFactoryNew())
   pm.set_editor_property('friction', 0.05); pm.set_editor_property('static_friction', 0.05)
   pm.set_editor_property('restitution', 0.6)
   pm.set_editor_property('friction_combine_mode', unreal.FrictionCombineMode.MIN)
   pm.set_editor_property('restitution_combine_mode', unreal.FrictionCombineMode.MAX)
   pm.set_editor_property('override_friction_combine_mode', True)
   pm.set_editor_property('override_restitution_combine_mode', True)
   unreal.EditorAssetLibrary.save_asset('/Game/Smoke/PM_Slick', only_if_is_dirty=False)
   ```
3. Map and floor (Python): `unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).new_level('/Game/Smoke/L_Phys')`;
   `EditorActorSubsystem.spawn_actor_from_object(unreal.load_asset('/Engine/BasicShapes/Cube'), unreal.Vector(0,0,-10))`,
   scale (100,100,0.2), `static_mesh_component.set_phys_material_override(pm)`; a `DirectionalLight` for screenshots;
   `save_current_level()`. (`LevelToolset.AddToSceneFromAsset` spawns the cube as well; the material override is the
   part that needed Python.)
4. Probe Blueprint (MCP): `BlueprintToolset.Create {"folderPath":"/Game/Smoke","assetName":"BP_PhysProbe","assetType":{"refPath":"/Script/Engine.StaticMeshActor"}}`,
   `AddVariable Frame int`, then `BlueprintDslToolset.Write` with:

   ```lisp
   (event `self.ReceiveBeginPlay`
     (set `self.Frame` 0)
     (`PrimitiveComponent.AddImpulse` (`self.StaticMeshComponent`) (300 0 0) "None" true))
   (event `self.ReceiveTick`
     (params (DeltaSeconds))
     (set `self.Frame` (+ (`self.Frame`) 1))
     (if (or (== (`self.Frame`) 60) (== (`self.Frame`) 120) (== (`self.Frame`) 180))
       (bind loc (`Actor.K2_GetActorLocation` self))
       (bind sf (`KismetStringLibrary.Conv_IntToString` (`self.Frame`)))
       (bind sx (`KismetStringLibrary.Conv_DoubleToString` (.x loc)))
       (bind sy (`KismetStringLibrary.Conv_DoubleToString` (.y loc)))
       (bind sz (`KismetStringLibrary.Conv_DoubleToString` (.z loc)))
       (bind sd (`KismetStringLibrary.Conv_DoubleToString` DeltaSeconds))
       (`KismetSystemLibrary.PrintString` (`KismetStringLibrary.JoinStringArray` (|MakeArray "TJPHYS" sf sx sy sz sd :!type "String") " ") true true)))
   ```
   then `CompileBlueprint`. The impulse is a velocity change (`bVelChange=true`) of 300 cm/s along X.
5. Ball (Python, because it sets component state): spawn the probe class at (0,0,100), then on
   `static_mesh_component`: `set_mobility(MOVABLE)`, `set_static_mesh(/Engine/BasicShapes/Sphere)`,
   `set_phys_material_override(pm)`, `set_simulate_physics(True)`, `set_collision_profile_name('PhysicsActor')`; save
   the level. (`ObjectToolset.SetProperties` on the instance should also do it; not tried.)
6. Run, N times: `EditorToolset.EditorAppToolset.StartPIE {}` (2.2 s incl. 2 s warmup), poll
   `ConsoleToolset.LogsToolset.GetLogEntries {"logCategory":"LogBlueprintUserMessages","pattern":"TJPHYS","maxEntries":50}`
   until the frame-180 line appears (3 s of game time = 3 s of wall time at 60 fps when the editor keeps up),
   `StopPIE {}`. Compare the lines between runs. About 6 calls and 6 s per run.

Item timing: 5 min wall, 49 MCP calls including four PIE runs with polling (minimal recipe about 15 calls plus 6 per
run); Python for the physical material, the floor, the ball's component settings and the live engine setting; no human.

## Notes for gameplay code

- Count frames, not seconds: the Tick counter above is what made runs comparable. With a fixed frame rate the engine
  falls behind wall time when a frame takes longer than 1/60 s (by design, `UnrealEngine.cpp`), so wall-clock waits in
  test scripts must be generous and the log is the clock.
- Any PIE-based test that does not set the fixed frame rate inherits the focus-dependent delta time measured above.
