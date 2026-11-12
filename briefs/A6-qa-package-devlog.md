# Lane A6: QA gate, Shipping packages, dev log (Glass Box), submission skeleton (device A, from H5.5)

You are lane A6 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Read first: `docs/PLAN.md` (2, 3, 4b, 8), `docs/GATES.md`, `docs/THEME.md`, `devlog/README.md`,
`devlog/build.mjs` and its lib, `tools/run/package.ps1`, `tools/run/run-packaged.ps1`, `tools/gate/*.mjs`, `ASSETS.md`, `status/A.md`, `briefs/*.md` (the dev log's spine), `ownership.json`.

You own: `devlog/**` (except `devlog/README.md`'s existing content; append), `docs/SUBMISSION.md`, `tools/gate/premerge.ps1`, `tools/gate/package-schedule.ps1`, `status/A.md` (append `## H<n>`
blocks for your own results only). Run no git command (read-only `git log` for the dev log is fine). One heavy job at a time: a package is a heavy job; other lanes run editors on
8001-8004 and compile; never kill a process you did not start. Free-space floor 40 GB.

## Deliverables
1. **Shipping baseline now**: `tools/run/package.ps1 -Project Game/ThirdJam.uproject -Config Shipping -Out <scratch> -Zip -Name Hokej_Hokej` on the current skeleton, then
   `run-packaged.ps1 -Seconds 10`; record staged size, zip size, cook time, the ten largest folders; append to `status/A.md` `## H6 package`. This is the engine slice measurement for the
   real project (prep measured 316 MB on an empty map). If a cook error comes from another lane's in-progress files, record it and retry once after 20 minutes.
2. **`tools/gate/premerge.ps1 -Device A|B [-Base main]`**: one command the Director runs at every merge window: `check-ownership.mjs`, `check-assets-ledger.mjs`, `node --test` for
   schema and tools, `build.ps1` (editor target), and the automation tests that exist (`UnrealEditor-Cmd -ExecCmds="Automation RunTests Hokej" -nullrhi -unattended -nopause -log`, parse
   the result), PASS/FAIL lines and a JSON last line; exit codes 0/1/2.
3. **`tools/gate/package-schedule.ps1`**: runs a Shipping package every 6 hours from H6 (or on demand), keeps the last three zips in `Game/Packaged/` (gitignored) with the hour in the name,
   logs sizes to `status/A.md`, warns at 900 MB.
4. **Dev log (Glass Box)**: run `node devlog/build.mjs` against the real repo (`--repo .`), the real transcripts of this project (the Claude Code project folder for `thirdjam`
   under `C:/Users/kazda/.claude/projects/`, read-only, with the redaction pass) and `devlog/shots/`; fix what breaks on real data; write `devlog/notes.md` entries for H0 (exists), H4, H5
   (what each lane was told and what it reported, from `briefs/` and `status/A.md`), with the Director's Cut framing: this log is a director directing agents. Capture two
   screenshots of the current editor state through MCP on port 8005 if you need a view (the Rink map), named per the convention, into `devlog/shots/`. Build the page to `devlog/out/index.html`
   and check it opens (structure and text; no secrets: grep the output for `sk-`, `@`, the home path).
5. **`docs/SUBMISSION.md`**: the submission form filled as far as known (team Hokej, one member, the Epic-account email placeholder `<operator fills>`, game name Hokej, video link TBD,
   dev log link TBD, tools used: Unreal Engine 5.9, Claude Code (Opus/Sonnet/Fable), Codex image generation (reference only), Meshy (figure), Blender (evaluation), ffmpeg;
   sourced assets: from `ASSETS.md` at submission time), the zip naming `Hokej_Hokej.zip`, the checklist from `docs/GATES.md` H84/H88 copied in with checkboxes, and the rule reminders (no logos,
   English, teen, 1080p video 30-60 s, under 1 GB).

Report: the package numbers, the gate script's output on the current tree, what the dev log build needed fixing, every error. Time box 2.5 hours; report at 60 minutes either way.
