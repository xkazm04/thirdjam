# Lane A219, track F3 (what a hit looks like), Sunday: nothing technical is ever drawn at a cross-check (device A, Fable: judgement by looking)

You are lane A219 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0067 and R-0069** at the end of `contracts/RULINGS.md`, `status/A210.md` (the track's night: HitRead and the hits' marks are yours now:
`Game/Source/JamGame/Private/HitRead.cpp`, `Public/HitRead.h`), `status/A211.md` EYE READY and `docs/experiments/EYE.md` (the critic lane's
method and tools under `tools/qa/eye/`: yours to copy and change under `tools/qa/eye2/`).

Your worktree is `.claude/worktrees/f3c`, on main (the Director names the commit). Status file `status/A219.md`. Two heavy processes at most.
Nothing of yours runs from the main checkout EXCEPT reading: you may copy the package `Game/Packaged/Accept_H39` (the Development twin of the
package he played: cooked content, a console, `-dpcvars`) into your worktree (`tools/qa/eye2/pkg/`) and run YOUR COPY off screen
(`-RenderOffScreen`, `-dpcvars=r.RDG.AsyncCompute=0`); never a Shipping exe (it writes his profile); no window, no cursor. Stop a process only
by the id you recorded when you started it.

## His words
20:25 last night: "crosscheck has bug in animation displaying technical blue rectangle instead of animating player with stick into its position".
10:35 today, after playing H39: **"Crosscheck still displays blue line/rectangle in front of player. This should player never see"**.

The night switched off A's bar across the chest and the streak on the stick (`jam.hit.BraceBar 0`, `jam.hit.Streak 0`) and the critic lane
counted "no rectangle or slab in 42 hit strips": on the EDITOR's binary, with a scripted hand, in stills a lane judged. He still sees it, in
the PACKAGE, with his own LMB. **Nobody has looked at a cross-check in the packaged game. You do that first.**

## 1. Find it by looking (the first 30 minutes; change nothing yet)
In your copy of the Development package (cooked content, as he plays it), from the game camera at 1280 x 720 and 1920 x 1080, EVERY sim frame
from 20 frames before to 60 after: the human's LMB without the puck: tapped (a miss in open ice; one that lands) and HELD (standing, skating,
into a man); on the clay look (PLAY NOW), the night look and Glow Ice; rookie. Use the game's own drills (`jam.ctrl.Drill 2` and its
neighbours, `jam.hit.Capture`) or the critic's scripted hand (`jam.ScriptHuman 2`), whatever presses LMB as a person does. Then the same
frames on the editor binary of main, to see what differs.
Look at every frame as he does and write down EVERYTHING that is not the figure, its stick, the ice and the rings under skaters: any line, bar,
rectangle, arc, dashes, chevron, streak, plate, label; its colour and where it stands relative to him. Candidates the Director read in the
code, all still ON by default: `jam.hit.Tell 1` (a chevron of light on the ice 1.4 to 2.4 m ahead of a checker; "the held brace's streak and
arc"), `jam.hit.BraceArc 1` (the arc of dashes ahead of a held cross-check: white under `jam.hit.Quiet 1`, in the TEAM's colour, blue for
him, if Quiet does not reach it somewhere), the HUD's lines on the ice (`HUDGroundStrip.cpp`, the pass line in his colour, `jam.hud.*`),
device B's takes (`PlayCrossCheck`, `SetCrossCheckHold`: does the figure's blueprint spawn a mesh, a trail or an effect of its own? Read
`Game/Plugins/JamAssets` only by loading it in the editor: you change nothing there), and anything that exists only in the cooked build (a
material that did not cook, a default mesh). Write `SEEN` lines with the stills the minute you have them: one contact sheet per case, the
thing named in your words (frame, colour, place). The Director looks at the sheets himself.

## 2. The rule (R-0069 point 3)
**Nothing technical is ever drawn on or in front of a skater at a cross-check, tapped or held, his or a bot's**: no line, bar, rectangle, arc,
dashes, chevron, streak. Only the figure and its stick (device B's takes) and, at contact, the burst on the ice. The same for the body check's
chevron: it goes. What tells him the brace is on is his figure (the stick held up in both gloves) and the pill at the bottom left (it already
reads BRACED): nothing on the ice. If the thing he sees is device B's (inside `JamAssets`), you do not edit it: you stop calling what draws it,
or hide that component from A's side by name, and write the exact asset and component under `FOR B`.

## 3. Prove it on the same frames
The same cases in a Development package of YOUR tree (`tools/run/package.ps1 -Config Development`, out under your worktree), every frame,
both resolutions, three looks: zero marks. Twelve contact sheets before and after, side by side. `Jam.Hit.*` tests updated to the new defaults
(no mark at a cross-check; the burst at contact stays); the full `Jam` filter in a cleared environment; Shipping compiles; the sim's hash
equal (presentation only).

`BAR FOUND` (what it is, with the sheet) within 40 minutes of your start; `BAR GONE READY` with the files by name, the sheets and the gate
**by 12:30 by the machine clock**. Keep the files closed after the line.
