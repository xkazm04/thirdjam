# Commentary tooling (lane A32, B-to-A-0026)

| File | Does |
|---|---|
| `check.mjs` | the manifest check for `Game/Plugins/JamAssets/Content/Audio/VO/commentary.json`: ten keys per line, id `<event>.<situation>.<nn>`, team, priority, cooldown, every sound on disk, text through the bark filter; `node tools/commentary/check.mjs` prints ERROR/INFO lines and a JSON line, exit 1 on any error |
| `embed.mjs` | writes `Game/Source/JamGame/Private/CommentaryData.inl` from the manifest (the packaged game carries it without a staging rule); run after B changes the manifest |
| `commentary.test.mjs` | the committed manifest is clean and embedded, the check rejects what the contract forbids; runs in the premerge gate |

The runtime, the rules and the seven events are in `docs/experiments/COMMENTARY.md`. The bank and its register are B's: `briefs/commentary/README.md`.
Workflow after a bank change on `dev-b`: `node tools/commentary/check.mjs` -> `node tools/commentary/embed.mjs` -> build (the C++ test `Jam.Commentary.BankClean`
fails when the embedded copy differs from the file).
