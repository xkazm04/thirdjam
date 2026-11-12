# Device B lane board (the lead rewrites this file whenever a lane starts or reports)

Updated 2026-11-12 14:50 CET (H8.8). Two editors by the operator's decision; lanes without an editor are pushed (operator, 14:10: "push lanes requiring no editor and
track in the github so Device A is informed and can sync"). Reports land in `status/`; briefs in `briefs/`; lab scripts in `retro/device-b-lab/`.

## Running

| Lane | What | Editor | Started | Due | Report (when done) |
|---|---|---|---|---|---|
| B9 | P3 clay tier, pass 1: `MPC_Look`, `BP_LookParams`, `LR_P3_Clay`; also adds `GoalLineOffsetCm` to `M_Ice` (R-0016) | port 8000, main tree | 14:05 | 16:30 | `status/B9-report.md` |
| B10 | Zone strips `MI_Zone_<kind>`, `MI_Toon_Bumper` (R-0019), the pickup base and idle look | port 8002, `wt\b-vfx` | 14:30 | 16:00 | `status/B10-report.md` |
| COOK | Cook-only check of the whole project on device B (do B's assets cook), from `wt\b-skater` | none (CPU) | 14:30 | about 15:10 | `status/B.md` |

## Waiting for the operator's ears or eyes (pages on device B)

| Page | What to decide |
|---|---|
| `C:\Users\kazda\jam\audio-lab\ready\picks.html` | the picked effects and music in import-ready form: two boosted takes (clapperboard 05, puck post 01) and three loops to confirm |
| `C:\Users\kazda\jam\audio-lab\vox\listen.html` | commentary in voice C1: delivery (calm / house / hyped) for six key lines; keep or redo for the first 40 lines |
| `C:\Users\kazda\jam\image-lab\pick.html` | Codex images: backdrop flat, two crests, number sheet, five pickup glyphs, three zone tiles (one per group) |

## Queued (needs an editor slot)

| Lane | What | Waits for |
|---|---|---|
| P2a | Dressing moved in for the 90 x 45 rink, truss over the new pools (the rig scale is accepted: R-0016 point 5) | an editor slot (after B10) |
| P2b | Atmosphere pass applied: backdrop flats, haze, faded shafts (tested in `status/B2c-report.md`) | the operator's veto window (21:00) |
| AUD | Import of the operator's picks (`status/B-audio-picks.md`) as `SFX_*` / `MUS_*`; MetaSounds layered or swapped; the rebuilt `MS_faceoff` | the operator's confirmation on the ready page; an editor slot |
| SKB | Import of the 15 stick-verb clips into both skater visuals; stick aim by a blend of four aim poses | device A's answer to B-to-A-0014 (interface calls; `game.stick.reach_m` is 1.6 m in R-0018 against 1.0 m poses) |
| PRP | Import of the batch-2 props: six pickup props, bench, penalty box, scoreboard housing, goal light, crate, coach's stool | lane B10 (the pickup base) and an editor slot |
| IMG | Import of the picked Codex images (backdrop flats, crests, number sheet, glyphs, zone tiles) | the operator's picks |
| VOX2 | The remaining 300 commentator lines in voice C1 (about 3,250 credits) | the operator's delivery choice |

## Done today (newest first; every one has a report under `status/`)

N1 Codex images batch 1 · S5 hybrid with 31 takes · VOX commentary first batch · N3 audio ready-making · N6 Rodin props batch 2 · N5 stick-verb clips ·
N2 commentary script · N4 dev log notes · B3b effects and sounds for the new events · B8 texture rules and skater tweaks · B1c GPU-hang hardening and 90 x 45 pool re-lay ·
AU1 audio lab · B6s skater polish and real-match preview · B2c stage light and atmosphere test · B4 scripted skater as T-B1 and the hybrid as T-B2 · S6 texture cleanup ·
S4 scripted original skater · B2b Rodin props in the arena · B1b look fixes on the real rink · V1 visual-upgrade plan · S3 Rodin route · S2 motion clips ·
S1 figure styles · B5 Meshy skater path · B3 puck kit, effects, sounds · B2 dressing pass 1 · B1 painted-cel look and night rig.

## What device A can sync on

- Assets: everything under `Game/Plugins/JamAssets/Content/` on `dev-b`, ledgered in `ASSETS.md` (163 rows; art 17.3 MB, audio 3.9 MB, vfx/other 7.6 MB).
- Open requests to A: `handoff/B-to-A-0012` (the `Variant` input for event kinds), `0013` (include `devlog/notes-b.md` in the build), `0014` (interface calls for the
  stick verbs; reach values). Answered by A in R-0016: `0007` to `0011`. Answered by B: `A-to-B-0001` done, `0002` accepted (first images generated, picks pending),
  `0003` done on B's side (A's one-editor soak at 14:30), `0004` done (needs `bFinalUsesRDO=True` in A's config), `0005` parked by R-0020, `0006` accepted.
- Known on B: a Development **package does not complete on this machine**: `link.exe` for the game executable hangs under x64 emulation (twice, 16 and 22 minutes
  idle). Device B checks its assets with a cook-only run instead; packaging stays device A's.
