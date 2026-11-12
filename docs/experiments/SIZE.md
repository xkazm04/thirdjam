# Size playbook: what each trick saves, measured before we need it (lane A13, 2026-11-12, from H6.3)

Owner: lane A13 (device A). Contest limit: a zip **under 1 GB** (`docs/PLAN.md` 2, 4b). Every number here comes from a Shipping package
of the real project (`tools/run/package.ps1 -Config Shipping -Zip`) made from a private short-path copy (`%TEMP%\a13\Game`, deleted at the end),
one technique at a time, against **b0**: the copy's package at 12:21, which reproduced lane A11's 12:04 package of record exactly
(**zip 376.2 MB, staged 548.0 MB**). "Zip" is the contest zip as `package.ps1` makes it (.NET Deflate, Optimal). Deltas are zip MB unless marked.

Tools: `tools/size/report.mjs` (breakdown by folder, asset class and source, the 30 largest packages, the delta against the last report) now runs
at the end of every `package.ps1`; history in `tools/size/history/` (repo project) or `<Out>\size-history\` (any other copy).
`tools/size/default-plugins.mjs` lists the engine plugins that reach the game target.

## 0. What the 376 MB was

| Slice | Staged MB | Zipped MB (approx.) | Note |
|---|---|---|---|
| `ThirdJam-Windows.ucas` (cooked content, Oodle Kraken 4) | 218.9 | 214 | does not deflate further |
| of which: **NNEDenoiser neural-network models** | 98.4 | 98 | engine default plugin, nothing in the game uses it (section 1, row 1) |
| of which: shader code (SM6 + SM5, shared library) | 81.3 | 81 | |
| of which: game UI textures (`/Game/UI`, uncompressed UserInterface2D) | 20.6 | 20 | ten 1536 x 1024 BGRA8 cards, 6.0 MB raw each |
| of which: engine content (EngineMaterials, MapTemplates sky cubemap, Interchange LUT, ...) | 16 | 16 | |
| `ThirdJam-Win64-Shipping.exe` | 171.7 | 68.9 | |
| `nvngx_dlss.dll` (DLSS-SR, the only staged DLSS file) | 56.2 | 29.4 | already single-DLL (DLSS.md 2.1) |
| `DirectML.dll` + `onnxruntime.dll` (pulled in by NNEDenoiser -> NNERuntimeORT) | 34.2 | 14.8 | |
| VC++ redistributables x64 + arm64 | 28.9 | 28.5 | already compressed |
| the rest (D3D12 agility, Aftermath, MsQuic, dbghelp, Vorbis, pak) | ~16 | ~10 | |

**The 98 MB of NNE models**: `Engine/Plugins/NNE/NNEDenoiser` is `"EnabledByDefault": true` in UE 5.9, and its own `Config/Game.ini` says
`+DirectoriesToAlwaysCook=(Path="/NNEDenoiser")`, so its OIDN denoiser weights (two "large" fp16 models of 47.6 and 44.1 MB raw plus four small ones)
are cooked into every package whether anything references them or not. It depends on `NNERuntimeORT`, which stages `onnxruntime.dll` (16.5 MB) and
`DirectML.dll` (17.7 MB). It is the path tracer / Movie Render Queue denoiser; the game never uses it. Not DLSS, not Streamline, not ML Deformer.
Allowing it in the editor only (`"TargetAllowList": ["Editor"]`) is **not enough**: the cook then fails ("imports class /Script/NNEDenoiser.NNEDenoiserAsset,
which is in a module that is not available on the platform"). It has to be disabled in the `.uproject`.

## 1. The playbook, in order (measured; one technique at a time against b0)

| # | Technique | Zip delta | Staged delta | Risk | Status |
|---|---|---|---|---|---|
| 1 | **Disable the NNEDenoiser engine plugin** | **-113.3** | -132.8 | none for the game (editor loses the path-tracer denoiser) | **applied 12:44** |
| 2 | **Pak compression Oodle Leviathan level 7** (Kraken 4 is the engine default) | **-16.3** | -17.5 | none (lossless); package +20 s, load-time decode a little slower | **applied 12:44** |
| 2a | ... Leviathan 9 | -17.6 | -18.9 | as 2; package +35 s | not worth it over 7 |
| 2b | ... Kraken 7 | -10.3 | -11.4 | none | superseded by 2 |
| 2c | ... Mermaid 4 | +2.0 | +10.2 | | never |
| 3 | **Disable 38 unused default runtime plugins** (list in 3.1) | **-4.2** | -9.7 (exe -7.1) | low: a lane that later needs one re-enables it | **applied 12:44** |
| 4 | Drop SM5 (D3D11) shaders: SM6 only | -7.0 | -7.6 | **medium**: a judge PC that falls back to D3D11 cannot start | emergency ladder 1 |
| 5 | **Never-cook editor-only engine folders** (VREditor, EditorMeshes, Tutorial, MobileResources, Interchange/Utilities) | **-1.0** | -1.1 | none (cook verifies) | **applied 12:44** |
| 5a | ... plus MapTemplates, EditorResources, EditorMaterials, ArtTools | cook FAILS | | `Rink` and `LR_L2_Night` reference `/Engine/MapTemplates/Sky/DaylightAmbientCubemap` (4.3 MB); Landmass materials reference the editor folders | never |
| 5b | ... a plugin's root (`/SpeedTreeImporter`) | cook FAILS (ensure in AssetReferenceRestrictions: "path registered for two domains") | | | never |
| 6 | `bSkipEditorContent=True` | 0.0 | 0.0 | none | no effect in 5.9 with an explicit cook list; not applied |
| 7 | Zip step: Deflate level 9 (Python zlib) instead of .NET Optimal | -0.6 | | none | not worth changing `package.ps1`; 7-Zip is not installed (`-mpass=15` would add an estimated 1-2 MB on the exe) |
| 7a | Zip "store" (no compression) | +171.8 | | | never |
| 8 | `bShareMaterialShaderCode` + `bSharedMaterialNativeLibraries` (already on) | turning them **off** costs +27.7 | +25.0 | | keep on |
| 9 | Shader debug info / symbols | 0 | | | already off in 5.9 (`r.Shaders.Symbols=0`, `-nodebuginfo`) |
| 10 | Exe flags (`bAllowLTCG`, `bUseLoggingInShipping`, `bOmitFramePointers`) | not buildable | | | the installed (launcher) engine refuses: "ThirdJam modifies bDefineForceInlineHintToInline ... has build products in common with UnrealGame" (needs `BuildEnvironment = Unique`, i.e. a source engine). Logging is already off in Shipping |
| 11 | Prerequisites (`IncludePrerequisites`, `-prereqs`) off | -28.5 (est. from the files) | -28.9 | **high** (rule 7(f)): a PC without the VC++ 2015-2022 runtime cannot start the game | keep; emergency only |
| 11a | Delete only `vc_redist.arm64.exe` from the stage before zipping | -11.0 (est.) | -11.2 | low: only Windows-on-ARM machines run it, and the game needs a DX12 SM6 GPU | emergency ladder 2 |
| 12 | DLSS | | | | already single-DLL (56.2 MB, 29.4 MB deflated); dropping DLSS saves 29.4 MB zipped, R-0006 owns it |

**Combined, applied (package c1 at 12:46, today's tree incl. the morning's content and code): zip 249.8 MB, staged 395.3 MB**, and the build ran 15 s
(`run-packaged.ps1`). Sum of the single deltas is -134.8 MB; the content and code added since 12:04 account for the rest.
**Re-baseline on the shared tree, 13:00 link window: zip 247.5 MB, staged 393 MB** (`Game/Packaged/Hokej_Hokej_H7.zip`, sha256 `f1fcaf72...5e48`, ran 10.4 s); `status/A.md` `## H7 size`.

### 1.1 Exact config lines (applied)

`Game/ThirdJam.uproject` (plugins, engine plugins only, appended as `{ "Name": X, "Enabled": false }`):
NNEDenoiser, Paper2D, ResonanceAudio, MediaPlate, MediaCompositing, WmfMedia, AvfMedia, WebMMoviePlayer, AndroidFileServer, AndroidPermission, GooglePAD,
AppleImageUtils, LocationServicesBPLibrary, MobilePatchingUtils, ChunkDownloader, LauncherChunkInstaller, ArchVisCharacter, MetaHumanSDK, DatasmithContent,
VariantManagerContent, GLTFExporter, InterchangeAxfAssets, ChaosVD, ChaosCloth, GeometryCache, CableComponent, CustomMeshComponent, AnimationSharing,
NFORDenoise, MsQuic, BackChannel, WorldMetrics, RuntimeTelemetry, OnlineSubsystem, OnlineSubsystemNull, OnlineSubsystemUtils, OnlineBase, FacialAnimation, AudioCapture.

Kept on purpose (used, or likely used by device B tomorrow): Niagara, ChaosNiagara, EnhancedInput, TextToSpeech, DLSS, ProceduralMeshComponent (JamGame depends on it),
ACLPlugin, ControlRig, IKRig, RigVM, Metasound, Synthesis, SoundFields, PCG, Cascade, EngineCameras, GameplayCameras, XInputDevice, WindowsMoviePlayer,
Interchange (editor import), Tcp/UdpMessaging (editor tooling), PlatformCrypto. Sources grep: the Jam modules use AI, AnimGraphRuntime, DLSSBlueprint, EnhancedInput, HTTP,
Json, Networking, Niagara, ProceduralMeshComponent, Sockets, UMG; the content references only /Script/Engine, CoreUObject, NavigationSystem, PhysicsCore, JamGame
(and Interchange import data). The cook (an editor process with every editor plugin) loaded with this list without a dependency error.
Also measured without effect: Landmass, Water, MovieRenderPipeline off (0.0).

`Game/Config/DefaultGame.ini`, `[/Script/UnrealEd.ProjectPackagingSettings]`, after `IncludePrerequisites=True`:
```
PackageCompressionMethod=Leviathan
PackageCompressionLevel_TestShipping=7
PackageCompressionLevel_Distribution=7
+DirectoriesToNeverCook=(Path="/Engine/VREditor")
+DirectoriesToNeverCook=(Path="/Engine/EditorMeshes")
+DirectoriesToNeverCook=(Path="/Engine/Tutorial")
+DirectoriesToNeverCook=(Path="/Engine/MobileResources")
+DirectoriesToNeverCook=(Path="/Interchange/Utilities")
```
(`package.ps1` does not pass `-distribution`, so the TestShipping level is the one used; both are set.) `DefaultEngine.ini`: unchanged.

## 2. Asset-side rules (measured on a generated sample set, not on B's assets)

Lab: `/Game/SizeLab` in the private copy, sources generated by script (2048 px "painted-cel" texture with flat bands and brush grain, 2048 px multi-octave "photo" noise,
a normal map from that height field, a 262k-triangle bumpy sphere, 60 s of stereo 48 kHz music-like audio, a 1.5 s hit). Sizes are what the asset occupies in the
container after Oodle (stored), per asset, read with `UnrealPak -ListContainer`. RDO needed `[/Script/Engine.TextureEncodingProjectSettings] bFinalUsesRDO=True`
in the copy; the per-texture amount is `LossyCompressionAmount`.

### 2.1 Textures (2048 x 2048 with mips, MB stored)

| Variant | Painted | Photo | Normal map |
|---|---|---|---|
| BC1 (Default, no alpha) | 1.49 | 2.02 | 1.32 (BC1 normal: visibly worse) |
| BC7 | 4.40 | 5.09 | |
| BC5 (Normalmap) | | | 4.66 |
| BC1 + RDO Low / Medium / High / Highest | 0.83 / **0.62** / 0.43 / 0.39 | 1.87 / **1.57** / 1.36 / 1.20 | |
| BC7 + RDO Low / Medium / High / Highest | 3.03 / **1.65** / 1.23 / 1.02 | 4.81 / **4.46** / 4.12 / 3.54 | BC5 + RDO Medium: **1.28** |
| BC1, max size 1024 | 0.16 | 0.48 | BC5 1024: 1.16 |
| BC1, LODBias 1 | 0.22 | 0.48 | |

Reading: RDO Medium (lambda about 20-30) cuts flat painted textures by 58-62 % and smooth normal maps by 73 %, noisy photo textures only by 12-22 %.
BC7 is 2.2-3x BC1 after Oodle. 1K instead of 2K is -75 to -89 %. `LODBias 1` strips the top mip at cook (same as 1K).

### 2.2 Audio (MB stored)

| Variant | 60 s stereo music | 1.5 s mono SFX | 1.5 s stereo SFX |
|---|---|---|---|
| Bink Audio (the 5.9 default; "platform specific" gives the same) quality 40 / 70 | **0.80** / 1.28 | 0.01 / 0.02 | 0.02 |
| Opus quality 40 / 70 | 0.69 / 1.03 | 0.01 / 0.01 | 0.02 |
| ADPCM | 1.53 | 0.02 | 0.03 |
| PCM | 6.44 | 0.07 | 0.08 |

Reading: compressed audio is small: 3 minutes of music at Bink q40 is about 2.4 MB, at q70 3.8 MB. The 80 MB audio budget is far away unless something ships PCM.

### 2.3 Meshes (262k triangles, MB stored)

| Variant | MB |
|---|---|
| Nanite off, 1 LOD | 5.95 |
| Nanite on | 5.10 |

Nanite data is smaller than the classic vertex buffers for a dense mesh, so Nanite is not a size cost here. LOD and distance-field-scale variants did not take
effect in the commandlet (two attempts: `EditorStaticMeshLibrary` reports LOD count -1 and 0 vertices for an Interchange-imported OBJ under `-nullrhi`, `set_lods` returns -1): not measured. `KeepMobileMinLODSettingOnDesktop`: only matters with MinLOD set; not measured.
Animation: ACL is the 5.9 default codec; no skeletal content to measure (estimate: a 2 s 60-bone clip is under 0.1 MB).

## 3. Emergency ladder (if the zip is still over budget at H72)

Current headroom after the applied rows: about 750 MB under the limit. In this order:

1. Drop SM5/D3D11 shaders (`-D3D11TargetedShaderFormats=PCD3D_SM5` only, remove the `+` line): **-7.0 MB** now, grows with every material. Risk: D3D11 fallback gone.
2. Delete `Engine/Extras/Redist/en-us/vc_redist.arm64.exe` from the stage before zipping: **-11.0 MB** (estimate from the file).
3. Global texture RDO: `[/Script/Engine.TextureEncodingProjectSettings] bFinalUsesRDO=True`, `FinalRDOLambda=30`: on painted textures -58 to -71 % (lab), but on today's content **-0.1 MB** (measured, package c2: the only big textures are uncompressed UI cards, which RDO does not touch). Becomes worth it once device B's BC textures arrive.
4. UI cards from uncompressed UserInterface2D to BC7 (lane A8): 10 x 6.0 MB raw -> 1.5 MB raw; estimated **-10 to -14 MB** of the 19.4 MB stored. Not measured: three lab attempts (duplicates of `T_UI_TitleSlate` set to BC7 / BC1 / BC7+RDO, UI and World LOD groups, saved through EditorAssetLibrary) all cooked at the same 6.0 MB raw, so the commandlet route does not change this asset's cooked format; A8 should flip one card in the editor and read the `textures` line of the next `report.mjs` table.
5. Development-only content out of the cook (test maps, debug widgets, `Designer` cache if it grows): check `tools/size/report.mjs` `sources_mb` for `game:*` folders.
6. Music: Bink quality 40, under 3 minutes unique: about **2.4 MB** total, so only if someone shipped PCM (PCM is 8x).
7. Textures 1K for props (-75 % per texture), then 1K for everything but hero assets.
8. The SkyLight cubemap `/Engine/MapTemplates/Sky/DaylightAmbientCubemap` (4.3 MB stored, 16 MB raw) referenced by `Rink` and `LR_L2_Night`: a captured sky instead (device B rigs) saves about **4 MB**.
9. Prerequisites off: **-28.5 MB**, last, it breaks PCs without the VC++ runtime (rule 7(f)).
10. DLSS off: -29.4 MB zipped, -56 MB staged (R-0006 decides).
11. Content cuts (`docs/PLAN.md` 9).
