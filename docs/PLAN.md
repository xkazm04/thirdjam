# Third Jam: the plan for the 2026 Epic AI MegaJam

Status: v2, 2026-11-12 ~01:00 CET (v1 2026-11-11 evening). The game is decided: ice hockey 3v3, see `docs/HOCKEY.md`; the two-machine split is in `docs/TRACKS.md`; both override this file where they differ. Planning only. Under rule 7(h) of the official rules nothing here is
game content: it is tooling, schema, prompts, scripts and a schedule. The Unreal project is created after
**Thu 2026-11-12 06:00 CET** (jam start 00:00 EST). Source of decisions: spark `megajam-unreal` in the third-life vault.

## 1. What we are doing and why

Enter the Unreal Engine track with **one excellent game plus a live Designer layer**, built by agents on two devices, from
scratch, in about 93 hours. The entry validates Third Life's vision (an LLM turns a sentence into a playable variation of a
game, over a data-driven rules layer) in a contest with judges, and it brings back lessons, mechanics and perhaps assets.

Win conditions, in order: (1) the game is fun with no explanation in the first two minutes (ties are re-scored with double
weight on Fun Factor); (2) it packages and runs on a judge's machine with no extra software (rule 7f); (3) it visibly uses the
theme (rule 7g); (4) the Designer layer is a real feature that earns points under Ambition and Gameplay, not an AI badge
(disclosed AI use "will not, on its own, increase or decrease a score"); (5) the dev log is good enough for Glass Box.

## 2. Rules that shape the plan (digest of the official rules, pp. 4-11, and the itch page)

| Rule | Consequence |
|---|---|
| 7(h) only tools, agent configs, MCP settings, prompt libraries, scripts may exist before the start; sponsor may ask for them or the VCS history | prep lives in this repo, committed with dates; `git tag pre-jam` at 05:59 CET; no game content, levels, meshes, textures or audio before 06:00 |
| Sourced (non-original) assets must be noted at submission, are not scored; AI-generated content is not "sourced" | keep an `ASSETS.md` ledger from hour one: source, licence, tool |
| No trademarks or logos except the Unreal Engine logo | no Claude, Anthropic, NVIDIA or Meshy logos in the game or the video; text credits go in the dev log |
| 7(d)(e) not a tutorial extension; custom gameplay beyond starter templates | start from a blank C++ project, not the Third Person or Vehicle template |
| 7(f) no additional software; 7(i) all English; teen (13+) content | the Designer must work offline by default; no profanity, gore, real brands |
| Video 1920x1080 or higher, 30 to 60 s; **zip under 1 GB** named TeamName_GameName; Windows is the primary target | one packaged Windows build under the size budget of section 4b, measured at every package; web is not a target (experimental, 500 MB total, 200 MB per file) |
| One main prize and one modifier prize per submission | pick one modifier push (see 8); entering the others costs nothing |
| Glass Box dev log must be submitted together with the entry | dev log link ready by H80 |
| Late submissions are disqualified | internal hard deadline H90, target H88 |

## 3. Clock (all times CET; H0 = Thu 06:00)

Window: Thu 06:00 to Mon 02:59 CET = 92 h 59 m. Epic's daily livestream is 20:00 CET (Thu to Sun): listen for tips and for the
NVIDIA and partner prize criteria.

| When | Hours | Gate |
|---|---|---|
| Thu 06:00-07:00 | H0-1 | **Theme binding** (section 6): skin and one twist for hockey; `GAME.md`, `THEME.md`, the `game` schema (HOCKEY.md 4); A commits the seam stubs (TRACKS.md 3). Hard cap one hour |
| Thu 07:00-12:00 | H1-6 | greybox playable on A (core verb works, one arena, placeholder shapes); B fixes look direction and builds the first asset batch; first **package test** |
| Thu 12:00 | H6 | Gate 1 = **M0 greybox** (HOCKEY.md 3): score on an empty net; packaged build runs. If not, cut scope now |
| Thu 12:00-Fri 06:00 | H6-24 | core loop to fun; Designer D0 (cartridge presets); audio pass 1; look pass 1 |
| Fri 06:00 | H24 | Gate 2 = **M1 regular hockey, the MVP**: bots pass, a fresh player scores within 2 minutes, L1 look. Second package test |
| Fri 06:00-Sat 06:00 | H24-48 | levels and progression; Designer D1 (text to validated JSON via the API); assets pass 2 |
| Sat 06:00 | H48 | Gate 3 = **M2 raw play** on L2 night; skater track T-B1/T-B2 verdicts. Third package test |
| Sat 06:00-Sun 06:00 | H48-72 | **M3 boost + M3b arenas by H56** on L3 neon (or L2 if the frame gate fails); then polish, audio pass 2, Designer D1 hardening, tutorial text |
| Sun 06:00 | H72 | **Feature freeze**. Only fixes, polish, packaging from here |
| Sun 06:00-18:00 | H72-84 | packaging, clean-machine test, video capture, dev log, assets ledger, submission text |
| Sun 18:00 | H84 | submission candidate built and tested on a clean profile |
| Sun 22:00 | H88 | submit. Buffer to the real deadline (Mon 02:59) is 5 h |

Human attention windows (the Director decides at the gates; agents run with budgets in between): H0-8, H12 (Gate 1),
H24 (Gate 2), H36 (10 min check), H48 (Gate 3), H60, H72 (freeze), H80-88. Sleep goes in the gaps; no gate decision waits
on a sleeping human for more than 6 hours.

## 4. Architecture

```
Game/                         Unreal 5.9 C++ project, blank template, created at H0
  Source/JamCore/             rules runtime: Cartridge (data), RuleEngine, Scoring, Spawner, AI bots
  Source/JamDesigner/         the live Designer: HTTP client, schema validation, cache, prompt assembly
  Source/JamGame/             the playable game (pawn, camera, input, HUD, game mode)
  Plugins/JamAssets/          content-only plugin: ALL art, audio, materials, lighting presets. Owner: device B
  Content/Cartridges/         JSON and DataAssets: shipped presets, one per variation
  Content/Maps/               levels. Owner: device A
```

- **Cartridge**: one JSON document (with a Data Asset mirror) describing a variation of the game: arena parameters, rules,
  scoring, spawn tables, AI behaviour parameters, theme skin tokens. It is Third Life's pack manifest reduced to what a
  4-day game needs. Unknown keys are errors; an absent value is an omitted key; lists, not singletons (laws that carry over).
  The schema is prepared before the start (`schema/cartridge.schema.json`) as a **generic envelope** with no game-specific
  fields, and is extended in hour 1.
- **Designer**: player types a sentence, `JamDesigner` assembles a prompt (schema, rules of thumb, the current cartridge),
  calls an API model (fast tier: `claude-haiku-5-5` first, `claude-sonnet-5-5` as a retry), validates the JSON against the
  schema and against engine limits, and either applies it or shows a readable error. **Tiers**: D0 shipped preset
  cartridges selectable in-game (always works); D1 live generation with a user-supplied API key typed into the settings
  screen (never bundled in the build, never logged); D2 an in-game "workshop" view that shows the Designer's steps as a
  readable log. D0 is mandatory, D1 is the target, D2 is first to cut.
- **Offline default**: with no key the Designer UI offers the cached sentences and their cartridges, so a judge sees the
  feature working in 30 seconds without any key or network.
- **Time is physics frames** (60 Hz) in tests and bots; randomness from one seeded stream (laws carried over). Automation
  tests run through `UnrealEditor-Cmd` and Epic's AutomationTest toolset.

## 4b. Size budget (the packaged game, not the repo)

The contest limit is a zip **under 1 GB** for the Windows build (web builds would be 500 MB total and 200 MB per file; not a target).
Unreal eats most of that by itself if nobody watches: a Shipping build of a blank project is roughly 150-300 MB with engine content,
one 4K PBR texture set is 50-100 MB uncompressed, a Fab environment pack can be several GB. So the budget is set now and owned:

| Slice | Budget | Owner | How it is held |
|---|---|---|---|
| Engine and runtime (exe, shaders, engine content, Chaos, Slate, VC++ redistributable) | 340 MB | A | **Measured 2026-11-11 (P1): an empty-map Shipping zip is 316 MB** (staged 466 MB: exe 177, paks 203 of which about 130 MB is shader code, redistributable 29); Development is 484 MB zipped and never ships. Shipping config, no debug info, no editor content cooked, only the maps in the cook list, unused plugins disabled; dropping the SM5/D3D11 fallback saves about 16 MB if needed |
| Art: meshes, textures, materials (`JamAssets`) | 350 MB | B | textures at 2K max (1K for small props), one texture set per material family, Nanite only where it pays, generated meshes decimated on import, nothing imported "to try" |
| Audio | 80 MB | B | OGG/ADPCM compression, music under 3 minutes of unique material, stereo only for music |
| VFX, UI, fonts, cartridges, Designer cache | 40 MB | A | |
| Reserve | 190 MB | Director | not spendable before H72 |

Rules: `package.ps1` prints the staged size and the zip size at every package (H6, H24, H48, H72, then every 6 h) and warns at 900 MB; a
package over budget at H48 is a gate failure that stops asset import until it is back under; every asset row in `ASSETS.md` carries its
on-disk size so the ledger is also the budget ledger; **a Fab or generated pack is imported selectively** (the few assets used), never as a whole;
the cook list is explicit (`+MapsToCook` in `DefaultGame.ini`) so stray test maps never ship. Device B packages its plugin alone first (section 5, rule 8)
and reads the size before it hands assets over.

## 5. Two devices, one human, many agents

| | Device A (gameplay) | Device B (look and assets) |
|---|---|---|
| Owns | `Source/**`, `Content/Maps`, `Content/Cartridges`, Designer, HUD, packaging | `Plugins/JamAssets/**`, lighting presets, post process, audio, VFX, thumbnails and video capture |
| Agents | Director session, C++ builder, Blueprint/level builder, QA bot | asset builder (Fab sourcing, Meshy/Tripo/Rodin generation, import and material setup), lighting/look builder, audio builder |
| Editor | one UnrealEditor, one MCP-driving agent at a time | same |

Rules of the road:

1. **One owner per path.** The table above is the lock. A change across the line goes through a written request in
   `handoff/` (a file named `A-to-B-0007.md`), never an edit.
2. **Git + LFS**, one repo, branch per device (`dev-a`, `dev-b`), the Director merges into `main` at least every 3 hours.
   Disjoint ownership means merges are fast-forward in practice. LFS tracks `.uasset`, `.umap`, `.wav`, `.png`, `.fbx`,
   `.uexp`. **Size budget**: assets repo under 8 GB total (free GitHub LFS is about 10 GB); no raw 4K texture sets or
   uncompressed audio committed; originals stay in a local `source/` folder outside git.
3. **Same engine build on both devices** (promoted 5.9.0, CL 60351404). A different engine build breaks asset exchange. The
   shipped build uses stock 5.9 unless NvRTX is proven on both devices by H12 (see `NVRTX-NOTES.md`).
4. **Epic's MCP serves one call at a time on the editor's game thread**, so each editor has one agent driving it. Other agents
   work on text files (C++, JSON, prompts, docs) and queue editor work through the driver. Long builds (Live Coding, cooking)
   are split into steps shorter than the client's timeout.
5. **Contracts first.** Before two agents build in parallel, a contract in `contracts/` names every wire-level identifier:
   cartridge keys, enum strings, asset names (`SM_`, `M_`, `MI_`, `T_`, `SFX_`), folder names, data asset names. Every new
   input names its producer and its first consumer. Builders may counter-propose; their measurement outranks the brief.
6. **Hourly sync**: each device writes `status/<device>.md` (done, next, blocked, size of LFS) every hour; the Director
   reads both at each gate.
7. **Autosave and crash recovery**: the editor will crash overnight. A watchdog script restarts `UnrealEditor.exe` with the
   project, and agents commit at least hourly. Nothing important lives only in an unsaved editor.
8. **Package early**: a packaged build at H6, H24, H48, H72 and then every 6 hours. Cook failures found at H80 lose the
   entry. Packaging runs on A; B packages its plugin alone first to catch asset errors.
9. **No running of two heavy jobs at once on one device** (the lesson of 2026-11-11: wall-clock limits were meaningless while
   the Director verified, a job ran and three builders worked on one machine).

## 6. Theme binding (hour 0 to 1): the game is fixed, the theme is a skin and a twist

The game is ice hockey 3v3 (`docs/HOCKEY.md`); hour 1 does not choose a spine. It binds the theme in two layers:

1. **Skin** (always): theme name, mood words, palette, trim colour, the arena's dressing and backdrop, pickup names and shapes, the
   jersey motif, the title. All of it is cartridge `theme` and `look` data plus device B's dressing list.
2. **Twist** (one, chosen by the rubric below): one rule or pickup that only makes sense under the theme, as a cartridge entry in M3's
   pickup table or M3b's arena list (examples of the kind: a rink that shrinks each goal, a puck that splits, a boost that only the losing
   team can take). The twist is the "use of theme" judges can see in 30 seconds without being told.

| Criterion | Question |
|---|---|
| Theme visibility | can a judge see the theme in the first 30 s (title, arena skin, the twist) |
| Fun within 2 minutes | does the twist make M1 more fun, or only different |
| Buildable by H56 | is the twist data over existing systems (pickups, rules, rink), not a new system |
| Designer fit | can a sentence turn the twist up or down |
| Look ceiling | does the skin fit the art ladder's chosen tier |

Hour 1 ends with `GAME.md` (one page: pitch, theme skin, the twist, five cartridge parameters the Designer may change, the 2-minute first-play
script) and `docs/THEME.md` (the theme's exact wording).

## 7. Prep before 06:00 CET (what we may and may not do)

Allowed (rule 7h) and decided: **agent harness + MCP smoke test; rules schema + Designer prompts + eval script; dev log
pipeline; look studies as reference images only.** All of it goes in this repo, committed with dates.

| # | Task | Output | Done when |
|---|---|---|---|
| P1 | MCP smoke test: throwaway blank project in the scratch folder, enable Unreal MCP + toolsets, generate `.mcp.json`, connect Claude Code, spawn an actor, read a screenshot via SceneCapture, run an automation test, **then delete the project** | `tools/mcp/README.md`, `.mcp.json` template, a smoke script | a single script proves editor start, MCP answer and packaging of an empty map in minutes |
| P2 | Editor watchdog and packaging scripts (`UnrealEditor.exe`, `RunUAT BuildCookRun`), log capture | `tools/run/*.ps1` | restart and package work from one command |
| P3 | Cartridge envelope schema, validator (Node), Designer prompt library and eval harness with 10 sample sentences | `schema/`, `prompts/`, `tools/designer-eval/` | eval passes against a mocked model, and against the API with a key |
| P4 | Dev log pipeline: capture transcripts, commit messages and editor screenshots into a timeline (HTML) | `devlog/` | `node devlog/build.mjs` makes a page from fake data |
| P5 | Look studies: Codex image generation of 3 to 4 art directions, **never imported**, kept in `prep-reference/` with a README declaring them, committed before 05:59 | `prep-reference/` | declared in the dev log at hour 0 |
| P6 | Disk and accounts: free space on both devices (C: has 73 GB free of 1.9 TB, target 200 GB or a second drive), GitHub account linked to the Epic account (NvRTX and Unreal source access), LFS enabled, partner perks (Anthropic credits, Meshy, Tripo, Rodin) claimed by the operator | checklist | every box ticked |
| P7 | Orchestration skeleton: ownership table, `handoff/`, `status/`, contracts folder, `ASSETS.md` ledger template, gate checklist | repo folders | a dry run of an A-to-B handoff works |

Not allowed before 06:00: creating the real game project, any level, mesh, texture, audio, animation, any Fab download that is
used in the game (browsing and licence reading are fine), any cartridge for the real game.

## 8. Modifier choice (only one can be won)

Plan around **Render Royalty** (device B's whole job is visuals, and visuals count in the main score anyway) while making
**Glass Box** nearly free by logging as we go. Because only one modifier can be won, the Director decides at H48 which one the
final hours serve. Solo Driver is available because there is one human; it applies automatically if the entry is solo.

## 9. Cut lines (in this order)

1. Designer D2 workshop view. 2. L3 neon (ship L2). 3. M3b arena manipulation. 4. M3 pickups beyond two kinds. 5. Live generation D1 reduced to "pick from generated variants". 6. Skater tracks T-B1/T-B2 (the toy figure ships). 7. M2 raw play (ship M1 alone). Never cut: packaging, the 2-minute first-play experience, audio, the dev log, the assets ledger.

## 10. Risks

| Risk | Mitigation |
|---|---|
| Packaging fails late | package at H6 and every 6 h; cook warnings are errors from H24 |
| Editor crash loops | watchdog, hourly commits, blank-project smoke script |
| MCP experimental API changes under us | smoke script at the start of each session; fall back to Python/console toolsets or plain C++ |
| LFS quota or disk exhaustion | size budget, check at each hourly status |
| Packaged game over 1 GB | budget per slice (4b), size printed at every package, selective imports, 2K texture cap, explicit cook list; over budget at H48 stops imports |
| Two devices diverge | ownership table, same engine build, merges every 3 h |
| Designer disappoints judges | D0 presets always work offline; scripted first 30 s |
| Theme fits the spine badly | the rubric in section 6 may pick a different spine; the one-hour cap stops dithering |
| Pre-jam material questioned | everything dated in git, `pre-jam` tag, reference images declared |
| Over-using one machine for several heavy jobs | one heavy job per device at a time |

## 11. What comes back to Third Life

Capture in `retro/` as we go: which cartridge fields the Designer could change reliably, which prompts failed, how long each
stage took per agent, asset pipeline timings, what the data layer in C++ needed that GDScript did not. Candidates to port back:
the cartridge schema (as a pack manifest extension), the Designer prompts and eval harness, gameplay recipes that worked,
and any original assets (assets made in the jam are ours, and the licence given to the sponsor is only for publicizing the contest).

## 12. Open items

- Pages 1-3 and 12-16 of the rules (eligibility and legal text) not read.
- Whether the installed UE 5.9 is the "MegaJam Preview" needed for experimental web builds (not a target, so low priority).
- VS 2022 "Game development with C++" components: the toolchain folders exist; the smoke test (P1) is the proof.
- Second device: specs, free disk, UE 5.9 installed.
- NVIDIA and Hyper3D prize criteria: ask at the kickoff stream or the jam Discord.
