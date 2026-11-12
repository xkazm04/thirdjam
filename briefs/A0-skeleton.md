# Lane A0: the Unreal project skeleton, seam stubs and contracts (H4, device A)

You are lane A0 on device A of the Third Jam (2026 Epic AI MegaJam, team Hokej). The jam started 2026-11-12 06:00 local; it is now about 10:00, so everything you
make is jam content and belongs in the repo. Repo: `C:/Users/kazda/kiro/thirdjam`. Read first: `docs/PLAN.md` (4, 4b), `docs/HOCKEY.md` (2, 4, 5, 6, 7), `docs/TRACKS.md` (2, 3),
`tools/mcp/README.md`, `tools/mcp/ANIMATION.md` (the `UTJAnimGraphEdLibrary` source and the fixed-frame-rate finding), `tools/mcp/PHYSICS.md`, `tools/skills/look-painted-cel.md`
(the project settings it needs), `tools/run/*.ps1`, `ownership.json`, `contracts/README.md`, `contracts/TEMPLATE.md`.

You own: `Game/**` except `Game/Content/Cartridges/**` (lane A4 writes those), and `contracts/*.md`. Run no git command (the Director commits). Never kill a process you did
not start. One heavy job at a time (build, editor, package). Free-space floor 40 GB. Download nothing; import nothing from `prep-reference/`.

## Laws
Unknown keys are errors; absent value = omit the key; lists not singletons; time is physics frames at 60 Hz; randomness only from a seeded stream; packs/data never bundle secrets.
Your measurement outranks this brief: state the constraint and the evidence, counter-propose rather than guess.

## Deliverables, in order
1. **`Game/ThirdJam.uproject`**: blank C++ project (no template content), `EngineAssociation` "5.9", modules `JamCore` (Runtime: simulation, rules, cartridge types),
   `JamGame` (Runtime: pawns, game mode, HUD, camera; depends on JamCore), `JamDesigner` (Runtime stub: one `UJamDesignerSubsystem` with a `TODO` and no network code yet),
   `JamEditor` (Editor: carries `UTJAnimGraphEdLibrary` from ANIMATION.md verbatim, adapted to the module name). Plugins enabled: `ModelContextProtocol`, `ToolsetRegistry`,
   `AllToolsets` (all three `"TargetAllowList": ["Editor"]` so they never cook), plus a content-only plugin **`Game/Plugins/JamAssets/JamAssets.uplugin`** (`CanContainContent`
   true, no modules, empty `Content/`). Use the 5.9 C++ template's `BuildSettingsVersion.V8` / `EngineIncludeOrderVersion.Unreal5_9` in Target.cs files (R1 found these).
2. **Config**: `DefaultEngine.ini`: DX12 default RHI, SM6 shader format, `r.GenerateMeshDistanceFields=True`, `r.CustomDepth=3`, Lumen GI + reflections, virtual shadow maps, fixed frame rate
   60 (`bUseFixedFrameRate=True`, `FixedFrameRate=60.000000` under `[/Script/Engine.Engine]`), game default map `/Game/Maps/Rink`, editor startup map the same.
   `DefaultGame.ini`: project name `Hokej`, explicit `+MapsToCook=/Game/Maps/Rink` under `[/Script/UnrealEd.ProjectPackagingSettings]`, `bUsePakFile=True`, `bCompressed=True`.
   `DefaultEditorPerProjectUserSettings.ini`: `[/Script/ModelContextProtocol.ModelContextProtocolSettings] bAutoStartServer=True`. `DefaultInput.ini`: Enhanced Input default classes.
   `.mcp.json` in `Game/` from `tools/mcp/mcp.json.template`.
3. **Map** `Game/Content/Maps/Rink.umap`: empty level with a PlayerStart out of view (E4 lesson), a directional light off, a sky light, a 120 x 60 m flat plane as a temporary floor
   (lane A1 replaces it with the cartridge-built rink). Create it through the editor (MCP or Python) and save.
4. **Seam stubs** (TRACKS.md 3), compiling and committed before any other lane starts:
   - `IJamSkaterVisual` (JamCore, `UINTERFACE`): `SetTeam(int32 TeamId, FLinearColor Colour)`, `SetNumber(int32 N)`, `UpdateMotion(float Speed, float TurnRate, FRotator Facing)`,
     `PlayShoot()`, `PlayCheck()`, `SetKnockedDown(bool bDown, float DirX, float DirZ)`, `SetHasPuck(bool b)`, `SetBoost(FName Kind)` (`NAME_None` = none). Socket names as constants:
     `stick_socket`, `puck_socket`, `head_socket`. Material parameter names as constants: `TeamColour`, `NumberTens`, `NumberOnes`, `Emissive`, `Wet`, `Scuff`.
   - `UJamEventBus` (JamCore, `UGameInstanceSubsystem`): `Broadcast(FName Event, const FJamEventPayload&)` with payload `{ Who (int32 skater index or -1), Where (FVector), Strength (float) }`,
     a multicast delegate, and a logger that prints every event; event names as constants: `shot`, `board`, `goal`, `faceoff`, `check`, `knockdown`, `pickup_spawn`, `pickup_take`, `win`, `lose`.
   - Three light-rig placeholders in `Game/Plugins/JamAssets/Content/Rigs/`: `LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon` as **empty sub-level maps** (B fills them; say so in the contract),
     plus `ULookTierSwitcher` (JamGame) that loads one by `game.look.tier` string (`l1_day` | `l2_night` | `l3_neon`) and unloads the others.
5. **Contracts** from `contracts/TEMPLATE.md`: `contracts/skater-visual.md`, `contracts/look-tiers.md`, `contracts/events.md`, `contracts/assets-naming.md` (prefixes `SM_ M_ MI_ T_ SFX_ NS_ MS_ BP_ LR_`,
   folders in JamAssets, 2K texture cap, one ledger row per file with `size_mb`). Every type, field, enum string and asset name; every input names its producer and first consumer; absent-value convention stated once.
6. **Prove it**: `tools/run/build.ps1 -Project Game/ThirdJam.uproject` passes; `tools/run/editor.ps1 -Project ... -WaitMcp` reaches MCP; `tools/mcp/mcp.mjs list` answers; save the map;
   `tools/run/package.ps1 -Project ... -Config Development -Out <scratch>` packages and `run-packaged.ps1 -Seconds 10` passes; record staged size. Stop your editor at the end (another lane will start its own).

## Report
Build time, editor-to-MCP time, package time and size, every error with its message, the exact file list you created, anything in the plan that did not survive contact with the engine. Time box 75 minutes.
