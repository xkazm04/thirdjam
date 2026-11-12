# Device B lane board (the lead rewrites this file whenever a lane starts or reports)

Updated 2026-11-12 14:35 CET (H8.6). Two editors by the operator's decision; lanes without an editor are pushed (operator, 14:10: "push lanes requiring no editor and
track in the github so Device A is informed and can sync"). Reports land in `status/`; briefs in `briefs/`; lab scripts in `retro/device-b-lab/`.

## Running

| Lane | What | Editor | Started | Due | Report (when done) |
|---|---|---|---|---|---|
| B9 | P3 clay tier, pass 1: `MPC_Look`, `BP_LookParams`, `LR_P3_Clay` (`briefs/B9-p3-clay.md`) | port 8000, main tree | 14:05 | 16:30 | `status/B9-report.md` |
| PKG | first Development package on device B (ARM64), from `wt\b-skater` at `dev-b` 47d0bb0 | none (CPU) | 14:03 | about 14:40 | `status/B.md` |
| B10 | Zone strips (`MI_Zone_<kind>`) and the pickup look (`SM_Pickup_Base`, `MI_Pickup_<kind>`, idle motion) for C-0005 | port 8002, `wt-vfx` | 14:30 | 16:00 | `status/B10-report.md` |
| N1 | Codex images, batch 1 (A-to-B-0002): backdrop flats, two invented crests, a number sheet, pickup glyphs, zone tiles; triage page for the operator | none | 14:20 | 15:30 | `status/B-images-n1.md` |
| N2 | Commentary and director script: lines per event for a commentator voice and the director's calls, with the event mapping | none | 14:20 | 15:30 | `briefs/commentary/` + `status/B-audio-n2.md` |
| N3 | Audio ready-making: loudness targets, loop cross-fades and loop points for every generated take, so a pick imports as is | none | 14:20 | 15:00 | `status/B-audio-n3.md` |
| N4 | Device B's dev log: notes per hour from the status blocks, reports and stills | none | 14:20 | 15:00 | `devlog/notes-b.md` |
| N5 | Stick-verb clips for both skater figures (R-0013): poke, lift, stumble, faceoff ready and draw, last-man crouch, pass, shot charge | none (Blender) | 14:20 | 15:15 | `status/B-skater-s4.md`, `status/B-skater-s5.md` |
| N6 | Rodin props, batch 2: film-themed pickup props, bench, penalty box, scoreboard housing | none | 14:20 | 15:00 | `status/B-props-n6.md` |

## Queued (needs an editor slot)

| Lane | What | Waits for |
|---|---|---|
| P2a | Dressing moved in for the 90 x 45 rink, truss over the new pools | device A's answer to B-to-A-0011 (rig scale) |
| P2b | Atmosphere pass applied: backdrop flats, haze, faded shafts (tested in `status/B2c-report.md`) | the operator's veto window (21:00) |
| AUD | Import of the picked audio takes as `SFX_*`; MetaSounds layered or swapped | the operator's triage of `C:\Users\kazda\jam\audio-lab\listen.html` |
| SKB | Import of the new stick-verb clips into both skater visuals; stick aim by IK | lane N5; device A's interface for stick aim, poke and lift |
| PRP | Import of batch-2 props and pickup meshes | lane N6; the operator's pick |
| MIC | Voice-signal test for A-to-B-0005 | the operator at device B's microphone |

## Done today (newest first; every one has a report under `status/`)

B3b effects and sounds for the new events (handoff 0012) · B8 texture rules and skater tweaks · B1c GPU-hang hardening and 90 x 45 pool re-lay · AU1 audio lab (ElevenLabs) · B6s skater polish and real-match preview ·
B2c stage light and atmosphere test · B4 scripted skater as T-B1 and the hybrid as T-B2 · S6 texture cleanup · S5 hybrid retarget · S4 scripted original skater ·
B2b Rodin props in the arena · B1b look fixes on the real rink · V1 visual-upgrade plan · S3 Rodin route · S2 motion clips · S1 figure styles · B5 Meshy skater path ·
B3 puck kit, effects, sounds · B2 dressing pass 1 · B1 painted-cel look and night rig.

## What device A can sync on

- Assets: everything under `Game/Plugins/JamAssets/Content/` on `dev-b`, ledgered in `ASSETS.md` (143 rows; art 17.4 MB, audio 1.8 MB).
- Open requests to A: `handoff/B-to-A-0007` (binder parameters, puck kit), `0008` (T-B1 ruling; puck at `puck_socket`), `0009` (five look defects on A's side),
  `0011` (scale the rig with the rink), `0012` (the `Variant` input for event kinds). Answered by B: `A-to-B-0001` done, `0002` accepted, `0003` done on B's side (A's one-editor check open), `0004` done
  (needs `bFinalUsesRDO=True` in A's config), `0005` accepted (needs the operator's voice), `0006` accepted (plan in `status/B.md` H7.7).
