# BRAIN-2: the brain plays hockey with the human (lane A115, track Q2, evolution 2; Fri 13 Nov 20:11-22:10)

Follows `BRAIN.md` (lane A94, the hockey sense) and `REVIEW-CODE.md` (lane A105, evolution 1). Source: `Game/Source/JamCore/AI/JamAICore.h/.cpp`,
`JamSimAdapter.cpp` (the hook), tests `Game/Source/JamCore/Tests/HockeyBrain2Tests.cpp` (`Jam.Bot.Hockey2.*`). Raw logs are under
`Game/logs/a115/` (they do not land), the measuring scripts under `tools/perf/results/a115/`; the cells are in `tools/perf/results/a95/cells.jsonl` (labels `A115-*`, `A115x-*`).

## 1. What was measured before (the built evolution 1, 100 seeds a rink, the regular class at pro, one traced process a rink)

| | hockey.arena.large | hockey.regular |
|---|---|---|
| matches with a 30 s stretch without a human touch | 67 % | 7 % |
| in the long stretches: opponents / a mate / loose | 71 / 2 / 24 % | 38 / 8 / 51 % |
| bot reverse brakes a match | 403 | 2,225 |
| bot carrier sprint frames a match | 0 | 0 |
| passes received by the human a match | 2.0 | 8.4 |

Two things the numbers did not say until the trace was read:

- **The big rinks are three a side.** `hockey.arena.large` (and wide, and take-7) play player + chaser + back against chaser + support + back. The shape
  jobs of A94 need five skaters (`AssignJobs`), so none of the hockey sense's shapes exist there. The human forechecks an opposing carrier for 4,586 of 12,247
  frames in seed 1 while **both of his mates stand off**: the yield rule (`Decide`, `bYield`) read only "the human is nearer the puck and within 15 m" and
  never asked whose puck it was. That is the 71 %.
- **The human's mates have no stick verb.** `FJamAIParams::Mate` is the default `FJamBotSkill`: `PokeFrames -1`, `bLift false`, `CheckMinClosing -1`. A mate
  won a puck only by the unpressed blade (the passive take). Track F2's `game.stick.passive_take false` ends the passive take: without a poke the human's
  side could never take a carried puck again.

## 2. The rules (each a field of `FJamHockeySense`; off = the brain of evolution 1; `ProHockeySense()` turns them on through `SetEvolution2`)

| field | rule | where |
|---|---|---|
| `bPassRoom` | A mate passes to the human only where he has room: the nearest standing opponent 3 m from him or more (2 m when he is set up for the one-timer). When he is open and ahead of the play (2 m up the ice from the puck, or better placed) and the sim's own pass would go to another teammate, the carrier turns to him and gives it by the aimed tap. | `Attack` |
| `bSupportHuman` | The yield is for a loose puck and the human's own carry: nobody yields an opposing carrier. When the human carries, a mate offers a lane: `OfferLane` picks the best of six spots beside and ahead of him (lane open by 2 m, free ice, how far up, how near); the close support of a five uses it too. | `Decide`, `JobSteer`, `OfferLane` |
| `bChaseAhead` | The loose puck's chaser is the skater that reaches where the puck will be first (`LoosePuckOwner`: steps of a tenth of a second up to two seconds, the puck slowing as under `puck.drag` 0.5). The last toucher of a puck that leaves it faster than 6 m/s starts a second late: the passer is not the chaser of its own pass. No mate yields to a human who is down or in the box. | `AssignJobs`, `Decide`, `LoosePuckOwner` |
| `bBrakeMeant` | A reversal of the input against the velocity is the sim's hard stop only where the brain decided one: on its spot (inside the slow radius: the post, the blue line), turning back for a loose puck within 6 m, the carrier's stop (below). A decided stop is held straight against the velocity until the skater is under 1 m/s (one stop, one count), and the next waits 45 frames. Every other reversal is carved: the input 115 degrees off the velocity on the input's side (the playtester's rule of A95), towards the middle of the rink when it was straight back. | `Think` |
| `bStopCut` | (with `bBrakeMeant`) A carrier that runs at more than 5 m/s into a defender inside 4 m on its lane, with no pass on, stops before it cuts 55 degrees (stop, cut, go: the cut then starts with the sim's first-step burst). | `Attack`, `Think` |
| `bCarrySprint` | A bot carrier sprints while its lane is open: nobody on the lane 15 m ahead of its way, nobody within 6 m, no shot or pass in hand. `game.ai.sprint 0` stops it like every bot sprint. | `Think` |
| `MateStickFrames` (10) | The human's mates poke an opposing carrier once its puck has sat 10 frames inside the blade's reach, and lift at contact at most every 90 frames (pro: 6 frames, every 90). -1 = never, as before. The opposing side is untouched. | `StickPlay` |
| `bGapControl` (**off as shipped**) | The presser goal-side of a carrier that comes at it faster than 3 m/s, 3.5 to 12 m off, stops and waits on the line carrier -> net, facing it, creeping; inside 3.5 m it steps up. See section 5. | `JobSteer` |
| `bReadShield` | The hooks for F2's body shield and take grace: no poke or lift the sim would refuse. See section 6. | `StickPlay`, `Attack`, `StickCannotTake` |

Switches: the profile tune `brain2=0` (both sides, every rule off) or one rule (`opp.hk_pass_room=0`, `mate.hk_support_human=0`, `hk_chase_ahead`, `hk_brake_meant`,
`hk_stop_cut`, `hk_carry_sprint`, `hk_gap_control`, `hk_read_shield`, `mate.hk_mate_stick_frames=-1`); for a measurement without a tune, the environment
`JAM_BRAIN2_OFF=all` or letters `p s c b x r g h m`.

## 3. The numbers after (the same 100 seeds, the shipping configuration)

| | large before | large after | regular before | regular after | the bar |
|---|---|---|---|---|---|
| matches with a 30 s no-touch stretch | 67 % | **25 %** | 7 % | **3 %** | under 35 %; not above 6 %: **met** |
| longest stretch, p50 / p90 | 37 / 60 s | 25 / 39 s | 19 / 29 s | 17 / 26 s | |
| in the long stretches (p80-p95): opponents / a mate / loose | 71 / 2 / 24 | 57 / **6** / 33 | 38 / 8 / 51 | 32 / 4 / 61 | a mate 15 %: **not met** |
| bot reverse brakes a match | 403 | 159 | 2,225 | **334** (cell of 600: 336) | under 500 at pro: **met** |
| bot carrier sprint frames a match | 0 | 796 | 0 | 780 | (REVIEW-1: 120 or more) |
| passes received by the human, a match (a minute) | 2.0 (0.57) | 3.7 (1.06) | 8.4 (1.66) | 7.7 (2.14) | before and after |
| match frames | 12,546 | 12,513 | 18,191 | 12,942 | |
| human touches a minute of match | 10.5 | 13.8 | 16.0 | 16.7 | |

**The mate's share is not reached, and it is not a brain number today.** Over every frame 10 s or more after the human's last touch (`poss.mjs`): a mate
has the puck 3 -> 4 % (large) and 6 -> 5 % (regular); over all play a mate carries 3 to 5 % in both arms, the human 10 to 13 %. The puck is loose in 42 to 68 %
of those frames (R-0048: nobody keeps the puck), and a mate that wins it gives it to an open human inside 0.2 s (`MinCarryFrames` 12), which is what the track
wanted of it. The stretches themselves shrank (the opponents' share of them 71 -> 57 %), and what the mates win goes to the human at once (passes received
+85 % on the large rink). To be measured again on F2's `passive_take false`, where a carry lasts.

**After a mate's pass** (REVIEW-1 item 6; `docs/design/review-1/humancarry.mjs` on 30 traced seeds an arm, hockey.regular at pro, about 145 such carries an
arm): the puck still the human's two seconds later 14 -> **6 %** (the bar was 30 %: **not met**, and it moved the wrong way); one second later 44 -> 28 %;
the carry's median 0.88 -> 0.72 s; but it ends lost in 64 -> **52 %** of the cases: with a mate standing on an open lane he passes on sooner instead of
losing it. By rule: without `bSupportHuman` 12 % held, median 0.90 s, 58 % lost (evolution 1's carry back); without `bChaseAhead` 9 %; without `bPassRoom`
6 % (the room rule is not the cause). "Held two seconds" stays out of reach while any carry lasts 0.7 s (R-0048): to re-measure on F2's rule.

Each rule alone switched off (100 seeds; the share of matches with a 30 s stretch, large / regular): all on 25 / 3; without the carrier's sprint 33 / 1;
without the mates' stick 32 / 2; without the brake rule 41 / 4 (and 2,188 reverse brakes, 17,287 frames a match on regular); without chase ahead 39 / 3;
without support 44 / 1; without pass room 25 / 2.

## 4. The three target cells (600 each, `curve.mjs`, hockey.regular) and what moved them

| cell | R-0046 | evolution 1 (A105) | evolution 2 | off by |
|---|---|---|---|---|
| novice at rookie | 85.3 | 84.5 | **80.3** (77.0..83.3) | -5.0 |
| regular at pro | 48.3 | 47.2 | **43.3** (39.4..47.3); the final build again, no tune: 43.3 | -5.0 |
| good at all_star | 26.3 | 25.7 | **16.5** (13.7..19.7) | -9.8 |

All three went **down**: the brief expected the human's side to gain; on the regular rink the opposing side gains more. Per rule at pro (600 each): without
`bChaseAhead` 52.5 (48.5..56.5), so the chaser picked by arrival costs the human about 9 points (the opposing five reach loose pucks first, and 60 % of play
is a loose puck); without `bSupportHuman` 48.8 (44.9..52.8), about 5 points (with the human's median carry under a second, the close support that went
ahead to offer a lane is no longer behind him when he loses it). On the large rink (not a target cell; 100 seeds) the same rules read the other way: the
human wins 56 % with the mates' stick and 35 % without it. Nothing was tuned: the curve track re-sets the ladder on F2's rule anyway. The neutral value
(`brain2=0`, or `SetEvolution2(H, false)` in `ProHockeySense`) gives back evolution 1 hash for hash (section 7).

## 5. Gap control: built, measured, not switched on

Lane A102 found the pro's "gap control" arriving head-on at 11 m/s, beaten by a stop-and-go 200 times of 200. "Back off at the carrier's speed" cannot be
played in this sim: a body faces its move, nobody skates backwards, so a presser matching the carrier's velocity turns its back to the puck and its lift
cone (70 degrees) away. What the sim can do: goal-side of a carrier that comes at it, the presser makes a decided stop and waits on the line to the net,
facing the carrier (standing it is braced, law 1, and its momentum is not committed), and steps up inside 3.5 m. Measured (100 seeds, pro): the large rink
is unchanged to the digit (three a side has no Press job), the regular rink 3 % idle, 345 brakes, 41 wins of 100 against 39: no gain that 100 seeds can
show. It ships off (`SetEvolution2` leaves it false). It needs A102's carry drill as its proof, which is not in this tree yet: the first "next".

## 6. The two hooks for track F2: the body shield (lane A102) and the take grace (lane A121)

Neither rule is in this tree, so nothing fills the two world fields; with them empty the brain is unchanged, bit for bit.

- **Body shield** (`game.stick.body_shield`): `FJamAIWorld::bBodyShield` and `BodyShields(W, Poker, Carrier)`, the sim's `IsBodyShielded` on the brain's world
  (within 2 m, the puck within 60 degrees of the line defender -> carrier carried on, the defender more than 60 degrees off the carrier's front).
- **Take grace** (`game.stick.take_grace_frames`): `FJamAISkater::TakeGraceFrames`, the frames left in which a skater that has just won the puck cannot be
  poked or lifted (`FJamSkaterState::TakeGraceLeft`).
- **One read for both:** `StickCannotTake(W, Poker, Carrier)`: "this carrier's puck cannot be taken by a stick right now". With `bReadShield` on, a bot
  presses neither poke nor lift then (`StickPlay`; the scripted `FJamPlayerBot` likewise), and `Attack` decides instead: **round the body** when the body
  shields (the point 2 m beside the carrier on the bot's nearer side, half a metre ahead, where the puck is in the open), **close and contain** during the
  grace (the point 1.8 m goal-side of the carrier, at stick length, so the poke or the lift is there the frame the grace ends). The body check is never
  refused by either rule and goes when its speeds are met.
- Proof on a pure world: `Jam.Bot.Hockey2.ShieldHook` (behind a carrier: lifts today; shield unread: the refused lift; read: no lift, the way goes round)
  and `Jam.Bot.Hockey2.TakeGraceHook` (in front at lift range: lifts today; grace unread: the refused swing; read: no lift, the way goes back goal-side).

**What the Director wires at landing** (`Game/Source/JamCore/AI/JamSimAdapter.cpp`, `BuildWorld`; both are there, commented, under "HOOK (lane A115"):

```
W.bBodyShield = G.Stick.bBodyShield;                 // after W.SweepBotReactFrames (with the three constants on the next line)
O.TakeGraceFrames = K.TakeGraceLeft;                 // in the skater loop, after O.bDown
```

Then the proofs are F2's own: the pro's refused lifts in 200 clean carries (158 today), and the share of refused swings among the bots' lifts (nine in ten
with the take grace). The human-like playtester needs the same read and is not this track's file: `JamPlaytester.cpp:475-476`.

## 7. Tests and the gate

`Jam.Bot.Hockey2.*` plays both arms in one run; the "before" arm (every field off) must miss the bar:

| test | before (evolution 1's brain) | after |
|---|---|---|
| `PassRoom` | passes onto a marked blade (an opponent 2.3 m off); the carrier's way towards an open human ahead: cosine 0.32 | no pass; with 6 m free the pass goes; cosine 1.00, and lined up the tap goes |
| `SupportsHuman` | the mate stands off the carrier the human forechecks (the support's place); the human carries: the mate drops back | the mate goes at the carrier; it skates to an offered spot whose lane is open by 2 m, beside or ahead |
| `LoosePuck` | the passer has the puck job of its own 20 m pass (five and three a side); the mate holds off a puck 1.5 m from the downed human | the receiver has it, the passer takes the support's place; the mate takes the puck |
| `BrakeOnPurpose` | a chase reverses straight against 8 m/s (the sim brakes); ten bots, 2 x 3,600 frames at pro: 922 reverse brakes, 0 carrier sprint frames | carved to 115 degrees (no brake); a loose puck 4 m behind is a decided, held stop; 235 reverse brakes (234 of them decided stops; 9,888 reversal frames carved), 726 sprint frames |
| `ShieldHook` | shield unread: the bot lifts into the body | shield read: no lift, the way goes round (sideways share 0.74) |
| `TakeGraceHook` | grace unread: the bot swings the lift the sim refuses | grace read: no lift, it contains goal-side of the carrier |
| `Neutral` | hockey.regular and hockey.arena.large, seeds 1 and 2, every field off: evolution 1's hashes `a31f6bc64fef07f3`, `1e3bb78feb1e30ad`, `6cbdb935ec9cd348`, `c23153964bdb9614` | evolution 2 plays other matches |

Neutral at volume: `-aitune=brain2=0`, 100 seeds on each rink: **200 of 200 hashes equal** to the logs of the build before any edit.
The rules untouched: in the full `Jam` run every scene without bots has evolution 1's hash (the collision scene `b4a833e559e045ca`, hockey.regular seed 1
frame 600 `d192973ac7c97102` and `f3613b5fcd33ce32`, hockey.raw `b02f67a2da0314b4`, the twelve contact scenes, the ability scenes), and the pinned
older-profile matches hold (`874f67830b20dc4f`, `59762ff73c73f6e3`, `1a33b22080d0f0df`): `PreA95Tune` now starts with `brain2=0`.

**The full `Jam` filter on the final build (21:44, one headless editor, `-Multiprocess`): 166 of 166.** (Evolution 1: 160. The first run, on the first
build, was 163 of 165: this lane's own bar, and `Jam.Core.ModeKeys.Sprint`, where `game.ai.sprint 0` did not stop the new carrier sprint: answered in the
brain, section 8 has the test's other line.) The final build's default brain against the first build with the gap-off tune, 100 seeds a rink: 200 of 200
hashes equal, so every number in sections 3 and 4 is the final tree's.

## 8. Outside the family (three edits the gate needed)

- `Game/Source/JamCore/AI/DifficultyProfile.cpp`: the tune keys above; `PreA95Tune` strings start with `brain2=0` (the neutral pins replay an older brain).
- `Game/Source/JamCore/Private/Tests/JamMovementTests.cpp` (`Jam.Core.Movement.BotMatchPro`): "the brain's carriers never sprint" is now "fewer sprinting than
  skating carry frames".
- `Game/Source/JamCore/AI/ModeKeysTests.cpp` (`Jam.Core.ModeKeys.Sprint`): "the human sprints only with the puck" was an equality that held on one match
  (the playtester's carry test and the sim's carrier differ at a carry's edge: 20 against 16 frames on evolution 2's match): now "at least half of his sprint
  frames with the puck, and under half of what `-playersprint` gives".

## 9. How to repeat

```
sh tools/perf/results/a115/pair.sh <tag> [-aitune=...]        # both rinks, 100 seeds, traced; then agg.mjs and tools/soak/playtest/idle.mjs
node tools/perf/results/a115/poss.mjs <traced log>            # who has the puck, all play and 10 s after the human's last touch
node tools/perf/results/a115/hashcmp.mjs <log A> <log B>      # per-seed hashes
node tools/perf/results/a95/curve.mjs A115-pro-regular regular pro regular 600
sh tools/perf/results/a115/tests.sh Jam <tag>                 # the gate's filter, headless, -Multiprocess
```
