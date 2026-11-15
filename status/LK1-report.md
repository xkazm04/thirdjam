# Lane LK1 report: the looks as the trailer will show them: look, list, fix (device B), 2026-11-15

Tree `C:\Users\kazda\jam\wt\b-aud` (branch `lane/b-aud`), port 8006. Machine clock 12:38 to 14:32 for everything below. No commit, push, cook or build by me: the lead committed three hand-backs (`ebbdcd46`, `d6ada773`, `c70df7bb`). No paid service, no download, nothing sent outside. Nothing written under device A's folders. Skater Blueprints, takes and figure materials not touched.

**Housekeeping**
- GPU editor, five slots, each taken by removing `C:\Users\kazda\jam\GPU-FREE.txt` and given back with a line:
  - 12:39 to 13:05: pid 38620 **died of a GPU hang** (`DXGI_ERROR_DEVICE_HUNG`) at 12:41, 25 s into its first play session while the lead's cook ran; I stopped that process and its crash reporter (both mine). Then pid 3460, left through `ExitEditor`.
  - 13:32 to 13:44: pid 28660. 13:47 to 14:03: pid 23280. 14:04 to 14:13: pid 32772. 14:23 to 14:29: pid 3532. All left through `ExitEditor`, process gone, no dirty package.
- Settings as found at each exit: `enable_game_sound` false, `jam.ScriptHuman` 0, `r.Editor.Viewport.OverridePIEScreenPercentage` 1, `jam.Cut hockey.regular`.
- Editor pids 15860 and 34920 (tree b-fig, `-nullrhi`, port 8004) and the lead's cook processes were not mine and not touched.
- Backups of every file before I changed it: scratch `d3\lk1\bk\` (with `sha1.txt`). Scripts: scratch `d3\lk1\` (`rig.py`, `look.sh`, `all.sh`, `sg.sh`, `pin.sh`, `day1.py`, `day2.py`, `neon1.py`, `crowd56.py`, `crowd56b.py`, `patch_sg.mjs`, `patch_sg2.mjs`, `ice.py`, measures `diff.py`, `meas.py`, `sgmeas.py`, `icemeas.py`).
- How the stills were made: a play session on the mode's own cartridge with `jam.ScriptHuman 1`, `HighResShot 1920x1080` from device A's own follow camera at set world times (no HUD in them). The camera in play sits 3.5 m up at pitch -17.7 to -32, FOV 70.

## 1. Done / not done

| Item | Verdict |
|---|---|
| Job 1: every look filmed at A's game camera and from the faceoff, looked at, faults listed worst first | **done** (section 2) |
| Pond (`l1_day`): the day rig, which was an empty placeholder | **done, filmed, merged** |
| Shotgun blast readable | **done twice**: a wide fan (hand-back 2), then on the lead's ask a toy blast: flash, streaks, gone by frame 14 (hand-back 3) |
| Glow Ice: figures and lines crisp in the dark | **done**: outline drawn in light, frame line toned, bloom, cleaner ice |
| Found on the way: Shotgun and Bruisers (56 x 28) had no stands and no crowd | **done**: an eighth crowd actor |
| Clay ice: scratches at the feet, band at the board foot | **done**, the middle of three values |
| Second look at all ten looks | **done** (section 2, end) |
| The blast fired by a real F press, aim turned, on night and neon | **not done** (section 3) |
| The goal in team-colour light on Glow Ice | **tried, not kept**: it needs the tier from device A (section 4, item 1) |

## 2. The list (job 1), then each fix

### What a stranger saw as unfinished, worst first (12:45 to 12:55, before any change)
Sheet of the ten looks before: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-130500-B-LK1-looks-before.png`.

1. **Pond (trailer shot M01): black above the boards**, a flat grey slab behind them, no sun, no shadows, the trim glowing as at night. `Rigs/LR_L1_Day` was still lane A0's 8 KB placeholder. B's.
2. **Shotgun (M07, M09, M10): the blast was a slate smoke puff**, 2.9 % of the frame at its largest, gone by frame 18. B's.
3. **Shotgun and Bruisers: an empty barn.** No stands, no crowd on the 56 x 28 rink: the crowd actors existed for seven rink lengths and both modes were moved to 56 m afterwards. B's. (Found while filming the blast.)
4. **Glow Ice (the finale): bodies in two pieces.** Black pants, gloves and skates vanished into the black ice; the frame line was blown out to white, brighter and fatter than the puck's trail; brown reflection blotches between the frame line and the boards. B's.
5. **Clay barn (seven looks): good.** The scratch crosshatch at the feet and a pale ragged band at the board foot were the busiest things on the ice. B's.
6. **Night (Fog of War): reads well.** The lamp pools draw as hard bullseye rings. Left as is.
7. Device A's: on Glow Ice the penalty boxes and clocks are lit cream, the brightest things outside the play; the fog of Fog of War is not visible. The lead has both.

### Item A: the Pond's day rig
- **What changed**: `Rigs/LR_L1_Day.umap` is now the night rig's own dressing (snow field, banks, mounds, sky dome, ground planes, capture dome, seven crowd-and-stand actors, post volume), copied with `save_map` from `LR_L2_Night`, without its 150 night-only actors, plus one sun. No new mesh. Three new material instances carry the day colours (`MI_Snow_Day`, `MI_DaySky`, `MI_SkyAmbient_Day`). Stands and fans use the clay tier's existing instances.
- **Proving numbers** (faceoff, same camera, 1920x1080): black pixels **13.9 % -> 0.5 %** of the frame; top band luma **0 -> 223**; the centre's shadow on the ice 0 -> 27 luma under the ice beside it; the orange jersey 237/136/66 -> 215/114/3. The first try at 55 lux turned the orange yellow; it ships at 16 lux. Exposure stays EV 4.2 as on every rig.
- **Other tiers unchanged**: `LR_P3_Clay`, `LR_L2_Night`, `LR_L3_Neon` byte-identical at that point; re-filmed faceoffs differed in 364 / 256 / 1993 of 2,073,600 pixels.
- **Stills**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-130501-B-LK1-pond-faceoff-ab.png`, `...\20261115-130502-B-LK1-pond-play-ab.png`, `...\20261115-130503-B-LK1-pond-corner.png`, `...\20261115-130504-B-LK1-outnumbered-day.png` (70 x 35), `...\20261115-134405-B-LK1-day-regular-rink.png` (90 x 45), `...\20261115-130505-B-LK1-other-tiers-same.png`.
- **Looked at with the Read tool**: the faceoff sheet; both halves of the play sheet, the corner, Outnumbered and the 90 m still at full size. The other-tiers sheet was not opened; its six sources were.

### Item B: the shotgun blast (`NS_ability`, variant 0 only; the other three kinds untouched)
- **Hand-back 2, a fan**: 120 sky-blue dabs in a flat fan and 60 shards, held to frame 24. Share of the frame changed against the same timeline with nothing spawned: +8 frames 2.91 % -> 6.58 %, +18 gone -> 6.71 %. The lead: bigger, but still soft smoke puffs.
- **Hand-back 3, the toy blast** (the lead's ask):
  - frames 0 to 3: a near-white flash, a new emitter `Flash`, two soft discs 170 to 220 cm for 4 to 5 frames. It sits round the shooter, not at the blade: from A's camera anything on the ice within about 5 m in front of him is hidden by his own body.
  - 140 thin blue streaks 9 x 300 cm at 60 to 140 m/s, velocity aligned, fanning along the aim, life 10 to 18 frames.
  - dabs 120 -> 30, faster, life 10 to 14 frames.
  - share of the frame: **+2 frames 4.71 %, +4 3.47 %, +8 2.27 %, +12 0.57 %, +14 0.46 %**, +18 0.38 % (0.37 % is the floor with nothing left).
- **Tried and dropped**: the star sprite `MI_Fx_Star` drew nothing in the new emitter; the flash is on `M_Fx_Chalk`. A first flat fan at 90 dabs lost half its dabs under the ice (2.3 %).
- **How it was filmed**: hand-spawned as the binder does (`User.Dir` = the human's facing, Strength 1, Variant 0) at the faceoff of the Shotgun mode, world slowed to 0.05, stills at game frames. I could not press F in a scripted match.
- **Stills**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-134400-B-LK1-shotgun-blast-ab.png` (old | fan), `...\20261115-134401-B-LK1-shotgun-blast-f08.png`, `...\20261115-135800-B-LK1-shotgun-toyblast-ab.png` (fan | toy blast at +4 +8 +12 +18, and the toy blast at +1 +2 +3 +6), `...\20261115-135801-B-LK1-shotgun-toyblast-f02.png`.
- **Looked at**: both sheets as sheets; the old +8, the fan +8 and +18, the toy blast +2 and +6 at full size.

### Item C: Glow Ice (`LR_L3_Neon`, two instances; no figure material, no Blueprint)
- **Outline drawn in light**: a new instance `MI_PP_Outline_Neon` (lilac 1.2 / 0.9 / 2.0) replaces the ink outline in `PPV_L3_Neon` only. Pants, gloves, skates, sticks, nets and the glass rail get a lilac line. Lit pixels (over 40 of 255) in the far figures' leg boxes: **7.4 % -> 34.9 %** and 7.5 % -> 33.9 %; the human's pants and skates 34.5 % -> 52.5 %.
- **Frame line** `MI_Neon_BoardLine` Emissive 20 -> 6: brightest pixel 250/234/252 (white) -> 220/178/230 (orchid). The puck's white ring and trail are the brightest things again; they are unchanged.
- **Bloom** 1.3 -> 1.8. **Reflections** screen-space (as on night): the brown blotches by the boards are gone (seen, not measured).
- `MI_Neon_Grid` was re-saved with the same value and put back byte-identical.
- **Stills**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-134402-B-LK1-glowice-faceoff-ab.png`, `...\20261115-134403-B-LK1-glowice-play-ab.png`.
- **Looked at**: all six source tiles at full size (the two sheets themselves not opened), and a third play still at 8 s.

### Item D: stands and crowd on the 56 m barn (`LR_P3_Clay`)
- **What changed**: an eighth actor `Crowd_P3_Clay_k0622` (a copy of the 60 m owner at scale 90 / 56), refilled from lane AR13's own layout script at 56 m: 36 stand pieces, 472 fans with the six floats; its six end-wall pieces moved in by 56 / 60. It destroys itself on every other rink as the seven do.
- **Proving numbers** (faceoff): Shotgun 187,997 and Bruisers 187,763 of 2,073,600 pixels changed (**9.1 %**); the regular rink 404 and Cage 51 (unchanged).
- **Still**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-134404-B-LK1-stands-56m-ab.png`.
- **Looked at**: five of its six tiles at full size (the Bruisers faceoff pair only measured); the sheet itself not opened.

### Item E: the clay ice (`MI_Ice_Night`, three scalars read on the clay tier only)
- **What changed**: ScratchAmt 0.7 -> 0.55, ScratchLift 0.35 -> 0.28, SnowAmt 1 -> 0.3.
- **Proving numbers** (pinned camera 7.5 m from the side boards): scratch lines over their local ice mean **0.84 -> 0.60**, 99th percentile 6 -> 4 luma; the pale band at the board foot **31.0 -> 19.4 luma** over the ice beside it.
- **Tried and not taken**: 0.4 / 0.2 (mean 0.42): the ice read as plain plastic. Three wrong knobs first (BoardBandK, ClayIceBumpMul, ScratchBump): the white band is B19's snow dusting, the scratches are the base-colour layer. All three are back at their old values (ScratchBump is written at its default 0.6).
- **Other tiers unchanged**: night 283, day 312 of 2,073,600 pixels (noise level); neon 2,500 against its own still of hand-back 2.
- **Stills**: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-140300-B-LK1-clay-ice-ab.png`, `...\20261115-140301-B-LK1-clay-ice-three.png`.
- **Looked at**: the three-version sheet; crops of all four tiles of the A/B sheet at full size (the sheet itself not opened).

### The second look (14:05 to 14:13, everything in, tree at `c70df7bb`)
Sheet: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-141300-B-LK1-looks-after.png` (read as a sheet; regular at 18 s, Marathon at 22 s, Shotgun at 11 s, Power Play at 11 s read at full size, and crops of Shotgun at 6 s, Glow Ice faceoff, Pond at 11 s, Fog of War at 18 s). What a stranger still sees, worst first:
1. **Glow Ice: the goal's dust is a grey-white smoke cloud**, the one thing in the finale that is not light (next section). B's effect, but it needs the tier from device A.
2. Night: the bullseye rings of the lamp pools (a look decision, not a fault of finish).
3. Pond: the stand blocks behind the far boards leave gaps of snow between them (the night layout); the stands cast no shadow.
4. Glow Ice: the high pink wall line has a visible step near both ends of the frame at the faceoff; the outline pales the orange figures a little close up.
5. Marathon: the corner behind the boards shows bare timber at the game camera's steepest pitch.
6. Not a fault: two soft white smears over the stands in a Shotgun corner view at 6 s are the goal's cream dust seen from close by (`C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-142601-B-LK1-clay-corner-goal-dust.png`, read); I first took them for a lamp's haze.

### A trial that was not kept: the goal's dust in the team colour (14:26 to 14:29)
- On Glow Ice the goal's dust (lane B45's cream-led `NS_goal`) reads as a **grey-white smoke cloud** over the net: the one thing in the finale that is not light.
- I tried the team colour at 1.6 on the chalk dab. On Glow Ice: a flat periwinkle cloud. In a real orange goal on clay: **brown smoke**, which is what B45 moved away from. Not kept: `NS_goal.uasset` is back byte-identical from my backup (`git status` shows no change under `Game`).
- Still: `C:\Users\kazda\jam\wt\b-aud\devlog\shots\20261115-142600-B-LK1-glowice-goal-as-shipped.png` (I read both source sheets: the shipped six frames and the trial).

## 3. Not verified
- The blast fired by a real F press in a match, with the aim turned, and on the night and neon tiers: every blast still is a hand-spawn straight down the ice on clay, world slowed to 0.05.
- The other three ability kinds re-filmed after the two patches (their emitters were not touched).
- Glow Ice with ten figures held still for 3 s as the trailer wants it, and a real scored goal there (the goal's dust was hand-spawned).
- The outline in light on Pinball, the other `l3_neon` mode.
- The day rig on the 120 and 140 m rinks (60, 70 and 90 m filmed); its frame time with the sun's three shadow cascades.
- The clay ice change on the 140 m Marathon rink from the pinned side view (game camera stills only).
- A cook, a packaged build, frame time of anything. Nothing was heard.
- The HUD over any of it: `HighResShot` stills carry no HUD.
- Ice blotches on Glow Ice: seen gone in two stills, not measured.

## 4. For the engine lane / for device A
1. **The goal on Glow Ice is grey smoke** (still above). An effect cannot tell the tier. Either the binder sends the tier to `NS_goal` (for example `User.Variant` 2 and 3 on `l3_neon` in `UJamEffectsBinder::GoalVariantFor`, with B adding a bright team-colour branch for those two values: about 20 minutes of B's work, a no-op until A sends them), or `MPC_Look` gets a scalar `Neon` that `ULookTierSwitcher` writes 1 on `l3_neon`.
2. **Trim on `l1_day`** is Emissive 30 (`LookTierSwitcher.cpp`, `GTierTrim`): in daylight a soft yellow line; 8 to 12 would sit better.
3. **Glow Ice**: the penalty boxes and their clocks are lit cream, the brightest things outside the play (`a_neon_t140`, in the looks-before sheet's source set). **Fog of War**: no fog is visible at `fog_density` 0.04.
4. **Crowd actors exist for rink lengths 56, 60, 64, 70, 80, 90, 120, 140 m.** A mode moved to any other length has an empty barn until B adds an owner (ten minutes with `crowd56.py`).
5. **The blast's flash sits round the shooter** because the blade is hidden from A's camera; if A wants it at the blade, the binder would need to send the blade's side.
6. For the figure lane (E4): nothing changed in figure Blueprints, takes or materials. The Glow Ice outline is a post-process line on every mesh of that tier, figures included.

## 5. Decisions for the operator
1. The blast: about 0.2 s of flash and streaks now; the wider, longer fan of hand-back 2 is one script away (`patch_sg.mjs`).
2. The clay ice scratches: 0.55 as shipped, or the cleaner 0.4 (third column of `20261115-140301-B-LK1-clay-ice-three.png`).
3. The Glow Ice outline colour: lilac now; white is one number.
4. The goal on Glow Ice: grey-white dust as on every tier, or a glow in the team colour (needs A, item 1 above).
5. Night's bullseye lamp pools: a look, kept.

## 6. The next three steps
1. With A's tier signal: the goal's burst in team-colour light on Glow Ice, proven beside `20261115-142600`. Proof: the burst's mean saturation over the net at +10 above 0.5 and no grey.
2. A real F press: the blast filmed in a Shotgun match with the aim turned 45 degrees and on night. Proof: the same share-of-frame row at +2, +8, +14.
3. The Pond's far stands closed up (two more stand blocks in the gaps) and the stands given a shadow. Proof: no snow visible between stand blocks behind the far boards at the faceoff.

## 7. Ledger rows (the 11 columns of `ASSETS.md`; replace the rows of the same path, add the four new ones)
| Game/Plugins/JamAssets/Content/Rigs/LR_P3_Clay.umap | other | 1.71 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | P3 clay rig, barn with calm ice: PPV (EV100 4.2, SSR override, focus 16 m f/4, no depth blur, motion blur 0), sky light a movable real-time capture (no cubemap; intensity 0.35 and colour unchanged) of the capture dome SkyCap_P3_Clay (engine sphere, scale 700, MI_SkyAmbient_Clay, outside the barn; B34), 4 shadowed warm key spots (5200 cd, no specular, shadow resolution 1.0 and source radius 60 cm since B34), 8 shadowless lamps over the side lanes (2100 cd, broad beams, no specular; B28), 6 shadowless fills at lower intensity (B28), 18 lamp shades (6 of them decorative beyond the end boards, twice as wide since B29), LookParams (FeltRim 1.4); plank walls, posts, trusses, roof, floor; no bulb strings; all shadowless meshes, no collision; the old ten trusses replaced by ten roof bays (SM_Barn_TrussBay, height scale 0.82), 32 wall bays in front of the plank walls and 12 lengths of bunting hung at 13.05 m, all instances on one actor Dress_P3_Clay that scales with the rig; timber stands and a clay crowd as instances on seven actors Crowd_P3_Clay_k0667, k0711, k0778, k0889, k1000, k1333, k1556 (class BP_ArenaCrowd, one layout in real size per rink length 60, 64, 70, 80, 90, 120, 140 m, each scaled by 90 / length so that only the one whose rink is loaded has scale 1 and stays; the others destroy themselves at BeginPlay): on the regular rink 56 stand pieces and 664 fans (twelve figures with a far level, per-instance cheer weight, coat value, accent value, end and delay, front row empty, corner rows three quarters full), the ice gate, its props, a loft of two bays and the ice machine at the +x end wall; no light added, no shadows, no collision (lanes B9, B14, B18, B19, B22, B23, B27, B28, B29, B34, AR2, AR5); job 2 of AR8: 18 lamp arms on Dress_P3_Clay (6 SM_Barn_LampArm_075, 12 SM_Barn_LampArm_225, placed from the rig real lamp shades, each on its lamp rod) and a sliding door, a firewood stack and a skate bench on the -x end wall on each of the seven crowd owners (lane AR8 job 2); since AR13 six floats per fan (0 weight, 1 coat, 2 accent, 3 end, 4 delay by the distance to the end net, 5 target A or B) and the hung scoreboard on the owners of the 80, 90, 120 and 140 m rinks (lane AR13); since LK1 an eighth owner Crowd_P3_Clay_k0622 for the 56 m rink of Shotgun and Bruisers (a copy of the 60 m owner at scale 90 / 56, refilled from lane AR13's own layout at 56 m: 36 stand pieces, 472 fans with the six floats, its six end-wall pieces moved in by 56 / 60; no hung scoreboard at that size), which destroys itself on every other rink as the seven do (lane LK1) (1789304 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L3_Neon.umap | other | 0.15 | original | UE 5.9 editor via MCP/Python (scratch neon.py; d3/lk1/neon1.py) |  |  | 2026-11-15 | B | y | the neon tier's rig (l3_neon): PPV_L3_Neon, Sky_L3_Neon, Spot_L3_Neon_1-4, Fog_L3_Neon; a lattice of lilac light lines on the ice at +/-30 deg 4 m apart clipped 2.3 m inside the boards, a 28-segment orchid frame line 2.2 m inside, four dark hall walls with magenta lines; no collision; since LK1 the post volume draws the outline in light (blendable MI_PP_Outline_Neon in place of MI_PP_Outline, the toon ramp unchanged), bloom 1.8 (was 1.3) and screen-space reflections (override, as on the night rig; it was the project default); seen on the GPU from the game camera in a running Glow Ice match (lanes G1, LK1) (162504 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Neon_BoardLine.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | instance of M_Trim, TeamColour orchid (0.85, 0.35, 1.0), Emissive 6 (was 20: it drew white and outshone the puck's trail); the frame line of LR_L3_Neon (lanes G1, LK1) (7658 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_PP_Outline_Neon.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | instance of M_PP_Outline with MI_PP_Outline's scalars (WidthMode 1, ColourMode 0, InnerLines 1) and OutlineColour and InkColour lilac (1.2, 0.9, 2.0): the outline as a line of light; the blendable of PPV_L3_Neon only, so figures, sticks, nets and the glass rail are drawn in light on Glow Ice; no other tier uses it (lane LK1) (9554 bytes) |
| Game/Plugins/JamAssets/Content/Rigs/LR_L1_Day.umap | other | 0.93 | original | UE 5.9 editor via MCP/Python (scratch d3/lk1/day1.py, day2.py) |  |  | 2026-11-15 | B | y | the day tier's rig (l1_day), filled from lane A0's placeholder: the night rig's dressing copied (snow field, six banks, four mounds, sky dome, four ground planes, capture dome, seven BP_ArenaCrowd actors with stands and fans per rink length, PPV) without its 150 night-only actors (spots, fills, dark room, truss, lamps, towers, haze cards); Sun_L1_Day a movable directional light 16 lux, colour 255/240/214, pitch -38 yaw 55, three cascades to 160 m, specular 0.5; Sky_L1_Day a real-time capture sky light 0.9; PPV_L1_Day EV100 4.2 fixed, bloom 0.35, vignette 0.12, saturation 1.05, toon ramp and outline as on night; snow on MI_Snow_Day, dome on MI_DaySky, capture dome on MI_SkyAmbient_Day, stands and fans on the clay tier's MI_ClayBarn_Stand, MI_ClayBarn_StandDark, MI_ClayCrowd_Skin, _Fan, _Scarf, _Tin; no new mesh, no collision; seen on the GPU from the game camera on Pond (60 x 30), Outnumbered (70 x 35) and the regular rink (90 x 45) (lane LK1) (980410 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Snow_Day.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | instance of M_Snow (unlit) for the day rig: SnowLit (6.0, 6.8, 8.0), SnowShade (3.0, 4.0, 6.2), FarColour (5.5, 6.6, 8.5), Glow 1, PoolScallop 0, Bands 3, BankLit 1.15, BankShade 0.8; on the snow field, banks, mounds and ground planes of LR_L1_Day (lane LK1) (10558 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_DaySky.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | instance of M_NightSky (unlit) as a day sky: SkyMode 1, no stars (StarBright 0, StarDensity 0), HorizonColour (3.2, 5.8, 11.5), ZenithColour (0.8, 2.4, 8.0), hills and tree line on (HillFarColour (3.0, 4.4, 7.0), HillNearColour (2.0, 2.9, 4.2)), GlowColour (4, 4, 3); on SkyB_L1_Day_Dome of LR_L1_Day (lane LK1) (14029 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_SkyAmbient_Day.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-15 | B | y | instance of M_SkyAmbient (unlit): UseFlat 1, FlatColour (2.0, 2.8, 4.4), Gain 1; the capture dome SkyCap_L1_Day that the day rig's sky light captures (lane LK1) (10208 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_ability.uasset | vfx | 1.67 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-15 | B | y | event ability on a use (Strength above 0.5), by User.Variant, small chalk dabs (M_Fx_Chalk): 0 shotgun a toy blast along User.Dir of three emitters: Flash, 2 near-white soft discs 170 to 220 cm round the shooter for 4 to 5 frames (new emitter, M_Fx_Chalk); Shards, 140 thin blue streaks (0.2, 0.45, 1.6) 9 x 300 cm at 60 to 140 m/s from a 85 cm sphere, velocity aligned, life 10 to 18 frames; Blast, 30 sky-blue dabs (0.45, 0.75, 1.7) 44 to 70 cm growing 1.8 times, 35 to 90 m/s, life 10 to 14 frames (4.71 % of the frame at +2, 2.27 % at +8, gone by +14; lane LK1 after the lead's ask), 1 hip check an ochre arc, 2 shield a slate-blue ring of 60 with white shards, still there at +45 (3.60 %), 3 slow puck a frost patch; a grant shows nothing (lanes B40, B42, FILM1, B43, B44, LK1) (1749253 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Ice_Night.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-15 | B | y | instance (B13 surface, B14 clay/broadcast, B15 tiles, NightWet 0.3; B19 clay ice; B23 pools, contact, board band; B27 calm clay ice: BulbReflB22 0, LampSheen 1.3, neutral pool tint, gloss 0.08; B28 body (0.32, 0.42, 0.97)); since LK1 the clay ice is calmer at the feet: ScratchAmt 0.55 (was 0.7), ScratchLift 0.28 (was 0.35), SnowAmt 0.3 (was 1: the pale band at the board foot 31 -> 19 luma over the ice), ScratchBump written at its default 0.6; all four are read on the clay tier only (lane LK1) (38171 bytes) |
