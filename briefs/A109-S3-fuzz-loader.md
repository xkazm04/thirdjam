# Lane A109, track S3: S3-fuzz-loader (device A, evolution 1, Sonnet)

You are lane A109 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a109` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A109.md`. Your remote-console port if you run a game: 4709.

## Track S3: fuzz and the loader
Family: `tools/fuzz/**`, `Game/Source/JamCore/Private/Tests/JamFuzzTests.cpp` (new: `Bar.Fuzz.*` names so the gate's `Jam` filter skips the long runs, plus one short
`Jam.Core.FuzzSmoke`), `docs/experiments/FUZZ.md`. A loader or sim fault is a finding with the cartridge that shows it (saved under `tools/fuzz/found/`); you fix nothing in
the sim or the loader (track Q2 owns the loader).
First evolution: **no cartridge can crash or hang the game.** (a) a generator from `schema/game.schema.json` and `limits.json`: schema-valid random cartridges with keys at
their limits, at mid-range and in adversarial combinations (the smallest rink with five a side and the most pickups, the shortest timer, every ability at once, the contact
keys at their extremes, a tap window longer than the charge, the smallest reach); 5,000 cartridges through the Node validator and the C++ loader (they must agree, as
`Jam.Core.CartridgeParity` demands) and 600 frames of sim headless with bots: crashes, ensures, NaN positions, a puck outside the boards, a match that cannot end, one seed
giving two hashes; (b) schema-invalid cartridges (a wrong type, an unknown key, a missing required key, a number out of range): the loader refuses each with the pointer and
never crashes; (c) the twelve modes and every shipped cut through the same sim check for 36,000 frames. Report by fault class with the smallest cartridge that shows each.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
