# Lane A8: main menu, cut selection, Designer UI, Director's Call UI, settings, pause (device A, from H5)

You are lane A8 of the Third Jam (team Hokej). Repo `C:/Users/kazda/kiro/thirdjam`. Read first: `GAME.md`, `docs/THEME.md` (the film-slate skin is the theme made visible in the
first 30 seconds), `docs/PLAN.md` 4 (Designer tiers), `briefs/A7-designer.md` (lane A7 is building `UJamDesignerSubsystem` in parallel: `ListCuts`, `LoadCut`, `SaveCut`, `RequestCut`,
`DirectorsCall`, `SetApiKey`; build against those names, and if a call is missing when you compile, declare the minimal interface in your own header and name it in your report),
`briefs/A5-camera-hud.md` and `Game/Source/JamGame/Public/HUDWidget.h`, `HUDFeed.h`, `InputMapping.h` (A5's HUD and input, finished; reuse its style: C++-drawn UMG, no widget blueprints),
`Game/Source/JamGame/Public/HokejGameMode.h` (A1), `contracts/RULINGS.md`, `tools/mcp/README.md`, `tools/run/*.ps1`.

You own: `Game/Source/JamGame/Menu*`, `Game/Source/JamGame/Settings*`, `Game/Source/JamGame/DirectorsCall*`, `Game/Content/UI/**`, `Game/Content/Maps/Menu.umap`,
`Game/Config/DefaultGame.ini` **only** the default map lines (say what you changed). Run no git command; editor on MCP port **8006**; never kill a process you did not start; one heavy job
at a time; free-space floor 40 GB. The shared `Game/Source` tree must compile at all times: never leave a half-written file when you pause; run `tools/run/build.ps1` before every report.

## Deliverables (M1 menu by H10, polish later)
1. **Menu map and flow**: `Menu.umap` (black, a slate-grey film-slate card, the night rig if cheap) as the default game map; flow Menu -> match (`Rink.umap` with the chosen cut) -> win/lose card
   -> back to menu or restart. The main menu is a film slate: title **HOKEJ**, subtitle "a director's cut", then: **Choose your cut** (list of cuts from `ListCuts()`: Regular, Raw, Boost, Small Rink,
   Wide, My Director's Cut when present; each with a one-line description and the key tunables), **Direct a cut** (text box: one sentence -> `RequestCut`; shows "rolling..." while async,
   the resulting cut's name and the three biggest changes, Play / Save as My Director's Cut / Try again; with no key: the cached cuts and a hint where to add a key), **Settings**
  (API key entry masked, stored through `SetApiKey`; controls summary; AI difficulty `rookie | pro | all_star` once lane A3's enum exists, else a placeholder; look tier switch; volume),
   **Credits** (English, no logos; "Built with Unreal Engine" text only; tools used list), Quit.
2. **Director's Call UI** (in-match): on the pause key (Esc / Start) once per period: a card with 3-5 tunables (speed, shot power, ice friction, pickup rate, opponents' skill) as sliders
   within their ranges, Apply calls `DirectorsCall(key, value)` and shows the scoreboard line "DIRECTOR'S CALL: <text>"; once used, greyed until the next period. Pause menu otherwise: resume, restart, quit to menu.
3. **Period and match framing**: "TAKE 1 / TAKE 2 / TAKE 3" period cards at faceoff (hook into the game mode's period events; if A1 has no periods, periods = thirds of `timer_frames` or
   goals milestones in `first_to`; say which), "THAT'S A WRAP" / "RETAKE?" are A5's cards: reuse, do not duplicate.
4. **Gamepad-complete**: every screen navigable with the stick and A/B; keyboard and mouse too.
5. **Tests**: `MenuListsCuts` (lists the shipped presets), `DirectorsCallOncePerPeriod` (second call in the same period refused), `MenuToMatchAndBack` (automation: load menu, pick Regular, match
   starts, quit returns).
6. Screenshots of every screen to `devlog/shots/` (naming in `devlog/README.md`).

Report: what works, screenshots, every error, the seam names you assumed from A7 and A3, deviations with evidence. Time box 3 hours; report at 60 minutes with the state either way.
