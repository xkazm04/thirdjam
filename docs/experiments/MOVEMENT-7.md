# Movement 7: the player can see why, and where a carry ends (lane A163, track F2, evolution 6, 2026-11-14)

The brief: REVIEW-2 read the human's carry at a median of 0.90 s at pro (the bar is 1.5), the human on the ice 7.8 % of play, and MOVEMENT-6 left two states
nobody can see: the take grace that refuses half of his pokes, and the stick lock after his own miss. This lane shows both (presentation, section 1),
traces where his carries end and where he falls (sections 2 and 4), and builds one rule behind a key at its neutral value (section 3).
Status with timestamps: `status/A163.md`. Raw logs, edit scripts and runners: `Game/logs/a163/` (not landed); cells: `tools/perf/results/a163/`
(`cells.jsonl` lands, `raw/` does not). The main it stands on: a7d89871.

## 1. The cues (presentation: no sim state, no new widget, no new figure call)

| State | What the player sees | HUD element (it existed) | Headless read-out |
|---|---|---|---|
| **The take grace** (`game.stick.take_grace_frames` 30: after a skater wins the puck no stick takes it), on any carrier | the ring under the carrier turns the HUD's yellow and is drawn 1.5 x its size, closing to its own size as the grace runs out: a clock. When the ring has closed and gone back to its colour, the puck is fair game | `YouRing` (the human's ring, team colour) and `CarrierRing` (the white ring under any other carrier) | `JAMVIS takegrace frame= skater= team= human= on= frames=` from the pawn, one line as a grace begins (every skater) and one as the human's ends; `JAMHUD frame= cue=take_grace carrier= ring= scale=` from the HUD |
| **A press into a carrier's grace** (Space with the opponent's ring yellow: the press is refused and costs nothing) | the contact line says **JUST WON IT** | the contact line (`ContactPlate` / `ContactText`, through `ShowContactLine`: track F1's plate that says MISSED, TOO SOFT, JUST GOT UP) | `JAMHUD frame= cue=press_into_grace contact_line=JUST_WON_IT` |
| **The stick lock** (`game.stick.miss_recovery_frames` 20: after a person's missed poke Space and RMB do nothing) | the contact line says **STICK LOCKED** for exactly as long as the lock runs, then fades (15 frames) | the same line | `JAMVIS sticklock frame= skater= human=1 on=1 frames=20`, and `on=0` when it ends; `JAMHUD frame= cue=stick_lock contact_line=STICK_LOCKED lock_left=` |

Code. One HUD function, `UJamHudWidget::UpdateCarrierCues` (`HUDWidget.cpp`), called once as the last line of `RefreshFromFrame`, after `UpdateYours` has
placed and coloured the rings; it re-tints and re-sizes a ring only while a grace runs and never shows a ring `UpdateYours` hid (lane A158 hides the
human's ring while he is down: the cue respects it). Its rules are three static functions in `HUDWidget.h` (`TakeGraceRingScale`, `CarrierCueLine`,
`CarrierCueFadeStart`) and three members. It reads `UJamSim::InTakeGrace`, `FJamSkaterState::TakeGraceLeft`, `StickLockLeft`, `PokeWindow`, `bPokeArmed`;
it writes nothing to the sim. The pawn (`AJamSkater::ApplySimFrame`, `JamPawns.cpp`) writes the two `JAMVIS` lines from the same read-outs (two members in
`JamPawns.h`). No other HUD function was touched: not `UpdateYours`, `UpdateMarker`, `UpdatePuckMarker`, `UpdateCommentary`, `UpdateContactLine` or `BuildTree`.

The figure. `contracts/skater-visual.md` has no call for either state, so nothing new is called. What the figure already gets: `PlayPoke(false)` on the
miss's frame (the contract's text still says "a stumble follows": for a person it is now the lock) and `PlayStickHit(false)` on a carrier that lost the
puck to a poke. **Asked of device B (by name, v6 of C-0001, the Director's text):** `SetStickLock(bool bOn, int32 Frames)` (level, on the change: the
stick lowered or held back while the lock runs) and `SetTakeGrace(bool bOn, int32 Frames)` (level: the puck or the blade glows while no stick takes it);
the pawn's two `JAMVIS` blocks are where the calls go (`CallFigure`, as `PlayStumble` and `SetBraking`).

Proof (build 2b, the cue tree):
- `Jam.Hud.CarrierCues` (new, in the gate; `JamGame/Private/CarrierCueTests.cpp`): the ring's scale (1.5 x at the start, 1.25 half way, its own size with
  no grace or with the rule off, only ever closing); the line's words and which wins; the fade's start; and on a staged sim of `hockey.regular`: the human's
  short poke misses and **STICK LOCKED stands for 20 frames** (`miss_recovery_frames`); the human's aimed poke takes the puck and **he carries inside his
  grace for 29 frames, the ring from 1.48 x to 1.02 x**; a placed puck's first carrier has no grace and no cue.
- The pawn's lines in the real game mode, headless (`Game/logs/a163/run-game.ps1`: the editor binary as the game, `-game -nullrhi -Multiprocess`,
  `hockey.regular`, `jam.ScriptHuman=1`, the proof's switch `jam.ScriptHumanPoke=45`, 116 s): 65 `JAMVIS takegrace` lines (the human's `on=1 frames=30`
  at frame 287 and `on=0` at 317; 1606 and 1636; the bots' on both teams) and 16 `JAMVIS sticklock` lines (`on=1 frames=20` at frame 1723, `on=0` at 1743:
  eight misses, each lock 20 frames).
- The full `Jam` filter on that build: 250 pass, 0 fail (`Jam.Hud.ContactLine` untouched and green); 251 on the final tree. The Shipping target compiles on both.
- The HUD's own lines (`JAMHUD cue=`) and the picture need a renderer: the HUD does not tick under `-nullrhi` (R-0060 point 5). Section 6 says what was
  captured off screen and what is the acceptance lane's to read in the window.

`jam.ScriptHumanPoke N` (`HokejGameMode.cpp`, default 0: nothing runs) lets the scripted human of the real game mode press Space next to an opposing
carrier, at most once every N frames: the proof's switch, for this proof and for an acceptance row.

## 2. Where the carry ends (measurement first)

`Bar.Movement.CarryTrace` (new, outside the gate; `Game/Source/JamCore/Private/Tests/JamCarryTests.cpp`; env
`JAM_CARRY_TRACE="level=pro;player=regular;seeds=100;rows=0;set=key:v,key:v"`) follows the human through real matches frame by frame. A carry is 20
frames or more of the sim's `CarrierIndex()` being his (shorter possessions are touches, as in REVIEW-1's script); it ends by the first cause the sim
shows on the frame the puck leaves him: the whistle, his own shot or pass (the release flags, dated by 12 frames: a pass at 14 m/s needs four frames and
more to leave the stick's reach), an opponent's clean poke or lift (the events), a knockdown or a stumble with `ContactBy`, another skater's touch, or
nothing at all for half a second. For every carry it also reads what was open **a second earlier (or as the carry began, when it was shorter)**: a pass
(`UJamSim::BestPass` found a receiver), the stop of law 3 (the skater who ended it 7 to 11 m ahead and closing at 3 m/s or more, he at 3 m/s or more, no
brake in the carry: the window REVIEW-2 measured, 66 to 100 %), the spin (ready a fifth of a second before a stick took it). These are "an option was
open": an upper bound on what a person could have kept, not a replay.

Pro, the regular player, **100 matches (seeds 1 to 100), the tree as it ships: 1,662 carries of the human, 16.6 a match, median 0.88 s** (REVIEW-2: 0.90);
30.2 % last 1.5 s or longer; 1.60 a match last 3 s. Of play, he carries 15.5 %, his mates 7.3 %, opponents 27.5 %, **nobody 49.7 %**.

| How the carry ended | Carries | Share | Median length | A second or less | A pass was open | The stop was on | The spin was ready | Any of the three |
|---|---|---|---|---|---|---|---|---|
| his own shot | 401 | 24.1 % | 1.02 s | 49.6 % | | | | |
| his own pass | 367 | 22.1 % | 1.03 s | 49.3 % | | | | |
| **a poke took it** | 264 | 15.9 % | 1.07 s | 47.3 % | 72.7 % | 4.9 % | 64.8 % | 95.1 % |
| **a lift took it** | 151 | 9.1 % | 0.90 s | 55.0 % | 69.5 % | 4.0 % | 49.0 % | 88.1 % |
| **a check: down** | 220 | 13.2 % | 0.88 s | 63.2 % | 74.5 % | 7.7 % | (no help against a body) | 76.4 % |
| a check: a stumble, the puck loose | 20 | 1.2 % | 0.97 s | 55.0 % | 55.0 % | 10.0 % | | 55.0 % |
| **lost handle: it left the blade and nobody touched it for half a second** | 142 | 8.5 % | **0.42 s** | 94.4 % | 59.2 % | 0 | | 59.2 % |
| lost handle: an opponent's touch (no poke, no lift: a shove, a body) | 55 | 3.3 % | 0.42 s | 78.2 % | 70.9 % | 3.6 % | | 72.7 % |
| lost handle: a mate took it | 25 | 1.5 % | 1.00 s | 52.0 % | 84.0 % | 16.0 % | | 88.0 % |
| the whistle (a goal, the horn) | 17 | 1.0 % | 1.28 s | 47.1 % | | | | |

What the table says:

- **Nearly half of his carries (46.2 %) end by his own shot or pass, at a median of one second.** The carry's median is first of all the instrument's
  patience (it shoots when lined up inside 30 m and passes under pressure). No rule about taking the puck touches these.
- **He loses 852 of 1,662 (51.3 %)**: a stick 415 (25.0 %), a body 240 (14.4 %), the handle 197 (11.9 %). **62 % of the lost carries are shorter than a
  second**: there was no "second earlier" in them, only the start.
- **What a person could have kept, as an upper bound**: a pass was open a second earlier or at the start in 69.8 % of the lost carries, the spin was
  ready against the stick in 28.8 % (64.8 % of the pokes), **the stop was on in 4.7 %** (the taker is almost never 7 to 11 m ahead a second before: he is
  already there); one of the three in 80.6 % (87.5 % of the 321 lost carries of a second or longer). So at pro the honest advice is the pass and the
  spin; the stop saves a carry in one loss of twenty.
- **The stick's 415 takes by the carry's length**: 96 within 6 frames of the grace's end and 57 in the next 9 (37 % inside a quarter second after the half
  second of grace: the pro waits the grace out and takes), 55 up to a second, 106 in the second second, 101 later. His own stop (a reverse brake begun at
  6 m/s or more) had begun inside the half second before the take in 39 of them (9.4 %).
- **The 142 carries that "left the blade" (8.5 %, median 0.42 s, 86 % inside 40 frames)** are neither a release (1 of 142 flew at 14 m/s or more) nor
  his brake (8.5 %): he meets the puck at 9.5 m/s, it is 1.77 m from his body moving 7.2 m/s against him, the carry assist does not seat a puck faster
  than 4 m/s against the body, and it slides out of reach. **He has it again himself, nobody else touching it, in 101 of them (71 %), a median of 1.02 s
  later.** For a person this is "I skated over the puck and it did not stick". It is the one class with no opponent in it.

**What a longer grace buys (the upper bound of every rule on the stick), 40 matches each on the same seeds:** `take_grace_frames 45`: a stick ends 136
carries (176 at 30 frames), a check 128 (84), the median **0.87 s** (0.85 on those 40 seeds), he lies down 9.55 % of play (8.68 %).
`take_grace_frames 120` (two seconds: sticks nearly off, 80 takes): the median **0.87 s**, a check ends 122, down 9.21 %. **The body takes what the
stick is refused, and the median does not move.** The bar of 1.5 s at pro cannot be bought with a stick rule at any price.

## 3. The rule, behind a key at its neutral value, and three code-only arms

| Rule | Where | Default | Neutral |
|---|---|---|---|
| **The stop tucks the puck**: the frame the reverse brake of a carrier a person steers begins, at 6 m/s or more (`UJamSim::StopGraceMinMps`), its puck has the take grace for this many frames: no poke, no lift; a body check still loosens it (law 1); a sprinting carrier has to stop first (law 4); a shuffle on the spot earns nothing | `game.stick.stop_grace_frames` (`FJamStickConfig::StopGraceFrames`; `UJamSim::StepSkater`, the brake's first frame) | **0** | 0 |
| arm: the stop's grace for bots too | `FJamStickConfig::bStopGraceBots` (code only) | false | false |
| arm: the take's first step (the frame a person wins the puck its first-step burst runs N frames) | `FJamSkaterConfig::BurstTakeFrames`, `bBurstTakeBots` (code only; `UJamSim::Step`, where the take grace is set) | 0 | 0 |
| arm: the pickup settles (the frame a person becomes the new carrier the puck is seated on the blade for N frames, as a pass's reception is) | `FJamStickConfig::PickupSettleFrames`, `bPickupSettleBots` (code only; the same place) | 0 | 0 |

No state field was added: the stop's grace is the take grace's own `TakeGraceLeft` (hashed while above 0 since lane A121), the first step is `BurstLeft`,
the settle is `SettleFrames` (both hashed while above 0). With the key and the arms at 0 no line of the lane runs. Because the stop's grace IS the take
grace's state, section 1's cue shows it with no further code: brake with the puck and your ring turns yellow and closes.

Why the stop: law 3 makes the stop the carrier's weapon, today it beats a body (the stop begun at 8 m passes 100 %) and does nothing against the stick
that waits beside him; and 37.5 % of the carries a poke ended had a brake in them. It was the candidate before the table was read; the table and the
arms say what it is worth.

**The arms (pro, the regular player, 100 matches each, seeds 1 to 100, the trace):**

| Arm | Carries a match | Median | 1.5 s or longer | 3 s, a match | A stick ended | A body ended | It left the blade | He lies down | Wins of 100 |
|---|---|---|---|---|---|---|---|---|---|
| the tree as it ships | 16.6 | **0.88 s** | 30.2 % | 1.60 | 415 | 240 | 142 | 9.07 % | 63 |
| `stop_grace_frames 30` | 16.0 | 0.82 s | 28.9 % | 1.40 | 360 | 270 | 140 | 9.52 % | 55 |
| `stop_grace_frames 45` | 15.8 | 0.83 s | 29.4 % | 1.45 | 332 | | | 9.32 % | 64 |
| 30, bots too | 17.9 | 0.82 s | 29.8 % | 1.69 | 401 | | | 9.34 % | 55 |
| the take's first step, 20 frames | 16.6 | 0.83 s | 30.1 % | 1.75 | 408 | 285 | 139 | 9.29 % | 49 |
| the stop 30 + the first step 20 | 16.7 | 0.87 s | 29.9 % | 1.58 | 352 | | | 9.58 % | 56 |
| the pickup settles, 6 frames (a person's) | 15.2 | 0.92 s | 32.6 % | 1.61 | 409 | 228 | **59** | 10.83 % | **83** |
| the pickup settles, 6 frames, every skater | 12.1 | 0.95 s | 33.0 % | 1.22 | 334 | 217 | 45 | 9.79 % | **38** |
| the pickup settles, 12 frames (a person's) | 15.9 | 0.88 s | 30.4 % | 1.40 | 435 | 258 | 48 | 11.21 % | 76 |

(The last three rows are build 4's; "a body ended" = a check that put him down or stumbled the puck loose.)

**The third arm is the one that does something, and it is not about the carry's length.** With the pickup settled for six frames the carries that
left the blade fall from 142 to 59 and the possessions under 20 frames from 2,146 to 1,175: half of all his short touches were pucks that did not
stick. He then has the puck 18.5 % of play (15.5 %), shoots 464 times (401) and **wins 83 of 100 (63)**. Given to every skater, the opponents carry
34.4 % of play (27.5 %), **the puck is on nobody's blade 43.2 % of play (49.7 %; REVIEW-2's bar is 45 %)** and he wins 38 of 100. The median carry
moves by the noise (0.92, 0.95 s). So: a large lever on who has the puck and on the ladder (20 points and more, either way), a small one on the bar of
this brief. It ships as code only at 0; its key, for every skater, belongs to the curve's next re-set (section 9, the first evolution).

**The cells (600 a cell, seeds 1 to 600, paired; `tools/perf/results/a163/cells.mjs`, three processes, `-Multiprocess`; the ladder's instrument
`JAM_PLAYTESTER_CHECK=closing`, R-0061):**

| Arm | novice at rookie | regular at pro | good at all_star | Against the base |
|---|---|---|---|---|
| base (`baseC`, build 1 = a7d89871) | 86.5 | 53.2 | 30.0 | |
| `stop_grace_frames 30` (`stop30C`) | 86.5 | 52.8 | 34.0 | 0 / -0.4 / **+4.0** |
| the pickup settles (code only) | | | | its 600-seed cells were still running when the tree was closed: `status/A163.md` has them |

With the default instrument (lane A151's press) the base reads 87.0 / 54.7 / 29.8 on the same seeds.

**Neutral, and the proof that the default match did not move.**
- The key ships at 0 and the three code arms at 0 / false: no line of the lane runs in a shipped match. `JamBaseline::MakeNeutral` carries the key's
  line for the day it ships on.
- **Hash identity at scale**: the arm `n0` (the cartridge as it ships, the final tree, build 4) against the arm `base` (build 1, the tree as it came:
  a7d89871), 600 matches a cell, seeds 1 to 600, the default instrument: **the same hash, frame count and score in 600 of 600 matches of each of the three cells (novice at rookie, regular at pro, good at all_star)** (`node tools/perf/results/a163/same.mjs base n0`).
  The brief's 100 hashes at pro and at all_star are inside it six times over.
- In the gate: `Jam.Core.Movement.StopGrace` plays pro seed 3 three times (the shipped cartridge, the key at 0, the key at 30): the first two hashes are
  equal, the third differs (logged, not pinned).
- In a cartridge: `"stick": { "stop_grace_frames": 30 }` turns the rule on; leave the key out for 0.

**Verdict on the bar.** The bar was a median of 1.5 s at pro with the cells inside 2 points. **It is not met and no carrier rule measured here moves
it**: every arm reads 0.82 to 0.88 s (the spread of 100 matches). The stop's grace does what it says (stick takes 415 to 360, 332 at 45 frames) and the
body takes them back (240 to 270 carries ended by a check; he lies down 9.5 % of play against 9.1 %); at all_star, where the good player brakes 45 times
a match, it is worth +4.0 points, outside the bar. **Recommended value of `game.stick.stop_grace_frames`: 0.** It stays in the tree as a measured key
because it is the one arm a person's hand may read differently from the script's (a stop with a visible reward); if the operator wants it tried, 30 costs
0 / -0.4 / +4.0 and the curve track answers with one all_star number.

## 4. Time on the ice lying down

The same trace reads every knockdown of the human against the frame before it and against the second before it (or the moment he got the puck, when that
is nearer). Pro, the regular player, 100 matches: **676 knockdowns (6.76 a match, 2.6 a minute of play), 126 frames each, 9.07 % of play on the ice**
(REVIEW-2 read 7.8 % on the ladder's commit; the poke's stumble is gone since lane A152: 0.75 stumbles a match are left, all of them hits). Every one of
them is a check he took (in the 600-match cell his knockdowns and the checks he took are the same number, 7.02 a match).

| The knockdown (the first row that applies) | Count | Of knockdowns | Of the frames down | Frames each | Closing speed | 10 m/s or more | The puck his for | Into his front arc | The hitter a second earlier (or as he got the puck) | A pass was open then |
|---|---|---|---|---|---|---|---|---|---|---|
| without the puck | 21 | 3.1 % | 3.0 % | 122 | 13.9 m/s | 61.9 % | | 85.7 % | 10.8 m | |
| carrying, hit from behind his facing (unseen) | 7 | 1.0 % | 0.9 % | 105 | 5.5 m/s | 0 % | 0.73 s | 0 % | 4.5 m | 85.7 % |
| **the puck his for under half a second: hit as he won it** | **439** | **64.9 %** | **67.6 %** | 131 | **16.1 m/s** | 79.3 % | **0.14 s** | 77.0 % | 3.7 m | 60.6 % |
| carrying, he braked inside the last half second: the stop begun too late | 29 | 4.3 % | 3.7 % | 109 | 9.2 m/s | 34.5 % | 0.62 s | 72.4 % | 4.1 m | 62.1 % |
| **carrying, no brake, the hitter 7 m or more off a second earlier: the stop was on** | 103 | 15.2 % | 15.4 % | 127 | 14.1 m/s | 81.6 % | 1.44 s | 63.1 % | 11.4 m | 75.7 % |
| carrying, no brake, the hitter inside 7 m a second earlier: no room | 77 | 11.4 % | 9.5 % | 106 | 8.8 m/s | 32.5 % | 1.14 s | 51.9 % | 3.9 m | 76.6 % |

No knockdown found him sprinting: the instrument sprints with the puck only when nobody is inside 5 m.

- **Two in three of his knockdowns (65 %, 6.1 of the 9.1 % of play) come within half a second of his first touch of the puck, on average 8 frames after
  it, at 16 m/s of closing**: two skaters race head-on for a loose puck, he touches it first, that makes him the carrier and the pro's pressed check
  lands a quarter of a second later, for 131 frames. Law 1 is kept to the letter (it is the biggest hit in the game) and no hand answers in 8 frames. A
  person avoids it by how he arrives, a second earlier: brake before the puck (a slow carrier facing the hit keeps the stand brace), let the pro take it
  and hit him (the carrier cannot hit; the one who arrives second can), or cut across and take it after the pro has gone by. **How he would know:** an
  opponent skating at him with the puck between them; nothing on the screen says "this one is contested" today.
- **15 % (1.4 % of play) are law 3's own case**: he had the puck for a second and a half, the hitter was 11 m ahead a second earlier, he did not brake.
  The stop begun at 8 m passes 100 % at pro. He would know by the defender in front at five stick lengths; the HUD says it once (`UpdatePrompts`, the
  brake prompt), and nothing marks the distance on the ice (REVIEW-2 item B).
- **11 % had no room** (the hitter inside 7 m a second earlier, 8.8 m/s of closing): a pass was open in three of four of them; the spin does not help
  against a body (it lowers stability).
- **4 % are a stop begun too late** (inside 6 m: REVIEW-2's "a late stop loses the feet").
- Without the puck 3 %, unseen from behind 1 %: nothing to learn there.

So about **80 % of the time he lies down is avoidable in principle by a decision a second earlier, and for two thirds of it the game shows him nothing
that asks for the decision**. The cheapest help is a cue, not a rule (section 9, the track's next evolutions, and one line for the contact track: a hit
on a skater inside its first 15 frames with the puck is where the 131-frame falls are; capping that one case at a stumble would take up to 6 points of
play off the ice, and it is the contact track's rule to weigh against law 1).

## 5. For the operator's sheet (Space as it is on this main, and what the cue shows)

> **Space pokes: with your stick aimed roughly at the puck (within 60 degrees) and the puck inside your stick's reach, it takes a carried puck, unless the
> carrier has just won it (half a second), has his body between you and the puck, or is sweeping or spinning; a miss no longer trips you: your stick is
> locked for a third of a second (Space and RMB do nothing) and you skate on.**
> **The ring under a carrier turns yellow and closes while no stick can take his puck (yours does the same when you win it), so press when it has closed;
> STICK LOCKED at the bottom of the screen is the third of a second after your miss, JUST WON IT a press the grace refused.**

`docs/PLAY-ME.md:53` (the Director's file) still says Space rarely takes the puck and that a miss trips you: both are out of date since lane A152.

## 6. What is proven how, and what is not

| Claim | Proof | Not proven |
|---|---|---|
| the ring turns yellow and closes while a grace runs | `Jam.Hud.CarrierCues` (the scale, on a staged sim); the HUD's `JAMHUD cue=take_grace ... shown=1 scale=1.50` lines and three stills from an off-screen run (`docs/experiments/movement-7/cue-*.png`: a mate's ring at frame 140, the human's at frame 300, the human's ring before at frame 280) | the closing seen frame by frame (stills every 20 frames) |
| STICK LOCKED stands for the lock's 20 frames | `Jam.Hud.CarrierCues` (the words and the 20 frames on a staged miss); the pawn's `JAMVIS sticklock on=1 frames=20` / `on=0` 20 frames later in the real game mode, headless | **the line on a still**: the off-screen run reached 360 sim frames and no miss fell in them. The acceptance lane reads it in the window (`jam.ScriptHumanPoke 45`) |
| JUST WON IT on a press into a grace | `Jam.Hud.CarrierCues` (the words, the order) | the line on a still, and the moment in a match (the scripted human of the game mode pressed into no grace in the captured frames) |
| nothing of the cue reaches the sim | the cue files hold no sim code; the default match on the final tree is build 1's hash for hash (section 3) | |

Two acceptance rows proposed (`HokejAcceptDrill.cpp`, the acceptance lane's): (1) with `jam.ScriptHumanPoke 45`, a `JAMVIS sticklock on=1` line is followed
by `JAMHUD cue=stick_lock contact_line=STICK_LOCKED` within two frames and by `on=0` 20 frames later; (2) every `JAMVIS takegrace ... human=1 on=1` has a
`JAMHUD cue=take_grace ... ring=YouRing shown=1` on the same frame.

## 7. Findings outside the family

- `contracts/skater-visual.md` (C-0001, `PlayPoke`) and `Game/Source/JamCore/Public/JamSkaterVisual.h:93`: "false = mistimed, a stumble follows" is no longer
  true for a person (the lock, lane A152). Proposed text: "false = the window ran out: a bot stumbles for `stick.stumble_frames`; a person's stick is locked
  for `stick.miss_recovery_frames` and it skates on". And the two calls asked of device B in section 1.
- `Game/Source/JamGame/Private/HUDWidget.cpp:1476` (`UpdateYours`): warning C4701, `OldHead` may be used uninitialised (the screen track's; it was there before this lane).
- The body takes what the stick is refused (section 2: with the take grace at 45 frames the stick ends 23 % fewer of his carries and the check 52 % more; he
  lies down 9.6 % of play against 8.7 %). Any rule that shields the puck from sticks at pro is paid in knockdowns unless the brain's presser is told the
  difference (`opp.hk_*`: the brain track's and the curve's).
- `Game/Source/JamCore/AI/JamPlaytester.cpp` (the instrument): the regular player lets the puck go by his own shot or pass in about half of his carries, most
  of them inside a second (section 2). The carry's median at pro is first of all this script's patience: a bar on the median reads the script before it
  reads the game. A bar that reads the game: the share of carries LOST inside a second, and the median of the carries that were lost.
- `docs/PLAY-ME.md:53`: section 5.

## 8. Tests

In the gate (the full `Jam` filter: **251 pass, 0 fail** on the final tree; 249 on the main it stands on):
- `Jam.Hud.CarrierCues` (section 1).
- `Jam.Core.Movement.StopGrace`: the key is 0 when absent; key 0: the stop earns nothing and the defender's stick is not refused; key 30: a person's stop
  begun at 8.17 m/s gives grace 30 as its first frame ends, the stick in front is refused on that frame, the grace runs 30 frames; a reverse begun under
  6 m/s earns nothing; a bot's stop earns nothing (with the arm: 30); with `take_grace_frames 0` nothing; pro seed 3: shipped = key 0, key 30 differs.
Outside the gate: `Bar.Movement.CarryTrace` (sections 2 to 4; its `set` takes `stick.stop_grace_frames`, `stop_grace_bots`, `burst_take_frames`,
`burst_take_bots`, `pickup_settle_frames`, `pickup_settle_bots`, `stick.take_grace_frames`, `stick.poke_assist_deg`, `stick.miss_recovery_frames`,
`contact.down_frames_max`). `node --test schema/*.test.mjs`: 99 pass. The Shipping target compiles (`tools/run/build.ps1 -Project Game -Target Game -Config Shipping`, on the cue tree and again on the final tree).

## 9. Next evolutions (ranked)

1. **The pickup settles, for every skater, as a key, with the curve's re-set** (`game.stick.pickup_settle_frames`; the code is in the tree at 0:
   `FJamStickConfig::PickupSettleFrames`, `bPickupSettleBots`). A puck met at more than 4 m/s against the body is touched and slides on: half of the
   human's short touches and 8.5 % of his carries. Proof: the puck on nobody's blade at pro 49.7 % -> 45 % or less (the arm read 43.2 %), the carries that
   leave the blade with no cause 8.5 % -> under 4 % (3.7 %), possessions under 20 frames halved; and the three cells back inside 3 points of 85 / 48 / 26
   after the curve lane's numbers (the arm alone reads pro 63 -> 38 of 100 with the default instrument: it cannot land without them).
2. **The contested puck: a cue, and the arrival.** 65 % of his knockdowns (6.1 of the 9.1 % of play he lies down) fall within half a second of his first
   touch, at 16 m/s of closing. A mark on a loose puck an opponent will reach head-on within a quarter second of him (the track's HUD function, one
   element), and the instrument braking for it, so the number can be read. Proof: knockdowns "as he won it" 4.4 a match -> under 2.5; he lies down under
   6.5 % of play at pro; cells inside 2. (The rule beside it is the contact track's: a hit inside a carry's first 15 frames capped at a stumble.)
3. **The stop's distance on the ice.** 15 % of his knockdowns are law 3's own case with the stop not used (the hitter 11 m ahead a second earlier), and
   the stop saves only one lost carry in twenty because the taker is seldom ahead at a distance. A mark at five stick lengths in front of the carrying
   human that lights when a closing defender crosses it (presentation, the same function as section 1's cues), read with drill 21 script 2. Proof: the
   knockdowns of that class 1.03 a match -> under 0.5 with the instrument braking on the mark; the share of first tries in the drill that begin inside
   the 7 to 11 m window, before and after.
