# Commentary: generation plan (lane N2)

Voice settings: as AU1 (`eleven_multilingual_v2`, stability 0.35, style 0.4 for the commentator; the director as AU1 used for Bill / Liam). Tool:
`C:/Users/kazda/jam/eleven-tools/eleven.mjs tts` with `--ledger`, one request per line, text exactly as in `lines.json`, output `mp3_44100_128`, then convert to
`VO_<Speaker>_<id with dots as underscores>.wav` (mono, 48 kHz, 16-bit, trimmed at -55 dB) as in README section 5. Check the quota first (`eleven.mjs quota`).

Skip the 21 lines AU1 already recorded with the same text in the picked voices (README section 6): rename those files instead of paying for them again.

## Batch 1: "first 40" (a whole match, commentator, 841 characters)

Covers a match from intro to end card: faceoff, shots, goals for and against, check, poke, foul, Director's Call, take end, win, loss.

```
period.take_1.01  period.take_2.01  faceoff.drop.01  faceoff.drop.02  faceoff.blue.01  faceoff.orange.01  faceoff.after_goal.01
shot.blue.01  shot.blue.02  shot.orange.01  shot.orange.02  shot.hard.01  save.any.01  post.any.01
goal.blue.01  goal.blue.02  goal.orange.01  goal.orange.02  goal.for.01  goal.against.01  goal.tie.01  goal.match_point.01
check.blue.01  check.orange.01  check.big.01  knockdown.any.01  poke.clean.01  poke.miss.01  lift.clean.01
foul.behind.01  foul.no_puck.01  directors_call.any.01  directors_call.any.02  clock.last_10.01  period_end.any.01
win.any.01  win.any.02  lose.any.01  lose.any.02  breakaway.blue.01
```

## Batch 1D: the director's twelve (one per situation, 140 characters)

```
dir.action.01  dir.cut_goal.01  dir.from_the_top.01  dir.take_2.01  dir.take_3.01  dir.take_end.01
dir.call_open.01  dir.call_apply.01  dir.foul.01  dir.win.01  dir.lose.01  dir.menu.01
```
Most of these exist from AU1 if Bill or Liam is picked; generate only the missing ones (`dir.take_end.01`, `dir.foul.01`, `dir.call_apply.01` if "Rolling." is not wanted there).

**Stop here and play a match with these 52 lines** before spending the rest: the triage below decides whether the voice and settings hold.

## Batch 2: every situation covered once (the remaining 106 `.01` lines, 2,847 characters)

All ids ending `.01` not in batch 1: the other goal states, saves, posts, boards, carries, passes, pickups and effects, zones, `announce`, every
`directors_call.<key>_up` / `_down`, the take and clock states, win and loss variants, the eight `season_take.level_<n>` intros. After this batch no situation is silent.

## Batch 3: second alternates (`.02`, 2,887 characters), then batch 4: third and fourth (`.03`, `.04`, 1,675), then the director's remaining 13 alternates (254)

Order inside each batch: priority 5 first, then 4, down to 1, so a stopped run leaves the important lines done.

## What to listen for (triage, per line, mark keep / regenerate / rewrite)

1. **Clipped starts:** the first consonant swallowed ("lue scores"), common on very short lines and lines that start with a plosive (B, P, T, K). Regenerate with a different seed; if it repeats, the trim threshold is too high.
2. **Stress on the team names:** "BLUE scores", "ORANGE wins the draw": the colour must carry the stress and sound the same in every line (one pitch shape for "blue", one for "orange"). Watch "orange" turning into two flat syllables, and "blue" rising like a question.
3. **Long lines:** any quick line (event other than `period`, `period_end`, `season_take`, `win`, `lose`, `goal.pause`) over 2.0 s after trimming is too long for live play: rewrite shorter in `gen-lines.mjs`, do not speed it up.
4. **Wrong energy:** goals and the last ten seconds must lift; `goal.against`, `lose.*` and fouls must stay level, not gleeful. Fillers (`pass`, `board`, `zone_*`) should be noticeably quieter in delivery than goal calls.
5. **Questions that are not questions:** lines ending in "!" read as questions by some voices; "Retake?" is the only intended question.
6. **Artefacts:** breaths at the end, a trailing "uh", room tone, sibilance on "since", "seconds", "spotless"; a stray accent drift between lines of one voice.
7. **Sound-alike:** a designed voice can land near a real broadcaster; if a line sounds like one, drop that voice.
8. **Loudness:** after the batch, every line within about 3 LU of the voice's median (AU1 measured -13 to -27 LUFS across voices); normalise in the engine, but re-generate outliers more than 6 LU away.
