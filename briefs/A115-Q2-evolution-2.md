# Lane A115, track Q2 (the brain), evolution 2 (device A, Fable)

You are lane A115 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full.**
Then `docs/EVOLUTION.md`, `docs/VISION.md` (law 2: the hockey brain), R-0048 point 5 and R-0049 at the end of `contracts/RULINGS.md` (from the main checkout), the
design eye's `docs/design/REVIEW-1.md` (main checkout; the sections on the brain), lane A107's soak `docs/experiments/SOAK-2.md` section 2 (copy at
`C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a107\docs\experiments\SOAK-2.md`), and the track's first evolution in your worktree: `status/A105.md`,
`docs/experiments/REVIEW-CODE.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a105` (the track's: evolution 1 is committed there as the baseline and is landing; its editor is built).
Your status file: `status/A115.md`. Port 4715. Family: `JamAICore.cpp/.h`, `JamBrain.cpp`, `JamSimAdapter.cpp`, the bot tests, `docs/experiments/BRAIN-2.md`. The sim's
rules are not yours (tracks F1 and F2 are changing contact and the stick tonight): read what they expose, change nothing there.

## Evolution 2: the brain plays hockey with the human
Three measurements say the bots play around the human, not with him:
- On big ice the human goes 30 s without touching the puck in 71 % of matches (large rink, regular class at pro; wide rink 91 %); during those stretches the
  opposing side holds the puck 73 to 81 % of the time and **a teammate 1 to 2 %**.
- The bots brake by reversing 670 to 2,330 times a match (at pro 2,146) and never sprint with the puck.
- Mates hold off a loose puck when the human lies next to it, and the passer chases its own pass (the track's own findings, severity 2 and 3).

Do, in this order, each with a test that fails first:
1. **Mates pass to the human where he has room** (the space read already scores a lane and a receiver: the human is a receiver like any other, with a small
   preference when he is open and ahead of the play), and **the human's side supports him**: a mate offers a lane when he carries.
2. **The chaser is picked by where the puck will be**, the yield next to a downed human ends, the passer is not the chaser of its own pass.
3. **The brain brakes on purpose**: the reverse brake is a decision (to stop in front of a defender, to hold the blue line, to turn back for a loose puck), not the
   by-product of steering; and a bot carrier sprints on an open lane.

Proving numbers (few processes, many seeds; `-Multiprocess` on every headless start):
- the share of matches with a 30 s no-touch stretch at pro on the large rink from 71 % to under 35 %; on hockey.regular not above today's 6 %;
- a teammate's share of the puck during the human's idle stretches from 1-2 % to 15 % or more;
- reverse brakes per match at pro from 2,146 to under 500;
- passes completed to the human per match (before and after);
- the three target cells (600 a cell). More than 4 points off in a cell is expected here (the human's side gets better): report them and do not tune the levels:
  the curve track re-sets.

The gate: the full `Jam` filter green; the rules untouched (the hash of a match with no bots unchanged). Time box 120 minutes; interim in the status file.
