# B32 report: the slide trail redone, the hit effects in real knockdowns (device B)

Lane B32, device B, machine clock 13:11 to 13:36. Tree `wt/b-vfx` (`dev-b` 1592000), editor on port 8000 (pid 30708), left through `ExitEditor` with no dirty package; the process is gone (the remaining `UnrealEditor` pid 21692 is the skater lane's). **No `BP_SkaterVisual_*` saved** (both files still 11:57 from B30). No git, build or cook. Backup of the B30 trail: scratch `bk/NS_slide_trail.uasset`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b32` (`spec32.mjs`, `mksoft.py`, `real.sh` the drill-6 capture with a freeze-on-knockdown tick callback and one-frame stepping, `tk.py`).

## Verdicts
| Item | Verdict |
|---|---|
| 1. Snow part | **PASS**: small soft puffs, no rim, no shading |
| 2. Scrape | **PARTIAL**: a dark soft streak under the path now reads on clay and night, about **16 luma** below the clay ice (target 20); at full speed it can read slightly dashed; not a ribbon (flat quads did not render, see below) |
| 3. Night colour | **PASS**: darker on every non-clay tier, reads on lit and blue ice |
| 4. Real slides | **PARTIAL**: two real drill-6 knockdowns of the human (clay and night: the same knockdown, the drill is deterministic) captured frame by frame with the hit-stop; both ended in a pile of three figures near the boards, so the slide was short and under another figure: **the trail is not seen in a real open-ice slide**; the trail was checked on a test figure sliding 4.4 -> 1.7 m/s |
| 4b. Hit effects in real knockdowns | **PASS**: during the 3 held frames and after, only small low chips and puffs at the skates and the flat ring show; nothing over trunks or faces |
| 5. Blue ring on clay | **PASS, no change**: light-blue dots spreading on the lavender ice for about 10 frames; dotted and moving, it does not read as the solid blue line or the slow-ice zone ring |

## Values
**New material `VFX/M_Fx_SlideSoft`** (translucent, unlit, two-sided, Niagara sprites): colour = particle colour x `lerp(NightMul 0.55, 1, saturate(MPC_Look.Clay))` (darker on night and every non-clay tier); opacity = particle alpha x `(1 - r)^Soft` (Soft 1.6: a soft round edge, no rim, no shading) x depth fade 3 cm (soft where it meets the ice; no z-fight). Not the tier's toon particle master: that master is what drew the rim and the sphere shading.

**`VFX/NS_slide_trail`** (rebuilt; same asset, so the Blueprints' `SlideFx` keeps working), both emitters on `M_Fx_SlideSoft`, both silent below 1 m/s:
- `Spray`: every 0.05 s, `16 x saturate((speed - 100) / 400)` puffs (more at high speed), **4-8 cm**, colour **(0.55, 0.62, 0.78)** alpha 0.85, at 2 cm, thrown **backwards and sideways** (velocity origin 40 cm ahead along the body's motion, 80-240 cm/s), gravity -500, drag 4 (they stay low), grow 1.6x and fade over 0.3-0.5 s.
- `Scrape`: every 0.03 s, 2 soft quads **18-24 cm** at the body (radius 4 cm) at 3 cm, colour (0.22, 0.25, 0.34), alpha `0.4 x saturate(speed/300 + 0.4)` fading over **1.4-1.6 s**: overlapping, they make one soft dark streak. I first tried them lying flat on the ice (custom facing): **they did not render** (the same as B17 found), so they are camera-facing and softened into the ice by the depth fade.
- **Measured** (clay, game-like camera, test slide): ice median luma 196, streak 180 (about 16 darker); night: ice 221, streak 206 (15 darker).
- **Particle counts at full slide speed (live):** B30 about 50 spray + 170 flecks = 220; B32 about **160 spray + 107 scrape = 267** (up, mostly the many small puffs); at 2 m/s about half; 0 below 1 m/s.

## Real knockdowns (drill 6, `jam.hitstop 3`, game camera, frozen on the first tick the human's visual is down and stepped one frame at a time)
| Frame | Clay: what covers the two figures | Night |
|---|---|---|
| +0, +1, +2 (held) | the white contact ring (small, on the ice) and a few low chips at the skates: under 5 % of the figures | the same, a soft white puff at the skates: about 5-8 % |
| +3 | chips and two small puffs at skate height: about 5 % | about 5-8 % |
| +6 | a few chips and ring dots on the ice: under 3 % | under 3 % |
| +9, +12 | nothing over the figures | nothing |
These are by eye (no figure mask in a real match); the posed-figure measurement of B30 (0-8 %) stands. **What the 3 held frames show:** the attacker and the human frozen in contact, the flat ring opening on the ice between their skates, a few chips moving out at skate height; the effect frames the contact and does not cover it.

## Stills (`C:/Users/kazda/jam/wt/b-vfx/devlog/shots/`, each with `.txt`)
- `20261113-133529-B-b32-slide-before-after-clay.jpg`
- `20261113-133530-B-b32-slide-night.jpg`
- `20261113-132553-B-b32-real-knockdown-clay.jpg` (hit, +3, +6, +9, +12, +20, +31, +83)
- `20261113-133009-B-b32-real-knockdown-night.jpg`
- `20261113-132337-B-b32-hitstop-frames.jpg` (+0, +1, +2 held, +3)
- `20261113-133530-B-b32-ring-blue.jpg`
The brief's trail stills at +10/+20/+30 and after the get-up from a real slide do not exist: the real slide was under a pile (see `real-knockdown-*`, +20 and +31).

## Assets (bytes, `Game/Plugins/JamAssets/Content/`)
| File | Before -> after |
|---|---|
| `VFX/NS_slide_trail` | 513,984 -> 520,907 |
| `VFX/M_Fx_SlideSoft` | new, 14,954 |

Ledger rows (replace `NS_slide_trail`; add `M_Fx_SlideSoft`):
```
| Game/Plugins/JamAssets/Content/VFX/NS_slide_trail.uasset | vfx | 0.5 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | slide trail while a knocked-down figure slides: small soft blue-grey puffs thrown back and out and a soft dark streak under the path fading over 1.5 s, rate by the owner's speed (0 under 1 m/s), darker on night; played by SlideFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B30, B32) (520907 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_SlideSoft.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | translucent unlit soft round particle (no rim, no shading), depth-faded into the ice, darkened on non-clay tiers via MPC_Look.Clay (lane B32) (14954 bytes) |
```

## What I could not verify
- The trail on a **real open-ice slide** (both real knockdowns ended in a pile); a real slide at the boards.
- Flicker or dashing of the streak in motion (stills only; at full speed the overlapping quads may read as a dashed streak).
- Coverage numbers from real knockdowns beyond the eye estimate above.

## Decisions for you
1. Accept the streak at about 16 luma (target 20), or let B raise it once more (alpha 0.4 -> 0.5) after one real open-ice slide is seen.
2. A real open-ice slide would need a drill that knocks the human down mid-ice (drill 6 ends at the pile near the boards); worth asking A, or accept the test-figure check.
