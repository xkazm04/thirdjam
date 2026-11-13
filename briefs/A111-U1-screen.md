# Lane A111, track U1: the screen at five a side (device A, evolution 1, Fable)

You are lane A111 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0048 and R-0049 at the end of `contracts/RULINGS.md` (read them from the main checkout), `docs/design/REVIEW-1.md`
(items on the HUD, the hint bar and the first five minutes) and device B's play-through `handoff/B-to-A-0038.md` (the Director's copy: `C:\Users\kazda\AppData\Local\Temp\tj\B-to-A-0038.md`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a100` (the design eye's worktree at the vision's commit; its `docs/design/**` changes are already exported:
leave them). Your status file: `status/A111.md`. Your remote-console port if you run a game: 4711.

## Track U1: the screen
Family: `HUDWidget.*` layout and texts (not its tick cost: track Q3's; not the contact lines: track F1's), `CameraFollow.*` framing near the boards (not the kick: F1's),
the fade-in in `HokejGameMode.*` / `RinkActor.*` at match start, `docs/experiments/SCREEN.md`.

First evolution: **the player finds himself and the ice in the first second.** Device B's play-through and the design eye agree on four faults:
1. **Five blue figures and one ring.** The human's figure carries a marker a newcomer cannot miss at five a side (above the figure, in the HUD's voice, visible on clay and
   night; it must not hide the stick). Proof: a still at the faceoff on each look tier, five a side, where a reader finds the human in under a second.
2. **The first match shows a dark rink for about 3 s.** The fade-in holds until the light rig reports loaded (bounded: never more than 2 s extra). Proof: the first match of a
   fresh process, a still at the first visible frame, on both tiers.
3. **The hint bar** covers the bottom of the frame for 15 s of every match and lists ten verbs but no brake. Show it in the first match of a session only (or until the
   first shot), shorter, with the brake in it; later matches show nothing or one line. **The commentary box** floats over the play: dock it at an edge. **The distance
   labels** are small and white on pale clay: a dark plate or outline, and never over the goal sparks.
4. **In the corners the camera looks out over the boards.** Keep more ice in frame when the followed puck is within about 5 m of a board (a bias toward centre ice,
   eased; no snap). Proof: stills of a corner battle before and after, both tiers; the acceptance run's aim rows still green (the cursor's point on the ice must stay true:
   `CursorAimRink`).
Cut at 90 minutes: 1 and 2 first, then 3, then 4; what does not fit is your first "next".

The gate: the full `Jam` filter green, the hash unchanged (nothing here is sim), and you queue the acceptance run with the Director in your report (track S2 owns the
cursor): name the package folder if you built one.
