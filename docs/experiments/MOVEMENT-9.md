# Movement 9: the marks a real window can read, and what obeying them is worth (lane A178, track F2, evolution 8, 2026-11-14)

Package H30 carries the two warnings of MOVEMENT-8 as the screen lane and the design review left them: the contested ring (12 m/s of closing) and the
stop's LINE (8 m/s, lit to 9.5 m). A real window read them (`.claude/worktrees/a108/status/A155.md`, rows H30 LINE and H28 ROW 2): the ring showed twice
in a minute and was on no still (a median of 5 frames on screen); the lit line was 8 px high at 1920 x 1080 and its chalk state lasted a tenth of a
second; the line off to one side was never seen. This lane makes both legible (presentation), judges the side case, and measures what the two marks are
worth to a player who obeys both (the instrument's switches, off by default). Status with timestamps: `status/A178.md`. Raw logs, edit scripts and
runners: `Game/logs/a178/` (not landed); cells: `tools/perf/results/a178/`. The main it stands on: 3fa9c2e6.

## 1. The contested mark can be seen

| | Package H30 | Now |
|---|---|---|
| what is drawn | one ring of 1.2 m round the loose puck | **two rings round the puck (1.8 m and 1.1 m: a target, larger than any ring under a skater) and a ring of 1.0 m under the opponent who is coming**, all three in his colour, breathing together |
| how long | the rule frame by frame | **24 sim frames at least once lit, and through a gap of the rule of 12 frames: one episode, one showing** (`JamCarrierMarks::StepShow`; cvar `jam.hud.ContestHold`, 0 = H30) |
| after his touch | gone (the rule needs a loose puck) | the pair round the puck goes (it would lie on his own ring and on the yellow of his take grace); **the ring under the one who is coming stays** for the rest of the showing: the hit lands eight frames after his touch |

`JamCarrierMarks.h` gains the showing (`FShowRule`, `FMarkShow`, `StepShow`): a pure state stepped by the sim's frame, the same in the HUD and in the
trace, so what the trace counts is what is on the screen. The rule itself (`Contested`, the Director's 12 m/s) is untouched.

The marks as shown against the trace (`Bar.Movement.CarryTrace`, 100 matches a cell; the H30 columns are the same build with
`JAM_MARK_RULE="minshow=1;linger=0;armclosing=8"`):

| The contested mark | H30, pro | Now, pro | H30, rookie | Now, rookie |
|---|---|---|---|---|
| showings a match | 20.3 | **13.2** | 16.1 | **11.2** |
| **median frames on screen** | 4 | **28** | 6 | **25** |
| showings of one frame / under 12 frames | 818 of 2,027 / 64.2 % | **0 / 0.1 %** | 553 of 1,611 / 66.0 % | **0 / 0 %** |
| precision: down inside half a second of a marked touch | 32.2 % | 30.4 % | 21.6 % | 20.1 % |
| ... of a touch with no mark | 5.3 % | 5.2 % | 1.5 % | 1.5 % |
| recall: of the hit-as-he-won-it knockdowns (439 at pro, 154 at rookie) | 64.5 % | 65.6 % | 72.1 % | 72.1 % |
| lit a third of a second or more before his touch (of the class) | 55.1 % | 56.5 % | 58.4 % | 58.4 % |

(The screen lane counted a median of 5 frames and 8 one-frame showings in a minute of window: the trace agrees: 4 frames, four showings in ten of one
frame. H30's 20 showings a match were 13 episodes, flickering.) A wider rule does not buy lead: `hsec=1.4` leaves recall at 65.6 %.

## 2. The stop line can be seen

| | Package H30 | Now |
|---|---|---|
| the lit line | 0.40 m deep, 9 px at least: **8 px by 295 px** at 1920 x 1080 | **1.0 m deep, 20 px at least: 22 px by 295 px at 1920 x 1080, 14 px by 197 px at 1280 x 720** (the same two stick lengths wide: the screen lane's `Jam.Hud.ClearEdges` holds the width, and the window asked for depth) |
| the chalk line | 0.16 m, 4 px, 60 % opacity; up only once the two close at 8 m/s | 0.45 m, 9 px, 90 %; **up from the moment he carries at an opponent ahead** (`FStopRule::ArmClosingMps` 2; to LIGHT it keeps the Director's 8 m/s) |
| where it lies | at the point 8 m from him towards the opponent | **across his path always: straight ahead of him at 8 m** (section 3; cvar `jam.hud.StopBarMode`, 0 = H30) |

| The stop line against the trace | H30, pro | Now, pro | H30, rookie | Now, rookie |
|---|---|---|---|---|
| chalk before the light, median frames | 0 | 7 | 2 | 15 |
| on the ice (chalk or lit), share of his carry frames | 28.3 % | 41.4 % | 28.4 % | 45.8 % |
| lit showings a match; lit frames at a time (median) | 12.3; 10 | the same | 12.0; 14 | the same |
| recall of "the stop was on" / precision (no brake) / down when he braked | 66.0 / 25.5 / 5.6 % | the same | 73.4 / 8.5 / 2.5 % | the same |

**What an eye learns from it in two carries**, on an approach from afar: a pale bar lies across my way from the moment I carry at one of them; when he
is about ten metres off it turns his colour, more than doubles in depth and breathes. First carry I skate on and am put down just past it; second carry I
know the bar turning is the last moment a stop works.

**What stays short, and why no rule of the mark lengthens it:**
- In matches the chalk precedes the light by under 12 frames in 56 % of the lit showings at pro (46 % at rookie): he mostly wins the puck with the
  opponent already inside the lit distance. There the line is right to come up lit.
- In drill 21 the chalk lasts 6 to 10 frames (armed at 13.2 m, lit at 11.7 m): the drill's defender stands 12 m off when the carry begins. A drill
  that starts him 18 m off would show chalk turn to colour for half a second (the drill's file is the track's; it was not opened here).
- The lit state itself lasts a median of 10 frames at pro: head-on at 15 m/s the window from 13 m to 7 m is 0.4 s, and most showings begin inside it.

## 3. The line off to one side: judged, and one rule

Staged off screen in free play (`a178b-free-m1`, `-m0`: the same deterministic match, stills two frames into each lit showing): at frame 3412 the
human has just won a draw and a defender closes from 9.4 m, **55 degrees off his travel**; at frame 2688 another from 10.8 m at 56 degrees.

- **By the numbers** the side cases are not rare and not harmless: the opponent is 40 degrees or more off his travel in 363 of 1,234 lit showings at
  pro (29.4 %; rookie 29.0 %), and he is down inside 1.5 s after 34.2 % of them when he does not brake (21.9 % inside 40 degrees; rookie 12.5 %
  against 7.0 %). A cone of 40 degrees (`cone=0.766`) takes the recall from 66.0 to 54.4 % and the precision from 25.5 to 21.2 %. So the cone stays 60.
- **By the picture**: `movement-9/side-h30-place-f3412.png` and `side-across-path-f3412.png` are the same frame with the line in
  H30's place and across his path. In H30's place the bar lies at the left edge of the screen, half off it, beside the own-net marker, across
  nobody's way: **it does not say "stop"; it reads as a stray strip**. Across his path it lies ahead of him where a stop line belongs.
- **The rule**: the line lies across his path always, straight ahead of him at the mark's distance (`FStopMark::AtPath`), whichever side the opponent
  comes from. It is about HIS motion (press against it), so it belongs on his path. Recall and precision are the cone's, unchanged (66.0 % / 25.5 %).

One thing the stills show that no rule here fixes: the line is an overlay, so a skater standing nearer than 8 m is painted over by it (frame 3412: the
opposing centre). Every ground mark of the HUD shares that; it needs the screen lane's figure boxes.

## 4. What the two marks are worth if a player obeys both (the instrument's switches, off by default)

`JAM_PLAYTESTER_STOPMARK=brake` (new; `JamPlaytester.cpp/.h`): when the line lights while he carries, the scripted human plays law 3's move as drill 21
plays it (the first move of `FJamCarryDrill`): back and to the open side until he is slower than 2 m/s, a cut of 60 degrees off his line to the side the
opponent is not on for 45 frames, then the script goes on; not again for 84 frames. `=stop` is the stop alone (24 frames, then straight on).
`JAM_PLAYTESTER_CONTEST=brake` is MOVEMENT-8's. Absent = the player as shipped.

Pro, the regular player, 100 matches (the trace, seeds 1 to 100):

| Arm | Wins of 100 | Knockdowns a match | "the stop was on" a match | "braked too late" a match | "as he won it" a match | He lies down |
|---|---|---|---|---|---|---|
| as shipped | 63 | 6.76 | 1.03 | 0.29 | 4.39 | 9.07 % |
| the contested ring obeyed | 55 | 4.73 | 1.03 | 0.20 | 2.25 | 5.91 % |
| **the stop line obeyed (stop and cut)** | **26** | 8.62 | 0.92 | 1.16 | 5.07 | 9.22 % |
| the stop alone | 29 | 9.43 | 1.06 | 1.47 | 5.26 | 9.28 % |
| **both obeyed** | **20** | 5.91 | 0.93 | 1.10 | 2.63 | 6.27 % |

Rookie (the novice): as shipped 90 wins, 2.85 knockdowns a match, 3.15 % of play down; the stop line obeyed 47 wins, 5.29, 3.33 %; both 38 wins,
5.15, 2.81 %. (Matches last longer when he wins less: his carries go from 16.6 to 20.2 a match at pro, so "a match" counts rise with them; per frame
of play he lies down as much as before.)

The cells (600 a cell, seeds 1 to 600, paired, the ladder's instrument; `baseC` against each arm):

| Arm | novice at rookie | regular at pro | good at all_star | His knockdowns a match (rookie / pro / all_star) |
|---|---|---|---|---|
| as shipped (`baseC`) | 86.5 | 53.2 | 30.0 | 3.21 / 6.86 / 8.48 |
| the contested ring obeyed (`contestC`) | 84.3 (**-2.2**) | 50.2 (**-3.0**) | 31.5 (**+1.5**) | 2.87 / 4.70 / 6.01 |
| **the stop line obeyed, stop and cut (`stopC`)** | 44.8 (**-41.7**) | 25.3 (**-27.9**) | 14.8 (**-15.2**) | 5.25 / 8.59 / 9.09 |
| both obeyed (`bothC`) | 44.7 (**-41.8**) | 20.3 (**-32.9**) | 15.8 (**-14.2**) | 5.00 / 6.43 / 6.30 |

Goals for and against a match, as shipped and with the stop line obeyed: 2.74:1.10 -> 1.84:2.04 (rookie), 2.13:2.00 -> 1.40:2.55 (pro),
1.59:2.45 -> 1.14:2.74 (all_star): he scores a third less and concedes a quarter to nearly twice as much.

**The stop line, obeyed, does not do what the brief hoped.** The class it is for goes from 1.03 to 0.92 knockdowns a match (the target was under
0.5), "braked too late" quadruples, and the wins halve: on 600 paired matches a cell, -41.7 points at rookie, -27.9 at pro, -15.2 at all_star, with
MORE knockdowns a match at every level. Three reasons, each measured:
1. **Three lit showings in four end without a fall anyway** (precision 25.5 % at pro, 8.5 % at rookie): a player who stops at every one pays a second
   and a half of attack twelve times a match for a fall he would have had three times.
2. **The stop that passes in the drill is against ONE defender on open ice.** In a match the others arrive while he stands: of the showings in which
   the instrument stopped and cut, he was down inside 1.5 s in 15.1 % (5.6 % for the stops the script makes of its own accord, which are turns away
   from pressure, not stops in front of it). **MOVEMENT-8's "where the stop followed the mark the fall came a fifth as often" was that correlation;
   this arm is the intervention, and it does not hold.**
3. A line that lights only for an opponent who himself skates at him (`FStopRule::DefTowardMps`, a measurement's number, 0 = off) is sharper and
   still costs: at 5 m/s 7.7 lit a match, recall 50.5 %, precision 29.1 %, the instrument obeying it 39 wins and 7.23 knockdowns a match; at 7 m/s
   5.7, 40.8 %, 30.5 %, 43 wins, 7.39.

**The contested ring is the mark that pays**: obeyed alone it takes 30 % of his knockdowns and a third of his time on the ice for 8 wins in 100 at pro
(MOVEMENT-8 read the same on the old rule).

## 5. Proof

- **Tests in the gate** (the full `Jam` filter on the final tree: **268 pass, 0 fail**; 267 on the main it stands on): `Jam.Core.Movement.MarkShow` (new: one frame of the rule is 24 frames on
  screen; a flicker is one showing; a long yes stands 12 frames past its end; a restart; `minshow 1, linger 0` is the rule frame by frame),
  `Jam.Core.Movement.Marks` (two new lines: the pale line for an opponent backing off at 5 m/s, not lit; an opponent 45 degrees off: the angle, and the
  line's place straight ahead of him), `Jam.Hud.Marks` (the sizes; the line's 20 and 9 px floors through `StopBarLayout`).
- **Hash identity**: the default match on build 4 (arm `n0`) against the tree as it came (arm `base`, build 1 = main 3fa9c2e6), 600
  matches a cell, the default instrument: **the same hash, frame count and score in 600 of 600 matches of each of the three cells**
  (`node tools/perf/results/a178/same.mjs base n0`). Builds 5 and 6 changed the HUD only (the held ring once the puck is his; the line's width); the
  gate's process on build 6 also played the trace's 100 pro matches: the `base` arm's hash for hash, 100 of 100. The base was built before the first
  edit (its DLLs are older than every edited file). The instrument's switches are off unless the environment sets them.
- **Off screen** (`tools/qa/screen/u1-stills.ps1`, the editor binary off screen on port 4778; the HUD takes its own stills N frames into a showing,
  now of the chalk state too): the drill at 1920 x 1080 and at 1280 x 720, free play at both. Landed under `docs/experiments/movement-9/`: `line-lit-1080-f520.png`, `line-chalk-1080-f510.png`, `line-lit-720-f520.png` (drill 21, the final build), `contested-1080-f1018.png` (after a draw he won: the
  pair round the puck, the ring under the centre who comes), `contested-1080-f2320.png` (an opponent on the loose puck), and the two side stills of
  section 3.
- The Shipping target compiles (`tools/run/build.ps1 -Project Game -Target Game -Config Shipping`, on the final tree).
- **Not proven**: that a person reads either mark; the window lane's rows on the next package are the proof (the ring on a still; the lit line's
  height in pixels; chalk before colour on a long approach in free play, not in the drill).

## 6. For the operator's sheet

> **A double ring in the other team's colour round a loose puck, with a ring under one of them, means he will be there when you are: take it and you
> are hit as you touch it; brake short of it, or let him have it and hit him. It now stays up long enough to see.**
>
> **The bar across your path ahead marks where a stop still works against the one coming at you: it turns from chalk to his colour and thickens when
> that moment is on. Treat it as an offer, not an order: stop and cut to the side when he is truly charging you; the scripted player who stopped at
> every lit bar fell no less and won half as often.**

## 7. Findings outside the family

- The sheet and the drill's line ("STOP WHEN HE IS FIVE STICK LENGTHS AWAY. CUT. GO.") teach the stop as a rule; section 4 says it is a move for one
  situation. `docs/PLAY-ME.md` (the Director's) should say "an offer".
- Drill 21 (`JamCarryDrill.h`, `DefenderM` 12): the defender stands 12 m off as the carry begins, inside the line's lit distance at once: the drill
  cannot show chalk turn to colour. 18 m would.
- The brain (track Q2): a carrier who stops and cuts in front of one presser is flattened by the next (15.1 % down inside 1.5 s). The second man's
  arrival is what makes law 3's move a drill move at pro.
- The screen lane: the stop line (and every ground mark) is painted over a skater who stands nearer than the mark; the keep-clear boxes hold the
  human and the downed figures only.

## 8. Next evolutions (ranked)

1. **The line says when, or it leaves the match.** Lit 12 times a match, right once in four at pro and once in twelve at rookie, and costing half
   the wins of a player who obeys it, the line as it lights today is a drill's mark. Either a rule that lights it only where stopping is the better
   move (from the trace: the lit showings that ended on the ice against those that did not: the second opponent's distance, a pass open, the room
   to cut), or the line shown in drill 21 and the first match only. Proof: precision 50 % or more at 3 showings a match or more, and the instrument
   obeying it inside 4 points of every cell with "the stop was on" under 0.5 a match; or, for the other road, the operator's word.
2. **The contested ring's other third, and the better answer.** Recall is 65.6 % at pro; and "brake" costs 3 points where "hit him instead" (the
   one who arrives second may check) may cost none: `JAM_PLAYTESTER_CONTEST=hit`. Proof: recall 80 % or more at a precision of 28 % or more; the
   hit arm's three cells against -2.2 / -3.0 / +1.5 with knockdowns no higher than the brake arm's.
3. **The ground marks in the world, under the figures** (section 3's overlay; the Director's question, answered in `status/A178.md`): one flat
   quad per mark with an unlit material, placed from the same rules, with the screen lane. Proof: in 20 captured showings no frame with a mark's
   pixels over a standing figure (the down probe's method), and the opponents' colour inside a tolerance on clay and at night.
Carried over, still first for the puck itself: the pickup settles for every skater, as a key, with the curve's re-set (MOVEMENT-7: the puck on
nobody's blade 49.7 -> 43.1 % at pro; +5.8 / -9.0 / -5.0).
