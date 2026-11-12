# Device A status (append-only, `## H<n>` blocks; H0 = 2026-11-12 06:00 local)

## pre-jam
- 2026-11-11 20:30 to 11-12 02:00: plan, tooling, four experiment lanes, tag `pre-jam` at `88d1e77`. Machine rebooted 02:33; the 06:00 scheduled start never ran (interactive logon required).

## H4 (10:05)
- done: operator awake; theme received (Director's Cut, `docs/THEME.md`); `GAME.md`; rink stays 120 x 60 by decision; lanes A0 (skeleton, seams, contracts; Opus) and A4 (cartridge schema, presets; Sonnet) running.
- next: A1 core sim, A2 AI, A5 camera/HUD start when A0's skeleton compiles; A6 packages at H6; device B starts on `docs/DEVICE-B.md`.
- blocked: nothing.
- machine: C: 445 GB free; NvRTX 5.8 build died at 7110/9110 in the reboot, not resumed (would compete for CPU); no editor running yet.

## H4.3 (10:20, approx.)
- done: A0 skeleton (build 56 s, MCP 40 s, Development package 50 s, runs); A4 schema + 5 presets (88 tests). Contracts C-0001..C-0004 and `contracts/RULINGS.md` R-0001. Pushed for device B.
- next: A1 core sim, A2 AI + tournaments, A5 camera/HUD/toy figure running in parallel (ports 8001-8003); A4 resumed to add `game.skater.visual`.
- blocked: nothing. ToolsetRegistry ships some runtime code (cannot be editor-only). Development package 1036 MB staged (pdb 363 MB), never ships.

## H4.5 (10:30, approx.)
- done: merged `dev-b` (first window; status/B.md, five handoffs, ledger rows); answered B-to-A-0001..0005 (R-0002); ARM64 .NET fix in tools/run; ownership: briefs any_device, GAME.md director, Placeholders A. Lanes A6 (QA/package/dev log) and A7 (Designer) added; five builders live on A (43.7 GB RAM free, CPU 43% before they started).
- next: lane reports as they land; H6 (12:00) Shipping baseline from A6; merge windows every 3 h from 12:00.
- blocked: nothing. Device B: ARM64, 64 GB unified, caps lowered to 4 builders / 2 editors (R-0002 point 6).

## H4.8 (10:47)
- done: A1 core on main (`432791b`): sim, loader with Node parity, rink with stencils/trim/beacons, game mode; scripted human 3:0 in PIE. A5 on main (`47a1f54`): camera, toy figure, HUD, radar, input, effects binder; seven tests. Cross-lane break (JamDesigner HTTP dep) fixed by the Director.
- next: A2 must move BotRefSim out of Tests/ so the Game target links; then the Development package proves M0. A6 Shipping baseline pending the same link. A7 Designer in progress.
- blocked: Game target link (A2). Gate 1 = M0 passes on tests and PIE; the packaged proof waits on the link.

## H4 package
- config: Shipping, Hokej_Hokej_H4.zip, verdict pass
- staged_mb: 465.9
- zip_mb: 322.5
- cook_s: 18.92 (whole package 72.4 s)
- run: stayed up 10 s

## H6 package
- lane: A6, Shipping baseline of the real project (Game/ThirdJam.uproject, -Config Shipping -Zip -Name Hokej_Hokej, then run-packaged -Seconds 10)
- first try (10:36): FAIL after 54 s, link error LNK2001 for five JamAI::FRefSim members from JamTournamentCommandlet.cpp (lane A2's files were mid-edit; BotRefSim.cpp was being rewritten). Retried at 10:57 after A2's edit: PASS.
- staged_mb: 465.9 (Development package was 1036 MB with 363 MB of pdb; Shipping has none)
- zip_mb: 322.5 (zip step 13 s); limit 1000, warn line 900: PASS with 677 MB spare
- time: whole package 72.4 s (compile of 25 actions, cook 18.9 s, stage and pak, 13 s zip), on a machine with four lanes compiling
- run-packaged: stayed up 10.7 s (ThirdJam.exe and ThirdJam-Win64-Shipping.exe), stopped
- ten largest (MB, package.ps1 order): ThirdJam\ 407.1; ThirdJam\Content\Paks\ 212.8; ThirdJam\Content\ 212.8; ThirdJam\Content\Paks\ThirdJam-Windows.ucas 198.1; ThirdJam\Binaries\ 194.3; ThirdJam\Binaries\Win64\ 194.3; ThirdJam\Binaries\Win64\ThirdJam-Win64-Shipping.exe 166.6; Engine\ 58.4; Engine\Extras\Redist\ 28.9; Engine\Extras\Redist\en-us\ 28.9
- reading: the engine slice is 166.6 MB exe + 58.4 MB redist (prep measured 316 MB zip on an empty map); the 198 MB ucas is the cooked content of the current skeleton (engine content plus Rink), not yet game art. The 230 MB reserve of plan 4b is untouched.
- note: the schedule script's own test round wrote the block "H4 package" above (hour from the clock); the numbers are the same package family, this block is the baseline of record.
- artifacts: zip kept at C:\Users\kazda\AppData\Local\Temp\a6pkg2\Hokej_Hokej_H4.zip (scratch, not in the repo)

## H5 (11:05)
- done: A1 live Director's Call + takes + package from pak (`433f624`); A6 Shipping baseline 322.5 MB zip, premerge gate, schedule, dev log, SUBMISSION.md (`083a265`); dev-b merged early (`0bf3047`: painted-cel masters, LR_L2_Night, arena dressing); B-to-A-0006 accepted (R-0005). CLI transport probed (haiku via `claude -p`, 3 s).
- next: lanes A3 (difficulties), A7 (CLI transport + 30 offline cuts), A8 (menu), A9 (multiplayer proof), A10 (feel QA) running; next merge window 14:00 (H8), Shipping package at H6 via schedule.
- blocked: nothing. Editor target transiently red while A3 edits the brain (R-0003 point 5 applies).
- machine: 46 GB free, CPU low between builds; editors up on A: lanes A8/A9/A10 as they start.

## H6 (12:05)
- done since H5: A7 CLI transport + 30-cut library (twice: regular changed to 90 x 45 by R-0010); A10 feel QA (cartridge rejection, GPU hang on L2, puck cannot be carried; proposals routed); A5 feel fixes (brake, hints, own-net marker, camera clamp); A9 multiplayer holds and is built (JamNet, lockstep, invite code); A3 interim curve rookie 85 / pro 50 / all_star 25 on 120 x 60, presets switched to the new regular; rulings R-0006..R-0010 (NVIDIA DLSS path, ACE/barks side track, Codex images, link windows, rink 90 x 45).
- gate 12:01 (-SkipBuild): ownership PASS after adding Barks/JamNet/Designer; ledger PASS; node tests 141/141; automation 34 passed, 4 failed (A3's PlayerBotWins re-targeting, A1's LiveConfig to check, A8's two menu tests mid-work); dev log PASS.
- running: A3 sweep + presets, A8 menu tour, A11 DLSS 5.9 rebuild, A12 barks; B: B2 dressing pass 2, B3 effects, B5 Meshy skater, handoff 0003 (GPU hang) open.
- blocked: nothing on A. Risk: the shipped night rig hangs the GPU until B answers 0003; L1 day is the fallback tier.
- machine: NvRTX 5.8 built (276 GB), idle; editors on A observe link windows at :00 and :30.

## H6 package
- config: Shipping, Hokej_Hokej_H6.zip, verdict pass
- staged_mb: 548
- zip_mb: 376.2 (394472063 bytes)
- zip_sha256: 39b92e1986085954215b0fc9df6cc29924e6ff9839b426429643e23b40cf3b4b
- cook_s: 13.39 (whole package 44.2 s)
- run: stayed up 10 s
- largest: ThirdJam 489.3 MB; ThirdJam\Content\Paks 233.7 MB; ThirdJam\Content 233.7 MB; ThirdJam\Content\Paks\ThirdJam-Windows.ucas 218.9 MB; ThirdJam\Binaries 199.4 MB
