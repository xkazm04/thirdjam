# Lane A54: the eyes regression grid of the packaged build (device A, from H13.3, night filler 2 of 11, Sonnet)

You are lane A54 of the Third Jam (team Hokej). Photograph the shipped build from every angle a player sees and let the eyes tool (Gemini) judge every still, so the
morning starts with a ranked table of what looks wrong.

Read first: `tools/eyes/README.md` (presets `readability`, `hud`, `bug-hunt`, `p2-bar`, `figure`; `node tools/eyes/eyes.mjs --still <png> --ask <preset> --json <out>`),
`docs/experiments/STORYBOARD-H17.md` and `tools/qa/storyboard/**` (A48: how the packaged build was launched and photographed, which console commands survive a package),
`docs/experiments/CONTROLS.md` (A49's packaged Development build and SendInput harness in `tools/qa/ctrl/`), `Game/Content/Cartridges/*.cartridge.json` (the cuts),
`docs/experiments/CUTS.md`, `docs/experiments/EYES.md`, `Game/Source/JamGame/Private/HokejGameMode.cpp` (the `jam.*` console commands and their `UE_BUILD_SHIPPING` guards).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a54` (created at main a6b6f5b). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`
(the eyes tool reads the key itself); never kill a process you did not start (the games you launch are yours). Open no interactive editor: use the packaged build
`Game\Packaged\Hokej_Hokej_H19\Windows\ThirdJam.exe` (Shipping) and, where a console command is compiled out of Shipping, a Development game package you build in your
worktree (`tools/run/package.ps1 -Config Development`, say which build each still comes from). One game at a time, 1080p windowed. Gemini calls: at most 400 tonight.

You own: `tools/qa/eyesgrid/**`, `docs/experiments/EYES-GRID.md`, `docs/experiments/img/eyesgrid/**`, `docs/experiments/eyes/a54-*.json`, `status/A54.md`.

## Job
1. `tools/qa/eyesgrid/grid.ps1` (or `.mjs`): for each cut (`hockey.regular`, `hockey.boost`, and every cut the Designer library lists) x each look tier the cut allows x
   three cameras (game camera at a faceoff, at a goal, the far net from the end boards) plus the menu, the pause menu, the season screen and a Director's Call card:
   launch, drive the scripted human or the playtester to the moment, take a 1080p still, close. Name stills `<cut>-<tier>-<moment>.png`.
2. Eyes on every still: `bug-hunt` on all, `readability` on game-camera stills, `hud` on stills with the HUD; parse severities into one table (still, preset, severity,
   the finding in ten words).
3. `EYES-GRID.md`: the table sorted by severity, the ten worst stills inline, the findings deduplicated across cuts (the same wrong thing in six cuts is one row with six
   stills), and a line per finding naming the owning area (HUD, rink, figure, camera, menu) so the Director can route it.
4. Re-run the grid when main moves: a loop every 90 minutes checks `git -C C:\Users\kazda\kiro\thirdjam log -1 --format=%h main` (reading the ref is allowed; no other
   git) and, when a new package `Game\Packaged\Hokej_Hokej_H2*` exists, repeats the grid on it and appends a "what changed since the last grid" section.
5. Stop at 06:30; `status/A54.md` says how many stills and calls.

Report: the ten worst findings with still paths, the counts, the file list. Interim at 60 minutes with the first full grid. Time box: until 06:30.
