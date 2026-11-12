# Lane A11: NVIDIA DLSS in the shipped build (device A, from H5.2)

You are lane A11 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Read first: `contracts/RULINGS.md` R-0006 (the NVIDIA track requirement and the eligibility path),
`docs/PLAN.md` 2 and 4b (size budget: the plugin's binaries count), `docs/SUBMISSION.md`, `ASSETS.md` (ledger rules), `Game/ThirdJam.uproject`, `Game/Config/DefaultEngine.ini`,
`briefs/A8-main-menu.md` (lane A8 owns Settings; you hand it the DLSS quality switch API), `tools/run/*.ps1`, `tools/mcp/README.md`.

You own: `docs/experiments/DLSS.md`, `Game/Plugins/DLSS*/**` (once approved), `Game/Source/JamGame/Dlss*` (a small `UJamDlssSettings` helper), `Game/Config/DefaultEngine.ini` **only** the
DLSS lines (say what you added). Run no git command; editor on MCP port **8011**; never kill a process you did not start; one heavy job at a time.

## Phase 1: research and STOP before downloading (30 minutes)
Find NVIDIA's official DLSS plugin for Unreal Engine that supports **5.9** (the "DLSS 4" / "DLSS 4.5" Unreal plugin from NVIDIA's developer site, or its Fab listing): the exact download
(URL, file name, size), whether it needs an NVIDIA developer login, the licence/EULA terms that matter for a jam build (redistribution of the DLSS DLLs inside a packaged game, attribution,
any "evaluation only" clause), which features the 5.9 build offers (Super Resolution, Ray Reconstruction, Frame Generation, DLAA), the minimum driver, and what happens on non-NVIDIA GPUs
(it must no-op). Write `docs/experiments/DLSS.md` section 1 with the sources and the dates seen, then **report and wait**: the Director asks the operator to approve the download. Do not download.

## Phase 2: integrate (after approval; the Director tells you the file is in place or lets you fetch it)
1. Install the plugin into `Game/Plugins/`, enable it in the `.uproject`, build the editor target, confirm it loads (`DLSS` in the editor log), and that the RTX 4090 here reports DLSS supported.
2. `UJamDlssSettings` (JamGame): `IsSupported()`, `SetMode(auto | quality | balanced | performance | dlaa | off)` applying the plugin's console variables/Blueprint library, persisted in
   `GameUserSettings`; default `auto` = on at the plugin's recommended quality when supported, `off` otherwise. Expose it for lane A8's Settings screen (name the calls in your report).
3. Prove it in PIE and in a **Shipping package**: a screenshot with DLSS on and off, `stat gpu` GPU ms both ways at 1080p and at 1440p, the packaged exe running 10 s with DLSS on,
   the staged size delta the plugin adds (must stay inside the 1 GB budget; report it), and the behaviour with the plugin forced unsupported (console variable or the plugin's test switch).
4. Ledger: one `ASSETS.md` row for the plugin (`sourced`, licence named, URL, size) and a line in `docs/SUBMISSION.md` "Tools used" and the NVIDIA questions draft in `DLSS.md` section 3.

Report: Phase 1 findings first (and wait), then Phase 2 numbers, every error. Time box 2.5 hours total.
