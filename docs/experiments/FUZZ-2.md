# FUZZ-2: what broke three release attempts cannot break the package (lane A159, track S3, evolution 4, 14 Nov 2026)

No game source changed. Tree a7d89871.

## Item 1: the test-guard gate step (seconds, no Unreal)
`node tools/gate/check-test-guards.mjs` (premerge step 3b). Reads `Game/Source` and `Game/Plugins` (not JamAssets). Rules: `unguarded-test` (an
`IMPLEMENT_*_AUTOMATION_TEST`, `*DEFINE_SPEC`, latent command or `FAutomationTestBase` outside `#if WITH_DEV_AUTOMATION_TESTS` / `WITH_AUTOMATION_TESTS`; `||`, `!` and
the `#else` of a positive guard do not count), `no-guard-in-tests` (a file under `Tests/` with no guard), `code-outside-guard` (anything but comments, includes and pragmas
outside the guard in a `Tests/` file), `guard-only-symbol` (non-test code names a function or type declared only inside a guard; a name also declared anywhere outside a
guard is not flagged: heuristic, favours silence), `unbalanced`. Proof, `tools/gate/check-test-guards.test.mjs` (8 tests): the 13 Nov shape (a test below the `#endif`), a Tests/
file with no guard, a guard-only symbol called from non-test code, an unclosed guard and a spec outside it, a real test file with its last test moved below the `#endif`,
the guard forms; the tree as it is: 0 faults in 189 files, 1 s.
Limits: a guard-only symbol that shares its name with any unguarded declaration is not caught; macros that expand to tests are only seen by name.

## Item 2: a `-cut=` reaches its match (`tools/fuzz/cut-reach.mjs`)
One headless `UnrealEditor-Cmd` per cut (Menu map, `-game -nullrhi -Multiprocess`, `-cut=<value>` as accept.ps1 passes it), `jam.menuscreen play` over the remote console
(UJamMenuWidget::PlayNow). Reads the log: the LAST `JAMMATCH` line gives cut, id, source; exactly one `JAMMATCH` and one `JAMCART loaded`. 45 rows: the four accept cuts, the
17 matrix cuts (12 modes, 5 tiers), the regular cut by path and by name, the arenas by name, 12 mode cuts and 8 takes by path, and the player's press with no `-cut=`.
See status/A159.md for the counts. The double build of 14 Nov (the Designer's scheduled apply) did not show from a cold menu on this tree: the Designer logs
"no running game mode took the cut"; it needs a match already running. A row that presses PLAY NOW from inside a running match is the next step.

## Items 3 and 4: the corpora (`tools/fuzz/gen-tonight.mjs`, run by `tools/fuzz/run.mjs`, oracles of `Bar.Fuzz.Cartridges`)
`keys`: six classes (stick, skater, contact, brain, level, five-a-side) of 200 matches: every key alone at both ends, then random inside the ranges with each key at an end (40 %)
or uniform. `cuts`: the twelve mode cuts and the eight takes at rookie, pro and all_star, 20 seeds each (1,200). Oracles: validator parity, brain-vs-brain twice with equal hashes, the
playtester twice, no non-finite state, nothing beyond the boards, the long run must end, the hard-stop oracle. Run: `JAM_FUZZ_HARDSTOP=1 node tools/fuzz/run.mjs --corpus <dir> --workers 3 --long-every 1`.
Results in status/A159.md.

## Item 5: what a gate step covers and what only the release chain catches
| Fault | premerge now | Only the release chain |
|---|---|---|
| Test code outside its guard (Shipping compile) | yes, step 3b (A159) | was: Shipping compile |
| Shipping compile errors of other kinds (a symbol under `#if !UE_BUILD_SHIPPING`, a missing include in the game target) | no | Shipping build (package) |
| Editor build, Jam automation, node tests, ledger, ownership, devlog | yes | |
| Sim/brain drift | only with `-Drift` | |
| Cartridge crashes, NaN, hangs, hash drift (fuzz smoke, regress) | yes (`Jam.Core.FuzzSmoke`) | |
| Every `-cut=` reaches its own match | no (`cut-reach.mjs` by hand, 3 min, heavy) | acceptance run |
| Menu press builds one match, at the player's level | partly (`Jam.Menu.*`, in the editor, one cut) | acceptance run, by eye |
| Cook: missing asset, material usage flag in a package | usage-flag line only | cook and packaged run |
| Packaged boot, shipping console absent, first-run settings | no | package-schedule 10 s run, acceptance on Development |
| Window, cursor, input, framerate, look on a real GPU | no | acceptance run, eye |
| Online match between two machines | lockstep tests headless | operator |
