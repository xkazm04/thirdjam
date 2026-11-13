# Contact feel 1: every outcome reads (lane A101, track F1, device A, 2026-11-13)

Lane A92 made a hit obey momentum (`docs/experiments/CONTACT.md`): impulse against stability, three outcomes. What the player got to SEE and FEEL of it stayed where lane A73
had left it: a hit-stop of three frames and a camera kick on a knockdown that involves the human, and nothing else. A stumble (the new middle outcome) passed without a
frame held, a press refused because the player carried the puck passed without a word, a player who ran into a braced skater and bounced was not told why its legs went,
and the boards were scenery. This evolution makes each outcome read, and makes the boards a body. Worktree `.claude/worktrees/a101` at main ee14438.

## 1. What changed, by outcome

| What happens | Before | Now |
|---|---|---|
| The human presses check while carrying (refused, `bCheckRefused`) | nothing | HUD line **NOT WITH THE PUCK**, bottom centre above the stamina bar, 60 frames (the last 15 fade) |
| The human's own hit makes it stumble (`bRecoilThisFrame`: it ran into a skater more stable than itself) | nothing | HUD line **BOUNCED OFF**; hit-stop 1 frame; camera kick 6 cm along the push on the human |
| A bump (ratio under `bump_ratio` 0.4) | figure's hit reaction | unchanged: no hold, no kick (a bump is not an event) |
| A stumble the human takes or lands (0.4 to 1) | no hold; taken: no kick; landed: a knockdown-sized kick (6 to 12 cm) | hit-stop **1 frame, 2 from the middle of the band** (ratio 0.7); kick **3 cm at 0.4 to 6 cm at 1** |
| A knockdown that involves the human (1 and up) | hit-stop 3 frames whatever the hit; kick 6 to 12 cm by the event's strength | hit-stop **2 frames at ratio 1 to 5 at ratio 3 and up**; kick unchanged |
| A landed hit that pushes its victim into boards within 1.5 m | the open-ice hit | impulse x **`game.contact.board_bonus` 1.25**; the victim's board read-out (`PlayBoardHit`) and a `board` event with the check's strength (glass, sound) |

The ladder a player feels, weakest to strongest: bump (nothing) -> light stumble (1 frame, 3 cm) -> hard stumble (2 frames, up to 6 cm) -> knockdown (2 to 5 frames, 6 to 12 cm).
Each step is at least the one before it (`Jam.Core.Contact.HitStop` and `Jam.Camera.StumbleKick` sweep the ratio and fail on a step down).

## 2. The boards are a body (the one rule change)

`UJamSim::BoardFactor(victim, Dir)`, read by `ResolveContact`:

    gap     = the victim's centre to the boards (BoardDistance)                         only when gap <= 1.5 m (UJamSim::ContactBoardRangeM)
    into    = dot(Dir, the boards' outward normal)                                      Dir = the push, attacker towards victim
    factor  = 1 + (board_bonus - 1) x clamp(2 x into, 0, 1)                             the full bonus inside 60 degrees of straight in, none along the boards or away
    impulse = ContactImpulse x factor                                                   the ratio, the outcome and the frames down all read it

A skater standing against the boards cannot give way into them: the boards send the hit back, so the same closing speed does more. A hit ALONG the boards or away from
them is the open-ice hit (the victim is free to go), which is why the direction is in the rule and the brief's "within 1.5 m" alone is not.
The attacker's own recoil test reads the impulse WITHOUT the factor: the boards answer the victim, not the attacker (measured: 6.5 m/s into a braced skater on open ice
is a stumble at 0.89 AND the attacker recoils; against the boards it is a knockdown at 1.11 and the attacker keeps its feet).

On a landed hit (stumble or knockdown) with a factor above 1 the sim sets, on the victim: `bContactBoard`, `bBoardHitThisFrame` (the pawn calls `PlayBoardHit`: device B's
board-hit take), the counter `ContactBoardHits`, and emits a `board` event: Who = the victim, Kind `skater`, **Strength = the check's strength** (closing speed /
`skater.speed`), Where = the point of the boards nearest the victim, Dir = the normal into the rink. `EffectsBinder` already answers a skater `board` event with the glass
wobble and the board sound. The victim's own slide into the boards within 30 frames of that hit is the same hit: its `board` event and take are suppressed
(`LastContactBoardFrame`). All of these are read-outs: none is hashed, none is read by a rule.

Key: **`game.contact.board_bonus`**, default 1.25, range 1..2 (`schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`, `FJamContactConfig::BoardBonus`, the loader's
`game.contact` block). **Neutral value 1**: `BoardFactor` returns exactly 1.0, the impulse is multiplied by 1.0 (bit-identical), no read-out is set and no event is emitted.

## 3. The presentation

- **Read-out `ContactBy`** (`FJamSkaterState`, one frame, never hashed): the attacker of the hit `ContactOutcome` reports. A stumble emits a `check` event that names the
  attacker only; with `ContactBy` the hit-stop, the camera and the tests find both ends.
- **Hit-stop** (`AJamSkater::HitStopFramesFor(outcome, ratio, contact config, base)`; `AHokejGameMode::HitStopRoute` calls it with `jam.hitstop` as the base). At the default
  base 3: stumble 1 frame under the middle of its band (`(bump_ratio + down_ratio) / 2` = 0.7), 2 from it; knockdown `round(lerp(2, 5, (ratio - down_ratio) / (down_full_ratio - down_ratio)))`;
  the human's own bounce 1; a bump 0. `jam.hitstop` scales the table (x base / 3, a landed hit never under 1, never over 6); 0 turns it off. A knockdown the momentum model
  did not resolve (`contact.momentum false`) holds the base, as before. The log line `JAMHITSTOP` gained `outcome=`, `ratio=`, `base=`; its `kind=` reads `stumble` or `bounce`
  for the two new cases.

  | hit | ratio | frames held |
  |---|---|---|
  | lightest stumble | 0.40 | 1 |
  | side body check from 4 m/s on a standing skater | 0.61 | 1 |
  | the same against the boards | 0.76 | 2 |
  | hardest stumble | 0.99 | 2 |
  | knockdown at `down_ratio` | 1.00 | 2 |
  | full-speed charge into a braced skater | 1.39 | 3 |
  | the acceptance drill's check | 2.34 | 4 |
  | `down_full_ratio` and above (head-on at full speed 4.61) | 3.00 | 5 |

- **Camera kick** (`UJamFollowCamera::StumbleKickScale`, `ContactKickObserve`, `OnJamEvent`). A stumble kicks `KickCm 12 x StumbleKickStrength 0.5 x scale`, the scale
  0.5 at `bump_ratio` to 1.0 at `down_ratio`: **3 cm to 6 cm**, over the same 8 frames and with the same roll rule as lane A73's kick. Three cases: a stumble the human
  takes (no event names the human: the camera reads the sim's read-outs each sim frame; the push is the human's `BumpDir`), a stumble it lands (the `check` event used to
  start a knockdown-sized kick; it now starts the stumble's), and its own bounce (6 cm, along the push on the human, so against its travel). `jam.hitkick` scales all of them.
  Log: `JAMKICK ... stumble_taken | stumble_landed | bounce`.
- **HUD contact line** (`UJamHudWidget::UpdateContactLine`, `ShowContactLine`, `ContactLineFor`). The HUD reads the human's counters `ChecksRefused` and `ContactRecoils` from
  the sim and shows the line when one grows (a counter survives a frame the hit-stop held; a restart sets it back and shows nothing). The bounce wins when both happen in
  one frame. Log: `JAMHUD frame=N contact_line=NOT_WITH_THE_PUCK | BOUNCED_OFF`. The words follow the voice (`docs/experiments/VOICE.md`: short, upper case, never cruel).

## 4. The tests

| Test | What it proves | Measured |
|---|---|---|
| `Jam.Core.Contact.Boards` | one side body check on a standing skater in six places: 1 m from the boards and into them the ratio is the open-ice ratio x `board_bonus`, the board read-out fires, one `board` event for the victim with the check's strength (and none more while it slides into the boards); along the boards, away from them, 2 m out: the open-ice hit, no read-out; half the bonus at dot 0.25; the hit the boards decide; `board_bonus 1`: the open-ice hit, no read-out, no event; the key loads, a value under 1 is refused | 4 m/s: 0.61 -> **0.76**; dot 0.25: 0.69; 6.5 m/s: open ice 0.89 a stumble and the attacker recoils, into the boards **1.11 down 65 frames**, no recoil; bonus 1: 0.89 |
| `Jam.Core.Contact.HitStop` | the hold's table (16 rows: both bands, the base 0, 1, 3, 6, the neutral contact) and that it never shrinks as the ratio grows from 0.40 to 5.00 | section 3's table |
| `Jam.Core.ContactBoardMatch` | three pro matches of three minutes, the bonus against its neutral value: counted (section 6); **the neutral arm hashes as main did** | section 5 |
| `Jam.Camera.StumbleKick` | the stumble's kick: 3.00 cm at ratio 0.40, 4.50 at 0.70, 5.99 at 0.999; never above a knockdown's smallest (6 cm); a knockdown's 6 to 12 cm unchanged; monotone; a degenerate band is finite | as listed |
| `Jam.Hud.ContactLine` | the two lines, the bounce wins, no line for neither; 60 frames with a 15-frame fade | |

`JamContactTests.cpp` (the three `Jam.Core.*` tests; the match counter gained the board bins) and the new `Game/Source/JamGame/Private/ContactFeelTests.cpp`.

**The full `Jam` filter on the final tree: 161 PASS, 0 FAIL** (322 s; `Game/logs/a92-tests-a101-full-ue.log`, the gate's invocation through `tools/qa/ctrl/a92-tests.ps1`;
main's gate at 18:46 had 156: the five new tests are the difference). Three editor builds PASS (181 s, 194 s, 353 s; the last one waited behind the UBT mutex).
A Shipping compile of the Game target on this tree fails only where ee14438 itself fails (`Jam.Core.LiftCone` outside the test guard, lines 945 to 977 of
`JamContactTests.cpp`; main has the fix); every file this lane touched compiled for Shipping without an error. Node tests (the gate's step 3): 210 of 211; the one is
`tools/barks/barks.test.mjs:60`, a CRLF checkout of `BarkData.inl` in this worktree (the test passes in the main checkout).
The three generators re-run after the schema change: gen-schema-data 3 files, gen-parity 87 cases, gen-designer-data 36 cuts.

## 5. The hash, before and after, and the neutral proof

`Jam.Core.Determinism` (hockey.regular, seed 1, frame 600): `f3613b5fcd33ce32` on main (the Director's gate log `Game/logs/premerge-automation-ue-20261113-184648.log`) and
`f3613b5fcd33ce32` on this tree: no landed hit near the boards inside those 600 frames. The matches show the rule:

| `Jam.Core.ContactMatch`, 10 800 frames | main (ee14438) | this tree, `board_bonus` 1.25 (the default) | this tree, `board_bonus` 1 |
|---|---|---|---|
| pro seed 1 | `6ed8f16ab724086b` | `b59b988423ca43cf` | **`6ed8f16ab724086b`** |
| pro seed 2 | `dc1a21fe22e0bc6f` | `2333d70ef430d6f3` | **`dc1a21fe22e0bc6f`** |
| pro seed 3 | `7d34793757849ca9` | `4f3c95b28136e412` | **`7d34793757849ca9`** |
| all_star seed 1 | `81305e9768fec7fe` | `a3716bff403f4f6c` | |
| all_star seed 2 | `d36f3723a630196f` | `75b02e84a155fb56` | |
| all_star seed 3 | `4d48718f184eed47` | `4d48718f184eed47` (no board hit in it) | |

The neutral arm is asserted by `Jam.Core.ContactBoardMatch` against main's three hashes, and its whole count line equals main's field for field (146 / 35 bump read-outs,
10 / 6 check events, 7 / 5 knockdowns, 839 / 686 frames down for seed 1, and so on: `Game/logs/a101-main-contactmatch.txt` beside this tree's log). In the paired curve run
of section 7, 771 of the 1 800 match pairs end on the identical hash: in those matches the boards answered no hit at all.

**Neutral value / revert path:** `"contact": { "board_bonus": 1 }` in a cartridge gives the game before this lane, state hash and events included. The presentation
(hit-stop table, stumble kick, HUD line) is not a rule: `jam.hitstop 0` and `jam.hitkick 0` turn the hold and the kick off as before; the HUD line has no switch.

## 6. Three pro matches, counted (the brief's measurement; `Jam.Core.ContactBoardMatch`, seeds 1 / 2 / 3, the Regular playtester as the human, three minutes each)

| | `board_bonus` 1.25 | `board_bonus` 1 (= main) |
|---|---|---|
| hits resolved (a pressed check at contact or a cross-check) | 17 / 24 / 12 = 53 | 17 / 18 / 14 = 49 |
| ... as a bump | 3 / 6 / 1 = 10 | 1 / 2 / 1 = 4 |
| ... as a stumble | 3 / 5 / 4 = 12 | 4 / 2 / 4 = 10 |
| ... as a knockdown | 11 / 13 / 7 = 31 | 12 / 14 / 9 = 35 |
| knockdowns of the human / of a bot | 5 / 9 / 1 and 6 / 4 / 6 | 5 / 9 / 2 and 7 / 5 / 7 |
| the human's recoils (bounces) | 1 / 1 / 0 | 1 / 0 / 0 |
| **hits on a victim within 1.5 m of the boards** | 4 / 2 / 3 = 9 = **17 %** | 6 / 3 / 3 = 12 = **24 %** |
| ... pushing straight into them (dot 0.5 and up): bump / stumble / down | 0 / 0 / 1 | 0 / 0 / 2 |
| ... glancing (dot 0 to 0.5) | 2 / 0 / 2 | 1 / 0 / 3 |
| ... along the boards or away | 0 / 1 / 3 | 0 / 1 / 5 |
| hits 1.5 to 3 m from the boards (all along or away) | 1 | 3 |
| landed hits the boards answered (read-out, event) | 1 / 1 / 1, all knockdowns of a bot | 0 |
| ... of which the bonus changed the outcome | 0 | |
| goals | 1:2, 1:1, 2:1 | 1:3, 1:0, 2:1 |

**Reading.** About a fifth of the hits happen within 1.5 m of the boards, but half of those run along the boards or away from them (a chaser hits from behind along the
wall): the boards answer **one landed hit a match**, and in these three matches each was a knockdown already. The rule is right where it applies (section 4) and rare in
play: it will be felt as "that one hit into the boards" and not as a new way of playing. Making it a play (the brain finishing its check INTO the boards, the hit that
sends a victim into boards 3 m away) is the next evolution of this track (section 9).

One headless game process (`-game -nullrhi`, the scripted human, hockey.regular, 10 632 frames; `tools/commentary/out/a101-match/game.log`): the human went down 9 times;
`JAMHITSTOP` held 3, 2, 5, 3, 5, 3, 4, 2, 2 frames for ratios 1.82, 1.29, 3.98, 1.46, 5.58, 1.35, 2.31, 1.12, 1.17 (main: 3 every time); `JAMKICK` 6.0 to 12.0 cm;
`JAMVIS board ... checked=1` four times (twice on the human) beside 29 plain board hits. The tackle drill (`jam.ctrl.Drill 1`, the human's ten scripted tackles;
`tools/commentary/out/a101-drill1/game.log`): eight landed, held 5, 3, 3, 2, 3, 2, 3, 2 frames for ratios 4.76, 2.00, 1.39, 1.17, 1.79, 1.26, 1.52, 1.27.
The stumble's path in a live process, on a variant with `contact.down_ratio 5` so that a match's hits land as stumbles (the band is then 0.4 to 5, its middle 2.7;
`tools/commentary/out/a101-stumble-match/game.log`, 5 976 frames): the human stumbled five times; `JAMHITSTOP ... kind=stumble` held 1, 2, 1, 1, 1 frames for ratios 1.82,
2.74, 2.18, 1.12, 2.04 and `JAMKICK ... stumble_taken` kicked 3.9, 4.5, 4.2, 3.5, 4.1 cm along the push; the one knockdown in it (ratio 5.06) held 5 and kicked 12.0 cm.
The same variant under the tackle drill (a stumble the human LANDS) did not start inside the time box: its process sat behind the UBT mutex for seven minutes.

## 7. The three target cells (R-0046; 600 matches a cell and arm, paired: seeds 1 to 600 in both arms; `tools/perf/results/a101/paired.mjs`, rows in `cells.jsonl`)

| cell | target | `board_bonus` 1.25 (95 % interval) | `board_bonus` 1 | difference | pairs that flipped (1.25 wins only / 1 wins only) | identical hash |
|---|---|---|---|---|---|---|
| novice at rookie | 85 | **85.2** (82.1 to 87.8) | 83.7 (80.5 to 86.4) | +1.5 | 45 / 36 | 290 of 600 |
| regular at pro | 50 | **53.8** (49.8 to 57.8) | 51.3 (47.3 to 55.3) | +2.5 | 85 / 70 | 246 of 600 |
| good at all_star | 25 | **25.3** (22.0 to 29.0) | 26.0 (22.7 to 29.7) | -0.7 | 66 / 70 | 235 of 600 |

No cell moves by more than 4 points, and no difference is distinguishable from zero on 600 pairs (the flips are 45 against 36, 85 against 70, 66 against 70). Checks
taken a match: 3.67 / 8.44 / 7.76 with the bonus, 3.64 / 8.44 / 7.71 without. **The bonus ships at its default 1.25.** The pro cell reads 53.8 on these seeds, 3.8 from
its target and inside the 4-point bar; its neutral arm reads 51.3 on the same seeds (R-0046's 48.3 was on A53's seed bands at 2 000), so 2.5 of the 3.8 is the bonus
and 1.3 the seed set. If the curve track wants the cell nearer 50, `board_bonus` 1.15 is the first lever to try.

Why this tool and not `tools/perf/results/a95/curve.mjs`: every `UnrealEditor-Cmd` start runs `Build.bat -Mode=ValidatePlatforms`, which queues behind the UBT mutex; with
five lanes building and the soak's workers starting, a tournament process sat 8 minutes before its first match. `curve.mjs` starts 12 processes a cell; `paired.mjs`
starts one per cell arm (six in all, 371 s for the 3 600 matches), and pairing the seeds gives the difference the brief asks for directly.

## 8. For device B, and the contract texts this lane proposes (the Director's to write)

- **C-0001 (`contracts/skater-visual.md`), `PlayBoardHit`:** "one-shot: this skater hit the boards at 4 m/s or more along the normal, **or took a landed check that pushed
  it into boards within 1.5 m (`game.contact.board_bonus` above 1)**; the frame of the skater `board` event, Who = this skater; Strength on the bus: normal speed /
  `skater.speed` for the former, the check's strength for the latter. On a check into the boards the same frame also carries `SetKnockedDown(true, ...)` or
  `PlayHitReaction` (a stumble): the figure decides which take wins."
- **`contracts/events.md`, `board`:** add the skater case: Who = the skater, Kind `skater`, Strength = normal speed / `skater.speed`, or the check's strength when the
  skater was checked into the boards; Dir = the boards' normal into the rink (the row today describes the puck only).
- **Asks to B** (nothing under `Game/Plugins/JamAssets` was touched): (1) a board-hit take that reads with a knockdown in the same frame (the fall against the boards);
  (2) the board sound and the glass wobble scaled by the event's Strength, if they are not; (3) a stumble take of its own: today a bump and a stumble both arrive as
  `PlayHitReaction(DirX, DirZ)`; a `PlayStumble(DirX, DirZ, Frames)` (the sim's `StumbleFrames`, 16) would let the figure lose its feet for exactly the frames the player
  has no control (C-0001 v5, section 9 point 2).

## 9. Not proven here, and what comes next

**Not proven by this lane (it does not hold the cursor): queued for a package.**
1. The acceptance rows **check / crosscheck / checked** on a Development package of this tree (`tools/qa/accept/accept.ps1 -From 7 -To 8`). Expected to hold: the drill's
   hits are in open ice, and the drill reads the sim's `KnockFrames` and `ContactOutcome`, which the presentation does not touch.
2. How the HUD line, the 1-to-5-frame hold and the 3-to-6 cm kick LOOK: one capture of a refused press, one of a bounce, one of a stumble taken, one of a check into the
   boards (`jam.ctrl.Stills 1` covers knockdowns and board hits today; a stumble has no still trigger yet).
3. The HUD line in a live process: the scripted human and drill 1 never press check while carrying and never bounced in the runs above, so `JAMHUD contact_line=` was not
   logged headless. The line's inputs are the two counters, which `Jam.Core.Contact.CarrierRefused` and `Jam.Core.Contact.AttackerBounces` cover; `UJamHudWidget::GetContactText()`
   is there for an acceptance row (ask to S2: "refused": the human carries and presses RMB, the row reads the text).

**Small edges, known and left** (presentation only, no rule): the `JAMHITSTOP` line of a stumble prints `strength=0.00` (the strength is read from the knockdown event,
which a stumble has not; `ratio=` is the number to read); a frame that carries a bot-on-bot knockdown AND a stumble of the human holds nothing (the route reads the
knockdown first); `tools/qa/ctrl/a73-analyze.mjs` counts `JAMHITSTOP` lines against the human's knockdowns and would now count stumbles too (filter on `outcome=3`).
`jam.ctrl.Stills 1` already captures the human's board hit, so a check of the human into the boards gets its eight stills in a package without a new trigger.

**The next three evolutions of track F1, ranked, each with its proving number.**
1. **The board check as a play.** Today the boards answer one hit a match. (a) The hit that SENDS a victim into the boards counts: a stumbling or falling skater that
   reaches the boards within its stagger at 3 m/s or more takes the boards as a second hit (a stumble becomes a knockdown, a knockdown lasts longer), so a check 3 m out
   towards the wall matters; (b) the ask to Q2's brain: the defender that steers the carrier to the boards finishes INTO them (its approach angle), not along them.
   Proving number: landed hits the boards answered per pro match from **1.0 to 3 or more** (`Jam.Core.ContactBoardMatch`'s bins), the three cells within 4 points, neutral
   key `contact.board_slam` 0.
2. **The soft hit explains itself, and the stumble has its own body.** A pressed check that resolves as a bump says nothing: the player pressed and nothing happened
   (10 of 53 hits in section 6). HUD line **TOO SOFT** for the human's own bump (and **MISSED** for a window that closes on nobody), a stumble take from B
   (`PlayStumble`), a still trigger for stumbles. Proving number: share of the human's check presses that end with no feedback at all (no line, no hold, no kick, no
   event), counted in a soak: from today's share (the bumps and the whiffs) to **0**; the stumble's pose distinct from the bump's in two stills.
3. **A hard turn lowers stability** (VISION law 1's factor that lane A92 left out for want of a turn state): `contact.turn_stability` 0.85 at full yaw rate, neutral 1.
   The carrier's cut then has its price, which is what makes law 3's "stop, cut, go" a risk and not a free move. Proving number: `Jam.Core.Contact.TurnFalls` (the same hit:
   the straight skater stumbles at 0.9, the turning one falls at 1.06), the three cells within 4 points, the hash at the neutral value = main's.

## 10. Files

Sim and key: `Game/Source/JamCore/Private/JamSim.cpp` (`BoardFactor`, `ResolveContact`, the board event's gate in `CollideSkaterStatic`), `Public/JamSim.h`,
`Public/JamCartridge.h`, `Private/JamCartridge.cpp`; schema `game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md` and the generated `JamSchemaData.inl`,
`JamDesignerData.inl`. Presentation: `JamPawns.cpp/.h` (`HitStopFramesFor`), `CameraFollow.cpp/.h`, `HUDWidget.cpp/.h`, and **one function outside the track's named
files: `AHokejGameMode::HitStopRoute`** (`HokejGameMode.cpp`; the hit-stop's routing lives there). Tests: `JamContactTests.cpp`, `JamGame/Private/ContactFeelTests.cpp`.
Measurement: `tools/perf/results/a101/paired.mjs`, `cells.jsonl`. Status: `status/A101.md`.
