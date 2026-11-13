# PUCK-MARKER: the HUD points at the puck, the camera stays bound to the human (lane A74, device A, R-0038)

## The rule
`UJamHudWidget::UpdatePuckMarker` (`HUDWidget.cpp/.h`), presentation only (sim hash and input path untouched, `CameraFollow.cpp` untouched):
- Shown when the puck is **outside the inner 85 % of the frame** (centred rectangle, 7.5 % margin each side) or behind the camera. A chevron stands on the ray from
  the screen centre towards the puck where it meets the rectangle 64 px from the sides, 150 px from the top (under the scoreboard) and 170 px from the bottom
  (above the controls strip); the radar corner is kept clear. Behind the camera the direction comes from the camera-space vector (the projection is mirrored).
- The distance in metres (human to puck) stands beside it, toward the screen centre.
- Colour: the puck carrier's team colour (the sim's carrier, else the skater within 1.2 m); a loose puck is grey.
- Fades in over 6 sim frames and out over 6 (`PuckMarkerFadeFrames`); gone at once on a restart or when the pause menu is up.
- Never shown during the faceoff countdown / MATCH card (`FaceoffCountdownFrames > 0`, first 150 frames of a match), a goal pause, the pause menu, the end cards.
- `jam.hud.PuckMarker 0/1` (default 1).
- The far net: the net-distance diamond (`UpdateMarker`) already clamps to the frame with the chevron towards it, and A64 put its label beside it; nothing changed
  there (see "Not done").

## Proof
Development package of the A74 worktree, A64's driver (`tools/qa/eyesgrid/a74grid.mjs`, same timeline, cursor parked up-ice), cells `hockey.regular` l2_night,
`hockey.boost` l3_neon, `hockey.arena.large` l2_night, 5 stills each. **Before = the same package with `-ExecCmds=jam.hud.PuckMarker=0`** (`-off-` in the name),
after = marker on. Eyes: `bug-hunt` and `hud` on all 30, `readability` (6 m) on faceoff and far-net (12 stills); `docs/experiments/eyes/a74-*.json`
(`a74-*-off-*.json` before). Scoring: `tools/qa/eyesgrid/a74score.mjs`.

Stills (`docs/experiments/img/a74/`): `hockey.regular-l2-faceoff.jpg` (carrier-less puck, grey "24 m" arrow at the left edge), `hockey.regular-l2-farnet.jpg`
(orange arrow "52 m" at the bottom, puck behind the camera), `hockey.boost-l3-faceoff.jpg` ("7 m", orange), `hockey.arena.large-l2-faceoff.jpg` ("6 m", orange),
`hockey.arena.large-l2-farnet.jpg` ("112 m", blue). Off twins: `...-off-...jpg`. The card stills (`*-card.jpg`) show no marker (MATCH card).

## Eyes, before and after (severity 3 or more, stills of 15 pairs)
| Finding | Before (marker off) | After (marker on) |
|---|---|---|
| puck / carrier "scored 1" in readability (faceoff + far-net stills, 6 of 15) | 6 | 6 |
| far net "scored 1" | 0 | 0 |
| net-distance marker on boards / net (regex over bug-hunt and hud) | 4 | 12 (noise: the regex also catches own-net and pause-menu rows) |
A64's grid (47 stills) had 19 stills with "puck / carrier not visible"; this smaller grid has 6 of 6 in the shot moments.

**The count did not fall.** The readability question asks whether the puck is visible on the ice; an edge arrow is not the puck, and Gemini says so on every still
("the puck is not visible on the ice"), arrow or none. By the eyes tool's question the marker changes nothing; by the stills (the arrow, the distance, the carrier
colour are there in each of the six moments) it does what R-0038 asks. A human check is the real measure; a better eyes preset question would be "can the viewer
tell where the puck is" (not run).

## Not done / seen
- The far-net diamond still sits on the far crossbar at 49-66 m (faceoff stills; `hud` row "badge covers the crossbar" on 3 of 3): at that range 3 m above the net mouth
  is still inside the net's pixels and the 150 px top clamp. Lifting the world point or the label would fix it; not touched (outside the time box).
- At the left edge the puck arrow can stand beside the own-net label ("own 46 m") and a skater (boost / arena faceoff stills).
- A first shoot of this run played the fallback cut (tier `l2` is not a tier; `l2_night`, `l3_neon` are); thrown away.
- The Game exe launcher `Windows/ThirdJam.exe` is a stub: the shoot script needs `Windows/ThirdJam/Binaries/Win64/ThirdJam.exe` (A64's note missed this).
