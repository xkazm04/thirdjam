# B38 report: the puck's crescent ladder, the pale ovals, darker joints, the wide rinks' wall (device B)

Lane B38, device B, machine clock 18:41 to 19:25 (2026-11-13). Main tree `C:/Users/kazda/jam/thirdjam` (`dev-b` 95c2031, read only). Editor on port 8002 (pid 32260, mine, the only editor on the machine), left through `ExitEditor` at 19:19:40 (`LogExit: Exiting.`, no play session open, dirty list empty just before, process gone).

Housekeeping:
- No git write, no build, no cook. **No frame numbers.**
- `/Game/Maps/Rink` not saved (18,966 bytes). Both rig maps are unmodified in git. Nothing in `VFX/`, `Skaters/`, `Audio/` or of device A's changed.
- Console variables set back: `r.SSR.Quality 3`, `r.AntiAliasingMethod 4`, `r.Lumen.DiffuseIndirect.Allow 1`, `r.BloomQuality 5` (read back after each test), `jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.ctrl.Drill 0`. 0 "overflow" lines in the log.
- Scratch, with backups of every changed file in `bk/`: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b38`.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | The crescent ladder behind a fast puck on clay | **PASS: cause found and fixed on B's side, clay only.** Night ring unchanged (0.24 to 0.30) | `b38-ladder-tests`, `b38-ladder-ab`, `b38-ring-night` |
| 2 | The pale rimmed ovals near a scrum | **Finding: not the puff.** The large pale rimmed ovals beside the puck are prints of the puck's bright disc (item 1's cause; gone with its fix). The small rimmed discs are chip particles; two pale shapes are on the ice itself | `b38-ovals`, `b38-ladder-ab` row 3 |
| 3 | Joints 0.7 → 0.35 | **DONE** | `b38-joints-ab` |
| 4 | Wide rinks' dark upper wall | **PASS**: frame under luma 12 at the wide corner 10.8 % → 1.2 %; regular rink and Pond unchanged. Two wrong tries on the way, both corrected before filing (below) | `b38-wide-wall-ab` |

## 1. The ladder

**The cause, plainly.** It is Lumen's diffuse gather, not reflections. The gather's screen traces read the previous frame's picture, find the puck's very bright disc and light the ice around it. On static ice the gather keeps several frames of history, so every place the disc has just been keeps its light for a few frames. That lingering light is one long pale smear behind a fast puck; the dark rim round the disc (which exists on clay only) cuts the smear into one crescent per frame.

**The bright disc** is the mesh's `Halo` section (radius 0 to 30 cm) with `Look/MI_Trim_PuckHalo`: opaque, lit, emissive 18 x (0.75, 1, 0.15). **The dark rim** is the `Ring` section (radius 25 to 35 cm) with `Look/MI_Fx_PuckRing` (masked; its master draws it on clay and broadcast only). `MI_Fx_PuckMarker` and `MI_Fx_PuckBeacon` are a small post and a beam and play no part.

**The tests** (one real un-frozen match per test, the same moment each time: drill 6, 36 to 44 ticks after the first knockdown; console variables read back):

| # | Switched | Ladder |
|---|---|---|
| 1 | nothing | there |
| 2 | whole disc hidden | **gone** |
| 3 | bright disc (`Halo` slot) off | **gone** |
| 4 | dark rim (`Ring` slot) off | **one long bright smear instead** |
| 5 | beacon slot off; marker slot off | there |
| 6 | `r.SSR.Quality 0` (read back 0) | there |
| 7 | `r.AntiAliasingMethod 0` (read back 0) | there |
| 8 | `r.BloomQuality 0` | there |
| 9 | `r.Lumen.DiffuseIndirect.Allow 0` | **gone** |
| 10 | clay post volume: Lumen gather screen traces off | **gone** |
| 11 | clay post volume: gather update speed 4 | shorter, still there |
| 12 | disc emissive 18 → 6 | weaker, still visible; disc dim and greenish |
| 13 | bright disc drawn translucent, after depth of field | **gone**, disc as bright as before |

Tests 9, 10 and 3 name the cause; test 4 shows why it is a ladder and not a smear. A puck I moved by hand in a frozen match did not show it; only real play does.

**Why not the simple fixes.**
- Test 10 (screen traces off for the whole clay tier) removes the ladder but moves the clay picture: game camera 2.66 mean difference (an unchanged frame: 0.60), side boards 5 to 9 luma darker, side ice 139 → 144.
- Test 12 dims the ring and leaves a ladder.
- Making the disc itself translucent (test 13) would also remove night's glow, since both tiers share the slot.

**The fix (clay only, three small parts).**
1. **`Look/M_PuckGlow`** (new): unlit, translucent, drawn after depth of field, writes velocity. Emissive `Colour` (0.75, 1, 0.15) x `Glow` 18, the same light the halo had. Opacity 1 on clay, 0 on every other tier.
2. **`Props/SM_PuckDisc`**: `M_PuckGlow` is the **overlay material of the `Halo` slot**. Device A's puck actor picks it up with no code change (shown by the "after" frames, which use A's actor untouched).
3. **`Look/M_Trim`**: one new value `ClayGlowMul` (master 1 = unchanged) in `B23TrimEmis`; **0 on `MI_Trim_PuckHalo`**. On clay the opaque disc under the overlay no longer emits, so the gather finds nothing; the overlay, drawn after the picture the gather reads, supplies the brightness.

Night and broadcast: the overlay's opacity is 0 and `B23TrimEmis` returns its input unchanged below Clay 0.5, so the opaque halo is the disc exactly as before.

**Cost, to be honest about it.**
- `M_Trim` is not bit for bit: its pixel shader goes 221 → 223 instructions for every `M_Trim` user (the cap instances 313 → 315). The values are identical wherever `ClayGlowMul` is 1.
- The halo section (576 triangles) is drawn once more, translucent, on every tier (invisible off clay). `M_PuckGlow`: 153 instructions. Not measured.

**Proof.**
- Same moment before and after, clay: ladder gone, disc and rim as before (`b38-ladder-tests`, tile 14).
- Fast pucks of a real un-frozen clay match, before and after (`b38-ladder-ab`): the pucks leave at 9.27 / 13.30 / 18.90 s before and 9.28 / 13.30 / 18.93 s after, so they are the same shots, not identical frames. The first of the three was hidden behind figures in both runs; the still shows the knockdown moment and the other two. That is three fast pucks, one of them from the test runs.
- **Night ring, un-frozen, same sim ticks before and after:** whole frame 0.24 / 0.30 / 0.30, ring crop 0.59 / 0.47 / 0.40 (36, 40, 44 ticks). Unchanged.
- Night frozen, against B36's stills: boards 0.48, 24 m 0.78, game camera 0.51 outside figure outlines (unchanged scene: 0.49, 0.79, 0.50).
- Clay ice regions, game camera: all thirteen equal before and after (138 to 199).

**To know.**
- **Night has the same lag, much stronger:** a yellow-green smear some metres long behind every fast puck, before and after (`b38-ring-night`). I did not touch it (night is to stay as it is). The same overlay would remove it: one value in `M_PuckGlow`'s gate plus `ClayGlowMul` on night; it would also remove night's glow on the ice round the puck.
- On clay the faint glow on the ice round a resting puck is gone too; it was the same light.

## 2. The pale ovals near a scrum

Method: a real knockdown at centre ice, un-frozen frames 2, 6 and 24 ticks after it, three runs: effects as shipped; every hit effect hidden as it spawns (`NS_check`, `NS_knockdown`, `NS_lift`, `NS_shot` were hidden, four components); only `NS_knockdown` hidden. I edited nothing in `VFX/`.

**For the effects lane, precisely:**
- **The large pale rimmed ovals beside the puck are not effects: they are prints of the puck's bright disc.** When the puck moves more than its own width per frame, the lingering light of item 1 shows as whole pale ovals with the dark rim's outline instead of crescents (`b38-ladder-ab`, row 3, before). They are gone with item 1's fix (same row, after). This is very likely what the QA still `qa2-puck-ring-arcs` shows at its lower left.
- **Not the puff.** `VFX/MI_Fx_Puff` has `Shape` 3 on this build: `M_Fx_Tier`'s soft falloff branch, which sets the rim to 0. It draws no rim on any tier. It is used by `NS_board`, `NS_effect_start`, `NS_knockdown`, `NS_lift`. Hiding `NS_knockdown` alone changes nothing about the rimmed discs.
- **The rimmed discs are the chips.** The small pale-violet discs with a darker edge round a hit vanish when the effects are hidden and stay when only `NS_knockdown` is hidden. They are particles with `VFX/MI_Fx_Chip` (`Shape` 0, a disc) from `NS_check`, `NS_lift` or `NS_shot`. Why a rim on clay: in `FxTierEmissive`, shape 0 mixes `ClayRimCol` (0.1, 0.06, 0.035) into the outer part of the disc (from radius 0.74, `RimW` 0.1) by `ClayRim`, which is 0.55 on `MI_Fx_Chip`; the clay branch also shades the disc like a lit ball. `MI_Fx_Spark` has the same shape with `ClayRim` 0.6. To lose the rim: `ClayRim` 0 on those instances, or shape 3.
- **Two pale shapes are not effects at all.** With every hit effect hidden, a pale grey oval on the red line at the centre faceoff spot and pale crescents beside it are still there, in the same place 18 ticks later. They are on the ice (the centre spot and marks of the ice or rink), not particles. Whose they are (the ice material or device A's rink markings): not checked.
- **Not reproduced:** the white ring with a dark outline beside the human's blue ring in `qa2-clay-play-2.png`. Nothing in my runs drew it; by its look it is a marker ring of device A's, not a particle. Not checked further.

## 3. Joints

`MI_Toon_Board` `JointDark` 0.7 → **0.35** (the joint is now 65 % darker than the plaster; it was 30 %). Along the boards the frame moves by 0.65 (tile max 2.90); `b38-joints-ab`.

## 4. The wide rinks' wall

`M_ClayBarn`'s wall wash (`B26BarnWash`) now multiplies its height by the rig's scale when the rig is larger than layout size, for objects that stand on the floor (the walls and the posts). The rig's scale is read in the material as the world length of the mesh's local x axis; "stands on the floor" is the bottom of the object's bounds under 1.5 m. Hanging objects (lamp shades, gables, trusses) and rigs at or below layout size keep the fixed 300 cm: so Pond is deliberately untouched, as asked.

| Rink | View | Mean difference | Frame under luma 12 | Top of the frame (luma) |
|---|---|---:|---|---|
| Wide 140 x 70 | corner | 3.35 | 10.8 % → **1.2 %** | 5 → 14 |
| Wide | side board | 3.47 | 8.7 % → **0.1 %** | 10 → 20 |
| Regular 90 x 45 | corner | 0.47 | 1.7 % → 1.6 % | 15 → 15 |
| Regular | side board | 0.59 | 0.4 % → 0.4 % | 26 → 26 |
| Pond 60 x 30 | corner | 0.48 | 0.3 % → 0.3 % | 20 → 20 |
| Pond | side board | 0.49 | 0.1 % → 0.1 % | 32 → 32 |

Ice in the wide views: 125.6 and 146.6 before and after. The wide wall is now lit about as high as the regular rink's in proportion; its very top is still a little darker (14 against 15, 20 against 26).

**Two wrong tries, both mine, both caught by these same checks before anything was filed:**
1. First I stored each wall's own scale as per-object data in the rig and read it in the material. The data never reached the material, and the regular rink's walls came out 40 to 55 luma brighter. That state was saved for about seven minutes (19:05 to 19:12) and no still of it is filed. The rig save had in fact written nothing: `LR_P3_Clay.umap` is unmodified in git.
2. The second version tested "stands on the floor" against the object's position, which is the centre of its bounds; nothing changed on any rink.
The third version is the saved one. Left over from the first: an unused parameter `WashOwnScaleZ` in `M_ClayBarn`.

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Look/M_PuckGlow.uasset` | new | 11,339 |
| `Props/SM_PuckDisc.uasset` | 28,212 | 28,697 |
| `Look/M_Trim.uasset` | 24,965 | 25,774 |
| `Look/MI_Trim_PuckHalo.uasset` | 8,037 | 8,473 |
| `Look/M_ClayBarn.uasset` | 26,467 | 29,813 |
| `Look/MI_Toon_Board.uasset` | 18,747 | 19,079 |

`M_ClayBarn` pixel shader 319 → 331 instructions.

## Ledger rows

The first is **new**; the others **replace** the rows of the same path.

```
| Game/Plugins/JamAssets/Content/Look/M_PuckGlow.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | the puck's bright disc on the clay tier: unlit, translucent, after depth of field, writes velocity; Colour x Glow 18; opacity 1 on clay and 0 elsewhere (B38GlowGate). Overlay material of SM_PuckDisc's Halo slot, so Lumen's gather does not pick the disc up and leave a trail (lane B38) (11339 bytes) |
| Game/Plugins/JamAssets/Content/Props/SM_PuckDisc.uasset | mesh | 0.03 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | puck halo disc, 60 cm x 4 mm (engine cylinder vertices rescaled), material MI_Trim_PuckHalo, no collision; Halo slot overlay material M_PuckGlow (lane B38) (28697 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Trim.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | master material (painted-cel look) + clay cap path B23TrimBase/Emis/Rough: matte painted cap without team glow where ClayCap > 0, Clay >= 0.5 only (B23); B34CapChips behind the static switch CapChips (default off): chipped paint on the clay cap, base colour only (B36); ClayGlowMul (master 1): emissive scale on clay for instances without ClayCap (B38) (25774 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Trim_PuckHalo.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; ClayGlowMul 0: on clay the opaque halo does not emit, its light is drawn by the overlay M_PuckGlow (lane B38) (8473 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_ClayBarn.uasset | material | 0.03 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | barn wood master: world triplanar grain, dents, roughness (lane B19); B26BarnWash: faint warm emissive wash rising from the floor, off by default (lane B26), its height follows the rig's scale above layout size for walls and posts (lane B38); B36FloorSpill: warm fill of the floor outside the rink's footprint, off unless SpillK > 0 (lane B36) (29813 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Board.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python (recipe tools/skills/look-painted-cel.md) |  |  | 2026-11-13 | B | y | instance; kick plate and rail band (B13-B15); clay: panels with joints every 2.44 m, red band (B19); static switch BoardWear on, PanelVar 0.15, WearTint (0.9, 0.8625, 0.7805), WearDirt 0.25 (B36); JointDark 0.35 (B38) (19079 bytes) |
```

## What I could not verify

- A cook and a packaged build. In particular whether a per-slot overlay material on a static mesh behaves the same in a package; the new master and the changed mesh need the cook check.
- Frame cost of the overlay draw and of the two extra instructions in `M_Trim`.
- The saved assets in a fresh editor process (saves returned true, the dirty list was empty, the after-frames come from fresh play sessions after the saves).
- The ladder on other rinks, in a network match, and on the broadcast tier (not run; the gate leaves broadcast as it was).
- Whether the overlay disc holds up under the game's pause, replay or any camera other than the game camera.
- Item 2: the white ring of the QA still; whose the static pale shapes on the ice are; the corner scrum of the QA still (my knockdown was at centre ice).
- The wide arena beyond two views; Marathon and the large arena (rig scales 1.556 and 1.333) not run.
- Motion over 30 frames for the wall and the joints: not taken in this lane.

## Decisions for the lead

1. **Night's smear behind a fast puck:** leave (night stays as it is), or apply the same overlay there. It would also remove night's glow round the puck.
2. **`M_Trim` is two instructions heavier for all its users.** Accept, or ask for a static switch as on the caps (one more permutation for the halo instance).
3. **The effects lane:** pass section 2. The fix there is theirs (`ClayRim` on `MI_Fx_Chip` and `MI_Fx_Spark`).
4. **The pale grey oval at the centre spot and the crescents beside it:** ask device A whether they are its rink markings, or give me a lane to look in the ice material.
5. Paste the ledger rows; cook check before pushing (new master, mesh and two masters changed).

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with a `.txt`)

- `20261113-191934-B-b38-ladder-tests.jpg`
- `20261113-191934-B-b38-ladder-ab.jpg`
- `20261113-191934-B-b38-ring-night.jpg`
- `20261113-191934-B-b38-ovals.jpg`
- `20261113-191934-B-b38-joints-ab.jpg`
- `20261113-191934-B-b38-wide-wall-ab.jpg`
