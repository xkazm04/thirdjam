# FILM1 report: the night's unseen work filmed, and two of my own finished (device B)

Lane FILM1, device B, machine clock 03:11 to 03:45. Main tree `C:\Users\kazda\jam\thirdjam` (`dev-b` at 6a88eb33 when I started and 09b37131 when I finished: other lanes committed reports meanwhile; you named f1f1f0e6), port 8002, one GPU editor session (pid 39716), left through `ExitEditor` with no dirty package; that process is gone. The editor on 8004 was not touched. Saved, with no play session open: `VFX/NS_brake`, `VFX/NS_ability`, `VFX/M_Fx_Chalk` (new). **No figure Blueprint, animation Blueprint or anything under `Skaters/` was opened for edit or saved**; the three lab files in `Skaters/Lab/` were only played. No commit, no push, no build, no cook. `r.Editor.Viewport.OverridePIEScreenPercentage` back to 1, `Slate.bAllowThrottling` back to 1 (I set it to 0 as SK25's trap says), the `jam` variables back to 0 / 0 / empty. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/film1` (`tk.py` the camera, the un-paused film runner and every pass; `spec43.mjs`, `mkchalk.py`; all frames in `shots/`; lane-start backups of the two systems in `bk/`).

Every strip is un-paused unless it says frozen: the game's own screenshot requested on chosen ticks while the world runs at its fixed 60 frames a second. "Game scale" is a camera 15 m from the subject at the game camera's height and angle; "close" is the lane's own suggested camera.

## 1. The figure lane's film (SK25's work, merged in this tree)
| Strip | What I filmed | What I saw, as a viewer | Still |
|---|---|---|---|
| Two lying players (rule A above, rule B below) | knockdown frames 40 to 76 every 6th, from straight above and at game scale | **Rule B reads from above**: the top knee draws up at f52 to f58 and goes back by f64. **Rule A's forearm drag I could not see** in these frames. At game scale neither can be made out: two small lying figures that look alike | `film1-lying` |
| The whip at Strength 1.0 against 0.3 | frames 0 to 20, from the side and at game scale | At 1.0 the whole upper body bows forward at f2 to f4 and is back by f9 to f12; at 0.3 a small nod. **The difference reads at game scale** at f4. It reads as a bow from the waist, not as a head snap | `film1-whip` |
| A plain collision: whip 0 (default) against 0.3 | the `Check_Hit_Back` take on both, frames 0 to 18 | The take itself folds both figures deeply. **0 and 0.3 cannot be told apart at game scale**, and barely close up | `film1-collision-pair` |
| The back get-up | knockdown frames 75 to 110 every 5th, side and game scale | **The stick stays clear of the legs** the whole way: carried out in front, blade on the ice from f85. Between f75 (flat on the back) and f80 (on a knee, trunk upright) most of the rise happens in 5 frames: it reads as a jump | `film1-getup` |
| The check in three sizes | `Check_Light`, `Check`, `Check_Heavy` forced by me on three idle figures (the film script does not), frames 0 to 30 | **Light reads as different**: the figure hardly moves. **Middle and heavy look alike at game scale**: both lunge low onto a knee; the heavy holds a little longer | `film1-check-sizes` |

Not filmed: the same things in a real match at the listed seeds and sim frames (10,163 frames in is three minutes of play per take); the stumble variant of the whip; the whip from the front and the sides.

## 2. The physics lane's film: red against blue (lane RAG's lab files)
The child Blueprint loaded and ran against the newer parent with no error line in the log; the lane's own script reported 10 pairs, `done`, no error. Red (physics) stands on the left of each pair in my frames, blue (hand-made) on the right.

| What | Seen | Still |
|---|---|---|
| Four pairs close (back 60 frames, forward 74, two sides 88 and 96), every 3rd frame to f30, then every 6th | **The bodies match frame for frame.** No limb through the ice, no jitter, no pop at the hand-back in these samples | `film1-pairs-close` |
| Four more pairs at game scale (104 to 138 frames) | **Red cannot be told from blue** in any of them | `film1-pairs-game` |
| Two pairs with the stiff motors | the same as soft | `film1-pairs-stiff` |

**The one thing that differs is the red figure's stick, and it reads as a fault, not as life.** In the forward fall (pair 1) the red stick swings out flat to the side at f006, lies about a metre from the hand at f009 (soft; closer with the stiff motors), and points far out to the side again in the get-up at f057 to f060, while the blue stick stays under the body. So, plainly: **the red figure does not read as more alive; it reads as the same figure with a loose stick.**

## 3. My own two
**(a) The stop's fan, lighter and smaller.** Saved: pale blue-grey chalk streaks and a few discs with a soft, slightly darker edge. Measured as in B42 (frozen pair, pixel difference over 12 of 255): **2.99 % of the frame at +6**, 3.02 % at +12, gone by +20 (B42's dark fan: 3.98 %). 2.6 % of the figure's own pixels changed at +6. **That is above the 2.0 to 2.5 % you asked for.** I tried the fan at the ice's own value first (particle colour 1.75, 1.95, 2.3): it measured 2.09 % and could hardly be seen. Pale starts to read at about (1.15, 1.34, 1.75), which is what is saved; a quarter fewer streaks would bring it into the band, not measured. Still `film1-fan-lighter`.

**(b) The ability puffs.** The large soft sprites are replaced by two to three times as many small dabs on a new material, `M_Fx_Chalk` (a feathered disc with a slightly darker edge, no wide gradient). No squares in frozen or un-paused frames. Shares at +4, frozen: shotgun **2.34 %**, hip check **2.20 %**, shield **3.10 %**, slow puck **3.22 %** (B42: 2.46 / 3.29 / 3.91 / 3.61). Still `film1-ability-puffs`. Two things I saw and did not fix: the shotgun's cloud is a neutral dark grey (the blue tint is lost), and the shield's ring is not in the un-paused frame at +30 although it should last about 50 frames.

A finding that explains B42's fan: an unlit particle colour of 0.8 draws as mid grey on the clay ice. The lit ice is about twice as bright, so "pale" needs particle colours near 2.

## 4. Together with the other device's answers (un-paused, clay)
- **A real hard board hit** (skater 3, Strength 1.00, sim frame 1828 of the plain scripted match): **their flash wins and mine is not seen.** A blue-white pool whites out the hit figure and the board for about four frames and a cap glows blue on the board top; my chips and snow cloud cannot be made out under it at the game camera. Still `film1-board-together`.
- **A goal fired by me on the bus** (the rink logged `react goal`), from centre ice and from 14 m, stands in frame: **the colours fight.** Their red light washes the end boards, the stands and the ice round the net (pink out to the face-off circles from 14 m); my blue dust in front of it turns muddy purple, reads from centre ice at +25 to +85 and is gone by +100. Still `film1-goal-together`.

## 5. Night (one strip, `film1-night`)
The fan is a pale grey patch on the dark ice at +3 and gone by +12: it reads. A real check's streak chips read white. The goal's blue dust is dark on the dark hall and hard to see; the cream fountain reads; the red light covers the ice round the net.

## Not verified
- Rule A's forearm move; the whip in a real match; the check sizes as the game picks them (I forced the takes).
- The physics pairs between my samples (every 3rd frame at best): a one-frame pop or jitter would not show. Pairs 8 and 9 were filmed and not looked at.
- The fan at 2.0 to 2.5 %; the fan for bots at distance; the shield ring's life in play.
- `M_Fx_Chalk` on tiers other than clay and night; the abilities on night.
- My board crash on a hard hit without their flash, to say what it adds.
- Cook: not run. Two systems changed, one material is new.

## For the figure lane
1. The middle and heavy checks need a different silhouette, not a longer hold: at game scale they are one take.
2. The back get-up's first five frames (f75 to f80) carry most of the rise: spread it over ten.
3. Rule A's forearm drag is too small to see even from above; rule B's knee is the size that reads close up. Neither reads at game scale, so do not spend more on the lying loops for the game camera.
4. The whip reads at 1.0 and is a nod at 0.3. If it should read as a head snap, move more of it from the spine to the neck.

## For the physics lane
1. Bodies: no visible gain over the hand-made falls at either scale. As a viewer I would not ship it for the look.
2. The stick is the visible difference and it looks loose: out to the side at f6, a metre from the hand at f9, out again in the get-up. Your own report names the stick body as the next step; it is the only thing worth doing before another film.

## For the other device
1. **The goal's red light still reaches the ice** (pink to the face-off circles from 14 m, on clay and night). B's advice was radius 350.
2. **The hit flash whites out the hit figure** for about four frames at Strength 1.0 and hides everything at the foot of the boards. Half the candelas, or tint only the board top.
3. Unchanged from B42: `User.Dir` for `ability` and `deke` in the binder; the shotgun at the blade; the slow puck at the puck.

## Decisions for the operator (each with its two stills)
1. **The whip on a plain collision: 0 or 0.3.** Both halves are in `film1-collision-pair`; set it against `film1-whip`. At game scale they look the same, so 0 (the default) costs nothing.
2. **Physics on the falls: keep the hand-made falls or go on with the lab.** `film1-pairs-close` against `film1-pairs-game`: no visible gain, one visible fault.
3. **The stop's fan: today's pale fan or B42's dark one.** `film1-fan-lighter` against `b42-brake-fan`.
4. **The ability dabs: today's small chalk dabs or B42's soft clouds.** `film1-ability-puffs` against `b42-ability-kinds`.
5. **The goal: blue dust under a red light, or one of them.** `film1-goal-together` against `b42-goal`.
6. **The board hit: the flash alone, or the flash dimmed so the crash shows.** `film1-board-together` against `b42-chips-streaks` (the crash without a flash).

## Peak particle counts (from spawn counts; none measured live)
`NS_brake` per stop 58 -> **42**. `NS_ability`: shotgun 48 -> **68**, hip check 22 -> **46**, shield 40 -> **70**, slow puck 26 -> **60**. The worst moment of B42 (no ability in it) falls from about 420 to about 390.

## Assets (bytes, `Game/Plugins/JamAssets/Content/VFX/`)
| File | Before -> after |
|---|---|
| `NS_brake` | 552,130 -> 550,573 |
| `NS_ability` | 1,628,462 -> 1,623,601 |
| `M_Fx_Chalk` | new, 14,742 |

## Ledger rows (full rows, the ledger's 11 columns; replace the rows of `NS_brake` and `NS_ability`, add `M_Fx_Chalk`)
```
| Game/Plugins/JamAssets/Content/VFX/NS_brake.uasset | vfx | 0.53 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | the hard stop's fan: one burst 0.04 s after activation of 6 to 10 pale blue-grey chalk discs and 18 to 32 pale streaks 11 x 42-56 cm (M_Fx_Chalk) thrown ahead of the skates along the owner's travel, sized by the owner's speed clamped to 14 m/s and scaled down with the distance to the camera, nothing on a near-zero time step or outside 2 to 20 m/s, gone by +20 frames; played by BrakeFx on BP_SkaterVisual_TB1 and _TB1_P3 (lanes B40, B42, FILM1) (550573 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_ability.uasset | vfx | 1.55 | original | UE 5.9 Niagara via MCP/Python |  |  | 2026-11-14 | B | y | event ability on a use (Strength above 0.5), by User.Variant, small chalk dabs (M_Fx_Chalk): 0 shotgun a grey blast of 44 dabs and 24 white shards along User.Dir, 1 hip check an ochre arc of 46 on the lunge side, 2 shield a slate-blue ring of 60 with 10 white shards holding about 50 frames, 3 slow puck a frost patch of 60 at the skater; a grant (Strength 0) shows nothing; User.Dir read, default zero (lanes B40, B42, FILM1) (1623601 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_Chalk.uasset | material | 0.01 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | translucent unlit chalk dab for particles: the particle colour with a slightly darker feathered edge, depth-faded into the ice, the same on every tier; used by NS_brake and NS_ability (lane FILM1) (14742 bytes) |
```
