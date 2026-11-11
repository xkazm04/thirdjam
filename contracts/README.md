# contracts/: what two agents agree on before they build in parallel

`docs/PLAN.md` section 5, rule 5: before two agents build in parallel, a contract in this folder names every identifier that crosses between
them. If it is not named here, it does not exist; builders do not invent a second spelling. Builders may counter-propose (a builder's
measurement outranks the brief), but they do it in a handoff request or in their report, never by editing a contract. Only the Director
edits files in `contracts/` (`ownership.json`).

## When you need one

- Two devices or two builders will touch the same data, or one produces what the other consumes.
- Anything is read from a file by code that someone else wrote: the cartridge JSON, a data asset, a folder of assets, a log line a script parses.

## What a wire-level contract must name

Every one of these, exactly as it will be spelled in files and code (case and underscores included):

1. **Types**: every struct, class, data asset class or JSON object, with every field: name, type, unit, allowed range, whether it is required.
2. **Enum strings**: the full list of allowed values of every enumerated field. Unknown values are errors.
3. **Asset name prefixes**: `SM_` static mesh, `M_` material, `MI_` material instance, `T_` texture, `SFX_` sound, `NS_` Niagara system, `BP_` blueprint. Plus the name itself (`SM_TestAsset`), not just the pattern, for anything another agent must find.
4. **Folders**: the exact path of each folder an agent reads from or writes to, relative to the repository root.
5. **Data asset names**: the asset name and the class of each data asset, and which cartridge key points at it.
6. **Inputs, with a producer and a consumer.** Every new input (a key, a file, a parameter, an event, a log line) names **who produces it** and **who consumes it first**. An input with no consumer is not added; an input with no producer is a bug waiting to happen.
7. **Acceptance**: how anyone can check that both sides agree (a validator command, a test, a file that must exist).
8. **Version**: a contract has an id (`C-0001`) and a version number. A change is a new version with a line saying what changed and who must re-check.

## The absent-value convention (stated once, here)

- A value that is **not there is an omitted key**. Never `null`, never `0`, never an empty string standing in for "none".
- `0` and `false` are real values and mean what they say.
- **Unknown keys are errors** in every JSON schema. A validator that ignores an unknown key is broken.
- **Lists, not singletons**, for anything a game may have more than one of, even if there is one today.
- Time is counted in physics frames (60 Hz) in tests and bots, never wall-clock; randomness comes from the one seeded stream.

Other contracts refer to this section instead of repeating it.

## Files

- `contracts/TEMPLATE.md`: copy it to `contracts/C-0001-short-name.md`.
- Number contracts in order, never reuse a number. Retired contracts get `status: retired` in their header and stay in the folder.
- The theme-binding hour (H0 to H1) ends with the first game-specific contract; everything before the start is generic.
