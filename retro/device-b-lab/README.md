# Device B lab: the scripts behind B's generated and scripted assets

Copies of the tools and scripts device B wrote **during the jam** outside the repository (because `tools/**` is device A's path), kept here so the provenance of
B's assets is in git and device A can read or adopt them. Nothing here ships in the game; no generated binary is included; no key is in any file
(checked against `.env` before the commit). Sources live in `C:\Users\kazda\jam\` on device B; this folder is refreshed by the device B lead.

| Folder | What | Produces |
|---|---|---|
| `skater-s4/` | `build_skater.py`, `build_clips.py`: a Blender 4.2 script builds an articulated toy hockey skater and its clips from pose tables; `run_all.sh` rebuilds everything | track T-B1 (`SK_SkaterTB1`, `AS_SkaterTB1_*`), `original` |
| `skater-s5/` | `retarget.py`: drives the Hyper3D Rodin mesh (rigged in Meshy) with the scripted clips; adds a root, an ankle, the stick | track T-B2 "the hybrid" (`SK_SkaterTB2`, `AS_SkaterTB2_*`) |
| `skater-s2/` | `make_engine_clip.py`: strips pelvis travel, trims, cross-fades loops on Meshy text-to-motion clips | superseded by the hybrid; kept for the method |
| `skater-s6/` | `clean_texture.py`: removes lace-like marks from the Rodin skater texture inside the lower-leg UV region | `T_SkaterTB2_BaseColor` |
| `rodin-tools/` | `rodin.mjs`: zero-dependency client for the Hyper3D Rodin API | the skater mesh and seven set props |
| `eleven-tools/` | `eleven.mjs`: zero-dependency client for ElevenLabs (speech, voice design, sound effects, music) | the audio lab takes (not imported yet) |
| `visual-tests/` | editor Python for the visual-upgrade tests of `briefs/B7-visual-upgrade-plan.md` (the `FIXED` files are the ones that ran) | test results in `status/B1b-report.md`, `status/B2c-report.md` |
| `apply_import_rules.py` | applies device A's texture size rules (A-to-B-0004) to every texture under `/JamAssets/` | run after each bulk import |
| `ue-env.ps1` | the .NET fix for Unreal's batch files on Windows ARM64 (now also in `tools/run`) | |
| `audio-lab/table.md` | measurements of the generated audio takes | |

Known when reading the scripts: Unreal 5.9's FBX importer rejects files rewritten by `strip_tracks.py` (the step is disabled in both `run_all.sh`); packaging and
builds must not overlap machine-wide (`briefs/B-machine-notes.md` rule 11).
