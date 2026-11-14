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
