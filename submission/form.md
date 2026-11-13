# Submission form: draft answers (lane A96, 2026-11-13; the operator pastes, the Director checks against `docs/SUBMISSION.md`)

Text credits only: vendor names are allowed here and in the dev log, never in the game or the video (rules digest, `docs/PLAN.md` section 2).

| Field | Draft |
|---|---|
| Team name | Hokej |
| Members | TEAM: <names and roles, operator> (the older skeleton in `docs/SUBMISSION.md` says 1; `docs/PLAN.md` section 8 says Solo Driver does not apply to two people) |
| Game name | Hokej |
| Track | Unreal Engine |
| Theme | Director's Cut |
| Pitch (one line) | `page.md` section A |
| Blurb (60 words) | `page.md` section B |
| Zip | `Hokej_Hokej.zip` (`tools/run/package.ps1 -Submission`), top folder `Windows/`, start `ThirdJam.exe`, `README.txt` beside it |
| Video | `submission/video/hokej-v2.mp4` (script `video/script-v2.md`); v1 is the fallback |
| Dev log (Glass Box) | `submission/devlog/index.html` (rebuild: `node tools/media/devlog-curated.mjs --repo <main checkout>`), hosting link `<operator>` |
| Main prize | `<Director decides>` |
| Modifier prize | `<Director decides>`: Glass Box is the natural claim (the log exists, builds and carries 60 curated pictures) |

## Tools you used (the form's field; R-0006 says partner prizes depend on it)
- Unreal Engine 5.9 (C++ gameplay, Blueprint visuals, Niagara, MetaSounds)
- Claude Code with Claude Opus, Sonnet and Fable models: planning, all C++ and tooling, the tests, the Designer prompts, the dev log builder
- Codex image generation: concept images and menu/UI art (ledger rows `ai-generated`, tool `Codex image generation`)
- Meshy (text-to-3D, text-to-motion, rigging), Hyper3D Rodin (3D generation and textures), Tripo3D through fal.ai (3D generation): generated skater figures and animation, all listed `ai-generated`, never `sourced`
- ElevenLabs: commentator and crowd voice, sound effects and music (`ai-generated`)
- NVIDIA DLSS 4.5 (Unreal plugin v8.9.0, Super Resolution and DLAA, ported to 5.9): Settings > DLSS, on by default on RTX GPUs (`docs/experiments/DLSS.md`); answers the NVIDIA questions
- Blender: evaluating generated meshes
- ffmpeg: capture and video encoding
- Anthropic API: the Designer's live sentence-to-cartridge call (the key is typed by the player; none is bundled; the submitted build ships with the CLI transport off, R-0006/`bAllowCliTransport`)
Check against `submission/credits.md` (from `ASSETS.md`) before pasting: `ai-generated` tools there are Meshy, Hyper3D Rodin, Tripo3D, ElevenLabs and Codex image generation.

## Sourced assets (not scored)
`submission/credits.md`: 62 ledger rows, 1 shipped (the DLSS library, NVIDIA licence); the other 61 are Epic template mannequin clips and materials, `shipped` = n (propose to delete the P4 ones: `ledger-proposals.md`).
