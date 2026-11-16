---
status: DRAFT (lane R, 2026-11-16; nothing dispatched; for the owner's gate on night 2 of NIGHTLY-FORGE-PLAN.md)
source: thirdjam/docs/registry/LESSONS.md rows J11 to J17; thirdjam contracts/RULINGS.md R-0042, R-0044, R-0048 to R-0051, R-0065, R-0066, R-0069; status/A218.md
run_id: spark@jam-lessons-home
bundle: game-production
category: systems-canon
subject: ball-possession-and-pickup
resolved_path: knowledge/game-production/systems-canon/ball-possession-and-pickup/
category_count: systems-canon holds 7 subjects (agent-behaviour-authoring, arpg-systems-canon, design-canon-as-executable-law, cross-runtime-rules-conformance, game-economy-tuning, realtime-combat-semantics, racing-career-economy) and 8 after. No subcategory, so the both-kinds prohibition is not engaged.
---

# Spec: ball-possession-and-pickup

## Why a subject

Seven candidates from one jam share one home the corpus does not have: the rules that decide who has the ball (or puck, or disc)
in a real-time team game, how it is won and lost, and what the controlled figure is owed. Index queries for possession, pickup,
ball, puck, carrier and loose returned zero techniques. The nearest subjects own something else:

- `realtime-combat-semantics` owns damage, defence, health truth and start protection; it has no notion of a carried object.
- `agent-behaviour-authoring` owns how a computer opponent decides (`commitment-and-recovery-windows`), not what the rules allow it.
- `mass-based-arcade-collision` owns body contact between vehicles; a carried object is not in it.
- `design-canon-as-executable-law` owns where thresholds live; this subject is one of the canons it would hold.

The mechanical trigger (three or more candidates, one home-if-new) fires. The evidence is one game (an arcade ice hockey game, five
a side, a mouse-aimed stick), so the golden path is argued from what ball sports and their digital adaptations hold in common.

## The subject's job, in one sentence

**A carried object is the game's central state; this subject owns the rules by which it changes hands, the one reach every rule
reads, the protections a figure is owned after an exchange, and what the person's controlled figure is given that the rules'
other users are not.**

## Golden path (draft, purity-clean, eight sentences)

In a team game with one carried object, who has it is the state every other rule reads, so the rules of possession are written
first and kept few. The object leaves its carrier only by an action somebody chose (a shot, a pass, a poke, a lift, a hit that
loosens it, the carrier's own release), never because a stick or a body happened to touch it; a carry the field takes by contact
lasts under a second and no one can play the game around it. Every rule that asks whether a figure carries the object (shoot,
pass, charge, carry, and the opponents' own logic) reads one carrier reach computed in one place, because two reaches a few
centimetres apart produce a game in which nothing fires while the player aims. A loose object is won by the body's reach, not by
the aim: inside a ring drawn under the controlled figure, or along the path its body moves through, it becomes that figure's on that
frame, because a person's aim is somewhere else. The figure's own release is kept from it while it is still leaving, and only inside
that ring, so a pass is not taken back and a pass to a mate is not slowed. After an exchange each side is owed a measured grace: a
figure that just rose cannot be knocked down again at once, and a figure that just won the object cannot be stripped by a stick on
the next frames, with each length chosen from a run that counts repeat events. A held input does nothing across a change of
possession until it is released and pressed again, and a held charge dies with the object. Any rule that helps the controlled
figure more than the others is drawn where the player can see it, and every possession rule has a neutral value that gives back the
previous game, so its price on the difficulty ladder can be measured and paid.

## Boundaries the golden path must state

- `realtime-combat-semantics/start-protection-window`: owns a grace at the start of a match; this subject's graces follow an
  exchange. Cite, do not restate.
- `agent-behaviour-authoring/commitment-and-recovery-windows`: owns when an opponent may change its mind; this subject owns what the
  rules permit it to take.
- `difficulty-design-and-adaptation`: every possession rule moves the ladder; this subject requires the price to be measured and
  hands the setting to that subject.
- `couch-and-tv-play/.../pointer-capture-per-control`: owns a touch that slides off a control; T6 is the same instinct for a
  held press across a change of state, on any device.

## Candidate techniques

| Slug | One-line rule | use_when |
|---|---|---|
| `possession-leaves-only-by-an-action` | The carried object leaves its carrier only by a chosen action; an unpressed touch of a stick or a body never takes it, and never passes it to a teammate. | carries last under a second; a carrier loses the object to a touch nobody pressed; writing the first possession rules of a ball game |
| `one-carrier-reach` | Compute the carrier's reach once (from the body radius and the stick or limb reach) and read it in every carrier test and in the opponents' logic. | shots or passes do not fire while the player aims; two systems disagree about whether a figure has the object; the carried object's distance from the body changes |
| `pickup-by-the-body-for-the-controlled-figure` | A loose object inside a drawn ring under the controlled figure, or on the path its body moves through, is that figure's on that frame whatever the aim points at; the ring is drawn, and its exceptions (an opponent's fast shot, a pass in flight, a press's cooldown with the presser near) are listed. | players say the object passes through them; deciding whether a pickup follows the aim or the body; giving a person's figure a help the others do not have |
| `own-release-leaves-before-it-returns` | Keep the controlled figure's own release from it while the release is still leaving (a fixed number of frames, or while faster than the figure by a set margin), and only inside its ring. | a pass or shot is retaken at once by its own figure; a pickup assist slows a pass to a mate |
| `grace-after-an-exchange` | After a knockdown a figure is protected from a second one, and after winning the object it is protected from a stick take, each for a length chosen from a run that counts repeat events; a hit inside the grace is a bump. | a figure is knocked down again within a second of rising; a won object is stripped on the next frames; choosing a grace length |
| `held-input-dies-with-possession` | A button held across a change of possession does nothing until released and pressed again, and a held charge dies when the object is lost. | a held press fires a shot or pass after a turnover; a charge carried into the next possession |
| `shield-refuses-the-stick-not-the-body` | The carrier's body may shield the object from a stick behind it, never from a body contact, and the shield is checked together with the opponents' logic that reads it. | adding a body shield; a defender's refused stick turns into a shove; a shield rule passes alone and fails with the opponents' logic (evidence weak: the jam never measured it alone) |

## Laws cited

- `one-authority-per-quantity` (L3): the carrier's reach; who owns the object on a frame.
- `every-effect-on-the-rules-is-visible` (L15): the controlled figure's pickup ring is a help the others do not get, so it is drawn.
- `a-number-carries-its-unit-and-basis` (L2): graces in frames at a stated tick rate; reaches in metres from a stated origin.
- `unmeasured-is-not-a-pass` (L1): a rule's price on the ladder is measured, not assumed.

## Evidence this jam gives (n = 1 game)

- R-0048: the controlled player's carry lasted a median 0.87 / 0.75 / 0.67 s at the three levels; R-0050: with the
  action-only rule the ladder moved to 99.8 / 72.0 / 39.8 against 85 / 48 / 26, and was re-set on it.
- R-0042, R-0044: six packages could not shoot or pass while aiming (the object at 1.6 m, the shot test at 1.5 m); one reach: 30 shots
  fired at three charges where none had.
- A218: 27 cases with the aim held away: 21 not his within 2 frames before, 0 after; the release lock (8 frames, 6 m/s) after a pass
  was retaken one frame after release; the mate-pass guard 19 / 18 frames, equal to the neutral; the central drill identical to the
  digit with the ring at 0 and on; the neutral (ring 0) = the previous game to the hash on 6 of 6 cells.
- R-0049, R-0051: the controlled figure down every 7 s in a play-through; release grace 90 frames still allowed one repeat at 111
  frames, 120 allowed none in 1,000 s, cells within 1 point; take grace 30 frames.
- R-0065 (3), R-0066 (2): held input dies with possession; no cell moved more than 2.2 points.
- R-0050 (2), R-0053, R-0055: the shield rule and an opponent-logic change passed alone and broke the drill together; the measured
  cause was in the logic.

## What a second project must show

- A second ball or puck game (a football, basketball or disc game, a different input device) in which carries measured under a
  second rose when touches stopped taking the object. Until then the golden path's "under a second" is this jam's figure and must
  be written as an example, not a norm.
- For `pickup-by-the-body-for-the-controlled-figure`: a second game where pickup by aim was compared with pickup by body for a
  person, with a person's verdict. The jam's table used scripted aims standing in for a person's; the owner did not play the cure.
- For the grace lengths: no second figure is needed for the rule (choose from a run that counts repeats); the numbers stay in the
  application.

## Open questions the drafter must DECIDE

1. **The asymmetry.** The pickup ring serves the controlled figure only. L15 is satisfied because the ring is drawn; decide whether
   the golden path also requires the ring's help to be measured against the opponents (the jam measured +18 points at pro) and
   disclosed, or whether drawing it is enough.
2. **Slug.** `ball-possession-and-pickup` names the common case; `carried-object-possession` is more general and less readable.
   Decide before writing.
3. **Whether `one-carrier-reach` is a technique here or an application of L3.** The recommendation: a technique, because its
   failure (nothing fires while aiming) is specific and recurring in sports games.

## What the drafter must NOT do

- Name the game, its sport-specific rule names as if they were general (the "stop-and-go drill" is evidence, not a term), its
  engine or its people in the upper layers.
- Restate `start-protection-window` or `commitment-and-recovery-windows`.
- Write the jam's numbers into the techniques as thresholds; they belong in an application with `stack: cpp` citing the jam's
  simulation source, written by the intake director.
- Run any git command.
