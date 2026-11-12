## H0 What existed before the jam, declared (rule 7(h))

Everything up to tag `pre-jam` (`88d1e77`) is tooling, prompts, scripts, reference and plans: the plan (`docs/`), the cartridge envelope schema and Designer prompts, the
Unreal MCP client and run scripts, a dev-log builder, gate checks, 29 reference stills (`prep-reference/`, never imported), four measured experiments whose only outputs are
recipes, numbers and small screenshots (`docs/experiments/`), and a Meshy API client. No game content, level, mesh, texture, audio or animation existed. The machine rebooted
at 02:33, so the scheduled 06:00 start did not run; the human Director started the build at about 10:00 CET (H4).

## H4 Theme and the first two lanes

Theme: **Director's Cut**. Our binding: every variation of hockey is a *cut*; the Designer lets the player direct their own cut from a sentence; the twist *Director's Call*
changes one tunable live once per period. Lanes A0 (project skeleton, seam stubs, contracts) and A4 (cartridge schema and presets) dispatched to agents with written briefs
(`briefs/`). Rink stays 120 x 60 m by the operator's decision, with the 90 x 45 alternative queued for the balancing lane.

## H4.5 The first two briefs: what each lane was told, what came back

This log is a director directing agents. The human Director sets the theme and the rulings; Claude sessions play the roles (the Director, the lane agents, the QA lane). A lane is told four things in a written brief under `briefs/`: what to read, which paths it owns, the laws, the deliverables in order. Everything else is its own measurement, and the briefs say so: *your measurement outranks this brief*.

- **A0, skeleton (Opus).** Told: a blank C++ project with four modules (`JamCore`, `JamGame`, `JamDesigner`, `JamEditor`), seam stubs for the other lanes, the project settings the painted-cel look needs, four contracts. Owns `Game/**` and `contracts/*.md`. Reported at H5: the skeleton builds in 56 s, MCP is up in 40 s, a Development package takes 50 s and runs. Four contracts (C-0001 to C-0004) and the first ruling (R-0001) went in.
- **A4, cartridge schema (Sonnet).** Told: the hockey `game` object from `docs/HOCKEY.md` as a JSON schema with unknown keys as errors, a zero-dependency validator, five preset cuts, tests. Owns `schema/**`, `prompts/**` and `Game/Content/Cartridges/**`. Reported at H5: schema, validator and five presets, 88 tests.

## H5 Three more lanes in parallel, and the first thing the build taught us

Lanes A1 (core simulation, rink, loader), A2 (bot brain and tournaments) and A5 (camera, radar, HUD, toy skater) were dispatched together, each with its own editor port (8001 to 8003) and its own paths, so that no two lanes write the same file. A4 was resumed to add `game.skater.visual` (toy, humanoid_b1 or generated_b2) for device B's swap test, ending at 90 tests. Lane A7 (the Designer) follows at H5.5.

What the build taught us at once: the ToolsetRegistry plugin ships some runtime code, so it cannot be editor-only, and the Development package is 1036 MB staged because of a 363 MB debug-symbol file that never ships. The house rules that made parallel work possible: one owner per path (`ownership.json`), one heavy job at a time, requests across the line go through `handoff/`, and only the Director commits.

## H6 The QA lane's first measurements, and what a real-data build of this page needed

The dev log was built for the first time on real data (28 commits, 8 agent sessions, 7 screenshots). Three things broke or misled. (1) Commits shared by `main` and `dev-b` were labelled device B, because the label came from whichever branch git named first, so the 21 preparation commits all read as B; a commit is now B only when no other branch reaches it. (2) A Claude project folder holds every older session of its folder, so the builder gained `--since` to ignore turns before H0. (3) The jam transcripts are not under a folder named for the repository: they sit with the Director's own session (the folder the Director was started in), and the lane agents are its `subagents/`; the README says so. One more finding: a test still expected the empty ASSETS.md template and failed once lane A0 added rows; it now asserts only that the ledger parses and passes.

First Shipping package attempt (H4.7): the link failed, see `status/A.md` `## H6 package`.
