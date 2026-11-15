# Hardening 5: today's faults before the freeze (lane A198, track S3, evolution 5, device A, Sat 14 Nov 2026 14:33 -)

Tree: main 97cf7add (the held cross-check and protocol 9, wind-ups 4 / 4 ON, the breakaway bonus 0.3, the bindings file, the camera's bound, the new
front end, boosts' beacons, the GOAL banner). Every run headless or off screen, `-Multiprocess`, four processes of mine at most, loopback only.
Status with times and process ids: `status/A198.md`. Faults are reported there as `FAULT <n>` lines the minute they are reduced.

## What is new in the family

| file | what |
|---|---|
| `Game/Source/JamGame/Private/HardTests.cpp` | `Jam.Hard.InputFuzzSmoke`, `Jam.Hard.BindingsFile` (gate); `Bar.Hard.InputFuzz`, `Bar.Hard.BindingsReach`, `Bar.Hard.RebindRepro`, `Bar.Hard.BindingsWrite` / `BindingsRead` (long runs); the QA console command `jam.hard.menufocus` |
| `Game/Source/JamNet/Private/Tests/JamNetSoakTests.cpp` | the lockstep soak now carries the held cross-check: `ToInput` and the bots' copy keep `bCrossHold`, the playtester braces as `RunPlaytestMatch` sets it (`BraceMode`), `verbs_applied.cross_hold_frames` counts it |
| `tools/soak/lockstep/twoproc5.ps1` | `tools/net/twoproc.ps1` plus `-Mode guestleaves`, `-Mode latejoin` (a third process) and `-PauseAt/-PauseFor/-GuestRcPort` (the guest opens its pause card mid-match) |
| `tools/soak/lockstep/a198-twoproc.sh` | the two-process rows of item 2, one after the other |
| `tools/fuzz/menu-fuzz.mjs` | the front-end fuzz (item 5) |
| `tools/fuzz/bindings-file.sh` | the bindings file variants (item 4) |

## Input fuzz (`Bar.Hard.InputFuzz`)
One skater of each team is a fuzzing hand: random presses and releases (held about a fifth of a second, taps inside one step, bursts of mashing) of
every key and pad button of the bindings, after 0 to 12 random rebindings of random verbs (reserved keys included: Rebind must refuse them), resolved
exactly as the game does (`FJamButtonResolver` for the two-role buttons with the sim's possession, `bCrossHold` as `JamControls::SetCrossHold` gates it,
the plain verbs from the bindings). The brain plays the other skaters. 48 cases: hockey.regular, the large arena, boost, chaos and the twelve modes at
rookie, pro and all_star. Every match is played twice. Laws: nothing not finite, no puck more than 1.5 m and no skater more than 0.75 m beyond the
boards, every pause ends on its frame (a goal's `goal_pause_frames`, a foul's `UJamSim::FoulPauseFrames` 60; a wind-up now runs out inside the pause),
the match is over by its hard stop, the two plays' `LockstepStateHash()` equal, every rebinding keeps the table valid and round-trips the saved form.

## 1. The headless soak on main 97cf7add (item 1)
`node tools/soak/drift.mjs --uproject <tree> --baseline tools/soak/out/a198/baseA.json --update-baseline --max 3` (run A), then the same without
`--update-baseline` (run B, 2 workers) against A's digests. 60 cells x 100 seeds: the twelve modes at their own level (Marathon with sprint, cap
36,000), the eight season takes x novice / regular / good, the regular cut at rookie / pro / all_star, boost / chaos / raw at pro, the large and wide
arenas at pro, each x three classes. Every match is a full match (to its end or its hard stop): far more than 30 simulated minutes a cell.

| run | matches | faults (crash, ensure, error line, JAMFX missing) | wall |
|---|---|---|---|
| A (3 workers) | 6,000 | 0 | 745 s |
| B (2 workers) | 6,000 | 0 | 1,457 s |
| A against B | 60 of 60 cells bit-identical (every seed's hash) | | |

Determinism holds with the held cross-check's bit and the wind-up state (`CrossTarget`, `bCrossTargetStickOnly`): the playtester braces wherever the
cartridge has `contact.cross_hold` (all of them), and the wind-ups (4 / 4) run in every match. Memory (`tools/soak/memwatch.mjs`, every 15 s, the
processes of this tree): the soak's own processes live 20 to 40 s (one chunk of 50 matches) and are not judged; the long-lived ones (lockstep soak,
input fuzz, 97 to 239 s) moved +0.1 / +0.2 / +0.5 / +5.7 % from their first to their last quarter.

## 2. Lockstep on protocol 9 (item 2)
### In one process, two peers over the soak's link (`Bar.Net.SoakClean`, `Bar.Net.SoakLossy`, `tools/soak/lockstep/run.ps1`)
The soak harness did not carry the held cross-check: its `ToInput` and its copy of the bots' inputs dropped `bCrossHold`, and its playtester never
braced (`BraceMode` 0). Fixed in the harness (my family). The same three gaps are in the game's online path, outside my family (finding F1 below).

| profile | matches x frames | desync | lost | edge presses lost | cross-hold frames on the wire |
|---|---|---|---|---|---|
| clean | 12 x 36,000 (10 min) | 0 | 0 | 0 of 7,194 | 3,680 |
| loss30 | 8 x 36,000 | 0 | 0 | 0 of 6,079 | in the run's lines |
| jitter0-6 | 8 x 36,000 | 0 | 0 | 0 of 5,060 | in the run's lines |

Every compared frame of every match bit-identical on both peers.

## 3. Input fuzz (item 3)
`Bar.Hard.InputFuzz` (method above), 200 matches, each played twice, full length (`JAM_HARD_FRAMES` 36,000):

| law | result |
|---|---|
| the match ends by its hard stop | 200 of 200 over |
| the two plays' `LockstepStateHash()` | equal 200 of 200 |
| nothing not finite; no puck or skater outside the rink | 0 |
| every pause ends on its frame | 707 pauses: each 120 frames (a goal) or 60 (a foul, `UJamSim::FoulPauseFrames`): 0 longer, 0 other |
| the held cross-check pressed | 3.5 M frames of `bCrossHold` from the two hands |

The first run's oracle counted only `goal_pause_frames`; its 110 lines were the 60-frame foul pauses of boost, chaos and Glass House (fouls on).
The oracle now knows both. The gate's `Jam.Hard.InputFuzzSmoke` plays 4 of the matches for 3,000 frames.

## 4. The bindings file (item 4)
**A rebinding survives a restart.** `tools/soak/out/a198/bindrestart.sh`: process 1 (`Bar.Hard.BindingsWrite`, `-MultiprocessSaveConfig` so a
`-Multiprocess` process writes) rebinds Pass to Z and Spin to G and saves through `FJamSettings::Save` (to_disk=1, `[Jam.Controls]` in
`Game/Saved/Config/WindowsEditor/GameUserSettings.ini`); process 2 (`Bar.Hard.BindingsRead`) reads Pass on Z, Spin on G, the table valid. PASS.

**The file as a disk can leave it** (`tools/fuzz/bindings-file.sh`): each variant is the file this tree's game wrote, with `[Jam.Controls]` damaged;
a new process reads it through `FJamSettings::Load`. No variant crashed or failed to start; every table read back is valid (no clash).

| variant | read back by a new process (`FJamSettings::Load`) | verdict |
|---|---|---|
| the game's own file (control) | 15 verbs, Pass on Z, Spin on G | PASS |
| missing / empty / binary garbage | defaults | PASS |
| cut in half (ends mid-line) | the 4 whole lines, the rest default | PASS |
| two verbs on one key (`Poke=W\|`) | defaults whole (the clash falls back), no word | PASS (defaults) |
| a duplicate and a broken `[Jam.Controls` header | the later section's line wins | PASS |
| unknown keys (`Shoot=NoSuchKey\|Gamepad_Nope`) | **Shoot with no key and no pad** | FAULT 1 |
| a key that no longer exists (`Shoot=Mouse_Left\|RightTrigger`) | **Shoot with no key and no pad** | FAULT 1 |
| every verb `\|` | **all 15 verbs unbound**, the table "valid" | FAULT 1 |
| read-only file: read / save | read 15; the save logs `to_disk=1` and changes nothing | PASS (finding F3) |
| no `[/Script/Engine.GameUserSettings]` Version section, writable | `[Jam.Controls]` not read (defaults); read-only: read | finding F4 (engine) |

### Two processes on this machine (`tools/soak/lockstep/twoproc5.ps1`, loopback 127.0.0.1 only, UDP 4796/4797, the good playtester on both seats)
| row | result |
|---|---|
| Marathon to its end (8:00 clock), every verb | PASS: frame 25,277 (7 min of play), final hash equal, 43 checkpoints equal, 0 desync, 0 lost; cross-check presses 536 / 547 and checks 32 / 27, all applied |
| a guest on protocol 8 | PASS: refused in 0.026 s, its screen "the host runs a different build (protocol 9, you have 8): update both" |
| the guest's pause card, 10 s and 45 s | **FAULT 4**: the guest's world pauses, its lockstep stops, the host calls the connection lost 3.1 s later, both matches end |
| the guest leaves at frame 607 / 1803 | PASS (after FAULT 3 retries): the host shows "Bo left, 2.01 s after the last frame", no freeze |
| a third process joins the host 45 s into the match | PASS for the match (to its end, hash equal, 52 checkpoints, 0 desync); the late guest gets no answer at all (finding F5) |
| the start of a match | **FAULT 3**: 9 of 15 starts on this machine died at frame 0 as the slow-loading guest arrived (`tools/soak/lockstep/a198-twoproc2.sh` retries) |

The held bit itself cannot travel from a scripted seat online (finding F1): the two-process rows exercise the wind-ups (every cross-check and
check press runs through them) but not `bCrossHold`; the in-process soak above carries it.

## Faults (each reduced to its smallest repeat; none fixed here: the owners' families)
| # | fault | smallest repeat | where |
|---|---|---|---|
| 1 | a saved verb whose keys are all unknown, reserved or empty is left with no key and no pad, silently | `Jam.Hard.BindingsFile` (JAMHARD lines) / `Bar.Hard.BindingsReach`; through the file: `[Jam.Controls] Shoot=Mouse_Left\|RightTrigger` | `InputMapping.cpp:264-267` (`FromSaved`) |
| 2 | one rebinding strips a verb that never touched the key: `Rebind(CrossCheck, Down)` leaves Shoot with no mouse key and puts LMB on skate back | `Bar.Hard.RebindRepro` | `InputMapping.cpp:183-184`, `:190-205` (`Rebind`) |
| 3 | an online match dies at frame 0 when the guest's rink loads more than 3 s after the host's | `Bar.Net.LateFirstPacket` (waits of 200, 300, 600 ticks lose; 100, 170 play) | `JamLockstepDriver.cpp:46`, `:76` |
| 4 | the pause card ends an online match (the paused machine's lockstep stops; the other calls the connection lost after 3.1 s) | `twoproc5.ps1 ... -GuestRcPort 4798 -PauseAt 20 -PauseFor 10` | `DirectorsCallController.cpp:75-77`, `:214-219`; `LobbyGameMode.cpp:76-78` |
| 5 | the editor binary's first match (off screen) dies of a GPU page fault in 9 of 30 starts; 0 of 20 with `r.RDG.AsyncCompute=0`; packages 0 of 80 | `node tools/fuzz/gpu-starts.mjs --exe <UnrealEditor.exe> --uproject Game/ThirdJam.uproject --res 1920x1080 --cuts ",perf.p3_clay" --processes 10 --first-only` | the engine's async compute (Lumen radiance cache) in the first frames after the rink's assets compile; guard: `r.RDG.AsyncCompute=0` for off-screen QA |
| 6 | PLAY WITH A FRIEND opens the lobby with the focus on the viewport: a pad cannot use or leave it; the lobby widget later steals the menu's pad focus | `node tools/fuzz/menu-repro.mjs ... "jam.menuscreen start" "jam.lobby" "jam.menuscreen key down" "jam.menuscreen credits" "jam.menuscreen key down"` | `LobbyWidget.cpp:55-60` |
| 7 | SETTINGS: Up into the Designer key box, Enter: the focus is cleared and Esc / B / pad do nothing (stuck until a mouse click) | `node tools/fuzz/menu-repro.mjs ... "jam.menuscreen settings" "jam.menuscreen key up" "jam.menuscreen key enter" "jam.menuscreen key esc"` | `SettingsWidget.cpp:227-237`, `:382-386` |

## Findings (not faults of today's features, or not reduced)
- **F1** the online path drops the held cross-check from every scripted and brain seat: `LobbyGameMode.cpp:62-71` (`FromAIInput`), `:311-326`
  (`GatherBotInputs`), and the online playtester never braces (`:104-110`, `:673-678`; `RunPlaytestMatch` sets `BraceMode`, `JamPlaytester.cpp:803`).
  No effect for a human today (`ApplyButtons` writes the bit); bots online would not brace if a profile let them.
- **F2** `JamAI::RunMatch` (`JamBotMatch.h:106`) is not exported from JamCore: a test in another module cannot call it (link error LNK2019).
- **F3** on a read-only settings file the save logs `to_disk=1` and keeps nothing (`SettingsWidget.cpp:105-117`).
- **F4** (engine, editor binary) a writable settings file with no `[/Script/Engine.GameUserSettings]` Version loses `[Jam.Controls]` at load.
- **F6** QUIT on HOME ends the game on one press (pad A / Enter), no confirmation (`MenuWidget.cpp`, QuitButton).
- **F5** a guest who joins a host already in a match gets no answer (the host's driver drops lobby packets, `JamLockstepDriver.cpp:74`); after 60 s its
  screen says "no answer from the host in 60 s (same network? port forwarded?)" (`JamLobbySubsystem.cpp:294`): the words should say the host is playing.

## 5. The front end (item 5): `tools/fuzz/menu-fuzz.mjs`
One editor-binary game process on the Menu map, off screen (`-RenderOffScreen`, no window grab, no cursor), driven over its remote console
(127.0.0.1:4798) by the menu's QA command: random pad and keyboard keys (`jam.menuscreen key ...`), Slate-routed clicks on every named place, Back,
and jumps to each screen in turn (landing, HOME, MODES, SEASON, ALL RINKS, SETTINGS, CONTROLS, CREDITS, MAKE YOUR OWN: on MAKE YOUR OWN only
Back is pressed, never a sentence, no Designer call). After every step `jam.hard.menufocus` names the map, the game mode, the screen, the widget
that holds the keyboard focus, whether it is inside the menu, the card it is on, and (build 5) its owner widgets and whether it is shown.
A press that starts a match is judged (exactly one `JAMMATCH` and at most one `JAMCART loaded` more), the match is ended (`jam.ctrl.EndMatch 1`)
and the menu brought back (`jam.pausemenu quit`). Then the twelve mode cards from MODES by pad (Down per row, Right per column, Enter).

## 6. Frame time (item 6): `tools/fuzz/frame-run.sh` (`frame-bot.ps1`, cuts in `tools/fuzz/frame-cuts/`)
Off screen at 1920 x 1080, five a side (all bots), the boost cut's pickups standing (beacons), A175's goals on set frames (the GOAL banner), 100 s
of play, one game at a time, two rounds. Before: a copy of the 13:21 Accept_H32 package; after: this tree's Development package. Busy machine
(my off-screen menu fuzz rendered beside every run, both arms alike).

| tier | arm | frame mean (r1 / r2) | p99 | worst frame | GPU mean | at the goals: worst frame median / max |
|---|---|---|---|---|---|---|
| clay | before | 17.44 / 17.26 | 35.0 / 31.7 | 247 / 96 | 7.1 / 5.4 | 18.1 / 21.1 (4 goals) |
| clay | after | 17.20 / 17.17 | 28.1 / 28.8 | 159 / 173 | 6.7 / 5.4 | 17.0 / 18.5 (3 goals) |
| neon | before | 17.31 / 17.27 | 27.9 / 29.9 | 157 / 132 | 6.2 / 5.1 | 17.1 / 18.8 (4 goals) |
| neon | after | 17.31 / 16.82 | 27.8 / 19.3 | 201 / 23 | 6.1 / 4.3 | 17.6 / 19.1 (3 goals) |

No frame over 33 ms in any of the 14 goal windows; this morning's clay goal frames (PROFILING-8) were 23.6 / 41.9. The single worst frames are
lone hitches of a shared machine in both arms. Verdict: the beacons and the banner cost nothing measurable.

## 7. The GPU crash at a match start (Director 16:07): `tools/fuzz/gpu-starts.mjs`, `tools/fuzz/gpu-ship.mjs`
Each process starts a match off screen through PLAY NOW (or opens the rink directly: Shipping has no console), plays 20 s, and (gpu-starts) starts a
second match in the same process after going back to the menu. The GPU processes alive on the machine at every start are listed (read only).

| build | starts (first) | GPU deaths |
|---|---|---|
| editor binary, 1920x1080, PLAY NOW / clay | 30 (30) | **9** (all first starts; 0 of 15 second starts; 0 of 119 later starts in one menu-fuzz process) |
| editor binary, `r.SkyLight.RealTimeReflectionCapture=0` | 10 (10) | 2 |
| editor binary, `r.RDG.AsyncCompute=0` | 20 (20) | **0** |
| Development package, 1280x720, PLAY NOW / clay / night / neon | 40 (20) | 0 |
| Development package, 1920x1080, PLAY NOW / clay | 20 (10) | 0 |
| Shipping H32 copy, 1920x1080, direct rink starts (clay, night, neon, clay 5 v 5) | 20 | 0 |

Every death: `DXGI_ERROR_DEVICE_HUNG`, "Page fault info: Read, Failed to translate the virtual address" in compute shader 16151227421261170552,
the compute queue in Lumen `LumenScreenProbeGather > UpdateWorldRadianceCaches`, the render thread usually in the sky light's
`ComputeSingleAverageBrightnessFromCubemap`, p3_clay, the first frames (0 to 314) of the process's first match, just after the rink's assets finished
compiling (`JAMPRELOAD done`, "Waiting on static mesh ... being ready": an editor-only path). Deaths came with 2 to 4 GPU processes alive and so did
survivals: not the machine's load. The same signature is in A195's, A187's and lane f2b's logs. FAULT 5 in `status/A198.md`.

### Results (runs 1 to 4, editor binary off screen)
| run | steps | matches started from random presses | builds per press | Backs judged (stayed) | focus outside the menu | ended by |
|---|---|---|---|---|---|---|
| smoke (seed 7) | 200 | 10 | 1 each | 18 (0) | 1 (pad A on FRIEND: FAULT 6) | the end |
| 3 (seed 2) | 1,756 | 120 | 1 each | 112 (0) | 5 (FRIEND, then the lobby's BACK: FAULT 6) | pad A on QUIT (F6) |
| 4 (seed 4) | 3,300 | 234 | 1 each | 191 (3: FAULT 7; 2 on CONTROLS, not reduced) | FAULT 6, FAULT 7 | the end; then all 12 mode cards |

Never two builds from one PLAY (the morning's `StartMatchAtLevel` fault stays fixed); Back always left its screen. Run 2 died of FAULT 5 at its
second match. The twelve mode cards: all twelve started their own mode in run 4 (run 1's 10 of 12 was the driver pressing keys 120 ms apart). Off-screen QA runs now pass `r.RDG.AsyncCompute=0` (FAULT 5).
