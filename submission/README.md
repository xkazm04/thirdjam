# Submission checklist (lane A88 first version 14:45; lane A96 pass 2, 2026-11-13 17:55, main bb18439; refresh on every H package)

Rules digest: `docs/PLAN.md` section 2. Deadline Sunday 22:00 CET internal, Monday 02:59 real. Pass 2 needed no game window: what is left needs one (the video, the clean-profile repeat, a real package) and is for Saturday and Sunday.

| # | Deliverable (rule) | Status after pass 2 | Owner | What is left |
|---|---|---|---|---|
| 1 | Windows zip under 1 GB, named TeamName_GameName | **script ready, not run for real**: `tools/run/package.ps1 -Submission` writes `Hokej_Hokej_<H>.zip` and a byte-identical `Hokej_Hokej.zip`, stages `README.txt` (how to start, controls, `vc_redist.x64.exe` and when, the firewall prompt, credits) at the package root beside `ThirdJam.exe`, and writes entry names with `/` (`tools/run/zipfwd.ps1`: the H24/H25 zips have `\` in their entry names, which Explorer accepts and other unzippers do not). Unit run on a replica of H25's staged folder: 9 PASS (`tools/media/zipstep-test.ps1`). Last measured: H24 zip 272.5 MiB (27 % of the limit) | Director / A95 | one real package run with `-Submission` (AutomationTool is A95's until about 19:30); unzip it and check `README.txt` and the entry names; confirm the firewall sentence (below) |
| 2 | Video 30-60 s, 1080p+ | **v1 done** (`video/hokej-v1.mp4`, 39.5 s, silent, H24 plus an older Development take); **v2 scripted**: `video/script-v2.md` (48 s, 11 shots, the staging command per shot) | capture lane (Saturday) | film v2 on H26 after the sim freeze (12:00 Sat); no drill stages the stop-and-go past a defender (write drill 21, or the operator plays); audio: question for device B below |
| 3 | Stills for the page | **done, 8 at 1080p** (`shots/`, H24) | capture lane | refresh from H26 (menu, faceoff, a check, a knockdown, a goal, the Director's call) |
| 4 | Page text | **v2 written**: `page.md` with the three lengths (pitch 21 words, blurb exactly 60, full page about 940 words), controls as bound at bb18439, the menu's twelve modes, the tools in plain words, `TEAM: <names>` placeholder. Lines marked VERIFY depend on A92 and A93 | A96 / operator | team names; re-read `InputMapping.cpp` and the VERIFY lines on the final package; `form.md` has the form's draft fields and the tools list |
| 5 | Sourced assets noted; AI-generated not "sourced" | **done**: `credits.md` rebuilt from the repaired ledger (62 sourced rows, 1 shipped: the DLSS library, NVIDIA licence; no row without a url) | A96 | the in-game CREDITS screen: not checked whether it lists the AI tools; propose P4 removal: `ledger-proposals.md` |
| 6 | No trademarks or logos (only the Unreal Engine logo) | page, README.txt, script and captions use no brand word; vendor names only in `credits.md`, `form.md` and the dev log (text credits) | A96 / Director | not verified without a window: the game's CREDITS screen, the plugin name in the package, the Windows firewall dialog (publisher text "Hokej") |
| 7 | Glass Box dev log with the entry | **rebuilt**: `devlog/index.html` 7.8 MB, 490 commits, 123 sessions (8 main + 115 subagent), 11,999 turns, 60 pictures embedded (1280 px JPEG; list `devlog/curated/shots.txt`, 59 files plus a rendered table of the acceptance run), redacted 0 secrets / 0 emails / 10 paths; grep outside the images: no user name, no drive path, no key pattern | A96 | device B's transcripts (none found; B has 30 notes in `devlog/notes-b.md`); entries thin before H4; hosting link; rebuild on Sunday: `node tools/media/devlog-curated.mjs --repo <main checkout>` |
| 8 | Modifier prize | **no decision recorded in the repo**; Glass Box is the natural claim (the log exists and builds); Solo Driver does not apply to two people | Director / operator | decide (below) |
| 9 | Clean-profile run of the zip | PASS on H24 (A88) | capture lane | repeat on H-final, and test Cancel on the firewall prompt (only claimed, never tested) |
| 10 | One main prize + one modifier | n/a | Director | |

## The asset ledger (item 5), after pass 2
`node tools/gate/check-assets-ledger.mjs` **PASS**, 1,654 rows (was parsed as 1,401: a paragraph inside the table ended it, so 253 rows of device A's `Game/Content/**` were never validated; moved above the table). Fixed: `T_UI_RingPlain` origin `original` (a PowerShell-drawn circle, no generative model), nine P4 sourced animation rows now carry licence and url, twelve `AS_P4_*` size_mb rows updated. Six WARN left, all device B's (`BP_SkaterVisual_*`, listed in `ledger-proposals.md`), and one for device A's `vfx/font/other` 76.5 MB against a 40 MB slice, which is the 56.2 MB DLSS library alone. P4 removal (149 rows, 100 MB of A's files, all `shipped` = n) is a proposal for the Director, nothing deleted.

## What is left, by day
**Saturday**: after the sim freeze (12:00) and A95's curve pass, a Shipping and a Development H26 package (A95/Director); one `package.ps1 -Submission` run; film the video v2 shots 5 to 8 (they need H26's contact, movement and brain); stills refresh; drill 21 for the stop-and-go; device B's logo and banner and the audio answer; the in-game CREDITS screen read.
**Sunday**: assemble `hokej-v2.mp4` (`tools/media/cut.sh`), mux audio, rebuild the dev log, repeat the clean-profile run on the final zip, fill `TEAM:` in `page.md` and `form.md`, upload, submit 22:00.

## The operator's decisions needed
1. **Team names and roles** (page, form; `docs/SUBMISSION.md` still says 1 member).
2. **The modifier**: Glass Box (log exists) or something else; the plan said Render Royalty and the Director decides at H48, no record exists.
3. **Who presses submit**, and when (Sunday 22:00 internal).
4. **Vendor names**: the page says "an image-generation model, 3D-generation services, a voice-and-audio service" and sends readers to `credits.md` and the dev log; the form's "Tools you used" field names them (`form.md`). Say if the page should name them too.
5. **Development takes in the video**: drills (the contact shots 5 and 6, the ability shot 8) exist only in a Development package, whose ice looked darker than Shipping's in A88's measurement. Allow them, or have the operator play the contact shots by hand.
6. **The P4 removal** (`ledger-proposals.md`): yes or no.
7. **Audio**: can device B capture the final 30 to 60 s with sound (A-to-B-0020 point 5, still open)? Otherwise a silent cut with the game's own commentator and music muxed in.
8. **Hosting link** for the dev log (a 7.8 MB self-contained page).

## Clean-profile test of H24 (item 9, lane A88)
Unzipped to `%LOCALAPPDATA%\Temp\a88\H24` (outside the repo); `%LOCALAPPDATA%\ThirdJam\Saved` renamed away, game run from the root `ThirdJam.exe`, folder restored (the fresh one kept as `Saved.a88-clean` beside it).
Menu on screen 5 s after launch; a posted Return started PLAY THE SEASON take 1; match alive 75 s, 1.4 GB working set, score 1:0 by 60 s with an idle human, no crash.
The fresh profile wrote only `Config/Windows/GameUserSettings.ini` (Difficulty=pro) and a CrashReportClient ini. Stills: `video/takes/clean-profile-H24/`.
What a first-time machine would meet or lack:
- a **Windows Firewall prompt** on first launch from a new path (seen over the game in an early take). The game opens a socket at start. Solo play after Cancel was not tested. `README.txt` says Cancel is fine: that is a claim to test.
- the zip holds `Engine\Extras\Redist\en-us\vc_redist.x64.exe`; `README.txt` now says when to run it. Whether the game starts without the VC++ runtime cannot be tested here (this machine has it).
- D3D12 only; the DLSS dll ships and needs no setup.
- the menu hint line is clipped by the slate border and the menu still said SCENE 3 v 3 (stale against 5 v 5, R-0041).
Shipping cannot be asked anything: no log, no console, no state hash (PARITY.md). A posted Return starts a match.

## Findings (pass 2)
- **Zip entry names** had `\` (PowerShell 5.1's `CreateFromDirectory`): fixed in `package.ps1` through `zipfwd.ps1`; H24 and H25's existing zips keep the old names.
- **Ledger gate blind spot**: it stops at the first non-table line (above). A gate that fails when a table row follows prose would have caught it.
- **The old page listed Blind Side and Pinball** among the twelve modes; the menu's twelve are now Pond, Cage, Sniper, Marathon, Power Play, Shotgun, Sudden Death, Outnumbered, Glass House, Arsenal, Bruisers, Fog of War (`UJamMenuFlow::ModeCards`); the other two are parked. `Game/Content/Cartridges/modes/MODES.md` still says "two ability modes without a card yet" (stale, not mine).
- **Dev log**: the transcripts folder is the third-life project's (the jam lanes ran there), plus the thirdjam one; this lane's own session is in it.
- This lane ran one read-only `git log` on the main checkout by hand (ASSETS.md history) outside the forms `devlog/build.mjs --repo` runs; no other git.

## Files (relative to the worktree)
`submission/README.md`, `page.md`, `form.md`, `credits.md`, `ledger-proposals.md`, `package/README.txt`, `video/script-v2.md`, `devlog/index.html`; `devlog/curated/shots.txt`, `devlog/curated/img/accept-table-a91.{txt,png}`; `tools/run/package.ps1`, `tools/run/zipfwd.ps1`; `tools/media/{devlog-curated.mjs,table-still.sh,zipstep-test.ps1,ledger-fix.mjs}`; `ASSETS.md` (rows of A's own assets, the paragraph moved above the table); `status/A96.md`. Unchanged from A88: `shots/`, `tagline.md`, `video/hokej-v1.mp4`, `video/cutlist.md`, `video/takes/`, `tools/media/{take.ps1,cut.sh,clean-profile.ps1,credits.mjs}`.
