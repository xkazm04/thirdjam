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

## 2. Integration (Phase 2, pending approval)

## 3. NVIDIA questions draft (Phase 2)
