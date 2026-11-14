# Lane A148, track F0 (the design eye), REVIEW-2 (device A, Fable)

You are lane A148 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/VISION.md` (the four laws: your yardstick), your track's first review `docs/design/REVIEW-1.md` with its scripts under `docs/design/review-1/` (same
seeds, same measures: this review is a before and after), the rulings it caused and what followed (R-0048 to R-0056 at the end of `contracts/RULINGS.md`),
the last board in `docs/EVOLUTION.md`, `docs/PLAY-ME.md` (what the operator is told to try at 10:00 this morning), and the lanes' own accounts of what
changed: `docs/experiments/INTEGRATION.md`, `CURVE-7.md`, `MOVEMENT-4.md`, `CONTACT-4.md`, `BRAIN-2.md`, `MODES-4.md`, `SOAK-3.md`, `SCREEN.md`, `MENU.md` if present.

Your worktree and the commit it stands on are named in the Director's message that starts you: a **fresh worktree at the commit package H27 is cut from**
(the ladder re-set, the whistle out, the first look clay, the whole game integrated). It has no binaries: build the editor first. Your status file:
`status/A148.md`. Port 4748. Family: `docs/design/**` only. **You change no source, no cartridge, no test.** A fault is a finding with its file, its line and
the track that owns it.

## REVIEW-2: the game the operator sits down to at 10:00, measured against the vision and against REVIEW-1
REVIEW-1 (Friday 19:30, 924 matches) found that nobody kept the puck (the human's carry lasted a median 0.87 / 0.75 / 0.67 s at rookie / pro / all_star, the
puck was on nobody's blade 60 % of play), that the whistle ruled a game sold as whistle-free, that take 3 was a wall, that the bots braked by accident
thousands of times and never sprinted with the puck. Fourteen hours and about forty landings later:
1. **The same measurements on the same seeds**: every number of REVIEW-1's tables, now. One table, before and after, with the lane that moved each.
2. **The four laws, one by one**: does the build keep each? Law 1 (a hit obeys momentum): the three outcomes by closing speed in real matches. Law 2 (the
   brain knows hockey): passes to the human, the human's idle stretches, shot selection (the lockstep lane logged 273 bot shots in a match, many from the
   corners, one goal). Law 3 (speed change and the stick beat a defender): how often a human-class player gets past a defender in a real match and by what
   (not the drill: matches). Law 4 (sprint and the puck's distance): is the sprinting carrier easier to rob?
3. **The first ten minutes of a newcomer** on this build: PLAY NOW or take 1 at rookie through take 4: wins, shots, knockdowns taken and given, the first
   time each verb is used, where he gets stuck. The season's ramp (the curve lane measured a novice at 86 % on take 3 and 14 % on take 7).
4. **Ten things, ranked**: what a first-time player and a judge will feel in five minutes that is still wrong, each with its number, its owner track and the
   smallest change that would fix it before Sunday. Then **five things that are now good** and must not be broken in the last day.
5. **A verdict on `docs/PLAY-ME.md`**: is any line of it false of this build? Say which.

Write `docs/design/REVIEW-2.md` (the ranked ten first: the Director reads the top of the file at 09:00) with your scripts and data under
`docs/design/review-2/` (bulk raw output in a `raw/` folder that does not travel). Few processes, many seeds; `-Multiprocess` on every headless start; check
memory before each batch (start nothing new over 50 GB in use). Time box 120 minutes: write an interim top-ten into the status file at 60 minutes.
