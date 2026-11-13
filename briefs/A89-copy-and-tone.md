# Lane A89: copy and tone: the game speaks rough (device A, from H20, R-0041 point 4, Sonnet)

You are lane A89 of the Third Jam (team Hokej). The operator (R-0041): the game is raw and brutal contact hockey, and that goes "into its brand (logo, banner, tone of the
game)". Device B owns the logo and the banner; A owns every word the player reads or hears. Your job: one voice across the menu, the modes' cards, the match cards, the HUD
words and the commentator, rough and funny, never cruel: the jam's teen rating holds (R-0021: no gore, no slurs, clay figures; mild words at most one line in ten).

Read first: R-0021 (points on tone and the teen rating), R-0033 point 3 (the commentator: random, no repeats), R-0041, `docs/PLAN-RECONCILED.md`, `docs/experiments/COMMENTARY.md`
(the bank's situations, priorities, `when` conditions, how lines are chosen; `tools/commentary/` and `embed.mjs`), the bank itself (find it from `embed.mjs`), `docs/experiments/
MODES-2.md` and `Game/Content/Cartridges/modes/*.cartridge.json` (`game.meta { pitch <= 48 chars, facts[] <= 3 x 20 chars }`, the twelve modes), `Game/Source/JamGame/Private/
{MenuWidget,MenuFlow,DirectorsCallWidget,HUDWidget,SettingsWidget}.cpp` (every player-facing string: grep `TEXT("` and `NSLOCTEXT`/`FText::FromString`), `docs/experiments/UI.md`
section 8 (the table of HUD elements), `docs/experiments/CONTROLS.md` (the controls card), `GAME.md` if present.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a89` (created at the current main). Run no git command; never touch `Game/Plugins/JamAssets`; never print `.env`;
no editor, no build needed unless you change a C++ string (then build the editor once to prove it compiles and run `Jam.Menu` + `Jam.Hud` + `Jam.Commentary` headless).
File families in use by others, which you must not edit: the commentary bank file and its embedded data (lane A84 adds ability barks there), `InputMapping.*` and the
controls card's key names (A84 moves the pass to Q), `HokejGameMode.cpp`, anything under `Game/Source/JamCore`. Unknown keys are errors; the cartridges' `game.meta` limits hold.

You own: `docs/experiments/VOICE.md` (the voice guide: ten rules with a do and a don't each, the rating's line, a glossary of the game's own words), the `game.meta` pitch
and facts of the twelve mode cartridges (text only), the menu's and the match cards' strings in `MenuWidget.cpp`, `MenuFlow.cpp`, `DirectorsCallWidget.cpp` (text only, no
layout: a string longer than the one it replaces by more than 20 % is a finding, not an edit), `tools/commentary/proposed/a89-lines.json` (new lines in the bank's exact
schema, NOT merged into the bank: the Director merges them after A84 lands), `submission/tagline.md` (three title treatments in words and five taglines for lane A88's page),
`status/A89.md`.

## Job
1. `VOICE.md`: the voice in ten rules; the line the rating draws (what a knockdown may be called, what it may not); the glossary.
2. The twelve mode cards rewritten in the voice within the limits (pitch 48, facts 3 x 20); the validator green (`node schema/validate.mjs` or the schema tests as
   `schema/README.md` says).
3. The menu and match-card strings in the voice (the film words stay in the menu per R-0021 point 3: Director's choices, takes, cuts; the ice stays hockey).
4. Sixty proposed commentator lines in the bank's schema: twenty for knockdowns and checks by `Strength`, ten for cross-checks, ten for the human knocked down, ten for a
   massacre shift (three knockdowns inside 20 s: name the `when` condition you would need; if the bank cannot express it, say so), ten for a goal after a hit; mild words in
   at most six of the sixty; no real names, teams or brands.
5. `submission/tagline.md`.

Report: the voice guide's rules in one list, the before and after of the twelve cards, the strings changed with their lengths, the count of proposed lines per situation,
the file list. Time box 75 minutes.
