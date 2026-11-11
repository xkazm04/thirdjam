# Prep checklist (P6): disk, accounts, devices

Owner: the operator for anything that needs a login or a sign-up (agents do not create accounts, accept terms or enter credentials).
Status date: 2026-11-11 evening CET. Tick boxes in place.

## A. Must be done before Thu 06:00 CET

- [ ] **Registered for the jam** on the itch.io page (https://itch.io/jam/2026-epic-ai-megajam) with the Epic account. Entry is by registration; the submission later needs the team name, member name and the email of the Epic account.
- [ ] **Decide the team name** (the zip is named `TeamName_GameName`, English only, no trademarks).
- [ ] **Disk space on device A**: C: is the only drive and has about 73 GB free of 1.9 TB (97% used). Target at least 150 GB free before H0. A game project with Lumen and Nanite content, its derived data cache, three or four packaged builds and generated or bought assets takes tens of GB. Candidates to clear are the operator's call (old engine versions, old derived data caches, other build output). I will list the largest folders on request; I will not delete anything without a named target.
- [ ] **Device B** ready: same promoted UE 5.9.0 build as A (CL 60351404, check `Engine/Build/Build.version`), Visual Studio 2022 with the Game development with C++ workload, Git and Git LFS, Node 24, Claude Code, at least 150 GB free (more if NvRTX is tried), GPU noted here: ______.
- [x] **GitHub repo for `thirdjam`**: private, created 2026-11-12 ~01:15 CET with the gh CLI, `main` pushed (`origin` set). Git LFS 3.4.0 is installed on A; patterns in `.gitattributes`. Device B: `git lfs install` once, then `git clone https://github.com/xkazm04/thirdjam` and `git checkout -b dev-b`. Size budget about 8 GB.
- [ ] **GitHub account linked to the Epic account** in the Unreal dashboard (needed for Unreal source and NvRTX access, and for the Epic MCP/plugin sources if any need it).
- [ ] **`ANTHROPIC_API_KEY`** available in the environment of device A for the live Designer eval (not stored in any file; the game never bundles a key). Partner credits claimed if you want them for in-jam agent use.
- [ ] **Partner perks** you intend to use are claimed and tested once: Meshy, Tripo, Hyper3D Rodin (needed for the Hyper3D pool: list it under tools used), fal, PBRgen. WeLoveIndies audio is non-commercial only: note that in the assets ledger if used, because a development grant would turn the project commercial.
- [ ] **Approve the `unreal-mcp` server once in Claude Code** (operator-only, interactive): with an editor running and a `.mcp.json` from `tools/mcp/mcp.json.template` in the project folder, start `claude` there and accept the project MCP server prompt; `claude mcp list` then shows it connected instead of "Pending approval". Agents cannot click this. Do it on both devices during the first 15 minutes after H0 (the project folder does not exist before).
- [ ] **Skater sources** (from `docs/SKATERS-RESEARCH.md`, the "before H0" list): favourite the Fab pack "Hockey Player Animated" (Epic skeleton, 14 skating clips + shot, about USD 19; confirm its licence and 5.9 support on the listing) for track T-B1; a Meshy account for T-B2 (its rigging can target the UE5 mannequin skeleton, vendor claim); Rodin only if the Hyper3D pool matters (mesh in Rodin, rig in Meshy). Nothing is downloaded before H0.
- [ ] **Jam Discord** joined (NVIDIA and partner prize criteria are asked there). Epic's daily livestream is at 20:00 CET Thursday to Sunday.
- [ ] **Sleep plan** written for the 93 hours: human attention windows are in PLAN.md section 3.
- [ ] **Art ladder for hockey**: nine stills (L1 day / L2 night / L3 neon, three variants each) in `prep-reference/HOCKEY-LADDER.md` and `LOOK-STUDIES.html`; pick one variant per tier (a, b or c) so the arena climbs the ladder as one place. The pick sets device B's first dressing list at H1.
- [ ] **Art direction (generic studies, superseded by the hockey ladder)**: open `prep-reference/LOOK-STUDIES.html` in a browser (four directions, 11 stills, feasibility, scores), rate each as base / layer / fallback / no, and paste the generated line here. Reviewer's recommendation: D1 toy diorama as the base, D4 tilt-shift as the layer; D3 neon if the theme turns dark. If left open, the hour-1 rubric decides.

## B. Verified by agents tonight (see the reports)

- [x] P1 MCP smoke test (2026-11-11 21:11-21:37 CET): C++ project built in 65 s (MSVC 14.44, SDK 26100), editor to MCP-ready 44 s cold / 11 s warm, spawn + read-back + 1280x720 screenshot + automation test + console all worked; 106 toolsets, 1871 tools (`tools/mcp/TOOLS.md`); empty map packaged Shipping 316 MB zip / Development 484 MB zip, both ran 10 s. Smoke folder deleted.
- [x] P2 scripts: `tools/run/{common,build,editor,package,smoke,run-packaged}.ps1`; watchdog restart proven; `smoke.ps1` passes in 55 s. From Git Bash set `MSYS_NO_PATHCONV=1` (path rewriting broke `-Map /Engine/...` once).
- [x] P3 schema, validator (61 tests), Designer prompts, mock eval pass. [ ] One live eval run with a key (needs `ANTHROPIC_API_KEY` in the shell).
- [x] P4 dev log builder (45 tests with P7); fixture page viewed in Chrome; redaction proven.
- [x] P5 11 look-study images, declared reference-only in `prep-reference/README.md`.
- [x] P7 ownership check, assets ledger check with size budgets, handoff/status/contracts templates, `docs/GATES.md`.

## C. Not before 06:00 CET

No game project, level, mesh, texture, material, audio, animation or cartridge for the real game; no Fab download that goes into the game
(reading licences and browsing is fine); no pre-made gameplay code. At 05:59 CET: commit everything in this repo, `git tag pre-jam`, push.
The dev log's first entry at H0 lists exactly what existed before the start (the tooling above, the schema envelope, the prompts, the reference images).

## D. First 15 minutes after 06:00 (so nobody improvises)

1. Watch or read the theme again; paste it into `docs/THEME.md` (copy the exact wording).
2. Run `tools/run/smoke.ps1` on both devices; both must pass before anything else.
3. Start the theme-binding rubric (PLAN.md section 6), one hour cap.
4. Create the real project from a blank C++ template in `Game/`, commit, push, B clones.
