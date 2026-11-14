# REVIEW-3: what the operator meets at 10:00, said before he meets it (lane A169, track F0, Sat 14 Nov 05:40-)

The critic's read of main `c5dc15e5`, the commit package H28 was cut from, against `docs/VISION.md` and against `docs/design/REVIEW-2.md` (01:30, main `2a42d88`)
on the same seeds with the same scripts. No source, cartridge or test was changed. Status with timestamps: `status/A169.md`. Scripts and tables:
`docs/design/review-3/` (bulk logs in `review-3/raw/`, which does not travel).

**What was run** (this worktree's own editor build of c5dc15e5, headless, `-nullrhi -Multiprocess`, three processes at most): **11,708 matches** of the
playtester (`novice` = a first-time player, `regular` = someone who has played a hockey game, `good` = the target of all_star); 1,500 matches inside
`Bar.Movement.CarryTrace` (four cells of 100 and eleven arms of the marks' rules); 109 automation tests, all Success, with the stop's sweep at 29 distances and
levels, 200 seeds a row. REVIEW-2's three commands again (the 3 x 3 grid, 40 seeds a cell; the eight takes, 60 seeds, each at its own level; the three
traced cells, 12 seeds), the large rink, the twelve modes, the 3 x 3 at 200 seeds on both instruments, five rows of the modes lane's table on fresh seeds,
and the measured arms of rules and numbers (the marks' rules, the masher, the long shot at rookie, the carrier's speed on big ice; none shipped).
**The instrument**: unless a row says otherwise, the press the ladder was set with and REVIEW-2 was measured with (`JAM_PLAYTESTER_CHECK=closing`, R-0060);
the modes, the masher and the carry trace use the shipped press, as their lanes did.
**No window**: the cursor is the release chain's and lane A155's. The two marks on the ice and the flattened puck marker are judged from the lanes' own
off-screen stills, their log lines and the rules' numbers, never from a frame I saw move. A number not measured here names its document.
**H29**: at 06:02 the Director moved the stop mark's far edge (`FStopRule::LitFarM` 11 to 9.5, from section 0's row 5) and cut H29 from that commit; H28 is
withdrawn. Everything below is c5dc15e5; where the mark matters, H29's number is my `JAM_MARK_RULE far=9.5` arm and is named.

## 0. `docs/PLAY-ME.md`, line by line: the sentences that were false of this build

Line numbers are the sheet's at c5dc15e5 (98 lines). Rows 1 to 7, 11 to 14 and 16 were corrected in the sheet on main at 06:02 (row 5 in the build: H29); **what is still to change
in the corrected sheet is the list under the table**. The sentences that stand, with their numbers, are section 6.

| # | Line | The sheet said | What is true of this build | Measurement |
|---|---|---|---|---|
| 1 | 82 | "The loss card says only REMATCH?" | **False.** It says **BEATEN**; a level score at the horn says NO WINNER; the key line reads `R  REMATCH      ESC  MENU` | `HUDWidget.cpp:428`, `:437`, `HUDWidget.h:341` (lane A144) |
| 2 | 70 | "nothing on screen warns you that an opponent arrives with it. A mark for that, and the stop's distance drawn on the ice, are being built now" | **Stale.** Both marks are in the build and on (`jam.hud.Marks` 1); lines 18 and 68 of the same sheet said so | `HUDMarks.cpp:26`; `Jam.Core.Movement.Marks`, `Jam.Hud.Marks` green here |
| 3 | 55 | "one of the five-a-side ones: Arsenal ... or the Marathon" | **The Marathon is three a side** (its own card: "Three on three on endless ice"). Five a side: Arsenal, Bruisers, Glass House, Pond, Shotgun, Sudden Death | six skaters in `hockey.mode.marathon.cartridge.json` |
| 4 | 39 | press S "when the chalk ring ahead of you turns his colour (he is about five stick lengths away)" | **The parenthesis was wrong.** The ring **lies** five stick lengths (8 m) ahead of him. On H28 it turned the defender's colour when he was 11 m away plus a quarter second of closing (about 15 m in a head-on rush) | `JamCarrierMarks.h:75-83`; `Jam.Core.Movement.Marks` (lit at 13.5 m when closing at 13 m/s) |
| 5 | 17, 39 | the ring "turns the defender's colour when the stop is on"; "at pro it should work nearly every time from there" | **True at rookie, close at pro, false at all-star on H28.** The stop begun at 11 / 12 / 13.5 m passes 100 / 100 / 100 % at rookie, **83.5 / 56.5 / 52 % at pro, 53 / 28.5 / 19.5 % at all-star**; from 12 and 13.5 m he ends on the ice in 40 and 49 of 200 tries at pro, 52 and 65 at all-star. Safe at every level: **8 to 9.5 m** (100 / 100, 100 / 97, 100 / 92). **Fixed in H29** (the far edge at 9.5 m) | the sweep, 200 seeds a row (section 4, law 3) |
| 6 | 36 | "From 3 to 6 m it gets you flattened" | **Pro only** (0 / 5.5 / 7.5 / 7.5 % from 3 / 4 / 5 / 6 m). At **rookie**, where he starts, the stop begun at 3 m passes 95.5 %, at 4 m 74.5 %, at 6 m 70.5 %; only 5 m is bad (28.5 %, down 132 of 200). All-star: 41.5 % from 3 m, 4.5 % from 5 m, 55.5 % from 6 m | the same sweep |
| 7 | 17 | a ring round a loose puck: "take it and you are hit" | **One time in four at pro, one in six at rookie.** Of the ringed pucks he takes he is down inside half a second after **26.8 % (pro), 15.6 % (rookie), 21.5 % (all-star)**; the ring shows 19 times a match (29 at all-star). The row's two facts are right at pro (64.9 % of his knockdowns as he wins the puck; 15.2 % with the stop on and unused) | `Bar.Movement.CarryTrace`, 100 matches a level |
| 8 | 81 | "Sniper is hard on a newcomer (one in ten)" | **Low.** The novice wins Sniper **26 %** (100 fresh seeds) and 20 % (seeds 1-40). His one-in-ten modes are **Shotgun** (10 % of 100 fresh seeds; the first mode the sheet sends him to), Arsenal and the Marathon (10 % of 40) | `data/modes-fresh-novice.md`, `modes-novice.md` |
| 9 | 81 | "A good player wins Cage, Fog of War and Power Play nine times in ten" | Fog of War 92 %, Power Play 91 %: true. **Cage 83 %** | `data/modes-fresh-good.md` (100 fresh seeds) |
| 10 | 24 | "the Marathon gives you the puck" | True for a regular player (a half-minute without the puck in 14 % of 200 matches; 71 % before). **Not for a newcomer: 27 of his 40 Marathon matches hold such a stretch** | `data/modes-fresh-regular.md`, `modes-novice.md` |
| 11 | 94 | question 3: "the cooldown (mashing stops paying; costs the ladder under a point)"; the lunge "about three points at all-star, paid back by the curve" | **At 36 frames mashing still pays**: 9.84 hits against 4.57, 58.2 % against 55.0 %. It stops at 60 frames (55.3 against 55.7). And the curve has **no number that takes all-star down** (CURVE-9: five tried, flat or the wrong way) | CONTACT-7 sections 2 and 3; CURVE-9 section 2 |
| 12 | 48 | step 6: "The season's take 3 and take 4 (the ramp was re-set in the night)" | Takes 3 and 4 are both rookie now and **read the same**: novice 87 / 83 %, regular 100 / 98 %. The steps a person can feel: take 4 to take 5 (novice 83 to 63) and the finale (novice 62 to 7, regular 90 to 28, good 95 to 52) | `data/season.cells.jsonl`, 60 seeds a take |
| 13 | 47 | step 5: "Pro, one match; all-star, one match. Three different games?" | For one and the same player the three levels read: a newcomer **89 / 16 / 2 %**, a regular player **100 / 53 / 14 %**, a good player **100 / 76 / 28 %**. A player who has played a hockey game wins his first PLAY NOW (rookie) in 1.8 minutes, 3.0 : 0.4, every time | `data/grid200.cells.jsonl`, 200 seeds a cell |
| 14 | 41 | "`jam.ctrl.Drill 21` in a Development build stages it" | True as written; H28 was a Shipping package: name the Development one | the sheet's own line 3 |
| 15 | 37 | "Mashing RMB beats timing it today (about 20 hits a match against 4.6, and four points more wins at pro)" | The hits stand: **25.9 / 20.9 / 23.5 against 4.2 / 4.5 / 4.3** a match (rookie / pro / all-star). The four points at pro are CONTACT-7's 600 seeds; my 200 read 55 against 59.5 at pro and **40 against 29.5 at all-star** | `data/mash.cells.jsonl`, `grid200-shipped.cells.jsonl` |
| 16 | 62 | Controls: "S against your motion brake" | True, and it left out that **Ctrl brakes too** (the game's own prompt says "or hold Ctrl"); mouse 4 lifts, R restarts | `InputMapping.cpp:99`, `:111`, `:103` |

**Still to change in the corrected sheet (main, 106 lines, read at 06:03; `status/A169.md`, PLAY-ME VERDICT 2):**

1. **Line 49, "when he reaches the ring it is too late": false, and the wording was mine** (my partial verdict's row 4). The ring lies 8 m from him, and a
   stop begun with the defender **on the ring is the best there is: 200 of 200 at rookie, pro and all-star**. Too late is past it: a metre past (7 m)
   66 % at pro, two metres past (6 m) 7.5 %. Write: "press before he is past the ring".
2. **Line 40-41, "the window is 7 to 9.5 m"**: at pro it is **8 to 9.5 m** (7 m: 66 %, 43 of 200 on the ice).
3. **Line 7, H28 "lost the feet half the time"**: it **failed** half the time (56.5 and 52 % from 12 and 13.5 m); he lost his feet in one try of four or
   five (40 and 49 of 200); the rest were lifts, touches and time-outs.
4. **Line 86**: rows 8 and 9 above (Sniper 26 %, Shotgun the hard one; Cage 83 %) are not yet in the sheet. **Line 60**: row 10 (the newcomer's Marathon).
5. **Lines 3 and 8**: at 06:03 main's `status/A.md` ended its package lines at H28 and `Game/Packaged` held no `Hokej_Hokej_H29` or `Accept_H29`: true
   once H29 is cut.
6. **Line 48-50, with H29's mark**: "six to eight stick lengths away" holds up to about 13 m/s of closing (9.5 m plus a quarter second: 11.5 to 12.75 m);
   in a head-on rush at 16 to 20 m/s it lights at 13.5 to 14.5 m. "Nearly every time from there" holds **for a hand that takes the quarter second** (the
   stop then begins at 9.5 m: 97 % at pro, 92 % at all-star); a press on the very frame it lights in such a rush begins at 13.5 m: 52 % and 19.5 %. One
   clause: "count a beat, then stop".

## 1. The ten things, ranked by what the operator and a judge feel in five minutes

Each: what he feels; the number; the owner; the smallest change before Sunday 22:00 with the number that proves it; and **whether it can be fixed without
moving the ladder** (the Director can cut a package for a presentation fault until about 09:00; the sim is frozen for the operator's play).

### 1. "Orange rings keep appearing and I do not know which one wants what." (new tonight; presentation; no ladder)
- **Number.** At rookie, where every first match is played, the contested ring shows **18.6 times a match** and the stop mark lights **15.2 times** (14.4
  on H29): an orange ring every six seconds of play, for **2.85 knockdowns** a match. The contested ring is followed by the fall it warns of in **15.6 %**
  of the ringed pucks he takes (26.8 % at pro, 21.5 % at all-star); a lit stop mark he does not heed is followed by a fall in **8.2 %** with H29's rule (24.7 % at pro,
  33.6 % at all-star). The rules were set on pro's trace; at rookie five rings in six are a false alarm.
- **From the stills (not a window).** The two marks are **the same thing to the eye**: an orange ring on the ice of 1.2 m (round the puck) and 1.1 m (the lit
  stop mark), both breathing (`movement-8/contested-f2320.png`, `stop-lit-f1040.png`), and they ask for opposite acts: do not take it / brake now. Only
  the puck inside tells them apart. In `contested-f1017.png` the ring is round **the puck he has just won at the draw**: the first ring of every match
  whose draw he wins says "theirs" about his own puck (lane A166's own finding 2). In `stop-lit-f1040.png` the lit mark lies at the right edge of the
  frame, the defender it speaks of is out of the picture, two other opponents stand within 2 m of him, and the faceoff card covers the one-time brake
  prompt. No word on the screen says what either ring means: the sheet does, and a judge does not read the sheet.
- **Owner.** F2 (the rules, `JamCarrierMarks.h`, `HUDMarks.cpp`) with U1 (the shape).
- **Smallest change, in order of size; all presentation, the hash unchanged.** (a) The lit stop mark is not a ring: a bar or a chevron across his path at
  the same place. Proof: a still in which the two cannot be confused. (b) No contested ring inside the 60 frames after a draw. Proof: the carry trace's
  showings a match with and without. (c) One number for every level, swept without a
  build on H29's far edge (`JAM_MARK_RULE`, 100 matches a level; `data/marks-levels.txt`; rookie / pro / all-star):

  | the contested ring asks for (`FContestRule::ClosingMps`), the stop mark for (`FStopRule::ClosingMps`) | contested rings a match | followed by the fall | finds, of the falls it is for | stop marks lit a match | finds |
  |---|---|---|---|---|---|
  | 8 and 3 m/s (H29) | 18.6 / 19.3 / 29.4 | 15.6 / 26.8 / 21.5 % | 79.9 / 72.0 / 80.2 % | 14.4 / 13.8 / 13.3 | 81.2 / 73.8 / 77.4 % |
  | **12 and 8** | **11.4 / 13.4 / 22.6** | 21.6 / 32.2 / 25.2 % | 72.1 / 64.5 / 73.9 % | 12.0 / 12.3 / 12.2 | 73.4 / 66.0 / 74.5 % |
  | 14 and 10 | 8.3 / 10.7 / 18.5 | 26.5 / 35.7 / 28.3 % | 68.8 / 59.2 / 71.5 % | 10.0 / 11.1 / 11.5 | 68.8 / 61.2 / 73.6 % |

  12 and 8 take a third of the rings off every level for six to eight points of recall; 14 and 10 give rookie pro's precision and cost pro 13 points of
  recall. Read by level (the HUD would need the level: the match card has it), rookie could take 14 and pro stay at 8. No number here makes the stop mark
  matter at rookie: an unheeded one is followed by a fall in 8 to 9 % whatever the threshold, because at rookie almost any stop works. (d) Done in H29: the far edge at 9.5 m (recall 76.7 to 73.8 % at pro, 78.4 to 77.4 at all-star,
  82.8 to 81.2 at rookie; lit frames down a fifth).

### 2. "I touched the puck and I was on the ice." (the largest number in the review; any cure moves the ladder)
- **Number.** He lies down **9.07 % of play at pro** (REVIEW-2: 7.8; REVIEW-1: 4.6), **11.4 % at all-star** (10.0), 3.15 % at rookie (3.7). At pro 64.9 %
  of his 6.76 knockdowns a match land within half a second of his first touch of the puck, at 16.1 m/s of closing, for 131 frames (all-star 59.1 %, rookie
  54.0 %); 15.2 % with the stop on and unused (22.5 % at rookie, 24.6 % at all-star); 11.4 % with no room; 4.3 % a stop begun too late. Law 1 is kept to
  the letter: head-on at full speed is the biggest hit, and it is the hit he gets as his reward for winning a race.
- **Owner.** F1 (the hit) with Q2 (the presser that charges the first toucher); C0 pays.
- **Without the ladder?** Only by teaching: item 1's ring, obeyed, takes it from 9.07 to 5.65 % and costs 3.7 points at pro (MOVEMENT-8). **Smallest rule**
  (unbuilt; the contact track's, proposed by MOVEMENT-7 and 8): a hit on a skater inside the first 15 frames of his carry is capped at a stumble. Proof:
  knockdowns "as he won it" 4.39 a match to under 2.5; under 6.5 % of play at pro; the three cells re-read (it favours the human at pro, which is 4.4 over).

### 3. "The puck does not stick, and nobody has it." (REVIEW-1's first finding; a third review without the bar)
- **Number.** The puck is on nobody's blade **61.8 / 55.5 / 56.7 %** of play in the traces (60.3 / 55.0 / 59.9 at REVIEW-2; the bar is 45), 56.5 / 49.7 /
  55.0 % by the carry trace's count. His carry: median **1.25 / 0.78 / 0.78 s** (1.22 / 0.90 / 0.75; the bar 1.5 s at pro). New: **at rookie 16.5 % of a
  newcomer's carries (245 of 1,484) slide off his own blade with nobody near** (pro 8.5 %) and 10.2 % go to an opponent's bare touch: a quarter of his
  carries end with no press by anyone. The sheet's "you keep the puck until somebody presses for it" is the rule; this is what the hand feels.
- **Owner.** F2 with C0.
- **Without the ladder?** No. **Smallest change**: the pickup that settles, for every skater (`FJamStickConfig::PickupSettleFrames` 6, `bPickupSettleBots`;
  built, at 0): nobody's blade 49.7 to 43.2 % at pro, the carries that leave the blade 142 to 45 (MOVEMENT-7); the cells **+5.8 / -9.0 / -5.0**. It is the
  one waiting rule that takes pro and all-star **down**, where they sit 4.4 and 2.8 over with no other lever (section 7).

### 4. "Hammering the right button works best." (a judge mashes; a sim rule, half a point)
- **Number.** RMB every 12 frames lands **25.9 / 20.9 / 23.5** checks a match against 4.2 / 4.5 / 4.3 timed, on this commit (200 seeds a cell); it wins
  94 / 55 / 40 % against 90 / 59.5 / 29.5 (CONTACT-7's 600 seeds at pro: 59.3 against 54.7). A tackle is "an event, not a rhythm" (R-0049); mashed it is a rhythm.
- **Owner.** F1.
- **Without the ladder?** Nearly: **`check_cooldown_frames` 60 with the restart on a too-early press** (CONTACT-8 on main, not in my tree, not measured
  by me): the masher lands 2.93 a match, under the timed hand; cells +0.5 / 0.0 / -0.5. A sim rule: the first thing to turn on after the play (section 7).

### 5. "Rookie was over in two minutes. Pro flattened me." (the three levels are for three players; one person climbs them; words only)
- **Number.** One and the same player on the three levels: a newcomer **89 / 16 / 2 %**, a regular player **100 / 53 / 14 %**, a good player
  **100 / 76 / 28 %**. A fresh profile starts at rookie: a judge who has played any hockey game wins it 3.0 : 0.4 in **1.8 minutes, 200 times in 200**. A
  newcomer who then lights PRO: 16 %, no shot at all in 38 of 200 matches, a half-minute without the puck in 71. The ladder is right for its three target
  players (89 / 53 / 28); nothing on the menu says which of them a level is for, and nothing after a match says where to go next.
- **Owner.** U2 (the menu and the end card). The sheet now says it for the operator.
- **Without the ladder?** Yes. **Smallest change**: one line under the level switch ("FIRST TIME" / "PLAYED BEFORE" / "GOOD AT IT"), and the win card
  names the next step: TRY PRO after a rookie win by two goals or more, PLAY THE SEASON otherwise (the season is the bridge: a newcomer wins takes 5 to 7
  63 / 62 / 62 %). Proof: a still; `Jam.Menu.FitsTheScreen` green.

### 6. "They score from the other side of the red line." (at every level, rookie included; free at rookie)
- **Number.** Traced goals against released 25 m or more from the net: **4 of 16 at rookie, 9 of 21 at pro, 4 of 27 at all-star** (2 of 13, 7 of 21, 10 of
  40 at REVIEW-2). The opponents' fast releases leave from a median of **37.1 / 37.6 / 40.9 m** (28.8 / 35.7 / 41.8), 42 / 44 / 50 % of them from their own
  half; at rookie 6 % of those are on the net (clears and shots are one number in the trace).
- **Owner.** C0 with Q2.
- **Without the ladder?** At rookie, yes: `opp.shoot_extra_m` 25 to 13 **for rookie alone** reads 89 to **90 %** (200 seeds), the opponents' shots 13.0 to
  10.1 a match, goals against 1.1 to 0.9, traced goals from 25 m or more 4 of 16 to 1 of 12. A profile number, so after the play. At pro and all-star it
  is 10 and 23 points (REVIEW-2): the curve's next pass.

### 7. "A man is down and somebody stands on him and shoves him along the ice." (the key feature's picture; four points at all-star)
- **Number.** Not measured here: CONTACT-7 reads an opponent's root within 0.9 m of a downed skater for more than 6 frames **7.8 times a pro match**
  (longest 128 frames) and a downed man carried 7.35 m in the staged case; SCREEN-2's still of seed 123 shows it under the cleaned HUD.
- **Owner.** F1.
- **Without the ladder?** No: `contact.downed_radius_m` 0.6 costs -1.0 / +1.0 / **+4.0** (CONTACT-7 at 1,200 seeds; +2.9 at 2,000 in CONTACT-8), at the
  level that has no lever down. Section 7 says how it is paid.

### 8. "Space works one time in ten, and the first screen never told me it exists." (the strip is presentation; the rate is not)
- **Number.** Presses that take the puck: **8.6 / 9.8 / 7.5 %** (2.6 / 3.6 / 3.7 at REVIEW-2, 200 seeds; the bar was 15: met only on presses at a carrier
  out of his grace and unshielded, 15.4 %, MOVEMENT-6). His own-poke stumbles are gone (section 2). The hint strip of the first faceoff names seven verbs
  and neither Space nor F: `SKATE WASD  BRAKE ...  AIM MOUSE  SHOOT HOLD LMB  PASS Q  CHECK RMB  SPRINT SHIFT` (`HUDWidget.h:58`).
- **Owner.** U1 (the strip); F2 (the rate).
- **Without the ladder?** The strip, yes: `POKE SPACE` in it. Proof: the HUD's fit test; a still. The rate: half the misses are the carrier's grace (49.8 %,
  MOVEMENT-6), now shown as the yellow ring; a hand that waits for it is MOVEMENT-6's read arm (+1.8 at pro).

### 9. "On the big sheet I skate for half a minute and never touch it." (unchanged at pro; one cartridge key measured)
- **Number.** `hockey.arena.large`, regular at pro: a half-minute without the puck in **125 of 200 matches (62.5 %)**; 21 of 40 on REVIEW-2's seeds (26
  then); the bar of R-0058 is 25 %. A newcomer at rookie there: 126 of 200. Take 7 (140 x 70): 42 of 60; take 2: 20 of 60 (37: its clock was cut).
- **Owner.** Q2; the Director for content.
- **Without the ladder?** The six ladder cells on the regular rink, yes. `skater.carry_speed_mult` 0.8 in the large cut (it plays 0.92; the Marathon's
  lever, MODES-6): regular at pro **125 to 50 of 200 (25 %: the bar)**, win 47 to 61 (the large rink's red skill pays 14 points); novice at rookie **126 to
  78 of 200**, win 85 to 86 (free). Proof on takes 2 and 7: the same two numbers, not measured.

### 10. "The season was seven walkovers and a wall." (outside the five minutes; content)
- **Number.** By take: a regular player **98 / 100 / 100 / 98 / 100 / 100 / 90 / 28 %**; a newcomer 77 / 83 / 87 / 83 / 63 / 62 / 62 / **7**; a good player
  98 / 97 / 98 / 100 / 100 / 100 / 95 / 52. R-0057's ramp is met for the newcomer to take 7 and then falls 55 points; for anyone else there is no step before the finale.
- **Owner.** C0 with the Director (one `ai.red_skill` a take: 0.37 / 0.45 / 0.9 today).
- **Without the ladder?** Yes (takes only). **Smallest change**: takes 6 and 7 one step harder each, so the newcomer reads about 63 / 50 / 35 and a
  regular player meets a take he can lose before the last. Proof: 200 seeds a take and class.

**Seen and not ranked.** The faceoff card covers the brake prompt in a lane's still (item 1). Cage and Bruisers have drifted towards the regular game
(section 5c). The camera in corners is trimmed, not cured (SCREEN-2: 84 to 71 frames steeper than 60 degrees). Passes received a minute are level:
0.81 / 1.37 / 1.62 to 0.83 / 1.32 / 1.57 (200 seeds); still his two seconds after a mate's pass 19 to 28 % at pro (bar 30).

## 2. Five things that are now good and must not be broken

1. **The level reaches the match, and the ladder is where it was set, on two instruments.** PLAY NOW played pro whatever was lit at REVIEW-2; now the
   lit level travels on the URL to the brain, a fresh profile is rookie, the card says it. 200 seeds a cell: **89 / 53 / 28** with the ladder's press
   (CURVE-9 at 2,000: 85.8 / 52.4 / 28.8), **90 / 59.5 / 29.5** with the shipped one.
2. **One stop distance at every level.** The stop begun at 8 m: **200 of 200 at rookie, at pro and at all-star** (all-star was 0 of 200 at REVIEW-2); a
   straight charge 0 of 200 at each. In H29 the mark lights inside that window.
3. **Space no longer trips him, and takes the puck three times as often.** His stumbles a match 6.3 / 6.3 / 4.2 to **0.2 / 0.8 / 0.8**; takes 2.6 / 3.6 /
   3.7 % to 8.6 / 9.8 / 7.5 %.
4. **The whistle is where the mode is about it, and the season is a school.** No foul in 8,600 matches of the regular cut, the large rink and the eight
   takes; in 3,060 mode matches only Glass House's (28 of 40 for a regular player). The newcomer by take: 83 / 83 / 90 / 20 / 12 / 8 / 15 to **77 / 83 / 87 / 83 / 63 / 62 / 62**;
   take 2's waiting 37 to 20 of 60; take 4's matches without a shot by him 17 to 4.
5. **Law 1 in real matches, and a fall that is left alone.** Frames down follow the speed brought (pro, 123 knockdowns: under 6 m/s 70 to 77 frames, 6 to 8:
   81, 8 to 10: 102, 10 and up: 144). Nothing is drawn on the downed player (SCREEN-2: 0 of 2,432 frames, the lane's count), and the black post at the
   faceoff is a flat disc in the lane's before-and-after still.

## 3. Before and after: REVIEW-2's numbers on the same seeds

Novice at rookie / regular at pro / good at all_star unless said. "Moved by" is read from the rulings and the lanes' documents, not measured by arm.

| Number | REVIEW-2 (2a42d88) | Now (c5dc15e5) | Bar | Moved by |
|---|---|---|---|---|
| **what PLAY NOW plays** | pro, whatever is lit: a newcomer 18 % | the lit level; a fresh profile rookie: **89 %** | the level reaches the match: **met** | A144 (R-0059) |
| the three target cells, 200 seeds | 86 / 51 / 23 | **89 / 53 / 28** | 85 / 48 / 26 (CURVE-9 at 2,000: 85.8 / 52.4 / 28.8: pro 4.4 over, as ruled) | A154, A162 (CURVE-8, 9) |
| **pokes that take the puck**, 200 seeds | 2.6 / 3.6 / 3.7 % | **8.6 / 9.8 / 7.5 %** | 15 %: **no** (15.4 % on an open carrier) | A152 |
| **his own-poke stumbles** a match | 6.3 / 6.3 / 4.2 | **0.2 / 0.8 / 0.8** | under 1: **met** | A152 (a miss locks the stick) |
| **fouls in the modes** (matches of 40 with one) | Arsenal 26, Fog of War 21, Outnumbered 19, Glass House 18, Shotgun 15, Sniper 13, Sudden Death 2 | **Glass House 28; the other eleven 0** | 0 outside Glass House: **met** | A149 (R-0059) |
| **the first minute** (PLAY NOW's cut at rookie, novice, 200 seeds): first shot; first goal by him; a goal inside two minutes; no shot at all | 35.4 s; 57.0 s; 149; 1 | **42.0 s; 63.1 s; 152; 4** | 45 s; 40 of 60: met, slower | not traced |
| take 1, the novice: first shot; a goal inside two minutes; won | 35.3 s; 42 of 60; 83 % | 41.4 s; 45 of 60; 77 % | 45 s; 40: met | |
| **he lies down**, share of play (12 traced) | 3.7 / 7.8 / 10.0 % | **3.2 / 8.1 / 9.3 %** (100 matches: 3.15 / 9.07 / 11.42) | under 6 % at pro: **no** | |
| ... **by cause** (100 matches, share of knockdowns) | not split (90 % with the puck at pro) | as he won it **54.0 / 64.9 / 59.1**; the stop on and unused 22.5 / 15.2 / 24.6; no room 16.8 / 11.4 / 10.9; a late stop 4.2 / 4.3 / 3.2; without the puck 2.1 / 3.1 / 0.7 | | A163's trace |
| knockdowns of the human a minute, 200 seeds | 1.08 / 2.52 / 3.38 | 1.02 / 2.59 / 3.23 | | |
| **the carry's length**, median | 1.22 / 0.90 / 0.75 s | **1.25 / 0.78 / 0.78 s** | 1.5 s at pro: **no** | |
| **the puck on nobody's blade** | 60.3 / 55.0 / 59.9 % | **61.8 / 55.5 / 56.7 %** | 45 %: **no** | |
| **where the opponents release from**, median; from their own half | 28.8 / 35.7 / 41.8 m; 37 / 41 / 52 % | **37.1 / 37.6 / 40.9 m**; 42 / 44 / 50 % | | not traced |
| traced goals against from 25 m or more | 2 of 13, 7 of 21, 10 of 40 | 4 of 16, **9 of 21**, 4 of 27 | under 10 %: no | |
| the stop begun at 8 m | 100 / 100 / **0** % | **100 / 100 / 100 %** | 80 % at every level: **met** | A153 (R-0058) |
| defenders beaten by a stop, a match (12 traced) | 0.42 / 1.25 / 1.67 | 0.67 / 0.67 / 0.67 | | the script's hand |
| passes received a minute, 200 seeds | 0.81 / 1.37 / 1.62 | 0.83 / 1.32 / 1.57 | not under: level | |
| still his two seconds after a mate's pass | 35 / 19 / 8 % | 39 / **28** / 14 % | 30 % at pro: nearly | A153 |
| the season, the novice by take | 83 / 83 / 90 / 20 / 12 / 8 / 15 / 3 | **77 / 83 / 87 / 83 / 63 / 62 / 62 / 7** | no more than 20 points a take to take 7: **met**; the finale is the cliff | A154 (R-0057) |
| take 2: a half-minute without the puck (novice) | 37 of 60 | **20 of 60** | | A154 |
| the large rink at pro: the same (regular) | 26 of 40 | 21 of 40; **125 of 200** | 25 %: **no** | A134 (teams under five) |
| bot reverse brakes a match at pro; bot carrier sprint frames | 270; 787 | 252; 668 | under 500; over 120: met | |

Tables: `review-3/data/grid.md`, `large.md`, `modes-*.md`; every read-out a cell in `*.cells.jsonl`; the laws and the carries `*.laws.json`,
`*.carry.json`, side by side in `lawsum.txt`; the carry trace `carry-*.txt`; the sweep `sweep-rows.txt`; the marks' arms `marks-sweep.txt`.

## 4. The four laws, one by one, in real matches

**Law 1, a hit obeys momentum: kept.** Drills green (`Jam.Core.Contact`, 109 of 109 in the run). Matches (pro, 12 traced, 123 knockdowns; `laws.mjs`):

| closing speed | under 2 m/s | 2 to 4 | 4 to 6 | 6 to 8 | 8 to 10 | 10 and up |
|---|---|---|---|---|---|---|
| knockdowns | 2 | 2 | 8 | 12 | 16 | 83 |
| frames down, median | 77 | 70 | 73 | 81 | 102 | 144 |

Head-on with both moving: 55, at 20.6 m/s, 146 frames. The same way: 13, at 6.0 m/s, 77 frames. The human is the victim in 74 (closing 16.8 m/s, 140
frames, 88 % with the puck), the attacker in 20 (6.9 m/s, 87 frames). From his side it is still one size, the largest (item 2). The mashed button (item 4)
is the law's leak: 21 landed hits a match is not momentum, it is a held window.

**Law 2, the brain knows hockey: half, as before.** It brakes on purpose (252 reverse brakes a match at pro; 2,146 on Friday) and sprints with the puck
(10.6 s a match). Shot selection is unchanged (item 6); the mates release from inside 15 m 0.25 / 0.75 / 0.50 times a match; a newcomer at rookie goes a
half-minute without the puck in 39 of 200 matches (53 at REVIEW-2), a regular player at pro in 12 of 200; on big ice it is the rule (item 9).

**Law 3, speed change and the stick beat a defender: true at one distance at every level; rare in a match.** The stop by where it begins (past the
defender, %; 200 seeds a row, the real brain; `review-3/sweep.txt`):

| the stop begins at | 3 m | 4 | 5 | 6 | 7 | 8 | 9.5 | 11 | 12 | 13.5 | straight |
|---|---|---|---|---|---|---|---|---|---|---|---|
| rookie | 95.5 | 74.5 | 28.5 | 70.5 | 100 | **100** | 100 | 100 | 100 | 100 | 0 |
| pro | 0 | 5.5 | 7.5 | 7.5 | 66 | **100** | 97 | 83.5 | 56.5 | 52 | 0 |
| all_star | 41.5 | | 4.5 | 55.5 | 97.5 | **100** | 92 | 53 | 28.5 | 19.5 | 0 |

Neither side of the window is soft: late, he is on the ice (pro: 199, 179, 153, 111 of 200 from 3 to 6 m); early, the defender adjusts (pro from 12 m: 40
down, 26 held to the time-out; all-star: 52 and 73). In matches the human gets past 9.0 / 5.0 / 7.2 defenders a match (50 / 40 / 35 % of those he
meets; 6.0 / 6.0 / 8.1 at REVIEW-2), by a stop 0.67 times a match at every level, by plain speed 6.5 / 3.3 / 5.1: the stop is still the script's hand. The
carry trace says where it would count: the stop was open a second before 4.7 % of his lost carries and before 15 to 25 % of his knockdowns.

**Law 4, sprint and the puck's distance: the rules are in; the gamble is still not felt.** Drills green. In the traces he loses 34 carries a carried
minute at a slow pace, 24 skating and 10.5 above 10.5 m/s at pro (the same shape as REVIEW-2: he sprints only on open ice); he sprints with the puck
0.8 s a match, the bots 10.6 s.

## 5. The first ten minutes, the season, the modes

### 5a. A newcomer through PLAY NOW at rookie (hockey.regular at rookie, the novice, 200 seeds; marks from 100 matches)

| When | What happens | Number |
|---|---|---|
| 0:02 | he wins the draw; if he does, an orange ring is round his own puck | first touch 1.8 s; `contested-f1017.png` |
| 0:42 | his first shot | 42.0 s (35.4 at REVIEW-2); none at all in 4 of 200 |
| 1:03 | his first goal | 63.1 s; inside two minutes in 152 of 200 |
| 1:12 | the first goal against, when there is one | 72.5 s |
| all match | an orange ring every six seconds | 18.6 contested rings and 15.2 lit stop marks a match |
| | Space, sixteen times; a puck 1.4 times | 8.6 %; a puck by a poke in 127 of 200 matches; 0.2 stumbles |
| | he hits and is hit | 3.6 landed (in 180 of 200); down 3.3 times (in 182), 3.15 % of play |
| | the puck slides off his blade | 2.5 carries a match of 14.8 (16.5 %) |
| 3:24 | the match ends | **89 %** won, 2.8 : 1.1; ten minutes are three such matches |

Verbs he uses (matches of 200 with at least one): shot 196, pass 182, pass received 174, a hit landed 180, a poke 198, the puck held 3 s 189, a reverse
brake 200 (21 a match, none of them meant), a lift 73. He never cuts, dekes, sprints or one-times: the model's first-timer does not know those verbs.
**Where he gets stuck at rookie**: a half-minute without the puck in 39 of 200 matches; nowhere else. **Then he lights PRO: 16 %**, first shot after
67.5 s, no shot in 38 of 200, a goal by him in 81, a half-minute without the puck in 71 (item 5). A regular player through the same door: 100 % in 1.8
minutes, then 53 % at pro in 3.0 minutes, first shot 25.6 s, down 7.1 times a match.

### 5b. The season's takes 1 to 5 (60 seeds a take; each at its own level)

| take | level | novice | regular | good | the novice's match | note |
|---|---|---|---|---|---|---|
| 1 | rookie | 77 % | 98 | 98 | 2.1 min; first shot 41.4 s; a goal inside two minutes in 45 of 60 | |
| 2 | rookie, 120 x 60 | 83 | 100 | 97 | 2.3 min; a half-minute without the puck in 20 of 60 | 37 of 60 before |
| 3 | rookie | 87 | 100 | 98 | 3.6 min | |
| 4 | rookie (was pro) | **83** | 98 | 100 | 2.8 min; no shot by him in 4 of 60 | 20 % and 17 of 60 before |
| 5 | pro, red skill 0.37 | **63** | 100 | 100 | 4.4 min; a puck by a poke in 53 of 60 matches | 12 % before |

Takes 6, 7, 8: novice 62 / 62 / 7, regular 100 / 90 / 28, good 100 / 95 / 52 (item 10).

### 5c. The twelve modes, one line each (the shipped press; the regular class on seeds 1-40 unless said; "x" = the mode's own number against the regular game's)

| mode | a different game? | winnable? (regular / novice, %) |
|---|---|---|
| Arsenal | yes: 4.85 tricks on F a match (the regular game: none) | 65 / 10; the novice waits a half-minute in 19 of 40 |
| Bruisers | **barely: hits 1.34x; 1.33x at 200 seeds, a hair under its third** (1.43x in MODES-6) | 45 (56.5 at 200) / 17.5 |
| Cage | **less than it was: checks landed 1.28x; 1.47x at 200 seeds** (1.96x in MODES-6) | 65 (72.5 at 200) / 35; good 83 |
| Fog of War | the most: breakaways 2.71x; **2.89x on 200 fresh seeds** (MODES-6: 3.11x). A solo game: 0.8 passes a play minute | 77.5; **80.5 fresh** / 38 to 55; good 92 |
| Glass House | yes: 12.9 s in the box a match; the one whistle (a foul in 28 of 40) | **32.5** (43.5 in MODES-6) / 17.5 |
| Marathon | yes: sprint 1.63x; 1.66x fresh; 6.8-minute matches | 70; 74.5 fresh / 10; the novice waits in 27 of 40 |
| Outnumbered | yes: shots against 1.66x; 1.64x fresh | 52.5; 63 fresh / 15 to 26 |
| Pond | yes: passing 1.65x; no knockdowns | 70 / 20 |
| Power Play | yes: passing 1.51x (1.31x for a good player) | 52.5 / 22.5; good 91 |
| Shotgun | yes: bodies down 1.59x; 1.46x fresh | 57.5; 56.5 fresh / 25; **10 fresh** |
| Sniper | yes: goals per shot 1.93x; 77-second matches | 45 / 20; 26 fresh |
| Sudden Death | yes: 38.5 s (0.21x); 49.6 s fresh | 50; 36 fresh / 12.5 |

Every one of the 3,060 mode matches ended. Three rows of MODES-6 checked on fresh seeds (1001-1200), and two more: Fog of War 2.89x and 80.5 % (3.11x, 79), Shotgun
1.46x and 56.5 % (1.67x, 46.5), Outnumbered 1.64x and 63 % (1.68x, 57), Marathon 1.66x, 74.5 %, a half-minute without the puck in 14 % (1.60x, 73.5, 16),
Sudden Death 0.27x and 36 % (0.27x, 34.5): **five of five hold**. Fog of War at three a side is what the modes lane says it is: the slip-by mode, and the
easiest for everyone. What does not hold is Cage's and Bruisers' distance from the regular game: the regular game's own landed checks rose from 3.50 to
4.5 a match after the modes lane measured (the brace and the shipped press), so the two hitting modes are nearer to it than their table says.

## 6. The sheet's sentences that stand, with the number

- The package line: main's `status/A.md:447` named H28 from c5dc15e5 at 05:46 (my tree's copy is older).
- The level switch reaches the match; a fresh profile is rookie; takes and modes keep their own; the cards show it: `MenuWidget.cpp:248`,
  `MenuFlow.cpp:31-34`, `:61-62`, `HokejGameMode.cpp:512-513`, `SettingsWidget.h:35`, `DirectorsCallWidget.cpp:345-348`, `:576-582`; `Jam.Menu` green.
- The ladder: 89 / 53 / 28 here against the sheet's 86 / 52 / 29; the player who lands his checks 90 / 59.5 / 29.5 against 87 / 56 / 31.
- No whistle but in Glass House (section 2, item 4). The stop from 8 m at every level (200 of 200, three times).
- Space, the grace and STICK LOCKED as written (`JamSim.cpp:1596`, `:1646-1657`; 3.0 m; `Jam.Hud.CarrierCues` green).
- Hits have sizes, a standing skater holds a side hit, nothing drawn on a downed player, two seconds of grace (`ReleaseGraceFrames` 120): tests green.
- On his back 9 % of play at pro (9.07); the opponents' median release 36 m at pro (37.6); big ice at pro "more than half" (62.5 % of 200); his carry
  "a second" (0.78 to 0.88 s).
- The three modes named win 45 to 80 % (56.5 / 80.5 / 63); Arsenal's tricks have three charges; the passes at five a side "about three" (3.24 / 2.32 / 3.28).
- The controls and the first screen's keys (`InputMapping.cpp:79-127`, `MenuWidget.cpp:910-934`, `:1045-1049`). 412 commentary lines exist as assets.
- **Not judged** (no window, no sound): the clay look with the stands and the crowd (the rig `LR_P3_Clay` names them), the arena's answers, the camera,
  the menu's look, a joined match, whether the booth speaks.

## 7. What is switched off and waiting: the two I would turn on first

| Rule (the lane's cells: rookie / pro / all_star) | What a player gets | Verdict |
|---|---|---|
| the check's cooldown, 60 frames with the restart (CONTACT-8: +0.5 / 0.0 / -0.5) | mashing lands fewer hits than timing | **first** |
| the pickup that settles, every skater, 6 frames (MOVEMENT-7: **+5.8 / -9.0 / -5.0**) | the puck sticks when he skates onto it; nobody's blade 49.7 to 43.2 % | **second, with a curve pass** |
| the push-out round a downed skater, 0.6 (CONTACT-7 and 8: -1.0 / +1.0 / +2.9 to +4.0) | nobody stands in or shoves a downed man | third: the second pays for it |
| the lunge, 12 frames and 1.5 m/s (CONTACT-7: -1.2 / +1.5 / +3.3) | a press commits the body | waits: all-star has no payer |
| the half-second screen (BRAIN-4: +1.5 / +2.2 / -1.4) | a mate stands in his checker's way; waiting at rookie 28.5 to 22.3 % | waits: pro goes to 6.6 over |
| the stop's grace, 30 frames (MOVEMENT-7: 0 / -0.4 / +4.0) | a stop shields the puck from sticks | no: the carry's median does not move, the body takes what the stick is refused |

1. **The check's cooldown.** A judge mashes, and today that is the best play (item 4). Price: nothing the curve can read (half a point either way).
2. **The pickup that settles.** It answers the thing felt on every touch (item 3, a third review without its bar), and it is the first lever found
   that takes **pro down without touching the stop** (R-0062) and all-star down at all. **What the curve must pay**: from 85.8 / 52.4 / 28.8 the arm
   alone lands near **91.6 / 43.4 / 23.8**. All-star is then inside 3 of its 26. Rookie owes about 6.6 points back (its red skill up: the direction CURVE-9
   used on the large rink) and pro is owed about 4.6 (its red skill down from 1.07, about half a point per 0.01 on this rink: the direction that
   **widens** the stop's window, so law 3's gate holds). Two profile numbers, 2,000 a cell on both rinks, law 3 re-read. With it on, the push-out's
   three to four points at all-star land on 23.8 and need no lever. Without that pass, turn on the cooldown and the push-out and accept all-star near 32.
   The arithmetic assumes the arms add; nobody has measured them together.

## 8. Findings outside `docs/design` (no edit made)

| Where | What | Track |
|---|---|---|
| `Game/Source/JamCore/Public/JamCarrierMarks.h:22-26`, `:75-83`; `Game/Source/JamGame/Private/HUDMarks.cpp:115`, `:131`, `:142` | The marks' rules do not read the level: at rookie 34 rings a match at 8 to 16 % precision; both marks are an orange ring of one size. Item 1, with swept numbers | F2, U1 |
| `Game/Source/JamGame/Private/HUDMarks.cpp` (the contested ring after a draw) | The ring is drawn round the puck the human has just won at the faceoff | F2 |
| `Game/Source/JamGame/Private/HUDMarks.cpp:9` | Stale comment: `HUDClearDown.cpp` "not in this tree"; it is | U1 |
| `Game/Source/JamGame/Public/HUDWidget.h:58` | The first faceoff's hint strip names neither Space nor F. Item 8 | U1 |
| `Game/Source/JamGame/Private/DirectorsCallWidget.cpp:576-582` | The faceoff card covers the one-time brake prompt (`stop-lit-f1040.png`); and a coach's note replaces the line that shows the level | U1 |
| `Game/Source/JamGame/Private/MenuWidget.cpp:248`; the end card (`HUDWidget.cpp:424-438`) | No word says whom a level is for or where to go after a win. Item 5 | U2 |
| `Game/Source/JamCore/AI/DifficultyProfile.cpp` (rookie's shot from reach + 25 m) | Free to shorten at rookie: 89 to 90 % with 13 m. Item 6 | C0 |
| `Game/Content/Cartridges/hockey.arena.large.cartridge.json`, `ladder/take-2`, `take-7` | `skater.carry_speed_mult` 0.8 takes the half-minute waits from 62.5 to 25 % at pro (+14 points) and from 63 to 39 % at rookie (free). Item 9 | Q2, C0 |
| `Game/Content/Cartridges/ladder/take-6`, `take-7` | Flat for the newcomer (62 / 62), no contest for a regular player (100 / 90), then the finale. Item 10 | C0 |
| `Game/Content/Cartridges/modes/hockey.mode.{bruisers,cage}.cartridge.json`; `docs/experiments/MODES-6.md` (the table) | 1.33x and 1.47x of the regular game at 200 seeds on this commit (1.43x, 1.96x in the table): `modes.mjs` exits 1 on Bruisers | M1 |
| `docs/experiments/MODES-6.md`, `docs/PLAY-ME.md:86` (main) | The novice's Sniper is 20 to 26 %, his Shotgun 10 % | M1, the Director |
| `Game/Source/JamGame/Private/HokejGameMode.cpp:515` | `JAMMATCH level=` is a `UE_LOG`: a Shipping package's log cannot show which level a match played | S2 |
| `docs/design/review-2/first.mjs` | Two runs of one cartridge, level and player given together are summed into one cell (a variant and its base): pass them one file at a time | F0 |

## 9. What this review could not see

No frame of the game in motion: the two marks, the flattened marker, the clay look, the stands, the camera and the HUD are read from four of the lanes'
stills and from source. No person: the playtester brakes, pokes and shoots by rule; the stop's use, the poke's one in ten and the carry's median are a
script's. The marks' precision is "the fall came", not "a person understood the ring". Samples: a 40-seed cell carries about 8 points of noise, 60 seeds
6, 100 seeds 5, 200 seeds 3.5; the traced medians rest on 150 to 270 carries a cell; a difference of two 200-seed cells that are not paired carries about
5. CONTACT-8 and the corrected sheet are on a later main than my tree and are quoted, not measured. The arms of section 7 were never run together.

## 10. The track's next three evolutions (F0)

1. **The operator's play against this file**: his words at 10:00 beside each of the ten items and each row of section 0, and the table of section 3 with
   a third column on the commit of his priorities. Proof: every item kept, dropped or re-ranked with his sentence.
2. **The marks as a person reads them**: drill 21 by his own keys with `jam.hud.Marks` 1 and 0, and the carry trace's showings split by cause (a draw, a
   rebound, a race), at rookie and pro. Proof: the share of first brakes begun at 8 to 9.5 m with the mark and without; rings a match at rookie under 20
   with precision over 25 %.
3. **Section 7's arms together**: the pickup that settles, the cooldown and the push-out in one build, 600 seeds a cell, both rinks, before the curve
   lane spends a pass on them. Proof: whether the arms add (91.6 / 43.4 / 23.8, then the push-out's points), and nobody's blade under 45 % at pro.

## 11. Reproduce

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\run\build.ps1 -Project <worktree>\Game
bash docs/design/review-3/batchA.sh     # the grid (1-40), the takes (1-60), the large rink, the 3 x 3 at 200 seeds on both instruments
bash docs/design/review-3/batchB.sh     # traces.sh (three traced cells; review-2/laws.mjs, review-1/humancarry.mjs), then auto.sh: the drills with the
                                        #   stop's sweep (JAM_SWEEP_FILE=review-3/sweep.txt) and Bar.Movement.CarryTrace (JAM_CARRY_TRACE="level=..;player=..;seeds=100")
bash docs/design/review-3/marks.sh      # the marks' rules without a build (JAM_MARK_RULE="far=9.5" | "closing=14;stopclosing=10" ...); marks2.sh: one set for every level
bash docs/design/review-3/batchC.sh     # the twelve modes on seeds 1-40 (regular, novice); five rows on fresh seeds (1001-1400)
bash docs/design/review-3/batchD.sh     # the masher (JAM_PLAYTESTER_MASH=12), opp.shoot_extra_m=13 at rookie, skater.carry_speed_mult=0.8 on the large rink, Cage and Bruisers at 200
node docs/design/review-2/first.mjs <lines.jsonl>                    # one line a cell (one run a call: it sums runs that share a cell)
node docs/design/review-3/compare.mjs <before.cells.jsonl> <after.cells.jsonl> [fields=a,b]
node docs/design/review-3/lawsum.mjs                                 # the laws and the carries, REVIEW-2 beside REVIEW-3
bash docs/design/review-3/carrysum.sh <carry trace log>              # the carry trace's summary lines
```
