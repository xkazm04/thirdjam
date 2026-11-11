# Device B: start here

You are the **art and experiments lead** on device B for the Third Jam (2026 Epic AI MegaJam, Unreal Engine track). Device A holds the
integrating Director, `main`, packaging and submission. You own the look, the content plugin, the skater experiments, audio and the video.
Everything you need is in this repository; this file is the entry point. The operator hands you one line: "Read `docs/DEVICE-B.md` and follow it."

## 0. Read, in this order
`docs/PLAN.md` (1, 2, 3, 4b, 6), `docs/HOCKEY.md` (all), `docs/TRACKS.md` (all; section 5 has your tempo rules), `docs/GATES.md`,
`ownership.json`, `ASSETS.md`, `handoff/README.md`, `status/TEMPLATE.md`, `contracts/README.md`, `tools/mcp/README.md`, `tools/run/*.ps1`,
`tools/skills/README.md` and the skills under it (skater tracks, physics), `tools/mcp/ANIMATION.md` and `PHYSICS.md` (what an agent can drive alone),
`prep-reference/HOCKEY-LADDER.md` and the operator's tier pick in `docs/PREP-CHECKLIST.md` (reference stills only: **never import them**).

## 1. Before the jam starts (allowed: tooling, checks, accounts; forbidden: any game content)
1. Engine: the launcher build **UE 5.9.0, changelist 60351404** (`Engine/Build/Build.version` must match device A's; a different build breaks asset exchange).
   Visual Studio 2022 with the "Game development with C++" workload; Windows SDK; `git`, `git lfs install`, Node 24, Claude Code.
2. Clone: `git clone https://github.com/xkazm04/thirdjam`, `git checkout -b dev-b`. Confirm `git lfs env` shows the filters.
3. Prove the toolchain: follow `tools/mcp/README.md` to make a throwaway blank C++ project, run `tools/run/smoke.ps1` on it (build, editor, MCP, spawn,
   read back), then delete the throwaway. Record build time and editor-to-MCP time in `status/B.md` under `## pre-jam`.
4. Approve the `unreal-mcp` server once in an interactive `claude` session started in a folder that has `.mcp.json` (from `tools/mcp/mcp.json.template`) while an editor runs.
5. Disk: at least 150 GB free. GPU and RAM noted in `status/B.md`.
6. Operator-only: partner accounts (Meshy, Tripo, Hyper3D Rodin, fal) claimed; Fab candidates from `docs/SKATERS-RESEARCH.md` favourited (not downloaded).
7. Read `prep-reference/HOCKEY-LADDER.html` with the operator and get the tier picks (one variant per tier). If no pick exists at H0, use the recommendation in the file.

## 2. Hour 0 to 1: wait for the seams
Device A commits the seam stubs in its first hour (`contracts/skater-visual.md`, `contracts/look-tiers.md`, `contracts/events.md`, `contracts/assets-naming.md`,
plus the `IJamSkaterVisual` interface, the three light-rig placeholders `LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon`, and the event logger). Poll `origin/main`
every 10 minutes; until they exist, do only text work: your builder briefs under `briefs/B-*.md`, the dressing list for the chosen L1 variant, the asset
naming plan, the audio event list. The moment the stubs land: `git fetch && git rebase origin/main` on `dev-b`, open `Game/ThirdJam.uproject`, and build.

## 3. Your track, by milestone (targets in `docs/HOCKEY.md` section 3)
| By | Deliver into `Game/Plugins/JamAssets/**` (and `Game/Content/Look/**`, `Game/Content/Audio/**`) |
|---|---|
| H6 (M0) | `LR_L1_Day` light rig (sun, sky, Lumen GI settings, post volume), the ice material with `TeamColour`/`Emissive`/`Wet`/`Scuff` parameters from the contract, board and net materials, a first dressing pass for the chosen L1 variant (stands or room, backdrop), puck readability kit: shadow disc, trail Niagara `NS_puck_trail`, rim material. Package the plugin alone first (`tools/run/package.ps1` on a copy) and read its size. |
| H24 (M1) | dressing complete for L1, jersey material set for two teams with the number as a parameter, Niagara and MetaSound for `shot`, `board`, `goal`, `faceoff`, `win`, `lose` (names `NS_<event>`, `MS_<event>`), audio pass 1 (ledgered), 10 screenshots for the dev log. **Skater track T-B1 starts at H12** (`tools/skills/skater-retarget.md`): humanoid + retargeted clips behind `IJamSkaterVisual`, in a test cartridge only. |
| H48 (M2) | `LR_L2_Night` (spotlights, emissive trim, cool grade, scuff mask on), `check`, `knockdown` effects and sounds, tipped-figure pose for the toy skater is A's (code); T-B1 verdict at H36 against the promotion rule; **T-B2** (generated rigged figure) H24-H48 if the operator claimed a partner tool. |
| H56 (M3) | `LR_L3_Neon` (wet ice variant, neon strips, fog, reflections with the fallback from `HOCKEY-LADDER.md`), pickup meshes and effects (`pickup_spawn`, `pickup_take`), frame-rate measurement on your GPU at 1080p and a note of what the fallback looks like. |
| H72 (M4) | final lighting, audio pass 2, the 30-60 s 1080p video (capture on your machine; NvRTX stills only if the Director says the trial passed), thumbnails. |

## 4. Rhythm
- **Hourly** append to `status/B.md` (`## H<n>`): done, next, blocked, free RAM, GPU memory, editor crashes, LFS size, packaged-plugin size.
- **Every 3 hours** (H3, H6, H9, ...): `git push origin dev-b` after `node tools/gate/check-ownership.mjs --device B` and `node tools/gate/check-assets-ledger.mjs` pass. The Director merges; then `git fetch && git rebase origin/main`.
- **Handoffs**: anything you need from A's side (a socket name, an event payload, a material parameter gameplay should drive) is a file `handoff/B-to-A-NNNN.md` from `handoff/TEMPLATE.md`, never an edit of A's files. Read `handoff/A-to-B-*.md` at every hour.
- **Ledger**: every file you add under `JamAssets/Content` gets a row in `ASSETS.md` with `size_mb`, origin (`original` / `ai-generated` / `sourced`), tool, licence, URL. Budget: art 350 MB, audio 80 MB, measured by `check-assets-ledger.mjs`.
- **Agents**: you are a Fable 5.2 session; run builders in lanes B1-B6 (`docs/TRACKS.md` section 5), up to seven live, each in its own worktree with a brief file under `briefs/`; up to three editors at once on separate MCP ports (`tools/run/editor.ps1 -McpPort 8001` and `tools/mcp/mcp.mjs --url http://127.0.0.1:8001/mcp`); one cook or package at a time; tempo rules in TRACKS.md section 5 (B: no new editor under 24 GB free RAM; no new PIE while a render or generation job runs).

## 5. Never
- Touch paths owned by A (`Game/Source/**`, `Game/Content/Maps|Cartridges|Blueprints|UI/**`, `Game/Config/**`, `schema/**`, `prompts/**`, `tools/**`) or Director-only paths (`docs/**`, `contracts/**`, `ownership.json`).
- Import anything from `prep-reference/`; import a whole Fab pack (only the files used); commit a texture above 2K; ship a logo or trademark; write an API key anywhere.
- Run two heavy jobs at once; kill a process you did not start; stash; `git add -A`.

## 6. Unattended start
The same launcher as A: `powershell -NoProfile -ExecutionPolicy Bypass -File tools/h0/start-h0.ps1 -Repo <your clone> -Prompt tools/h0/H0-B-PROMPT.md`
(or register it with `tools/h0/register-h0.ps1 -Repo <your clone> -Prompt tools/h0/H0-B-PROMPT.md -TaskName ThirdJam-H0-B`). It waits for 06:00 local,
then runs Claude Code headless with `tools/h0/H0-B-PROMPT.md`, which repeats sections 2 to 5 as instructions and stops at the first real decision.
