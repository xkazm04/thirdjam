# Lane S4 report: the shot's streak seen from the game camera at play distance (device B), 2026-11-15

Tree `C:\Users\kazda\jam\wt\b-aud` (branch `lane/b-aud`, at dev-b 10d2e337). I rebuilt it first (ThirdJamEditor Development: pass, 108.6 s). Port 8006.

**Housekeeping**
- GPU editor sessions:
  - pid 34372, 10:59 to 11:33. GPU-FREE.txt was taken ("free from E3 at 01:04") and written back "free from S4 at 11:33".
  - pid 35968, 11:39 to 11:45. Taken after P2's release ("free from P2 at 11:39") and written back "free from S4 at 11:45".
  - Both editors were left through `ExitEditor` and both processes are gone. I never touched P2's editor.
- Settings are as I found them:
  - `enable_game_sound` false (that is what it was at the start);
  - `jam.ScriptHuman` 0;
  - `r.Editor.Viewport.OverridePIEScreenPercentage` 1;
  - `jam.Cut hockey.regular`.
- No dirty package at exit. The temporary folder `/JamAssets/VFX/_S4tmp` (the committed trail rebuilt there, used for the old frames on neon and for the constant-speed run) is deleted and is not on disk.
- `git status`: only `M_Fx_PuckTrail.uasset` and `NS_puck_trail.uasset` are modified, plus the new stills. No commit, push, cook or paid service.
- Backups of both assets as committed: scratch `s4/bk/`.
- Scripts are in scratch `s4/`: `s4rig.py` (rig), `s4trail.mjs` (emitter), `mkstreak.py` (material), `meas.py` / `carry.py` (measures), `sheet.py` (strips).

## 1. Done / not done

| Item | Verdict |
|---|---|
| Hard shot seen from A's game camera at 20 / 30 / 35 m, clay | **done, filmed on the GPU, old beside new, measured** |
| Same on neon (l3_neon) | **done, filmed and measured** (the old streak already read on neon; the new one is now at least 6 px there too) |
| Passes at 14 and 23.3 m/s: a thinner streak | **done, filmed on both tiers** |
| Nothing under 10 m/s, nothing on a carried puck | **done** (carried at a boosted 16.5 m/s: no streak, measured) |
| Full opacity for the first 8 frames, then the fade | **built** (the emitter's alpha); not measured separately |
| A real match shot with the sim's puck | **not done**: a stand-in puck on the game camera's geometry (section 3) |

## 2. What changed

**`NS_puck_trail`** (emitter `Trail`; one emitter, CPU, ribbon, no light; 338,883 -> 348,614 bytes):
- **Speed**: still max(User.Speed, owner velocity). It now rises within one frame (time constant 0.016 s) and falls over 0.1 s, so the streak is there on the release frame. Teleport guard 90 m/s, as before.
- **New emitter value: the shot latch `Emitter.ShotAge`.**
  - It starts when the speed passes 30 m/s and counts seconds.
  - It clears under 15 m/s or when `User.Carried` is 1.
  - Why: A207's shot fades to 27.7 m/s by 30 m from the camera. A plain "over 30 m/s" rule would have shrunk the streak exactly where the critic looks.
- **Spawn**: 240 a second (was 60), so the ribbon's head stays within about 21 cm of the puck at 51.6 m/s. At most 36 live particles (0.15 s life).
- **Floor**: 10 m/s when `User.Carried` is 0 (A sends it now). It stays 12 m/s while `User.Carried` is -1 (unknown). Off at `User.Carried` 1.
- **Ribbon geometry**: a constant 44 cm (screen-facing, as before, at the trail's attach point 3.5 cm above the ice). The material draws the visible streak inside it.
- **Width factor** (particle colour R, set at spawn):
  - shot (latched): 0.8 to 1.0 (full from 30 m/s);
  - pass from 20 to 30 m/s: 0.55 to 0.8;
  - from 10 to 20 m/s: 0.4 to 0.55.
- **Opacity** (particle alpha), as an expression:
  `vis x (shot ? lerp(1.0, 0.8, (ShotAge - 0.1333) / 0.35) : lerp(0.55, 0.8, (speed - 12) / 18)) x (1 - age^2)`
  - `vis` = off when carried, 0 to 1 from the floor to the floor + 3 m/s.
  - So a shot is at 1.0 for its first 8 frames, then eases to 0.8.
- **Length** unchanged: life 0.15 s, so the streak is speed x 0.15 s long (7.7 m at 51.6 m/s).
- `User.Colour` is no longer read: the streak is neutral white on every tier, with no team colour. `User.Speed` and `User.Carried` stay.

**`M_Fx_PuckTrail`** (rebuilt; translucent, unlit, two-sided, no texture; 15,702 -> 21,931 bytes):
- **White core**:
  - width = width factor x taper x max(12 cm, distance to the camera x 0.0072);
  - so at 1280 x 720, FOV 70 it never gets narrower than about 6.6 px, and near the camera it is 12 cm;
  - taper = (1 - U)^0.8 along the ribbon, so it is widest at the puck and 0 at the tail.
- **Dark rim**: 0.0018 x distance (about 1.6 px) on each side, colour (0.025, 0.03, 0.04) at 0.6. This is what makes white read on the near-white clay ice.
- **Core brightness**: 8.0 on clay, 3.0 off clay (MPC_Look.Clay). The clay look renders 1.6 as a grey darker than the ice (measured 177 against ice 205), so it needs the high value. Neon is lower so it does not glow.
- Antialiased across the width with fwidth. Depth-tested; it never draws over a figure in front of it.

## 3. The proof (GPU, PIE)

**Rig** (`s4rig.py`):
- **Camera**: the human's `FollowCamera` with its tick off, pinned at (-30, 10, 3.5) m, pitch -25 deg, yaw 0, FOV 70. Checked with `project_world_to_screen`: a point 30 m out lands at (659.0, 56.4), exactly the analytic projection.
- **Stand-in puck**: two PIE copies of static actors given the puck's mesh and disc. The real puck is hidden.
- **Trail**: a spawned `NS_puck_trail` that moves with the stand-in, carried (`User.Carried` 1) at 9 m/s, then released 6.5 m ahead of the camera (`User.Carried` 0).
- **Speeds**:
  - A207's shot: 51.6 m/s for 8 m, then 25.8 + 25.8 x e^(-(x-8)/6);
  - passes at 14 and 23.3 m/s heading 35 deg right.
- **Capture**: HighResShot 1280x720 at set frames after the release and when the puck reaches 20 / 30 / 35 m from the camera. slomo 0.05 gives exact timing; one run at real time matches (below).
- **Old frames**:
  - on clay: the committed assets, filmed before any edit;
  - on neon and for the constant-speed run: a temporary copy rebuilt with V3's exact expressions and material script. Its clay frame matches the original old frame.
- **Measure** (`meas.py`):
  - The still minus the same run filmed with no trail, in luminance, sampled across the streak at 0.25 px steps.
  - "Core" = px brighter than the background by 20+ levels. "Total" adds the dark rim (px off by 20+ either way).
  - Visible length = how far back along the path a changed pixel continues.

**Numbers, A207 shot**: core / total px, 1 m behind the puck. Length in m on the ice (px on screen).

| Tier | Distance (speed) | Old | New |
|---|---|---|---|
| clay | 20 m (36.1 m/s) | 0 / 0 px; 0.85 m (7 px) | **6.0 / 8.0 px**; 6.0 m (69 px) |
| clay | 30 m (27.7 m/s) | 0 / 0 px; 0.95 m (4 px) | **6.2 / 7.2 px**; 4.2 m (19 px) |
| clay | 35 m (26.6 m/s) | 0 / 0 px; 1.15 m (3.5 px) | **6.2 / 8.0 px**; 4.1 m (13 px) |
| clay | 8 frames (13.4 m) | 0 / 0 px | 8.0 / 10.0 px; 6.65 m (206 px) |
| neon | 20 m | 9.0 px; 6.5 m | 7.0 px; 6.35 m |
| neon | 30 m | 4.2 px; 4.6 m | **7.2 px**; 4.4 m |
| neon | 35 m | 4.2 px; 4.4 m | **6.2 px**; 4.4 m |

- **Constant 51.6 m/s** (clay): new core 7.0 / 7.2 / 7.0 px at 20 / 30 / 35 m, length 7.2 / 7.2 / 7.6 m (91 / 36 / 28 px). Old: 0 px.
- **Passes** (clay): new core 1 m behind, 14 m/s 0 to 5 px, 23.3 m/s 5.0 to 6.5 px. Old: 0 px.
- **Carried** at 16.5 m/s (`User.Carried` 1): no streak.
  - clay: brightest pixel 0.35 to 1.2 m behind the puck 160.7, against 152.0 one metre aside;
  - for comparison, the new shot at 2 frames gives 235.7 on its path.
- **Real time** (slomo 1), clay shot at 30 m: the same white streak, reaching the puck.

**Stills** (each with a `.txt` caption):
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-shot-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-shot-neon-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-shot51-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-pass-clay-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-pass-neon-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-d30-clay-full-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-d30-neon-full-ab.png`
- `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-115000-B-S4-carry-realtime.png`

**What I looked at** with the Read tool: the shot sheets for clay and neon, the constant-speed sheet, the clay pass sheet (23.3 m/s), the neon pass sheet, the full clay 30 m frame, the carry and real-time crop, and zooms of the old 30 m frame.
- **Clay, old**: what the critic saw. At 30 m the puck is a dark dot about 6 x 3 px on its pale disc, with a faint greenish line about 2 px wide and 10 px long that does not separate from the ice.
- **Clay, new**: a crisp white wedge with thin dark edges, starting at the puck and as wide as the puck at its base. It reads at 20, 30 and 35 m. Near the release it is a long white needle.
- **In the full 1280 x 720 frame at 30 m** it is still small: a white tick of about 7 x 19 px under the puck. It is the brightest thing on the ice there, but it is small, because at 30 m the game camera sees the 4 m streak almost end-on.
- **Neon**: the old cream band already showed on the black floor. The new one is a brighter, tapered white line of similar length, at least 6 px wide at 30 and 35 m. No glow cloud.
- **Passes**: thin white lines, thinner for the 14 m/s tap.
- **Calm**: nothing is drawn over a figure (the ribbon is depth-tested at ice level). No glow, no team colour, no text. It never goes under the ice and never z-fights it (the ribbon lies 3.5 cm up).

## 4. Not verified
- A real match shot with the sim's puck and A's real camera rig (which moves). The stand-in uses the same camera geometry the brief gives.
- The 8-frame full-opacity window is not measured on its own.
- The cook (both assets were already in the cook; the material now uses a Custom node with fwidth).
- Scalability tiers below the editor default.
- Night (`l2_night`) tier: the white is 3.0 off clay, as on neon, but not filmed.

## 5. For device A
- **Nothing is required**: the streak reads the puck's velocity, and A already sends `User.Carried` (`AJamPuck::SetTrailCarried`).
- `User.Colour` is ignored now. If A ever wants a tint, say so.
- If a shot can stay above 15 m/s after a save or block, it keeps the shot's width until it drops under 15 or is carried. Send `User.Carried` 1 on a catch, as A does today.

## 6. Decisions for the operator
1. At 30 m from the game camera the streak is a small white tick (7 x 19 px). It cannot be long on screen, because the camera sees it almost end-on. Wider would mean more than about 7 px. Keep, or widen the minimum (`MinRad` 0.0072 on `M_Fx_PuckTrail`; 0.01 gives about 9 px)?
2. The dark rim on clay: keep it (it is what separates white from white ice), or lighten it (`RimOpacity` 0.6)?

## 7. Next three steps
1. The lead's cook-only run on this tree before pushing dev-b (`M_Fx_PuckTrail` recompiled with a Custom node).
2. One still from a real match shot (A's camera, the sim's puck) from a package build, to confirm the stand-in.
3. Film the night tier.

## 8. Ledger rows (both **replace** their rows)

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.33 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-15 | B | y | puck streak by speed (one CPU ribbon emitter, no light): speed max(User.Speed, owner velocity), rising within 0.016 s and falling over 0.1 s (held on a near-zero time step or above 90 m/s); shot latch Emitter.ShotAge from 30 m/s until under 15 m/s or carried; spawn 240/s over the floor (10 m/s with User.Carried 0, 12 m/s at -1, off at 1); life 0.15 s so length = speed x 0.15 s; constant 44 cm screen-facing ribbon with particle colour R = width factor (shot 0.8 to 1.0, pass 20 to 30 m/s 0.55 to 0.8, 10 to 20 m/s 0.4 to 0.55) and alpha = floor ramp x (shot: 1.0 for 8 frames then 0.8; pass 0.55 to 0.8 by speed) x (1 - age^2); material M_Fx_PuckTrail; User.Speed (m/s, default 0), User.Carried (default -1); User.Colour no longer read (lanes B3, B18, B37, V3, S4) (348614 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckTrail.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | translucent unlit two-sided streak for the puck ribbon: inside the 44 cm ribbon a white core of width factor x (1 - U)^0.8 x max(12 cm, camera distance x 0.0072) (never under about 6.6 px at 1280x720 FOV 70) with a dark rim (0.025, 0.03, 0.04) at 0.6 of 0.0018 x distance each side, antialiased with fwidth; core brightness 8.0 on clay, 3.0 off clay (MPC_Look.Clay); opacity = particle alpha; no texture, no team colour (lanes B37, V3, S4) (21931 bytes) |
