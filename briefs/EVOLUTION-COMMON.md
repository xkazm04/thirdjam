# Evolution tracks: the rules every track lane works under (Director, Fri 13 Nov 18:50; R-0047)

The operator (18:45): "We need to iterate on top of existing concepts and make evolution after evolution in tracks where possible. Next 24h will be key for hackathon to
perfect the game into its best shape possible. We should always have RAM 60-80% used, Fable agents thinking how to improve game mechanics and design, executing in blender.
Opus subagents to improve the codebase and quality of implementation, Sonnet subagents to run tests and harden the game." The difficulty ladder of R-0046 is accepted as set.

Read first, always: `docs/VISION.md` (the game's laws), `docs/PLAN-RECONCILED.md`, `docs/EVOLUTION.md` (the tracks and their queues), R-0041 to R-0047 at the end of
`contracts/RULINGS.md`, then your track's brief. Read them from the main checkout `C:\Users\kazda\kiro\thirdjam` if your worktree's copy is older.

## How a track works
- A track is a file family and a goal. A lane does **one evolution** (a change a player can feel or a defect removed, with its proof), reports, and proposes the **next
  three evolutions** for its track, ranked, each with the number that would prove it. The Director lands the evolution and starts the next lane on the track at once.
- An evolution is small enough to land in 60 to 90 minutes. If yours grows past that, cut it at the last green point and report the rest as the next evolution.
- **Nothing lands red.** Sim, brain or feel changes: the full `Jam` automation filter (the invocation in `tools/gate/premerge.ps1`), the hash before and after, and a neutral
  value that gives back the previous game where the change is a rule (a cartridge key or a profile number; prove it as lanes A92 and A93 did). A verb the player uses:
  the acceptance run (`tools/qa/accept/accept.ps1`, `docs/experiments/ACCEPT.md`) green on a Development package of your tree.
- The difficulty ladder (R-0046, `docs/experiments/CURVE-5.md`) is set. A lane that moves a win rate measures the three target cells (600 matches each with
  `tools/perf/results/a95/curve.mjs`) and reports them; more than 4 points off in any cell means the evolution ships with its neutral value until the curve track re-sets it.

## The machine (the operator's band: RAM 60 to 80 % of 63 GB, so 38 to 50 GB in use)
- Check memory before every batch (`Get-CimInstance Win32_OperatingSystem`). Under 38 GB in use: you may add headless workers (soak workers up to 16, a second headless
  measurement beside your build). Over 50 GB: finish what runs, start nothing new. Never push the machine past 56 GB.
- One lane owns the **cursor and game windows** at a time (the acceptance run and any posted-input proof move the system cursor). The owner is named in `docs/EVOLUTION.md`;
  every other lane proves headless (`-nullrhi`, automation tests, the remote console on its **own port**: 4700 + your lane number's last two digits) or queues its packaged
  proof with the Director in its report. The Director's release chain takes the cursor for two minutes per package: a disturbed run is repeated, never trusted.
- At most three interactive `UnrealEditor.exe` on the machine; use one only when the brief says so, on your own MCP port, closed after. Builds: `tools/run/build.ps1`; packages:
  `tools/run/package.ps1 ... -ExtraArgs "-ubtargs=-WaitMutex"`; wait out a UBT ConflictingInstance and an AutomationTool already running.
- **Few processes, many seeds.** Every `UnrealEditor-Cmd` start takes a turn at the one build tool lock (the design eye's commandlets waited 3 to 16 minutes each to play
  60 matches in 7 seconds). Batch a measurement into as few process starts as you can; never one process per match or per cell when one can run a list.
- **`-Multiprocess` on every headless start** (lane A107, 20:00): without it every `UnrealEditor-Cmd` start runs `Build.bat -Mode=ValidatePlatforms` and queues on the
  build tool's mutex (a 3-match run: 5 m 17 s; with the flag 9.5 s, identical hashes). The shared drivers on main carry it from 20:10; add it to any command line of
  your own (`-nullrhi -Multiprocess -unattended`) and to your worktree's copies of `tools/playtest/run.mjs`, `tools/soak/playtest/soak.mjs`, `tools/perf/results/a95/curve.mjs`.
- Blender runs headless (`blender -b -P script.py`); find the install as `docs/experiments/BLENDER.md` says.

## The walls
- No git command of any kind (the Director exports and lands your worktree's diff). Never print `.env`. Never kill a process you did not start. Never touch
  `Game/Plugins/JamAssets` (device B's): a figure, material, effect or sound need goes to B in your report with the call or asset name.
- Stay inside your track's file family (named in your brief and in `docs/EVOLUTION.md`). A fault outside it is a finding with the file and line, not an edit, unless your
  gate cannot go green without a one-line fix, which you then name in the report.
- Contracts (`contracts/*`) are the Director's: propose the text in your report.
- The teen rating holds (R-0021, `docs/experiments/VOICE.md`): rough, never cruel.

## Disk is the memory
- Write `status/<lane>.md` from the first minute: what you read, each measurement as it completes, each decision with its number. The Director's process has restarted
  three times in two days; a new agent must be able to resume your evolution from that file and your worktree alone.
- Put bulk output (raw soak data, stills by the hundred, packages, logs) under `Game/Packaged/`, `Game/logs/`, `tools/**/out/` or a `raw/` folder: these never travel in the
  landing. What must land: source, schema, tests, a doc under `docs/experiments/`, up to ten stills, your status file.

## The report (your last message)
1. The evolution in two sentences: what a player or a maintainer gets.
2. The proof: tests with results, the hash before and after, the acceptance rows if any, the numbers.
3. The neutral value or the revert path.
4. Findings outside your family, with file and line.
5. The next three evolutions for your track, ranked, each with its proving number.
6. The file list relative to your worktree.
