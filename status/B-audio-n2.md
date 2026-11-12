# Lane N2 report: the commentary script (device B), 2026-11-12 about 15:00

Text only: no Unreal, no git, no audio, no network. Everything is in `briefs/commentary/`: `lines.json` (the bank), `README.md` (schema, register, picking rules, file
names, counts, event coverage), `generate-plan.md` (batches and triage), `gen-lines.mjs` (the tables the bank is generated from), `check.mjs` (validator; passes: JSON parses,
ids unique, every event known or listed as assumed, no line over 90 characters, no digits, no number above ten, no gendered pronoun, colour lines say their colour, no banned phrase).

## Counts

| Speaker | Situations | Lines | Characters |
|---|---|---|---|
| Commentator | 138 (69 x 3 alternates, 2 x 4, 58 x 2, 9 x 1) | 340 | 8,250 (mean 24.3 characters, 4.7 words; longest 64) |
| Director | 12 | 25 | 394 |
| Total | 150 | 365 | 8,644 |

Credits at about 1 per character for one voice per speaker: **about 8,650, under the 12,000 cap.** At AU1's measured rate on this account (about 0.43 per character) it is about
3,700. First playable set (batch 1 + 1D, 52 lines): 981 characters. 21 lines reuse AU1's recordings word for word (9 commentator, 12 director positions), so they cost nothing in
the picked voices; AU1's "He shoots... he scores!" is deliberately left out (a real broadcaster's call).

Events covered: `faceoff` (drop / won / lost), `shot`, `goal` (colour call plus 12 score and mood states), `check`, `knockdown`, `poke`, `lift`, `foul` (both kinds), `pass`, `board`,
`pickup_spawn` / `pickup_take` (all five built-in kinds), `effect_start` / `effect_end`, `zone_enter` (five zone kinds) / `zone_exit`, `announce`, `directors_call` (all nine cartridge
tunables, up and down, plus a fallback), `period`, `win`, `lose`. No line on purpose: `bark`, `signal` (README section 7).

## Assumptions

- **Nine triggers that no contract defines:** `save`, `post`, `breakaway`, `carry`, `period_end`, `clock` (Kind `last_10s` / `sudden_death`), `season_take` (Kind `level_1`..`level_8`),
  `directors_call_open`, `menu` (the last two are UI hooks, not bus events). Their meaning is in README section 7.
- Team 0 is blue and team 1 is orange; "for/against" is relative to `UJamSim::GetHumanTeam()`.
- `faceoff` Kind `won` / `lost` is the outcome for Who's team; `effect_end` Kinds equal the pickup kinds; `zone_enter` Kind is the zone id and the picker maps it to the zone kind.
- `directors_call` Strength is the new value's position in its range, so "up" / "down" means upper / lower half, not the direction of the change.
- The pickup tunable in Director's Call (the widget takes any key containing "pickup") has no key-specific line: the fallback covers it.
- R-0015 is not in this checkout's `contracts/RULINGS.md` (it exists on `origin/main` as commit b741708, "barks regenerated at a harsher register within the Teen bar"; not read,
  no git in this lane). The register keeps the commentator clean regardless, so a harsher bark register only widens the gap.

## Handoff for device A (bark and binder code)

A commentary player is a sibling of `UJamBarkSystem`: subscribe to `OnJamEvent`, load `briefs/commentary/lines.json` (or a copy under `Game/Content/Commentary/`), and on each event
build candidates by `event`, `kind` (omitted = any) and `team` (colour of Who's team; the winning team for `win`/`lose`), evaluate each situation's condition from the README
section 4 table (Strength thresholds, `Where` against the rink, score before and after, time left, the human's team, and a short event history it keeps itself), drop situations
inside `cooldown_s` and the speaker's last line, let a director candidate silence the commentator, take the highest priority (most specific on a tie, then random), and play it only
if 2.5 s have passed since the last voice line (drop, never queue), from the sound `VO_<Speaker>_<id with dots as underscores>` on a 2D voice channel ducking music by a few dB;
the subtitle can reuse the bark HUD strip. It must also give the barks right of way or a shared gate (a bark bubble and a commentary line together are fine; two voices at once are
not, if barks are voiced with `jam.BarkVoice 1`). For the bank to be complete, gameplay must broadcast the nine assumed triggers (or the lane tells N2 which to drop): `save` and
`post` from the sim's collision code, `breakaway` and `carry` from the carrier tracking, `period_end` and `clock` from the rules clock, `season_take` from the progression subsystem,
`directors_call_open` and `menu` from `DirectorsCallWidget` and `MenuFlow`; and `poke`, `lift`, `foul`, `pass`, `announce`, `effect_*`, `zone_*` must exist in `JamEventBus` (they are
ruled, but this checkout's header has only the first fourteen names).

## What I could not determine from the contracts

1. Whose perspective `faceoff` Kind `won` / `lost` takes, and whether `foul` also carries the fouled team (needed for the "free faceoff" line).
2. How many takes a match has in `first_to` mode, and whether a take has a clock at all there (the last-ten-seconds lines only make sense with one).
3. Whether a goal that ends the match fires `goal` before `win` with enough time for both (the bank assumes yes; the gate then drops one of them unless the end card waits).
4. The exact key of the pickup-rate tunable and the effect ids a Designer-made cut may invent (lines only exist for the built-in kinds).
5. Whether the director's short lines should share the 2.5 s gate with the commentator (README section 3 point 7: the operator should decide; the bank works either way).
6. R-0015's text (see assumptions).
