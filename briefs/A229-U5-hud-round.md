# Lane A229, track U5 (the HUD), Sunday evening: one more round of HUD UI / UX (device A, Opus)

You are lane A229 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then R-0064 (his order of a "profi" front end: a standard of finish, no marks copied), **R-0069, R-0070, R-0071** at the end of
`contracts/RULINGS.md`, the front end's style (`MenuWidget.cpp` `FJamMenuLook`, `MenuStyle.h`), the HUD (`HUDWidget.cpp` and its `HUD*.cpp`
parts), `status/A222.md` (the controls panel: the look the HUD's newest part has), `status/A226.md` with today's pictures of the game
(`submission/shots/h43-*.png`, `h43-contact.jpg`, and the faults sheet `docs/experiments/img/a226/faults.jpg`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A229.md`. Two heavy
processes at most; ONE film at a time. Off screen only. Nothing of yours runs from the main checkout.

## His words (15 Nov 17:16, verbatim)
"In parallel dispatch one Opus subagent for one more round of HUD UI/UX improvement"

## The Director's reading
The front end got its look this week and the controls panel followed it today. The rest of the match HUD is older and of several hands:
look at `h43-contact.jpg`: the score plates and the period plate, the match card at a faceoff ("MATCH 1 / PERIOD 1 - FACEOFF - ROOKIE"),
the faceoff dial and its hint, the GOAL banner, the commentary strip, the bark bubble, the puck's distance label and the edge markers,
the YOU tag and the chevron over him, the minimap, the pause card, the end card with its stats, the "RULE CHANGE READY" pill. One product:
1. **One look.** Every plate, card, label and strip in the menu's own type, sizes, colours, corner cut and edge (the controls panel is the
   reference); two type sizes for labels and one for values, no third; team colours only where a thing belongs to a team.
2. **Order and air.** A safe margin on all four sides; nothing touches or overlaps another element at 1280 x 720 and 1920 x 1080 (the
   distance label over a downed skater, an orange skater cut by the minimap, the banner over the net: find what you can lay out better);
   the centre of the ice stays free in play: transient things (banner, card, dial) leave on time.
3. **Hierarchy.** What he needs every second (the score, the period and its clock, his panel) reads first; what happens once (GOAL, the
   faceoff, the period's card) is large and brief; what is commentary is quiet. The end card is the last picture a judge sees: make it the
   best-set card of the game (the result, his line of stats in a clear table, the two actions).
4. **Nothing on the ice.** No new mark on or near a skater, no line, bar or meter on the ice (R-0069 / R-0070). You add nothing to the
   controls panel's rows.

## Do not collide
Lane A227 (the last fix lane, landing at 18:30) is changing right now: the hit effects' spawning (`EffectsBinder`), the camera while he is
boxed and a plate `IN THE BOX n`, the sprint bar (off the ice, the panel's SPRINT row), the clamp of an edge triangle above the panel,
Glow Ice's "speed" plate. Leave those five things and their functions alone; your work is diffed against your base and merged three-way
after A227's landing: keep your edits in whole functions you own for this round and list each function in your READY.
Lane A231 changes the MODES screen's pictures and lane A230 some modes' looks: not your screens.

## Prove
Before / after pairs of the same frames at 1920 x 1080 and 1280 x 720 (faceoff, open play with and without the puck, a GOAL, the pause
card, the end card), on clay and on Glow Ice (the HUD must hold on the dark sheet too) under `docs/experiments/hud/`; a table: element,
what was wrong, what changed; `Jam.Hud.*`, `Jam.Menu.FitsTheScreen` and the tests that pin a box or a string updated and said; the full
`Jam` filter in a cleared environment; Shipping compiles; the sim's hash equal (presentation only).

`SEEN` with the first before / after pair as soon as it exists (the Director looks himself); `HUD READY` with the files and functions by
name, the pairs and the gate **by 19:15 by the machine clock**. Keep the files closed after the line. Land less, finished, rather than
more, half done.
