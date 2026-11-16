---
status: DRAFT (lane R, 2026-11-16; nothing dispatched; for the owner's gate on night 2 of NIGHTLY-FORGE-PLAN.md)
source: thirdjam/docs/registry/LESSONS.md rows J01 to J05 (and J07)
run_id: spark@jam-lessons-home
bundle: game-production
shape: DEEPEN, not a forge (decided below): two new techniques and two amendments in existing subjects
targets:
  - knowledge/game-production/craft-judgment/playtest-signal-to-defect/ (new technique; amendment to session-instrumentation-contract)
  - knowledge/game-production/balance-validation/difficulty-design-and-adaptation/ (new technique)
  - knowledge/game-production/balance-validation/combat-pacing-and-dramatic-arc/ (amendment to proxy-driver-limit-declaration)
category_count: no subject is added, so no category count moves (craft-judgment 6, balance-validation 6)
---

# Spec: a scripted hand against a person's hand, as an instrument

## The decision: techniques in existing subjects, not a new subject

The scout's reading offered a subject "a scripted hand versus a person's hand as an instrument". Checked against the index on this
machine, it is better placed as techniques, for three reasons.

1. **Every half already has an owner.** `proxy-driver-limit-declaration` (combat-pacing-and-dramatic-arc) owns "a figure from a
   stand-in is a figure for the stand-in" and its declaration block already names the input path. `proxy-corpus-miss-ledger`
   (tracked-input-record-and-replay) owns the ledger of where real hands contradicted a proxy. `owner-anchor-for-a-model-panel`
   (concept-scene-parity) owns "the person the work is for anchors the panel". A new subject would restate all three or link
   across them with nothing of its own at the centre.
2. **What the jam adds is two procedures and one instrument, each with a clear owner.** Turning a person's report into a
   reproduced defect is the stated job of `playtest-signal-to-defect` ("getting what happened to a player into a defect somebody
   owns"); it has `repro-minimization-protocol` but no rule about reproducing *as the person met it* or about confirming the build.
   Measuring a ladder through the player's input path is a difficulty-validation question and belongs with
   `difficulty-design-and-adaptation`.
3. **The forge's XL trigger does not fire.** The trigger is three or more candidates sharing one home-if-new; here every candidate
   has an existing home.

If the owner prefers a subject anyway, the slug would be `stand-in-player-evidence`, in craft-judgment (6 -> 7), and these four
items would be its techniques; the three neighbours above would each need a boundary sentence. This spec does not recommend it.

## The concern, in one sentence

**A machine stand-in for the player is an instrument with an author's idea of a hand inside it; when its figures disagree with a
person's play, the person's play is the measurement, and the work is to reproduce what the person met, on the build the person
ran, before anything is called fixed.**

## The golden-path paragraph this adds to `playtest-signal-to-defect` (purity-clean draft)

A scripted player is built around the rule it tests, so it tends to aim where the rule looks and to press when the rule expects.
A person's aim is elsewhere, buttons stay held, and the build the person runs is whatever was last started on that machine. When a
person's report contradicts a green figure, the report is not a complaint to be argued with the figure; it is a measurement taken
with a better instrument. The first act is therefore to reproduce the report as the person met it: the person's aim, the person's
held input, the shipped build rather than a development binary, the default mode the person starts first. The reproduction is shown,
as a table or as frames, before a cure is written, and the cure is accepted only on that same reproduction. Before any of this, the
build the person actually started is confirmed from the machine's own record, because notes about one build are often notes about
another. Every figure produced by a stand-in keeps its label until a person has played the build it describes.

## Candidate techniques

### T1 `reproduce-as-the-person-met-it` (new, in `playtest-signal-to-defect`)

- **Rule.** When a person's report contradicts a green metric, rebuild the case with the person's input as it was (aim away from
  the target, buttons held through the event, the speeds a person skates at), on the shipped build, and show the reproduction
  before writing a cure; accept the cure only when the same reproduction passes, and keep the reproduction as a test.
- **use_when:** a person's report contradicts a green automated figure; a fix is about to be called done on a stand-in's numbers;
  a report says "still" about something a lane marked fixed.
- **Decision rules to write:** the reproduction names each axis it varies (aim offset, held buttons, speed, build); a reproduction
  that cannot be made to fail on the old build is not a reproduction; the stand-in's own figure is reported beside the person's
  and never replaces it.
- **Laws:** `structural-proof-is-never-sufficient` (L9), `no-gate-self-certifies` (L7), `an-instrument-proves-it-had-input` (L12).

### T2 amendment to `session-instrumentation-contract`: the build is read from the machine

- **Rule.** A session record carries the identity of the build that was played, read from the machine (what was started, and
  when) and shown on the first screen of every build, never taken from the tester's memory or from the newest build's name.
- **use_when (added):** play notes arrive for a build that was handed over; several builds of one game sit on the person's machine.
- **Laws:** `a-verdict-is-bound-to-its-content` (L6).

### T3 `ladder-through-the-players-input-path` (new, in `difficulty-design-and-adaptation`)

- **Rule.** Set a difficulty ladder for people through the people's own input path (the real input resolver with its tap and
  hold thresholds, a person-like hand with its late and coarse steering), prove that path equal to the shipped game to the
  simulation hash, and when it disagrees with a decision-level instrument, find the cause by switching one component of the
  stand-in off at a time.
- **use_when:** a difficulty ladder is about to be set or re-set; a decision-level simulation and the shipped game disagree about
  a win rate; a person finds a level much harder or easier than its number.
- **Decision rules to write:** report both instruments side by side while they disagree; a ladder number is quoted with its
  instrument and its seed count; the decomposition table (component off -> figure) is part of the report.
- **Laws:** `a-number-carries-its-unit-and-basis` (L2), `unmeasured-is-not-a-pass` (L1).

### T4 amendment to `proxy-driver-limit-declaration`: the instrument's version, and two columns when it changes

- **Rule.** The declaration block names the stand-in's version (what changed in its policy since the last figure); when the
  stand-in changes, the same commit carries the old and new stand-in's figures side by side, and the target is not silently
  re-set on the new one.
- **Laws:** `a-number-carries-its-unit-and-basis` (L2).

Not proposed: a technique on "clumsy, not absent" opponents at a low level (J07). It is a sound application of
`skill-scaling-versus-power-scaling` and `tier-changes-decisions-not-specs-test`, and the jam measured nothing after it.

## Evidence this jam gives (n = 1 project, one owner as the person)

- R-0069 and A218: a 27-case table with the aim 90 or 180 degrees off: 21 of 27 not his within 2 frames on main, 0 of 27 after
  the cure; the night's "taken 99.6 %" had come from a stand-in whose stick pointed at the puck. Through the person's path
  (100 seeds): rookie 92 -> 98 %, pro 55 -> 73 %.
- R-0069 addition, BAR section 1: the build the notes named was not the build started; the machine's start record showed one five
  packages older, started four minutes before the notes.
- A217: the input-path instrument equal to the game to the hash, 12 of 12 checkpoints; pro 31 % (decisions injected) against
  17.7 % (person's path); one switch at a time: buttons off 12, person-like hand off 34, sprint off 16.
- R-0060: one change to the stand-in's press moved the ladder's cells by 2 to 4 points on 600 paired seeds; two columns reported.
- R-0067 (2), R-0068 (3): the ladder was on target at every level the evening the person called the game unplayable.

## What a second project must show before this is more than one project's habit

- A second game (another genre, another input device) where a person's report contradicted a stand-in's green figure and a
  reproduction made as the person met it failed on the old build and passed on the cure. One case is enough to admit T1 as
  `draft`; three before `forged`.
- For T3: a second game where the input-path instrument and a decision-level instrument were run on the same seeds and the gap was
  decomposed. Without a second decomposition, the claim "the loss is in the hand, not the buttons" stays this jam's finding and must
  not appear in the technique as a general rule; the general rule is only "measure through the input path and decompose".
- For T2: one more case of notes about the wrong build, or a team's practice of build tags on the first screen that prevented one.

## What the drafter must NOT do

- Name the game, its engine, its tools, its levels' names or its people in the techniques or the golden-path paragraph.
- Restate `proxy-driver-limit-declaration` or `proxy-corpus-miss-ledger` inside T1; cite them by slug in prose.
- Promote the jam's figures to thresholds. The figures are evidence for an application, written by the intake director, not by the
  forge worker.
- Run any git command.
