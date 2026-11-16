# The first night of applying the jam's lessons (third-life, 16 Nov 2026)

The lessons of `LESSONS.md` were applied the same night to another engine and another codebase: the Godot sandbox
`third-life`, whose hockey house game got possession by reach, hits shown by the figures alone, modes as data, a cursor
aim, and a difficulty ladder read through a model of a person's hand. This file records what held when applied, what the
application taught on top, and the eight leads it added to `.ai/registry-leads.jsonl` (41 in all). Figures are from
scripted runs of that night (third-life `main` at `df0052d`); nobody had played the game by hand when this was written,
and the hand's profiles are reasoning, not measured on a person.

## Which jam lessons held when applied

| jam lesson | applied as | what the application showed |
|---|---|---|
| a perfect bot measures the game, not the player (J1 to J5) | `TLHand` (keys, then the cursor) and `tl ladder` | a perfect hand overstated the player by 15 to 30 points at every level; with the cursor the same levels moved +25 and -20 points against the key-only read |
| reproduce as a person meets it | one still a package looked at by the director; a look builder with before/after pairs | found what no bot said: no visible puck, HUD text across the ice, developer wording, the carried puck under its carrier |
| a stopped run is never a pass | gate steps report found and completed | the same contract then caught "fewer rows than games" in the new world step's own tests |
| which build is this | a build tag in the exe and a check that refuses a build of another commit | the check refused a stale build in a test tree the first time it ran for real |
| the puck in his ring is his | possession by reach, then the carry on the stick's blade | the blade was 3.0 m out where the brief assumed 1.9; moving the carry exposed three stalls |
| nothing is drawn at a hit | the kernel's fallen figure, stumble and shove | a knockdown reads from the play camera; a stumble does not read in a still |

## What the application taught on top (A1 to A8, each a lead)

- **A1** A difficulty ladder is read through the input device the player uses, one hand profile a level, and quoted with
  hand and seeds. Key-only: pro 60 %, all-star 30 %. With the cursor: 85 % and 10 % on the same levels.
- **A2** A win-or-lose test is counted over 20 seeds before it enters a gate. Two tests judged on seed 1 passed 10 and 19
  of 20; the line that held on all 20 was "a perfect hand beats the easiest level".
- **A3** A check on a pre-merge list is run last. The world-as-alone check caught a kernel fault two hours after it was
  merged; as a gate step it fails on the fault put back.
- **A4** A helper that asks the physics world asks the caller's own space. In a composed world the default space is empty,
  and one query made a game diverge at tick 681.
- **A5** The ball is carried where the tool is drawn, measured from the rig, and every rule that used the old carry point
  is re-read (reach, poke, boards, behind the net, the goal volume).
- **A6** When a perfect hand does worse than a person's, look at the bot's policy first: it shot from centre ice at the
  first chance (93 shots against 47) and each miss was a counter-attack.
- **A7** Score by where the ball is each tick, not only by the enter event: a puck released inside a net never scored and
  the match stood still.
- **A8** A brief says the builder's measurement outranks it. Builders refused three of the director's lines with figures
  and a substitute proof, and each refusal was right.

## For the forge plan

Nights 2 and 3 of `NIGHTLY-FORGE-PLAN.md` (the person's-hand spec and the possession spec) can cite these as a second
project's evidence: A1, A2 and A6 for `spec-person-hand-reproduction.md`; A5 and A7 for
`spec-ball-possession-and-pickup.md`; A3 and A8 for `spec-directed-parallel-lanes.md`; A4 is an engine-integration lead of
its own.
