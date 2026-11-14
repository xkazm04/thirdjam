# The menu and the first run (track U2, lane A144, Sat 14 Nov)

**Two evolutions.** Sections 1 to 6 are evolution 1 (the first screen, the cards, the controls, the lobby), corrected where evolution 2 changed them.
Section 7 is evolution 2 (the operator's first two minutes: a rookie first match, the brand line that follows the cut, the end card, the hook that ends a
match, the pad walk) and the level wire. Section 8 is evolution 3 (the level switch on the first screen, the pause card). Section 9 is evolution 4
(lane A164: no dead key on the first screen, the save path, the walk's verdict, the places printed by the game).

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

**Do not copy numbers from this page into a driver: ask the game.** `jam.menuscreen where` prints every place of the shown screen from the widgets'
own painted geometry (`JAMMENU place screen=main label="PLAY NOW" x=960 y=473 w=556 h=108 at1080=960,473 painted=1`: x, y are the game window's client
pixels, at1080 the same place on a 1920 x 1080 client); `u2-stills.ps1 -Where` prints all six screens at both sizes into `where-lines.txt`. The tables
below are evolution 1's history; the current places, as the game printed them on main a9446394 (lane A164, 1920 x 1080 client; 1280 x 720 is x 2/3,
within a pixel), are in section 9.

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
| `Jam.Menu.OldProfileThenPro` (evolution 3) | a profile saved before the level wire shows and plays rookie; PRO picked then stays PRO across three restarts (the saved form's round trip); an old all-star stays; nothing saved is rookie |
| `Jam.Menu.PickSurvivesSaveAndLoad` (evolution 3) | the game's `Save()` and `Load()` through the config system (a scratch section, emptied after): an old profile's three lines load as rookie; PRO picked is saved under its key and loads as PRO with the rest of the profile |
| `Jam.Menu.PauseCardFits` (evolution 3) | the pause card's controls table: five lines, each inside the card's inner width with the card's font; every verb of the binding table named |
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
   That button is PLAY NOW. The level switch under it takes no focus (left and right on PLAY NOW step it), so this holds with the switch, and
   (evolution 4) a click on a segment gives the focus back to PLAY NOW: "click PRO, press Return" plays pro.
2. PLAY NOW obeys the `-cut=` launch argument: the run starts the package with its own cut (`accept-cut.json`: first to 9, rookie on purpose) and
   a PLAY NOW that loads `hockey.regular` instead ends the run after its third shot (it did, once: H27 was not cut).
3. The launch cut plays its own level: the settings' level never reaches it (`source=launch`).
4. The log line keeps its first words: `JAMMENU start match` (it now ends `cut=<cut> level=<level or cartridge>`).
5. Nothing on the first screen takes the focus from PLAY NOW after it opens (the lobby's BACK gives it back). Evolution 4: a focus that lands on the
   menu widget itself (a click on a segment, the title or the card's cloth) goes home to PLAY NOW on the next frame, and a Return or pad A that
   reaches the menu widget presses PLAY NOW: on the first screen Return and A always start the match (a focused button takes the key as its own click).

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

## 8. Evolution 3: the level is chosen where it is played

**The switch.** Under PLAY NOW: `<  ROOKIE | PRO | ALL-STAR  >`, the level in force lit (a yellow edge on a yellow wash), and one line on what it means.
PLAY NOW's small line follows it (*ONE MATCH, PRO.  ENTER / A, OR CLICK.*).
- **A click** on a segment picks it. **Left and right on PLAY NOW** step it, with the arrows, the D-pad or the stick: a custom navigation rule on the
  button (`OnPlayNowNavigate`), so the focus never leaves PLAY NOW and one Enter still starts the match. The ends hold. The segments take no focus.
- It writes the setting the SETTINGS button writes (`FJamSettings::Difficulty` with `bDifficultyChosen`); each screen re-reads it when it shows, so
  the two stay in step.
- **What a level means** (`UJamMenuFlow::LevelMeaning`; the voice's own ladder words, true of the ladder as set: a newcomer wins 85 % at rookie, a
  regular player half at pro, a good player a quarter at all-star):
  - rookie: *Rookies rarely hit. Learn the hits here: a first-timer wins most nights.*
  - pro: *Pros hit as a habit. Even odds for a regular.*
  - all-star: *All-stars hunt whoever has the puck. A good player wins one night in four.*
  If the curve moves a cell by more than a few points, these three lines move with it.
- **A profile saved before the level wire** (a saved `pro` and no pick key) shows ROOKIE on the switch, with no message; a player who then picks PRO
  keeps PRO across restarts (`Jam.Menu.OldProfileThenPro`: the saved form `FJamSettings::ToSaved` / `FromSaved` is the one `Save()` and `Load()` use;
  `Jam.Menu.PickSurvivesSaveAndLoad`: the game's own `Save()` and `Load()` through the config system, in a scratch section).
- **What no headless proof shows:** the file on disk. A process started with `-Multiprocess` (the gate, the off-screen stills) writes no config file,
  so a level picked there lives for that process only (the next off-screen process shows ROOKIE again: that is the flag, not the game). A package
  writes `Saved/Config/Windows/GameUserSettings.ini`, section `[Jam.Settings]`; a profile from an earlier package holds `Difficulty=pro` and no pick
  key there, which is exactly the old profile above: it meets ROOKIE.

**Proof.** In the game process, off screen, by real key presses given to Slate (`u2-stills.ps1 -Switch`, `jam.menuscreen key right|left|arrowright|enter`):
D-pad right twice (pro, all-star), right again (the end holds), left twice (pro, rookie), the keyboard's right arrow (pro), Enter:
`JAMMENU start match cut=hockey.regular level=pro`, `JAMMATCH level=pro source=settings`; every press logged `focus_on_play_now=1`.
`Jam.Menu.LevelReachesTheBrain` plays two more matches from the switch (clicked to all-star, stepped right to pro) and reads the brain's profile.
`Jam.Menu.FirstScreen` holds the segments, the lit one, the two lines and the ends; `Jam.Menu.FitsTheScreen` holds the first screen at or under 1000
of 1080 units (measured 981; the title picture went from 480 to 400 units wide to pay for the switch) and each meaning on one line.

**The pause card** (the Director's order for this card). Its controls table was three lines of 100, 105 and 150 characters on a card 744 units wide
inside: at 1280 x 720 the third reached past both edges. Now five lines of at most 80 characters and a wrap at the card's width
(`UJamDirectorsCallWidget::PauseControlsText`; `Jam.Menu.PauseCardFits`: the widest line is 730 of 744, and every verb of the binding table is named).
**The ability slot** (`HUDWidget.cpp`): the plate was 34 units high for a 32-unit line with 12 of padding, so the words ran down over the cooldown fill at
the plate's foot (a rule through "F  SHIELD  READY"); the plate is 46 high and the fill sits under the words.

**Buttons moved** (centres at 1920 x 1080; x 2/3 for 1280 x 720). None renamed. New: the three segments (click only).

| Button | Evolution 2 | Evolution 3 |
|---|---|---|
| PLAY NOW | 960, 528 | 960, 472 |
| ROOKIE / PRO / ALL-STAR (new) | none | 788, 551 / 959, 551 / 1131, 551 |
| PLAY THE SEASON | 960, 622 | 960, 635 |
| CHOOSE YOUR CUT | 960, 691 | 960, 702 |
| DIRECT A CUT | 960, 760 | 960, 769 |
| BRAWL WITH A FRIEND | 960, 829 | 960, 836 |
| CONTROLS / SETTINGS / CREDITS / QUIT | y 895 | y 901 (x unchanged: 696, 871, 1047, 1223) |
| pause card: RESUME / DIRECTOR'S CALL / RESTART / QUIT TO MENU | 960, 453 / 522 / 591 / 660 | 960, 432 / 501 / 570 / 639 |

**Stills** (`docs/experiments/img/a144-3/`): `first-720-rookie.png`, `first-720-pro.png`, `first-1080-rookie.png`, `first-1080-allstar.png`,
`pause-720.png` (the table inside the card), `ability-1080.png` (the slot's words clear of its rule), `settings-720-in-step.png` (PRO picked on the
switch, the SETTINGS button reads PRO in the same process).

## 9. Evolution 4 (lane A164): the first screen has no dead key

**1. Return after a click on a level segment.** Seen twice in a real window (A155): click PRO, press Return, nothing. Cause, read in the code and the
engine: the segments take no focus (`IsFocusable = false`), the menu widget does (`SetIsFocusable(true)`), and on a mouse-down that sets no focus
Slate gives the keyboard focus to the leaf-most focusable widget under the pointer (`FSlateApplication::RoutePointerDownEvent`): the menu widget
itself, whose key handler knew only Back. Now (`MenuWidget.cpp`): a segment's click picks the level and puts the focus back on PLAY NOW
(`OnSegmentClicked`); a focus on the menu widget itself goes home to PLAY NOW on the next frame (`NativeTick`, first screen only; a focus anywhere else,
the lobby's boxes included, stays); a Return or pad A that reaches the menu widget on the first screen presses PLAY NOW (`NativeOnKeyDown`; log
`JAMMENU start key <key> reached the menu: PLAY NOW`). Proof: `Jam.Menu.ClickThenReturn` clicks PRO **by Slate's own pointer routing** (hover, down,
up down the segment's widget path: `UJamMenuWidget::ClickForQa`, `jam.menuscreen click <rookie|pro|all_star|play|settingslevel>`), reads where the
mouse-down left the focus (`down_focus=menu_itself`), presses Return and plays pro; then pad A with the focus on the menu itself plays all-star. With
the two fix lines disabled the same test fails exactly as the window did (Return started nothing, "timed out"). Off screen in the game process:
`u2-stills.ps1 -ClickReturn`: `JAMMENU click label="PRO" at=960,551 down_focus=menu_itself focus_on_play_now=1`, `JAMMATCH level=pro source=settings`.
A null-RHI editor game paints and ticks no widget: the test sets PLAY NOW's focus itself (what the screen's first tick does), and the next-frame
focus-home is proven in the game process, not in the gate.

**2. The save path.** Every way a level is picked (a segment's click, the pad's or arrows' left / right on PLAY NOW, the settings screen's button,
`jam.menuscreen level`) sets `Difficulty` and `bDifficultyChosen` together and `FJamSettings::Save()` writes all six keys in one loop and one flush; a start
reads them back through the same `Load()` in a package and in the editor game. The settings panel's other presses (look, volume, barks; and the
slider, which fires when the panel sets it) now load the profile before they save, so no older copy can put back an older level or pick key.
`Jam.Menu.ClickThenReturn` reads both keys from the config system after each of the four ways (scratch section, never the player's). Each save logs
`JAMSETTINGS saved difficulty=<x> chosen=<0|1> to_disk=<0|1>`. **The editor case of A155 (a pick "saved" without `DifficultyChosen`, the restart
playing rookie) was not the game**: the driver starts the uncooked game with `-game -Multiprocess` (`tools/qa/accept/menu-row.ps1`, line 28), and under
`-Multiprocess` the engine writes no config file (UE `ConfigContext.cpp` 688-690, `ConfigCacheIni.cpp` 1335-1336); the file it read,
`Game/Saved/Config/WindowsEditor/GameUserSettings.ini`, holds an older run's five lines (`Difficulty=pro`, no pick key): the old profile, which the rule
reads as rookie. To prove a pick across a restart in the editor game: add `-MultiprocessSaveConfig` (or drop `-Multiprocess`); the save line then says
`to_disk=1`. The editor game writes `Saved/Config/WindowsEditor/`, a package `Saved/Config/Windows/`.

**3. The walk in a window that is not the active one** (`verdict=blind`, `reached=0` of 76, A155). The old blind test predicted from the opening
place's tick geometry and the hit-test grid (the walk's entry step) and then skipped the directions. Now the walk always presses every direction from
every place and judges what Slate's own navigation did: a screen is `blind` only when no press from any place moved the focus **and** the grid has no
menu under the opening place; nothing moving while the grid holds the menu is a fail (every place a trap). Each screen's line now carries `moves=`
and `grid_hit=`. Off screen it is still blind (`moves=0 grid_hit=0` on the four screens with more than one place: that process's grid ends at the
viewport, so Slate's navigation itself cannot move there); in a window the verdict reads the moves, whatever window is active. Slate's navigation
does not ask for the active window (`FSlateApplication::AttemptNavigation` / `ExecuteNavigation`); whether the grid holds the menu in an inactive
window is what the next window run's `grid_hit=` and `moves=` answer. The walk no longer presses left / right on PLAY NOW (the level switch: the walk
was changing the player's level while it probed).

**4. The places, printed by the game** (`u2-stills.ps1 -Where`, 1920 x 1080 client, main a9446394 + this evolution):

| Screen | Place | Centre |
|---|---|---|
| first | PLAY NOW | 960, 473 (556 x 108) |
| first | ROOKIE / PRO / ALL-STAR | 789, 551 / 960, 551 / 1131, 551 (167 x 40) |
| first | PLAY THE SEASON / CHOOSE YOUR CUT / DIRECT A CUT / BRAWL WITH A FRIEND | 960, 635 / 702 / 769 / 836 |
| first | CONTROLS / SETTINGS / CREDITS / QUIT | 696 / 872 / 1048 / 1224, 901 |
| cuts | mode cards | x 519, 813, 1107, 1401; y 221, 407, 593 |
| cuts | cut cards (Regular, Raw, Boost / Large Rink, Wide, Chaos) | x 566, 953, 1341; y 787, 891 |
| cuts | BACK | 429, 969 |
| direct | sentence box / ROLL / BACK | 897, 422 / 1309, 422 / 609, 772 |
| settings | API key box / level / look / volume / voice / barks / BACK | 688, 375 / 626, 548 / 650, 677 / 1104, 349 / 1071, 489 / 1072, 641 / 609, 835 |
| controls, credits | BACK | 429, 907 / 609, 892 |

A155's window read PLAY NOW at 960, 472 and PRO at 959, 551: the same places within a pixel. At 1280 x 720 the game printed PLAY NOW 640, 318 and PRO 640, 369.

**For the operator's sheet:** *The first screen: Enter or A plays one match at the level that is lit; left and right (keys, D-pad or stick) change the
level; a click on ROOKIE, PRO or ALL-STAR picks it and Enter still plays; Esc or B goes back from any other screen.*

**Who each level is for (lane A176, the design review's item 5).** The line under the switch now opens with who the lit level is for, in the same
widget, type and wrap, so nothing on the screen moves (`u2-stills.ps1 -Where` before and after: every place of all six screens identical at both
sizes); PLAY NOW's caption is unchanged (*ONE MATCH, ROOKIE.*). No number in it, so the curve cannot make it stale (`Jam.Menu.FirstScreen` holds that):
- rookie: *New to hockey games? Start here: rookies rarely hit.*
- pro: *Played one before? Pros hit as a habit.*
- all-star: *The wall. All-stars hunt the puck. Beat pro first.*
These replace evolution 3's three lines in section 8 (which carried the ladder's numbers). Still: `img/a164/first-1080-rookie-who.png`.

