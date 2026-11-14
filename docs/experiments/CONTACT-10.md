# Contact 10: a cross-check you carry into a man (lane A184, track F1, evolution 9; 2026-11-14)

On main `943df8e7`. The operator after package H31 (R-0064 points 1 and 5): "Crosscheck should be holdable so player with speed momentum can run with held
crosscheck into another player" and "Animations of crosscheck and body check are short and not clear". Four things: **the held cross-check** (a brace),
**the bots' and the scripted human's use of it**, **a wind-up the sim owns** for the body check and the cross-check, **the figure's calls** by name.
Every cell with the ladder's instrument (`JAM_PLAYTESTER_CHECK=closing`), seeds 1 to 600, paired. Status with timestamps: `status/A184.md`. Drivers:
`tools/perf/results/a184/` (copies of lane A177's, each editor pid written to `pids.txt`). Raw: `tools/perf/results/a184/raw/` (not landed).

## 0. The choice (what ships)

| Key (`game.contact`) | Ships | Neutral | What it does |
|---|---|---|---|
| `cross_hold` | **true** | false | the held cross-check (section 1) |
| `check_windup_frames` | 0 (**recommended 4**) | 0 | a pressed body check's window opens N frames after the press |
| `cross_windup_frames` | 0 (**recommended 4**) | 0 | a pressed cross-check (the lift's swing) lands N frames after the press |
| AI tune `cross_hold_closing` (every level) | -1 (never) | -1 | a bot braces on a carrier it closes on at this many m/s |

**For a player, in two sentences:** hold the cross-check button (LMB without the puck) and after a sixth of a second you skate with your stick across
your body: the first opponent you run into takes a cross-check as hard as your speed into him, then let go and press again for the next one. While you
hold it you cannot poke, lift, take a pass or pick up a loose puck, you cannot sprint, you turn slower and top out a little lower, and a faster man who
hits you then knocks you down more easily.

**The three cells** (novice at rookie / regular at pro / good at all_star; the tree as it arrived: **86.5 / 53.2 / 30.0**):

| arm | cells | delta | his braced hits a match | his landed cross-checks | knockdowns he gives | knockdowns he takes |
|---|---|---|---|---|---|---|
| neutral (every key off) | 86.5 / 53.2 / 30.0 | 600 / 600 hash-identical a cell | | 2.75 / 2.08 / 1.84 | 0.97 / 1.91 / 3.09 | 3.21 / 6.86 / 8.48 |
| **as shipped: the brace on** | **86.8 / 56.2 / 32.8** | **+0.3 / +3.0 / +2.8** | 1.07 / 1.46 / 0.77 | 3.51 / 3.10 / 2.35 | 1.58 / 2.19 / 1.86 | 3.02 / 6.84 / 8.69 |
| the brace held all match (when he does not carry) | 83.2 / 49.0 / 26.2 | **-3.3 / -4.2 / -3.8** | 15.9 / 11.6 / 12.1 | 17.1 / 12.6 / 13.2 | 14.1 / 10.7 / 11.9 | 3.15 / 6.79 / 7.89 |
| recommended next: the brace + wind-ups 4 / 4 | 85.2 / 53.2 / 30.0 | **-1.3 / 0.0 / 0.0** | 1.04 / 1.52 / 0.78 | 2.86 / 2.79 / 1.60 | 1.25 / 1.75 / 0.99 | 2.99 / 7.18 / 8.64 |
| the brace + wind-ups 6 / 6 | 87.3 / 49.2 / 27.7 | +0.8 / -4.0 / -2.3 | 1.15 / 1.54 / 0.82 | 2.79 / 2.77 / 1.64 | 1.33 / 1.67 / 1.03 | 2.79 / 6.64 / 7.55 |

A paired 600-seed difference carries about 2.7 points. The brace as shipped is worth about 3 points to him at pro and all_star (inside the bar of 4; the
curve lane re-sets the ladder after this lane and the speed lane). **Holding it all match loses at every level** (3 to 4 points): he fells 11 to 14 men
a match and still loses, because a braced stick takes no pass, no loose puck and no puck off a carrier.

## 1. The held cross-check (`contact.cross_hold`)

`FJamInput::bCrossHold` (a level; the controls lane sets it every frame LMB is held while the human does not carry; the press itself still sends
`bLift`, today's cross-check, unchanged). `UJamSim::StepCrossHold`, at the start of the skater's input step:
- the hold counts while the skater could brace (on its feet, out of the box, not the carrier); from `CrossHoldTapFrames` (10) it is **braced**; shorter
  is a tap (the press's own cross-check, as today);
- braced, the first opponent his body meets (`CollideSkaters`, after any pressed check of either) takes **the pressed cross-check's own hit**:
  `ResolveContact(..., EContactKind::Cross)`: closing speed along the line plus `thrust_mps`, the same table; not into a back (a pressed cross-check from
  behind is a stick lift), not on a body the spin or the sweep shields, not by a carrier;
- then the brace is **spent** until the button is released (also when he loses his feet while braced).

**Law 1's line, held against pressed** (`Jam.Core.Contact.CrossHold`, a standing carrier hit head-on; `JAMBRACE` lines):

| closing | ratio | pressed: frames down | held: frames down |
|---|---|---|---|
| 4 m/s | 1.17 | 67 | 67 |
| 8 m/s | 1.83 | 97 | 97 |
| 12 m/s | 2.50 | 127 | 127 |
| 16 m/s | 3.17 | 149 | 150 |

These are the frames at the tree's speeds; **they scale with closing speed, not with the skater's top speed**: the speed lane's 15 % (A185) makes the
closings higher on the shipping tree, so more braced hits sit in the lower rows of this table (a 9 m/s skater into a standing man: 9 m/s, about 105
frames; at 10.35 m/s about 115). The prices and the tap's 10 frames do not scale. Of the cells, the braced hits a match and the knockdowns given will rise
with the speed lane; re-read them on the merged tree.

**The prices** (`UJamSim::CrossHold*`, constants): braced, no poke, no lift, no reception (the blade takes no pass and no loose puck; the body still
dribbles) and no sprint; top speed x 0.9 (momentum is kept: the cap only stops thrust above it) and turn rate x 0.7; a braced skater hit by a man
closing faster than he skates into it himself stands at 0.75 of his stability.

**State** (`FJamSkaterState`): `CrossHoldFrames`, `bCrossBraced`, `bCrossSpent` (hashed behind tag -1841 while not at default); read-outs
`CrossBraceFrames`, `CrossBraceHits`, `bCrossBraceHit` (listed in `SkaterReadouts`).

## 2. The bots and the scripted human

- **The scripted human** (`FJamPlaytester::BraceMode`): with the rule on (and the sim reading it) he holds the brace from 5 m out when skating at an
  opposing carrier at 3 m/s of closing or more, not into his back, and presses no body check while braced. Env `JAM_PLAYTESTER_BRACE=off | always`
  names a measurement's arm (`always` = the "held all match" arm). He now braces where he used to press the check (check presses 10.3 -> 0 a pro match;
  his landed hits 3.10 against 3.28).
- **The bots** (`FJamBotSkill::CrossHoldClosing`, tune `cross_hold_closing`, -1 every level): at 4 m/s with the wind-ups at 6 the bots brace 0.5 / 0.27
  times a pro / all_star match and the human wins **+8.3 / +3.5** more against the same tree: the brace replaces the bot's stick lift and helps him.
  **Not recommended**; ships at -1. (If it is ever switched on in the game, `HokejGameMode.cpp:1306` and `LobbyGameMode.cpp:69` / `:323` must copy
  `bCrossHold` from the brain's output to the sim input as they copy `bLift`.)

## 3. The wind-ups (`contact.check_windup_frames`, `contact.cross_windup_frames`)

The press commits (its cooldown, the figure's call, `bCheckThisFrame` / `bLiftThisFrame`); the check's window (and its lunge) opens N frames later, the
cross-check's swing (shove, strip or miss) lands N frames later on the positions of that frame (`UJamSim::StepWindups`, `OpenCheckWindow`,
`ResolveLift`). A press inside a running wind-up is the same press; a skater that loses its feet, gets the puck, or has its stick locked loses it.
State `CheckWindupLeft`, `LiftWindupLeft` (hashed behind tag -1842); read-out `WindupFrames` (the frames told to the figure on the press).

**What a wind-up does to the hits a match** (pro):
- as first built (the pressers unchanged): wind-ups 6 / 6: **+2.3 / +10.0 / +15.7**; 4 / 4: +3.0 / +11.1 / +13.7. The bots' landed body checks fall
  from 10.6 to 2.5 a pro match: the brain presses 0.5 m from contact for a 6-frame window, so the window opened after the bodies had met;
- **with the pressers anticipating** (the brain's arm gap grows by closing speed x N / 60; the scripted human's course window and closing press shift by
  N frames; `FJamAIWorld::CheckWindupFrames`; at 0 the presses are the same, proved 600 / 600 identical): 6 / 6: -1.0 / -0.5 / -3.2, the bots' landed
  checks 10.4 a pro match (restored), his own landed cross-checks 2.08 -> 1.38; 4 / 4: +2.7 / -1.0 / -2.7. With the brace: section 0's table.

**Why they ship at 0 in this landing**: with 4 / 4 on, the full `Jam` filter was 255 PASS / 19 FAIL; with the wind-ups set back to 0 by the
measurement seam 17 of them pass: they stage a press and expect the hit on the same frame: `Jam.Bot.Hockey.BoxedIsOffIce`, `Jam.Core.Abilities.Shield`,
`Jam.Core.Contact.LawOneDrill`, `Jam.Core.ModeKeys.PenaltyFrames`, `Jam.Core.Movement.LateAnswer`, `.SpinPrice`, **`.StopAndGo` (law 3's drill)**,
`.TakeGrace`, `Jam.Core.SpinCheckedFalls`, `.SpinShieldsLift`, `.Stick`, `.SweepShieldsLift`, `.TacklingEvents`, `.TacklingReadouts`, `Jam.Fx.BusForB`,
`Jam.Rink.HitSeenBoxUsed`, `Jam.Rink.PenaltySeat`; and the acceptance run's drill stages a check and a cross-check the same way
(`HokejAcceptDrill.cpp`). Naming the wind-up in each (or `JamBaseline`-style) is ten files outside this family and the acceptance run on a package:
the next evolution (section 6).

The hit-stop by strength exists (lane A101, `AJamSkater::HitStopFramesFor`: a stumble 1 to 2 frames, a knockdown 2 at `down_ratio` to 6 at
`down_full_ratio`, `jam.hitstop` scales it) and reads the braced hit like any other (the victim's `ContactOutcome` / `ContactRatio`). Unchanged.

## 4. The figure's calls (by name: `AJamSkater::CallFigure`)

The contract's `PlayCheck` takes no argument (C-0001), so nothing on the interface moves. Three calls by name; a figure without them gets what it got:
- `PlayCheckWindup(Frames)`: on a body-check press that has a wind-up (`PlayCheck` still plays on the press); Frames = frames to the window.
- `PlayCrossCheck(Frames)`: on every cross-check press (the lift's swing; `PlayLift` still plays), Frames = frames until it lands (0 today).
- `SetCrossCheckHold(bOn)`: level, on the change of `bCrossBraced`; the human's `JAMVIS crosshold` line logs it with the call sent or `none`.

## 5. The proof

- **Neutral**: every key at its neutral value (build 1 and build 2, `off2`, `off3`): **600 / 600 matches hash-identical to the tree as it arrived in each
  cell**. `JamBaseline::MakeNeutral` carries the three keys. The shipped cartridge (build 4, `ship`) is 600 / 600 identical to the measured `hold` arm.
- **Gate**: full `Jam` filter on the final tree: **274 PASS, 0 FAIL** (`Game/logs/a184/gate2.log`). `Jam.Net.HashCoverage` and `Jam.Net.InputCoverage`
  green. `Bar.Curve.Law3` PASS; `Jam.Core.Movement.StopAndGo` + `Bar.Curve.Law3` with the brace on and off: the LAW3 lines identical character for
  character (pro stop at 8 m 100 %, all_star 100 %).
- **Lockstep**: `FJamInput::bCrossHold` travels as flag bit 4096 (a held level, not in `EdgeFlags`); `From`, `ToInput`, `CoveredFields`; **protocol 9**
  (`JamNet::Version` 8 -> 9; a protocol-8 peer is refused at HELLO, as for every bump). `WireBytes` unchanged (10: the bit fits the two flag bytes).
- **Schema**: `game.schema.json`, `limits.json` (the two integers), `DEFAULTS.md`, `README.md`; both generators run; `node --test schema/*.test.mjs` 99 pass.
- **Shipping** compile of the Game target: PASS (54 s).
- Not run: the acceptance run (headless lane; the controls lane's LMB mapping is not on this tree, so no human input reaches `bCrossHold` before A183 lands).

## 6. The next three evolutions

1. **The wind-ups on** (4 / 4, the cells measured -1.3 / 0.0 / 0.0 with the brace): the 17 staged tests and the acceptance drill name their wind-up, law
   3's drill re-read with them on. Proving numbers: the full `Jam` filter green with 4 / 4 shipped, the LAW3 lines unchanged, the acceptance run green on a
   package, the bots' landed checks a pro match within 0.5 of 10.6.
2. **The scripted human mixes the brace and the check** (the instrument: today with the brace on he never presses the check). Brace on a long approach,
   press on a short one, by the frames to contact. Proving numbers: check presses back to at least 5 a pro match with braced hits at least 1.0, the
   three cells within 2 of the shipped brace's.
3. **The brace reads**: a HUD line for a braced hit given and taken (`bCrossBraceHit`), and the braced skater's speed price measured on the speed lane's
   tree (A185's 15 %). Proving numbers: in one scripted pro match every braced hit on or by the human has its log line (count = `brace_hits`), the
   frames down at 4 / 8 / 12 / 16 m/s unchanged, holding all match still loses by 3 or more at every level on the merged tree.
