# Lane A67: Director's modes: cartridge-defined ways to change the character of a match, chosen from the menu (device A, from H14, R-0036, Opus)

You are lane A67 of the Third Jam (team Hokej). The operator (R-0036): "director's choices -> ways/modes to change character of the game for creative variations of
possibilities." Today a match is one of 36 Designer cuts or a season take; a player who wants a different kind of hockey edits JSON or asks the Designer. Your job: six
modes a player picks from the menu as a Director's choice, each a cartridge that changes the match's character through existing keys (rules, rink, team size, physics,
bots, look), each with a name, a one-line pitch and a proof that it plays.

Read first: `contracts/PACK.md`-equivalents here: `schema/README.md`, `schema/DEFAULTS.md`, `schema/limits.json` (every key and range), `Game/Content/Cartridges/*.cartridge.json`
and `ladder/take-*.cartridge.json` (what exists: regular, boost, chaos, arena.large, raw, the takes with pillars and hooks), `docs/experiments/CUTS.md`, `docs/experiments/
DESIGNER-2.md` (the Designer's 36 cuts and how a cut is described), `Game/Source/JamGame/Private/{MenuWidget,MenuFlow,HokejGameMode}.cpp` (how a cut is chosen and
loaded; the cut cards with the rink size text), `docs/experiments/UI.md`, `docs/experiments/PLAYTEST.md` (the three playtester classes drive a match headless),
`tools/playtest/run.mjs`, `docs/experiments/STICK.md`, `LIFTED-PUCK.md`, `TACKLING.md` (today's mechanics and their keys), `Game/Content/Cartridges/hockey.boost` (pickups).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a67` (created at the current main; build it yourself, unity off; one editor on MCP port 8033 only while
proving; at most three editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets` (a mode that wants its own look names B's asset and uses an
existing tier meanwhile); never print `.env`; never kill a process you did not start. A mode changes no sim code: if a character you want needs a key that does not exist,
write the key's name, range and effect into your report as an ask, and ship the mode without it. Unknown keys are errors; absent value = omit the key.

You own: `Game/Content/Cartridges/modes/*.cartridge.json` (six new), `Game/Content/Cartridges/modes/MODES.md`, the menu's mode cards (`MenuWidget.cpp`, `MenuFlow.cpp`:
a "Director's choices" row of six cards with name, pitch and the three facts that differ from regular), `tools/playtest/modes.mjs` (the proof runner), `docs/experiments/
MODES.md`, `status/A67.md`.

## Job
1. Six modes, each one cartridge, each a different character, from the keys that exist (your pick; these are the Director's suggestions, replace any that does not play):
   **Pond** (no penalties, no offside, small rink 60 x 30, 3 v 3, ice friction low, rookie bots that never check: skating and passing), **Cage** (boards restitution high,
   pillars, checks on, penalties off, all_star bots: chaos and collisions), **Sniper** (goal height 1.22, pads low, shot speed high, charge frames long: the lifted shot
   decides), **Marathon** (timer mode 10 minutes, stamina drains fast, sprint bonus high: pacing), **Power Play** (4 v 3 for the human's side, pro bots, score to win 5:
   passing lanes), **Blind Side** (bots' reaction frames high, human's check from 1 m/s, knockdown frames long, deke offset wide: tackling and dekes).
2. Each mode validated by the loader (the Node validator and the C++ `Jam.Core.CartridgeRoundTrip` path), each proven headless: the regular playtester at the mode's
   level for 20 seeds: win %, goals, the human's touches, checks, the one number that shows the mode's character (checks per match for Cage, goals per match for Sniper,
   passes for Power Play, sprint frames for Marathon, dekes for Blind Side, touches for Pond). A mode whose number is not different from regular by a third is
   re-tuned or replaced.
3. The menu: a Director's choices row with six cards (name, pitch, three facts), chosen like a cut; the match card prints the mode's name; the pause menu's controls card
   unchanged. PIE still of the row and of one mode's match card.
4. `MODES.md`: the six modes with their keys, their numbers against regular, the asks (keys that do not exist and what they would allow), and six more mode ideas
   with the keys each would need.

Report: the six modes with their numbers, the stills, the asks, the file list. Interim at 45 minutes with three modes proven. Time box 100 minutes.
