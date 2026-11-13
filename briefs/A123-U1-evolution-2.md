# Lane A123, track U1 (the screen), evolution 2 (device A, Fable)

You are lane A123 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0048 to R-0050 at the end of `contracts/RULINGS.md` (main checkout), the design eye's `docs/design/REVIEW-1.md` (in your
worktree: its items on the HUD and the first five minutes), and the track's first evolution in your worktree: `status/A111.md`, `docs/experiments/SCREEN.md`,
`tools/qa/screen/u1-stills.ps1`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a100` (the track's: evolution 1 is committed there as the baseline and is landing; its editor is built).
Your status file: `status/A123.md`. Port 4711 (the track's). Family: `HUDWidget.*` layout and texts, `BarkWidget.*`, `DirectorsCallWidget.*` (its pill only),
`CameraFollow.*` framing, the scoreboard plate, `docs/experiments/SCREEN.md`. Not yours: the HUD's contact line (track F1), the HUD tick's cost (track Q3).

## Evolution 2: nothing sits on the play, and the screen teaches the two new verbs
Your own stills show what is left:
1. **The marker follows the figure**: anchor the chevron and the YOU tag to the figure's head (the visual's socket or its bounds), so it stays on a leaning or
   downed human; at the faceoff it must not sit on the puck or the dot. Bar: the tip within 20 px of the helmet's top on every frame of a 100 s pro match,
   knockdowns included (measure it headless from the projection, as your corner test does).
2. **No box over the middle third of the frame during play**: the bark bubble (`BarkWidget.cpp:26`: 235 cm over the feet, mid-frame) docks or shrinks; the
   "ESC / START  CALL READY" pill shows when a call is ready by the HUD's own rule (after a goal), not from the first faceoff; the scoreboard's centre plate
   ("PERIOD 1: FIRST TO 3" on the top border over an empty panel when there is no timer) is one clean plate. Bar: 30 stills a tier, none with a HUD box in the
   middle third.
3. **The screen teaches the brake and the kept puck**: the design eye found that the hint bar listed ten verbs and no brake (you added it). The vision's two skills
   (stop and go: S against your motion; the puck is yours until someone presses for it) deserve one short prompt each, shown once, at the first moment they apply
   (the first time a defender closes inside 10 m of the carrying human: "S AGAINST YOUR MOTION: STOP, THEN GO"), in the HUD's voice, never again in the session.
   Proof: the prompt's trigger in a test, a still of each.
4. **The marker and prompts on the night tier and on the 140 m rinks** read as well as on clay (stills).

Stills through your own off-screen game process only (no cursor, no foreground). The acceptance run is queued with the Director for the next package: say which rows
your changes could move. The gate: the full `Jam` filter green, the hash unchanged (nothing here is sim). Each rule keeps a console variable that gives back the old
screen. Time box 90 minutes; interim in the status file.
