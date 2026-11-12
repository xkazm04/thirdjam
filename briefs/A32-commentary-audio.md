# Lane A32: the commentator player, the staged VO bank, music and skate loops, seven missing events (device A, from H11.4)

You are lane A32 of the Third Jam (team Hokej). Ruling R-0027 point 4. Read first: `handoff/B-to-A-0026.md` (after the dev-b merge; else `git show origin/dev-b:handoff/B-to-A-0026.md`,
read-only), `briefs/commentary/README.md` (B's rules for the commentator, on main after the merge), `Game/Plugins/JamAssets/Content/Audio/VO/commentary.json` (340 lines: id, event, kind,
team, priority, cooldown, when, text, sound path, seconds), `Game/Source/JamGame/Private/{BarkSystem,EffectsBinder,HokejGameMode}.cpp` and `JamBarkSystem.h` (how the barks pick a line
and how the binder plays MetaSounds with inputs), `contracts/events.md` (C-0003), `tools/barks/filter.mjs` (the content filter: the VO text passes it too), R-0021 point 2
(hockey first), R-0027.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a32` (detached at the merge commit; build it yourself; editor on MCP port 8018 only while proving; at most three
editors on the machine). Run no git command; never touch `Game/Plugins/JamAssets` (B's bank and sounds); never kill a process you did not start.

You own: `Game/Source/JamGame/{Public,Private}/CommentarySystem.*` (new), `JamCommentaryData.*` (the manifest loader), one hook each in `HokejGameMode.cpp` (loops and the new events)
and `JamSim.cpp`/`JamEventBus` for the seven events, `Game/Config/DefaultGame.ini` (cook and stage lines), `tools/commentary/**` (a manifest check), `docs/experiments/COMMENTARY.md`,
`status/A32.md`.

## Job
1. **Stage and cook**: `+DirectoriesToAlwaysCook=(Path="/JamAssets/Audio/VO")` and `commentary.json` staged as a non-UFS file (or embedded as a data table generated from the JSON
   at build time by `tools/commentary/embed.mjs`, the way `tools/barks/embed.mjs` embeds the bark bank: the Director prefers the embed, no staging rule to forget); a node check
   that every line's sound path exists and its text passes the bark filter.
2. **The commentary player** beside the bark system: on an event, lines whose `event` matches, `kind` empty or equal to the payload's, `team` any or the event's, `when` holds;
   highest `priority` wins; a cooldown per situation (the id without its last number); never the same line twice running; one line at a time, at most one per 2.5 s, a late
   quick-play line is dropped, never queued; the 53 stoppage lines (`period`, `period_end`, `season_take`, `win`, `lose`, `goal.pause.*`) only in a pause; the commentator yields to
   a bark addressed to the human. Play through `MS_commentary` (wave input `Line`; it ducks `SC_Music` by 7 dB). Sim-frame timing, nothing from delta time. `game.commentary
   { enabled, intensity: quiet | normal | loud }` in the cartridge like the barks (schema, limits, README, DEFAULTS; Designer data regenerated with
   `node Game/Source/JamDesigner/Tools/gen-designer-data.mjs`); console `jam.Commentary -1|0|1`, `jam.say_line <id>` for proofs. Log line per play: `JAMVO id=<id> event=<e> frame=<f>`.
3. **Seven events** the bank has lines for: `save` (the last man's blade or body stops a shot on goal), `post` (puck hits a post, with `MS_post`), `breakaway` (carrier past every
   defender with the net ahead), `carry` (carrier keeps the puck 5 s), `period_end`, `clock` (last 10 s of a timed period), `season_take` (a season match starts). Each as a sim
   event in C-0003 terms (name, Kind, Who, Where, Strength), on the bus, appended to `contracts/events.md` under "Proposed, for the Director"; the hash stays unchanged (events
   are read-outs).
4. **Loops**: `MS_music_match` from match start, `Level` 0.35 in play, 0.6 in pauses and menus, smoothed by the sound; `MS_skate_loop` on the human's skater with `Speed` and
   `Turn` per B-to-A-0007 point 4.
5. Proof: a 60-second bot match with the log showing JAMVO lines spaced at least 2.5 s, a stoppage line only in a pause, no overlap with a human-addressed bark; `jam.say_line`
   on five ids; an eyes-free item (audio): record the match audio with the editor's movie capture or note the log only. Tests: Jam.Commentary (selection rules on a fake
   manifest, headless), node tests for the embed and the check.

Report: the rules as implemented, the seven events' definitions, the proof log excerpt, the file list. Time box 2 hours; interim at 45 minutes with lines playing in PIE.
