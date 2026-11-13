# Submission form: draft answers (lane A96, 2026-11-13; rows re-checked against the repository by lane A142 the same night, `submission/TRUTH.md` FM01 to FM12; the operator pastes)

`[waits: X]` = true only when lane or device X delivers; `<...>` = the operator's blank. Strip the marks when pasting.

Text credits only: vendor names are allowed here and in the dev log, never in the game or the video (rules digest, `docs/PLAN.md` section 2).

| Field | Draft |
|---|---|
| Team name | Hokej |
| Members | `<TEAM: names and roles, operator>` (two devices worked on it; `docs/PLAN.md` section 8 says Solo Driver does not apply to two people) |
| Game name | Hokej |
| Track | Unreal Engine |
| Theme | Director's Cut |
| Pitch (one line) | `page.md` section A |
| Blurb (60 words) | `page.md` section B |
| Zip | `Hokej_Hokej.zip` (`tools/run/package.ps1 -Submission`), top folder `Windows/`, start `ThirdJam.exe`, `README.txt` beside it [waits: the Director's real package run: the script is not yet run for real] |
| Video | `submission/video/hokej-v1.mp4` today (39.5 s, silent, the H24 build); `hokej-v2.mp4` does not exist yet [waits: the capture lane on the next package, device B for sound] (script `video/script-v2.md`) |
| Dev log (Glass Box) | `submission/devlog/index.html` (rebuild: `node tools/media/devlog-curated.mjs --repo <main checkout>`), hosting link `<operator>` (the page is self-contained, 7.8 MB at A96's build; rebuild on Sunday) |
| Main prize | `<operator: no decision is recorded in the repository>` |
| Modifier prize | `<operator: no decision is recorded>`; Glass Box is the claim the repository supports (the log exists and builds); one open gap: device B's transcripts are not in it yet [waits: device B, BS1] |

## Tools you used (the form's field; R-0006 says partner prizes depend on it)
Drafted by A96 and not edited by A142: two lines need the operator's answer before pasting (O5): whether a Rodin or Tripo asset is shipped (the `P4/Rodin` and `P4/Tripo` ledger rows in `credits.md` are Epic clips, `shipped` n; `docs/SUBMISSION.md` says list Rodin only if shipped), and whether the page may name vendors.
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
