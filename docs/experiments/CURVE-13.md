# The difficulty curve, pass 13: the ladder read through a person's hand (lane A217, track C0, the night of 14 to 15 November)

Scope (brief `briefs/A217-C0-night-ladder-by-a-persons-hand.md`; the Director's ruling of 15 Nov 03:00: "the ladder is read through the person's path").
Targets 85 / 48 / 26 within 3, for the player each level is meant for (rookie the newcomer, pro the regular, all_star the good one). Every step with its
time is in `status/A217.md`; the drivers and rows are under `tools/perf/results/a217/` (`raw/` does not land).

## 1. The instrument: `-run=JamHandCells`

`Game/Source/JamGame/Private/JamHandCellsCommandlet.{h,cpp}` (a JamGame commandlet, measurement only) plays the game mode's match without its actors:
`AHokejGameMode::BuildMatch` (rule hook and zone source before `UJamSim::Init`, the effect system after it, `UJamBrain::ApplyCartridge` by the level),
`GatherInputs` (the brain's world from `FJamSimAdapter::BuildWorld`, the pickups and zones refilled as the game mode does, the bots' verbs copied as the
game mode copies them) and `StepOnce` (the score mirror, the step, the zones, the effects). The human's skater is lane A211's `jam.ScriptHuman 2`
(`ScriptHuman2.cpp`, called through `JamScriptHuman2Step`): the playtester decides, lane A209's hit hand presses the hits, LMB / RMB go through
`FJamButtonResolver` with the shipped bindings, taps and holds by the hand's thresholds, the pass's aim held through its release.

Seeds: match s plays the sim on seed s and the hand on seed s (the game itself always plays the cartridge's seed 1; `-simseed=1` gives that back).
`tools/perf/results/a217/hand.mjs` runs a cell on `curve.mjs`'s seeds, so both instruments play the same sim seeds.

**Proof.** The game itself, headless (`gamehash.ps1`: `UnrealEditor.exe /Game/Maps/Rink -game -nullrhi`, `jam.ScriptHuman=2`), and the commandlet with
`-hashes` give the same `UJamSim::StateHashHex` at every 600 frames to f3600: pro hand seed 1 (899d19f0cc15f2aa ... 6545cc5cf6d393c3) and rookie hand
seed 2 (785b892b0159e933 ... 4fd5553ab2bd8313), 12 of 12 checkpoints. The same 7 seeds twice: 7 of 7 identical. Speed: about 3 s a match a process
on the saturated machine (the game: 41 s for 3600 frames).

**The critic's fourteen rows** were taken on build B (2971a99e + the pickup + the aim hold), which no tree holds any more; re-taken on build C
(9e6815a0 + the aim hold) by the instrument: pro 2 won 5 lost (B: 2 won, 3 lost, 2 open), rookie 2 won 0 lost 5 open at the critic's 9000-frame cap
(B: 1 won, 2 lost, 4 open); the f360 goal against of build B is gone. One f600 hash is shared by both builds (pro seed 5, 055d138b464fcd60). Table in
`status/A217.md` (04:03).

## 2. CURVE READY 1: the six cells by both instruments on build C (no tune)

| cell (target) | verbs, the ladder's instrument (100) | buttons, the person's path (300) | gap |
|---|---|---|---|
| rookie regular (85) | 86 | 87.7 (83.5-91.0) | +1.7 |
| rookie large (85) | 90 | 87.3 (83.1-90.6) | -2.7 |
| pro regular (48) | 31 | 17.7 (13.8-22.3) | -13.3 |
| pro large (48) | 6 | 2.3 (1.1-4.7) | -3.7 |
| all_star regular (26) | 4 | 4.3 (2.5-7.3) | +0.3 |
| all_star large (26) | 2 | 1.7 (0.7-3.8) | -0.3 |

The gap at pro is the person-like hit hand, not the buttons (pro regular, 100 seeds, the game's path: buttons off 12, hand off 34, sprint off 16,
all three off 31). Where the button hand loses: at pro and all_star 71 to 84 % of goals against come from a last shot of 20 m or more (46 to 72 % from
28 m or more); his passes reach a mate 25 to 36 % of the time; on his back 2 to 7 % of play; at rookie 4.7 % of matches have a goal against inside
f600 (the bar is 5 %). Per-level expectations for the night's landings and the levers held in reserve: `status/A217.md`, CURVE READY 1.

## 3. What moved the cells through the night (person's path, 300 a cell)

| cell (target) | build C (READY 1) | + pass / shot / faceoff (READY 2) | + wave 4 + CROSS (READY 3) |
|---|---|---|---|
| rookie regular (85) | 87.7 | 92.3 | 97.3 |
| rookie large (85) | 87.3 | 90.3 | 91.7 |
| pro regular (48) | 17.7 | 22.0 | 46.0 |
| pro large (48) | 2.3 | 12.0 | 18.7 |
| all_star regular (26) | 4.3 | 2.7 | 2.7 |
| all_star large (26) | 1.7 | 5.7 | 6.0 |

Findings on the way: the gap between the instruments at pro is the person-like hit hand, not the buttons; with the far goals simply removed
pro did not come back before wave 4 (the open net was the cause, the far shot the symptom); rookie's red skill is dead under the opponents' pace
floor `Hk.OppPace9` (JamAICore.cpp:383-384); PASS FATES (status/A217.md): the human's pass dies at the passer (6 in 10 opponent touches within
3 m of him, the intended mate within 3 m of the puck in 1 to 2 %).

## 4. LADDER SET 3 (the closed sim of 06:22)

The numbers: pro's shot reach +30 -> +25 m (`JamAICore.h` ProOppSkill, the default: Jam.Bot.DifficultyProfilesResolve keeps pro = the defaults)
and its large-rink red 1.165 -> 1.03; rookie's pace floor `Hk.OppPace9` 0.9 -> 1.0 and its large-rink red 1.11 -> 1.125; all_star's dodge share
0.5 -> 0.2, its speed 1.45 -> 1.15 and its large-rink speed 1.47 -> 1.15 (the rest in `DifficultyProfile.cpp`). All_star's clumsier aim and poke read
better (22 / 24 at 300) but break two design assertions (the levels' aim and poke ordered rookie >= pro >= all_star; the evolution-9 clumsiness is
rookie's alone): the speed carries the set. Pro's body-check closing speed 1.0 -> 5 m/s brought his knockdowns 1.45 -> 1.11 a minute but pro
regular to 60 (100): not set. Jam.Core.NetTrap re-staged from seed 15 to 17 (the Director's ruling 07:40: a live brain fault, not the set's). The
tables: `status/A217.md`, LADDER SET 3.

`Jam.Curve.PersonPath` (`Game/Source/JamGame/Private/JamCurveTests.cpp`) plays seed 1 of each cell for 1800 frames through `-run=JamHandCells`
and compares the end hash with `tools/perf/results/a217/curve-pins.txt`, and plays pro twice. Re-pin after any change that moves the sim, the brain,
a level or the hand: `cd tools/perf/results/a217 && node hand.mjs PIN-pro-regular pro regular 1 --seeds 1-1 --maxframes 1800 --workers 1` (and the
five others; the `hash` of `raw/PIN-*.jsonl` is the pin; `Game/logs/a217/w10.sh` writes all six).
