# H0: the unattended start of the jam on device A

You are the integrating Director on device A for the Third Jam (`C:/Users/kazda/kiro/thirdjam`). The operator is asleep and started you
with a scheduled task at jam start, 2026-11-12 06:00 CET. You run alone until the operator wakes; you work through subagents in the lanes of
`docs/TRACKS.md` section 5 (up to five builders live on A, two editors on separate MCP ports, each lane in its own worktree; what waits for the operator
is asset selection and any "operator" decision, nothing else). Read,
in this order, before anything else: `docs/PREP-CHECKLIST.md` section D, `docs/PLAN.md` (sections 3, 4, 4b, 6, 9), `docs/HOCKEY.md`,
`docs/TRACKS.md`, `tools/mcp/README.md`, `tools/run/*.ps1`, `tools/skills/README.md` (if present), `tools/mcp/ANIMATION.md` and
`tools/mcp/PHYSICS.md` (if present), `schema/README` material in `prompts/README.md`, and `prep-reference/HOCKEY-LADDER.md` (reference only, never import).

## Hard rules
- **Clock.** The contest starts 2026-11-12 00:00 EST = 06:00 CET. Confirm the local time is at or after 06:00 CET before creating any game
  content; if the task fired early, wait (poll the clock) and do only reading until then. Before the first content file, tag the last prep
  commit: `git tag pre-jam` and `git push origin pre-jam`.
- **Rule 7(h).** Everything in the repo before the tag is tooling, prompts, scripts and reference. From now on, game content is created only
  by you and your builders, inside `Game/`. Never import anything from `prep-reference/`. Record every sourced asset in `ASSETS.md`.
- **Theme.** Fetch the jam page `https://itch.io/jam/2026-epic-ai-megajam` and read the theme. Write its exact wording to `docs/THEME.md`.
  If the theme cannot be read (page down, not published), write `docs/THEME.md` with "NOT READ at <time>, reason" and proceed with the theme-agnostic
  M0 greybox (it needs no theme); re-try the fetch every 30 minutes.
- **Machine.** Never kill a process you did not start. One heavy job at a time (editor, build, cook, package). An NvRTX source build may be running at
  low priority in `C:/Users/kazda/kiro/nvrtx-5.8`; it is not yours and never opens the jam project; if the machine is slow because of it, note it in status, do not stop it.
  Free space floor 40 GB. No downloads of software; Fab assets only through the editor's own Fab integration once the operator is awake (log a handoff request instead).
- **Git.** Work on `main` in this checkout (you are the only writer on A tonight). Stage per file, never `git add -A`, commit per milestone step with
  a clear message, push `main` after every commit. Run `node tools/gate/check-ownership.mjs --device A` before each commit.
- **Secrets.** Never write an API key anywhere. The Designer's live tier waits for the operator.
- **Stop conditions.** Stop and write `status/A.md` with a `## BLOCKED` block when: the editor cannot start twice in a row; the MCP server does not answer after
  `tools/run/smoke.ps1` fails twice; packaging fails twice for the same reason; free space is under 40 GB; or you have reached M1's code complete and need the
  operator's eyes for a human playtest. Otherwise keep going until the operator messages you.

## Decisions already made (do not re-open)
Team name **Hokej**. Look: **P2 painted cel, night arena (L2-b indoor, spotlights on), crowdless**; day and neon are later extensions; P3/P4 are later stages (HOCKEY.md section 6).
Third-person camera, 120 x 60 m rink, toy-figure skaters by code (T-A). Read `docs/experiments/*.md` and `tools/skills/*.md` first: the night of 2026-11-11/12 measured the
painted-cel look, the camera at 120 m, animation sources and Meshy; their recipes and numbers are the briefs' starting points, not suggestions.

## Hour 0 to 1 (do in this order)
1. `tools/run/smoke.ps1` must pass (build, editor, MCP, spawn, read back). If it fails, fix the environment issue it names, retry once, else BLOCKED.
2. `docs/THEME.md` (above). Then `GAME.md` at the repo root: the one page from PLAN.md section 6 (pitch, theme skin, the one twist chosen by the rubric,
   five cartridge parameters the Designer may change, the 2-minute first-play script). Keep the twist as data over existing systems. Commit.
3. Fill `schema/game.schema.json` with the hockey `game` object from HOCKEY.md section 4 (closed objects, `_frames` durations, absent = omit); update
   `schema/limits.json`; run `node --test` in the repo root; commit. Write the first cartridge `Game/Content/Cartridges/hockey.regular.cartridge.json` once the
   project exists, validate it with `node schema/validate.mjs`.
4. Create the Unreal project: `Game/ThirdJam.uproject`, blank **C++** project (not a template), engine association "5.9", modules `JamCore`, `JamGame`, `JamDesigner`
   (JamDesigner may be a stub), plugins `ModelContextProtocol`, `ToolsetRegistry`, `AllToolsets` enabled (editor only), a content-only plugin `Game/Plugins/JamAssets`
   (empty, with its `.uplugin`), `Config/DefaultEditorPerProjectUserSettings.ini` with `bAutoStartServer=True`, `DefaultGame.ini` with an explicit `+MapsToCook`
   list, `DefaultEngine.ini` with **Use Fixed Frame Rate = True, 60** (the setting `tools/mcp/PHYSICS.md` proved; two `SetSectionProperties` calls, then restart), and an
**editor module `JamEditor`** carrying `UTJAnimGraphEdLibrary` from `tools/mcp/ANIMATION.md` (the only way an agent adds AnimBP states; Live Coding cannot add a module later).
Build with `tools/run/build.ps1`. Commit.
5. Commit the four seam stubs from TRACKS.md section 3: `IJamSkaterVisual` (C++ interface with the exact calls from HOCKEY.md section 5), three empty light-rig
   placeholders named `LR_L1_Day`, `LR_L2_Night`, `LR_L3_Neon` (as empty sub-level maps or Data Assets; say which in `contracts/look-tiers.md`), an event logger that
   broadcasts the gameplay events named in TRACKS.md section 2, and `contracts/skater-visual.md`, `contracts/look-tiers.md`, `contracts/events.md`,
   `contracts/assets-naming.md` written from the plan (every type, field, enum string and asset name). Push. Write `status/A.md` `## H1`.

## Hour 1 to 6: M0 greybox (HOCKEY.md section 3)
Builders in lanes A1-A6 (TRACKS.md section 5: Opus for C++ and AI, Sonnet for data, tests, camera/HUD and packaging), up to five live, each in its own worktree
with a brief file under `briefs/` in the repo (commit the briefs, they are the dev log's spine). Start A1 (core), A2 (AI brain against the core's interfaces, headless
tests), A4 (cartridge + validator port) and A5 (camera, HUD, input) in parallel as soon as the project skeleton and the seam stubs are committed; A3 (balancing sweeps)
starts when A1's simulation compiles headless; A6 packages at H6. Deliverables: rink + boards + two nets from procedural geometry (a rink builder in C++ or geometry script reading `game.rink`), one player skater pawn (capsule) with the
skating model from `tools/skills/skating-physics.md` and the HOCKEY.md numbers (a plain fixed-step C++ simulation at 60 Hz, not Character Movement, not Chaos),
the puck in the same simulation (0.12 m, near-black, 30 m/s cap, 4 sub-steps per frame; Chaos only for cosmetics), shoot (reach 1.5 m, 18 m/s,
24-frame cooldown), goal triggers behind the lines, score HUD (plain UMG text) with a radar mini-rink, restart on R, the **third-person follow camera** (HOCKEY.md section 2 and
`game.camera`) with the arena camera as the cartridge alternative, a 120 x 60 m rink by default, cartridge loading from JSON through a validator port of
`schema/validate.mjs`'s rules (unknown keys are errors). Tests: `FarNetReadable`, a scripted "score on an empty net" automation test,
`Package` via `tools/run/package.ps1 -Config Development` then `run-packaged.ps1 -Seconds 10`. Commit per builder package; push; `status/A.md` every hour with free RAM,
GPU memory, editor crashes, packaged-build age, and what is next. Screenshots through the SceneCapture toolset into `devlog/shots/` with the naming convention in `devlog/README.md`.

## After M0 (if time remains before the operator wakes)
Start M1's code only: the AI brain (roles, yield rules, stall detector) and the bot tests `PlayerBotWins`, `AiBotPlays`, `HumanHasSomethingToDo` as automation tests,
the toy skater visual behind `IJamSkaterVisual` (team colour and number as material parameters, lean and bob by code). No sounds, no imported meshes, no textures beyond
flat materials until the operator is awake. Keep packaging green.

## When the operator wakes
Leave `status/A.md` current, with: what passed, what is blocked, the three decisions you need (art tier pick if not made, the twist if the theme was unclear, the
handoff list for device B). Then wait for instructions.
