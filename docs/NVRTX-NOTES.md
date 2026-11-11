# NvRTX: what it could bring, and what it would cost

Status: web research on 2026-11-11, from NVIDIA's developer pages and forums. Nothing was downloaded. Evidence is vendor text and
forum posts; none of it is measured on this machine.

## What it is

NVIDIA's RTX branch of Unreal Engine ("NvRTX") is a source fork of Unreal that adds NVIDIA ray tracing and path tracing work ahead
of, or beside, stock Unreal. Per NVIDIA: DirectX Raytracing enhancements, DLSS 4, and (experimental branch) path tracing for
Nanite geometry via RTX Mega Geometry, real-time path-traced hair via Linear Swept Spheres, RTXDI, RTXGI, Shader Execution
Reordering (an older NVIDIA claim: up to 40% faster ray and path tracing on an RTX 4090 for a 5.1-era branch) and Opacity
Micro-Maps. A recent release (5.7.4) lists fixes for Mega Geometry shader compilation, Opacity Micro-Map stability, Substrate
materials and NvAPI. The DLSS plugin (Multi Frame Generation, Ray Reconstruction, Super Resolution) is a separate plugin, not exclusive to the branch.
NVIDIA also advertises a DLSS 5 Neural Rendering plugin for UE 5.8.2 (access by request); unknown for 5.9.

## What it would bring us in theory

- A path-traced look for stills, the trailer and possibly gameplay on a 4090: the strongest single visual upgrade over Lumen.
- Path-traced Nanite geometry, which matters if device B builds dense scenes.
- A credible entry for the NVIDIA RTX Award, whose criteria are not in the rules; the itch page only says to list tools used.

## What it would cost

- **Source build.** Access needs a GitHub account linked to the Epic account in the Unreal dashboard, OAuth authorization and an
  emailed invitation. Then clone, run setup, generate project files and compile the whole engine. NVIDIA's forums say you need a
  good understanding of building Unreal itself. Disk: not stated anywhere I found; a source Unreal tree plus build is typically
  well over 100 GB, so treat 150 GB as the planning figure (unverified).
- **No 5.9 branch found.** The results covered up to 5.8. A community compile patch existed for 5.8 before the official release.
  Whether an official NvRTX for 5.9 exists, or comes with the "MegaJam" tooling, is unconfirmed.
- **Experimental branch caveat** (NVIDIA): not fully validated, may contain regressions, not suitable for shipping.
- **Different engine build from device A.** Assets from a source-built engine may not open in the launcher build and vice versa.
  A split-engine setup breaks the asset exchange rule in PLAN.md section 5.
- **Performance.** Path tracing is expensive: one report puts a heavy NvRTX demo at 13 FPS at native 4K on an RTX 5090. A
  judge's machine is not a 4090.

## Decision (operator, 2026-11-11)

Defer. Device B may download and experiment while it creates assets, and the question is revisited with real numbers.
Guardrails:

1. Device A and the shipped game use stock UE 5.9 (promoted build 5.9.0) unless NvRTX is proven on both devices by H12.
2. Device B's NvRTX build lives in its own folder and never opens the shared project until the test in guardrail 3 passes.
3. Test: open a copy of the shared project in NvRTX on B, save one asset, open it on A with stock 5.9. If it opens, NvRTX is a candidate
   for hero stills and the video only; if it does not, NvRTX stays an experiment and its footage can still be used in the dev log.
4. Check the kickoff stream, the daily 20:00 CET livestream and the jam Discord for what the NVIDIA award requires before spending hours on it.
5. No NVIDIA logo in the game or video (rule 8(d) trademarks); name the tools in the dev log and the submission's tools field.

## Sources

- https://developer.nvidia.com/game-engines/unreal-engine/rtx-branch
- https://developer.nvidia.com/blog/optimize-your-ray-tracing-graphics-with-the-new-nvidia-rtx-branch-of-unreal-engine-5/
- https://forums.developer.nvidia.com/t/unreal-engine-nvrtx-branch/337739
- https://forums.developer.nvidia.com/t/official-dlss-5-neural-rendering-plugin-access-for-unreal-engine-5-8-2/382731
