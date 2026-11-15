# The puck is seen to belong to someone (lane A210, track F3, wave 4, the night of 14 to 15 Nov, device A)

The operator after package H34 (R-0067 point 2): "Puck does not stick with players, it moves through them without locking". The sim's share is
lane A208's (a loose puck is his who touches it). This is the screen's share: what a player reads possession from. Presentation only: no sim rule,
no state field; the sim's hash is the same with and without (section 6). Code: `Game/Source/JamGame/Public/PuckRead.h`, `Private/PuckRead.cpp`
(`FJamPuckShow`), a few lines in `JamPawns.h/.cpp`, `HokejGameMode.h/.cpp`, `EffectsBinder.h/.cpp`.

## 1. What was wrong on the screen (measured on main 015f1618, drill 6 at rookie on clay, 4800 frames a run)

Device B reported (H55.6, `status/E3-report.md`) the figures told `SetHasPuck(true)` while the sim's carrier is somebody else, and the puck drawn on
the blade on 86 % of the human's and 18 to 46 % of the bots' carrying frames. Measured here with a probe once a sim frame (`jam.puck.Probe`):

| what | package H34's path | now |
|---|---|---|
| flagged frames that disagree with the sim's carrier of record | **0** of 1282 (human), 0 of 2272 (bots): B's flag finding is not reproduced since A208's pickup landed | 0 (human); 0 to 5 % (bots: the frames a drop-out is bridged) |
| **the blade's socket more than 1 cm from the drawn puck when the frame is drawn** | **54 to 66 % of carry frames** (mean 5.5 to 7 cm, worst 80 to 122 cm; over 10 cm on 13 to 22 %) | 0 outside a pickup's blend |
| the jump at a pickup | up to 2.9 m in one frame | 2 frames (a release cuts to the sim's puck at once: a loose puck is never drawn off the sim's) |
| the human's carried puck seen at his camera (twelve straight-carrying stills at 1280 x 720) | 2 of 12 | 10 of 12 |

**The lag is the fault.** The game mode placed the puck on the carrier's `puck_socket` BEFORE the frame's animation was evaluated: the socket it read
carried last frame's pose, so every stride and every stick move left the drawn puck behind the blade: the puck "does not stick".

## 2. What changed

1. **One shown carrier a frame** (`jam.puck.Show 1`): `FJamPuckShow::StepSim` runs after the sim's step and before the pawns apply it; it takes the
   sim's carrier of record (`UJamSim::CarrierIndex`), keeps him at most `jam.puck.Keep` 6 frames over a drop-out (the puck still his last touch, he
   can act, no shot or pass, the puck within 1.35 of his reach; never while down or hit), and tells every pawn (`AJamSkater::SetShownCarrier`): the
   figure's `SetHasPuck` and the drawn puck follow the same answer.
2. **The drawn place** (`FJamPuckShow::Render`): on the shown carrier's socket, else exactly where the sim has it; a pickup runs the offset out over
   2 frames instead of jumping; a release, a loss and a goal cut to the sim's puck on their frame (PUCK READY 2: the offset is for a carried puck only).
   A man who shot or passed is not bridged for 30 frames. After a goal the sim's puck keeps the height it crossed the line at; the drawn one falls to
   the ice in 0.17 s (`FJamPuckShow::GoalDropCm`): `img/a210/net-after-goal-before-after.jpg`.
3. **Through the frame's animation** (`jam.puck.Follow 1`, `AJamPuck::SetFollow` / `Tick` in `TG_PostUpdateWork`): once on the blade the puck takes the
   socket's place again after the figures' poses of the frame are evaluated: 0 cm from the blade when the frame is drawn.
4. **The human's carried puck is not behind his hips** (`jam.puck.CarryAim 30`, `AJamSkater::CarryAimDeg`): section 3.
5. **Device B's play calls by name** (`jam.puck.Calls 1`): section 4.

6. **A loose puck is shown on a stick once the sim has named the man 2 frames running** (`jam.puck.SettleLoose 2`; `jam.puck.Settle` for a
   stick-to-stick change ships at 0): section 3.2.

`jam.puck.Show 0`, `jam.puck.Follow 0`, `jam.puck.CarryAim 0`, `jam.puck.Calls 0` = package H34's puck (the settle rules are off with `jam.puck.Show 0`).

## 3. The hidden puck (device B's finding 1, the eye's EYE 4: "he cannot see his own puck")

From his camera (6 m behind, 3.5 m up) the sight line to a puck 1.25 m straight ahead passes through his thighs at 0.6 m: the camera would need 1.8 m of
side offset or 8.7 m of height to show it. What presentation can move is the figure's stick: while the human carries, the aim sent to his figure
(`SetStickAim`) never comes closer to his line than a floor: 30 degrees on the backhand (the figure's right: its `puck_socket` is then 100 cm ahead
and 75 cm out), 45 on the forehand (left: 100 ahead, 70 out; at 30 it is only 48 cm out and stays behind the hip: measured, `JAMPUCK socket` lines).
From the middle it goes to the side the raw aim leans to and changes side only once the aim is clearly on the other. The sim's blade and puck are
not touched.

- Stills: `img/a210/carry-puck-seen-now.jpg` (twelve STRAIGHT-carrying stills at 1280 x 720: drill 21's three carries at 9 to 9.8 m/s, frames +12,
  +18, +24, +30 of each) against `carry-puck-hidden-h34.jpg` (the same sim frames): the puck shows beside his left skate in 10 of 12 (2 of 12
  before; the eye, with a hand that carries straight, counted 2 of 12 too). It is small: at skate level, half under the HUD's YOU ring.
- **The price**: the drawn puck against the sim's on his carry frames: mean 70 to 92 cm with the floor, 42 to 66 cm without; the worst is the same
  (174 to 276 cm, at pickups and in hard cuts, where B's figure brings its stick round slower than the sim's blade). About 28 cm more, on average,
  between the puck he sees and the puck a stick takes.

### 3.1 The cheaper lie, tried (the Director's ruling of 02:10: "the DRAWN puck no more than 45 cm from the sim's, the blade reaching to it")

The human's carry frames, 600-frame windows, drills 21 and 6 at rookie (builds 22 and 25; `jam.puck.MaxOff`, `jam.puck.CarrySide`):

| the drawn puck | seen in straight-carrying stills | on the blade (within 10 cm) | off the blade, mean | drawn to the sim's, mean |
|---|---|---|---|---|
| no floor (package H34's stick: `jam.puck.CarryAim 0`) | 2 of 12 | 100 % | 0 | 42 to 66 cm |
| the floor 30 / 45 (PUCK READY 1; ships) | 10 of 12 | 96 to 100 % | 0 to 4 cm | 70 to 92 cm |
| the floor + capped 45 cm from the sim's | **1 of 8** | 14 to 38 % | **23 to 46 cm** | 43 to 52 cm |
| the floor + capped 70 cm | not counted | 31 to 78 % | 9 to 27 cm | 56 to 72 cm |
| the floor by the side the sim's puck is on, no cap | not counted | 96 to 100 % | 0 to 4 cm | 66 to 95 cm |

**It fails the eye's bar, and the first row says why**: device B's figure holds its blade 42 to 66 cm from the sim's puck on average with no floor
at all. The figure's stick does not land where `SetStickAim(aim, reach)` points (sent 30 degrees / 1.2 m puts the socket at 22 degrees / 1.26 m; -30
at 37 degrees; -45 at 52), and the sim's carried puck floats up to 50 cm either side of the sim's own blade point as he strides. A puck kept within
45 cm of the sim's is therefore a puck 23 to 46 cm OFF the figure's blade (a stick out to the side with nothing on it) and behind his hips again.
Following the sim's puck from side to side made the stick change sides every half second to two seconds and did not bring the mean down.
Kept: the floor. The cap and the side rule stay in the code at 0 for a later try. What closes the gap is the blade where the aim says (device B).

**What a player meets at a change of hands, and because of the 70 to 92 cm** (the `JAMPUCK hands` lines: the distance the 2-frame blend runs out
at every change of hands on the screen; build 26):

| what happened (the human's; cm: mean / worst) | drill 6 (five a side, 6450 frames) | the play drill (five against two, 4070 frames) |
|---|---|---|
| he picks up a loose puck | 9: 114 / 163 | 5: 142 / 169 |
| he takes it straight off a stick | 19: 186 / 292 | 8: 93 / 239 |
| it is taken straight off his stick | 24: 138 / 242 | 6: 185 / 387 |
| it leaves him loose (poked away, his pass, his shot) | 3: 45 / 50 | 7: 195 / 261 (the sim's puck is one or two frames into its flight) |
| the bots among themselves (package H34's stick, no floor) | 101: 155 / 396 | 20: 210 / 450 |

1. The jump at a change of hands is not the floor's price: the bots jump the same 1.5 m among themselves with package H34's stick. It is the
   distance from where the sim hands the puck over to where the figure holds its blade. A snap to the stick, not a catch: a gather take is asked
   of device B (FOR B 16).
2. While he carries, the puck a defender's stick takes is 70 to 92 cm from the puck he sees (42 to 66 cm in package H34, where he could not see it):
   a poke that looks short can take it. The three times it was poked loose in drill 6 the two pucks were 42 to 50 cm apart.
3. His shot and pass leave from the sim's puck: the drawn puck cuts to it on the frame of the release (up to 90 cm from the blade).

### 3.2 The puck thrown about in a scrum (`jam.puck.SettleLoose 2` ships; `jam.puck.Settle` is a switch at 0)

In drill 6 at rookie the sim's carrier of record changed 156 times in 6450 frames, 79 of them straight from one stick to another; of 117
possessions 23 lasted 4 frames or less: the drawn puck thrown 1.2 to 1.9 m to another blade and back inside a tenth of a second, or thrown to
the blade of a man who only deflected it. Two rules in `FJamPuckShow::Settled` (pure, tested in `Jam.Puck.Show`):

- a change straight from one stick to another is shown once the sim has named the new man 4 frames running; a shorter hold is never shown. At
  once when the man before is down, hit or released it, when the new man released it, and when the new man is the human holding a shot or a pass;
- a loose puck is shown on a stick once the sim has named the man 2 frames running.

| the same sim frames | changes of hands shown | possessions shown | of 4 frames or less | the median possession | the drawn puck's jumps, summed |
|---|---|---|---|---|---|
| drill 6, frames 131 to 4740: PUCK READY 1 | 119 | 91 | 19 | 21 frames | 181 m |
| + `jam.puck.Settle 4` (tried alone; not shipped) | 114 | 86 | 18 | 22 | 178 m |
| + `jam.puck.SettleLoose 2` (both; measured on build 28) | 105 | 81 | 14 | 31 | 148 m |
| the play drill, frames 131 to 4203: before | 46 | 32 | 4 | 37 | 81 m |
| both rules (build 28) | 42 | 29 | 1 | 41 | 63 m |

A fifth of it is gone. The human's jump at a loose-puck pickup falls from 114 to 62 cm on average (median 131 to 34): the two frames waited are
the frames the sim brings the puck to his blade. What is left is the sim's: a bot holding it 5 to 10 frames off the human until he is hit, two
mates a stick apart passing it back and forth (lane A208). The price: a real steal shows 4 frames (67 ms) late, a pickup 2 frames late; on those
frames the sim's carrier is not the man shown (the human's on-blade count 89 to 99 % a window, 96 to 98 % before; the drawn puck to the sim's on
his carry frames 78 to 141 cm a window with the held frames counted in, 70 to 92 before). `jam.puck.Settle 0` and
`jam.puck.SettleLoose 0` = PUCK READY 1.

**What ships (PUCK READY 2)**: `jam.puck.SettleLoose 2` alone. The wait of 4 frames before a stick-to-stick change took 5 of 119 changes off the
screen on its own, and on its frames the disc is on one man's blade while the HUD has its ring under the other: it stays in the code at 0. On the
two frames a loose puck waits, the disc is at the sim's puck (inside the HUD's ring), not on a blade.

## 4. The play calls (device B's H55.6; all by name, a figure without one gets nothing)

| call | when A sends it | source |
|---|---|---|
| `SetLookAtPuck(false)` | once on the human's figure, again when it is respawned | `FJamPuckShow::StepSim` |
| `PlayReceive()` | the frame a mate's pass is seated on a skater (the sim's own `PassesReceived` grows: blade, body and A207's catch) | same |
| `SetPassCharge(0..1)` | every frame a pass is held with the puck (`PassChargeFrames` over `pass.power.full_frames`), 0 once when the hold ends | same |
| `PlayCallForPass()` | while the human carries: the mate his pass would go to now (the sim's `HumanPassTarget` along his stick), 3 m or more ahead of where he looks, no opponent within 2.5 m; at most every 0.5 s, the same mate after 2.5 s | same; the brain's own read is asked of lane A206 |
| the trail's `User.Carried` | each change of the shown possession | `AJamPuck::SetTrailCarried` |
| `shot` Variant 1 | the puck leaves at over 30 m/s, or a one-timer | `UJamEffectsBinder::ShotVariant` |

## 4a. The pass and the shot, seen (drill 24: the human holds the buttons; `tools/qa/screen/a196/cut-clay-play-open.json`: five against two)

| play | what the screen shows | frames |
|---|---|---|
| a tap pass (2 frames held) | `SetPassCharge` for 2 frames, `PlayPass`; the mate's `PlayReceive` on the frame the sim seats it | release to seat 2 to 12 |
| a full-hold pass (33 frames) | `SetPassCharge` 0 to 1 over 30 frames: the figure turns its shoulders about 35 degrees to the stick side and leans over the puck, plain from half charge; square for the whole hold without the call; the same figure again 2 frames after the release (`img/a210/pass-full-hold-charge.jpg`: `jam.puck.Calls 0` above, the default below, the same sim frames); the release at strength 0.78 | 33 held |
| a tap shot | the swing; 20.6 m/s | |
| a half-charged shot (15 frames) | the stick winds up; 43.6 m/s | 15 held |
| a full-charged shot (32 frames) | the stick winds up high over 32 frames, the HUD's bar fills; the swing through on the release; **51.6 m/s: the puck is in the net from the blue line in about 24 frames: a puck 10 px wide with a streak too faint to see in a still**: not a teleport if the eye is on it, and nothing draws the eye to it (`img/a210/shot-full-52mps.jpg`) | 32 held, 24 in flight |

## 5. The eye's other items in this wave

- **EYE 12 (a board bump turns his figure pale blue and throws the stick up)**: no board take on a plain run into the boards (`PlayBoardHit` only when
  he was checked into them or goes down: `jam.board.BumpTake 0`); the board caps' flash is a hit's: 90 cd warm white for a man checked into the
  boards (300 cd in the half's colour before), nothing for a plain bump (`jam.rink.react.hit.bump 0`). Stills, the same sim frames:
  `img/a210/board-bump-before-after.jpg` (clay), `board-bump-night-neon.jpg` (night and neon).
- **EYE 7 (a miss must look like a miss)**: after a check or a cross-check that met nobody the drawn figure tips into a dip and is back in 14 frames
  (`jam.hit.Whiff 0.8`), the stick is sent to the front of the body for 30 frames after a missed cross-check (`AJamSkater::SetAimRecover`), and the
  figure is told `PlayWhiff(Kind)` by name for the take device B is asked for. `img/a210/whiff-before-after.jpg` (a miss in open ice: the stick
  leaves the left side 13 frames after the miss; the dip is small: the take is what will make it a whiff).

## 5a. Device B's hit takes of H56.0 / H56.5 from the game camera (a match by the playtester's hand, the same sim frames with and without my figure bridges)

- **The held cross-check** reads from behind on B's hold alone (the stick across above his shoulders, out past both gloves, the elbows out); my
  set on the figure made it a wide, short body beside it: `jam.hit.BraceSet` 1.5 -> 0. `img/a210/brace-b-hold-alone.jpg` (defaults of build 29 above,
  B's hold alone below).
- **The body check and the cross-check**: B's crouch at press + 4, the stick over the helmet at press + 10, both arms and the stick above the
  helmet at press + 3 of a cross-check: the same figure with and without my coil, and my coil lowered the cross-check's raised arms:
  `jam.hit.Pop` 1 -> 0. `img/a210/cross-b-take-alone.jpg`.
- **A miss looks like a miss**: `PlayCheckWhiff()` (by name, on the frame the window closes on nobody) throws him past the man with an arm out
  wide; without the call he skates on. `img/a210/miss-check-whiff-take.jpg` (the call above, no call below). A missed cross-check has no take yet.

## 6. Proof

In `status/A210.md` under PUCK READY 1 and PUCK READY 2: the probe's lines, the hashes, the gate.
