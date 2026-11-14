# Lane A211, track E1 (the eye), the night: play it as he will at 10:00 and say what is wrong (device A, Fable: judgement by looking)

You are lane A211 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md`, then `briefs/NIGHT-COMMON.md`: their rules are yours in full.**
Then **R-0067 and R-0068** at the end of `contracts/RULINGS.md` (what the operator said after playing H34: "effectively unplayable", and what he ordered
for the night: the weaknesses turned into strengths, a publishable product judged at 10:00), `docs/PLAY-ME.md` (what he was told to try), and the
status files of the five lanes that change the game tonight (A206 the brain, A207 the pass and the shot, A208 possession, A209 hits that land, A210
the hits' animation: in `status/` on main as far as they are landed, the live ones in `.claude/worktrees/{q2b,f2b,f0b,f1b,f3c}/status/`).

Your worktree and the commit it stands on are named in the Director's message that starts you. Status file `status/A211.md`. Port 4811. **You change
no game source.** Yours: capture tools under `tools/qa/eye/` (new; copy the screen track's drivers `tools/qa/screen/a196-strips.ps1` and its python
helpers: do not edit theirs), `docs/experiments/EYE.md` with its stills, your status file. Off screen only, every run with
`-dpcvars=r.RDG.AsyncCompute=0`; you do not own the cursor; two heavy processes at most.

## Why this lane exists
Yesterday every number was green and the operator found the game unplayable in his first minute. Five lanes now measure their own piece with
scripted hands. **Nobody looks at the whole game the way he will.** You are that look: a critic with a player's eyes, not a lane's pride. Your
product is a list a lane can act on within the hour, ranked by what would make him say "unplayable" again.
1. **Play what he plays** (first 40 minutes, then again after every landing on main: the Director moves your tree and tells you): PLAY NOW at
   ROOKIE on the regular rink, the game camera, the scripted human with a person's hand (lane A209's `JAM_PLAYTESTER_HAND=person` when it is in your
   tree; before that the regular script), full matches: a still every 20 frames for the first two minutes, then every 60; and frame-by-frame
   strips (every sim frame, 3 seconds) of: his first touch of the puck, his first pass, a pass he receives, his first shot, a goal for and against,
   a body check he lands, one he takes, a cross-check, a held brace, a loose puck he skates onto, a scramble in front of a net, the bots' breakout,
   a faceoff. Then the same at PRO.
2. **Read the pictures as a stranger** and answer, each with the frames that show it:
   - In the first ten seconds: does it look like hockey? Who has the puck: can he tell at a glance, every frame? Where is the puck when he carries it?
   - Do ten skaters want the puck and the net (count: skaters moving toward the play, standing, facing away), frame by frame through one attack?
   - A pass: does it leave toward the man, arrive, and is it taken with a pose? A shot: does it look fast, and does anything stop it?
   - A hit: who hit whom, from the game camera, in real time (step through at 60 Hz: how many frames does the eye get)?
   - What on the screen is noise (marks, rings, lines, labels, text that covers the play) and what is missing (a cue for the pass's power, for
     possession, for a hit landed)?
   - Anything that reads as a bug: a slab, a rectangle, a figure sliding without moving its legs, a puck inside a body, a skater facing backwards,
     a stick through a body, a flash, a camera jump, a label that lies.
3. **The list**: `EYE LIST n` in your status file after each pass: at most twelve items, each: what a person sees (one sentence), the still or strip
   that shows it (file), how often (of how many situations looked at), whose it is (lane and file if you can name it), how bad (he stops playing /
   he notices / polish). The Director sends each item to its lane.
4. **Verify fixes**: when the Director says a fix landed, look again at the same situation and write `SEEN FIXED` or `STILL THERE` with the still.
5. **A minute of footage for the page** (after 06:00, only if the game then deserves it): the twelve best stills of a match, from the game camera,
   with what each shows (for `submission/`: do not write there: list them in your status file).

`EYE LIST 1` within 50 minutes of your start; a new list after every move of your tree; `EYE READY` at 09:15 with the standing list: what is fixed,
what remains, and your honest sentence on whether a stranger would play a second match. Time box: the night.
