# The menu and the first run (track U2, lane A144, Sat 14 Nov)

A judge opens the game, reads the first screen, picks something and plays. This page is what that path is after lane A144, what it was on main
`e71ebf7`, and how to look at it again without a window or a cursor. Family: `MenuFlow.*`, `MenuWidget.*`, `SettingsWidget.*`, `MenuGameMode.cpp` (the QA
hook), the lobby's widget (`JamNet/Private/LobbyWidget.cpp`), `MenuTests.cpp`.

## 1. The first sixty seconds now

1. **The first screen** says what the game is in one line (*Five-a-side hockey. WASD skates, the mouse is your stick.*), the brand's law under it
   (*The speed you bring is the hit you get.*), and opens on **PLAY NOW** (twice the size of any other button, a yellow edge, *ONE MATCH, THE REGULAR CUT.
   ENTER / A, OR CLICK.* inside it). Enter, pad A or a click starts `hockey.regular` at the settings' difficulty: no choice to make.
2. Under it the four other ways to play, one button each, then one row of four small buttons: **CONTROLS**, SETTINGS, CREDITS, QUIT.
3. **CONTROLS** is one press away: fifteen rows, each a verb, its mouse and keys, its pad button and one line on what it does (the tap that is a flat shot,
   the held pass that is a saucer, S against your motion as the brake).
4. **CHOOSE YOUR CUT** shows the twelve Director's choices four a row in three rows (the name at 22 units, the pitch on two lines, three facts one to a
   line) and the cuts as compact cards three a row (icon, name, two lines), two rows visible, the rest scrolls.
5. **BRAWL WITH A FRIEND** opens the lobby over a darkened screen, with `JOIN-GUIDE.md`'s own words over the host's row, over the guest's row and under the codes.

The brand line is R-0054 point 5's fallback, not "Hockey with the whistle taken out": `hockey.regular` has `rules.penalties` true on this main, and
`Jam.Menu.FirstScreen` fails the day the line promises a whistle-free game the first cut does not play. When the whistle leaves the regular cut, change
`UJamMenuFlow::BrandLine()` (one string).

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

Not fixed here (outside the family; see the lane's report): the loss card reads **REMATCH?** over a level 01:01 and never says the match is lost or why
(`HUDWidget.cpp:261`; still `before-720-end-card-loss.png`); `hockey.regular`'s `theme.name` "Night Rink"; `modes/MODES.md` lists fourteen modes and
"(no card yet)" for two that have cards.

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
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\screen\u2-stills.ps1 -Tag end -EndCard -ScriptHuman 1
```
One editor-binary game process on the Menu map, `-RenderOffScreen -Multiprocess`, driven through its remote console on port 4744 by the menu's own QA
command (`jam.menuscreen main|cuts|direct|settings|credits|controls`, `play`, `choose <cut>`, `say <sentence>`), the lobby's (`jam.lobby`, `jam.host`,
`jam.leave`) and `jam.screen.Still <name>`; both sizes in one process (`r.SetRes`). Sixteen stills in one minute, under `tools/qa/screen/out/u2-<Tag>/`.
- `-Say` also directs one sentence on the Direct screen. With an API key on the machine that is a **live, paid call**: off unless asked.
- `-EndCard` starts the season from the menu, waits for `JAMSEASON take N ... WON|LOST` and takes the card (ten minutes: a match in real time).
- The hosting stills show the machine's user name and network addresses: they stay under `out/`, they do not land.
- **Never while a build of the same tree runs**: the game holds the modules the link writes.
- Not reachable this way: the lobby with a guest in it (two processes: `tools/net/twoproc.ps1` with `jam.net.AutoShot 1`), a **win** card (the scripted
  human lost take 1 at the horn; there is no console way to end a match), and anything about the cursor itself (hover, the click that lands, BACK's cursor).

## 5. The tests (`Jam.Menu.*`, `MenuTests.cpp`)

| Test | Holds |
|---|---|
| `Jam.Menu.FirstScreen` | nine buttons in reading order, PLAY NOW first; it starts `hockey.regular`; the cut is five a side as the line says; the brand line promises no whistle-free game while the cut has a whistle |
| `Jam.Menu.FitsTheScreen` | every screen's own desired size inside 1920 x 1080 slate units (the 720p viewport under the DPI curve); twelve mode cards on the screen; with the cards' own fonts each mode name on one line, each pitch on two, each fact on one; each shipped cut's name on one line and its line on two; no theme name in a shipped cut's line |
| `Jam.Menu.ControlsComplete` | every key `UJamInputMap` binds is on a row and every key a row names is bound (15 rows, 37 keys, 37 mappings); the rows say tap = flat shot, held pass = saucer, S against your motion, MMB lift, Q, F, E, wheel, Shift, Space |
| `Jam.Menu.DirectorsChoices` (A67, A81) | twelve cards on disk with a pitch and three facts |

Measured (slate units, the widget's desired size; the viewport is 1920 x 1080): first screen 880 x 969, cuts 1240 x 1053, direct 880 x 491, settings
880 x 753, credits 880 x 936, controls 1240 x 921. The stills show the cards 25 to 40 units shorter than the measure (first screen 941, cuts 1016 px at
1080p): the measure is the safe side. The cuts screen is the tall one: a thirteenth mode card (a fourth row, about 185 units) does not fit; the grid
then needs five a row or a scroll, and the test will say so.

The measure works because every text of these screens wraps at a fixed width (`SetWrapTextAt`): an auto-wrapped text reports one long line before its first
layout pass, so a desired size said nothing. A new text on these screens takes a wrap width, not `SetAutoWrapText`. The widths and fonts the cards use
are `UJamMenuWidget::ModeTextWidth()`, `CutTextWidth()`, `ModeNameFont()` and their kin: the widget and the test read the same ones.

## 6. Stills (`docs/experiments/img/a144/`)

Before (main `e71ebf7`): `before-720-main.png`, `before-720-cuts.png`, `before-720-settings.png`, `before-720-lobby.png`, `before-720-end-card-loss.png`.
After: `after-720-main.png`, `after-1080-main.png`, `after-720-cuts.png`, `after-1080-cuts.png`, `after-720-controls.png`, `after-720-settings.png`,
`after-720-lobby.png`.
