# Lane A42: HUD pass 2: which skater is yours, how a faceoff is won, one bark at a time, the own-net marker (device A, from H12.1)

You are lane A42 of the Third Jam (team Hokej). Lane A27's eyes pass (`docs/experiments/EYES.md`) and the bake-off strips found four HUD gaps for a first-time player: nothing says
which skater is yours; nothing shows how a faceoff is won (hold, then press at the drop); the bark is drawn twice (bubble and strip) and the bubble sits over the MATCH banner; the
own-net marker hugs the screen edge. Read first: `docs/experiments/EYES.md` (HUD section), `docs/experiments/UI.md` (A18's kit, type scale, colours, the HUD layout), `docs/experiments/
FIGURE-BAKEOFF.md` (bug-hunt lines about the banner), `Game/Source/JamGame/Private/{HUDWidget,BarkWidget,DirectorsCallWidget}.cpp` and headers, `Game/Source/JamGame/Public/MenuStyle.h`,
R-0013 (faceoff rules: early mover loses, first clean press wins), R-0021 point 3 (hockey wording on the ice), `tools/eyes/README.md` (`hud` preset).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a42` (the Director creates it at the current main; build it yourself, unity off; one editor on MCP port 8023 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets`; presentation only, no sim change.

You own: `HUDWidget.*`, `BarkWidget.*`, the HUD part of `DirectorsCallWidget.*`, `docs/experiments/UI.md` (section 7), `status/A42.md`; A18's kit tokens in `MenuStyle.h` stay.

## Job
1. **Yours**: a ring under the human's skater (team colour, 1.1 m, stencil-free, fades at 24 m to a small chevron above the head) and a "YOU" tag for the first 5 seconds of a
   match and after every faceoff for 2 seconds; the puck carrier gets a thin white ring when it is not you.
2. **Faceoff**: during the countdown the ring shows "HOLD" then "DROP" (A18's ring does the countdown); a one-line hint under it the first three faceoffs of a session: "press
   shoot at the drop, not before"; after the draw, "WON" or "LOST" in the team colour for 40 frames.
3. **Barks once**: the bark shows in the bubble over the speaker only; the strip becomes the commentator's line (lane A32's `JAMVO` text, 2 lines, bottom centre, same type); the
   bubble never overlaps the MATCH banner or the call card (y-slots).
4. **Own-net marker** stays 48 px inside the screen edge with an arrow; the far-net marker likewise.
5. Proof: stills at 1080p and 720p for match start, a faceoff countdown, the draw result, a bark and a commentary line together, the own-net marker at both ends; the eyes `hud`
   preset on each; the tests in `CameraTests`/`MenuTests` updated for the new strings (hockey wording).
6. `docs/experiments/UI.md` section 7 with the before/after and the eyes verdicts.

Report: the still paths, the eyes verdicts, the file list. Time box 75 minutes.
