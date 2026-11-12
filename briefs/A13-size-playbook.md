# Lane A13: the size playbook, measured before we need it (device A, from H6.3)

You are lane A13 of the Third Jam (team Hokej). The contest zip must be under 1 GB (`docs/PLAN.md` 2, 4b). The current Shipping package is **376 MB zipped with DLSS** (lane A11,
12:04; staged 548 MB; engine-and-code slice: exe 167 MB, pak about 200 MB of which about 130 MB shader code, VC redistributable 29 MB). Tomorrow device B's textures, figures, audio and
Codex art arrive; the operator wants **tricks measured and ready instead of cutting content first**. Read first: `docs/PLAN.md` 4b, `tools/run/package.ps1`, `tools/gate/package-schedule.ps1`,
`status/A.md` (`## H6 package`), `docs/experiments/DLSS.md` 2 (what the plugin stages), `Game/Config/DefaultEngine.ini`, `DefaultGame.ini`, `Game/ThirdJam.uproject`, `contracts/assets-naming.md`,
`ASSETS.md`, `tools/run/common.ps1`.

You own: `docs/experiments/SIZE.md`, `tools/size/**`, `Game/Config/DefaultGame.ini` and `DefaultEngine.ini` **packaging and compression lines only** (say every line you change), the plugin
enable list in `Game/ThirdJam.uproject` **for engine plugins only** (never the Jam*, DLSS, MCP, Toolset or JamAssets entries). Run no git command. One heavy job at a time: a package is a
heavy job and it needs the shared binaries, so run packages **inside the link windows** (:00 and :30, five minutes; one package takes about 70 s) or from a private copy of the project
(`Source/Config/Content/Plugins/.uproject` under a short scratch path, as lane A9 did, deleted at the end). Never kill a process you did not start. Keep the shared tree compiling.

## Deliverables
1. **`tools/size/report.mjs`**: given a staged folder (or a zip), prints a size table by folder, by asset class (textures, meshes, audio, shaders, maps, code, redist, plugins) using
   `UnrealPak -List` on the `.ucas/.utoc` (or the cook's `Metadata/`), the 30 largest assets, and the delta against a previous report (JSON kept under `tools/size/history/`). Wire it into
   `package.ps1`'s output (one call after a successful stage) so every package from now on records the breakdown.
2. **Measure each technique on the real project**, one at a time against the 12:04 baseline, zip size and staged size, and write `docs/experiments/SIZE.md` as an ordered playbook:
   technique, measured delta (MB zipped), risk, exact config lines or steps, "apply now" or "apply when over X MB". Candidates (measure; do not assume):
   - pak compression: `bCompressed`, `PackageCompressionFormat=Oodle`, `PackageCompressionMethod` (Kraken / Mermaid / Leviathan), `PackageCompressionLevel_Distribution`, `bForceUseProjectCompressionFormat`;
   - the zip step: Deflate level 9 with `-mpass=15 -mfb=258` (7-Zip's zip, if installed; else PowerShell `Compress-Archive` Optimal) versus store; the contest needs a standard zip;
   - shader code: drop the D3D11/SM5 target (`TargetedRHIs` SM6 only) and measure; `bShareMaterialShaderCode=True`, `bSharedMaterialNativeLibraries=True`; `r.ShaderCompiler.AllowDebugInfo` off;
   - engine content: `bSkipEditorContent=True`, `DirectoriesToNeverCook` for engine folders we do not use (EngineDebugMaterials, EditorMaterials, Tutorial, VR, MobileResources, etc.), cook only the listed maps (`bCookAll=False`), `+DirectoriesToAlwaysCook` only where needed;
   - engine plugins: disable the default plugins the game does not use (list each: Paper2D, VR/AR, OnlineSubsystems, Media players, Bink, ChaosCloth, Water, etc.) and measure the exe and pak deltas; keep Niagara, Enhanced Input, Sockets/Networking, TextToSpeech, DLSS and anything a lane uses (grep the sources);
   - executable: `bUseLoggingInShipping=False`, strip symbols, `bOmitFramePointers`, link-time code generation (`bAllowLTCG`), `bUsePCHFiles` irrelevant; measure the exe delta;
   - prerequisites: `IncludePrerequisites` (the 29 MB redist): measure and state the rule-7(f) trade-off (a judge's PC without the VC runtime cannot start the game; keep unless desperate);
   - textures (for device B's rule set, measured on a sample set you generate yourself with primitives or the engine's own textures, not B's assets): Oodle Texture RDO (`TextureEncodeOodleRDO`, lambda 20-40), BC1/BC5 vs BC7, mip stripping (`LODBias`), max size 2048 vs 1024 for props, no virtual textures;
   - audio: Opus/OGG quality 40 vs 70, ADPCM for short SFX, mono for SFX, streaming music;
   - meshes: Nanite off for small props, LOD removal, `KeepMobileMinLODSettingOnDesktop` off;
   - animation compression (ACL) defaults for any skeletal figures;
   - DLSS: already single-DLL; note it.
3. **Apply now** the techniques that are free of risk and measured positive (pak Oodle, zip settings, shader sharing, editor content skip, never-cook lists, unused default plugins, Shipping exe flags), in the Config files you own, and re-run one Shipping package through `tools/run/package.ps1 -Config Shipping -Zip` to confirm the new baseline; record it in `status/A.md` `## H7 size`.
4. **Handoff to device B** (`handoff/A-to-B-0004.md`): the asset-side rules with the measured numbers (texture RDO and formats, mip bias, audio formats and quality, Nanite policy, LODs) as import defaults they set in `JamAssets` (per-folder `DefaultEngine.ini` texture LOD groups or a bulk-edit script through MCP), and the per-asset budget line that `check-assets-ledger.mjs` already enforces.
5. **Emergency ladder** at the end of SIZE.md: the order of further cuts if the budget is still exceeded at H72 (drop SM5 shaders, drop Development-only content, reduce music, 1K textures, then content), each with its measured or estimated saving.

Report: the playbook table with measured deltas, the new baseline zip size, the config lines changed, the handoff, every error. Time box 3 hours; report at 60 minutes with the first six measurements either way.
