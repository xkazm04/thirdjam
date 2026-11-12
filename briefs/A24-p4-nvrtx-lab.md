# Lane A24: P4 broadcast tech lab in the NvRTX build (device A, from H9.3)

You are lane A24 of the Third Jam (team Hokej). The operator asked for tech experiments toward P4 (stylized broadcast) while the machine is idle. An NvRTX 5.8 engine build exists at
`C:\Users\kazda\kiro\nvrtx-5.8` (276 GB, built last night; `docs/NVRTX-NOTES.md` and `docs/experiments/DLSS.md` say what was proven and what was not). The shipped game stays on the
launcher 5.9 engine; this lab decides whether any NvRTX feature is worth a trailer or an optional "broadcast" tier, and what it costs.
Read first: `docs/NVRTX-NOTES.md`, `docs/experiments/{DLSS,ACE,LOOK-P2-NIGHT}.md`, `docs/HOCKEY.md` (P4 bar: "regular hockey works and looks broadcast-like"), `contracts/look-tiers.md`,
R-0006 (NVIDIA track), R-0021 point 4, `tools/run/common.ps1` (how the engine root is found: `UE_ROOT`), `tools/dlss/fetch-dlss.ps1`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a24-nvrtx` (detached at 6a2681e). The project targets 5.9; the NvRTX build is 5.8: first find out whether the
project opens in it at all (a copy of the worktree's `Game/` with `EngineAssociation` switched, plugins that 5.9 has and 5.8 does not disabled in the copy). If it does not open within
45 minutes, stop, write why in `docs/experiments/NVRTX-LAB.md`, and report; that is a valid result. Never build inside `nvrtx-5.8` itself; never touch `Game/Plugins/JamAssets`.
One editor, MCP port 8012, stopped whenever you are not measuring; before starting it check `tasklist` for `UnrealEditor.exe` and wait if three run. Run no git command.

## Job (if the project opens)
1. The rink map in the NvRTX editor: RTXDI on the twelve spots, ReSTIR GI vs Lumen, ray-traced reflections on the ice, DLSS Ray Reconstruction on vs off; stills at 10 / 24 / 40 m and
   `ProfileGPU` per variant at 1440p on the 4090 (table: ms per feature).
2. Does the painted-cel post (toon ramp, outline) survive the denoiser? Stills.
3. Capture a 20-second clip of a bot match with the best combination (ffmpeg from a sequence or the in-editor movie capture) for the trailer question; size and frame rate noted.
4. Write `docs/experiments/NVRTX-LAB.md`: what opened, what did not, the cost table, three recommendations (ship nothing / trailer only / optional tier) with the submission-form text
   for "tools used" if any NvRTX feature stays.

Status in `status/A24.md` (main tree) with real `date` timestamps. Report: the open/no-open result, the cost table, the clip path, the recommendation. Time box 2 hours.
