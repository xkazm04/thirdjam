# B1c report: the L2 night rig and the GPU hang (handoff A-to-B-0003), device B

Lane B1c, device B, 2026-11-12, 13:00 to 13:40. No git command was run. Build `ThirdJamEditor` passed in 36 s on the merged tree (717335d + dev-b). I drove **the only editor on the machine** (port 8000, pid 3464) through `tools/mcp/mcp.mjs`.

At the end:
- The editor is open on `/Game/Maps/Rink`, PIE is stopped, and there are no dirty packages.
- `Rink` was never saved, and nothing under `/Game/` was written.
- `jam.Cut` is back to `hockey.regular`, and `jam.ScriptHuman` is back to 0.

How I measured:
- Viewport realtime was off, and the PIE window was floating at 1920x1080.
- `jam.ScriptHuman 1` ran the human skater, so all 6 skaters were bots.
- `GetFrameTimings` was sampled for the whole run (about 540 samples per 120 s, first 5 s dropped).
- At the end of each run I took one `TriggerGPUProfile`.
- `nvidia-smi` was read at the start and end of each run.
- For the log greps I read the log file itself (`Game/logs/editor-20261112-130124-0.log`).
- **These are device B numbers** (RTX Spark N1X, x64 editor under emulation). The 8 ms gate is device A's.

## Verdict

**I did not reproduce the hang.** None of 7 runs (11.5 min of PIE with one editor, 4 mid-PIE rig stream-ins) logged `GPU crash`, `DEVICE_HUNG`, `Device Removed`, `DXGI_ERROR` or `TerminateOnGPUCrash`. None logged a virtual-shadow-map warning (pool, overflow), and the VSM stats show `OVERFLOW_FLAGS` 0 in every frame.

The rig is hardened and re-laid for the 90 x 45 m default rink, and saved.

## Which cut actually loads

`JAMCART loaded .../hockey.regular.cartridge.json id=hockey-regular ... rink=90x45 visual=toy tier=l2_night`, then `JAMLOOK tier=l2_night map=/JamAssets/Rigs/LR_L2_Night.LR_L2_Night`. The default is 90 x 45 (R-0010).

The cut-load command is `jam.playcut <name>` (`JamDesignerConsole.cpp:28`). The `jam.Cut` cvar only applies at the next PIE start. Through MCP it has to run in the PIE world: `unreal.SystemLibrary.execute_console_command(pie_world, "jam.playcut ...", pc)`. MCP's `RunConsoleCommand` runs in the editor world, which has no game instance.

## Soak tables

GPU ms comes from `GetFrameTimings`. Shadow nodes come from the end-of-run profile, PIE view plus the hidden level-viewport view (two view families).

| Run | Rig | Cut and events | Hang / device removed | VSM warnings | GPU ms median | GPU ms p99 | GPU max | frame p99 | `ShadowDepths` | VSM Nanite / non-Nanite | VSM projection | GPU memory start → end |
|---|---|---|---|---|---:|---:|---:|---:|---:|---|---:|---|
| base1 | as found | regular, 120 s | no | 0 | 11.62 | 13.13 | 15.07 | 20.51 | 1.012 | 0.803 / 0.096 | 0.206 | 6270 → 6294 MiB |
| base2 | as found | regular, 120 s, VSM stats from 30 s | no | 0 | 11.91 | 12.77 | 14.36 | 17.96 | 1.027 | 0.697 / 0.195 | 0.214 | 6294 → 6299 MiB |
| base3 | as found | regular; `jam.playcut hockey.boost` (l3_neon, rig unloads) at 20 s; `hockey.regular` (night rig streams in mid-PIE, A's case) at 45 s; `hockey.raw` (120 x 60) at 80 s | no | 0 | 12.14 | 13.12 | 13.67 | 19.01 | 0.083 (on `raw`, no VSM pass in that frame) | - | - | 6299 → 6157 MiB |
| after1 | hardened | regular, 120 s | no | 0 | 11.65 | 13.28 | 13.66 | 20.38 | 0.524 | 0.368 / 0.045 | 0.099 | 6183 → 6183 MiB |
| after2 | hardened | regular, 120 s, VSM stats from 30 s | no | 0 | 12.00 | 13.43 | 14.53 | 22.71 | 0.981 | 0.702 / 0.096 | 0.232 | 6192 → 6192 MiB |
| after3 | hardened | same cut sequence as base3 | no | 0 | 11.45 | 13.19 | 13.63 | 19.47 | 0.077 (on `raw`) | - | - | 6192 → 6049 MiB |
| large1 | hardened | `hockey.arena.large` 120 x 60, 60 s | no | 0 | 11.16 | 12.85 | 12.97 | 18.84 | - | - | - | 6192 → 6192 MiB |

Around the mid-PIE stream-in (5 s windows, median / max): base3 was 12.37 / 12.98 ms for 45-55 s; after3 was 12.03 / 13.14 ms. Neither run had a spike at the rig load.

**Matched A/B** (same PIE session, 9 profiles each, medians). I switched the PIE world back to the old spot values with the 13 shadow-casting dressing pieces:

| Node | old rig | new rig |
|---|---:|---:|
| `ShadowDepths` | 1.088 | **0.841** (-23 %) |
| `RenderVirtualShadowMaps(Nanite)` | 0.787 | 0.600 |
| `RenderVirtualShadowMaps(Non-Nanite)` | 0.107 | 0.092 |
| `InitializePhysicalPages` | 0.041 | 0.031 |

Other nodes, typical at the end of the default-cut runs:
- Lumen: `LumenSceneLighting` 0.9-1.8, `DiffuseIndirectAndAO` 5.6-8.2 (async compute).
- `LumenReflections` 0.11-0.31 (the editor view, not the rig).
- `PostProcessing` 2.3-3.7.
- Lumen GI on async compute, not shadows, is the large GPU item on this machine.

**VSM statistics** (`r.Shadow.Virtual.AccumulateStats 600`, CSV in `Game/Saved/Profiling/VSMStats(...)`, 599 frames each, median / max):

| Stat | before | after |
|---|---|---|
| pages requested per frame | 77 / 176 | 84 / 177 |
| static cached pages | 76 / 173 | 83 / 170 |
| dynamic invalidated pages | 76 / 173 | 83 / 170 |
| non-Nanite instances total / drawn | 384 / 53 | 408 / 58 |
| new pages allocated | 0 / 14 | 0 / 14 |
| overflow flags | 0 / 0 | 0 / 0 |

The physical page pool is `r.Shadow.Virtual.MaxPhysicalPages` 4096, so the rig uses at most about 4 % of it.

## Shadow-load inventory (PIE world, default cut)

The four spots as found:
- Position (x, 0, 25 m) with x = -45, -15, 15, 45. Pitch -90.
- Cones 22 / 30 degrees, attenuation 40 m, 20 000 cd.
- Source radius 30 cm, soft radius 0. Contact shadow 0.
- Shadow resolution scale 1.0. Ray-traced shadows on `USE_PROJECT_SETTING`. Distance-field shadows off. Volumetric shadow off.
- Bias 0.5, slope 0.5. Movable. Lighting channel 0. Affects translucency.

The 8 `Practical_` spots cast no shadow.

Shadow-casting primitives in the PIE world: 67 before, 54 after.

| Group | Before | After | Nanite |
|---|---:|---:|---|
| `Dress_` | 13: film lights 290-297 and director chairs 298-302 | 0 | all 13 Nanite |
| `Room_` | 0 | 0 | - |
| Other `Dress_` (177, incl. the 35 Nanite props) | 0 | 0 | - |
| A's `RinkActor0` | 4: `Ice`, `Boards`, `Nets`, `Obstacles` | 4 | no |
| A's `ArenaFloor_UnderIce` | 1 | 1 | no |
| 6 skaters | 8 meshes each, 450 tris each | same | no |
| puck | 1 | 1 | no |

Casters inside each spot's cone and attenuation sphere (bounds test). Before, the spots were at ±45 / ±15:
- Spot 1: 4 (floor, ice, boards, nets).
- Spot 2: 4.
- Spot 3: 20 (+16 skater meshes).
- Spot 4: 12.
- None of the 13 casting dressing pieces was inside a cone. They sit at the set's edge, at |y| ≥ 38 m or |x| ≥ 74 m.

After (±11.25 / ±33.75):
- 4 / 13 / 29 / 29 casters. The difference is where the skaters were at that moment.
- Spot 4 also holds the puck.

So **one spot's shadow pass draws, every frame:**
- A's ice, under-floor, boards and nets as non-Nanite dynamic geometry: the procedural mesh is movable, so every requested page is a dynamic page and is re-cleared and redrawn.
- 0 to 24 skater meshes (450 tris each).
- The Nanite VSM pass. It runs because Nanite geometry exists in the scene: 0.6-0.8 ms even with nothing Nanite in a cone, and it stays when every Dress shadow is off.

About 80 pages per frame across all 4 lights.

## Changes (applied to `LR_L2_Night`, saved, read back in PIE from disk)

| Actor | Property | Old | New |
|---|---|---|---|
| `Dress_L2_Night_290`..`_297` (`SM_FilmLight`, Nanite) | `cast_shadow` | True | False |
| `Dress_L2_Night_298`..`_302` (`SM_DirectorChair`, Nanite) | `cast_shadow` | True | False |
| other 177 `Dress_`, 6 `Room_` | `cast_shadow` | False | False (checked; unchanged) |
| `Spot_L2_Night_1` | location | (-45, 0, 25) m | (-33.75, 0, 25) m |
| `Spot_L2_Night_2` | location | (-15, 0, 25) m | (-11.25, 0, 25) m |
| `Spot_L2_Night_3` | location | (15, 0, 25) m | (11.25, 0, 25) m |
| `Spot_L2_Night_4` | location | (45, 0, 25) m | (33.75, 0, 25) m |
| `Spot_L2_Night_1..4` | inner / outer cone | 22 / 30 deg | 19 / 26.5 deg |
| `Spot_L2_Night_1..4` | attenuation radius | 4000 cm | 3000 cm (the cone edge on the ice is 27.9 m from the lamp) |
| `Spot_L2_Night_1..4` | intensity | 20 000 cd | 53 600 cd (offsets the attenuation window at 25 m: 0.718 → 0.268) |
| `Spot_L2_Night_1..4` | shadow resolution scale | 1.0 | 0.5 |
| `Spot_L2_Night_1..4` | `cast_raytraced_shadow` | `USE_PROJECT_SETTING` | `DISABLED` |
| `Spot_L2_Night_1..4` | distance-field shadows, contact shadow length | False, 0 | False, 0 (set explicitly; unchanged) |

Unchanged: the PPV (still screen-space reflections), the sky, the practicals, the room, and every dressing transform. I deleted nothing.

**Is there a single setting that removes most of `ShadowDepths`?** No.
- Turning off the Dress casters took the Nanite VSM pass from about 0.79 to 0.60 ms, which is most of the 23 % saving.
- `r.Shadow.Virtual.ResolutionLodBiasLocal 1` changed nothing measurable.
- Ice and under-floor shadows off (a PIE-only test on A's actors) changed nothing measurable.
- `r.Shadow.Virtual.Enable 0` removes the VSM passes, but conventional `ShadowDepths` then costs 1.25 ms. The frame's graphics queue was about 2 ms lighter in that test (9 profiles, one session; unconfirmed).

**A trap I found:** setting `cast_shadow` on *all* Dress actors blacks out every pool, because the truss pieces `Dress_5/10/15/20` hang at 25 m right under the lamps. Pool luma fell to 48 (PIE-only test, not saved). The dressing must stay shadowless.

## Pool re-lay and brightness check

The spots are at x = ±11.25 and ±33.75 m, z 25 m, with a 26.5° outer cone.
- The top view at 26.1 px/m measures each pool at about **24.1 m** across the outer band, with a bright plateau of about 20.3 m. Before, the pools were 28.5 m and 22.6 m.
- Neighbouring outer bands meet at x = 0 and ±22.5 m.
- The end pools stop at |x| ≈ 45.8 m, at the end boards of the 90 m rink.
- Figure shadows still show under the skaters (`after-pool` still).

Pool-centre luma uses full range, `ffmpeg crop,scale=out_range=pc,format=yuvj444p,signalstats`. The camera sits at (pool x, -18, 6) m, pitch -20, yaw 90, FOV 70. The crop is 150x60 at (1100, 470).

| Pool | YAVG | YMIN-YMAX |
|---|---:|---|
| before, x 15 | 218.7 | 213-223 |
| after, x 11.25 | **218.0** | 213-224 |
| after, x 33.75 | **217.3** | 213-223 |

All three are inside the 216-218 target, within 0.7 %. The second crop (200x80 at 1010, 520): 218.4 before, 218.7 after (x 33.75). At x 11.25 that crop held a skater.

**Large and wide cuts (not fixed, for the lead):**
- On `hockey.arena.large` (120 x 60), the last 14 m at each end (x 46-60 m, with the net at 56 m) lie outside the pools.
- On `hockey.arena.wide` (140 x 70) it is 24 m per end (x 46-70 m, net at 66 m).
- The figures and nets at the ends then read only on the blue ambient ice (`IceAmbient`, luma about 40-55), with no pool.
- Stills: `b1c-large-end`, `b1c-wide-end`.
- The rig needs to scale with the rink (A's ask), or carry per-cut spot sets.

## Stills filed (`devlog/shots/`, JPEG 1920x1085 plus caption `.txt`, 20 files, 1.85 MB)

- `20261112-133443-B-b1c-before-game`
- `20261112-133444-B-b1c-before-top`, `-before-pool`
- `20261112-133444-B-b1c-after-game`
- `20261112-133445-B-b1c-after-top`, `-after-pool`, `-after-pool34`
- `20261112-133446-B-b1c-after-over`, `-large-end`, `-wide-end`

## Files changed

| File | Before | After |
|---|---:|---:|
| `Game/Plugins/JamAssets/Content/Rigs/LR_L2_Night.umap` | 451,276 | 452,071 |
| `status/B1c-report.md` | - | new |
| `handoff/A-to-B-0003.md` | open | Response written, status `done` |

Ledger: the `LR_L2_Night.umap` row keeps size class 0.5. Append "shadowless dressing, pools re-laid for 90 x 45 (lane B1c)" to the note.

## What I could not verify

- **The hang itself.** It never reproduced on device B. I have no driver 610.88, no RTX 4090, and no 4-5 editors (rule: one editor). The cause on A stays unproven.
- **4090 frame numbers and the 8 ms gate.** Device B's GPU ms (11.2-12.1 median) is a slower GPU under emulation, with a second (level viewport) view family in PIE.
- **The `r.Shadow.Virtual.Enable 0` saving.** One session of 9 profiles; not soaked.
- **Whether shadow resolution scale 0.5 lowers VSM page counts.** The page counts did not drop, so on VSM the per-light scale may do little. I kept it because it is conservative and visually neutral at this scale.
- **The base3 / after3 end profiles** were taken on `hockey.raw` and show no VSM render pass in that frame. I did not investigate further.

## What the lead and device A must decide

1. **Device A:** run the confirmation in the handoff Response (one editor, 60 s, log greps, `stat gpu`). If it hangs again with one editor on the new rig, the hang is not shadow load. Send the new `.nv-gpudmp` and the driver version, and consider a driver update.
2. **Device A, own actors:** `RinkActor.Ice` and `ArenaFloor_UnderIce` cast shadows that fall on nothing visible. Turning them off costs nothing (no measurable saving on B either). Also, the procedural ice being movable makes every VSM page dynamic.
3. **Lead / device A:** scale the rig with the rink. For 120 x 60, 4 spots at ±15 / ±45 (the old layout); for 140 x 70, 5 or 6 spots. Device A's `SetTier` could move the `Spot_` actors from the cartridge's rink length, or B can ship per-size spot groups.
4. **Lead:** the truss lamp pieces `Dress_5/10/15/20` still hang at the old spot positions (x ±15, ±45). On the default rink, the lamps no longer sit above the pools. Moving dressing was outside this lane.
