# Movement 12: the stop line lights only where stopping pays (lane A201, track F2, evolution 11, 2026-11-14)

The stop's line (`JamCarrierMarks::StopMark`) is the game's only teacher of law 3, and on main it taught wrongly: a scripted player who stopped at every
lit line won 29 of 100 at pro against 58 for one who ignored it (MOVEMENT-11). This lane measured why, and gave the light gates that keep the line pale
(shown, armed, the chalk line) where the stop loses. Presentation only: no sim state, no cartridge key, no brain change. Status with timestamps:
`status/A201.md`. Drivers and readers: `tools/perf/results/a201/` (`m*.sh`, `scenes.mjs`, `pairs.mjs`, `rules.mjs`, `arms.mjs`); logs `Game/logs/a201/`
(not landed). The main it stands on: 58af359f (A194's evolution 10, the hits' wind-ups 4 / 4, brain7).

## 0. The instrument now obeys what the screen draws

Lane A194's `JAM_MARK_RULE` arms played identical matches: the trace read the env, the scripted player read the built `FStopRule`. Now one parse
(`JamCarrierMarks::ApplyStopRuleSpec`, `StopRuleFromEnv`) feeds the trace and the scripted player, and the HUD draws `StopMark` with the shipped
`FStopRule`: an arm that changes the light changes what the obeying player obeys (the arms below differ match by match). `Bar.Movement.CarryTrace` plays
several arms in one process (`JAM_TRACE_ARMS = "label@stopmark@rule@hand|..."`, the same seeds each arm) and writes one `LITSCENE` row per lighting:
the scene at the light and what followed in 90 frames.

## 1. Why obeying loses (the line as main ships it, 200 seeds, regular player)

| | pro: ignores it | pro: obeys every light | rookie: ignores | rookie: obeys |
|---|---|---|---|---|
| wins of 200 | **105** | **38** | **194** | **157** |
| lit a match | 13.7 | 15.2 | 14.2 | 24.1 |
| 90 frames after a light: kept it, shot or passed | 52 % | 53 % | 67 % | 70 % |
| ... lost it to the man the line was for | 11 % | 11 % | 6 % | 5 % |
| ... lost it to ANOTHER man | 37 % | 36 % | 26 % | 25 % |
| goals for minus against inside 10 s, per 100 lights | +14 | 0 | +29 | +9 |

The stop does not lose the puck more often than the scripted player's own play (his cut, his pass, his shot). It loses the attack: the goals inside ten
seconds of a light fall from +14 to 0 per hundred. And the man the line is for takes the puck only 11 % of the time when the light is ignored: the
line warns of a danger that, in a match, is mostly another man's.

**The paired read** (each scene's own effect). Arm `kN` obeys only the N-th light of a match (N = 1..12) and is the ignoring player's match, frame for
frame, up to that light; the first scene that differs is the obeyed one. 862 pairs at pro, 1018 at rookie (`pairs.mjs`, `out/pairs-m2.json`):

| the scene at the light (pro) | share of lights | lost to the man: ignored -> stopped | lost to another | kept / shot / passed | goals +/- in 10 s per 100 | **the match, (wins - losses) per 100 stops** |
|---|---|---|---|---|---|---|
| inside 12 m of the net he attacks | 7 % | 5 -> 8 % | 59 -> 58 | 36 -> 34 | +33.9 -> +20.3 | **-37.3** |
| a second man inside 0.7 s of the stopping point, attacking half | 46 % | 13 -> 7 | 31 -> 41 | 56 -> 52 | +20.6 -> +13.5 | **-6.5** |
| alone with his man (second man 0.7 s or more), attacking half | 5 % | 17 -> 13 | 7 -> 11 | 76 -> 74 | +15.2 -> +8.7 | **+4.3** |
| a second man inside 0.7 s, his own half | 35 % | 14 -> 7 | 37 -> 38 | 49 -> 55 | +1.6 -> -8.5 | **-20.3** |
| alone with his man, his own half | 6 % | 17 -> 25 | 21 -> 12 | 62 -> 63 | -3.8 -> -3.8 | **-11.5** |
| all | 100 % | 13 -> 8 | 33 -> 38 | 53 -> 54 | +13.0 -> +4.9 | **-13.2** |

Rookie, the same classes: -0.0 / -1.3 / 0.0 / -1.6 / 0.0 (all -1.2): a rookie's defence forgives the stop; a pro's punishes it everywhere except one
scene. The second man's time is to the **stopping point** (where his reverse brake ends from his speed, v^2 / 2a on his travel), at a free skater's
true speed (his effective speed x `skater.free_speed_mult`, the brain's planning speed since brain7), computed in the mark's own code.

## 2. The hand: the line is timed for a person

Law 3's drill with the stop begun N frames after the line lights (`FJamCarryDrill::LightReactFrames`, `Bar.Curve.Law3`, 200 seeds):

| on the light + | 0 | 6 | 9 | 12 | 15 | 18 | 21 | 24 |
|---|---|---|---|---|---|---|---|---|
| pro | 34.5 | 42.5 | 60.5 | 90.5 | **100** | 100 | 98.5 | 58.5 |
| rookie | 95.5 | 100 | 100 | 100 | 100 | 100 | 98 | 48.5 |
| all_star | 30.5 | 75.5 | 90 | 99.5 | 100 | 100 | 100 | 95.5 |

A press 0.2 to 0.35 s after the light passes a pro 90 % or more: the line's quarter-second lead (`FStopRule::ReactSec`) is right for a person. The
regular scripted player sees the light and presses 6 frames later (its `ReactionFrames`), which is 42.5 % at pro: a hand faster than a person's. The
obeying player is measured both ways below: `h` = its own 6 frames, `h15` = a quarter second (`JAM_PLAYTESTER_STOPMARK_HAND`).

## 3. The rule

**The line lights as before (one opponent ahead, closing, 7 m to 9 m plus the hand's quarter second) and then only if three gates pass; where one
fails it stays the pale chalk line.** All four numbers live in `FStopRule` (`JamCarrierMarks.h`) and read only positions, velocities and the
cartridge's speeds the mark's code already had; nothing is added to the sim's state.

| gate | number (shipped) | what it says | 0 = |
|---|---|---|---|
| the second man | `SecondSec` **0.5 s** | no other opponent can reach the stopping point within half a second at a free skater's true speed | off |
| the slot | `SlotM` **12 m** | not within 12 m of the mouth of the net he attacks: there the shot is the play | off |
| his own half | `OwnHalfSecondSec` **1.5 s** | behind the centre line (as far from the net he attacks as the goal line's x) the second man must be 1.5 s off: alone with his man, as drill 21 is | off |
| the boards | `BoardRoomM` 0 (off) | the stopping point this close to the boards | measured: the paired table showed no cost by the boards (2-5 m: 0.0 per 100); left off |

The far edge (9.0 m) and the line's place (8.5 m) stay: law 3 on the light (section 2) passes a pro 90 % or more for any press 12 to 21 frames after it
lights. The neutral value is every gate at 0 (`JAM_MARK_RULE "second=0;slot=0;own=0"`): lane A194's line, frame for frame.

**The second arm for the Director** (`second=0.7`): the line lights about once a match at pro (1.0 to 1.3), and an obeying player is as close to the
ignoring one as the first (section 4). The honest rule, read from the paired table alone (the stop pays only alone with his man in the attacking half),
lights about 0.7 times a match: under the brief's two a match. The shipped arm is the one that both keeps the obeying player within 5 points and still
lights about twice a match.

## 4. Proof

**Obeying against ignoring, the same seeds** (`final.sh` on build 6: the rule numbers of the landing; `h` = the class's 6-frame hand, `h15` = a quarter second):

| arm | pro, 600 seeds: wins | lit a match | rookie, 400 seeds: wins | lit a match |
|---|---|---|---|---|
| ignores the line | **294 (49.0 %)** | 2.3 | **382 (95.5 %)** | 2.1 |
| obeys the shipped line, h | **259 (43.2 %)** | **2.2** | **383 (95.8 %)** | 2.4 |
| obeys the shipped line, h15 | 250 (41.7 %) | 2.2 | 381 (95.3 %) | 2.3 |
| obeys the second arm (`second=0.7`), h | 256 (42.7 %) | 1.2 | 387 (96.8 %) | 1.0 |
| obeys the second arm, h15 | 277 (46.2 %) | 1.2 | 386 (96.5 %) | 1.1 |
| obeys lane A194's line (every gate 0), h | 118 (19.7 %) | 14.9 | 323 (80.8 %) | 23.6 |

The paired noise of a 600-seed arm is about 2.4 points. Pooled with the 400-seed arms of `m6` (build 5, the same numbers), 1000 seeds at pro: the shipped
line costs the obeying player **5.4 points** (43.6 % against 49.0 %), the second arm 5.5 (h) / 3.6 (h15); lane A194's line cost **29.3** (19.7 against
49.0). At rookie the obeying player is level with the ignoring one or ahead (383 / 387 against 382). **The brief's bar (within 5 points at pro) is met
at 400 seeds (-4.7) and missed by 0.8 at 600 (-5.8): on the bar, not under it.** The line lights 2.2 times a match at pro (14.9 before): the brief's
two a match, just. Every arm that lit less than that cost about as much: a stop at the light costs a pro about a third of a point a light even where it
is right, because the stopped carry is slower to the net than the scripted player's own cut (section 1: the goals inside 10 s).

**Law 3 on the light** (the drill's stop begun 15 frames after the shipped line lights; `Jam.Core.Movement.SecondMove`, in the gate): **pro 200 of
200 (100 %)**; the rows of section 2 for every hand and level (`Bar.Curve.Law3`).

**The default match's hash**: the regular player with the ladder's closing press, 100 seeds a level, this tree against main 58af359f's binaries
(`hash-main.sh`, `hash-fin.sh`, `same.mjs`): **100 of 100 identical at rookie, pro and all_star** (on build 2 as well: the instrument changed nothing a default match plays).

**Tests**: the full `Jam` filter in a clean environment (`jam.sh`, build 7): **295 pass, 0 fail**; `check-test-guards` PASS; Shipping compiles. Moved: `Jam.Core.Movement.Marks` (the window's scenes read the one-opponent rule with the gates off; new scenes: the stopping point, the second man's gate, the slot, his own half alone and crowded, the boards, the parse, the shipped rule alone with his man and with a second man 3 m from the stopping point), `Jam.Hud.Marks` (the HUD's rule carries the three gates), `Jam.Core.Movement.SecondMove` (pro: the stop begun 15 frames after the light passes 90 % or more: 200 of 200). No state field was added (the sim, the skater and the puck state are untouched).

## 5. What it looks like

Off-screen stills from the game camera, drill 21 (script 1) on the 90 m rink, 1280 x 720, `tools/qa/screen/a188-stills.ps1` with `jam.hud.MarkStills`
(6 = six frames into a showing, 12 = near its last lit frame; the line stays lit 16 to 20 frames in the drill before the stop puts it out):
`movement-12/clay-armed-f302.png`, `clay-lit-f311.png`, `clay-lit-last-f317.png`, `night-armed-f302.png`, `night-lit-f311.png`.

- **Lit**: a solid bar in the closing opponent's colour (orange here) across his path between him and his man, on the clay ice and on the night look's
  blue-white ice alike: the one saturated shape in the middle third of the screen, found in a glance (well inside a third of a second).
- **Armed**: the thin chalk line, faint on white ice (clay): the pale state reads as "not yet / not here", which is what it now means far more often.
- **Its last moment**: the bar sits just short of the defender's skates; still the same thickness and colour (no fade), so a late eye still sees it.
- Thickness and colour by the existing values (`StopBarLitThickM`, `StopBarLitMinPx` 20 px, the team colour); no new look. In drill 21 the HUD log reads
  `gate=0 second_s=1.79-1.89` at the light: alone with his man, the own-half gate (1.5 s) lets it light. A GPU crash (D3D12 device lost) ended the first
  attempt of the clay run; the retry was clean.

## 6. Next evolutions (ranked)

1. **The go that keeps the attack's tempo.** Even where the stop is right, the obeying player loses about a third of a point a light at pro: the stopped
   carry reaches the net later (goals inside 10 s +15.2 -> +8.7 per 100 lights alone with his man). Read the go after the stop (the cut's side and angle
   toward the net he attacks, the drill's "CUT. GO." against the scripted player's own cut) with the paired instrument. Proof: the obeying player within 2
   points of the ignoring one at pro (600 seeds), and the goals inside 10 s of an obeyed light at least the ignoring player's.
2. **The late hand.** The stop on the light passes a pro 90 % or more for a press 12 to 21 frames after it lights and 58.5 % at 24 (0.4 s). A line that
   goes out at the window's far end (it says "too late" by vanishing, as it already does inside 7 m) would stop the late stops a slow hand begins.
   Proof: law 3 on the light + 24 frames 90 % or more at pro with + 15 frames still 100 %.
3. **The loose-puck ring, the same question** (this brief's item 5, not reached): the paired instrument on `JAM_PLAYTESTER_CONTEST=brake:N` (obey only
   the N-th ring). Proof: the table of section 1 for the ring, and the player who obeys every ring within 5 points of the one who ignores it at pro.

# Evolution 12 (lane A201, the same tree: main 58af359f plus evolution 11)

## 7. The late hand: the bar goes out when a press would begin too late (ON)

Law 3's drill, the stop pressed N frames after the line lights, **made only if the bar is still lit on that frame** (`FJamCarryDrill::bOnlyWhileLit`;
a press the bar no longer invites is not made: "not invited"), 200 seeds, pro, by the line's near edge (`FStopRule::LitNearM`, `JAM_MARK_RULE near=`):

| near edge | + 12 | + 15 | + 18 | + 21 | + 24 |
|---|---|---|---|---|---|
| 7.0 m (evolution 11) | 90.5 % of 200 | 100 of 200 | 100 of 200 | 98.5 of 200 | **61.0 % of 187 invited** |
| **7.5 m (shipped)** | **90.5 % of 200** | **100 of 200** | **100 of 200** | **99.5 % of 197** | **not invited (0)** |
| 8.0 m | 90.5 of 200 | 100 of 200 | 100 of 198 | not invited | not invited |

**`LitNearM` 7 -> 7.5 m**: the bar (lit and pale alike) goes out when the defender is 7.5 m off, where a stop begun loses the feet (law 3 at pro:
7.5 m 69 %, 7 m 38 %). Every press a 12- to 24-frame hand makes while it is lit passes 90 % or more; the 24-frame hand is no longer invited
(all_star: + 21 frames 100 % of 158 invited; rookie: + 18 frames 100 % of 146). 8 m would take the 21-frame hand's good stops away. Neutral: `near=7`.
In the gate: `Jam.Core.Movement.SecondMove` (LATE rows: hands 12 / 18 / 24 at pro, 90 % or more of the invited) and `Jam.Core.Movement.Marks`
(7.75 m lit, 7.25 m gone).

## 8. The drill's words (ON)

Drill 21's caption (`FJamCarryDrill::CoachLine`, scripts 1 and 2) was "STOP WHEN HE IS FIVE STICK LENGTHS AWAY. CUT. GO. STILL ON YOUR SHOULDER? CUT
BACK" (two lines and a half in the caption box at 1280). It is now **"LINE LIT: STOP. CUT. GO."** (24 characters, one line; the first wording, "LINE IN HIS COLOUR: STOP. CUT. GO.", wrapped in the caption box: it holds about 30 characters a line at 1280; still `movement-12/clay-caption-e12-f311.png`): what the
line teaches since section 3. The late answers (scripts 3 and 4) and the straight charge keep their lines. `Jam.Core.Movement.SecondMove` holds it
(the words, 28 characters or fewer).

## 9. The loose-puck ring: is going for the puck right when it shows? (ON)

The ring (`JamCarrierMarks::Contested`, shown through `StepShow`'s hold) says "he will be there when you are: going in, you are flattened as you win
it". The obeying player (`JAM_PLAYTESTER_CONTEST=brake`) holds off: he stops short and lets the man have it. It now obeys the ring **as shown** (the
HUD's hold, not the rule frame by frame) and by `brake:N` only the N-th showing (the paired instrument). `RINGSCENE` / `RINGMATCH` rows in
`Bar.Movement.CarryTrace`; readers `rings.mjs`, `ringrules.mjs`.

**The ring as main ships it** (200 seeds, r1): pro: ignoring wins 105, obeying every ring 81 (40.5 %); 20.9 rings a match; his knockdowns 7.95 -> 6.39
a match, at a loose puck (without the puck, or the puck his under half a second) 5.05 -> 3.54. Rookie 194 / 193.

**The paired read** (pro, 1062 rings; going in -> holding off):

| the scene at the ring | share | got it and stayed up 60 frames | down within 2.5 s | the man got it | match (w - l) per 100 |
|---|---|---|---|---|---|
| another opponent at the puck within 0.3 s after him | 63 % | 29 -> 4 % | 24 -> 4 | 3 -> 17 | **-5.4** |
| ... 0.3 to 0.8 s after him | 9 % | 25 -> 20 | 18 -> 13 | 25 -> 29 | +2.1 |
| ... 0.8 s or more | 28 % | 38 -> 24 | 17 -> 13 | 29 -> 33 | 0.0 |
| a mate at the puck before the man | 51 % | 29 -> 3 | 27 -> 4 | 3 -> 21 | -6.6 |
| he is 0.7 s or more from the puck | 50 % | 33 -> 2 | 29 -> 2 | 3 -> 20 | -7.1 |
| all | | 31 -> 11 | 21 -> 7 | 12 -> 22 | -3.2 |

Holding off always saves the knockdown; it costs the match where the puck is a crowd's (another opponent or a mate arrives with them): there the
knockdown is the price of a puck his team keeps. Where he and the one man are alone at it, holding off costs nothing.

**The rule: `FContestRule::OthersSec` 0.3 s** (neutral 0 = the ring of before): no ring when another opponent reaches the puck within 0.3 s after
him. Whole matches, obeying every ring (r2, pro 400 seeds, rookie 200):

| ring rule | pro wins | rings a match | his knockdowns a match | at a loose puck | rookie wins |
|---|---|---|---|---|---|
| ignores it | 196 (49.0 %) | 20.9 | 7.89 | 4.99 | 194 |
| obeys today's ring | 81 of 200 (40.5 %) | 20.6 | 6.39 | 3.54 | 193 |
| **obeys `others=0.3`** | **193 (48.3 %)** | **12.1** | 7.10 | 4.46 | 194 |
| obeys `hsec=0.7` | 172 (43.0 %) | 18.0 | 5.55 | 3.00 | 190 |
| obeys both | 181 (45.3 %) | 11.3 | 6.95 | 4.36 | 192 |
| obeys `hsec=0.55` | 184 (46.0 %) | 17.2 | 5.77 | 3.13 | 196 |

The proof on the final build: section 10.

## 10. Proof (evolution 12; tree 58af359f plus evolutions 11 and 12)

**The ring** (`final2.sh` on build 9, the final rule numbers; the obeying player holds off at every ring as shown):

| | pro, 600 seeds: wins | rings a match | his knockdowns a match | at a loose puck | rookie, 400 seeds: wins | at a loose puck |
|---|---|---|---|---|---|---|
| ignores the ring | **294 (49.0 %)** | 12.4 | 7.73 | 4.90 | **382 (95.5 %)** | 2.29 |
| **obeys the shipped ring** | **289 (48.2 %)** | **12.1** | 7.01 | **4.36** | **385 (96.3 %)** | 1.21 |
| obeys today's ring (`others=0`) | 251 (41.8 %) | 20.2 | 6.08 | 3.31 | 383 (95.8 %) | 1.03 |

Within 5 points at pro (-0.8; before -7.2), ahead at rookie. The ring shows 12 times a match instead of 20. A player who obeys it is down at a loose
puck 4.36 times a match against 4.90 ignoring it (today's ring saved more knockdowns, 3.31, and cost seven points: the knockdowns it saved were the
price of pucks his team kept).

**The late hand** (section 7): in the gate (`Jam.Core.Movement.SecondMove`, LATE rows): hands of 12 / 18 frames invited 200 / 200 and pass 90.5 / 100 %,
the 24-frame hand not invited. **The drill's words** (section 8): in the gate.

**Gate, hash, Shipping**: the full `Jam` filter in a cleared environment on build 11 (the final tree): **295 pass, 0 fail**; test guards PASS; Shipping compiles (build 11's source). The default match's hash on build 10 (build 11 changes only the caption's words): **100 of 100 identical to main 58af359f at rookie, pro and all_star**. No sim state, no cartridge key, no brain change, no state field.

## 11. Next evolutions (ranked)

1. **The go that keeps the tempo** (section 6, item 1): the stop line's obeying player still sits 5 points under the ignoring one at pro. Proof: within 2 points (600 seeds).
2. **The ring tells him where to go instead**: holding off concedes the puck (the man got it 12 -> 22 %); a ring that points to the body position (between the man and the puck) might keep the puck and the feet. Proof: knockdowns at a loose puck under 4 a match at pro with wins within 2 points of the ignoring player.
3. **The stop line's numbers after the 17:50 ladder and the 8-frame body-check wind-up**: re-run `final.sh` and `final2.sh` (paired) on the new main. Proof: both obeying players still within 5 points at pro.
