# B0: device B lead plan (written H4, 2026-11-12 10:30; the lead keeps it current)

Device B is the art and experiments lead (`docs/DEVICE-B.md`). This file is the lead's own plan: what the lanes are on this machine, in what order,
and the names B intends to use until `contracts/assets-naming.md` and `contracts/look-tiers.md` exist. The contracts win the moment they land.

## Where B stands at H4

- B lost H0 to H4: Windows Update restarted the machine at 02:34 and it sat in standby until 09:56, so the 06:00 task never ran (`status/B.md`).
- Seams are not on `origin/main` yet (lane A0 started about 10:00 with a 75-minute box). Until they land B does text work only.
- The look is decided (`docs/HOCKEY.md` 6, `GAME.md`): **P2 painted cel, night arena L2-b, no crowd**. The ladder starts at night, so B's first rig is
  `LR_L2_Night`, not `LR_L1_Day` as the H6 row of `docs/DEVICE-B.md` still says. Day and neon are extensions.
- Theme: Director's Cut (`docs/THEME.md`). B's part of the skin: the dark arena reads as a **sound stage** (film lights on stands and truss, a slate,
  a director's chair by the boards), the faceoff sound is a clapperboard, the scoreboard is a film counter (the counter itself is A's UI).

## This machine changes the lane plan

`docs/TRACKS.md` 5 sizes B for 128 GB and a discrete GPU. This machine has 64 GB shared between CPU and GPU, and the editor runs as x64 under
emulation on ARM64. So, until measured otherwise:

| Rule | Value on this machine |
|---|---|
| Editors at once | **1**; a second only after the first shows its memory at rest and at PIE, and only with 24 GB free |
| Builders live | up to 3 text lanes beside the one editor driver |
| Heavy jobs | one at a time: build, cook/package, Meshy import, capture |
| Every engine shell | starts with `. C:\Users\kazda\jam\ue-env.ps1` (the .NET fix; handoff B-to-A-0001) |
| Frame numbers | measured here are **not** the gate numbers (the gate is device A's 4090); B reports its own and says so |

## Order of work

| When | Lane | Brief | Editor |
|---|---|---|---|
| now, until seams | lead | this file, B1, B2, B3, B5 briefs; `status/B.md` | no |
| seams + 0 h | lead | rebase, build, editor up, `smoke.ps1` on the real project, MCP approval with the operator | yes |
| seams + 0 to 2 h | B1 | materials, both post passes, `LR_L2_Night` | yes (the one driver) |
| after B1's rig is up | B3 | puck readability kit, then event VFX and MetaSounds | yes, after B1 releases the editor |
| after B1 | B2 | room, trim, sound-stage dressing from basic shapes | yes, in turn |
| H8 at the earliest | B5 | Meshy skater T-B2 (`docs/experiments/MESHY.md` 6) | generation: no; import: yes, in turn |
| each milestone | B6 | screenshots into `devlog/shots/`, later the video | yes, in turn |

H6 (12:00) is 90 minutes away and the seams are not in: the H6 row will not be met in full. Target for the H6 push: briefs, status, and whatever of B1
exists. The realistic date for the full H6 row is H9.

## Names B will use until the contracts say otherwise

Content root: the `JamAssets` plugin, `/JamAssets/...` in the editor, `Game/Plugins/JamAssets/Content/...` on disk.

| Folder | Holds |
|---|---|
| `Rigs/` | `LR_L2_Night`, later `LR_L1_Day`, `LR_L3_Neon` (A's placeholders live here) |
| `Look/Materials/` | `M_Toon`, `M_Trim`, `M_Ice`, `PP_ToonRamp`, `PP_Outline` and every `MI_*` from `tools/skills/look-painted-cel.md` 4 |
| `Arena/` | `SM_*` dressing meshes, `BP_*` dressing groups |
| `VFX/` | `NS_puck_trail`, `NS_<event>`, their materials |
| `Audio/` | `MS_<event>`, `SFX_*` waves, sound classes and mixes |
| `Skaters/TB2/` | `SK_Skater_TB2`, its clips, `BP_SkaterVisual_TB2` |
| `Props/` | pickups (M3) |

Open naming questions for the Director, to raise as one handoff once the contracts are in and still silent on them:
1. `PP_` is not in the prefix list (`SM_ M_ MI_ T_ SFX_ NS_ MS_ BP_ LR_`); the recipe uses `PP_ToonRamp`, `PP_Outline`. Also `SK_` and `A_`/`AS_` for the skater.
2. The stencil convention (0 room and emissives, 1 figures/boards/nets, 2 ice, 3 puck) must be set by whoever spawns the mesh: A for the rink, the toy
   skater and the puck. Without it the ramp and outline do nothing.
3. Which material names A's rink builder assigns (B proposes `MI_Ice_Night`, `MI_Toon_Board`, `MI_Toon_NetFrame`, `MI_Toon_NetMesh`, `MI_Toon_Puck`).

## Waits for the operator

- `.env` with the Meshy key on this machine (it is gitignored, so it did not come with the clone): needed for B5.
- Blender 4.2 is not installed here; `tools/meshy/inspect.py` and the pelvis-drift check need it. Python has no PIL (only the strip sheets need it).
- The `unreal-mcp` approval click once the project exists.
- Picks: generated-mesh picks for B2's props and B5's figure; music source for B3.
- Pause Windows Update for the jam (it restarted both machines last night).
