# B31 report: clay tier pass 10: pickups, lying figures, slide trail, bumper, slow-ice ring (device B)

Lane B31, device B, machine clock 12:30 to 13:15 (2026-11-13). Tree `C:/Users/kazda/jam/wt/b-vfx` (`dev-b` f866666, read only). No git write, no build, no cook, **no frame numbers**.

**Two things about the editor that differ from the brief:**
- I ran two editor processes, both mine. The first (port 8000, pid 40368) I stopped by force at 12:39 without saving: my first approach had patched the shared master `Arena/M_PropCel` in memory, I then decided to leave that file alone, and the editor would neither reload it nor exit cleanly with it unsaved. `Arena/M_PropCel` on disk is unchanged (33,620 bytes, 11-12 15:07).
- After that stop, port 8000 stayed held by a dead listener (owner pid 0), so the second editor (pid 29796) ran on **port 8001**. It left through `ExitEditor` at 13:05:16 (`LogExit: Exiting.`, dirty list empty, process gone). The skater lane's editor on 8002 was never touched. Port 8000 is free again (checked at 13:06).

Housekeeping: `/Game/Maps/Rink` not saved (18,966 bytes, unmodified in git). Nothing of A's, `Skaters/` or `VFX/` changed. `jam.Cut hockey.regular`, `jam.ScriptHuman 0`, `jam.ctrl.Drill 0` set back. 0 `[VSM] Non-Nanite Marking Job Queue overflow` lines. `Game/Saved/Screenshots/WindowsEditor/` empty. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b31`.

Because of the restart, every still except the pickups before/after pair was taken from the saved assets in a fresh editor process.

## Verdicts

| # | Item | Verdict | Still |
|---|---|---|---|
| 1 | Pickups on clay, materials only | **PASS on clay; night not checked by a still** | `b31-pickups-before-after`, `b31-pickups-all-kinds` |
| 2 | Figures lying flat | **PASS** at all six positions (56 or more from the ice); contact shade weak away from mid ice | `b31-lying-mid`, `-wing`, `-corner` |
| 3 | Slide trail on the clay ice | readable but reads as bubbles; no scraped mark seen. Words for the effects lane below | `b31-slide-trail` |
| 4 | `MI_Toon_Bumper` Emissive | **DONE**: 2.0 → 0.7 | `b31-bumper` |
| 5 | Slow-ice ring colour | **DONE** | `b31-ring-slow-ice` |
| | Motion (pickups) | **PASS** | `b31-motion` |
| | Night stability | **PASS** where measured | `b31-tier-stability` |

## 1. Pickups

**What changed.**
- **Base** (`Look/M_Pickup`, instance `MI_Pickup_Base`): a new clay-only, default-off node `B31PickupBase` mixes the base colour toward an amber clay body. On `MI_Pickup_Base`: `ClayBodyMix` 0.8, `ClayBody` (0.80, 0.24, 0.03), `ClayPickupGlow` 0.18 → 0.07, `ClayPickupAmber` (1, 0.45, 0.12) → (1, 0.38, 0.06). The tan disc is now a low amber clay disc with a soft glow.
- **Props** (`Props/MI_PickupProp_<kind>`, six instances): stronger own colour and a warm emissive rim on clay: `ClayProp` 1, `ClayPropSat` 1.7, `ClayPropGlow` 0.3, `ClayRim` 1.6, `ClayRimPower` 1.6, rim colour (1, 0.55, 0.2).
- **One new master, `Props/M_PickupProp`.** The props' parent was the shared `Arena/M_PropCel` (also used by arena props). Rather than change it, I duplicated it with the two new clay-only nodes (`B31PropBase`, `B31PropEmis`, off unless `ClayProp` > 0) and re-parented the six instances. 422 pixel shader instructions against 396 for `M_PropCel`.
- `MI_Pickup_<kind>` (the tokens on `M_Pickup`) are not used by the pickups in a match today (the props and the base are); unchanged.

**Result on clay.**
- The base marks the spot from the far end as a small orange disc (`b31-hero-game`, far pickups).
- Each kind reads by its own colour with a warm edge; pickups stay small, raised and warm, the zone rings large, flat and one colour.
- Mirror image: the ice in front of the bases changes by +0.5 luma on average and +3 to +7 at the 95th percentile. My 99th percentile (up to +16 in one view) includes the base's own outline pixels and is not a clean number.
- Motion, 30 consecutive frames, held game camera in play: pickup crops 0.17 to 0.35 mean \|d\| (the props spin and bob), near ice 0.26. Nothing flickers.

**Not shown.** The `custom` kind: the cartridge loader refuses it without a declared effect. Its instance carries the same values.

**Night.** Both paths return the old value below `Clay` 0.5. I did not take a night still with pickups (the night cuts I ran have none), and I did not judge whether the change would help there.

## 2. Figures lying flat

Method: a scratch cartridge puts a blue and an orange default figure at mid ice, on a wing and in a corner; in a frozen frame every visual is set to the last frame of a knockdown take (lying; takes `Knockdown_Back`, `Fall_Forward`, `Knockdown_Left`, `Knockdown_Right`, `Knockdown_Left_B`); camera 12 m behind at game height. Team-coloured parts = jersey, helmet and socks together (they are one colour on this figure); dark parts = trousers, gloves, skates.

| Position | Team | Team-coloured parts / ice (difference) | Dark parts | Darkest ice beside the figure |
|---|---|---|---:|---:|
| Mid ice | blue | 80 / 198 (-118) | 30 | -37 |
| Mid ice | orange | 122 / 182 (-60) | 16 | -54 |
| Wing | blue | 48 / 178 (-130) | 25 | -11 |
| Wing | orange | 114 / 172 (-58) | 21 | +3 |
| Corner | blue | 49 / 171 (-122) | 22 | -11 |
| Corner | orange | 115 / 171 (-56) | 20 | -11 |

- **Every lying figure separates from the ice by 56 or more**, and the black trousers by about 150. Who is down and where reads at a glance in all six stills. No fix was needed.
- **Standing**, same frame before laying them down: all pass except one borderline: orange standing in the near corner 146 / 170 (**-24**, one under the target). Lowering the end fills and wall washes did not change it; I left the rig as it is.
- **Contact shade.** At mid ice a lying figure has a clear soft shadow (the keys). On the wings and in the corners there is next to none (0 to 11 under the ice). The figure's own contact plane is too faint to see under a lying figure there, and my ice contact shade from pass 5 only runs along the board foot. A stronger, larger contact under a lying figure has to come from the figure side.
- The jersey / helmet split asked for cannot be given on this figure by colour; I did not hand-mask them.

Seen while doing this: from cameras near the ends, the overhead lamps' mirror images show as pale scratch-broken patches, about +9 luma (184 against 175). Inside the limit; noted so it is not a surprise.

## 3. The slide trail

One real slide: `jam.ctrl.Drill 6`, the human checked into the side boards at x 38, y 22 m (strength 0.55), frozen 14 frames after it went down (`b31-slide-trail`).
- **Readable**, not dirty: a cluster of pale round blobs behind the body.
- **But it reads as soap bubbles or pebbles**, not as scraped snow: each particle is a shaded round disc with a visible edge, up to about 15 cm.
- **No scraped mark** is visible on the ice at the game camera or from 9 m.
- Only one slide, short and against the boards; a long open-ice slide was not seen.

Words for the effects lane (I changed nothing):
- **Size:** about half, 3 to 7 cm.
- **Shape:** flat soft puffs lying on the ice, no sphere shading and no outline.
- **Colour:** a slightly darker blue-grey than now, about (0.55, 0.62, 0.78), so it reads as shadowed snow on ice of luma 170 to 200.
- **Scrape:** one continuous streak under the body, 15 to 25 cm wide, about 20 luma darker than the ice, instead of separate flecks.
- **Lifetime:** 1.5 s is right.

## 4 and 5. Bumper and slow-ice ring

- `MI_Toon_Bumper`: `Emissive` 2.0 → **0.7**. On night it is a darker, shaded magenta drum; on clay a pink clay drum (`b31-bumper`). Device A's cap of 0.8 no longer acts.
- `MI_Zone_slow_ice_Ring`: `ZoneColour` (0.35, 0.65, 1.0) → **(0.25, 0.32, 1.0)**, a deeper, slightly violet blue; clearly apart from the other four (`b31-ring-slow-ice`).
- In this tree device A's code still puts the translucent `MI_Zone_<kind>` on the rings (the chaos cut on clay shows a thin pale ring). The rings in my stills carry the `_Ring` instances assigned at run time.

## Tier stability

Night (`l2_night`, chaos cut), before any change against after the last save:

| Still | Mean \|d\| | Tile max | Frame luma | Note |
|---|---:|---:|---|---|
| Boards 8 m | 0.49 | 1.43 | 60.61 → 60.60 | no bumper, no pickup in view |
| Game camera | 1.28 | 12.31 | 143.76 → 143.79 | the two bumpers changed on purpose; figure outlines |
| Bumper view | 0.73 | 7.07 | 79.38 → 79.36 | the bumper changed on purpose |

## Assets (bytes on disk, `Game/Plugins/JamAssets/Content/`)

| File | Before | After |
|---|---:|---:|
| `Props/M_PickupProp.uasset` | new | 39,209 |
| `Props/MI_PickupProp_speed.uasset` | 10,730 | 14,562 |
| `Props/MI_PickupProp_precision.uasset` | 10,931 | 12,451 |
| `Props/MI_PickupProp_big_puck.uasset` | 9,135 | 10,675 |
| `Props/MI_PickupProp_heavy_puck.uasset` | 11,221 | 12,708 |
| `Props/MI_PickupProp_reverse_other.uasset` | 9,378 | 10,881 |
| `Props/MI_PickupProp_custom.uasset` | 9,665 | 11,234 |
| `Look/M_Pickup.uasset` | 38,438 | 40,124 |
| `Look/MI_Pickup_Base.uasset` | 12,279 | 12,346 |
| `Look/MI_Toon_Bumper.uasset` | 8,714 | 8,775 |
| `Look/MI_Zone_slow_ice_Ring.uasset` | 7,142 | 7,204 |

## Ledger rows

The first row is **new**; the others **replace** the rows of the same path.

```
| Game/Plugins/JamAssets/Content/Props/M_PickupProp.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | master for the pickup props: duplicate of Arena/M_PropCel plus B31PropBase/B31PropEmis (stronger own colour and a warm emissive rim, clay tier only, off unless ClayProp > 0) (lane B31) (39209 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_speed.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (14562 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_precision.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (12451 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_big_puck.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (10675 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_heavy_puck.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (12708 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_reverse_other.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (10881 bytes) |
| Game/Plugins/JamAssets/Content/Props/MI_PickupProp_custom.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_PickupProp (B31; was M_PropCel), Emissive 5 (lane B11); ClayProp 1: stronger colour and warm rim on clay (B31) (11234 bytes) |
| Game/Plugins/JamAssets/Content/Look/M_Pickup.uasset | material | 0.04 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | pickup master (B10, B12) + B19PickupClay: warm glow on the clay tier only (B19) + B31PickupBase: amber clay body on clay where ClayBodyMix > 0 (B31) (40124 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Pickup_Base.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | pedestal, ring glow (B10); amber ring on the clay tier (B19); amber clay body with a soft glow on clay: ClayBodyMix 0.8, ClayPickupGlow 0.07 (B31) (12346 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Toon_Bumper.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | M_Toon instance, hot magenta, bumper obstacle (lane B10, R-0019); Emissive 2.0 to 0.7 (A-to-B-0016 point 1, lane B31) (8775 bytes) |
| Game/Plugins/JamAssets/Content/Look/MI_Zone_slow_ice_Ring.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | instance of M_ZoneRing (lane B29); a deeper, slightly violet blue (0.25, 0.32, 1.0) (lane B31) (7204 bytes) |
```

## What device A must know

- `A-to-B-0016` point 1 is done: `MI_Toon_Bumper` Emissive 0.7.
- In the real knockdown I froze, the lying human sat at the left edge of the game camera's frame, half cut off (camera at x 43.3, y 20.8, in the corner by the boards). One frame, one case; worth a look.

## What I could not verify

- Pickups on the night tier (no still; the paths are gated in the material code only).
- The `custom` pickup kind in a match.
- A long slide on open ice; the trail on other tiers.
- Lying figures in a real knockdown at each position (five of the six were posed, not knocked down); helmet and jersey separately.
- A cook (one new master, eleven changed assets); frame cost.
- Why port 8000 stayed held for some minutes after my first editor was stopped.

## Decisions for the lead

1. **`Props/M_PickupProp`**: accept the new master, or tell me to put the two nodes into `Arena/M_PropCel` instead and drop the duplicate (cheaper to cook, but it touches a shared arena material).
2. **Contact under lying figures on the wings and in the corners**: ask the skater lane for a stronger, larger contact plane when down.
3. **Slide trail**: pass the words above to the effects lane, or accept it as it is.
4. **Standing orange in the corners** is at the limit (-24). Accept, or a lane for the corners.
5. Pickups on night: leave off (as saved), or a look with stills.
6. Paste the ledger rows; cook check before pushing.

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with a `.txt`)

- `20261113-130515-B-b31-hero-game.png`
- `20261113-130515-B-b31-pickups-before-after.jpg`, `20261113-130515-B-b31-pickups-all-kinds.jpg`
- `20261113-130515-B-b31-lying-mid.jpg`, `20261113-130515-B-b31-lying-wing.jpg`, `20261113-130515-B-b31-lying-corner.jpg`
- `20261113-130515-B-b31-slide-trail.jpg`
- `20261113-130515-B-b31-bumper.jpg`
- `20261113-130515-B-b31-ring-slow-ice.jpg`
- `20261113-130515-B-b31-tier-stability.jpg`
- `20261113-130515-B-b31-motion.jpg`
