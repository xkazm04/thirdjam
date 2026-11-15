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
| the jump at a pickup or a release | up to 2.9 m in one frame | 2 frames |
| the human's carried puck seen at his camera (twelve straight-carrying stills at 1280 x 720) | 2 of 12 | 10 of 12 |

**The lag is the fault.** The game mode placed the puck on the carrier's `puck_socket` BEFORE the frame's animation was evaluated: the socket it read
carried last frame's pose, so every stride and every stick move left the drawn puck behind the blade: the puck "does not stick".

## 2. What changed

1. **One shown carrier a frame** (`jam.puck.Show 1`): `FJamPuckShow::StepSim` runs after the sim's step and before the pawns apply it; it takes the
   sim's carrier of record (`UJamSim::CarrierIndex`), keeps him at most `jam.puck.Keep` 6 frames over a drop-out (the puck still his last touch, he
   can act, no shot or pass, the puck within 1.35 of his reach; never while down or hit), and tells every pawn (`AJamSkater::SetShownCarrier`): the
   figure's `SetHasPuck` and the drawn puck follow the same answer.
2. **The drawn place** (`FJamPuckShow::Render`): on the shown carrier's socket, else where the sim has it; a change of hands runs the offset out over
   2 frames (in and out) instead of jumping.
3. **Through the frame's animation** (`jam.puck.Follow 1`, `AJamPuck::SetFollow` / `Tick` in `TG_PostUpdateWork`): once on the blade the puck takes the
   socket's place again after the figures' poses of the frame are evaluated: 0 cm from the blade when the frame is drawn.
4. **The human's carried puck is not behind his hips** (`jam.puck.CarryAim 30`, `AJamSkater::CarryAimDeg`): section 3.
5. **Device B's play calls by name** (`jam.puck.Calls 1`): section 4.

`jam.puck.Show 0`, `jam.puck.Follow 0`, `jam.puck.CarryAim 0`, `jam.puck.Calls 0` = package H34's puck.

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

## 4. The play calls (device B's H55.6; all by name, a figure without one gets nothing)

| call | when A sends it | source |
|---|---|---|
| `SetLookAtPuck(false)` | once on the human's figure, again when it is respawned | `FJamPuckShow::StepSim` |
| `PlayReceive()` | the frame a mate's pass is seated on a skater (the sim's own `PassesReceived` grows: blade, body and A207's catch) | same |
| `SetPassCharge(0..1)` | every frame a pass is held with the puck (`PassChargeFrames` over `pass.power.full_frames`), 0 once when the hold ends | same |
| `PlayCallForPass()` | while the human carries: the mate his pass would go to now (the sim's `HumanPassTarget` along his stick), 3 m or more ahead of where he looks, no opponent within 2.5 m; at most every 0.5 s, the same mate after 2.5 s | same; the brain's own read is asked of lane A206 |
| the trail's `User.Carried` | each change of the shown possession | `AJamPuck::SetTrailCarried` |
| `shot` Variant 1 | the puck leaves at over 30 m/s, or a one-timer | `UJamEffectsBinder::ShotVariant` |

## 5. The eye's other items in this wave

- **EYE 12 (a board bump turns his figure pale blue and throws the stick up)**: no board take on a plain run into the boards (`PlayBoardHit` only when
  he was checked into them or goes down: `jam.board.BumpTake 0`); the board caps' flash is a hit's: 90 cd warm white for a man checked into the
  boards (300 cd in the half's colour before), nothing for a plain bump (`jam.rink.react.hit.bump 0`). Stills, the same sim frames:
  `img/a210/board-bump-before-after.jpg` (clay), `board-bump-night-neon.jpg` (night and neon).
- **EYE 7 (a miss must look like a miss)**: after a check or a cross-check that met nobody the drawn figure tips into a dip and is back in 14 frames
  (`jam.hit.Whiff 0.8`), the stick is sent to the front of the body for 30 frames after a missed cross-check (`AJamSkater::SetAimRecover`), and the
  figure is told `PlayWhiff(Kind)` by name for the take device B is asked for. `img/a210/whiff-before-after.jpg` (a miss in open ice: the stick
  leaves the left side 13 frames after the miss; the dip is small: the take is what will make it a whiff).

## 6. Proof

In `status/A210.md` under PUCK READY 1: the probe's lines, the hashes, the gate.
