# The front end, evolution 5 (lane A187, track U2, Sat 14 Nov)

The operator after H31 (R-0064 point 3): "Menu HUD is a mess, leave concept of director's cut and try to create profi NHL-like menu UI and landing
experience". The ruling: the menu stops presenting the game as cuts to direct; a sports-game finish of the game's own (its name, its two team colours),
with no league, team or broadcaster name, logo, mark or likeness, and no copied layout. `MENU.md` is evolutions 1 to 4; this page is evolution 5.

## 1. The screens

| screen | what is on it | home focus | Back goes to |
|---|---|---|---|
| landing | HOKEJ at 176 units, the two team colours under it, the brand line, PRESS ENTER (breathing), "OR A, OR A CLICK" | the whole screen (one button) | stays |
| main (HOME) | PLAY NOW (one match, the level switch ROOKIE / PRO / ALL-STAR and its line under it), SEASON, MODES, PLAY WITH A FRIEND, a row CONTROLS / SETTINGS / CREDITS / QUIT; a feature panel on the right says what the focused entry is | PLAY NOW | landing |
| season | CONTINUE: TAKE n OF 8, the eight takes (number, title, WON / NEXT / OPEN / LOCKED; a locked take takes no focus), the focused take's note on the right | CONTINUE | main |
| modes | the twelve modes as cards five a row (picture slot, name, one line, players a side), then ALL RINKS and MAKE YOUR OWN; the focused card's facts on one line under the grid | the first card | main |
| cuts (ALL RINKS) | the shipped rinks by their own names (REGULAR, RAW, BOOST, LARGE RINK, WIDE, CHAOS), a saved game, the played takes | the first card | modes |
| direct (MAKE YOUR OWN) | the Designer's screen of lane A8, unchanged in what it does (sentence box, ROLL, the shot sentences, PLAY / SAVE / TRY AGAIN) | ROLL | modes |
| settings, credits, controls | as before, on the new panel | the first control / BACK | main |

Every screen but the landing: a header (HOKEJ, the screen's name, the level in force as a blue chip), the two team colours as a rule under it, and a
footer with the live key hints (Enter / A, Esc / B, the pad's names as text: "LEFT / RIGHT  LEVEL" only while PLAY NOW has the focus). Back (Esc, pad B,
a BACK button) works everywhere and lands on the entry it left from.

**Gone from the front end**: DIRECT A CUT as an entry of the first screen, CHOOSE YOUR CUT and the cut list as the first thing, the film slate (clapper,
PROD / SCENE / TAKE lines). **The Designer is still reachable**: MODES, last card MAKE YOUR OWN (`jam.menuscreen make`). The cut list is MODES, ALL RINKS.

## 2. The style table (`FJamMenuLook`, `MenuWidget.h`)
- Spacing on an 8-unit grid (header 80, footer 56, side margin 96, item padding 24 x 16, panel padding 32; `Jam.Menu.FrontEnd` holds every constant on it).
- Type: 176 hero, 48 PLAY NOW, 32 titles, 28 entries, 18 small entries and body, 14 captions; Roboto Black / Bold, wide tracking on capitals.
- Colours: home blue `#2F6BFF` = what is CHOSEN (the lit level, the level chip, a won take); away orange `#F26A1B` = where the FOCUS is; ink `#0A0C10`
  ground, panel `#141821` at 0.90.
- Focus: a solid orange fill with a white bar at the item's left edge (a small item: a bar along its foot), white type; the mouse moves the focus with it,
  so exactly one item is lit and Enter presses what the pointer is on. A level segment lights under the pointer (it takes no focus).
- Motion: a screen fades and slides 24 units in 0.15 s; PRESS ENTER breathes once in 1.6 s.
- Fit: `Jam.Menu.FitsTheScreen` measures every screen (landing 685, main 841, modes 989, controls 918, settings 823 of 1080 units). 1280 x 720,
  1920 x 1080 and 2560 x 1440 are the same 1920 x 1080 units (the DPI curve scales by the short side), so one measure holds the three sizes.

## 3. Device B's kit by name (soft references with fallbacks; `FJamMenuLook::Kit`, `KitNames`)
Under `/JamAssets/UI/`: `T_UI_MenuBackdrop` (the barn at rest, 1920 x 1080, drawn under a 0.62 scrim), `T_UI_Logo_Hokej`, `T_UI_Panel`,
`T_UI_Button_Normal / _Focused / _Pressed / _Disabled`, `T_UI_Segment_Lit / _Unlit`, `T_UI_Card_Mode`, `T_UI_HeaderBar`,
`Modes/T_UI_Mode_<slug>` (pond, cage, sniper, marathon, power_play, shotgun, sudden_death, outnumbered, glass_house, arsenal, bruisers, fog_of_war,
rinks, make_your_own), `Fonts/F_Hokej_Display` (a UFont with Regular / Bold / Black faces). A missing name is asked once per process.
**Cook**: `/JamAssets/UI` must be added to `DirectoriesToAlwaysCook` in `Game/Config/DefaultGame.ini` when the kit lands (nothing holds a hard reference).

## 4. What the drivers press now
- A start with `-cut=` (the acceptance run) opens on MAIN with PLAY NOW focused: its one Return still plays (`JAMMENU opens on main`).
- A player's start opens on the landing (`JAMMENU opens on landing`). The first Return goes to MAIN with PLAY NOW focused
  (`JAMMENU landing press: main, focus=play_now`); the second Return plays. A return from a match opens on MAIN.
- `jam.menuscreen play` starts PLAY NOW from any screen, the landing included. `jam.menuscreen start` is the landing's press; `back` is Esc / B;
  every screen by name: `landing main season modes cuts|rinks direct|make settings credits controls`; `click start|modes|season|continue|make|rinks|back`.
- `jam.menuscreen where` on MAIN prints `label="PLAY NOW"` as before (1920 x 1080 client: x=416 y=188). On the landing it prints the one landing place.

## 5. Proof (main 943df8e7 + this lane, editor build, off screen)
- `Jam.Menu` 16 of 16 (FrontEnd new), the full `Jam` filter 274 of 274; Shipping Game target compiles.
- Slate keys, off screen (`tools/qa/screen/out/u2-a187ret`): landing, Enter: `screen=landing->main`; Enter: one `JAMMENU start match`, one
  `JAMMATCH level=rookie source=settings cut=hockey.regular`. The switch (`u2-a187sw`): right, right, right, left, left, arrow right, Enter: `JAMMATCH level=pro source=settings`.
- Not proven here (no cursor, no window): the pad walk's directions (blind off screen), the mouse following focus, the look at 2560 x 1440 in a window.

## 6. After the Director's look (13:35)
- The cards' two-line description was clipped at its descenders (a 42-unit box): the box is now `ModePitchBoxHeight()` 56 units, two full lines of
  the card face; `Jam.Menu.FitsTheScreen` asserts two lines of the face inside the box and every card's laid-out description (`GetModePitchSizes`) inside it.
- CONTROLS opens the controls lane's rebinding screen when `UJamSettingsWidget::MakeControlsScreen(UObject*)` exists on the tree (a C++20
  requires-expression in `MenuWidget.cpp`, `JamMenuControlsHook::Make`: no define, nothing named when it is absent); today's card otherwise. Its
  `OnBack` goes back once (one Back a frame: its Esc does not also reach the menu's).
- `tools/qa/accept/release-menu.ps1`: `PastLanding` presses the landing's Return and waits for `JAMMENU landing press` before each scenario's
  own press (`return`: then PLAY NOW's Return; `click`: then `where` and the click); `-OldMenu` keeps the old single press. Parse-checked; a window proves it.
- The level chip in the header is the level in force on every screen (re-read at every screen change), so it stays.
- Stills: `docs/experiments/menu-2/` (landing, home, the level switch at 720, modes, season, controls).
