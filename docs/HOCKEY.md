# Hockey 3v3: the game and its milestone ladder

Status: plan v2, 2026-11-12 ~01:00 CET, before the jam. This is a plan (milestones, contracts, acceptance), not game content: no code,
level or asset exists. Rule 7(h) allows plans, prompts and scripts; everything below is built after 06:00 CET. Source: spark
`megajam-unreal` second phase; the Foreman's Godot hockey job (`third-life/docs/reports/JOB3-HOCKEY.md`) is the evidence base.

## 1. The game in one paragraph

Ice hockey, three against three. The player controls one skater (WASD or stick, one button to shoot, one to check), two bot
teammates, three bot opponents, one puck, two nets, no goalies, first to N goals or a timer. It is readable from a high three-quarter
camera that always shows both nets. Every number that makes it a *variation* (rink size, friction, board bounce, speeds, shot power,
rule flags, pickups, team colours, light tier) lives in a cartridge the Designer can edit. Difficulty tiers are also art tiers:
the regular game is a bright toy rink, raw play is the same rink at night, boost play is the same rink in full neon.

## 2. What the Godot job already taught (carry over, do not rediscover)

| Fact | Value | Why it matters |
|---|---|---|
| Rink | 60 x 30 m, corner radius 5 m, boards all round | fits the camera and six skaters; keep |
| Goals | 4.0 m wide, 1.5 high, 1.5 deep, at x = +-26; trigger box behind the line, puck-only | goal detection by overlap, scoring side = opposite |
| Skater | speed 8 m/s, accel 12, turn 5 rad/s, lateral grip 3, mass 80, glide drag 0.147 m/s^2 (0.015 x 9.8), brake as a separate decel | arcade skating that still slides; keep as tunables (`rink.ice_friction` is the glide friction coefficient, 0.015) |
| Simulation | **plain fixed-step C++ at 60 Hz for skaters and puck; Chaos only for cosmetics** (tipped figures, debris) | Chaos substepping and async tick are experimental in 5.9 and carry no determinism guarantee (`docs/SKATERS-RESEARCH.md` 4-5); the same-hash-per-seed test needs our own integrator, as the Godot kernel did. Puck: 4 sub-steps per frame so 30 m/s never tunnels a post |
| Puck | radius 0.12 m, height 0.08, max 30 m/s, near-black | **0.35 m made the player lose 1:3**; readability comes from colour and VFX, never size |
| Shot | reach 1.5 m, exit 18 m/s regardless of mass, cooldown 24 frames, must face within 60 deg | keep; add charge-up as a tunable later |
| Goal pause | 120 frames, then faceoff reset at centre, skaters at spawns facing the far net | keep |
| Camera | Godot used a fixed arena camera [0, 52, 14] fov 55 that cropped both nets | **Decision 2026-11-12 (operator): third-person camera** behind and above the player's skater, following with lag, looking down the ice; the fixed arena camera stays as cartridge option `game.camera.mode = "arena"`. A radar (mini-rink) in the HUD and a far-net marker replace "both nets in frame" as the readability device |
| Rink size | Godot 60 x 30 m | **Decision 2026-11-12 11:45 (operator, R-0010): the default is 90 x 45 m with skater speed 11**, chosen on measured pacing (a goal every 70 s vs 130 s, 16 vs 2 of 20 bot matches finishing, lanes A2 and A10); 120 x 60 stays as the `Large Rink` cut. Earlier that morning the default had been set to 120 x 60 (`game.rink.length_m` 120, `width_m` 60); the Godot numbers for speed and shot are kept as the starting tunables and re-balanced by the A3 sweep lane (a skater crosses 120 m in about 15 s at 8 m/s). **Evidence against the default** (lane E4, 2026-11-12): six skaters on 7200 m^2 are about 4.5x sparser than an NHL 3-on-3 overtime rink and the view feels empty; the A3 lane's first sweep is `rink 90 x 45, skater.speed 11` versus `120 x 60, speed 8`, and the operator decides on the bot-tournament numbers and a look at both |
| AI | stateless per tick; roles chaser / support / back; stall detector 120 frames; **yield rules**: blue bots yield when the human is nearer the puck (+2 m, within 15 m); red non-backs hold when the human is within 4.5 m of the puck | without the yield rules the human never touched the puck and the core bot had 0 shots |
| Tests | player bot must WIN with >= 3 shots; AI bot: every skater moved >= 3 m, puck path > 30 m, >= 1 AI shot | "the human has something to do" is a gate, not a hope |
| Critic | puck invisible from above, team colours hard to read from above | jersey colours must contrast on ice from the top; puck gets a trail and a shadow disc |

## 3. Milestones (H = hours after Thu 06:00 CET)

| M | Name | Target | Art tier | What exists at the end | Acceptance (checkable) |
|---|---|---|---|---|---|
| M0 | Greybox | H6 | none (unlit) | rink + boards + two nets from procedural geometry, one player skater (capsule), puck, shoot, goal trigger, score HUD, restart | player can score on an empty net in under 30 s; packaged Win64 Development build runs 10 s |
| M1 | **Regular hockey (MVP)** | H24 | L1 daylight toy | 3v3 with bots and the yield rules, faceoff reset, first-to-3 or 3-minute timer, toy skaters with team colours, shoot and pass-by-shooting, cartridge `hockey.regular`, D0 Designer presets, audio pass 1 | automated player bot wins with >= 3 shots at seed 1, 2, 7; AI bot passes; human playtest: a fresh player scores within 2 minutes; package Shipping under 700 MB |
| M2 | Raw play | H48 | L2 night arena | body check (one button): contact above a speed threshold knocks the target down for `knkdown_frames`, puck drops; no rules (no offside, no penalties); scuffed ice and tipped figures; cartridge `hockey.raw` | check lands in the bot test >= 5 times per match; the human is knocked down at most 40% as often as bots at default tuning; frame p99 under 8 ms at 1080p on A |
| M3 | Boost play | H56 | L3 full neon | pickups from the spawn table: speed, shot precision (auto-aim cone), heavy puck, big puck (sic: readable only in neon), reversed controls for the other team; cartridge `hockey.boost` | pickups spawn, apply and expire by frames; each pickup has a sound and VFX; L3 passes the frame gate or ships as L2 with neon trim |
| M3b | Arena manipulation | H56 | any | cartridge changes rink length and width, board restitution, ice friction, goal width, static obstacles on ice; the rink is rebuilt from the cartridge at load | three preset arenas load and the bot test passes on each; the Designer reaches them from a sentence |
| M4 | Polish | H72 | best tier that passes | tutorial text, menu, Designer D1 live with a key, replay of the last goal, final audio, final lighting | feature freeze; clean-machine run; size under 1 GB |

The ladder is cumulative: M2 keeps M1 as a cartridge, M3 keeps both. The submitted build offers all cartridges that pass their tests;
the first-play experience is M1 with the highest art tier that passes the frame gate.

## 4. Cartridge for hockey (the `game` object of `schema/cartridge.schema.json`, written in hour 1)

Wire names (snake_case; durations in `_frames`; absent value = omit the key):

```
game.rink: { length_m, width_m, corner_radius_m, board_restitution, ice_friction, goal_width_m, obstacles: [ {kind, x, z, size} ] }
game.skater: { speed, accel, turn_rate, lateral_grip, mass, glide_drag, brake_decel, check_enabled, check_speed_min, knockdown_frames }
game.puck: { radius_m, max_speed, drag, colour_hex }
game.shot: { reach_m, speed, cooldown_frames, aim_cone_deg }
game.rules: { mode: "first_to" | "timer", score_to_win, timer_frames, goal_pause_frames, offside: false, penalties: false }
game.teams: [ { id, colour_hex, skaters: [ { role: "player" | "chaser" | "support" | "back", spawn: [x, z] } ] } ]
game.ai: { yield_margin_m, yield_radius_m, hold_radius_m, speed_scale, red_skill }
game.pickups: [ { kind: "speed" | "precision" | "heavy_puck" | "big_puck" | "reverse_other", spawn_every_frames, duration_frames, magnitude } ]
game.look: { tier: "l1_day" | "l2_night" | "l3_neon" | "p3_clay" | "p4_broadcast", trim_colour_hex, fog_density, wet_ice: bool, trim_emissive? (0..100; absent = the tier's default, 30 or 12 for clay) }
game.camera: { mode: "third_person" | "arena", distance_m, height_m, look_ahead_m, lag_frames, fov_deg, radar: bool }
  defaults measured by lane E4 (`docs/experiments/CAMERA-120M.md`): distance 6, height 3.5, look_ahead 5, lag 4 frames, fov 70 (wide option 8 / 4 / 6 / 6 / 85);
  pitch derived as -atan(height / (distance + look_ahead)); the pawn sits 0.7 m to the side of the camera axis or it hides the puck for the first 6.4 m;
  lag is frame-based on the sim tick (the engine spring arm's DeltaSeconds lag drifts 0.67 m at 60 Hz vs 0.43 m at 30 Hz); far net = UMG screen marker with
  distance text (beacon optional, brighter frame useless past 60 m); radar = C++-driven UMG mini-rink 200 x 100 px bottom-right; puck ships plain near-black with a
  trail and a contact-shadow disc (emissive rim marginal).
```

Binding of `values` to `game` (lane A4, accepted): a tunable whose key names a game field (`skater_speed`, `shot_speed`, `ice_friction`, ...) dials that field; `values[key]` overrides the field when present; the engine loader mirrors `schema/validate.mjs` exactly (`schema/README.md`). `rink.obstacles[].kind` is `box | pillar`; `rules.offside` and `rules.penalties` are always `false`.

Presets shipped as D0 cartridges: `hockey.regular` (M1), `hockey.raw` (M2), `hockey.boost` (M3), three `hockey.arena.*` (M3b).
The Designer may change `values`, `theme`, `pickups`, `rules.mode/score_to_win/timer_frames`, `look.tier`, `rink` numbers inside limits.

## 5. Skaters: one contract, three tracks

The skater is the biggest risk and the key to fun. Gameplay never knows what a skater looks like: it drives an `AJamSkater` pawn
(movement, puck handling, checks) and talks to its visual through one interface so the look can be swapped without touching play code.

`IJamSkaterVisual` (C++ interface, implemented by a component the pawn owns; wire names fixed now):
`SetTeam(team_id, colour_hex)`, `SetNumber(n)` (drives material params `NumberTens`, `NumberOnes`), `UpdateMotion(speed, turn_rate, facing)`,
`PlayShoot()`, `PlayCheck()`, `SetKnockedDown(bool down, float dir_x, float dir_z)` (the direction the figure falls), `SetHasPuck(bool)`, `SetBoost(kind | none)`.
Recipes: `tools/skills/skater-toy-figure.md` (T-A: engine basic shapes posed by C++ each tick, no skeleton; pose timings reuse our own Godot
rig's keyframes, declared in the dev log), `tools/skills/skater-retarget.md` (T-B1/T-B2), `tools/skills/skating-physics.md` (movement, puck, determinism test).
Verified by R2 (`tools/mcp/ANIMATION.md`, `PHYSICS.md`): MCP builds an Animation Blueprint, its variables, state contents and transition rules, a Control Rig
with nodes, a pawn with a C++ lean component, a Niagara system; it cannot add **states or transitions** to a state machine (no tool, no Python, no DSL), so the
project carries an 80-line C++ editor helper `UTJAnimGraphEdLibrary` (source in ANIMATION.md) from H0, because Live Coding cannot add a module later. Retargeting
UE4 -> UE5 mannequin works in one Python script (`run_batch_retarget`), no human needed. Chaos repeats to six decimals across runs and a restart only with
**Use Fixed Frame Rate = 60** (`DefaultEngine.ini`, set at H0); our own fixed-step sim still owns gameplay.

| Track | Owner | Window | What | Promotion rule |
|---|---|---|---|---|
| T-A toy figure (MVP) | device A | H1-H24 | capsule body, round head, stick, jersey colour and number as material params; lean into turns, bob with speed, stick swing and knockdown tip-over by code; deterministic | ships in M1; stays the fallback forever |
| T-B2 generated rigged (Meshy), **now first** | device B | H8-H24 | proven on 2026-11-12 (lane E1, `docs/experiments/MESHY.md`): text-to-3D toy skater in the P2 look, 20.8k tris, one 2K texture, `ue5_mannequin` bone names, text-to-motion skating stride, slap shot, library knock-down; about 55 credits and 5 minutes of API time per figure; strip the pelvis drift; attach the stick to `hand_r`; counts toward partner prizes | swap into a test cartridge at H24; promoted only if the player bot still wins and the frame gate holds |
| T-B1 humanoid, sourced clips (fallback) | device B | H24-H40 | the Fab "Hockey Player Animated" pack retargeted UE4 -> UE5 by the one-script path R2 proved, plus Mannequin falls; implements the same interface | same rule; evaluated at H40; only if T-B2 failed its gate or its clips do not loop |

Both device-B tracks are experiments by design: the game ships on T-A unless a track clears its gate.

## 6. Art ladder on one geometry

**Decision 2026-11-12 01:00 (operator):** the look to chase is **P2 painted cel at tier T2, night arena** (`prep-reference/STYLE-PATHS.md`), on the baseline ladder's
**L2 "Indoor, spotlights on"** arena (`prep-reference/HOCKEY-LADDER.md`, L2-b): house lights down, spot cones, emissive trim in team colours, scuffed ice, dark ceiling,
**no crowd**. The ladder starts at night; day (L1) and neon (L3) variants are extensions if time allows. **P3 clay miniature and P4 stylized broadcast (both at T2) are later
stages or levels**, taken only on the success of P2 and the progress of the parallel lanes. The table below keeps the three lighting tiers as the mechanism; the shipped order is L2 first.
Team name: **Hokej**.

| Tier | Game tier | Lighting rig | Post process | Materials (same assets, parameters change) | Costly part |
|---|---|---|---|---|---|
| L1 day | M1 | sun + sky light, Lumen GI, soft shadows | mild bloom, warm grade | flat tint, rim fresnel, plastic gloss | the sky and stands dressing |
| L2 night (**ships first**, painted cel) | M1 | low cool sky light, 4 shadowed spots, blue and orange emissive trim; **measured by lane E3** (`docs/experiments/LOOK-P2-NIGHT.md`, `tools/skills/look-painted-cel.md`): 4.3 ms GPU at 1080p on the 4090, 2.9 ms at the low profile (`sg.ShadowQuality 1`, `sg.GlobalIlluminationQuality 1`), Lumen is 2.5-3 ms of it | bloom 0.9, exposure fixed EV 4.2, cool shadow grade, vignette 0.3, no fog (one volumetric spot costs 0.3-0.5 ms and washes far figures out); **toon ramp and outline are post-process passes by custom stencil** (0.04 ms together); outline fades with distance (15 to 40 m, min 0.3) or far sticks turn solid black | same masters; `TeamColour`, `Emissive`, `Scuff` params | the puck: a 4 x 1 px dash at 40 m, invisible on dark ice; ship a saturated glowing disc under it plus the trail; the far net needs its beacon or the screen marker |
| L3 neon | M3 | neon strips (emissive), fog, Lumen reflections on wet ice | heavy bloom, high contrast, chromatic touch | ice material wet variant (roughness down, reflection on), glowing pickups | reflections: hardware RT on, software fallback looks flat; fog cost |

The three rigs are three sub-levels (or Data Assets) toggled by `game.look.tier`; the arena geometry is one set. Reference stills for the
three tiers (nine images, reference only) are in `prep-reference/HOCKEY-LADDER.md` once builder B5 finishes; the operator picks there.

## 7. Tests that gate every milestone

- `PlayerBotWins`: scripted player bot vs AI at seeds 1, 2, 7: WIN with >= 3 shots, same state hash per seed (determinism).
- `AiBotPlays`: every skater moved >= 3 m, puck travelled > 30 m, >= 1 AI shot, no skater outside the rink.
- `HumanHasSomethingToDo`: the player's skater touches the puck within 600 frames of every faceoff at default tuning.
- `FarNetReadable`: in third-person mode the far net's marker is inside the viewport and the radar shows all six skaters and the puck; in arena mode both goal triggers project inside the viewport.
- `FrameBudget`: p99 frame under 8 ms at 1080p Shipping on device A for the shipped tier.
- `Package`: `tools/run/package.ps1` Shipping passes; staged size and zip under budget.
- `CartridgeRoundTrip`: every shipped cartridge validates, loads, and the bots pass on it.
