# Lane A217, track C0 (the curve), the night: the ladder is read through a person's hand, then set (device A, Opus)

You are lane A217 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0066 to R-0068** at the end of `contracts/RULINGS.md`; the track's last set (`status/A189.md`: `LADDER SET 2`, `docs/experiments/CURVE-12.md`,
the drivers under `tools/perf/results/a189/`) and the freeze check (`briefs/A205-*.md`, what it found is in R-0066); the brain lane's
`docs/experiments/BRAIN-9.md` and `status/A206.md` (its six cells tonight and "one lever per level" when its READY 4 has it); the contact
track's person-like hit hand (`Game/Source/JamCore/AI/JamHitHand.h`, `JamPlaytester.cpp/.h`); and the critic lane's scripted hand
(`Game/Source/JamGame/Private/ScriptHuman2.h/.cpp`, `tools/qa/eye/`, and in `.claude/worktrees/a108/status/A211.md` the sections
"EYE TOOL READY", "EYE TOOL GATED 2", "The pass counts re-taken" and its seven-seed tables).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A217.md`. Port 4817.
Family: `tools/perf/results/a217/` (copy the track's drivers from `a189/`: do not edit theirs), the ladder's own values (the level cartridges'
`ladder/*` and `ai.*` keys the earlier sets moved: read CURVE-12 for which; NOT the brain's code: `JamAICore.*` and `DifficultyProfile.cpp`
are lane A206's), a person's-path arm for the headless playtester (new files; where an existing file of another lane must gain a call or a
flag, keep it to a few lines and name them in your READY: `JamPlaytester.*` is lane A215's, `ScriptHuman2.*` lane A211's), tests
`Jam.Curve.*`, `docs/experiments/CURVE-13.md`. Headless (`-nullrhi`); four processes at most until 04:30, then as the Director says.
Nothing of yours runs from the main checkout.

## Why this lane exists tonight
The ladder (rookie 85 %, pro 48 %, all_star 26 % wins for the player the level is meant for, within 3) was set last evening with a
playtester that SENDS VERBS to the sim. A person presses buttons: LMB / RMB resolved by possession, a pass by tap or hold, a shot charged by
hold, an aim that follows a mouse. Since tonight's rules (the pass by power, the pickup by touch, hits that find their man, the doubled shot)
the two differ by a class: the critic's newcomer hand, through the button resolver, wins 1 rookie match in 7 where the ladder says 85 %; the
regular hand at pro goes 3:0, 0:3, 2:3, 0:1, 1:1, 3:0, 1:3 over seven hand seeds; one held aim on one pass turned a 2:3 loss into 3:0. And
the brain lane's verb-instrument cells collapsed with the pickup (pro 34.3 regular / 7.2 large, all_star 9.7 / 2.7). **The Director's
ruling: the ladder is read through the person's path.** The operator plays at 10:00; the level HOME tells him to start on must be winnable
and must not feel absent ("a level is easy by clumsiness, never by absence").

## Phase 1, now to about 04:30: the instrument
1. A headless playtester arm that plays as the critic's `jam.ScriptHuman 2` does (the same decisions, the hit hand, presses through the
   button resolver with the shipped bindings, taps and holds by the thresholds, the aim held through a release) at the speed of the track's
   cell runs (thousands of matches: not real time). Cheapest true way first: if `ScriptHuman2`'s path can run under the commandlet the cells
   use, do that; if the resolver must be reachable from JamCore, move nothing: call it from a JamGame-side commandlet of your own.
   Determinism: the same cartridge + seed + hand seed = the same state hash, twice.
2. **Reproduce the critic's tables** on the same build it used (it names the build and the hand seeds in its status file): rookie newcomer
   seven matches (3:1, 0:2, 2:2, 1:3, 1:1, 1:3, 2:2) and pro regular seven. If yours differ, find why before anything else (ask under
   `ASK A211` in your status file; it answers in its own).
3. On your tree's main: the six cells (rookie / pro / all_star x regular / large) at 300 seeds with BOTH instruments (verbs, buttons): one
   table, the gap per cell; and where the button hand loses its matches (goals against by shot distance, passes arriving, time on his back,
   time to the first goal against): three lines a level. Write `CURVE READY 1` with it.

## Phase 2, from the Director's word (about 04:30) to 06:30: LADDER SET 3
The Director moves your tree onto the main that has the night's rule changes (the pass solved for the mate and the narrower interception,
the long shot's fade, the faceoff rules, the own-net wall, the brain's wave 4, the pressed cross-check's live window) and says so.
4. Set the six cells through the person's path: rookie 85, pro 48, all_star 26 (within 3 at 600 seeds, confirmed at 2,000), using the
   levers the earlier sets used plus the brain lane's one lever per level. Rules: rookie is clumsy, never absent (no lever that makes bots
   stand still, skip their jobs or leave the net); law 3 stays the gate at pro and all_star (stop begun at 8 m at least 90 %, the straight
   charge 0: read it with lane A206's or the contact track's driver after every candidate); the neutral proof for anything that is a key.
5. Also read, and say plainly if out: no goal against inside the first 600 frames of a rookie match in more than 1 match of 20; the human
   on his back under 8 % of play at pro; the season's novice ramp (no step over 20 points from take 3 to take 7; 200 a take); the twelve
   modes with a regular hand (100 seeds each: every mode outside 15 to 85 % listed).
6. `LADDER SET 3` by 06:30 with the cartridge files by name, the table (both instruments, so tomorrow's reader can compare with SET 2), the
   full `Jam` filter green in a cleared environment, Shipping compiles.

## Phase 3, to 09:00
After each landing the Director names: the six cells again at 300 through the person's path; a cell more than 5 off is a `FAULT` line with
the lever that would bring it back.
