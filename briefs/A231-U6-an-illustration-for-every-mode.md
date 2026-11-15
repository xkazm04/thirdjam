# Lane A231, track U6 (the front end's art), Sunday evening: a custom illustration for every mode (device A, Fable: art direction)

You are lane A231 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then R-0008 (generated 2D content: invented text only, no real brand, team, league or person, teen-appropriate), R-0064, **R-0069 to
R-0071** at the end of `contracts/RULINGS.md`, `tools/images/gen.mjs` (the Codex image generation runner: one image a run, a job list a
batch; read its header and `tools/images/menu-jobs.json` for the format and the house style), `status/A220.md` (MODES: a list on the left,
the chosen mode's picture on the right half, drawn whole in its own proportions; where the pictures are loaded: `MenuWidget.cpp`,
`MenuFlow.*`, today from device B's `/JamAssets/UI/Modes/T_UI_Mode_<id>`), lane A226's fault F1 (`docs/experiments/img/a226/faults.jpg`),
today's pictures of the game as reference for what each look really is (`submission/shots/h43-*.png`), `ASSETS.md` (the ledger: every
shipped generated file has a row) and how A's own UI textures under `Game/Content/UI/` were imported (find the lane and its script).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A231.md`. Two heavy
processes at most. Off screen only. Nothing of yours runs from the main checkout.

## His words (15 Nov 17:16, verbatim)
"for each mode generate custom illustration with codex image generation rather than screenshots cut out of the context"

## The Director's reading
The fourteen pictures of MODES (twelve modes, ALL RINKS, MAKE YOUR OWN) are screenshots, and three of them show a game that is not the
one that plays. He wants each to be an ILLUSTRATION made for its mode: a picture of the mode's idea, in one family.
1. **One family.** One style line for all fourteen (a job list `tools/images/modes-jobs.json`): the game's own world (round clay-toy
   hockey skaters, blue against orange, a puck, the rink of that mode), painted, bold shapes, readable at 800 px wide and at 400; the
   front end's colours (deep navy darks, the blue and the orange); landscape 16:9; **no lettering at all** (the screen sets the mode's
   name in type); no logo, no brand, no real team, league or person, no number that reads as a real player's; teen-appropriate.
2. **Each picture says its mode's rule at a glance** (read each cartridge and its line on MODES): Outnumbered: three against five;
   Sniper: a big net and a lifted puck; Cage: a tight box, bodies close; Marathon: three a side lost on endless ice; Shotgun: a blast
   clearing a lane through ten bodies; Sudden Death: one puck, one goal ends it; Pond: open sky, pines, nobody hit; Fog of War: skaters
   coming out of fog; Glow Ice: the dark sheet drawn in light; Power Play: five on three, the puck moved round; Bruisers: heavy bodies,
   a hit; Arsenal: a boost grabbed, a trick fired; ALL RINKS: several rinks at once; MAKE YOUR OWN: a rink being drawn by a hand of light.
   **The look in the picture is the look the mode plays**: lane A230 is moving two or three clay modes onto the neon arena this hour:
   read its line `NEON MODES: <ids>` in `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\f3c\status\A230.md` (due by 17:55) before you
   prompt Arsenal, Marathon, Cage and Sniper; do the other ten first.
3. **Generate, look, pick.** Run the runner into a scratch folder (its default, never into `Game/Content`); look at every result at full
   size; re-prompt what misses (a wrong count of skaters, a letter that crept in, a face that reads as a real person, a muddy
   thumbnail); two tries a mode at most, then keep the better and say what is weak. The prompts carry no personal data, no path, no name.
4. **Into the game.** Import the fourteen picked pictures as A's own textures (`/Game/UI/Modes/T_UI_ModeArt_<id>`, 1280 x 720, the UI
   texture group and compression the other `T_UI_*` of A use; no mips beyond what the group gives) with a script others can repeat
   (`tools/images/import-modes.py` or as the earlier lane did); MODES loads A's picture for a mode when it exists and device B's as the
   fallback (a few lines in the loader; the test of proportions stays within 1 %); a row for each in `ASSETS.md` (origin ai-generated,
   tool "Codex image generation", url = the job list's path, size) by the ledger's own tool or format; the PNG sources under
   `tools/images/modes/` (they are small); the package's size said before and after (the budget is in `contracts/ASSETS.md`).

## Prove
A contact sheet of the fourteen (`docs/experiments/modeart/contact.jpg`) as soon as the first ten exist: a `SEEN` line (the Director looks
himself); stills of MODES with the first, a middle and the last row selected at 1920 x 1080 and 1280 x 720; `Jam.Menu.*` green (the list,
the proportions, FitsTheScreen); the ledger's gate (`tools/media/credits.mjs` and the asset gate the repo has: find it) green on your
tree; the full `Jam` filter in a cleared environment; Shipping compiles; a cook-only check if the repo has one.

`MODE ART READY` with the files by name, the sheet and the gate **by 19:15 by the machine clock**. Keep the files closed after the line.
If the generator fails or is refused, write `ASK` at once with its last lines: do not look for another service.
