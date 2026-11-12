# Lane A48: the shipped build, played and photographed: a storyboard of H17 for the operator (device A, from H12.6, Fable 5.2)

You are lane A48 of the Third Jam (team Hokej). The operator returns at about 22:45 and must see what the shipped build does, not what PIE does. Read first: `docs/experiments/
FEEL.md` (what the operator felt at H7), `docs/experiments/UI.md` section 7 (HUD pass 2), `docs/experiments/PAYOFFS.md`, `docs/experiments/TACKLING.md`, `docs/experiments/CAMERA.md`,
`docs/experiments/COMMENTARY.md`, `tools/run/run-packaged.ps1` and `tools/qa/` (how a packaged build is launched and stays up), `Game/Source/JamCore/AI/JamPlaytester.cpp` (`-player=`),
`tools/eyes/README.md` (presets `readability`, `hud`, `bug-hunt`, `p2-bar`), `devlog/README.md` (shot naming), R-0021 point 3, R-0025 point 1, R-0028, R-0030.

The build: `C:\Users\kazda\kiro\thirdjam\Game\Packaged\Hokej_Hokej_H17\` (the staged folder next to the zip; the Shipping exe inside `Windows\`). Launch it with the console
unlocked if the Shipping build allows (`-log` and the `jam.*` commands that survive Shipping: check `HokejGameMode.cpp` for `UE_BUILD_SHIPPING` guards; if `jam.ScriptHuman` or the
playtester is compiled out of Shipping, say so and use the Development build from `tools/run/build.ps1 -Target Game -Config Development` in your worktree instead, stating which
build each still comes from). No editor. Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a48` (the Director creates it at the current main). Run no git command;
never kill a process you did not start (the game you launch is yours).

You own: `tools/qa/storyboard/**` (the launch and capture scripts), `docs/experiments/STORYBOARD-H17.md`, `docs/experiments/img/storyboard/**`, `docs/experiments/eyes/a48-*.json`,
`status/A48.md`.

## Job
1. Play the shipped build with the playtester as the human (`-player=regular` if it survives Shipping; else the scripted human) on `hockey.regular` at rookie for a full match,
   capturing a 1080p still every 2 seconds (`HighResShot` or the game's screenshot command) plus the log; then the same at pro for two minutes; then the menu, a cut change through
   the Designer's offline library (`jam.playcut hockey.boost`), the season screen, a Director's Call, the pause menu, the settings.
2. Pick 16 stills that tell the match as a first-time player sees it: menu, match card, faceoff HOLD/DROP, the YOU ring, a carry, a pass received, a shot, a goal with the
   commentator line, a check landing with the camera punch, a knock-down and get-up, a pickup, a Director's Call card, the far net from the end boards, the win or loss card, the
   season screen. Caption each with the frame, the sim event behind it, and what the player should notice.
3. Eyes on every chosen still: `readability` at the game camera, `hud`, `bug-hunt`; one `p2-bar` on the best play still. Table of verdicts; anything severity 4 or more is a finding.
4. Measure from the log: match length, goals, the human's touches, shots, passes received, one-timers, checks landed and taken, barks and commentary lines, frame time
   (`stat unit` if available, else the log's frame counters), any `JAMFX missing`, `missing usage flag`, `LoadErrors`, ensure or warning lines.
5. `docs/experiments/STORYBOARD-H17.md`: the 16 captioned stills in order, the eyes table, the log measurements, five findings ranked by what the operator should fix first, and
   the three things that work best. The Director turns it into the operator's page.

Report: the still paths, the five findings, the three strengths, the measurements. Time box 75 minutes.
