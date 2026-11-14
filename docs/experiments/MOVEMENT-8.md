# Movement 8: the player is warned before the two knockdowns (lane A166, track F2, evolution 7, 2026-11-14)

MOVEMENT-7 section 4 read where the human lies down at pro (9.07 % of play, 676 knockdowns in 100 matches): **65 % within half a second of his first
touch of the puck** (two skaters race head-on for a loose puck, he touches it first, the pressed check lands eight frames later) and **15 % with the
puck his for a second and a half, the hitter 11 m off a second earlier and the stop not used** (law 3's own case). The game showed him nothing before
either. This lane draws one mark for each (presentation, no sim change), measures both marks against that trace, and measures what the first is worth
to a player who obeys it (the instrument's switch, off by default). Status with timestamps: `status/A166.md`. Raw logs and runners: `Game/logs/a166/`
(not landed; the edit scripts are `Game/logs/a163/edit-a166-*.py`); cells: `tools/perf/results/a166/`.

## 1. The two rules (one header, three readers)

`Game/Source/JamCore/Public/JamCarrierMarks.h` (new, header only): pure read-outs of the sim. The HUD draws them, the trace measures them, the
instrument may obey the first. Nothing in the sim reads the file.

| Mark | The rule says yes when | Numbers (`FContestRule`, `FStopRule`) |
|---|---|---|
| **The contested puck** (`Contested(Sim, Human)`) | nobody carries; the puck and the human close (he skates at it, or a pass comes at him) and he has it inside a second; an opponent closes on the puck too and is there from a quarter second before him to 0.6 s after; and the two close on each other head-on | he: 3 m/s at the puck or more, 1.0 s or less; the opponent: 3 m/s or more, -0.25 to +0.6 s of him; the two: 8 m/s of closing or more; "has it" = the puck inside 1.6 m of the body |
| **The stop's distance** (`StopMark(Sim, Human)`) | he carries at speed and is not braking; an opponent within 60 degrees of his travel closes on him. The mark stands **8 m (five stick lengths) from him towards that opponent**. Armed (chalk) while the opponent is up to 20 m off; **lit (the opponent's colour) from a quarter second before the opponent is 11 m off until he is 7 m off**; gone inside 7 m (a stop begun there loses the feet) and while he brakes | he: 3 m/s or more; closing 3 m/s or more; the window 7 to 11 m is REVIEW-2's (the stop begun there passes 66 to 100 % at pro); the quarter second is the hand's (`ReactSec`): head-on at 15 m/s the window itself lasts a quarter second |

Three choices the trace made (section 3): the contested rule reads the puck's motion (with a lying puck only, it showed 31.8 times a match for the same
recall); the stop's cone is 60 degrees, not 35 (recall 59 to 72 % at a higher precision); the stop mark has the hand's lead (it was lit for two to six
frames in the first capture: unreadable).

## 2. What the player sees

| Mark | On the ice | Where and when | HUD |
|---|---|---|---|
| The contested puck | a ring of 1.2 m round the loose puck **in the opponents' colour**, breathing (1 to 1.2 x, three times a second) | from the frame the rule says yes to the frame somebody has the puck; 19.3 times a match at pro (3 % of play), 15 frames at a time on average and 28 frames (the median) before a touch that ends on the ice | `ContestRing`, made on first use by `UJamHudWidget::UpdateMarks` |
| The stop's distance | a small chalk ring (0.7 m, 60 % opacity) on the ice between him and the closing defender, 8 m ahead; **it takes the opponents' colour, grows to 1.1 m and breathes** when the stop is on | on the ice 39.5 % of his carry frames at pro, lit 22.9 % (14.7 times a match, 23 frames at a time) | `StopMarkRing`, the same |

Code: `Game/Source/JamGame/Private/HUDMarks.cpp` (new: `UpdateMarks`, the two rings, the cvars), a block of declarations in `HUDWidget.h` after lane A163's,
and **one call line in `HUDWidget.cpp`**, before `UpdateCarrierCues(F)`. No other HUD function is touched, `BuildTree` included: the two rings are made
on first use, added to the root canvas at z order -1 (on the ice: under every other element). `jam.hud.Marks 0` turns both off.
**Nothing on a downed figure**: each mark's box is tested against a box round every downed skater (1.3 m of ice round it and the point over its head)
and the mark is not drawn where they meet (`UJamHudWidget::MarkMeetsDown`). Lane A158's exact boxes (`DownBoxesNow`, `HUDClearDown.cpp`) are on main and
not in this tree: on main the local boxes can be replaced by them in one line (`HUDMarks.cpp`, the `Downs` array).

The colours say two things without words: **yellow** (the ring under a carrier) = no stick takes that puck (A163); **their colour on the ice** = one of
them is coming and it is your move: round a loose puck, he is there when you are; the ring ahead of you, stop now. (The stop mark was lit yellow at
first; the second capture showed it beside the human's own yellow ring after a draw he won, two yellows saying two things, and it changed.)

## 3. The marks against the trace (pro, the regular player, 100 matches, seeds 1 to 100; `Bar.Movement.CarryTrace`)

The trace evaluates the same two functions on every frame of the same matches whose knockdowns MOVEMENT-7 classed (its base is that run match for
match: 1,662 carries, 63 wins), and reads each mark against what then happened. `JAM_MARK_RULE="early=..;late=..;closing=..;hsec=..;cone=..;react=.."`
sweeps the rules without a build.

**The contested puck.**

| | |
|---|---|
| shown | 1,932 times, 19.3 a match, 3.05 % of play |
| his possessions (every one, the short touches too) | 3,808, 38.1 a match |
| the mark was lit before the touch | 1,178 (30.9 %) |
| **precision**: of those, he was down inside half a second | **316 (26.8 %)** |
| with no mark before the touch, down inside half a second | 121 of 2,630 (4.6 %): a marked puck is 5.8 times as dangerous |
| **recall**: of the 439 knockdowns of the class "hit as he won it", the mark had been lit before his touch | **316 (72.0 %)** |
| ... a third of a second (20 frames) or more before his touch | 261 (59.5 %) |
| median lead | 28 frames (0.47 s) |

Sweep: the first rule (a lying puck only) 31.8 showings a match, recall 71.8 %, precision 24.0 %; its looser variant (`early 0.5`, `late 0.9`,
`closing 6`, `hsec 1.3`) 43.2 showings, 76.5 % and 19.5 %; its tighter one (`early 0.15`, `late 0.4`, `closing 10`) 66.7 % and 28.3 %, with 20 frames of
lead in only 29.8 %. With the puck's motion read (shipped): 19.3 showings, 72.0 % and 26.8 %; wider windows on top (`hsec 1.3`, `early 0.4`, `late 0.8`)
72.7 % and 25.0 %.
The 28 % it does not see are not bought by a wider window: they are another picture (the first of the next evolutions).

**The stop's distance.**

| | |
|---|---|
| lit | 1,469 times, 14.7 a match, 23 frames at a time |
| **recall**: of the 103 knockdowns of the class "the stop was on", the mark was lit inside the second before | **79 (76.7 %)** |
| **precision**: of the lit showings in which he did not brake inside 1.5 s, he was down inside 1.5 s | **262 of 1,090 (24.0 %)** |
| of the lit showings in which he braked inside 1.5 s, he was down | 20 of 379 (**5.3 %**) |

The last two rows are the same hand in the same matches: where the stop followed the mark, the fall came a fifth as often. Sweep: a cone of 35 degrees
recall 59.2 %, precision 19.8 %; 60 degrees 71.8 % and 24.7 % (shipped); 90 degrees 82.5 % and 24.8 % (lit 18 % of his carry frames: "ahead" would no
longer be true of it). The hand's lead: none 71.8 % / 24.7 %, lit 16 frames at a time; **0.25 s 76.7 % / 24.0 %, 23 frames (shipped)**; 0.4 s 78.6 % / 24.9 %.

## 4. What the first mark is worth to a player who obeys it (the instrument's arm)

`JAM_PLAYTESTER_CONTEST=brake` (`JamPlaytester.cpp/.h`; absent = the player as shipped: every earlier measurement stays comparable): while the contested
rule says yes and the puck is not his, the scripted human stops short of the puck (against his motion while he moves, then stands) and lets the other
one have it; his reaction lag applies as to every decision of his. Nothing else of the script changes.

Pro, the regular player, 100 matches (the trace):

| | as shipped | brakes for the mark | the brief's target |
|---|---|---|---|
| knockdowns a match | 6.76 | **4.62** | |
| knockdowns "as he won it" a match | 4.39 | **2.20** | toward 2.5: met |
| he lies down, share of play | 9.07 % | **5.65 %** | under 6.5 %: met |
| wins of 100 | 63 | 52 | |
| opponents carry, share of play | 27.5 % | 30.2 % | |
| his carries a match / median | 16.6 / 0.88 s | 16.6 / 0.87 s | |

The cells (600 a cell, seeds 1 to 600, paired, the ladder's instrument `JAM_PLAYTESTER_CHECK=closing`; `baseC` -> `brakeC`):

| | novice at rookie | regular at pro | good at all_star |
|---|---|---|---|
| win, as shipped | 86.5 | 53.2 | 30.0 |
| win, braking for the mark | 84.7 (**-1.8**) | 49.5 (**-3.7**) | 32.8 (**+2.8**) |
| his knockdowns a match | 3.21 -> 2.54 (-21 %) | 6.86 -> 4.86 (-29 %) | 8.48 -> 5.47 (-35 %) |
| goals for : against | 2.74:1.10 -> 2.75:1.10 | 2.13:2.00 -> 2.07:2.08 | 1.59:2.45 -> 1.67:2.40 |

**So the mark, obeyed blindly, takes a quarter to a third of his falls and 3.4 points of play off the ice, and costs the puck he did not race for:
2 to 4 points of winning at rookie and pro, nothing at all_star** (where the race was mostly lost anyway). A person does better than "always brake":
the mark also says when to hit instead (the one who arrives second may check; the carrier may not) and when to cut across. The switch stays off: it is
the instrument, and the ladder was set with the player who races.

## 5. Proof

- **Tests in the gate** (the full `Jam` filter on the final tree: **253 pass, 0 fail**; 251 after lane A163): `Jam.Core.Movement.Marks` (staged scenes: head-on for a loose puck 6 m off at
  8 m/s each is contested, 0.55 s each, 16 m/s of closing; the opponent skating away, two seconds off, there half a second before him, side by side,
  the human standing, a mate instead: not; the stop mark armed at 15 m, 8 m from him towards the defender, lit at 13.5 m when closing at 13 m/s and not
  when closing at 8, lit at 10 and 7.5 m, gone at 6 m, to the side, behind, with nothing closing, and while he brakes) and `Jam.Hud.Marks` (the
  breathing by the sim's frame, the sizes, the mark's 8 m inside its own window, the downed-figure rule).
- **Hash identity**: the default match on the final tree (arm `n0`) against the tree as it came to lane A163 (arm `base`, a7d89871), 600 matches a cell,
  the default instrument: **the same hash, frame count and score in 600 of 600 matches of each of the three cells** (build 4; the final build 5 differs from it by the lit mark's colour in `HUDMarks.cpp`, and its 100 pro matches of the trace are the base's again, 100 of 100) (`node tools/perf/results/a166/same.mjs base n0`). The marks read; the instrument's switch is off unless the
  environment sets it.
- **Off screen** (`tools/qa/screen/u1-stills.ps1 -Tier clay -Mode after -PlaySeconds 75 -Port 4766 -ExtraCVars "jam.hud.MarkStills=8"`, the editor
  binary off screen, 4,200 sim frames): the HUD's own lines, `JAMHUD frame=1009 mark=contested on=1 opp=5 human_s=0.65 opp_s=1.21 closing=11.9 shown=1`
  and `JAMHUD frame=1036 mark=stop state=lit def=7 dist=7.5 closing=21.9 shown=1`, and stills (three captures; the last on the final build) the HUD takes itself a few frames into a showing (new proof
  cvars `jam.hud.MarkStills N`, `jam.hud.MarkStillTag`): `docs/experiments/movement-8/contested-f1017.png` (after a draw he won: the puck ringed in orange
  ahead of him, his own ring blue), `contested-f2320.png` (an opponent on the loose puck, the ring round it, the human 2 m short). `stop-lit-f1040.png`: the stop mark lit in the opponents' orange 8 m from the human towards a closing defender, the human's own ring yellow (he has just won the puck).
- The Shipping target compiles (`tools/run/build.ps1 -Project Game -Target Game -Config Shipping`, on the final tree).
- **Not proven**: that a person reads either mark in a third of a second (the stills say the contested ring is the largest thing near the puck; nobody
  has played with it); that no mark is ever drawn over a downed figure in a match (the rule is tested, the capture did not look for the case; lane
  A158's probe `jam.hud.DownProbe` on main will count it once the two rings are in its list of elements); **drill 21's first tries inside the 7 to 11 m
  window with the stop mark and without can only be judged by a person** (the scripted carrier of the drill brakes at a programmed distance: it cannot
  see a mark).

## 6. For the operator (one sentence each)

> **A ring in the other team's colour round a loose puck means one of them will be there when you are: take it and you are hit as you touch it, so
> brake short of it, or let him have it and hit him.**
>
> **When you carry and a defender comes at you, a chalk ring lies on the ice between you, five stick lengths ahead: the moment it turns his colour and grows, press
> against your motion (S if you skate with W); stop there and he goes past, stop later and you are on the ice.**

Drill 21 (script 2, by his own keys) with `jam.hud.Marks 1` and then 0: the question only he can answer is whether his first brake falls inside the
window more often with the ring.

## 7. Findings outside the family

- `Game/Source/JamGame/Private/HUDClearDown.cpp` on main (lane A158): the loop that hides a HUD element over a downed figure walks a list of widgets;
  `ContestRing` and `StopMarkRing` are made later than `BuildTree` and are not in it. They keep off downed figures by their own rule; adding them to
  that list and to the down probe is the screen lane's line.
- The contested ring shows in the second after most draws the human wins (the opposing centre is head-on by construction). It is true there (lane
  A163's trace: the hit as he won it), but the faceoff's own ring and words are on screen at that moment: whether the mark should wait for them is the
  screen lane's call; the trace can count how many of the 19.3 showings a match are a draw's.
- The window lane's question about STICK LOCKED (answered in `status/A166.md`, `LOCK ANSWER`): a press is a miss only next to an opposing carrier
  (about 3 m); `docs/PLAY-ME.md` should carry that condition.
- The brain (track Q2): 65 % of the human's knockdowns are a pressed check within half a second of his first touch at 16 m/s of closing. The instrument
  that declines the race loses 3.7 points at pro and falls 29 % less: the pro's presser is rewarded for charging the puck's first toucher rather than
  the puck. A hit capped at a stumble inside a carry's first 15 frames (the contact track's, MOVEMENT-7) would now have a measured upper bound: these
  2.2 knockdowns a match.

## 8. Next evolutions (ranked)

1. **The instrument obeys the stop mark too, and a person's choice at the contested puck** (the switch gains `stop` and `hit`: brake when the stop mark
   lights; at a contested puck press the check instead of braking when he will be the second there). Proof: knockdowns of the class "the stop was on"
   1.03 a match -> under 0.5 and he lies down under 5 % of play at pro with both obeyed; the three cells: whether "hit instead" buys back the 3.7 points
   the brake costs at pro.
2. **The 28 % the contested ring does not see.** 123 of the 439 hit-as-he-won-it knockdowns had no mark before the touch and wider windows do not find
   them: trace them by what the puck was doing in the second before (poked loose beside him, a rebound, a pass from behind his view, a hitter who was
   not skating at the puck but at him). Proof: recall 72 % -> 85 % or more at a precision of 25 % or more and no more than 25 showings a match.
3. **The pickup settles, for every skater, as a key, with the curve's re-set** (carried over from MOVEMENT-7; the code is in the tree at 0). Proof: the
   puck on nobody's blade at pro 49.7 % -> 45 % or less (the arm read 43.1 %), and the cells back inside 3 points after the curve lane's numbers
   (the arm alone: +5.8 / -9.0 / -5.0).
