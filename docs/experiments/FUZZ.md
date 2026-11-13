# FUZZ: no cartridge can crash or hang the game (lane A109, track S3, 13 Nov 2026)

What this is: a generator of cartridges, a headless runner, and the faults they found. Nothing in the sim or the loader was changed (track Q2 owns the loader):
a fault is a finding with its smallest cartridge under `tools/fuzz/found/`.

## The tools
| File | Does |
|---|---|
| `tools/fuzz/gen.mjs` | Seeded generator from `schema/game.schema.json` and `schema/limits.json`. Valid family: every scalar key at its limits, mid-range, uniform or mixed per key, plus 21 named adversarial scenarios (smallest rink 5 a side with 8 pickups, shortest timer, every ability, contact keys at both extremes, tap longer than the charge, smallest and largest reach, ...). Cross-field rules the validator checks (corner radius, spawns, check speed, ability and pickup links) are repaired on the way: 5,000 of 5,000 validate by construction. Invalid family: wrong type, unknown key, missing required key, out of range (both sides), null, fractional frames, and 24 text corruptions (BOM, truncation, comments, trailing comma, `1e999`, `NaN`, duplicate keys, 300,000-deep nesting, a 5 MB string, 20,000 keys). Also 28 hand-made minimal cartridges (`min-*`). `--safe` leaves out what the findings below show to fail (the smoke corpus). |
| `tools/fuzz/run.mjs` | Driver: shards the corpus over N headless `UnrealEditor-Cmd` workers, restarts past a crash (the case named in `progress.txt` is the culprit), kills only its own child on a hang, waits while memory is over 50 GB, writes `report-*.json`, copies the first faults per class to `found/`. |
| `Game/Source/JamCore/Private/Tests/JamFuzzTests.cpp` | `Jam.Core.FuzzSmoke` (gate; `tools/fuzz/smoke`, 80 cases, 600 frames each, 3 s), `Bar.Fuzz.Cartridges` (the corpus from `JAM_FUZZ_DIR`), `Bar.Fuzz.ModesAndCuts` (every cartridge in `Cartridges/`, `modes/`, `ladder/` for 36,000 frames, two seeds). |
| `tools/fuzz/gen.test.mjs` | Node test: same seed gives the same corpus, valid family validates, the smoke manifest matches the validator. |

Per valid cartridge the C++ test does: validator parity (verdict and pointer set against Node's), loader, brain-vs-brain 600 frames twice (hash), the playtester (abilities, sprint)
600 frames twice (hash), every frame checked for non-finite positions and velocities, the puck beyond the boards (1.5 m) and a skater beyond the boards (0.75 m), error-level log lines
(ensures included), and on every 25th cartridge and every `min-*` one the 36,000-frame run that must end (the sim's own stop is `max(33000, timer + 6000)`; the cap here adds a margin).
An invalid cartridge must be refused by `Validate` and `LoadFromString` with a message, never crash.

Run: `node tools/fuzz/gen.mjs --out tools/fuzz/out/run2 --count 5000 --invalid 1500 --seed 2` then
`node tools/fuzz/run.mjs --corpus tools/fuzz/out/run2 --workers 3 --long-every 25`. 6,528 cases take about 7 minutes on three workers.

## Result (run2, seed 2: 5,028 valid including 28 minimal, 1,500 invalid)
No crash, no hang, no ensure; 300,000-deep nesting, a 5 MB string and 20,000 keys are refused cleanly. Faults by class:

| Class | Count | Smallest cartridge (`tools/fuzz/found/`) | What |
|---|---|---|---|
| nan | 9 | `nan--min-setparam-puck-max-speed-neg.json` | hockey.regular plus one hook `on_frame` every 60 frames `set_param puck_max_speed -20`: the puck's position is not finite at frame 2,986. `set_param` accepts -20..20 for any parameter whatever its range. |
| puck-outside | 1 | `puck-outside--min-shrink-every-60.json` | a hook `shrink_rink 0.5` every 60 frames: the puck ends 4.35 m beyond the boards (frame 5,449). |
| skater-outside | 13 | `skater-outside--min-spawn-in-big-box-at-boards.json` | a spawn inside a size-10 box near the boards: the obstacle's push throws the skater 3.4 m out through the boards at frame 1. The validator allows an obstacle over a spawn. |
| never-ends | 5 | `never-ends-unminimized--*.json` (5 files) | match not over after 36,000 to 43,000 frames. All carry extreme `set_param` values (skater speed 0, accel -20, puck drag -20, goal width -20) with `end_period` or `swap_nets` hooks. Not minimised. Hypothesis: the hard stop at `JamSim.cpp` line 1110 runs only while `Phase == Playing`, so a faceoff or pause that cannot resolve never reaches it. |
| parity | 65 | `parity--*.json` | text-level: a malformed file gets the pointer of the key being read (`/game/skater/speed`), Node says `/`; `-1e999` and `NaN` are refused by both with different pointer sets; deep nesting likewise. |
| loader-refuses-valid | 10 | `parity--duplicate-key.json` | duplicate keys: C++ refuses ("duplicate key"), Node accepts (last wins). C++ is the stricter, Node should agree. |
| loader-accepts-invalid | 7 | `parity--bom.json` | a UTF-8 BOM: `FFileHelper` strips it so the loader accepts the file, Node refuses it. |

Run1 (seed 1) found the same classes; its 30 "never-ends" were mostly timer matches of about 36,000 frames that end later by design (oracle fixed), 5 stay open above.
Of the 17 `min-setparam-*` cartridges only `puck-max-speed-neg` faulted: the other extremes (mass -20, friction 20, radius -20, skater speed -20, goal width -20, unknown names) ran 36,000 frames clean on their own. The nan and never-ends cases with several hooks were not reduced further (counts above are fault lines: one cartridge can fault in the brain, playtester and long run).

## Neutral value and revert
Nothing in the game changed. The gate gains `Jam.Core.FuzzSmoke` (3 s) and `tools/fuzz/gen.test.mjs` (0.7 s). Remove the two files and `tools/fuzz/smoke` to revert.

## Proposed fixes (Q2 and the sim's owners; not made here)
1. `set_param`: clamp the value to the parameter's own range (`limits.json game_ranges`), or refuse the cartridge when a value is outside it; unknown parameter names an error.
2. `CollideSkaterStatic`: after pushing a skater out of an obstacle, clamp to the boards; or the validator refuses an obstacle within the skater radius of a spawn.
3. `shrink_rink`: keep the puck and skaters inside the new boards when the rink shrinks.
4. The match hard stop should run in every phase.
5. Node `validate.mjs` rejects duplicate keys and a BOM; the C++ parse error uses `/` like Node.

## The twelve modes and every shipped cut (`Bar.Fuzz.ModesAndCuts`)
35 cartridges (the cuts in `Cartridges/`, the twelve in `modes/`, the ladder's takes and fallbacks), two seeds each, brain-vs-brain twice (hash) and the playtester once,
36,000 frames or the timer's own stop plus a margin: 70 runs, 0 faults (no non-finite state, nothing beyond the boards, every match over, every seed one hash). 138 s on one worker.

## Evolution 2 (lane A118, 13 Nov 2026): the seven fault classes closed
Each fix has a test that failed first (`Game/logs/a118/failfirst2.log`: the regression corpus 18 cases, 9 faults; `Jam.Core.FuzzSetParamClamp` red) and the found cartridge as
its regression case (`tools/fuzz/regress/`, built from `found/` by `tools/fuzz/gen-regress.mjs`; `Jam.Core.FuzzSmoke` runs it after the smoke corpus, with the long run on every valid case).

| Class | Fix | Where |
|---|---|---|
| nan (set_param) | **Refused** by both validators: `param` must be one of the nine tunables (README "Tunables"), `value` inside the field's `game_ranges` range. The sim also clamps (`UJamSim::SetParam`) for callers that bypass the validator. Refusal over clamp-only because it tells the Designer what it did wrong; no shipped cartridge has a `set_param`, so all stay valid. | `validate.mjs` `SET_PARAM_FIELD`/`checkHook`, `JamCartridge.cpp` `CheckHook`, `UJamCartridge::SetParamRange`, `JamSim.cpp` `SetParam` |
| puck-outside (shrink_rink) | `ApplyPendingRink` scales the pending end-zone faceoff dot (icing, a foul) with the boards; before, the drop landed on the old dot, outside the new rink. | `JamSim.cpp` `ApplyPendingRink` |
| skater-outside | **The sim clamps**: after the obstacles push, a skater beyond the boards is put back on them (no validator change: an obstacle over a spawn stays legal). Untouched unless the obstacles left the skater beyond the boards. | `JamSim.cpp` `CollideSkaterStatic` |
| never-ends | Hypothesis confirmed: `Step` returns early in Faceoff / GoalPause, the hard stop ran only in Playing; a hook `on_faceoff -> end_period` loops GoalPause -> Faceoff forever. The hard stop now holds in every phase. | `JamSim.cpp` `Step` (one block before the phase branches) |
| parity / BOM / duplicate keys | One parser both sides: `schema/strict-json.mjs` is a port of `FParser` (`JamJson.cpp`): duplicate keys refused at the repeated key's pointer, a BOM tolerated (the C++ loader always did; README updated), a syntax error at the pointer of the value being read, the same words and offset. `validateCartridgeText`, the CLI, `gen.mjs`, `gen-parity.mjs` use it. | `schema/strict-json.mjs`, `validate.mjs`, README |

`--safe` no longer leaves out anything the findings showed (set_param, shrink_rink, swap_nets, end_period, obstacles, shrink_per_goal are back in the smoke corpus); it still
leaves out the 28 hand-minimal `min-*` cartridges (each runs a 36,000-frame match). The valid family draws `set_param` only inside the rules; the invalid family gains `bad-set-param`.

### Evolution 2 result
`Jam.Core.FuzzSmoke` (smoke 80 cases + regress 18 cases, 6 s) and `Jam.Core.FuzzSetParamClamp` green; the full `Jam` filter 158 of 158; `Bar.Fuzz.ModesAndCuts` 35 shipped files, 70 runs,
0 faults, every brain hash, frame count, score and playtester frame count identical to the baseline (0 of 70 lines differ). Fresh run3 (seed 3, 6,528 cases = 5,011 valid
+ 17 refused minimal + 1,500 invalid, set_param / shrink_rink / swap_nets / end_period / obstacles / BOM / duplicate keys all included): **0 faults**, 0 crashes, 0 hangs, 140 s on three workers
(`-Multiprocess` on the driver's command line: 404 s before).

## Evolution 3 (lane A141, 13-14 Nov 2026): every key fuzzed, the Designer's replies parse one way
- **Key sweep** (`tools/fuzz/keys.mjs`, wired into `gen.mjs`; `--no-sweep`, `--sweep-every N`, `--chain N`): the list of keys is derived from `game.schema.json` (137 scalar keys today, `schemaLeaves()`), so a new key is swept without an edit. Each key at both ends alone (466 cases with the pairs), 24 named pairs at four corners (the three graces with each other and `puck.count`, passive take / body shield, board bonus with restitution, stability, rink width and speed, reverse brake with brake / accel / speed, burst own stop with burst frames, ...), the graces against the hooks that end a period (every period-ending hook, three-period clocks), the board bonus against four shrinking-rink forms, and take grace / burst against `set_param` at both ends of each tunable. A pair naming a key that left the schema throws. `gen.test.mjs` proves every key and every `limits.json` scalar is reached and no sweep case is refused.
- **Designer-reply parity**: `parseDesignerReply` now goes through `parseStrict`, as its C++ mirror always did (`FJamDesignerPrompt::ParseReply` -> `FJamJson::Parse`). `tools/fuzz/gen-replies.mjs` makes mutated replies (duplicate keys, BOM, truncated, out of range, unknown key, prose or a fence around the JSON, refusals, locked keys, deep nesting); `Jam.Designer.ReplyParity` compares kind and pointer set. 56 cases in the gate (`tools/fuzz/replies`), 1,000 with `JAM_FUZZ_REPLIES=tools/fuzz/out/replies`: 0 parity faults.
- **Two limits**: `rules.timer_frames` 600..28800 (a clock above 29,400 can end past the 36,000 cap; the largest shipped is 28,800); `goal_pause_frames` is dropped from the `set_param` tunables (eight left): a hook value is -20..20 so no value could be valid. The sim still clamps it (`UJamCartridge::SetParamRange`) for callers that bypass the validators.
- **Timer oracle**: the smoke checks on every case that its hard stop `max(33000, timer + 6000)` is under 36,000, and runs 4 real idle matches (hook-carrying cases) to Over; an idle run on every case cost 25 s, so the full oracle is `JAM_FUZZ_HARDSTOP=1` for `Bar.Fuzz.Cartridges`. Smoke 9.4 s.
- **Found and fixed**: parity, a hook with `when: null` and `every_frames`: Node added an `every_frames` pointer the C++ validator does not (`validate.mjs` `checkHook` now tests `typeof h.when === 'string'`). Regression case `tools/fuzz/found/parity-hook-when-null--fuzz-bad-01256.json` (19 regress cases).
- **Runs**: 6,528 cases seed 41 (5,011 valid incl. 466 sweep and 500 chained-hook, 1,500 invalid, hard-stop oracle on every valid case): 0 faults, 427 s on 3 workers. 10,011 chained-hook cases (seed 42): 0 faults, 191 s. Note: the sim's hook depth cap is 2 (`JamSim.cpp` `RuleDepth >= 2`), not 8.
