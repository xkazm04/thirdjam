# Lane V3 report: the puck's streak by speed; the pass, shot, possession and body-block sounds (device B), 2026-11-14

## Third session (after the lead's commit 6f69295e): the streak in clay cream on every tier

- GPU from 23:10 to 23:14: GPU-FREE.txt taken ("free from V3 at 22:28"; the b-fig editor then running was headless), written back "free from
  V3 at 23:14".
- Editor pid 36428 left through `ExitEditor`; the process is gone. Settings restored: `enable_game_sound` true, `jam.ScriptHuman` 0,
  `OverridePIEScreenPercentage` 1, `jam.Cut hockey.regular`. No dirty package; no temporary asset. No commit.
- Changed: **`NS_puck_trail` only** (338,770 -> 338,883 bytes). Its `User.Colour` default is now cream (0.96, 0.92, 0.80), was yellow-green
  (0.75, 1, 0.15).
  - Off clay, the material multiplies it by `NightBright` 2.5, which gives exactly the clay cream (2.4, 2.3, 2.0), so the streak is one colour on every tier.
  - **`M_Fx_PuckTrail` is unchanged.** I left its colour logic alone, so device A can still tint the streak through `User.Colour` if it ever wants to.
  - Nothing else changed: widths, alphas and the speed rules are as before. Compile up to date, no errors or warnings.
- Backup of the trail as committed: scratch `v3/bk/NS_puck_trail.6f69295e.uasset`.

**Seen on the GPU** (the same rig and camera as before: the drop moment of a fresh match, four trails at 8 / 20 / 22 / 44 m/s, the doubled shot
along the red or centre line):
- `C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-231348-B-v3-streak-cream-tiers.png` (+ `-crop.png`, each with a `.txt` caption).
- Rows: night yellow-green (committed) / night cream / neon yellow-green / neon cream / clay cream.

**What I looked at** (Read: `sheet_cream_c.png`):
- Night: the streaks are pale cream lines. The doubled shot's line on the red line reads clearly and is no longer a saturated yellow-green; the 20 and
  22 m/s streaks stay faint.
- Neon: thin cream-grey lines, calmer than the yellow-green, still readable against the black floor and the white and purple grid.
- Clay: as before (cream).
- Nothing is drawn over a figure on any tier: the ribbon lies on the ice and is depth-tested.
- Number, night: the doubled shot's streak row stands +68 levels (of 255) above the rows 4 px around it, against +35 in yellow-green. Brighter, but not
  saturated.
- Neon: my row measure is swamped by the white centre line the streak runs along, so I have no clean number there; judged by eye only.

Ledger rows for this session (`NS_puck_trail` and `M_Fx_PuckTrail` **replace** the rows given in the first hand-back):

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.34 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-14 | B | y | puck ribbon trail by speed: speed max(User.Speed, owner velocity) eased over 0.1 s (held on a near-zero time step or above 90 m/s); visible from a floor (12 m/s while User.Carried is -1, the default; 5 m/s at User.Carried 0; off at 1) over 4 m/s; brightness lerp(0.4, 1, (speed - 12) / 28); width 8 + 12 x (speed - 12) / 28 cm; life 0.15 s, so length = speed x 0.15 s; material M_Fx_PuckTrail; User.Colour default cream (0.96, 0.92, 0.80: x 2.5 off clay = the clay cream on every tier); User.Speed (m/s, default 0), User.Carried (default -1) (lanes B3, B18, B37, V3) (338883 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckTrail.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | translucent unlit soft ribbon for the puck trail: soft across its width; on clay an HDR cream (2.4, 2.3, 2.0) at ClayOpacity 1.0 times the emitter's speed alpha; off clay the particle colour (NS_puck_trail's User.Colour, cream by default) x 2.5 at 0.8 (MPC_Look.Clay); unchanged in V3's third session (lanes B37, V3) (15702 bytes) |

---

## Second session (after the lead's commit 8c4ece6e): the streak on night and neon; the thresholds for A207

**No asset changed in this session.**
- GPU from 22:22 to 22:28: GPU-FREE.txt taken ("free from E3 at 22:22"), written back "free from V3 at 22:28".
- Editor pid 30872 left through `ExitEditor`; the process is gone.
- The before copy (`/JamAssets/VFX/_V3tmp`: the old trail's expressions and an instance with the old clay values) was made again, used and deleted.
  It is not on disk. Settings restored, no dirty package.

**Streak on two more tiers** (the same rig and camera as the clay still; `hockey.regular` with `"tier"` set to `l2_night` and to `l3_neon`, the neon
mode's tier; P4 broadcast is dropped). Stills, each with a `.txt` caption:
- `C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-222740-B-v3-streak-night-ab.png` (+ `-crop.png`)
- `C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-222740-B-v3-streak-neon-ab.png` (+ `-crop.png`)

**What I looked at** (Read: `sheet_tiers.png`, `sheet_tiers_c.png`, `sheet_night_c.png`).
- Night: before, three thin faint yellow-green lines. After, the doubled shot is a solid yellow-green line along the red line, longer and wider; the
  20 and 22 m/s streaks stay faint over the lit white patches. Nothing is drawn over a figure: the slow pass's line with `User.Carried` 0 passes
  behind #7 and is hidden by him, because the ribbon is depth-tested on the ice.
- Neon: the same yellow-green lines, a little wider, readable against the black floor and the white and purple grid. They do not compete with the
  centre circle.
- Calm: the streak never rises off the ice and fades over 0.15 s. The night look is louder than before only for the doubled shot.

**Number** (pixels with green over blue > 40, the yellow-green streak, in the band left of the heads, x 300..640, y 280..480):
- night 853 -> 1454;
- neon 884 -> 1441.

**Off clay the colour is `User.Colour`.** Device A sets none, so it is the default yellow-green (0.75, 1, 0.15) x 2.5 at 0.8, as before V3. Decision for
the operator: keep it, or set the default to the clay cream so the streak is one colour on every tier. That is one value on `NS_puck_trail`.

**Thresholds re-set to A207** (origin/main d0d132bf, 23:02, read after `git fetch` only; `status/A207.md` "PASS READY 1").
- Headless editor pid 10656, 23:06 to 23:09, left through `ExitEditor`, process gone.
- Changed: `MS_pass` and `MS_shot` only. `MS_possess` was re-saved by accident (my save script saves all four) and is restored byte for byte to its
  committed content (256,337 bytes; `git status` clean for it).

What A207 ships, and what the sounds receive:
- **Pass**: a tap is 14 m/s; the hold grows the speed evenly to 23.3 m/s at 30 frames; bots' passes are solved between the two. `pass` Strength =
  speed / `puck.max_speed` (30, unchanged), so tap 0.467 and full hold 0.777.
- **Shot**: tap / half / full = 35.7 / 39.9 / 51.6 m/s. `shot` Strength = speed / (`max_speed` x `shot.speed_scale` 2) = speed / 60, so 0.595 / 0.665 /
  0.86: the same Strength range as today's shots. **With the first hand-back's ramp (0.88) the doubled shot played no crack at all**: measured
  below, it was identical to today.

The change (graph values; one pair of edges re-routed in `MS_pass`):
- `MS_pass`, the snap ramp: from Strength 0.4667 (the 14 m/s tap: no snap) to 0.7767 (the 23.3 m/s full hold: full snap). `Subtract_f0fe` 0.45 ->
  0.4667, `Multiply_9787` 2.857 -> 3.226.
- `MS_pass`, the lowpass: now driven by that same ramp t (0 at tap, 1 at full) instead of raw Strength: cutoff = 1500 + 12000 x t^2 Hz, so the tap is
  soft (1.5 kHz) and the full pass bright (13.5 kHz).
  - The square's two inputs moved from `Clamp_6fdc` to `Clamp_1cd2`; `Multiply_2e92` 18000 -> 12000; `Add_3c9e` 600 -> 1500.
  - The tap is now softer than today's pass on purpose: the brief's "soft tap".
- `MS_shot`, the crack ramp: from 30 m/s (Strength 0.5) to 42 m/s (0.7). `Subtract_2da4` 0.88 -> 0.5, `Multiply_7986` 8.333 -> 5. Every doubled shot cracks:
  the tap lightly (0.48), half and full fully. Variant 1 still forces the crack.

| Case (A207 speed) | Before (committed 8c4ece6e) LUFS / peak, centroid | After |
|---|---|---|
| pass tap 14 m/s (S 0.467) | -35.1 / -18.0, 1184 Hz | -34.1 / -18.0, **553 Hz** (soft) |
| pass 10-frame hold 16 m/s (0.533) | -33.3 / -17.0, 2369 Hz | -33.4 / -17.1, 2262 Hz |
| pass full hold 23.3 m/s (0.777) | -30.1 / -12.3, 3639 Hz | -29.8 / -12.0, **3688 Hz** (snap) |
| shot tap 35.7 m/s (0.595) | -34.1 / -17.4 (= today's tap) | **-31.3 / -13.3** (light crack) |
| shot half 39.9 m/s (0.665) | -33.5 / -15.8 | **-28.0 / -8.6** |
| shot full 51.6 m/s (0.86) | -32.0 / -13.1 | **-25.8 / -6.3** |

- All peaks are inside -3 dBTP.
- WAVs: `C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-230843-B-v3-a207-*.wav` (after) and `...-a207-before-*.wav`, six cases each.
- What I looked at: the numbers above (measure.py, wavstat.py). Not heard.
- For device A, this replaces items 2 and 3 of the first hand-back's section 4: **no binder change is needed for the pass or the shot now**.
  Shot Variant 1 remains an optional "force the crack" (for example a one-timer). The `possess` event, `shot_blocked` Variant 1 and the optional
  `User.Carried` call still stand.
- Caveat for device A: with `game.shot.speed_scale` back at 1 (the neutral), Strength is speed / 30 and today's 17.8 m/s tap would sit at 0.59,
  over the crack's 0.5 start. With the keys ON as A207 ships them, it is right.

Ledger rows for this session (`MS_pass` and `MS_shot` **replace** the rows given in the first hand-back):

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_pass.uasset | audio | 0.24 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; pass: 300 Hz blip + 2 kHz noise click + delayed 1.8 kHz swoosh, through a lowpass whose cutoff is 1500 + 12000 x t^2 Hz, with t = clamp((Strength - 0.4667) x 3.226): soft at the 14 m/s tap, bright at A207's 23.3 m/s full hold (Strength = speed / 30); plus a snap (input Snap = SFX_SlapShot_02, +4 semitones, gain 1.5) at t; level 0.3 + 0.7 x Strength; FxTrim 0.70 (lanes AU8, V3) (237539 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot.uasset | audio | 0.27 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; shot: 2 kHz noise crack + sine thump 150 + 60 x Strength Hz + SFX_SlapShot_01, level 0.3 + 0.7 x Strength; a hard-shot crack (input Crack = SFX_SlapShot_02 gain 1.6, 3.5 kHz noise burst 0.5/12 ms gain 0.9 with seed 7, 95 Hz thump 1/80 ms gain 0.8) at clamp(clamp(Variant,0,1) + (Strength - 0.5) x 5) x (0.6 + 0.4 x Strength): from 30 m/s to full at 42 m/s with A207's Strength = speed / 60 (shot.speed_scale 2), or at Variant 1; FxTrim 0.70 (lanes AU8, V3) (266449 bytes) |

---

# First hand-back

- Tree `C:/Users/kazda/jam/wt/b-aud`, branch `lane/b-aud` at dev-b 27a0ba99. Built first (ThirdJamEditor Development: pass). Port 8006.
- Editor sessions: headless (`-nullrhi -unattended`, pid 40444) for the sounds, then GPU (pid 18808) from 21:41 to 21:52 for the streak. GPU-FREE.txt
  was taken at 21:41 and written back at 21:52 ("free from V3 at 21:52"). Both editors left through `ExitEditor`; both processes are gone. Another
  editor (pid 29976, b-fig, headless) ran the whole time and I did not touch it.
- No commit, push, cook or paid service. No ElevenLabs call: the one new wave is a take that was already on disk (`audio-lab/ready/SFX_SlapShot_02.wav`).
- Settings I changed and put back: background audio (on, then off), CPU throttle (off, then on), `enable_game_sound` (false, then true), `jam.ScriptHuman`
  (1, then 0), `r.Editor.Viewport.OverridePIEScreenPercentage` (0, then 1), `jam.Cut` (left at hockey.regular). No dirty package at exit. The temporary
  folder `/JamAssets/VFX/_V3tmp` (the old trail copy used for the before frames) was deleted and is not on disk.
- Backups of the assets as they were at lane start: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/v3/bk/`.
  Scripts: same folder (`edit_shot.js`, `edit_pass.js`, `edit_blk.js`, `build_possess.js`, `run.js`/`render.py` renders, `measure.py`, `gdiff.js`,
  `trail_edit.mjs`, `mktrail.py`, `rig.py`).
- **Nobody has heard the sounds.** Every sound statement below is a measurement of the editor's own output, recorded through the submix.

## 1. Done / not done

| Item | Verdict |
|---|---|
| Puck streak by speed (`NS_puck_trail`, `M_Fx_PuckTrail`) | **done, seen on the GPU** from the game camera, before and after |
| Pass snap by Strength (`MS_pass`) | **done**: soft tap at low Strength, sharp snap at high Strength; today's pass level unchanged |
| Harder shot (`MS_shot`) | **done**: a crack layered over today's shot at Strength 0.88 and over, or at Variant 1; today's shots (Strength up to 0.867) unchanged |
| Clack when the puck reaches a stick (`MS_possess`, new) | **built**: **no event sends it yet.** A must add one (section 4) |
| Padded thud when a shot hits a body (`MS_shot_blocked` Variant 1) | **built**: Variant 0 (today's no_shoot refusal) is **sample-identical**. A must send Variant 1 (section 4) |
| A real match shot or pass filmed with the new streak | **not done**: the proof uses test trails moved at set speeds (section 2) |

## 2. The streak

**What it was:** a 9 cm ribbon, life 0.15 s, the same alpha above 16 m/s and none at 12 m/s or below. On clay it was blue-grey at 0.35 opacity. Its
length already grew with speed (speed x 0.15 s), but a doubled shot drew the same faint line as today's. Its teleport guard dropped any speed over
45 m/s, so a doubled full shot (52 m/s) would have frozen the eased speed.

**What changed** (`NS_puck_trail`, emitter `Trail`):
- Speed: still `max(User.Speed, owner velocity)`, eased over 0.1 s. The guard is now 90 m/s (was 45).
- Brightness: alpha = `vis x lerp(0.4, 1.0, saturate((speed - 12) / 28)) x (1 - age)`, so 22 m/s is about 0.54 and 44 m/s is 1.0. `vis` is the old 4 m/s ramp
  from a floor.
- Width by speed: `8 + 12 x saturate((speed - 12) / 28)` cm, so 22 m/s is 12 cm and 40 m/s or more is 20 cm (was a flat 9).
- Length: unchanged rule (speed x 0.15 s), so a doubled shot leaves a streak twice as long: 3.3 m at 22 m/s, 6.6 m at 44 m/s.
- **New user float `User.Carried`**, default -1. At -1 (unknown, what A sends today) the floor stays 12 m/s, which hides a carried puck at 11 m/s.
  At 0 (loose) the floor drops to 5 m/s, so a slow pass draws a faint streak. At 1 (carried) the streak is off. Spawning follows the same floor.
- `M_Fx_PuckTrail` (rebuilt with B37's script, two values changed): on clay the colour is an HDR cream (2.4, 2.3, 2.0) instead of blue-grey
  (0.40, 0.45, 0.58), and `ClayOpacity` is 1.0 instead of 0.35, because the emitter's alpha now carries the speed. Night and other tiers are unchanged.
  Compile state: up to date, no errors or warnings.

**Proof** (PIE, hockey.regular, the human's game camera: 3.5 m up, pitch -17.65 deg; each row is a fresh match captured at the same moment, 3 s
after the drop). Four trails cross the view together, nearest to farthest: 8 m/s (slow pass), 20 m/s (full pass), 22 m/s (today's shot, between the
tap at 18 and the full charge at 26), 44 m/s (doubled shot).
- Still: `C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-215046-B-v3-puck-streak-ab.png` (full frames) and `...-ab-crop.png`, each with a `.txt` caption.
  Rows: before / after / after with `User.Carried` 0.
- **What I looked at** (Read tool: `sheet5.png`, the crop sheet, plus the full frames `after1`, `after2`, `before2` and `sheet4` from earlier tries).
  - Before: three faint thin lines, barely apart from the ice scratches.
  - After: three cream streaks. The 44 m/s streak is clearly the longest (it runs off the frame's left edge), the widest and the brightest; 20 and
    22 m/s are shorter and dimmer. The slow pass draws nothing.
  - After with `Carried` 0: the slow pass also draws a short, faint fourth line.
  - The streaks lie on the ice and pass behind the figures, never over them.
- Number (mean luminance per row in an empty-ice band x 290..400, against the row's 15-row neighbourhood):
  - after: the streak rows stand +13.3 and +16.6 levels (of 255) above the ice;
  - before: no streak row reaches above the ice's own scratch texture (+8.5 at most, and that row is not a streak).
- Earlier tries in a live match (`after3`, `after3L`) caught the hit camera and the boards, so only the fresh-drop frames count.

## 3. Sounds: what changed and the numbers

Method as CHK56: `create_sound2d` in the editor world with `Strength` and `Variant` set before play, recorded with `AudioMixerLibrary` start/stop
recording, one clip per case; LUFS and true peak by ffmpeg ebur128; null test after vs before. **`MS_pass` and `MS_shot` contain Noise generators
with a random seed**, so two renders of the *same* asset only null to -20 to -24 dB. For those two, "unchanged" is proven at graph level. Graph diff of
`DescribeGraph` before/after:

| Asset | Old nodes kept | Old pin values changed | Old edges removed (re-routed) | Edges added |
|---|---|---|---|---|
| MS_pass | 26/26 | 0 | 1: `AudioMixerMono3_dbee -> Multiply_5475` (now through the lowpass and a mixer) | 16 |
| MS_shot | 25/25 | 0 | 1: `AudioMixerMono2_807b -> Multiply_333d` (now old + crack mixer) | 28 |
| MS_shot_blocked | 22/22 | 0 | 1: `Multiply_6bd1 -> Multiply_acd4` (now old x (1 - V1 gate) + V1) | 30 |

Same LUFS and peak within the noise's spread (three repeat renders of today's shot at Strength 0.6: -34.0 / -34.0 / -34.1 LUFS).

**MS_pass** (Strength = pass speed / `puck.max_speed`):
- A lowpass on today's mix, cutoff `600 + 18000 x S^2` Hz (Bandwidth 1.9, no resonance): about 780 Hz at S 0.1 and 4.6 kHz at today's 0.47.
- A snap layer (new input `Snap` = `SFX_SlapShot_02`, +4 semitones, gain 1.5) fades in from S 0.45 to 0.8.
- Level rule unchanged (0.3 + 0.7 S, FxTrim 0.7).

| Case | Before LUFS / peak | After LUFS / peak dBFS | Spectral centroid before / after |
|---|---|---|---|
| S 0.1 (tap) | -39.6 / -22.5 | -38.8 / -22.8 | 1402 / 463 Hz |
| S 0.3 | -35.9 / -19.9 | -35.9 / -19.9 | 1410 / 663 Hz |
| S 0.47 (today's 14 m/s pass) | -34.0 / -18.1 | -34.1 / -18.1 | 1425 / 1458 Hz |
| S 0.7 | -32.0 / -16.1 | -31.5 / -14.0 | 1416 / ~3700 Hz |
| S 1.0 (full) | -30.0 / -14.0 | -28.4 / -10.7 | 1393 / ~3900 Hz |

The centroids at S 0.7 and 1.0 were measured at snap gain 2.2 (the a3 renders); the final gain is 1.5.

**MS_shot** (Strength = exit speed / `puck.max_speed`):
- New inputs `Variant` (default 0) and `Crack` (`SFX_SlapShot_02`).
- crackT = `clamp(clamp(Variant,0,1) + clamp((S - 0.88) x 8.33, 0, 1), 0, 1)`; crack level = crackT x (0.6 + 0.4 S).
- Layers: the wave (gain 1.6), a 3.5 kHz highpass noise burst of 0.5/12 ms (0.9, fixed seed 7), and a 95 Hz thump of 1/80 ms (0.8).

| Case | Before | After |
|---|---|---|
| V0 S 0.4 | -35.9 / -18.9 | -35.9 / -19.3 (no crack) |
| V0 S 0.6 (today's tap) | -34.0 / -17.2 | -34.0 / -17.3 (no crack) |
| V0 S 0.867 (today's full charge) | -31.9 / -13.0 | -31.9 / -12.9 (no crack) |
| V0 S 0.95 | -31.3 / -12.8 | **-28.2 / -10.8** (partial crack) |
| V0 S 1.0 | -30.9 / -12.5 | **-25.2 / -5.9** |
| V1 S 0.6 | -34.0 / -17.3 | **-27.1 / -7.1** |
| V1 S 1.0 | -30.9 / -12.9 | **-25.1 / -6.0** |

**MS_shot_blocked**:
- New `Variant` (default 0), `BodyPad` (`SFX_BodyCheck_02`) and `Pock` (`SFX_PuckBoards_01`).
- Variant 1 layers: the pad (-4 semitones, lowpass 700 Hz, AD 2/200 ms, gain 1.0), the puck's pock (+2 semitones, lowpass 2.5 kHz, 0.55), and a
  70 Hz thump of 2/120 ms (0.65).
- Variant 1 level = 0.35 + 0.65 S. Variant 0 is today's sound x 1 + 0.

| Case | Before | After | Null |
|---|---|---|---|
| V0 S 0 / 0.5 / 1 | -29.0 / -27.7 / -26.5 | same | **exact (max diff 0.0) in all three** |
| V1 S 0.2 | (played V0: -28.5) | -32.1 / -17.0, 0.26 s | new |
| V1 S 0.6 | (-27.5) | -28.3 / -13.3 | new |
| V1 S 1.0 | (-26.5) | -25.7 / -10.6 | new |

**MS_possess** (new; mono one-shot; inputs Strength 0.4, Variant 0, FxTrim 0.7, Clack = `SFX_PuckBoards_01`):
- Variant 0 is the stick: the wave at +9 semitones through a 500 Hz highpass, a 1250 Hz wooden tok (0.5/25 ms), and a 2.8 kHz click (0.3/6 ms).
  Level 0.35 + 0.65 S.
- Variant 1 is the skates/body: the wave at -2 semitones through a 900 Hz lowpass, plus a 140 Hz tock (2/60 ms). Level 0.3 + 0.5 S.
- Ends at 0.4 s.

| Case | LUFS / peak | Duration | Centroid |
|---|---|---|---|
| V0 S 0.1 | -38.3 / -20.6 | 0.13 s | 2.2 kHz |
| V0 S 0.4 | -35.9 / -17.3 | 0.13 s | 2.2 kHz |
| V0 S 0.8 | -32.8 / -14.2 | 0.13 s | 2.2 kHz |
| V1 S 0.1 / 0.4 / 0.8 | -40.4 / -37.3 / -34.4 | 0.36 s | ~570 Hz |

All peaks are inside -3 dBTP (the hottest is the hard shot at -5.9). WAVs (stereo 48 kHz, the recorder's format):
`C:/Users/kazda/jam/wt/b-aud/devlog/shots/20261114-213901-B-v3-*.wav` (16 clips: pass_s01/s047/s1 and before-pass_s1, shot_v0_s06/s1, before-shot_v0_s1, shot_v1_s06,
blk_v0_s05, blk_v1_s02/s06/s1, pos_v0_s01/s04/s08, pos_v1_s04). All clips: scratch `v3/wav/` (prefixes `before`, `final`).

**What I looked at:** the numbers above (measure.py, wavstat.py), the graph diffs (gdiff.js), and the import settings read back (mono, 0.393 s,
q40, project-defined compression as SlapShot_01, SC_SFX, no attenuation). No still for the sounds.

## 4. For device A (exact names)

1. **Puck streak**: nothing is required; it reads the puck's own velocity.
   - Optional, one call in `AJamPuck`, on every change of possession and at spawn:
     `Trail->SetVariableFloat(TEXT("User.Carried"), bCarried ? 1.f : 0.f)`. With it, slow passes (5 to 12 m/s) draw a faint streak and a carried puck
     never does, even when boosted to 16.5 m/s, which today draws a faint streak. Without it the streak is as in row 2 of the still.
   - Speeds over 90 m/s are treated as a teleport. Say so if the doubled shot with the shotgun ability (1.5 x 52 = 78 m/s) can go higher.
2. **Pass** (`pass` -> `MS_pass`, no binder change): Strength as now (speed / `puck.max_speed`). The snap starts at 0.45 and is full at 0.8. **If
   tonight raises `puck.max_speed`** (to carry a 52 m/s shot), passes shrink in Strength: a 21 m/s full pass becomes 0.4 against 60 and plays soft.
   Then send the pass's Strength as `speed / 30` clamped to 1, or tell me the new max and I move the ramps.
3. **Shot** (`shot` -> `MS_shot`): the crack plays at Strength 0.88 and over by itself. For every doubled shot to crack whatever the max speed, have
   `VariantFor(shot, ...)` return **1** when the exit speed is over 30 m/s (above today's top), or for Kind `one_timer`. Variant 0 is today's shot.
4. **Puck reaches a stick**: a new bus event **`possess`** (constant e.g. `JamEvents::Possess`; the binder then plays `MS_possess` by name):
   - When: a loose puck becomes a skater's (pickup, pass received, steal off a loose puck); not a carry continuing.
   - Who: the new carrier. Where: the puck. Strength: the puck's speed relative to the skater at the take / `puck.max_speed`.
   - Variant **0** when the stick/blade took it, **1** when it stopped at the skates or body first (A208's body collision).
   - Fires often: it is the quietest event sound (-36 LUFS at 0.4). No `NS_possess` exists; the binder should treat a missing burst as silent.
5. **Shot hits a body**: send **`shot_blocked` with Variant 1** (e.g. Kind `body`, `VariantFor(shot_blocked, body) = 1`) with Strength = the puck's
   speed at the hit / `puck.max_speed`. Today's `shot_blocked` (the no_shoot refusal, Variant 0) is unchanged.
   - Or tell me to move this onto `save` if A prefers. `save` already covers "last man's body" with a crowd ooh; that ooh is too much for every
     body block.

## 5. Not verified
- Any sound by ear. The pass, the crack, the clack and the thud in a match (none of the new Variants or events is sent yet).
- A real game shot or pass with the new streak: the proof used test trails at set speeds, not the sim's puck. The streak on the night tier and the
  other tiers (only the clay frame was looked at).
- The cook (no cook run). `MS_possess` and `SFX_SlapShot_02` are new. `SFX_SlapShot_02` is referenced by `MS_shot` and `MS_pass`; `MS_possess` by
  nothing until A's event. The cook check before pushing dev-b should list both with 0 errors.
- CPU cost of the extra Wave Players (gated to silence when unused).

## 6. Decisions for the operator
1. The streak on clay is now a clear cream line (was nearly invisible). Too strong? `ClayOpacity` (1.0) on `M_Fx_PuckTrail` is the one knob.
2. The hard shot is about +5.7 LU over today's full charge and is the loudest event sound after the big hit. Lower `Crack`'s mixer `AudioMixerMono3_ca74` gains if it is too hot.
3. The pickup clack on every possession: keep it, or only on received passes?

## 7. Next three steps
1. When A sends `User.Carried`, `possess`, shot Variant 1 and `shot_blocked` Variant 1: a 100 s clay match that counts each event from the log, with
   one un-frozen still of a real doubled shot's streak from the game camera.
2. Re-time the pass and crack ramps to A207's final `puck.max_speed` and pass speeds (read `status/A207.md` when it lands).
3. The cook-only run on this tree before the lead pushes dev-b.

## 8. Ledger rows

`MS_pass`, `MS_shot`, `MS_shot_blocked`, `NS_puck_trail` and `M_Fx_PuckTrail` **replace** their rows. `MS_possess` and `SFX_SlapShot_02` are **new**.
`SFX_PuckBoards_01` and `SFX_BodyCheck_02` replace their rows for the note only (assets unchanged).

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_pass.uasset | audio | 0.24 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; pass: 300 Hz blip + 2 kHz noise click + delayed 1.8 kHz swoosh, now through a lowpass whose cutoff is 600 + 18000 x Strength^2 Hz (soft tap at low Strength) plus a snap (input Snap = SFX_SlapShot_02, +4 semitones, gain 1.5) fading in from Strength 0.45 to 0.8; level 0.3 + 0.7 x Strength; FxTrim 0.70 (lanes AU8, V3) (237539 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot.uasset | audio | 0.27 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; shot: 2 kHz noise crack + sine thump 150 + 60 x Strength Hz + SFX_SlapShot_01, level 0.3 + 0.7 x Strength; a hard-shot crack (input Crack = SFX_SlapShot_02 gain 1.6, 3.5 kHz noise burst 0.5/12 ms gain 0.9 with seed 7, 95 Hz thump 1/80 ms gain 0.8) at clamp(clamp(Variant,0,1) + (Strength - 0.88) x 8.33) x (0.6 + 0.4 x Strength): Variant 1 or Strength 0.88 and over; Variant 0 below 0.88 plays as before; FxTrim 0.70 (lanes AU8, V3) (266449 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot_blocked.uasset | audio | 0.27 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; Variant 0 (a shot refused in a no_shoot zone, unchanged): SFX_PuckBoards_01 -7 semitones lowpass 1.4 kHz + SFX_BodyCheck_02 -3 semitones lowpass 900 Hz + 85 Hz sine, level 0.75 + 0.25 x Strength; Variant 1 (a shot stopped by a body): padded thud BodyPad (SFX_BodyCheck_02 -4 semitones, lowpass 700 Hz, 2/200 ms, gain 1.0) + Pock (SFX_PuckBoards_01 +2 semitones, lowpass 2.5 kHz, 0.55) + 70 Hz thump 2/120 ms (0.65), level 0.35 + 0.65 x Strength; FxTrim 0.70 (lanes AU8, V3) (265648 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_possess.uasset | audio | 0.26 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono one-shot for the proposed bus event possess (the puck becomes a skater's); inputs Strength (puck speed at the take / max speed, default 0.4), Variant, FxTrim 0.70, Clack (SFX_PuckBoards_01); Variant 0 stick clack: Clack +9 semitones highpass 500 Hz + 1250 Hz tok 0.5/25 ms + 2.8 kHz click 0.3/6 ms, level 0.35 + 0.65 x Strength; Variant 1 skates/body: Clack -2 semitones lowpass 900 Hz + 140 Hz tock 2/60 ms, level 0.3 + 0.5 x Strength; ends 0.4 s; unreferenced until device A sends possess (lane V3) (256337 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_SlapShot_02.uasset | audio | 0.03 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane N3 make_ready.mjs (ffmpeg) + lane V3 ffmpeg +2.8 dB | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/ledger.jsonl (prompt as SFX_SlapShot_01, take 2; no new generation) | 2026-11-14 | B | y | slap-shot crack, a sharp transient; mono 48 kHz 0.39 s, -27.3 LUFS (peak-bound), -3.1 dBTP, Bink q40, SC_SFX, no attenuation; in MS_shot Crack and MS_pass Snap (+4 semitones) (lane V3) (26754 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_PuckBoards_01.uasset | audio | 0.0 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane N3 make_ready.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/ledger.jsonl (prompts; status/B-audio-au1.md) | 2026-11-12 | B | y | mono 48 kHz, Bink q40; puck on boards in MS_board (also puck-drop tock in MS_faceoff V0, low block in MS_shot_blocked V0, Pock in MS_shot_blocked V1, Clack in MS_possess: lane V3) (17013 bytes) |
| Game/Plugins/JamAssets/Content/Audio/SFX/SFX_BodyCheck_02.uasset | audio | 0.05 | ai-generated | ElevenLabs sound effects (eleven_text_to_sound_v2) + lane N3 make_ready.mjs (ffmpeg) | ElevenLabs Terms (paid Creator plan: commercial use of Output) | C:/Users/kazda/jam/audio-lab/ledger.jsonl (prompts; status/B-audio-au1.md) | 2026-11-12 | B | y | mono 48 kHz, Bink q40; no longer in MS_check (lane AU6); referenced by MS_shot_blocked (V0 and BodyPad V1, lane V3) and MS_save (49828 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_puck_trail.uasset | vfx | 0.34 | original | UE 5.9 Niagara via MCP |  |  | 2026-11-14 | B | y | puck ribbon trail by speed: speed max(User.Speed, owner velocity) eased over 0.1 s (held on a near-zero time step or above 90 m/s); visible from a floor (12 m/s while User.Carried is -1, the default; 5 m/s at User.Carried 0; off at 1) over 4 m/s; brightness lerp(0.4, 1, (speed - 12) / 28); width 8 + 12 x (speed - 12) / 28 cm; life 0.15 s, so length = speed x 0.15 s; material M_Fx_PuckTrail; User.Speed (m/s, default 0), User.Carried (default -1), User.Colour (lanes B3, B18, B37, V3) (338770 bytes) |
| Game/Plugins/JamAssets/Content/VFX/M_Fx_PuckTrail.uasset | material | 0.02 | original | UE 5.9 editor via MCP/Python |  |  | 2026-11-14 | B | y | translucent unlit soft ribbon for the puck trail: soft across its width; on clay an HDR cream (2.4, 2.3, 2.0) at ClayOpacity 1.0 times the emitter's speed alpha; off clay the particle colour x 2.5 at 0.8 (MPC_Look.Clay) (lanes B37, V3) (15702 bytes) |
