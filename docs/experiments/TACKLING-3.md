# Tackling, pass 2: B's SK12 takes in the shipped build, the figure against the ice, the hit-stop against the get-up (lane A80, device A, 2026-11-13)

Same chain as `TACKLING-2.md` (A73) on main at c45face, which carries device B's SK12 takes (new lying poses, `Knockdown_Left/_Right` and the `_B` spin variants,
`GetUp_Side_*`). Nothing of the sim moved (`JamSim.*` has no diff). One presentation-only read-out was added to measure the figure: `JAMLOW` in
`Game/Source/JamCore/Private/JamPawns.cpp` (`jam.vis.lowlog 1`, off by default; per sim frame, from a knockdown to 70 frames after the release: the lowest body bone, pelvis /
hand / head / foot Z in cm above the pawn's ice plane, the figure's active montage by its sequence name). It reads bones with `GetBoneLocation` (so **joint centres, not
skin**) and forces the figure's bone refresh on, so an off-screen bot is measured like a seen one. That file is outside the lane's list; the Director decides whether it stays.
Driver `tools/qa/ctrl/a80-prove.ps1` (A73's, with `-UntilFrame`, `-ScriptHuman`, `jam.vis.lowlog 1`), analysers `a80-analyze.mjs`, `a80-pop.mjs`; package `Game/Packaged/A80-dev2`
(Development, 1280 x 720; three copies of it run side by side; logs in `tools/qa/out/ctrl/a80-*/game.log`, out of the tree).

## 1. Counts against A73's (the same analyzer, `a73-analyze.mjs`)

| Run | Drill | Frames | `JAMVIS hit` | `board` | `pin` | `stickhit` | knockdowns (of the human) | get-ups | human's checks landed | `JAMHITSTOP` | `JAMKICK` | held presentation | camera on the lying human |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `a80-side` (drill 5, strict rule) | 5 | 9 000 | 84 | 64 | 28 | 22 | 3 (0) | 3 | 0 of 1 press (3.3 m/s) | 0 | 0 | - | (never down) |
| `a80-t-hs3` (drill 6, hit-stop 3) | 6 | 9 000 | 59 | 29 | 40 | 23 | 37 (**0**) | 37 | 37 of 37 | 37 | 37 | 37 of 37 | (the human never down) |
| `a80-t-hs0`, `a80-t-hs6` (drill 6) | 6 | 9 000 each | 59 | 29 | 40 | 23 | 37 (0) | 37 | 37 of 37 | 0 / 37 | 37 | - | - |
| `a80-botmatch` (`jam.ScriptHuman 1`, no drill, 3 min) | 0 | 10 800 | 97 | 68 | 93 | 117 | 5 (**5**) | 5 | 0 | 5 | 5 | 5 of 5 | **445 of 445** in the middle third (max 0.188 / 0.229) |

A73's: side2 80 / 44 / 6 / 22, 1 knockdown, 0 of 3 presses; taken2 96 / 51 / 22 / 45, 57 knockdowns (2 of the human), 55 of 55 checks landed: the same shape. Every knockdown has
its `JAMVIS knockdown ... sim_frames=90 sent_frames=87 hitstop=3` and a `JAMVIS getup` 90 frames later (`sent = sim - hitstop` true on all 42). The sim's hash at frame 8400 is the same
with hit-stop 0 and 6 (`d41504fb2871c162`): the hold never reaches it.

Takes the sim's kinds picked (all runs): front hit -> `Fall` (16, the victim lies on its back), behind -> `Knockdown_Back` (11, face down), side_l -> `Knockdown_Left` / `_Left_B` (3 + 3),
side_r -> `_Right` / `_Right_B` (3 + 3); 3 "behind" hits fell sideways (the take follows the push against the facing, not the kind). A get-up montage follows each (`GetUp` after `Fall`,
`GetUp_Front` after `Knockdown_Back`, `GetUp_Side_Left/Right` after the side takes).

Pro bots check the human rarely, as in A73: in the three minutes with the scripted human they checked it **5 times** (all side hits: side_l 0.73 / 0.74 / 0.59, side_r 0.77 / 0.69) and
**no bot ever checked another bot** (section 4). The drill 5 side check landed 0 of 1 press in 150 s, as A73's strict run did.

## 2. The figure against the ice (cm, joint centres; the minimum over each knockdown's lying window, rel +20 .. release -20; 42 knockdowns, `a80-analyze.mjs`)

| Take (what the sim sent) | n | lowest body bone | pelvis | hands | head | feet |
|---|---|---|---|---|---|---|
| `Fall` (front hit, lies on the back) | 16 | 13.0 (foot) | 24.5 | 13.7 | 30.0 | 13.0 |
| `Knockdown_Back` (hit from behind, face down) | 11 | 13.1 (foot) | 24.5 | 18.2 | 26.8 | 13.1 |
| `Knockdown_Left` | 4 | 4.8 (foot) | 38.0 | 13.0 | 44.9 | 4.8 |
| `Knockdown_Left_B` (spin) | 3 | 4.8 | 38.0 | 13.0 | 44.9 | 4.8 |
| `Knockdown_Right` | 4 | 4.6 (foot) | 38.0 | 13.0 | 44.9 | 4.6 |
| `Knockdown_Right_B` (spin) | 4 | 4.6 | 38.0 | 13.0 | 44.9 | 4.6 |

(Standing, the same read-out: pelvis 77.9, feet 13.0, head 125.) **Nothing is under the ice** at any frame of any take: the lowest joint of the whole body is 4.6 cm above the plane, the
pelvis never below 24.5 cm: A73's finding (pelvis 10-20 cm under) is gone with the SK12 poses; the 2 cm rule gives **zero findings**. The bones cannot say how far the skin is from the ice; the
lying still does: in `a80-kd-human-f1674-left-strip.png` (raw still `a73-kd-human-1674-f001706.png`) the side-lying figure is level with the torso at about the height of the boards' kick plate,
**well above the ice** (pelvis centre 38 cm and head centre 45 cm, where a body on its side would be near 12-15 cm). The question has turned round, from "under" to "propped over the ice":
about 20-30 cm on the side takes, about 10 cm on the back / face-down takes (pelvis 24.5). That is an estimate from joint centres and one still, not a skin measurement.

## 3. The hit-stop against the get-up (drill 6, 37 knockdowns per setting, all with the human as the checker)

The frame the figure stands (pelvis back at 90 % of its standing height, after the lying minimum) minus the frame the sim releases it (`JAMVIS getup`, `SetKnockedDown(false)`); negative = the
figure is up before the release; `sent_frames` = 90 - N as shipped.

| `jam.hitstop` | frames sent to B | median offset | mean | range | at 80 % standing, mean | by take (median) |
|---|---|---|---|---|---|---|
| 0 | 90 | **-12** | -12.5 | -29 .. -10 | -17.1 | Fall -12, Back -12, Left -10, Right -13 |
| 3 (default) | 87 | **-15** | -15.3 | -29 .. -13 | -19.9 | Fall -15, Back -15, Left -13, Right -16 |
| 6 | 84 | **-18** | -18.4 | -36 .. -16 | -23.0 | Fall -18, Back -18, Left -16, Right -19 |

Three readings.

- **Each frame of hit-stop moves the get-up one frame earlier, not onto the release**: the offset grows by exactly N (-12, -15, -18). B's figure freezes for N frames (the montage stands:
  pelvis 76.3 cm over the held frames in the log) while the chain still ends `sent` frames after the fall began in **world time**, so lowering the number by N (A73's `KnockFrames - N`) counts the
  hold twice. That is the "timer or Delay" case of A73's ask 2, confirmed by measurement; the default 3 drifts **-3 frames (50 ms)**: the number for SK13. Not changed here (the brief); the
  one-line alternatives are to send the unreduced `KnockFrames` (the offset is then -12 at every N) or for B to read the montage's time, or `CustomTimeDilation < 0.01`, and add N to its timer.
- **The baseline is already -12 at hit-stop 0**: fall, lie and get-up are done 12 frames (200 ms) before the sim lets the skater act, so the figure stands for the last 8-12 frames of every
  knockdown while the sim still holds it down. A lie 12 frames longer, or a slower get-up, puts it on 0. The side takes end 2-3 frames later than the others.
- The outlier (-29 / -36) is one `Fall` whose get-up was cut by a `Board_Pin` (the montage list of that knockdown reads `Fall+GetUp+Board_Pin`).

No pop and no figure staying down: at each release frame the pelvis was at or above 90 % of standing in all 31 knockdowns with a pose (6 releases had no JAMLOW row at the release frame), the largest one-frame
pelvis step in -2..+10 frames of the release was 8.7 cm, and 0.1 cm in the bot-match run (`a80-pop.mjs`).

## 4. Bot against bot, three minutes (`a80-botmatch`, 10 800 frames, pro bots, `jam.ScriptHuman 1` plays the human's skater by script, cut `a73-cut.json`)

- JAMVIS: bumps received by a skater other than the human 64 (by the human 33); board hits 52 / 16; pin edges on 24 (off 24) / 23; stick hits on a bot 5, on the human 112; **checks 5, knockdowns
  5, all on the human's skater**: the bots checked each other **0 times** in 180 s. A bump is a collision at 3 m/s closing or more with no check landed (`BumpMinClosing`), so a bump's partner may be the human.
- The pro brain lifts the puck (117 stick hits) and pins on the boards (93 pin edges) but never hits a bot down; the 5 knockdowns of the scripted human were the only ones. The check rule is
  A70's lever (A73 said the same).
- No figure stayed down after its release and none popped (5 of 5).

## 5. What B still lacks (centimetres and frames)

1. **Side takes hover.** `Knockdown_Left/_Right` (and `_B`): pelvis 38 cm, head 45 cm, hands 13 cm, feet 4.6-4.8 cm over the ice for the whole lie (rel +26 .. +45); a body on its side would sit at
   12-15 cm; the still agrees. Ask: lower the lying torso of those four takes by about 20-25 cm. `Knockdown_Back` / `Fall` (pelvis 24.5) by about 10. SK13's "no part under" is right: the problem is the other way.
2. **The hit-stop drift**, -1 frame per frame of hit-stop (default -3, 50 ms; -6 at 6): time the chain from the montage, or add N to the timer when the actor was held. A can instead send the unreduced
   `KnockFrames` (one line in `ApplySimFrame`): B to say which.
3. **The get-up is 12 frames early** at hit-stop 0 (the figure stands 200 ms before the sim lets it skate; 10 frames on the side takes): a longer lie or slower get-up.
4. **A pin cutting a get-up** (one `Fall` at -29): check the get-up resumes after `Board_Pin` ends.
5. Not measured here: a still of `Fall` (front) and the `Right` takes (the stills capture two sequences per kind and the first two knockdowns were `Knockdown_Back` and `Left`), and no eyes pass (section 6).

## 6. Eyes

`tools/eyes` answered `402 RESOURCE_EXHAUSTED` ("prepayment credits are depleted") on all four bug-hunt calls (`docs/experiments/eyes/a80-*.json` hold the refusals): no Gemini reading this
pass; the key's owner has to top it up. The lane read the strips itself (the side lie above; the back-fall strip shows the victim lying with the stick across and the camera following the human).

## 7. Files (relative to the worktree)

- `Game/Source/JamCore/Private/JamPawns.cpp`: the `JAMLOW` read-out, `jam.vis.lowlog` (presentation only; outside the lane's list).
- `tools/qa/ctrl/a80-prove.ps1`, `a80-analyze.mjs`, `a80-pop.mjs`.
- `docs/experiments/TACKLING-3.md`, `docs/experiments/img/a80/*.png` (6 strips), `docs/experiments/eyes/a80-*.json` (4 refusals), `status/A80.md`, the reply in `handoff/A-to-B-0017.md`.
- Not touched: `Game/Plugins/JamAssets`, `JamSim.*`, the contracts. `.env` was copied from the main checkout for the eyes tool (not printed).
