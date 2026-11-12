# The zone trigger and the headless wiring (lane A41, 2026-11-12 H12)

Status: built, tested, re-swept. Owner: lane A41. Follows A37's two findings in `docs/experiments/CUTS.md` ("Bugs for the Director" 1 and the closing
question): the `zone_enter` / `zone_exit` effect trigger never fired anywhere, and every headless runner but A37's new commandlet played the cuts without
their rule hooks, zones and effects. Files: `Game/Source/JamCore/Effects/JamEffectSystem.{h,cpp}`, `AI/JamSimAdapter.{h,cpp}`, `AI/JamCutSweepCommandlet.{h,cpp}`,
`AI/JamSystemsTests.cpp` (new), `tools/barks/filter.mjs` + `barks.test.mjs`, call sites listed in section 5.

## 1. The dead trigger

`UJamEffectSystem::Step` scanned `Sim->GetFrameEvents()` for `zone_enter` / `zone_exit`. The sim never produces them: `UJamZoneSystem::Update` computes
membership after the sim's step and keeps that frame's events in its own `GetFrameEvents()`, which the game mode only broadcasts to the bus. Carpet Cut's
`carpet-stick` (target `puck`, trigger `zone_enter`, zone `carpet-band`, puck drag x3 for 60 frames): 291 entries, 0 effects started in A37's sweep.

Fix (no new host call): the sim already holds the zone system as its `IJamZoneSource` (R-0019, `UJamSim::GetZoneSource()`, the only implementation is
`UJamZoneSystem`). The effect step reads that system's frame events, the same way `FJamSimAdapter::BuildWorld` reads the pickups through the sim's modifier
source. Both game modes (`HokejGameMode::StepOnce`, `LobbyGameMode`) already call `Zones->Update` before `Effects->Step`, so the order the trigger needs
was in place; events whose frame is not the current one are ignored (a host that skipped the zone update fires nothing rather than stale entries).

Who entered decides the reach, so the existing `self` semantics keep their meaning:

| event | fires effects with target | does not fire |
|---|---|---|
| the puck enters / leaves (`Who` = -1) | `puck`, `rink`, `everyone` | `self`, `own_team`, `other_team` (nobody to attach to) |
| a skater enters / leaves (`Who` = skater) | `self` (= the skater), `own_team`, `other_team`, `everyone` | `puck`, `rink` (the puck did not move onto the zone) |

The `zone` field still restricts a trigger to one zone id (absent = any zone). `Jam.Effects.ZoneTriggerCarpet` pins both rows: a skater placed on the
band starts nothing, the puck placed on it starts `carpet-stick` (one `effect_start`, puck drag x3 from the next frame), and a full bot match on the cut
counts entries and starts and replays to the same hash.

## 2. One bundle for the headless path

`JamAI::FJamSystems` (`AI/JamSimAdapter.h`) owns the three systems the game mode builds around a `UJamSim` and does what `AHokejGameMode::BuildMatch` /
`StepOnce` / `GatherInputs` do, in that order:

1. before `UJamSim::Init`: `UJamRuleSystem::Init(config)`, `Sim->SetRuleHook(adapter)` (so the opening faceoff reaches the hooks); `UJamZoneSystem::Init(config)`,
   `Sim->SetZoneSource(zones)`;
2. after `Init`: `UJamEffectSystem::Init(sim, raw cartridge json)` (binds itself as the modifier source; TAKE 1 effects fire);
3. per frame: `Rules->SetScore` mirror, `Sim->Step`, `Zones->Update`, `Effects->Step`;
4. the brain's world view: slow-ice circles (rect zones by their half diagonal) and the `no_shoot` flags, on top of `BuildWorld`'s pickups.

The adapter forwards rule events to the rule system and counts `end_match` / `end_period` actions; `FJamSystems::StateHash()` folds the three systems'
hashes (the tournament's `hash` field stays the sim's `state_hash`).

`FJamSimAdapter(const UJamCartridge&)` builds the bundle on every `Reset`. `FJamSimAdapter(const FJamGameConfig&)` stays the bare sim for component tests
that bring their own systems (`Jam.Rules.BotChaos`). Every headless runner now goes through the bundle:

| runner | before | after |
|---|---|---|
| `-run=JamTournament` (`tools/sweep/run.mjs`, `tools/tournament`) | bare sim | bundle |
| `-run=JamTournament -player=` (`tools/playtest/run.mjs`, A34's grid) | bare sim | bundle |
| `-run=JamCutSweep` (`tools/cuts/sweep.mjs`, A37) | its own copy of the wiring | bundle (the commandlet only counts) |
| `Jam.Bot.*`, `Jam.Bot.SeasonTakes`, `Jam.Difficulty.*`, `Jam.Bot.VisualSwap`, `Jam.Playtest.*` | bare sim | bundle |

The law: a cartridge with no hooks, zones, effects or pickups hashes exactly as before (`hockey.regular`, `hockey.raw`, `hockey.arena.large`, every cut
that only changes numbers). `Jam.Bot.BundleKeepsRegularHash` runs `hockey.regular` bare and bundled and compares every checkpoint hash; it also shows
`hockey.chaos` moving (its zone and hooks are live now). `Jam.Core.Determinism` runs the bare `UJamSim` and is untouched (section 4).

## 3. What moved: A37's 50-cut sweep, re-run through the bundle

`node tools/cuts/sweep.mjs --seeds 1-5 --jobs 2` on the a41 binary, 20:30-20:37 (53 cartridges: the 30 Designer cuts, 8 takes + 4 fallbacks, 6 presets, 5 test
presets; 5 seeds; 36,000 frames max; the cut's own level; 265 matches). Table and JSON: `tools/cuts/out/sweep.{md,json,jsonl}` (worktree a41). Counts:
**plays 49, ends late 4, no human touch 0, never ends 0, loader rejects 0, no goals 0** (A37 at 19:50 on 50 cartridges: 47 / 3 / 0 / 0 / 0 / 0; the three new rows are
the `hockey.test.figure-*` presets, each the Regular Cut's numbers).

**Two things moved between A37's table and this one, and only one is this lane.** A37 swept on main f89014c; main 74c7c0f carries A31's bot charge, which
"changes bot-match hashes by design" (status/A31.md), so no seed hash of any cut matches and every mean drifts a little (Regular Cut 9116 / 12097 ->
8111 / 11352 frames, 3-2-0 -> 4-1-0; Rough Cut, Midnight, Arcade, The Last Word, Visual A26 and the figure presets print the same numbers, being Regular with
mood words or a visual). On top of that drift, the bundle's own changes are visible exactly where a cut carries systems:

| cut | A37 (19:50, own wiring, dead trigger) | A41 (bundle, trigger live) | reading |
|---|---|---|---|
| **Carpet Cut** | 291 zone entries, **0 effects started** | 268 entries, **52 effects started**, 3-2-0, 7807 / 14211 frames | the trigger fires: `carpet-stick` on the puck's entries (about 10 a match) |
| Power-Up Cut | pickups 27 spawned / 19 taken (human 5), 19 effects | 22 / 13 (human 1), 13 effects, 8483 / 13993 | the same systems, shorter matches at the charged pace |
| Turbo Cut | 52 / 29 (human 5), 28 effects | 70 / 46 (human 7), 44 effects, 11966 / 18311 | the same systems, longer matches |
| Giant Puck Cut | 13 / 8 (human 0), 8 effects | 11 / 7 (human 1), 7 effects | same |
| Neon Cut | hook 1 / 20 fires, 292 entries | 1 / 19, 315 entries | same |
| Second Wind | 21 effects (goal_for trigger) | 22 effects | same |
| Set Pieces, Sudden Death, Maze, Marathon, Slow Motion Cut | hooks 1 / 4, 1 / 4, 1 / 1, 1 / 1, 1 / 3 | 1 / 4, 1 / 4, 1 / 3, 1 / 0, 1 / 3 | Marathon's nine is not reached in 5 seeds now (8.6 goals a match); Maze ends by its hook 3 of 5 |
| hockey.boost | 37 / 23 (human 6), 23 effects, 4 of 5 ended, mean 21,549 | 36 / 21 (human 6), 21 effects, 4 of 5, mean 29,392 | still "ends late" (A37's bug 3, a preset) |
| hockey.chaos | 2 hooks / 11 fires, 82 entries, 2225 / 5158 | 2 / 11, 118 entries, 4285 / 7806 | same systems |
| take-7 Slow Motion | 386 entries | 428 entries | same |
| Pond Cut | 306 entries | 257 entries | same |

Verdict changes: Family Cut (worst seed 18,929 -> 25,547, mean 10,575 -> 21,036) and Rookie Cut (one seed of 5 did not end: 36,000) are "ends late" now and
hockey.raw "plays" (worst 30,387 -> 19,938): rookie-level and 120 x 60 first-to-3 matches under the charge, the same family A37 noted as "long"; not
a systems effect (none of the three has one). hockey.test.visual-b2 has one unfinished seed. For the Director: the two rookie cuts may want take-4's
clock + hook pattern, as A37 did for Slow Motion / Marathon / Maze.

What did **not** move, on the same binary (the only comparison that isolates the bundle): `Jam.Bot.BundleKeepsRegularHash` runs `hockey.regular` bare and
bundled and finds every 600-frame checkpoint hash equal (section 5); the Regular Cut, its mood-word twins and the systems-free presets therefore print the
numbers the bare sim would print on this main. Cuts with hooks, zones or pickups change their hashes against a bare run by design (the brief), and A37's
sweep already ran them wired, so for them this sweep is A37's wiring moved into the adapter plus the live trigger.

## 4. What moved: A34's 18-cell playtester grid, re-run through the bundle

`node tools/playtest/run.mjs --seeds 1-20 --jobs 2 --out tools/playtest/out/a41-grid` on the a41 binary, 20:30-20:34, 18 cells x 20 seeds = 360 matches
(`tools/playtest/out/a41-grid.{jsonl,md}`; the full table with every A34 column is in the `.md`). Both grid rinks (`hockey.regular`, `hockey.arena.large`)
carry no hooks, zones, effects or pickups, so the bundle has nothing to add there; the same-binary proof is in `Jam.Bot.BundleKeepsRegularHash`, which runs
the grid's own path (`RunPlaytestMatch`, the novice at rookie) bare and bundled on both rinks:

| rink | seed | bare sim | bundle |
|---|---|---|---|
| hockey.regular | 1 | `f3114f8f73a9ff4a`, 10,607 frames | `f3114f8f73a9ff4a`, 10,607 frames (the same hash the grid's cell prints for this seed) |
| hockey.regular | 2 | `4187185013f11b6b`, 11,129 | identical |
| hockey.regular | 3 | `c881b01f93484522`, 24,012 | identical |
| hockey.arena.large | 1 | `d8399986e1cbd098`, 11,843 | identical |
| hockey.arena.large | 2 | `4dd6b7dd5ae8629d`, 20,331 | identical |
| hockey.arena.large | 3 | `ccfc15934d2795f0`, 11,523 | identical |

So the grid through the bundle is the grid the bare sim gives on this main, seed for seed.

**What the numbers say against A34's committed table (`grid2.md`, recorded on the main of ~19:00):** everything moved, and none of it is this lane.
Between that record and main 74c7c0f landed A31's bot charge ("changes bot-match hashes by design", status/A31.md) and R-0029's rookie profile knobs
(`DifficultyProfile.cpp`: the rookie back's guard post 10 / 20 m, shoot from reach + 5 m, A34's P1 / P3), so no seed hash of `grid2.jsonl` matches
today's and the rookie row jumped. A39's `a39-after*.jsonl` are not a baseline either (that lane's payoffs are its own binary, still open at 20:35).
The comparison is kept here so the next reader does not re-derive it:

| rink | level | player | win % grid2 (A34, ~19:00) -> a41 (main 74c7c0f + bundle) | GF / GA | < 2 min (of 20) |
|---|---|---|---|---|---|
| regular | rookie | novice / regular / good | 15 / 60 / 70 -> **80 / 100 / 100** | 1.1/2.8, 2.2/1.6, 2.6/1.6 -> 2.8/1.0, 3.0/0.2, 3.0/0.1 | 7 / 14 / 13 -> 14 / 17 / 18 |
| regular | pro | novice / regular / good | 5 / 40 / 50 -> 15 / 25 / 70 | 1.0/3.0, 1.9/2.3, 2.2/2.2 -> 1.2/2.8, 1.5/2.5, 2.5/1.6 | 6 / 10 / 16 -> 7 / 8 / 16 |
| regular | all_star | novice / regular / good | 10 / 30 / 40 -> 5 / 25 / 40 | 0.8/2.9, 1.3/2.3, 1.7/2.3 -> 0.7/3.0, 1.7/2.5, 2.0/2.4 | 8 / 7 / 11 -> 4 / 15 / 12 |
| large | rookie | novice / regular / good | 30 / 80 / 70 -> **85 / 95 / 95** | 0.8/1.4, 2.0/0.9, 1.8/0.7 -> 1.6/0.4, 2.3/0.1, 2.8/0.2 | 4 / 10 / 8 -> 6 / 13 / 15 |
| large | pro | novice / regular / good | 5 / 25 / 40 -> 10 / 15 / 55 | 0.4/2.2, 1.0/1.9, 1.2/1.4 -> 0.7/2.2, 0.8/2.1, 1.8/1.3 | 3 / 7 / 5 -> 6 / 7 / 11 |
| large | all_star | novice / regular / good | 5 / 10 / 20 -> 10 / 0 / 30 | 0.4/2.6, 0.6/2.6, 0.8/2.2 -> 0.3/2.5, 0.3/2.5, 1.0/2.2 | 3 / 2 / 4 -> 0 / 3 / 4 |

Reading for the Director: the rookie row now sits at 80-100 % for every playtester on both rinks (R-0029's knobs did what A34 measured for them: the novice's
"85 %" variant), pro and all_star are where A34 left them within 5-20 points of seed noise. A grid on a cut **with** systems (boost, chaos, the takes) is what
the bundle changes; this grid's cartridges have none.

## 5. Tests and files

Headless on the a41 binary (`UnrealEditor-Cmd -ExecCmds="Automation RunTests Jam.Core.Determinism+Jam.Effects+Jam.Bot+Jam.Rules+Jam.Playtest; Quit" -nullrhi`,
20:28-20:30, 23 tests, 23 pass):

| test | what it pins | reading |
|---|---|---|
| **Jam.Core.Determinism** (untouched, bare `UJamSim`) | the law | `hockey.regular` seed 1 frame 600 **`884f1c9a85793dd5`**, `hockey.raw` **`6027ac7eba473026`**: the same as A33's re-recorded baseline (status/A33.md). Did not move. |
| **Jam.Effects.ZoneTriggerCarpet** (new) | the trigger | unit: a skater on the band starts nothing, the puck on it starts `carpet-stick` once, puck drag x3 next frame; bots: Carpet Cut seed 1, 6,876 frames, 1:3, 53 zone entries, **10 effects started** (was 0 in every run before), two runs same sim and systems hash |
| **Jam.Bot.BoostPickupsBundle** (new) | pickups through the bundle | `hockey.boost` all_star seed 1: 18,698 frames, over, 8 spawned, **5 taken, 5 effects started**, two runs agree; the brain's view sees a pickup within 3,000 idle frames |
| **Jam.Bot.BundleKeepsRegularHash** (new) | the law on the adapter | `hockey.regular` seed 1 bare `a01647a89d7de188` = bundle `a01647a89d7de188` after 5,933 frames, every 600-frame checkpoint equal, 0 effects; `hockey.chaos` seed 1 bare `4befbe84cf2206bf` (6,000 frames, never ends) vs bundle `d0fb6b918ad0271a` (692 frames: 2 hooks, 2 fires, 7 zone entries): the systems are live |
| Jam.Bot.PlayerBotWins, AiBotPlays, HumanHasSomethingToDo, VisualSwap, Jam.Difficulty.* | the existing bars, now through the bundle | pass; `hockey.regular` / `hockey.arena.large` carry no systems so their numbers are main's (VisualSwap: the three figure tracks agree on `1587ed991295ccf5`; A33's `1871c1d348fa8b41` predates A31's bot charge, which "changes bot-match hashes by design" per status/A31.md; `Jam.Bot.BundleKeepsRegularHash` shows the bundle itself adds nothing on a systems-free cartridge) |
| Jam.Bot.SeasonTakes | the 8 takes with their hooks, zones, pickups | pass, every seed replays (`same`); takes 1-3 keep their hashes (no systems), take-4 onward play with the hooks / zones (hashes moved by design) |
| Jam.Effects.* (4 existing) | A14's unit tests | pass; BotMatchWithEffects `hockey.boost` seed 1 8,116 frames, 2 taken, hash `012f85cacddb9562` (unchanged: it wires its own rig) |
| Jam.Rules.* (7) | A15's unit tests, BotChaos on the bare constructor | pass; BotChaos `hockey.chaos` 3:2 in 8,319 frames, hash `26dbfb68c2ae13bc` (its own control, unchanged) |
| Jam.Playtest.NoviceScoresAtRookie | A34's bar through the bundle | pass, 14 of 20 seeds inside 7,200 frames, seed 1 replays `f3114f8f73a9ff4a` (hockey.regular: unchanged by the bundle) |

Node: `node --test tools/barks/barks.test.mjs` 7 of 8 (the new one passes; the failing one, "the embedded C++ copy matches barks.json", fails on the committed
`BarkData.inl` with CRLF line ends and passes on the main tree's working copy of that file: pre-existing, not touched here); `node tools/barks/filter.mjs`
1340 clean, 0 rejected; `node tools/cuts/text.mjs` 1 fail (take-8's card note; was 3).

Files: owned: `Game/Source/JamCore/Effects/JamEffectSystem.{h,cpp}`, `AI/JamSimAdapter.{h,cpp}`, `AI/JamCutSweepCommandlet.{h,cpp}`, `AI/JamSystemsTests.cpp` (new),
`docs/experiments/SWEEP-WIRING.md` (this file), `status/A41.md`. Outside the owned list, one constructor token each so the runner uses the bundle (for the Director):
`AI/JamTournamentCommandlet.cpp` (2 sites), `AI/JamPlaytester.cpp` (the test helper), `Tests/BotTests.cpp` (2), `Tests/SeasonBotTests.cpp` (1), `AI/DifficultyTests.cpp` (1),
`Tests/VisualSwapTests.cpp` (1); `tools/barks/filter.mjs` and `tools/barks/barks.test.mjs` (the brief's item 3). Not touched: `JamBotMatch.*` (the engine-free runner needed
nothing: the bundle lives in the adapter), the game modes, `Rules/JamRulesTests.cpp` (BotChaos stays on the bare constructor as its own control; it builds a rule and a zone
object it never attaches, a note for A15), `Game/Plugins/JamAssets`, every contract.

## 6. The "death" allow entry

`tools/barks/filter.mjs` gains `ALLOW_PHRASES = ['sudden death']`: a banned word inside the phrase does not reject the line, and the phrase's capitals
("Sudden Death", "SUDDEN DEATH:") are not read as a name; `death` alone and every other banned word still reject. `barks.test.mjs` pins it (nine
assertions). `tools/cuts/text.mjs` on the 111 cut lines: 3 fails -> 1 (take-8's card note with the hard film word `cut`, A37's rewrite stands; not mine).
