# Gates: checklists

Source: `docs/PLAN.md` sections 3, 4b, 5 and 9, and the digest of the official rules in section 2. H0 is Thu 2026-11-12 06:00 CET.
The dev log ruler reads the headings below (`## Name (H<hour>): what it decides`), so keep that form when editing.

How to read an item:

- `[auto]` items name the script that checks them and the command to run. Exit 0 is a pass.
- `[tool]` items are checked by running a project script or the packaged build; the script's own output is the evidence.
- `[human]` items are a person's judgement or a manual inspection. Record who looked and when in `status/`.

Every gate ends with one line in the dev log (`devlog/notes.md`): go, cut, or stop, and why. A gate that fails
cuts scope (section 9 of the plan) rather than moving the gate.

Always, at every gate:

- [ ] [auto] Ownership clean on both devices: `node tools/gate/check-ownership.mjs --device A` and `--device B` (exit 0).
- [ ] [auto] Assets ledger complete and inside budget: `node tools/gate/check-assets-ledger.mjs` (exit 0).
- [ ] [tool] Both `status/A.md` and `status/B.md` have an entry for the last hour (done, next, blocked, LFS size, engine build id, packaged-build age).
- [ ] [human] No open `handoff/` request is older than its deadline hour.
- [ ] [auto] Dev log builds: `node devlog/build.mjs --out devlog/out/index.html --embed` (exit 0).

## Gate 1 (H6): core loop playable for 2 minutes

Decides: is there a game to polish. If not, cut scope now.

- [ ] [human] `GAME.md` exists (one page: core verb, win and lose, pitch, theme skin, five cartridge parameters, first-play script).
- [ ] [human] The core verb works on greybox shapes: one arena, placeholder meshes, a player plays two minutes without a crash.
- [ ] [human] Theme is visible without explanation in the first 30 seconds (rule 7g).
- [ ] [tool] First package test: a packaged Windows build starts and reaches the arena (`tools/run/` packaging script from P2; output kept in the dev log).
- [ ] [tool] Packaged size read from the package script output and noted in `status/A.md` (budget in plan section 4b).
- [ ] [human] Device B has fixed the look direction and has the first asset batch in `Game/Plugins/JamAssets/`, each row in `ASSETS.md`.
- [ ] [human] Project started from a blank C++ project, not a starter template (rule 7d, 7e).
- [ ] [auto] `git tag pre-jam` exists and points at a commit before H0 (`git rev-parse pre-jam`; no game content before it).

## Gate 2 (H24): fun check

Decides: is the game fun with no explanation. Ties are scored with double weight on Fun Factor, so this gate is the one that matters.

- [ ] [human] A fresh player, or an agent playing a recorded session, gives a written verdict; kept in `devlog/notes.md`.
- [ ] [human] The first two minutes need no explanation (watch the player, do not talk).
- [ ] [tool] Second package test passes; cook warnings are treated as errors from here.
- [ ] [tool] Designer D0 works offline: the shipped preset cartridges are selectable in game with no key and no network.
- [ ] [auto] Cartridge schema validator accepts every shipped cartridge (validator from P3 in `schema/`; unknown keys are errors).
- [ ] [human] Audio pass 1 exists (at least move, hit, score, music bed) and look pass 1 is in.
- [ ] [auto] No file outside its owner's paths changed since Gate 1 (`check-ownership.mjs`, both devices).

## Gate 3 (H48): vertical slice

Decides: does the whole planned experience exist, ugly allowed. Also the first hard size check of the packaged game.

- [ ] [human] Start to end of the planned experience can be played once, from the main menu to the end screen.
- [ ] [tool] Third package test passes on a clean profile.
- [ ] [tool] Size budget: packaged zip under 1 GB, read from the `package.ps1` output (staged size and zip size). Over budget at H48 stops asset import until it is back under.
- [ ] [auto] Per-slice budget: `node tools/gate/check-assets-ledger.mjs` reports shipped art at most 350 MB and audio at most 80 MB (exit 1 over budget).
- [ ] [human] Designer D1 (text to validated JSON through the API) works with a user-supplied key, and failure shows a readable error.
- [ ] [human] The Director decides the modifier prize to serve in the final hours (plan section 8) and records it in the dev log.
- [ ] [human] Cut list reviewed against plan section 9; cuts recorded in `devlog/notes.md`.

## Feature freeze (H72): only fixes, polish and packaging from here

Decides: what is in the game. Nothing new after this hour.

- [ ] [human] Feature list frozen in `devlog/notes.md`; every later change is a fix with a reason.
- [ ] [tool] Fourth package test passes; packages every 6 hours from now on.
- [ ] [tool] Size budget: packaged zip under 1 GB, read from the `package.ps1` output. The 230 MB reserve becomes spendable only after this gate.
- [ ] [auto] Ledger and budgets: `node tools/gate/check-assets-ledger.mjs` exits 0 (art at most 350 MB, audio at most 80 MB, every shipped file has a row).
- [ ] [human] Every row in `ASSETS.md` has an origin (original, ai-generated or sourced), a licence for sourced rows and a tool or service for generated rows.
- [ ] [human] No logos or trademarks anywhere in the game, the menu or the credits, other than the Unreal Engine logo.
- [ ] [human] All text in the game is English and the content is suitable for teens (13+): no profanity, gore or real brands.
- [ ] [tool] Cook list is explicit (`+MapsToCook` in `DefaultGame.ini`), so test maps do not ship.
- [ ] [human] Designer: the offline path (cached sentences and cartridges) works with no key and no network.

## Submission candidate (H84): built and tested on a clean profile

Decides: is this the build that gets submitted. Buffer to the real deadline (Mon 02:59 CET) is about 9 hours; the internal target is H88.

- [ ] [tool] Packaged Windows build runs on a clean machine or clean user profile with no extra software installed (rule 7f), cold start to playing.
- [ ] [tool] Size budget: packaged zip under 1 GB, read from the `package.ps1` output; warning threshold is 900 MB.
- [ ] [human] The zip is named `TeamName_GameName.zip` and contains the game folder, a short README for judges and nothing else.
- [ ] [human] Video captured: 1920x1080 or higher, 30 to 60 seconds, shows the theme in the first seconds, no logos other than Unreal Engine.
- [ ] [human] Tools list written: engine version and build, agents, models, asset services (Meshy, Tripo, Rodin or others), each as text credit only.
- [ ] [human] Sourced assets list written from `ASSETS.md` (rows with origin `sourced`); AI-generated assets are listed as AI-generated, never as sourced.
- [ ] [auto] Ledger final: `node tools/gate/check-assets-ledger.mjs` exits 0.
- [ ] [auto] Dev log builds with screenshots embedded: `node devlog/build.mjs --out devlog/out/index.html --embed`, then the page is read once by a person for leaks.
- [ ] [human] Dev log is hosted and its link works from a logged-out browser.
- [ ] [human] Submission text drafted: pitch, how to play, what the Designer does, what is original and what is not.

## Submission (H88): submit

Decides: the entry goes in, five hours before the real deadline.

- [ ] [tool] The exact zip that is uploaded is the zip that passed the H84 clean-machine test (compare size and hash).
- [ ] [tool] Zip under 1 GB, named `TeamName_GameName`, opens and extracts with a standard tool.
- [ ] [human] Video file meets the rule: 1920x1080 or higher, 30 to 60 s, plays in a normal player.
- [ ] [human] Dev log link is in the submission (Glass Box needs it submitted together with the entry).
- [ ] [human] Tools list and sourced assets list are filled in on the submission form.
- [ ] [human] Main prize category and one modifier prize chosen on the form (only one modifier can be won).
- [ ] [human] Submission confirmed on the page after sending; screenshot of the confirmation saved in `devlog/shots/`.
- [ ] [human] Late submissions are disqualified: nothing is changed after H90 except a confirmed re-upload before the deadline.
