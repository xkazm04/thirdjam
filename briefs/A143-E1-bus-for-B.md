# Lane A143, track E1: the bus tells device B what it needs (device A, Opus)

You are lane A143 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `contracts/events.md` (the event bus: what each event carries: Who, Kind, Variant, Strength, Dir), `contracts/skater-visual.md`, the ability rule's doc
(`docs/experiments/ABILITIES*.md` or the section in `docs/experiments/MODES*.md` that describes `game.abilities`), `docs/experiments/COMMENTARY.md`, and device
B's two answers that ask for this: the Director's copy is `C:\Users\kazda\AppData\Local\Temp\tj\B-answer-0024.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the newest main**. It has no binaries:
build the editor first. Your status file: `status/A143.md`. Port 4743. Family: the event emission for `ability` and `check` (where the sim's events are turned
into bus events: `JamEffects*`, `EffectsBinder.*`, the event structs), the commentary bank's source and embed (`briefs/commentary/gen-lines.mjs`,
`tools/commentary/**`, `Game/Content/Commentary/**`), their tests. Not yours: the sim's rules, the figure, anything under `Game/Plugins/JamAssets`.

## Evolution 1: three things device B's sounds and figures wait for
Device B recorded all 412 commentator lines tonight and built the ability, deke, stumble and brake sounds. Three things on A's side keep them from playing right:
1. **The ability's kind travels as `Variant`** on the `ability` bus event: `shotgun` 0, `hip_check` 1, `shield` 2, `slow_puck` 3 (B's `MS_ability` and
   `NS_ability` read it; until it arrives every use plays the shotgun). A grant (the ability is given, not used) stays Strength 0 and is told apart from a use.
   The hip check's thump sits 0.18 s into its file, so its event must fire on the press frame, not on the hit: check when it fires today and say.
2. **A stumble is told from a hit on the bus**: today `check` arrives with Strength under 0.001 for a stumble and B guesses (3 of 4 right). Give the `check`
   event `Variant` 4 for a stumble (0 to 3 keep whatever they mean today: read the contract), or a `stumble` event: choose the one that changes least for every
   existing listener (the commentary, the barks, the Director's trigger, the arena) and prove none of them changes its count on a pinned match.
3. **`lose.blowout.01`**: B re-recorded it with a hockey line ("A night to forget. Back to practice.", 2.75 s). A's embedded copy of the bank and
   `briefs/commentary/gen-lines.mjs` line 164 still hold the old text: bring A's sources and the embed in line with B's `Audio/VO/commentary.json` for that id,
   and check the whole bank the same way: every line id's text on A equals B's (B's file is on main after the Director's next merge; if it is not in your tree
   yet, do the one id and write the comparison script for the rest).
4. **The caption's time**: 34 of B's 72 new lines run over 2.5 s and 15 over 3 s: does A's caption stay up for the line's length (the wave's duration) or a
   fixed time? If fixed, a caption should not vanish before its line ends, and two lines must not overlap: say what happens today and fix what is in your family.

Proposed contract text for `contracts/events.md` (the Director writes it): the `ability` event's Variant table and the stumble's mark. Nothing here enters the
sim: the lockstep hash and every pinned match are unchanged (prove it). The gate: the full `Jam` filter green. `-Multiprocess` on every headless start.
Time box 80 minutes; interim in the status file.
