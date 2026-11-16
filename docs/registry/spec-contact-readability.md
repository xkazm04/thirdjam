---
status: DRAFT (lane R, 2026-11-16; nothing dispatched; for the owner's gate on night 2 of NIGHTLY-FORGE-PLAN.md)
source: thirdjam/docs/registry/LESSONS.md rows J18 to J22; thirdjam docs/experiments/BAR.md; status/A219.md, A234.md
run_id: spark@jam-lessons-home
bundle: game-production
category: asset-production
subcategory: motion-and-audio
subject: contact-readability
resolved_path: knowledge/game-production/asset-production/motion-and-audio/contact-readability/
category_count: motion-and-audio holds 5 subjects (adaptive-music-authoring, motion-quality-gating, spatial-audio-scene-authoring, game-dialogue-voice-pipeline, generated-video-motion-extraction) and 6 after; asset-production holds 4 subcategories. engine-integration (10) is at the cap and was rejected for that reason; craft-judgment (6) was rejected on the merits (this subject owns presentation, not grading).
---

# Spec: contact-readability

## Why a subject, and why here

Five candidates from one jam share one home the corpus lacks: what a player must be able to read at a contact between figures
(a check, a tackle, a block, a strike) when the game draws no mark for it, and how a team finds out what is actually drawn there.
Index queries for hit, impact, feedback, contact, readability and "juice" returned no technique whose job this is. The neighbours
each own a slice and disclaim the rest:

- `motion-quality-gating` grades motion as motion (a rubric, a filmstrip for a critic). It does not own what else is on screen at
  the contact, nor the runtime arbitration that decides which motion plays.
- `generated-video-motion-extraction/contact-window-travels-with-the-clip` owns the active frames of an extracted clip. It does not
  own a reaction overriding the attacker's own take at run time.
- `realtime-combat-semantics/telegraph-or-homing-for-area-effects` owns telegraphs for area attacks as a fairness rule, not the
  readability of a body contact at play distance.
- `runtime-observation-evidence/tiers-of-truth` owns the evidence ladder; this subject consumes its perceptual rung.

Three or more candidates with one home-if-new fires the mechanical trigger. The evidence is one project and one person's eye, so
each technique below is argued from first principles and the neighbours, with the jam as the witness.

**Placement argument.** The merits are split between motion (the take carries the event) and verification (filming what is drawn).
motion-and-audio wins because four of five techniques are about what the figure's motion must do and what must not compete with it;
the filming technique is the subject's verification method, as `filmstrip-sampling-discipline` is motion-quality-gating's.

## The subject's job, in one sentence

**At a contact between figures, the player reads what happened from the figures alone; this subject owns what their motion must
carry for that to work, what the presentation layer must never add or override, and how a team proves what a player actually sees.**

## Golden path (draft, purity-clean, seven sentences)

A contact is the moment a player most needs to read the game and the moment a presentation layer is most tempted to explain it
with marks. A mark drawn from the mechanics (a bar for a reach, a ring for a grace, an arc for a held move, a plate naming the
outcome) reads to a person who did not build it as a defect in the animation, so at a contact nothing of the mechanics is drawn on
or in front of a figure, and the figures' motion carries the whole message. That puts a hard obligation on the motion: the
attacker's own take must play from its wind-up through the contact and the follow-through, sized for the eye at play distance, and
no generic collision reaction may replace it on the frame after the contact its own move caused. Where a rule's state still has to
be seen (a figure that may not be hit again yet, a figure that just won the ball), it is shown by posture or by a lasting element of
the scene, never by a transient overlay at the contact. What is drawn is known only by looking: every simulation frame around
contacts is filmed from the play camera in the packaged build, at the shipped looks and sizes, and read; counting which switches are
off, or sampling a development binary, has twice reported marks gone that the film still found. A fix that is presentation only is
proven on the same frames of the same binary, before and after, with the simulation's hashes equal, so it cannot have changed the
game.

## Boundaries the golden path must state (prose only; no cross-bundle links)

- `motion-quality-gating`: grades the take; this subject decides that the take is the message and keeps everything else off it.
- `generated-video-motion-extraction`: owns where a clip's active frames are; this subject owns that the runtime lets them play.
- `realtime-combat-semantics`: owns whether a hit lands and what it costs; this subject owns whether the player can see it did.
- Law L15 (`every-effect-on-the-rules-is-visible`): see the open question; this subject must not be read as licence to hide a rule.
- The media-generation bundle's frame direction: owns composed footage; a play camera is not a shot list.

## Candidate techniques

### T1 `the-figure-carries-the-contact`
- **Rule.** At a contact, draw nothing of the mechanics on or in front of a figure; the figures' motion, the camera's kick and a
  short hold of the frame carry the event, and a rule state that must stay visible is shown by posture or by a lasting scene
  element, never by an overlay at the contact.
- **use_when:** a contact needs feedback and someone proposes a mark; a player reports "a rectangle", "a line" or "a glitch" at a
  hit; deciding how a grace window or a won ball is shown.
- **Laws:** `every-effect-on-the-rules-is-visible` (L15), `structural-proof-is-never-sufficient` (L9).

### T2 `film-every-frame-of-the-shipped-build`
- **Rule.** To know what is drawn at contacts, film every simulation frame around them from the play camera, in the packaged build,
  at each shipped look and screen size, and read the frames; a count of switches, a lane's own stills or a development binary's
  frames are not evidence of absence.
- **use_when:** a "nothing is drawn" claim is about to be made; a player reports a mark a lane says is gone; a look or a screen size
  is added.
- **Decision rules to write:** the film names its build, look, size and frame range; frames are read by someone who did not make the
  change; a walk of the whole candidate (every screen, every mode) follows the targeted film.
- **Laws:** `an-instrument-proves-it-had-input` (L12), `a-verdict-is-bound-to-its-content` (L6), `no-gate-self-certifies` (L7).

### T3 `own-take-outranks-the-collision-reaction`
- **Rule.** A figure whose own move caused a contact keeps its take through the contact and follow-through; the presentation
  layer does not send it a generic collision reaction for that contact, while a stumble or knockdown that the rules impose still
  plays.
- **use_when:** an attack animation "does not play" or looks cut; a hit reaction and an attack share a figure on adjacent frames;
  wiring collision events to animation.
- **Laws:** `one-authority-per-quantity` (L3: one owner decides which motion plays on a frame).

### T4 `wind-up-sized-for-the-eye`
- **Rule.** Size the opponents' wind-up so a person can see the hit coming at play distance, keep the person's own press short
  so the button does not feel late, and measure the wind-up's cost on the difficulty ladder as a presentation change with a
  balance price.
- **use_when:** hits are "unrecognizable" or "too quick"; a wind-up is about to be lengthened; deciding whether a longer wind-up
  is a new dodge mechanic.
- **Laws:** `a-number-carries-its-unit-and-basis` (L2: frames at a stated tick rate).

### T5 `presentation-fix-on-equal-hashes`
- **Rule.** Prove a presentation-only fix on the same simulation frames of the same binary, before and after side by side, with
  the simulation's hashes equal line by line; a fix whose hashes differ is not presentation only and goes through the rules gate.
- **use_when:** a visual fix lands near the rules; a reviewer asks whether a cosmetic change moved the game; pairing before and
  after captures.
- **Laws:** `a-verdict-is-bound-to-its-content` (L6), `structural-proof-is-never-sufficient` (L9).

## Laws cited

L2, L3, L6, L7, L9, L12, L15 (all present in `game-production/_laws.md` on this checkout).

## Evidence this jam gives (n = 1 project; the grader was one person's eye)

- R-0067 (6), R-0069 (3), R-0070: "crosscheck has bug in animation displaying technical blue rectangle instead of animating player";
  "no aim line needed, the player animation should reflect the crosscheck movement and that should be all".
- BAR: nine layers still drawn in the newest build when the night's count said none (42 hit strips on the editor binary); every
  frame filmed from the play camera in the package found them; switched off, with tests that pin zero bars in every frame.
- BAR section 3: press at frame 1338, collision reaction at 1339, not one frame of the take drawn; a guard and a test landed.
- A234: the walk of the cooked candidate found three more things drawn at contacts (W10 a slide streak, W13 coloured balls at body
  collisions in the first two seconds of every match, W15 a man-down triangle inside the picture); A235 cured them with before and
  after films (TRUTH P6-PG03).
- R-0065 (1): opponents' wind-ups 8 and 12 frames, the person's 4 and 4; cost -0.3 / +5.8 / +4.0 points; a scripted dodge worth
  nothing.
- BAR: simulation hashes equal line by line on every film pair.

## What a second project must show

- A second game with a different camera (side view, first person) where marks at contact were removed and players' reading of
  hits was measured (a person's verdict or a recognition test) before and after. Without it, T1 is one owner's taste stated as a
  rule and must stay `draft`.
- For T2: a second case where a switch count or a development build said "absent" and a film of the packaged build said otherwise.
- For T3: an engine-independent second case of a reaction overriding an attack take; the jam's case is clear but one runtime.
- For T4: a second measured wind-up cost; the jam has one table.

## Open questions the drafter must DECIDE

1. **L15 against T1.** L15 says every effect on the rules is visible to the player. The jam removed plates that told the player a
   rule state ("just got up", "braced hit", the grace ring's swell). Decide and write: either posture and lasting scene elements
   satisfy L15 (the recommended reading: L15 requires visibility, not overlays), or T1 needs an exception list. Do not leave the
   two appearing to disagree.
2. **Camera kick and hit-stop.** The jam kept both "by design" but measured neither. State them as permitted, not as required,
   unless a primary corroborates them.
3. **Name.** `contact-readability` is this spec's choice; `hit-readability` is narrower (sports and fighting); decide before
   writing, because the slug is in every citation.

## What the drafter must NOT do

- Name the game, its engine, its upscaler, its looks or its tools in the golden path or techniques.
- Write an application document; the jam's application (`process--film-every-frame-of-the-shipped-build.md`, citing
  `docs/experiments/BAR.md` and `tools/qa/eye2/`) is the intake director's.
- Place it in engine-integration (at the cap) or subdivide any category as a side effect.
- Run any git command.
