# B45 report: the goal's dust cream-led, the dive calmer, the logo reimported (device B, GPU)

Lane B45, device B, machine clock 17:07 to 18:00. Tree `C:\Users\kazda\jam\wt\b-aud`: `git merge --no-edit dev-b` fast-forwarded 8289793a -> a5e48823 (no conflict), then built (`ThirdJamEditor Development` PASS, 160.8 s). `GPU-FREE.txt` deleted at the start, rewritten at the end ("free from B45 at 18:00"). Two GPU editor sessions on port 8006, one after the other (pid 22464, then pid 10052 for the final goal runs), both left through `ExitEditor` with no dirty package, both gone; no other editor was running. Cvars back: `jam.ScriptHuman 0`, `jam.ctrl.Drill 0`, `jam.ctrl.HandPuck 0`, `jam.Cut` empty, `r.Editor.Viewport.OverridePIEScreenPercentage 1`, `Slate.bAllowThrottling 1`. Changed in the tree: `VFX/NS_goal`, `VFX/NS_dive`, `UI/T_UI_Logo_Hokej`; nothing under `Skaters/`; two temporary assets (`UI/B45Tmp/WBP_B45Logo`, `UI/B45Tmp/M_B45UiPlane`) were made for the logo still and deleted with their folder. No commit, push or cook. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/b44` (`patch45.mjs`, `rgoal.sh`, `stg.sh`, `dive.sh`, `mask.py`, backups `bk/NS_goal_b45_before.uasset`, `bk/NS_dive_b45_before.uasset`).

**How measured:** as in B44. Real goals: the shots drill (`jam.ctrl.Drill 10`, `jam.ctrl.HandPuck 1`, `jam.ScriptHuman 1`); a watcher finds the binder's `NS_goal` component (on the world settings) the tick it appears, fixes the camera on that net (centre ice: 43 m from the net at 5.6 m high; 14 m: at 5.6 m high), takes un-paused `HighResShot` stills at +10, +40, +70, and freezes the world at +100 (`slomo 0.0001`) for a pair: the frame with the dust and the same frame with its component hidden; a pixel counts when a channel differs by more than 12 of 255 after a 1 px blur. Frames 1310 x 755. Particle counts from the sim cache. Every still named here I looked at.

## Verdicts
1. **`NS_goal`: cream-led, keep as tuned for the pre-freeze merge.** Seen in real scored goals: it reads to +100 on clay and night, from the game camera 19.42 % of the frame at +100 on night (asked: 2 % or more), stays above the boards, does not reach the ice or the crease.
2. **`NS_dive`: calmer, keep as tuned.** On the real `Dive_Block_Left` take the grey dust bank competed with the lying figure; halved it reads as a light haze with the white chips.
3. **`T_UI_Logo_Hokej`: reimported as asked, settings read back, letters opaque in an engine still.** Merge.
4. Everything else from B44 stays as merged.

## 1. `NS_goal` cream-led
**Change** (emitters `Left` and `Right`, `InitializeParticle` Color; life, alpha, gravity, counts and the fountain unchanged): `float4(lerp(float3(2.6, 2.45, 2.1), User.Colour.rgb * 2.383, 0.2), 0.95)`: a pale cream above 1 so it holds in the night tier's low light, a fifth of the scoring team's colour as the accent (blue scoring: grey-white; orange: warm beige). Before: `lerp(User.Colour.rgb, white, 0.35)` (team-led: blue-grey on clay, navy on night, not seen after +70 on night in B44). A first try (cream 2.2 / 2.0 / 1.6, a quarter team) read khaki on clay; the saved value is paler (staged pair at 14 m, `20261114-175502-B-b45-goal-cream-a-vs-b.png`). Compiles UpToDate, no errors, no warnings, saved.

**Real scored goals with the saved value** (variant 0, the human's team scores, puck at x 4112 to 4118 cm; live particles 94 at +10, +40, +70; 72 to 79 at +100):
| Where | Frame share at +100 (frozen pair) | What I saw |
|---|---:|---|
| Night, game camera | **19.42 %** | grey-white dust over the end boards and the stands at +10 to +100, cream streaks over the net; the ice in front clear; the haze partly covers a blue figure standing at the corner boards |
| Night, centre ice | 4.69 % | pale puffs over the far end and the stands to +100 |
| Night, 14 m | 24.74 % | big pale puffs over both sides of the stands, not over the net mouth; changed pixels: above the boards apart from a thin band at the board base and one fountain streak at the crossbar (`20261114-175505-B-b45-goal-night-14m-changed-pixels.png`) |
| Clay, centre ice | 5.64 % | white-cream puffs over the far end, in front of the stands, to +100 |
| Clay, 14 m | **no real scored goal came** in two waits (3 and 5 minutes); my own spawn at the net, frozen, orange colour: 36.6 % at +40, 30.3 % at +100, cream puffs over the stands |

Not muddy under the goal lamp: the lamp is a small red light on the boards; no red in the dust in any frame. Nothing on the ice outside the crease: the rows below the boards change by 0.0 to 1.9 % in every pair (the fountain's falling streaks near the net and the dust's lower edge at the board base). The goal pause does not freeze it (counts move through the pause, as in B44). Stills: `20261114-175500-B-b45-goal-real-night-gamecam.png`, `20261114-175501-B-b45-goal-real-centre-14m.png`. With the first cream (real goals, conceded variant, the other net): night centre 3.05 %, night 14 m 18.50 %, clay centre 3.97 %, clay 14 m 19.83 % at +100 (`20261114-175503-B-b45-goal-first-cream-real.png`).

## 2. `NS_dive` calmer
Seen on the real take: `ke * SK26Test 1` (`Dive_Block_Left`) on the standing human, `NS_dive` attached at the root on take frame 9 with `User.Dir` = the pawn's facing, frozen at frame 13, clay. As merged, a grey cloud bank lies beside the diving body and covers its feet from the side. **Change:** the three `LandDust` emitters 4 -> 2 dabs each, alpha 0.8 -> 0.55 (chips and the trail unchanged). Particles at frame 13: 37 -> 31.
| | Game camera | Side camera 6 m | Figure's pixels changed (side) |
|---|---:|---:|---:|
| As merged | 0.75 % | 2.26 % | 12.3 % |
| B45 | 0.22 % | 1.47 % | 10.6 % |
What I saw: B45 is a faint haze and white chips at the body; the figure reads first. Still `20261114-175504-B-b45-dive-calmer.png`.

## 3. The logo (the lead's extra)
Ran `C:\Users\kazda\jam\ui-lab\work\imp_logo.py` (read first; with `import unreal` added) through `execute_python`: reimported `/JamAssets/UI/T_UI_Logo_Hokej` from `C:\Users\kazda\jam\ui-lab\out\u2\T_UI_Logo_Hokej.png` (660,704 bytes). Read back: saved True, 920 x 716, `TC_BC7`, `TEXTUREGROUP_UI`, `TMGS_NO_MIPMAPS`, sRGB True, source file as above. Asset 1,066,698 -> **659,897 bytes**. The PNG's alpha: 312,776 px at 255, 19,819 px between 1 and 254 (the anti-aliased edge), 326,125 px at 0. Still: the logo on an unlit translucent plane over `T_UI_MenuBackdrop` at half brightness, about 75 % of the frame high, in a play session (a widget could not be created from Python in this build, so a plane as the brief allowed). What I saw: HOKEJ reads, the letters are solid ice-blue inside with a pale rim, the O counter and the K and E gaps stay open, nothing of the backdrop shows through a stroke. `20261114-172000-B-b45-ui-logo-engine.png`. (The black block at the top of that frame is a rink mesh; the backdrop may be drawn flipped on my plane: not a property of the asset.)

## Not verified
- A real scored goal on clay from 14 m with the saved colour (none came; staged instead).
- The orange team's accent in a real goal with the saved colour (the real goals were blue scoring; orange only staged).
- The dive with `User.Dir` from the figure lane's wiring (I passed the pawn's facing; the dust lay to the side of the body because the body dives left).
- Any other system after the merge (only these two seen today).
- Cook: not run.

## For the engine lane / device A
- `NS_goal`: no change to the call. The binder's `User.Colour` now tints a fifth; the scoring team still reads in the accent, the lamp and the caption.
- `NS_dive`: no change to the call (take frame 9, attached at the root, `User.Dir` = the facing toward the shooter).

## Decisions for the operator
1. The goal's dust from 14 m covers a quarter to a third of the frame over the stands for under two seconds: keep, or a third fewer puffs (`20261114-175501-B-b45-goal-real-centre-14m.png`, `20261114-175505-B-b45-goal-night-14m-changed-pixels.png`).
2. The accent at a fifth makes a blue goal grey-white and an orange goal beige; more team colour would read the team but brings back the navy on night (`20261114-175500-B-b45-goal-real-night-gamecam.png`, `20261114-175502-B-b45-goal-cream-a-vs-b.png`).

## The next three steps
1. A real scored goal on clay from 14 m and one by orange with the saved colour. Proof: at least 2 % of the frame at +100 and no changed pixels on the ice below the board base beyond 2 % of those rows.
2. The dive with the figure lane's `User.Dir`. Proof: the dust's centre within 40 cm of the line across the body's front at frame 13.
3. The goal's 14 m share brought to 15 to 20 % if the operator asks (counts 34 -> 24 per side). Proof: the same frozen pair.

## Verdict per system for the pre-freeze merge
| Asset | Verdict |
|---|---|
| `VFX/NS_goal` | merge as tuned (seen in real goals, clay and night) |
| `VFX/NS_dive` | merge as tuned (seen on the real take) |
| `UI/T_UI_Logo_Hokej` | merge (settings read back, seen in an engine still) |
| The other B44 systems | unchanged, as merged |

## Ledger rows (replace these three rows)
```
| Game/Plugins/JamAssets/Content/VFX/NS_goal.uasset | vfx | 0.67 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | event goal: cream-led chalk dust (cream 2.6/2.45/2.1 with a fifth of User.Colour as the accent) thrown from the net up over the end boards, 114 to 126 frames of life, and a cream fountain of streaks; Variant 1 at 60 % of the count; seen in real scored goals: 94 live to +70, 72 to 79 at +100, 19.42 % of the frame at +100 from the game camera on night, 4.69 / 5.64 % from centre ice on night / clay, above the boards, the ice clear (lanes B3, B42, B43, B44, B45) (705737 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_dive.uasset | vfx | 1.88 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | the dive block, attached at the root on Dive_Block frame 9: 25 streak chips across the body's front and 6 light dabs (alpha 0.55) along User.Dir, then a light trail by the owner's speed; 31 live at frame 13; 0.22 % of the frame at the game camera, 1.47 % from the side on the real take; seen on the GPU (lanes B43, B44, B45) (1970238 bytes) |
| Game/Plugins/JamAssets/Content/UI/T_UI_Logo_Hokej.uasset | texture | 0.63 | original | Blender (brand lab BR4) + Pillow scripts (ui-lab kit.py, logo_opaque.py) |  | C:/Users/kazda/jam/ui-lab/out/u2/T_UI_Logo_Hokej.png; C:/Users/kazda/jam/ui-lab/work/imp_logo.py | 2026-11-14 | B | y | 920x716 RGBA, UI group, no mips, BC7, sRGB, never streamed; letters opaque inside (pinholes filled, frost kept as colour over a solid ice base), counters and gaps open, 2 px anti-aliased edge; reimported and seen over T_UI_MenuBackdrop on the GPU in B45 (lanes U1, U2, B45) (659897 bytes) |
```
