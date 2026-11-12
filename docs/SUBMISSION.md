# Submission: Epic AI MegaJam 2026, Unreal Engine track

Skeleton of the submission form, filled as far as known at H4.7. Fields marked `TBD` are filled at H84; `<operator fills>` is for the human only.
Checklists are copied from `docs/GATES.md` (Submission candidate H84, Submission H88); tick them here when done and record who looked and when in `status/`.

## The form

| Field | Value |
|---|---|
| Team name | Hokej |
| Members | 1 |
| Contact (Epic Games account email) | `<operator fills>` |
| Game name | Hokej |
| Track | Unreal Engine |
| Main prize category | TBD (Director decides at Gate 3, H48) |
| Modifier prize (only one can be won) | TBD (Director decides at Gate 3, H48; recorded in the dev log) |
| Theme | Director's Cut (`docs/THEME.md`) |
| Pitch | TBD: one paragraph from `GAME.md` at H84. Hockey 3v3 where every variation of the game is a "cut" and the player directs their own cut from a sentence. |
| How to play | TBD: from `GAME.md` first-play script |
| What the Designer does | TBD: sentence to validated cartridge JSON (D1, with a player-supplied key) and cached preset cuts (D0, offline) |
| What is original and what is not | TBD: from `ASSETS.md` at H84 (see below) |
| Video link | TBD |
| Dev log link (Glass Box) | TBD (must be submitted together with the entry) |
| Zip | `Hokej_Hokej.zip` (`TeamName_GameName`) |

## Tools used (text credit only)

- Unreal Engine 5.9 (build id from `status/A.md` at H84)
- Claude Code with Claude Opus, Sonnet and Fable models: planning, all C++ and tooling, the Designer prompts, the dev log builder
- Codex image generation: reference images only (`prep-reference/`, never imported into the game)
- Meshy: generated skater figure (listed as AI-generated, never as sourced)
- Blender: evaluation of generated meshes
- ffmpeg: capture and video encoding
- NVIDIA DLSS 4.5 (Unreal plugin v8.9.0, Super Resolution and DLAA; plugin built for UE 5.8, ported to 5.9 by ten one-line format-string fixes): Settings > DLSS, on by default on RTX GPUs (`docs/experiments/DLSS.md`)
- Anthropic API: the Designer's live sentence-to-cartridge call (key typed by the player; no key is bundled)

## Sourced assets

Written from `ASSETS.md` at submission time: every row with origin `sourced`, with its licence and url. Rows with origin `ai-generated` are listed separately as AI-generated
(tool named). Rows with origin `original` need no credit. Generate the starting list with:

```
node tools/gate/check-assets-ledger.mjs     # exit 0 first; then copy the sourced and ai-generated rows
```

- Sourced: TBD (copy at H84)
- AI-generated: TBD (copy at H84)

## Rule reminders

- No logos or trademarks anywhere in the game, menu, credits or video, other than the Unreal Engine logo.
- All text English; content suitable for teens (13+): no profanity, gore or real brands.
- Video: 1920x1080 or higher, 30 to 60 seconds, theme visible in the first seconds.
- The zip is under 1 GB (warning line 900 MB), named `TeamName_GameName.zip` (`Hokej_Hokej.zip`), contains the game folder, a short README for judges and nothing else.
- Late submissions are disqualified. Real deadline Mon 02:59 CET; internal target H88 (Sun 22:00 CET); nothing changes after H90 except a confirmed re-upload.
- The project was started from a blank C++ project, not a starter template; everything before tag `pre-jam` is declared in `devlog/notes.md` H0.

## Checklist: Submission candidate (H84): built and tested on a clean profile

- [ ] [tool] Packaged Windows build runs on a clean machine or clean user profile with no extra software installed (rule 7f), cold start to playing.
- [ ] [tool] Size budget: packaged zip under 1 GB, read from the `package.ps1` output; warning threshold is 900 MB.
- [ ] [human] The zip is named `TeamName_GameName.zip` and contains the game folder, a short README for judges and nothing else.
- [ ] [human] Video captured: 1920x1080 or higher, 30 to 60 seconds, shows the theme in the first seconds, no logos other than Unreal Engine.
- [ ] [human] Tools list written: engine version and build, agents, models, asset services (Meshy, Tripo, Rodin or others), each as text credit only.
- [ ] [human] Sourced assets list written from `ASSETS.md` (rows with origin `sourced`); AI-generated assets are listed as AI-generated, never as sourced.
- [ ] [auto] Ledger final: `node tools/gate/check-assets-ledger.mjs` exits 0.
- [ ] [auto] Dev log builds with screenshots embedded: `node devlog/build.mjs --out devlog/out/index.html --embed`, then the page is read once by a person for leaks.
- [ ] [human] Dev log is hosted and its link works from a logged-out browser.
- [ ] [human] Submission text drafted: pitch, how to play, what the Designer does, what is original and what is not.

## Checklist: Submission (H88): submit

- [ ] [tool] The exact zip that is uploaded is the zip that passed the H84 clean-machine test (compare size and hash).
- [ ] [tool] Zip under 1 GB, named `TeamName_GameName`, opens and extracts with a standard tool.
- [ ] [human] Video file meets the rule: 1920x1080 or higher, 30 to 60 s, plays in a normal player.
- [ ] [human] Dev log link is in the submission (Glass Box needs it submitted together with the entry).
- [ ] [human] Tools list and sourced assets list are filled in on the submission form.
- [ ] [human] Main prize category and one modifier prize chosen on the form (only one modifier can be won).
- [ ] [human] Submission confirmed on the page after sending; screenshot of the confirmation saved in `devlog/shots/`.
- [ ] [human] Late submissions are disqualified: nothing is changed after H90 except a confirmed re-upload before the deadline.

## Zip hash record (fill at H84, compare at H88)

| Zip | Size (bytes) | SHA-256 | Tested on |
|---|---|---|---|
| `Hokej_Hokej.zip` | TBD | TBD | TBD |

`Get-FileHash Hokej_Hokej.zip -Algorithm SHA256` gives the hash.


## NVIDIA track (organisers' text relayed by the operator, 2026-11-12; ruling R-0006)
Eligibility for the NVIDIA RTX Award needs at least one feature from NvRTX, NVIDIA DLSS or NVIDIA ACE in the game, plus the NVIDIA questions answered at submission. Our path: **NVIDIA DLSS**
in the shipped build (lane A11; `docs/experiments/DLSS.md` holds the answers). NvRTX is an optional trailer experiment. Partner pools depend on the "Tools you used" field: list NVIDIA DLSS,
Meshy, and Hyper3D Rodin only if the ledger shows a shipped Rodin asset (handoff A-to-B-0001).
