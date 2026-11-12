# HOKEJ commentary: the line bank (lane N2, device B, 2026-11-12)

Files here: `lines.json` (the bank), `gen-lines.mjs` (the tables it is generated from: edit there, then `node gen-lines.mjs lines.json`), `check.mjs` (the validator:
`node briefs/commentary/check.mjs`, exit 1 on any error), `generate-plan.md` (what to synthesise first once a voice is picked).

## 1. Schema

`lines.json` is a JSON array. Each entry:

| Key | Type | Required | Meaning |
|---|---|---|---|
| `id` | string | yes | unique. Commentator: `<event>.<situation>.<nn>` (the first segment is the event, e.g. `goal.blue.01`). Director: `dir.<situation>.<nn>`. All entries that share an id without its `.<nn>` are one **situation** (alternates of one another). |
| `speaker` | `"commentator"` \| `"director"` | yes | |
| `event` | string | yes | an event name from C-0003 (including the C-0005 and R-0013 additions) or one of the assumed triggers in section 6 |
| `kind` | string | no | the payload `Kind` it needs; **omitted = matches any Kind** |
| `when` | string | yes | one plain-language condition; the runtime implements it per situation (section 4) |
| `team` | `"blue"` \| `"orange"` \| `"any"` | yes | the literal colour of the team the event belongs to (team of `Who`; for `win`/`lose`, the winning team). A colour line always says that colour; an `any` line never names one (except the match-intro line, which names both) |
| `priority` | integer 1..5 | yes | 5 = must-play (goals, wins, the Director's Call), 1 = filler (passes, boards, zones) |
| `cooldown_s` | number | yes | seconds after any line of this situation plays before the situation may play again (0 = no cooldown) |
| `text` | string | yes | what is spoken; no digits, numbers spelled out and never above ten |

Unknown keys are errors; an absent value is an omitted key (never `null` or `""`). Same rule as `contracts/README.md`.

## 2. Register (ten lines)

1. The commentator is a professional play-by-play voice in the booth (any of the five voices, so nothing in the text assumes a gender): describes the play, never talks to the players or mocks anyone.
2. Short: three to eight words for live play, so the line ends before the next event; full sentences only in stoppages (take start, goal pause, end of match, season intro).
3. Colour calls first: "Blue scores!" is the line; the mood line ("What a finish!") is the second beat in the goal pause.
4. A quiet home lean toward the human's team (`goal.for` is warm, `goal.against` is fair: "Credit where it is due"), never cheering against anyone.
5. Dry humour, one wink at a time ("Felt that one up here.", "Somebody call for a rewrite."); no pun in every line.
6. The film set comes through in the nouns: take, cut, scene, the set, the director, rehearsals. Use it in stoppages, not on every shot.
7. Teams are only "blue" and "orange"; skaters are unnamed and ungendered ("the carrier", "the last man", the colour).
8. No player names, numbers above ten, real teams, leagues, people, brands, or a real broadcaster's catchphrase (so not the classic "he shoots, he scores").
9. Rated for 13: hits are "big", fouls are "called"; the players' barks (R-0015) carry the trash talk, the commentator never repeats or answers it.
10. The director is a different person on the set floor: one to five words, imperative, film-crew vocabulary only ("Action!", "Cut!", "Take two.", "That's a wrap!").

## 3. How a runtime picks a line

On every bus event (and each assumed trigger in section 6):
1. **Candidates:** entries whose `event` equals the event name, whose `kind` is omitted or equals the payload `Kind`, whose `team` is `any` or the colour of the event's team, and whose `when` holds (section 4).
2. **Filter:** drop a situation still inside its `cooldown_s`; drop the line the same speaker said last (never the same line twice in a row; better, skip any of the last three lines of that situation).
3. **Speaker:** if any director candidate survives, the director speaks and the commentator stays silent for this event (the director outranks the commentator).
4. **Choose:** the highest `priority`; on a tie prefer the more specific situation (a `kind`, a colour, or a score/clock condition beats a plain one), then pick an alternate at random.
5. **Gate:** at most one voice line every 2.5 s across both speakers, measured from the start of the last line. If the gate is closed, **drop** the line; never queue it (a late call describes a play that is over).
6. **Two-beat goals:** on `goal`, the colour call (`goal.blue` / `goal.orange`) plays first; one score-state or mood line (`goal.tie`, `goal.for`, ...) may follow when the gate reopens if the goal pause is still running (the 2-second pause plus the faceoff countdown makes this fit); `goal.pause` is the third choice only if nothing else fits.
7. **Proposed exception (operator to decide):** the director's goal "Cut!" (0.4 to 0.6 s) blocks the commentator's goal call under rule 5. The bank limits the director's goal lines to once a minute (`cooldown_s` 60). The alternative is to exempt director lines shorter than 1 s from the gate and start the gate when they end.

## 4. Conditions the runtime must compute

The `when` text is authoritative; this is the machine reading of the situation suffixes.

| Suffix (situation) | Condition |
|---|---|
| `.blue` / `.orange` | team of `Who` (team 0 = blue, team 1 = orange; assumed) |
| `.for` / `.against`, `knockdown.human`, `breakaway.human`, `pass.to_human`, `win.*`, `lose.*` | relative to `UJamSim::GetHumanTeam()` / the human's skater index |
| `.hard`, `.big`, `.long` (pass), `.soft` | payload `Strength` thresholds given in `when` (0.8 / 0.7 / 0.3) |
| `shot.long`, `goal.long`, `shot.close`, `check.boards`, `board.rim` | `Where` against the rink: own half, inside the crease area, within 2 m of the boards, behind a goal line |
| `goal.first`, `.tie`, `.lead`, `.comeback`, `.blowout`, `.match_point`, `.own`, `win.shutout`, `win.close`, `win.comeback`, `lose.close`, `lose.blowout`, `period.take_tied`, `period_end.tied`/`.lead` | the score before and after the event, the match's lowest deficit, and `rules.mode` (`first_to` for match point) |
| `goal.late`, `post.late`, `clock.last_10_tied`, `clock.last_10_close` | time left in the take (timer mode) |
| `shot.human_streak`, `pass.blue`/`.orange` (three in a row), `board.bank` (goal within 1 s of a board hit), `faceoff.after_goal`, `faceoff.after_foul`, `carry.protect` | a short event history the picker keeps itself |
| `directors_call.<key>_up` / `_down` | `Kind` = the tunable key, `Strength` 0.5 or more = up, under 0.5 = down (the payload carries the new value's position in its range, not the old value) |
| `zone_enter.<zone kind>` | the payload `Kind` is the zone **id**; the picker looks the id up in `game.rink.zones` and matches the zone **kind** |

## 5. Audio file names

`VO_<Speaker>_<id with dots as underscores>.wav`, `<Speaker>` capitalised: `VO_Commentator_goal_blue_01.wav`, `VO_Director_dir_action_01.wav`. Mono, 48 kHz, 16-bit PCM,
silence trimmed at both ends (AU1's -55 dB trim), no reverb baked in. One folder per chosen voice; the voice name is not part of the file name, so a voice can be swapped by
replacing the folder.

## 6. Counts (from `check.mjs`)

| Speaker | Situations | Lines | Characters | Mean characters / words | Longest |
|---|---|---|---|---|---|
| Commentator | 138 (69 with three alternates, 2 with four, 58 with two, 9 with one) | 340 | 8,250 | 24.3 / 4.7 | 64 |
| Director | 12 | 25 | 394 | 15.8 / 2.9 | 29 |
| **Total** | **150** | **365** | **8,644** | | |

ElevenLabs credits at about 1 credit per character, one voice per speaker: **about 8,650** (under the 12,000 cap). AU1 measured about 0.43 credits per character on this
account (14 credits for a 33-character line), which would make it about 3,700. Synthesising the whole bank in two candidate voices doubles it; do not (see `generate-plan.md`).
Nine commentator lines run past eight words: the stoppage lines (match intro, take starts, season intros, goal pause, big wins and losses).

Already recorded by AU1 and reusable as-is in the picked voice (same text): commentator `period.take_1.01`, `faceoff.blue.03`, `check.boards.01`, `board.hard.01`,
`breakaway.orange.03`, `clock.last_10.01`, `win.blue.01`, `lose.any.04`, `directors_call.ice_friction_down.01`; director `dir.action.01`, `dir.cut_goal.01`, `dir.cut_goal.02`,
`dir.from_the_top.01`, `dir.take_2.01`, `dir.take_2.02`, `dir.take_3.01`, `dir.call_open.01`, `dir.call_apply.01`, `dir.win.01`, `dir.lose.01`, `dir.menu.01`. AU1's tenth commentator
sample, "He shoots... he scores!", is **not** in the bank: it is a real broadcaster's famous call.

## 7. Events and triggers

**Assumed triggers** (not in C-0003, C-0005 or R-0013; device A must add them or the lines stay silent):

| Name | When | Who / Kind | Why |
|---|---|---|---|
| `save` | the last man (R-0013 3) blocks a shot | Who = blocker | the brief asks for saves; no event says a shot was stopped |
| `post` | the puck hits a goal post | Who -1 | `board` covers the boards only |
| `breakaway` | a carrier passes the last defender with open ice to the net | Who = carrier | derived state, fire once per rush |
| `carry` | a carrier keeps the puck 30 m or more (Strength = distance / rink length) | Who = carrier | derived state |
| `period_end` | a take ends | Kind `take_<n>` | `period` only marks a take starting; the rule hook `OnPeriodEnd` exists but broadcasts nothing |
| `clock` | ten seconds left in a take; sudden death starts | Kind `last_10s` / `sudden_death` | no clock event exists |
| `season_take` | a season level starts, before its first drop | Kind `level_1` .. `level_8` | R-0014 3: "take" names both; `period` cannot carry the level |
| `directors_call_open` | the human opens the Director's Call screen | | UI only, not a bus event |
| `menu` | the main menu opens | | UI only, not a bus event |

**Assumed Kind values:** `effect_end` Kind `speed` / `reverse_other` assumes the built-in pickup kinds are also the effect ids; `faceoff` Kind `won` / `lost` is read as "Who's
team won / lost the draw" (R-0013 2 does not say whose perspective); `knockdown` Kind `check` / `boost` from the header comment in `JamEventBus.h`.

**Events with no line on purpose:**
- `bark`, `signal`: the players' own voices and the human's team signals; the booth does not narrate the bench.
- `poke` / `lift` with no Kind, `effect_start` per kind: covered by `pickup_take` lines (the effect starts on the same frame as the take).
- The penalty ending (the fouler's 300 frames run out): no event, and not worth one.
- `faceoff` Kind `lost` from a clean draw: the winning side's `won` line covers it.
- `announce`: one generic line set only; the rule's text is free-form and shown on the scoreboard.
