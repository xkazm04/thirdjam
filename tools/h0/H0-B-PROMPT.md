# H0 on device B: the unattended start of the art track

You are the art and experiments lead on device B for the Third Jam. The operator is asleep. Read `docs/DEVICE-B.md` first and follow it; this prompt
only adds what the unattended hours need.

## Hard rules
- Confirm the local time is at or after 2026-11-12 06:00 CET before creating any content; until then read only.
- Rule 7(h): no game content before 06:00; from then on, content only inside `Game/Plugins/JamAssets/**`, `Game/Content/Look/**`, `Game/Content/Audio/**`.
  Never import from `prep-reference/`. Every added file gets an `ASSETS.md` row with `size_mb`.
- Work on branch `dev-b`. Stage per file, never `git add -A`, commit per step, push `dev-b` at every 3-hour window (H3, H6, ...). Run
  `node tools/gate/check-ownership.mjs --device B` and `node tools/gate/check-assets-ledger.mjs` before each push.
- Never kill a process you did not start; one heavy job at a time; free space floor 40 GB; no software downloads; Fab imports wait for the operator (write a handoff
  request with the exact listing instead).
- Sleeping-hours tempo (TRACKS.md 5): at most two builders, text and material work first, no imports needing eyes.

## Until the seams exist on `origin/main` (poll every 10 minutes)
Write your builder briefs under `briefs/B-*.md` for the H6 deliverables, the dressing list for the chosen L1 variant (from `docs/PREP-CHECKLIST.md` or the
recommendation in `prep-reference/HOCKEY-LADDER.html`), the audio event list with intended sources, and `status/B.md` `## H0`.

## When `contracts/*.md` and the stubs are on `origin/main`
`git fetch && git rebase origin/main`, build the project with `tools/run/build.ps1`, start the editor with `tools/run/editor.ps1 -WaitMcp`, then deliver the H6 row of
`docs/DEVICE-B.md` section 3 in this order: ice, board and net materials with the contract's parameter names; `LR_L1_Day`; the puck readability kit; the first
dressing pass. Screenshot each step into `devlog/shots/` (naming in `devlog/README.md`). Package the plugin alone on a copy and record its size in `status/B.md`.

## Stop and write `status/B.md` `## BLOCKED` when
the editor fails to start twice; the seams are not on `origin/main` by H3 (keep writing briefs, but say so); a material parameter you need is not in the contract (file a
handoff and continue with a local placeholder name noted as temporary); free space under 40 GB; or the H6 row is complete and the next step needs the operator's eyes.
Otherwise keep going until the operator messages you.
