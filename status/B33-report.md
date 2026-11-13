# B33 report: the stick lift as a cross-check, the slide streak on night, the board crash by strength (device B)

Lane B33, device B, machine clock 15:26 to 16:23. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` c822183), editor on port 8002. No git writes, no build, no cook. Only `Game/Plugins/JamAssets/Content/VFX/` saved (five files); no `BP_SkaterVisual_*`, nothing under `Skaters/`, `/Game/Maps/Rink` not saved. Backups of the five assets as they were at 15:13: scratch `bk/`. Scratch (all scripts): `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b33` (`spec33.mjs` the four systems, `nb.mjs` the builder, `mksoft.py` the material, `tk.py` the test rig with a frame stepper and a real-match watcher, `posed.sh`, `slide4.sh`, `real2.sh`).

**Read this first: the editor crashed once.** My first editor (pid 31504) died at 16:11 with a GPU crash (`D3D12Util.TerminateOnGPUCrash`, device removed) during the real clay match. Nothing had been saved: every save I had issued ran while a play session was open, and none of them wrote a file (the save tool answered "Asset does not exist", the material's own save printed "ok" and wrote nothing). I restarted the editor (pid 30652, 16:16), rebuilt the four systems and the material from the same spec with no play session open, and they saved (16:17 to 16:18). So: **the stills of the cross-check and of the slide streak show the first session's builds; the saved assets are a rebuild from the same spec file.** After the rebuild I re-captured only the board frames, two posed lift frames (not zoomed) and one real clay knockdown. The saved slide trail and material were not looked at again. Editor left through `ExitEditor` at 16:20 with no dirty package; the process is gone (the one `UnrealEditor` left, pid 18428, is the sound lane's on port 8000; I did not touch it). Console variables `jam.ScriptHuman`, `jam.ctrl.Drill`, `jam.Cut` were set back to 0 / 0 / empty before the exit.

## Verdicts
| Item | Verdict | Still |
|---|---|---|
| 1. `NS_lift` as a cross-check, clean | **PARTIAL**: reads clearly on night; on clay the three amber cracks read on the figure but are thin, and the wood splinters are at the edge of visible at the game camera. Cracks last 3 frames; small chips cross the trunks until about +8 (the limit asked was 3 frames); everything is gone by +10. 20 particles against 72 for a body check with its knockdown | `b33-crosscheck-clay`, `b33-crosscheck-night` |
| 1b. `NS_lift` miss | **PASS**: two tiny chips at the blade for 5 frames (was six 25-40 cm puffs for half a second) | `b33-crosscheck-miss` (clay, posed only) |
| 1c. `NS_poke` in line | **PARTIAL**: rebuilt smaller (5 / 1 particles); seen in one posed frame per tier only | last tile of row 2 in `b33-crosscheck-night` |
| 2. Night streak | **PASS in light pools, one frame of evidence on dark ice**: a continuous dark blue-grey bar behind the body from +10 frames, readable at +20 from 24 m; continues through the pool's penumbra; on the dark blue ice it shows as a lighter grey bar in one real knockdown at +40. The pale snow edge is hardly noticeable. Clay colour path unchanged | `b33-slide-night-before-after` |
| 3. Clay streak continuous | **PARTIAL**: one continuous soft scrape, 14 to 18 luma below the ice (B32: 16); shown only in a test slide that an asset build disturbed, no clean re-run; flicker in motion not judged | `b33-slide-clay-before-after` |
| 4. `NS_board` skater variant | **PARTIAL**: Strength 0.5 = four chips and one small puff at the foot of the boards (5 particles, was 18); Strength 1.0 = 18 chips and four larger puffs. **The shake line on the board top does not show at Strength 1.0** in the saved asset's frames; cause not found | `b33-board-strength` |
| 5. Real matches | **PARTIAL**: a real clean lift on the human on both tiers (drill 6), with the human's check landing one frame later (so the check-plus-lift case is in the same frames). No real miss captured. Real knockdown streak: night at +40 on dark ice; on clay the knocked bot hardly slid and left no visible streak. Nothing remains at +100 on either tier | rows 3-4 of both `crosscheck` stills, row 3 of both `slide` stills, row 2 of `b33-overlap` |
| Worst-case particle count | **computed from spawn counts and lifetimes, not measured live**: about 465 (was about 575) | table below |

## How the binder tells clean from miss, and what the effect does with it
`VariantFor` gives `lift` / `poke` Variant 0 for `clean`, 1 for `miss`; Strength is 1 / 0; the burst is spawned at the victim's centre on the ice (clean) or at the attacker's blade (miss). **The binder sends no direction for `lift`** (`User.Dir` only goes to `check` and `knockdown`). The clean mark therefore cannot sit on the attacker's side: a single mark at the event point is inside the victim's trunk and is hidden (my first build did exactly that and showed nothing). The saved system puts three small cracks on a 26 cm ring round the point at 0.9 m, so some are always outside the body from any camera, and throws the splinters outward. `NS_lift` now has `User.Dir` (default zero): when A sends it the ring moves 30 cm towards the attacker and the splinters fly along the hit (posed frames `shots/aC-xd2.png`, `aC-xd4.png` in scratch; not filed).

## Values (60 fps; all from `spec33.mjs`)
**`NS_lift`** (was: 1 white star 70 cm at 0.3 m for 12 frames, 16 chips 6-11 cm for 0.3-0.45 s; miss 6 puffs 25-40 cm for 0.35-0.5 s)
| Emitter | Clean (Variant 0) | Miss (Variant 1) |
|---|---|---|
| Crack | 3 stars 18-26 cm, amber (1.0, 0.74, 0.22), ring 26 cm at 0.9 m, **0.05 s = 3 frames** | 0 |
| Wood | 6 splinters 4 x 13 cm, brown (0.42, 0.24, 0.09), velocity-aligned, 2.5-4.8 m/s, gravity, 0.12-0.17 s | 0 |
| Tape | 3 chips 4.5 x 6.5 cm, off-white, 0.12-0.17 s | 0 |
| Kick | 6 ice chips 4-6 cm at 4 cm, 0.12-0.17 s | 2 chips 3-4 cm, 0.07-0.09 s |
| Puff | 2 puffs 16-22 cm at 8 cm, alpha 0.4 fading, 0.12-0.16 s | 0 |

**`NS_poke`**: Clack 1 star 15-19 cm at 0.2 m for 2 frames, only when clean **and** Strength above 0.5 (the puck came off an opponent); Chips 4 (clean) or 1 (miss), 3-4.5 cm, 0.1-0.14 s. The old miss puffs are gone.

**`NS_board`**: puck variant unchanged from B30. Skater variant on `H = saturate((Strength - 0.5) x 2)` (0 at 4 m/s, 1 at 8 m/s) instead of `max(0.5, Strength)`: Body `4 + 14 H` chips, 3.5-5 cm growing to 5-10 cm; Puff `1 + 3.2 H`, 18-22 cm growing to 34-44 cm, alpha 0.7 fading; Shake 2 lines only above Strength 0.75 (does not show, see verdict). New user parameter `User.Normal` (the binder already sends it): chips and puffs leave the face into the rink. A real `board` event with Strength 0.446 was in the log, so H = 0 is the common case.

**`NS_slide_trail`**: Spray unchanged. Scrape: was 2 round quads 18-24 cm every 0.03 s; now **1 quad every 0.03 s, 26 cm wide and `30 + 0.11 x speed` cm long (80 cm at 4.5 m/s), velocity-aligned along the slide**, at 6 cm, alpha `0.36 x saturate(speed/300 + 0.4)` fading over 1.4-1.6 s. Consecutive quads overlap 3 to 6 times, so the gaps close at any tick rate down to 20 fps. Not spawn-by-distance: the builder has no call for that module. Camera-facing, softened by the depth fade: no flat quad, so nothing coplanar with the ice. Why flat quads did not render for B17 and B32 is recorded nowhere (both only say "did not render"); I did not test it.

**`M_Fx_SlideSoft`**: clay path unchanged (`lerp(..., PC, Clay)`, soft round falloff). Off clay, for the dark scrape particles only: colour `NightCol` (0.26, 0.32, 0.46) at the centre line blending to `EdgeCol` (0.80, 0.86, 0.96) between 35 % and 70 % of the half-width (`EdgeAmt` 0.85); opacity a band across the streak faded at both ends, times `NightOpacity` 2.0. The snow puffs keep B32's `NightMul` 0.55. All are material parameters.

## Coverage and counts
Figure pixels covered by the clean cross-check, clay, posed, game camera (B30's method, both figures together): hit **5 %**, +2 **7 %**, +4 **2 %**. Later frames and night: by eye only.

| System | Before | After |
|---|---:|---:|
| `NS_lift` clean / miss | 17 / 6 | **20 / 2** |
| `NS_poke` clean / miss | 9 / 6 | **5 / 1** (4 on a loose puck) |
| `NS_board` skater, Strength 0.5 / 0.75 / 1.0 | 18 / 20 / 24 | **5 / 13 / 24** |
| `NS_board` puck | 10-14 | unchanged |
| `NS_slide_trail` live at full slide speed | about 267 (160 spray + 107 scrape, B32) | **about 210** (160 + 50) |
| `NS_check` + `NS_knockdown`, Strength 1 | 49 + 23 | unchanged |

Worst case, two knockdowns and a cross-check overlapping: the bursts are 2 x 72 + 20 = **164** and are gone within 12 frames, while the two slide trails are still filling; the peak is about a second later with both trails full: 2 x 210 = 420, plus a lift 20 and a hard board crash 24 = **about 465** (before: about 575). Each further figure sliding at full speed adds about 210. All computed; no live count and no frame-rate number was taken.

## Stills (`C:/Users/kazda/jam/thirdjam/devlog/shots/`, each with `.txt`)
`20261113-162216-B-b33-crosscheck-clay.jpg`, `20261113-162417-B-b33-crosscheck-night.jpg`, `...162216-B-b33-crosscheck-miss.jpg`, `...162216-B-b33-slide-night-before-after.jpg`, `...162216-B-b33-slide-clay-before-after.jpg`, `...162217-B-b33-board-strength.jpg`, `...162217-B-b33-overlap.jpg`. The "before" slide frames were taken at 15:28 to 15:32, before any asset changed.

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Before -> after |
|---|---|
| `NS_lift` | 733,084 -> 1,192,018 |
| `NS_poke` | 736,361 -> 514,391 |
| `NS_board` | 967,241 -> 986,942 |
| `NS_slide_trail` | 520,907 -> 525,400 |
| `M_Fx_SlideSoft` | 14,954 -> 19,882 |
No asset created.

Ledger rows (replace the five existing rows; no new row):
```
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 1.14 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event lift as a cross-check: Variant 0 three amber cracks at stick height for 3 frames, wood and tape splinters, a small snow kick at the skates, gone by +10 frames, peak 20; Variant 1 two tiny chips; User.Dir added, default zero (lanes B3b, B17, B33) (1192018 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_poke.uasset | vfx | 0.49 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event poke: Variant 0 a 2-frame clack at blade height (only off an opponent, Strength above 0.5) and four chips, peak 5; Variant 1 one chip (lanes B3b, B17, B33) (514391 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_board.uasset | vfx | 0.94 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | event board: Variant 0 low puck chips; Variant 1 a skater's crash scaled by (Strength - 0.5) x 2: four chips and one small puff at the foot of the boards at 0.5, 18 chips and four larger puffs at 1.0, leaving the face along User.Normal; peak 24 (lanes B3, B17, B30, B33) (986942 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_slide_trail.uasset | vfx | 0.50 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-13 | B | y | slide trail while a knocked-down figure slides: small soft blue-grey puffs thrown back and out and one continuous soft streak of long velocity-aligned quads under the path fading over 1.5 s, rate by the owner's speed (0 under 1 m/s); played by SlideFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B30, B32, B33) (525400 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_SlideSoft.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | y | translucent unlit soft particle, depth-faded into the ice; on clay the particle colour with a soft round falloff; off clay the dark scrape is a mid blue-grey band with a pale edge and doubled opacity, the snow darkened (MPC_Look.Clay) (lanes B32, B33) (19882 bytes) |
```

## What I could not verify
- The saved slide trail and material after the rebuild (stills are from the first session's builds of the same spec); the saved lift only by two un-zoomed posed frames.
- The board shake line: absent at Strength 1.0 in two builds, present in my first build (threshold `saturate(Strength) > 0.7`); not explained.
- A real miss; a real poke; the miss and the overlap on night; the cross-check at more than one camera distance.
- The streak in a real open-ice slide of 3 to 5 m on either tier; on clay in any real slide; at +10 and +20 on dark night ice; a clean clay test slide.
- Flicker, shimmer or z-fighting in motion (stills only); frame cost; live particle counts.
- A small detached patch at the start of the night streak (one quad spawned before the owner has a velocity; seen once in `shots/aD-f20.png`).
- Cook: not run. Five VFX assets changed and `NS_lift` grew to 1.19 MB.

## Decisions for you
1. Cross-check size on clay: keep it this quiet, or raise the cracks to 26-34 cm and the splinters to 5 x 16 cm (they barely read at the game camera now). My recommendation: raise, after A sends the direction.
2. Splinters over the trunks until about +8 against your 3-frame limit: accept (they are 4-6 cm chips), or cut their life to 0.06 s.
3. Night streak weight: it is a bold bar about 26 cm wide in the pools. Keep, or set `NightOpacity` 2.0 -> 1.4. The pale edge adds little; raise `EdgeAmt` or drop it.
4. Board shake line: have it fixed, or drop it (B30 already asked whether to keep it).
5. Clay streak darkness is still about 16 luma (B32's open question): leave, or raise the scrape alpha 0.36 -> 0.45.
6. Process: saves issued during a play session do not write, and two jobs on one editor at once stall both (my lift build took 277 s instead of 49 s and froze the play clock). Worth a line in the house rules.

## What device A should send differently
1. **`Dir` for `lift` (and `poke`)**: the unit vector attacker -> victim in the payload, and `User.Dir` set by the binder as for `check`. `NS_lift` already reads it.
2. A drill that knocks a figure down in open ice at speed: in drill 6 the knocked bots hardly slide, so the streak cannot be judged in play.
3. Nothing else: `Variant` and `Strength` for `lift`, `poke` and the skater's `board` are enough as they are.
