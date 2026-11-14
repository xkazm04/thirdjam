# Submission form: draft answers (pass 4, lane A180, Sat 14 Nov 2026 08:00; rows FM01 to FM12 of `submission/TRUTH.md`; the operator pastes)

`[O<n>: ...]` = the operator's blank; the question with that number is in `submission/README.md` section 1. A slot is filled by his answer, never by a guess.
No `[waits: ...]` tag is left in this file. The game described is package **H30** (main `95ae15a6`); `[O15]` decides whether that is the package submitted.

Text credits only: vendor names are allowed here and in the dev log, never in the game or the video (rules digest, `docs/PLAN.md` section 2).

| Field | Draft |
|---|---|
| Team name | Hokej |
| Members | [O1: names and roles; two devices worked on it, so "Solo Driver" does not apply to two people (`docs/PLAN.md` section 8)] |
| Game name | Hokej |
| Track | Unreal Engine |
| Theme | Director's Cut |
| Pitch (one line) | `page.md` section A: "Hockey with the whistle taken out: clay skaters, and the speed you bring is the hit you get." |
| Blurb (60 words) | `page.md` section B (counted: 60) |
| Zip | [O15: which package is submitted]. Today: `Game/Packaged/Hokej_Hokej_H30/Hokej_Hokej_H30.zip` (main `95ae15a6`, acceptance run green, the fresh zip unzipped and played through the window: `status/A.md`, R-0063). The form's file is that package re-cut or renamed by `tools/run/package.ps1 -Submission` as `Hokej_Hokej.zip`, top folder `Windows/`, start `ThirdJam.exe`, `README.txt` beside it. About 267 MiB zipped (H29's measure). Not yet done on any package: the clean-machine test on device B ([O7]) |
| Video | [O17: which film, and who records it]. Nothing of the final film exists yet: `video/SHOTLIST.md` lists every shot and whether a capture exists. The edit is one command on device B (`edit.mjs` on an edit list, proved on stand-ins: `status/TRAILER-report.md` part two). `submission/video/hokej-v1.mp4` (39.5 s, silent) shows Thursday's build at three a side: a record, not the film to submit |
| Dev log (Glass Box) | `submission/devlog/index.html` (rebuild Sunday: `node tools/media/devlog-curated.mjs --repo <main checkout>`), [O8: the hosting link; whether device B's transcripts are exported into it] |
| Main prize | [O4: no decision is recorded in the repository] |
| Modifier prize | [O4: no decision is recorded]; Glass Box is the claim the repository supports (the log exists and builds); its open gap is [O8] |

## Tools you used (the form's field; R-0006 says partner prizes depend on it)
Drafted by lane A96 on Friday afternoon; one line (Blender) widened this morning. Two lines need the operator's answer before pasting ([O5]): whether a Rodin or Tripo asset is shipped (the `P4/Rodin` and `P4/Tripo` ledger rows in `credits.md` were Epic clips, `shipped` n, on Friday's tree; `docs/SUBMISSION.md` says list Rodin only if shipped), and whether the page may name vendors.
- Unreal Engine 5.9 (C++ gameplay, Blueprint visuals, Niagara, MetaSounds)
- Claude Code with Claude Opus, Sonnet and Fable models: planning, all C++ and tooling, the tests, the measured playtests, the Designer prompts, the dev log builder
- Codex image generation: concept images and menu/UI art (ledger rows `ai-generated`, tool `Codex image generation`)
- Meshy (text-to-3D, text-to-motion, rigging), Hyper3D Rodin (3D generation and textures), Tripo3D through fal.ai (3D generation): generated skater figures and animation, all listed `ai-generated`, never `sourced`
- ElevenLabs: commentator and crowd voice, sound effects and music (`ai-generated`)
- NVIDIA DLSS 4.5 (Unreal plugin v8.9.0, Super Resolution and DLAA, ported to 5.9): Settings > DLSS, on by default on RTX GPUs (`docs/experiments/DLSS.md`); answers the NVIDIA questions
- Blender: the arena's pieces, the stands and the crowd, figure takes; evaluating generated meshes
- ffmpeg: capture and video encoding
- Anthropic API: the Designer's live sentence-to-cartridge call (the key is typed by the player; none is bundled; the submitted build ships with the CLI transport off, R-0006/`bAllowCliTransport`)

Check against `submission/credits.md` before pasting. **`credits.md` was generated on Friday afternoon's tree**: since then device B's stands, crowd, roof, fourteen MetaSounds and three figure sets were merged. Regenerate it (`tools/media/credits.mjs`) and rerun `node tools/gate/check-assets-ledger.mjs` on the tree that ships ([O6]); until then the counts below are Friday's.

## Sourced assets (not scored)
`submission/credits.md` as generated on Friday: 62 ledger rows, 1 shipped (the DLSS library, NVIDIA licence); the other 61 are Epic template mannequin clips and materials, `shipped` = n (the proposal to delete the P4 ones: `ledger-proposals.md`, [O11]).
