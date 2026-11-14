# Contact 7: nobody stands in a man who is down, and a check is a commitment (lane A157, track F1, evolution 6; 2026-11-14)

On main `a7d89871` (the fifth evolution, the poke and the brain's one stop distance on it). Four things: **a push-out around a downed skater** (B's critic:
an opponent stood in the downed human for 24 frames), **a cooldown on the check press** (RMB every 12 frames holds the window open), **the lunge as a
motion** (a press that commits the body, and a miss that costs), and **the board slam's read**. Every rule is behind a cartridge key at its neutral
value: the default match is hash-identical to `a7d89871`. Status with timestamps: `status/A157.md`. Raw: `Game/logs/a157*`,
`tools/perf/results/a157/raw/` (not landed).

Code: `Game/Source/JamCore/Private/JamSim.cpp`, `Public/JamSim.h` (`PushOutOfDowned` and its branch in `CollideSkaters`; the cooldown and the lunge in the
press, `StepSkater`, `ContactImpulse`, `ResolveContact`, `StateHash`; two state fields, three read-outs), `Public/JamCartridge.h`, `Private/JamCartridge.cpp`
(four keys), `Public/JamBaseline.h` (their neutral lines), `AI/JamPlaytester.cpp/.h` (read-outs on the playtest line; the masher, measurement only), tests
in `JamCore/Private/Tests/JamContactTests.cpp`, `JamNet/Private/Tests/JamNetGuestTests.cpp` (the read-out list), `schema/` (four rows in each of the four
files), the two generated copies of the schema (`JamSchemaData.inl`, `JamDesignerData.inl`).

## 0. What ships, in one table

Cells: novice at rookie / regular at pro / good at all_star on `hockey.regular`, seeds 1 to 600, one process a cell, paired on the seed
(`tools/perf/results/a157/cells.mjs`, `pair.mjs`, `arms1.mjs`, `read.mjs`; rows in `cells.jsonl`). The tree as handed over (`base`): **87.0 / 54.7 / 29.8** with
the shipped press, **86.5 / 53.2 / 30.0** with the ladder's instrument (`JAM_PLAYTESTER_CHECK=closing`, R-0061).

| Rule | Key (all under `game.contact`) | Ships | Recommended value | What the value costs: rookie / pro / all_star |
|---|---|---|---|---|
| Nobody stands in a man who is down | `downed_radius_m` | **0 (neutral)** | **0.6** (roots kept 1.0 m apart) | **-1.0 / +1.0 / +8.4** at 600 seeds; all_star **+4.0** at 1,200 seeds (+8.4 on the first 600, -0.3 on the next 600; standard error 1.9) |
| A check press has a cooldown | `check_cooldown_frames` | **0 (neutral)** | **36** with the lunge, **60** without it | 36: **-0.2 / +0.3 / -0.1**; 60: **-0.7 / +1.0 / -1.0** |
| The lunge as a motion | `lunge_frames`, `lunge_step_mps` | **0, 0 (neutral)** | **12 and 1.5** | **-1.2 / +1.5 / +5.0** at 600 seeds; all_star **+3.3** at 1,200 seeds (+5.0 and +1.5; standard error 1.9) |
| Cooldown 36 and the lunge together | | | | **-1.2 / +1.5 / +5.0** |
| The three together (0.6, 36, 12 and 1.5) | | | | **+0.5 / +1.3 / +2.9** (87.5 / 56.0 / 32.7; 600 seeds) |
| Read-outs on the playtest line (`downed_*`, `hits_*`, `checks_cooling`, `lunges_missed`, `bot_*`) | none | on | | none: 600 of 600 matches a cell hash as before |

**What may go on, and what waits for the curve.** At the defaults nothing moves: the default match is hash-identical to `a7d89871` (section 6). The
cooldown moves no cell by more than a point at 36 or at 60 and could go on by itself. The push-out and the lunge each lift the human at all_star by three
to four points at 1,200 seeds: at the edge of the four-point rule and well over the one point the brief allows for landing on at once. They wait for the
curve lane's next pass, which can buy the points back with one all_star number, or for the operator's word at 10:00. Any of them moves the default hash the
moment it is on, so none goes on under a ladder that is being measured.

## 1. Nobody stands in a man who is down (item 1; B-to-A-0041 point 2)

**What the still shows, and what the sim does.** B's critic: an opponent skated into the downed human and stood in him for 24 frames
(`devlog/shots/20261114-004300-B-sk24-lying-loop-play.png`, cells f30 to f54). In the sim a downed skater is the same 0.4 m body circle as a standing one
(`UJamSim::CollideSkaters`): roots are kept 0.8 m apart, the overlap is split half and half, and the two bounce as equals. So a skater who keeps skating at a
man on the ice stops 0.8 m from his root, which is on the lying figure (it is longer than its circle), and **pushes him along the ice**. Staged
(`Jam.Core.Contact.DownedPushOut`: a hit from behind at 9 m/s, down 113 frames, the skater keeps skating at him): root to root exactly 0.800 m on 112 of
112 frames, and the downed man ends **7.35 m** from where his own slide would have left him.

**B's bar as written already holds on the tree.** No root within 0.6 m of a downed skater's for more than 6 frames: 0 spells in 600 matches in each cell,
not one frame. The fault the eye sees is one step out: a root within 0.9 m (at the body circle) for more than 6 frames. That happens **7.8 times a pro
match** (13.6 per 300 s; in 198 of 200 matches; the longest 128 frames).

**The rule** (`game.contact.downed_radius_m`, 0..2, default 0 = neutral; `UJamSim::PushOutOfDowned`). With the key above 0 a downed skater's body has that
radius for every skater on its feet: roots are kept 0.4 m + the key apart (never less than the two circles: 0.8 m). For a pair of which exactly one is down
the ordinary collision is replaced:
- the skater on its feet alone gives way, along the line between the two roots, at 6 m/s at most (a skater already inside when the other fell is eased
  out over a frame or two, not thrown);
- the speed it had into the body, measured against the downed man's own slide, turns along the body at 0.8 (what it had along the body it keeps; it never
  leaves with more speed than it came with), on the side it was already passing; its facing decides a dead-centre course. A skater steering into a downed
  man goes round him;
- the downed skater's position and velocity are never written: **no impulse to the downed man**; the puck is not touched and sees the same 0.4 m body as before;
- two downed skaters collide as before, and so does the hit that fells a skater (neither is down when that contact is read);
- deterministic (arithmetic on the two states in the pair order of `CollideSkaters`); no rule state (one read-out, `DownedPushFrames`).

Staged at 0.6 (keep-out 1.0 m): closest 0.919 m on the frame after the hit, eased out by the next; 0 frames within 0.9 m; the downed man's path is his own
slide to within 1e-9 m; the skater who keeps his course goes round and is 8.3 m past him when he gets up, never under 2.7 m/s. At 0.2 (under the body's own
radius) the two circles are kept apart as today and the downed man is still not pushed.

**The bar and the picture, 200 matches a cell** (`node tools/perf/results/a157/read.mjs --first=200 downed off3 on06`):

| 200 matches | rookie | pro | all_star |
|---|---|---|---|
| root within 0.6 m for more than 6 frames (B's bar), key off | 0 spells | 0 | 0 |
| the same, key 0.6 | 0 (longest 3 frames) | 0 (longest 1) | 0 (longest 6) |
| root within 0.9 m for more than 6 frames (standing on the figure), key off | 763 spells (3.8 a match), longest 81 frames | **1,554 (7.8 a match)**, longest 128 | 1,235 (6.2 a match), longest 120 |
| the same, key 0.6 | 1 | **3** | 5 |
| frames a match in which the push-out turned a course, key 0.6 | 59 | 106 | 84 |

The few frames inside 0.6 m with the key on (9 to 24 pair-frames in 600 matches) are a fast slider reaching a skater before the easing has moved him.
At 1.0 (keep-out 1.4 m) the easing takes six frames and one spell of 7 frames inside 0.6 m appeared in 200 pro matches: **0.6 is the value**, not more.

**The three cells, key on (0.6) against key off, 600 paired seeds:**

| cell | off | on | delta | flips (off only / on only) |
|---|---|---|---|---|
| novice at rookie | 87.0 | 86.0 | **-1.0** | 72 / 66 |
| regular at pro | 54.7 | 55.7 | **+1.0** | 141 / 147 |
| good at all_star | 29.8 | 38.2 | **+8.4** | 106 / 156 |

All_star moves on these seeds, so the rule ships at 0. More arms at all_star, 600 seeds each (the standard error of one paired difference there is about 2.7 points):

| arm at all_star | win rate | against its own off arm |
|---|---|---|
| 0.4 (the two circles as today, the downed man not pushed) | 33.0 | **+3.2** |
| 0.6 | 38.2 | **+8.4** |
| 0.6 with the ladder's instrument (`JAM_PLAYTESTER_CHECK=closing`; off 30.0) | 33.7 | **+3.7** |
| 1.0 | 33.2 | **+3.4** |

**All_star at 1,200 paired seeds** (`off12`, `on0612`; `node tools/perf/results/a157/pair1.mjs off12 on0612 as`): 31.3 against 35.3 = **+4.0** (standard error
1.9; flips 242 / 290). The first 600 seeds gave +8.4, the next 600 gave **-0.3** (32.7 against 32.3): the first read was a lucky half. The cost at all_star
is about four points, at the edge of the four-point rule, not eight.

Every arm is up: the effect is real and its sign is plain; its size is between 3 and 8 points. The 0.4 arm says where it comes from: not from the wider
body but from **the downed man no longer being pushed** (at 0.4 the distance is today's and only the push is gone). Why that lifts the human at all_star
is not established: the match line moves little (goals 1.60 : 2.47 to 1.72 : 2.33, the opponents' shots 17.5 to 16.7 a match). The guess to test: at
all_star the hunt takes every bot within 30 m to the puck, the human goes down 8.2 times a match, and the bots who skate through him carry him away from
the puck he just lost; with the rule he gets up where he fell. If so it is a fault corrected (a felled player bulldozed up to 7 m) that costs all_star's
bots an advantage the ladder was set with.

## 2. A check press has a cooldown (item 2; R-0060 point 4)

**The fault, measured.** A press opens a window (12 frames for the human, 6 for a bot) whenever it comes, and costs nothing. The scripted human with RMB
pressed every 12 frames on top of its own presses (`JAM_PLAYTESTER_MASH=12`, measurement only), 600 pro matches: **20.1 landed checks a match against 4.57**
for the timed script, and **59.3 % of matches won against 54.7 %**. Mashing is the better play today.

**The rule** (`game.contact.check_cooldown_frames`, 0..120, default 0 = neutral): for that many frames after a press that opened a window (or resolved as a
late press), the same skater's next check press is dropped: no window, no cue. Bots and the human alike. State: `FJamSkaterState::CheckReadyFrame` (hashed
behind 0); read-out `ChecksCooling`. Staged (`Jam.Core.Contact.CheckCooldown`): at 40, of a button mashed every 12 frames 2 presses in 7 open a window (24 of
80 frames open against 80 of 80), the bodies that meet between the two windows take no hit, and one press timed three frames before the contact lands at
either value.

**The masher under it, pro, 600 matches an arm** (`node tools/perf/results/a157/read.mjs press mash0 mash24 ...`):

| cooldown (frames) | 0 | 24 | 36 | 48 | 60 | 90 | the timed script, no cooldown |
|---|---|---|---|---|---|---|---|
| the masher's landed checks a match | 20.10 | 14.08 | 9.84 | 8.86 | 7.84 | 5.65 | **4.57** |
| the masher's presses that opened a window | 550 | 314 | 201 | 163 | 134 | 87 | 3.54 |
| the masher's win rate | 59.3 | 60.7 | 58.2 | 57.2 | **55.3** | 51.8 | 54.7 |
| the bots' presses dropped a match (of 14.9) | 0 | 0 | 0.03 | 0.11 | 0.18 | 0.40 | 0 |

**Verdict.** *No cooldown up to 90 frames makes a mashed button land no more checks than a timed one* (5.65 against 4.57 at 90; the line reaches the timed
count at about two seconds, which is not a check any more). The reason is in the rule's shape: a mashed window catches every body contact that falls into
it, slow ones too, and the scripted human only presses for a hit it sees coming at over 4 m/s. What a cooldown does buy is the result: **from 60 frames
on mashing wins no more matches than timing** (55.3 % against 55.7 % for the timed script under the same cooldown), and at 90 it loses three points.
The timed players do not feel it: the script spaces its presses 30 frames apart and the brain 30.

**The three cells, cooldown on against off, 600 paired seeds** (the shipped instrument; off 87.0 / 54.7 / 29.8):

| value | cells | delta | results that flipped (off only / on only) | presses dropped a match: human, bots |
|---|---|---|---|---|
| 36 | 86.8 / 55.0 / 29.7 | **-0.2 / +0.3 / -0.1** | 1/0, 3/5, 2/1 | 0.01 / 0.00 / 0.01, 0.00 / 0.04 / 0.02 |
| 60 | 86.3 / 55.7 / 28.8 | **-0.7 / +1.0 / -1.0** | 6/2, 11/17, 12/6 | 0.04 / 0.02 / 0.05, 0.01 / 0.18 / 0.09 |

## 3. The lunge as a motion (item 3)

**The rule** (`game.contact.lunge_frames` 0..30 and `game.contact.lunge_step_mps` 0..4, both default 0 = neutral; the name `lunge_mps` was taken by lane
A92's number in the impulse). A press that opens a window commits the body for `lunge_frames`:
- `lunge_step_mps` is added to its velocity along its facing at once: a standing press steps forward, a moving one surges;
- it turns at a quarter of its turn rate and cannot brake by reversing: the press holds its line;
- the step is real closing speed in the hit, so the impulse's own `lunge_mps` is reduced by it while the lunge runs: at 1.5 a head-on hit is the hit it was
  (staged: ratio 1.166 either way, one frame earlier);
- a lunge that ends in a body ends there; one that runs out on nobody is **the miss: the skater loses twice the step** (at 1.5: he ends 1.5 m/s slower
  than he pressed; a standing skater steps and stops);
- no second lunge starts inside one. State: `FJamSkaterState::LungeLeft` (hashed behind 0); read-out `LungesMissed`.

Staged (`Jam.Core.Contact.Lunge`, 12 / 1.5): standing: 1.50 m/s after the press, standing again 13 frames later; gliding at 6: 7.50 m/s, then 4.46 after
the miss (5.96 without the rule); turned 0.104 rad in 5 frames against 0.417.

**The figure call**: `PlayCheck` (`contracts/skater-visual.md`: "this skater threw a body check this frame") is the call that fits, and the pawn already makes
it on the press's frame (`bCheckThisFrame`); with the lunge on, that take now has a body that moves under it. Nothing new is called. No call in the
contract fits the miss (the recovery after a lunge that met nobody): a need for device B, named here, not called.

**The three cells, 12 frames / 1.5 m/s against off, 600 paired seeds** (off 87.0 / 54.7 / 29.8):

| arm | cells | delta | the human: landed, lunges missed a match (pro) | the bots: lunges missed a match (rookie / pro / all_star) |
|---|---|---|---|---|
| the lunge alone | 85.8 / 56.2 / 34.8 | **-1.2 / +1.5 / +5.0** | 4.39 (4.57 off), 0.41 | 0.53 / 2.63 / 2.10 |
| the lunge with cooldown 36 | 85.8 / 56.2 / 34.8 | **-1.2 / +1.5 / +5.0** | 4.40, 0.41 | 0.54 / 2.64 / 2.12 |

All_star at 1,200 paired seeds (`off12`, `lunge12`): 31.3 against 34.5 = **+3.3** (standard error 1.9; +5.0 on the first 600 seeds, +1.5 on the next 600).

Who pays: the bots. They press 14.7 times a match at pro and 2.6 of those lunges meet nobody (18 %); the scripted human presses 3.3 times and misses 0.4.
All_star moves (5.0 on these seeds, 3.3 at 1,200), so the lunge ships at 0: it is the curve's to turn on.

**The masher with the lunge on (pro, 600 matches):** no cooldown: 20.4 landed, 317 lunges missed a match, 56.3 % (59.3 without the lunge); **cooldown 36 + the
lunge: 9.85 landed, 204 lunges missed, 52.0 % against 56.2 % for the timed script under the same rules: mashing costs four points.** With both on, the
button is a decision.

## 4. The board slam as a play (item 4: the read, no rule changed)

**What the rule is today** (`UJamSim::BoardFactor`, lane A101; `game.contact.board_bonus` 1.25): the impulse of a hit counts x 1.25 when the victim's centre is
within 1.5 m of the boards (1.1 m of ice between his body and them) and the push runs into them: the whole bonus when the line from the attacker to the
victim is inside 60 degrees of straight at the boards, falling to nothing for a hit along them. It reads the direction of the hit, not the speed: there
is no threshold a player must beat, and nothing on screen says "that one was a slam" except the board sound and the figure's board take.

**What the bonus buys, from the contact law** (a carrier skating along the boards at 8 m/s, stability 430; the attacker's closing speed along the line between
the two bodies; the pressed check's 1.5 m/s is in):

| closing speed | open ice | into the boards |
|---|---|---|
| 1 m/s | 0.47 stumble | 0.58 stumble |
| 2 m/s | 0.65 stumble | **0.81 stumble, the puck comes loose** |
| 3 m/s | 0.84 stumble, puck loose | **1.05 down, 62 frames** |
| 4 m/s | 1.02 down, 61 frames | 1.28 down, 73 frames |
| 6 m/s | 1.40 down, 78 frames | 1.75 down, 94 frames |
| 8 m/s | 1.77 down, 95 frames | 2.21 down, 115 frames |
| 10 m/s | 2.14 down, 111 frames | 2.68 down, 135 frames |

So **what a player must do to earn it**: have the man between himself and the boards, inside a stick length of them, and drive at him across the ice,
not along the wall. What he gets: the man goes down from 2.8 m/s of closing where open ice needs 3.9, the puck pops from 1.5 m/s where open ice needs
2.3, and a knockdown lasts 12 to 24 frames longer (a fifth). It is the hit for a defender who has little speed: the angle does what the run-up does on open ice.

**Its read in a match today** (600 matches a cell on the tree as it is; `node tools/perf/results/a157/read.mjs hits off3`; a landed hit = a pressed check or
a cross-check that stumbled or felled):

| 600 matches a cell | rookie | pro | all_star |
|---|---|---|---|
| knockdowns where the boards answered, a match | 0.42 | **1.06** | 0.94 |
| knockdowns on open ice, a match | 7.26 | 12.67 | 13.64 |
| frames down, board hit | 111 | **118** | 129 |
| frames down, open-ice hit | 118 | **122** | 126 |
| the boards' share of all landed hits | 6.7 % | 8.0 % | 6.4 % |
| the human's own landed hits: at the boards / on open ice, a match | 0.44 / 3.92 | 0.76 / 3.81 | 0.58 / 3.71 |
| matches with a board knockdown (of 600) | 189 | 346 | 331 |

**Verdict: the board slam exists in the law and is not a play in the match.** One knockdown in twelve happens at the boards, and it does not last longer
than an open-ice one (118 against 122 frames at pro): the bonus does not make the hits at the boards bigger, it turns slower contacts into hits, so the
average board hit is a weaker hit that the wall lifted over the line. A player cannot see that he earned anything. To make it a play it needs a
threshold he can aim for and a reward he can see: the proposal is the track's next evolution (section 8, number 3). No rule changed here.

## 5. For the operator's sheet (item 5): what RMB does

**Now:** RMB opens a fifth of a second in which your body meeting an opponent's is a hit as hard as the speed you close at; the press itself moves
nothing and costs nothing, so a button tapped five times a second always has the window open and lands four times the hits of a timed press (20 a match
against 4.6).
**With the cooldown (36 frames) and the lunge (12 frames, 1.5 m/s) on:** RMB is a lunge: your body surges forward and holds its line for a fifth of a
second, a hit lands as hard as before, a miss leaves you 1.5 m/s slower, and the next press is taken 0.6 s after the last, so the player who times one
press wins more than the one who mashes (56 % against 52 % at pro).

## 6. The proof

- **The default match is hash-identical to `a7d89871`.** Build 1 = the tree as handed over; the final tree = build 4 with every key of the lane at its
  default (0). `off3` (builds 3 and 4 differ only in the Designer's generated copy of the schema) against `base`: **600 of 600 matches hash-identical in
  each of the three cells** (600 at pro, 600 at all_star); `offc` (the final build, the ladder's instrument) against `basec`: **600 of 600 in each cell**;
  `off4` (the final build, 100 seeds a cell): **100 of 100 in each cell**. 3,900 matches, no hash moved.
- **The gate**: the full `Jam` filter on the final tree **252 PASS, 0 FAIL** (249 on the handed-over tree; the three new tests are the difference:
  `Jam.Core.Contact.DownedPushOut`, `.CheckCooldown`, `.Lunge`). Of the 228 lines that carry a hash, 226 are identical before and after character for
  character and the other two (`Determinism`, a timing in their text) carry the same hashes. The 11 `LAW3` lines are identical: law 3's gate is untouched.
  `Jam.Net.HashCoverage` green: `CheckReadyFrame` and `LungeLeft` are hashed behind 0 (tags -1572, -1573), `DownedPushFrames`, `ChecksCooling`,
  `LungesMissed` are listed read-outs.
- **Both validators**: `node --test schema/**/*.test.mjs` 99 pass, 0 fail (limits mirror the schema); the cartridge round trip (`Jam.Core.Cartridge*`) and
  `Jam.Designer.PromptParity` green with the two embedded copies regenerated (`gen-schema-data.mjs`, `gen-designer-data.mjs`).
- **Neutral values**: `JamBaseline::MakeNeutral` carries the four keys at 0 (they ship at 0, so the lines change nothing today and hold the pins the
  day a default moves). No pinned hash of a shipped cut was added.
- Not run: the Shipping compile (the landing chain's step, R-0057). Every test of this lane is inside its file's automation guard; the rest is plain C++
  and reflected fields. Not run: the acceptance run (nothing a player can do changes at the defaults; R-0056 asks for it the day a key goes on).

## 7. Findings outside the family

1. `tools/barks/barks.test.mjs:64` fails in this worktree ("BarkData.inl has the byte array"): the gate's node step reads 243 pass, 1 fail. Not this lane's files.
2. A schema key has TWO embedded copies to regenerate: `node Game/Source/JamCore/Tools/gen-schema-data.mjs` and
   `node Game/Source/JamDesigner/Tools/gen-designer-data.mjs`. The first one's header names only itself; `Jam.Designer.PromptParity` is the only thing that
   says so (it cost this lane one gate run).
3. `Game/Source/JamCore/AI/JamAICore.cpp:1124`: the brain spends its own 30-frame `CheckCool` on a press the sim's cooldown drops. Under 30 frames nothing
   happens; at 60 the bots lose 0.18 presses a match of 14.9. The brain should read the skater's `CheckReadyFrame` before it presses (the brain track's).
4. A press the cooldown drops is silent on screen (no `PlayCheck`, no HUD line), and a lunge that misses has no figure call: `contracts/skater-visual.md`
   has `PlayCheck` and nothing for the recovery. For device B and the HUD lane the day the keys go on.
5. The name `game.contact.lunge_mps` (lane A92) is the 1.5 m/s the impulse adds for a pressed check, not a motion: the brief's `contact.lunge_mps` at
   neutral 0 could not take it; this lane's key is `lunge_step_mps`.
6. With the push-out on, the bots still steer AT a downed man and are turned by the rule on 106 frames a pro match: the brain steering round him
   (B's second option) would make it a choice instead of a deflection.

## 8. The next three evolutions for the track

1. **The downed body and the lunge go on with the curve.** All_star measured at 2,000 paired seeds for `downed_radius_m` 0.6 and for the lunge, one
   all_star profile number moved to buy the points back, the brain steering round a downed man. Proving numbers: every cell within 3 points of its target
   with the three keys on; spells of a root within 0.9 m of a downed skater's for more than 6 frames under 5 in 200 matches a cell (3 / 1 / 5 today with the
   key on, 1,554 at pro without); B's own bar on a package: frames with a capsule over a downed figure 0 in two 100 s matches.
2. **A mashed button lands nothing.** The cooldown as measured cannot bring the masher's landed checks under the timed player's (9.8 against 4.6 at 36).
   The rule that can: a press inside the cooldown starts the cooldown again (one line, one key): a button mashed faster than the cooldown never opens a
   window, a timed press never notices. Proving numbers: the masher's landed checks a match at pro under 4.6 (the timed script's) over 600 matches, the
   three cells within 1 point, and a HUD cue for the dropped press read in the window.
3. **The board slam as a play**: the slam is earned (the victim within 1 m of the boards, closing along the boards' normal at 4 m/s or more) and paid
   (`contact.board_slam_frames`: frames down on top; the `board` event with kind `slam` for the glass, the sound and the HUD). Proving numbers: frames down
   of a slam at least 20 above an open-ice hit (118 against 122 today: none), the human's board knockdowns a pro match from 0.76 to 1.5, the cells within 4.
