# Assets ledger

One row per asset file that ships or might ship. The ledger is three things at once:

1. the **provenance** record the contest asks for (what is original, what is AI-generated, what is sourced),
2. the **size** ledger for the packaged-game budget (`docs/PLAN.md` section 4b: art 350 MB, audio 80 MB, zip under 1 GB),
3. the input of `tools/gate/check-assets-ledger.mjs`, which runs at every gate (`docs/GATES.md`).

Add the row in the same commit as the file. Device B owns `Game/Plugins/JamAssets/**`; any device may append rows.

## The three origins

Source: the official rules (digest in `docs/PLAN.md` section 2). Read the rules text again before submission; this is our reading of it.

| Origin | Meaning | What we must do |
|---|---|---|
| `original` | Made by us during the jam: modelled, drawn, recorded or composed by the team or its tools from nothing. | Nothing special. It is scored as ours. |
| `ai-generated` | Produced by a generative tool or service (an image, mesh, texture or audio model). **This is not "sourced"**: the rules treat AI-generated content separately. | Name the tool or service in the `tool` column and list it in the dev log and the tools list at submission. Disclosed AI use does not, on its own, raise or lower a score. |
| `sourced` | Anything not created by us and not generated: Fab or Quixel content, a marketplace pack, a free asset, a font, a sound library. | **Must be noted at submission.** Sourced assets are not scored. Record the `licence` and the `url`; no row without them. |

Rules that apply to every row:

- No trademarks or logos in the game or the video, except the Unreal Engine logo. No Claude, Anthropic, NVIDIA or Meshy logos; text credits go in the dev log.
- A row with `shipped` = `n` is kept (it records what was tried) but does not count toward the budget. Anything imported "to try" should not be committed.
- `size_mb` is the on-disk size of the asset file in megabytes with one decimal (for a `.uasset`, include its `.uexp` and `.ubulk`). The checker warns when it differs from the file by more than a quarter.
- `path` is relative to the repository root, with forward slashes, for example `Game/Plugins/JamAssets/Content/Props/SM_TestAsset.uasset`.
- `kind` is one of: `mesh`, `texture`, `material`, `audio`, `vfx`, `anim`, `font`, `other`.
  Budget groups: art = mesh, texture, material, anim; audio = audio; `vfx`, `font` and `other` are reported but have their own slice (40 MB, owned by A).
- `origin` is one of `original`, `ai-generated`, `sourced`. `device` is `A` or `B`. `shipped` is `y` or `n`. `date` is `YYYY-MM-DD`.
- Asset names follow the contract prefixes: `SM_` `M_` `MI_` `T_` `SFX_` `NS_` `BP_` (see `contracts/README.md`).

## Ledger

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|


Director ruling 2026-11-12: a mesh, texture, rig or motion clip produced by a generation service from our own prompt (Meshy, Tripo, Rodin, text-to-motion) is `ai-generated`, not `sourced`; a clip or mesh taken from a service's preset library (for example Meshy library action 187 `Knock_Down`) is `sourced` and carries the service's licence and the preset id in `url`.
