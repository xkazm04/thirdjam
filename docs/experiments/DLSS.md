# DLSS in the shipped build (lane A11, ruling R-0006)

Owner: lane A11 (device A). Section 1 is Phase 1: research only, nothing downloaded. Section 2 (integration numbers) and section 3 (NVIDIA questions draft) follow once the operator approves the download.

## 1. Research (2026-11-12, H5.2+)

### 1.1 Headline: NVIDIA has no 5.9 build yet

The engine here is **UE 5.9.0** (`C:/Program Files/Epic Games/UE_5.9/Engine/Build/Build.version`: 5.9.0, CL 60351404, `++UE5+Release-5.9`, promoted build).
The newest official NVIDIA DLSS Unreal plugin is **DLSS 4.5 plugin v8.9.0, dated 2026-10-16**. NVIDIA ships it for **UE 5.8, 5.7, 5.6 and 5.5 only**. On 2026-11-12 the download page had no 5.9 package and the archive thread had no 5.9 entry.
It is not on Fab: NVIDIA distributes it only from developer.nvidia.com.

The stock 5.9 engine has no DLSS of its own. `Engine/Plugins/Runtime/Nvidia/` holds only `GeForceNOWWrapper` and `Reflex`. Reflex is Epic's latency plugin and is not an R-0006 feature.

NVIDIA's plugin packages have shipped the plugin **source** in past versions. That makes this the realistic path: take the **UE 5.8 package** and rebuild the plugin against 5.9 inside `Game/Plugins/`.
Risk: the plugin hooks into the renderer (scene view extension, upscaler interface, RHI resources). If that renderer API changed between 5.8 and 5.9, the compile breaks. How big this risk is can only be known after the download.
Proposed time box for the port: 45 minutes of compile fixes. If it still does not build, stop and report. There is no fallback that would count for the award: NvRTX is 5.8-only and cannot open the project (R-0006 item 2).

### 1.2 The download

| | UE 5.8 (the one to port) | UE 5.7 (for reference) |
|---|---|---|
| URL | `https://developer.nvidia.com/downloads/assets/gameworks/downloads/secure/dlss/ue-dlss-5.8/8.9.0/2026.10.16_ue5.8_dlss4.5plugin_v8.9.0.zip` | same path with `5.7` |
| File | `2026.10.16_UE5.8_DLSS4.5Plugin_v8.9.0.zip` | `2026.10.16_UE5.7_DLSS4.5Plugin_v8.9.0.zip` |
| Size (HTTP Content-Length, from a HEAD request only) | **1,166,608,575 bytes (1.09 GiB)** | 969,496,365 bytes |
| Last-Modified | 2026-10-15 | 2026-10-15 |
| Contents (NVIDIA page) | Streamline SL 2.14.16, NGX 310.9.16 | same |

**Login:** not needed. A `curl -I` with no cookies gets a 302 to `developer.download.nvidia.com/...?__token__=exp=...~hmac=...` and then a 200 `application/zip`. Only the headers were fetched; nothing was downloaded.
The web page may still show the licence on click. Downloading counts as accepting the licence in 1.4.

**Size note for PLAN 4b:** the 1.09 GiB zip is the whole package: plugins, editor and runtime binaries, and probably docs and sample content. It sits on disk in `Game/Plugins/` and is not part of the shipped zip.
What ships is the set of runtime DLLs staged next to the exe:
- NGX `nvngx_dlss.dll` for Super Resolution
- `nvngx_dlssd.dll` for Ray Reconstruction
- Streamline `sl.*.dll` and `nvngx_dlssg.dll` for Frame Generation

**This size is not measured yet.** These DLLs are large with the second-generation transformer models. My estimate for the staged delta is **100 to 300 MB uncompressed**, which is a real share of the 1 GB budget.
If that is too much, ship Super Resolution only: do not stage the RR and FG DLLs, and disable the Streamline FG modules. R-0006 allows this ("Super Resolution at least"). Phase 2 measures the delta.
The `Binaries/` and `Intermediate/` folders from a 5.9 rebuild are also large. The Director decides whether the plugin folder stays out of git LFS or is ledgered in `ASSETS.md`.

### 1.3 Features in the 8.9.0 package

NVIDIA's DLSS page lists:
- **Super Resolution** (second-generation transformer model)
- **DLAA**
- **Ray Reconstruction**
- **Frame Generation / Multi Frame Generation**, including **Dynamic MFG** and a 6x mode
- **Reflex Low Latency**
- **NVIDIA Image Scaling (NIS)**

Hardware tiers come from a secondary source (strayspark). NVIDIA's own per-feature table is in the plugin's PDF guide inside the zip; confirm it in Phase 2.
- Super Resolution, DLAA, Ray Reconstruction: RTX 20 series and newer.
- Frame Generation / MFG: RTX 40 series and newer. 6x MFG: RTX 50 series only. The RTX 4090 here can run 2x FG (and Ada's MFG ceiling) but not 6x.
- Frame Generation needs DX12 and pulls in Reflex. The project's default RHI is DX12 (`DefaultEngine.ini` line 23).

For this game the target is Super Resolution + DLAA. Frame Generation is optional: a 3v3 arcade game with 60 Hz physics gains little from generated frames, and FG adds size and latency risk.

### 1.4 Licence (NVIDIA RTX SDKs License + RTX Supplement, both v. 2024-03-14, `github.com/NVIDIA/DLSS/LICENSE.txt`)

The plugin package carries its own licence file, which I expect to be this text. In Phase 2, read it inside the zip before copying anything into the repo. What matters for a jam build:
- **Redistribution is allowed.** We may ship SDK files in object code form inside our application (DLLs staged with the game). We may not ship them as a standalone product, and the application must add material functionality, which a game does. (§1c, §2a, §4b)
- **No evaluation-only clause**, and no free-use or non-commercial exemption. The same terms apply to a free jam game.
- **Attribution is required.** DLSS games must attribute the SDK and show NVIDIA marks on the **splash screen, the about box and the credits** (Supplement §7.1(b)). For lane A8 / credits: add an "NVIDIA DLSS" credit line.
  This conflicts with two other clauses. Using the "NVIDIA RTX" marks or logos needs prior written approval, requested two weeks ahead (§7.2c). NVIDIA marks may not be combined with other content without approval (Supp. §7.3b).
  The safe jam reading: a **plain text credit** ("NVIDIA DLSS technology, (c) NVIDIA Corporation") in the credits and about box, with **no NVIDIA logo**. Question for the Director/operator: is a text credit enough, or should we ask the organisers or an NVIDIA contact?
- **Notify NVIDIA before commercial release** (developer.nvidia.com/sw-notification, Supp. §4). The licence does not define "commercial release". A free jam submission is probably not one, but filing the notice costs little. Operator's call.
- The SDK is licensed only for systems with NVIDIA GPUs (Supp. §1). The code path must stay inactive on other GPUs, which is what we want anyway.
- Modified plugin **source** that we distribute, such as the 5.9 port patches, must carry "This software contains source code provided by NVIDIA Corporation." (§2b). We ship binaries only, so this applies to the repo, not the zip. Add the line to the headers of the patched files anyway.
- Other terms: no reverse engineering; no implied NVIDIA sponsorship (§4c); provided as-is with liability capped at US$10; Delaware law. NVIDIA may terminate the licence if it stops offering the SDK.

### 1.5 Minimum driver

NVIDIA's download page does not state it. A secondary source (strayspark guide) gives **572.40 or later** for the DLSS 4.5 plugin features. This machine runs an **RTX 4090 on driver 610.88** (`nvidia-smi`), well above any likely minimum.
The exact minimum per feature is in the plugin's release notes or PDF inside the zip. Record it in Phase 2.

### 1.6 Non-NVIDIA GPUs (must no-op)

Earlier plugin guides describe this behaviour; verify it with 8.9.0. At startup the DLSS module queries NGX. If the GPU, driver or OS is unsupported, `IsDLSSSupported` / `QueryDLSSSRSupport` returns a not-supported reason and the plugin does **not** register its upscaler. The engine then keeps **TSR**. Frame Generation (Streamline) likewise reports unsupported and stays off.
`UJamDlssSettings::SetMode` must check support first and fall back to `off` (TSR) when DLSS is unsupported. On AMD/Intel, `auto` resolves to off.
Risks to test in Phase 2:
- (a) Streamline's interposer DLL loading on non-NVIDIA hardware. Older Streamline versions had crash and warning reports. The plugin has a switch that stops Streamline loading on non-NVIDIA adapters; check that it is on.
- (b) There is no AMD GPU here, so use the plugin's own "force unsupported" test (a console variable or command-line switch) to simulate a non-NVIDIA machine.

### 1.7 What I need approved (the ask)

1. Download **`2026.10.16_UE5.8_DLSS4.5Plugin_v8.9.0.zip` (1.09 GiB)** from the URL in 1.2 into a scratch folder outside the repo. Downloading accepts NVIDIA's RTX SDK licence.
2. Port it to 5.9 by rebuilding it in `Game/Plugins/`, with a 45-minute time box for compile fixes; on failure, stop and report. Copy in only the runtime and editor plugins we use: DLSS (Super Resolution), plus Streamline only if FG is wanted. Leave out samples and docs.
3. Credit with the text "NVIDIA DLSS" in the credits and about box, and no logos unless the operator gets approval.
4. Decide whether Frame Generation is in or out. Default proposal: **out**, shipping Super Resolution + DLAA only, to keep size and risk down.

### Sources (seen 2026-11-12)

- NVIDIA DLSS download page: https://developer.nvidia.com/rtx/dlss/get-started ("Last Updated: October 2026"; v8.9.0 for UE 5.8/5.7/5.6/5.5; SL 2.14.16, NGX 310.9.16)
- NVIDIA DLSS overview: https://developer.nvidia.com/rtx/dlss (DLSS 4.5 feature list)
- NVIDIA forum, plugin archive: https://forums.developer.nvidia.com/t/nvidia-dlss-unreal-engine-plugin-archive/377564 (UE 5.8 first appears in 8.7.0/8.7.2, July 2026; no 5.9)
- NVIDIA blog, 2026-05-27: https://developer.nvidia.com/blog/whats-new-for-game-developers-in-nvidia-rtx-dlss-4-5-for-ue5-and-multilingual-ai-characters/ (DLSS 4.5 features, Streamline-based)
- Licence: https://github.com/NVIDIA/DLSS/blob/main/LICENSE.txt (NVIDIA RTX SDKs License + RTX Supplement, v. 2024-03-14)
- Secondary: https://www.strayspark.studio/blog/dlss-4-5-dynamic-multi-frame-generation-ue5-guide (driver 572.40+, per-feature GPU tiers; not NVIDIA's text)
- HTTP headers of the download URLs (HEAD only, 2026-11-12): sizes and Last-Modified in 1.2.
- Local: `UE_5.9/Engine/Build/Build.version`, `UE_5.9/Engine/Plugins/Runtime/Nvidia/`, `nvidia-smi`.

## 2. Integration (Phase 2, 2026-11-12, approved by the operator at about 11:10)

### 2.1 What was fetched and what went in

- Zip `2026.10.16_UE5.8_DLSS4.5Plugin_v8.9.0.zip`: 1,166,608,575 bytes, sha256 `adeeb59b9635794e05252438bf01307bf8a629769e8f0517237363f690bb380c`, 1104 entries.
  It contains no scripts or executables (`.bat/.cmd/.ps1/.exe/.py/.sh`), only plugins, PDFs, a sample project and `.reg` helper files. The listing was read before anything was unpacked.
- Contents: `Plugins/DLSS` (SR + RR + DLAA), `StreamlineNGXCommon` (a small common module, no DLLs), `StreamlineCore` / `StreamlineDLSSG` / `StreamlineReflex` / `StreamlineDeepDVC` (Frame Generation, Reflex, DeepDVC),
  `NIS`, `DLSSMoviePipelineSupport`, a `Samples/DLSS4Sample` project, `Documentation/*.pdf` (EULA, SR guide, release notes), and `Platforms/WinGDK`.
- **Installed into `Game/Plugins/`: only `DLSS` and `StreamlineNGXCommon`.** DLSS-SR does not need the Streamline DLLs (`DLSS.uplugin` depends only on `StreamlineNGXCommon`).
  Left out: Frame Generation, Reflex, NIS, DeepDVC, the movie pipeline module, the sample, the docs, the arm64 files, the `Intermediate/` folders, the `.pdb` files and the prebuilt 5.8 editor binaries.
  **`nvngx_dlssd.dll` (Ray Reconstruction, 48 MB) was deleted.** `NGX.Build.cs` stages only DLLs that exist, so Shipping stages `nvngx_dlss.dll` (56.2 MB) alone, and RR reports unsupported (`DLSS-RR=0` in the log, as intended).
- On disk: 130 MB of source plus DLLs, and the editor binaries built by UBT. Gitignored (`Game/Plugins/DLSS*/`, `Game/Plugins/Streamline*/`, added by the Director).
- `tools/dlss/fetch-dlss.ps1` repeats all of this for device B: download (or `-Zip <path>`), sha256 check, filtered extraction in .NET (nothing from the archive runs), and the 5.9 port edits.
  Tested into a scratch tree, the result is identical to the hand install except for line endings.

### 2.2 The 5.9 port (compile result: PASS, at about 11:22; the 45-minute cap started 11:18)

- Both `.uplugin` files: `"EngineVersion": "5.8.0"` became `"5.9.0"`.
- UE 5.9's compile-time format-string checker rejects enum and `int64` arguments for `%d`/`%u`. That needed **ten one-line fixes in two files** (log and `checkf` message arguments only), each marked `Third Jam 5.9 port` (§2b of the licence: the files keep NVIDIA's header).
  `DLSSLibrary.cpp`: two `%d` -> `%lld`, one `int32(...)` cast, one `uint32(...)` cast. `DLSSUpscaler.cpp`: one `int32(...)` cast and five `uint32(...)` casts. No behaviour change.
- All ten plugin modules then compiled and linked: DLSSUtility, DLSS, DLSSBlueprint, NGXRHI, NGXD3D11RHI, NGXD3D12RHI, NGXVulkanRHI, NGXVulkanRHIPreInit, DLSSEditor and StreamlineNGXCommon.
  No other API breaks between 5.8 and 5.9 hit this plugin.
- Editor load: `LogDLSSBlueprint: Loaded DLSS-SR plugin version 8.9.0-NGX310.9.16`, then `NVIDIA NGX DLSS supported DLSS-SR=1 DLSS-RR=0` on the RTX 4090 (driver 610.88, D3D12).
  **NGX reports the minimum driver for DLSS-SR as 470.0.** Supported quality fractions: Ultra Performance 0.333, Performance 0.50, Balanced 0.58, Quality 0.667, DLAA 1.0. Ultra Quality is unsupported.
- Engine-side files touched:
  - `Game/ThirdJam.uproject`: `{ "Name": "DLSS", "Enabled": true, "Optional": true, "PlatformAllowList": ["Win64"] }`. With `Optional`, a checkout without the plugin (device B before running the fetch script) still opens.
  - `Game/Source/JamGame/JamGame.Build.cs`: adds `DLSSBlueprint` and defines `JAM_WITH_DLSS=1` only when `Game/Plugins/DLSS/DLSS.uplugin` exists and the target is Win64. Otherwise `JAM_WITH_DLSS=0` and DLSS is compiled out.
  - `Game/Config/DefaultEngine.ini`: **one line added**, at the end under a new `[SystemSettings]` section: `r.NGX.Enable.AllowCommandLine=1`.
    Without it the plugin ignores `-ngxdisable` / `-ngxenable`. With it they work as a support kill switch in the shipped game and as the unsupported-hardware test. Otherwise the plugin's defaults stand.

### 2.3 `UJamDlssSettings` (JamGame, `Public/DlssJam.h`, `Private/DlssJam.cpp`)

The file is named `DlssJam.h`, not `DlssSettings.h`: UHT refuses two headers with the same name, and the plugin has `DLSSSettings.h`. The first name broke the editor target for other lanes for a few minutes around 11:14 to 11:20.

API for lane A8 (static, BlueprintCallable/Pure, category `Jam|DLSS`):
- `UJamDlssSettings::IsSupported()`: bool.
- `GetUnsupportedReason()`: FString. For example "needs an NVIDIA RTX GPU", "NVIDIA driver too old", "not in this build"; empty when supported.
- `GetAvailableModes()`: `TArray<EJamDlssMode>`. Returns all six modes when supported, otherwise only `Off`.
- `SetMode(EJamDlssMode)`: applies the mode and saves it as `[/Script/JamGame.JamDlssSettings] Mode=quality` in `GameUserSettings.ini`.
- `GetMode()`: the saved mode, default `Auto`. `GetEffectiveMode()`: what is running now (`Off` when unsupported).
- `ModeToString` / `ModeFromString`: `auto | quality | balanced | performance | dlaa | off`.
- `ApplySaved()`: re-applies the saved mode. Call it after a resolution change so `Auto` re-picks.
- `EJamDlssMode { Auto, Quality, Balanced, Performance, DLAA, Off }`.
- Console: `jam.dlss [mode]` sets the mode; with no argument it prints support and the saved and effective modes.

How a mode is applied:
- `Auto`: the plugin's recommendation for the current viewport size (`GetDLSSModeInformation(Auto, size)`). Quality, Balanced and Performance use the plugin's optimal screen percentage for that mode. DLAA runs at 100 %.
- Every on-mode sets `r.ScreenPercentage` (priority `SetByGameSetting`) and calls `UDLSSLibrary::EnableDLSS(true)`.
- `Off`, or any mode on unsupported hardware: `EnableDLSS(false)` and `r.ScreenPercentage 100`, so the engine keeps TSR.

`UJamDlssSubsystem` (a GameInstanceSubsystem) applies the saved mode once per game instance. In `-game` and packaged builds the game instance exists before the DLSS module loads.
The subsystem therefore waits for `FCoreDelegates::OnFEngineLoopInitComplete`. `OnPostEngineInit` is too early: UE 5.9 broadcasts it before it loads the PostEngineInit-phase modules (LaunchEngineLoop.cpp lines 4977 / 5005).
The first `-game` runs logged "IsDLSSSupported should not be called before PostEngineInit" and fell back to off. After the fix (editor build at 12:01), a `-game` run with no command logged `DLSS on: mode quality, screen percentage 66.7` at startup from the saved mode.

### 2.4 Measured (`-game` from the editor binaries, Rink map, CSV profiler `-csvGpuStats`, median and spread of the last 600 of 900 frames)

The game is capped at 60 fps, so FrameTime is 16.67 ms everywhere and GPU ms is the comparison. **Caveat: three or four other lanes' editors were running on the same RTX 4090 the whole time.**
Repeated runs of the same setting moved by up to 2x (1440p TSR measured 14.0 ms once and 6.8 ms the next time), so these numbers show trends only, not a final answer.

| Run | GPU ms p10 / p50 / p90 | Upscaler pass ms |
|---|---|---|
| 1080p TSR, `-ngxdisable` | 4.47 / 4.56 / 4.69 | TSR 0.58 |
| 1080p TSR (`jam.dlss off`), run 1 / run 2 | 4.21 / 4.99 / 5.43 ; 4.16 / 4.24 / 4.59 | TSR 0.53 |
| 1080p DLSS Quality (66.7 %), run 1 / run 2 | 4.73 / 6.09 / 6.91 ; 3.70 / 4.26 / 4.79 | DLSS 0.43 |
| 1440p TSR, run 1 / run 2 | 11.91 / 14.03 / 15.44 ; 5.89 / 6.77 / 9.21 | TSR 2.40 (run 1) |
| 1440p DLSS Quality, run 1 / run 2 | 6.12 / 8.57 / 10.88 ; 4.99 / 7.29 / 8.94 | DLSS 0.61 |
| 1440p DLAA (100 %) | 5.74 / 6.03 / 6.48 | DLSS 0.84 |

Reading:
- The game uses 4 to 7 ms of a 4090 at 60 fps, so DLSS has little to save here. The DLSS pass (0.4 to 0.6 ms) costs about what TSR costs.
- The passes that scale with resolution do shrink with DLSS Quality: Lumen reflections went from 0.79-1.02 ms to 0.45-0.53 ms at 1080p, and the base pass from 0.42-0.49 ms to 0.13 ms at 1440p.
- On a mid-range RTX card (the jury's machines are unknown) that saving is the real gain. On this 4090 it is inside the noise.
- A clean A/B with the other editors closed (a :00 or :30 link window) is the follow-up if the number is wanted for the NVIDIA questions.
- `stat gpu` on screen was not used: the CSV GPU columns are the same timers in a form that can be recorded.

Screenshots at 1080p after 25 s of the opening take (the AI skaters are moving, so the frames differ in their positions): `docs/experiments/img/DLSS-off-1080p.png` (TSR 100 %) and `docs/experiments/img/DLSS-quality-1080p.png` (DLSS Quality 66.7 %).
Same scene, no artefacts visible at this size.

### 2.5 Unsupported hardware

`-ngxdisable` is the plugin's switch for simulating a machine without NGX. **It only works with `r.NGX.Enable.AllowCommandLine=1`, whose default is 0.** So the earlier "`-ngxdisable`" rows (2.4, and the first package in 2.6) did not disable NGX.
The 1080p "`-ngxdisable`" row in 2.4 is TSR only because the apply had not run yet.
With the ini line, `-game -ngxdisable` logged:
- `NVIDIA NGX library loading has been disabled with r.NGX.Enable=0`
- `DLSS-SR=0`
- `LogJamDlss: DLSS off (requested quality, unsupported: not available on this machine)`

That run did 900 frames on TSR (GPU 5.31 ms, TSR 0.66 ms) with no error and no crash.
The DLL is loaded only through NGX on NVIDIA drivers. On AMD or Intel, `QueryDLSSSupport` returns `NotSupportedIncompatibleHardware`, `auto` resolves to off, and Settings shows only "Off" with the reason.

One crash during these runs: `-game` with `jam.dlss off`, at frame 0, `EXCEPTION_ACCESS_VIOLATION` in Engine under MoviePlayer, while first-run shaders were compiling (no symbols).
The identical command did not reproduce it in the next eight runs. Logged, not explained.

### 2.6 Shipping package (`tools/run/package.ps1 -Config Shipping -Zip`, output in the A11 scratchpad, not shipped)

**Package of record, 12:04** (after the startup fix and the ini line):
- PASS. Build, cook and stage took 68 s. **Zip 376.2 MB, staged 548 MB**, under the 1000 MB limit. The A6 baseline was 322.5 MB / 465.9 MB at about 10:00, before this morning's content and code.
- **DLSS adds exactly one staged file: `ThirdJam/Plugins/DLSS/Binaries/ThirdParty/Win64/nvngx_dlss.dll`, 56.2 MB (29.7 MB deflated)**, plus the plugin's code linked into the Shipping exe. That code is about 1.5 MB, judging by the editor module sizes; the exe is 171.7 MB.
  Nothing from Streamline, Frame Generation, Ray Reconstruction or NIS is staged.
  **Budget: about 57 MB staged / about 30 MB zipped.** It belongs in the engine/runtime slice (A) of PLAN 4b and comes out of the 230 MB reserve.
- `run-packaged.ps1 -Seconds 10` with the default mode: PASS, stayed up 10.4 s. On this RTX 4090 the default is Auto, so DLSS is on.
  Shipping writes no log, so this run proves only that the Shipping exe runs with the plugin. The startup apply itself is proven by the `-game` run in 2.3, which uses the same code.
- `run-packaged.ps1 -Seconds 10 -ExtraArgs "... -ngxdisable"` (NGX really off now): PASS, stayed up 10.4 s.

Earlier package at 11:47: PASS, but the zip was 611.2 MB. The stage picked up a stale Development game build (`ThirdJam.exe` 330 MB + `ThirdJam.pdb` 373 MB) that another lane had left in `Game/Binaries/Win64`; it was gone by 12:04.
**For the Director:** `package.ps1` should exclude or clean those files, because whichever lane packages next can ship 700 MB of debug build.

### 2.7 Post-jam items

- File NVIDIA's release notification (developer.nvidia.com/sw-notification, Supplement §4) if the game is ever released commercially or publicly beyond the jam. The Director chose not to file it now.
- A logo needs NVIDIA's written approval (§7.2c). Until then the credit is text only.

## 3. NVIDIA questions draft (for submission; to be checked against the actual form)

- **Which NVIDIA technology does the game use?** NVIDIA DLSS 4.5 Super Resolution and DLAA, through NVIDIA's official DLSS Unreal Engine plugin v8.9.0 (NGX 310.9.16). The plugin was rebuilt for Unreal Engine 5.9 with ten one-line format-string fixes.
- **How is it used?** DLSS is on by default (Auto) on any RTX GPU. Settings offers Auto / Quality / Balanced / Performance / DLAA / Off. On non-NVIDIA hardware the game runs with Unreal's TSR and the option shows as unavailable.
- **What did it bring?** Higher output resolution at the same GPU cost on mid-range RTX cards: Quality renders 66.7 % of the pixels and the resolution-scaled passes (Lumen reflections, base pass) shrink accordingly. DLAA gives the best anti-aliasing on fast cards.
  Numbers: section 2.4, to be refreshed with a clean A/B.
- **Not used:** Frame Generation, Ray Reconstruction, Reflex, NvRTX, ACE. These are kept out deliberately for size and latency in a 60 Hz arcade game.
- **Credit line** (title and credits screens, lane A8): `NVIDIA DLSS` as text, no logo.
