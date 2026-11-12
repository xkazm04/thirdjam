# Designer prompts (prep task P3)

The Designer turns a player's sentence into a modified cartridge. These files are the whole prompt library. Nothing here is game content: the schema is a genre-free envelope and the test cartridge is a fake ("test-arena").

| File | Role |
|---|---|
| `designer.system.md` | System prompt. Static, so it is the cacheable prefix. Placeholders `{{LIMITS}}`, `{{ENVELOPE_SCHEMA}}`, `{{GAME_SCHEMA}}` are filled by `tools/designer-eval/assemble.mjs` from `schema/limits.json`, `schema/cartridge.schema.json`, `schema/game.schema.json`. `blocked_terms` is never put in the prompt. |
| `designer.repair.md` | User-message template for a repair attempt. Placeholders `{{ERRORS}}`, `{{SENTENCE}}`, `{{CARTRIDGE}}`, `{{PREVIOUS}}`. |

The first-attempt user message is built in code, not templated: `<player_sentence>` then `<current_cartridge>` (compact JSON). Angle brackets in both are escaped, so a player cannot close our tags.

## Reply contract

The reply is exactly one JSON object, either the **full modified cartridge** or `{"refusal": "<text>"}` (that key only, at most `max_refusal_chars`). `parseDesignerReply` in `schema/validate.mjs` is strict: prose around the JSON, a markdown fence, a bad key, a value out of range, or a change to `schema`, `id`, `title`, `seed`, `tunables` or a protected part of `game` makes the reply invalid, and the validator's sentences go to the repair prompt. The Designer may change `values`, `theme`, the contents of `rules`, `entities`, `ai`, and the hockey `game` paths listed in `DESIGNER_GAME_EDITABLE` (`schema/validate.mjs`): rink numbers and obstacles, pickups, rules.mode / score_to_win / timer_frames, look.tier, ai.difficulty ("easier" / "harder"). Never roles, spawns, camera.mode.

Refusal policy (a design decision, easy to change in the system prompt): if nothing in the sentence can be expressed, refuse; if only part can, apply that part. If any part is inappropriate for 13+ or names a real person, company or brand, refuse the whole sentence.

Why the full cartridge and not a patch: one validator checks one document, the C++ side needs no merge logic, and a repaired reply is checked the same way. The price is output tokens (the fixture cartridge is about 2 KB). If live latency says that is too slow, a `values`-only patch shape is the first thing to try.

## Decision: how to force JSON (brief item 4)

**Chosen: plain text JSON, validated locally, repaired by a second request.** No forced tool call, no `output_config.format`.

Evidence (read from the `claude-api` skill, cached 2026-11-03; none of it was tested live because no live call is allowed in this task):

1. **A forced single tool call does not work on the model chain the plan names.** `tool_choice: {type: "tool", name: ...}` (and `any`) returns HTTP 400 on `claude-sonnet-5-5` ("tool_choice: type "tool" and "any" are not supported for this model"). Haiku 5.5 accepts it, but the third attempt of the chain is Sonnet 5.5, so the chain would break exactly at the retry that is supposed to rescue a failure. The documented replacement, `tool_choice: auto` plus `strict: true`, does not guarantee that a tool is called, so it still needs "was a tool called, else retry" code. A forced call on Haiku also skips thinking.
2. **Structured outputs (`output_config.format` with a JSON schema) cannot carry our real constraints.** It rejects `minimum`/`maximum`, `minLength`/`maxLength`, and requires `additionalProperties: false` on every object. The key rule "values keys must be tunable keys, each inside its own range" is data-dependent, so the response schema would have to be generated per cartridge (a new schema compiles on first use and is cached 24 hours per schema), and ranges, list lengths, unique ids and the immutability of `tunables` and `game` would still need the local validator. The refusal alternative needs an `anyOf`. It adds latency and a moving part without removing the validator.
3. **The local validator is needed whatever the wire format is.** Engine limits (entity totals, unique ids, `ai.applies_to` references, frames-only durations, immutability) cannot be written in a schema subset any model API accepts. The C++ `JamDesigner` must run the same checks on the reply anyway, and text JSON over plain HTTP is the simplest thing to implement there.
4. **Readable errors make the repair loop cheap.** The validator returns sentences that start with a JSON pointer; the repair prompt hands them back with the previous reply.

What to measure in the first live run (the Director has the key): the share of `valid_first_try` on Haiku. If it is clearly low, add `output_config.format` on the Haiku leg only, as a speed-up in front of the same validator, and keep this loop as the safety net. If fences or a leading sentence are the main failure, tolerate a pure markdown fence in `parseDesignerReply` (not done now, because the strict form is what the mock exercises).

## Attempt chain and request settings

1. `claude-haiku-5-5`, fresh request.
2. `claude-haiku-5-5`, repair request (errors + previous reply), only if 1 was invalid.
3. `claude-sonnet-5-5`, repair request built from the errors of attempt 2 and its reply, only if 2 was invalid.

A valid refusal ends the chain at any step. A `stop_reason: "refusal"` from the platform ends it too (Haiku 5.5 has no server-side fallback, so none is requested). A fatal HTTP error (400, 401, 403, 404) stops the chain; 429 and 5xx get one retry after 1.5 s.

Each attempt is one stateless request: system prompt, then one user message. No multi-turn history, so there are no thinking blocks to carry and the system prefix stays cacheable (`cache_control` on the system block; the prefix is about 3.9K tokens, near Haiku's caching minimum, so the first live run should check `cache_read_input_tokens`).

- `max_tokens: 8000` (adaptive thinking counts against it).
- `output_config.effort: "low"` on both models. Both default higher (Haiku `medium`, Sonnet `high`) and the Designer is latency-sensitive. Raise Sonnet if the retry leg proves too weak.
- No `temperature`, `top_p`, `top_k`: these models reject non-default values.
- No `thinking`, no `tools`, no `tool_choice`, no prefill (prefill is a 400 on these models).
- API version header `2023-06-01`, plain `fetch`; no SDK, no dependency.

## Safety notes

- The player's sentence is wrapped in `<player_sentence>` and the system prompt says it is data, never instructions. The eval includes an injection sentence.
- The validator cannot judge whether content is teen-appropriate. The prompt carries that; `blocked_terms` in `schema/limits.json` is an empty program-side list the Director can fill during the jam (matched, case-insensitive, against every string in the cartridge). It stays out of the prompt so it cannot be probed.
- No key is ever written to a file or printed. `live-model.mjs` reads it from the environment per call and uses it only as a header.

## Running the eval

```
node tools/designer-eval/run.mjs --mock          deterministic, no network (default)
node tools/designer-eval/run.mjs --live          needs the key in the environment, else exit 2
node tools/designer-eval/run.mjs --mock --verbose --only double-obstacles
node --test                                      all unit tests (see the note on directory arguments below)
```

Output: a PASS or FAIL line per sentence and a last JSON line `{"tool":"designer-eval","mode":...,"valid_first_try":n,"repaired":n,"failed":n,"refused":n,"tokens_in":n,"tokens_out":n,"ms_total":n}`. A refusal on a sentence marked `expect: "cartridge"` is a FAIL line (but counts in `refused`), as is a cartridge on a sentence marked `expect: "refusal"`. Mock mode reports 0 tokens and 0 ms. Mock results say nothing about real model quality; they only prove the plumbing.

Note on `node --test schema tools/designer-eval`: on Node 24 a directory argument is treated as a file name and fails. Use `node --test` with no arguments (it finds every `*.test.mjs`) or quoted globs: `node --test "schema/**/*.test.mjs" "tools/designer-eval/**/*.test.mjs"`.

## Hour 1 of the jam: what to change

1. Replace `schema/game.schema.json` with the real game-specific object. Keep `additionalProperties: false`.
2. Replace the tunables, values, rules, entities and ai of a new fixture with the real five or so parameters from `GAME.md`; keep `test-arena` for tests.
3. Add the real parameter names to `designer.system.md` guidance only if the descriptions on the tunables are not enough. Prefer better `description` strings: the model reads them to pick knobs.
4. Rewrite `sentences.json` for the real game and update the mock plans in `mock-model.mjs`, then run `--live` once.
5. If the game needs rule `kind` values beyond score, timer, win, lose, spawn, modifier, change the enum in `cartridge.schema.json` (a contract change: tell device B).
