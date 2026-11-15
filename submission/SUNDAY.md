# The last day: writing, recording and sending, counted back from Sunday 15 Nov 22:00 CET (lane A180, track D1; a proposal for the Director, written Sat 14 Nov 08:00)

Submit at **Sunday 22:00**. The jam's real deadline is Monday 02:59: that is five hours of reserve for an upload that fails, not for work.
Freezes already ruled (R-0047): rule changes land until **Saturday 19:00**, content until **Saturday 22:00**. Sunday changes words, pictures and the film, never the game.
Owners: **O** = the operator, **D** = the Director (device A and its lanes), **B** = device B. "T-n" = hours before the submit.
One rule runs through the whole list: **the film, the pictures and the page describe the package that is uploaded**, so the package comes first and everything else is made from it.

> **Lane A225, Sun 15 Nov 16:40: the plan below has slipped.** The candidate is **H43** (main `a12ead1b`, cut 16:08, acceptance run, menu row and ship row green; **without the DLSS plugin**, whose restore waits for the operator's word). The words were checked against it (`TRUTH.md` pass 5, ready by 17:30). The recording starts about 18:00, the answers cut-off stays **18:00**, the page is final at 19:00, the submit at 22:00. If DLSS comes back, the package is re-cut, the four DLSS sentences of `TRUTH.md` section 5.6 switch to their "with" version, and the film should be recorded on that package, not on H43. `README.txt` inside H43 is the text before pass 5: the re-cut, or a replaced file in the zip, carries the new one.

## Saturday (the game closes)

| When | T- | What | Owner | Waits on |
|---|---|---|---|---|
| Sat 10:00 | 36 | Play H30 with `docs/PLAY-ME.md`; answer its nine points and `submission/README.md` section 1 (O1 to O21). The ones that block others: **O15** (which package), **O17** (which film, whose hands), **O19** (a new verb or none), **O21** (the zip to device B) | O | nothing |
| Sat 11:00 | 35 | The page finished from his answers: the list under O15 in `submission/README.md` (ten minutes of edits), `TRUTH.md` header moved to the new commit if a later package is chosen | D (this track) | O15, O16, O19 |
| Sat 19:00 | 27 | Rule freeze: the last sim change is on main with its gate green | D | O15's yes or no to each waiting rule |
| Sat 22:00 | 24 | Content freeze: cartridges, cards, modes, the arena | D, B | B's last push merged and timed |
| Sat 23:30 | 22.5 | **The submission candidate**: `tools/run/package.ps1 -Submission` on the frozen main: acceptance run green, the frame photographed (not black), the zip unzipped into a fresh folder and played through the window, `README.txt` and the entry names checked, its line in `status/A.md` | D | both freezes; O11 (the P4 removal changes the zip's size) |

## Sunday (words, pictures, the film, the upload)

| When | T- | What | Owner | Waits on |
|---|---|---|---|---|
| Sun 09:00 | 13 | The candidate zip is on device B | O, D | the candidate; O21 |
| Sun 09:00 to 11:00 | 13 to 11 | **Clean-machine test on device B**: unzip into a new folder; first launch (seconds to the menu); the firewall prompt answered **Cancel**, then one solo match to its end card; a second launch answered **Allow**, then the host screen of BRAWL WITH A FRIEND shows an invite code; whether the game starts without running the redistributable; the booth heard or not. One line per step in B's status file. A failure here is the only reason to re-cut the package on Sunday | B | the zip |
| Sun 09:30 to 10:30 | 12.5 to 11.5 | The page's five pictures from the candidate, 1920 x 1080: the first screen with ROOKIE lit, the faceoff with the stands, a hit with a body down, the Director's Call over a clay match, the goal lamp (`video/SHOTLIST.md` section 1). Or pulled as frames from the recording below | D (the window lane; it takes the cursor) | the candidate; O20 |
| Sun 10:00 to 12:00 | 12 to 10 | **The video recorded**: one continuous session of 4 to 8 minutes on the candidate (Shipping), by hand, in the order of `video/SHOTLIST.md` section 3; pass 1 with the booth's voice off, pass 2 with it on; each take checked by B's `check-take.mjs` (1920 x 1080, 60 fps, sound present). The machine must record the game's sound: device B with its recipe, or his own if it can | O (the hands), B (the recorder and the check) | the candidate; O17; the clean-machine test passed on that machine |
| Sun 12:00 to 13:00 | 10 to 9 | A match between two physical machines, if both are in one room: `docs/experiments/JOIN-GUIDE.md`. Its result is one sentence on the page; if it is not played, the page already says "never been tried" | O with B | two machines on one network; optional |
| Sun 12:00 to 14:00 | 10 to 8 | **The video cut by B's one command**: the takes' `src`, `in`, `out` set in `A45.json` (or `A30.json`, `C45.json`), `edit.mjs` run on it; the last line reads length_ok true, 1920x1080@60, overruns 0, check PASS. Then a person watches it once with sound | B | the takes; the narration or caption lines (O17) |
| Sun 14:00 | 8 | The film's words checked against `TRUTH.md` N15 to N23 (no caption says the puck sticks, Space on demand, the line as an order, a win rate, a horn); no logo, no vendor or engine name on screen; the theme visible in the first five seconds; 30 to 60 s | D (this track) | the cut |
| Sun 14:00 to 15:00 | 8 to 7 | `credits.md` regenerated and the ledger gate rerun on the candidate's tree (O6); `package/README.txt` re-read against the controls screen; the dev log rebuilt (`node tools/media/devlog-curated.mjs --repo <main checkout>`) with B's transcripts if he said yes (O8), and placed where O8 says it is hosted | D, B | O6, O8 |
| Sun 18:00 | 4 | **Answers cut-off.** A slot still unanswered is removed with its claim, never filled (`submission/README.md` section 4). O1, O2 and O4 have no fallback | O | his answers |
| Sun 19:00 | 3 | **The page is final**: `grep -n "[[]O[0-9]" submission/page.md submission/form.md submission/tagline.md` returns nothing; the limits re-read against the candidate and against B's test; the pictures in place | D (this track) | the cut-off, the test, the pictures |
| Sun 20:00 | 2 | **The form filled** from `form.md` and saved as a draft; the zip and the video uploaded (the zip is about 270 MB: start it here, not at 21:55) | O, with D reading along | the page, the film, the zip, O1, O4, O5 |
| Sun 21:00 | 1 | The published preview read once from top to bottom on a machine that is not the one that wrote it; the uploaded zip downloaded back and its size compared with the candidate's | D, B | the upload |
| **Sun 22:00** | **0** | **Submit.** The confirmation screen photographed; the link and the time written into `status/A.md` | **O** (O2) | everything above |
| Mon 02:59 | | The jam's deadline. Used only if an upload failed at 22:00 | O | |

## What can slip, and what cannot

- **Cannot slip:** the candidate on Saturday night (everything on Sunday is made from it); the clean-machine test before the recording (a film of a package that does not start elsewhere is a film of the wrong game); the answers cut-off.
- **Can slip to its fallback:** the film (scenario C needs one recorded match; the 30 s cut needs eight shots; the last resort is a silent cut with the game's own music laid under it, and then "sound from the game" is not claimed); the five pictures (frames from the recording); the two-machine match (the page already says what was never tried); B's transcripts (the dev log says whose sessions it holds).
- **If the candidate must be re-cut on Sunday:** only for a start-up fault found on device B; the recording and the pictures are then repeated on the new one, and the day loses about four hours: the reserve after 22:00 is for exactly this.
