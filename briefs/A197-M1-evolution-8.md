# Lane A197, track M1 (the modes), evolution 8 (device A, Opus)

You are lane A197 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then **R-0064** at the end of `contracts/RULINGS.md` (the operator's words after playing; points 2 and 8), the track's `docs/experiments/MODES-6.md` and
`MODES-7.md` with the track's last status file (the twelve modes, their win rates this morning), `LookTierSwitcher.cpp/.h` and `RinkActor.cpp` (the look
tiers: clay, night, the neon tier as it is), `handoff/A-to-B-0027.md` point 4 with device B's answer (device B builds `/JamAssets/Rigs/LR_N1_Neon` and
names the tier: target 17:45), and the speed lane's `docs/experiments/MOVEMENT-10.md` (free skaters 15 % faster: every mode plays on it now).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A197.md`. Port 4797. Family:
`Game/Content/Cartridges/modes/**`, the modes' table in the menu's data (names, one-liners, players a side: not the menu's layout), the look tier's
selection per mode, A's presentation code for the neon mode (`RinkActor.*` light lines, trails in `EffectsBinder.cpp`: its own functions only: the hits
lane edits the binder today), the modes' tests, `docs/experiments/MODES-8.md`. Never `Game/Plugins/JamAssets`. No brand, film or game name or look.

## Evolution 8: one mode goes to the grid, and twelve modes survive the speed rule
His words: "Exercise in one director's mode further concept of neon Tron like arena". The ruling: one mode (the Director's pick: **Glass House's slot**,
renamed by you: a plain name of two words, nobody's mark: propose three, take the first unless the Director objects) becomes the neon mode: a dark
arena drawn in light. "Tron-like" is a mood (dark floor, a grid, lines of light, light trails), never a copied design, vehicle, suit or logo.
1. **What the neon tier is today** (20 minutes): three stills from the game camera, the frame time against clay (the capture driver's numbers), what
   reads badly (the puck? the lines? team colours?).
2. **A's half of the grid arena** (presentation code, the default match untouched): a dark sheet with a faint grid that brightens near the puck, the
   boards and the rink's lines as lines of light in the two team accents by half, each skater leaving a short light trail in his team's colour that
   fades in about half a second (a ribbon or decals: the cheapest that holds 60 frames a second with ten skaters: say the cost), the puck the brightest
   thing on the sheet with its own trail, goals: the net's frame lights and the grid pulses once. The radar and HUD keep working; team colours stay
   distinguishable for a colour-blind eye (shape or brightness as well as hue: say how).
3. **Rules that fit the look** (cartridge keys that exist: read `schema/README.md`): the mode should play differently too: propose two rule sets from
   existing keys (for example faster puck and fewer whistles; boosts on) and take the one that measures better: a regular player wins 35 to 65 %, goals
   a match between 4 and 12.
4. **Device B's rig**: when the Director says `LR_N1_Neon` is on main, the mode selects it by name with a fallback to today's tier; stills before and after.
5. **Twelve modes on the faster skaters**: 100 seeds a mode for a regular (pro) player with the playtest instrument: the table (win rate, goals a
   match, match length); every mode outside 15 to 85 % gets one number moved in its cartridge and is measured again. The newest main also carries the
   hits' wind-ups (4 frames) and the breakaway's bonus 0.3: say which modes they touch.

Write `GRID READY` (items 1 to 3: files, six stills, the frame time) **by 17:00 for the 18:30 package**, `MODES TABLE` (item 5) by 17:30, `RIG READY`
after item 4; built, the full `Jam` filter green, Shipping compiles, before each; `SOURCE OPEN` before any later source change. `-Multiprocess`; three
measurement processes at most; stop only processes whose ids you recorded. Time box 220 minutes.
