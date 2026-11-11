# tools/skills: builder recipes for the jam

A skill here is a **recipe an agent follows during the jam**: what to read, what to create in which order, the test that proves it, the pitfalls.
Recipes are prep tooling under rule 7(h): they contain instructions and formulas, never game content. Nothing they describe exists before
Thu 2026-11-12 06:00 CET.

| Recipe | Who | When | Builds |
|---|---|---|---|
| `skating-physics.md` | device A, C++ builder | H1 onward (M0 greybox needs it) | the deterministic 60 Hz sim: skater movement, rink, checks, puck, shots, determinism test |
| `skater-toy-figure.md` | device A, C++ builder | after the seam stubs (H1), before H24 | track T-A, the code-driven toy figure behind `IJamSkaterVisual` |
| `skater-retarget.md` | device B, animation builder | T-B1 H12-H36, T-B2 H24-H48 | a skeletal skater: import, IK rig, retarget by Python, minimal AnimBP, swap-in |

Evidence, licences, prices and the reasons behind each choice: `docs/SKATERS-RESEARCH.md`.

## How a builder uses a recipe

1. The lead names the recipe in the builder's brief (`Follow tools/skills/<name>.md`), together with the paths the builder owns (`ownership.json`).
2. The builder reads the recipe's "Before you start" list first; if a contract it names is missing, it stops and asks instead of inventing names.
3. Steps run in the order given. The test section is the acceptance: the builder reports each test as PASS/FAIL with the command it ran.
4. A recipe is advice, a measurement outranks it: if a number or API name here is wrong in the engine, the builder says so in its report
   (and in `handoff/` if another device must act). It does not edit contracts.
5. MCP and Python names in recipes were read from `tools/mcp/TOOLS.md` and the 5.9 headers, not called. Check each with
   `PythonScriptingTools.get_symbol` / `search_symbols` or `mcp.mjs describe` before relying on it.

Loading them as skills: Claude Code reads the front matter (`name`, `description`) if this folder is linked into `.claude/skills/`; Unreal's
`ToolsetRegistry.AgentSkillToolset.CreateSkill` can store the same text as an editor AgentSkill. Either way the file here is the source.

## Laws every recipe assumes

- Time is sim frames at 60 Hz; randomness only from the one seeded stream; visuals never feed gameplay.
- Unknown keys are errors, absent values are omitted keys (`contracts/README.md`).
- Every shipped file has an `ASSETS.md` row in the same commit; no logos or brands in anything that reaches the game.
