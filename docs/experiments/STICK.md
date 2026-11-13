# Stick handling with the mouse, the spin on E, the pass on F (lane A51, device A, R-0034, 2026-11-13 00:45-02:15 CET)

The operator asked for advanced stick and puck control with the mouse, so a player beats a defender with movement and the blade, and a special move on a
key such as E. R-0034: the carried puck lives where the blade is; a fast sweep shields it from a poke on the side it left; a defending bot reads the puck
on the carrier late by difficulty; a spin on **E** (gamepad B) rides the puck on the far side of the body; the pass moves to **F**; the spin takes lockstep
flag bit 512 (protocol 5); every control proof comes from a packaged Development build through SendInput. Status with timestamps: `status/A51.md`.

Code: `Game/Source/JamCore/{Public,Private}/JamSim.{h,cpp}` (the sweep, the spin, the carrier rule), `JamCartridge.{h,cpp}` (the keys), `AI/JamAICore.{h,cpp}`
(the bots' delayed read), `AI/DifficultyProfile.cpp` (14 / 8 / 5), `AI/JamSimAdapter.cpp`, `AI/JamPlaytester.{h,cpp}` (the regular and good players sweep and
spin), `JamNet/JamNetPackets.{h,cpp}` (bit 512, protocol 5), `JamGame/InputMapping.{h,cpp}` (E spin, F pass, B spin), `HokejGameMode.{h,cpp}` (`ReadHumanInput`,
the `JAMCTRL sweep` / `spin` lines, the 1v1 drill), `JamPawns.cpp` (the spin's turn of the figure), the three controls texts, `schema/game.schema.json`,
`limits.json`, `DEFAULTS.md`, tests `Private/Tests/JamStickHandlingTests.cpp`, harness `tools/qa/ctrl/a51-prove.ps1` + `a51-analyze.mjs`.

## 1. The model

**The puck lives where the blade is.** The carry assist already pulled a carried puck to `BladePos` every frame (68 % of the gap per frame at `carry_assist`
0.6: on the blade within three frames), and `BladePos` is the aim: `Pos + dir x (0.4 + |Stick| x reach_m)`. So as the cursor sweeps across the skater, the puck
goes forehand to backhand across the body. `game.stick.reach_m` keeps R-0022's **1.2** (the blade's full reach from the body 1.6 m) with the brief's range
0.6..1.6; the brief's default 1.1 was tried and measured (section 6): it halved the passes the regular player receives at pro (`Jam.Bot.PassesToHuman` 1.55
-> 0.80 against a bar of 1.0), so the Director decides whether the shorter reach is worth that; the code and the schema take either.
Found and fixed on the way: a puck at the blade's full reach sat exactly at `shot.reach_m` (1.5) and, on any fast aim change, fell off the carrier for the
frame in which the blade had moved and the puck had not (both of `CarrierIndex`'s tests failed at once: the body distance by floating point, the blade
distance by 2 m). `CarrierIndex` now carries anything inside `0.4 + reach_m + 0.05` of the body as well; without it no sweep could exist.

**The sweep** (`StepStick`): on the carrier, when the blade's direction turned more than `sweep.min_deg_per_frame` (6) since the last frame, the shield opens:
`SweepFrames = shield_frames` (12) and `SweepSide` = the side of the body the puck left (the sign of `Cross(Forward, blade - body)` of the old blade, or the
opposite of where it went when the blade was on the body line). An opponent's poke whose blade is on that side is refused while the shield runs
(`IsPokeShielded`; the window runs out as a miss and, armed next to the carrier, stumbles). The shield ends at once when the puck is no longer carried. A39's
tap deke (Q, wheel) arms the same shield for the side the puck leaves: it is the one-tap sweep. One `Sweeps` count and one `bSweepThisFrame` read-out per
start (a re-arm on the same side within the shield counts nothing).

**Beating a defender is real** (`FJamBrain::RecordPuck` / `PuckSeen`): the brain keeps a 32-frame ring of the puck's offset on the carrier's body. Against an
opposing carrier a bot's blade and poke timing read `carrier body now + offset React frames ago`: the body is read live (a straight carry is poked exactly
as before), the stick work late. React: rookie 14, pro 8, all_star 5 (`FJamBotSkill::SweepReactFrames`), or `game.stick.sweep.bot_react_frames` for every
level. A fresh carrier (changed inside the delay) is read live. A39's deke commit wins over it while it runs. So a sweep timed when the defender's stick
is on the puck leaves its blade on the old spot for React frames; its poke (after `PokeFrames` in reach) lands there and misses; a sweep too early is
simply re-read 8 frames later and the poke comes 7 frames after that (the test's first script found exactly this: the sweep is a timing, not a button).
The sweep never defends a body check: that is the sprint's and the spin's job.

**The spin** (`TrySpin` / `StepSpin`, `game.skater.spin`): E / gamepad B, an edge. Refused with an empty bar, inside the cooldown, while already spinning,
down, stumbling or boxed. On the press: `stamina_cost` (0.25) of the bar is taken (the bar refills 180 ticks a resting frame as A49 built it: a quarter
refills in 90 frames, the cooldown's length), `SpinFrames = SpinLength = frames` (24), `SpinReadyFrame = now + frames + cooldown_frames` (90), the turn
direction away from the nearest opponent's side. For the 24 frames: the speed is capped at `speed_mult` (0.7) of the skater's speed; a spinning carrier's blade
is not re-aimed but dragged round to the far side of the body from the nearest opponent (`SpinPuckTarget`: 0.7 m from the centre, 40 % of the gap per
frame: there within 5 frames) and the carry assist takes the puck with it; `IsPokeShielded` is true for every opponent (the body is between). A body check
that lands mid-spin: `KnockDown` multiplies the frames by **1.5** (135 of 90) and ends the spin; the puck is stripped as any landed check on a carrier (C-0005).
The sim's own `Yaw` never turns (the body keeps its line, the physics stay A5's); the figure turns the full circle through the existing calls
(`UpdateMotion`'s facing and the actor's rotation get `SpinVisualYawDeg` = 15 degrees x frames elapsed; the stick aim is measured against the turned
facing so the blade stays over the puck). No new interface event (C-0001 v4 unchanged).

**Hash**: `SweepFrames` / `SweepSide` while the shield runs, `SpinFrames` / `SpinLength` / `SpinDir` while the spin runs, `SpinReadyFrame` once set; the
stamina as before (once it left full). `Sweeps` and `Spins` are stats like `Dekes`.

## 2. The keys (`schema/game.schema.json`, `limits.json` game_ranges, `DEFAULTS.md`; gen-schema-data, gen-parity 81 cases, gen-designer-data 36 cuts PASS)

| key | default | range | what |
|---|---|---|---|
| `game.stick.reach_m` | 1.2 | 0.6..1.6 | the blade's reach from the body's edge at full aim; the carried puck sits there (the brief's 1.1: see section 6) |
| `game.stick.sweep.min_deg_per_frame` | 6 | 1..45 | an aim turning more than this in one frame on the carrier is a sweep |
| `game.stick.sweep.shield_frames` | 12 | 0..60 | frames a poke from the side the puck left misses; 0 = off |
| `game.stick.sweep.bot_react_frames` | absent | 0..30 | a defending bot reads the puck's place on the carrier this late; absent = rookie 14, pro 8, all_star 5 |
| `game.skater.spin.frames` | 24 | 0..120 | the full turn; 0 = off |
| `game.skater.spin.cooldown_frames` | 90 | 0..600 | before the next spin |
| `game.skater.spin.speed_mult` | 0.7 | 0.2..1 | top speed while spinning |
| `game.skater.spin.stamina_cost` | 0.25 | 0..1 | share of the sprint bar taken on the press |

## 3. The bindings

E spin (gamepad B), F pass (gamepad X unchanged), Q / wheel deke, Shift sprint; the brake left gamepad B for the **left stick click** (Ctrl unchanged).
`UJamInputMap::BindingTableText`, the first-10-seconds strip, the settings card and the pause menu's controls card say so; `Jam.Input.Mapping` asserts E
spins, F passes, E no longer passes, B spins, B no longer brakes, the left stick click brakes. `ReadHumanInput` reads `bSpin` from `IA_Spin` and logs
`JAMCTRL frame=N btn=spin (E / B)` and `btn=pass (F / X)`; the game mode logs the sim's own read-outs on the human: `JAMCTRL frame=N sweep side=S
shield_frames=12 blade=(x,y)` and `JAMCTRL frame=N spin frames=24 dir=D stamina=0.75 carrying=1`.

## 4. The wire

`FJamNetQuantInput::EFlag::Spin = 512` (an edge, in `EdgeFlags`), `JamNet::Version = 5`, `CoveredFields` + `bSpin`, still 10 bytes per input (two flag bytes
since protocol 4). `Jam.Net.InputCoverage` (with a new spin-bit-alone assertion), `Jam.Net.PacketRoundTrip`, `Jam.Net.LockstepTwoPeers`: PASS.

## 5. The tests (`Jam.Core.*`, `Game/logs/a51-tests4.log`, `a51-tests5.log`)

| test | what | result |
|---|---|---|
| `SweepBeatsPoke` (a) | a standing carrier, the puck forward-left; a pro defender (the brain's rule scripted: blade on the puck's body offset 8 frames late, poke after 6 frames in reach) arrives 2 m in front at frame 13; the carrier sweeps the aim across the body when the defender's blade has sat on the puck 4 frames | **10 of 10 seeds** kept, the defender's poke (frame 26) a miss on the old side 10 of 10; the carrier that does not sweep loses it to a clean poke (frame 19) **10 of 10** |
| `SweepBeatsPoke` (b) | the shield alone: a 20-degree sweep, the defender's blade on the old spot 0.32 m from the puck (take radius 0.47) the same frame | shield 12: refused, the carrier keeps it; `shield_frames` 0: the same poke takes it clean |
| `SweepBeatsPoke` (c) | the brain on a hand-built world, pro skill: the puck jumps from the carrier's left to its right at frame 20 | the defender's blade still points left at frame 26, follows at frame 28 |
| `SpinShieldsPuck` | the carrier spins with a defender beside it, its blade on the puck, a poke on every window; the control does not spin | spin: 16 pokes pressed, none took in the 24 frames, the puck on the far side of the body 19 of 24 frames, a quarter of the bar on the press, speed held at 7.70 of 11 and released after; no spin: the first poke takes it (frame 1) |
| `SpinCooldown` | a press during the spin, on the cooldown's last frame, the first frame after; an empty bar; a sliver of stamina | 1 spin, still 1, still 1, 2; refused on the empty bar; allowed with 1800 ticks and spent to the resting refill |
| `SpinCheckedFalls` | a bot checks the carrier head-on at 9 m/s at frame 13 of the spin; the control does not spin | 135 knockdown frames mid-spin (90 without), the spin ends with the fall, the puck stripped both times |
| `StickHandlingKeys` | defaults, the profiles' 14 / 8 / 5, the keys read back from a cartridge text | PASS |

Beside them, all PASS on the same build: Determinism, Stick, Payoffs (Keys, OneTimer, Breakaway, Deke, EmptyNetScript), CartridgeRoundTrip, Seams, Feel,
FeelDepth, PassReception, PuckNeverTunnels, TacklingEvents, Input.Mapping, Net.InputCoverage, Net.PacketRoundTrip, Net.LockstepTwoPeers.

**Determinism baseline** (calibrations: the carrier rule, the bots' delayed read): hockey.regular seed 1 frame 600 **`b705e2def86ae838` -> `b46e3f4a6148e86f`**,
hockey.raw **`e09eebd2cebd1337` unchanged** (`a51-tests7.log`, `a51-tests8.log`, identical). With the brief's `reach_m` 1.1 the pair was `7159264d74c81ef4` /
`32e82ce94877c4cc` (`a51-tests1..5.log`); Seams' informational hash moved `44fe347c1d25b4f2` -> `5dea66e03fd8be4e` (the carrier rule).

## 6. The playtester and the curve (`tools/playtest/run.mjs --rinks regular --seeds 1-20`, `tools/playtest/out/a51-after.md`, 180 matches in 29 s)

The regular and good players (`bStickHandling`) carry the puck on the far side of the body from a defender within 2.5 m in front while they are outside
shooting range or their lane is blocked (the stick flips sides as the defender does: each flip is a sweep the sim shields; while lining up a shot the stick
stays on the puck), and spin when a defender within 2.5 m closes from behind above 2 m/s while they skate above 2 m/s; the novice knows neither. New acts
`sweep`, `spin` in the stats (pro / regular, 20 matches: 2894 sweep frames, 181 spins; the novice 0 / 0).

| cell (regular rink) | before (PAYOFFS.md section 9, main after A39, 20 seeds) | after, final build (`a51-after2.md`) | moved | with `reach_m` 1.1 and the first playtester (`a51-after.md`) |
|---|---|---|---|---|
| rookie novice / regular / good | 80 / 100 / 100 | **90** / 100 / 100 | +10 / 0 / 0 | 90 / 100 / 100 |
| pro novice / regular / good | 15 / 50 / 80 | **30** / **75** / 85 | **+15 / +25** / +5 | 20 / 55 / 75 |
| all_star novice / regular / good | 0 / 15 / 50 | 10 / 25 / 50 | +10 / +10 / 0 | 15 / 20 / 70 |

**The regular player at pro moved 50 -> 75** (target about 50; the novice at pro 15 -> 30). Not re-tuned (A3 owns the curve). The cause is the mechanic
itself: the pro bots read the carried puck 8 frames late and the regular player's sweeps keep it (its passes received rose too: 1.65 a match against 1.55
on main); the good player is unchanged at all_star because it already beat the defence. A3's handle: `game.stick.sweep.bot_react_frames` 3..5 at pro (or
`opp.poke_frames` 4) brings the pro defence back; the ruling's 8 is the feel number, the Director decides.

**`Jam.Bot.PassesToHuman`** (A46's gate: the regular player receives at least 1.0 passes a match at pro and at rookie): 1.55 / 1.45 on main. With the
brief's `reach_m` 1.1 and the first playtester (sweeping whenever a defender was in front): 0.80 / 0.70, FAIL. Bisected with `tools/playtest/run.mjs --set`:
`stick.reach_m=1.2` alone restored pro (1.6) but not rookie (0.9); `stick.sweep.shield_frames=0` changed nothing (0.8 / 0.8), so the sim's shield is not
the cause; rookie's half was the playtester holding the stick to the side while in shooting range (passes made 0.30 -> 0.05: it carried instead of
shooting or passing). Final: 1.65 / 1.00, PASS (`a51-tests7.log`). `Jam.Bot.BoostPickupsBundle` failed on the 1.1 build on a bot-match trajectory (a take
without an effect start) and passes on the final one (9 spawned, 6 taken, 6 effects).

## 7. The proofs in the packaged Development build (SendInput, `tools/qa/ctrl/a51-prove.ps1`, `a51-analyze.mjs`)

The game (`Game/Packaged/A51-dev/Windows/ThirdJam/Binaries/Win64/ThirdJam.exe`, BuildCookRun Development 122.7 s, the final code) runs `jam.ctrl.Drill 3`
(sweep) or `4` (spin): the skating is scripted (carry the puck at the nearest defender at 60 % speed, hold the line 1.6 m out, chase a loose puck), the
cursor and the keys stay the real input path (`ReadHumanInput`). The driver (A49's user32 SendInput bridge) enters the match from the film-slate menu with
Enter, then crosses the cursor over the skater every 0.6 s in three steps (sweep) or presses E every 2.5 s (spin). `StickDrillObserve` opens an attempt on
every human sweep / spin the sim reports and logs `JAMDRILL stick_result <kind> n= kept= poked_clean= knocked= knock_frames=` 30 frames later.

Logs: `tools/qa/out/ctrl/a51-sweep/game.log`, `a51-spin/game.log` (+ `events.log`, what was sent and when). What they show, as they are:

| run | the real input path | the sim's read-outs | the drill's outcome |
|---|---|---|---|
| sweep (100 s) | `JAMCTRL ... src=mouse` on 598 frames (415 `none`: the cursor parked on the skater): **the cursor aims the blade in the package** | **7 `JAMCTRL sweep` lines**: the mouse crossings the sim counted as sweeps on the carrier (side -1 / +1 alternating, `shield_frames=11` after the frame) | 4 attempts logged, **3 kept the puck 30 frames later**, 0 clean pokes against them |
| spin (100 s) | `src=mouse` 4202 frames; **19 E presses arrived as `btn=spin (E / B)`**, 0 `pass` edges (F untouched: E no longer passes) | **19 `JAMCTRL spin` lines**: every press accepted (frames=23 after the press frame, stamina 0.75 = a quarter gone, dir away from the defender) | 30 attempts in three summaries (kept 1 / 9 / 3); most E presses came while the drill's human did **not** carry (`carrying=0`: the timer presses E every 2.5 s whatever the possession), so "kept" is meaningful only for the carried ones: the one spin with the puck (frame 1023, defender 6.9 m) kept it; no check landed mid-spin in the run (0 knockdowns), so the 135-frame fall is proven in `Jam.Core.SpinCheckedFalls` only |

Short of the brief's bar (ten sweeps past the pro defender, ten carried spins, one spin checked mid-turn) for two reasons, both in the harness, not the
mechanic: (1) **the foreground**: the first two runs lost the game window's foreground a second into the match (the `-log` console A49's driver inherits
takes it on the map load) and, as A49's driver did, sent anyway: nothing reached the game. The driver now takes the foreground back whenever it loses it
(it was lost and retaken every 2-8 s: the sweep run's game had not closed on the driver's WM_CLOSE and fought the spin run's window; the driver must wait for its own game's exit, as A49's does with its kid-process stop; the sends between the losses are what the table counts) and
aborts when it cannot. (2) **the drill's timing**: E is pressed on a timer, not when the human carries with a defender closing; the sweep run's human had the
puck for a fraction of the 100 s (the pro defender, read 8 frames late, still has the body). The next run should press E only on `carrying=1` and a
defender within 2.5 m (the drill can expose that as a log line the driver reads, or the drill can hold Move until the defender closes), and the cursor
crossing should be triggered by the defender's blade coming within 0.7 m of the puck (as the test's carrier does) rather than every 0.6 s.

Not done in the box: the 10-second still capture of the puck on the blade through a sweep (stills 4 frames apart, eyes `bug-hunt`): the packaged game has
no screenshot hook in the harness yet (A49's did not need one); a `HighResShot` console press from the driver is the next step. The pause card's text and
the stamina bar during a spin were not photographed either.

## 8. What I would do next for variety

- **Toe drag** (12 frames): a held E + a backward cursor pull while carrying: the puck is drawn 0.8 m back along the body line over 6 frames and released to
  the far side over 6 more; shielded like a sweep for 12. Beats a defender lunging at the puck's current spot (the bot's delayed read makes it a miss).
- **Between-the-legs** (18 frames): a sweep whose cursor path crosses behind the skater (|raw aim| > 150 degrees while moving): the puck passes under the
  body to the other side; the body shields it like the spin for 10 frames, the speed x0.85. Needs B's figure to have a legs-apart pose (one call,
  `PlayStickHit`-style one-shot) or it reads as a teleport.
- **Saucer pass** (the flight 20-40 frames): F held 10+ frames: the pass lifts (A50's `goal_height_m` work gives the sim a puck height), passes over a
  defender's stick inside 6 m of the lane, lands flat for a receiver beyond; a poke never reaches a lifted puck. The lift and the landing need the puck visual
  to leave the ice (the puck actor already takes a height on shots?).
- **Bot stick handling**: the pro and all_star carriers using the sweep against the human's poke (the brain's `StickPlay` for the carrier: flip the blade to
  the far side when the human's blade is within 0.7 m of the puck) so the human's forecheck has to be timed too; rookie never.

## 8. The pro bots stopped shooting: the bot's own blade held the puck out of shot reach (lane A66, hotfix, 2026-11-13)

`Jam.Bot.SeasonTakes` went red with A51 merged: take-3 and take-4 seed 1 (pro bots) played 4770 / 5033 frames with **0 AI shots**. Measured with a
temporary counter per bot per 600 frames (removed): the bot that last touched the puck rode it **1.5-1.8 m** from its body for most of a window (bot 3,
frames 1200-1800: 549 of 600 frames), while the adapter's carrier (within `shot.reach_m` 1.5) was -1 and `bCanShoot` false. Section 1's carrier rule
carries the puck out to `0.4 + reach_m + 0.05` = 1.65 m, and the brain's `StickPlay` aims the blade at the puck with `|Stick| = (D - 0.4) / reach_m`, i.e.
at the puck's own distance: a puck that drifted to the full reach (1.6 m) was held there by the bot's own blade, carried by the sim but past
`TryShoot`'s and `StepCharge`'s reach (1.5). Chasing it never closed (it rides with the body). The few aimed decisions went to the pro charge hold, whose
`ChargeFrames` grow only in reach, so the release fired nothing; pro never taps, so 0 shots. Not the delayed read (`PuckSeen` reads only an opposing
carrier) and not the penalties rule (takes 1 and 2 have it at rookie and pass).

**The fix** (logic, `AI/JamAICore.cpp` `StickPlay`): on its own puck (`PuckLastTouch == Id`) inside its shooting range (`dgoal < shot.reach_m +
shoot_extra_m`), a bot caps the blade's reach fraction at `(shot.reach_m - 0.1 - 0.4) / reach_m` (0.83 at the defaults: the blade, and so the puck,
at 1.4 m). Outside the range the long carry stays as A51 made it. A first version capped every own puck: SeasonTakes went green but
`Jam.Bot.PassesToHuman` rookie fell 2.25 -> 0.65 (bar 1.0), so the cap is limited to where a shot is possible. No key changed; the sim and the human's
stick are untouched.

| test | before | after |
|---|---|---|
| SeasonTakes take-3 seed 1 | 3:1 in 4770 frames, **AI shots 0**, FAIL | 1:3 in 7349 frames, **AI shots 8**, PASS |
| SeasonTakes take-4 seed 1 | 3:1 in 5033 frames, **AI shots 0**, FAIL | 3:2 in 13582 frames, **AI shots 10**, PASS |
| PassesToHuman pro / rookie | 1.70 / 2.25 | 1.35 / 2.15 PASS |
| ProCharge (hockey.regular pro seed 1) | 27 of 37 charged | 30 of 37 charged, PASS |

`Jam.Core+Jam.Bot+Jam.Playtest+Jam.Net+Jam.Tackling`: 46 passed, 0 failed. `Jam.Core.Determinism` (scripted, no brain) unchanged:
hockey.regular `ccb658f82e4ca910`, hockey.raw `e09eebd2cebd1337`. Bot-match hashes move (the bots' blades), as any brain change moves them.
