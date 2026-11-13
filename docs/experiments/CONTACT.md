# Contact by momentum: impulse against stability, three outcomes, the carrier cannot hit (lane A92, device A, 2026-11-13)

R-0045 and `docs/VISION.md` law 1: "a hit obeys inertia". Before this lane a check landed when the checker's own speed along the contact passed
`skater.check_speed_min` inside the press window, and it always knocked the victim down for `skater.knockdown_frames`: two skaters going the same way hit as hard
as a head-on charge, and a standing skater fell like a sprinting one. This page is the model that replaces it, its keys, the neutral block that gives the old
game back, the tests, and the numbers of a bot match against main's. Worktree `.claude/worktrees/a92` at main 8f6b157.

## 1. The model

Where it lives: `UJamSim::StabilityOf`, `UJamSim::ContactImpulse`, `UJamSim::ResolveContact` (`Game/Source/JamCore/Private/JamSim.cpp`), called from the
skater-skater collision (a pressed body check at contact) and from `TryLift` (the cross-check). There was no `TryCheck` function on main: the check rule lived
inline in `CollideSkaters`; it is still there, verbatim, as the `contact.momentum false` branch.

**A hit is resolved only for a pressed check at contact or a cross-check.** A collision nobody pressed stays the bump and the bounce it was (the read-out
`bBumpThisFrame` from 3 m/s of closing, the elastic exchange with restitution 0.2). Otherwise every unpressed head-on collision of two bots would be a knockdown and
"the carrier cannot hit" would mean nothing (a carrier skating into a defender would still knock it down).

**Impulse** (kg m/s), along `Dir` = the unit vector from the attacker to the victim:

    closing  = dot(attacker.Vel - victim.Vel, Dir)
    impulse  = mass(attacker) * max(0, closing + push)        push = lunge_mps (body check, 1.5) | thrust_mps (cross-check, 3.0)

**Stability** (kg m/s) of the victim against that hit, `StabilityOf(skater, Dir, kind)`:

    stability = mass(victim) * stability_mps (6)
      * stand_brace (1.5)       when |Vel| < stand_speed_mps (1) and the victim sees the hit
                                (not from behind; and not a cross-check inside its front arc: the thrust "to the chest" ignores the brace)
      * lerp(1, speed_stability (0.85), (|Vel| - stand_speed_mps) / (top speed - stand_speed_mps))     when it moves
      * sprint_stability (0.7)  while it sprints (`bSprinting`)
      * spin_stability (0.8)    in a spin, or with a full shot wind-up (`ChargeFrames` at `shot.charge_frames_max`)
      * unseen_stability (0.8)  when the push runs along its facing (dot(Dir, Forward) > 0.5: from behind); such a hit also finds a standing skater unbraced

"From behind" and "front arc" are the thresholds the check event's `Kind` and the foul rule already used (0.5 and -0.5 of dot(Dir, Forward)).

**Ratio and outcome**:

    ratio = impulse / stability        (for the outcome, x 5 / skater.check_speed_min of the attacker; the frames down read the unscaled ratio)
    ratio <  bump_ratio (0.4)             bump:    both keep their feet and their control; the speed changes hands in the collision's bounce only
    bump_ratio <= ratio < down_ratio (1)  stumble: the victim loses half its speed and its control for stagger_frames (16; the sim's existing `StumbleFrames`:
                                                   no thrust, no steering, no stick); from loose_ratio (0.7) the carried puck comes loose
    ratio >= down_ratio                   down:    the existing knockdown path (`KnockDown`: the event, the strip, the spin's x1.5), for
                                                   lerp(down_frames_min (60), down_frames_max (150), (ratio - down_ratio) / (down_full_ratio (3) - down_ratio))
                                                   * skater.knockdown_frames / 90 frames

A stumble and a knockdown are a landed check: the `check` event (Strength = closing speed / `skater.speed`, clamped, as `contracts/events.md` says; Kind front /
side_l / side_r / behind), the `on_check` rule hook, the foul rule (from behind on the carrier: `foul_from_behind`; on a skater without the puck: `foul_no_puck`)
as before. A bump emits no event. A skater already stumbling is not stumbled again (the hit reads as a bump), so a bot leaning on a victim cannot lock it.

**The attacker pays.** The pushed speed changes hands: the victim gains half of the lunge (or, for a cross-check, half of closing + thrust, since the bodies may
not touch) times mass(attacker) / mass(victim), the attacker loses half of it; a body check's closing speed changes hands in the collision's exchange right after.
And the same impulse comes back on the attacker: when its own stability is the lower of the two, the victim stays up and the impulse is at least the attacker's own
stability (x `down_ratio`), the attacker is the one who stumbles (`bRecoilThisFrame`: half its speed, `stagger_frames` without control). A sprinting skater into a
braced standing one bounces off.

**The carrier cannot hit** (`UJamSim::CarrierMayNotHit`). A check pressed while carrying opens no window (`bCheckRefused` for one frame, the counter
`ChecksRefused`; no `PlayCheck`), and a window opened earlier resolves nothing at contact once the skater carries. One grace, from a measurement: a skater is
"the carrier" for this rule only when no opponent carried the puck within `stick.foul_grace_frames` (12). Charging a carrier head-on, the attacker's body or blade
touches the puck a frame or two before the bodies meet, and on the first model that made the attacker the carrier and refused its hit (the acceptance drill's
step 7 failed exactly so). It is the mirror of R-0029 point 3, where the victim that just lost the puck still counts as the carrier for the foul rule; it reads
the same `LastCarryFrame`. The armed hip check (A84's ability) is exempt. The cross-check needed no change: the lift looks for an opposing carrier, so a
carrier's lift was a miss already. `contact.carrier_can_hit true` gives the press back.

**The cross-check** is the lift (MMB) at an opposing carrier: its body part is resolved before the strip, inside three limits and one grace, each from a measurement:

- **At arm's length only**: the gap between the bodies within `cross_reach_m` (0.5 m: the shaft, not the blade). Farther out, up to `stick.lift_range_m` (1.0),
  the press is the stick lift it was. On the first model every lift in range shoved, and the brain lifts as a matter of course (40 to 60 presses by bots in three
  minutes): the scripted human went down 14 / 14 / 10 times in three minutes at pro (main: 4 / 1 / 1), every one of them by a cross-check, and lay on the ice for
  10 to 16 % of the match. With the reach: 4 / 4 / 5, and 6 / 3 / 6 on the final tree once the take in the same breath also counts (section 6).
- **Not from behind**: a lift from behind the carrier stays a stick lift (no shove, no foul). The brain steals the puck that way and the regular cut has
  penalties on: a shove from behind would be a `foul_from_behind` on every such steal.
- **The take in the same breath**: with no opposing carrier in range, the thrust lands on an opponent at arm's length that carried within
  `stick.foul_grace_frames` (the attacker's blade took its puck on the way in, or the puck just left it); the lift has nothing to take and reads as a miss.
  The acceptance run found it (`a92-final4`: on a slow glide the human's blade took the standing carrier's puck and the MMB hit nobody).
- **Not where the lift itself is shielded** (the spin's body between, the sweep's side, the shield ability): the press reads as the miss it was. VISION law 3 says
  the sweep and the spin shield the puck from sticks, never from bodies, and the body check does beat them (a check mid-spin weighs x1 / 0.8 and lies x1.5);
  the cross-check is delivered with the stick, and R-0040's bars for the spin and the sweep (still unmet, PLAN-RECONCILED point 3) would fall further.

**What stays of the older levers.** `skater.knockdown_frames` scales the down frames (value / 90), so the raw cut (150) and the modes that lengthen knockdowns
keep their meaning. `skater.check_speed_min` scales the hit (5 / value) for the outcome: Bruisers (1.5), Fog of War (1), Cage and Glass House (3) wrote it low to
make hits land from low speeds, and now a hit there weighs x3.3, x5, x1.7 when the outcome is decided; the frames down read the physical ratio, so a low-speed
knockdown in such a mode is a short one. The brain still reads the key as the closing speed a bot presses from. Section 6 has those modes measured. The armed hip check (lane A84) keeps its rule (lands
from a standstill, down for magnitude x `knockdown_frames`, the push). The human's check no longer lands from 2 m/s of its own speed (`HumanCheckSpeedMinMs`, lane
A49): under the model the human hits by the same law as a bot; its longer press window (12 frames) stays.

Worked numbers at the defaults (mass 80 both, `skater.speed` 11):

| Hit | closing + push | victim's stability / mass | ratio | outcome |
|---|---|---|---|---|
| same direction, same speed, pressed from behind (victim at 8 m/s) | 0 + 1.5 | 6 x 0.895 x 0.8 = 4.30 | 0.35 | bump |
| the same from the side | 1.5 | 5.37 | 0.28 | bump |
| head-on, both at 11 m/s | 22 + 1.5 | 6 x 0.85 = 5.1 | 4.61 | down 150 |
| head-on at 4 and 3 m/s | 7 + 1.5 | 5.82 | 1.46 | down 81 |
| side body check from 4 m/s on a standing skater | 4 + 1.5 | 6 x 1.5 = 9 | 0.61 | stumble |
| front cross-check from 4 m/s on a standing carrier | 4 + 3 | 6 | 1.17 | down 67 |
| side cross-check from 4 m/s on a standing carrier | 4 + 3 | 9 | 0.78 | stumble, the puck loose |
| side hit closing 3.5 m/s on a victim walking at 5 m/s | 5 | 5.6 | 0.90 | stumble, the puck loose |
| the same on a sprinting victim | 5 | 5.6 x 0.7 = 3.9 | 1.28 | down 73 |
| full-speed charge (11 m/s) into a standing braced skater | 12.5 | 9 | 1.39 | down 78 |

## 2. The keys: `game.contact` (all optional; unknown keys are errors; `schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`, `FJamContactConfig`)

| Key | Default | Range | Meaning |
|---|---|---|---|
| `momentum` | true | bool | false = the neutral value: the check rule before this lane |
| `carrier_can_hit` | false | bool | true = the neutral value: the carrier may body-check |
| `lunge_mps` | 1.5 | 0..6 | speed a pressed body check adds to the closing speed |
| `thrust_mps` | 3 | 0..10 | speed a cross-check adds; 0 = the cross-check has no body part (the lift only) |
| `cross_reach_m` | 0.5 | 0..1.5 | the gap between the bodies inside which the lift is a cross-check; `stick.lift_range_m` or more = every lift in range shoves |
| `stability_mps` | 6 | 1..30 | the closing speed of an equal-mass hit that puts a slow, unbraced, seeing skater down |
| `stand_speed_mps` | 1 | 0..5 | under it a skater stands braced; above it stability falls with speed |
| `stand_brace` | 1.5 | 1..3 | the standing brace |
| `speed_stability` | 0.85 | 0.3..1 | stability factor at top speed |
| `sprint_stability` | 0.7 | 0.3..1 | while sprinting |
| `spin_stability` | 0.8 | 0.3..1 | in a spin or at a full wind-up |
| `unseen_stability` | 0.8 | 0.3..1 | hit from behind |
| `bump_ratio` | 0.4 | 0..1 | under it: a bump |
| `loose_ratio` | 0.7 | 0..3 | a stumble from this ratio knocks the puck loose |
| `down_ratio` | 1 | 0.2..5 | from it: down |
| `down_full_ratio` | 3 | 1..10 | the ratio of `down_frames_max` |
| `down_frames_min` | 60 | 15..300 | frames down at `down_ratio` (at `knockdown_frames` 90) |
| `down_frames_max` | 150 | 15..300 | frames down at `down_full_ratio` and above |
| `stagger_frames` | 16 | 0..60 | the stumble; 0 = no stumble (the hit is a bump until it is a knockdown) |

The brief's eleven names are kept; added: `momentum`, `carrier_can_hit` (the two neutral switches), `cross_reach_m`, `stability_mps` (the base the ratios are read against),
`stand_speed_mps`, `speed_stability`, `spin_stability` (VISION's "moving lowers it with speed", "a spin or a full shot wind-up"), `down_full_ratio`. No cross-field
check was added: the sim reads `max(down_frames_min, down_frames_max)` and tests `down_ratio` before `bump_ratio`. VISION's "a hard turn lowers it" is not in: the
sim has no turn state the contact may read without touching the movement step (A93's); it would be one factor on `YawRate`.

## 3. The neutral block

    "contact": { "momentum": false, "carrier_can_hit": true }

`momentum false`: a pressed check lands when the checker's own speed along the contact passes `skater.check_speed_min` (the human from 2 m/s) and knocks down for
`skater.knockdown_frames`; the cross-check is the lift only; every other key of the block is unread. `carrier_can_hit true`: the carrier's press opens its window.
The two are independent (momentum on with the carrier allowed to hit, or the old rule with the carrier refused, both work). `FJamContactConfig::SetNeutral()` sets
both. Proof: section 5.

## 4. The tests (`Game/Source/JamCore/Private/Tests/JamContactTests.cpp`, filter `Jam.Core.Contact`)

| Test | What it proves | Measured |
|---|---|---|
| `Jam.Core.Contact.SameDirection` | same direction and speed, pressed from behind and from the side: a bump, no event, the victim keeps its speed | ratio 0.35 / 0.28 |
| `Jam.Core.Contact.HeadOn` | head-on at full speed: down for `down_frames_max`, strength 1; a slower head-on: down for fewer frames | 4.61 -> 150; 1.46 -> 81 |
| `Jam.Core.Contact.StandingBrace` | standing skater: side body check from 4 m/s stumbles it at most; front cross-check from 4 m/s (gap 0.4 m) puts it down; side cross-check: a stumble; the same front press from the blade's length (gap 0.8 m): the lift only; from behind: the lift only, no foul; `StabilityOf` by the numbers (six cases) | 0.61 stumble; 1.17 down 67; 0.78 stumble |
| `Jam.Core.Contact.SprintFalls` | the same side hit: the walking victim stumbles, the sprinting one falls | 0.90 stumble; 1.28 down 73 |
| `Jam.Core.Contact.AttackerBounces` | sprinting at 5 m/s into a braced skater's side: the victim stays up, the attacker recoils (16 stumble frames, under half its speed); walking in at 3 m/s: no recoil | victim 0.85, attacker 5.91 -> 1.53 m/s |
| `Jam.Core.Contact.CarrierRefused` | the carrier's press: refused, no window, no hit head-on at 8 m/s each; `carrier_can_hit true`: the same press is down 150; a check at a carrier with the puck between the bodies lands (the take in the same breath); a cross-check on a skater that lost the puck 2 frames ago lands (down 67), 30 frames ago hits nobody; the carrier's cross-check hits nobody | |
| `Jam.Core.Contact.Fouls` | from behind on the carrier: the hit lands and `foul_from_behind`, the fouler in the box; on a skater without the puck: `foul_no_puck`; clean front hit: no foul; a bump from behind: no foul | 1.61 down 87; 2.00 down 105 |
| `Jam.Core.ContactNeutral` | the neutral block's bot match hashes as main's; the block loads from cartridge text; absent = the model; an unknown key is an error | section 5 |
| `Jam.Core.ContactMatch` | the three-minute match under the model, counted; two runs agree | section 6 |
| `Jam.Core.Contact.Levers` | `knockdown_frames` 150: the biggest hit lies 250; `check_speed_min` 2.5: the side check that stumbled a standing skater (0.61) puts it down (1.22) for `down_frames_min` | 250; 60 |
| `Jam.Core.ContactTakes` | a measurement: the two rookie takes over ten seeds, model against neutral (the AI's shots, the score) | section 9 point 1 |
| `Jam.Core.ContactModes` | a measurement: Bruisers, Fog of War, Cage, Glass House and the raw cut, model against neutral | section 6 |
| `Jam.Core.TacklingEvents` (A33) | now runs on the neutral block: the old rule's twenty hits, kinds, strengths, 3 m/s gate, human's 2 m/s | unchanged |
| `Jam.Core.TacklingReadouts` (A45) | on the model, unchanged: the unpressed bump, the board flag, the pin, the lift from behind (no shove) | unchanged |

Four tests of other lanes assumed the old rule and got one marked edit each: `Jam.Core.Abilities.Shield` (`JamAbilityTests.cpp`: "the check lands" on the
unshielded standing carrier is a stumble at 7 m/s, ratio 0.9, not a knockdown), `Jam.Core.ModeKeys.BodyCheck` (`ModeKeysTests.cpp`: the variant writes
`contact.thrust_mps 0` so only body checks are counted: with `ai.body_check 0` the bots' cross-check still shoved the human), `Jam.Core.SpinCheckedFalls`
(`JamStickHandlingTests.cpp`: the plain knockdown is inside `down_frames_min..max`, the spin's at least 1.5x it: 140 against 77), `Jam.Core.Stick`
(`JamStickTests.cpp`: the puck the lift sent to the charger is parked, so the charger is not the carrier). `Jam.Core.Abilities.HipCheck` and
`Jam.Core.SpinShieldsLift` pass untouched (the hip check's exemption, the shield limit).

Full `Jam` filter on the final tree: **143 PASS, 1 FAIL** (`Game/logs/a92-tests-final3-ue.log`); the failure is `Jam.Bot.SeasonTakes`
take-2 seed 2 (section 9 point 1). Schema tests 99 of 99; the three generators re-run (gen-schema-data 3 files, gen-parity 87 cases, gen-designer-data 36 cuts).

## 5. The hash, before and after, and the neutral proof

`Jam.Core.Determinism` (seed 1, frame 600, the scripted run with random check presses):

| | hockey.regular | hockey.raw |
|---|---|---|
| main 8f6b157, untouched sim (`Game/logs/a92-tests-before-ue.log`) | `bd698403ed205fbd` | `0e5637bb401646ae` |
| this lane, the model (the default) | `41a567c37ddcbee6` | `0e5637bb401646ae` (no pressed contact in its 600 frames) |

The neutral proof is a harder scene than those 600 frames: `Jam.Core.ContactMatch`'s match (hockey.regular, penalties off, no score limit, 10 800 frames, seed 1,
the Regular playtester as the human, the brain everybody else), measured first on the untouched sim with only the new test file added:

| | pro | all_star |
|---|---|---|
| main 8f6b157, untouched sim | `987573b13c831bda` | `fe206fa475ecbfd6` |
| this lane, the neutral block (`Jam.Core.ContactNeutral`) | `987573b13c831bda` | `fe206fa475ecbfd6` |
| this lane, the model | `827a01512554df0b` | `53f75bf1748a425d` |

No hashed field was added (the stumble is the existing `StumbleFrames`); the read-outs (`ContactOutcome`, `ContactRatio`, `bContactCross`, `bCheckRefused`,
`bRecoilThisFrame`, the counters) are never hashed and never read by the sim.

## 6. A bot match, three minutes, pro and all_star (headless, `Jam.Core.ContactMatch`)

hockey.regular (5 v 5), penalties off, no score limit, 10 800 frames; the Regular playtester plays the human, the brain everybody else. "main" is the neutral
block in the same binary (its seed 1 hashes are main's, section 5). Knockdowns are `knockdown` events; stumbles and pressed bumps are the model's read-out
(`ContactOutcome`) by victim; "by cross-check" is the share of those delivered by the lift's shove. Final tree, `Game/logs/a92-tests-final3-ue.log`.

| Level, seed | Knockdowns of the human: main -> model | of a bot (bot on bot): main -> model | Stumbles human / bot | Pressed bumps human / bot | By cross-check (stumble / down) on the human, on a bot | Check events by bots / by the human: main -> model | Frames down, human / bots | Recoils | Refused | Goals for:against: main -> model |
|---|---|---|---|---|---|---|---|---|---|---|
| pro, 1 | 4 -> 6 | 3 (2) -> 7 (5) | 11 / 1 | 3 / 7 | 11 / 4, 0 / 4 | 6 / 1 -> 22 / 3 | 651 / 798 | 1 | 0 | 3:2 -> 0:2 |
| pro, 2 | 1 -> 3 | 6 (1) -> 2 (0) | 3 / 4 | 4 / 6 | 3 / 2, 4 / 1 | 2 / 5 -> 6 / 6 | 289 / 150 | 0 | 0 | 1:2 -> 3:1 |
| pro, 3 | 1 -> 6 | 7 (2) -> 3 (1) | 6 / 5 | 2 / 7 | 6 / 2, 5 / 1 | 3 / 5 -> 15 / 5 | 512 / 274 | 0 | 0 | 3:1 -> 1:2 |
| all_star, 1 | 0 -> 7 | 6 (2) -> 8 (7) | 1 / 6 | 2 / 5 | 1 / 5, 6 / 6 | 2 / 4 -> 18 / 4 | 833 / 969 | 0 | 0 | 0:1 -> 1:2 |
| all_star, 2 | 2 -> 4 | 1 (0) -> 8 (3) | 5 / 5 | 3 / 8 | 5 / 3, 4 / 4 | 2 / 1 -> 12 / 10 | 508 / 756 | 1 | 0 | 1:2 -> 2:1 |
| all_star, 3 | 2 -> 4 | 5 (2) -> 6 (3) | 3 / 6 | 3 / 6 | 2 / 3, 5 / 2 | 4 / 3 -> 14 / 6 | 520 / 543 | 1 | 0 | 1:2 -> 1:3 |

Model hashes: pro `827a01512554df0b`, `8d8eafe91e551308`, `6ac8bae86d5263d9`; all_star `53f75bf1748a425d`, `566297708ab6c8e5`, `6724bc89d9c35aff` (two runs of
each seed 1 agree). Lift presses by bots 36 to 58 a match, by the playtester 19 to 35, as on main (32 to 57, 16 to 39).

What the numbers say:

- **Knockdowns of the human: 15 in the three pro matches (main 6), 15 at all_star (main 4)**; of bots 12 (main 16) and 22 (main 12). Two to three times main for
  the human, the same order for the bots. A80's packaged match on main (the scripted human of `jam.ScriptHuman`, 3 v 3 then) had 5, all of the human, none bot on
  bot; this headless match on main has bot-on-bot knockdowns already (2 / 1 / 2 at pro), and the model has 5 / 0 / 1 at pro and 7 / 3 / 3 at all_star.
- **The new outcome is the stumble**: 20 of the human at pro, 9 at all_star; 10 and 17 of bots. Every stumble of the human at pro is a cross-check (20 of 20): the
  pro brain's lift when it happens at arm's length. About half of the knockdowns are cross-checks too (the human: 8 of 15 at pro, 11 of 15 at all_star).
- **The human loses control about six times as often at pro** (35 stumbles and knockdowns in three matches against 6 knockdowns on main), four times at all_star
  (24 against 4), and the playtester scored 4 in the three pro matches (main 7) and 4 at all_star (main 2). Three seeds are a small sample; the curve needs its
  pass on the whole (A86, after A93 and A94 land).
- **Down time**: the human lies 289 to 651 frames of 10 800 at pro (2.7 to 6.0 %; main 90 to 360), 508 to 833 at all_star (main 0 to 180).
- Recoils are rare (3 in six matches): bots seldom charge a standing skater. The carrier's refused press: 0 (the playtester does not press check while carrying).

Two earlier states of the model, for the record: with every lift in range shoving and no grace for the take (`a92-tests-full1-ue.log`) the human went down
14 / 14 / 10 times at pro and 7 / 8 / 9 at all_star, all by cross-check, 1 127 to 1 682 frames down at pro; with `cross_reach_m` but without the cross-check's
take in the same breath (`a92-tests-full2-ue.log`) 4 / 4 / 5 and 1 / 3 / 6.

**The cuts that wrote the older levers** (`Jam.Core.ContactModes`: one seed each, their own level and rules, up to 10 800 frames):

| Cut (`check_speed_min`, `knockdown_frames`) | Knockdowns of a bot / of the human: main -> model | Stumbles bot / human | Frames down, bots / human: main -> model | Check events by bots / by the human: main -> model | Goals: main -> model |
|---|---|---|---|---|---|
| Bruisers (1.5, 150), rookie | 5 / 2 -> 7 / 8 | 2 / 0 | 750 / 300 -> 899 / 986 | 5 / 2 -> 9 / 8 | 1:1 -> 1:1 |
| Fog of War (1, 90), pro | 0 / 0 -> 0 / 9 | 0 / 0 | 0 / 0 -> 0 / 605 | 0 / 0 -> 9 / 0 | 3:0 in 3 043 frames -> 3:1 in 9 602 |
| Cage (3, 120), pro | 3 / 6 -> 5 / 12 | 2 / 2 | 360 / 720 -> 710 / 1 914 | 6 / 3 -> 16 / 5 | 3:4 -> 2:3 |
| Glass House (3, 150), pro | 7 / 3 -> 5 / 4 (8 618 frames) | 1 / 1 | 1 050 / 450 -> 722 / 697 | 5 / 5 -> 9 / 2 | 1:2 -> 3:2 |
| raw (5, 150), pro | 3 / 1 -> 4 / 3 | 6 / 3 | 450 / 150 -> 585 / 451 | 1 / 3 -> 6 / 10 | 3:0 in 9 295 frames -> 2:0 |

The low-`check_speed_min` modes hit the human harder than on main: Cage 12 knockdowns and 1 914 frames down (18 % of the three minutes; main 6 and 720),
Bruisers 8 (main 2), Fog of War 9 where main's match had none (its match also ran three times as long). These cuts were written for the old rule; each needs a
look, and `game.contact` in the cut (or `check_speed_min` back to 5) is the lever.

## 7. The acceptance run's steps 7 and 8 (`HokejAcceptDrill.cpp`, the two rows of `tools/qa/accept/accept.mjs`)

Re-staged for the model; under the neutral block the drill stages and judges as before (it reads `contact.momentum`).

| Step | Verb | The hand | Pass rule |
|---|---|---|---|
| 7 | check | W held from 4 m/s at a carrier that skates at the human; RMB on the cue, which now comes ten frames before contact (`gap <= closing x 10 frames`): by momentum the press must precede the hit | a `check` event, a knockdown, the puck stripped, the get-up within 10 frames of the `KnockFrames` the sim gave, and that number inside `down_frames_min..max` (x `knockdown_frames` / 90) |
| 7 | crosscheck | the vision's case: the carrier stands facing the human and holds the puck behind its body (stick back; a creep keeps it on the blade); the human glides in at 4 m/s (W for 30 ms); MMB on the cue at `gap <= cross_reach_m`, at the carrier's chest | the sim's `ContactOutcome` a stumble or more (the body part landed) and the puck free 20 frames later |
| 8 | checked | none: the opponent sprints 7 m at the standing carrying human and checks at contact | the human down; the view target stays the human's pawn, the camera within 25 m; up within 15 frames of the `KnockFrames` the sim gave |

Runs on `Game/Packaged/A92-dev` (Development), `tools/qa/accept/accept.ps1`, in order. The package was rebuilt for each change of the tree; the last three rows are the
final tree.

| Run | Tree | Result | Rows 7 and 8 |
|---|---|---|---|
| `a92-s78-1` (steps 7-8) | first model | check **FAIL**, crosscheck PASS, checked PASS | check: the RMB arrived on the contact frame and the human's body had touched the carrier's puck: it was "the carrier", the press was refused (outcome 0). Why the take's grace exists and why the cue moved before the contact |
| `a92-final` (full) | second model | disturbed, discarded | lane A93's acceptance run ended while this one played and its `quit` went to the shared remote console port 4661: this game exited at step 4. Every later run has its own port (`-Port 4692`) |
| `a92-final2`, `a92-final3` (full) | second model | 17 verbs PASS, 1 FAIL (`shoot_full`), twice | check PASS (down 120 / 121 = the sim's, ratio 2.34 / 2.35), crosscheck PASS (down 70, ratio 1.24), checked PASS (down 83, ratio 1.51) |
| `a92-final4` (full) | + frames by the physical ratio | 15 PASS, 3 FAIL | check PASS (122), checked PASS (83), **crosscheck FAIL**: on the slow glide the human's blade took the standing carrier's puck, the lift found no opposing carrier (outcome 0). Why the cross-check has the take's grace too and why the staged carrier now holds the puck behind it. (`shoot_tap` also failed on this run, 4 charge frames against 3: A91's row; the machine was packaging for another lane) |
| **`a92-final5`** (full, no other game window, no warning) | **final** | **17 verbs PASS, 1 FAIL, exit 1** | **check PASS**: check 1, knockdown 1, strip 1, down 120 frames = the sim's 120 (range 60..150), ratio 2.34, closing 10.11 m/s. **crosscheck PASS**: stick hit 2 (the lift took the puck), outcome down, ratio 1.24, 70 frames, closing 4.46 m/s at a gap of 0.49 m. **checked PASS**: down 83 frames = the sim's 83, ratio 1.51, closing 11.60 m/s, view target the human, camera within 6.1 m |
| `a92-s78-final1`, `a92-s78-final2` (steps 7-8, no warning) | final | 3 verbs PASS of 3, twice (exit 0) | check PASS (down 78 = the sim's 78, ratio 1.40, closing 6.22 m/s: in a run that starts at step 7 the first W does not take, the human glides in at 3.9 m/s and the hit is still a knockdown), crosscheck PASS (down 70 / 71, ratio 1.24 / 1.28), checked PASS (down 83, ratio 1.51) |

On the final tree the three rows passed in three runs of three (9 of 9), across closing speeds of 6.2 to 10.1 m/s for the check.

The one failing verb of the final run is **`shoot_full` (step 4), lane A91's row**: the 30-frame hold fires (charge 1.00) and does not score from 10 m, as on main
(`docs/experiments/ACCEPT.md`: "the one failure is real: shoot_full"; A91 moves `shot.lift.full_m`). Every other row passes: skate, sprint, sprint_carry, aim,
carry, shoot_tap, shoot_half, pass, saucer, onetimer, check, crosscheck, checked, deke, spin, sweep, resume_aim.

For whoever runs two acceptance runs on one machine: `accept.ps1` ends the game with `quit` on `-Port` (default 4661); two lanes on the default port end each
other's game. Give each run its own port.

## 8. What B's figure should show

The calls exist (contract `skater-visual.md` v4); nothing new is asked of the seam.

- **Stumble**: `PlayHitReaction(DirX, DirZ)` through the bump read-out (`bBumpThisFrame`, `BumpDir` = the push), on the frame of the hit, always (the 20-frame gap
  of plain bumps does not apply). The skater then has no control for `stagger_frames` (16): the figure should stagger for about a quarter of a second, the upper
  body thrown along the push, the stick dropped off line, and recover on its feet; today's hit reaction is the short flinch of a bump and reads too small for a
  stumble that loses the puck. A longer take, or the same take played at full weight when the sim's `IsStumbling()` is true for more than 8 frames, would tell the
  two apart. The attacker's recoil is the same call on the attacker (push = back along its path).
- **Bump** (a pressed check under 0.4, or an unpressed collision from 3 m/s): the same call, at most once per 20 frames, as before.
- **Down**: `SetKnockedDown(true, DirX, DirZ, KnockFrames)` as before, with `KnockFrames` now between 60 and 150 (x `knockdown_frames` / 90, x 1.5 in a spin)
  instead of always 90: the get-up must be timed from the number it is sent (A80's finding that the figure stands 12 frames early holds per frame, so at 150 it is
  the same 12 frames). The hit-stop and the camera kick hang on the `knockdown` event with Kind `check` and are unchanged, for a cross-check's knockdown too.
- **Refused press**: no figure call (`bCheckThisFrame` stays false, so `PlayCheck` does not play); `bCheckRefused` is there for the HUD.

## 9. For the Director, A94 and A86

1. **One gate test fails on this tree: `Jam.Bot.SeasonTakes`, take-2 (The Big Room, rookie, timer), seed 2**: the scripted human wins 3:0 and the AI takes 0
   shots in 11 523 frames; the test wants one in any match of a minute or more. Seeds 1 and 7 of the same take pass with 3 AI shots each; main had 4 / 4 / 4, this
   lane's first model 2 / 2 / 3. The margin was 4 against 1 and the model moved it to the edge: rookie bots never check and never cross-check, so under the model
   they only take hits. Sized over ten seeds with the same runner (`Jam.Core.ContactTakes`, a measurement with no bar; `a92-tests-final3-ue.log`):

   | Rookie take, seeds 1..10 | AI shots, total | matches of a minute or more with no AI shot | the scripted human's goals for:against | wins |
   |---|---|---|---|---|
   | take-2, neutral (= main: seeds 1 and 2 read 2:0 and 1:0 with 4 shots, as main's log) | 46 | 0 | 19:2 | 10 |
   | take-2, the model | 28 | **1** (seed 2) | 30:0 | 10 |
   | take-1, neutral | 40 | 0 | 20:1 | 10 |
   | take-1, the model | 84 | 0 | 19:2 | 10 |

   In The Big Room the scripted human scores half as much again and the rookie AI shoots 40 % less; one seed in ten trips the clause and it is one of the gate's
   three. Not tuned away (levels are A86's: a rookie that never hits is now a softer opponent); the merge with A93 and A94 will move the trajectory again and the
   test must be read there.
2. **The curve**: at pro the playtester is stumbled or knocked down 35 times in three matches (main 6) and scores 4 (main 7). The levers, in the order I would
   reach for them: the brain's lift policy (A94: lift from the blade's length, shove only as a decision), `contact.thrust_mps`, `contact.cross_reach_m`,
   `contact.stand_brace`, `contact.stagger_frames`.
3. **`ai.body_check 0` no longer means "the human is never knocked down"**: the bots' lift at arm's length shoves. A mode that leans on it needs
   `"contact": { "thrust_mps": 0 }` or the brain's lift off (`Jam.Core.ModeKeys.BodyCheck` now writes the first).
4. **The modes that lowered `skater.check_speed_min`** (Bruisers 1.5, Fog of War 1, Cage and Glass House 3) weigh a hit x3.3, x5, x1.7 under the model. Measured
   headless, one seed each (section 6): Cage knocks the human down 12 times in three minutes (main 6) for 18 % of the time, Bruisers 8 (main 2), Fog of War 9
   (main 0). The scale keeps their intent (hits from low speeds) and overshoots it; read them before they ship, or write `contact` numbers into them.
5. **The human's hit**: the 2 m/s calibration of A49 is gone under the model; the human hits by the same law. And **the press must come before the hit**: a press
   after the first touch resolves on the closing speed that is left (a bump). The 12-frame window covers an early press; a late one has nothing. If the operator's
   hand presses late, the follow-up is a short memory of the last unpressed impact (about 6 frames) that a late press may claim; it needs a hashed field.
6. **The HUD**: `FJamSkaterState::bCheckRefused` (one frame) and `ChecksRefused` exist; no HUD file is in this lane's fence and I did not find the miss text the
   brief names, so the refused press shows nothing yet. One line where the HUD reads the human's skater.
7. **Contracts** (not edited): `events.md` `check` now also fires for a stumble (no `knockdown` follows) and for a cross-check's shove (the `lift` event still
   fires too); Strength is still closing speed / `skater.speed`. `skater-visual.md`: `SetKnockedDown`'s `KnockdownFrames` now runs 60 to 150 (x
   `knockdown_frames` / 90, x1.5 in a spin). Commentary's `check.cross` and the massacre count read the same events and were not touched.
8. **For the brain (A94)**: `UJamSim::StabilityOf`, `ContactImpulse` and `CarrierMayNotHit` are public and const. A bot needs 7.5 m/s of closing to put a standing
   braced skater down with a body check, 3 m/s with a cross-check from the front at arm's length; against a carrier at full speed 3.6 m/s of closing is a
   knockdown from the side. A check pressed with no chance is a wasted press and, into a braced skater, a recoil.
9. **Not in**: VISION's "a hard turn lowers stability" (one factor on `YawRate`, the movement step is A93's); a cross-check on a skater without the puck (the lift
   targets the carrier only, as before).
10. **Velocities**: `ResolveContact` writes `Vel` of the two skaters (the stumble's half, the pushed speed changing hands, the recoil): the contact's exchange,
    like the collision's bounce beside it. Nothing in the movement step, the sprint or the stick was touched. `CarrierMayNotHit` reads `LastCarryFrame`, which is
    not hashed; the foul rule has read it the same way since R-0029.

