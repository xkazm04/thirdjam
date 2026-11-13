# Lane A108, track S2: S2-acceptance-matrix (device A, evolution 1, Sonnet)

You are lane A108 of the Third Jam (team Hokej, Unreal 5.9 repo at `C:\Users\kazda\kiro\thirdjam`). **Read `briefs/EVOLUTION-COMMON.md` first: its rules are yours in full**
(how a track works, the machine's band and the cursor, the walls, the status file, the report with the next three evolutions). Then `docs/EVOLUTION.md` and `docs/VISION.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a108` (the Director created it at the commit that carries the whole vision: contact by inertia, the
movement laws, the hockey brain, the ladder of R-0046). Your status file: `status/A108.md`. Your remote-console port if you run a game: 4708.

## Track S2: the acceptance matrix (you own the cursor and the game windows)
Family: `tools/qa/accept/**` (drivers and cuts; the drill's source belongs to track F2), `tools/media/**` for stills, `docs/experiments/ACCEPT-MATRIX.md`. No source edits: a red
row is a finding with its run folder.
First evolution: **the acceptance run on everything a player can pick.** Wait until `status/A.md` on the main checkout says H26 is packaged (the Director's release run uses the
cursor first). Then, on a Development package of your worktree's commit (build it): (a) the acceptance run three times on hockey.regular: the same verdict each time or the
flaky row named; (b) the run on each of the twelve mode cartridges and on the five look tiers (`-Cut` with a copy that sets the mode or the tier): which rows a mode's rules
change by design (Pond has no checks; Sniper's net is tall) and which are real failures; (c) the Shipping zip H26: the clean-profile test with lane A88's
`tools/media/clean-profile.ps1`, 120 s alive in a match; (d) twelve stills at 1080p for the page from the runs (a faceoff at 5 v 5, a head-on hit, a stumble, the stop-and-go, a
pass play, the menu's twelve cards, an ability), named for `submission/shots/`. Read the stills yourself (the image service has no credits) and say in one line what each shows.

Time box 90 minutes for this evolution. Interim in the status file at 40 minutes.
