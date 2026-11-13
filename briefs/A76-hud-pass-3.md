# Lane A76: HUD pass 3: what the eyes grid and the marker lane left (device A, from H16, Sonnet)

You are lane A76 of the Third Jam (team Hokej). Three HUD rows are still open after lanes A64 and A74: the far-net distance diamond sits on the far crossbar (`hud` rows
"badge covers the crossbar" 3 of 3 in `docs/experiments/PUCK-MARKER.md`), the puck marker can stand beside the "own 46 m" label and a skater on the left edge, and the
match card still covers the far centre on the large and boosted rinks (A64 row 6, partial). Fix the three, prove them with A64's and A74's drivers, and leave the HUD's
rules written in one place.

Read first: `docs/experiments/EYES-GRID-FIXES.md` (rows 2, 6, 10), `PUCK-MARKER.md`, `UI.md` section 7 (HUD pass 2: the YOU ring, the diamonds, the radar, the controls
strip), `Game/Source/JamGame/Private/HUDWidget.cpp` + `.h` (`UpdatePuckMarker`, the net-distance diamonds, the label layout), `DirectorsCallWidget.cpp` (the match card),
`tools/qa/eyesgrid/a74grid.mjs`, `a74eyes.mjs`, `a74score.mjs`, `cursor.ps1`, `shoot.ps1` (A74 and A64's drivers; note the launcher stub: the game is
`Windows/ThirdJam/Binaries/Win64/ThirdJam.exe`; tier names are `l2_night`, `l3_neon`), `tools/eyes/README.md`, R-0021 point 3, R-0038.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a76` (created at the current main; build it yourself, unity off; one editor on MCP port 8037 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env` (copy the main checkout's `.env` into the
worktree for the eyes tool without printing it; say so). Presentation only: the sim hash and the input path never change; `CameraFollow.cpp` is not yours.

You own: `HUDWidget.*`, `DirectorsCallWidget.*` (the card's placement rule only), `docs/experiments/UI.md` (a section 8: HUD pass 3 and the one table of every HUD element
with its anchor, its size at 1080p and 720p, what hides it), `docs/experiments/img/a76/**`, `docs/experiments/eyes/a76-*.json`, `status/A76.md`.

## Job
1. **The far-net diamond** never covers the net: project the world point 1.2 m above the crossbar's centre and clamp the badge to the frame's inner 90 %; when the net is off
   screen the diamond becomes the edge chevron A74 made (one element, two states), with the label beside it.
2. **Edge layout**: one edge-slot rule for every edge element (the puck chevron, the own-net and far-net chevrons, the "own 46 m" label): slots along each edge, no two
   elements closer than 48 px, the puck first; a skater under a label is accepted (labels are over the world), a label over another label is not.
3. **The match card on large and boosted rinks**: the card leaves the frame's middle third free during the drop at every rink size: scale the card to 50 pt on 90 m,
   40 pt on 120 and 140 m rinks, or move it to the top quarter when the far centre falls under it (test both, keep the one the eyes rate higher).
4. Proof with A74's driver on a Development package: `hockey.regular` l2_night, `hockey.boost` l3_neon, `hockey.arena.large` l2_night, the Marathon mode (140 x 70), five
   moments each; eyes `hud` and `readability` with a new question in `tools/eyes` presets if the old one cannot see a marker ("can the viewer tell where the puck and the
   far net are"); the before/after table.
5. `UI.md` section 8 with the table of every HUD element.

Report: the before/after table, the still paths, the file list. Time box 75 minutes.
