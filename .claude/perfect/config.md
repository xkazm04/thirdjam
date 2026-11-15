---
product: "Hokej (Third Jam, Epic AI MegaJam 2026 entry)"
stack: "Unreal Engine 5.9 C++ (modules JamCore, JamGame, JamNet, JamDesigner, JamEditor), node/PowerShell tooling under tools/"
vault: []
vault_subdir: Perfect
base_branch: main
wave_size: 1            # PLAN rule 9 + live lanes: one UE builder at a time on this machine
lot_caps: {ue: 1}
pool_target: 5
round_shape: round
cooldown_rounds: 2
commit_format: "perfect(<context>): <title>"
context_map: ""         # none in repo; provisional map lives in the vault's Perfect.md
active_runs_ledger: "status/A.md"
locale_count: 1
---

## Gates
always: the full `Jam` automation filter in a cleared environment (invocation in `tools/gate/premerge.ps1`); `node tools/gate/check-test-guards.mjs`; Shipping compiles (`tools/run/build.ps1 -Project Game -Config Shipping`)
when a sim/brain rule changes: the hash before and after with a neutral value (100 seeds a level); `Jam.Net.HashCoverage`; law 3 rows
builder: build.ps1 Editor Development green; the touched `Jam.*` test groups green; headless only (`-nullrhi -Multiprocess -unattended`)
visual: off-screen stills from the game camera with `-dpcvars=r.RDG.AsyncCompute=0`, 1280x720 and 1920x1080 (the cursor belongs to the lanes)

## Class B
- `status/*.md` (anchored append only)
- `docs/experiments/*.md` index lines

## Class C
- `contracts/*` (the lane Director's), `ownership.json`, `briefs/*`, `status/A.md`
- the git index and `main` (landing waits for the operator's word)

## Repo law
- Read `briefs/EVOLUTION-COMMON.md` and `briefs/NIGHT-COMMON.md`: memory band 38-50 GB in use, never past 56; stop processes only by recorded pid; `-Multiprocess` on every headless start; builds share one `-WaitMutex` lock, timeouts > 20 min.
- Never touch `Game/Plugins/JamAssets`, `Game/Content/Look/**`, `Game/Content/Audio/**` (device B's).
- Files held by live lanes are reserved (read `status/A2*.md` for the current set before each wave).
- The operator's eye is the bar: every direction states what a person at the keyboard sees or feels, and proves it moved.
- New sim state fields go in `UJamSim::StateHash()` or in the readout lists in `JamNetGuestTests.cpp`.
- After the H72 freeze: fixes and polish to the shipped game only, plus anything the operator orders.
- Teen rating; no brand, league, team or film names or looks.

## Context sources
- No context map. Provisional map (one context per gameplay surface) lives in `.perfect/Perfect/Perfect.md`.
- No `.personas/` ledger in this repo: coverage outbox skipped.

## User taste
- The operator plays the default first (PLAY NOW, rookie, regular rink, mouse and keyboard) and judges by eye; scripted-hand numbers have been wrong about what he sees (R-0067, R-0069).
- 2026-11-15: chose the full loop (option 3) during the live jam push, over read-only handoff.

## Skill improvement log
- 2026-11-15: in this repo the live lanes, not the context map, decide the write sets. Read `status/A2*.md` and every worktree's `git status` immediately before each build: within one hour a lane took half of an accepted direction (A219, the stop line), and a new lane (A222) took HUDWidget.cpp/.h.
- 2026-11-15: a watch on a status file must match the READY line itself, not a brief's "READY due HH:MM" line (the first watch fired on the deadline line). Background watches get killed under memory pressure: on resume, re-check memory and main rather than re-arming.
- 2026-11-15: a fresh worktree builds the editor target in about 2 minutes here (shared engine), and the full Jam filter takes about 10 minutes headless, so an S direction is cheaper done inline by the Director than through a builder.
