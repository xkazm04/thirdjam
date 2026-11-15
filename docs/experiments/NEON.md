# The neon arena in more modes: its state per rink size, three modes moved to it, an accent each (lane A230, track L2, Sunday 15 November)

The operator (17:16): "Check state of Neon/Tron arenas to combine in some of the existing modes". Until this lane one mode of twelve played on the grid
arena (`look.tier` `l3_neon`: A's dark sheet, lines of light and trails in `UJamNeonArena` over device B's rig `LR_L3_Neon`): Glow Ice, 90 m. The arena was
filmed at every rink size a clay mode uses, with that mode's own rules and bodies; three modes were moved; each of the four neon modes has an accent of its
own. Presentation only: no sim file, rule, size or bot is touched and the sim's hashes are equal.

"Tron like" (R-0064) is a mood: lines of light on a dark sheet. Nothing of any film or game is copied: the grid, the colours and the shapes are this game's.

## The state per rink size (the editor binary of the lane's tree, game camera, off screen, 1920 x 1080 and 1280 x 720, the mode's seeded match at rookie)

Sheet: `docs/experiments/neon/seen-neon-per-size.jpg` (1080 above 720).

| Rink | Filmed with | Holds | Breaks |
|---|---|---|---|
| 56 x 28 m | Bruisers, five a side | grid to the boards (the rig scaled 0.622: the densest grid), boards and glass meet, nets, creases, rings, trails, HUD | nothing |
| 60 x 28 m | Cage, four a side, four pillars | the same (rig 0.667); the burst at the near net | the four pillars stood as pale grey drums (the boards' daylight material): cured here |
| 60 x 30 m | Power Play | the same | nothing |
| 90 x 45 m | Arsenal, five a side, boosts (and Glow Ice) | the size the rig was drawn for; the boosts are toys of light found from across the rink | nothing |
| 120 x 60 m | Sudden Death, five a side | the same (rig 1.333); the end card reads on the dark sheet | nothing of A's; orange's goal burst reads as peach puffs (device B's effect) |
| 140 x 70 m | Marathon, three a side | the same (rig 1.556): grid cells one and a half times Glow Ice's, the far net a mark on the horizon | nothing |

At no size, 90 m included, is a crowd or a stand seen from the game camera on this tier: beyond the glass the hall is dark. The same sim frames and the
same hashes at both picture sizes in every mode filmed.

## The three modes moved, and why those

| Mode | Rink | Why the arena fits | Its accent (`look.trim_colour_hex`) |
|---|---|---|---|
| **Arsenal** | 90 x 45, 5 v 5 | An arcade idea: grab a boost, get a trick. On the dark sheet the boosts (a cyan bolt on a cyan beam, a pink target on a pink beam, each in a dashed ring) are the brightest toys in the frame and are found from across the rink; in the timber hall they were small props on pale ice. | lime `#A3FF3D` |
| **Marathon** | 140 x 70, 3 v 3 | "Endless ice" needs a horizon. Three skaters a side on a grid whose cells are 1.5 times Glow Ice's, running to a net that is a mark in the distance, say the mode's one idea in a still; the clay hall at 140 m is a very large room. | ice cyan `#22E5FF` |
| **Cage** | 60 x 28, 4 v 4, four pillars | A tight box: the dense grid, the boards' lines close on every side and four drums rimmed in light make a cage of light (the cartridge's own mood words are "loud, hard, neon"; it played on `l3_neon` until lane A197 made that tier the grid arena). | hot red-pink `#FF2D6F`, also on the pillars' rims |
| Glow Ice (unchanged) | 90 x 45, 5 v 5 | The whistle and the speed boost on the sheet the operator asked for by name. | white (the schema's default trim colour: the picture of before) |

Kept where they were: Bruisers and Shotgun (the timber hall and its crowd at 56 m are their point), Power Play, Sniper and Sudden Death on clay (the neon
arena holds at their sizes, but four neon modes of twelve is the share where each still reads as itself, and PLAY NOW's look stays the game's main look),
Fog of War on night ice, Pond and Outnumbered in daylight. PLAY NOW stays on clay.

## How a mode gets its accent (within the tier's keys: no new schema key)

- `look.trim_colour_hex` is a key every cartridge has (default `#38BDF8`; before this lane only the beacon's stand-in material read it). On the grid arena
  `UJamNeonArena::AccentFor` now reads it: the schema's default means "no accent asked" and draws the white of before; any other colour is the mode's
  accent, drawn on the centre line, the centre circle and dot, and the rims of light on the rink's obstacles. `jam.neon.accent 0` gives white in every mode.
- The Cage's pillars (`ARinkActor::Obstacles`, section 0: pillars and boxes) take the dark sheet's material on the grid arena and get a band of light round
  the top edge, the lid's edge and a ring on the ice (`UJamNeonArena::DarkenObstacles`, the rims in `BuildLines`). `jam.neon.obstacles 0` gives back the pale
  drums. Off the tier (a switch to clay, `jam.neon 0`) the pillars have their own material back.
- What tells the four apart in a still besides the accent: Arsenal by its boosts on the sheet, Marathon by the scale of the grid and three bodies a side,
  the Cage by its pillars and the dense grid of a 60 m rink, Glow Ice by its penalty boxes and its one kind of boost.
- The team's own lines (blue solid on his half, orange dashed on the other), the puck's white trail (the brightest thing) and the lilac grid are the same
  in all four: the tier is one look.

Sheet of the four accents: `docs/experiments/neon/accents-four-modes.jpg`.

## Proof

All films are of the EDITOR binary of the lane's tree (not a cooked package), off screen, the person-like scripted hand, a still every 30 sim frames.

- **Before / after per moved mode**, clay above neon, the same sim frames (a faceoff, open play, a goal), 1920 x 1080 above 1280 x 720, one binary
  (build 2): `docs/experiments/neon/arsenal-before-after.jpg`, `marathon-before-after.jpg`, `cage-before-after.jpg`.
- **The four accents** side by side at both sizes: `docs/experiments/neon/accents-four-modes.jpg`.
- **Not moved, not changed**: Glow Ice before the lane's code (build 1) above after (build 2), both sizes, the same picture with its white centre:
  `docs/experiments/neon/glow-ice-unchanged.jpg`. One still each of Bruisers, Power Play, Shotgun, Sniper, Sudden Death, PLAY NOW, Fog of War, Pond and
  Outnumbered on build 2: `docs/experiments/neon/unmoved-modes-one-still-each.jpg`; their cartridges are untouched, their logs name their own tier and no
  `JAMNEON grid arena` line (the lane's code runs only on the neon tier).
- **The sim's hashes** (one line every 600 frames, 60 s of the seeded match): 14 of 14 pairs EQUAL, 6 lines each (`tools/qa/eye8/out/_hashes-final.txt`):
  clay copy against the neon cartridge for Cage, Arsenal and Marathon at both sizes (one binary); Glow Ice build 1 against build 2 at both sizes; each
  moved mode's neon film before the lane's code (build 1, trial copy) against after (build 2); 1080 against 720. No pinned hash moved.
- **Tests**: `Jam.Rink.NeonArena` now holds the four neon modes by file name (it held "one mode on a neon tier: Glow Ice"), that their accents are told
  apart pairwise, that the default trim colour draws the white of before and `jam.neon.accent 0` gives it to every mode, that Glow Ice keeps its white,
  that the Cage's four pillars are on the dark sheet (0 with `jam.neon.obstacles 0`, 0 after a switch to clay, 0 on Glow Ice). No other test pinned a
  moved cartridge's look: `Jam.Look.*`, the other `Jam.Rink.*` and `Jam.Menu.*` are unchanged and green.
- **The gate**: the full `Jam` filter in a cleared environment on build 2: 380 Success, 0 Fail (`Game/logs/a230/auto-jam-1.log`); the Shipping target
  compiles (115 s; nothing Shipping was started); test-guards 0 faults in 241 files; the schema validates the three cartridges; the schema's and the soak
  drift gate's node tests 108 pass.

## Wave 2: the rig scaled per axis on a rink that is not 2 : 1 (the Cage)

Device B's rig `LR_L3_Neon` is drawn for 90 x 45 m and its edge line is one closed loop just inside the boards. A streams the rig scaled by the rink's
length / 90, so the loop meets every 2 : 1 rink; on the Cage (60 x 28) device B measured it 0.47 m from the long boards and 1.47 m from the ends.
`ULookTierSwitcher::SetTier` now takes the rink's width too and streams the neon rig scaled length / 90 along the rink and width / 45 across it when the
rink is not 2 : 1 (`RigCrossScaleFor`; the Cage: 0.667 along, 0.622 across). With B's own two numbers the loop then sits 1.37 m from the long boards and
1.47 m from the ends (computed from the scale, not measured in the picture; the stills agree by eye: the line at the long board is about three times
as far from the board as before, the line at the end is where it was).

- Only `l3_neon`, only a rink that is not 2 : 1: every 2 : 1 rink and every other tier get the uniform scale of before, exactly (the clay rigs' stands and
  crowds are not touched). `jam.look.RigPerAxis 0` gives the uniform scale back everywhere. Heights and the lights' reach keep the length's scale.
- Nothing else of the rig is bent at 60 x 28 on film: the pink line overhead, the glow outside the boards, the goal's burst and the grid (its diamonds
  are 7 % narrower across, not seen as a fault) are as before. Sheet: `docs/experiments/neon/cage-rig-per-axis-before-after.jpg` (the Cage with the
  variable 0 above 1 at a long board, f3150, and at an end, f1800; under it Glow Ice on build 2 above build 3: the same picture).
- The sim's hashes: the Cage with the variable 0 against 1, and against the wave 1 film, six of six EQUAL each; Glow Ice build 2 against build 3 EQUAL.
- Test `Jam.Rink.RigPerAxis` (in `NeonArenaTests.cpp`): the Cage's two scales, the loop's two distances within 0.15 m of each other (a metre apart
  under the uniform scale), six 2 : 1 rinks uniform exactly, clay uniform at 60 x 28, no width given uniform, the variable 0 uniform.
- The gate on build 3: `Jam.Rink+Jam.Look` 10 Success, 0 Fail; the full `Jam` filter in a cleared environment 381 Success, 0 Fail
  (`Game/logs/a230/auto-jam-2.log`); the Shipping target compiles; test-guards 0 faults; the node tests that run no git 108 pass.
- Not seen: a Director's Call that changes the tier in a running Cage match (it passes the width now; no film), a clay cut at a rink that is not 2 : 1
  (kept uniform by the test, not filmed), and whether device B's later rigs hold a scale that differs per axis: B should say if a part of the rig is
  rotated in the map (a rotated part takes the two scales in its own axes).

## Seen and not cured here

- Device B's: when orange scores, the goal's burst on this tier is peach to brown puffs that read nearer smoke than light; blue's is a wash of blue-white
  light (`docs/experiments/neon/forb-goal-burst-orange-vs-blue.jpg`).
- The HUD lane's: the boost's plate under the scoreboard (`speed 4.8s`) is about 8 px high at 1280 x 720 and reads as a debug label on the dark sheet.
- No crowd or stand is seen from the game camera on this tier at any rink length, 90 m included.

## The method (`tools/qa/eye8/`)

`eye8-run.ps1` is lane A223's `eye3-run.ps1` (port 4730, process ids in `Game/logs/a230/<run>.pid`); `chain.ps1` runs a list of films one after another (ONE
editor film at a time: they share one screenshot folder); `looks.py` (sheet, before / after), `pick.py` (stills of several runs on one sheet), `stack.py`,
`hashes.py`; `auto.ps1` (the automation filter in a cleared environment), `build.ps1` (one build through `tools/run/build.ps1`, its slot queue untouched).
A first look at a mode on another tier without touching its cartridge: a copy of the file with only `look.tier` changed, chosen as
`modes/../../../../tools/qa/eye8/trial/<name>` (a `modes/` name plays the level lit on HOME, so the match is the mode's own seeded match). The before
pictures of the three moved modes are filmed the same way from copies of the cartridges as they were (`tools/qa/eye8/trial/clay/`), on the same binary as
the after pictures.
