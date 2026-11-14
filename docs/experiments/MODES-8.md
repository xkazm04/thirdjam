# Director's choices, pass 8: one mode on a grid arena (Glow Ice), twelve modes on the faster skaters (lane A197, track M1 evolution 8, device A, 2026-11-14)

Owner: lane A197. Earlier passes: `MODES.md` (A67) to `MODES-7.md` (A173). Ruling: R-0064 point 8 ("one director's mode on a neon arena"), point 2 (free
skaters 15 % faster). Tree: worktree `m1b` on main **97cf7add** (free skaters 1.15, the held cross-check, the hits' wind-ups 4 / 4, the breakaway bonus 0.3, the
new front end), **before the curve lane's first ladder set**. Status log: `status/A197.md`. Raw output: `tools/playtest/out/a197/`, `tools/perf/out/a197*` (do not travel).

## 1. The neon tier as it was (item 1)

Package `Game/Packaged/Accept_H32` (Development), off screen (`-RenderOffscreen` 1920 x 1080, no window, no cursor), all bots, Glass House's file as the cut,
stills by the game's own `jam.screen.StillEvery=900` (with the HUD), CSV profiler over 60 s of play; the same file on `p3_clay` beside it.

| tier | GPU ms mean / p50 / p99 | render thread ms | frame ms p50 / p99 |
|---|---|---|---|
| l3_neon (as it was) | 4.57 / 4.41 / 10.5 | 3.80 | 16.67 / 25.5 |
| p3_clay | 5.61 / 5.29 / 8.51 | 5.04 | 16.70 / 18.3 |

What read badly (stills `img/a197-before-neon-f00900.jpg`, `-f01800`, `-f02700`):
- **It was not dark.** `LR_L3_Neon` is still lane A0's empty placeholder map, so the switcher keeps the persistent sky and directional lights on: a pale blue
  day rink with a warm white trim and orange lamps. Nothing on it was neon.
- No grid and no lines of light; the rink's red and blue lines were plain paint.
- The puck was a dark disc on light ice (only its white ring read).
- Both halves wore one warm trim colour: no team accent by half.

## 2. A's half of the grid arena (item 2)

`UJamNeonArena` (`Game/Source/JamGame/Public/NeonArena.h`, `Private/NeonArena.cpp`), a scene component the rink makes at every `Build` and on a tier switch
(two call lines in `RinkActor.cpp`). On a neon tier (`l3_neon`, or a device B tier named `n1_*`) it draws; on any other tier it is empty and does not tick.
- **The dark sheet.** Every tint of the ice instance (B's `M_Ice`: IceColour, FrostColour, BoardBandColour, BandColour, LineBlue, LineRed, ScuffColour,
  SheenColour, LampSheenColour, AmbientColour; the placeholder's Color) is saved and set near black; the boards take a dark instance; the painted marks and
  the ochre kick plate are hidden. All of it is given back on a switch to another tier (`Jam.Rink.NeonArena` checks it).
- **The grid.** A 2.5 m grid of 2.4 cm lines, clipped to the rounded outline, in 7.5 m tiles (72 on 90 x 45), each tile its own instance. At rest Emissive
  0.4, faintly tinted to its half's team colour (white across the centre). Tiles within 8 m of the puck brighten to 6 (squared falloff), every sim frame.
- **The lines of light**, by half: the blue line, the goal line, the two end-zone faceoff circles with their dots, the crease, and a 10 cm band at the board
  foot in the half's team colour; the centre line, circle and dot in white. The board-top trim strips were already split by half (B's MI_Trim_Blue / Orange).
- **The goal frames**: two posts, the crossbar and the cage's top and foot, 4 cm bars, in the colour of the half they stand in.
- **Trails**: every skater leaves a ribbon on the ice in his team's colour, 30 sim frames long (0.5 s), tapering to nothing at the tail; the puck leaves a 20
  frame white ribbon and has a ring of light under it at Emissive 45: the brightest thing on the sheet. The ribbons are one procedural mesh rebuilt once a sim
  frame (three sections: blue, orange, puck). Cost: section 4.
- **A goal**: the scored net's frame flashes to Emissive 60 and the whole grid pulses once (+8), both falling off over 45 frames (`GoalPulseAt`).
- **Colour-blind reading** (shape and brightness as well as hue): blue trails are a double line, orange trails one solid line; the orange half's lines are
  dashed (1 m lit, 0.4 m dark), the blue half's solid. Blue and orange also differ in brightness (blue lines 10, orange 4: orange blooms to yellow past 5).
- **The radar and the HUD**: untouched (stills show both).
- **Presentation only.** The component reads the pawns' and the puck's transforms and the event bus's goal event; it writes nothing the sim reads, and no
  random number. Materials: only the rink's own trim placeholder `/Game/Placeholders/M_Trim` (opaque, TeamColour, Emissive) in dynamic instances. No light added.
- Switches: `jam.neon 0` gives the tier back as it was; `jam.neon.trails 0` draws no trails (the cost A/B).

## 3. Rules that fit the look (item 3)

100 seeds, regular class at pro, against `hockey.regular` on the same seeds (`tools/playtest/out/a197/r3.md`). Bars: win 35 to 65 %, goals a match 4 to 12.

| rule set | keys (from Glass House's file) | win % | goals a match | box s | match s |
|---|---|---|---|---|---|
| Glass House as landed | whistle, first to 3 | 62 | 4.2 | 9.9 | 182 |
| A: fast puck, no whistle | `rules.penalties` false, `shot.speed` 24, `puck.max_speed` 40, `puck.drag` 0.3 | **28** | 4.0 | 0 | 160 |
| A4 | A, `score_to_win` 4 | **29** | 5.7 | 0 | 231 |
| B: whistle and boosts | Glass House + `pickups` speed (every 240 f, 300 f, 1.4x, 2 on the ice) | 54 | 4.0 | 10.8 | 193 |
| **B4 (taken)** | B, `score_to_win` 4 | **56** | **5.7** | 14.7 | 267 |

Taken: **B4**. The fast puck without the whistle loses the regular player the match (28 %); the whistle and the boosts keep him at 56 % with 5.7 goals a
match (B alone sits on the 4.0 edge). The mode keeps Glass House's rule set (fouls and the box) and its character number (box seconds: 14.7 a match).

**The mode**: file `hockey.mode.glass-house.cartridge.json` (kept: `Jam.Rink.BoxSeenFromIce`, `Jam.Core.ContactModes` and the menu's order read it), title
**Glow Ice** (proposed: 1. Glow Ice, 2. Night Lines, 3. Wire Ice; "grid" avoided: it reads as a film's word). Card: "Dark ice drawn in light. Boost, but skate
clean." / "first to 4, whistles" / "speed boosts on ice" / "foul: sit 10 s". Every line through `tools/barks/filter.mjs` `checkLine`: clean (the title gets
the capitalised-word note every mode title gets, as "Glass House" did).

Cage and Bruisers played on `l3_neon` too: they move to `p3_clay` (the shipped look), so one mode of the twelve is on the grid (`Jam.Rink.NeonArena` checks it).

## 4. Device B's rig (item 4)

B filled the existing slot: **`/JamAssets/Rigs/LR_L3_Neon`, tier `l3_neon`** (in this tree at 16:55): a dark violet hall, a diamond grid of lilac lines (two
sets at +-30 degrees, 4 m apart), an orchid board-foot line, magenta lines on the far walls. The switcher already maps it; `ResolveRigMapPath` (which
prefers an `LR_N1_Neon` if one ever exists, `jam.neon.rig`) stays as a harmless fallback.

A's half with the rig, decided by stills (`tools/perf/out/a197/rig-all/`, `rig-g0s0/`):
- **Two grids fought** (A's square grid over B's diamond grid): A's grid is off by default, **`jam.neon.grid` 0** (no deletion; 1 brings it back with the
  brightening near the puck).
- **B's rig alone leaves the ice lilac-grey and the boards white**: A's dark sheet stays on (`jam.neon.sheet` 1): with it the hall reads dark.
- Kept: the lines and the board-foot band by half (`jam.neon.lines` 1), the goal frames with their flash, the trails (blue double, orange single), the
  puck's trail and ring.
- **The figures' trims** (B's ask): on the grid arena the trim (slot 2) and helmet (slot 4) dynamic instances of each figure take `TeamColour` = the team
  colour, `Emissive` 6, `RimColour` = 4 x team colour + 0.05, `RimStrength` 1.5, each behind an exists-check, saved first and given back on a tier switch or a
  rebuild (`jam.neon.figures`, default 1). Done from `UJamNeonArena` (the pawns' own files are not touched): the log line `JAMNEON figures lit: N` counts the slots.
- **Cage and Bruisers stay on p3_clay**: one mode owns the grid (Glow Ice), and their character is the hits, which the clay rig's light reads best.

## 5. The twelve on the faster skaters (item 5)

**Measured on main 97cf7add, before the curve lane's first ladder set, before the brain's evolution 8 and the bots' longer wind-ups (8 / 12
frames)**: those move every mode; a re-read after the freeze is asked. **Ladder state: main 97cf7add** (free skaters 1.15, wind-ups 4 / 4, breakaway bonus 0.3), before the curve lane's first set; main has since moved to
df03350f (device B's puck ring, assets only: no sim change). Eleven modes inherit their level (pro; Sudden Death all_star): re-read them on the new ladder.
`node tools/playtest/modes.mjs --seeds 1-100` (regular class), the landed files after this evolution, beside `hockey.regular` at the same level
(`tools/playtest/out/a197/base.md` before, `final.md` after). Goals = both teams a match.

| mode | level | win %, regular class (before -> after) | goals a match | match s | character: mode / regular = ratio | moved |
|---|---|---|---|---|---|---|
| Arsenal | pro | 57 | 4.03 | 182 | 5.21 tricks a match (new) | no |
| Bruisers | pro | 54 | 2.39 | 58 | 5.37 / 3.71 = 1.45 | look tier only (p3_clay) |
| Cage | pro | 50 | 7.79 | 211 | 4.87 / 3.68 = **1.32** (fails a third) | look tier only (p3_clay) |
| Fog of War | pro | 56 | 4.03 | 97 | 2.12 / 0.74 = 2.86 | no |
| **Glow Ice** (was Glass House) | pro | 62 -> **56** | 4.2 -> **5.70** | 182 -> 267 | box 9.9 -> 14.7 s a match (new) | first to 4, speed pickups, the grid |
| **Marathon** | pro (+sprint) | **86 -> 48** | 9.73 | 401 | 416.7 / 253.5 = 1.64 -> 355.6 / 253.5 = **1.40** | `ai.hold_radius_m` 1.5 -> 1.0 |
| Outnumbered | pro | 53 | 4.02 | 89 | 7.76 / 4.61 = 1.68 | no |
| Pond | pro | 66 | 9.16 | 215 | 8.04 / 4.74 = 1.70 | no |
| Power Play | pro | 59 | 10.55 | 280 | 6.81 / 4.74 = 1.44 | no |
| Shotgun | pro | 45 | 4.11 | 112 | 6.90 / 4.42 = 1.56 | no |
| Sniper | pro | 48 | 4.13 | 135 | 0.42 / 0.26 = 1.64 | no |
| Sudden Death | all_star | 38 | 1.00 | 49 | 49 s / 185 s = 0.27 | no |

The regular game itself: 55 % at pro (4.18 goals, 182 s), 47 % with sprint, 15 % at all_star. **On the faster skaters one mode left 15 to 85 %: Marathon
(86 %)**; `ai.hold_radius_m` (the opposing forwards hold off a puck the human is within that many metres of) read 1.5: 86, 1.25: 83 (1.60x), 1.0: **48**
(1.40x), 0.5: 45, none: 37 (`r5.md`, `r5b.md`). Taken 1.0: the middle of the band with its third. Since MODES-7 (main ca493723, before the speed rule) the
regular class moved: Marathon 76 -> 86, Cage 62.5 -> 50, Fog of War 61 -> 56, Sudden Death 40.5 -> 38, Pond 70 -> 66; Cage's third 1.39x -> 1.32x.

What the newest main's rules touch: **the hits' wind-ups (4 frames)** every mode with checks on, that is all but Pond (`skater.check_enabled` false);
**the breakaway bonus 0.3** is the default every mode inherits (no mode file sets `breakaway`): it reaches all twelve, most in Marathon (14.5 human
breakaways a match on 140 x 70) and Fog of War (its own number is breakaways: 2.86x).


## Revert path

Content and presentation only. No sim, brain, profile or state field changed; no field added to skater or puck state.
- The grid arena: `jam.neon 0` (the tier as before, at run time); or remove `UJamNeonArena::ApplyFor` / `OnTierChanged` from `RinkActor.cpp` (two lines) and the two
  new files. B's rig selection: `jam.neon.rig 0`, or `ResolveRigMapPath` back to `GetRigMapPath` in `SetTier`.
- Glow Ice: `title` / `theme.name` "Glass House", `rules.score_to_win` 3, no `pickups`, the card of MODES-7 ("The only whistle left. Foul and sit 10 s." /
  "no hits from behind" / "hit the carrier only" / "hits 1.7x harder").
- Marathon: `ai.hold_radius_m` 1.5. Cage, Bruisers: `look.tier` l3_neon.
- **The sim's hash**: `base.jsonl` (build of 97cf7add as it was) against `final.jsonl` (the build with the grid arena, the rig selection and the new files):
  **1,300 of 1,500 matches replay the same hash**: the three regular cells and ten modes, Cage and Bruisers (tier changed) among them; the 200 that differ are
  exactly Glow Ice's and Marathon's, whose rules changed. The look is not in the hash.


## Frame time with device B's rig (this machine)

Editor `-game -RenderOffscreen` 1920 x 1080 (Development editor binaries, uncooked: read the differences, not the absolutes), Glow Ice's file, 60 s of play,
p50 ms. **l3_neon with B's rig and A's half (trails on): GPU 4.52, render 4.08, game 4.38, frame 16.68**; the same file on l2_night: GPU 4.39, render 4.88,
game 3.87. Neon is +0.13 ms GPU over night here (B measured 1 to 1.5 ms on its weaker machine). A's trails alone: +0.28 ms game thread, 0 GPU (before the rig).

## Findings outside the family

1. A 25 cm ochre band still shows at the board foot on the grid arena (stills `img/a197-glow-ice-rig-*`) although A hides its kick plate and zeroes the ice's
   BoardBandCm / BandCm: it is in B's rig or B's board pieces; B's to name or drop on `LR_L3_Neon`.
2. The editor `-game` start of B's new rig content stalled 4 minutes once (21 frames after the `open`, no shader workers seen), then loaded in 44 s on the next
   start: uncooked first-load cost; a cooked package does not pay it. The release chain should still look at the first match load on H33.
3. `tools/soak/baseline.json`: the `m-glass-house` and `m-marathon` cells change with this landing (rules changed); not re-taken here.
4. `Game/Content/Cartridges/modes/MODES.md`'s Marathon row said "no `breakaway` bonus (A122)": no mode file sets one; every mode inherits 0.3.

## Next evolutions for the track, ranked

1. **Glow Ice's light that reads the play**: the grid tile under the puck brightening came off with A's grid (two grids fought); bring it onto B's diamond grid
   as a light pool that follows the puck (one moving emissive disc, not a second grid). Proof: stills with the puck lit in each third; GPU within +0.2 ms.
2. **Cage's third**: 1.32x on 97cf7add (fails a third; 1.39 on ca493723). Levers untried: Cage on 56 x 26, two more pillars, `contact.board_bonus` read as its
   own number. Proof: 1.5x with the regular class inside 15-85 and the good class under 85.
3. **The twelve re-read on the new ladder and the brain's evolution 8** (bots leave a downed man, no long shots with a mate open, 8 / 12 wind-ups): the table
   of section 5 again, 100 seeds a mode; every mode outside 15-85 moved by one number.
