# Submission checklist (lane A88 first version 14:45; lane A96 pass 2 17:55; lane A142 pass 3, Fri 13 Nov night, main 41df3e6)

Rules digest: `docs/PLAN.md` section 2. Submit Sunday 22:00 internal; Monday 02:59 is the real deadline. The text of the page, the form, the tagline and the video script is now
checked sentence by sentence in **`submission/TRUTH.md`** (96 rows: before 47 true, 16 false, 30 unproven, 3 n/a; after 66 true of main, 20 waiting for a lane, 10 a marked question).
Pass 3 ran no game and no build: it is reading and writing text.

## 1. The operator's questions, in the order he will meet them
Each is a blank or a choice in the text: the file and the mark are named. Nothing below was invented to fill a gap.

| # | Question | Where it bites | Why the repository cannot say |
|---|---|---|---|
| O1 | **Team names and roles** (and how many people: two devices worked on it) | `page.md` Team, `form.md` Members, `docs/SUBMISSION.md` | his to give; the old skeleton said 1 |
| O2 | **Who presses submit, and when** (Sunday 22:00 internal) | the form | his |
| O3 | **Which look does a newcomer meet first, and is it clay?** The regular cut is the night tier (`l2_night`); six modes are `p3_clay`; R-0041 point 5 calls P3 clay the shipped look. Also: may the page claim an ink outline and spot pools? Needs a still of the package at 1080p | `page.md` "Look and sound", taglines 1 and 2 | no document shows the packaged first screen; B owns the look |
| O4 | **Main prize and the one modifier prize.** Glass Box is the one the repository supports (the log exists and builds); the plan said Render Royalty; Solo Driver does not apply to two people | `form.md` | no decision recorded |
| O5 | **May the page name tool vendors?** The page says "an image-generation model, 3D-generation services, a voice-and-audio service"; `form.md` "Tools you used" names them. **Is a Rodin or Tripo asset shipped?** (`docs/SUBMISSION.md`: list Rodin only if the ledger shows a shipped Rodin asset; the P4 rows in `credits.md` are Epic clips, `shipped` n) | `page.md`, `form.md` | policy and a ledger fact |
| O6 | **The "original" claim for the arena pieces** (benches, boxes, goal frames, goal light, scorer's table: A's Blender work) needs their ledger rows: run `node tools/gate/check-assets-ledger.mjs` on the tree that ships | `page.md` "How it was made" | the gate was last run on A96's tree |
| O7 | **Test Cancel on the firewall prompt** (solo play after Cancel), once, on a clean profile | `page.md` limits and How to run, `package/README.txt` | "only claimed, never tested" (clean-profile section below) |
| O8 | **Hosting link for the dev log** (a 7.8 MB self-contained page; rebuild Sunday), and whether device B's transcripts exist | `form.md` | his; B's transcripts: none found |
| O9 | **The same-direction shot** (shot 5 of the video): no drill stages it. Play it by hand, have a lane write a drill, or drop the split frame | `video/script-v2.md` | no console drill; drill 5 is a stumble |
| O10 | **Development takes in the video**: drills (shots 5 to 7, 9) exist only in a Development package, whose ice measured darker than Shipping's. Allow them, or play by hand | `video/script-v2.md` | A88's measurement |
| O11 | **The P4 removal** (`ledger-proposals.md`: 149 rows, 100 MB of A's files, all `shipped` n): yes or no | the zip's size and the ledger | a proposal for the Director |
| O12 | **Audio**: can device B capture the final 30 to 60 s of play with sound from the next package (A-to-B-0020 point 5)? Otherwise a silent cut with the game's own commentator and music muxed in | `video/script-v2.md` | open since Friday |
| O13 | **The first two-machine match**: the joined match is tested as two processes on one machine through a match and a rematch; two real machines and a real firewall prompt are the operator's (with B). Send the result and the page's two-machine sentence is finished | `page.md` "Two machines" | `JOIN-GUIDE.md` section 5 |
| O14 | **The banner if the curve lane does not land**: "Hockey with the whistle taken out" needs the whistle out of the regular cut and the takes (A133). Fallback: "Real hits. The speed you bring is the hit you get." Say which he prefers if it comes to that | `tagline.md`, `page.md` | R-0048 point 2 waits on A133 |

## 2. What is done (true of main today, by the truth table)
- `page.md` v3: three lengths (pitch 13 words, blurb exactly 60), the first minute, the three things no other hockey game says, the bots, the twelve cuts and what the Director's Call really is (one dial, once a period), the joined match with its limit, the arena that answers a goal, the controls table (15 rows, matches `InputMapping.cpp`), the tools in plain words, known limits said plainly.
- `form.md` re-checked (the video row no longer points at a file that does not exist; the prizes are the operator's); `docs/SUBMISSION.md` skeleton no longer says 1 member or "3v3".
- `video/script-v2.md` names for each shot the drill, console command or cut that stages it today (`jam.ctrl.Drill 21` with `jam.ctrl.CarryScript 1` for the stop-and-go: it exists now, the old script said to write it), and what device B captures with sound.
- Not touched, still right at A96's tree: `credits.md` (62 sourced rows, 1 shipped), `package/README.txt`, `devlog/index.html`, `shots/` (the 8 stills are of H24).

## 3. What waits for the next package or a lane (each is tagged `[waits: ...]` in the text)
| Mark | Sentence in the text | When it is true |
|---|---|---|
| A133 | "Hockey with the whistle taken out" (page, tagline 1, the video's end card); take-3 at rookie; "the harder the level, the more it hits, the top one is faster"; Sudden Death "against the top level"; "the ladder is being re-set" | the curve lane lands: the whistle off in `hockey.regular` and the takes (R-0048 point 2), the ladder re-set |
| A138 | the brain brakes on purpose, a bot carrier sprints, bots read your shield; the shove under a shield | the integration lane lands the brain's second evolution and the shove rule (R-0053 points 2 and 3) |
| A135 | the brake prompt on the ice; the clipped hint line | the screen lane lands (camera, prompts) |
| A132 | the screen shows a knockdown better than a bump (a limit, not a promise) | the hit's size on screen lands |
| A139 | Fog of War "slow" bots removed from the card; the modes' second evolution | the modes lane lands |
| device B BF1 | the brake's spray and pose (video shot 6) | B delivers |
| device B BF2 | a crowd (the page says there is none yet) | B delivers |
| device B BF3 | the 72 contact lines with a voice; the ability's burst and sound (shot 7) | B delivers (R-0051 point 2) |
| device B BS1 | the dev log's second half (B's transcripts); the video with sound | B delivers |
| the Director | `Hokej_Hokej.zip` from a real `package.ps1 -Submission` run on the final tree, unzipped and checked (`README.txt`, entry names with `/`); the clean-profile run repeated on it; the next package (H27) for every video shot | after A133 and A138 (R-0053 point 6) |
| capture lane | the stale "3 v 3" menu line checked in the take; five skaters a side counted in the take; the stills refreshed from the next package (the 8 in `shots/` are H24; the design eye asks for four more, `REVIEW-1.md` section 7) | after the next package |

## 4. If a lane does not land (so Sunday has no surprises)
Delete the tagged sentences and use the fallback beside them. A133 not landed: banner "Real hits. The speed you bring is the hit you get."; drop "Hockey with the whistle taken out" and the level sentences. A138: drop the one tagged sentence in "The bots". A135 and A132: the limits stay as they read. Device B items: the limits already say they are missing. The page is true of the package that ships when every `[waits: ...]` is stripped or its sentence deleted: find them with `grep -n "waits:" submission/page.md submission/form.md submission/video/script-v2.md`.

## 5. What only the operator can do
O1, O2, O4, O5, O8, O13 (names, submit, prize, vendor policy, hosting, the first two-machine match), plus the final upload and the confirmation screenshot (`docs/SUBMISSION.md` checklist).

## The asset ledger (item 5), after pass 2
`node tools/gate/check-assets-ledger.mjs` **PASS**, 1,654 rows (was parsed as 1,401: a paragraph inside the table ended it, so 253 rows of device A's `Game/Content/**` were never validated; moved above the table). Fixed: `T_UI_RingPlain` origin `original` (a PowerShell-drawn circle, no generative model), nine P4 sourced animation rows now carry licence and url, twelve `AS_P4_*` size_mb rows updated. Six WARN left, all device B's (`BP_SkaterVisual_*`, listed in `ledger-proposals.md`), and one for device A's `vfx/font/other` 76.5 MB against a 40 MB slice, which is the 56.2 MB DLSS library alone. P4 removal (149 rows, 100 MB of A's files, all `shipped` = n) is a proposal for the Director, nothing deleted.

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

## Files added or changed by lane A142 (pass 3)
`submission/TRUTH.md` (new), `submission/page.md`, `submission/form.md`, `submission/tagline.md`, `submission/README.md`, `submission/video/script-v2.md`, `submission/video/cutlist.md` (header), `docs/SUBMISSION.md`, `status/A142.md`. The repository `README.md` describes the prep repository, not the game: unchanged.
