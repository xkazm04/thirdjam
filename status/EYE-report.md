# Lane EYE report: the night's critic, the cost of the look, the operator's morning page (device B), 2026-11-14 02:10

No GPU, no editor, no Blender, no outside service. Nothing written in the repository except this report and one still with its caption
(`devlog\shots\20261114-015522-B-eye-faults.png` / `.txt`). Lab folder `C:\Users\kazda\jam\review\night\` (scripts, crops, `index.html`).
One headless Edge screenshot of the page, with `--disable-gpu`, to check it renders; the process exited and its profile folder is deleted. Nothing left running.

## 1. Summary

Evolutions 1 and 2 are done, and the first version of the morning page is built. **Evolution 1:** twenty faults a player would see, ranked. Each one is pointed at
in a named still that I looked at with the Read tool, at full size where the detail mattered. The two most visible faults hide the key feature: device A's
HUD (the selection ring, the caption and the taunt bubbles) is drawn over the downed figure, and in a real match an opponent stands on a downed player for at
least 24 frames. **Evolution 2:** a ledger of counts. JamAssets is 1,674 files and 343 MB on disk (decimal MB throughout). 183 MB of that (53 %) is figures that no shipped cut selects.
The figure Blueprint grew from 4.21 to 6.36 MB in five lanes since 21:20, all of it graph logic, and the same graph is carried twice. The worst moment is about
600 sprites. The clay rig has 18 local lights, 4 of them shadowed. I give ten cost evolutions and five things that only device A can time. **Page:**
`C:\Users\kazda\jam\review\night\index.html`, rebuilt by `node C:\Users\kazda\jam\review\night\build.mjs`, with its content in `night\data.mjs`.
**Not done:** a re-read of the night lanes' work. Only RAG had filed a report when I looked, and SET-dive, SET-dash, SET-weapons and TRAILER had filed
nothing. I heard none of the sounds. I timed nothing.

## 2. Per evolution

### Evolution 1: the critic's first pass (what a player notices in ten minutes on clay)

What I looked at: every `-B-` still from 13 Nov 21:15 on that shows the engine in play (QA4 1 to 4, first-seconds, bench-box; SK21 board fall; SK22 stops; SK23
resolution, get-up strips; SK24 stumble, stop, lying loop; B40 brake, abilities, cross-check; B41 goal clay, proposed and night; arena check; AR1 crowd render).
I also read the captions and the grouping in `C:\Users\kazda\jam\review\build.mjs`. Un-paused engine stills come first. The Blender sheets were read only to
know what is coming. Coordinates are pixels in the file at full size unless a row says "displayed".

| # | Fault | Still (full path) and where | Whose | Evolution, with proving number |
|---|---|---|---|---|
| 1 | The human's selection ring (a heavy blue ellipse about 330 px wide) is drawn over his own figure in every frame. When he is down it cuts across the body | `C:\Users\kazda\jam\thirdjam\devlog\shots\20261113-211603-B-qa4-clay-play-4.png` bottom centre (690-1040, 830-1000), across the lying #7; also `...-211559-B-qa4-clay-play-1.png` (660-975, 720-865) | device A, HUD | ring under the skates only (behind the figure, thinner), faded while down: ring pixels inside the human's silhouette **0** in 10 un-paused stills and during a knockdown |
| 2 | The commentary caption sits on the human (lower centre). In a knockdown it covers the body. At the faceoff the caption, the controls strip and the YOU tag stack on the figure | `...-211603-B-qa4-clay-play-4.png` (670-1255, 985-1025); `...-211719-B-qa4-first-seconds.png` (600-1430, 840-990) | device A, HUD | captions in the top band: caption box over the human's screen box in **0 of 200** caption frames |
| 3 | Opponents' taunt bubbles sit over the net and across the middle ice | `...-211602-B-qa4-clay-play-3.png` (750-1245, 205-258), over the net; `...-211603-B-qa4-clay-play-4.png` (620-1185, 328-383) | device A, HUD | taunts at a screen edge or over the speaker: **0** taunt boxes over the net or the puck in 100 s |
| 4 | The top-centre HUD box is an empty dark panel, and "PERIOD 1: FIRST TO 3" sits on its top border in every frame | `...-211559-B-qa4-clay-play-1.png` (845-1075, 25-105); the same in play-2 to 4 and first-seconds | device A, HUD | the text inside its panel on all three tiers |
| 5 | An opponent stands on (through) the downed human for at least 24 frames. The key moment is buried, and it reads as stomping on a man who is down | `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-004300-B-sk24-lying-loop-play.png` cells f30, f42, f54 (x 1280-3200) | device A, sim (bodies pass through a downed skater); B figure lane | keep skaters out of a downed body's footprint: frames with another skater's capsule over a downed figure **0** in two seeded 100 s matches |
| 6 | Hit chips are round salmon beads in an arc round the feet. They read as candy, not ice or clay | `...-20261114-004259-B-sk24-stumble-real-caller.png` row 1, f0 and f3 (0-1120, 40-440); `...-211602-B-qa4-clay-play-3.png` (900-1070, 520-615) | B effects (`NS_check` dot ring) | B40 job 4, chips as short ice-white streaks: **0** round dot sprites in `NS_check` |
| 7 | The four abilities look alike and barely show: white flecks or a grey smudge at the skates. The shield is gone by frame 12 of its 360 | `...-20261114-001015-B-b40-ability-kinds.png` rows 1-2 (frozen) | B effects (B42 queued) | one colour and shape per kind: each covers **at least 2 %** of the frame at +4 at game distance; a viewer names the kind from a still **4 of 4** |
| 8 | The hard stop's spray cannot be seen at game size: a dozen white flecks under the boots | `...-20261114-001015-B-b40-brake-strip.png` row 2, both tiles, at the blue skater's boots | B effects (B42) | a fan sized by the start speed: **at least 1.5 %** of the frame at +6 |
| 9 | The goal reads either as pink ice (today) or as a small blue clump at a net 30 m away (proposed). Neither is a payoff at the game camera | `...-20261114-011347-B-b41-goal-clay-proposed.png` +4 and +10 tiles; `...-011347-B-b41-goal-clay.png` +4 to +60, red on the end boards and the ice | B look and effects, with device A's light numbers | light off the ice and the burst raised above the boards in team colour: ice mean shift **0**, burst **at least 3 %** of the frame at +10 |
| 10 | The far half of the rink washes out. Pale ice and pale boards merge into one band, and far figures are 40 px blobs | `...-20261113-221556-B-sk22-stop-bot.png` every tile, upper half; `...-211559-B-qa4-clay-play-1.png` (1300-1920, 180-400) | B look, clay rig | far boards against far ice **at least 25 luma**; far figure against ice **at least 40** |
| 11 | In the corners the camera looks almost straight down, and three bodies pile into one shape. The stumble reads for about 6 frames | `...-20261114-004259-B-sk24-stumble-real-caller.png` row 2, f0-f21 (#9, #88, #7) | device A, camera (A135) | pitch capped in corners: the human's screen box overlaps no other figure in **at least 80 %** of corner frames |
| 12 | The barn is empty: bare planks behind the boards, no stands, no people | `...-20261114-011347-B-b41-clay-arena-check.png` all tiles; `...-211601-B-qa4-clay-play-2.png` top half | B arena (AR2 running) | stands in the game camera on all eight rinks, figures still **at least 27 luma** from the ice |
| 13 | The ice scratches are dense white crosshatching near the camera, busiest exactly at the figures' feet | `...-20261113-233243-B-sk23-resolution-clay.png` top right crop, around #7's skates | B look (`M_Ice` scratch layer) | half the near scratch contrast within 6 m: line contrast under the feet **at most 8 luma** |
| 14 | The penalty-box clocks are black discs, the darkest things in the clay frame, right above the play | `...-211601-B-qa4-clay-play-2.png` (650-740, 230-310), (1340-1400, 205-275); still dark in `b41-clay-arena-check` | device A pieces; B look (B41 fixed night only) | clock face **at least 60 luma** on clay |
| 15 | A white misty band runs along the board foot all round the rink, like fog or frost | `...-20261114-011347-B-b41-clay-arena-check.png` top tiles, inside the yellow kick line (y 340-380 displayed) | B look (B23 board-foot band) | band **at most 6 luma** over the ice beside it |
| 16 | The distance markers ("own 28 m", "own 12 m") sit on the far goalie and on the net | `...-211559-B-qa4-clay-play-1.png` (1535-1625, 210-235); `...-211602-B-qa4-clay-play-3.png` (970-1060, 275-295) | device A, HUD | marker hidden when its target is on screen: **0** over an on-screen net or skater |
| 17 | The cross-check mark is brown bean shapes and white stars stuck on the victim's back, over the number | `...-20261114-001016-B-b40-crosscheck-mark.png` row 2, +8 to +14 | B effects | fewer, smaller marks beside the body: the number readable in **every** frame of the mark |
| 18 | The faceoff dots are flat saturated red discs, the loudest colour on the ice, louder than the puck | `...-211559-B-qa4-clay-play-1.png` (1265-1410, 930-1010), (30-100, 452-468) | B look (inlaid markings) | dot saturation **no higher** than the zone lines' |
| 19 | A lifted puck shows as a black post standing on a lit disc. It reads as a pin, not a puck in the air | `...-20261114-001016-B-b40-crosscheck-mark.png` row 2, every tile, top centre (crop: `C:\Users\kazda\jam\review\night\crops\puck-post.png`) | device A, lifted-puck marker | a shadow dot on the ice and the puck at its height, no post: "puck in the air" read from a still |
| 20 | The first frames of the first match show the un-rigged rink: pale blue ice, black beyond the boards, washed figures (86 rendered frames) | `...-20261113-211719-B-qa4-first-seconds.png` whole frame | device A (load order; a fix, 2250116, arrived after that run) | re-shot: the **first** rendered frame of a match is clay |

Eight of these are on one sheet, cropped at full size from the sources (no new render), each tile labelled with its source:
**`C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-015522-B-eye-faults.png`**. Proving number for this evolution: **20 of 20 faults point at a named still and
a region**. 11 are device A's (HUD 6, sim 1, camera 1, pieces 1, puck marker 1, load order 1). 9 are B's (effects 5, look 4), plus the arena, which is in progress.

What I looked at and judged well: the figures at the window's resolution are not blocky any more (`sk23-resolution-clay`, right). The lying figure reads at
game size, and the limb shift shows (`sk24-lying-loop-play` f42 to f66). The stumble reads as feet-out-standing-up (`sk24-stumble-real-caller` row 1).

### Evolution 2: what the look costs (counts only; this device does not time frames)

How I measured: file sizes on disk (PowerShell and node `fs.stat`); the ledger parsed by `C:\Users\kazda\jam\review\night\ledger.mjs`; references by
`C:\Users\kazda\jam\review\night\refs.mjs`, which scans the name table of every package for `/JamAssets/...` strings and also scans C++, ini and json.
`refs.mjs` is a name scan, not the engine's reference viewer. The Blueprint's history comes from the LFS pointer sizes of each commit (`git show <c>:<path>`).
Particle counts come from the ledger notes and B30, B3 and B40. The lights come from the rig rows in the ledger and from B27, B28 and B41.

**Totals.** JamAssets: **1,674 files, 343.3 MB** (decimal MB, as everywhere in this report). Skaters 234.8, Audio 51.0, Look 18.9, Arena 15.9, VFX 15.9, Props 3.7, Rigs 1.7, UI 0.9. The ledger has 1,918
rows; 701 rows (322 MB, including device A's `Game/Content/P4` 100 MB and `Characters` 73 MB) are marked shipped `n`. 20 files on disk (1.9 MB) have no row:
AR2's stands, crowd and materials, written at 01:47 and not committed. No row is missing its file.

**The figures.** The shipped figure is `humanoid_b1` (`JamCartridge.h` line 62, the default): `BP_SkaterVisual_TB1_P3` on clay and `_TB1` on the other
tiers (`HokejGameMode.cpp` 293-298). Both tiers together are 298 files, 59.7 MB. **Never selected by a shipped cut**: the `generated_b2` family (TB2, TB2_P3,
TB2_P4, 73.4 MB), its generated meshes and takes (Gen P2C/P3C/P4D, 67.8 MB), and the P4 lab (P4E, `BP_SkaterVisual_P4Lab`, `Look/P4Lab`, 41.8 MB). That
is **about 183 MB, 53 % of JamAssets**. Only `hockey.test.visual-b2`, `hockey.test.figure-b2`, `VisualSwapTests.cpp` and the fuzz sweep `skater.visual`
reach the generated_b2 family; nothing reaches the P4 lab.

**The largest thirty assets** (MB on disk, what uses them):

| MB | File | Used by |
|---|---|---|
| 6.36 | Skaters/BP_SkaterVisual_TB1_P3 | clay figure (shipped) |
| 6.15 | Skaters/BP_SkaterVisual_TB1 | night figure (shipped) |
| 5.22 | Skaters/SK_SkaterGen_P3C | generated_b2 on clay: test cartridges only |
| 5.22 | Skaters/SK_SkaterGen_P2C | generated_b2: test cartridges only |
| 5.04 | Skaters/SK_SkaterGen_P4D | generated_b2 broadcast: test cartridges only |
| 5.00 | Skaters/SK_SkaterGen_P4E | P4 lab: not selected |
| 4.61 | Skaters/T_SkaterGen_P4E_Normal | P4 lab |
| 4.21 | Skaters/SK_SkaterTB2_P4 | old hybrid: test only |
| 3.74 | Skaters/SK_SkaterTB2 | old hybrid: test only |
| 3.67 | Audio/Music/MUS_MatchLoop_01 | match music (shipped) |
| 3.67 | Skaters/T_SkaterGen_P4E_BaseColor | P4 lab |
| 3.55 | Skaters/SK_SkaterTB2_P3 | old hybrid: test only |
| 3.03 | Skaters/T_SkaterTB2_P4_BaseColor | test only |
| 2.99 | Skaters/T_SkaterTB2_Clay_BaseColor | test only |
| 2.96 | Skaters/T_SkaterGen_P3C_BaseColor | **named by nothing** (replaced by `_v2`) |
| 2.79 | Skaters/BP_SkaterVisual_P4Lab | **named by nothing** |
| 2.78 | Skaters/BP_SkaterVisual_TB2_P3 | generated_b2 on clay: test only |
| 2.78 | Skaters/BP_SkaterVisual_TB2 | generated_b2: test only |
| 2.75 | Skaters/T_SkaterGen_P2C_BaseColor | test only |
| 2.64 | Skaters/T_SkaterGen_P4D_BaseColor | test only |
| 2.55 | Skaters/BP_SkaterVisual_TB2_P4 | test only |
| 2.55 | Skaters/BP_SkaterVisual_TB2_old | **named by nothing** |
| 2.55 | Skaters/BP_SkaterVisual_TB2_P3_old | **named by nothing** |
| 2.52 | Skaters/T_SkaterGen_P4E_AO | P4 lab |
| 2.36 | Skaters/T_SkaterTB2_BaseColor | test only |
| 2.08 | Skaters/T_SkaterTB2_P3_BaseColor | test only |
| 1.89 | Skaters/SK_SkaterTB1_P3 | clay figure mesh, 12.0k triangles (shipped) |
| 1.74 | Look/Textures/T_Clay_Thumb_N | clay surfaces (shipped) |
| 1.61 | Look/P4Lab/T_P4_IceScratch_N | P4 lab |
| 1.58 | Look/Textures/T_Ice_Scratch_M | clay ice (shipped) |

Of the thirty, five are what the shipped game draws (the two Blueprints, the clay mesh, two clay textures) and one is the music.

**Particles, peak per spawn** (ledger notes unless named): check 49 (39 at Strength 0.5), knockdown 23, board crash 39, poke 5, cross-check 27, save 35, brake
18-40 shavings + 6-12 puffs, ability up to 24 (shield ring), deke 8 + 3, blade spray 40, shot 10-22, goal 60 + 100 x Strength (160 at 1; B3), win confetti
180 (GPU sim; B3), slide trail **about 220 live** at 4.7 m/s (50 spray + 170 flecks, bursting every 0.06 s while active; B30, before B32/B33 reshaped it).
`NS_goal`, `NS_win`, `NS_faceoff`, the pickups and the zones have no count of their own in the ledger. **The worst moment** (ten figures; a check that takes
a man into the boards and slides him; a goal; two hard stops at once): 49 + 23 + 39 + 220 + 160 + 2 x 52 = **about 600 sprites**, all CPU-simulated
(`NS_goal` runs on the CPU, B3). The slide trail is the largest term, larger than the goal.

**Lights per rig** (ledger rows): clay `LR_P3_Clay` has **18 local lights**: 4 shadowed warm key spots (5,200 cd, shadow resolution 1.0, source radius 60 cm),
8 shadowless side-lane lamps (2,100 cd), 6 shadowless fills, no specular on any, plus a movable sky light capture. Device A's goal point light (6,000 cd,
16 m radius, 3.6 m over the crease) runs for 108 frames on each goal (B41). Night `LR_L2_Night`: 4 spots, 2 shadowless end fills, the practicals (8
shadowless in B11) and B41's 3 shadowless rect fills (450 cd, 7 m). Broadcast (lab): 14 glint lights, 7 of 14 lamps shadowed.

**Textures** (as far as the ledger tells it): JamAssets has 58 textures. The 33 not in the figure labs are 22.0 MB on disk; the 25 in the labs are 40.1 MB.
**The shipped figure uses almost no texture of its own.** It is vertex colour, with `T_Felt_Fibre` (0.97 MB) and the number atlas. The lab figures' maps are
2,048 source capped at 1,024 (BC1 colour; P4E's normal and AO at 1,024). The clay look: `T_Clay_Thumb_N` (BC5, 1K), `T_Ice_Scratch_M` (1,024 = 4 m, masks),
`T_Ice_Scratch_N` (BC5, 1,024), `T_Zone_FastIce` (BC1, 1,024). There are duplicates: `T_Crest_*` and `T_Numbers_Varsity` exist in both `UI/` and
`Look/Textures/` (the `UI/` copies, 0.98 MB, are named by nothing). The 6 film-prop textures (camera crane, dolly, flight cases, slate, director chair,
film light, 4.4 MB) are marked shipped `y`, but their meshes are named by nothing. The ledger gives no GPU memory figure.

**The figure's Blueprint** (clay; the night one tracks it 0.2 MB lower). It is **graph logic**: events, variables and the per-call paths. The takes are
separate assets, so it holds no animation data. Size by commit: 1.99 MB (SKI3, 12 Nov 22:51) to 2.91 (SK12), 3.04 (SK14), 3.60 (SK15), 3.74 (SK16), 3.94
(SK18), 4.10 (SK19), 4.21 (QA3/QA4, 21:20), then **5.16 (SK21/SK22, +0.96)**, 5.17 (AU11), **6.07 (SK23, +0.90)**, 6.08 (B40), **6.36 (SK24, +0.28)**.
SK23 alone added 6 events (`FallPick`, `FallDo`, `ShiftTick`, `PlayStumble`, `SetBraking`, `SetTestMode`), changed 9 events, and added 11 variables
(SK23 report, "Blueprint changes as saved"). SK21 put `StopTick` into every `UpdateMotion`, about 30 nodes on the usual path. QA4 took one redundant push
out (412 to 366 nodes per call). **The same graph is in two files** (12.5 MB together), so every figure lane edits it twice. The cooked size, with the
editor graph stripped, is not measured.

**Shipped but unused** (refs.mjs, then filtered by hand; "named by nothing" is not proof). The binder builds `NS_<event>`, `MS_<event>`, the commentator waves
and the pickup and zone instances at run time, so those are taken as used.

- **Takes imported and not wired:** 16 late get-ups per figure (`GetUp_Late*`, `GetUp_LateLong*`, reachable only through `SetTestMode`, off by default:
  32 takes), `Stride_Sprint` x 2 (SK23), `GetUp_Late_Wall*` x 4 per figure (named by no package). `Burst`, `Stumble`, `Stop`, `Check`, `Celebrate`,
  `Idle_Faceoff`, `Shot_Charge` and 4 `Aim_*` per figure are also named by no package: 16 per figure, 5.1 MB in all. Some of these may be loaded by a name
  built at run time. Not verified.
- **Generated figures:** 105 `AS_SkaterGen_*` takes (19.8 MB) and 24 `AS_SkaterTB2_*` (4.1 MB) are named by nothing, on top of the test-only family above.
  `BP_SkaterVisual_TB2_old`, `_TB2_P3_old` and `_P4Lab` (7.9 MB) are named by nothing.
- **Sounds named by nothing** (about 6 MB): crowd alternates `CRD_Bed_a`, `CRD_Win_a`, `CRD_GoalAgainst_b/c`, `CRD_GoalFor_e`, `CRD_Roar_b/c`,
  `CRD_Save_a/c`, `CRD_PeriodEnd_b`. The second takes `SFX_Ability_*_b` (4), `SFX_Stumble_b` (AU12: imported, unused), `SFX_Brake_b`, `SFX_Deke_b`,
  `SFX_GetUp_b`, `SFX_HitBig_b`, `SFX_Slide_b`, `SFX_BoardsBody_b/c`, `SFX_BoardsFall_a/b` (AU11: the board fall keeps the old sounds),
  `SFX_GlassRattle_b`, `SFX_Bump_b/c`. Also `SFX_UiClick_01`, `SFX_Clapperboard_05` and `SFX_ProjectorLoop_01` (the film theme is gone).
- **Arena pieces named by nothing:** the film props (`SM_CameraCrane`, `SM_CameraDolly`, `SM_FilmLight`, `SM_FlightCases`, `SM_DirectorChair`, `SM_Slate`)
  and B11's `SM_Bench`, `SM_PenaltyBox`, `SM_GoalLight`, `SM_ScoreboardHousing`, `SM_TrussLamps`, `SM_CoachStool`, `SM_WaterCrate` (device A's own pieces
  replaced them): 13 meshes, 4.9 MB, plus their 13 base-colour textures, 8.7 MB.

**Ten cost evolutions, ranked by saving per risk**

| # | Evolution | Count it moves | Risk | Lane |
|---|---|---|---|---|
| 1 | Delete the three Blueprints named by nothing (`_TB2_old`, `_TB2_P3_old`, `_P4Lab`) and `T_SkaterGen_P3C_BaseColor` (replaced by `_v2`) | JamAssets -10.9 MB; 4 packages | very low: named by nothing; a cook-only run proves it | B lead (with cook check) |
| 2 | Delete the film props and B11's replaced arena pieces with their textures (13 meshes, 13 textures) and the two film sounds | -14.0 MB; 28 packages | low: named by nothing; check the night rig and menu by name first | B arena/look |
| 3 | Cap `NS_slide_trail`'s live count (burst every 0.06 s forever while active) at about 80 | worst-moment sprites ~600 to ~460; the biggest single term | low: re-shoot the slide strip on clay | B effects (B42) |
| 4 | Keep the goal's point light off the ice (B41's numbers: radius 1,600 to 350, 800 cd) | one 16 m moving light for 108 frames to a 3.5 m one; pink ice 0 | low; also fixes fault 9's pink half | device A (numbers in its actor) |
| 5 | Play `MS_brake` and `NS_brake` only for the human and figures within about 15 m of the camera (AU11 decision 2) | 46 to 61 brake sounds per 100 s to about 15; same for sprays | low: one `if` in both Blueprints | B figure lane |
| 6 | Drop the unused second takes and crowd alternates, or wire them as variety (AU12 next step 3) | about 6 MB, or 0 MB and fewer repeats | low | B sound |
| 7 | Keep the generated_b2 family and the P4 lab out of the cook: `DirectoriesToNeverCook`, or move them to a lab plugin; drop or keep the two test cartridges | packaged art about -183 MB (53 % of JamAssets) | medium: `VisualSwapTests`, the fuzz sweep `skater.visual` and two cartridges name TB2; the menu must not offer it | device A (config, tests), then B moves the files |
| 8 | One figure Blueprint: a parent with the logic and a clay child that only swaps the mesh, takes and materials | 12.5 MB editor to about 6.5; every figure lane edits once, not twice | high tonight: the figure is the key feature; needs a full re-proof of the chain on both tiers | B figure lane, after 10:00 |
| 9 | The clay rig's 6 low fills merged into the sky light or into 2 broader fills | 18 local lights to 14 | medium: the readability numbers (figures 27 luma or more from the ice at eleven places) must be re-measured | B look |
| 10 | Device A's own lab content out of the package: `Game/Content/P4` (100 MB) and `Characters/Mannequins` (73 MB, the Manny normal alone 20 MB), plus the six 2.3 to 2.8 MB menu card textures at 1,024 | the package's art total, by about 170 MB | medium; device A's | device A |

**What only device A can time, on a quiet GPU:** (1) the cost of the 4 shadowed key spots on clay at shadow resolution 1.0 against 0.5, the largest per-frame
light cost of the shipped tier; (2) a goal frame with the 6,000 cd, 16 m point light against B41's 350 radius; (3) the worst moment above (hit, slide,
goal, two stops) as GPU and game-thread milliseconds, with the Niagara CPU simulation; (4) the cooked size of the two figure Blueprints, and their
`UpdateMotion` cost per tick for ten figures (SK21 added about 30 nodes per call); (5) `M_Ice` on clay, with B19, B22, B23 and B26's Custom nodes and A23's
noise, over the full screen at the window's resolution (1920x1085 since SK23; it was 1398x790 before).

### Evolution 3, started: the morning page, first version

`C:\Users\kazda\jam\review\night\index.html`. The top screen holds "What changed since you went to bed" (nine tracks, each with its best still) beside
"What you must decide" (nine choices, each with A, B and two stills: get-up, goal loudness, stands, unheard sounds with the three listening pages, effect
size, bot stop bar, fall sizes, night fill, new sets). Below them: "What to try when you play" (nine steps in the order he meets them) and "What is not
proven". In depth: the critic's twenty with the stills, the cost summary with the thirty largest files (recomputed from disk on every build), every night
still grouped by track with its caption (39 stills since 13 Nov 22:00), and every report changed since 22:00 (42). It also shows which night-lane reports
exist yet. Rebuild: `node C:\Users\kazda\jam\review\night\build.mjs`. The hand-written content is in `C:\Users\kazda\jam\review\night\data.mjs`.
Checked rendered: `C:\Users\kazda\jam\review\night\crops\page-top.png` (headless Edge, CPU).

## 3. Not verified

- Frame time of anything (the rule for this GPU). Every cost above is a count, or bytes on disk; cooked sizes are not measured.
- "Named by nothing" comes from a name-table scan. An asset loaded by a soft path built at run time (as the binder does for `NS_`, `MS_`, VO, pickups and zones,
  which I excluded), or by a Blueprint string, would show as unused. Run the engine's reference viewer before deleting.
- The slide trail's live count is B30's (13:00), from before B32, B33 and B35 reshaped it.
- Faults 1 to 4, 16 and 20 are from QA4's stills (21:15). Device A has changed the HUD and the load order since (A135, A144, 2250116), and I have no newer
  in-match still with the HUD. Fault 5 is one sequence in one match.
- Fault 9's "small at the game camera" uses the staged goal view, held still; a real goal's camera may sit closer.
- Luma targets in the proving numbers are mine, chosen by eye against the stills; nothing was measured on pixels for this report.
- Nothing heard. The night lanes' work (SET-*, TRAILER, RAG's experiments) was not yet filed, so it is not judged.

## 4. For the engine lane

No takes from me. What the engine lane's work must never do, from what I saw:
- A downed figure must not be stood on. If the sim keeps letting skaters through a downed body, the figure lane should at least keep the other figure's
  skates off the lying mesh (fault 5; `sk24-lying-loop-play` f30-f54).
- Effects must not cover the number or the downed body for more than 2-3 frames (faults 6, 17).
- B42, when it runs: brake spray at least 1.5 % of the frame at +6; abilities at least 2 % at +4, distinct by kind; chips as streaks, no beads; the slide
  trail capped (cost evolution 3).

## 5. For the other device

1. **HUD over the play** (faults 1 to 4, 16): the selection ring drawn over the human's figure and over his downed body; captions at lower centre over the human;
   taunt bubbles over the net and the middle ice; the period text on the border of an empty panel; distance markers on visible targets. Stills:
   `...\20261113-211603-B-qa4-clay-play-4.png`, `...-211602-B-qa4-clay-play-3.png`, `...-211559-B-qa4-clay-play-1.png`. A fresh in-match still on your
   current build would show which of these A135 and A144 already fixed.
2. **A skater through a downed body** (fault 5): `...\20261114-004300-B-sk24-lying-loop-play.png`, seed 1, sim frame 5242 + 30 to 54. Is a downed skater a
   blocker in the sim? If not, a push-out radius of about 0.6 m around a downed skater for others.
3. **The corner camera** (fault 11): `...\20261114-004259-B-sk24-stumble-real-caller.png` row 2: a pitch cap in the corners.
4. **Cook exclusions** (cost evolution 7): `DirectoriesToNeverCook` for the generated_b2 family and the P4 lab, or your word that `generated_b2` must stay
   selectable; and the tests that name TB2.
5. **The goal light** (cost evolution 4, fault 9): B41's numbers (radius 350, 800 cd on clay, 320 on night, the scored end's light placed where
   `jam.rink.react.loud 0` puts it).
6. **The lifted-puck post** (fault 19) and the clay clocks (fault 14).
7. Timings on a quiet GPU: the five items listed under Evolution 2.

## 6. Decisions for the operator

All of these are on the page, each with two stills.
1. How a downed player gets up (SK23, SK24): lie longer and spring up, or rise slowly as now. Note that a new strip from export26 to 29 is due first.
2. How loud a goal is (B41): pink ice today, or lamp, burst and net only. The critic's view: neither reads as a payoff yet; take B41's light numbers and ask for
   a bigger burst (fault 9).
3. How full and bright the stands are (AR1; AR2 running).
4. The unheard sounds: three listening pages; the roar at Strength 0.75, the quiet stumble, the hard stop on all ten figures (AU11, AU12).
5. Effect size: keep small, or make them read at game size (B40). The critic recommends reading at game size.
6. A bot's stop bar: 8 s or 6 s; the human's lean-in (SK24).
7. Fall-size bars (SK23); night fill (B41); the new sets when they arrive.
8. From the cost audit: whether `generated_b2` must stay selectable at all (if not, 53 % of JamAssets can leave the package).

## 7. The next three evolutions

1. **The page, current at every round:** re-read what lands (SET-dive, SET-dash, SET-weapons, TRAILER, RAG, the engine lane's AR2 and the animation import) and
   rebuild. Proving number: every report under `status\` changed since 22:00 is listed with its newest still, and every open decision in them has its two
   stills on the page: **0 missing** at each rebuild.
2. **The critic's second pass on the new arrivals:** the three sets' sheets and AR2's stands in the game camera, judged against fault 12 and the readability
   bars. Proving number: each new still gets a verdict line, and a fault list of at least five, each pointed at a region, or "no fault" with the reason.
3. **The reference check made sure:** cross-check `refs.mjs`'s 256 "named by nothing" packages against the two figure Blueprints' text dumps (SK23 and SK24
   kept them in their lab folders) and the binder's name rules. Proving number: a delete list on which **every** entry is named by no package, no string and
   no name rule, with its MB total. That is the input for cost evolutions 1, 2 and 6.

---

# Round 2 (02:05 to 02:40): the second pass, the delete list made sure, the page tightened

*(Written after the lead's re-brief. Round 1 above stands. New page content is listed under Evolution 6.)*

## 1. Summary (round 2)

Three evolutions are done.

- **Evolution 4, the second pass:** a verdict line on every new still that landed after 01:55 (SKF29, TRAILER, AR2 ×10, AR3 ×5, SKF30 ×2), and at least four pointed faults per lane. The trailer's scenario A is read as a jam judge reads it.
- **Evolution 5, the delete list made sure:** a reachability walk over every package's name table. Its roots are game code, the binder's and the rink's run-time name rules, `commentary.json`'s 412 lines, the rigs and device A's packages. Result:
  - **445 files, 131.0 MB, are safe for B to delete**;
  - **214 files, 68.9 MB, need device A's word**;
  - 48 files, 7.0 MB, are held back by hand with a reason each.
  - Nothing was deleted.
- **Evolution 6, the page tightened:**
  - The first screen now holds one line and one still per track, and only the choices that need the operator.
  - The measured defaults moved to their own list.
  - Added: the trailer, the nine-lane plan, the two handoffs, the second pass and the delete list.

SETS-PLACEHOLDER

## 2. Per evolution (round 2)

### Evolution 4: the second pass

Verdicts (looked at with the Read tool, at full size where it mattered):

| Still | Verdict |
|---|---|
| `C:\Users\kazda\jam\thirdjam\devlog\shots\20261114-013753-B-lab-skf29-back-getups.png` | real gain, invisible at sheet scale: from above the stick lies beside the right skate (export29); from the side the two exports look the same. The proof is the measure (40 → 6 leg frames), not the picture |
| `...\20261114-015039-B-trailer-storyboard-A.png` | complete and honest (every tile marked placeholder); weak opening, 9 s of menus |
| `...\20261114-021041-B-ar2-game-ab.png` | the proof frame: stands fill the top band, ice -0.03 %, jerseys within 2.4; fault 12 (empty barn) is fixed |
| `...\20261114-021041-B-ar2-game-play-1.png` | un-paused, clay: the house reads as rows of small dolls in a dark timber band; the play is unaffected |
| `...\20261114-021041-B-ar2-game-play-2.png` | the corner from almost straight above: near stands read as empty benches; the caption ("near centre ice") does not match the picture |
| `...\20261114-021041-B-ar2-side-corner-ab.png` | before/after at board height: the stands replace the bare apron cleanly; no piece of device A's is crossed |
| `...\20261114-021041-B-ar2-low.png` | low view: the house is dim (band luma 30) and the fans in team-less clothes |
| `...\20261114-021041-B-ar2-wide.png` | fault: fans scaled to 2.3 m on the wide arena, heads as large as the players' |
| `...\20261114-021041-B-ar2-far-end.png`, `-large.png`, `-pond.png` | captions read, not judged at full size: placement on four rink sizes, nothing crosses device A's pieces |
| `...\20261114-021017-B-ar3-goal-gamecam.png` | the goal answer at the game camera changes 3.1 % of a dark band: it cannot be read |
| `...\20261114-021017-B-ar3-goal-low.png` | from a low camera the cheer reads (arms up, half risen) but stays in shadow |
| `...\20261114-021017-B-ar3-pieces.png` | the figures are good clay toys close up; muted, scarfless |
| `...\20261114-021017-B-ar3-barn-low.png` | the roof reads only from a low camera; the bunting is a faint row on dark timber |
| `...\20261114-021017-B-ar3-barn-upice.png` | caption read: 0.74 % of the game camera's frame; nothing to judge |
| `...\20261114-021203-B-lab-skf30-hit-sizes.png` | the body check reads in three sizes; the cross-check in two |
| `...\20261114-021203-B-lab-skf30-stick-fixes.png` | the fix shows only in the measure; in Stumble_Left the stick is out of sight for frames 22 to 32 in both exports |

**Faults per lane, pointed** (full wording on the page and in `C:\Users\kazda\jam\review\night\round2.mjs`).

**AR2:**
1. At the game camera the crowd is a dim brown lattice, not people. Band luma fell from 57 to 40 (`ar2-game-ab`, top band of both "after" tiles).
2. From the corner camera the near stands show empty timber, and the caption does not match the picture (`ar2-game-play-2`, top right, 1000-1900 by 0-450).
3. On the wide arena the fans are 2.3 m tall, larger than the players (`ar2-wide`, side-board and bench tiles).
4. No team colour anywhere in the house (`ar2-low`; `ar2-side-corner-ab` after).
5. Dark slabs sit on the board cap, and were there before this lane (`ar2-low` at 330-360, 630-660 and 900-920 by 593-610). Owner unknown.

**AR3:**
1. The goal answer cannot be read at the game camera: 3.1 % of the frame changes, in shadow (`ar3-goal-gamecam`, top against bottom).
2. The cheer is a pose swap with no motion between, so 231 fans would pop in one frame.
3. The roof, bunting and gate appear in no game-camera frame, or in 0.74 % of one. They serve only a trailer, menu or replay camera, and none of those exists.
4. The bunting is a faint muted row (`ar3-barn-low` bottom, 300-820 by 820-845).
5. Tonight's scarfless fans mixed with AR1's scarved ones will make an inconsistent house (`ar3-pieces`).

**SKF30:**
1. The light body check is hard to tell from gliding: row 1, and row 7 on clay.
2. Lift and Lift_Heavy look nearly the same: rows 5-6 and 11-12, frames 3 to 10.
3. The heavy body check (24 frames) recovers before the medium one (30): row 3 is up by frame 18.
4. In the medium and heavy checks the stick trails a metre behind, flat, and reads like a swung bat from above (rows 2 and 3, frames 4 to 12). Check it against the teen rule at the game camera.
5. The sheet has no victim and only the right shoulder. In `skf30-stick-fixes`, Stumble_Left loses sight of the stick for frames 22 to 32.

**SKF29 (four, all I can point at):**
1. Export29's stick points straight ahead of the sitting figure, about a body length, from frame 6 to 14 (rows 2, 4, 6, 8). From the high game camera it may read as a raised stick.
2. The proving number is not met: 83 leg frames and 31 trunk frames remain over the 32 get-ups.
3. The ready stance itself has the shaft 1 to 3 cm inside the thigh.
4. There is no game-camera row: the claims are made from views the game never shows.

**The trailer, read as a jam judge reads its first ten seconds.**

The first ten seconds are menus:
- 0 to 2.5 s: the slate.
- 2.5 to 5 s: twelve cards in text too small to read at video size.
- 5 to 9 s: a text box being typed into.

The first hockey frame is at 9.0 s and the hit at about 12.5 s. A judge sees UI, not clay toys on ice, and not the tackle, which is the game. Further weaknesses:
- "make it faster" produces no visible change in the film.
- The theme is only words on screens until the Director's Call at 26.5 s.
- Every shot is the high game camera, so the film inherits the HUD faults (ring and caption over the downed player, in shot 6).
- The end card is a Blender render, the best-looking frame in a Shipping film.
- The recorder (Game Bar with sound, 60 fps) is untried.

**Proposal: keep A, but open cold on the hit.** Shot 5 at 0.0 to 2.5 s (contact, the hold, the slide), then the menu as the "how". Nothing new needs filming.

### Evolution 5: the delete list made sure (`C:\Users\kazda\jam\review\night\deletelist.mjs`; table `C:\Users\kazda\jam\review\night\deletelist.md`, 707 rows; data `deletelist.json`)

**Method.** Every `/JamAssets/...` string in every package's name table is an edge. The walk starts from these roots:
- every JamAssets name in game C++, ini and json, including the plugin's own `commentary.json` (all 412 line ids have a wave);
- the binder's and the rink's run-time name rules: `NS_<event>` and `MS_<event>` for the 33 events in `JamEventBus.cpp`, `VO_Commentator_<line id>`, `MI_`/`SM_Pickup_<kind>`, `MI_Zone_<kind>[_Ring]`;
- the rigs, named by `LookTierSwitcher.cpp`;
- device A's packages under `Game/Content`.

A package is "needs A" when it is reached only through:
- `BP_SkaterVisual_TB2`, `_TB2_P3` or `_TB2_P4`, which only `generated_b2` selects (the test cartridges `hockey.test.visual-b2` and `figure-b2`, `VisualSwapTests.cpp`, the fuzz sweep);
- or test files;
- or device A's packages.

The figure Blueprints name their takes by hard path (checked in the bytes: `AS_SkaterTB1_P3_Deke_Left`, `_Stumble_Back`, `_Stop_Hard`, `_Check_Hit_*`), so a take named in no package is truly unwired. That was cross-read with `skf\export26\READY.txt` and the SK21 to SK24 reports.

| Class | Files | MB | What it is |
|---|---|---|---|
| **safe for B** (one engine slot after 10:00, then a cook-only run) | 445 | 131.0 | the old hybrid figure (SK_SkaterTB2, _TB2_P3, their 151 takes, `_old` Blueprints): 43.1; the P4 lab figure and its looks (`BP_SkaterVisual_P4Lab`, P4E, `M_/MI_P4_*`, `NS_P4Lab_spray`): 40.7; generated P4D: 21.1; the film props and the replaced arena pieces with their textures and instances (42 files): 13.4; generated P2C/P3C leftovers (old base colour etc.): 6.8; 22 leftovers of the shipped figure (old aim holds Front/Left/Right/FarReach, Celebrate, the old Check, Idle_Faceoff, Shot_Charge, the old Stop and Stumble, both styles; `MI_SkaterTB1_P3_Pads`, `SM_SkaterTB1_P3_Stick`): 3.3; the UI copies of the crests and numbers, the backdrop hills, film-themed sounds (clapper, projector, UI click), the unused fx masters `M_Fx_Sprite`/`M_Fx_Ribbon`/`MI_Fx_Ring`: about 2.7 |
| **needs A's word** | 214 | 68.9 | the generated_b2 figure on its three tiers: `BP_SkaterVisual_TB2`/`_P3`/`_P4`, their meshes SK_SkaterGen_P2C/P3C and SK_SkaterTB2_P4, 200 takes, textures, AnimBPs, blend spaces |
| **keep, held by hand** | 48 | 7.0 | second takes and crowd alternates until heard (23, 4.6 MB); `GetUp_Late_Wall*`, `Stride_Sprint`, `Burst` (the operator's get-up choice, SK23's next steps; 12, 1.9 MB); `MS_bump` + `SFX_Bump_a/b/c` (waits for A's bump event); AR2's two not-yet-ledgered pieces; `MS_arena_room`; `SFX_BoardsFall_a/b` (AU11 decision 1); `MI_Trim_Lamp`, `MI_Toon_NetCord` (B41's, for A to bind); `SC_Ambience`, `SCM_Volume` (a sound class can be set in project settings) |

**Proving number:** every one of the 445 "safe" entries is reached by no package, no game or test string, and no name rule. The method has only these limits, written in `deletelist.mjs`:
- a path built by a Blueprint from string pieces would not be seen. None was found for the figures: their takes are hard paths.
- device A's packages outside `Game/Content` are not scanned.

That 131 MB is 38 % of JamAssets. With A's word on the 68.9 MB the total is 200 MB (58 %).

### Evolution 6: the page, tightened (`C:\Users\kazda\jam\review\night\index.html`; content in `round2.mjs`, rebuilt by `build.mjs`)

**What is new on the page:**

1. **What changed:** nine tracks, one line and one still each, rewritten short.
   - Arena now shows AR2's engine A/B still.
   - Trailer and Critic rows were added.
   - New sets show their newest `-B-set-` still as soon as one lands.
2. **What you must decide** is cut to seven: get-up; goal loudness; the crowd (neutral and half full, or team colours rising on a goal); which trailer and whether it may say "cut"; listen to the unheard sounds; whether `generated_b2` must stay selectable; the new sets.
   - Each is shown as A or B with two stills (150 px high, so the first screen is shorter).
   - The duplicate get-up still is fixed.
   - "Effect size" was dropped: B42 has it.
3. **Defaults I set, change if you disagree** (new): fourteen measured defaults with their lane. These include the roar at 0.75, the ooh floor, the quiet stumble, the hard stop on ten figures, the board-fall sound, the bot's 8 s bar, the fall-size bars, the lamp at 12, the night fill, the cross-check mark, the stands at 489 fans, the crowd's scaling, the scoreboard and AR3's roof.
4. **Device B tonight: nine lanes** (new table): lane, what it loops on, and what has landed. Report and still counts are read from disk at each build.
5. **Handoffs to device A tonight** (new): B-to-A-0040 and 0041, one line each.
6. **The trailer** (new section): the pick, the seven lines with their times, the three questions it leaves, the critic's five points on the first ten seconds, and links to `storyboard.html`, `read\listen.html` and `animatic-A.mp4`.
7. **The critic's second pass** (new): a verdict and faults for SKF29, TRAILER, AR2, AR3 and SKF30.
8. **The delete list** (new): the three classes with counts and MB, linked to `deletelist.md`.

## 3. Not verified (round 2)

- The delete list is a static walk. Before deleting, open the engine's reference viewer on the 445 and run a cook-only check after the delete. A Blueprint path assembled from string pieces would escape the walk; none was found.
- The AR2 verdicts are from frozen A/B pairs and two un-paused stills taken at 8 to 17 ticks a second. AR3 is Blender only, at 12 samples, and the denoiser smears faces.
- The trailer verdict is from the storyboard sheet and the report. I did not play the animatic or listen to the read.
- The SKF30 stick-trail and "swung bat" reading is from above. The game camera view of it is not rendered.

## 4. For the engine lane (round 2)

- **AR2 follow-up:**
  - light on the crowd, or brighter hats and scarves, so the band reads as people;
  - on Pond, wide and large, scale the stands' depth, not the fans;
  - fill the near corner rows, so the corner camera does not show empty benches.
- **If AR3's cheer goes in:** blend over 6 to 10 frames, not a one-frame swap, and only the scoring side's half. Proving number: the cheer covers at least 5 % of the game camera's frame and no fan changes pose in a single frame.
- **The delete slot after 10:00:** delete the 445 "safe" files in one editor session with redirectors fixed, then a cook-only run. Proving number: cook PASS and JamAssets down by 131 MB.

## 5. For the other device (round 2)

- **B-to-A-0041 point 4, made exact:** 214 files, 68.9 MB, leave the package if `generated_b2` is dropped from `hockey.test.visual-b2`, `hockey.test.figure-b2`, `VisualSwapTests.cpp` (line 22) and the fuzz sweep `skater.visual`, and `HokejGameMode::VisualClassPathsFor`'s `VisualB2` branch goes. The list is `C:\Users\kazda\jam\review\night\deletelist.md`, class "needs A".
- **For the trailer (A owns the copy):** a cold open on the hit, and a `-cut=` start in Shipping (already TRAILER's ask).

## 6. Decisions for the operator (round 2)

1. The crowd: neutral and half full as placed, or team-colour scarves with the scoring half rising on a goal. The critic says the second, with light on it.
2. The trailer: scenario A or C; whether it may say "cut"; and the critic's cold open on the hit.
3. Whether `generated_b2` must stay selectable (68.9 MB).
4. As before: the get-up and the goal loudness. The new sets come when they land.

## 7. The next three evolutions (round 2)

1. **The sets, judged as they land:** dive, dash and weapons sheets at game size. Proving number: per lane, a verdict on every sheet and at least five pointed faults, with explicit lines on "flat by the frame it claims", "prop in the hands" and "reads as a real weapon or cruelty".
2. **One engine picture of tonight's work for the operator:** once B42 and the animation import land, a single sheet with the same moments before and after (stop spray, ability, goal, a downed player with the stands behind). Proving number: 4 of 4 moments in un-paused engine stills, each against its critic's number (spray at least 1.5 %, ability at least 2 %, burst at least 3 %, the downed figure clear of HUD and other skaters).
3. **The delete list turned into the engine lane's script input:** a plain path list (445 lines), plus a one-page check, in the delete slot's order, of what to verify in the editor (the reference viewer on the 10 largest). Proving number: the list's MB equals the cook's drop within 1 %.
