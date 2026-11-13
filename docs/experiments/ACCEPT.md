# The acceptance run (lane A87, R-0043 point 1)

H19 to H24 shipped unable to shoot or pass while the player aimed (R-0042). No gate could see it: bots and playtester classes write `FJamInput` directly, and every
package proof checked a verb's read-out, never a goal through the cursor and the buttons. The acceptance run plays the first minutes of a match in a packaged
Development game **through the real input path** (the cursor on the ice, the keys, the mouse buttons and wheel, Esc) and says pass or fail per verb.
R-0043: no H package from H25 on without it green.

## Run it (one command, after packaging)

```
powershell -NoProfile -ExecutionPolicy Bypass -File tools\qa\accept\accept.ps1 -Package Game\Packaged\<dev package>
```

It takes about 90 s on a free machine. Exit 0: every verb passed. 1: a verb failed or never reported. 2: could not run (no exe, no log, the drill never began).
The last line is `{"tool":"thirdjam.accept/1","verdict":...,"warnings":[...],"steps":[...]}`. Output: `tools\qa\out\accept\<Name>\` (`game.log`, `events.log`
= what the driver did and when, `run.json`, `verdict.txt`). `-Name` names the folder, `-From N -To M` runs a part, `-Cut <cartridge>` plays another cut.

**Needs the machine's cursor to itself.** The driver moves the OS cursor (`SetCursorPos`) because Slate reads it on the posted move message, and there is one OS cursor
per machine. With another lane's game driven the same way, aim targets never arrive (the control run's contended attempt lost 3 of 4). The verdict lists a `WARN` in
`warnings` when another ThirdJam process ran at the start or the end; a verdict with warnings is not a clean one.

## How it works

- **The game** runs `jam.ctrl.Drill 20` (`Game/Source/JamGame/Private/HokejAcceptDrill.cpp`). The cut is `tools/qa/accept/accept-cut.json` (A77's rookie cut: penalties off,
  9 to win). The human's input is untouched (`ReadHumanInput`). The drill stages each step with `SetSkater` / `SetPuck`: the human, one mate, one opponent; every other
  skater is parked in the human's own corner, and the net is empty. It scripts the mate and the opponent, tells the driver what to do, and judges:
  - `JAMACCEPT cursor step=S k=K sx= sy=`: put the cursor at these client pixels (the drill projects a rink point; a point off the screen is pulled in).
  - `JAMACCEPT act step=S k=K act=<keys|lmb|pass|probe_pass|rmb|mmb|wheel|key|sweep|pause_resume|lmb_onetimer|none>`: press.
  - `JAMACCEPT step=S verb=V ok=0|1 <facts>`: one line per verb; `JAMACCEPT summary ok= fail=` at the end.
- **The driver** (`tools/qa/accept/accept.ps1`) is the hand, the way A77 and A82 drove their drills without the foreground (a foreign "Windows Security" dialog may hold it).
  The cursor uses `SetCursorPos` plus a posted `WM_MOUSEMOVE`. Keys use posted `WM_KEYDOWN` / `WM_CHAR` / `WM_KEYUP`; buttons use posted `WM_[LRM]BUTTON*`; the wheel uses a
  posted `WM_MOUSEWHEEL`. PLAY is one posted Return, and the end is `quit` over the TCP remote console. **Shift:** UE polls `GetKeyState(VK_LSHIFT)` every frame and
  sends itself a Shift-up when the state says up, so a posted Shift lasts one frame (run 1: 1 sprinting frame). While Shift is held, the driver attaches to the game's
  input thread and marks Shift down in the shared key state (`SetKeyboardState`), which is the state a real key press leaves. **The pass key is read live:** the driver
  taps Q, then F, and keeps the one that raised `JAMCTRL ... btn=pass held=1` (today F; A84 moves the pass to Q).
- **The verdict** (`tools/qa/accept/accept.mjs <game.log> [events.log]`) makes the table. It attaches the `JAMCTRL` button edges that reached the game and the
  `JAMSHOT release` read-outs inside each verb's window, so a failure says whether the press arrived and what the sim saw.

## The steps and their pass rules

| step | verb | the hand | pass rule |
|---|---|---|---|
| 1 | skate | W 2 s from rest | top speed > 6 m/s within 120 frames of the first move frame |
| 1 | sprint | W + Shift 2 s, no puck | sprinting frames > 0 and top speed > walking top + 0.2 |
| 1 | sprint_carry | W + Shift 2 s with the puck handed | carried >= 60 frames, 0 sprinting frames while carrying, top < sprint top - 0.2 (R-0041 point 6) |
| 2 | aim | the cursor at 4 points (ahead, left, behind, right; 2.5 m, pulled in to stay on screen) | the blade's world angle within 15 deg of the point inside 12 frames of the cursor's arrival, all 4 |
| 3 | carry | the puck handed; the cursor swept left-right-left at full reach x3 | the puck never leaves the human (last touch and carrier) for 300 frames |
| 4 | shoot_tap / shoot_half / shoot_full | the cursor 6 m ahead toward the empty net from 10 m; the left button 0 / 8 / 30 frames | a shot and a goal; charge frames <= `game.shot.tap_frames` (4) and the shot released as charge 0 / 5..12 / >= 26 (lane A91, R-0044: the tap band was <= 3; a 4-frame click is the flat tap the ruling asks for) |
| 5 | pass / saucer | the cursor toward the mate 8 m away; the pass key tapped / held 14 frames (saucer half 8 + 6) | a pass event and the mate's reception / a pass with the saucer flag |
| 6 | onetimer | the mate passes back (scripted); the left button pressed 10 frames before the ETA, released on the reception line | a goal, or a shot with the one-timer flag, in 1 of 2 tries |
| 7 | check | W held, 4 m/s start at the carrying opponent; RMB on the contact cue | a check, a knockdown, the puck stripped, the get-up 90 +-10 frames later |
| 7 | crosscheck | the same; MMB on the contact cue | a stick hit on the carrier (`StickHitResult`) and the puck free 20 frames later |
| 8 | checked | none: the scripted opponent checks the carrying human | the human down; the view target stays the human's pawn and the camera within 25 m; up after knock frames +-15 |
| 9 | deke / spin / sweep | the wheel / E / the cursor across, with the puck | the read-out fires and the puck stays the human's for 30 frames after |
| 10 | resume_aim | Esc, 1.2 s, Esc; then 2 aim points | a pause seen (a wall-clock gap > 0.7 s between sim frames), then both points followed as in step 2 |

## Today's results (main 8fc69d3, the A87 Development package, run 2: `tools/qa/out/accept/run2/`)

Exit 1: **12 verbs pass, 6 fail.** The six failures are R-0042 exactly. The run sees what the gates could not. Run 3 (`run3-repeat/`) and `final-main/` (the
final driver, 15:05) repeated it verb for verb.

Known quirk: since about 14:40 the packaged game no longer opens the `-abslog` file. That held for both cuts and fresh folder names; runs 1-3 did open it, and the cause
is not found. The driver then tails `Saved\Logs\ThirdJam.log` from 20 s on (`NOTE` in events.log). Its read offset never moves back, and every JAMACCEPT line is handled
once, so the timing steps hold (`control-reach165-final`: deke and spin 2 frames after the act, one-timer 2 of 2).

| step | verb | today | facts |
|---|---|---|---|
| 1 | skate | PASS | top 11.00 m/s, moving 2 frames after the act line |
| 1 | sprint | PASS | top 12.65 m/s against 11.00 walking, 121 sprinting frames |
| 1 | sprint_carry | PASS | top 10.12 m/s, 121 frames carried, 0 sprinting frames |
| 2 | aim | PASS | 4 of 4 points followed at 0 frames after the cursor arrived, error 0.0-0.1 deg |
| 3 | carry | PASS | 13 sweeps at full reach, the puck kept, puck at 1.60 m |
| 4 | shoot_tap | **FAIL** | button down/up reached the game; `JAMSHOT release charge_frames 0 puck_dist 1.6 reach 1.5 carrier 0`; no shot, no goal |
| 4 | shoot_half | **FAIL** | the same, 8-frame hold |
| 4 | shoot_full | **FAIL** | the same, 30-frame hold: charge_frames 0 throughout |
| 5 | pass | **FAIL** | the pass key F reached the game (`btn=pass held=1/0`); no pass event; puck 1.60 m > reach 1.50 |
| 5 | saucer | **FAIL** | the same, 14-frame hold |
| 6 | onetimer | **FAIL** | the mate's pass was received twice (35 frames); charge 3 frames while the puck passed through reach, then the release fired nothing (puck seated at 1.60 > 1.50) |
| 7 | check | PASS | knockdown, strip, down 90 frames (cfg 90); closing 11.06 m/s at the cue |
| 7 | crosscheck | PASS | stickhit 2 (lift), the puck free |
| 8 | checked | PASS | down; view target the human; camera 6.1 m; up after 90 frames |
| 9 | deke / spin / sweep | PASS | each fired 1-3 frames after the input; the puck kept 30 frames |
| 10 | resume_aim | PASS | Esc paused (1.25 s gap), Esc resumed, 2 of 2 points followed |

**The control** (`-Cut tools\qa\accept\accept-cut-reach165.json`: `game.shot.reach_m` 1.65 = R-0042's carrier reach `max(1.5, 0.4 + 1.2) + 0.05`, a cartridge value,
no code) stands in for A85's fix. Clean run (`tools/qa/out/accept/control-reach165-final/`, no other game running): **17 of 18 verbs PASS**:
shoot_tap (charge 2, goal), shoot_half (charge 9, goal), pass (received in 22 frames), saucer (flag, received), onetimer (2 of 2 with the one-timer flag), and
everything that passed before. **The one failure is real: shoot_full.** The shot fired at charge 30 from 10 m, centred on the empty net, and 20 frames later raised
`JAMEVENT post` at the mouth's centre (x 41.0, y 0.0): the crossbar. A full wind-up from the slot hits the bar (R-0035's `full_m` 0.6 against the bar; for A85 / A86,
not tuned here). When A85 lands, the plain command on the H25 Development package must exit 0, or the full shot must be ruled out of the bar.

## What each failure looks like in the log

- **Shot not fired (the R-0042 fault):** `JAMCTRL ... btn=shoot held=1` then `held=0` (the button arrived), `JAMSHOT release ... charge_frames=0 ... puck_dist=1.60 reach=1.50`
  (the puck lies beyond the shot reach), no `JAMSHOT frame=... human` line, and `JAMACCEPT step=4 verb=shoot_* ok=0 ... shot=0 goal=0 charge_frames_max=0`.
- **Pass not fired:** `JAMCTRL btn=pass held=1/0`, no `JAMPASS ... human` line, and `step=5 verb=pass ok=0 pass=0 ... puck_dist_at_act=1.60 reach=1.50`.
- **Input never arrived** (a driver or binding fault, not the sim): the verb's `input=[...]` is empty in the table, with no `JAMCTRL btn=` line in the window. For the pass
  probe, `events.log` says `pass probe: Q raised no JAMCTRL btn=pass line`.
- **Aim:** `t<k>=(arrive=-1 ...)`: the cursor never reached the point (off screen, or another driver moved the OS cursor). `lat>12`: the blade lagged.
- **Pause:** `step=10 verb=resume_aim ok=0 pause_seen=0`: Esc did not pause. No step-10 line at all, with `events.log` saying `no JAMACCEPT resumed line`: Esc did not resume.
- **A step that never reports:** `no verdict line` in the table; `events.log` shows the last act the driver did.

## Shipping (job 2: `Game/Packaged/Hokej_Hokej_H24`, the Shipping zip build)

`tools\qa\accept\shipping-probe.ps1` (`tools/qa/out/accept/shipping-h24/probe.txt`) found the following:
- The Shipping exe has no `RemoteConsole` string (byte count 0, as in PARITY.md), and port 4662 refused the connection.
- `-abslog` wrote nothing, and no `Saved/Logs` file appeared. `UE_LOG` is compiled out, so no `JAMSHOT` / `JAMMENU` strings are in the binary.
- H24 also predates drill 20.

So the run **cannot be driven in Shipping**: the driver follows the drill's log lines, and none exist. **No step can be judged by machine in Shipping.** By eye,
from the HUD score in a still, only goals can be judged (steps 4 and 6). The input path code (`ReadHumanInput`, `CursorAimRink`, the sim) is the same in Development
and Shipping; only the logging differs. So the Development verdict is the verdict for the build that gets zipped. Run it on the Development package built from the
same commit as the Shipping zip.

## Files

`tools/qa/accept/accept.ps1`, `accept.mjs`, `accept-cut.json`, `accept-cut-reach165.json`, `shipping-probe.ps1`; `Game/Source/JamGame/Private/HokejAcceptDrill.cpp` (new);
`Game/Source/JamGame/Public/HokejGameMode.h` (two declarations); `Game/Source/JamGame/Private/HokejGameMode.cpp` (three hook lines and the cvar help: drill 20 keeps
`ReadHumanInput`, the input hook after the A82 hand-puck block, the observe hook before `MechDrillObserve`; drills 7-11 untouched).
