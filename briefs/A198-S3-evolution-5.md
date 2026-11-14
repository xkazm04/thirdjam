# Lane A198, track S3 (hardening), evolution 5 (device A, Opus)

You are lane A198 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (what the operator ordered at noon and the clock: packages at 15:30 and 18:30, **freeze at 20:00**,
after which only faults are fixed), the track's `status/A159.md` and its hardening docs (the soak set, the fuzzers, the two-instance LAN run), and
today's landings in `git`-free form: the status files `status/A183.md`, `A184.md`, `A191.md`, `A192.md`, `A185.md`, `A194.md`, `A186.md`, `A187.md`,
`A188.md` (what each added: read their READY sections only).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A198.md`. Port 4798. Family:
`tools/soak/**`, `tools/fuzz/**`, the hardening tests (`Jam.Net.*` soak rows, `Jam.Hard.*`), `docs/experiments/HARDENING-5.md`. **You fix nothing in
another lane's family: you find, reduce to the smallest repeat, and report to the Director with the file and line.** Exception: a crash or a hang with a
one-line guard: write the patch as a separate file `status/A198-fix-<n>.patch.md` (the lines, not a git patch) and say so at once.

## Evolution 5: find today's faults before 20:00, so the night only hardens
Since 12:30 main took: new buttons and rebinding with a saved bindings file; free skaters 15 % faster; the camera's bound at the boards; a new front
end (landing, main, modes cards); the held cross-check (a new input bit, **lockstep protocol 9**); wind-ups before hits; the breakaway bonus; boosts'
beacons and the GOAL banner. Nothing of it has soaked.
1. **The headless soak on the newest main** (start it first): the soak set at every level and both rinks, the twelve modes, 30 simulated minutes each:
   no ensure, no check, no NaN, the hash equal between two runs of the same seed (determinism with the new input bit and the wind-up state fields
   `CrossTarget`, `bCrossTargetStickOnly`), memory flat.
2. **Lockstep on protocol 9**: two instances on this machine (the track's driver; loopback only: no firewall prompt answered, no system setting
   changed), host and guest both using the held cross-check and the wind-ups, 10 minutes: no desync; a guest on protocol 8 is refused with words; a
   late join, a guest leaving in a wind-up frame, a pause in a wind-up frame.
3. **Input fuzz**: random inputs with the new bit and every rebindable action at random on both sides for 200 matches: no crash, law checks hold
   (nobody outside the rink, no puck NaN, a goal pause always ends: a wind-up now also runs out in a goal pause: prove the pause still ends on its frame).
4. **The bindings file**: missing, empty, cut in half, unknown keys, two actions on one key, a key that no longer exists, read-only: the game starts
   with words or defaults, never a crash; a rebinding made in a match survives a restart (headless: write, restart, read).
5. **The front end fuzz**: `jam.menuscreen walk` with random keys and clicks for 5,000 steps from every screen: never a screen without focus, never two
   builds from one PLAY (the morning's fault: `StartMatchAtLevel`), Back always returns; each of the twelve modes started from its card and ended.
6. **Frame time**: the capture driver's frame times on clay and on the neon tier with boosts' beacons, the banner and ten skaters: the worst frame
   against this morning's numbers.

One table in your status file, refreshed every 30 minutes: the run, PASS or the fault with its smallest repeat. A fault goes to the Director the minute
it is reduced (a line starting `FAULT` in the status file: the Director's watcher reads it). Write `HARD READY 1` by 17:30 (items 1 to 3) and
`HARD READY 2` by 19:30 (4 to 6). `-Multiprocess`; four processes at most; stop only processes whose ids you recorded. Time box 300 minutes.
