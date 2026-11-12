# Lane A41: the zone effect trigger is dead and the headless sweeps run without the game's systems (device A, from H12.0)

You are lane A41 of the Third Jam (team Hokej). Lane A37 found two faults while sweeping every cut (`docs/experiments/CUTS.md`): (1) effect triggers `zone_enter` / `zone_exit` never fire:
`UJamEffectSystem::Step` scans the sim's frame events, but zone events are raised by `UJamZoneSystem` straight onto the bus; (2) the headless path (`JamTournament`, `FJamSimAdapter`,
the Jam.Bot tests, `tools/sweep`, the playtester grid) never creates the rule, zone or effect systems, so every sweep ran every cut without its hooks, variants, zones and pickups;
A37 wired them in a new `JamCutSweepCommandlet` instead. Read first: `docs/experiments/CUTS.md`, `contracts/depth.md` (C-0005), `Game/Source/JamCore/Effects/JamEffectSystem.cpp`,
`Rules/JamZones.cpp`, `AI/JamSimAdapter.cpp`, `AI/JamCutSweepCommandlet.cpp` (A37's, in worktree a37 until merged), `AI/JamBotMatch.cpp`, `AI/JamPlaytester.cpp`,
`Game/Source/JamGame/Private/HokejGameMode.cpp` (how the game wires the three systems), R-0017, R-0019, R-0028.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a41` (the Director creates it at the main that carries A37's commandlet; build it yourself, unity off; headless only).
Run no git command. The sim hash of a cut with no zones, effects or hooks must not change (Jam.Core.Determinism); cuts with them change their hashes by design (say so in the report).

You own: `JamEffectSystem.cpp/.h` (the trigger source), `JamSimAdapter.cpp/.h` and `JamBotMatch.*` (one `FJamSystems` bundle: rule + zone + effect systems created from the cartridge
and stepped like the game mode does, used by the commandlets, the tests and the playtester), `JamCutSweepCommandlet.*` (folds into the bundle), `docs/experiments/SWEEP-WIRING.md`,
`status/A41.md`.

## Job
1. Zone triggers: the effect system subscribes to the zone system's events (or the zone system queues them into the sim's frame events as read-outs): `zone_enter` / `zone_exit`
   effects fire; `Jam.Effects` gains a test with the Carpet cut (291 entries, at least one effect started).
2. One bundle for the headless path: every headless runner builds the rule, zone and effect systems from the cartridge and steps them in the game mode's order; `tools/sweep`,
   A3's tournament, A34's playtester grid and A37's sweep all go through it. Re-run A37's 50-cut sweep and A34's grid once: report what moved (the curve numbers may move for
   boost and chaos cuts; the regular preset must not).
3. The "death" allow entry in `tools/barks/filter.mjs` for "Sudden Death" (hockey vocabulary), with a test.
4. Tests: Jam.Effects zone trigger, Jam.Bot on `hockey.boost` taking pickups through the bundle, Jam.Core.Determinism unchanged on `hockey.regular`.

Report: the moved numbers, the tests, the file list. Time box 75 minutes.
