# Lane A118, track S3 (fuzz and the loader), evolution 2 (device A, Opus)

You are lane A118 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, and the track's first evolution in your worktree: `status/A109.md`, `docs/experiments/FUZZ.md`, `tools/fuzz/**` (13 cartridges in `found/`).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a109` (the track's: evolution 1 is committed there as the baseline and is landing; its editor is built).
Your status file: `status/A118.md`. Port 4718.

Family for this evolution (wider than the track's, by the Director's order, because these are the faults the fuzz found): the hook and effect code that applies
`set_param`, `shrink_rink`, `end_period`, `swap_nets` (`Game/Source/JamCore/Effects/**`, `Rules/**` and the lines in `JamSim.cpp` they call), `CollideSkaterStatic` and the
match's hard stop in `JamSim.cpp` (only those two functions), `JamCartridge.cpp` (the loader), `schema/validate.mjs` and the parity tool, `tools/fuzz/**`, the fuzz tests.
Tracks F1, F2 and Q2 are editing contact, the stick and the brain in `JamSim.cpp` tonight: keep your edits inside the functions named, so the landings merge.

## Evolution 2: no cartridge breaks the game (the seven fault classes closed)
Each with a test that fails first, and the found cartridge as its regression case:
1. **`set_param` cannot make the sim non-finite**: a hook's value is clamped to the named parameter's range in `limits.json` (or refused by both validators when
   outside it: choose the one that keeps shipped cartridges valid, and say which), and an unknown parameter name is an error in both validators.
2. **Nothing ends outside the boards**: `shrink_rink` keeps the puck and the skaters inside the new boards; a skater pushed by an obstacle is clamped to the boards
   (or the validators refuse an obstacle over a spawn: say which).
3. **Every match ends**: the hard stop applies in every phase (confirm lane A109's hypothesis at `JamSim.cpp` near line 1110 first); the five never-ending
   cartridges end.
4. **The two validators agree**: Node rejects duplicate keys and a BOM as the C++ loader's contract says (or the loader accepts what Node accepts: one rule, both
   sides, the README updated), and a syntax error reports the same pointer.
5. **The guard**: the fixed classes leave the `--safe` exclusion, `Jam.Core.FuzzSmoke` covers them, and a fresh 6,500-case run shows 0 faults.

The gate: the full `Jam` filter green; the hashes of shipped cartridges unchanged (prove it by the gate's hash lines and a run of the 35 shipped cartridges); a
contract sentence for the Director (a hook's `set_param` value lies inside the parameter's range). Time box 100 minutes; interim in the status file.
