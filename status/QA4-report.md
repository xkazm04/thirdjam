# Lane QA4 report: the play-through on the new build, and B's figure against the new contact model

Tree `C:\Users\kazda\jam\thirdjam` (`dev-b` 40a4c1c), port 8002, one editor (pid 3752), left through `ExitEditor` at 21:12:46 and confirmed gone. My clock 20:45 to 21:20 (it read 20:45 when your brief said about 21:05).

## Verdict

1. **The build passes the knockdown lines and fails the puck line.** The human goes down 3 and 2 times in 100 s and nobody falls again within 2 s of a release; but the same carrier loses the puck again inside half a second 3 and 9 times.
2. **B's figure holds its timing on the new model and misses its new events.** The get-up ends on the release at every length and hit-stop; but a stumble plays the bump's take (and one of three did not show at all), the board take never plays when it arrives with a knockdown or a stumble, and a hard stop and the abilities show nothing.
3. **One safe saving went into both Blueprints**, proven on two seeds with identical log lines. The machine was loaded: the game ran at 27 and 37 sim frames per wall second, in slow motion, so every number here is in sim frames.

## How it was run

- Regular cut, five a side, `pro`, the scripted human, on scratch copies of `hockey.regular` with the tier set to `p3_clay` (outside the repository, deleted).
- Seeds 1 and 7920, each to sim frame 6090 (the drop at frame 90 plus 100 s).
- **Tick rate:** 27.5 sim frames per wall second on seed 1, 36.7 on seed 7920; a first short run fell to 11. The sim steps once per rendered frame, so the matches took 226 and 171 s of wall time.
- `jam.vis.lowlog 1` was on for the `JAMLOW` read-out.

## 1. The acceptance lines

| Line | Seed 1 | Seed 7920 | Verdict |
|---|---|---|---|
| The human knocked down fewer than five times in 100 s | 3 | 2 | pass |
| Nobody down again within 2 s of a release | 0 cases | 0 cases | pass |
| A carrier keeps the puck at least half a second between stick takeaways | 3 cases under 30 frames | 9 cases under 30 frames | **fail** |

- **The puck line.** Each case is the same carrier losing the puck to a clean poke or lift less than 30 frames after losing it the last time. Seed 7920 has two runs of clean pokes on the human 14 frames apart (frames 2132 to 2174 and 4261 to 4317). Seed 1 has frames 5059, 5073, 5078. The shortest gap is 4 frames.
- **Who falls.** Five of the six knockdowns are the human's; one bot fell in 200 s.

Events in 100 s (sim's `JAMEVENT` lines):

| Event | Seed 1 | Seed 7920 |
|---|---|---|
| check | 4 (side 3, front 1) | 5 (front 5) |
| of those: knockdown | 3 | 3 |
| of those: stumble | 1 | 2 |
| lift | 17 (7 clean, 10 miss) | 12 (7 clean, 5 miss) |
| poke | 10 (7 clean, 3 miss) | 14 (13 clean, 1 miss) |
| board | 32 (21 skater, 11 puck) | 33 (21 skater, 12 puck) |
| shot | 9 | 11 |
| goal | 0 | 1 |
| pass | 84 | 64 |
| ability | 0 | 0 |
| cross-check (`check` kind `cross`) | 0 | 0 |

- **Bumps are not an event**, so I cannot count them. `PlayHitReaction` reached the figures 108 and 75 times; all but the 3 stumbles are logged as collisions.
- Knockdown lengths: 99, 114, 149 frames (seed 1); 70, 110, 150 (seed 7920). Hit-stops on the human: 3, 4, 5 and 2, 5; a stumble holds 1 frame.

### The QA2 table, with this build

| # | QA2 fault | QA3 | This build |
|---|---|---|---|
| 1 | Black beyond the boards | fixed | still fixed; benches and boxes now stand there |
| 2 | The human knocked down again and again | 15 in 109 s | **3 and 2 in 100 s** |
| 3 | The same reactions over and over at the boards | stick reactions played 49; the human's busiest 2 s had 6 take starts | stick reactions played **7 and 9**; the busiest 2 s of any figure has 5 take starts |
| 4 | Stick and board events about 2.3 a second | lift 0.72, poke 0.62, board 0.83 a second | lift **0.17 and 0.12**, poke **0.10 and 0.14**, board 0.32 and 0.33 a second |
| 5 | The first match starts without its lights | seen, 2 s | **seen again**: un-rigged at 3 s of wall time, clay at 8 s (`qa4-first-seconds`); 86 rendered frames from build to "rig loaded" on this loaded machine |
| 6 | The get-up runs on after a foul whistle | fixed on B's side | no foul in either match; not seen |
| 7 | Pale arcs behind a fast puck | not seen | not looked for |
| 8 | The attacker's cross-check does not show | 7 of 79 | lift take blocked 2 of 17 and 3 of 12 |
| 9 | The editor asserts on stop after the menu | not re-tested | not re-tested |

## 2. The figure against the new contact model

### Every take against its event (100 s, both seeds; sent by the sim / what the figure did)

| Call or event | Seed 1 | Seed 7920 | What plays, and why some are held back |
|---|---|---|---|
| `PlayHitReaction` | 108: 94 played, 13 skipped, 1 blocked | 75: 66 played, 7 skipped, 2 blocked | `Check_Hit_Left` / `_Right` / `_Back` by direction. Skipped while down or getting up, within 0.6 s of the last hit, or within the 0.35 s gap after a stick reaction |
| of those: a stumble | 1: **skipped** (gap, 19 frames after a stick reaction) | 2: played as `Check_Hit_Right` | the same take as a bump; the figure cannot tell them apart |
| `PlayBoardHit` | 21: 17 played, 2 blocked, 2 skipped | 21: 17 played, 4 skipped | `Board_Hit`. Skipped while down or within 0.6 s of a hit |
| of those: with a knockdown or a stumble in the same frame | 1: skipped | 2: skipped | see below |
| `PlayStickHit` | 13: 7 played, 6 skipped | 20: 9 played, 11 skipped | `Lift_Hit` / `Poke_Hit`; the 0.35 s gap holds the rest back |
| `PlayLift` (attacker) | 17: 15 played, 2 blocked | 12: 9 played, 3 blocked | `Lift`; blocked when the check lunge is playing |
| `PlayPoke` (attacker) | 10 played; 3 miss stumbles | 14 played; 1 miss stumble | `Poke`; `Stumble_Short` on a miss |
| `SetBoardPin` on | 8 played, 3 waited, 4 ignored | 12 played, 4 waited | `Board_Pin` |
| `SetKnockedDown` | 3 falls, 3 get-ups | 3 falls, 3 get-ups | `Knockdown_Right`, `_Right_B`, `Fall`; `Fall` x2, `Knockdown_Left_B` |
| `check` on the bus (the attacker's lunge) | 4 events | 5 events | `Check_Deliver_Left` / `_Right`; the figure logs no line for it, so not counted |

### Bump and stumble

- Both arrive as `PlayHitReaction`, so both play the direction take. In play the stumble reads as a 15-frame jolt (`qa4-stumble`).
- **The 0.35 s gap and the 0.6 s rule are wrong for a stumble.** One of the three stumbles did not show because a stick reaction had played 19 frames earlier. A stumble should be exempt from both; the figure cannot do that until it can tell a stumble from a bump.
- All three stumbles came with a clean lift in the same frame; the stick take was skipped each time.
- The take `Stumble` exists and nothing references it.

### Knockdowns of 60 to 150 frames

In play (scaled hit-stop, `JAMLOW`):

| Length | Hit-stop | Fall, get-up | Get-up end minus release | Pelvis at 90 % minus release |
|---|---|---|---|---|
| 70 | 2 | `Knockdown_Left_B`, `GetUp_Side_Left` | 0.0 | -9 (standing height read mid-reaction; see note) |
| 99 | 3 | `Knockdown_Right`, `GetUp_Side_Right` | 0.0 | -3 |
| 110 (a bot) | 0 | `Fall`, `GetUp` | 0.0 | -2 |
| 114 | 4 | `Fall`, `GetUp` | 0.0 | -3 |
| 149 | 5 | `Knockdown_Right_B`, `GetUp_Side_Right` | 0.0 | -3 |
| 150 | 5 | `Fall`, `GetUp` | 0.0 | -3 |

On test figures (no hit-stop, pelvis height recorded per frame):

| Length | Get-up starts | Pelvis back at 90 % | Frames with the pelvis under 45 % of standing |
|---|---|---|---|
| 60 | frame 25 | frame 57 | 24 |
| 90 | frame 55 | frame 87 | 54 |
| 120 | frame 85 | frame 117 | 84 |
| 150 | frame 115 | frame 147 | 114 |

- **The 60-frame knockdown is not a twitch.** It is a whole fall and a whole get-up with no lying hold between: flat on the ice from about frame 9 to 24, sitting up at 27, standing by 57 (`qa4-short-knockdown`).
- Note on the 70-frame row: `JAMLOW` took 65.6 cm as the standing height because the figure was mid-reaction at the hit; against 74 cm the figure would read like the others. Not re-measured.
- In the test the get-up ended 1 frame before my release; that is my callback releasing a frame late, not the figure.

### `PlayBoardHit` in the same frame as a knockdown or a hit reaction

- **With a knockdown** (seed 1, frame 2953): the sim sends `SetKnockedDown` first, then `PlayBoardHit`. The fall wins (the wall rule picked `Fall`), the board take is skipped because the figure is down. Nothing pops.
- **With a stumble** (seed 7920, frames 1403 and 1705): `PlayHitReaction` first, then `PlayBoardHit`, then `PlayStickHit`. The direction take wins; the board take and the stick take are skipped.
- So the board take played in **0 of 3** same-frame cases. Nothing reads as "driven into the boards". The test strip shows both cases (`qa4-board-and-down`).
- In that strip the skates of the figure playing `Check_Hit_Left` leave the ice around frames 9 to 12.

### The rest

- **Cross-check under the cone.** No `check` of kind `cross` and no cross-check drill run in this lane, so the figure's turn was not exercised. The code is unchanged: it turns toward the event's position on a clean `lift`, up to 60 degrees.
- **`NS_lift` and `User.Dir`.** The binder sets `User.Dir` on `lift` and `poke` (`EffectsBinder.cpp` line 400). Whether the mark sits on the attacker's side I did not see in a frame.
- **The release grace.** Held in both matches: no second fall within 120 frames of a release, so no get-up was cut.
- **The hard stop.** The figure shows nothing. It gets only `Speed` through `UpdateMotion`, and the stride blends down with it (`qa4-brake`, 11 m/s taken down at 20 m/s^2). The visual interface has no brake call. The take `Stop` exists and nothing references it.
- **The `ability` event.** None fired in 200 s. The figure has no branch for it. The binder logs `NS_ability` and `MS_ability` missing. Missing for all four kinds: an effect, a sound, and any pose.

## 3. The wall rule and the rink read

- **The rink read still works.** Test figures read 4500 / 2250 / 375 cm (half length, half width, corner radius); the sim's line says 90 x 45 m, corner 3.75.
- **No accessor exists.** `RinkActor.h` exposes `IceMid` as read-only but no size or corner radius; nothing in `contracts/` names one.
- **Benches and boxes do not clip.** They stand outside the boards. A figure knocked toward the boards under the bench and beside the boxes lies inside the rink along the wall (wall kinds 1 and 2 chosen; `qa4-bench-box`).

## 4. The cost of the figure Blueprint

**Applied, in both Blueprints:** `SK2Tick` pushed the aim values to the anim instance twice per `UpdateMotion`; the first push was overwritten by the second in the same call. I removed the first.

| Per `UpdateMotion` (nodes in the text of `UpdateMotion` + `SK2Tick` + `SK3Fix` + `PushAnim`) | Before | After |
|---|---|---|
| `PushAnim` runs | 2 | 1 |
| All nodes | 412 | 366 |
| Function-call nodes | 106 | 101 |
| Variable writes | 46 | 34 |
| Casts of the anim instance | 4 | 3 |

**Proof:** the same seeded match before and after, compared line by line (sim events, `JAMVIS`, and the figures' own lines with their take names):

| Seed | Compared to frame | Lines | Identical in order |
|---|---|---|---|
| 1 | 1140 | 117 (31 `JAMVIS`, 27 take lines) | 117 |
| 7920 | 2600 | 270 (80 `JAMVIS`, 64 take lines) | 270 |

After the write, 43 of the 44 event blocks read back unchanged in each Blueprint, and the 44th is exactly the intended text.

**Found and not applied** (each needs a variable or changes an order, so not plainly safe):

| Where | Work per call | Could be |
|---|---|---|
| `SetStickAim`, `SetShotCharge` | each runs `PushAnim` again | pushed once, from `SK2Tick`; depends on the order the pawn calls them in |
| `UpdateMotion`, `SK3Fix`, `PushAnim` | `GetAnimInstance` and a cast each | one reference kept from `BeginPlay` |
| `SK2Tick` | the frame time read and clamped 4 times | once, into a local |
| `SK3Fix` | `WasRecentlyRendered` 3 times, `IsAnyMontagePlaying` 3 times, `stick_socket` looked up by name twice | once each, into locals |
| `UpdateMotion`, standing | the mesh rotation set every call | only when the yaw changes |
| `ReceiveTick` | anim instance, active montage and game time read every tick | skipped when the figure is not down, getting up, pinned or shaded |
| `OnBusEvent` | every figure answers every bus event with three name compares | per event, not per frame; left |

**Not on the per-call path:** the rink read and the wall test (only at a knockdown), the contact shade (only while down), log lines (only on events), the slide effect (only at a knockdown). `JAMLOW` is off by default.

I could not time any of it.

### Files and ledger rows

| File (under `Game/Plugins/JamAssets/Content/Skaters/`) | Bytes before | Bytes after |
|---|---|---|
| `BP_SkaterVisual_TB1.uasset` | 4,003,455 | 4,001,197 |
| `BP_SkaterVisual_TB1_P3.uasset` | 4,207,480 | 4,205,098 |

Saved at 21:11 with no play session open. End check: 739 assets load, 16 Blueprints up to date, no dirty package, `jam.*` defaults restored, `jam.vis.lowlog` 0.

```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.82 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3): a gap of RxGap 0.35 s between a figure's stick and standing reactions, an early release finishes the get-up in 6 frames, a board pin waits out a reaction younger than RxGap and a stick reaction does not replace a pin younger than RxGap; SK2Tick pushes the aim values to the anim instance once per call, not twice (lane QA4) (4001197 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 4.01 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16); SK2Tick pushes the aim values once per call (lane QA4) (4205098 bytes) |
```

## 5. The look on clay

- **Benches and boxes:** they fit the clay voice (thumb-pressed walls, team-coloured rails). Their walls are greyer than B's cream boards, and the black clock faces are the darkest thing in the frame.
- **Five a side:** reads; ten figures, the dark numbers legible in play (`qa4-clay-play-1` to `-4`). Nothing dark or default-material in the 39 captures, looked at as contact sheets and six at full size.
- **The figures are still stepped in play** in these captures. I did not read the drawn resolution in this lane.
- **The HUD on the pale ice (the other device's):** the plates and the commentary box read; the white distance labels ("49 m", "own 43 m") are faint on the ice.

## What is missing on B's side for the vision, by how visible it is

1. **The hard stop shows nothing**: no pose, no spray, no sound from the figure. It is the carrier's main move.
2. **A stumble looks like a bump**, and can be held back by the reaction gap.
3. **A check into the boards has no take of its own**: the board take played in 0 of 3 same-frame cases.
4. **The abilities**: no effect, no sound, no pose for any of the four kinds.
5. **`NS_deke` and `MS_deke`** do not exist (the binder logs it every session).
6. **`Check_Hit_Left` lifts the skates off the ice** for a few frames.

The Blender lane has files in the tree for a stumble, a hard stop, a short knockdown and board falls (`lab-skf21-*`); I did not look at them.

## For the other device

1. **The puck line fails.** Clean pokes land on the same carrier 14 frames apart (seed 7920 frames 2132 to 2174 and 4261 to 4317; seed 1 frames 5059 to 5078). The take grace does not seem to cover a carrier who gets the puck straight back.
2. **A stumble is logged and sent as a collision.** `JAMVIS hit` says "no check landed" for a stumble, and the stumble's `check` event carries strength 0.000 where the contract says it carries the ratio (seed 7920 frames 1403, 1705; seed 1 frame 5078).
3. **`PlayStumble`**: yes, as B answered; this lane's numbers back it (1 of 3 stumbles did not show).
4. **The order of calls in one frame** is `SetKnockedDown` then `PlayBoardHit`, and `PlayHitReaction` then `PlayBoardHit` then `PlayStickHit`. B's figure depends on it; please keep it or write it into C-0001.
5. **A rink accessor** (half length, half width, corner radius) on the rink actor would replace B's read of the `Ice` bounds and the ice material.
6. **A brake read-out for the figure**: either a call on the visual, or say that B should derive it from `Speed`.
7. **A way to fire each ability kind** without a human (a drill or a console variable): none fired in 200 s of `pro` play.
8. **The first match of a session** still shows the un-rigged rink when the rig loads slowly (86 rendered frames here).
9. **Five of six knockdowns are the human's.**

## What I could not check

- The night figure in play on this build. The same one-line edit is in its Blueprint and compiled; the before-and-after proof was run on clay only.
- The cross-check, its turn and the `NS_lift` mark: no cross-check happened and I ran no drill.
- Any timing of the Blueprint.
- The abilities, the deke, a foul.
- Sound, by ear.
- Whether seed 1 is QA2's seed; I took the cartridge's own.
- The cook (yours).

## The next three evolutions for "the quality of B's implementation"

1. **The hard stop shows on the figure.** Derive the brake from `Speed` in `UpdateMotion` (a drop of more than 0.25 m/s in a frame is over 15 m/s^2), play the unused `Stop` take and a spray. **Proving number:** reverse brakes that show a brake pose within 3 frames, out of the sim's `ReverseBrakes` count in a 100 s match: from 0 today to all of them.
2. **A stumble and a board fall of their own.** Play `Stumble` on a stumble (through `PlayStumble` when it exists), exempt from the 0.35 s gap and the 0.6 s rule, and one take when the board hit arrives with a knockdown. **Proving number:** stumbles and same-frame board cases that play their own take in the two seeded matches of this lane: from 0 of 3 and 0 of 3 to 3 of 3 and 3 of 3; two stills where the bump and the stumble differ.
3. **The per-call cost, the rest of it.** Keep the anim instance from `BeginPlay`, compute `SK3Fix`'s repeated reads once, guard `ReceiveTick`, push the aim values from one place. **Proving number:** nodes per `UpdateMotion` from 366 to under 200 with the two seeded logs identical line for line, and the other device's 0.59 ms for ten figures re-measured on their profile.

## Stills

In `C:\Users\kazda\jam\thirdjam\devlog\shots\`, each with a `.txt`; un-frozen:

- `20261113-211559-B-qa4-clay-play-1.png` (open ice), `-211601-...-2` (the boards with the penalty boxes), `-211602-...-3` (a scrum, a bot knocked down), `-211603-...-4` (the human knocked down)
- `20261113-211605-B-qa4-stumble.png`: a real stumble into the corner boards, every 3rd frame
- `20261113-211606-B-qa4-short-knockdown.png`: 60 frames, every 3rd frame, a test figure
- `20261113-211607-B-qa4-board-and-down.png`: both same-frame cases, test figures
- `20261113-211608-B-qa4-brake.png`
- `20261113-211610-B-qa4-bench-box.png`
- `20261113-211719-B-qa4-first-seconds.png` (not asked for: fault 5)
