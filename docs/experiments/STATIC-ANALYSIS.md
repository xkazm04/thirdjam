# STATIC-ANALYSIS: analyzers, compile time, dead code and warnings over the five game modules (lane A55)

Lane A55, 2026-11-13 01:22-02:25 CEST, worktree `.claude/worktrees/a55` from main a6b6f5b. Modules: JamCore, JamNet, JamGame, JamDesigner, JamEditor (93 translation units,
about 63 k lines with the generated `.inl` tables). Engine: installed UE 5.9 (binary), MSVC 14.44.35207, Clang 19.1.5 (Visual Studio's `VC/Tools/Llvm`).
Every number below comes from a log in `Game/logs` (names given) or from a script in `tools/analysis/`.

## The scripts (tools/analysis)

| Script | What it does |
|---|---|
| `ubt.ps1 -Label <l> -Extra '<ubt args>'` | One UBT run of the editor target with extra arguments, the way `tools/run/build.ps1` builds (`-WaitMutex`, watched, log `Game/logs/a55-<l>-<stamp>.log`). |
| `warnings.mjs <log> [--list]` | Counts MSVC and clang warnings in a UBT or clang log, deduped by file + line + code + text, per module, file and code, with a class per code. |
| `clang-deep.mjs` | Re-runs clang-cl `--analyze` per TU on the response files a UBT clang-analyzer run left, with the checkers UE turns off put back on (below). |
| `tu-times.mjs` | Compiles every TU alone, one after the other, with `/Bt+ /d1reportTime`: front-end and back-end seconds per TU, inclusive seconds per header. |
| `compiletime.mjs` | Reads what a UBT `-Timing` build and a plain build leave in Intermediate: `.timing` per TU, `.dep.json` inclusion counts. |
| `includes.mjs` | Include candidates: a `#include` whose header declares nothing the `.cpp` names (heuristic; the compile decides). |
| `deadcode.mjs` | Functions declared in a header and never mentioned outside their declaration and definition (code, tests, tools, config). |
| `jamtests.ps1` | The gate's automation step alone (`premerge.ps1` step 5, the same UnrealEditor-Cmd line, filter `Jam`); prints the Determinism line. |

## Premises measured (where the brief was wrong or incomplete)

- `-StaticAnalyzer=VisualCpp` and `=Clang` both exist in UBT 5.9. Any analyzer forces `bDisableLinking` and `bIgnoreBuildOutputs` (UEBuildWindows.cs 1426): no binaries, and the next
  plain build recompiles every project TU.
- **UE's clang analyzer run is nearly blind by default.** UBT passes `-analyzer-disable-checker` for core.NullDereference, core.DivideZero, core.CallAndMessage, core.uninitialized.*,
  core.NonNullParamChecker, core.UndefinedBinaryOperatorResult, deadcode.DeadStores, cplusplus.NewDelete/NewDeleteLeaks/Move, unix.cstring.NullArg (ModuleRules.StaticAnalyzerDisabledCheckers;
  no command-line switch changes it), and `-Wno-uninitialized`, `-Wno-unused-variable`, `-Wno-inconsistent-missing-override` among the warnings. So it found 0. `clang-deep.mjs`
  reuses its response files with those switches removed and `-analyzer-checker=core,cplusplus,deadcode,unix,security.FloatLoopCounter` added.
- **There is no `-TraceTimes` in UBT 5.9.** `-Timing` adds `/Bt+ /d2cgsummary /d1reportTime` and writes `<TU>.timing`. Its numbers measure contention, not cost: 16 compiles at once
  next to other lanes' builds gave DifficultyProfile.cpp 11.8 s and `<cmath>` 4.9 s inside it; the same TU alone is 1.6 s and `<cmath>` 0.03 s. The compile-time tables below are
  `tu-times.mjs` (serial), not the `-Timing` build.
- **The linker's unreferenced-symbol list is not available** without changing the shared link line (`/VERBOSE` on the editor DLLs); `deadcode.mjs` does a declaration-against-use count
  instead (reflection, `virtual`, `override`, UFUNCTION excluded: Blueprints and the engine reach them by name).
- `TODO` / `FIXME` / `HACK` / `XXX`: **none** in Game/Source/Jam* (grep, case-sensitive and not). Nothing to date.
- C:\Program Files\LLVM is clang 22.1.8, outside UE 5.9's preferred 18.1.8-20.x; UBT picked VS's 19.1.5 itself.

## Counts before and after

| Measure | Before (main a6b6f5b) | After (this lane) | Log |
|---|---|---|---|
| Full rebuild, `warning C` lines (editor target, Development) | 2 (1 file, C4701 x2) | 0 | build-ThirdJamEditor-Development-20261113-012311.log / build-ThirdJamEditor-Development-20261113-022021.log (full recompile after the analyzer runs, 80 s) |
| MSVC `/analyze` (UBT VisualCpp, project only), unique in Jam* | 10 (JamCore 9, JamGame 1) | 8 (all reviewed: 3 false positives, 5 noise) | a55-analyze-msvc-20261113-012632.log / a55-analyze-msvc-after-20261113-021058.log |
| Clang analyzer, UE defaults | 0 | 0 | a55-analyze-clang-20261113-013037.log / a55-analyze-clang-after-20261113-021520.log |
| Clang analyzer, checkers back on (`clang-deep.mjs`) | 3 | 1 (noise) | a55-clang-deep-20261113-003600.log / a55-clang-deep-20261113-011741.log (UTC stamps) |
| Include candidates (`includes.mjs`, with its known-symbol table) | 16 (33 before the table) | 8 (all needed: see the last table) | |
| Dead-function candidates (`deadcode.mjs`) | 8 | 8 (listed, not removed) | |
| `Jam` automation | 98 passed, 0 failed | 98 passed, 0 failed (after every fix) | a55-automation-ue-*.log |
| `Jam.Core.Determinism` | Success | Success after every fix | same |

## Real-bug list, ranked

None of the findings touches the shipped game's behaviour; the ranked list is short because the code is clean under both analyzers with every checker on.

1. **tools/tournament native driver does not compile** (build hygiene, not a C++ warning). `Game/Source/JamCore/AI/JamAICore.h:265`
   `JAMCORE_API bool CreaseStance(...)` (lane A50) is engine-free code that `tools/tournament/run.mjs --runner native` (the default runner) compiles with clang++ outside UBT:
   `error: unknown type name 'JAMCORE_API'` in all five sources. What goes wrong: every native tournament run dies at build. Fix (not landed: outside the brief's fix list, owner
   A50 / the Director): `#ifndef JAMCORE_API` / `#define JAMCORE_API` / `#endif` at the top of JamAICore.h (UBT always defines it, so nothing changes in the editor), or `-DJAMCORE_API=`
   in run.mjs. Proven: the same command with `-DJAMCORE_API=` compiles clean under `-Wall -Wextra -Wshadow-all`. **Landed? no.**
2. **Uninitialised read in tests** `Game/Source/JamGame/Private/CameraTests.cpp:123/127, 165/168, 488/490, 536/540` (clang core.CallAndMessage at 127; the other three by hand,
   same pattern): `FVector2D Px;` is filled only when `JamA5Test::Project` returns true, then printed and compared regardless. What goes wrong: when the net is behind the camera
   the test message prints garbage and the in-frame check reads garbage (it can pass by chance after the "in front" check has already failed). **Landed (fix 5).**
3. **Uninitialised read in tests** `Game/Source/JamCore/Private/Tests/JamPayoffTests.cpp:381/406` (MSVC C4701 in the plain build): `PuckAtShot`, `BackAtShot` printed by AddInfo
   when no shot event came. What goes wrong: garbage in the test log line on a run without a shot. **Landed (fix 1).**
4. **Escaping difference** `Game/Source/JamGame/Private/Progression.cpp:84` `Quote()` escapes only `\` and `"`, while `JamCartridge.cpp:52` `Quote()` goes through FJamJson (control
   characters escaped). What goes wrong: a progression string with a newline or tab would write invalid JSON. Inputs today are cut ids and paths, so latent. **Landed? no** (behaviour
   change in a save path; owner JamGame / progression lane).

Reviewed and dismissed (each read in the code):
- C6385 / C6386 `JamAICore.cpp:603/605` (`Pressed[Id]`, Id not proven < 10): every caller loops `i < W.NumSkaters`, and NumSkaters is bounded by kMaxSkaters in all three writers
  (JamSimAdapter.cpp:210 `FMath::Min`, JamBotMatch.cpp:125 parse check, tests set 4). False positive. A guard at the top of StickPlay would silence it; not landed (bot brain path).
- C6387 `JamBotMatch.cpp:131` (`atoi(Tok[0])` may be null): `N != 4` is rejected two lines above. False positive.
- C6336 x5 (`i % 2 ? a : b`): intended, noise. security.FloatLoopCounter `CameraTests.cpp:401` (`Yaw += 30.f` from -180): exact in float, noise.

## Compile time (tu-times.mjs: each TU alone, serial, plain-build flags + /Bt+ /d1reportTime)

Serial total 273 s for 103 TUs (93 sources + generated): JamGame 123.9 s, JamCore 91.5 s, JamNet 25.0 s, JamDesigner 25.0 s, JamEditor 7.7 s. The full parallel build is 90-120 s.

| # | TU | total s | front end | back end |
|---|---|---|---|---|
| 1 | JamGame/Module.JamGame.gen.cpp | 5.40 | 4.19 | 1.17 |
| 2 | JamGame/EffectsBinder.cpp | 5.17 | 4.54 | 0.59 |
| 3 | JamGame/SkeletalSkaterVisual.cpp | 5.15 | 4.23 | 0.87 |
| 4 | JamGame/ToySkaterVisual.cpp | 4.80 | 4.14 | 0.62 |
| 5 | JamGame/HUDWidget.cpp | 4.76 | 3.45 | 1.26 |
| 6 | JamCore/JamPickupActor.cpp | 4.58 | 3.91 | 0.62 |
| 7 | JamGame/DirectorsCallWidget.cpp | 4.56 | 3.33 | 1.19 |
| 8 | JamGame/HokejGameMode.cpp | 4.55 | 3.50 | 1.01 |
| 9 | JamGame/BarkSystem.cpp | 4.34 | 3.30 | 0.99 |
| 10 | JamCore/JamPawns.cpp | 4.29 | 3.68 | 0.57 |
| 11 | JamGame/MenuWidget.cpp | 4.19 | 3.13 | 1.02 |
| 12 | JamGame/CommentarySystem.cpp | 4.00 | 3.07 | 0.88 |
| 13 | JamGame/SettingsWidget.cpp | 3.84 | 3.01 | 0.77 |
| 14 | JamGame/BarkWidget.cpp | 3.71 | 3.06 | 0.60 |
| 15 | JamGame/RinkActor.cpp | 3.70 | 1.96 | 1.69 |
| 16 | JamGame/MenuTests.cpp | 3.65 | 3.05 | 0.55 |
| 17 | JamGame/SignalSystem.cpp | 3.53 | 2.94 | 0.54 |
| 18 | JamGame/CommentaryTests.cpp | 3.51 | 2.66 | 0.80 |
| 19 | JamNet/LobbyGameMode.cpp | 3.50 | 2.80 | 0.66 |
| 20 | JamCore/JamCartridge.cpp | 3.45 | 1.91 | 1.50 |

The five slowest after the generated file are the five TUs that open the Niagara headers (EffectsBinder, the two skater visuals, PickupActor, Pawns): about 1 s of headers each.
All five use UNiagaraSystem, UNiagaraComponent and UNiagaraFunctionLibrary by name; none of those includes is removable.

### Heavy headers

Beyond the shared UnrealEd PCH (what each TU opens itself). By inclusive time over all TUs (tu-times.mjs, serial):

| # | Header | seconds, all opens | opens |
|---|---|---|---|
| 1 | UMG Blueprint/UserWidget.h | 12.45 | 19 |
| 2 | UMG Animation/WidgetAnimationState.h (via UserWidget.h) | 5.71 | 19 |
| 3 | Niagara NiagaraSystem.h | 5.32 | 5 |
| 4 | JamCore/Public/JamCartridge.h | 4.16 | 64 |
| 5 | UMG Components/Widget.h | 3.56 | 20 |
| 6 | JamGame/Public/HUDWidget.h | 3.55 | 6 |
| 7 | JamCore/Public/JamSim.h | 3.51 | 49 |
| 8 | Niagara NiagaraComponent.h | 3.36 | 5 |
| 9 | Niagara NiagaraSettings.h (via NiagaraFunctionLibrary.h) | 3.15 | 5 |
| 10 | Niagara NiagaraFunctionLibrary.h | 3.14 | 5 |

By inclusion count (`.dep.json` of the plain build, TUs that open it beyond the PCH): JamCartridge.h 61, JamRulesConfig.h 61, JamRandom.h 52, JamModifiers.h 51, JamRuleHooks.h 47,
JamSim.h 46, JamEventBus.h 43, JamAICore.h 25, `<cmath>` 25, JamEffectTypes.h 25. JamCartridge.h is the hub: it pulls JamRulesConfig.h, JamModifiers.h and JamRuleHooks.h into 61
TUs at about 65 ms each (4.2 s per full build); not split here (a header change in the contract types is not a safe fix).

## Dead and duplicate code

Dead-function candidates (`deadcode.mjs`; declared, defined, never called, named in no test, tool or config): `UJamBarkSystem::SpeakerFor` (BarkSystem.h:168),
`UJamBarkWidget::GetVisibleBubbleCount` / `GetStripText` (BarkWidget.h:51-52), `UJamPresentation::PushFromSim` (HUDFeed.h:167; its comment still says the game mode calls it),
`UJamHUDWidget::FormatEffects` (HUDWidget.h:59), `UJamInputMap::BindingTableText` (InputMapping.h:52; its comment names CONTROLS.md and the pause card, neither calls it),
`USkeletalSkaterVisual::FigureMeshPath` (SkeletalSkaterVisual.h:126), `UJamLobbySubsystem::GetLink` (JamLobbySubsystem.h:70). Not removed: the brief's safe list does not include
dead functions, and three of them are test hooks or documented API that a lane may be about to use (owners: JamGame barks/HUD/input lanes, JamNet).

Duplicated helpers:

| Helper | Where | Same body? | Done |
|---|---|---|---|
| Clamp, Sign, DegToRad, RadToDeg, Rotate | JamAICore.cpp (AICoreLocal), JamPlaytester.cpp (PlaytesterLocal) | yes | **folded** into `JamAI::AIMath` in JamAICore.h (fixes 13a-c) |
| SmoothStep01 | ToySkaterVisual.cpp, SkeletalSkaterVisual.cpp | yes, and = `FMath::SmoothStep(0, 1, x)` | **folded** onto the engine's (fixes 11, 12) |
| SetErr | DifficultyProfile.cpp, JamBotMatch.cpp (both JamAI, engine-free) | yes | listed: a shared home needs `<cstdio>` in JamAICore.h or a new header |
| RotateToward, MoveToward, wrap | JamSim.cpp (FMath), JamBotRefSim.cpp (std::) | same arithmetic | listed: the reference sim is engine-free on purpose and JamSim is the hash path |
| Fnv, Q (quantise) | JamEffectSystem.cpp, JamBotRefSim.cpp, JamSim.cpp AddQ | same arithmetic (`RoundToDouble` = `floor(x + 0.5)`) | listed: hash path |
| Quote | JamCartridge.cpp (via FJamJson), Progression.cpp (hand-rolled) | **no**: see real bug 4 | listed |
| ReadName / ReadNum family | JamEffectTypes.cpp, JamCartridge.cpp | different contracts (bool return vs in-place) | listed |
| LoadPreset / MakeSim / GivePuck | ten JamCore test files | near-identical test fixtures | listed: a shared test header is a test-owner call |
| MakeText | HUDWidget.cpp, LobbyWidget.cpp | no (different styling) | leave |

## Fixes landed (each: one file, built with tools/run/build.ps1, `Jam` automation 98/98, Jam.Core.Determinism Success, 0 `warning C`)

| # | File | Change |
|---|---|---|
| 1 | Game/Source/JamCore/Private/Tests/JamPayoffTests.cpp | `PuckAtShot`, `BackAtShot` zero-initialised (C4701 x2) |
| 2 | Game/Source/JamCore/AI/JamSystemsTests.cpp | `#include "JamEventBus.h"` removed |
| 3 | Game/Source/JamCore/AI/JamCutSweepCommandlet.cpp | `#include "UObject/StrongObjectPtr.h"` removed |
| 4 | Game/Source/JamGame/Private/CameraTests.cpp | `#include "InputAction.h"` removed |
| 5 | Game/Source/JamGame/Private/CameraTests.cpp | `Px` (x3), `PxA`, `PxB` zero-initialised (clang core.CallAndMessage) |
| 6 | Game/Source/JamGame/Private/MenuFlow.cpp | `#include "Engine/Engine.h"` removed |
| 7 | Game/Source/JamNet/Private/Tests/JamNetTests.cpp | `#include "Misc/FileHelper.h"` removed |
| 8 | Game/Source/JamDesigner/Private/JamDesignerModule.cpp | `#include "JamDesignerSubsystem.h"` removed |
| 9 | Game/Source/JamGame/Private/MenuWidget.cpp | `Components/CanvasPanel.h`, `Components/CanvasPanelSlot.h` removed |
| 10 | Game/Source/JamCore/AI/JamPlaytester.cpp | never-read `NearOpp` index removed (clang deadcode.DeadStores) |
| 11 | Game/Source/JamGame/Private/ToySkaterVisual.cpp | local SmoothStep01 -> `FMath::SmoothStep(0.f, 1.f, x)` |
| 12 | Game/Source/JamGame/Private/SkeletalSkaterVisual.cpp | the same |
| 13a | Game/Source/JamCore/AI/JamAICore.h | `namespace JamAI::AIMath` with the five shared helpers |
| 13b | Game/Source/JamCore/AI/JamAICore.cpp | AICoreLocal uses AIMath, its copies removed |
| 13c | Game/Source/JamCore/AI/JamPlaytester.cpp | PlaytesterLocal uses AIMath, its copies removed |
| 14 | Game/Source/JamDesigner/Private/JamDesignerConsole.cpp | `#include "JamCartridge.h"` removed |

Missing `override`: none (clang `-Winconsistent-missing-override` back on: 0). Unused variables: one (fix 10). Signed/unsigned, shadowing, deprecated API: none in the full rebuild
(UE 5.9 builds with shadowing as an error already).

## Fixes not landed, and why

| Item | Why not | Owner |
|---|---|---|
| JAMCORE_API guard in JamAICore.h (real bug 1) | outside the brief's fix list; one line, no editor effect | A50 / Director |
| Progression.cpp Quote through FJamJson (real bug 4) | changes what a save path writes | progression lane |
| bound check in FJamBrain::StickPlay (C6385/6386) | bot-brain path: no behaviour change for valid input, but not a listed fix class | A2 / Director |
| eight dead functions | dead-function removal is not in the safe list; some are test hooks | JamGame / JamNet owners |
| SetErr, RotateToward/MoveToward, Fnv/Q, test fixtures | engine-free boundary or the hash path, or test ownership | Director |
| JamCartridge.h split (64 opens, 4.2 s per build) | contract types header | Director |
| MenuTests.cpp without MenuWidget.h | tried: C2027 at line 187 (a dereferenced `UJamMenuWidget*`), restored | (needed) |
| The 8 remaining include candidates (JamCartridge.h in four JamGame files, MenuWidget.h in MenuTests.cpp, DirectionalLightComponent.h, GameViewportClient.h, DLSSLibrary.h) | MenuWidget.h proven needed (above); the JamCartridge.h four dereference a returned UJamCartridge* (e.g. HUDWidget.cpp:504 `GetCartridge()->SourcePath`), so the include is what makes that compile; DirectionalLightComponent.h gives ULightComponent::SetIntensity at CameraProofScene.cpp:120; DlssJam.cpp is behind JAM_WITH_DLSS (plugin absent in this worktree, untestable) | (needed) |
