# Lane A169, track F0 (the design eye), REVIEW-3 (device A, Fable)

You are lane A169 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (the four laws: your yardstick), the track's `docs/design/REVIEW-2.md` with its scripts under `docs/design/review-2/` (same seeds, same
measures: this review is its after), R-0056 to R-0062 at the end of `contracts/RULINGS.md`, `docs/PLAY-ME.md` (what the operator reads at 10:00: it was
rewritten at 05:35 for this commit) and the lanes' accounts of the night: `docs/experiments/CURVE-9.md`, `CONTACT-6.md`, `CONTACT-7.md`, `MOVEMENT-6.md`,
`MOVEMENT-7.md`, `MOVEMENT-8.md`, `BRAIN-4.md`, `MODES-6.md`, `SCREEN-2.md`, `ARENA-2.md`, `PROFILING-7.md`, `FUZZ-2.md`.

Your worktree and the commit it stands on are named in the Director's message that starts you: the track's worktree moved to **the commit package H28 is
cut from: the build the operator plays at 10:00**. It was built hours ago on an older commit: build the editor first. Your status file: `status/A169.md`.
Port 4769. Family: `docs/design/**` only. **You change no source, no cartridge, no test.** A fault is a finding with its file, its line and its track.

## REVIEW-3: what the operator will meet at 10:00, said before he meets it
REVIEW-2 (01:30, 7,312 matches) found the level never reached the match, a poke that took the puck 4 % of the time and tripped the poker, the whistle in
seven modes, opponents scoring from the red line, the human on the ice 8 % of play. Four hours and about twenty landings later:
1. **REVIEW-2's tables on the same seeds, now**: one table, before and after, with the lane that moved each number. In particular: the poke's take
   rate and the human's own-poke stumbles (the bar was 15 % and under 1 a match), fouls in the modes, the first minute, the human's time down by cause,
   the carry's length, the puck on nobody's blade, where the opponents shoot from.
2. **The four laws, one by one**, in real matches (not the drills), as REVIEW-2 did.
3. **The first ten minutes of a newcomer at ROOKIE through PLAY NOW** (the level reaches the match now), then pro: wins, shots, knockdowns, the first use
   of each verb, where he gets stuck. Then the season's takes 1 to 5.
4. **The twelve modes, one line each**: is each a different game from the regular one, and winnable (the modes lane's table is in `MODES-6.md`: check three
   of its rows yourself on fresh seeds, Fog of War among them: it was kept at three a side on purpose).
5. **Ten things, ranked**: what the operator and a judge will feel in five minutes that is still wrong, each with its number, its owner track and the
   smallest change that would fix it before Sunday 22:00. Then **five things that are now good** and must not be broken.
6. **`docs/PLAY-ME.md`, line by line**: any sentence false of this build, with the measurement that says so. The Director corrects the sheet before 09:30
   from this section: write it FIRST into the status file as soon as you have it, even partial.
7. **What is switched off and waiting** (the push-out round a downed skater, the check's cooldown and lunge, the half-second screen, the pickup that
   settles, the stop's grace): from the lanes' own cells, which two would you turn on first for Sunday's build, and what the curve must pay for them.

Write `docs/design/REVIEW-3.md` (the PLAY-ME verdict and the ranked ten first) with your scripts and data under `docs/design/review-3/` (bulk raw output in
a `raw/` folder that does not travel). Few processes, many seeds; `-Multiprocess` on every headless start; check memory before each batch (start nothing
new over 50 GB in use). **Hard times: the PLAY-ME verdict in the status file by 08:15; the review's top (verdict and ranked ten) by 09:15.** Time box 150 minutes.
