# The Designer learns tonight's keys and gains six cuts (lane A44, device A, 2026-11-12 H12.4 to H13.9)

Status: built, tested, swept 21:01-22:40 (the brief's 90 minutes ran to about 100). Owner: lane A44. Worktree `.claude/worktrees/a44` at main 1f964a9. Builds on A7 (the Designer), A37 (CUTS.md: the sweep,
the text check, the three empty spots), A39 (PAYOFFS.md section 2: the sentences per payoff), A32 (the booth), A40 (the camera keys), A16 (foul grace), A21 / A31
(trim glow, the broadcast tier). CLI transport: the operator's Claude Code subscription through `claude -p --model claude-haiku-5-5` (R-0004 point 2); 15 calls of the 40 allowed.

## 1. The keys (`schema/validate.mjs` `DESIGNER_GAME_EDITABLE`, the one-liners in `prompts/designer.system.md` section "The payoffs, the booth, the camera and the look")

| Family | Path(s) added | Range (limits.json) | Default | The sentence that drives it (prompt wording) | Paraphrase, live (haiku, first try) | Changed, and nothing else |
|---|---|---|---|---|---|---|
| one-timer | `shot.one_timer` (whole object) | `window_frames` 0..60, `cone_bonus_deg` 0..30 | 10 / 8 | "deadly one-timers": window 20-30, cone 15-20; "no one-timers": window 0 | "make one-timers deadly" | `{ window_frames: 25, cone_bonus_deg: 18 }` |
| breakaway burst | `skater.breakaway` (whole object) | `speed_bonus` 0..1, `frames` 0..300 | 0.2 / 60 | "give breakaways a burst": 0.35-0.5 for 90-150 frames; "no burst": 0 | "give breakaways a burst" | `{ speed_bonus: 0.4, frames: 120 }` |
| deke | `stick.deke` (whole object) | `offset_m` 0..2, `commit_frames` 0..60 | 1.0 / 15 | "let me deke the last man", "dangles": 1.3-1.6 m, 25-35 frames; "no dekes": 0 | "let me deke the last man" | `{ offset_m: 1.5, commit_frames: 30 }` |
| foul grace | `stick.foul_grace_frames` | 0..120 | 12 | "no cheap fouls after a strip", "let me finish my check": 60-90; "strict refs": 0 | "no cheap fouls after a strip" | `75` |
| the booth | `commentary` (whole object) | `enabled`, `intensity` quiet \| normal \| loud | true / normal | "a quiet commentator": quiet; "no commentary": enabled false; "a loud booth": loud; the booth is not `barks` | "a quiet commentator" | `{ enabled: true, intensity: "quiet" }` |
| chase camera | `camera.boards_margin_m`, `camera.end_height_m`, `camera.far_net_bias` | 0..5, 1..15, 0..1 | 0.8 / 5 / 0.5 | "camera higher at the end boards": end_height_m 10-14; "hug the boards": margin 0.5; "look at the far net more": bias 0.7-0.9; never the other camera keys | "camera higher at the end boards" | `camera.end_height_m: 12` |
| the look | `look.tier` gains `p4_broadcast` in the one-liner; `look.trim_emissive` | tier enum; 0..100 | l2_night; by tier (30, clay 12) | "the broadcast look", "like it is on TV": p4_broadcast; "glowing boards" 60-80; "dim the trim" 5-10 | "the broadcast look" | `look.tier: "p4_broadcast"` |

Still the host's: `camera.mode`, `distance_m`, `height_m`, `look_ahead_m`, `lag_frames`, `fov_deg`, `radar`; the rest of `skater`, `shot`, `stick`; `teams`, `puck`, the `ai`
numbers; `skater.visual`. `Jam.Designer.KeyFamilies` (new, `Private/Tests/JamDesignerKeysTests.cpp`) proves per family: a mock reply over hockey.regular parses
as a cartridge with only the family's pointers changed; an out-of-range value comes back as the pointer error (`at /game/shot/one_timer/window_frames: ...`);
nine protected keys next to the new ones (`camera.fov_deg`, `camera.mode`, `shot.speed`, `shot.charge_speed_bonus`, `skater.speed`, `skater.pass_speed`,
`stick.reach_m`, `look.fog_density`, `camera.height_m`) are still refused; the prompt names every family and stays under 70 KB. Its live section
(`TJ_DESIGNER_CLI_LIVE=1`) runs the seven sentences through the game's own CLI transport with the same check; not run tonight (the Node twin ran instead,
one set of calls, same chain byte for byte: PromptParity).

The paraphrase run (`Game/Source/JamDesigner/Tools/paraphrase-keys.mjs`, 22:1x): 7 of 7 first try, 7 calls, 42.3 s wall clock (5.5-6.3 s a sentence),
167,825 tokens in / 10,839 out, $0.009 as the CLI reports it. Every reply changed exactly the family's key, with a value inside the prompt's suggested band, and
nothing else: not `values`, not `barks`, not even `theme.name`. The tool's first diff reported the two `stick` families at the parent pointer (`/game/stick`,
because hockey.regular has no `stick` object); the diff now walks an added object leaf by leaf (tool and C++ test alike); the recorded changes were already unambiguous.

## 2. The prompt: 69,274 -> 60,503 bytes with the new section in (brief item 3)

The system prompt sat at 69,274 bytes before this lane; the new section alone would have pushed it past 70 KB. `{{GAME_SCHEMA}}` is now the game schema trimmed to
`DESIGNER_GAME_EDITABLE` (`trimGameSchema` in `tools/designer-eval/assemble.mjs`, `TrimGameSchema` in `JamDesignerPrompt.cpp`, key order kept): a top-level key named
whole in the list (`pickups`, `effects`, `barks`, `commentary`) stays whole; one named through sub-keys (`rink.length_m`, `shot.one_timer`, `camera.end_height_m`) keeps
those sub-keys and loses the rest, its `additionalProperties: false` and the `required` entries that went; `teams` and `puck` go. The prompt says so next to the
excerpt ("the current cartridge's `game` has other keys too ... copy those exactly as they are"). `Jam.Designer.PromptParity` (FNV-1a over the C++ and Node texts)
held on the first build after the change: the two trims agree byte for byte. Per attempt the CLI now reports about 23,975 input tokens (prompt plus the
compact hockey.regular) against about 27,500 before; the cached prefix is the system block.

## 3. The six cuts (`gen-cut-library.mjs --merge`, library 30 -> 36, base guard unchanged: `base_sha256` dfc7b19c... is today's hockey.regular)

The generator gained `--merge`: cuts the current file already has over the same `base_sha256` are kept (re-validated, keywords refreshed from `LIBRARY`), only the
sentences it lacks go to the model, the file is written in `LIBRARY` order under the same guard; `--only` still writes nothing; the JSON line carries `calls` and
`cost_usd`. No cut was edited by hand.

| Cut | Sentence (menu) | What the model did (diff against hockey.regular) | Chain | Fun-space spot |
|---|---|---|---|---|
| Carry It In Cut | "carry it in, no shots from far out" | `rink.zones`: two `no_shoot` rects x +-22, 30 x 45 m (shots only inside 7 m of centre or 8 m of an end board) | haiku first try (second sentence; the first, "no long shots, carry it into the slot", gave one 60 x 45 centre rect plus an unasked breakaway burst and was redone) | A37's `no_shoot` (none used) |
| Comeback Cut | "help whoever is losing" | `rules.hooks`: `on_goal`, team `conceding`, `apply_modifier speed 480` + announce "CATCH-UP BOOST" | haiku first try | A37's goal-against boost (as a hook, not an effect) |
| Shield Cut | "a two metre shield in front of each net and a four minute clock" | `rink.obstacles`: `goal_shield` x +-40 size 2; `rules.mode timer`, `timer_frames 14400` | haiku first try (second sentence; "a shield in front of each net ..." gave size 4 on a 4 m net, 0.8 goals a match and an overtime that never ended: see the sweep) | A37's `goal_shield` (none used) |
| One-Timer Night | "one-timers that always find the net" | `shot.one_timer { 30, 20 }` | haiku first try | payoff 1 |
| Breakaway Cut | "give breakaways a burst" | `skater.breakaway { 0.4, 120 }` | haiku first try | payoff 2 |
| Deke Cut | "let me deke the last man" | `stick.deke { 1.4, 30 }` | haiku first try | payoff 3 |

Cost of the six: 8 calls (two redos: Carry It In for a stray key, Shield for the sweep), 67 s of model time, 191,823 tokens in / 15,494 out, $0.029 as the CLI counts it. Text check (`tools/cuts/text.mjs`, 130 lines):
no fail among the 36 cuts; names and sentences carry "Cut" under the menu allowance; "CATCH-UP BOOST" is clean on the scoreboard. The one FAIL in the table is
take-8's note (A37's finding, ladder, not this lane's).

## 4. The sweep (`tools/cuts/sweep.mjs --group cuts`, 36 cuts, 5 seeds, the cut's own level, the scripted human)

Run 22:33-22:35 (36 cuts, 105 s, 3 commandlets at a time; the real brain at the cut's level, the scripted human; outputs `tools/cuts/out/sweep.{md,json,jsonl}`
in the worktree), then the redone Shield Cut alone (22:36). The six:

| cut | level | rules | seeds | W-L-D | ended | end by | frames mean/max | goals | human touches | hooks/fires | zones/enters | periods | verdict | detail |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Carry It In Cut | pro | first to 3 | 5 | 5-0-0 | 5/5 | score | 8929/10455 | 3.8 | 96 | 0/0 | 2/718 | 1 | plays | the two no_shoot zones were entered 718 times; the human won every seed (the bots will not shoot from inside them) |
| Comeback Cut | pro | first to 3 | 5 | 4-1-0 | 5/5 | score | 13567/23747 | 4 | 134 | 1/20 | 0/0 | 1 | plays | long: worst seed 23747; the catch-up hook fired 20 times in 5 matches |
| Shield Cut (first sentence, size 4) | pro | timer 14400 | 5 | 3-1-0 | 4/5 | timer/none | 25005/36000 | 0.8 | 326.2 | 0/0 | 0/0 | 3 | **ends late** | a 4 m shield in front of a 4 m net: 0.8 goals a match; a tie at the horn went to overtime and one seed never scored; redone |
| Shield Cut (shipped: "a two metre shield ...", size 2) | pro | timer 14400 | 5 | 5-0-0 | 5/5 | timer | 14954/15334 | 2.2 | 154.4 | 0/0 | 0/0 | 3 | plays | the clock ends every seed; 2.2 goals a match through the 1 m gaps |
| One-Timer Night | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 13817/19786 | 4.8 | 149 | 0/0 | 0/0 | 1 | plays | long: worst seed 19786 |
| Breakaway Cut | pro | first to 3 | 5 | 4-1-0 | 5/5 | score | 11015/19738 | 3.6 | 121.8 | 0/0 | 0/0 | 1 | plays | long: worst seed 19738 |
| Deke Cut | pro | first to 3 | 5 | 3-2-0 | 5/5 | score | 16277/21572 | 4.8 | 179 | 0/0 | 0/0 | 1 | plays | long: worst seed 21572 |

Verdicts on the 36: **34 play, 2 ends late**, 0 no human touch, 0 never ends, 0 loader rejects, 0 no goals. The two late ones are A37's, untouched by this lane and
untouched since A37's sweep: Big Glass Cut (1 of 5 seeds did not end; mean 20,598; A37 measured 13,959 / 23,569 "long") and Final Cut (first to 5; mean 22,977, worst
27,388; A37 measured 13,439 / 15,422). The sim moved under them between f89014c and 1f964a9 (the payoffs, the bots' checks at pro): a 160 x 80 rink and a
first-to-five at pro now run past the bar in 5 seeds. For the Director / A37's owner: the take-4 pattern (a clock plus the first-to hook) would close both; not
regenerated here (a redo costs a call and changes the cut's sentence to say the clock).

## 5. First-try validity and cost on the 36 (brief item 3; A7 had 24 of 30)

| | Cuts | First try | Repaired | Failed |
|---|---|---|---|---|
| A7's 30 (as shipped, `outcome` in the file) | 30 | 23 | 7 | 0 |
| A44's 6 (tonight, haiku, the trimmed prompt) | 6 (8 runs) | 8 of 8 | 0 | 0 |
| the 36 shipped | 36 | 30 | 6 | 0 |
| the 7 paraphrases (not in the library) | 7 | 7 | 0 | 0 |

The 30 were not re-sent (36 calls would have spent the budget on cuts that are still valid over the same base); their outcome is the one recorded when they
were made. Tonight's 15 calls: 15 of 15 first try, 359,648 tokens in / 26,333 out, 110 s of model time, $0.038 as the CLI counts it. The repair chain (haiku ->
haiku repair -> sonnet) is unchanged and untouched tonight: no repair was needed.

## 6. Tests

- `Jam.Designer.*` headless on the final build (22:37, UnrealEditor-Cmd -nullrhi): 10 of 10 PASS: CliLive (skipped inside, opt-in), CliTransportParsesResult, CutsRoundTrip (36 sentences each resolve to their own cut and are legal Designer edits), DesignerOfflineResolves, DesignerValidatesOutput, DirectorsCallRange, KeyFamilies (new), NoKeyNoNetwork, PromptParity (C++ system text = Node text, trimmed alike; embedded files current), RulesRoundTrip. The intermediate run at 21:16 caught two things fixed since: the diff at the parent pointer for an added `stick`, and the embedded cuts lagging the file while the generator was writing it.
- `gen-designer-data`: PASS, 36 cuts valid, 6 files embedded, system prompt 60,503 bytes.
- Node: 183 of 184 (`schema/**`, `tools/**`); the one failure is `tools/barks/barks.test.mjs` "the embedded C++ copy matches barks.json" on committed main's
  `BarkData.inl` layout (the main tree passes it): foreign, not touched.

## 7. Files

Owned: `Game/Source/JamDesigner/Private/JamDesignerPrompt.cpp` (TrimGameSchema), `Public/JamDesignerPrompt.h` (comment), `Private/Tests/JamDesignerKeysTests.cpp` (new),
`Private/JamDesignerData.inl` (regenerated), `Tools/gen-cut-library.mjs` (--merge, six sentences, counters), `Tools/paraphrase-keys.mjs` (new),
`Game/Content/Designer/cached-cuts.json` (36 cuts, through the generator only), `docs/experiments/DESIGNER-2.md`, `status/A44.md` (main tree).
Outside the lane's listed paths, device A's, as the brief's job needs: `schema/validate.mjs` (nine paths in `DESIGNER_GAME_EDITABLE`), `schema/hockey.test.mjs`
(the camera assertion now excludes `camera.mode` and roles only, plus a payoffs round trip), `schema/README.md` (one line), `prompts/designer.system.md` (the
paragraph of what may change, the new section, the note next to the schema excerpt), `prompts/README.md` (one line), `tools/designer-eval/assemble.mjs`
(`trimGameSchema`). No contract touched.

## Questions for the Director

- `Jam.Designer.CutsRoundTrip` allows 24 to 40 cached cuts (R-0004): 36 now; the next four sentences fill it.
- The Comeback Cut's boost is a rule hook (`apply_modifier speed 480` on the conceding team) rather than A37's `goal_against` effect; the sweep's `rule_fires`
  shows it firing. Keep, or redo with a sentence that asks for the effect form?
- The two `stick` families (deke, foul grace) diff at `/game/stick` when hockey.regular has no `stick`; the validator's own error for a protected `stick.reach_m`
  therefore says `at /game/stick:` (the object as a whole). Fine for the repair loop; noted for whoever reads the pointers.
