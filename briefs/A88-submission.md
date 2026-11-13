# Lane A88: the submission: video, page text, dev log, ledger, clean-profile test (device A, from H20, R-0043 point 4, Sonnet)

You are lane A88 of the Third Jam (team Hokej). The entry is due Sunday 22:00 CET (real deadline Monday 02:59) and until now no lane owned what is submitted. Your job: every
deliverable the rules ask for, built now from the latest package so that Sunday is a refresh and not a first attempt.

Read first: `docs/PLAN.md` section 2 (the rules' digest: a video 1920x1080 or higher of 30 to 60 s; a Windows zip under 1 GB named TeamName_GameName; sourced assets noted,
AI-generated content is not "sourced"; no trademarks or logos except the Unreal Engine logo: no Claude, Anthropic, NVIDIA, Meshy names or logos in the game or the video;
the Glass Box dev log submitted with the entry; one main prize and one modifier), sections 4b (size budget) and 8 (the modifier push), `docs/PLAN-RECONCILED.md`, R-0041
(the brand: raw and brutal within the teen rating; R-0021: no gore, mild words at most one in ten), `GAME.md` if present, `ASSETS.md` and `tools/gate/check-assets-ledger.mjs`,
`devlog/README.md` and `devlog/build.mjs` (the Glass Box timeline), `docs/experiments/STORYBOARD-H17.md` and `tools/qa/storyboard/**` (A48: photographing the packaged build),
`docs/experiments/TACKLING-2.md` and `tools/qa/ctrl/a73-*` (driving a packaged game to a knockdown; the remote console), `docs/experiments/MODES-2.md` (the twelve modes),
`docs/experiments/NIGHTLY.md` and `PARITY.md` (A57: what Shipping compiles out; the packages), `tools/size/**`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a88` (created at main 8fc69d3). The build you record and test: the newest `Game\Packaged\Hokej_Hokej_H*`
Shipping package in the main checkout (H24 now; H25 when it appears: redo the video then). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`;
never kill a process you did not start; no interactive editor. ffmpeg is on the path (`where ffmpeg`); record the game window with `gdigrab` at 1920x1080, 60 fps, with the
game's own audio if a capture device exists (say so if not: the video then needs audio added on Sunday). One game at a time; other lanes run games too: a capture with a
foreign window over it is thrown away.

You own: `submission/**` (new): `submission/README.md` (the checklist with a status per item), `submission/page.md` (the page text), `submission/video/` (the cut list, the
script, the takes, the 30 to 60 s cut), `submission/shots/` (eight stills), `submission/credits.md` (from the ledger), `tools/media/**` (record and cut scripts), `status/A88.md`.

## Job
1. `submission/README.md`: the rules' checklist, one row per deliverable with status, owner, what is missing; the zip's name by the rule (TeamName_GameName: Hokej_Hokej) and
   its size against 1 GB (H24: 272.5 MB); the modifier chosen in `docs/PLAN.md` section 8 and whether the entry still qualifies for it.
2. The video: a cut list for 30 to 60 s that sells the game the operator described (R-0041): a faceoff, a body check that knocks a skater down, a cross-check, a goal, the
   Director's choices menu, one mode (Bruisers, 5 v 5), the clay look; record takes from the package (the remote console and A73's drills get to a knockdown fast; a bot match
   gives goals), cut with ffmpeg, no logos, no trademark words on screen; `submission/video/hokej-v1.mp4` at 1080p. A second cut when H25 lands (the player can then shoot).
3. Eight stills at 1080p for the page from the same takes.
4. `submission/page.md`: the title, a two-line pitch, what the player does, the controls table as shipped (read `InputMapping.cpp`; the pass moves to Q and F becomes the
   ability when lane A84 lands: write both and mark which is live), the Director's modes, the tools used (text only, the rules' wording on AI-generated content), the team;
   the voice raw and rough, teen-rated.
5. The dev log: `node devlog/build.mjs` on the real data; the page it makes, its size, whether it redacts secrets and paths (the gate's devlog step prints "redacted");
   what is missing for a judge to follow the build.
6. The ledger: `node tools/gate/check-assets-ledger.mjs`; `submission/credits.md` listing every sourced row (origin not original and not ai-generated) with licence and URL;
   rows with an empty licence or URL are findings.
7. A clean-profile test of the newest Shipping zip: unzip to a temp folder outside the repo, run from there with a fresh `%LOCALAPPDATA%` save (rename the game's Saved
   folder for the run and restore it), 60 s alive, the menu reached, a match started through the remote console if Shipping allows; what a first-time machine would lack
   (the prerequisites installer in the package: present or not).
8. Refresh on every new H package until Saturday 22:00; `status/A88.md` says what changed.

Report: the checklist with statuses, the video path and its cut list, the findings (ledger, dev log, clean profile), the file list. Interim at 60 minutes with items 1, 5,
6, 7. Time box: three hours now, then the refreshes.
