# Contact feel 2: a tackle is an event, not a rhythm (lane A113, track F1, device A, 2026-11-13)

Device B played the build before the vision for 267 s (`B-to-A-0038`): the human went down 15 times in 104 s at pro, 7 of them within a second of his release, 10 by
the same bot from the same side. Ruling R-0049 point 1 answers the part the vision's contact law did not: a skater that has just got up is owed a moment. This
evolution adds that moment as a rule with a neutral value, tells the brain not to waste a check on it, and finishes the HUD line of evolution 1 (`CONTACT-2.md`)
so a pressed check never ends in silence. Worktree `.claude/worktrees/a101`, on top of evolution 1.

**R-0051 (the Director, 20:30, on this lane's report): the default is 120 frames, and the shotgun respects the grace.** Sections 1 to 3 describe the final
tree. Sections 4 to 7 were measured while the default was the first ruling's 90: where they say "grace 90" that is the arm, not today's default; the 120 arm
beside it is today's default. The gate and the hashes of the final tree are in section 11.

## 1. The rule: release grace

    on the frame a knockdown ends (KnockFrames counts down to 0):   GraceFrames = game.contact.release_grace_frames        (StepSkater)
    every frame after:                                              GraceFrames - 1 until 0                               (StepSkater, first line)
    a hit resolved on a victim with GraceFrames > 0:                the outcome is a bump at most                         (ResolveContact)
    the brain's check press:                                        not on a target with GraceFrames > 0                  (JamAICore.cpp, the check press)

- **A bump at most.** `UJamSim::ResolveContact` computes the hit as before (impulse, the boards' factor, stability, ratio) and then caps the outcome: no stumble, no
  knockdown, no `check` event, no foul, no loose puck. The victim's read-outs still say what hit it: `ContactOutcome` 1 (bump), `ContactRatio` the real ratio,
  `bContactGrace` true, counter `ContactGraced`. The figure plays its bump reaction (`PlayHitReaction`).
- **The attacker pays as for any bump.** The press is spent (`CheckWindow` 0), the collision's exchange of speed runs, and the recoil test is the bump's: a sprinting
  attacker that runs into a braced skater inside its grace still bounces. An armed hip check (lane A84's ability) is capped too and **keeps its charge** (it is spent
  only by the knockdown it causes).
- **What starts a grace.** Only the end of a knockdown's own count. A faceoff sets everybody up and clears it (the whistle's early release of a downed figure goes
  through the faceoff, so it starts none).
- **The shotgun respects it** (R-0051 point 3): `UJamSim::StepShotgun` passes over a skater inside its grace as it passes over a shielded one (the skater stays
  up; its body can still stop the puck). `Jam.Core.Abilities.Shotgun` has the case: the third skater on the path just released, the two before it go down.
- **What the grace does not cover:** the stumble a skater gives itself by a mistimed poke (its own mistake, not a hit).
- **Part of the momentum model.** Under `contact.momentum false` (lane A92's neutral block) no grace starts, so that block still hashes as the game before the model
  (`Jam.Core.ContactNeutral` stays as it was).

Key: **`game.contact.release_grace_frames`**, integer, default **120** (R-0051; the first ruling's 90 left re-downs at 93 to 111 frames), range 0..300 (`schema/game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md`,
`FJamContactConfig::ReleaseGraceFrames`, the loader's `game.contact` block). **Neutral value 0**: no grace ever starts, `GraceFrames` stays 0 and is not hashed.

**The hash.** `GraceFrames` is rule state that lives across frames. `UJamSim::StateHash()` adds it behind a tag only while it is not 0
(`if (S.GraceFrames != 0) { H.AddI(-113); H.AddI(S.GraceFrames); }`), the pattern every payoff field uses, so the neutral value hashes as the baseline. This worktree
(at ee14438) has no `LockstepStateHash()`; main's folds `StateHash()` and its header says a new rule field goes "into StateHash() (behind its default) or here": the
lockstep checkpoint covers the counter through `StateHash()` once this lands. The read-outs (`bContactGrace`, `ContactGraced`, `ChecksSoft`, `ChecksGraced`,
`ChecksMissed`) are in neither hash (`Jam.Core.Contact.ReleaseGrace` point 6 asserts both).

**The brain** (the one line in track Q2's file): `Game/Source/JamCore/AI/JamAICore.cpp`, in `FJamBrain`'s body-check press, the reach condition

    if (Dist > 1e-6 && Dist <= 2.0 * W.SkaterRadius + CheckArmGapM && !T.bDown && T.GraceFrames <= 0)

`FJamAISkater::GraceFrames` (one field in `JamAICore.h`) is filled by `FJamSimAdapter::BuildWorld` (`JamSimAdapter.cpp`, one line beside `bDown`). The reference sim of
the bot tests leaves it 0. The brain does not chase a graced carrier differently: it only keeps its press (and its 30-frame cooldown) for a hit that can land.

## 2. The soft hit explains itself (the HUD's contact line, evolution 1's next-2)

The sim counts, on the attacker, what became of a pressed body check; the HUD shows the line when a counter of the human grows (as it did for the two lines of
evolution 1, so a frame the hit-stop held is not lost):

| What happened to the human's press | Counter (read-out, never hashed) | Line |
|---|---|---|
| it ran into a more stable skater and the human stumbles | `ContactRecoils` | **BOUNCED OFF** (evolution 1) |
| pressed while carrying | `ChecksRefused` | **NOT WITH THE PUCK** (evolution 1) |
| it met a skater inside its release grace | `ChecksGraced` | **JUST GOT UP** |
| it resolved as a bump (ratio under `bump_ratio`, or the victim was already stumbling) | `ChecksSoft` | **TOO SOFT** |
| its window (12 frames) ran out on nobody | `ChecksMissed` | **MISSED** |

One line a frame, in this order: JUST GOT UP (a charge into a skater that just got up can bounce the human too, and the grace is the reason), BOUNCED OFF, NOT WITH THE PUCK, TOO SOFT, MISSED. `ChecksMissed` does not count a window that closed because the skater took the puck inside it.
A cross-check that is the body part of a lift counts for none of the three (the press was the stick's). A landed hit (stumble, knockdown) needs no line: the hold,
the kick and the figure say it. Log: `JAMHUD frame=N contact_line=TOO_SOFT | MISSED | JUST_GOT_UP ... soft= missed= graced=`.

**The two edges evolution 1 left** are closed by one function, `AJamSkater::HitStopPick` (`JamPawns.cpp`; `AHokejGameMode::HitStopRoute` now calls it and holds on its
pick):
- a frame that carries a knockdown between two bots AND a stumble (or a knockdown) of the human used to hold nothing, because the route read the frame's last
  knockdown first: the pick looks, in order, for a knockdown the human took, one it landed, a stumble it took, one it landed, its own bounce, and both ends of the
  hit come from the victim's `ContactBy`;
- the `JAMHITSTOP` line of a stumble printed `strength=0.00` (read from a knockdown event a stumble does not have): it now prints its `check` event's strength.

## 3. The tests

| Test | What it proves | Measured |
|---|---|---|
| `Jam.Core.Contact.ReleaseGrace` (new) | head-on at full speed puts the victim down; the same hit 10 frames after its release is a bump with no `check` and no `knockdown` event, the real ratio in the read-out, the grace flag and the attacker's `ChecksGraced`; the attacker pays the bump's price; 100 frames after the release the hit lands again; grace 0: it lands at once; `contact.momentum false`: no grace starts; grace 200 still holds at 100 frames; the countdown; `StateHash()` covers the counter and none of the read-outs; the soft hit and the whiff count on the attacker; the key loads, -1 / 301 / 1.5 are refused | ratio 4.61: down 150 frames -> **bump** inside the grace (attacker 11.0 -> 4.4 m/s, it recoils) -> down 150 frames after it; 90 -> 60 after 30 frames -> 0; soft hit at ratio 0.35; the whiff on frame 12 |
| `Jam.Core.Contact.HitStopPick` (new) | the hit the hold is for, six frames built by hand: a stumble of the human carries its check event's strength; a bot-on-bot knockdown in the same frame does not hide it; of two knockdowns the human's is picked with its own checker and kind; bot on bot alone: nothing; a knockdown the human lands; the neutral contact (the event names the checker, the base holds); the human's bounce; the shotgun's knockdown is not a check | holds 2 / 1 / 3 / none / 4 / 3 / 1 / none |
| `Jam.Core.ContactGraceMatch` (new) | the neutral proof (section 4) and the brief's measurement (section 5); asserted: no hit lands on a skater inside its grace in the ten matches at 90 and at 120; at 0 no hit meets a grace | sections 4 and 5 |
| `Jam.Core.ContactBoardMatch` (evolution 1's, edited) | its neutral arm now sets BOTH keys neutral (`board_bonus 1`, `release_grace_frames 0`) and still asserts main's three hashes | section 4 |
| `Jam.Hud.ContactLine` (evolution 1's, extended) | the three new lines and the order | |

## 4. The hash, before and after, and the neutral proof

Baseline = this track's evolution 1 (the tree this lane started from). `Jam.Core.ContactMatch` / `ContactBoardMatch` / `ContactGraceMatch`, pro, 10 800 frames, the
Regular playtester as the human:

| | main (ee14438) | baseline (evolution 1) | this tree, `release_grace_frames` 0 | this tree, grace 90 (the default when measured) | this tree, both keys neutral |
|---|---|---|---|---|---|
| pro seed 1 | `6ed8f16ab724086b` | `b59b988423ca43cf` | **`b59b988423ca43cf`** | `b59b988423ca43cf` | **`6ed8f16ab724086b`** |
| pro seed 2 | `dc1a21fe22e0bc6f` | `2333d70ef430d6f3` | **`2333d70ef430d6f3`** | `2333d70ef430d6f3` | **`dc1a21fe22e0bc6f`** |
| pro seed 3 | `7d34793757849ca9` | `4f3c95b28136e412` | **`4f3c95b28136e412`** | `4f3c95b28136e412` | **`7d34793757849ca9`** |

The two bold columns are asserted (`ContactGraceMatch` against the baseline, `ContactBoardMatch` against main). **On these three seeds grace 90 hashes as the
baseline too**: in those nine minutes no hit fell inside 90 frames of a release and the brain held no press back (the shortest release-to-down gaps are 111, 93 and
621 frames), so the matches are frame for frame the same. That the rule is a rule shows where it fires: the scene (`ReleaseGrace`: grace 0 and grace 90 end in
different states) and three of the ten 100-second matches of section 5 (seeds 5, 6, 7 hash differently at 0 and at 90). `Jam.Core.Determinism` (hockey.regular,
seed 1, frame 600) and the all_star seeds: see the full filter's line in section 6.

**Neutral value / revert path:** `"contact": { "release_grace_frames": 0 }` gives the baseline, state hash included; with `"board_bonus": 1` beside it, main at
ee14438. The HUD lines and the hit-stop pick are presentation (no rule, no hash); the lines have no switch, `jam.hitstop 0` turns the hold off as before.

## 5. The brief's measurement: 100 s at pro, five a side, the scripted human, ten seeds, one process

`Jam.Core.ContactGraceMatch`: hockey.regular at pro (penalties off, no score limit), the Regular playtester as the human, seeds 1 to 10, 6 000 frames each, the same
seeds in every arm (grace 0 = before, 90 = the first ruling, 120 = two seconds, the default since R-0051).

| ten matches of 100 s | grace 0 (before) | grace 90 (the first ruling) | grace 120 (the default) |
|---|---|---|---|
| knockdowns of the human, total | 30 | **26** | 26 |
| ... per seed 1 to 10 | 3 4 0 5 4 5 3 0 3 3 | 3 4 0 5 3 2 3 0 3 3 | 3 4 0 5 3 2 3 0 3 3 |
| ... matches with five or more | 2 | **1** (seed 4) | 1 (seed 4) |
| knockdowns of a bot, total | 34 | 36 | 35 |
| figures down again within 120 frames (2 s) of their release | 2 | **1** | **0** |
| ... the shortest release-to-down gap | **41 frames** (the human, seed 6) | 111 frames (a bot, seed 1) | 312 frames |
| stumbled by a hit within 120 frames of release | 1 | 0 | 0 |
| hits the grace made a bump | 0 | 1 (on the human, thrown by a bot, seed 5) | 1 |
| landed hits on a skater inside its grace (asserted 0) | | 0 | 0 |
| matches that changed against grace 0 (hash) | | 3 of 10 (seeds 5, 6, 7) | 4 of 10 (and seed 1) |
| the human's own check presses: resolved soft / whiffed | 0 / 49 | 0 / 51 | 0 / 50 |

The three-minute matches of section 4 (the same game at grace 0 and 90): knockdowns of the human 5 / 9 / 1; down again within 120 frames of release: a bot once at
111 frames (seed 1), **the human twice at 93 frames and up (seed 2)**: each just after a 90-frame grace would have ended.

**Reading.**
1. The rhythm device B played is mostly gone already on this build (B played the tree before the vision: there a bot's check landed from any speed and the carrier
   could be hit by anybody). Before this lane the human goes down 3.0 times per 100 s, not 15, and a figure is put down again inside two seconds twice in 1 000 s.
2. The grace removes what is left of it: seed 6 is the case B described (down again 41 frames after the release, five knockdowns in 100 s); with the grace the
   human goes down twice in that match. The brain's line does most of the work: one hit in ten matches met a grace at all, the others were not thrown.
3. **"No figure down again within 2 s of its release" is met at 120 frames, not at 90.** At the first ruling's 90 one bot is down again 111 frames after its release
   in the ten matches (and in the three-minute matches three re-downs sit at 93 to 111 frames): a check started the moment the grace ends lands inside the two
   seconds the proof names. 90 frames is 1.5 s. **R-0051 set the default to 120** on these numbers; its cells are in section 6.
4. **"The human down fewer than five times" holds in nine matches of ten** (mean 2.6 per 100 s). Seed 4 has five at every grace: none of them is a re-down (its
   shortest gap is 312 frames); they are five separate checks on a carrier, which is the level's aggression (R-0046), not this rule's.
5. The scripted human's own presses whiff: about 50 windows in 1 000 s close on nobody and none resolves soft (in the three-minute matches 30 whiffs against 10
   landed checks). With the new line a player sees **MISSED** about once in 20 s if it presses as the script does: the line is honest, and the number is the
   case for the track's next evolution (section 9, point 1).

## 6. The gate, and the three target cells

**The full `Jam` filter on the final tree: 164 PASS, 0 FAIL** (80 s; `Game/logs/a92-tests-a113-full2-ue.log`, the gate's invocation through
`tools/qa/ctrl/a92-tests.ps1`; evolution 1 had 161: the three new tests are the difference; the same count on the build before the pawn's log lines, 99 s).
Four editor builds PASS (121 s, 50 s, 60 s, 256 s: the last one queued behind another lane's build).
Hashes on the default: `Jam.Core.Determinism` (hockey.regular, seed 1, frame 600) `f3613b5fcd33ce32` = main's and evolution 1's; `Jam.Core.ContactMatch` pro
`b59b988423ca43cf` `2333d70ef430d6f3` `4f3c95b28136e412` and all_star `a3716bff403f4f6c` `75b02e84a155fb56` `4d48718f184eed47` = evolution 1's (no hit fell inside a
grace in those six matches). Node tests (the gate's step 3): 210 of 211; the one is `tools/barks/barks.test.mjs`, the CRLF checkout of `BarkData.inl` in this worktree
that evolution 1 reported (it passes in the main checkout). The three generators were re-run after the schema change (gen-schema-data 3 files, gen-parity, gen-designer-data 36 cuts).

**The cells** (R-0046; 600 matches a cell and arm, paired: seeds 1 to 600 in both arms; `tools/perf/results/a101/paired.mjs`, rows `g90-vs-g0` and `g90-vs-g120` in
`cells.jsonl`; whole matches, about 300 s each):

| cell | target | grace 90 (95 % interval) | grace 0 (the baseline) | difference | pairs that flipped (90 wins only / 0 wins only) | identical hash | knockdowns of the human a match, 90 / 0 |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85 | **84.8** (81.7 to 87.5) | 85.2 | -0.4 | 4 / 6 | 556 of 600 | 3.60 / 3.67 |
| regular at pro | 50 | **51.8** (47.8 to 55.8) | 53.8 | -2.0 | 29 / 41 | 394 of 600 | 8.32 / 8.44 |
| good at all_star | 25 | **25.3** (22.0 to 29.0) | 25.3 | 0.0 | 22 / 22 | 411 of 600 | 7.69 / 7.76 |

| cell | target | grace 120 | grace 90 | difference | pairs that flipped (120 wins only / 90 wins only) | identical hash | knockdowns of the human a match, 120 / 90 |
|---|---|---|---|---|---|---|---|
| novice at rookie | 85 | **85.3** | 84.8 | +0.5 | 3 / 0 | 575 of 600 | 3.57 / 3.60 |
| regular at pro | 50 | **51.0** | 51.8 | -0.8 | 19 / 24 | 446 of 600 | 8.10 / 8.32 |
| good at all_star | 25 | **25.3** | 25.3 | 0.0 | 10 / 10 | 454 of 600 | 7.66 / 7.69 |

No cell is more than 4 points from its target and no difference is distinguishable from zero on 600 pairs (the flips are 4 against 6, 29 against 41, 22 against
22): **the grace moves no cell and the curve needs no re-set for it.** The pro cell reads 1.8 over its target on these seeds (evolution 1 left it 3.8 over).
Two thirds of the matches at pro and all_star, and nine in ten at rookie, are the same match with and without the grace: the rule fires in one match in three, once
or twice. Knockdowns of the human fall by about 0.1 a match: the grace takes the re-downs, not the knockdowns.
**At 120, the default since R-0051, every cell is within 1.0 of its target**: the value that meets the two-second proof (section 5) costs the curve nothing.

## 7. A live game process (`-game -nullrhi`, the game mode's scripted human)

`tools/commentary/match.ps1` (hockey.regular = pro, `jam.ScriptHuman 1`), 6 021 frames, once on the default and once on the variant with
`release_grace_frames 0`; read by `tools/perf/results/a101/grace-live.mjs` (`JAMVIS knockdown` / `getup`, `JAMHITSTOP`, `JAMHUD`):

| first 6 000 frames | grace 90 | grace 0 |
|---|---|---|
| knockdowns of the human / of a bot | **6** / 2 | 6 / 2 |
| figures down again within 120 frames of release | 0 | 0 |
| the human's release-to-down gaps (frames) | 455, 573, 1909, 228, 1303 | the same |
| `JAMHITSTOP` holds (all knockdowns of the human) | 3, 2, 5, 3, 5, 3 frames for ratios 1.82, 1.29, 3.98, 1.46, ... | the same |

The two runs are the same match: no hit fell inside a grace. The hold's pick works in a live process (the same holds as evolution 1's run of this match).
**The scripted human of the game mode goes down six times in 100 s, with and without the grace**: R-0049's "fewer than five" is not met in this run and the grace
cannot meet it, because the six are separate checks 3.8 s and more apart (section 9, findings).

The tackle drill (`jam.ctrl.Drill 1`, the human's ten scripted tackles; `tools/commentary/out/a113-drill1b/game.log`): eight land (holds 5, 3, 3, 2, 3, 2, 3, 2
frames, as on evolution 1) and the two that do not are told now: `JAMVIS press frame=1276 ... result=missed` and `frame=2937 ... result=missed` (the press of
attempt 4 came with the two bodies drifting apart at 0.08 m/s: the window closed on nobody). No `JAMHUD` line: the HUD does not run under `-nullrhi`.

## 8. For device B, and the contract texts this lane proposes (the Director's to write)

- **`contracts/events.md`, `check` and `knockdown`:** no change of shape. Add one sentence to `check`: "a hit on a skater inside its release grace
  (`game.contact.release_grace_frames` after a knockdown ends) emits no `check` event: it is a bump (the victim's `PlayHitReaction`)."
- **C-0001 (`contracts/skater-visual.md`):** no new call. `SetKnockedDown(false)` now also starts the grace: for `release_grace_frames` the figure can only be
  bumped. **Ask to B (BF1):** the get-up may take up to those frames and read as "not yet steady" (a short wobble on a bump taken inside it); nothing depends on it.
- **R-0049 point 1:** superseded by R-0051 (default 120, neutral 0); the schema's default column says 120.
- **For S2's acceptance rows** (bump, refused, stumble, braced): the pawn logs the cause of a press that did not land, `JAMVIS press frame=N skater=K team=T human=1
  result=just_got_up|too_soft|missed soft= missed= graced=`, and `JAMVIS grace frame=N skater=K ... by=A ratio=R grace_left=F` for a hit the grace softened; the HUD's
  `GetContactText()` returns the line on screen. The HUD does not run under `-nullrhi` (no `JAMHUD` line exists in any headless log), so the rows read the HUD
  only in a package.

## 9. Not proven here, findings, and what comes next

**Not proven by this lane (it does not hold the cursor): queued for a package.**
1. The acceptance rows **check / crosscheck / checked** on a Development package of this tree (`tools/qa/accept/accept.ps1 -From 7 -To 8`). Expected to hold: the
   drill's hits are on skaters that were not just released (the tackle drill above: 8 of 10 landed, as on evolution 1), and the grace touches nothing else.
2. How the three new lines LOOK (one capture each of TOO SOFT, MISSED, JUST GOT UP), and the line itself in a live process: the HUD does not tick headless, so
   `JAMHUD contact_line=` is proven by its counters (`Jam.Core.Contact.ReleaseGrace` points 1 and 7), its text table (`Jam.Hud.ContactLine`) and the pawn's
   `JAMVIS press` line only.
3. Whether MISSED once in 20 s reads as noise to a person: a taste call for the operator's play (the line can be dropped by leaving `ChecksMissed` out of
   `UpdateContactLine`; the counter and the log stay).

**Findings outside the rule** (file and line in the report):
- The game mode's scripted human is knocked down 6 times in 100 s at pro with or without the grace; the playtester 2.6 times, five in one match of ten. These are
  separate checks 3.8 s and more apart on a carrier: the level's aggression and the brain's choice of victim. A per-victim rest in the brain ("the same bot does
  not check the same skater again within N frames", or a team-wide rest on one victim) is the lever; it is track Q2's file and the curve track's number.
- A full-speed charge into a skater inside its grace bounces the attacker (the brief's "the attacker's own price as for any bump": the recoil test is the bump's).
  For a bot this is almost never seen (the brain holds its press). For the human it is the price of mashing check on a skater that just got up; the HUD says JUST
  GOT UP. If the operator finds it harsh, the change is one condition (`!bGraced` in `bRecoil`).
- The shotgun's knockdown ignored the grace in this lane's first cut; R-0051 point 3 closed it (section 1).
- Every headless start in this lane used `-Multiprocess` (the Director's note, lane A107's finding): the full filter ran in 99 s (evolution 1: 322 s), the 3 600
  paired matches in 190 s (371 s), no start waited on the build tool lock.

**The next three evolutions of track F1, ranked, each with its proving number.**
1. **The human's check finds its man.** The scripted human's presses whiff: about 50 windows in 1 000 s close on nobody, none resolves soft; in the three-minute
   matches 30 whiffs against 10 landed checks (three presses in four do nothing). A press should carry the body the last half metre: a short lunge along the
   line to the nearest opponent inside a 45-degree cone and `contact.check_reach_m` of the edge (default 0.5, neutral 0), the lunge's speed counted in the impulse
   as the 1.5 m/s it already is. Proving number: the playtester's whiff share from **75 %** to under **40 %** (`ChecksMissed` against landed checks in
   `Jam.Core.ContactGraceMatch`'s matches), the human's landed checks from 1.9 per 100 s (10 in the three three-minute matches) to 3 or more, the three cells
   within 4 points, the neutral value = this tree's hashes.
2. **The hit's size on screen** (the queue's line after "every outcome reads"): the hold 2 to 6 frames and the kick by the frames down, so a 150-frame knockdown
   looks twice the 60-frame one; the braced bounce as an acceptance row and its own figure take. Proving number: hold and kick monotone in `KnockFrames` 60..150
   (a sweep test as `Jam.Core.Contact.HitStop`'s), a kick of 12 cm only at 150 frames; the row "braced" green in S2's matrix.
3. **The board check as a play** (evolution 1's first): the hit that sends a victim into the boards counts as the second body within its stagger, and the
   brain finishes into the boards. Proving number: landed hits the boards answered per pro match from **1.0 to 3 or more** (`Jam.Core.ContactBoardMatch`'s bins),
   the three cells within 4 points, neutral key `contact.board_slam` 0.

## 10. Files

Sim and key: `Game/Source/JamCore/Private/JamSim.cpp` (`StepSkater`: the countdown and the release; `ResolveContact`: the cap and the two counters; `KnockDown`,
`FaceoffReset`, `Restart`: the reset; the check window's whiff counter in `Step`; `StateHash`), `Public/JamSim.h` (the state and the read-outs),
`Public/JamCartridge.h`, `Private/JamCartridge.cpp`; schema `game.schema.json`, `limits.json`, `DEFAULTS.md`, `README.md` and the generated `JamSchemaData.inl`,
`JamDesignerData.inl`. **The brain (track Q2's file, one condition): `Game/Source/JamCore/AI/JamAICore.cpp`, the body-check press's reach condition**; one field
in `JamAICore.h`; one fill line in `JamSimAdapter.cpp`. Presentation: `JamPawns.cpp/.h` (`HitStopPick`, `FJamHitStopPick`, the `JAMVIS press` / `grace` lines),
`HUDWidget.cpp/.h` (three lines), **`AHokejGameMode::HitStopRoute`** (`HokejGameMode.cpp`: the inline selection replaced by the pick; nothing else in that file).
Tests: `JamContactTests.cpp` (three new tests, the counter's grace fields, `Match`'s frames parameter, `ContactBoardMatch`'s neutral arm),
`JamGame/Private/ContactFeelTests.cpp` (HUD rows). Measurement: `tools/perf/results/a101/paired.mjs` (`-Multiprocess`), `grace-live.mjs` (new), `cells.jsonl`
(six rows added). Status: `status/A113.md`.

## 11. The final tree after R-0051 (default 120, the shotgun respects the grace)

**The full `Jam` filter: 164 PASS, 0 FAIL** (91 s; `Game/logs/a92-tests-a113-full3-ue.log`; build 5 PASS). The count is unchanged: the shotgun's case is a row
inside `Jam.Core.Abilities.Shotgun` (the third skater on the path inside its grace: down 1 1 0, two `knockdown` events, its grace still running).

| pro, 10 800 frames | baseline (evolution 1) | this tree, `release_grace_frames` 0 (asserted) | this tree, the default 120 | both keys neutral (asserted = main) |
|---|---|---|---|---|
| seed 1 | `b59b988423ca43cf` | **`b59b988423ca43cf`** | `7fb31c1f16b3f8c4` | **`6ed8f16ab724086b`** |
| seed 2 | `2333d70ef430d6f3` | **`2333d70ef430d6f3`** | `a2be45743aebf331` | **`dc1a21fe22e0bc6f`** |
| seed 3 | `4f3c95b28136e412` | **`4f3c95b28136e412`** | `4f3c95b28136e412` (no hit inside a grace) | **`7d34793757849ca9`** |

all_star seeds 1 to 3 on the default: `a3716bff403f4f6c` `75b02e84a155fb56` `4d48718f184eed47` (unchanged). `Jam.Core.Determinism` (hockey.regular, seed 1,
frame 600): `f3613b5fcd33ce32` (unchanged).

The three-minute pro matches on the default 120: knockdowns of the human **4 / 5 / 1** (5 / 9 / 1 at grace 0 and at 90), nobody down again within 120 frames of
its release (shortest gaps 475 / 539 / 621 frames; at 0 and 90: 111 / 93 / 621), no landed hit inside a grace (asserted). Seed 2 is the match where the human went
down again 93 frames after a release, twice: at 120 it goes down five times, not nine. The ten 100-second matches are as in section 5's 120 column (the shotgun's
condition changed none of them): the human down 26 times, nobody down again within two seconds, shortest gap 312 frames.

`Jam.Core.Contact.ReleaseGrace` on the default: grace 120 at the release, 110 at a charge ten frames later (a bump, the attacker recoils), 90 left after 30 frames,
0 after 130, where the same hit lands for 150 frames again. The cells were not re-measured (section 6 has them at 120: 85.3 / 51.0 / 25.3).
