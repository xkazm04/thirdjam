# Lane A96: submission pass 2: everything that needs no game window (device A, from H23, R-0043 point 4, Sonnet)

You are lane A96 of the Third Jam (team Hokej). Lane A88 built the submission's first version (`submission/README.md` is the checklist) and listed what is missing. A
capture lane cannot run now: lane A95 owns the machine's cursor and game windows for its acceptance runs until about 19:30. Your job: every open item that needs no game
window, so the Sunday pass is only the video and a refresh.

Read first: `submission/README.md`, `submission/page.md`, `submission/credits.md`, `submission/video/cutlist.md`, `status/A88.md`, `docs/PLAN.md` section 2 (the rules' digest)
and section 8 (the modifier), `docs/PLAN-RECONCILED.md`, `docs/VISION.md`, R-0041, R-0043, R-0045, `docs/experiments/VOICE.md` and `submission/tagline.md` (A89's voice and
taglines), `docs/experiments/CONTROLS.md`, `ABILITIES.md`, `MOVEMENT.md` if on main (the bindings today: Q pass, held Q saucer, F ability, E spin, wheel deke, RMB check, MMB
cross-check, Shift sprint, S against your motion brakes, Space poke), `ASSETS.md` and `tools/gate/check-assets-ledger.mjs`, `devlog/README.md`, `devlog/build.mjs`, `tools/run/
package.ps1` (how the zip is named and what is staged), `tools/media/*` (A88's scripts).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a96` (created at the current main). Run no git command except the read-only `git -C <main checkout> log` forms
the dev log's builder itself runs with `--repo` (say which). Never touch `Game/Plugins/JamAssets` (a ledger row about B's asset is a finding for B, not an edit of the asset);
never print `.env`; never kill a process you did not start; **launch no game and no editor** (lane A95 has the cursor); no package builds (AutomationTool is A95's).

You own: `submission/**`, `ASSETS.md` rows for A's own assets only (`Game/Content/**`), `devlog/curated/**` (a shot list, not copies of the 1.1 GB), `tools/media/**`,
`tools/run/package.ps1` only for the two packaging items below, `Game/Build/` or wherever the staged extras live for a `README.txt` in the package (find how files are added
to the staged build: `DefaultGame.ini` `+DirectoriesToAlwaysStageAsNonUFS` or a post-stage copy in `package.ps1`), `status/A96.md`.

## Job
1. **The zip's name and the readme**: `package.ps1 -Zip` also writes `Hokej_Hokej.zip` (TeamName_GameName, the rules' form) beside the versioned zip for a submission
   candidate (a `-Submission` switch), and the staged build carries a `README.txt` (how to start, the controls, the prerequisites installer `vc_redist.x64.exe` and when to run
   it, the Windows Firewall prompt on first launch and that Cancel is fine for solo play, credits pointer). Prove by reading the script's dry logic and a unit run of its zip
   step on an existing staged folder (`Game/Packaged/Hokej_Hokej_H25` in the main checkout: copy the folder list, do not rebuild).
2. **The ledger**: the findings A88 listed: `T_UI_RingPlain` with origin `generated` (find how it was made from the history in `status/` and `docs/experiments/UI.md`; set the
   allowed origin and the tool); the nine unshipped sourced rows without a URL (P4 is descoped, R-0036: if the files are A's under `Game/Content/P4`, propose removing files and
   rows in a list for the Director, do not delete); the six size-mismatch warnings (update `size_mb` for A's rows; list B's for B). `check-assets-ledger.mjs` green after.
3. **The dev log**: build with `--repo` so commits show; a curated shot list of 60 (`devlog/curated/shots.txt`: the first working rink, the controls fix, the figures, tackling,
   clay, the modes menu, the acceptance run's table; chosen by file name and date from `devlog/shots` and `docs/experiments/img`) and `--embed` on that list only; the page's
   size, the redaction counts, a grep for names, keys and drive paths; what a judge can follow.
4. **The page text** in A89's voice: the controls as they are now, the vision in four sentences a player understands (hits obey momentum; the carrier wins by stopping and
   cutting; five a side that pass; modes and abilities), the modes, the tools sentence in the rules' wording, a placeholder line `TEAM: <names>` for the operator; three
   lengths (a one-line pitch, a 60-word blurb, the full page).
5. **The video's script v2**: a shot list for 30 to 60 s that needs H26 (contact by momentum: a head-on hit against a same-direction bump; the stop-and-go past a defender; a
   pass play by the bots; an ability; the menu), with the drill or console command that stages each shot (from `TACKLING-2.md`, `ACCEPT.md`, `MECHANICS-4.md`), ready for the
   capture lane on Saturday; and the question for device B: capture with audio on B (A-to-B-0020 point 5).
6. `submission/README.md` updated: every row's status, what is left for Saturday and Sunday, the operator's decisions needed (team names, the modifier, who presses submit).

Report: the checklist, the ledger result, the dev log's numbers, the files. Write `status/A96.md` from the first minute. Time box 90 minutes.
