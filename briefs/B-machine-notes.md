# Device B machine notes: every lane reads this before it touches the engine

Written by the device B lead at H4.6 (2026-11-12 10:40) from what was measured on this machine. It overrides the device A numbers and habits in the
recipes where they differ.

## The machine

Windows 11 on ARM64 (NVIDIA N1X), 64 GB memory shared between CPU and GPU. The Unreal editor and the whole toolchain are x64 binaries running under
emulation: they work, about 1.3 to 2 times slower than device A's numbers. Use generous timeouts (`--timeout 600` for material or shader compiles).

## Rules that are specific to this machine

1. **.NET fix**: start every PowerShell that runs an engine script (`build.ps1`, `editor.ps1`, `package.ps1`, `smoke.ps1`) with
   `. C:\Users\kazda\jam\ue-env.ps1`. Without it UnrealBuildTool exits -2147450730 after 2 seconds with an empty log.
2. **Editors**: at most three on the machine, one per lane, each on its own MCP port and its own project copy.
   - main tree `C:\Users\kazda\jam\thirdjam\Game` : port 8000 (the lead's editor; lanes B1 and B2 work here in turn)
   - `C:\Users\kazda\jam\wt\b-vfx\Game` : port 8001
   - `C:\Users\kazda\jam\wt\b-skater\Game` : port 8002
   Start: `powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\editor.ps1 -Project <tree>\Game\ThirdJam.uproject -WaitMcp -McpPort <port> -McpTimeoutSec 500`
   (run from the tree; the editor targets are already built in all three trees). Client: `node tools/mcp/mcp.mjs <cmd> --url http://127.0.0.1:<port>/mcp`.
   Before starting an editor, check free memory: under 24 GB free, do not start one; report instead.
3. **After MCP first answers the editor still refuses tools for 5 to 10 seconds** (`Not run: the Unreal Editor is running a slow task`). Retry `toolsets`
   every 5 s until it passes before the first real call. `tools/run/smoke.ps1` does not retry and reports FAIL here although nothing is wrong: do not use it.
4. **Stopping an editor**: save, then call `EditorToolset.EditorAppToolset.ExitEditor {}` through MCP (full name; it exits cleanly and frees the port at once). `editor.ps1 -Stop` force-stops after its grace period, and then the
   port stays LISTENING (owner pid 0) for about 100 s and `editor.ps1` refuses to start on it (exit 2). If that happens, wait for
   `netstat -ano | findstr :<port>` to show no LISTENING line.
5. **A modal `Wait for ZenServer?` box** can block the first editor start after a reboot (the cache server takes over 20 s to come up). The editor log
   stops at `Waiting for ZenServer to be ready...` and the window title is `Wait for ZenServer?`. Stop that editor and start it again.
6. **Python through MCP**: tool `editor_toolset.toolsets.python_scripting.PythonScriptingTools.execute_python`, args
   `{"script":{"content":"<python>","language":"Python"}}` written to a file with Node (`JSON.stringify`) and passed as `@<file>`. The last expression of
   the script comes back as `returnValue`. From Git Bash set `MSYS_NO_PATHCONV=1` when a `/Game/...` or `/JamAssets/...` path is on the command line.
7. **Screenshots that show the look** only through PIE plus `SceneCaptureToolset.SceneCaptureToolset.CaptureGameViewport` (the other capture tools skip the
   toon ramp and the outline). The 1920 x 1080 PIE window captures at 1920 x 1085. File stills with `node devlog/shot.mjs B <label> <file> "<caption>"`.
8. **Lessons from lane B1**: adding a map as a streaming sub-level makes it the *current* level, so whatever you spawn next lands in it and is saved with
   it: set the current level back, or check the actor list of every map before you save it. Deleting a map through Python reported success and did
   nothing: delete the file on disk, then rescan. Nothing may remain under `Game/Plugins/JamAssets/Content/_Scratch/` when you report (the ledger gate counts
   every file under the plugin's `Content/`). The engine has no basic capsule shape (cube, sphere, cylinder, cone, plane only).
9. **One heavy job at a time on the machine**: a build, a cook or package, a Meshy import, a long capture. Editors may run side by side; PIE frame
   numbers taken while another editor renders are noisy: say so next to the number.
10. Frame numbers measured here are device B numbers. The gate numbers are device A's (RTX 4090).

## What exists already (all under `/JamAssets/`, repo path `Game/Plugins/JamAssets/Content/`)

- `Look/`: `M_Toon`, `M_Trim`, `M_Ice`, `M_PP_ToonRamp`, `M_PP_Outline`; instances `MI_Toon_Blue`, `_Orange`, `_Board`, `_Helmet`, `_Stick`, `_NetFrame`,
  `_NetMesh`, `_Puck`, `_Dark`; `MI_Trim_Blue`, `_Orange`, `_PuckRim`, `_PuckHalo`, `_Beacon`; `MI_Ice_Night`; `MI_PP_ToonRamp`, `MI_PP_Outline`.
  Parameters: `TeamColour` (vector), `Emissive`, `Roughness`, `RimStrength`, `RimPower` on `M_Toon`; `TeamColour`, `Emissive`, `BaseTint` on `M_Trim`;
  `Scuff`, `Wet`, `RinkHalfLength`, `RinkHalfWidth` on `M_Ice`.
- `Rigs/LR_L2_Night`: `PPV_L2_Night`, `Sky_L2_Night`, `Spot_L2_Night_1..4` (x = -45, -15, 15, 45 m at 25 m), `Room_L2_Night_1..6` (inner faces at +-82 m
  and +-47 m, ceiling 28 m). Exposure is fixed at EV100 4.2: emissive values assume it.
- Stencil convention (custom depth): 0 room and emissive parts, 1 figures/boards/nets (ramp + outline), 2 ice (ramp only), 3 puck (outline only).
- The persistent map `/Game/Maps/Rink` is device A's: never save it. It has a temporary 120 x 60 m floor plane; A's rink, skaters and puck are not in yet.

## Always

Run no git command unless your lane prompt allows commits on your lane branch. Never edit `tools/**`, `docs/**`, `contracts/**`, `Game/Source/**`,
`Game/Config/**`, `Game/Content/**`, `ASSETS.md` (ledger rows go into your report; the lead pastes them). Never kill a process you did not start. Download and
install nothing. Import nothing from `prep-reference/`. No text, letters, numbers, logos or brand shapes in any asset. Textures 2K at most.
