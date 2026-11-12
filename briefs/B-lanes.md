# Device B lane board (the lead rewrites this file whenever a lane starts or reports)

Updated 2026-11-12 15:18 CET. Two editors at most by the operator's decision. The operator's direction at 14:55: **"keep hyped v3 as overall rule"** for the
commentary, and **"keep focusing on Blender and P2, P3, P4 skater development"**. Reports land in `status/`; briefs in `briefs/`; lab scripts in `retro/device-b-lab/`.

## Running

| Lane | What | Editor | Started | Due | Report (when done) |
|---|---|---|---|---|---|
| SKF | Scripted skater family in Blender: one builder, three styles on the same skeleton (`p2_cel`, `p3_clay`, `p4_broadcast`), 31 clips each, stick reach 1.0 / 1.6 m (R-0018) | none | 14:58 | 17:30 | `status/B-skater-skf.md` |
| SKH | Generated-mesh skaters for clay and broadcast: retarget made generic, Meshy lean athlete and new Rodin meshes for P4, clay Rodin meshes and a clay surface on the P2 hybrid for P3 | none | 14:58 | 17:30 | `status/B-skater-skh.md` |
| SKG | The goalie in Blender: a kit of rigid pieces on the skater skeleton (leg pads, blocker, catching glove, chest shell, mask, goalie stick) in three styles, and 15 goalie clips (stance, shuffles, butterfly, saves, poke, freeze) | none | 15:25 | 17:55 | `status/B-skater-skg.md` |
| B12 | The operator's twelve picked images imported; jersey numbers (`SetNumber`) and team crests on both skater visuals; glyphs on pickup tokens; zone tiles tried; backdrop flat material | port 8000, main tree | 14:58 | 17:15 | `status/B12-report.md` |

## Waiting for the operator's ears or eyes (pages on device B)

| Page | What to decide |
|---|---|
| `C:\Users\kazda\jam\audio-lab\ready\picks.html` | the picked effects and music in import-ready form: two boosted takes (clapperboard 05, puck post 01) and three loops to confirm |
| `C:Userskazdajamaudio-labox2listen.html` | the commentator bank (340 lines, voice C1): the first section asks whether the bank's delivery (normal capitals, speed 1.15) is right or the bank is regenerated as the picked v3 exactly; then spot-check, untick to reject |
| no page yet | no pick was made for the director's voice, the goal horn, the room tone and the menu theme |

## Queued (needs an editor slot)

| Lane | What | Waits for |
|---|---|---|
| SKI | Import of the best P3 and P4 skaters from lanes SKF and SKH as visuals behind `IJamSkaterVisual`; the 15 stick-verb clips into the P2 visuals | lanes SKF and SKH; device A's answer to B-to-A-0014 for the calls |
| B9b | Clay tier pass 2: stronger clay at game distance, the skaters in clay, readability stills, focus that follows the player | `main` compiling on B (handoff 0020); the editor on port 8002 |
| RINK | Stills of the re-laid rig, the pickups and both skaters on the real rink at 90 x 45 and on the large cut | `main` compiling on B (handoff 0020); the editor on port 8002 |
| P2b | Atmosphere pass applied: backdrop flats (the picked hills image), haze, faded shafts (tested in `status/B2c-report.md`) | the operator's veto window (21:00) |
| AUD | Import of the operator's picks (`status/B-audio-picks.md`) as `SFX_*` / `MUS_*`; MetaSounds layered or swapped; the rebuilt `MS_faceoff`; the commentary bank as sound waves with its manifest | the operator's confirmation on the ready page; lane VOX2; an editor slot |

## Done today (newest first; every one has a report under `status/`)

VOX2 commentator bank: 340 lines, 607 s, 55.6 MB of WAV, two lines over 2.5 s accepted (`status/B-audio-vox2.md`, in git when lane B12 frees the main tree) · B11 night rig re-laid for 90 x 45, six arena pieces and six pickup props (handoff 0019) · B9 clay tier pass 1 (handoff 0018) · B10 zone strips, pickup look, blade spray (handoff 0016) ·
COOK cook-only check: 540 packages, 0 errors (handoff 0015) · N1 Codex images batch 1 · S5 hybrid with 31 takes · VOX commentary first batch · N3 audio ready-making ·
N6 Rodin props batch 2 · N5 stick-verb clips · N2 commentary script · N4 dev log notes · B3b effects and sounds for the new events · B8 texture rules and skater tweaks ·
B1c GPU-hang hardening and 90 x 45 pool re-lay · AU1 audio lab · B6s skater polish and real-match preview · B2c stage light and atmosphere test ·
B4 scripted skater as T-B1 and the hybrid as T-B2 · S6 texture cleanup · S4 scripted original skater · B2b Rodin props in the arena · B1b look fixes on the real rink ·
V1 visual-upgrade plan · S3 Rodin route · S2 motion clips · S1 figure styles · B5 Meshy skater path · B3 puck kit, effects, sounds · B2 dressing pass 1 ·
B1 painted-cel look and night rig.

## What device A can sync on

- Assets: everything under `Game/Plugins/JamAssets/Content/` on `dev-b`, ledgered in `ASSETS.md` (224 rows; art 29.1 MB, audio 4.1 MB, vfx/other 8.4 MB).
- Open requests to A: `handoff/B-to-A-0012` (the `Variant` input), `0013` (dev log notes), `0014` (interface calls for the stick verbs), `0016` (zones, pickups,
  obstacles), `0018` (tier value `p3_clay`), `0019` (pickup props to attach; rig check on the real rink), **`0020` (`main` 5760168 still does not compile from a clean checkout: uncommitted headers and
  unity-build clashes; B's build at 15:08 failed after 56 s)**. `0015` (cook list) is fixed in `main` 5760168; `0017` is answered in part and continued as `0020`.
- Answered by B: `A-to-B-0001` done, `0002` accepted (images generated, the operator's picks in `status/B-image-picks.md`, import in lane B12), `0003` done,
  `0004` done (needs `bFinalUsesRDO=True` in A's config), `0005` parked by R-0020, `0006` accepted.
- Device B cannot package (the linker hangs under x64 emulation); it checks its assets with a cook-only run. Packaging stays device A's.
