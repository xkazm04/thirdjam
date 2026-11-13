# Lane SK20 report: the figures look blocky in play

Tree `C:\Users\kazda\jam\wt\b-vfx`, port 8000, one editor (pid 4036), exited at 20:34:52 and confirmed gone. Report written 20:36.

## Read this first

- **The blockiness is not fixed by anything of B's, and cannot be.** The cause is the resolution the game is drawn at, which is the other device's setting. The line for them is below. I did not change it.
- **What I did change is the number on the clay patch** (two material files), so it reads in play at the present setting. This changes the look of the clay figure's number everywhere, the banner figure included. It is your call to keep it.
- **I lost 16 minutes (20:18 to 20:34) to a file search over the worktree that stalled.** The test it cost is the number's readability beyond 7.5 m, which is not measured.
- A spec file of mine landed in `C:\Users\kazda\jam\thirdjam` for under a minute through a working-directory slip. I moved it out at once; `git status` there showed nothing afterwards.

## The cause

The game is drawn at 1398x790 and enlarged to the 1920x1085 window, 72.8 % per axis. TSR is meant to rebuild the missing detail from earlier frames, but it throws its history away on the moving figures, so on them you see the 790-line picture enlarged.

- **Where the 72.8 % comes from:** `r.ScreenPercentage` is 0 and `r.ScreenPercentage.Default.Desktop.Mode` is 1, so the engine picks the fraction from the display size. Nothing in the project sets either.
- **The test that showed it first:** `r.ScreenPercentage 100` at the same un-frozen moment. The steps are gone (cell 11 of `sk20-blocky-tests`). `r.AntiAliasingMethod 0` at the same moment is crisp too (cell 10).
- **The number:** read from a GPU profile in the play session, pass name `TemporalSuperResolution(sg.AntiAliasingQuality=3) 1398x790 -> 1920x1085`.
- **The history rejection:** `ShowFlag.VisualizeTSR 1` at the same moment shows the figures red in the "History Rejection" panel while the boards are mostly kept.
- **It is the game's own path, not only the editor's:** with `r.Editor.Viewport.OverridePIEScreenPercentage 0` the play session takes the game's screen-percentage path and still draws 1398x790. Adding `r.ScreenPercentage.Default.Desktop.Mode 0` gives `1920x1085 -> 1920x1085`.

### Why the frozen stills do not show it

A frozen figure does not move, so TSR keeps its history and gathers the picture to full detail over the next frames. Shown in `sk20-frozen-vs-play`, same moment (6.017 s), same setting:

| Frame | Look |
|---|---|
| Un-frozen | stepped |
| First frame after freezing with `slomo 0.001` | as stepped as play |
| Same freeze about 5 s later | crisp, crisper than play at 100 % |

A true pause (`set_game_paused`) did not sharpen in the one capture I took: the picture stayed as the last frame. The earlier frozen stills were made with time dilation, which is why they are crisp.

### Every candidate, tested

Method for all: clay regular match (seeded, scripted human), a fresh un-frozen play session per switch, one switch set at start and reset afterwards. A slate tick callback issues console `Screenshot` at match times 6.017, 9.017 and 12.017 s. Two sessions with the same setting differ by 0.2 of 255 on average, so the moments match.

| Candidate | Switch | Result |
|---|---|---|
| The capture itself | console `Screenshot` and the MCP capture | both 1920x1085, the window's own pixels, no rescale; not the cause |
| Screen percentage | `r.ScreenPercentage 100` | **steps gone** |
| Screen percentage | `r.ScreenPercentage 200` | sharper still |
| Anti-aliasing | `r.AntiAliasingMethod 0` | crisp, aliased edges (drawn at 1920x1085) |
| Anti-aliasing | `r.AntiAliasingMethod 2` (TAA) | still stepped |
| TSR history | `r.TSR.History.ScreenPercentage 200` | no change |
| TSR rejection | `r.TSR.ShadingRejection.Flickering 0` | no change |
| Velocity | `r.Velocity.EnableVertexDeformation 1` | no change |
| Motion blur | `r.MotionBlurQuality 0`, `r.MotionBlur.Amount 0` | no change |
| Texture streaming, mip bias | `r.MipMapLODBias -2`, `r.TextureStreaming 0` | no change |
| Felt normal and fibre | `FeltBump 0`, `FeltMix 0`, `FeltMottle 0` on every figure material | flat cloth, steps stay |
| Rim | `RimStrength 0`, `RimNight 0` on every figure material | darker, steps stay |
| Dither, depth offset, position offset, mask | read from the materials | none used; all figure materials are opaque |

The felt and rim switches were set on play-session material instances only; nothing was saved from them.

Read back before any switch: `r.ScreenPercentage` 0, `r.ScreenPercentage.Default` 100, `r.ScreenPercentage.Default.Desktop.Mode` 1, `r.TSR.History.ScreenPercentage` 100, `r.SecondaryScreenPercentage.GameViewport` 0, `r.AntiAliasingMethod` 4, `r.DynamicRes.OperationMode` 0, `sg.ResolutionQuality` 0, `r.Editor.Viewport.OverridePIEScreenPercentage` 1, `r.Editor.Viewport.ScreenPercentageMode.RealTime` 1. The display is 3840x2400 at 250 % scaling; the play window is 1920x1085 real pixels.

## The line for the other device

In `Game/Config/DefaultEngine.ini`, section `[/Script/Engine.RendererSettings]`:

```
r.ScreenPercentage.Default.Desktop.Mode=0
```

With `r.ScreenPercentage.Default` at 100 as it is, the game is then drawn at the window's size.

- **Proof:** `sk20-figures-play-ab` right column and `sk20-night-ab` right column, both with the GPU profile reading `1920x1085 -> 1920x1085`.
- **How it was tested:** as console variables in the play session, together with `r.Editor.Viewport.OverridePIEScreenPercentage 0` so the session used the game's path. Not tested as an ini line.
- **Editor play sessions are a separate setting.** They keep the editor's own viewport choice unless `r.Editor.Viewport.OverridePIEScreenPercentage` is 0 as well, so stills taken in the editor stay at 72.8 % with the ini line alone.
- **Cost, on this machine, in the editor:** clay 54.0 to 51.0 frames a second, night 54.7 to 53.8 (ticks per wall second over 6 s of match, one run each). I do not know whether a cap holds the higher figures down.
- **`r.TSR.History.ScreenPercentage=100` (lane A65) can stay.** Raising it to 200 changed nothing at the present setting and little at 100 %.

## What I changed

Both saved at 20:13 with no play session open (the save script checks for a game world first).

| File (under `Game/Plugins/JamAssets/Content/Skaters/`) | Bytes before | Bytes after | Change |
|---|---|---|---|
| `M_SkaterTB1Jersey.uasset` | 73,597 | 73,444 | JerseyDeco node: `NumberOn` above 1 draws the digits' fill in dark ink (0.02, 0.02, 0.03) and drops the atlas outline; 0 and 1 behave as before |
| `MI_SkaterTB1_P3_TrimDeco.uasset` | 15,681 | 15,098 | `NumberOn` 1 to 2, `NumberWidth` 0.3 to 0.42, `NumberHeight` 0.5 to 0.7 |

Not changed: the two Blueprints (4,003,455 and 4,207,480 bytes), textures, anything outside `Skaters/`.

End check: 739 assets load, 16 Blueprints up to date, no dirty package; `jam.Cut` `hockey.regular`, `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.hitstop` 3; every console variable I switched read back at its starting value. The 93 screenshot files in `Game/Saved/Screenshots/WindowsEditor/` and the two scratch cartridges are deleted.

## The number on the back

The limit was the patch, not only the resolution: the digit was a pale fill with a thin dark outline on a cream patch. On a moving figure the outline is lost and the fill has almost the patch's colour.

Distances are camera to figure, read in the same tick as each screenshot. The game camera held the 7 at 7.0 to 7.5 m in all three moments.

| Match time | Figure, distance | Before | After, same setting |
|---|---|---|---|
| 6.017 s | 7 at 7.5 m | readable, pale (figure nearly still on screen) | clear |
| 6.017 s | 21 at 5.9 m | broken up | clear |
| 9.017 s | 7 at 7.1 m | not readable (skating) | readable |
| 9.017 s | 9 at 6.3 m | not readable | a dark 9, rough |
| 12.017 s | 7 at 7.0 m | faint | clear |
| 12.017 s | 9 at 7.2 m | not readable | readable |

- So before the change the 7 read by how still it was, not by distance. After it, the 7 reads at every distance captured, 7.0 to 7.5 m.
- I tried three other forms and kept the fourth. Ink over the outline as well turned 21 into a blob like 81 and 9 into a block. Ink on the fill alone at the old size was too faint at 7 m.
- With the other device's line the digits are cleaner again (`sk20-number-ab`, top right).

## The night figure

Same fault, same cause: stepped outlines and board edge at 1398x790, clean with the line (`sk20-night-ab`). Its number (white with an outline on the jersey) reads in both. Nothing of B's changed on it: the night frame before and after my material edit differs by 0.16 of 255 on average, the run-to-run level.

## Stills

In `C:\Users\kazda\jam\wt\b-vfx\devlog\shots\`, each with a `.txt`:

- `20261113-201727-B-sk20-blocky-tests.png`: the same un-frozen moment under twelve switches
- `20261113-201728-B-sk20-frozen-vs-play.png`: un-frozen, first frozen frame, the freeze 5 s later, un-frozen with the line (not asked for; it is the proof for the frozen stills)
- `20261113-201730-B-sk20-figures-play-ab.png`: clay, before, after my change, after with the line; full frame and 2x crop
- `20261113-201731-B-sk20-number-ab.png`
- `20261113-201732-B-sk20-night-ab.png`

## Ledger rows (replace the two)

```
| Game/Plugins/JamAssets/Content/Skaters/M_SkaterTB1Jersey.uasset | material | 0.07 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-13 | B | n | M_Toon copy + JerseyDeco (B12) + vertex colour path behind VColMix (SK6) + felt path behind FeltMix (default 0): fibre normal, mottle, FeltDarken, hem band, sheen colour, ClayBumpScale (lane SK7); NumberOn above 1 draws the digits' fill in dark ink without the atlas outline (lane SK20) (73444 bytes) |
| Game/Plugins/JamAssets/Content/Skaters/MI_SkaterTB1_P3_TrimDeco.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python | | | 2026-11-13 | B | y | instance of M_SkaterTB1Jersey on the Trim slot: cream felt (0.80, 0.70, 0.50) patch, rolls, bands and stitches; back number via UV0 on the patch, crest off; VColMix 1, FeltMix 1 (lanes SKI3, SK6, SK7); number in dark ink, NumberOn 2, NumberWidth 0.42, NumberHeight 0.7 (lane SK20) (15098 bytes) |
```

## What I could not verify

- A packaged build. The game's path in a play session gives the same 1398x790, but I ran no cooked game.
- The line as an ini line, and whether a scalability setting overrides it there.
- Full screen at 3840x2400, where the display-based choice would likely pick a lower fraction still.
- The number beyond 7.5 m (lost to the stall), and two-digit numbers other than 21.
- The banner figure with the new number in a banner framing. The only frozen frame of it is the third cell of `sk20-frozen-vs-play`.
- The cook (yours). The base material is shared by five instances; the editor compiled it without an error in the log.
- The resolution TAA drew at (cell 9); I read the profile only for TSR.
- The log has `JAMFX missing` for `NS_deke` and `MS_deke`, not mine, not looked into.

## Decisions for you

1. **Send the line to the other device?** It removes the steps on both figures and costs about 3 of 54 frames a second on clay here. Without it nothing of B's can make the figures in play look like the frozen stills.
2. **Keep the dark number on the clay patch?** It reads in play now, but the numeral is no longer the pale outlined one in the banner stills. To go back, set `NumberOn` 1, `NumberWidth` 0.3 and `NumberHeight` 0.5 on the instance; the material's new branch can stay unused.
3. **B's un-frozen captures until the line lands:** take them as the project is (stepped, true to what a player sees today), or set `r.ScreenPercentage 100` in the session so they show the look after the line? I have left them as the project is.
4. **The banner stills are frozen frames**, crisper than any frame of play even at 100 %. If the banner must match play, it needs an un-frozen frame taken with the line.
