# Device A status (append-only, `## H<n>` blocks; H0 = 2026-11-12 06:00 local)

## pre-jam
- 2026-11-11 20:30 to 11-12 02:00: plan, tooling, four experiment lanes, tag `pre-jam` at `88d1e77`. Machine rebooted 02:33; the 06:00 scheduled start never ran (interactive logon required).

## H4 (10:05)
- done: operator awake; theme received (Director's Cut, `docs/THEME.md`); `GAME.md`; rink stays 120 x 60 by decision; lanes A0 (skeleton, seams, contracts; Opus) and A4 (cartridge schema, presets; Sonnet) running.
- next: A1 core sim, A2 AI, A5 camera/HUD start when A0's skeleton compiles; A6 packages at H6; device B starts on `docs/DEVICE-B.md`.
- blocked: nothing.
- machine: C: 445 GB free; NvRTX 5.8 build died at 7110/9110 in the reboot, not resumed (would compete for CPU); no editor running yet.

## H5 (10:50)
- done: A0 skeleton (build 56 s, MCP 40 s, Development package 50 s, runs); A4 schema + 5 presets (88 tests). Contracts C-0001..C-0004 and `contracts/RULINGS.md` R-0001. Pushed for device B.
- next: A1 core sim, A2 AI + tournaments, A5 camera/HUD/toy figure running in parallel (ports 8001-8003); A4 resumed to add `game.skater.visual`.
- blocked: nothing. ToolsetRegistry ships some runtime code (cannot be editor-only). Development package 1036 MB staged (pdb 363 MB), never ships.

## H5.5 (11:25)
- done: merged `dev-b` (first window; status/B.md, five handoffs, ledger rows); answered B-to-A-0001..0005 (R-0002); ARM64 .NET fix in tools/run; ownership: briefs any_device, GAME.md director, Placeholders A. Lanes A6 (QA/package/dev log) and A7 (Designer) added; five builders live on A (43.7 GB RAM free, CPU 43% before they started).
- next: lane reports from ~12:20; H6 Shipping baseline from A6; merge window H9 (15:00).
- blocked: nothing. Device B: ARM64, 64 GB unified, caps lowered to 4 builders / 2 editors (R-0002 point 6).

## H6 (12:00)
- done: A1 core on main (`432791b`): sim, loader with Node parity, rink with stencils/trim/beacons, game mode; scripted human 3:0 in PIE. A5 on main (`47a1f54`): camera, toy figure, HUD, radar, input, effects binder; seven tests. Cross-lane break (JamDesigner HTTP dep) fixed by the Director.
- next: A2 must move BotRefSim out of Tests/ so the Game target links; then the Development package proves M0. A6 Shipping baseline pending the same link. A7 Designer in progress.
- blocked: Game target link (A2). Gate 1 = M0 passes on tests and PIE; the packaged proof waits on the link.
