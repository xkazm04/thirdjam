# Lane A63: the highlight reel tool (device A, night filler, Sonnet)

You are lane A63 of the Third Jam (team Hokej). The jam page and the devlog need moving pictures of the game. Your job: a tool that records bot matches in the packaged
build, cuts goals, checks and saves into short clips and one 60-second reel, and runs again on every later package.

Read first: `devlog/README.md` (shot naming, what the devlog wants), `docs/experiments/STORYBOARD-H17.md` and `tools/qa/storyboard/**` (A48: launching and photographing the
packaged build, which commands survive Shipping), `docs/experiments/CONTROLS.md` (A49's Development package and harness), `Game/Source/JamGame/Private/HokejGameMode.cpp` (the
match log lines for goal, check, save, post, knockdown with their frame numbers: the cut list comes from the log), `tools/run/run-packaged.ps1`, ffmpeg (on the path; `tl doctor`
in the sibling repo found it; else `where ffmpeg`), Windows Game Bar is not scriptable: record with ffmpeg `gdigrab` of the game window or the engine's own
`MovieSceneCapture` / `-MovieCapture` if the packaged build allows (say which).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a63` (created at the current main). The build you record: `Game\Packaged\Hokej_Hokej_H19\Windows\ThirdJam.exe`
first, then any newer `Hokej_Hokej_H2*` or the nightly `Hokej_Hokej_N*` package when it appears. Run no git command; never touch `Game/Plugins/JamAssets`; never print
`.env`; never kill a process you did not start. Open no interactive editor. One game at a time, 1920x1080 windowed, 60 fps capture; stop recording when free memory is under 10 GB.

You own: `tools/media/highlights/**`, `devlog/highlights/**` (clips and reels; keep under 200 MB total: 1080p, 6 Mbit/s, 10-15 s clips), `docs/experiments/HIGHLIGHTS.md`,
`status/A63.md`.

## Job
1. `record.ps1`: launch the build into a bot match (`hockey.regular` pro, and `hockey.boost` once), record the window with ffmpeg for a full match, and keep the log with
   frame numbers aligned to the recording's start (a visible frame counter or a log timestamp at record start: state the alignment method and its error in frames).
2. `cut.mjs`: from the log, a cut list: goals (6 s before, 4 s after), checks that strip the puck, saves, posts, knockdowns; each clip named `<event>-<frame>-<cut>.mp4`;
   a 60-second reel of the best twelve moments with a 0.5 s cross-fade, no music (B's audio is in the game sound; keep the game audio).
3. Three animated GIFs (720p, under 8 MB each) of the three best moments for the devlog.
4. `HIGHLIGHTS.md`: how to run it on any package in two commands, the clip list with what each shows, what a first-time viewer notices first (your words), and three things
   the recording shows the Director should fix (ranked).
5. If a newer package appears before 06:00, record it too and note what changed on screen.

Report: the reel path, the clip list, the three things to fix, the file list. Interim at 60 minutes with the first reel. Time box 120 minutes.
