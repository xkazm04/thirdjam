# Two machines, two tracks, one game

Status: plan v2, 2026-11-12 ~01:00 CET. Replaces the "two devices" table in PLAN.md section 5 where they differ.

## 1. Machines

| | Device A (this machine) | Device B |
|---|---|---|
| Hardware | 64 GB RAM, RTX 4090 24 GB, C: 440 GB free after cleanup | 128 GB RAM, high-end gaming GPU, less free disk (operator: cleanup pending) |
| Lead | Fable 5.2 session, the **integrating Director**: owns `main`, merges, gates, packaging, submission | Fable 5.2 session, the **art and experiments lead**: owns the content plugin, light rigs, skater tracks T-B1/T-B2, video, NvRTX trial |
| Agents under the lead | C++ builder, cartridge/Designer builder, QA bot builder (Opus or Sonnet) | asset builder (Fab sourcing, generation, import), lighting builder, animation builder, audio builder |
| Editors | one UnrealEditor, one MCP driver | one UnrealEditor, one MCP driver; a second engine folder only for the NvRTX trial |

## 2. Scope lines (ownership.json is the lock)

| Track A (gameplay, A) | Track B (look, B) |
|---|---|
| `Game/Source/**` (JamCore, JamGame, JamDesigner), `Game/Content/Maps/**`, `Game/Content/Cartridges/**`, `Game/Content/Blueprints/**`, `Game/Content/UI/**`, `Game/Config/**`, `schema/**`, `prompts/**`, `tools/**` | `Game/Plugins/JamAssets/**` (meshes, materials, textures, VFX, audio, light rigs as sub-levels or Data Assets, skater visuals for T-B1/T-B2), `Game/Content/Look/**`, `Game/Content/Audio/**` |
| The skater pawn and `IJamSkaterVisual`; the toy figure T-A (it is code-driven) | every other skater visual, animation, retarget |
| Light tier **switch** (`game.look.tier` -> which rig loads) | the three light rigs themselves |
| Pickup logic, VFX **hooks** (events `shot`, `board`, `goal`, `faceoff`, `check`, `knockdown`, `pickup_spawn`, `pickup_take`, `win`, `lose`) | the VFX and sounds bound to those events |

Shared, any device: `handoff/**`, `status/**`, `ASSETS.md`, `devlog/**`, `retro/**`. Director only: `docs/**`, `contracts/**`, `ownership.json`.

## 3. Seams (the contracts both sides build against, written by the Director before fan-out)

1. `contracts/skater-visual.md`: the `IJamSkaterVisual` calls (HOCKEY.md section 5), the socket names on any skater mesh
   (`stick_socket`, `puck_socket`, `head_socket`), the asset name of the swap-in (`BP_SkaterVisual_<track>`), the test cartridge that swaps it.
2. `contracts/look-tiers.md`: three rig asset names (`LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon`), what each contains, the material parameter
   names gameplay may drive (`TeamColour`, `Emissive`, `Wet`, `Scuff`), the fog and post volume names, and the frame budget per tier.
3. `contracts/events.md`: the gameplay event names above with their payloads (who, where, strength) and the Niagara/MetaSound asset
   naming (`NS_<event>`, `MS_<event>`), so B can build effects against a stub emitter before A's gameplay exists.
4. `contracts/assets-naming.md`: `SM_ M_ MI_ T_ SFX_ NS_ BP_ LR_` prefixes, folder layout in `JamAssets`, the 2K texture cap, the ledger row per file.

A stub for each seam (empty interface, empty rigs, an event logger) is committed by A at H1 so B builds against something real from hour 1.

## 4. Sync (controlled, not continuous)

- One repo, Git + LFS, branches `dev-a` and `dev-b`, `main` moves only by the Director on A.
- **Merge windows every 3 hours** (H3, H6, H9, ...): B pushes `dev-b`; A runs `tools/gate/check-ownership.mjs --device B` on it, merges,
  packages, runs the bot tests, pushes `main`; B rebases on `main`. A failed merge is reverted on `main`, not fixed there, and B gets a
  handoff note naming the failing test.
- A change across the ownership line is a `handoff/<A|B>-to-<B|A>-NNNN.md` request with acceptance and a deadline hour; the owner does it.
- Hourly `status/<A|B>.md` append: done, next, blocked, free RAM/GPU, editor crashes, LFS size, packaged-build age.
- Nothing live is shared: no shared editor, no shared DDC, no shared port. Reference stills travel in the repo; big source files (raw
  generations, 4K originals) never enter git and are exchanged only if a handoff asks, via a dated zip outside the repo.

## 5. Tempo (the leads balance it; nobody runs the machine into the ground)

Each lead applies these rules every hour, from the numbers it reads itself (`Get-CimInstance Win32_OperatingSystem` for free RAM, `nvidia-smi`
for GPU memory and load, the editor log for crashes):

| Reading | Rule |
|---|---|
| free RAM < 16 GB (A) / < 32 GB (B) | no new builder; the next cook or package waits |
| one editor + one cook or package running | no second heavy job; builders do text-only work (C++, JSON, prompts, docs) |
| GPU memory > 80% or a render job (video, NvRTX, generation) running | no editor PIE sessions; no screenshot runs |
| any wall-clock failure (frame budget, timeout) while another heavy job ran | re-run alone before it is believed (lesson from the Godot night) |
| builder silent 15 minutes past its time box | check it on disk; restart it from its brief file |
| hour of the day with the human asleep | at most two builders per machine, both on text-only or test work; no asset imports (they need eyes) |

NvRTX trial on A (tonight, if access arrives): clone and `Setup.bat` run at low priority; the engine build runs only when no jam work is on A,
and never past H0 while A's editor is in use. The trial engine never opens the shared project (NVRTX-NOTES.md guardrails).
