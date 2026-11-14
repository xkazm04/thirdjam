# Lane AU12 report: the stumble on its own signal, the hit's size on the new Strength (device B), 2026-11-14 01:36

- Main tree `C:/Users/kazda/jam/thirdjam`, `dev-b` at aca03468. Editor on port 8002 from 01:24 to 01:33, left through `ExitEditor` (pid 18368 gone, log closed cleanly). No commit, push, build, cook or service call. Nothing under device A's paths written.
- **Nobody on this device has heard any of this.** Every statement is a measurement of recordings of the editor's output, or a line of the log.
- Changed: `MS_check` and `MS_knockdown` only. `/Game/Maps/Rink` not saved, no dirty package at exit, `jam.Cut`, `jam.ScriptHuman`, `jam.ctrl.Drill` as found; "allow background audio" and "use less CPU in background" put back.
- Log: no MetaSound or audio error, no `JAMFX missing` and no `JAMVO missing` line.

## Verdicts

| Job | Verdict | Evidence |
|---|---|---|
| 1. The stumble on `Variant` 4 | **PASS** | `MS_check` plays the skate scrabble when `Variant` is 4 and a hit otherwise; the Strength test is gone. Through the binder: four stumbles gave the scrabble and no hit take, four knockdown checks gave a hit. The scrabble rises 4 LU from Strength 0 to 0.9 and stays 2 to 6 LU under the weakest hit. |
| 2. The hit's size on the new Strength | **PASS, on a small sample** | 10 knockdowns and 6 stumbles in two 100 s clay matches. Big-hit take and roar moved from 0.8 to **0.75**: 3 of 10 knockdowns (was 2 of 10), 0 of 6 stumbles. The "ooh" stays at 0.5. |
| 3. The abilities with the real kind | **not in play** | No `ability` and no `deke` event in 366 s (three clay matches and one 62 s run of the arsenal mode). Through the binder the four kinds now give four different sounds; a grant is silent. |
| 4. The mix line | **PASS** | Clay, seed 1, `pro`, 100 s at **54.4 ticks a second**: -24.4 LUFS, -6.2 dBTP, nothing at full scale; music + commentary + crowd about 81 to 84 %. |

## 1. The stumble

- **What the code says.** The binder sends `Variant` 4 for a check whose `Outcome` is `stumble` (`VariantForPayload`). A check's Strength is `clamp(closing speed / skater.speed, 0, 1)`, 1 for a hip check, so **a Strength of 0 can still arrive**, on a stumble or on a knockdown, when the two skaters are not closing along the contact line. One did in these matches (a stumble at 0.000).
- **So the old test is dropped, not kept.** A Strength-0 check that knocks down is a hit and plays a hit at the 0.5 floor; a stumble is told by `Variant` alone.
- **In the graph:** new float input `Variant` (default 0). Stumble = 1 when `Variant` is over 3.5. The hits are multiplied by (1 - stumble), the scrabble by stumble x (0.6 + 0.4 x Strength), at the same 0.72 on `AudioMixerMono2_7ede`. `FxTrim` 0.80 as before.

| Through the binder | LUFS | Peak dBFS | What played |
|---|---|---|---|
| check, stumble, Strength 0.0 | -37.0 | -27.1 | scrabble |
| check, stumble, Strength 0.3 | -35.4 | -25.5 | scrabble |
| check, stumble, Strength 0.9 | -32.9 | -23.0 | scrabble (no hit, no big hit) |
| check, down, Strength 0.0 | -30.9 | -13.3 | `Hit_b` at the floor |
| check, down, Strength 0.6 | -29.4 | -9.4 | `Hit_c` |

- A fourth stumble (Strength 0.61) read -33.0 LUFS with a longer tail; a match sound had leaked into its window.
- Before this lane a stumble with a real Strength (5 of the 6 seen) would have played a body hit.

## 2. The thresholds on the new Strength

Two clay matches at `pro`, scripted human, 100 s each, seeds 1 and 7920 (55.7 and 54.2 ticks a second). Landed checks from the log, by `Outcome`:

| Outcome | n | Strengths |
|---|---|---|
| `down` | 10 | 0.356, 0.372, 0.468, 0.554, 0.663, 0.666, 0.675, 0.773, 0.807, 1.000 |
| `stumble` | 6 | 0.000, 0.142, 0.192, 0.306, 0.485, 0.610 |

| Rule | Old | New | Knockdowns it takes (of 10) | Stumbles (of 6) |
|---|---|---|---|---|
| Big-hit take in `MS_check` (`Subtract_cfdd`) | 0.8 | **0.75** | 2, now 3 | 0 (and none can: a stumble never reaches the hit path) |
| Roar in `MS_knockdown` (`Subtract_e569`) | 0.8 | **0.75** | 2, now 3 | 0 (a stumble sends no `knockdown`) |
| "Ooh" in `MS_knockdown` (`Subtract_29bc`) | 0.5 | 0.5 | 5, now 4 (0.5 to under 0.75) | 0 |
| No crowd | under 0.5 | under 0.5 | 3 | all |
| Level | max(0.5, Strength) | unchanged | | |

- **Why 0.75.** A quarter of ten is two and a half; the strengths have their widest gap between 0.675 and 0.773, and 0.75 sits in it. At 0.8 the roar had fallen to a fifth of knockdowns.
- The strongest stumble seen is 0.61, under both thresholds; but the guarantee is the wiring, not the number.
- Checked at the edge through the binder: a check at 0.74 plays `Hit_b`, at 0.75 `HitBig_a`; a knockdown at 0.49 has no crowd, at 0.74 the "ooh" (`CRD_Hit_c`), at 0.75 the roar.
- In the third match (seed 1 again, after the change): knockdowns at 0.554, 0.663, 0.666 and 0.807, so three "ooh" and one roar.

## 3. The abilities

- **None fired in play:** no `ability` and no `deke` event in the three clay matches (300 s) or in one 62 s run of `hockey.mode.arsenal` (the one drill I staged).
- Through the binder, with the kind in the payload: shotgun -27.1 LUFS, hip check -24.6, shield -35.0, slow puck -31.9, each the level of its own take in AU11's audition. So the kind now reaches `MS_ability`. A grant (Strength 0) is silent.

## 4. The mix line

Clay, seed 1, `pro`, scripted human, recorded after the changes of this lane.

| | AU12, 100 s | AU11, 100 s |
|---|---|---|
| Ticks a second | **54.4** | 38.5 |
| Integrated | -24.4 LUFS | -23.6 LUFS |
| True peak | -6.2 dBTP | -5.7 dBTP |
| Samples at full scale | 0 | 0 |
| Music + commentary + crowd | **about 81 to 84 %** | about 84 % |
| Gameplay effects + hits | about 15 to 18 % | about 15 % |
| Hit moment over the 0.9 s before: min / median / max | -3.8 / 4.0 / 5.8 dB (8 checks, 4 of them stumbles) | 2.2 / 5.3 / 7.7 dB (4 hits) |

- Shares, percent of the modelled total: music 41, commentary 18, crowd bed 11, crowd reactions 14, gameplay effects 9, hits 6, bark 1.
- **The range comes from the hard stop.** My count used the `SK21 stop start` print (5 in the 100 s). The Blueprint now prints 61 gated stops (`SK23 brake level pose` 54, `SK23 brake short` 7), which fits the 60 you named. With 61 stops at AU11's level the stop is about 5 % of the match and the dominant tracks about 81 %; with 5 they are 84 %. I did not open the Blueprint to see which print carries the sound.
- The model sums to 132 % of the measured energy, so the shares are good to several percent.
- The hit-moment figure now includes stumbles, which are quiet on purpose; the lowest value (-3.8 dB) is one of them.
- Events in the 100 s: 47 skater board hits, 38 passes, 17 puck board hits, 12 pokes, 11 shots, 9 clean lifts, 8 checks (4 down, 4 stumble), 1 goal, 20 commentary lines.

## 5. Not verified

- **Anything by ear**: that the scrabble reads as a stumble, and under a hit.
- **The thresholds on more than 10 knockdowns.** One more or one fewer hard hit moves "3 of 10" by ten points.
- **A hip check's check** (Strength 1 by rule): none occurred. It will take the big-hit take and the roar.
- **Abilities and the deke in a match.**
- **Which Blueprint print carries `MS_brake`** (section 4).
- The share model's split between stumbles and hits: my recorder does not carry `Outcome`, so three stumbles were counted as weak hits (about 1 % of the energy).
- Cooked or packaged behaviour. No cook was run.

## 6. For the other device

- **Nothing is needed.** `Variant` 4 and the ability kinds arrive as written; `MS_check` now has a `Variant` input and ignores 0 to 3.
- For your notes: a stumble's check can still carry Strength 0.000 (seen once in 6). The sound no longer depends on it.
- If a headless run can print landed checks by `Outcome` and Strength over 1,000 s or more, that list is the sample the thresholds want (next step 1).

## 7. Decisions for the operator

1. **The roar and the big hit come at 0.75**, on about three knockdowns in ten. Rarer (0.8) or more often is one number in each of two sounds.
2. **Knockdowns under 0.5 (3 of 10) get the hit at its floor and no crowd.** If every knockdown should draw at least the "ooh", lower `Subtract_29bc` in `MS_knockdown`.
3. **The stumble is quiet** (-37 to -33 LUFS, under music and bed). It is one gain (`AudioMixerMono2_7ede` Gain 1, 0.72) if it should be heard more.

## 8. The next three steps, each with its proving number

1. **Thresholds from a real sample.** 10 knockdowns today. From 100 or more: the roar on 20 to 30 % of knockdowns, and 0 stumbles above the threshold.
2. **Abilities in a match.** 0 of 4 kinds heard in 366 s. A match or mode that uses each: 4 of 4 found in the recording at their gains.
3. **The stumble's second take.** `SFX_Stumble_b` is imported and unused. With both in rotation: no two stumbles in a row with the same take, counted over the 6 or more stumbles of a 200 s log.

## 9. Files and ledger rows

| File | Before | After |
|---|---:|---:|
| `Audio/MS_check.uasset` | 290,501 | 304,392 |
| `Audio/MS_knockdown.uasset` | 261,713 | 261,717 |

Both rows **replace** the existing ones:

| path | kind | size_mb | origin | tool | licence | url | date | device | shipped | note |
|---|---|---|---|---|---|---|---|---|---|---|
| Game/Plugins/JamAssets/Content/Audio/MS_check.uasset | audio | 0.30 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, mono; body check: one of three hits at random (Hits, HitTrims), the big hit (HitBig) at Strength 0.75 and over, level x max(0.5, Strength); a stumble (float input Variant 4 from the binder) plays SFX_Stumble_a (input Stumble) at 0.72 x (0.6 + 0.4 x Strength) in place of a hit; FxTrim 0.80 (lanes AU6, AU8, AU11, AU12) (304392 bytes) |
| Game/Plugins/JamAssets/Content/Audio/MS_knockdown.uasset | audio | 0.26 | original | UE 5.9 MetaSounds via MCP/Python |  |  | 2026-11-14 | B | y | MetaSound source, stereo; the fall after a check: landing (Lands, random) at 0.17 s, slide (Slide) from 0.20 s, get-up scrape (GetUp) at 1.05 s, crowd from 0.05 s at 0.6: ooh (CrowdOohs, random) at Strength 0.5 to under 0.75, roar (CrowdRoar) at 0.75 and over; FxTrim 0.70 on landing, slide and get-up only (lanes AU6, AU7, AU8, AU12) (261717 bytes) |

Recordings in `C:/Users/kazda/jam/audio-lab/verbs/engine/`: `au12_match_clay_seed1_100s.wav` (after the changes; the mix line), `au12_match_clay_seed7920_100s.wav` (stumble on `Variant` 4 already, thresholds still at 0.8), `au12_trigger_test.wav` (all sixteen binder triggers in order), and clips cut from it: `au12_stumble_s00.wav`, `au12_stumble_s03.wav`, `au12_stumble_s09.wav`, `au12_hit_s00.wav`, `au12_hit_s074.wav`, `au12_bighit_s075.wav`, `au12_knockdown_s049.wav`, `au12_knockdown_s074_ooh.wav`, `au12_knockdown_s075_roar.wav`, `au12_ability_shotgun.wav`, `au12_ability_hip_check.wav`, `au12_ability_shield.wav`, `au12_ability_slow_puck.wav`. Scratch: `C:/Users/kazda/AppData/Local/Temp/claude/6d8eaecb999746c2/1f5c7b1e-557b-4c70-b752-8370d9dbef0b/scratchpad/au12/` (scripts, the two seeded cartridges, `backup/` with the assets as they were).
