# Lane A55: static analysis and build hygiene over the game modules (device A, from H13.3, night filler 3 of 11, Opus)

You are lane A55 of the Third Jam (team Hokej). Run the analyzers the day never had time for over `JamCore`, `JamNet`, `JamGame`, `JamDesigner`, `JamEditor`, land the safe
fixes, and leave a ranked list of the rest.

Read first: `Game/Source/*/*.Build.cs` (unity off everywhere, R-0022), `Game/Source/ThirdJam.Target.cs`, `Game/Source/ThirdJamEditor.Target.cs`, `tools/run/build.ps1`,
`tools/gate/premerge.ps1` (what the gate runs; the automation filter 'Jam'), `contracts/RULINGS.md` R-0022 (clean build before push, anonymous-namespace clashes),
`engine/` does not exist here: this is the Unreal repo. The engine is installed at `C:\Program Files\Epic Games\UE_5.9` (binary build: no engine source changes).

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a55` (created at main a6b6f5b; build it yourself). Run no git command; never touch `Game/Plugins/JamAssets`;
never print `.env`; never kill a process you did not start. Open no interactive editor. Build concurrency: pass `-ubtargs=-WaitMutex` equivalents where the tooling allows;
other lanes build at the same time, so a `ConflictingInstance` is waited out, not fought.

You own: `tools/analysis/**` (the scripts), `docs/experiments/STATIC-ANALYSIS.md`, `status/A55.md`, and in `Game/Source/**` only the fixes below (one file at a time, each
compiled and the Jam tests green before the next; a fix that changes the sim hash is not safe and is listed instead).

## Job
1. **UBT static analyzer**: `Build.bat ThirdJamEditor Win64 Development -Project=... -StaticAnalyzer=VisualCpp` (and `=Clang` if the clang toolchain is present; say which
   ran); collect warnings per module and per file; dedupe; classify: real bug (uninitialised read, null deref, order of evaluation), smell, noise.
2. **Compile time**: `-Timing` per module and the twenty slowest translation units (`-TraceTimes` or the UBT timing report), the ten heaviest headers by inclusion count;
   where a `.cpp` includes a heavy header it does not need, remove it (compile proves it).
3. **Dead and duplicate code**: functions never referenced (the linker's unreferenced symbol list with `/VERBOSE` or a grep of declarations against uses), duplicated
   helpers across modules (A31/A34 had a duplicate `bShootHeld`; find the rest), `TODO`/`FIXME`/`HACK` lines with their age.
4. **Warnings as the compiler sees them today**: a full rebuild log of the editor target, every `warning C` line counted per file; fix the ones that are mechanical
   (unused variable, signed/unsigned, shadowing, deprecated API with a one-line replacement).
5. **Safe fixes land** (your worktree, compiled, `Jam.*` automation green via the same invocation as the gate, hash unchanged: `Jam.Core.Determinism` green): unused
   includes, unused variables, missing `override`, `const` correctness that changes no behaviour, duplicate helpers folded into one. List every fix with the file.
6. `STATIC-ANALYSIS.md`: the counts before and after, the real-bug list ranked with the file and line and what would go wrong, the compile-time table, the heavy-header
   table, the fixes landed, the fixes not landed and why (hash, risk, owner).

Report: the real-bug list, the fix list, the counts, the file list. Interim at 60 minutes with step 1. Time box 150 minutes.
