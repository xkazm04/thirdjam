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

## 5. Parallel lanes (both machines are cleared for this project; use them)

The unit of parallelism is a **lane**: one builder, one git worktree (`.worktrees/<lane>` on branch `lane/<device>-<name>`), its own project copy,
its own project-local derived-data cache, and, when it needs the editor, its own editor instance on its own MCP port (`tools/run/editor.ps1 -McpPort 8001`, `8002`, ...,
which passes `-ModelContextProtocolPort`; the client follows with `tools/mcp/mcp.mjs --url http://127.0.0.1:8001/mcp`). Builders never share an editor, a port or a cache. A lane that only edits text (C++, JSON, prompts,
tests, docs) needs no editor at all; the lead compiles and tests its result in the lead's own worktree before merging.

| Device A (64 GB, RTX 4090), the integrating Director's machine | Device B (128 GB, high-end GPU), the art lead's machine |
|---|---|
| **A1 core**: rink builder, skater sim, puck, shooting, goals, rules, cartridge loader (Opus) | **B1 materials and light rigs**: ice/boards/net masters, `LR_L1..L3`, post volumes (Opus) |
| **A2 AI and simulation**: bot brain, roles, yield rules, headless bot-vs-bot tournaments over seeds (Opus) | **B2 dressing and meshes**: arena surroundings for the chosen path, Fab selections, generated props, 2K cap (Sonnet) |
| **A3 balancing sweeps**: headless parameter sweeps on the fixed-step sim (shot speed, skater grip, check thresholds, pickup magnitudes); reports curves, proposes cartridge values (Sonnet) | **B3 VFX and audio**: Niagara per event, MetaSounds, puck readability kit, pickups (Opus) |
| **A4 cartridge and Designer**: `game` schema, presets, Designer D0/D1, validator port to C++ (Sonnet) | **B4 skater T-B1**: humanoid + retarget behind `IJamSkaterVisual` (Opus; needs a 10-20 min human step for the AnimBP graph) |
| **A5 camera, HUD, input, first-play script**: third-person camera, radar, tutorial text, controller support (Sonnet) | **B5 skater T-B2**: generated rigged figure via partner tools (Sonnet; needs operator accounts) |
| **A6 QA, packaging, dev log**: automation tests green, Shipping packages every 6 h, screenshots, `devlog/` build (Sonnet) | **B6 capture and video**: screenshots per milestone, the 1080p trailer, NvRTX stills if the trial passed (Sonnet) |
| Editors: up to **2** at once (two lanes driving MCP), one cook/package at a time | Editors: up to **3** at once, one cook/package at a time, one generation or render job at a time |
| Builders: up to **5** live, plus the Director | Builders: up to **7** live, plus the lead |

Lanes merge upward: lane branch -> device branch (`dev-a` / `dev-b`) by the device lead every 90 minutes after the lane's own tests pass; device branch ->
`main` by the Director at the 3-hour windows (section 4). A lane that touches another lane's files is split, not merged. Headless work (A2, A3, A4 tests) runs
the fixed-step simulation without rendering, so it costs CPU only and scales with cores, not with the GPU.

**Tempo rules**, read by each lead every hour from its own measurements (`Get-CimInstance Win32_OperatingSystem` free RAM, `nvidia-smi` GPU memory, editor logs):

| Reading | Rule |
|---|---|
| free RAM < 12 GB (A) / < 24 GB (B) | no new editor; new builders only on text lanes |
| free RAM < 6 GB | no new builder of any kind; the next cook or package waits |
| a cook or package running | no second cook or package on that device; editors keep running |
| GPU memory > 85% or a render, generation or NvRTX job running | no new PIE session; screenshots wait |
| any wall-clock failure (frame budget, timeout) while another heavy job ran | re-run alone before it is believed (lesson from the Godot night) |
| builder silent 15 minutes past its time box | check it on disk; restart it from its brief file |
| the human asleep | the lane counts above still apply; what waits for eyes is asset *selection* (Fab picks, generated-mesh picks) and any decision in a brief marked "operator"; everything else runs |

NvRTX trial on A (tonight, if access arrives): clone and `Setup.bat` run at low priority; the engine build runs only when no jam work is on A,
and never past H0 while A's editor is in use. The trial engine never opens the shared project (NVRTX-NOTES.md guardrails).
