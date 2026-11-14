# Submission checklist (lane A88 first version Fri 14:45; A96 pass 2 Fri 17:55; A142 pass 3 Fri night; **lane A180 pass 4, Sat 14 Nov 08:00, main 9e452219 = the game of package H30**)

Rules digest: `docs/PLAN.md` section 2. Submit Sunday 22:00 internal; Monday 02:59 is the real deadline. The last day as a checklist with times and owners: **`submission/SUNDAY.md`**.
The text of the page, the form and the taglines is checked sentence by sentence in **`submission/TRUTH.md`** (pass 2: Friday's 96 rows and one new tagline re-read against this main: 55 true, 31 changed by the night,
5 false, 6 the operator's; plus 14 facts of this morning that must be findable and 9 things measured false that must not be said). The video's shots against what exists: **`submission/video/SHOTLIST.md`**.
Pass 4 ran no game and no build, and recorded nothing: it is reading, looking at stills, and writing.

## 1. The operator's questions, re-read on Saturday morning

He plays package H30 at 10:00 (`docs/PLAY-ME.md`). **Nothing in the submission's text assumes an answer**: each open question is a slot `[O<n>: ...]` in the file named, and the sentence round it is true without the answer.
Find every slot: `grep -n "[[]O[0-9]" submission/page.md submission/form.md submission/tagline.md`.

### 1a. The fourteen of Friday night

| # | Question | This morning | Where it bites now |
|---|---|---|---|
| O1 | **Team names and roles**, and how many people | **OPEN** | `page.md` Team, `form.md` Members |
| O2 | **Who presses submit, and when** | **OPEN** | `SUNDAY.md` (Sunday 22:00, the last line) |
| O3 | Which look a newcomer meets first, and is it clay | **ANSWERED BY THE NIGHT**: the first look is clay (R-0054 point 1), and H30 shows the clay rink with timber stands and a clay crowd from the first match (`docs/PLAY-ME.md` line 15). What is left is his eye's yes at 10:00 (the sheet's question 4). The ink outline and the spot pools stay off the page: no still of a package shows them | one slot in `page.md` "The rink"; `tagline.md` line 3 |
| O4 | **Main prize and the one modifier prize** (Glass Box is the claim the repository supports) | **OPEN** | `form.md` |
| O5 | **May the page name tool vendors? Is a Rodin or Tripo asset shipped?** | **OPEN** | `page.md` "How it was made", `form.md` tools |
| O6 | The "original" claim needs the asset ledger's gate on the tree that ships | **OPEN, and wider**: the night merged device B's stands, crowd, roof and walls, fourteen MetaSounds and three figure sets; `credits.md` and the gate's last run are Friday afternoon's. The Director regenerates `credits.md` and reruns `node tools/gate/check-assets-ledger.mjs` on the final tree (Sunday); the operator only has to accept the sentence the result allows | `page.md` "How it was made", `form.md` sourced assets |
| O7 | Test **Cancel on the firewall prompt** once on a clean profile | **OPEN, and moved**: in three fresh unzips on this machine (H27, H28, H29) no firewall prompt of ours appeared, so Cancel could not be pressed here (`status/A155.md`). Device B's clean-machine test of the zip is where it gets pressed | `page.md` limits and How to run |
| O8 | **Hosting link for the dev log**, and device B's transcripts | **OPEN**: device B waits for his word on exporting its transcripts (`docs/PLAY-ME.md` question 8) | `page.md` "How it was made", `form.md` |
| O9 | The same-direction shot: no drill stages it | **ANSWERED BY THE NIGHT**: `jam.ctrl.Drill 22` stages both halves, the nudge and the hit (lane A132; frames `shots/a155-06`, `-07`) | `video/SHOTLIST.md` shots 5 and 6 |
| O10 | May Development takes be in the video | **ANSWERED** by R-0054 point 4: only beside another Development take (a split frame), never cut against Shipping footage. What is left is in O17 | `video/SHOTLIST.md` |
| O11 | **The P4 removal** (149 rows, 100 MB, all unshipped): yes or no | **OPEN**; device B asks the same about 183 MB of unused figures in the cook (handoff B-to-A-0041) | the zip's size, the ledger |
| O12 | Can device B capture play with sound | **HALF ANSWERED**: a recipe exists and is not run (the system's game recorder on device B records the game's sound; ffmpeg there cannot; this machine has no capture device). Who records, and where: O17 | `video/SHOTLIST.md` section 4 |
| O13 | **The first two-machine match** | **OPEN**: two processes on one machine play to the final card with a rematch; two physical machines have never been tried (`docs/PLAY-ME.md` step 8) | `page.md` "Two machines" |
| O14 | The banner if the whistle's removal did not land | **ANSWERED BY THE NIGHT**: it landed. No foul in 8,600 measured matches of the regular cut, the large rink and the season; eleven of twelve modes; Glass House keeps the whistle on purpose. The game prints "Hockey with the whistle taken out." on its own first screen | the banner stands (`tagline.md`) |

**Still open: O1, O2, O4, O5, O6, O7, O8, O11, O13** (nine). **Answered by the night: O3 (but for his eye), O9, O10, O14** (four). **Half: O12** (folded into O17).

### 1b. New questions the night raised

| # | Question | Why it is his | Where it bites |
|---|---|---|---|
| O15 | **Which package is submitted: H30 as he played it, or a later one?** Waiting for his word, each built and measured (`docs/EVOLUTION.md`, the board of 08:20; `docs/PLAY-ME.md` questions 2, 3, 3a): pro's dial (pro at its bar; everywhere, or pinned to the regular game and the season); the check's cooldown; the lunge; the hit that is a stumble as you win the puck; the push-out round a downed skater; the marks made to read; the modes' re-tune; the arena's seventh evolution. Rule changes freeze Saturday 19:00, content Saturday 22:00 | each one changes what is true: see the list under this table | `page.md` "Known limits" and "How to run"; `form.md` Zip; every "about" on the page |
| O16 | **Does the booth speak?** 412 lines are recorded; whether they are heard in the package depends on device B's audio at that commit (`docs/PLAY-ME.md` line 96). Also: did device B's goal effect show | only an ear in front of the package can say; the page promises captions only until then | `page.md` "Sound" |
| O17 | **The film**: which scenario (device B ranks A, "The Director's Chair", first, with a 30 s cut and scenario C as the fallback); **who records it and on which machine** (it needs the game's sound and a hand on the mouse; about 30 to 45 minutes); the narration lines (B's seven proposals put the word "cut" on the ice, which the voice guide keeps for the menu: allowed in a trailer, or the copy lines of `video/SHOTLIST.md`); B's excited or dry delivery; a second voice or the commentator alone; whether a split frame of two Development drills may sit inside a Shipping film; the engine's logo on the end card (the rule allows it, nobody has the file: leave it out unless he wants it) | copy and tone are ours, the choice and the hands are his | `video/SHOTLIST.md`, `form.md` Video, `SUNDAY.md` |
| O18 | **Which three modes lead**, if only three could (`docs/PLAY-ME.md` question 5) | the page lists twelve as the menu does; the film has room for one (today: Shotgun, if its blast reads on screen) | `video/SHOTLIST.md` shot 11 |
| O19 | **Does the last day add a verb** from device B's new sets (a dash, a slide tackle, a dive block, toy weapons)? None is wired | a new verb is a new row in the controls table, on the controls screen and in the truth table, and a new shot; "none" keeps today's text final | `page.md` Controls |
| O20 | **The page's five pictures.** The twelve frames of the real window taken in the night are of H27 (no stands, no crowd, the old cards): one is page-worthy, three are fallbacks (`video/SHOTLIST.md` section 1). Five are to be retaken from the submitted package, or pulled as frames from his own recording | which five he wants the page to open with; the Director's window lane takes them | the page's pictures, `shots/` |
| O21 | **The zip to device B for the clean-machine test**: device B waits for it (`docs/PLAY-ME.md` question 8) | his word starts it; it also settles O7 and can carry O13 | `SUNDAY.md` |

**What each answer to O15 does to the text** (so the page can be finished in ten minutes after he speaks):
- Pro's dial, either patch: nothing. The page says "about even", never a number.
- The check's cooldown on: delete the limit "Hammering the right button lands more checks than timing it."
- The hit as you win the puck is a stumble: soften the limit "At pro you spend real time on the ice"; and "the speed you bring is the hit you get" then has one stated exception (a quarter second after anyone wins the puck), which the page must say in the same breath.
- The push-out on: delete the limit "A skater can stand at a downed body and push it along the ice."
- The marks made to read: the "Two marks" paragraph stands; "thickens" is truer.
- The modes' re-tune: re-read four mode lines against the new cards (Sniper becomes five against four, Bruisers moves to a smaller rink) and delete the limit about Bruisers and Cage.
- A new verb (O19): one row in Controls.
- Any later package: "How to run" and the form's Zip row take its name; the truth table's header takes its commit.

## 2. What is done (true of main this morning, by the truth table)
- `TRUTH.md` pass 2: Friday's rows re-read with a source line each; the corrected sentences; the facts that must be findable (the level switch, one stop distance, hits by closing speed, no whistle outside one mode, Space and the locked stick, nothing drawn on a downed player, the two marks and "an offer, not an order", the twelve modes as their cards print them, the clay rink with stands and a crowd, the season's eight takes and its ramp, the joined match and exactly what was never tried).
- `page.md` v4: three lengths (the blurb counted at 60 words), no `[waits: ...]` tag, no win rate, the limits rewritten from the play sheet and REVIEW-3, ten slots. `form.md` pass 4. `tagline.md`: the banner is true; a list of lines not to write.
- `video/SHOTLIST.md`: the twelve stills judged one by one; the thirteen shots of a 45 s film with what exists and what must be recorded (twelve of thirteen).
- `SUNDAY.md`: the last day counted back from Sunday 22:00.
- `package/README.txt` (the text file that ships beside the exe): eight lines made true of this morning (PLAY NOW and the three levels, the menu's name BRAWL WITH A FRIEND, no whistle but in one mode, the nudge at matched speed, the stop from five stick lengths, Space at one in ten, the pad not play-tested); the claim "Cancel is fine" is replaced by "solo play does not need network access" until device B's test (O7). It changes the zip's contents by one text file: the Director takes it with the next package or leaves it.
- Not touched by this pass, and stale in places: `credits.md` (Friday's tree), `video/script-v2.md` (its shot table is superseded by `video/SHOTLIST.md`; its staging notes for Shipping and Development still hold), `devlog/index.html` (rebuilt Sunday).

## 3. What still waits, and for whom
| Waits for | What |
|---|---|
| the operator at 10:00 | O1, O2, O4, O5, O8, O11, O13 and O15 to O21 (section 1) |
| the Director, after his word | the submitted package cut and named (`package.ps1 -Submission`), unzipped and checked; `credits.md` regenerated and the ledger gate rerun on that tree (O6); `package/README.txt` re-read once more against the final controls screen; the five page pictures (O20); the dev log rebuilt |
| device B | the clean-machine test of the zip (O7, O21); the recording with sound if he records there (O17); the cut by `edit.mjs`; its transcripts if he says yes (O8); a match between two machines if both are at hand (O13) |
| this track's next lane | the page finished from his answers (ten minutes: the list under O15), the slots stripped, the film's captions checked against the takes, `SUNDAY.md` ticked |

## 4. If an answer does not come
A slot whose question is not answered by Sunday 18:00 is removed **with its sentence's claim**, never filled: O13 leaves "Two physical machines have never been tried." as the last word; O16 leaves the captions; O6 unanswered removes the word "Original" and its list; O5 unanswered leaves the vendors unnamed on the page (they are named in `credits.md` and the form); O7 leaves "not tested". O1, O2 and O4 have no fallback: the form cannot be sent without them.

## 5. What only the operator can do
O1, O2, O4, O5, O8, O13, O15, O17 (names, submit, prize, vendor policy, hosting, the first two-machine match, which package, which film and whose hands), plus the final upload and the confirmation screenshot (`docs/SUBMISSION.md` checklist).

---

*Below: the record of passes 1 to 3 (Thursday and Friday), kept as written. Package and lane names in it are of those days.*

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

## Files added or changed by lane A180 (pass 4)
`submission/TRUTH.md` (pass 2), `submission/page.md` (v4), `submission/form.md`, `submission/tagline.md`, `submission/README.md` (sections 1 to 5), `submission/video/SHOTLIST.md` (new), `submission/SUNDAY.md` (new), `submission/package/README.txt` (eight lines), `submission/video/script-v2.md` (one pointer line under the title), `status/A180.md`. Not touched: `credits.md`, `ledger-proposals.md`, `devlog/`, `shots/`, `video/cutlist.md`, `video/*.mp4`, `video/takes/`, `docs/SUBMISSION.md`.
