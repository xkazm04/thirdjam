# AR9 report: the board caps' usage flag on `M_Toon` (device B, GPU, main tree)

Lane AR9, device B, machine clock 04:59 to 05:17 (2026-11-14). Main tree `C:\Users\kazda\jam\thirdjam`, `dev-b` at dfabb6be, port 8002. Two GPU editor processes of mine: pid 28964 (reference, the flag, the save; exit 05:04:54) and pid 40468 (the fresh process for the proof; exit 05:14:19). Both left through `ExitEditor`, both confirmed gone, `LogExit: Exiting.` in both logs. No commit, no push, no cook. Console variables read back restored. My renderer-less editor in `wt\b-arena` was already closed; nothing of job 2 is in this.

## Verdict

**Done. One file changed: `Game/Plugins/JamAssets/Content/Look/M_Toon.uasset`, 67,992 → 68,056 bytes.** The usage flag for instanced static meshes is set on the master; its other flags (static mesh, skeletal mesh) were set before and are unchanged; the graph is untouched (308 vertex / 549 pixel instructions before and after). When I saved, `M_Toon` was the only package the editor had marked; no instance was marked and none was saved.

**One thing you did not ask for and must know: the picture changes at the caps, in the editor too.** Before the flag the caps drew as dark brown lumps in the editor (mean colour 64, 41, 25): the team colours never showed. After it they are orange and blue domes along the board tops (0.66 % of a close frame; 0.34 % of the side view). That is what lane A145 built and nobody had seen. Everything else is equal within noise. If bright team-coloured caps on the boards are not wanted in the operator's package, the lever is the other device's (`RinkActor.cpp`, the cap's team material), not this flag.

## 1. Before

A clay play session in my own log (`Game/logs/editor-20261114-045938-0.log`): exactly the two lines, `MI_Toon_Blue` and `MI_Toon_Orange` "missing usage flag InstancedStaticMeshes! Default Material will be used in game." Reference stills, frozen at 2.3 s: the far side boards from 1.9 m at the centre line (caps of both halves, boards, crowd), the side boards from 3.2 m with both teams, and the game camera.

## 2. The change

`MaterialEditingLibrary.set_material_usage(M_Toon, MATUSAGE_INSTANCED_STATIC_MESHES)`, recompile, save `M_Toon` only. 0 material errors in the log.

## 3. After, in a fresh editor process (`editor-20261114-050650-0.log`)

| | Count |
|---|---:|
| Play sessions | 3 (clay, night, clay) |
| Lines with "missing usage flag" (any material, any usage) | **0** |
| Lines with "Default Material" | **0** |
| Material errors, asserts, GPU errors | 0 |

The same frozen stills, before against after, and after against a second unchanged session (my noise: two play sessions do not freeze in the same game state, so the figures differ):

| View | Mean absolute difference, before / after | The same between two sessions after (noise) |
|---|---:|---:|
| Close at the caps (no figures in frame), everything but the caps | **0.362** | 0.363 |
| Close at the caps, the caps themselves | (64, 41, 25) → (144, 88, 62), 0.66 % of the frame | stable (99.9th percentile of the difference 4) |
| Side boards with both teams | 3.33 | 4.74 |
| Game camera | 11.2 | 14.7 |

- **The boards, the crowd, the benches and the ice are pixel-equal within noise** (0.362 against 0.363 in the view without figures).
- **The figures:** in the two views that hold them the before/after difference is smaller than the difference between two unchanged sessions, so I can say no more than "inside the noise"; a same-state pair was not possible across an editor restart.
- **Night:** one session, one still: the caps are orange and blue there too, with the outline; 0 warning lines.

## 4. Anything else on that line

In the fresh process's log: no "usage flag" line of any kind. Last night's GPU logs also carried `missing usage flag SkeletalMesh` for `MI_Toon_Stick`, `MI_SkaterTB1_JerseyDeco` and once for `M_Toon` itself, and `InstancedStaticMeshes` for `MI_Toon_KickPlate` (4 lines); none of them appeared in my three sessions. `MI_Toon_KickPlate` is an instance of `M_Toon`, so the flag covers it. The skeletal ones are the figure lane's assets: not touched; if they return, they are theirs to look at.

## Not verified

- **The cook** (the added permutation compiles there) and the caps in a packaged game.
- Other look tiers than clay and night; other rink sizes.
- The figures against a same-state reference (see above).
- Whether the operator wants the caps in team colours: they are new to the eye.

## Files

- `Game/Plugins/JamAssets/Content/Look/M_Toon.uasset`: 67,992 → 68,056 bytes (a copy of the file before is in `scratchpad\ar2\bk9\M_Toon.before.uasset`).
- `devlog/shots/20261114-051354-B-ar9-caps-before-after.png` (+ `.txt`)
- `devlog/shots/20261114-051354-B-ar9-clay-before-after.png` (+ `.txt`)
- `devlog/shots/20261114-051354-B-ar9-night.png` (+ `.txt`)
- `status/AR9-report.md`

(`status/EYE-report.md` was modified in the tree before I started; it is not mine.)

## Ledger row (11 columns; replaces the row of the same path)

```
| Game/Plugins/JamAssets/Content/Look/M_Toon.uasset | material | 0.07 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-14 | B | y | master material (painted-cel look) + Construct (B13), clay pass 2 / broadcast mode (B14), CelConstructGate (B15), B19ToonBase: clay-only base override and painted board panels with joints (B19), B34BoardWear behind the static switch BoardWear (default off, so no other instance compiles it): clay-only dirt band, rubber puck smears, stick scuffs and plaster tint on the boards (B36); since AR9 the usage flag for instanced static meshes is set (the rink actor draws its board caps as instanced components on MI_Toon_Blue and MI_Toon_Orange; without the flag a cooked game gave them the default material); nothing else in the graph changed (lane AR9) (68056 bytes) |
```
