# Submission checklist (lane A88, 2026-11-13 14:45 CET, build H24; refresh on every H package)

Rules digest: `docs/PLAN.md` section 2. Deadline Sunday 22:00 CET internal, Monday 02:59 real.

| # | Deliverable (rule) | Status | Owner | What is missing |
|---|---|---|---|---|
| 1 | Windows zip under 1 GB, named TeamName_GameName | **partly**: H24 zip is `Hokej_Hokej_H24.zip`, 285,702,127 bytes = 272.5 MiB (27 % of the limit), 441 MB unpacked. Final name `Hokej_Hokej.zip` (no H suffix) | Director | rename at packaging; top folder inside is `Windows/`, exe is `ThirdJam.exe` (project name, not Hokej); add a one-page readme in the zip (prerequisites below) |
| 2 | Video 30-60 s, 1080p+ | **v1 done**: `submission/video/hokej-v1.mp4`, 39.5 s, 1920x1080 30 fps, no brand words | A88 | no audio (no capture device); knockdown footage from a Development N1015 package, not H24; no cross-check, no human goal, no 5 v 5. Redo with H25 / H-final (`video/cutlist.md`) |
| 3 | Stills for the page | **done, 8 at 1080p**: `submission/shots/` | A88 | refresh from the final build |
| 4 | Page text | **drafted**: `submission/page.md` (controls table marks live vs planned: pass F -> Q, ability F) | A88 | team member names; re-read `InputMapping.cpp` on the final package; confirm the firewall sentence |
| 5 | Sourced assets noted; AI-generated not "sourced" | **done**: `submission/credits.md` | A88 | 1 sourced row ships (NVIDIA DLSS dll, licence named, url present); 9 sourced rows (not shipped, P4 retarget anims) have an empty url; 1 row (T_UI_RingPlain) has origin `generated`, not one of the three allowed origins; whether the in-game CREDITS screen lists the AI tools was not checked |
| 6 | No trademarks or logos (only the Unreal Engine logo) | video and stills clean by eye | A88 / Director | not verified: the game's CREDITS screen, the DLSS plugin name in the package, the Windows firewall dialog (shows publisher text "Hokej" and the engine icon) |
| 7 | Glass Box dev log with the entry | **built**: `submission/devlog/index.html` (173 KB, 111 sessions, 10,731 turns, redacted 0 secrets / 10 paths; grep for keys, user name and drive paths: clean) | A88 | 0 commits (this lane runs no git: rebuild with `--repo`), 0 screenshots (2,606 files, 1.1 GB: needs a curated `--shots` set; `--embed` of all overflows), device B transcripts (none found), entries thin before H4, hosting link |
| 8 | Modifier prize | **Glass Box** is the natural claim (log exists and builds); `docs/PLAN.md` section 8 planned Render Royalty and says the Director decides at H48: **no decision is recorded in the repo** | Director | decide; Glass Box qualifies if the log is submitted with the entry; Solo Driver does not apply (two people) |
| 9 | Clean-profile run of the zip | **PASS (H24)**, below | A88 | repeat on H-final |
| 10 | One main prize + one modifier | n/a | Director | |

## Clean-profile test of H24 (item 9)
Unzipped to `%LOCALAPPDATA%\Temp\a88\H24` (outside the repo); `%LOCALAPPDATA%\ThirdJam\Saved` renamed away, game run from the root `ThirdJam.exe`, folder restored (the fresh one kept as `Saved.a88-clean` beside it).
Menu on screen 5 s after launch; a posted Return started PLAY THE SEASON take 1; match alive 75 s, 1.4 GB working set, score 1:0 by 60 s with an idle human, no crash.
The fresh profile wrote only `Config/Windows/GameUserSettings.ini` (Difficulty=pro) and a CrashReportClient ini. Stills: `video/takes/clean-profile-H24/`.
What a first-time machine would meet or lack:
- a **Windows Firewall prompt** on first launch from a new path ("allow public and private networks to access Hokej"; seen over the game in an early take). The game opens a socket at start. Solo play after Cancel was not tested.
- the zip holds `Engine\Extras\Redist\en-us\vc_redist.x64.exe` but nothing runs or mentions it: add a readme and, if wanted, an `install-prereqs.bat`. Whether the game starts on a machine without the VC++ runtime cannot be tested here (this machine has it).
- D3D12 only; the DLSS dll ships and needs no setup.
- the menu hint line is clipped by the slate border and the menu still says SCENE 3 v 3 (stale against the 5 v 5 direction, R-0041).
Shipping cannot be asked anything: no log, no console, no remote console, no state hash (PARITY.md). "A match started through the remote console" is therefore not possible; a posted Return starts it.

## Findings
- **Ledger**: `node tools/gate/check-assets-ledger.mjs` PASS, 1,401 rows = 1,401 files, shipped device B art 93.4 MB, audio 41.5 MB, other 20.3 MB (budgets 350 / 80); 6 WARN size mismatches (`BP_SkaterVisual_*`, 1.6 vs about 2.5 MB). ASSETS.md as a whole: 1,654 rows (976 shipped): original 1,010, ai-generated 581, sourced 62, generated 1. Shipped: 523 original, 451 ai-generated, 1 sourced, 1 generated.
- **Dev log**: see row 7. It names the AI vendors in text (allowed: "text credits go in the dev log").
- **Capture**: no audio device; gdigrab is spoilt by any window in front (the firewall dialog was in front in the first take), so recording is PrintWindow-based, 27 ms per grab = 30 fps; posted keys and mouse buttons work in Shipping, the cursor cannot be posted.
- **Page**: GAME.md is stale (3 v 3, checks in Raw Cut only): not used for the page.
- A foreign game (A84's Development package) ran during some takes; my window sat at x=1500 and none overlapped.

## Files (relative to the worktree)
`submission/README.md`, `submission/page.md`, `submission/credits.md`, `submission/devlog/index.html`, `submission/shots/01..08*.png`, `submission/video/hokej-v1.mp4`, `submission/video/cutlist.md`, `submission/video/takes/*/take.log` and clean-profile stills,
`tools/media/{take.ps1,cut.sh,clean-profile.ps1,credits.mjs}`, `status/A88.md`.
