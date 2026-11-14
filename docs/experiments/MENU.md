# The menu and the first run (track U2, lane A144, Sat 14 Nov)

**Two evolutions.** Sections 1 to 6 are evolution 1 (the first screen, the cards, the controls, the lobby), corrected where evolution 2 changed them.
Section 7 is evolution 2 (the operator's first two minutes: a rookie first match, the brand line that follows the cut, the end card, the hook that ends a
match, the pad walk).

A judge opens the game, reads the first screen, picks something and plays. This page is what that path is after lane A144, what it was on main
`e71ebf7`, and how to look at it again without a window or a cursor. Family: `MenuFlow.*`, `MenuWidget.*`, `SettingsWidget.*`, `MenuGameMode.cpp` (the QA
hook), the lobby's widget (`JamNet/Private/LobbyWidget.cpp`), `MenuTests.cpp`.

## 1. The first sixty seconds now

1. **The first screen** says what the game is in one line (*Five-a-side hockey. WASD skates, the mouse is your stick.*), the brand's law under it
   (the banner or the law, by the cut: section 7), and opens on **PLAY NOW** (twice the size of any other button, a yellow edge, *ONE MATCH, ROOKIE.
   ENTER / A, OR CLICK.* inside it). Enter, pad A or a click starts the regular cut at rookie on a first run, at the player's own level once he has
   picked one in the settings (the level wire, section 7): no choice to make.
2. Under it the four other ways to play, one button each, then one row of four small buttons: **CONTROLS**, SETTINGS, CREDITS, QUIT.
3. **CONTROLS** is one press away: fifteen rows, each a verb, its mouse and keys, its pad button and one line on what it does (the tap that is a flat shot,
   the held pass that is a saucer, S against your motion as the brake).
4. **CHOOSE YOUR CUT** shows the twelve Director's choices four a row in three rows (the name at 22 units, the pitch on two lines, three facts one to a
   line) and the cuts as compact cards three a row (icon, name, two lines), two rows visible, the rest scrolls.
5. **BRAWL WITH A FRIEND** opens the lobby over a darkened screen, with `JOIN-GUIDE.md`'s own words over the host's row, over the guest's row and under the codes.

The brand line is read from the first cut each time the screen is built (evolution 2): "Hockey with the whistle taken out." when `hockey.regular` has
`rules.penalties` false, R-0054 point 5's fallback "The speed you bring is the hit you get." while it still has a whistle. Nothing to change by hand.

## 2. What a first-time player could not find, could not read or was told wrongly (stills of main `e71ebf7`, both sizes)

| # | Screen | Fault | Now |
|---|---|---|---|
| 1 | first | no line says what the game is; the first press is the season's take 1; no plain "play" | the two lines; PLAY NOW first and focused |
| 2 | first | the controls are not on it (a paragraph at the bottom of SETTINGS) | CONTROLS button, its own screen |
| 3 | every | the key line (STICK / ARROWS move ...) sits on the card's bottom edge, struck through | 30 units under it |
| 4 | cuts | OUTNUMBERED and SUDDEN DEATH run into the card's edge (172 units for a name at six a row) | four a row: 262 units, the widest name is 210 |
| 5 | cuts | facts wrap an orphan unit ("catch passes at 3 / m", "won puck safe 1.5 / s"); pitch and facts 9 px high at 720p | one fact a line (the widest is 190 of 262); pitch 16, facts 15 units (11 and 10 px) |
| 6 | cuts | the cut rows' film-strip frame covers their words (the name struck through, the tunables under the sprocket holes); one cut of six visible | compact cards, six visible, no tunables line |
| 7 | cuts | the regular cut's row reads "Night Rink: ..." while the cut plays on clay (the cartridge's `theme.name`) | a shipped cut's card is its own facts; the theme name is a finding for the content track |
| 8 | settings | the controls: a run-on paragraph that splits verb from key at line ends; no tap shot, no lift, no saucer | the CONTROLS screen; settings points to it |
| 9 | settings, direct, lobby | the text boxes' grey hint on a light box cannot be read; the key box's hint is clipped | chalk on dark; a shorter hint |
| 10 | lobby | a fixed grey box over the live menu (CREDITS and QUIT show and click under it); nothing says which row is the host's, which the guest's, which code to read out | a dark backdrop; HOST / GUEST lines; the code line; the box as high as its rows |
| 11 | lobby | BACK hides the cursor and gives the keys to the game: over the menu the first screen is dead after it (read in the code; not provable without the cursor) | BACK over the menu restores the cursor and the menu's focus |
| 12 | first | "Opening the lobby (jam.lobby)..." names a console command | "Opening the lobby..." |
| - | first | "SCENE 3 v 3" (the known stale line) | already gone on `e71ebf7`: PROD. HOKEJ / SCENE ROUGH / TAKE A HIT |

Fixed in evolution 2 (section 7): the loss card that read **REMATCH?** over a level 01:01 (still `before-720-end-card-loss.png`); `modes/MODES.md`.
Still open: `hockey.regular`'s `theme.name` "Night Rink" (the places that print it are listed in the lane's report).

## 3. Buttons moved or renamed (for the acceptance run's cursor driver)

Centres in pixels on a 1920 x 1080 client (x 2/3 for 1280 x 720), read from the stills. **No button was renamed; two are new.**

**First screen**

| Button | Before | After |
|---|---|---|
| **PLAY NOW** (new; the screen opens on it) | none | 960, 528 (556 x 108) |
| PLAY THE SEASON | 960, 475 | 960, 622 |
| CHOOSE YOUR CUT | 960, 546 | 960, 691 |
| DIRECT A CUT | 960, 617 | 960, 760 |
| BRAWL WITH A FRIEND | 960, 688 | 960, 829 |
| **CONTROLS** (new) | none | 696, 895 |
| SETTINGS | 960, 759 | 871, 895 |
| CREDITS | 960, 830 | 1047, 895 |
| QUIT | 960, 901 | 1223, 895 |

**One Return on the first screen** still starts a match and logs `JAMMENU start match`: it is now `cut=hockey.regular` (PLAY NOW), where it was the
season's take 1. `accept.ps1` waits for that line and its `-cut=` argument wins either way.

**CHOOSE YOUR CUT**

| What | Before | After |
|---|---|---|
| mode cards | six a row, two rows: x 470, 666, 862, 1058, 1254, 1450; y 260, 471 | four a row, three rows: x 519, 813, 1107, 1401; y 221, 407, 593 |
| order (unchanged) | Pond, Cage, Sniper, Marathon, Power Play, Shotgun / Sudden Death, Outnumbered, Glass House, Arsenal, Bruisers, Fog of War | Pond, Cage, Sniper, Marathon / Power Play, Shotgun, Sudden Death, Outnumbered / Glass House, Arsenal, Bruisers, Fog of War |
| cuts | full-width rows in a list, one visible: Regular at 955, 745 | cards three a row: x 566, 953, 1341; y 787, 891 (Regular, Raw, Boost / Large Rink, Wide, Chaos; a saved cut and played takes scroll) |
| BACK | 428, 955 | 428, 969 |

**Other screens.** CONTROLS (new screen): BACK at 428, 907. SETTINGS: its buttons did not move on the card, BACK moved up with the paragraph gone
(609, 923 to 609, 812; the card is shorter, so it sits lower on the screen: read the still). DIRECT A CUT, CREDITS: nothing moved on the card.
Lobby, idle (names unchanged: `<`, `>`, HOST, JOIN, BACK, START): `<` 638, 410 to 608, 518; `>` 1173, 410 to 1203, 518; HOST 1256, 410 to 1286, 518;
the code box 909, 464 to 909, 606; JOIN 1260, 464 to 1290, 606; BACK 662, 570 to 632, 714. While hosting, the rows close up (the box is as high as its
rows): read the positions from a still of that state.

## 4. Looking at it again (no window, no cursor)

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag after
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag end -EndCard
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag walk -Walk
```
One editor-binary game process on the Menu map, `-RenderOffScreen -Multiprocess`, driven through its remote console on port 4744 by the menu's own QA
command (`jam.menuscreen main|cuts|direct|settings|credits|controls`, `play`, `walk`, `choose <cut>`, `say <sentence>`), the lobby's (`jam.lobby`, `jam.host`,
`jam.leave`) and `jam.screen.Still <name>`; both sizes in one process (`r.SetRes`). Sixteen stills in one minute, under `tools/qa/screen/out/u2-<Tag>/`.
- `-Say` also directs one sentence on the Direct screen. With an API key on the machine that is a **live, paid call**: off unless asked.
- `-EndCard` presses PLAY NOW, ends the match for the other side by real goals (`jam.ctrl.EndMatch 1`), takes the loss card at both sizes, restarts
  (the R key's command) and ends it for the player's side (`jam.ctrl.EndMatch 0`): the win card. Four stills in 80 s.
- `-Walk` runs the pad walk (`jam.menuscreen walk`) and prints its lines; exit 2 when the process cannot judge the directions (section 7).
- The hosting stills show the machine's user name and network addresses: they stay under `out/`, they do not land.
- **Never while a build of the same tree runs**: the game holds the modules the link writes.
- Not reachable this way: the lobby with a guest in it (two processes: `tools/net/twoproc.ps1` with `jam.net.AutoShot 1`), anything about the cursor
  itself (hover, the click that lands, BACK's cursor), and the D-pad's directions (an off-screen process has no menu in its hit-test grid: section 7).

## 5. The tests (`Jam.Menu.*`, `MenuTests.cpp`)

| Test | Holds |
|---|---|
| `Jam.Menu.FirstScreen` | nine buttons in reading order, PLAY NOW first; it starts `hockey.regular`; the cut is five a side as the line says; the brand line promises no whistle-free game while the cut has a whistle |
| `Jam.Menu.FitsTheScreen` | every screen's own desired size inside 1920 x 1080 slate units (the 720p viewport under the DPI curve); twelve mode cards on the screen; with the cards' own fonts each mode name on one line, each pitch on two, each fact on one; each shipped cut's name on one line and its line on two; no theme name in a shipped cut's line |
| `Jam.Menu.ControlsComplete` | every key `UJamInputMap` binds is on a row and every key a row names is bound (15 rows, 37 keys, 37 mappings); the rows say tap = flat shot, held pass = saucer, S against your motion, MMB lift, Q, F, E, wheel, Shift, Space |
| `Jam.Menu.DirectorsChoices` (A67, A81) | twelve cards on disk with a pitch and three facts |
| `Jam.Menu.FirstMatchLevel` (evolution 2, the level wire) | a fresh profile is rookie; a picked level plays itself; a saved "pro" without the pick key is no pick; the rule (regular cut: the player's level; mode, take, saved cut, path: their own); the `?level=` option; the brain's profile for each of the three levels |
| `Jam.Menu.LaunchCutWins` (Director, 14 Nov) | with `-cut=` on the command line PLAY NOW starts that cut, not the regular one; the brain plays the cut's own level with all-star picked; the source reads `launch` |
| `Jam.Menu.LevelReachesTheBrain` (the level wire) | five matches through the menu in the game: the brain's profile name and its source for PLAY NOW fresh, pro picked, all-star picked, a season take, a Director's mode; the match card ends with the level |
| `Jam.Menu.BrandLineFollowsTheCut` (evolution 2) | the rule for both values of `rules.penalties`, and the line on screen against the cut on disk |
| `Jam.Hud.Text` (`CameraTests.cpp`, evolution 2) | the end card: YOU WIN, BEATEN, NO WINNER when level; "YOU  1 : 3  THEM"; "R  REMATCH      ESC  MENU" |

Measured (slate units, the widget's desired size; the viewport is 1920 x 1080): first screen 880 x 969, cuts 1240 x 1053, direct 880 x 491, settings
880 x 776 (753 before evolution 2's longer caption), credits 880 x 936, controls 1240 x 921. The stills show the cards 25 to 40 units shorter than the measure (first screen 941, cuts 1016 px at
1080p): the measure is the safe side. The cuts screen is the tall one: a thirteenth mode card (a fourth row, about 185 units) does not fit; the grid
then needs five a row or a scroll, and the test will say so.

The measure works because every text of these screens wraps at a fixed width (`SetWrapTextAt`): an auto-wrapped text reports one long line before its first
layout pass, so a desired size said nothing. A new text on these screens takes a wrap width, not `SetAutoWrapText`. The widths and fonts the cards use
are `UJamMenuWidget::ModeTextWidth()`, `CutTextWidth()`, `ModeNameFont()` and their kin: the widget and the test read the same ones.

## 6. Stills (`docs/experiments/img/a144/`)

Before (main `e71ebf7`): `before-720-main.png`, `before-720-cuts.png`, `before-720-settings.png`, `before-720-lobby.png`, `before-720-end-card-loss.png`.
After: `after-720-main.png`, `after-1080-main.png`, `after-720-cuts.png`, `after-1080-cuts.png`, `after-720-controls.png`, `after-720-settings.png`,
`after-720-lobby.png`.

## 7. Evolution 2: the operator's first two minutes

**PLAY NOW is a rookie match on a first run.** A judge is a newcomer. Until the player has picked a level in the settings, PLAY NOW plays `rookie`; once
he has, it plays his level; the small line on the button names it (*ONE MATCH, ROOKIE.  ENTER / A, OR CLICK.*), and the settings button shows the same
level (ROOKIE on a first run).
- *What "picked" means* (`FJamSettings::IsDifficultyChosen`): the saved `DifficultyChosen` key (written from the first press of the settings button on),
  or a saved level other than `pro`. A saved `pro` alone is no pick: `Save()` wrote it on every volume change before the key existed, so an old settings
  file does not make its owner a pro.
- *How the level reaches the match: the level wire* (Director's order after REVIEW-2: the settings' AI DIFFICULTY reached no match; the regular
  cut played pro whatever was picked). **The rule:** the player's level wins over the cartridge's for the **regular cuts** (the shipped cuts of
  `Content/Cartridges`: PLAY NOW and the cut cards of CHOOSE YOUR CUT); the cartridge's own level stays for a **Director's mode**, a **season take**, a
  played take listed as a cut, a saved Director's cut, a **joined match** (no setting crosses the wire; the lobby says so), a tool's `-cut=` start, and a
  cut loaded later in the same world: their level is their design.
- *The path.* `UJamMenuFlow::PlayerLevelFor` decides; `StartMatchAtLevel` opens the rink with `?level=<x>` (an option of that one open, never a console
  variable: nothing outlives the match); `AHokejGameMode::InitGame` reads it for the cut the open names; `BuildMatch` gives it to
  `UJamBrain::ApplyCartridge(cut, level)`, and a live cut keeps it. A start that names no level plays exactly as before.
- *Nobody plays pro believing it is rookie.* The log says `JAMMATCH level=rookie source=settings cut=hockey.regular id=hockey-regular` once a match
  (`source=cartridge` otherwise); the match card's line ends with the level (*PERIOD 1   -   FACEOFF   -   ROOKIE*); the pause card reads
  *PERIOD 1   -   ROOKIE (YOUR LEVEL)* or *(THIS CUT'S OWN)*; the settings caption reads *For PLAY NOW and the cuts. The season and the modes set
  their own.*
- *Proof.* `Jam.Menu.LevelReachesTheBrain` plays five matches through the menu and reads the brain's profile name: PLAY NOW on a fresh profile
  -> rookie (settings); the regular cut with pro picked -> pro; PLAY NOW with all-star picked -> all_star; the season's take with all-star picked ->
  rookie, the take's own (cartridge); Sudden Death with rookie picked -> all_star, the mode's own. `Jam.Menu.FirstMatchLevel` holds the rule, the
  option and the brain's call for each level. Look again: `u2-stills.ps1 -Tag level -Level [-LevelCut modes/hockey.mode.outnumbered]`.
- *The launch cut wins* (Director, 14 Nov, the H27 release). With `-cut=<name or path>` on the command line PLAY NOW starts **that** cut (the first
  lines of `UJamMenuWidget::PlayNow`, the Director's own on main), and the match plays the **cartridge's own level** whatever the settings say:
  `JAMMATCH level=rookie source=launch ... id=hockey-a87-accept`. `Jam.Menu.LaunchCutWins` appends the argument to the process's command line, presses
  PLAY NOW with all-star picked, and reads the cut's id, the brain's profile and the source; off screen: `u2-stills.ps1 -Level -LaunchCut <path>`.
- *If the regular cut ever carries explicit `game.ai.*` numbers*, they override the profile at every level (the resolution's order): the player's
  rookie would then not be the measured rookie. Keep the regular cut's `ai` block to the level alone.

**What the acceptance run depends on in the menu** (`tools/qa/accept/accept.ps1`; change none of these without running it):
1. The first screen opens with the keyboard focus on a button that starts a match: one posted Return must log `JAMMENU start match` within 6 s.
   That button is PLAY NOW.
2. PLAY NOW obeys the `-cut=` launch argument: the run starts the package with its own cut (`accept-cut.json`: first to 9, rookie on purpose) and
   a PLAY NOW that loads `hockey.regular` instead ends the run after its third shot (it did, once: H27 was not cut).
3. The launch cut plays its own level: the settings' level never reaches it (`source=launch`).
4. The log line keeps its first words: `JAMMENU start match` (it now ends `cut=<cut> level=<level or cartridge>`).
5. Nothing on the first screen takes the focus from PLAY NOW after it opens (the lobby's BACK gives it back).

**The end card says what happened** (`HUDWidget.cpp`, by the Director's order for this card): the word (**YOU WIN**, **BEATEN**, or **NO WINNER** when the
match stopped level at the horn: the sim names no winner there and the card used to read as a loss), the score with the player's side first
(*YOU  3 : 1  THEM*), and the keys (*R  REMATCH      ESC  MENU*; the season's key lines say ESC MENU too). Esc opens the pause card, where QUIT TO MENU is.

**`jam.ctrl.EndMatch <team>`** (`EndMatchHook.cpp`) ends the running match for a side by real goals: whenever the sim is in play, the puck is placed 0.4 m
inside the net that team scores in (`UJamSim::SetPuck`, the staging hook the drills use); the sim's own step does the rest (`InGoal` -> `ScoreGoal` -> the
goal event, the pause and faceoff, and on the last goal Phase Over, the winner, the win / lose event, the season's result, the card). A match first to 3
from 0:0 takes three placed pucks and their pauses (7.4 s from the command to the card in the proof run). Team 0 is the player's side in a menu-started match. Refused in a joined match. Log:
`JAMCTRL endmatch done winner=1 human_team=0 result=loss score=0:3 frame=1120 pucks_placed=3`.

**The pad walk** (`jam.menuscreen walk`, `u2-stills.ps1 -Walk`): on every screen it stands on each place a pad can stand on, presses the D-pad each way
(key events given to Slate), follows the focus, then presses B. One `JAMMENU walk` line a screen, a `walk done` line with the verdict.
- What it proved off screen: every screen **opens on** the right place (PLAY NOW, POND, ROLL, the level button, BACK, BACK) and **B leaves** all five
  screens for the first one (the key went down the real focus path).
- What it could not: the directions. An off-screen process's hit-test grid ends at the game viewport (`SWindow > SOverlay > SVerticalBox > SViewport`:
  the menu is drawn but not in the grid), and directional navigation reads nothing else, so no press moves the focus there. The walk measures this and
  says `grid=blind` / `verdict=blind` instead of a false fail. **Queued for whoever owns the game windows:** one console command in a windowed
  Development game, `jam.menuscreen walk`; the proving numbers are 9 of 9 on the first screen, 19 of 19 on the cuts screen, 0 traps.

**Stills** (`docs/experiments/img/a144-2/`): `end-win-720.png`, `end-win-1080.png`, `end-loss-720.png`, `end-loss-1080.png`, `main-720.png` (PLAY NOW
says ROOKIE), `settings-720.png` (the level button and where it applies), `match-card-1080.png` (the level on the match card), `pause-720.png` (the
level and whose it is).
