# Lane AU8 report: gameplay effects down 30 %, hits down 20 % (device B), 2026-11-13 16:01

**What was done.** 26 MetaSounds each got one named graph input that scales only their gameplay-effect signal: `FxTrim`, default 0.70 for effects and 0.80 for hits (`MS_board` has two: `FxTrim` 0.70 for the puck case and `HitTrim` 0.80 for the skater boom). Music, commentary, the crowd bed and every crowd layer inside an event sound are untouched. The levels moved by -3.1 dB and -1.9 dB wherever I have a clean before and after; the crowd layers I could isolate did not move. **Two parts of the proof fell short:** the same-session "trims off / trims on" pass was ruined by a starved machine, so most "before" figures come from AU6 and AU7 recordings of the same triggers; and the two matches recorded after the change ran in slow motion (4 to 20 ticks a second against 42 to 55 in AU7), so their loudness is not comparable. The track shares below are therefore a projection from the AU7 matches, not a measurement of new ones. Nobody on this device can hear; the operator can and did.

- Tree `C:/Users/kazda/jam/wt/b-vfx`, editor on port 8000 from 15:25 to 15:57, left through `ExitEditor` (pid 34768 gone). No git write, build, cook or outside service. Only `Game/Plugins/JamAssets/Content/Audio/MS_*.uasset` changed, plus this report.
- `/Game/Maps/Rink` not saved. No dirty package at exit. `jam.ScriptHuman` 0, `jam.ctrl.Drill` 0, `jam.Cut` `hockey.regular`; the editor's background throttle is back on. The editor on 8002 was not touched (it was using 4 to 5 cores and the shared GPU while I recorded).
- Log: no MetaSound error or warning. The only `JAMFX missing` lines are `NS_deke` / `MS_deke`.

## 1. Where each asset stands

"Whole sound" means one `Multiply (Audio by Float)` by the input, placed right before the output.

| Asset | Group | Input, default | Where it acts |
|---|---|---|---|
| `MS_skate_loop`, `MS_shot`, `MS_shot_blocked`, `MS_pass`, `MS_poke`, `MS_lift`, `MS_post`, `MS_faceoff`, `MS_pickup_spawn`, `MS_pickup_take`, `MS_zone_enter`, `MS_zone_exit`, `MS_effect_start`, `MS_effect_end`, `MS_breakaway`, `MS_clock`, `MS_foul`, `MS_lose`, `MS_season_take` | effects | `FxTrim` 0.70 | whole sound (none of them has a crowd layer) |
| `MS_check`, `MS_bump` | hits | `FxTrim` 0.80 | whole sound |
| `MS_board` | both | `FxTrim` 0.70, `HitTrim` 0.80 | `FxTrim` on the puck graph (`Multiply_3e6f`), `HitTrim` on boom and rattle (`Multiply_3e66`) |
| `MS_knockdown` | effects | `FxTrim` 0.70 | landing, slide, get-up (`Multiply_135c` after `AudioMixerMono3_f522`); "ooh" and roar not touched |
| `MS_save` | effects | `FxTrim` 0.70 | thump and clack (`Multiply_a95a`); the save "ooh" not touched |
| `MS_win` | effects | `FxTrim` 0.70 | arpeggio and thump (`Multiply_933c`); the celebration not touched |
| `MS_goal` | effects | `FxTrim` 0.70 | the burst (`Multiply_cfb7`); cheers and groan not touched |
| `MS_period_end` | crowd | none | applause only, no synth layer: unchanged |
| `MS_music_match`, `MS_commentary`, `MS_crowd_bed` | dominant | none | unchanged |
| `MS_bark`, `MS_announce`, `MS_signal`, `MS_directors_call` | voice / interface | none | unchanged |
| `MS_arena_room` | ambience bed | none | unchanged. It is a synthesised room tone (hum and lamp buzz, `SC_Ambience`, no waves), so I treated it like the crowd bed |

- Assets not on your lists that I placed: `MS_lose` and `MS_season_take` are mono synth with no crowd, so effects x 0.70; `MS_period_end` has nothing but crowd.
- No deviation from your reading. No sound class, mix, music or commentary asset was touched.

## 2. Per asset, before and after

"After" is one pass of every asset triggered alone with the trims on (clean: 37 of 37 delimiters found). "Before" is the same trigger from the lane named; where none exists the cell says so. LUFS / peak dBFS.

| Asset and case | Factor | Before | After | Change |
|---|---|---|---|---|
| `MS_skate_loop`, Speed 1 | 0.70 | -29.0 / -15.8 (this session, trim off) | -32.3 / -18.9 | energy -3.0 dB, peak -3.1 |
| `MS_shot`, S 1 | 0.70 | -26.9 / -9.7 (AU7) | -30.0 / -12.7 | -3.1 / -3.0 |
| `MS_poke`, clean | 0.70 | -26.0 / -10.6 (AU7) | -29.1 / -13.6 | -3.1 / -3.0 |
| `MS_lift`, S 1 | 0.70 | -28.1 / -17.2 (AU7) | -31.3 / -20.4 | -3.2 / -3.2 |
| `MS_board`, puck, S 1 | 0.70 | -25.7 / -11.0 (AU7) | -28.8 / -14.2 | -3.1 / -3.2 |
| `MS_board`, skater, S 1 | 0.80 | -29.0 / -12.1; boom gain -6.0 dB (AU7) | -31.0 / -14.0; boom gain -7.9 dB | -2.0 / -1.9; gain -1.9 |
| `MS_check`, S 1 | 0.80 | -22.3 / -9.4; `HitBig_a` gain -4.1 dB (AU7) | -24.3 / -11.3; gain -6.0 dB | -2.0 / -1.9; gain -1.9 |
| `MS_check`, S 0.6 | 0.80 | `Hit_b` gain -5.4 dB, trim taken out (AU7) | -7.4 dB | -2.0 |
| `MS_bump`, S 1 | 0.80 | -31.2 / -14.7 (AU7) | -33.2 / -16.6 | -2.0 / -1.9 |
| `MS_knockdown`: get-up | 0.70 | gain -0.1 dB (AU7) | -3.2 dB | -3.1 |
| `MS_knockdown`: slide, S 0.6 | 0.70 | gain -7.5 dB (AU7) | -10.6 dB | -3.1 |
| `MS_knockdown`: landing (`Land_c`), S 0.6 | 0.70 | gain -9.4 dB, trim taken out (AU7) | -12.6 dB | -3.2 |
| `MS_knockdown`: **roar**, S 1 | crowd | gain -7.4 dB (AU7) | -7.4 dB | **0.0** |
| `MS_knockdown`: **"ooh"**, S 0.6 | crowd | gain -11.8 to -11.9 dB (AU7) | -11.8 dB | **0.0 to +0.1** |
| `MS_save`: thump alone (ooh swapped for a click) | 0.70 | -22.7 / -7.3 (AU7) | -25.8 / -10.4 | -3.1 / -3.1 |
| `MS_foul`, single whistle | 0.70 | -17.3 / -10.2 (AU7) | -20.4 / -13.2 | -3.1 / -3.0 |
| `MS_win`: synth alone (cheer swapped for a click) | 0.70 | -15.3 LUFS (AU7) | -18.3 LUFS | -3.0 |
| `MS_goal`: burst alone (groan swapped for a click) | 0.70 | -21.1 / -4.2 (AU7) | -23.2 / -4.3 | -2.1 LUFS; the peak is the click on the **crowd path: -0.1** |
| `MS_lose` | 0.70 | -17.6 / -8.3 (AU3) | -20.8 / -11.4 | -3.2 / -3.1 |
| `MS_shot_blocked`, S 0 | 0.70 | about -25.0 by AUD's law, not recorded at S 0 | -28.1 / -12.3 | about -3.1 |
| `MS_post`, S 1 | 0.70 | -28.4 / -11.7 (AUD) | -31.0 / -14.3 | -2.6 / -2.6 |
| `MS_pass`, S 0.5 | 0.70 | -30.9 / -14.8 at S 0.47 (AU7) | -33.7 / -17.8 | -2.8 / -3.0 |
| `MS_period_end` | none | -20.2 / -10.3 (AU3) | -20.0 / -10.3 | +0.2 / 0.0 |
| `MS_faceoff` drop | 0.70 | peak -13.8 (AU7) | peak about -16 (read from the envelope; its segment was misplaced) | about -2 to -3 |
| `MS_faceoff` won / lost, S 0 | 0.70 | none | -29.8 / -20.4, -27.6 / -17.3 | not measured |
| `MS_poke` miss, `MS_foul` double | 0.70 | none | -41.0 / -26.4, -21.8 / -13.6 | not measured |
| `MS_pickup_spawn`, `MS_pickup_take` | 0.70 | none at this trigger | -20.6 / -11.9, -25.0 / -14.1 | not measured |
| `MS_zone_enter`, `MS_zone_exit` | 0.70 | none | -27.2 / -12.5, -27.3 / -12.0 | not measured |
| `MS_effect_start`, `MS_effect_end` | 0.70 | none | -21.2 / -11.4, -24.8 / -16.2 | not measured |
| `MS_breakaway`, `MS_clock` (first 3 s), `MS_season_take` | 0.70 | none | -33.0 / -15.4, -32.7 / -21.4, -24.3 / -17.0 | not measured |
| Whole `MS_knockdown` S 1 / 0.6, `MS_save`, `MS_win`, `MS_goal` (cheer) | mixed | | -24.7 / -10.3, -30.1 / -14.9, -28.8 / -10.2, -18.9 / -5.7, -16.9 / -4.2 | for reference |

- **Measured and inside your limits:** 15 effect or hit cases at -3.0 to -3.2 dB or -1.9 to -2.0 dB; three crowd checks at 0.0 to 0.1 dB (roar, "ooh", and the click riding the goal's crowd path) plus the untouched `MS_period_end`.
- **Outside 0.3 dB:** `MS_post` (-2.6) and `MS_pass` (-2.8), both against a before taken under other conditions (AUD's rig; a slightly different Strength).
- **Not measured, 13 cases in 11 assets** (the rows marked so): they have no earlier recording of the same trigger. They carry the same single multiply as the measured ones (section 1), which is all I can say.
- **Not isolated this lane:** the save "ooh", the win celebration and the goal cheer as layers (the matched filter did not lock on them in this pass). By the graphs the trim sits only on their mixers' input 0, and the click test on the goal shows that path unmoved.
- **Why the pair pass failed:** my plan was each asset twice in one session, `FxTrim` set to 1.0 at play and then left at its default. The trims-off pass came out with 27 of 37 delimiters lost to stalls; I had no time to repeat it.

## 3. The two matches and the dominance

### What I recorded after the change (not comparable)

| Recording | Editor ticks per second | Events in it | Integrated | True peak |
|---|---|---|---|---|
| AU7 plain, 90 s (before) | 42 | 43 lifts, 39 pokes, 27 shots, 4 checks | -24.0 LUFS | -5.9 dBTP |
| AU8 plain, 90 s (`match_plain_90s.wav`) | 20 | 13 lifts, 4 pokes, 14 shots, 2 checks | -26.6 LUFS | -8.5 dBTP |
| AU8 plain, second try | 4 | 5 lifts, nothing else | -26.9 LUFS | -10.8 dBTP |
| AU7 drill, 60 s (before) | 55 | 12 checks, 1 goal | -21.6 LUFS | -3.0 dBTP |
| AU8 drill, 60 s (`match_drill_60s.wav`) | 13 | 2 checks, 1 goal | -25.4 LUFS | -6.4 dBTP |

- The sim steps once per editor tick, so at 4 to 20 ticks a second the match plays in slow motion and far fewer events land in the recording. **These numbers say the mix is quieter, but mostly because less happened.** Nothing clips.
- The one full-strength tackle with room around it (slow drill): the hit moment is 10.9 dB over the 0.9 s before it.

### Projection from the AU7 matches (same events, each sound at its new level)

Energy shares by AU7's method: one render of each sound x the logged events, crowd and commentary from their files x gains, music and bed from their stems. Percent of the modelled total (the model sums to 117 % and 115 % of the measured energy before, 98 % and 96 % after, so read the shares to a few percent).

| Track | Plain, before | Plain, after | Drill, before | Drill, after |
|---|---|---|---|---|
| Music | 40 % | 48 % | 22 % | 27 % |
| Commentary | 14 % | 16 % | 11 % | 13 % |
| Crowd bed | 11 % | 13 % | 6 % | 8 % |
| Crowd reactions | 2 % | 2 % | 21 % | 26 % |
| **Music + commentary + crowd** | **66 %** | **79 %** | **61 %** | **74 %** |
| Gameplay effects | 29 % | 17 % | 20 % | 12 % |
| Hits | 5 % | 3 % | 18 % | 14 % |
| **Effects + hits** | **33 %** | **20 %** | **38 %** | **26 %** |
| Interface blips (bark) | 1 % | 1 % | 1 % | 1 % |
| Dominant tracks over effects + hits | 3.0 dB | 5.9 dB | 2.1 dB | 4.6 dB |

- The skate loop is not in the model (its level follows the skater's speed).
- **Projected loudness:** the modelled energy falls 0.8 dB in both matches, so about -24.8 LUFS plain and -22.4 LUFS drill. The plain match would then be under the -23 window by about 0.8 LU; the operator's instruction outranks that target, but you should know.
- **The hit moment, projected:** the hit itself is 1.9 dB lower and music, bed and crowd are where they were, so it drops by at most 1.9 dB over bed and music: drill median about 7.2 to 5.3 dB, plain about 3.2 to 1.3 dB or better (where the "before" was other effects, they fell 3.1 dB too). A full-strength tackle alone is now -24.3 LUFS / -11.3 dBFS with its roar unchanged at -7.4 dB; bed and music are about -26 LUFS. By the numbers it is still an event; at Strength 0.6 (-29.3 LUFS) it is 3 LU under bed and music and I would not call it one from numbers alone.

## 4. How he changes it again in one step

- **One sound:** open the MetaSound, Members panel, input `FxTrim` (or `HitTrim` in `MS_board`), change "Default Value", save. 1.0 is the AU7 level, 0.70 is now, 0.5 is half.
- **All of them at once, from B:** `node trim.js` in the scratch folder adds the inputs; a five-line variant with `SetDefaults` on the graph input `FxTrim` rewrites all 26 defaults in about a minute with the editor open.
- **All of them at once, at run time (an ask for A, not needed today):** the binder already calls `SetFloatParameter` for `Strength` and `Variant`; one more line, `SetFloatParameter("FxTrim", x)`, would put the whole effects level on one console variable or slider.

## 5. Files (bytes, `Game/Plugins/JamAssets/Content/Audio/`)

26 MetaSounds changed, each by about 4.7 kB (one input, one node; `MS_board` two): 5,277,612 bytes before, 5,406,021 after, +128,409. Every replacement row below carries its file's bytes.

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_board.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph inputs FxTrim 0.70 (puck case) and HitTrim 0.80 (skater boom and rattle) (operator's 15:22 instruction, lane AU8) (256833 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_breakaway.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (145900 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_bump.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.80 (operator's 15:22 instruction, lane AU8) (113649 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.80 on the hits (operator's 15:22 instruction, lane AU8) (269153 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_clock.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (163742 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_effect_end.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (186431 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_effect_start.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (230632 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_faceoff.uasset | audio | 0.4 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (370208 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_foul.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (338526 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_goal.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the burst only; crowd cheers and groan untouched (operator's 15:22 instruction, lane AU8) (306068 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on landing, slide and get-up only; crowd ooh and roar untouched (operator's 15:22 instruction, lane AU8) (261713 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lift.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (193679 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_lose.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (148921 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pass.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (172610 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_spawn.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (130830 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_pickup_take.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (208413 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_poke.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (242352 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_post.uasset | audio | 0.1 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (107509 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_save.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on thump and clack only; crowd ooh untouched (operator's 15:22 instruction, lane AU8) (227206 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_season_take.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (188992 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (181224 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_shot_blocked.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (177490 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_skate_loop.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (174626 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_win.uasset | audio | 0.3 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the arpeggio and thump only; crowd celebration untouched (operator's 15:22 instruction, lane AU8) (300469 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_zone_enter.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (154425 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_zone_exit.uasset | audio | 0.2 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-13 | B | y | MetaSound source, content as in its previous row; graph input FxTrim 0.70 on the whole sound (operator's 15:22 instruction, lane AU8) (154420 bytes) |

## 6. Recordings

In `C:/Users/kazda/jam/audio-lab/tackle/engine/` (AU7's set moved to `engine/au7/`) and in the scratch folder `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au8/`:

- `match_plain_90s.wav`, `match_drill_60s.wav`: after the change, **slow-motion matches** (section 3).
- `before_after.wav`: 15 s of the AU7 plain match, 1 s of silence, 15 s of the AU8 plain match. **Not the same passage**: the slow match never reached it. The second half holds one tackle (at about 8 s into it).
- **Not delivered: `chain_s06.wav`, `chain_s10.wav`.** I did not record the joint check-and-knockdown trigger in this lane. AU7's are in `engine/au7/`; today's hit is 1.9 dB under them and the landing, slide and get-up 3.1 dB.
- Scratch only: `a1.wav` (every asset alone, trims on, a clap before each), `b1.wav` (the failed trims-off pass), `plainA.wav`, `drill8.wav`, `backup/` (all 35 MetaSounds as they were at 15:25).

## 7. What I could not verify

- **I cannot hear; the operator can and did.** Whether 30 % and 20 % are what he meant by ear is his to say.
- **A real-speed match after the change.** Both attempts ran slow; loudness, true peak and the hit moment at full speed are projections.
- **13 cases in 11 assets** have no before figure (section 2).
- **The save "ooh", win celebration and goal cheer as isolated layers** (section 2).
- **`MS_bump`** still has no event; it was triggered directly.
- The share model overshoots the measured energy by about 16 % before the change, so its split is good to a few percent, not better.
- Cooked or packaged behaviour. No cook was run.

## 8. For the lead

1. **Re-record the two matches when the machine is free** (nothing else rendering): two minutes of editor time, scripts `rec_match.py` and `mm.py` in the scratch folder. That closes the open half of proof 2.
2. **The plain match will sit near -24.8 LUFS**, under the -23 window. If that matters after his change, the lever is the music `Level` (device A) or the dominant tracks, not the effects.
3. **A tackle at Strength 0.6 is now under bed and music by the numbers** (-29.3 LUFS against about -26). His instruction asked for exactly this direction; say if the hits should come back up (`FxTrim` in `MS_check`, one number).
