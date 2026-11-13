# Lane A122, track M1: the Director's modes under the vision (device A, evolution 1, Fable)

You are lane A122 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md`, R-0036, R-0041 and R-0048 to R-0050 in `contracts/RULINGS.md`, lane A107's soak `docs/experiments/SOAK-2.md` (sections on
modes and takes) and the modes' own measurement doc (`docs/experiments/MODES*.md`: how lanes A81 and A90 measured each mode's character against the regular game).

Your worktree and status file are named in the Director's message that starts you. Family: `Game/Content/Cartridges/modes/**` and the mode cards' text (`game.meta`:
pitch and facts), `tools/playtest/modes.mjs`, `docs/experiments/MODES-3.md`. Not yours: the sim, the brain, the difficulty levels (track C0 is re-setting the ladder on
the same build: a mode's bot level is a level name, never numbers of its own), the season's takes (C0's).

## Why
The operator's direction (R-0036): the Director's modes are "ways to change the character of the game for creative variations". Each mode was built to differ from
the regular game by at least a third in its character number. The vision changed the regular game under them (contact by momentum, the puck kept, five a side that
pass), and lane A107's soak on 200 seeds a mode found:
- **Cage** 0.86, **Outnumbered** 1.22, **Shotgun** 0.67 of the regular game: no longer a third apart (regular play now makes 17.8 knockdowns a match; Shotgun's 12.0
  is below it).
- **Marathon**: its 10:00 timer equals the harness cap, the match plays past its own horn (40,010 to 42,000 frames), and the regular class wins 1 %.
- The mode cards still quote the old contact rule ("checks from N m/s").

## Evolution 1: twelve modes, each a different game again
1. **Measure all twelve** on the build you are given (200 seeds a mode, regular class at the mode's level, few processes, `-Multiprocess`): the character number
   each mode claims, its win rate, its length, knockdowns of the human. One table.
2. **Re-make the three that faded** with the vision's own levers (`game.contact`: momentum, ratios, down frames, board bonus, release grace; `game.stick`:
   passive take, body shield, take grace; `game.skater`: burst, brake, sprint; `game.abilities`): Cage is the game of boards and bodies, Outnumbered is the game of
   keeping the puck under pressure, Shotgun is the one-shot ability on F (the operator's own example). Each differs from the regular game by a third or more in its
   character number and reads as that in one sentence on its card.
3. **Marathon ends on its horn** and a regular player can win it (at least 25 %): if the horn is a sim fault, it is a finding with the file and line, and the
   cartridge works around it.
4. **Every card says what the build does**: pitch and three facts per mode in the brand's voice (raw, brutal, teen rating: `docs/experiments/VOICE.md`), no number
   that the vision made false.
5. Any mode whose win rate for the regular class is outside 15 to 85 %: say so, with the lever you would turn; turn it if it is inside the cartridge.

Proof: the table before and after; the menu test still counts twelve cards; the cartridges pass both validators (`node schema/validate.mjs`, the loader's test);
the full `Jam` filter green. Time box 100 minutes; interim in the status file.
