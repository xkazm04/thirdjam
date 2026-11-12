# Lane B3b report: effects and sounds for the new gameplay events (device B)

- Worktree `C:/Users/kazda/jam/wt/b-vfx`, branch `lane/b-vfx`, fast-forwarded to `dev-b` (`4c415cc`) at 13:38.
- Build: PASS on the first try (114.6 s). There was no locked or half-written intermediate, so nothing was deleted.
- Editor on port 8002 (43.5 GB free at start), 13:42 to 14:03.
  - **The first session did not end cleanly: the editor crashed** (details under "Not finished"). Every asset had been saved before the crash, and `Rink` was not written.
  - I restarted the editor and loaded all 21 assets back from disk. All of them load, the Niagara systems compile `UpToDate` with no errors, and no package was dirty. That session closed with `ExitEditor` (log: `LogExit: Exiting.`, no fatal line).
- Built: 9 new Niagara systems and 11 new MetaSounds. `MS_faceoff` was rebuilt with a `Variant` input.
- Verified in PIE on `/Game/Maps/Rink` with the cartridge-loaded night rig (`LR_L2_Night` streamed in by the game, `HokejGameMode`, a live match).
- Commits: `2bc21b4` (the first five events), and the final commit that carries this report.

## 1. Gap list (C-0003 v1 + additions, C-0005 §6, the header `JamEventBus.h`, and the R-0013 names from the brief)

**Important:** R-0013 (stick poke, lift, fouls, the real faceoff) is **not on `dev-b`**. `contracts/RULINGS.md` there ends at R-0012. `handoff/A-to-B-0004.md` is not on `dev-b` either.
- The prompt allowed only merge, add and commit, so I did not read either one from another branch.
- The event shapes for `poke`, `lift`, `foul` and the faceoff kinds come from the lane brief. **The foul kinds and their order are unknown to me** (see "Lead must decide", point 1).

| Event | Wants | Reading | NS_ before | MS_ before | Now |
|---|---|---|---|---|---|
| shot | both | the puck leaves the stick | yes | yes | unchanged |
| board | both | puck hits the boards | yes | yes | unchanged |
| goal | both | the big moment | yes | yes | unchanged |
| faceoff `drop` / `won` / `lost` | both | the drop is the clapper "action!"; won and lost are small accents | no | yes (clapper) | **`NS_faceoff` new, `MS_faceoff` + Variant** |
| check | both | body contact | yes | yes | unchanged |
| knockdown | both | skater falls | yes | yes | unchanged |
| pickup_spawn / pickup_take | both | pickup appears / is taken | yes | yes | unchanged |
| win / lose | sound (+ confetti on win) | match end | win only (by design) | yes | unchanged |
| directors_call | both | operator tunable changed | yes | yes | unchanged |
| period (in the header, `Kind = take_<n>`) | sound, maybe | a TAKE starts; the HUD card carries it | no | no | **not built** (outside this brief; lead decides) |
| poke `clean` / `miss` | both | a stick check on the carrier: a crisp hit or a whiff; frequent, so small | no | no | **new** |
| lift | both | puck flipped off the ice: a scoop and a spray | no | no | **new** |
| foul (kinds per R-0013) | both | play is stopped for an infraction: whistle plus a red "cut" light | no | no | **new** |
| pass | both, tiny | very frequent; a soft tock and a flick | no | no | **new** |
| effect_start / effect_end | both, short bursts | a boost attaches to / leaves a skater (no looping aura) | no | no | **new** |
| zone_enter / zone_exit | both, subtle | a skater crosses a zone edge; the strip itself is the zone's visual | no | no | **new** |
| announce | sound only | a rule text card on the scoreboard; the card is the visual | no | no | **new (`MS_` only)** |
| bark | sound only, tiny UI blip | a skater speaks; device A voices the line | no | no | **new (`MS_` only)** |
| signal `defend`/`space`/`pass`/`praise` | sound only, tiny UI blip | the human's team signal; device A voices it | no | no | **new (`MS_` only)** |

## 2. PASS / FAIL per asset with evidence

How each asset was checked:
- **Effects:**
  - spawned from Python in the PIE world at the lit pool (x 11.25 m, y 0), with `Strength` / `Variant` / `Colour` set;
  - time dilation 0.1;
  - `CaptureGameViewport` through a helper `CameraActor` (placed in `Rink` in memory only, removed before exit, never saved);
  - the previous burst was hidden before each spawn, so frames do not overlap.
- **Sounds:**
  - spawned 2D in PIE at Strength 0.2, then 1.0, 2.9 s apart, once per Variant;
  - the output submix was recorded to wav;
  - **gameplay was frozen with time dilation 0.01 while recording**, because a first pass picked up the live match's own event sounds;
  - each wav was measured: peak, and length until the level falls below -45 dBFS.
- Peaks below are given as "at 0.2 / at 1.0".

| Asset | Verdict | Evidence |
|---|---|---|
| `NS_poke` | **PASS** | **V0 clean:** 10 + 10 x S sparks in the puck accent (0.75, 1, 0.15) mixed 30 % with Colour, 4-9 cm, 0.12-0.22 s, gravity. **V1 miss:** 6 + 6 x S grey-blue dust puffs, 8-16 cm, 0.25-0.4 s. Still `b3b-fx-poke` (both variants at 1.0 / 0.2). |
| `MS_poke` | **PASS** | **V0:** 3.2 kHz band click (8 ms), 420 Hz knock and 1.2 kHz click; -16.1 / -10.3 dBFS, 35 ms. **V1:** band-noise swish sweeping 600 to 2500 Hz; -22.7 / -16.4 dBFS, 190 ms. Gain 0.4 + 0.6 x S. |
| `NS_lift` | **PASS** | 14 + 16 x S ice chips, upward fan with gravity, 0.3-0.5 s, ice white tinted 20 % by Colour. Still `b3b-fx-lift`. |
| `MS_lift` | **PASS** | Sine scoop gliding 180 to 420 Hz, a 4 kHz flick and an airy 1.5 kHz tail; -12.6 / -6.5 dBFS, 170-265 ms. |
| `NS_foul` | **PASS** | 32-sprite ring (red mixed 35 % with Colour) at 40 cm, expanding at 350 + 150 x Variant cm/s, 0.5 s, plus one fixed red 1.4 m flash for 0.2 s. Still `b3b-fx-foul` (V0 / V1 / V2). |
| `MS_foul` | **PASS** | Referee whistle: a 2.9 kHz sine with a 28 Hz +-110 Hz trill, plus breath noise. **V0:** one blast, 440 ms. **V1:** two blasts, 375 ms. **V2:** a long blast plus a 110 Hz buzzer, 865-880 ms. Peaks -13.6 / -10.1 (V0), -13.8 / -10.5 (V1), -8.4 / -5.0 dBFS (V2). |
| `NS_faceoff` (new; C-0003 already names it) | **PASS** | **V0 drop:** 24-sprite ice ring at the dot, 0.3 s. **V1 won:** 30 sparks rising in Colour. **V2 lost:** 12 slower sparks, Colour half-greyed. Still `b3b-fx-faceoff`. |
| `MS_faceoff` (rebuilt) | **PASS** | **V0** = B3's clapper, unchanged: -14.7 / -11.1 dBFS, 95 ms (B3 measured -14.7 / -11.3, 95 ms), so the default behaviour is preserved. **V1 won:** G5 then C6, 295 ms, -14.8 / -11.4 dBFS (zero-crossing estimate 787 Hz). **V2 lost:** E4 then C4, low-passed saws, 380 ms, -13.1 / -9.7 dBFS (333 Hz). Gain 0.6 + 0.4 x S, so it is audible at the contract's Strength 0. |
| `NS_pass` | **PASS** | 6 sprites, 3-6 cm, 0.12-0.18 s, speed by `saturate(S)`. Deliberately tiny. Still `b3b-fx-pass`. |
| `MS_pass` | **PASS** | 300 Hz tock, 2 kHz click and a light swish; -17.8 / -10.9 dBFS, 70-110 ms. Strength is clamped to 0..1 inside the graph. |
| `NS_effect_start` | **PASS** | 40-sprite ring of 60 cm at the skates rising at 3-4 m/s, plus one 1.6 m flash at 0.9 m; Colour; 0.5-0.65 s. Still `b3b-fx-effect` (top row). |
| `MS_effect_start` | **PASS** | Sines on a fifth gliding up (500 to 1000 and 750 to 1500 Hz in 0.25 s) and a 6 kHz sparkle; -9.8 / -8.3 dBFS, 420 ms. |
| `NS_effect_end` | **PASS** | 32-sprite ring of 70 cm starting at 1.7 m and falling, Colour 40 % greyed, 0.45-0.55 s. Still `b3b-fx-effect` (bottom row). |
| `MS_effect_end` | **PASS** | Sine plus a soft saw falling 900 to 450 Hz over 0.3 s; -14.6 / -13.1 dBFS, 360 ms. |
| `NS_zone_enter` | **PASS** | 20 glints on a 40 cm ring expanding at 1.5-1.8 m/s, Colour, 0.3-0.35 s. Still `b3b-fx-zone` (top row). |
| `NS_zone_exit` | **PASS** | 20 half-grey glints on a 90 cm ring contracting (negative speed), 0.3-0.35 s. Still `b3b-fx-zone` (bottom row). |
| `MS_zone_enter` / `MS_zone_exit` | **PASS** | An E4 to B4 blip (exit: B4 to E4) over an 80 Hz thump; -10.9 / -9.4 and -10.4 / -8.9 dBFS, 195-200 ms. |
| `MS_announce` | **PASS** | Brass-ish stab: G-major triad, then a C-major chord at 0.16 s, low-passed saws, over a 65 Hz drum; -6.5 dBFS at both strengths (fixed gain), 755 ms. The first build peaked at -2.1 dBFS and was turned down. |
| `MS_bark` | **PASS** | Speech-bubble pop: 1319 Hz, then 1760 Hz; -18.7 dBFS, 85 ms. Fixed gain, quiet on purpose. |
| `MS_signal` | **PASS** | Two 45-60 ms pulses, 70 ms apart; -16.9 dBFS, 135 ms. Pitch by Variant, checked by zero crossings: V0 543 Hz (C5), V1 657 Hz (E5), V2 800 Hz (G5), V3 1057 Hz (C6). |

Acceptance checks:
- **Self-destruction: PASS.**
  - All 14 system / variant combinations were spawned with auto-destroy at Strength 1. All were active at spawn and none was active 4.5 s later (real time at dilation 1).
  - Every emitter comes from the `OmnidirectionalBurst` template, as in lane B3: life cycle Self, loops Once, inactive response Complete.
- **Sprite cap: PASS, by construction** (no tool reads live counts). Peaks:
  - poke 20 (clean) / 12 (miss);
  - lift 30, foul 33, faceoff 24 / 30 / 12, pass 6;
  - effect_start 41, effect_end 32, zone_enter and zone_exit 20 each.
  - All are far under 200.
- **MetaSounds:** all are one-shots that end through `OnFinished`, synthesised in-graph with no waves, class `SC_SFX` (checked after the reload from disk).
- **Frame cost:** not measured. Two editors were up, so any number would have been "unconfirmed, two editors up". The bursts are CPU sprites, smaller than lane B3's `NS_goal`.

## 3. Parameters and inputs by asset (device A's binder must use exactly these names)

**Niagara** (all new systems): `User.Strength` (float 0..1, default 0.5; `NS_faceoff` defaults to 0) and `User.Colour` (linear colour 0..1, default white). From code:
- `SetVariableFloat("Strength", s)`;
- `SetVariableLinearColor("Colour", c)`;
- `SetVariableFloat("Variant", v)`.

**MetaSounds**: float input `Strength` on every one, and the float input `Variant` on the four listed below.

**Variant mappings** (the payload `Kind` turned into a float index; integers only):

| Asset | `Variant` 0 | 1 | 2 | 3 |
|---|---|---|---|---|
| `NS_poke`, `MS_poke` | `clean` | `miss` | - | - |
| `NS_faceoff`, `MS_faceoff` | `drop` | `won` | `lost` | - |
| `NS_foul`, `MS_foul` | 1st foul kind in R-0013's order (single whistle) | 2nd kind (double whistle) | 3rd kind (long whistle + buzzer, wider ring) | - |
| `MS_signal` | `defend` | `space` | `pass` | `praise` |

- **The foul row is provisional:** R-0013's foul list is not on `dev-b`. Severity rises with the index; the lead maps the kinds.
- A missing `Variant` means 0, which is the old behaviour for `MS_faceoff`.

What each Strength means, and the ignored inputs:
- **Strength meaning:**
  - poke and lift: hit or lift speed, 0..1;
  - pass: the pass speed **normalised 0..1** (C-0005 says "Strength = speed"; it is clamped inside both assets, so sending m/s would saturate rather than break).
- **Inputs exposed but ignored:**
  - `Strength` is ignored by `NS_foul`, `NS_faceoff`, `NS_effect_*` and `NS_zone_*`;
  - `MS_announce`, `MS_bark` and `MS_signal` play at a fixed gain;
  - `MS_effect_*` and `MS_zone_*` vary by only 0.8 to 1.0;
  - `NS_foul`'s flash is fixed red; its ring takes 35 % of Colour.

## 4. Files created (bytes on disk) and ledger rows

New, total **6,534,114 bytes**:
- **VFX:** `NS_poke` 569,986, `NS_lift` 331,442, `NS_foul` 564,393, `NS_faceoff` 570,147, `NS_pass` 328,503, `NS_effect_start` 558,510, `NS_effect_end` 323,877, `NS_zone_enter` 322,701, `NS_zone_exit` 323,052.
- **Audio:** `MS_poke` 237,579, `MS_lift` 188,906, `MS_foul` 333,753, `MS_pass` 167,837, `MS_effect_start` 225,859, `MS_effect_end` 181,658, `MS_zone_enter` 149,652, `MS_zone_exit` 149,647, `MS_announce` 261,783, `MS_bark` 137,241, `MS_signal` 279,483.
- **Changed:** `MS_faceoff` 154,032 → 328,105 (its ledger row must be updated).
- Nothing exists under `_Scratch/`. Captures, sheets and wavs stay in my scratchpad (`.../scratchpad/b3b/`) and in `Game/Saved/BouncedWavFiles/b3b/` (not committed).

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/VFX/NS_poke.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event poke; User.Strength, User.Colour, User.Variant (0 clean, 1 miss) (569986 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_lift.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event lift (331442 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_foul.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event foul; User.Variant = foul kind index (564393 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_faceoff.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event faceoff; User.Variant 0 drop, 1 won, 2 lost (570147 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_pass.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event pass (328503 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_effect_start.uasset | vfx | 0.6 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event effect_start (rising ring + flash) (558510 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_effect_end.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event effect_end (falling ring) (323877 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_zone_enter.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event zone_enter (322701 bytes) |
| Game/Plugins/JamAssets/Content/VFX/NS_zone_exit.uasset | vfx | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | event zone_exit (323052 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_poke.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); inputs Strength, Variant (237579 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lift.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (188906 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_foul.uasset | audio | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); inputs Strength, Variant (333753 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_faceoff.uasset | audio | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); inputs Strength, Variant (0 drop clapper, 1 won, 2 lost) (328105 bytes; replaces the 154032-byte row) |
| Game/Plugins/JamAssets/Content/Audio/MS_pass.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (167837 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_effect_start.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (225859 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_effect_end.uasset | audio | 0.2 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (181658 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_zone_enter.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (149652 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_zone_exit.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (149647 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_announce.uasset | audio | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (261783 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_bark.uasset | audio | 0.1 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves) (137241 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_signal.uasset | audio | 0.3 | original | UE 5.9 Niagara / MetaSounds via MCP/Python |  |  | 2026-11-12 | B | y | MetaSound source synthesised in-engine (no waves); inputs Strength, Variant (0 defend .. 3 praise) (279483 bytes) |

## 5. Stills (`devlog/shots/`, PNG contact sheets of centre crops from 1920-wide PIE captures, each with a caption `.txt`)

- `20261112-135617-B-b3b-fx-poke`, `-fx-lift`, `-fx-foul`, `-fx-faceoff`, `-fx-pass`
- `20261112-135737-B-b3b-fx-effect`
- `20261112-135738-B-b3b-fx-zone`

## 6. Commits

- `2bc21b4`: poke, lift, foul, faceoff (NS new, MS + Variant), pass, and their five stills.
- The final commit (this report): effect_start/end, zone_enter/exit, announce, bark, signal, and the two stills.

## 7. Unfinished or unverified

- **Nobody has listened to any sound.** They are verified by recorded peak and length, Variant pitch by zero-crossing counts, and graph read-back (inputs `Strength` and `Variant` present after the reload). The timbres are first drafts.
- **Editor crash (my mistake).** To discard the in-memory helper camera before exit, I called `EditorLoadingAndSavingUtils.reload_packages` on the open `/Game/Maps/Rink`.
  - The editor died with `Fatal error: World Memory Leaks` (EditorServer.cpp:1950); the crash dump is in `Game/Saved/Crashes/` (ignored).
  - `Rink.umap` is untouched (mtime 12:40, 18,966 bytes), and every asset was saved before the crash and reloads clean.
  - **Pitfall for the next lane:** never reload the open map. `ExitEditor` refuses while `Rink` is dirty, so put helper cameras in the PIE world some other way, or accept that the map must be closed without saving.
- **Readability on lit white ice.** The additive sprites (`M_Fx_Sprite`) only lighten, so grey and white parts (poke miss, lift, faceoff lost) read faintly on the lit pools; colour parts read well. The night bands and backdrop show them better (see the effect still).
- **Not run through device A's binder.** Python called the functions the binder will call.
- **No GPU numbers.** Two editors were up.
- **R-0013 unread.** As above.
- **`period` has no asset.**
- **Commands outside the allowed list.** Besides the allowed merge / add / commit, I ran read-only `git status`, `git log` and `git show --stat` in the worktree, and `git log` in the main tree, to check state. None of them changes anything.

## 8. What device A's binder must do (and may not do yet)

1. **Kind to Variant.** Map `Kind` to the `Variant` float using the table in section 3, and set it on both the Niagara component and the audio component before activating. The binder's current rule (`NS_<event>`, `MS_<event>`, Strength and Colour) does not know Variant yet.
2. **2D sounds.** Play `MS_bark`, `MS_signal` and `MS_announce` as 2D UI sounds, not at `Where`, and spawn no `NS_` for them.
3. **Placement and colour of `effect_start` / `effect_end`.** Spawn at the target skater's feet (Who), or at `Where` when Who is -1. Colour = the effect's or team's colour (the binder's choice; the asset just takes `Colour`).
4. **`zone_enter` / `zone_exit`.** Kind is a zone **id**, so B cannot give a Variant per zone kind. The binder should look up the zone's kind and pass a colour per kind (for example slow blue, fast orange) through `Colour`.
5. **Faceoff.** Fire `drop` with Variant 0; old binders that send no Variant still get the clapper. Decide whose colour `won` / `lost` use; the asset uses `Colour` for the won sparks.
6. **Pass Strength.** Send the speed normalised 0..1, not m/s.

## 9. What zone strips and pickup looks need from device B (not this lane; for the lead's brief)

- **Zone strips** (C-0005 `game.rink.zones`, kinds `slow_ice`, `fast_ice`, `bumper`, `no_shoot`, `boost_only`; shapes rect and circle):
  - a decal or `MI_Ice_Night` parameter set per kind, readable on both the lit pools and the dark bands, with no text or letters;
  - a colour per kind agreed with the binder (the same colours feed `NS_zone_*`);
  - an animated edge (scroll or pulse) for `fast_ice` and `slow_ice`;
  - `visible: false` zones need nothing;
  - A15 must say whether it renders rect and circle meshes itself (then B ships materials `M_Zone` / `MI_Zone_<kind>`) or wants decals (`T_Zone_*`).
- **Pickup looks:**
  - a toy pickup mesh or material and the glyphs `T_Pickup_*` for the built-ins (`speed`, `precision`, `heavy_puck`, `big_puck`, `reverse_other`), plus a generic glyph for Designer-made effects (`icon` field);
  - an idle bob or spin (in A's actor, or a looping `NS_pickup_idle`, which would not be an event effect);
  - a colour per kind, so `NS_pickup_spawn` / `NS_pickup_take` / `NS_effect_*` get a consistent `Colour`.

## 10. Lead must decide

1. **Foul kinds.** Confirm the foul kinds and order from R-0013 for the `MS_foul` / `NS_foul` Variant (0 = mildest), and get R-0013 and A-to-B-0004 merged into `dev-b`.
2. **`period`.** Does it get an `MS_period` (for example the clapper plus a take-number tone)? It was not in this brief.
3. **Additive effects on white ice.** Accept that greys read faintly on lit white ice, or add an alpha-blended (darkening) sprite material for the dust parts.
4. **Faceoff colour.** Whose colour the faceoff `won` / `lost` accents use, and whether `lost` should play at all for the AI side.
5. **Contract update.** Propose adding `User.Variant` / `Variant` and the Kind-to-index tables to C-0003, so the binder change is contractual.
