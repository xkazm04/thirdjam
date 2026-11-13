# Lane QA3 report: the clay tier after B's fixes, against QA2's list (device B)

**Verdict**
- **Fixed:** the black beyond the boards is gone on the regular rink, Pond and the wide arena; the early release after a foul ends in 6 frames; the ladder behind the puck was not seen again; the reactions at the boards are down from 141 stick reactions to 49 in the same match.
- **Worst now:** what the other device has not changed: the human is knocked down 15 times in 109 s of the same-seed match, and the puck is taken 1.3 times a second; the first match of a session still starts 2 s without its lights.
- **Not checked:** sound by ear; the shot effect, the chips and the puck trail at the moment they fire (my captures are one every 2 to 3 s and did not catch them; I did not get to the burst script); the menu.

- Tree `C:\Users\kazda\jam\wt\b-vfx` (`dev-b` 02dc850), the only editor on the machine, port 8000 (pid 15112), left through `ExitEditor`; the process is gone and no editor is running. No git write, no cook. **Saved: only the two scripted Blueprints (item 4), with no play session open.** Clock 19:38 to 19:53.
- Method as QA2: play in the editor on scratch copies of the repository's cartridges with the tier set to `p3_clay` (outside the repository, deleted); the scripted human; **all stills are un-frozen captures with the HUD as the player sees it**, one every 2 to 3 s.
- Sessions (game time): the first 14 s of the first match; regular with another seed 111 s; **regular with QA2's seed 109 s**; Pond 30 s; wide 17 s; five a side 41 s; a tier switch clay to night to clay; drill 6 on the regular cut (two fouls); and, after item 4, the QA2-seed match once more.
- Scratch: `C:\Users\kazda\AppData\Local\Temp\claude\6d8eaecb999746c2\1f5c7b1e-557b-4c70-b752-8370d9dbef0b\scratchpad\qa2` (captures under `img\`, each match's log under `logs\full_*.txt`; QA2's own captures moved to `old\` and `logs2\`).

## QA2's faults, one by one

| # | QA2 fault | Was | Now | Evidence |
|---|---|---|---|---|
| 1 | Black beyond the boards | 5 | **Fixed.** A lit plank floor and the barn wall fill the frame above the boards, on the regular rink, Pond and the wide arena. Figures and the blue helmet read against the warm brown; nothing is lost. The apron is even, not glowing, not too bright anywhere I saw | `qa3-fault1-corner`, `-side`, `-pond`, `-wide` |
| 2 | The human knocked down again and again (other device) | 5 | **Same.** QA2's seed: 15 knockdowns in 109 s, every one the human. Another seed: 4 in 111 s, every one the human | log |
| 3 | The same reactions over and over at the boards | 4 | **Better.** Same-seed match: stick reactions played 141 (QA2), 64 (SK19), **49** (after item 4). The human's busiest two seconds: 17 take starts, then 8, now **6**, at least 20 frames apart. The sim's rates are unchanged (79 lifts, 68 pokes) | table below |
| 4 | Cross-check, poke and board sounds about 2.3 times a second (by the log) | 4 | **Same by the log**: the events are unchanged (lift 0.72, poke 0.62, board 0.83 per second; the same event twice in a frame 5 times). Whether the sound follows the event or the figure I did not check; not judged by ear | log |
| 5 | The first match of a session starts without its lights | 3 | **Same.** Seen again: 2 s in, no rig; 4 s in, clay. The log has 1.4 s from match build to "rig loaded" (2.8 s in QA2). Later matches: clay from the first capture | `qa3-fault5-first-seconds` |
| 6 | After a foul whistle the figure gets up for half a second after the release | 3 | **Fixed on B's side.** Two fouls in the drill: both took the quick finish (`SK19REL fast rem_f 30`); the get-up is over in 6 frames. The whistle still releases 30 frames early | log |
| 7 | A ladder of pale arcs behind a fast puck | 3 | **Not seen again**, nor the pale ovals, in the 200 un-frozen frames of this lane; three frames with a loose puck looked at closely | `qa3-fault7-puck` |
| 8 | The attacker's cross-check does not show 7 times of 79 | 1 | Same, by decision | log |
| 9 | The editor asserts when a session is stopped after the menu | n/a | Not re-tested: I did not open the menu in this lane | |

**Fault 3 in numbers** (QA2's seed, 104 to 109 s; sent by the sim / played by the figures):

| Kind | Sent | QA2 | SK19 | After item 4 |
|---|---|---|---|---|
| `Lift_Hit` | 75 | 75 | 47 | 34 |
| `Poke_Hit` | 66 | 66 | 17 | 15 |
| standing hit | 78 | 43 | 43 | 46 |
| board pin | 68 | 59 | 59 | 41 (32 had to wait; 9 refused by a check lunge) |
| board hit | 49 | 49 | 49 | 49 |
| attacker's `Lift` | 79 | 72 | 72 | 72 |

The human's busiest two seconds now (frame: take): 0 `Poke_Hit`, 22 `Board_Pin`, 55 `Check_Hit_Right`, 77 `Poke_Hit`, 99 `Board_Pin`, 119 `Poke_Hit`.

## New faults

| # | Fault | Visible | Whose | Where seen | Smallest fix |
|---|---|---|---|---|---|
| N1 | **A bot's cross-check repeats every 30 frames.** With the reactions thinned, the busiest figure is now a bot: in two seconds it shoves at frames 14, 44, 74, 104 (the sim's lift cooldown), with two pins and two standing hits between | 3 | the other device (the bots lift as often as the cooldown allows); B plays each one | same-seed match, recorder | a longer pause between a bot's lifts |
| N2 | **The puck disc's dark rim is stepped** when the disc is close to the camera | 1 | B (the disc's overlay on clay, B38) | `qa3-fault7-puck`, middle crop | soften the overlay's edge |
| N3 | **On Pond the top half of the frame can be one flat dark-brown area** (floor and wall) when play is at a side board | 1 | B, the look | `qa3-fault1-pond` | none needed; a matter of taste |

**What the fixes could have broken, checked:**
- **The puck's ring on clay:** a dark ellipse with a bright pale ring and the puck in it; as readable as before on the ice and against the boards. It draws under a figure lying over it in the one frame I have of that. No frame shows it sorting wrongly; I have no frame of it passing behind a standing figure.
- **Worn boards:** small dark chips on the red cap and faint marks on the white; they do not compete with the puck and do not read as anything else at the game camera.
- **The lit apron:** even, no hot spot, no glow.
- **Chips without their rim, the shot effect, the puck trail on a hard shot:** not caught in any frame (32 shots in the same-seed match); not judged.

## What passed

- **Knockdowns, un-frozen:** 15 of 15 and 4 of 4 in the two regular matches and 3 of 3 five a side, each with its take and get-up, get-up end minus release 0.00 on all; after item 4 again 15 of 15 at 0.00.
- **Figures:** `BP_SkaterVisual_TB1_P3` in every clay match; no ghost figures; numbers and the "YOU" ring as in QA2; five a side readable.
- **Tier switch** clay to night to clay: the right rig and figures in my first capture after each switch (4 s and 2 s after); rig load 1.0 s and 0.2 s by the log.
- **Look:** no grey default, no magenta, nothing blown out; boards, kick plate, nets, lamps, floor and wall on all three rinks.

## The log

- Warnings or errors naming an asset of B's: **none**, in any of the nine sessions.
- `JAMFX missing`: `NS_deke` and `MS_deke`, once in the session. No MetaSound or Niagara warning or error.
- Events per match (game seconds):

| Match | s | lift | poke | board | check | knockdown | shot | same event twice in a frame |
|---|---|---|---|---|---|---|---|---|
| regular, QA2's seed | 109 | 79 | 68 | 90 | 15 | 15 | 32 | lift 2, poke 2, board 1 |
| regular, other seed | 111 | 42 | 19 | 72 | 4 | 4 | 4 | lift 3, poke 2 |
| Pond | 30 | 0 | 0 | 12 | 0 | 0 | 1 | none |
| wide | 17 | 1 | 0 | 4 | 0 | 0 | 1 | none |

## For the other device (unchanged since QA2, confirmed)

1. The human is knocked down every 7 s at `pro` in the QA2-seed match and is the only one knocked down in both regular matches.
2. Lifts and pokes take the puck 1.3 times a second in that match; a bot shoves every 30 frames.
3. The rig loads after the match is built: the first match of a session shows about 2 s of an unlit rink.
4. A foul whistle releases a knocked-down figure 30 frames early (B now finishes the get-up in 6 frames).
5. `NS_deke` and `MS_deke` are asked for and do not exist.
6. The HUD items of QA2's list (small white labels, the caption box, the hint bar for the first seconds): as before.

## What I could not check

- Sound by ear; whether the cross-check's sound follows the sim's event or the figure's take.
- The fast things: the shot effect, chips, sparks, the cross-check mark, the slide streak, the puck trail on a hard shot. I did not use the burst script in `scratchpad\b39`; adapting it did not fit the time.
- The puck's ring passing behind a standing figure.
- The menu, and fault 9.
- Motion: stills only.
- The night figure with item 4's change (same text, not run).

## Item 4: the pin against the reactions (SK19 decision 1), as saved in both Blueprints

- **The pin waits.** `SetBoardPin(true)` while the figure's last stick or standing reaction is younger than `RxGap` (0.35 s) does not play; it is remembered (`bPinWant`) and `ReceiveTick` starts it as soon as the reaction is `RxGap` old, if the figure is still free to play it. `SetBoardPin(false)` clears the wish.
- **A stick reaction does not replace a young pin.** `PlayStickHit` is held back while a pin younger than `RxGap` plays (`PinT`).
- The pin's own start moved into a new event `PinOn` (shared by the two places that start it). New variables `PinT`, `bPinWant`. Priorities unchanged.
- **Measured** (QA2's seed, 109 s): pins played 59, now 41 of 68 (32 had to wait, some of those ended before their turn; 9 refused by a check lunge as before); stick reactions 64, now 49; the human's takes in the match 104, now 76; its busiest two seconds 8 starts, now 6 (sequence above). The busiest figure is now a bot (N1).
- Not covered: a standing hit can still follow a pin after 5 frames (seen on the bot: pin at 0, `Check_Hit_Right` at 5).

| File (`Game/Plugins/JamAssets/Content/Skaters/`) | Before (bytes) | After (bytes) |
|---|---:|---:|
| `BP_SkaterVisual_TB1` | 3,898,568 | 4,003,455 |
| `BP_SkaterVisual_TB1_P3` | 4,102,906 | 4,207,480 |

Both saved at 19:48 with no play session open, both `BS_UP_TO_DATE`; 739 assets under `Skaters/` load, 16 Blueprints and AnimBPs up to date, no dirty package at the exit.

Ledger rows (replace the two):
```
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1.uasset | other | 3.82 | original | Blender 4.2 Python script (lane S4) + UE 5.9 via MCP/Python | | | 2026-11-13 | B | y | implements IJamSkaterVisual (lane B4); SetNumber live, SetTeam sets CrestTeam (lane B12); C-0001 v4 (lane SK6); tackling (lanes SK12, SK14, SK15); + SlideFx Niagara component (NS_slide_trail) (lane B30); walls (lane SK16); cross-check (lanes SK17, SK18); reactions (lanes SK19, QA3): a gap of RxGap 0.35 s between a figure's stick and standing reactions, an early release finishes the get-up in 6 frames, a board pin waits out a reaction younger than RxGap and a stick reaction does not replace a pin younger than RxGap (4003455 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/BP_SkaterVisual_TB1_P3.uasset | other | 4.01 | original | UE 5.9 editor via MCP/Python (Blueprint DSL) | | | 2026-11-13 | B | y | duplicate of BP_SkaterVisual_TB1 for the p3_clay figure: P3 takes, ABP, blade offset (5.13,-140.72,9.65), H0 4.6, number on the Trim-slot patch (lane SKI3); C-0001 v4 (lane SK6); ContactShadow plane (lane SK8); tackling, walls, cross-check and reactions as BP_SkaterVisual_TB1 (lanes SK12, SK14, SK15, SK16, SK17, SK18, SK19, QA3); + SlideFx Niagara component (NS_slide_trail) (lane B30); ContactShadow 2.2 x 0.95 m, Darkest 0.75, Flat 0.7 under the pelvis while down (lane SK16) (4207480 bytes) |
```

## Stills (`C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`, each with `.txt`; all un-frozen)

- `20261113-194159-B-qa3-fault1-corner.jpg`
- `20261113-194159-B-qa3-fault1-side.jpg`
- `20261113-194531-B-qa3-fault1-pond.jpg`
- `20261113-194532-B-qa3-fault1-wide.jpg`
- `20261113-194200-B-qa3-fault5-first-seconds.jpg`
- `20261113-194532-B-qa3-fault7-puck.jpg`
- `20261113-194533-B-qa3-clay-play-1.png`
- `20261113-194533-B-qa3-clay-play-2.png`
- `20261113-194534-B-qa3-clay-play-3.png`
- `20261113-194832-B-qa3-clay-play-4.png`
  (open ice; a corner scrum; in front of the net after a shot; five a side)
