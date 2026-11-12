# Status B

Device B, the art and experiments lead. Hourly blocks are appended below (`## H<n>`), format in `status/TEMPLATE.md`.

## pre-jam

Audit of this machine against `docs/DEVICE-B.md` section 1, run 2026-11-12 00:00-00:15 CET. Tooling checks only; no game content.

### Machine (differs from the plan's assumptions in `docs/TRACKS.md`)

- model: ASUS ProArt P16 H7607BA, **Windows 11 on ARM64** (10.0.28000), NVIDIA RTX Spark N1X: 20-core CPU, 6144-core Blackwell RTX GPU
- memory: **64 GB unified**, shared between CPU and GPU (TRACKS.md assumes 128 GB plus a discrete GPU). The GPU reports 64 GB "dedicated"; it is the same pool.
- disk: C: 258 GB free of 952 GB (floor for the jam 40 GB; need about 55-60 GB of it for UE 5.9)
- power: on AC, sleep and hibernate on AC set to never
- the Unreal editor is an x64 binary and runs under emulation here (editor log: `Architecture: x64`, `CPU: Virtual CPU`)

### Checklist

| Item | State |
|---|---|
| UE 5.9.0 CL 60351404 | OK since about 10:00 on 11-12 (`Engine/Build/Build.version`: 5.9.0, 60351404, promoted). UE 5.8.4 is still installed beside it. |
| Compiler | OK. Visual Studio **Build Tools** 2022 17.14.42 (no IDE): MSVC 14.44.35229 (folder 14.44.35207), x64 and ARM64 hosts, ATL, Windows SDK 10.0.26100, .NET 4.8 SDK. UBT accepts it (proven on 5.8, below). The "Game development with C++" workload is not installed and was not needed for the build. |
| git, git lfs | OK. git 2.56.0, git-lfs 3.8.0, filters present, long paths on |
| Node | OK. v24.22.1 |
| Claude Code | OK. 2.1.322 |
| gh | OK. logged in, `repo` scope |
| Clone and branch | OK. `C:\Users\kazda\jam\thirdjam`, branch `dev-b` created from `main` at ec5a2f7 |
| Disk at least 150 GB | OK. 258 GB free |
| Free RAM for an editor | **LOW.** 12-14 GB free. A ComfyUI server (`C:\Users\kazda\ai\ComfyUI`, pid 23148, started 2026-11-11 12:15) holds most of the pool: `nvidia-smi` shows 51 GB of 64 GB in use. Tempo rule for B is "no new editor under 24 GB free". Operator: close ComfyUI before H0 unless a generation job is intended. |
| `unreal-mcp` approval | waits for H0 (needs the real project folder and a running editor; operator-only click) |
| Scheduled unattended start | OK. Task `ThirdJam-H0-B`, next run 2026-11-12 06:00, registered 01:05 CET. `register-h0.ps1` first put it on **11 December** (date-format bug, handoff B-to-A-0002); trigger corrected by hand, battery stop conditions turned off, and the action dot-sources `C:\Users\kazda\jam\ue-env.ps1` before `start-h0.ps1` so the headless session gets the .NET fix below. |
| Partner accounts, Fab favourites, tier picks | operator-only; none recorded in `docs/PREP-CHECKLIST.md` yet |
| `tools/mcp/ANIMATION.md`, `tools/mcp/PHYSICS.md` | named in `docs/DEVICE-B.md` section 0 but **not in the repository** at ec5a2f7 |

### ARM64 blocker found, with a workaround (handoff B-to-A-0001)

`tools/run/build.ps1` fails out of the box on this machine: exit -2147450730 after 2 s, empty log. Cause: the engine's
`Engine/Build/BatchFiles/GetDotnetPath.bat` selects `win-arm64` when `PROCESSOR_ARCHITECTURE` is `ARM64`, but the launcher build ships only
`Engine/Binaries/ThirdParty/DotNet/10.0/win-x64`. `dotnet` then resolves to the system install (.NET 8 runtime, no SDK) and UnrealBuildTool,
which needs .NET 10, cannot start.

Workaround, proven: set these in the environment before any engine batch file runs (the batch file skips its own selection when
`UE_DOTNET_VERSION` is already `10.0`). `<engine>` is the engine root.

```
UE_DOTNET_VERSION=10.0
UE_DOTNET_ARCH=win-x64
UE_DOTNET_DIR=<engine>\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64
DOTNET_ROOT=<engine>\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64
DOTNET_MULTILEVEL_LOOKUP=0
DOTNET_ROLL_FORWARD=LatestMajor
PATH=<engine>\Engine\Binaries\ThirdParty\DotNet\10.0\win-x64;%PATH%
```

How B applies it: `C:\Users\kazda\jam\ue-env.ps1` (outside the repo, because `tools/**` is A's) sets the variables for the current process and
its children only. Dot-source it in every shell that runs an engine batch file: `. C:\Users\kazda\jam\ue-env.ps1` (5.9) or `-Assoc 5.8`. They are
deliberately **not** set at user or machine level: the system ARM64 `dotnet` comes earlier on the machine PATH than any user PATH entry, and a
global `DOTNET_ROOT` would break native ARM64 .NET programs. The partial 5.9 download already shows the same layout (`DotNet\10.0\win-x64` only).
Packaging (`RunUAT.bat`) uses the same batch file and needs the same variables; not yet tried.

### Toolchain proof, on UE 5.8.4 as a stand-in (must be repeated on 5.9.0)

Throwaway blank C++ project at `C:\Users\kazda\jam\tjsmoke`, MCP plugins enabled as in `tools/mcp/README.md` section 1, deleted afterwards.

| Step | Result | Device A on 5.9 for comparison |
|---|---|---|
| `build.ps1`, editor target, first build | 98.5 s (7 actions, UBA local executor), with the workaround above | 64.6 s |
| `smoke.ps1 -SkipBuild`: editor start to MCP ready, first start | 48.7 s | 43.5 s |
| `tools/list`, `list_toolsets` | answered (3 meta-tools on 5.8; 4 on 5.9) | |
| spawn and read back | **FAIL, expected on 5.8**: `Toolset 'EditorToolset.LevelToolset' not found`. The 5.8 toolset names differ from the ones `smoke.ps1` calls. Not a machine fault; it needs 5.9. | pass |
| editor stop | clean | |
| RHI | D3D12 on the N1X adapter. The log says feature level SM5 for this bare project (no `DefaultEngine.ini`); check SM6, Lumen and Nanite on the real project at H0. | |

Not measured yet: packaging on ARM64, frame rate, a second editor.

### Open before H0

1. Operator: UE 5.9.0 install started in the Epic launcher at 01:03 CET (about 55 GB); when done, confirm CL 60351404.
2. B: dot-source `ue-env.ps1`, run `tools/run/smoke.ps1` on a fresh throwaway 5.9 project, try one package, and replace the numbers above.
3. ComfyUI: **operator's instruction, 2026-11-12 about 01:15 CET: "if Comfy still running in 6:00, you are allowed to close it."** This is the one
   exception to "never kill a process you did not start", it applies from 06:00 on, and only to the ComfyUI server (the `python.exe` whose command
   line contains `ComfyUI\main.py`). The interactive B session has a check set for 06:01; if ComfyUI is still up after that, the H0 session may close it.
   Still the operator's: partner accounts, Fab favourites, the L1/L2/L3 tier picks.
4. Director: lane counts for B (three editors, seven builders, the 24 GB rule) were written for 128 GB; on 64 GB unified memory B plans one editor and adds a second only after measuring.

## H4 Late start: audit done, briefs written, waiting for the seams

- done: B was dark from H0 to H4. Windows Update restarted this machine at 02:34 (as it did device A at 02:33); it then sat in standby with nobody
  logged on until 09:56, so the 06:00 task `ThirdJam-H0-B` never ran, and the interactive session with its wake-ups died in the restart. UE 5.9.0
  CL 60351404 is installed (finished after the restart). `dev-b` fast-forwarded to `main` at 4fcaf9c. Written: `briefs/B0-lead-plan.md`, `B1-look.md`,
  `B2-dressing.md` (the dressing list: the night arena as a sound stage, for the Director's Cut theme), `B3-vfx-audio.md` (puck kit, event effects,
  the audio event list with synthesis notes), `B5-skater-meshy.md`. Handoffs opened: B-to-A-0001 (.NET on ARM64), 0002 (task date bug), 0003 (`briefs/**`
  has no owner, so B's briefs stay in unpushed commits until `ownership.json` allows them).
- done (10:15-10:22): **the seams landed** (`40f2316`, `b4ca93c`); `dev-b` fast-forwarded. Toolchain proof on UE 5.9.0 on this machine, every engine shell after
  `. C:\Users\kazda\jam\ue-env.ps1`:

  | Step | Result on B | Device A |
  |---|---|---|
  | throwaway blank C++ project, `build.ps1` first build | 104.6 s; incremental 4.1 s | 64.6 s |
  | throwaway, editor start to MCP answering | 102.6 s first ever, 14.6 s warm | 43.5 s / 10.5 s |
  | throwaway, spawn + read back through MCP | PASS by hand (`AddToSceneFromClass`, `FindActors`, 180 ms a call); project deleted | PASS |
  | **`Game/ThirdJam.uproject`**, `build.ps1` first build | **76.5 s** | 56 s |
  | real project, editor start to MCP answering / tools usable | **45 s / 53 s** (first start) | 40 s |
  | real project renderer | `Using Highest Feature Level of D3D12: SM6`; `r.CustomDepth` 3, Lumen GI and reflections 1, virtual shadow maps 1, distance fields 1, ray tracing 0 | same settings |
  | memory | editor at rest 4.1 GB working set, 3.1 GB GPU memory; 44.8 GB free with it up | |

  Three things the scripts do not survive on this machine (none is a machine fault):
  1. `smoke.ps1` calls `list_toolsets` and the spawn 1 to 7 s after MCP first answers; here the editor is still in its start-up slow task and refuses
     (`Not run: the Unreal Editor is running a slow task`). Tools were usable 7 to 8 s after MCP answered. So `smoke.ps1` reports FAIL on B although everything works.
  2. After `editor.ps1 -Stop` force-stops an editor, `127.0.0.1:8000` stays LISTENING with owner pid 0 for about 100 s; `editor.ps1` then refuses to start
     (`port 8000 is already in use by pid 0`, exit 2). Wait for the listener to clear, or exit the editor through `EditorAppToolset.ExitEditor`.
  3. The very first 5.9 editor start sat on a modal `Wait for ZenServer?` box: the Zen cache server took 38 s to come up (it had just been replaced by the 5.9
     version) and the editor asks after 20 s. Second start: no box. If it comes back after a reboot, start the editor again.

  Not tried yet: a package on this machine.
- next: lane B1 (`briefs/B1-look.md`): masters, both post passes, instances, `LR_L2_Night`, in the editor that is now up on port 8000. Then B3 part 1 (puck kit),
  B2 (room and sound-stage dressing).
- blocked: nothing blocks B1. Open towards A: B-to-A-0001 (.NET in the scripts), 0003 (`briefs/**` owner), 0004 (`FX/` or `VFX/` for the effects binder), 0005 (stencil
  and material names on A-built meshes). Operator: the `unreal-mcp` approval click in `Game/` (an editor is running now), the Meshy key in `.env` and Blender 4.2
  before B5 at H8, pausing Windows Update. The full H6 row will not be met at H6; estimate H9.
- lfs size: 0 (no LFS objects on `dev-b` beyond `main`)
- engine build: 5.9.0 CL 60351404 (`Engine/Build/Build.version`), launcher install
- packaged build age: none yet
- packaged size: none yet
- ledger: none
- machine: 64 GB unified memory, 50.6 GB free at 10:02 (ComfyUI is gone with the restart), C: 348 GB free, no editor running except the toolchain proof below

## H5 Look lane done, three lanes running in parallel

- done: lane B1 finished in 10 minutes (report `status/B1-report.md`, six stills in `devlog/shots/`): five masters, seventeen instances, `LR_L2_Night` with
  `PPV_L2_Night`, `Sky_L2_Night`, four spots and a dark room sized for the 140 x 70 cut; `M_Ice` takes `RinkHalfLength` / `RinkHalfWidth`. The painted-cel
  look works on this GPU: banded pools, ink outlines, both post passes 0.04 ms each. PIE GPU time on B 10.5 ms median with a second editor view in the frame
  (device B number, not the gate's). Pushed `dev-b` at `ff0a382` (materials, rig, stills, ledger 25 rows; assets gate PASS). Operator: Meshy key placed
  (balance 1204), Blender 4.2.16 installed, expired task `ThirdJam-H0-B` removed.
- lanes now (the operator asked for more builders; measured headroom allows it: 43 GB free and 3.9 of 64 GB GPU memory with one editor up, CPU idle):

  | Lane | Where | Editor | Model | Box ends |
  |---|---|---|---|---|
  | B2 dressing, pass 1 (sound-stage set from basic shapes into the rig) | main tree | port 8000 | Sonnet | 12:45 |
  | B3 puck kit, then the M1 effects and MetaSounds | `C:\Users\kazda\jam\wt\b-vfx`, `lane/b-vfx` | port 8001 | Opus | 14:30 |
  | B5 Meshy skater T-B2, started early (generation needs no editor) | `C:\Users\kazda\jam\wt\b-skater`, `lane/b-skater` | port 8002, after download | Sonnet | 14:45 |

  Worktrees live outside the repo (`.worktrees/` is not in `.gitignore`, so inside the repo they would show as unowned files to the ownership gate).
  Machine rules for every lane: `briefs/B-machine-notes.md` (local, like the other B briefs, until handoff 0003 is answered).
- next: merge lane branches into `dev-b` as they report; a lighting pass on `LR_L2_Night` once A's rink is in `Rink` (the ice between the four pools is
  pure black in B1's stills: fill light or wider cones, and the blue line blotches under the ramp close to the camera); first package on this machine.
- blocked: none. Open: B-to-A-0001, 0003, 0004, 0005 (no answers on `origin/main` at 10:40). Operator: `unreal-mcp` still shows "Pending approval" in `Game/`
  (not blocking: lanes use `tools/mcp/mcp.mjs`); pausing Windows Update.
- lfs size: 22 material files and one rig map from B, under 0.3 MB together
- engine build: 5.9.0 CL 60351404
- packaged build age: none yet on B
- packaged size: none yet
- ledger: 22 material rows, `LR_L2_Night` row updated

## H5.1 Two editors, seven builders: the skater lab is open

- done: lane B2 pass 1 reported and is pushed (`775457b`): the night rig now carries 289 `Dress_L2_Night_*` actors of engine basic shapes (truss and lamp
  heads, eight film lights, slate, director's chair, crane, dolly track, cases, benches, exit signs), 48.8k triangles, about +0.3 ms on B, everything
  outside the 140 x 70 m footprint; six `MI_Dress_*` instances under `Arena/`; three stills. Doors in the boards not built (the boards are A's). Merged `main`
  at `7b22c39` (R-0002: all five handoffs answered; the .NET fix is in `tools/run`). Ownership and assets gates PASS at the push. Operator added
  `RODIN_API_KEY`; a Rodin client was written outside the repo (`C:\Users\kazda\jam\rodin-tools\rodin.mjs`; balance 421 credits; only `balance` has run for real).
- operator decision at 10:55: **two editors**, more builders without an editor, to test skater approaches against the plan's baseline; lower priority, a
  plan for a higher visual tier tomorrow. The main-tree editor was closed at 10:57 through `EditorToolset.EditorAppToolset.ExitEditor` (clean, port free at once).
- lanes now:

  | Lane | Editor | Model | Reports by |
  |---|---|---|---|
  | B3 puck kit, M1 effects and MetaSounds, plus `directors_call` | port 8001 | Opus | 14:30 |
  | B5 Meshy skater figure 1: import, material, sockets, AnimBP | port 8002 | Sonnet | 14:45 |
  | S1 three more figure styles in Meshy (mascot, lean athlete, wooden toy) | none | Sonnet | 12:30 |
  | S2 motion clips: nine needs, text-to-motion against library presets, loop seams | none | Sonnet | 12:45 |
  | S3 Rodin mesh, Meshy rig and clip: does the Hyper3D route work | none | Sonnet | 12:30 |
  | S4 Blender-scripted original articulated skater with scripted looping clips | none | Opus | 13:15 |
  | V1 plan for tomorrow's visual upgrade, with ready-to-run test scripts | none | Opus | 12:45 |

  Shared brief and rubric: `briefs/B-skater-lab.md`. Lab output lives in `C:\Users\kazda\jam\skater-lab\`; reports come to `status/B-skater-<lane>.md`.
- next: rank the lab candidates when they report; queue the best one or two for import on port 8002 after B5; lighting pass on the rig once A's rink is in.
- blocked: none. Open: B-to-A-0006 (how the builder cap counts lanes without an editor).
- lfs size: under 1 MB from B (31 files)
- engine build: 5.9.0 CL 60351404
- packaged build age: none yet on B
- packaged size: none yet
- ledger: 6 `MI_Dress_*` rows; `LR_L2_Night` row now 0.6 MB

## H5.3 Effects and audio in; first skater path proven; Rodin route proven

- done since H5.1 (all pushed, `dev-b` at the commit after `cf5e937`; ownership and assets gates PASS, 61 ledger rows, audio 2 MB, vfx/other 4.1 MB):
  - **B3 merged** (`status/B3-report.md`): `SM_PuckDisc`, `NS_puck_trail`; `NS_`/`MS_` for all eleven events including `directors_call`; `MS_skate_loop`,
    `MS_arena_room`; sound classes and a mix. MetaSound graphs and Niagara stacks were built entirely through MCP (the calls are in the report). Nobody has
    listened to the sounds yet: they are checked by recorded level and length only. Handoff **B-to-A-0007** tells A's binder and puck what to set.
  - **B5 phase 1** (`lane/b-skater`, not merged): the whole import path for a generated skater works: mesh, 29-bone skeleton (importer adds a root),
    clips, hue-key team-colour material, `ABP_SkaterTB2`, `BP_SkaterVisual_TB2` answering all eight interface calls, PIE stills. Figure 1 itself is test
    only (numbers and marks on its texture). The promotion gate could not run: nothing on `main` reads `game.skater.visual` yet.
  - **Skater lab**: S1 three more Meshy styles (pick: big-head mascot; orange refine has different UVs, so team colour is a hue-key in the material);
    S2 clips for nine needs (generated strides do not loop: 53 to 85 degrees seam, 1.8 to 6 m drift; wrist shot and check unusable; a Blender script now
    strips, trims and loops); S3 **mesh in Hyper3D Rodin, rig in Meshy works** (0.5 Rodin credit, same 28 bones; the lead's first pick) plus seven Rodin set
    props for dressing pass 2 (`briefs/B2b-props-rodin.md`).
- lanes now: B5 phase 2 (the Rodin skater through the proven path, editor 8002, box 13:15); S2 follow-up (engine-ready clip sets for the Rodin skater and
  the mascot, 12:15); S4 scripted original skater (13:15); V1 visual-upgrade plan (12:45); B2b prop pass starts on port 8000 when the main tree has rebuilt.
- credits: Meshy balance 860 at 11:07 (1204 at the start; the lab's caps total 375); Rodin 415 of 421.
- next: merge `lane/b-skater` when phase 2 reports; verify the look on A's rink in `Rink` (handoff 0005 acceptance) and do the lighting pass; first package on B.
- blocked: none. Operator: Hyper3D licence wording for shipping Rodin output, and the prize criteria.
- lfs size: about 7 MB from B
- engine build: 5.9.0 CL 60351404
- packaged build age: none yet on B (A's Shipping baseline: 322.5 MB zip)
- packaged size: none yet on B
- ledger: 30 rows from lane B3

## H6.6 Both skater visuals in the engine; look fixed on the real rink; props in

- done since H5.3 (all pushed; `dev-b` at `3291d19`; ownership gate PASS; ledger 132 rows, art 17.3 MB of 350, audio 1.8 MB):
  - **Look on the real rink** (B1b, `status/B1b-report.md`): ice between the pools 1 -> 47 of 255 through `IceAmbient` in `M_Ice`, pools unchanged; blue-line
    blotch fixed; net mesh darkened; `PPV_L2_Night` on screen-space reflections (lighting pass about 8 -> 1.3 ms on B, unconfirmed, two editors up); handoff 0005
    accepted on A's meshes. Handoff **0009** lists five look defects on A's side (skater Emissive reset, two sky lights, coplanar floors, puck size, line positions).
  - **Dressing pass 2** (B2b): seven Hyper3D Rodin props imported base-colour-only at 1K with Nanite; 188.7k triangles of dressing; blue lines at a third of the half-length.
  - **Skater lab closed out**: S4 a Blender-scripted original skater with 16 exact-looping clips; S5 the same clips retargeted onto the Rodin mesh ("the hybrid");
    S6 the Rodin texture cleaned of lace-like marks. **T-B2 is now the hybrid** and **T-B1 the scripted original**, both behind `IJamSkaterVisual` with the same AnimBP
    structure (idle, glide-to-stride by speed, leans and crossovers by turn rate, slap shot and check montages, falls both ways with their get-ups), verified in PIE
    with stills (`devlog/shots/*-B-tb1-*`, `*-B-tb2h-*`). Finding for anyone exporting FBX: Unreal 5.9's importer rejects files rewritten by the labs' track-strip
    step; the step is disabled in both lab scripts.
  - Handoffs open: 0007 (wiring effects, sounds, puck kit), 0008 (T-B1 redefined; who reads `game.skater.visual`), 0009. None answered yet (A's last merge was 11:05).
- lanes now (two editors): B2c stage light on the real rig plus plan test 2 on scratch (port 8000, 13:15); B6s skater polish (mitt colour key, one boot mark)
  and a **real-match preview**: both visuals attached to A's pawns in PIE from Python, stills and a short clip (port 8002, 13:50).
- next: merge `main` (A is at `4043884`: DLSS plugin, barks) when the main editor is free, rebuild; first package on B; plan tests 2 and 3 results into
  `briefs/B7-visual-upgrade-plan.md`; the 15:00 merge window.
- blocked: the promotion rule for either skater track waits for A to consume `game.skater.visual` (0008). Operator: Hyper3D licence wording for shipping Rodin
  output (T-B2's mesh and seven props depend on it; T-B1 and the basic-shape dressing do not); `C:\r` (eight stray PNGs from a lab lane) to delete by hand.
- credits: Meshy 806 at 11:20 (lab spent about 400 of the 1204), Rodin 415 of 421.
- lfs size: about 30 MB from B
- engine build: 5.9.0 CL 60351404
- packaged build age: none yet on B
- packaged size: none yet on B
- ledger: reconciled with disk at `3291d19` (T-B2 rows replaced by the hybrid's, T-B1 rows `shipped` n until the ruling)
