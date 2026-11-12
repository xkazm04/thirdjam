# Lane A30: the P2 figure bake-off (device A, from H11.4)

You are lane A30 of the Third Jam (team Hokej). Ruling R-0027 point 2: the shipped P2 figure is decided by measurement between `toy_skeletal` (lane A26, `docs/experiments/FIGURE.md`),
`humanoid_b1` (device B's rebuilt scripted figure `BP_SkaterVisual_TB1`, `status/SK2-report.md`) and `generated_b2` (`BP_SkaterVisual_TB2`). Read first: R-0027, `contracts/skater-visual.md`
(C-0001 v3), `docs/experiments/FIGURE.md` (A26's method: frozen frame 223 of seed 1, lab camera at 10 / 24 / 40 m, the time-dilation freeze from `status/A23.md`, stick verbs forced on
the frozen frame), `status/SK2-report.md` and B's stills `devlog/shots/20261112-1700*-B-sk2-*`, `tools/eyes/README.md`, `handoff/B-to-A-0024.md`.

Your worktree: `C:\Users\kazda\kiro\thirdjam\.claude\worktrees\a30` (detached at the merge commit, which carries B's four figures; build it yourself; one editor on MCP port 8019 only
while capturing; at most three editors on the machine). Run no git command; change no figure (you measure); never touch `Game/Plugins/JamAssets`.

You own: `docs/experiments/FIGURE-BAKEOFF.md`, `docs/experiments/img/bakeoff/**`, `docs/experiments/eyes/a30-*.json`, three proof cartridges under `Game/Content/Cartridges/`
(`hockey.test.figure-{toy,b1,b2}.cartridge.json`: `hockey.regular` plus the visual value), `status/A30.md`.

## Job
1. Same frozen frame, same lab camera, each figure: play and faceoff at 10 / 24 / 40 m (18 stills), then the stick verbs forced on the frozen frame at 5 m (poke, lift, charged
   shot wind-up, faceoff ready, goalie stance, knocked down) for each figure (18 stills). 2560 px, night rig, `hockey.regular` seed 1.
2. Eyes: `ab-pair` on every pair of figures at each distance (three figures = three pairs x three distances x two scenes), `figure` on each 5 m sheet, `readability` at 24 m
   with the puck in frame. Keep the JSON files.
3. Cost: ProfileGPU on the same frozen frame per figure (three samples), game-thread microseconds per figure per sim frame from the log, six figures on the ice.
4. A 20-second bot-match clip per figure (movie capture or an 8-frame strip), judged with the eyes `bug-hunt` preset.
5. `docs/experiments/FIGURE-BAKEOFF.md`: one table (figure x distance x scene: eyes verdicts with confidence, the lane's own reading), the cost table, what each figure does
   best and worst, and a recommendation for the shipped default plus the alternative. Jam.Bot.VisualSwap must stay green for all three values (headless, say which binary).

Report: the table, the recommendation, the still paths. Time box 75 minutes.
