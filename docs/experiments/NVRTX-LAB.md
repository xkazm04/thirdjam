# NvRTX lab (lane A24, 2026-11-12, H9.9 to H10.4): can the Hokej project open in the NvRTX 5.8 build, and what would the P4 broadcast features cost?

Result in one line: **the project does not open in NvRTX 5.8.3. The C++ compiles unchanged, the editor starts, and then every one of the 183 content packages is refused:
UE 5.9 saved them with custom versions the 5.8 engine does not know (`Custom version is too new`).** Nothing was measured; the cost table is empty by construction.
Per the brief this is a valid result, reached inside the 45-minute cap (16:00 to 16:09). Shipped game untouched: launcher UE 5.9.0 CL 60351404.

## 1. What was tried, in order (all on a copy; nothing built inside `nvrtx-5.8`, nothing in `Game/Plugins/JamAssets` touched, no git command)

| Step | Time (`date`) | Result |
|---|---|---|
| Engine identity | 15:58 | `C:\Users\kazda\kiro\nvrtx-5.8\Engine\Build\Build.version`: **5.8.3**, CL 0, not a promoted build, branch `UE5`. `UnrealEditor.exe`, `UnrealEditor-Cmd.exe`, UBT present. NVIDIA plugins built in-engine (`Engine/Plugins/Runtime/Nvidia`: DLSS 8.9.0 with RR, Streamline FG/Reflex/DeepDVC, NRD, NIS). |
| Package-format pre-check | 16:00 | The 5.9 files carry UE5 object version **1018**, UE4 **522**; the 5.8.3 `ObjectVersion.h` AUTOMATIC_VERSION is **1018** as well. So the global file version is not the blocker (it would have been for a 5.7 branch). |
| Plugins the project enables | 16:00 | ModelContextProtocol, ToolsetRegistry, AllToolsets, TextToSpeech, NNEDenoiser, JamAssets (content-only), DLSS: **all exist in 5.8**, nothing had to be disabled. The gitignored `Game/Plugins/DLSS` is absent from the worktree, which is right: 5.8 has the engine copy. |
| Copy | 16:00 | `robocopy` of the worktree's `Game/` to `C:\Users\kazda\kiro\a24-lab\Game` without Binaries, Intermediate, Saved, DDC, logs (50 MB). |
| Edits in the copy only | 16:01 | `ThirdJam.uproject`: `"EngineAssociation": "C:/Users/kazda/kiro/nvrtx-5.8"` (a path is a legal identifier, `FDesktopPlatformBase::GetEngineIdentifierForProject`). Both `*.Target.cs`: `BuildSettingsVersion.Latest`, `EngineIncludeOrderVersion.Latest` (5.8 UBT stops at `Unreal5_8`). `DefaultEditorPerProjectUserSettings.ini`: MCP port 8012. `DefaultEngine.ini`: `r.RayTracing=True`, `r.SkinCache.CompileShaders=True`, `r.Lumen.HardwareRayTracing=1`, `r.RayTracing.Shadows=0`. |
| Build (`tools/run/build.ps1`, `UE_ROOT` = the NvRTX tree, `-WaitMutex`) | 16:02 to 16:03 | **PASS: ThirdJamEditor Development in 86.4 s, 0 errors, 0 warnings.** JamCore, JamGame, JamDesigner, JamNet, JamEditor compile against 5.8 without a source change. `JAM_WITH_DLSS=0` (the Build.cs looks for a project-local plugin). Log: `a24-lab/Game/logs/build-ThirdJamEditor-Development-20261112-160159.log`. |
| First editor start (`editor.ps1 -McpPort 8012 -WaitMcp`) | 16:04 | Editor up, `LogDLSSBlueprint: Loaded DLSS-SR plugin version 8.9.0-NGX310.9.16`, `Ray tracing is enabled (dynamic)`, bindless D3D12 for ray tracing. **Exit code 1 after 20 s: `Couldn't launch .../ShaderCompileWorker.exe! Make sure you build ShaderCompileWorker.`** Last night's engine build compiled the `UnrealEditor` target only (`Engine/Intermediate/Build/Win64` holds `UnrealEditor` and nothing else); no program targets, so no ShaderCompileWorker, no UnrealPak. The brief forbids building inside the tree, so it was not built. |
| Second start, in-process shaders | 16:06 | Copy's `DefaultEngine.ini` + `[DevOptions.Shaders] bAllowCompilingThroughWorkers=False` (builds nothing). The editor stays up and compiles the four engine default materials on one thread; the asset registry scan then logs the result below. |

## 2. Why it does not open: custom versions, not the file version

The asset registry scan of the copy refused **183 of 183** packages (40 in `Game/Content`, 143 in `Game/Plugins/JamAssets/Content`), including `Maps/Rink.umap`, `Maps/Menu.umap`,
all four rig maps, every `MI_Toon_*`, `MI_PP_ToonRamp`, `MI_PP_Outline`, the Niagara systems and the figures. The reason, from `LogAssetRegistry: Error: Package is unloadable ... Custom version is too new`:

| Custom version stream (name in the log) | In the 5.9 files | Newest the 5.8.3 engine knows (HeadCode) | Packages hit |
|---|---|---|---|
| FortniteMain (`FFortniteMainBranchObjectVersion`) | 273 | 268 | 183 |
| UE5-Main (`FUE5MainStreamObjectVersion`) | 127 | 123 | 166 |
| InterchangeAssetImportDataVer | 6 | 5 | 81 |
| FortniteRelease (`FFortniteReleaseBranchCustomObjectVersion`) | 21 | 20 | 32 |
| NiagaraVer | 94 | 93 | 10 |

Unreal never loads a package whose custom version is newer than the engine's, and the 5.9 editor has no "save as an older version".
So no copy, flag or re-save makes the 5.9 project readable by a 5.8 engine. The only ways to put this game into NvRTX are:

- **An NvRTX 5.9 branch.** None exists (`docs/NVRTX-NOTES.md`; `docs/experiments/DLSS.md` section 1.1: NVIDIA's newest is 5.8). Out of our hands.
- **Rebuild the scene inside 5.8 from sources, not packages.** Re-import the FBX figures and props (`prep-reference/`, Meshy and Rodin exports), rebuild materials, post and rigs with the agent recipe
  `tools/skills/look-painted-cel.md` (it made the whole painted-cel look from Python in one hour on a throwaway project), and script the bots. That is a second content pipeline, 3 to 6 hours of agent time, for footage only
  (packages made in 5.8 open in 5.9, but anything B changes afterwards must be redone by hand). Ruling R-0006 point 2 already calls NvRTX "the optional trailer experiment"; this is what that experiment would cost.
- **Cherry-pick NvRTX commits onto a 5.9 source build.** The NvRTX README lists its commits per feature for picking onto "every UE 5.8 hotfix or customized engine"; 5.9 is not a 5.8 hotfix, and a 5.9 source build is another 150 to 280 GB and a night. Not a jam-week item.

Also found, independent of the content: the engine build is incomplete for any project work (no `ShaderCompileWorker.exe`). Whoever uses `nvrtx-5.8` next needs the operator's leave to build the
`ShaderCompileWorker Win64 Development` program target (1 to 3 minutes; writes `Engine/Binaries/Win64/ShaderCompileWorker.*` and `Engine/Intermediate/Build/Win64/ShaderCompileWorker`), or every start compiles shaders on one thread.

## 3. Cost table (ms per feature at 1440p on the RTX 4090)

Not measured: no map loaded. Nothing in this file is a number from NvRTX. For the shipped 5.9 engine, the measured figures are in `docs/experiments/LOOK-P2-NIGHT.md`
(painted-cel look 3.8 to 4.3 ms at 1080p; the two post passes 0.04 ms; Lumen 2.5 to 3 ms; four shadowed spots 0.6 ms) and `docs/experiments/DLSS.md` section 2.4 (DLSS Quality at 1440p 7.3 to 8.6 ms p50 against TSR 6.8 to 14.0; DLAA 6.0).

| Variant | Direct light | GI | Reflections | Denoiser | GPU ms at 1440p | Stills 10 / 24 / 40 m |
|---|---|---|---|---|---|---|
| baseline (as shipped) | VSM spots | Lumen | Lumen | TSR | not measured | none |
| RTXDI on the spots | `r.SampledLighting.Direct 1` | Lumen | Lumen | NRD or RR | not measured | none |
| ReSTIR GI | RTXDI | `r.SampledLighting.Indirect 1`, `.Mode 1` | Lumen | RR | not measured | none |
| ray-traced ice reflections | RTXDI | ReSTIR GI | `r.SampledLighting.Lumen.Reflections 1` | RR | not measured | none |
| Ray Reconstruction off | same | same | same | NRD (`r.SampledLighting.Denoiser 2`) | not measured | none |

The CVar names are from the NvRTX 5.8 docs (`Docs/RTXDirectIllumination/README.md`, `Docs/RTXIndirectIllumination/Readme.md`, `Engine/Plugins/Runtime/Nvidia/DLSS/README.md`:
`r.NGX.DLSS.Enable 1` + `r.NGX.DLSS.DenoiserMode 1` for RR) and are the plan for a rerun, if the scene is ever rebuilt in 5.8. Note for that rerun: the night rig has **four** spots
(`Spot_L2_Night_1..4`), not twelve; RTXDI's selling point (cost flat in light count) needs the twelve-spot variant spawned for the test, and RTXDI with a single-digit light count can cost more than VSM (NVIDIA's own guide says so).

Painted-cel post against the denoiser (job 2): untested. The expectation, for the record: the toon ramp is a post-process material on the lit colour and the outline reads depth and normal,
both run after RR, so the ramp survives but its bands follow whatever RR converges to; a denoised, temporally stable light pool should band cleanly, a noisy one flickers at the band edges. Only a still can confirm this.

Clip (job 3): none. No bot match ran in 5.8.

## 4. Recommendations

1. **Ship nothing from NvRTX** (recommended). The shipped game cannot be opened by the NvRTX engine, and nothing made there can be judged on the jam build. The NVIDIA track is already
   covered by DLSS Super Resolution + DLAA in the 5.9 build (`docs/experiments/DLSS.md`, ruling R-0006 point 1): that is the "tools used" entry. Cost of this option: zero.
2. **Trailer only**: only if the operator wants path-traced hero shots badly enough to pay for a second content pipeline in 5.8 (section 2, 3 to 6 hours of agent time plus the ShaderCompileWorker build),
   and accepts that the footage shows a rebuilt scene, not the game. The dev log may then say "trailer stills rendered with the NVIDIA RTX branch of Unreal Engine 5.8 (RTXDI, ReSTIR GI, DLSS Ray Reconstruction)".
   No NVIDIA logo (rule 8(d), R-0006 point 5). Not before the game is finished (R-0021 point 1 order of work).
3. **Optional broadcast tier**: not possible. A tier is a rig map plus CVars inside the shipped 5.9 build, and RTXDI, ReSTIR and Ray Reconstruction do not exist in the launcher engine.
   The P4 "stylized broadcast" bar (`docs/HOCKEY.md`) is reachable in 5.9 with what exists: Lumen hardware ray tracing and ray-traced reflections on the ice (`r.Lumen.HardwareRayTracing 1`, `r.Lumen.Reflections.HardwareRayTracing 1`),
   the existing post stack, a broadcast camera rig and DLAA. That is a 5.9 experiment for a later lane, measured with the same `ProfileGPU` method as LOOK-P2-NIGHT.

Submission-form text for "tools used" if any NvRTX feature stays (option 2 only):
`NVIDIA RTX Branch of Unreal Engine 5.8 (NvRTX: RTXDI, ReSTIR GI, DLSS Ray Reconstruction) for trailer footage only; the game itself runs stock Unreal Engine 5.9 with NVIDIA DLSS 4.5 Super Resolution / DLAA.`
With option 1 the field stays as `docs/SUBMISSION.md` has it (Unreal Engine 5.9, NVIDIA DLSS).

## 5. Housekeeping

- Editor: one `UnrealEditor.exe` of this lane (pid 6596, port 8012), stopped with `editor.ps1 -Stop` (only the tree this lane started). Another session's editor (pid 26964) and lane a3's three `UnrealEditor-Cmd.exe` commandlets were left alone.
- Lab folder `C:\Users\kazda\kiro\a24-lab` (50 MB of copy, about 1 GB of Binaries/Intermediate): can be deleted; nothing in it is referenced.
- The NvRTX tree was not written to by this lane. `Engine/DerivedDataCache` inside it is the engine's own local DDC and received the editor's shader keys (the engine writes there by its default config).
