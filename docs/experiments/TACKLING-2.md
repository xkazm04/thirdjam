# Tackling felt: the hit-stop, the kick and the whole chain in the shipped build (lane A73, device A, H28-H30, 2026-11-13)

The operator at 09:43: "the ability to tackle other players to the ground, collide and animate fall, getting up, that would be key for the game." B-to-A-0036 point 4
asks A for the hit **felt**, not only seen: a hit-stop of 3 to 4 frames and a short camera kick along the hit direction on a knockdown that involves the human, and
the whole chain (press, hit, fall, slide, lying, get-up) proven in a packaged build. This page is the mechanism (nothing of it touches the sim: `JamSim.cpp` has no
diff, the state hash cannot move), the counts from the Development package, the stills, the cvars, the asks for B and the three things the operator will feel first.
Status blocks with timestamps: `status/A73.md`. Code: `Game/Source/JamCore/{Public,Private}/JamPawns.{h,cpp}` (the hold), `Game/Source/JamGame/{Public,Private}/CameraFollow.{h,cpp}`
(the kick), `HokejGameMode.{h,cpp}` (the routing, the drills 5 and 6, the stills), driver `tools/qa/ctrl/a73-prove.ps1` + `a73-analyze.mjs` + `a73-strip.py`, cut `tools/qa/ctrl/a73-cut.json`.

## 1. The hit-stop (presentation only; lockstep untouched)

The sim steps every frame whatever happens on screen. On a `knockdown` of Kind `check` whose victim **or** checker is the human, `AHokejGameMode::HitStopRoute` (after
`DeliverEvents`, before the pawns apply the frame) tells both pawns and the puck to hold `jam.hitstop` frames (N, 0..6, default **3**):

| Who | Hit frame H | Frames H+1 .. H+N-1 (held) | Frames H+N .. H+2N-1 (catch-up) | How |
|---|---|---|---|---|
| the two figures (`AJamSkater`) | applied in full: `SetKnockedDown(true, dir, KnockFrames - N)`, `PlayCheck`, the burst and the sound start; the pose is recorded | drawn at the recorded pose; **no** `UpdateMotion` / `SetStickAim` / `SetShotCharge` (the toy's curves count those calls, so they stand still); every edge and one-shot still goes out (nothing is lost); `CustomTimeDilation` 0.0001 on the pawn and on B's visual actor, so the montage freezes on its first fall frame | a blend from the held pose to the live one over N frames (no pop: at 6 m/s the three held frames are 30 cm, blended back over 50 ms) | `AJamSkater::BeginHitStop`, `ApplySimFrame`, `ApplyRender`, `SetVisualTimeDilation` |
| the puck (`AJamPuck`) | drawn once more where it is | held where it was last drawn (a stripped puck waits on the blade it left) | the same blend | `AJamPuck::BeginHitStop`, `StepSimFrame`, `Place` |
| the camera, the HUD, the radar | the presentation frame is pushed (the kick's first frame rides it) | **not pushed** (`PushPresentation` skipped); the render alpha is 1, so the camera sits on the hit frame's pose instead of interpolating backwards | pushed again at H+N: the lag filter (alpha 0.22 a frame) absorbs the N-frame jump of the pivot | `AHokejGameMode::StepOnce`, `Tick` |
| the other four skaters | live | live | live | untouched |

Why `KnockFrames - N` on the victim's `SetKnockedDown`: B's Blueprint times its get-up to end `KnockdownFrames` of its **own** time after the fall began, and its own time stood
still for N frames; with the sim's 90 it would stand up 3 frames (50 ms) after the sim released it. The pawn logs both numbers: `JAMVIS knockdown frame= skater= human= dir=
sim_frames=90 sent_frames=87 hitstop=3`, and `JAMVIS getup` on the release. The sim never sees any of this: `FJamSkaterState::KnockFrames` is what it was; `Jam.Core.Determinism`'s
hashes are what A49 recorded (`b705e2def86ae838` / `e09eebd2cebd1337`), because no file of `JamCore/Private/JamSim.cpp` or `JamSim.h` changed. A lockstep peer (A38's
`LobbyGameMode`) does not run `HitStopRoute` at all; it would need the same three lines to feel it.

`jam.hitstop 0` turns it off; `jam.hitstop 6` is the longest (100 ms). The operator tastes it in the console of the Development build or with `-dpcvars=jam.hitstop=4`.

## 2. The camera kick along the hit direction

`UJamFollowCamera::KickOffset` (pure, `KickFrames` 8, `KickCm` 12, `KickRollDeg` 1): on a `check` the human lands or a `knockdown` (Kind `check`) the human takes, the camera is
displaced along the event's `Dir` (A47's payload: the push, checker towards victim, world x/y) by `12 cm x max(0.5, Strength) x jam.hitkick` on the hit frame, decaying
`(1 - i/8)^1.5` over the next seven, with 1 degree of roll toward the side the push has in the camera's frame. The displaced point is projected back inside A40's boards
inset (`NearestInsideBoards`), so a hit at the glass never puts the camera in it. Budget: 12 cm at the 6 m arm moves the skater `0.12 / (6 tan 35)` = 0.03 of the frame; A49's
middle-third edge is 0.333 and the worst frame A49 measured was 0.237, so the kick stays inside the budget with 0.07 to spare. A33's 2 cm punch stays for an event without a
`Dir` (none since A47). `jam.hitkick` 0..3 (default 1) scales it; `JAMKICK frame= landed|taken strength= dir= cm= frames=` is logged per kick.

Both read-outs together: the hit frame shows the impact pose displaced by the kick, holds it N frames (the burst and the sound already playing), then the figures catch up
and the camera settles over eight frames. The Strength scale used by the kick, `max(0.5, Strength)`, is the one B should use for a stronger `check` / `knockdown` burst
and sound (section 7).

## 3. The drills and the stills (what the proof runs)

- `jam.ctrl.Drill 5`: the human checks the opposing carrier **from the side at 3-6 m/s**: it steers for a point 2.5 m beside the carrier (the side it is already on, led by the
  carrier's velocity) until level with it, then at it; coasts above 6 m/s; presses the check (one frame of `bCheck`, what the right button sends) only when the push is across
  the carrier's facing (|dot| < 0.6) and its own speed is at least 2.5 m/s. A49's attempt bookkeeping (`JAMDRILL tackle press / attempt / summary` after ten).
- `jam.ctrl.Drill 6`: the human carries the puck at the nearest standing pro bot at full speed and **gets checked** (pro presses at 5 m/s closing); `JAMDRILL kd_human n= frame=
  kind= strength= down_frames=` on each knockdown of the human, `getup_human n= frame= measured_down_frames=` on each release, `kd_summary` after ten get-ups.
- `jam.ctrl.Stills 1`: a screenshot (HUD included) every 4 sim frames after a knockdown that involves the human (24 stills = 96 frames: fall, lying, get-up, release), after the
  human's board hit (8) and after a stick hit on the human (8), twice per kind: `Saved/Screenshots/Windows/a73-<kind>-<start frame>-f<frame>.png`. The brief's "every 4
  frames, 12 stills" covers 48 frames and a knockdown is 90: the capture is every 4 frames for 96, and the strips below show every tile.
- `tools/qa/ctrl/a73-prove.ps1 -Exe <Development ThirdJam.exe> -Drill 5|6`: the game is launched on `a73-cut.json` (the v4 QA cut: `humanoid_b1`, pro bots, penalties off so
  no box pauses the drill) with `-RemoteConsoleIP=127.0.0.1 -RemoteConsolePort=4658`; the menu's PLAY is one Return **posted** to the Slate window (a foreign Windows
  Security dialog holds the foreground on this machine, as A54 / A58 found; SendInput cannot reach the game); the drill runs itself; `quit` goes over the remote console;
  the stills are moved to `tools/qa/out/ctrl/<name>/stills/`. `a73-analyze.mjs` reads the log.

## 4. Counts (Development packages `Game/Packaged/A73-dev` and `A73-dev2`, 1280 x 720, the human driven by the drills; logs in `tools/qa/out/ctrl/<run>/game.log`, verdicts by `a73-analyze.mjs`)

| Run | Drill | Frames | `JAMVIS hit` | `board` | `pin` | `stickhit` | knockdowns (of the human) | get-ups | the human's checks landed | strips | `JAMHITSTOP` | `JAMKICK` | presentation held | camera on the lying human |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| `a73-side` (package 1; the cut title was over the 60-character limit so it played `hockey.regular`, humanoid_b1, pro; the driver re-opened the map once, so the first 1700 frames are in the log twice) | 5 | 15 000 | 94 | 73 | 62 | 39 | 10 (2) | 9 (+1 still down at quit) | 6 of 14 presses (drill summary: 10 attempts, 4 landed, 6 stripped, 4 knockdowns, 4 `SetKnockedDown`) | 9 | 8 = every knockdown that involved the human | 8 | 8 of 8 | 178 of 178 JAMCAM frames in the middle third over 2 down windows, max 0.196 / 0.255 |
| `a73-taken` (package 1, `a73-cut.json`) | 6 | 8 400 | 42 | 24 | 4 | 23 | 32 (**1**) | 32 | 31 of 31 (the drill takes the puck back with a check when a bot carries: every press landed) | 31 | 32 | 32 | 32 of 32 | 89 of 89, max 0.151 / 0.258 |
| `a73-side2` (package 2: the coast rule in every branch, the press at 2.5-6.5 m/s on a side push judged at contact) | 5 | 9 600 | 80 | 44 | 6 | 22 | 1 (0) | 1 | **0 of 3 presses** (at 5.9, 6.1, 3.9 m/s; the strict rule pressed three times in 160 s) | 1 | 0 | 0 | (no hit involved the human) | (never down) |
| `a73-taken2` (package 1, 300 s) | 6 | 16 200 | 96 | 51 | 22 | 45 | 57 (**2**) | 57 | 55 of 55 | 55 | 56 (one knockdown fell inside a running hold) | 57 | 56 of 56 | 178 of 178, max 0.153 / 0.258 |

What the counts say:

- **The chain fires from the read-outs in the package on both figures' calls**: 10 + 32 knockdowns, each with its `JAMVIS knockdown ... sim_frames=90 sent_frames=87 hitstop=3` and its
  `JAMVIS getup` exactly 90 frames later (the analyzer checks `sent = sim - hitstop` on every one: true on all 42). Every check the human landed stripped the carrier (C-0005).
- **The hit felt fires on every knockdown that involves the human and on no other**: `JAMHITSTOP` 8 / 32 against 8 / 32 such knockdowns; the camera's `JAMCAM` line (one per
  pushed presentation frame) is present on the hit frame, absent on the two frames after it and present again on the release frame, on 40 of 40 hit-stops: the presentation
  held while `JAMSIM` kept hashing (`frame=15000 hash=256a859d9c6eb275`, `frame=8400 hash=7b046b4a0b904a04` in the two logs; the editor's `Jam.Core.Determinism` baseline is
  untouched because `JamSim.{h,cpp}` have no diff).
- **The brief's drill 6 premise measured wrong**: at pro the bots **lift** the puck off the human (23 `stickhit` on the human in 140 s, 7 clean lifts in the first 60 s) and almost
  never check it (1 knockdown of the human in 140 s, at frame 5740, strength 0.85, kind check). A45's bot check needs the carrier inside 1.3 m with the closing speed **and the bot's
  own speed along the line** both at 5 m/s or more and the push not from behind; a bot that closes on a carrier skating at it reaches `lift_range_m` (1.2 m beyond the bodies)
  first and lifts. "Ten of ten" bot checks on the human is a brain calibration (`opp.check_min_closing`, or a preference for the check over the lift when the closing speed is
  there) that this lane does not own; the one taken hit is the whole chain (section 5, `a73-kd-human-f5740-strip.png`) and the 300 s second run adds one more (frame 12353, strength 1.00): **2 bot checks on the human in 270 s of carrying at them**, against 55 lifts and bumps.
- **From the side at 3-6 m/s** (drill 5): run 1 pressed at 2.6-12.0 m/s because the puck chase before the approach arrived at top speed and the side was judged at the press, not at
  contact (the hits landed as `side_l`, `behind`, `behind`, `front`...); run 2 (the strict rule: 2.5-6.5 m/s and a side push judged at the predicted contact) pressed **three times in 160 s, all at 3.9-6.1 m/s, none landed**: against a pro carrier that turns and accelerates, the side-at-walking-pace geometry is rare and the press judged ahead of contact misses the 12-frame window. **The loose drill of run 1 is the useful one**; a scripted side hit at 3-6 m/s on a *standing* carrier (A16's way) would give ten of ten but would not be play. The operator's own side hits land the way run 1's did: the human presses when the bodies are about to meet, and the strength is whatever the closing speed gives (0.2-1.0).

## 5. Stills (`docs/experiments/img/a73/`, every 4th sim frame, the tile label is the sim frame and the offset from the hit)

| Strip | What it shows | Hit |
|---|---|---|
| `a73-kd-human-f5740-strip.png` | **the human knocked down by a pro bot** (drill 6, strength 0.85, front): +0 the contact and the burst, +4..+12 the backward fall with the hit-stop on +0..+2 (one pose over three frames: the stills every 4 cannot show it, the log does), +16..+68 lying on its back, the stick across, the **YOU ring** on it the whole time and the camera on it (89 of 89 frames in the middle third), +72..+88 the get-up, +92 standing and skating on the release frame +90 | 5740 |
| `a73-kd-human-f3730-strip.png` | the human knocked down in run 1 (front, strength 1.00) at the faceoff scrum after a goal; the same chain, the opponents around it | 3730 |
| `a73-kd-bot-f555-strip.png` | **the human's check** on orange 3 (side_l, 0.48, kick 6 cm): the burst, the victim tipping over the next 16 frames, lying with the stick while the human skates on with the puck, up at +90 (the victim recedes with the camera: the "vanishing" the eyes report is the perspective) | 555 |
| `a73-kd-bot-f864-strip.png` | the human's check at full strength (front, 1.00, kick 12 cm): the victim flat on its back by +12 | 864 |
| `a73-board-f712-strip.png` | the human into the boards (`PlayBoardHit`, `Board_Hit`): the lean over +0..+12 | 712 |
| `a73-stickhit-f886-strip.png` | a stick lift on the human (`PlayStickHit(true)`): the stick up and the puck gone | 886 |

Raw stills (128 + 104 + 32 + 128, `a73-<kind>-<start>-f<frame>.png`, 1280 x 720 with the HUD) in `tools/qa/out/ctrl/<run>/stills/` (out of the tree).

## 6. Eyes (`tools/eyes`, gemini-3.8-flash `--no-think`, bug-hunt, about a quarter of a cent each; JSON in `img/a73/eyes/`; advice, not instructions)

- `a73-kd-human-f5740`: "the knocked-down skater's limbs and torso clip flat into the ice" (severity 3, 0.85): **true**, B's fall take ends with the pelvis under the ice plane (A33 and
  A45 saw the same on the toy and on B's v4; section 7 ask 1); "the white ring floats detached behind the checking orange skater at +16" (3, 0.95): that is the carrier ring of
  orange 3, which just took the puck, around its own skates: **not a bug**.
- `a73-kd-human-f3730`: the same ice clip (3); "a disconnected helmet at the bottom edge" (2): the teammate behind the camera; "the bark banner over the heads" (2): the bark widget's
  anchor, lane A3's.
- `a73-kd-bot-f555`: "the orange skater shrinks and vanishes between +52 and +60" (4, 0.95): the camera follows the human away; the victim is 20 m off by then: **perspective, not a bug**;
  the ice clip on the victim (3); "a dark bar across the bottom of the first two tiles" (2): the controls strip of the first ten seconds.
- `a73-board-f712`: the puck's disc read as "a flat yellow ring" (2); "orange 88 standing inside the net's frame" (4, 0.95): the bots' crease stance behind the goal line, lane A50's,
  not tackling; the net cut by the tile edge (2).
None of it is a call that fired wrong or late.

## 7. What B needs (asks, with the call or parameter)

1. **The fall ends under the ice.** `Knockdown_*` / `Fall` leave the pelvis 10-20 cm below the ice plane in the lying pose (every eyes pass since A33 reads it as "sunk" or "clipping",
   the stills show it at 6 m). Ask: the lying pose's pelvis at skate height; no C-0001 change.
2. **The hit-stop stops the figure's clock, not its timers.** During the hold the pawn and B's visual actor get `CustomTimeDilation` 0.0001 for N frames (the montage freezes on its first
   fall frame) and the victim's `SetKnockedDown(true, dir, KnockdownFrames)` already carries `90 - N` (87 at the default). If the Blueprint schedules its get-up by the montage's own
   time, it ends on the release as measured (the strips: up by +88, skating at +92). If it schedules by a **timer or a Delay** (world time, which the dilation does not scale),
   the get-up ends N frames early. Ask: time the chain from the montage (or from `UpdateMotion` calls, which the pawn withholds while held), and B can read the hold as
   `GetOwner()->CustomTimeDilation < 0.01` if a take wants to know.
3. **The strength scale for a stronger `check` / `knockdown` burst and sound** (B-to-A-0036 point 4's question): the camera uses `max(0.5, Strength)`; the Niagara and MetaSound
   already receive `User.Strength` / `Strength` as the event's 0..1 (`EffectsBinder.cpp`, unchanged by this lane). A stronger variant by Strength needs no new Variant row: scale the
   burst count and the sound's gain by `max(0.5, Strength)` inside the assets, as the kick does; the human's own hits land at 0.2-1.0 (section 4), the pro bots' at 0.5-1.0. If B
   wants a **separate "human involved" variant** (the hit the operator feels), say so and the binder gets a Variant 4 for `check` / `knockdown` when Who or the victim is the human
   (one line in `VariantFor`, the human index from the sim).
4. **A side-fall take** is still the missing one (A33 section 6): the hits land `side_l` / `side_r` about a third of the time (run 1: side_l 0.48 at 555, side_l 0.55 at 714, side_r 0.53
   at 2145) and B's figure falls backward or forward turned toward the push. The direction is on `SetKnockedDown`'s `(DirX, DirZ)` as before.
5. **The board pin never showed in the stills**: `JAMVIS pin` fired 62 + 4 times but no capture is bound to it (this lane captured knockdown, board hit and stick hit). The call fires
   (A45 proved it on the toy); a `Board_Pin` still is one more `StillsObserve` kind if B wants it judged.

## 8. The three things the operator will feel first

1. **The hit lands with weight**: on every check the human throws or takes, the picture stops for three frames (50 ms) on the impact pose while the burst blooms, then everything moves
   again, the figures catching up over the next three; `jam.hitstop 0` makes the same hit feel like a bump, `jam.hitstop 6` makes it heavy.
2. **The camera is shoved the way the body goes**: 6-12 cm along the push, settling over eight frames; on a hit at the glass it slides along the boards instead of into them. The
   operator can turn it up with `jam.hitkick 2`.
3. **Taken down, you stay in the picture**: the fall, the lying body with the YOU ring on it and the get-up are framed in the middle third for all 90 frames, and the figure is back on
   its skates the frame the sim lets it act; what the operator will miss is the bots knocking him down at all: at pro they lift the puck off him instead (section 4).

## 9. Files (relative to the worktree)

- `Game/Source/JamCore/Public/JamPawns.h`, `Game/Source/JamCore/Private/JamPawns.cpp`: the hit-stop (`jam.hitstop`, `BeginHitStop`, the hold, the catch-up, `JAMVIS knockdown / getup` lines).
- `Game/Source/JamGame/Public/CameraFollow.h`, `Game/Source/JamGame/Private/CameraFollow.cpp`: the kick (`KickOffset`, `StartKick`, `jam.hitkick`, `JAMKICK`).
- `Game/Source/JamGame/Public/HokejGameMode.h`, `Game/Source/JamGame/Private/HokejGameMode.cpp`: `HitStopRoute`, the presentation hold, drills 5 and 6, `StillsObserve` (`jam.ctrl.Stills`), `KdObserve`.
- `tools/qa/ctrl/a73-prove.ps1`, `a73-analyze.mjs`, `a73-strip.py`, `a73-cut.json`.
- `docs/experiments/TACKLING-2.md`, `docs/experiments/img/a73/*.png`, `docs/experiments/img/a73/eyes/*.json`, `status/A73.md`.
- Not touched: `Game/Plugins/JamAssets`, `JamSim.*`, `EffectsBinder.cpp` (the Strength scale is an ask, section 7 point 3), the contracts.
