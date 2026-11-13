# Development against Shipping: do the two packages play the same game? (lane A57, device A, 2026-11-13 10:02-12:35 CET)

R-0032 hid for an evening because PIE and the Development package behaved and the Shipping package did not. This page answers, with the two packages of one commit (main
`59675fe`; `Game/Packaged/Hokej_Hokej_N1015-dev`, 948 MB staged, exe 337 MB with pdb; `Game/Packaged/Hokej_Hokej_N1015`, 363 MB staged, exe 173 MB), whether they run the
same match, and lists what a Shipping exe cannot do that every proof lane has been assuming. Status blocks: `status/A57.md`. Scripts: `tools/qa/parity/` (section 7).

## Verdict

**No difference found, and a frame-level proof is impossible.** A Shipping exe cannot be asked for a state hash, a score or an event: it writes no log (section 3), takes no
console, no `-ExecCmds`, no `-dpcvars`, no `.ini` cvar, no remote console, and cannot play itself. The only thing it shows is the screen. So the comparison is:

- **Development**: the `JAMSIM ... hash=` line every 600 frames (6 checkpoints up to frame 3600) and the event stream, from the log.
- **Shipping**: the HUD, photographed every 2 s (`PrintWindow` of the client area) and read by OCR: the two scores, the 3600-frame match clock, the REMATCH card.

On the five seeds (`hockey.regular`, pro, one human skater left idle, 3 periods, match clock 3600 frames; section 1):

- **Seeds 1, 2, 3, 4: identical on everything both builds show.** Same final score (0:0, 0:1, 0:3, 1:2), and every clock second that both builds photographed shows the
  same score (21/21, 26/26 and 14/14, 25/25, 21/21), goals first visible within about one still (2 s) of the time the Development log gives, the REMATCH card (or, tied at 0:0, play continuing at 0:00) in both. The Development
  hash trace of these seeds is bit-identical across launches (4 of 4 launches of seeds 1 and 2, 2 of 2 of seeds 3 and 4, all 6 checkpoints), so the reference is
  stable and a Shipping divergence would have shown. Hash parity itself cannot be tested: there is no Shipping hash to compare.
- **Seed 5: not comparable, for a reason that is a finding of its own.** The four Development launches give three different traces (final 0:1, 0:2, 0:1, 0:1; hashes
  differ from frame 1200) and the two Shipping launches give 0:2 and 0:1: the same two outcomes, not a Shipping-only one. The "idle" human is not idle (next point).
- **The idle human follows the mouse.** `AHokejGameMode::CursorAimRink` deprojects the cursor onto the ice and aims the blade at it, every sim frame. Anything that moves
  the cursor during a run (a person, another lane's `SendInput`) is a live input: the first two sweeps (cursor left alone) produced a Shipping or a Development launch that
  disagreed with the others in one seed each (seed 3 in the first sweep, seed 4 in the second: 1:1 against 0:3, 0:1 against 1:2) and both disagreements disappeared when
  the cursor was held on the skater (`park.ps1`) and the Development launches were screened for a non-zero stick (`jam.ctrl.Log 1`). Not a build difference: a hockey game whose
  blade follows the mouse is not reproducible with a mouse that moves. With `jam.ScriptHuman 1` (stick always zero) two Development launches of seed 1 gave 6 of 6 identical
  hashes; nine more Development launches of seed 1 with the cursor held (the control runs `ctl-a` to `ctl-i`, 50 s each) agree on the first three checkpoints.
- **Net: the Development and Shipping packages of 59675fe play the same matches as far as a Shipping exe can be observed (score, goals, match clock, end card).
  What nobody can say from outside is "bit-identical state"; if a lane needs that for Shipping it needs `bUseLoggingInShipping` or an on-screen hash, which the installed
  (launcher) engine refuses for this project (docs/experiments/SIZE.md, row 10).**

## 0. Premises of the brief that did not hold

| Brief says | Measured | Used instead |
|---|---|---|
| "whatever drives a match without a human in each (the playtester, `jam.ScriptHuman`, or a bot-only mode: say which survives Shipping)" | **None survives.** The playtester (`-player=novice\|regular\|good`) is a flag of the `JamTournament` commandlet; `ThirdJam.exe -run=JamTournament -player=regular ...` on the **Development** package answers `LogInit: Error: Cannot run editor commandlet Class /Script/JamCore.JamTournamentCommandlet with game executable.` So it runs in no packaged game. `jam.ScriptHuman` is compiled into both exes (`jam.ScriptHuman` is in the strings of both) but unreachable in Shipping: `-dpcvars=jam.ScriptHuman=1`, `-ExecCmds="jam.ScriptHuman 1"` and `[SystemSettings] jam.ScriptHuman=1` in `Saved/Config/Windows/Engine.ini` all left the human standing at centre ice (stills of `tools/qa/out/parity/ship-s1-ini`). There is no bot-only mode: `AHokejGameMode` always seats a human; `bScriptedHuman` is set by the camera proof scene only. | the human left idle in both builds |
| "compare the state hash per 600 frames if a hash is logged" | The hash is logged (`JAMSIM frame=N hash=... score=a:b`) by `UE_LOG` in `AHokejGameMode::StepOnce`; `ThirdJam.Target.cs` has no `bUseLoggingInShipping`, so Shipping compiles every `UE_LOG` out: the strings `JAMSIM frame` and `JAMEVENT` are in the Development exe and **not in the Shipping exe**. | score, clock and the end card from the HUD |
| "run `-ExecCmds` / the remote console like A58" | `TcpRemoteConsole` is not in the Shipping exe (no `RemoteConsole` string); A58 used it on Development only. | PostMessage keys (A54's `shoot.ps1`), screen stills |
| "5 seeds, 3600 frames, hockey.regular" | The shipped `hockey.regular` is `first_to 3`, three periods of 1200 frames plus a 90-frame faceoff each; the match length depends on the goals. | five cut copies (section 1): `seed` = 1..5 and `rules {mode: timer, timer_frames: 3600}`, nothing else changed. The clock reads 1:00 to 0:00 over the 3600 frames; the REMATCH card is up 66-76 s after ENTER in both builds (seed 1 ends 0:0: the clock sits at 0:00 and play goes on, no card, in both builds). |
| (implied) "an idle human is a constant" | The blade follows the cursor (verdict). | cursor held on the skater, Development launches screened (section 2) |
| Hash and score differ "between configurations" | Nothing differs between **configurations**; launches of the same configuration differ when the mouse moves (verdict). | |

## 1. Method

- **Cuts** (`tools/qa/parity/make-cuts.py`): `hockey.regular.cartridge.json` with `seed` 1..5, `id parity-sN`, `rules {"mode":"timer","timer_frames":3600}` (`score_to_win` removed).
  Difficulty pro, look tier, everything else as shipped. Passed as `-cut=<abs path>` (a command-line `-cut=` wins over the season take the menu starts).
- **Run** (`tools/qa/parity/run.mjs`, on A54's `tools/qa/eyesgrid/shoot.ps1`): the exe is started windowed 1280x720 at (40,40); ENTER on the film-slate PLAY (SendKeys when
  the game owns the foreground, else `PostMessage`: a foreign "Windows Security" dialog held the foreground at times); a still every 2 s; the exe is stopped by PID at the end (only the
  process the script started). Frame 0 of the match is about 1 s after ENTER. One game at a time on my side; other lanes' Development games and builds ran on the same machine all morning.
- **Development side**: `-log` (the log is copied from the package's `Saved/Logs/ThirdJam.log` when `-abslog` is ignored, which happened) and `-dpcvars=jam.ctrl.Log=1`
  (a `JAMCTRL frame=N stick=(x,y) src=...` line per sim frame; it changes nothing in the sim: hash 44c92c55 at frame 600 of seed 1 with and without it). `tools/qa/parity/devlog.py` reads the
  hashes, events and goal frames.
- **Shipping side**: `tools/qa/parity/shots.py` / `ocr.py` read the score boxes, the clock and the REMATCH card from the stills (Tesseract 5, crops of the 1280x720 HUD; the
  clock read 44 of 44 stills of the check run, the scores always). Because the clock is the match timer, a still is placed in the match by its clock reading, not by wall time:
  the comparison counts the **clock seconds that both a Development and a Shipping launch photographed** and checks the score read at each.
- **The mouse** (`tools/qa/parity/park.ps1`, started with every run): puts the cursor on the human's skater (client pixel 544,570 of the 1280x720 window, within the 0.7 m dead zone
  of `CursorAimRink`) every 400 ms. A cursor within 0.7 m gives a zero stick. When another lane moves the mouse or a check pushes the skater away the stick is not zero:
  `compare2.py` counts the non-zero stick frames up to frame 3600 of every Development launch and prints them. Protocol (`runall2.mjs`): per seed up to 4 Development launches
  until two have a zero stick throughout, then up to 2 Shipping launches until one agrees with the Development final score; **every launch is reported**.
- Three sweeps were run: sweep 1 with the cursor wherever it was, sweep 2 with it parked above the window, sweep 3 as above. The tables of the first two are in
  `tools/qa/parity/results/sweep1.compare.md`, `sweep2.compare.md` (they are why the verdict says what it says about the mouse).

## 2. The five-seed table (sweep 3; every launch in `tools/qa/parity/results/sweep3.compare.md`)

| seed | Dev hash 600 / 1200 / 1800 / 2400 / 3000 / 3600 (first 8 hex) | Dev launches with the same 6 hashes | Dev final | Ship final per launch | clock seconds read in both with the same score | goals: Dev sim frame (log) | goals: Ship first still showing the new score (match clock) |
|---|---|---|---|---|---|---|---|
| 1 | 44c92c55 05a65ae0 3c8a4d0e 9bc864e6 16269ecf 4d740981 | 4 of 4 | 0:0 | 0:0 | 21/21 | none | none |
| 2 | eda25e96 a2d68307 4812e0ac 22747e53 9ece208a 06dfe33f | 4 of 4 | 0:1 | 0:1, 0:1 | 26/26, 14/14 | 1585 | 0:36; 0:36 |
| 3 | 1bc4e230 45e1a231 1e0f421b 249288da 4b10f73b cc9f0926 | 2 of 2 | 0:3 | 0:3 | 25/25 | 384, 1830, 2504 | 0:56, 0:35, 0:27 |
| 4 | 7da39dc6 ff17ccf6 ae4c50d6 09654133 776360fb bbdf9aae | 2 of 2 | 1:2 | 1:2 | 21/21 | 369, 1984, 2976 | 0:56, 0:32, 0:19 |
| 5 | 6f1b71a1 63726b02 9c315978 743f8731 6662efd0 b1f7e3f9 (launch r1; r2-r4 differ) | 1 of 4 | 0:1 | 0:2, 0:1 | 10/11, 19/19 | 1405 | 0:39, 0:16; 0:39 |

The clock runs only while play runs (the 90-frame faceoffs and the 120-frame goal pauses stop it), so a goal at sim frame F shows at a clock of about `60 - (F - pauses) / 60` seconds:
384 -> 0:55, 1830 -> 0:36, 2504 -> 0:30 (3 goals, 3 pauses of 210 frames, 3 faceoffs); the stills read 0:56, 0:35, 0:27: inside the 2 s cadence plus the goal card. Seed 5 launch r1 of
Shipping (0:2, 10 of 11 clock seconds equal) shows a second goal at 0:16 that its Development reference lacks; Development launch r2 of the same seed (1547 non-zero stick frames) has the same 0:2.

Development event counts of the reference launch (Shipping has no events; this is what "checks, saves, posts" are in the match the Shipping exe also plays):

| seed | launch | goals | shots | saves | posts | checks (`lift`) | pokes | breakaways | faceoffs | board hits |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | dev-s1-r2 | 0 | 5 | 0 | 0 | 13 | 4 | 0 | 3 | 29 |
| 2 | dev-s2-r1 | 1 | 7 | 1 | 0 | 8 | 3 | 2 | 6 | 19 |
| 3 | dev-s3-r1 | 3 | 7 | 1 | 1 | 13 | 3 | 6 | 12 | 15 |
| 4 | dev-s4-r1 | 3 | 9 | 2 | 0 | 16 | 5 | 5 | 12 | 25 |
| 5 | dev-s5-r1 | 1 | 5 | 0 | 0 | 6 | 2 | 2 | 6 | 28 |

The Development hash of seed 1 at frame 600 (`44c92c550e839b08`) was reproduced by 15 launches (three sweeps, parked or not); seeds 1 to 4 are therefore usable as golden traces of main `59675fe` for
the next lane that needs one (the sim of a later commit that changes them has changed the game; the hash lines are the proof).

## 3. What Shipping compiles out, and what it merely cannot be asked

Method: the strings of both exes (UTF-16 `TEXT()` literals; `tools/qa/parity/` has no script for it, it is a 10-line search) plus the `UE_BUILD_SHIPPING` / `WITH_DEV_AUTOMATION_TESTS` /
`WITH_EDITOR` guards of `Game/Source`. "In the exe" means the symbol is registered at startup; it says nothing about being reachable.

**Compiled out of the Shipping exe (absent from its strings):**

| what | why | consequence |
|---|---|---|
| every `UE_LOG` line: `JAMSIM` (hash), `JAMEVENT`, `JAMCTRL`, `JAMCART`, `JAMVISUAL`, `JAMDRILL`, `JAMSEASON`, `JAMFX`, `JAMVO`, ... | no `bUseLoggingInShipping` in `ThirdJam.Target.cs`; `-log`, `-abslog` have no effect; the package has no `Saved/Logs` | **a Shipping run has no hash, no score, no event; proofs from the log are Development only** |
| console command `jam.key` | `#if !UE_BUILD_SHIPPING` in `JamDesignerConsole.cpp:67` | none for play; the Designer's API key comes from the environment / config |
| the Claude CLI transport of the Designer unless `[...]bAllowCliTransport` is true in `DefaultGame.ini` | `bool bAllowed = !UE_BUILD_SHIPPING` in `JamDesignerSubsystem.cpp:369` | the shipped Designer talks to the API only |
| every automation test: `Jam.Playtest`, `Jam.Net.LockstepTwoPeers`, all `*Tests.cpp` (`Jam.*` names) | `WITH_DEV_AUTOMATION_TESTS` | the test suite runs in the editor / `-nullrhi` Development only |
| code under `WITH_EDITOR` in `JamPickupActor.cpp:145` and `HokejGameMode.cpp:286` | editor-only helpers | none at runtime |

**In the Shipping exe but unreachable** (registered at startup: the names are in the strings of both exes; Shipping has no console, no `-ExecCmds`, no `-dpcvars`, no cvar from `.ini`,
no `TcpRemoteConsole`, no `~`): `jam.livecut`, `jam.playcut`, `jam.say`, `jam.call`, `jam.bark`, `jam.say_line`, `jam.pausemenu`, `jam.dlss`, `jam.ui.faceoff`, `jam.ui.draw`, `jam.ui.effects`,
`jam.menuscreen`, `jam.Season.Start`, `jam.Season.Restart`, `jam.Season.Status`, `jam.Season.Reset`, `jam.signal`, `jam.lobby`, `jam.host`, `jam.join`, `jam.start`, `jam.leave`, and the cvars `jam.Cut`,
`jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.ctrl.Log`, `jam.Barks`, `jam.BarkVoice`, `jam.Commentary`, `jam.SignalScript`, `jam.VoiceSignals`, `jam.HitHoldFrames`, `jam.camera.Legacy`, `jam.camera.Log`,
`jam.rink.construction`, `jam.rink.beacon`. The commandlets (`JamTournament` with `-player=`, `-seeds=`, `-cartridge=`, `-maxframes=`, `-trace=`, `-aitune=`; `JamCutSweep`) are classes in both exes
and run in neither: `Cannot run editor commandlet ... with game executable`.

**Still works in Shipping**: the game's own `PC->ConsoleCommand` calls (`MenuWidget.cpp:619`): the menu's "Play with a friend" opens the lobby through `jam.lobby` in the Shipping exe
(still `tools/qa/out/friend-ship/screen.png`: "Opening the lobby (jam.lobby)..."). `-cut=<name|path>` on the command line (it is read with `FParse::Value`, not through the console). Keys and mouse (so the
mouse-driven `CursorAimRink` above), `Saved/Progress.json` (written at the end of a season take only), `Saved/Config` `.ini` files for settings that are not cvars.

What a proof lane can therefore do in Shipping: start a cut with `-cut=`, press keys by `PostMessage`, read the screen. What it cannot: read a hash, a score or an event, set `jam.ScriptHuman`, switch a tier at
runtime, trigger `jam.menuscreen` stills (A54 reached the screens by keys). Anything that needs those is a Development proof, to be restated for Shipping in the terms of the screen.

## 4. Reproduce

```
powershell -File tools\qa\nightly\parity-packs.ps1 -Tag <label>          # the two packages of the worktree's commit (Development, then Shipping)
node tools\qa\parity\runall2.mjs --ship <...\Hokej_Hokej_N1015\Windows> --dev <...\Hokej_Hokej_N1015-dev\Windows> --out <dir>
python tools\qa\parity\compare2.py <dir> > tables; python tools\qa\parity\table.py results\parity3.compare.json
```

Scripts: `make-cuts.py` (the five cuts, written to `tools/qa/parity/cuts/`), `run.mjs` (one run), `runall2.mjs` (protocol 3), `runall.mjs` (the first two sweeps), `park.ps1` (cursor hold),
`devlog.py`, `ocr.py`, `shots.py`, `compare.py` (sweeps 1 and 2), `compare2.py`, `table.py`. Results kept in `tools/qa/parity/results/`; the stills and logs (several GB) stay in
`tools/qa/out/parity*` and are not for commit.
