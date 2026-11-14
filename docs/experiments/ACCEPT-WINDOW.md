# The package looked at through a window (lane A155, track S2, evolution 3, Sat 14 Nov 03:25 to about 06:00)

Every earlier proof of tonight's changes was headless or off screen, and the acceptance run reads log lines, never pixels. This lane drove the real
H27 packages (`Accept_H27` Development, `Hokej_Hokej_H27` Shipping, main e9d0b55) through a window and looked at them. **It found that every start from
the menu (PLAY NOW, a mode card, a `-cut=` start) drew a black world with only the HUD on top, and played the cartridge's level instead of the lit one.**
The acceptance run was green three times on that package (19 of 19 verbs) because it checks verbs, not the picture.

## 1. The finding (fixed on main a9446394, 04:27)

- A menu start built the match twice. The first build (InitGame, level from the menu's `?level=`) logged `JAMMATCH level=rookie source=settings cut=hockey.regular`;
  about 0.15 s later `D->LoadCut` (the Designer's apply, scheduled for the next frame) reached the new map and called the game mode's `LoadCut` with the file's **path**:
  a second build, `JAMMATCH level=pro source=cartridge cut=../../../ThirdJam/Content/Cartridges/hockey.regular.cartridge.json`, torn down and rebuilt visuals,
  and no `JAMLOOK` lines (the look was applied once, to the first build). The second build is the match that plays.
- What the player saw: ROOKIE lit, a match card that said PRO, and a black screen with the score boxes (no digits), the radar and the prompts. Stills of H27
  (Development and Shipping): lit share 0.011, mean luminance 0.5. A direct start (`open Rink?cut=hockey.regular`, one build) draws the rink: lit 0.9947,
  mean 148. The season's start (one build) draws it: 0.9955. H26 (no preload system): 3.1 MB stills, rink drawn.
- Not the cause (each tried on Accept_H27, black every time): `jam.PreloadMap 0`, `jam.WarmScene 0`, `jam.PreloadRig 0`, `jam.WarmScene.Match 0`.
- The headless test `Jam.Menu.LevelReachesTheBrain` builds once and cannot see a second build. The driver rule that does: **exactly one `JAMMATCH` and one
  `JAMCART loaded` per match start, the LAST `JAMMATCH` has the expected level and source, a `JAMLOOK tier=` line, and a still that is not black.**
- Verified on main a9446394 in the uncooked game (`UnrealEditor.exe Game\ThirdJam.uproject -game -windowed`, `menu-row.ps1 -Editor`): rookie lit + PLAY NOW click:
  one build, `rookie/settings`, `JAMDESIGNER scheduled apply taken back` x1, lit 0.9936; PRO lit + PLAY NOW click: `pro/settings`, lit 0.9947; Pond card:
  `pro/cartridge` (its own), lit 0.8704; `-cut=hockey.regular`: `pro/launch`, lit 0.9946. (`tools/qa/out/menu/a155-fix1`, `a155-fix2`.)

## 2. What the driver gained (all under `tools/qa/accept/`)

| file | what |
|---|---|
| `winhand.ps1` (new) | the window hand as a library: start a package or the uncooked game, find its window, posted keys and buttons plus the cursor, `PrintWindow` stills, remote console, log tail (falls back to `Saved\Logs\ThirdJam.log` when `-abslog` is not opened), close by `WM_CLOSE`, and `HW-Pixels` (the lit share and the mean luminance of a still) |
| `accept.ps1`, `accept.mjs` | **the pixel row, default on.** Before the quit, one still of the live window and the line `PIXEL lit= mean=`; `accept.mjs` adds step `px` (verb `pixels`) and fails the run (a real fail, exit 1) when the lit share is under 0.20. A rendered rink reads 0.99, the black world with its HUD 0.01, the menu card 0.38. No timing changes: the still comes after the last verb |
| `menu-row.ps1` (new) | the menu row: `rookie-return`, `rookie-click`, `pro-click` (saved settings across a restart), `pro-direct`, `pro-return`, `mode`, `season`, `launch`, `walk`, with the corrected rule above; verdict line `thirdjam.menurow/1`; `-Editor` / `-Project` run the uncooked game |
| `drills-row.ps1` (new) | the queued rows: drill 22 at LawHuman 0 and 1, drill 21 at three levels, EndMatch win and loss, the refused press, pause, board hit, shotgun, menus, stop-and-go; each with stills |
| `shipping-play.ps1` (new) | the Shipping zip played once through the window |

## 3. The rows (Accept_H27, main e9d0b55)

See `status/A155.md` for the log lines. In short:

- Acceptance run x3: 3 of 3 PASS (19 verbs), no machine repeat (the second saw the previous run's game still closing: warning only).
- Menu row, corrected: only the season start passes; PLAY NOW, mode and launch starts fail (two builds, black). Saved settings: click PRO writes
  `Difficulty=pro DifficultyChosen=True` at once and after quit; PRO is lit after a restart. The pad walk in the window: `verdict=blind` (76 focusables,
  6 of 6 screens blind, opens_on right, back yes): the game window was not the active one (Chrome held the foreground; `SetForegroundWindow` refused), so the
  menu lane's "9 of 9, 19 of 19" is still unproven.
- Law-one drill: LawHuman 0, 3 of 3 (bump ratio 0.35, both on their feet; hit down ratio 3.26, 150 frames); LawHuman 1, hit-stop 6 frames, kick 12 cm.
- Drill 21 (20 rounds, stop-and-go): rookie 20, pro 18, all_star 19 (bar 18).
- Goals: one `JAMRINK react goal` line a goal (3 and 3); end cards read YOU WIN / BEATEN with the score and `R REMATCH   ESC MENU`.
- Refused: a press while carrying logs `JAMHUD contact_line=NOT_WITH_THE_PUCK refused=1` and shows the plate; no `JAMVIS press` line exists for it.

## 4. Faults found (file or log line)

1. The double build (above): `HokejGameMode.cpp` near the `JAMMATCH` log (`bLevelFromSettings`) and the Designer's scheduled apply; fixed in a9446394.
2. After a mouse click on a level segment, **Return starts nothing** (focus has left PLAY NOW): `tools/qa/out/menu/a155-fix1/rookie-return`, `a155-fix2/pro-return`
   (`menu-row.ps1 -Scenarios pro-return`). `MENU.md` says the segments take no focus.
3. `jam.menuscreen walk` is blind in a window that is not the active one; `MenuWidget.cpp` (the blind test reads `GetTickSpaceGeometry` absolute position): a
   windowed game on a machine whose foreground is another app cannot prove the pad walk.
4. The law-one drill stages a round on the frame its summary is logged (`HokejGameMode.cpp` ~206-212); the abort cleans it the next frame. Harmless.
5. Shotgun: after `ability used kind=shotgun` the HUD still reads "F SHOTGUN ARMED" 0.3 s and 0.7 s later (no spent state in two stills).
6. The pause card's controls block says "BRAKE REVERSE (W THEN S) OR CTRL / L-STICK CLICK", the controls screen "S against your motion, or Ctrl".
7. `MENU.md` section 3's button centres are stale (PLAY NOW is at 960, 472 on the 1920 x 1080 client, not 528).
8. In the editor game a PRO pick saved without `DifficultyChosen=True` and the restart played rookie (package: written). The editor's profile is shared with other
   sessions, so unproven.

## 5. The matrix on tonight's modes (Accept_H27, cuts of main e9d0b55, driver with the pixel row)
Every run: 19 of 19 verbs for arsenal, bruisers, cage, fog-of-war, glass-house, outnumbered, power-play, shotgun, sniper, sudden-death and the tiers l1_day, l2_night,
l3_neon, p3_clay, p4_broadcast; pond 16 of 19 (check, crosscheck, checked off by `check_enabled=false`: PASS+DESIGN, already in `mode-table.json`). **Every one of them also fails
the pixel row** (lit 0.015, the black menu start) on this package; that is the finding, not a regression of the verbs.
**Marathon crashes the game** (3 of 3 runs, `m-mode-marathon`, `-r1`, `-r2`): at step 5 (the pass) `AHokejGameMode::AcceptDrillObserve()` at `HokejAcceptDrill.cpp:341`
(`Sk[G.Mate].Pos`) with `G.Mate` = -1: `Assertion failed: (Index >= 0) & (Index < ArrayNum) ... Array index out of bounds: -1 into an array of size 4`. Marathon is 2 v 2 with a back
each; the drill finds no mate. The probe pass before it already reports `puck_dist=71.39 carrier=-1` (the puck was not placed with the human). Development only (drills),
not the game a player gets; but steps 5 to 10 of marathon do not run. Steps 7 to 10 alone (`accept.ps1 -From 7 -To 10`): crosscheck, checked, deke, spin, sweep and resume_aim pass;
check is a stumble (ratio 0.40, closing 6.66). The drill needs a guard (`G.Mate < 0`: skip steps 5 and 6 and say so).

## 6. Not proven here
- The pad walk (`verdict=blind` in a window that is not the active one); the saved PRO pick across a restart in the uncooked game (shared profile); STICK LOCKED / JUST WON IT
  (not seen in 34 real pokes; the carrier lane is asked); a lit penalty box in the Glass House camera (the box is below the picture).
- The Shipping zip with the a9446394 fix: H27's zip still has the black menu start (`tools/qa/out/ship/a155-ship`).

## 7. H28 (main c5dc15e5) through the window
Faceoff: no post (`001-faceoff-a.png`). The stop mark (chalk ring ahead of the carrying human) reads: orange in the defender's colour when lit, thin pale grey when armed, off in every still of a downed human and at the faceoff;
`JAMHUD mark=stop state=lit def=5 dist=13.2`: the first measured distances were 12.4 to 13.5 m (H28 lit too early: H29 narrows it). The contested-puck ring: one 15-frame episode in 150 s, not visible in the one still inside it.
MARKS: KEEP. STICK LOCKED shown by hand (53 pokes beside the carrier: `contact_line=STICK_LOCKED` twice, `JUST_WON_IT` twice). Menu on a clean profile: click PRO, one Return: `pro/settings`, one build; the pad walk is still blind
(`grid_hit=0 moves=0` on four screens; a Windows Security window held the foreground all night). Shipping zip 279,575,025 bytes (266.6 MiB): starts, ROOKIE lit, renders with stands, quits clean (code 0), no firewall prompt of ours.
Matrix on H28: reg1 and arsenal green with the pixel row (lit 0.96); the rest was not run (the cursor went to the release chain). `make-matrix-cuts.mjs` takes `A155_MODES_DIR` to cut from another tree's cartridges.
Drivers added: `lock-row.ps1`, `h28-look.ps1`, `marks-row.ps1`, `h29-row.ps1`, `post-row.ps1`, `look-row.ps1`, `look2-row.ps1`.
