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
