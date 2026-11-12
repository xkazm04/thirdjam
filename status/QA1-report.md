# Lane QA1 report: device B's content in the game as it ships tonight (device B)

- Tree `C:/Users/kazda/jam/wt/b-vfx` (dev-b merged with A's main of 20:53, built before the lane). **No git command, no build, no asset saved.** Clock 21:01 to 21:37.
- Editor on port 8002 (pid 31020), started 21:02, left through `ExitEditor` at 21:36:52 (`LogExit: Exiting.`, PIE stopped first). **Dirty packages: none** (`get_dirty_content_packages` and `get_dirty_map_packages` both empty, checked twice). `/Game/Maps/Rink` never saved or reloaded (mtime still 12:40). Nothing under `_Scratch/`.
- Another editor (port 8000) shared the GPU: no frame numbers.
- Method: PIE on `Rink`; the menu opened inside PIE (`OpenLevel Menu`, `jam.menuscreen`), the regular cut chosen from it like a player. Bot matches with `jam.ScriptHuman 1`; tackling through A's `tools/qa/tackle.py` (a copy with a capture every 4th frame at 960 x 540, the original stalled the machine at one capture per frame), cartridges = scratch copies of `hockey.tackle.qa.json` / `hockey.regular` with only `visual` or `tier` changed; tier switches with `jam.livecut <path>`; overviews through the debug camera (`ToggleDebugCamera` + moving its spectator pawn). Probes read the tier, `MPC_Look.Clay` (through `ULookTierSwitcher::GetLookClay`), the visual classes, the audio components. Audio recorded from the PIE output submix (`AudioMixerLibrary.start/stop_recording_output`, as AUD and B3 did), measured with ffmpeg `ebur128` and numpy.
- Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/qa1/` (helpers `q.mjs`, `probe.py`, `vis.py`, `audio.py`, `dbgcam.py`, `tackle_qa1.py`; all captures under `shots/`; the editor log copy). Outside the repo's tracked files: `Game/Saved/BouncedWavFiles/qa1/{r1night,skate}.wav`, `Game/Saved/Screenshots/WindowsEditor/qa1-*.png` (tackle frames).

## Checklist

| # | Line | Verdict | Evidence |
|---|---|---|---|
| 1 | Menu look | PASS (B side) | `qa1-menu-main`, `qa1-menu-cuts`: no missing material; a 2D menu on black. The film theme is A's and the jam theme |
| 1 | First frame of a match, faceoff | PASS (B side) | `qa1-r1-bank-nogoal`: rig, sky with stars and tree line, glass, nets, ice with scuff and pools, figures in team colours with 7 and 13; no grey or default surface, no black sky, no z-fighting seen |
| 1 | Floating or unplaced actors | FAIL (A) | the goal light box of A's construction layer stands above the glass behind each net on every tier (visible large on broadcast, `qa1-livecut-strip` panel 4); known from A33's eyes. No B prop unplaced; B's 66 `Con_` stanchions are gone from `LR_L2_Night` (probe: 0) |
| 1 | Haze cards as the rig says | PASS | `Haze_L2_Night_1/2` hidden (saved hidden by B15; SK5 turns them on in the main tree, not in this build) |
| 1 | Log warnings and errors naming `/JamAssets/` | FAIL (B, small) | see "Log warnings" below: 13 `JAMFX missing` lines, the TB1 divide by zero; no material, mesh, Niagara or MetaSound warning |
| 2 | Night T-B1: team colours, numbers 7/13/42 and 9/21/88 | PASS | 7, 13 seen on blue; 21, 88, 9 on orange (`qa1-hit1-side-strip`, `qa1-r1-strip`, `qa1-bcast-hit-strip`); numbers read at the game camera when the back faces it |
| 2 | Crests | NOT SEEN | never faced the camera at a readable size |
| 2 | Stick in two hands, blade on the ice, carried puck on the blade | PASS | `qa1-hit1-side-strip`, `qa1-bcast-hit-strip` frame 1 (orange 21 with the puck on the blade); the idle human seen from behind shows no stick (hidden in front of the body; not proven a fault) |
| 2 | Skating and stopping | PASS | `qa1-r1-strip` |
| 2 | Shooting with a visible wind-up | FAIL (A tuning, 2) | the five bot shots of run 1 logged `JAMSHOT charge=0.00 / 0.07 / 0.00 / 0.00 / 0.00`: at pro the shots are taps, so no wind-up is there to see |
| 2 | Passes, pokes, lifts | PASS (events) / NOT JUDGED (look) | 40 pokes, 35 lifts, 5 passes in run 1, no `JAMFX missing` for them; their effects are small by design (B17) and I did not inspect them frame by frame |
| 2 | Last man's goalie stance | NOT SEEN | 22 `JAMGOALIE` lines in run 1; not caught in a still |
| 2 | Knockdowns by direction and the get-ups | PASS | side_r -> `Knockdown_Right` -> `GetUp_Side_Right`; behind -> `Knockdown_Back` face down -> `GetUp_Front` (`qa1-hit1-side-strip`, `qa1-hit2-behind-strip`, also on clay and broadcast). Front and left not triggered |
| 2 | Mistimed-poke stumble | NOT SEEN | the driver presses check only; no `poke miss` of the human |
| 2 | Ice, boards, glass, nets, snow, sky | PASS | `qa1-reg-overview`, `qa1-r1-strip` |
| 2 | Effects when their event fires | PASS for check, knockdown, goal; FAIL for save, carry (no assets); NOT JUDGED for board, poke, lift, shot, pass, foul, pickups, blade spray | `qa1-hit1-side-strip`, `qa1-r1-goal` |
| 2 | Puck readable in a pool, a dark band, the far end | PASS near, FAIL in motion (3) | the yellow halo marks it everywhere near; while carried at speed it draws a 1 to 3 m yellow-green smear (`qa1-b2-strip` panel 2, `qa1-r1-strip`); far end: the HUD marker carries it |
| 2 | Motion | FAIL (join, 4) | engine motion blur smears the whole frame on every camera turn |
| 3 | `generated_b2` on night | PASS, no better than T-B1 | `qa1-b2-strip`: `BP_SkaterVisual_TB2_C` x6; reads like T-B1 at the game camera, numbers thinner; same puck smear |
| 4 | Clay tier: class, rig, figures, puck, effects | PASS | T-B1 keeps `BP_SkaterVisual_TB1` (A maps tiers only for `generated_b2`); `generated_b2` gets `BP_SkaterVisual_TB2_P3_C` x6 (`JAMVISUAL ... tier=p3_clay`). `MPC_Look.Clay` 1.0. Check on clay: purple ring, grey crumbs, the face-down fall reads (`qa1-clay-hit-strip`) |
| 4 | Broadcast tier: rig, `MPC_Look.Clay` -1, class, glass, puck, effects | PASS | rig `LR_P4_Broadcast`, Clay -1.0, `MI_Bcast_Glass`, 10 lights; `_TB2_P4_C` x6 for `generated_b2`; check: blue-grey ring dots with dark rims, the star faint (`qa1-bcast-hit-strip`) |
| 4 | Live switches in every direction | PASS | night->clay->broadcast->night->broadcast->clay->night on T-B1, night->clay->broadcast->night on T-B2; probe after each: right tier, Clay 0/1/-1/0, right class; no previous sky or ground in any still (`qa1-livecut-strip`) |
| 5 | Large (120 x 60) and wide (140 x 70) rinks | PASS with a caveat | scale 1.333 / 1.556 (`JAMLOOK`); trusses over the ice, towers outside, snow field, horizon; nothing of the rig inside the boards (`qa1-large-overview`, `qa1-wide-overview`). Banks against the boards and benches at close range not judged |
| 6 | Sound | see "Audio numbers": the music plays and ducks 5 to 10 dB, 9 lines in 98 s, none overlap, 4 cut by barks, -23.0 LUFS, -2.8 dBTP, no clipping, the skate loop audible | |
| 7 | Film-set leftovers during a match | FAIL (A HUD, 4) | the "MATCH 1 / PERIOD n - FACEOFF" clapperboard card at every faceoff (`qa1-r1-period-card`); "ESC / START DIRECTOR'S CALL READY" in every frame. Nothing in B's visuals, sounds or the 340 commentary lines (scan for cut, take, director, camera, film, scene, action: only "Skaters on fast forward" / "in slow motion" in the Director's Call lines) |

## The ten faults a player notices first

| Rank | Fault | Visibility | Owner | Smallest fix |
|---|---|---|---|---|
| 1 | **Motion blur smears the whole frame on every camera turn** (`qa1-r1-strip`). No volume or camera overrides it: `PostProcessVolume_0` in `LR_L2_Night` has `override_motion_blur_amount` false, so the engine default (0.5, max 5 %) runs | 5 | join (B's rig PPVs, A's camera) | override Motion Blur Amount to 0 (or 0.1) in each rig's PPV, or A sets it in the follow camera's post settings |
| 2 | **The clapperboard "MATCH 1 / PERIOD n - FACEOFF" card on the ice at every faceoff**, and "DIRECTOR'S CALL READY" in every frame: film on the ice | 4 | A (HUD: `DirectorsCallWidget.cpp` 489 to 527, `HUDWidget.cpp` 338, 501) | a plain "PERIOD n - FACEOFF" banner without the slate; "ESC / START  RULE CALL READY" |
| 3 | **The commentator calls a goal that did not happen**: "Banked it in off the boards!" / "Used the boards like a teammate." on a plain `board` event, the first one 5 s into the first match at 0:0 (`qa1-r1-bank-nogoal`) | 4 | join (B's bank: situation `board.bank` under event `board`; A's picker treats it as a live line) | B gives `board.bank` a `when` that needs a goal in the last 180 frames, or moves it under `goal` (it is already in A's second-beat list) |
| 4 | **The shipped "Boost Cut" looks washed out**: tier `l3_neon`, pale cyan and peach figures blooming, mirror ice (`qa1-boost-washed`) | 4 (when chosen) | join (B's untouched `LR_L3_Neon`; A's cartridge) | A sets `hockey.boost` to `l2_night` (one line); later B re-exposes `LR_L3_Neon` |
| 5 | **Commentator lines cut after a word**: 4 of 9 lines in 98 s cut 0.02 to 0.32 s after they began by an opponent's `steal` bark on the same or the next frame (`JAMVO_CUT reason=bark_at_human`) | 4 (heard) | A (picker) | the picker reads the frame's (and next frame's) steal bark before starting a line, or a bark does not cut a line younger than 0.5 s |
| 6 | **Ghost figures** after a match is rebuilt in place (a Designer cut applied to a running match, `jam.playcut`): the old figures stay frozen on the ice; 18 extra after three rebuilds (`qa1-ghost-figures`) | 5 when it happens; not on the menu or season path | A (`AHokejGameMode::BuildMatch` destroys the skaters, not their attached visual actors) | destroy each skater's visual (the `ResetVisual` teardown) before `S->Destroy()` |
| 7 | **The carried puck draws a yellow-green smear 1 to 3 m long** while skating (`qa1-b2-strip` panel 2) | 3 | B (`NS_puck_trail` / halo; worse with rank 1) | shorten the trail's lifetime or fade it below a speed, after the motion blur fix |
| 8 | **A goal without its call**: "Orange scores!" fell to the 150-frame gate behind "Lifted and gone." 19 frames earlier; only "Nothing anyone could do." 2.2 s later | 3 | A (picker; open question 1 in `COMMENTARY.md`) | a priority-5 line cuts the playing line instead of waiting for the gate |
| 9 | **The commentary caption lingers**: a 2 s line stays on the strip 10 s and across a tier switch (`qa1-livecut-strip` panels 5, 6) | 2 | A (HUD) | clear the strip when the line's `dur` ends plus 1 s |
| 10 | **Own-goal sparks in the wrong colour**: blue skater 2 scores in blue's net, the net bursts blue (`qa1-r1-goal`) | 2 | A (binder colour from `Who`) | colour the goal effect by the scoring team |

## Everything else, by area

- **Menu (A)**: "Raw Cut" and "Boost Cut" cards say "90 x 45 m"; both cartridges are 120 x 60 (`JAMCART ... hockey.boost ... rink=120x60`; `MenuFlow.cpp` `DescribeCut`). Every cut is named "... Cut" and the menu is a film set (clapper, "TAKE 1 OF 8", "DIRECT A CUT"): the jam theme, off the ice; listed only.
- **Effects, missing (B)**: no `NS_save`, `MS_save`, `NS_post`, `NS_breakaway`, `MS_breakaway`, `NS_carry`, `MS_carry`, `NS_period_end`, `MS_period_end`, `NS_clock`, `MS_clock`, `NS_season_take`, `MS_season_take` for A32's seven new events: a `save` and a `carry` fired silently in run 1. Smallest fix: a save thud and a period-end horn first; carry, clock, season_take may stay silent by design (then A should not look them up).
- **Effects, check (B)**: on night the puffs and chips cover both figures for about 0.15 s at frame +8 of a hit (`qa1-hit1-side-strip` panel 3); the fall itself shows after. Visibility 2.
- **Figures (B)**: `BP_SkaterVisual_TB1` logs `Divide by zero: Divide_DoubleDouble` on 2 of 3 knockdowns (log only, visibility 1); fix: guard the divisor in the knockdown event.
- **Figures (A decision)**: with the default `humanoid_b1`, clay and broadcast show the night figure (A's resolver maps tiers only for `generated_b2`). It reads well under both lights; whether B's clay and broadcast figures should come with the default is a lead's call.
- **Camera (A)**: after a hit the follow camera leaves the fallen figure within 0.4 s, so get-ups are rarely seen; the camera clamp near the boards points past the human at times on broadcast (`qa1-bcast-hit-strip` before the hit). Visibility 2.
- **Bots (A tuning)**: pro shots are taps (`JAMSHOT charge` 0.00 to 0.07), so the charged wind-up is rarely shown.
- **Rig (B)**: `LR_P3_Clay` raised `[VSM] Non-Nanite Marking Job Queue overflow` once after the switch (performance only). The centre red line on the night ice reads dark maroon next to bright blue lines (visibility 1).
- **Log (A)**: `JAMLOOK rig ... loaded: MPC_Look.Clay=0.0` is printed before `BP_LookParams` runs, so it says 0 on every tier; the probe reads 1 / -1 a frame later. Misleading only.
- **Construction (A)**: the goal light box floats above the glass behind each net on every tier.

## Log warnings and errors (editor log `Game/logs/editor-20261112-210215-0.log`, 21:02 to 21:37, all runs)

Naming `/JamAssets/` or B's assets:
- `LogJamFx: JAMFX missing /JamAssets/VFX/NS_save` and 12 more (list above), each once per match build.
- `LogScript: Warning: Script Msg: Divide by zero: Divide_DoubleDouble ... called by BP_SkaterVisual_TB1_C` x2 (knockdown frames 1207 and 1420).
- `LogRenderer: Warning: [VSM] Non-Nanite Marking Job Queue overflow` x1 (after the switch to `LR_P3_Clay`; names no asset).

Not B's: `LogUObjectGlobals: Error: CDO Constructor (JamPickupActor): Failed to find /Engine/BasicShapes/Capsule` (A, known from COOK2), `LogPython` enum name clash, `LogOnline` OSS, `LogTextureGraph` NOT FOUND, `jam.Cut` priority note. No material usage, missing mesh, Niagara or MetaSound warning.

## Audio numbers

Run 1: night, `hockey.regular` chosen from the menu, `jam.ScriptHuman 1`, 98.05 s from the PIE output (`r1night.wav`, 20:05:35 to 20:07:13 log time). Music estimated from the side channel (L-R: only the stereo music lives there; voices and effects are mono 2D), speech from 300-3400 Hz of the mid.

| Measure | Value |
|---|---|
| Integrated loudness | **-23.0 LUFS**, LRA 8.8 LU |
| True peak | **-2.8 dBTP**; sample peak -2.84 dBFS; **0 clipped samples** |
| Loudest 400 ms | -16.0 dBFS RMS at 38.5 s: a scrum at the end boards (pass + puck `board` + `lift` clean 1.0 + `poke` clean 1.0 within 0.3 s); next -16.2 (two `lift` clean 1.0 and a steal), -16.5 (`lift` + a line + a `shot`) |
| Music | plays, low: side channel -42.2 dBFS RMS (Level 0.35 in play by A32's rule; with AUD's -23 LUFS at Level 1 that is about -32 LUFS, 9 LU under the mix) |
| Duck under a line (side channel, 1.5 s before vs 0.5-1.5 s into the line) | poke.clean.02 -40.9 -> -50.3 (9.4 dB); poke.orange.02 -43.0 -> -52.6 (9.6); board.rim.03 -44.7 -> -50.0 (5.3); carry.long.02 -41.5 -> -49.3 (7.8); goal.against.03 -36.0 -> -42.2 (6.2, goal pause). **Ducks 5 to 10 dB** |
| Commentator lines in 98 s | **9** started; 2 to 20 s apart; **0 overlaps** (log and speech band); barks are text only (`jam.BarkVoice` 0), so no voice-on-bark overlap is possible |
| Lines cut by a bark | **4 of 9**: lift.clean.03 after 0.05 s, lift.clean.01 0.32 s, shot.orange.01 0.26 s, pass.any.01 0.02 s |
| Events with no sound | `save` x1, `carry` x1 (assets missing) |
| Event density | about 2 min of play: 40 `poke`, 35 `lift`, 25 `board`, 5 `shot`, 5 `pass`: poke and lift sounds carry the scrums |
| Skate loop on the human (`MS_skate_loop`, spatialized; `skate.wav` 33 s, commentary off, the component's volume set to 0 in PIE for the middle 11 s) | **audible**: 1-6 kHz -33.7 / -34.5 dBFS with the loop against **-43.9** without (10 dB); the full mix moves 2 dB |

## Stills filed (`devlog/shots/20261112-2136*-B-qa1-*`)

`qa1-menu-main`, `qa1-menu-cuts`, `qa1-r1-bank-nogoal`, `qa1-r1-goal`, `qa1-r1-period-card`, `qa1-r1-strip`, `qa1-hit1-side-strip`, `qa1-hit2-behind-strip`, `qa1-bcast-hit-strip`, `qa1-clay-hit-strip`, `qa1-livecut-strip`, `qa1-b2-strip`, `qa1-ghost-figures`, `qa1-boost-washed`, `qa1-large-overview`, `qa1-wide-overview`, `qa1-reg-overview`.
Captures in the strips after the first match were taken while ghost figures stood on the ice (rank 6); from `qa1-bcast-hit-strip` on, PIE was restarted clean (6 visuals).

## Packages made dirty

None. Two checks (`EditorLoadingAndSavingUtils`), the last after StopPIE and before `ExitEditor`; no save prompt.

## What I could not reach or verify

- Front and left knockdowns, the human being checked (the pro bots never checked the human), the mistimed-poke stumble, the trip: the driver only presses check.
- The goalie stance, crests, the shot wind-up, the effects of board, poke, lift, shot, pass, foul, pickups and blade spray frame by frame: events fired (no missing line) but I did not freeze frames on them.
- `post`, `period_end`, `clock`, `season_take` did not fire in a first-to-3 cut.
- A Director's Call through the in-game card (only `jam.livecut`, which is the same `ApplyCartridgeLive`).
- The snow banks against boards and benches at close range on the large rinks; the debug-camera views were wide.
- A 3 s commentary window was measured per line rather than a single hand-picked one; the music level is an estimate from the side channel, not a solo of the music.
- Frame cost: another editor on the GPU.
